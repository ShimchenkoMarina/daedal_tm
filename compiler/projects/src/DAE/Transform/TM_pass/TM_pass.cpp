#include "TM_pass.h"

//#define AGGRESSION

bool TM_pass::runOnFunction(Function &F){
	for (auto &BB: F)
	{
		if (isTargetBB(BB, MODE::START))
		{
			outs() << "TxBegin\n";
			printTxRegionInst(BB);
			prefetchGlobalLoads();
			TX_inst.clear();
			outs() << "TxEnd\n";
			txDetected = false;
			//need to clear UsedBBs
			UsedBBs.clear();
			//PrintTxStatistics();//TODO print into output file
			//List contains instruction for the current BB
			//Need to be cleared before starting next BB
			InstList.clear();
			PrefetchedAddresses.clear();
			LoadNotPrefetch.clear();
			FunctionArgs.clear();
			loop_depth = 0;
		}
	}
	return false;
			
}
		

//Main function
bool TM_pass::runOnModule(Module &M){
	GVV.clear();
	for (auto gv_iter = M.global_begin(); gv_iter != M.global_end(); gv_iter++)
	{
		GlobalVariable * gv = &*gv_iter;
		GVV.push_back(gv);
	}
	for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI)
	{
		//Loop analisys
		if (MI->isDeclaration())
			continue;
		llvm::DominatorTree DT = llvm::DominatorTree();
		DT.recalculate(*MI);
		
		LoopInfoBase<BasicBlock, Loop> * KLoop = new LoopInfoBase<BasicBlock, Loop>();
		KLoop->releaseMemory();
		KLoop->analyze(DT);
		//KLoop->print(outs());
		//LoopInfo & LI = getAnalysis<LoopInfoWrapperPass>(*MI).getLoopInfo();
		LIBV.clear();
		LIBV.push_back(KLoop);
		//Alias analysis
		BasicAAResult BAR(createLegacyPMBasicAAResult(*this, *MI));
		AAResults AAR(createLegacyPMAAResults(*this, *MI, BAR));
		AA = &AAR;
		runOnFunction(*MI);
	}
	return false;
}
	
//this function is for current purpose
void TM_pass::countInst(Instruction &Inst)
{
	//go through instraction list and check whether this instruction their
	//if it is not there then add new pair to instruction list vector
	//if it is their then just increase number
	const char * name = Inst.getOpcodeName(Inst.getOpcode());
	bool fl = false;
	for (auto &it: InstList)
	{
		if (it.first == name)
		{
			(&it)->second++;
			fl = true;
		}
	}
	if (fl == false)
	{
		std::pair<const char *, int> temp_pair = std::make_pair(name, 1);
		InstList.push_back(temp_pair);
	}

}					
		
void TM_pass::PrintTxStatistics(){
	for (auto p:InstList)
		outs() << p.first << " : " << p.second << "\n";
}

StringRef TM_pass::get_function_name(CallInst *ci)
{
	Function *fun = ci->getCalledFunction();
	if (fun)
		return fun->getName();
	else return StringRef("inderect call");//Apparently nothing can be done with inderect calls
}

//This function goes through the BB and checks whether there is fakeCallBegin call there
bool TM_pass::isTargetBB(BasicBlock &BB, MODE mode){
			switch (mode){
			//BB should contain fakeCallBegin
			case MODE::START:
				//Case when fakeCallBegin and fakeCallEnd are in the same region
				for (auto &I:BB)
				{
					if(auto *CI = dyn_cast<CallInst>(&I))
					{
						StringRef name = get_function_name(CI);
						if (name == "fakeCallBegin")
						{
							return true;
						}
					}
				}
				break;
			//Successor should contain fakeCallEnd
			case MODE::END:
				for(auto it = succ_begin(&BB), et = succ_end(&BB); it != et; ++it)
				{
					BasicBlock* succ = *it;
					for (auto &I: *succ)
					{
						if(auto *CI = dyn_cast<CallInst>(&I))
						{
							StringRef name = get_function_name(CI);
							if (name == "fakeCallEnd")
							{
								return true;
							}
						}
					
					}
				}
				break;
			default:
				break;
			}
				
			return false;
}



void TM_pass::printTxRegionInst(BasicBlock &BB)
{
			//if BB was visited then don't do anything
			for (auto &bb: UsedBBs)
			{
				if (bb == &BB)
				{
					return;
				}
				
			}
			//If BB wasn't visited then marked it as visited
			UsedBBs.push_back(&BB);	
			//For each instruction check whether it is a function call 
			//If it's not a function call then just **print** this instruction
			//If it's a function call, go inside this function
			for(auto &I: BB)
			{
				//First check whether it is a call
				if (isa<CallInst>(&I) && !dyn_cast<CallInst>(&I)->isInlineAsm())
				{
					//TODO: delete if it is redundant
					CallInst * CI = dyn_cast<CallInst>(&I);
					Function *fun = CI->getCalledFunction();
					if (auto* CstExpr = dyn_cast<ConstantExpr>(I.getOperand(0)))
					{
						if (CstExpr->isCast() && isa<Function>(CstExpr->getOperand(0)))
						{
							outs() << "Wierd thing============================================================================\n";
							fun = dyn_cast<Function>(CstExpr->getOperand(0));
						}
					}
					//TODO: delete untill here
						
					StringRef name = get_function_name(CI);
					//outs() << "name =" << name << "\n";
					if (txDetected){
						if (name == "fakeCallEnd")
						{
							outs() << "fakeCallEnd" << "\n";
							return;
						}
						else
						{//another call
							//TODO:
							//Unfortunatelly asm insert is inderect call, but
							//currently I don't know how to check it.
							if (name != "inderect call")
							{
								Function *fun = CI->getCalledFunction();
								//outs() << "FUNCTION CALL:" << name << "\n";
								if (!(fun->isDeclaration()))
								{
									//outs() << "FUNCTION CALL:" << name << "\n";
	
									++TotalNUMFunctionCalls;
									for (Function::arg_iterator A = fun->arg_begin(); A != fun->arg_end(); ++A)
									{
										Value *arg_value = &*A;
										FunctionArgs.push_back(arg_value);
										//outs() << "Args: " << *arg_value << "\n";
									}
									
									llvm::DominatorTree DT = llvm::DominatorTree();
									DT.recalculate(*fun);

									LoopInfoBase<BasicBlock, Loop> * KLoop = new LoopInfoBase<BasicBlock, Loop>();
									KLoop->releaseMemory();
									KLoop->analyze(DT);
									
									//KLoop->print(outs());
									LIBV.push_back(KLoop);
									//LoopInfo &LocalLI = getAnalysis<LoopInfoWrapperPass>(*fun).getLoopInfo();
									//LIV.push_back(&LocalLI);
									//outs() << "Push Info\n";
									for (auto &BBfun: *fun)
									{
										printTxRegionInst(BBfun);
										//outs() << "FUNCTION CALL:" << name << "\n";									
									}
								}
								//LIV.pop_back();
							}else{
								//outs() << "Inderect call --> ";
								//outs() << I << "\n";
							}
						}
						
					}
					if (name == "fakeCallBegin")
					{
						//Check whether TX is inside loop
						//if it is a case then
						//Instruction is inside a loop only if this loop's depth is 2
						txDetected = true;
						FirstInst = &I;
						isInstInsideLoop(&I, true);
						++TotalNUMTX;
						//Don't need to go to next if statement
						continue;
					}

				}
				//Now we are sure that it is not a Function call,
				//check whether it is a load or a store
				if (txDetected)
				{
					//It should be changed on counting instructions
					countInst(I);
					TX_inst.push_back(&I);
					//Uncomment this to print instructions inside tx region
					//outs() << "Instruction is inside TX region: " << I << "\n";
				}

			}
			bool check;
			//If TxEnd wasn't found in current BB
			//TODO: appropriatly go throug complex CFGs
			if (!isTargetBB(BB, MODE::END))
			{
				//RTM_fallbacl_unlock wasn't found in current function
				const TerminatorInst *TInst = BB.getTerminator();
				for(unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I)
				{
					check = false;
					//print all instructions inside
					BasicBlock *Succ = TInst->getSuccessor(I);
					for (auto &bb: UsedBBs)
					{
						//Already visited this BasicBlock
						//Skip it
						if (Succ == bb)
						{
							check = true;
							continue;
						}
					}
					//If BB wasn't used
					if (!check)
					{
						printTxRegionInst(*Succ);
					}
				}
			}
			return;
}

void TM_pass::LoadVisibleDependencies(Instruction * inst)
{
	
	for (unsigned int i = 0; i < inst->getNumOperands(); i++)
	{
		Value *V = inst->getOperand(i);
		//outs() << "Value: " << *V << "\n";
		if (Instruction *Inst = dyn_cast<Instruction>(V))
		{
			//Handle recursion
			//If this instruction was marked as dependent before
			//Don't need to mark it as dependent again
			if (!(std::find(inst_vect.begin(), inst_vect.end(), Inst) != inst_vect.end()))
			{
				//Handle TX boundaries
				if (std::find(TX_inst.begin(), TX_inst.end(), Inst) != TX_inst.end())
				{
					inst_vect.insert(inst_vect.begin(), Inst);
					LoadVisibleDependencies(Inst);
				}
			}else return;
		}
	}
}
//Check if load is inside loop
//forTX flaf is used to detect that this check is for the beginning of TX region
//If TX region is inside loop then increase loop_depth for other instruction 
bool TM_pass::isInstInsideLoop(Instruction * li, bool forTX)
{
	BasicBlock *bb = li->getParent();
	//outs() << "BB:" << bb->getName() << "\n";
	for (auto *LI: LIBV)
	{ 
		int depth = (*LI).getLoopDepth(bb);
		if (forTX)
		{
			if (depth != 0)
			{
				loop_depth = depth;
				++TotalNUMTXInsideLoops;
				outs() << "Loop: YES\n";
				return true;
			}
		}else
		if (depth > loop_depth)
		{
			++TotalNUMLoadsInsideLoops;
			outs() << "Loop: YES\n";
			return true;
		}	
	}
	outs() << "Loop: NO\n";
	return false;
}

//Check two pointer points out to a global variable
AliasResult TM_pass::pointerAlias(Value* P1, Value* P2, const DataLayout &DL)
{
	uint64_t P1Size = MemoryLocation::UnknownSize;
	Type *P1E1Ty = cast<PointerType>(P1->getType())->getElementType();
	if(P1E1Ty->isSized())
	{
		P1Size = DL.getTypeStoreSize(P1E1Ty);	
	}

	uint64_t P2Size = MemoryLocation::UnknownSize;
	Type *P2E1Ty = cast<PointerType>(P2->getType())->getElementType();
	if(P2E1Ty->isSized())
	{
		P2Size = DL.getTypeStoreSize(P2E1Ty);	
	}
	return AA->alias(P1, P1Size, P2, P2Size);	
}

std::pair<TM_pass::GLOBALREASON, Instruction *>  TM_pass::isLoadGlobal(Instruction * LMemI)
{
	for (User::op_iterator it = LMemI->op_begin(), et = LMemI->op_end(); it != et; ++it)
	{
		if (!isa<GlobalVariable>(&*it))
		{
			//Case when load's operand is a pointer
			//it might be pointer to a global variable
			for (auto &gv: GVV)
			{
				for (auto user_of_gv: gv->users())
				{
					if (Instruction *instr_st_gv = dyn_cast<Instruction>(user_of_gv))
					{
						if (StoreInst *storeInst = dyn_cast<StoreInst>(instr_st_gv))
						{
				
							//outs() << "STORE INST  " << *instr_st_gv << "\n";
							Value * storePointer = storeInst->getPointerOperand();
							Value * loadPointer = (dyn_cast<LoadInst>(LMemI))->getPointerOperand();
							if (std::find(FunctionArgs.begin(), FunctionArgs.end(), loadPointer) == FunctionArgs.end())
							{
								AliasResult AAR = pointerAlias(storePointer, loadPointer, (dyn_cast<LoadInst>(LMemI))->getModule()->getDataLayout());
								if (AAR == AliasResult::MustAlias || AAR == AliasResult::MayAlias)
								{
									outs() << "GLOBAL: MAY/MUST\n";
									//outs() << "Alias match " << *instr_st_gv << "\n";
									return std::make_pair(GLOBALREASON::ALIASGLOBAL, instr_st_gv);
								}	
							}else{
								outs() << "GLOBAL: NO (function arg)\n";
								return std::make_pair(GLOBALREASON::NOTGLOBAL, LMemI);
							}
						}
					}
				}
			}	
		}else {
			outs() << "GLOBAL: YES (direct)\n";
			return std::make_pair(GLOBALREASON::DIRECTGLOBAL, LMemI);
		}
	} 
	outs() << "GLOBAL: NO\n";
	return std::make_pair(GLOBALREASON::NOTGLOBAL, LMemI);
}
//Check if load depends on another load
//If agression flag is not set then don't touch load incase it depends on another load
//With aggression flag set, prefetch this load as well
//TODO: add aggressive version of prefetching
bool TM_pass::isLoadDependLoad()
{
	bool flag = false;
	for (auto &inst: inst_vect)
	{
		if (isa<LoadInst>(inst))
		{
			#ifdef AGGRESSION
				LoadNotPrefetch.push_back(inst);
			#endif	
			flag = true;
		}
	}
	if(flag)
	{
		++TotalNUMLoadsDependOnLoads;
		outs() << "Dependancy: YES\n";
	}else{
		outs() << "Dependancy: NO\n";

	}
	return flag;
}

bool TM_pass::isAddressPrefetched(Instruction * inst)
{
	Value * value;
	if (StoreInst *sti = dyn_cast<StoreInst>(inst))
		value = sti->getPointerOperand();
	else if (LoadInst *ldi = dyn_cast<LoadInst>(inst))
		value = ldi->getPointerOperand();
	if (std::find(PrefetchedAddresses.begin(), PrefetchedAddresses.end(), value) != PrefetchedAddresses.end())
	{
		outs() << "Prefetched: Yes\n";
		return true;
	}
	else{
		outs() << "Prefetched: NO\n";
		 return false;
	}
}

void TM_pass::notAggressivePrefetching(Instruction * I, std::pair<TM_pass::GLOBALREASON, Instruction *> *metadata)
{
	//outs() << "LOAD: " << *LMemI << "\n";
	//If there are instruction which current load depends on
	//First duplicate all these instruction before TX region
	for (Instruction *Inst : inst_vect)
	{
		//outs() << "Dependent instruction" << *Inst << "\n";
		auto *new_inst = Inst->clone();
		new_inst->insertBefore(FirstInst);
		vmap[Inst] = new_inst;
		RemapInstruction(new_inst, vmap, RF_NoModuleLevelChanges | RF_IgnoreMissingEntries);
	}	
	inst_vect.clear();
	//If load points to a global variable directly then
	//Get address for prefetching directly from this load
	unsigned PtrAs;
	Value *DataPtr;
	LoadInst *LMemI = dyn_cast<LoadInst>(I);
	if (metadata->first == GLOBALREASON::DIRECTGLOBAL || metadata->first == GLOBALREASON::ALIASGLOBAL)
	{
		//Clone load, because after renaming all instruction which current load depends on
		//Have different names of their arguments
		//To mach this differencies, need to rename
		auto *new_inst = LMemI->clone();
		new_inst->insertBefore(LMemI);
		vmap[LMemI] = new_inst;
		RemapInstruction(new_inst, vmap, RF_NoModuleLevelChanges | RF_IgnoreMissingEntries);
		//prefetch load before fakeCallBegin
		LoadInst * new_load_inst = dyn_cast<LoadInst>(new_inst);
		PtrAs = new_load_inst->getPointerAddressSpace();
		DataPtr = new_load_inst->getPointerOperand();
		//To mark that this address has already prefetched
		//Don't want to prefetch the same address twice
		PrefetchedAddresses.push_back(DataPtr);
		//This instruction is an actual load
		//don't need it as the address will be prefetched
		new_inst->eraseFromParent();
	}	
	//If load aliases with a global variable, then
	//take address for prefetching from appropriate store instruction
	/*else if (metadata->first == GLOBALREASON::ALIASGLOBAL)
	{
		StoreInst * storeInst = dyn_cast<StoreInst>(metadata->second);
		outs() << "ALIASGLOBAL " << *storeInst << "\n";
		PtrAs = storeInst->getPointerAddressSpace();
		DataPtr = storeInst->getPointerOperand();
		//To mark that this address has already prefetched
		//Don't want to prefetch the same address twice
		PrefetchedAddresses.push_back(DataPtr);
	}*/
	LLVMContext &Context = DataPtr->getContext();	
	Type *I8Ptr = Type::getInt8PtrTy(Context, PtrAs);
	CastInst *Cast = CastInst::CreatePointerCast(DataPtr, I8Ptr, "", FirstInst);
	IRBuilder<> Builder(FirstInst);
	Module *M = FirstInst->getParent()->getParent()->getParent();
	Type *I32 = Type::getInt32Ty(FirstInst->getContext());
	Value *PrefetchFunc = Intrinsic::getDeclaration(M, Intrinsic::prefetch);
					
	//To remember: new_load_inst->mayReadFromMemory() ? 0 : 1	
	Builder.CreateCall(PrefetchFunc, 
		{Cast, 
		ConstantInt::get(I32, 0),
		ConstantInt::get(I32,3), ConstantInt::get(I32,1)});
	outs() << "Prefetch is done\n";	
	++TotalNUMPrefetchedLoads;	
	return;
}

void TM_pass::prefetchGlobalLoads(){
#ifdef AGGRESSION
	for (auto &I: TX_inst)
	{
		if (isa<LoadInst>(*&I))
		{
			//outs() << "LOAD is detected: " << *LMemI << "\n";
			//Check if load points out to global variable
			std::pair<GLOBALREASON, Instruction*> metadata = isLoadGlobal(*&I);
			//Prefetch only if load points to global variable and load is not inside loop
			//And its address wasn't prefetched before
			if( !(isInstInsideLoop(*&I, false)) &&  (metadata.first != GLOBALREASON::NOTGLOBAL) && !isAddressPrefetched(metadata.second))
			{
				//List of load's dependent instructions that should be prefetched
				inst_vect.clear();
				//Collect dependent inctructions
				LoadVisibleDependencies(*&I);
				//Remove all loads other loads depend on
				isLoadDependLoad();
			}else{
				inst_vect.clear();
				continue;
			}

		}
	}
#endif
	//chech whether an instruction is load or store
	for (auto &I: TX_inst)
	{
		if (LoadInst *LMemI = dyn_cast<LoadInst>(*&I))
		{
			outs() << "LOAD is detected: " << *LMemI << "\n";
			++TotalNUMLoads;
			//Check if load points out to global variable
			std::pair<GLOBALREASON, Instruction*> metadata = isLoadGlobal(*&I);
			bool insideloop = isInstInsideLoop(*&I, false);
			//Prefetch only if load points to global variable and load is not inside loop
			//And its address wasn't prefetched before
			if( (metadata.first != GLOBALREASON::NOTGLOBAL) && (insideloop == false) && !isAddressPrefetched(metadata.second) && (std::find(LoadNotPrefetch.begin(), LoadNotPrefetch.end(), *&I) == LoadNotPrefetch.end()))
			{
				//List of load's dependent instructions that should be prefetched
				inst_vect.clear();
				//Collect dependent inctructions
				LoadVisibleDependencies(*&I);
				//Now, don't touch a load if it depends on another load
				//TODO: add aggresive version
				if (!isLoadDependLoad())
				{
					notAggressivePrefetching(*&I, &metadata);
				}else{
					#ifdef AGGRESSION
						notAggressivePrefetching(*&I, &metadata);
					#endif
				}
			}else{
				inst_vect.clear();
				//outs() << "Inside Loop\n";
				//Workaround to test an idea
				//This instruction is supposed to calculate an address
				//Ideally want go through all dependencies
				continue;
			}

		}
	}
}

