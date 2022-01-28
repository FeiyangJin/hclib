#include "instrumentation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Attributes.h"
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
 private:
  FunctionCallee checkRead;
  FunctionCallee checkWrite;
  Function *fptr;
  ItaniumPartialDemangler demangler;
  SmallVector<StringRef,10> nsBlackList;
  SmallVector<StringRef,10> funcWhiteList;
  uint64_t skippedReads;
  uint64_t skippedWrites; 
  void instrumentLoadAndStore(Instruction *inst, const DataLayout &dl);
  void chooseInstructiontoInstrument(SmallVectorImpl<Instruction *> &local, SmallVectorImpl<Instruction *> &all);
  int getMemoryAccessSize(Value *addr, const DataLayout &dl);
};

RaceDetector::RaceDetector(Function &f) : fptr(&f), demangler(), nsBlackList(), funcWhiteList(), skippedReads(0), skippedWrites(0) {
  Module *m = f.getParent();
  IRBuilder<> irb(m->getContext());
  AttributeList attr;
  attr = attr.addAttribute(m->getContext(), AttributeList::FunctionIndex,
                           Attribute::NoUnwind);
  SmallString<32> readFuncName("asap_check_read");
  SmallString<32> writeFuncName("asap_check_write");
  checkRead = m->getOrInsertFunction(readFuncName, attr, 
                                     irb.getVoidTy(), irb.getInt8PtrTy(),
                                     irb.getInt32Ty());
  checkWrite = m->getOrInsertFunction(writeFuncName, attr, 
                                      irb.getVoidTy(), irb.getInt8PtrTy(), 
                                      irb.getInt32Ty());
  nsBlackList.append({"hclib"});
  funcWhiteList.append({"call_lambda"});
}

void RaceDetector::sanitizeFunction() {
  const DataLayout &dl = fptr->getParent()->getDataLayout();
  size_t size = 100;
  char *buf1 = static_cast<char *>(std::malloc(size));
  char *buf2 = static_cast<char *>(std::malloc(size));
  SmallVector<Instruction *, 8> localLoadsAndStores;
  SmallVector<Instruction *, 8> allLoadsAndStores;

  if (!demangler.partialDemangle(fptr->getName().data())) {
    StringRef contextName = demangler.getFunctionDeclContextName(buf1, &size);
    StringRef baseName = demangler.getFunctionBaseName(buf2, &size);
    for (auto &item : nsBlackList) {
      if (contextName.startswith(item)) {
        bool isIgnore = true;
        for (auto &item2 : funcWhiteList) {
          if (item2 == baseName) {
            isIgnore = false;
            break;
          }
        }
        if (isIgnore) {
          errs() << "Ignored: " << contextName << "::" << baseName << "\n";
          return;
        } else {
          break;
        }
      }
    }
  }

  //errs() << "Instrument " << fptr->getName() << "\n";
  for (auto &bb : *fptr) {
    for (auto &inst : bb) {
      if (isa<LoadInst>(inst) || isa<StoreInst>(inst)) {
        localLoadsAndStores.push_back(&inst);
      } else if ((isa<CallInst>(inst) && !isa<DbgInfoIntrinsic>(inst)) ||
                  isa<InvokeInst>(inst)) {
        chooseInstructiontoInstrument(localLoadsAndStores, allLoadsAndStores);
      }
    }
    chooseInstructiontoInstrument(localLoadsAndStores, allLoadsAndStores);
  }

  for (Instruction *i : allLoadsAndStores) {
    instrumentLoadAndStore(i, dl);
  }
}

void RaceDetector::instrumentLoadAndStore(Instruction *inst, const DataLayout &dl) {
  IRBuilder<> irb(inst);
  bool isWrite = isa<StoreInst>(inst);
  Value *addr = isWrite ? cast<StoreInst>(inst)->getPointerOperand()
                        : cast<LoadInst>(inst)->getPointerOperand();
  FunctionCallee func = isWrite ? checkWrite : checkRead;
  int size = getMemoryAccessSize(addr, dl);
  assert(size > 0);
  irb.CreateCall(func, 
                 {irb.CreatePointerCast(addr, irb.getInt8PtrTy()), 
                  irb.getInt32(size)});
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

} // namespace

PreservedAnalyses InstrumentationPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  
  RaceDetector rd(F);
  errs() << "Instrument " << F.getName() << "\n";
  rd.sanitizeFunction();
  errs() << "Skipped reads: " << rd.getSkippedReads() << "\n";
  errs() << "Skipped writes: " << rd.getSkippedWrites() << "\n"; 
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
