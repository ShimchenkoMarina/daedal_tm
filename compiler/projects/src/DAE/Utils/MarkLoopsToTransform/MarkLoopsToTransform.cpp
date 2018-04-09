//===- MarkLoopsToTransform.cpp - Kill unsafe stores with backups
//--------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file MarkLoopsToTransform.cpp
///
/// \brief
///
/// \Copyright (C) Eta Scale AB. Licensed under the Eta Scale Open Source License.
/// See the LICENSE file for details.
/// 
//
// Description of pass ...
//
//===----------------------------------------------------------------------===//
#include "llvm/Analysis/LoopPass.h"

#include "../../../DAE/Utils/SkelUtils/Utils.cpp"

#define KERNEL_MARKING "__kernel__tm__"

using namespace llvm;

static cl::opt<std::string> BenchName("bench-name",
                                      cl::desc("The benchmark name"),
                                      cl::value_desc("name"));

static cl::opt<bool> RequireDelinquent(
    "require-delinquent",
    cl::desc("Loop has to contain delinquent loads to be marked"),
    cl::init(true));

namespace {
struct MarkLoopsToTransform : public FunctionPass {
public:
  static char ID;
  MarkLoopsToTransform() : FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &F);

private:
  unsigned loopCounter = 0;
  bool markLoops(std::vector<Loop *> Loops, DominatorTree &DT);
};
}

bool MarkLoopsToTransform::runOnFunction(Function &F) {
	//Mark loop only if this loop is inside TX
	bool txDetected = false;
	for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
	{
		if (auto *CI = dyn_cast<CallInst>(&*I))
		{
			Function *fun = CI->getCalledFunction();
			if(fun)
			{
				if (fun->getName() == "fakeCallBegin")
				{
					txDetected = true;
					break;
				}
			}
		}
	}
	if(txDetected)
	{
  		DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  		LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  		std::vector<Loop *> Loops(LI.begin(), LI.end());

  		return markLoops(Loops, DT);
	}
	return false;
}

bool MarkLoopsToTransform::markLoops(std::vector<Loop *> Loops,
                                    DominatorTree &DT) {
  bool markedLoop = false;

  for (auto I = Loops.begin(), IE = Loops.end(); I != IE; ++I) {
    Loop *L = *I;
    if (loopToBeDAE(L, BenchName)) {
      BasicBlock *H = L->getHeader();
      H->setName(Twine(KERNEL_MARKING + H->getParent()->getName().str() +
                       std::to_string(loopCounter)));
      loopCounter++;
      markedLoop = true;
      continue; // if marked loop, don't check subloops
    }

    std::vector<Loop *> subLoops = L->getSubLoops();
    bool subLoopsMarked = markLoops(subLoops, DT);
    markedLoop = markedLoop || subLoopsMarked;
  }

  return markedLoop;
}

char MarkLoopsToTransform::ID = 1;
static RegisterPass<MarkLoopsToTransform>
    X("mark-loops", "Mark loops to transform pass", true, false);
