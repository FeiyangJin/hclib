#include "instrumentation.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

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
  void instrumentLoadAndStore();
};

RaceDetector::RaceDetector(Function &f) : fptr(&f) {
  Module *m = f.getParent();
  IRBuilder<> irb(m->getContext());
  AttributeList attr;
  attr = attr.addAttribute(m->getContext(), AttributeList::FunctionIndex,
                           Attribute::NoUnwind);
  SmallString<32> readFuncName("asap_check_read");
  SmallString<32> writeFuncName("asap_check_write");
  checkRead = m->getOrInsertFunction(readFuncName, attr, irb.getVoidTy(), irb.getInt8PtrTy());
  checkWrite = m->getOrInsertFunction(writeFuncName, attr, irb.getVoidTy(), irb.getInt8PtrTy());
}

void RaceDetector::sanitizeFunction() {
  this->instrumentLoadAndStore();
}

void RaceDetector::instrumentLoadAndStore() {
  for (auto &bb : *fptr) {
    for (auto &inst : bb) {
      if (isa<LoadInst>(inst) || isa<StoreInst>(inst)) {
        IRBuilder<> irb(&inst);
        bool isWrite = isa<StoreInst>(inst);
        Value *addr = isWrite ? cast<StoreInst>(&inst)->getPointerOperand()
                              : cast<LoadInst>(&inst)->getPointerOperand();
        FunctionCallee func = isWrite ? checkWrite : checkRead;
        irb.CreateCall(func, irb.CreatePointerCast(addr, irb.getInt8PtrTy()));
      }
    }
  }
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