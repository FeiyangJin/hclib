#include "instrumentation.h"
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
#include <cstdlib>

using namespace llvm;

namespace {
  
class RaceDetector{
 public:
  RaceDetector(Function &f);
  void sanitizeFunction();
 private:
  FunctionCallee checkRead;
  FunctionCallee checkWrite;
  Function *fptr;
  ItaniumPartialDemangler demangler;
  SmallVector<StringRef> blackList;
  void instrumentLoadAndStore();
  int getMemoryAccessSize(Value *Addr, const DataLayout &DL);
};

RaceDetector::RaceDetector(Function &f) : fptr(&f), demangler(), blackList() {
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
  blackList.append({"hclib"});
}

void RaceDetector::sanitizeFunction() {
  this->instrumentLoadAndStore();
}

void RaceDetector::instrumentLoadAndStore() {
  const DataLayout &dl = fptr->getParent()->getDataLayout();
  size_t size = 1;
  char *buf = static_cast<char *>(std::malloc(size));
  if (!demangler.partialDemangle(fptr->getName().data())) {
    StringRef contextName = demangler.getFunctionDeclContextName(buf, &size);
    for (auto &item : blackList) {
      if (contextName.startswith(item)) {
        return;
      }
    }
  }
  errs() << "Instrument " << fptr->getName() << "\n";
  for (auto &bb : *fptr) {
    for (auto &inst : bb) {
      if (isa<LoadInst>(inst) || isa<StoreInst>(inst)) {
        IRBuilder<> irb(&inst);
        bool isWrite = isa<StoreInst>(inst);
        Value *addr = isWrite ? cast<StoreInst>(&inst)->getPointerOperand()
                              : cast<LoadInst>(&inst)->getPointerOperand();
        FunctionCallee func = isWrite ? checkWrite : checkRead;
        int size = getMemoryAccessSize(addr, dl);
        assert(size > 0);
        irb.CreateCall(func, 
                       {irb.CreatePointerCast(addr, irb.getInt8PtrTy()), 
                        irb.getInt32(size)});
      }
    }
  }
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
  rd.sanitizeFunction();
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