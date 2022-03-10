#include "instrumentation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/ProfileData/InstrProf.h"
#include <cstdlib>

using namespace llvm;

namespace {

// Do not instrument known races/"benign races" that come from compiler
// instrumentation. The user has no way of suppressing them.
static bool shouldInstrumentReadWriteFromAddress(const Module *m, Value *addr) {
  // Peel off GEPs and BitCasts.
  addr = addr->stripInBoundsOffsets();

  if (GlobalVariable *gv = dyn_cast<GlobalVariable>(addr)) {
    if (gv->hasSection()) {
      StringRef sectionName = gv->getSection();
      // Check if the global is in the PGO counters section.
      auto of = Triple(m->getTargetTriple()).getObjectFormat();
      if (sectionName.endswith(
              getInstrProfSectionName(IPSK_cnts, of, /*AddSegmentInfo=*/false)))
        return false;
    }

    // Check if the global is private gcov data.
    if (gv->getName().startswith("__llvm_gcov") ||
        gv->getName().startswith("__llvm_gcda"))
      return false;
  }

  // Do not instrument acesses from different address spaces; we cannot deal
  // with them.
  if (addr) {
    Type *ptrTy = cast<PointerType>(addr->getType()->getScalarType());
    if (ptrTy->getPointerAddressSpace() != 0)
      return false;
  }

  return true;
}

class RaceDetector{
 public:
  RaceDetector(Function &f);
  void sanitizeFunction();
  uint64_t getSkippedReads() {
    return this->skippedReads;
  }
  uint64_t getSkippedWrites() {
    return this->skippedWrites;
  }
  uint64_t getInstrumentedReads() {
    return this->instrumentedReads;
  }
  uint64_t getInstrumentedWrites() {
    return this->instrumentedWrites;
  }
  uint64_t getSkippedInstParamReads() {
    return this->skippedInstParamReads;
  }
  uint64_t getInstrumentedAllocs() {
    return this->instrumentedAllocs;
  }
 private:
  FunctionCallee checkRead;
  FunctionCallee checkWrite;
  FunctionCallee start;
  FunctionCallee alloc;
  Function *fptr;
  ItaniumPartialDemangler demangler;
  DenseSet<StringRef> nsBlackList;;
  DenseSet<StringRef> funcWhiteList;
  DenseSet<StringRef> ignoredFuncCallSet;
  uint64_t skippedReads;
  uint64_t skippedWrites;
  uint64_t instrumentedReads;
  uint64_t instrumentedWrites;
  uint64_t skippedInstParamReads;
  uint64_t instrumentedAllocs;
  void instrumentLoadAndStore(Instruction *inst, const DataLayout &dl);
  void instrumentProgramInput();
  void instrumentAlloc(CallBase *invokeAlloc);
  void chooseInstructiontoInstrument(SmallVectorImpl<Instruction *> &local, SmallVectorImpl<Instruction *> &all);
  int getMemoryAccessSize(Value *addr, const DataLayout &dl);
  StringRef getFunctionBaseName(StringRef func);
  StringRef getNameSpace(StringRef func);
};

RaceDetector::RaceDetector(Function &f) : fptr(&f), demangler(), nsBlackList(), funcWhiteList(), ignoredFuncCallSet(), 
                                          skippedReads(0), skippedWrites(0), instrumentedReads(0), instrumentedWrites(0), 
                                          skippedInstParamReads(0), instrumentedAllocs(0) {
  Module *m = f.getParent();
  IRBuilder<> irb(m->getContext());
  AttributeList attr;
  attr = attr.addAttribute(m->getContext(), AttributeList::FunctionIndex,
                           Attribute::NoUnwind);
  SmallString<32> readFuncName("asap_check_read");
  SmallString<32> writeFuncName("asap_check_write");
  SmallString<32> initialFuncName("asap_start");
  SmallString<32> allocFuncName("asap_alloc");

  checkRead = m->getOrInsertFunction(readFuncName, attr, 
                                     irb.getVoidTy(), irb.getInt8PtrTy(),
                                     irb.getInt32Ty());
  checkWrite = m->getOrInsertFunction(writeFuncName, attr, 
                                      irb.getVoidTy(), irb.getInt8PtrTy(), 
                                      irb.getInt32Ty());
  start = m->getOrInsertFunction(initialFuncName, attr, 
                                 irb.getVoidTy(), irb.getInt32Ty(),
                                 irb.getInt8PtrTy());
  alloc = m->getOrInsertFunction(allocFuncName, attr, 
                                 irb.getVoidTy(), irb.getInt8PtrTy(),
                                 irb.getInt32Ty());
  nsBlackList.insert({"hclib"});
  funcWhiteList.insert({"call_lambda"});
  ignoredFuncCallSet.insert({"asap_check_read"});
  ignoredFuncCallSet.insert({"asap_check_write"});
}

void RaceDetector::sanitizeFunction() {
  const DataLayout &dl = fptr->getParent()->getDataLayout();
  SmallVector<Instruction *, 8> localLoadsAndStores;
  SmallVector<Instruction *, 8> allLoadsAndStores;
  DenseSet<Instruction *> loadsForIgnoredFuncCall;

  {
    StringRef baseName = getFunctionBaseName(fptr->getName());
    StringRef contextName = getNameSpace(fptr->getName());
    bool ignore = false;
    if (nsBlackList.find(contextName) != nsBlackList.end() && 
        funcWhiteList.find(baseName) == funcWhiteList.end()) {
      errs() << "Ignored: " << contextName << "::" << baseName << "\n";
      ignore = true;
    }
    if (!contextName.empty()) {
      delete[] contextName.data();
    }
    if (!baseName.empty()) {
      delete[] baseName.data();
    }
    if (ignore) {
      return;
    }
  }

  errs() << "Instrument " << fptr->getParent()->getName() << "::" << fptr->getName() << "\n";
  for (auto &bb : *fptr) {
    for (auto &inst : bb) {
      // inst.print(errs());
      // errs() << "\n";
      if (isa<LoadInst>(inst) || isa<StoreInst>(inst)) {
        localLoadsAndStores.push_back(&inst);
      } else if ((isa<CallInst>(inst) && !isa<DbgInfoIntrinsic>(inst)) ||
                  isa<InvokeInst>(inst)) {
        CallBase *cb = cast<CallBase>(&inst);
        // instrument memory allocation
        if (cb->getCalledFunction()) {
          StringRef funcName = cb->getCalledFunction()->getName();
          if (funcName == "malloc" ||
              funcName == "_Znam" ||
              funcName == "_Znwm") {
                instrumentAlloc(cb);
              }
        }
        
        // if (cb->getCalledFunction()) {
        //   StringRef baseName = getFunctionBaseName(cb->getCalledFunction()->getName());
        //   if (ignoredFuncCallSet.contains(baseName)) {
        //     for (auto &param : cb->args()) {
        //       Value *source = param.get();
        //       if (isa<LoadInst>(*source)) {
        //         loadsForIgnoredFuncCall.insert(cast<LoadInst>(source));
        //       }
        //     }
        //   }
        //   if (!baseName.empty()) {
        //     delete[] baseName.data();
        //   }
        // }

        chooseInstructiontoInstrument(localLoadsAndStores, allLoadsAndStores);
      }
    }
    chooseInstructiontoInstrument(localLoadsAndStores, allLoadsAndStores);
  }

  for (Instruction *i : allLoadsAndStores) {
    if (loadsForIgnoredFuncCall.find(i) == loadsForIgnoredFuncCall.end()) {
      instrumentLoadAndStore(i, dl);
    } else {
      skippedInstParamReads++;
    }
  }

  if (fptr->getName() == "main") {
    if (fptr->arg_size() < 2) {
      errs() << "Please set main function's signature to \"int main(int argc, char *argv[])\"\n";
      assert(0);
    } else {
      errs() << "Pass input parameter to data race detector\n";
      instrumentProgramInput();
    }
  }
}

void RaceDetector::instrumentLoadAndStore(Instruction *inst, const DataLayout &dl) {
  IRBuilder<> irb(inst);
  bool isWrite = isa<StoreInst>(inst);
  Value *addr = isWrite ? cast<StoreInst>(inst)->getPointerOperand()
                        : cast<LoadInst>(inst)->getPointerOperand();
  isWrite ? instrumentedWrites++ : instrumentedReads++;
  FunctionCallee func = isWrite ? checkWrite : checkRead;
  int size = getMemoryAccessSize(addr, dl);
  assert(size > 0);
  irb.CreateCall(func, 
                 {irb.CreatePointerCast(addr, irb.getInt8PtrTy()), 
                  irb.getInt32(size)});
}

void RaceDetector::instrumentProgramInput() {
  // This method should only be invoked on main function
  assert(fptr->arg_size() == 2);
  IRBuilder<> irb(&fptr->getEntryBlock().front());
  Value *argc = fptr->getArg(0);
  Value *argv = fptr->getArg(1);
  irb.CreateCall(start, {irb.CreateIntCast(argc, irb.getInt32Ty(), true),
                         irb.CreatePointerCast(argv, irb.getInt8PtrTy())});
}

void RaceDetector::instrumentAlloc(CallBase *invokeAlloc) {
  BasicBlock *bb = invokeAlloc->getParent();
  IRBuilder<> irb(invokeAlloc->getParent());
  if (invokeAlloc->getNextNode()) {
    irb.SetInsertPoint(invokeAlloc->getNextNode());
    Value *size = invokeAlloc->getArgOperand(0);
    irb.CreateCall(alloc, {irb.CreatePointerCast(invokeAlloc, irb.getInt8PtrTy()),
                           irb.CreateIntCast(size, irb.getInt32Ty(), true)});
  } else {
    InvokeInst *invoke = cast<InvokeInst>(invokeAlloc);
    BasicBlock *next = bb->getNextNode();
    BasicBlock *newBB = BasicBlock::Create(bb->getContext(), "", bb->getParent(), next);
    irb.SetInsertPoint(newBB);
    Value *size = invokeAlloc->getArgOperand(0);
    irb.CreateCall(alloc, {irb.CreatePointerCast(invokeAlloc, irb.getInt8PtrTy()),
                           irb.CreateIntCast(size, irb.getInt32Ty(), true)});
    irb.CreateBr(next);
    invoke->setNormalDest(newBB);
  }
  instrumentedAllocs++;
  
}

void RaceDetector::chooseInstructiontoInstrument(SmallVectorImpl<Instruction *> &local, 
                                                 SmallVectorImpl<Instruction *> &all) {
  DenseMap<Value *, size_t> writeTargets;
  for (Instruction *i : reverse(local)) {
    const bool isWrite = isa<StoreInst>(*i);
    Value *addr = isWrite ? cast<StoreInst>(i)->getPointerOperand()
                          : cast<LoadInst>(i)->getPointerOperand();
    if (!shouldInstrumentReadWriteFromAddress(i->getModule(), addr)) {
      continue;
    }

    // If there is a write operation, all prior write and read operations in the BasicBlock are skipped
    // if (!isWrite) {
    const auto writeEntry = writeTargets.find(addr);
    if (writeEntry != writeTargets.end()) {
      if (isWrite) {
        skippedWrites++;
      } else {
        skippedReads++;
      }
      continue;
    }
    // }

    all.push_back(i);
    if (isWrite) {
      writeTargets[addr] = all.size() - 1;
    }
  }
  local.clear();
}

int RaceDetector::getMemoryAccessSize(Value *addr, const DataLayout &dl) {
  Type *origPtrTy = addr->getType();
  Type *origTy = cast<PointerType>(origPtrTy)->getElementType();
  assert(origTy->isSized());
  uint32_t typeSize = dl.getTypeStoreSizeInBits(origTy);
  if (typeSize != 8  && typeSize != 16 &&
      typeSize != 32 && typeSize != 64 && typeSize != 128) {
    // Ignore all unusual sizes.
    return -1;
  }
  return typeSize / 8;
}

StringRef RaceDetector::getFunctionBaseName(StringRef func) {
  size_t size = func.size();
  char *buf = new char[func.size()];
  if (!demangler.partialDemangle(func.data())) {
    return demangler.getFunctionBaseName(buf, &size);
  } else {
    delete[] buf;
    return StringRef{};
  }
}

StringRef RaceDetector::getNameSpace(StringRef func) {
  size_t size = func.size();
  char *buf = new char[func.size()];
  if (!demangler.partialDemangle(func.data())) {
    return demangler.getFunctionDeclContextName(buf, &size);
  } else {
    delete[] buf;
    return StringRef{};
  }
}

} // namespace

PreservedAnalyses InstrumentationPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  
  RaceDetector rd(F);
  // errs() << "Instrument " << F.getName() << "\n";
  rd.sanitizeFunction();
  // errs() << "Skipped reads: " << rd.getSkippedReads() << "\n";
  // errs() << "Skipped writes: " << rd.getSkippedWrites() << "\n"; 
  errs() << "Instrumented reads: " <<  rd.getInstrumentedReads() << "\n";
  errs() << "Instrumented writes: " << rd.getInstrumentedWrites() << "\n";
  // errs() << "Skipped inst-routine param reads: " << rd.getSkippedInstParamReads() << "\n";
  errs() << "Instrumented malloc/new: " << rd.getInstrumentedAllocs() << "\n";

  return PreservedAnalyses::none();
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "asap-instrumentation", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "asap-inst") {
                    FPM.addPass(InstrumentationPass());
                    return true;
                  }
                  return false;
                });
          }};
}
