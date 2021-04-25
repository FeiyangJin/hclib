#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class InstrumentationPass : public PassInfoMixin<InstrumentationPass> {
 public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};

} // namespace llvm
#endif //INSTRUMENTATION_H