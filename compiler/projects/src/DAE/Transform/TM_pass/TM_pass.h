#pragma once
#include "llvm/ADT/Statistic.h"
#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
 
#include "ostream"
#include "fstream"
#include "iostream"
#include <vector>
#include <algorithm>

#define DEBUG_TYPE "tm_pass"
STATISTIC(TotalNUMLoads, "Total number of loads");
STATISTIC(TotalNUMPrefetchedLoads, "Total number of prefetched loads");
STATISTIC(TotalNUMLoadsInsideLoops, "Total number of loads inside loops");
STATISTIC(TotalNUMLoadsDependOnLoads, "Total number of loads that depend on loads");
STATISTIC(TotalNUMTXInsideLoops, "Total number of TX regions that are inside loops");
STATISTIC(TotalNUMTX, "Total number of TX");
STATISTIC(TotalNUMFunctionCalls, "Total number of functions calls");

using namespace llvm;
namespace{


	class TM_pass: public ModulePass{

	friend class StatisticInfo;

	public:
		static char ID;
		TM_pass(): ModulePass(ID){
		};
		~TM_pass(){};
		
		virtual void getAnalysisUsage(AnalysisUsage& AU) const{
			AU.addRequired<AAResultsWrapperPass>();
			AU.addRequired<LoopInfoWrapperPass>();
			AU.addRequired<AssumptionCacheTracker>();
			AU.addRequired<TargetLibraryInfoWrapperPass>();
		}

		bool runOnFunction(Function &F);
		virtual bool runOnModule(Module &M);

	private:
		//*****************************Data*************************************************************
		//LoopInfo *LI;
		//std::vector<LoopInfo*> LIV;
		std::vector<LoopInfoBase<BasicBlock, Loop> *> LIBV;
		//std::vector<Loop *> LV;
		std::vector<GlobalVariable *> GVV;
		AliasAnalysis *AA;

		int GlobalCount = 0;

		bool txDetected = false;
		//if TX is not inside loop then depth is 1 by default
		//Otherwise depth should be changed accordingly
		int loop_depth = 0;
		enum MODE{START, END};
		std::vector<std::pair<const char *, int>> InstList;//For statistic colecting, one for tx region
		
		//First purpose:Handle situation when BB cals itself. Don't want to go in the infinite loop.
		//Second purpose: Make one for the whole program to check whether this TX was visited
		std::vector<BasicBlock*>  		  UsedBBs;
		//SmallVector<Instruction *, 16> PrefLoads;
		Instruction *FirstInst;
		Instruction *BufferInst;

		ValueToValueMapTy vmap;
		std::vector<Instruction* > inst_vect;
		std::vector<Instruction *> TX_inst;
		std::vector<Value *> PrefetchedAddresses;
		std::vector<Instruction *> LoadNotPrefetch;
		std::vector<Value *> FunctionArgs;

		enum GLOBALREASON {DIRECTGLOBAL, ALIASGLOBAL, NOTGLOBAL};
		//*****************************Data*************************************************************
		



		//*****************************Servant**********************************************************
		//this function is for current purpose
		void countInst(Instruction &Inst);	
		void PrintTxStatistics();
		StringRef get_function_name(CallInst *ci);
		void LoadVisibleDependencies(Instruction *);
		bool isInstInsideLoop(Instruction *, bool);
		std::pair<GLOBALREASON, Instruction *> isLoadGlobal(Instruction *);
		bool isLoadDependLoad();
		AliasResult pointerAlias(Value *, Value *, const DataLayout&);
		bool isAddressPrefetched(Instruction *);
		//*****************************Servant**********************************************************
		
		

		//*****************************Core*************************************************************
		//This function goes through the BB and checks whether there is fakeCallBegin call there
		//Identifies the beginning of TX region
		bool isTargetBB(BasicBlock &BB, MODE mode);
		
		//TODO: come up with more appropriate function name
		//Core optimization function
		void printTxRegionInst(BasicBlock &BB);
		void prefetchGlobalLoads();
		void notAggressivePrefetching(Instruction *, std::pair<GLOBALREASON, Instruction *> *);
		//*****************************Core*************************************************************
			
	};

char TM_pass::ID = 0;
static RegisterPass<TM_pass> X("tm_pass", "TM_pass was successfuly registered", false, false);
}
