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
		if(!isconstant)
		{
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize(t->getElementType()));
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
	struct cookiePass : public ModulePass
	{
		static char ID;
		cookiePass() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M)
		{
			bool modified=false;

			// First pass adds stack cookie
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

			// Second pass replaces pointers, store and load
			for (auto &F : M)
			{
				DataLayout *D = new DataLayout(&M);

				LLVMContext &Ctx = F.getContext();
				std::vector<Type*> craftParamTypes = {Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt64Ty(Ctx)};
				Type *craftRetType = Type::getInt128Ty(Ctx);
				FunctionType *craftFuncType = FunctionType::get(craftRetType, craftParamTypes, false);
				Value *craftFunc = F.getParent()->getOrInsertFunction("craft", craftFuncType);
				CallInst *st_hash;

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

						if (auto *op = dyn_cast<StoreInst>(I)) {
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
							Type *storeptrtype = storetype->getPointerTo();

							IntToPtrInst *ptr = new IntToPtrInst(tr_lo,storeptrtype,"ptr",op);

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
							Type *loadptrtype = loadtype->getPointerTo();

							IntToPtrInst *ptr = new IntToPtrInst(tr_lo,loadptrtype,"ptr",op);

							op->setOperand(0,ptr);

						}

						else if (auto *op = dyn_cast<GetElementPtrInst>(I))
						{
							modified=true;
							Value *offset = resolveGetElementPtr(op,D,Ctx);
							//errs()<<"\n-----------\n"<<*offset<<"\n-----------\n";

							ZExtInst *zext_binop = new ZExtInst(offset, Type::getInt128Ty(Ctx), "zextarrayidx", op);
							BinaryOperator *binop =  BinaryOperator::Create(Instruction::Add, op->getOperand(0), zext_binop , "arrayidx", op);
							
							std::stack <User *> users;
							std::stack <int> pos;

							for (auto &U : op->uses())
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
						    }

						    --i;
						    op->dropAllReferences();
						    op->removeFromParent();
						}

						else if (auto *op = dyn_cast<AllocaInst>(I))
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
					}
				}
				/*errs()<<"\n*************************************************\n";
				errs()<<F;
				errs()<<"\n*************************************************\n";//*/
			}

			Module::FunctionListType &functions = M.getFunctionList();
			std::stack< Function * > to_replace_functions;
			std::stack< Function * > replace_with_functions;

			for (Module::FunctionListType::iterator it = functions.begin(), it_end = functions.end(); it != it_end; ++it)
			{
				//Function *func = dyn_cast<Function>(it);
				Function &func = *it;
				LLVMContext &Ctx = func.getContext();
				if(func.getName() == "llvm.RISCV.hash")
					continue;

				std::vector<Type*> fParamTypes;
				bool fnHasPtr = false;
				fnHasPtr = (func.getReturnType()->isPointerTy() ? true : false);

				Type *fRetType = (func.getReturnType()->isPointerTy() ? Type::getInt128Ty(Ctx) : func.getReturnType());
				for(FunctionType::param_iterator k = (func.getFunctionType())->param_begin(), endp = (func.getFunctionType())->param_end(); k != endp; ++k)
				{
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
			}

			while(!to_replace_functions.empty())
			{
				Function *funcx = to_replace_functions.top();
				Function *funcy = replace_with_functions.top();
				//errs()<<"\nto replace: "<<*funcx<<"\n";
				to_replace_functions.pop();
				replace_with_functions.pop();
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
				}
			}

			/*std::vector<Function*> origF;
			std::vector<Function*> repwithF;
			for (auto &F : M)
			{
				LLVMContext &Ctx = F.getContext();
				bool fnHasPtr = false;
				std::vector<Type*> fParamTypes;
				fnHasPtr = (F.getReturnType()->isPointerTy() ? true : false);

				Type *fRetType = (F.getReturnType()->isPointerTy() ? Type::getInt128Ty(Ctx) : F.getReturnType());

				for(FunctionType::param_iterator k = (F.getFunctionType())->param_begin(), endp = (F.getFunctionType())->param_end(); k != endp; ++k)
				{
					if((*k)->isPointerTy() && F.getName() != "llvm.RISCV.hash")
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

				if(fnHasPtr)
				{
					//errs()<<"--------------\nOld function:\n"<<F<<"\n----------------";
					FunctionType *fType = FunctionType::get(fRetType, fParamTypes, F.getFunctionType()->isVarArg());
					Function *NF = Function::Create(fType, F.getLinkage(), F.getName());
					NF->copyAttributesFrom(&F);
					NF->getBasicBlockList().splice(NF->begin(), F.getBasicBlockList());
					F.getParent()->getFunctionList().insert((&F), (&(*NF)));
					NF->takeName(&F);
					//F.replaceAllUsesWith(NF);
					//errs()<<"\nNew function: \n"<<*(NF)<<"\n--------------\n";
				}
			}//*/

			/*for (auto &F : M)
				for (auto &B : F)
					for (auto &I : B)
					{
						if (auto *op = dyn_cast<CallInst>(&I))
						{
							errs()<<"\n--------------\nFound callinst\n"<<*op<<"\n";
						}
					}//*/

			errs()<<"\n--------------\n"<<M<<"\n----------------\n";
			return modified;
		}
	};
}

char cookiePass::ID = 0;
static RegisterPass<cookiePass> X("t", "Shakti-T transforms");
