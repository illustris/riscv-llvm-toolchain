#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/CallSite.h"
#include <stack>

using namespace llvm;

Value* resolveGetElementPtr(GetElementPtrInst *GI,DataLayout *D,LLVMContext &Context)
{
	int offset = 0;
	Value *Offset,*temp;
	int c = 0;
	bool isconstant = true;

	if(ConstantInt *CI = dyn_cast<ConstantInt>(GI->getOperand(GI->getNumOperands()-1)))// get the last operand of the getelementptr
		c = CI->getZExtValue ();
	else
	{
		//suppose the last index was not a constant then set 'c' to some special value and get the last operand.
		isconstant = false;
		// Instruction *I = dyn_cast<Instruction>(GI->getOperand(GI->getNumOperands()-1));
		// Offset = I->getOperand(0);
		Offset = GI->getOperand(GI->getNumOperands()-1);
	}

	Type *type = GI->getSourceElementType(); //get the type of getelementptr

	if(StructType *t = dyn_cast<StructType>(type))
	{	//check for struct type
		const StructLayout *SL = D->getStructLayout(t);
		offset+= SL->getElementOffset(c);
	}
	else if(ArrayType *t = dyn_cast<ArrayType>(type))
	{
		ArrayType *rec = t;
		ArrayType *rec_old = t;
		//errs()<<0<<": "<<*rec<<"\n";
		int depth=1;
		std::stack <int> sizes;
		sizes.push(t->getArrayNumElements());
		while((rec = dyn_cast<ArrayType>(rec->getElementType())))
		{
			//errs()<<depth<<": "<<*rec<<"\n";
			sizes.push(rec->getArrayNumElements());
			rec_old = rec;
			depth++;
		}

		Type *baseTy = rec_old->getElementType();

		Type *gelType = Type::getInt128Ty(Context);
		while(depth)
		{
			int sz = sizes.top();
			sizes.pop();
			gelType = ArrayType::get(gelType,sz);
			depth--;
		}

		//errs()<<depth<<": "<<*gelType<<"\n";

		if(!isconstant)
		{
			//errs()<<"\n------\n"<<*GI<<"\n";
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize(baseTy->isPointerTy()?Type::getInt128Ty(Context):baseTy));
			//errs()<<*t->getElementType()<<"\n"<<D->getTypeAllocSize(t->getElementType())<<"\n";
			IRBuilder<> builder(GI);
			Offset = builder.CreateBinOp(Instruction::Mul,Offset, temp, "tmp");
		}
		else
			offset+=c*D->getTypeAllocSize(t->getElementType()); //D->getTypeAllocSize(t)/t->getArrayNumElements();		
	}
	else
	{	//basic pointer increment or decrements

		if(!isconstant)
		{
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize(type));
			IRBuilder<> builder(GI);
			Offset = builder.CreateBinOp(Instruction::Mul,Offset, temp, "tmp");
		}
		else
			offset+=c*D->getTypeAllocSize(type);
	}

	if(isconstant)
		Offset = llvm::ConstantInt::get(Type::getInt32Ty(Context),offset);

	return Offset;
}

namespace {
	struct shaktiPass : public ModulePass
	{
		static char ID;
		shaktiPass() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M)
		{
			bool modified=false;

			// First pass replaces malloc and free
			Value *mallocFunc;
			Value *freeFunc;

			for (auto &F : M)
			{
				modified=false;
				// Get the safemalloc function to call from the new library.
				LLVMContext &Ctx = F.getContext();
				// Malloc arg is just one unsigned long
				std::vector<Type*> mallocParamTypes = {Type::getInt64Ty(Ctx)};
				std::vector<Type*> safeMallocParamTypes = {Type::getInt64Ty(Ctx)};
				// Free arg is i8*
				std::vector<Type*> freeParamTypes = {Type::getInt8PtrTy(Ctx)};
				std::vector<Type*> safeFreeParamTypes = {Type::getInt128Ty(Ctx)};
				// Return type of malloc is i8*
				Type *mallocRetType = Type::getInt8PtrTy(Ctx);
				Type *safeMallocRetType = Type::getInt128Ty(Ctx);
				// Return type of free is void
				Type *freeRetType = Type::getVoidTy(Ctx);
				Type *safeFreeRetType = Type::getVoidTy(Ctx);
				// Put everything together to make function type i8*(i64)
				FunctionType *mallocFuncType = FunctionType::get(mallocRetType, mallocParamTypes, false);
				FunctionType *safeMallocFuncType = FunctionType::get(safeMallocRetType, safeMallocParamTypes, false);
				// Put everything together to make function type void*(i8*)
				FunctionType *freeFuncType = FunctionType::get(freeRetType, freeParamTypes, false);
				FunctionType *safeFreeFuncType = FunctionType::get(safeFreeRetType, safeFreeParamTypes, false);
				// Get function pointer for malloc
				mallocFunc = F.getParent()->getOrInsertFunction("malloc", mallocFuncType);
				// Get function pointer for free
				freeFunc = F.getParent()->getOrInsertFunction("free", freeFuncType);
				// Make safemalloc declaration, get pointer to it
				Value *safemallocFunc = F.getParent()->getOrInsertFunction("safemalloc", safeMallocFuncType);
				// Make safefree declaration, get pointer to it
				Value *safefreeFunc = F.getParent()->getOrInsertFunction("safefree", safeFreeFuncType);

				// Iterate over BBs in F
				for (auto &B : F)
				{
					// Iterate over Instrs in BB
					for (auto &I : B)
					{
						// If instruction is of type call
						if (auto *op = dyn_cast<CallInst>(&I))
						{
							// If call invokes malloc
							if((op->getCalledValue()) == (mallocFunc))
							{
								//errs()<<"\n----------------\nReplacing:\n";
								//op->print(llvm::errs(), NULL);
								//errs()<<"\nwith:\n";
								// Replace malloc with safemalloc in call
								op->setCalledFunction (safemallocFunc);
								op->mutateType(Type::getInt128Ty(Ctx));
								//op->print(llvm::errs(), NULL);
								//errs()<<"\n----------------\n";

								// Set flag if modified
								modified=true;
							}
							else if((op->getCalledValue()) == (freeFunc))
							{
								//errs()<<"\n----------------\nReplacing:\n";
								//op->print(llvm::errs(), NULL);
								//errs()<<"\nwith:\n";
								// Replace free with safefree in call
								op->setCalledFunction (safefreeFunc);
								//op->print(llvm::errs(), NULL);
								//errs()<<"\n----------------\n";

								// Set flag if modified
								modified=true;
							}
						}
					}
				}
			}

			dyn_cast<Function>(mallocFunc)->dropAllReferences();
			dyn_cast<Function>(freeFunc)->dropAllReferences();
			dyn_cast<Function>(mallocFunc)->removeFromParent();
			dyn_cast<Function>(freeFunc)->removeFromParent();//*/

			// Second pass adds stack cookie
			for (auto &F : M)
			{
				LLVMContext &Ctx = F.getContext();
				Module *m = F.getParent();
				Function *hash = Intrinsic::getDeclaration(m, Intrinsic::riscv_hash);	// get hash intrinsic declaration
				AllocaInst *st_cook;	// pointer to stack cookie
				for (auto &B : F)
				{
					if(&B == &((&F)->front()))
					{	// If BB is first
						Instruction *I = &((&B)->front());	// Get first instruction in function
						st_cook = new AllocaInst(Type::getInt64Ty(Ctx), 0,"stack_cookie", I);	// alloca stack cookie

						// Set up intrinsic arguments
						std::vector<Value *> args;
						args.push_back(st_cook);
						ArrayRef<Value *> args_ref(args);

						// Create call to intrinsic
						IRBuilder<> Builder(I);
						Builder.SetInsertPoint(I);
						Builder.CreateCall(hash, args_ref,"stack_hash");
						modified = true;

					}

					for (auto &I : B)	// Iterate over instructions
						if (dyn_cast<ReturnInst>(&I))
						{	// If return instruction

							// set up arguments
							std::vector<Value *> args;
							args.push_back(st_cook);
							ArrayRef<Value *> args_ref(args);

							// Call hash again to burn cookie
							IRBuilder<> Builder(&I);
							Builder.SetInsertPoint(&I);
							Builder.CreateCall(hash, args_ref,"stack_cookie_burn");
							modified = true;
						}
				}
			}
			//errs()<<"Second pass done\n";

			// Third pass replaces pointers, store and load
			for (auto &F : M)
			{
				DataLayout *D = new DataLayout(&M);

				LLVMContext &Ctx = F.getContext();
				std::vector<Type*> craftParamTypes = {Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt64Ty(Ctx)};
				Type *craftRetType = Type::getInt128Ty(Ctx);
				FunctionType *craftFuncType = FunctionType::get(craftRetType, craftParamTypes, false);
				Value *craftFunc = F.getParent()->getOrInsertFunction("craft", craftFuncType);
				CallInst *st_hash;

				if(F.isDeclaration())
					continue;

				for(Function::arg_iterator j = F.arg_begin(), end = F.arg_end(); j != end; ++j)
				{
					if(j->getType()->isPointerTy() && F.getName() != "llvm.RISCV.hash")
					{
						j->mutateType(Type::getInt128Ty(Ctx));
						//errs()<<*j<<"\n";
					}
				}//*/

				Module *m = F.getParent();
				Function *val = Intrinsic::getDeclaration(m, Intrinsic::riscv_validate);	// get hash intrinsic declaration

				for (auto &B : F)
				{
					for(BasicBlock::iterator i = B.begin(), e = B.end(); i != e; ++i)
					{
						Instruction *I = dyn_cast<Instruction>(i);

						if (auto *op = dyn_cast<AllocaInst>(I))
						{
							modified=true;
							//errs()<<"\n-----------\nAlloca:\t"<<*op<<"\n-----------\n";
							if(op->getAllocatedType()->isPointerTy())
							{
								//errs()<<"\n-----------\nptrAlloca:\t"<<*op<<"\n-----------\n";
								op->setAllocatedType(Type::getInt128Ty(Ctx));
								op->mutateType(Type::getIntNPtrTy(Ctx,128));
								/*for (auto &U : op->uses())
								{
									if(!flag)
									{
										flag = true;
										continue;
									}

									User *user = U.getUser();
									users.push(user);
									pos.push(U.getOperandNo());
								}*/
							}
							else if(op->getAllocatedType()->isArrayTy())
							{
								ArrayType *t = dyn_cast<ArrayType>(op->getAllocatedType());
								ArrayType *rec = t;
								ArrayType *rec_old = t;
								int depth=1;
								std::stack <int> sizes;
								sizes.push(t->getArrayNumElements());
								while((rec = dyn_cast<ArrayType>(rec->getElementType())))
								{
									//errs()<<depth<<": "<<*rec<<"\n";
									sizes.push(rec->getArrayNumElements());
									rec_old = rec;
									depth++;
								}

								Type *baseTy = rec_old->getElementType();

								Type *gelType = Type::getInt128Ty(Ctx);
								while(depth)
								{
									int sz = sizes.top();
									sizes.pop();
									gelType = ArrayType::get(gelType,sz);
									depth--;
								}

								if(baseTy->isPointerTy())
								{
									//errs()<<"\n*********\nALLOCATED TYPE = "<<*op->getAllocatedType()->getArrayElementType()<<"\n\n";
									//errs()<<*(ArrayType::get(Type::getInt128Ty(Ctx),op->getAllocatedType()->getArrayNumElements()))<<"\n";
									op->setAllocatedType(gelType);
									op->mutateType(gelType->getPointerTo());

								}
							}

							if (op->getName() == "stack_cookie")
							{
								st_hash = dyn_cast<CallInst>(op->getNextNode());
								continue;
							}

							PtrToIntInst *trunc = new PtrToIntInst(op, Type::getInt32Ty(Ctx),"pti",op->getNextNode());

							std::vector<Value *> args;
							args.push_back(trunc);
							args.push_back(trunc);
							if(dyn_cast<ConstantInt>(op->getArraySize()))
								args.push_back(
									llvm::ConstantInt::get(
										Type::getInt32Ty(Ctx),
										(D->getTypeAllocSize(op->getAllocatedType()))
									)
								);
							else
							{
								BinaryOperator *total_off =  BinaryOperator::Create(Instruction::Mul, op->getArraySize(), llvm::ConstantInt::get(Type::getInt64Ty(Ctx),(D->getTypeAllocSize(op->getAllocatedType()))) , "off", op);
								TruncInst *total_off_trunc = new TruncInst(total_off, Type::getInt32Ty(Ctx),"offt", op);
								args.push_back(total_off_trunc);
							}
							args.push_back(st_hash);
							ArrayRef<Value *> args_ref(args);

							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(trunc->getNextNode());
							Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fpr");

							bool flag = false;
							std::stack <User *> users;
							std::stack <int> pos;

							for (auto &U : op->uses())
							{
								if(!flag)
								{
									flag = true;
									continue;
								}

								User *user = U.getUser();
								users.push(user);
								pos.push(U.getOperandNo());
							}

							while(users.size())
							{
								User *u = users.top();
								users.pop();
								int index = pos.top();
								pos.pop();
								u->setOperand(index, fpr);	
							}
						}

						else if (auto *op = dyn_cast<StoreInst>(I)) {
							if(op->getOperand(1)->getType() != Type::getInt128Ty(Ctx))
								continue;
							modified = true;
							TruncInst *tr_lo = new TruncInst(op->getOperand(1), Type::getInt64Ty(Ctx),"fpr_low", op);	// alloca stack cookie
							Value* shamt = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
							BinaryOperator *shifted =  BinaryOperator::Create(Instruction::LShr, op->getOperand(1), shamt , "fpr_hi_big", op);
							TruncInst *tr_hi = new TruncInst(shifted, Type::getInt64Ty(Ctx),"fpr_hi", op);	// alloca stack cookie

							// Set up intrinsic arguments
							std::vector<Value *> args;

							args.push_back(tr_hi);
							args.push_back(tr_lo);
							ArrayRef<Value *> args_ref(args);

							// Create call to intrinsic
							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(I);
							Builder.CreateCall(val, args_ref,"");

							Type *storetype = op->getOperand(0)->getType();
							if (storetype->isPointerTy())
							{
								storetype = Type::getInt128Ty(Ctx);
								op->mutateType(storetype);
							}
							Type *storeptrtype = storetype->getPointerTo();

							Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
							BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32", op);
							IntToPtrInst *ptr = new IntToPtrInst(ptr32,storeptrtype,"ptr",op);

							new StoreInst(op->getOperand(0),ptr,op);

							--i;
							op->dropAllReferences();
							op->removeFromParent();

						}

						else if (auto *op = dyn_cast<LoadInst>(I))
						{
							if(op->getOperand(0)->getType() != Type::getInt128Ty(Ctx))
								continue;
							modified = true;
							TruncInst *tr_lo = new TruncInst(op->getOperand(0), Type::getInt64Ty(Ctx),"fpr_low", op);	// alloca stack cookie
							Value* shamt = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
							BinaryOperator *shifted =  BinaryOperator::Create(Instruction::LShr, op->getOperand(0), shamt , "fpr_hi_big", op);
							TruncInst *tr_hi = new TruncInst(shifted, Type::getInt64Ty(Ctx),"fpr_hi", op);	// alloca stack cookie

							// Set up intrinsic arguments
							std::vector<Value *> args;

							args.push_back(tr_hi);
							args.push_back(tr_lo);
							ArrayRef<Value *> args_ref(args);

							// Create call to intrinsic
							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(I);
							Builder.CreateCall(val, args_ref,"");

							Type *loadtype = op->getType();

							if (loadtype->isPointerTy())
							{
								loadtype = Type::getInt128Ty(Ctx);
								op->mutateType(loadtype);
							}
							Type *loadptrtype = loadtype->getPointerTo();

							Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
							BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32", op);
							//IntToPtrInst *ptr = new IntToPtrInst(ptr32,storeptrtype,"ptr",op);
							IntToPtrInst *ptr = new IntToPtrInst(ptr32,loadptrtype,"ptr",op);

							op->setOperand(0,ptr);

							//Tag the loaded value as fpld if a fatpointer was loaded
							if(op->getType() == Type::getInt128Ty(Ctx))
							{
								//errs()<<"FPR:\n"<<*op<<"\n";
								op->setName("fpld");
							}
							//errs()<<"\n=========\n"<<*ptr<<"\n-------\n"<<*op<<"\n-------\n"<<*loadtype<<"\n=========\n";

						}

						else if (auto *op = dyn_cast<GetElementPtrInst>(I))
						{
							modified=true;
							//errs()<<"\n-----------\n"<<*op<<"\n-----------\n";
							Value *offset = resolveGetElementPtr(op,D,Ctx);

							ZExtInst *zext_binop = new ZExtInst(offset, Type::getInt128Ty(Ctx), "zextarrayidx", op);
							BinaryOperator *binop =  BinaryOperator::Create(Instruction::Add, op->getOperand(0), zext_binop , "arrayidx", op);
							
							std::stack <User *> users;
							std::stack <int> pos;

							op->replaceAllUsesWith(binop);

							/*for (auto &U : op->uses())
							{
								User *user = U.getUser();
								users.push(user);
								pos.push(U.getOperandNo());
							}

							while(users.size())
							{
								User *u = users.top();
								users.pop();
								int index = pos.top();
								pos.pop();
								u->setOperand(index, binop);	
							}*/

							--i;
							op->dropAllReferences();
							op->removeFromParent();
						}
						else if (auto *op = dyn_cast<BitCastInst>(I))
						{
							//errs()<<"\n-----------\n"<<*(op->getSrcTy())<<", "<<*(op->getDestTy())<<"\n-----------\n";
							//if(op->getSrcTy() == Type::getInt128Ty(Ctx) && op->getDestTy()->isPointerTy())
							//{
								//errs()<<"\n-----------\n"<<*(op->getSrcTy())<<", "<<*(op->getDestTy())<<"\n-----------\n";
							op->replaceAllUsesWith(op->getOperand(0));
							--i;
							op->dropAllReferences();
							op->removeFromParent();
							//}
						}
						else if (auto *op = dyn_cast<CallInst>(I))
						{
							//errs()<<*op<<"\n";
							//errs()<<op->getCalledFunction();
							//errs()<<"\n";
							if(op->getCalledFunction() != NULL)
							{
								if(!(op->getCalledFunction()->isDeclaration())) // skip if definition exists in module
									continue;
								if(op->getCalledFunction()->getName().contains("safefree"))
									continue;
							}
							//errs()<<"\n*************************************************\n";
							//errs()<<*op<<"\n";
							for(unsigned int i=0;i<op->getNumOperands()-1;i++)
							{
								if(!op->getOperand(i)->getName().contains("arrayidx") && !op->getOperand(i)->getName().contains("fpr") && !op->getOperand(i)->getName().contains("fpld"))
								{
									//errs()<<"op "<<i<<".\t"<<*op->getOperand(i)->getType()<<"\n";
									continue;
								}

								TruncInst *tr_lo = new TruncInst(op->getOperand(i), Type::getInt64Ty(Ctx),"fpr_low", op);	// alloca stack cookie
								Value* shamt = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
								BinaryOperator *shifted =  BinaryOperator::Create(Instruction::LShr, op->getOperand(i), shamt , "fpr_hi_big", op);
								TruncInst *tr_hi = new TruncInst(shifted, Type::getInt64Ty(Ctx),"fpr_hi", op);	// alloca stack cookie

								// Set up intrinsic arguments
								std::vector<Value *> args;

								args.push_back(tr_hi);
								args.push_back(tr_lo);
								ArrayRef<Value *> args_ref(args);

								// Create call to intrinsic
								IRBuilder<> Builder(I);
								Builder.SetInsertPoint(I);
								Builder.CreateCall(val, args_ref,"");

								//errs()<<*op<<"\n";
								Type *ptype = op->getCalledFunction()->getFunctionType()->params()[i];
								//errs()<<i<<".\t"<<*ptype<<"\n";

								Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
								BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32", op);

								IntToPtrInst *ptr = new IntToPtrInst(ptr32,ptype,"ptr",op);

								op->setOperand(i,ptr);
								op->getOperand(i)->mutateType(ptype);
								//errs()<<*op<<"\n";
								//errs()<<*(op->getOperand(i)->getType())<<"\n";
							}
							//errs()<<"\n=************************************************\n";//*/
						}
						//errs()<<*I;
						//errs()<<"\n--------\n";

					}
				}
				//errs()<<"\n*************************************************\n";
				//errs()<<F;
				//errs()<<"\n*************************************************\n";
			}
			//errs()<<"Third pass done\n";

			// Fourth pass Fixes argument types in function calls within the module
			Module::FunctionListType &functions = M.getFunctionList();
			std::stack< Function * > to_replace_functions;
			std::stack< Function * > replace_with_functions;
			std::stack< int > argCount;

			for (Module::FunctionListType::iterator it = functions.begin(), it_end = functions.end(); it != it_end; ++it)
			{
				//Function *func = dyn_cast<Function>(it);
				Function &func = *it;

				LLVMContext &Ctx = func.getContext();
				if(func.isDeclaration())
				{
					//errs()<<"\n*****\n"<<func<<"\n*****\n";
					continue;
				}

				std::vector<Type*> fParamTypes;
				bool fnHasPtr = false;
				fnHasPtr = (func.getReturnType()->isPointerTy() ? true : false);

				int arg_index = 0;

				Type *fRetType = (func.getReturnType()->isPointerTy() ? Type::getInt128Ty(Ctx) : func.getReturnType());
				for(FunctionType::param_iterator k = (func.getFunctionType())->param_begin(), endp = (func.getFunctionType())->param_end(); k != endp; ++k)
				{
					arg_index++;
					if((*k)->isPointerTy())
					{
						//j->mutateType(Type::getInt128Ty(Ctx));
						fnHasPtr = true;
						fParamTypes.push_back(Type::getInt128Ty(Ctx));
						//errs()<<**k<<"\n";
					}
					else
						fParamTypes.push_back(*k);
					//errs()<<"\npushed "<<*(fParamTypes.back());
				}
				if(!fnHasPtr)
					continue;
				FunctionType *fType = FunctionType::get(fRetType, fParamTypes, func.getFunctionType()->isVarArg());
				Function *NF = Function::Create(fType, func.getLinkage(), func.getName());
				NF->copyAttributesFrom(&func);
				NF->getBasicBlockList().splice(NF->begin(), func.getBasicBlockList());
				to_replace_functions.push(&func);
				replace_with_functions.push(NF);
				argCount.push(arg_index);
			}

			while(!to_replace_functions.empty())
			{
				Function *funcx = to_replace_functions.top();
				Function *funcy = replace_with_functions.top();
				int arg_index = argCount.top();
				//errs()<<"\nto replace: "<<*funcx<<"\n";
				to_replace_functions.pop();
				replace_with_functions.pop();
				argCount.pop();
				M.getFunctionList().push_back(funcy);
				while (!funcx->use_empty()) {
					//errs()<<"USER:\n"<<*(funcx->user_back())<<"\n";
					CallSite CS(funcx->user_back());
					std::vector< Value * > args(CS.arg_begin(), CS.arg_end());
					Instruction *call = CS.getInstruction();
					Instruction *new_call = NULL;
					const AttributeList &call_attr = CS.getAttributes();
					new_call = CallInst::Create(funcy, args, "", call);
					CallInst *ci = cast< CallInst >(new_call);
					ci->setCallingConv(CS.getCallingConv());
					ci->setAttributes(call_attr);
					if (ci->isTailCall())
						ci->setTailCall();
					new_call->setDebugLoc(call->getDebugLoc());
					if (!call->use_empty())
					{
						call->replaceAllUsesWith(new_call);
					}
					new_call->takeName(call);
					call->eraseFromParent();
				}//

				Function::arg_iterator arg_i2 = funcy->arg_begin();

				for(Function::arg_iterator arg_i = funcx->arg_begin(), 
					arg_e = funcx->arg_end(); arg_i != arg_e; ++arg_i)
				{
					arg_i->replaceAllUsesWith(arg_i2);
					arg_i2->takeName(arg_i);
					++arg_i2;
					arg_index++;
				}

				//errs()<<"\n*************************************************\n";
				//errs()<<funcy->getName()<<"\n-----\n"<<*(funcy->getFunctionType());
				//errs()<<"\n*************************************************\n";
				funcx->dropAllReferences();
				funcx->removeFromParent();
			}
			//errs()<<"Fourth pass done\n";

			//errs()<<"\n--------------\n"<<M<<"\n----------------\n";
			return modified;
		}
	};
}

char shaktiPass::ID = 0;
static RegisterPass<shaktiPass> X("t", "Shakti-T transforms");
