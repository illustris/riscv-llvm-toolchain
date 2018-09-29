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
#include <map>
#include <set>
//#define debug_spass
#define debug_spass_dmodule

using namespace llvm;

Value* resolveGetElementPtr(GetElementPtrInst *GI,DataLayout *D,LLVMContext &Context);

namespace {
	struct shaktiPass : public ModulePass
	{
		static char ID;
		shaktiPass() : ModulePass(ID) {}

		std::map <StructType*, StructType*> rep_structs;

		virtual bool runOnModule(Module &M)
		{


			/* before doing anything check whether pointer decrements are there or not in any function. */
			std::set<std::string> func_name;
			for (auto &F : M){
				if (!F.isDeclaration()) {
					for (auto &B : F){
						for (auto &I : B){
							if(GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(&I)){
								if(ConstantInt *CI = dyn_cast<ConstantInt>(GI->getOperand(1))){
									if(CI->isNegative()){

										//errs() << "!!!!!!!!!!!Warning Pointer Decrement in function  : " << F.getName() << "!!!!!!!!!!!!!!!!!!!\n" ;
										//errs() << *GI->getOperand(0) << "\n" ;
										func_name.insert(F.getName());

									}
								}else{
									if(Instruction *I = dyn_cast<Instruction>(GI->getOperand(1))){
										if(I->getOpcode() == Instruction::Sub){

											//errs() << "!!!!!!!!!!!Warning Pointer Decrement in function  : " << F.getName() << "!!!!!!!!!!!!!!!!!!!\n" ;
											//errs() << *GI->getOperand(0) << "\n" ;
											func_name.insert(F.getName());
										}
									}
								}

							}
						}
					}
				}
			}
			if(!func_name.empty()) {
				errs () << " !!!!!!!!!!! Warning Pointer Decrement in function " ;
				std::set<std::string>:: iterator itr;
				for (itr = func_name.begin(); itr != func_name.end(); ++itr)
			    {
			        errs() << *itr << " " ;
			    }
			    errs() << " !!!!!!!!!!!!!!!!!!! \n" ;
			}
			/*checking for pointer decrement ends here .*/

			bool modified=false;

			unsigned long long ro_cook = 0xdeadbeef1337c0d3;
			unsigned long ro_hash = 0xcd9a7e3c;
			GlobalVariable *rodata_cookie = new GlobalVariable(M, Type::getInt64Ty(M.getContext()), true, GlobalValue::LinkageTypes::PrivateLinkage, ConstantInt::get(Type::getInt64Ty(M.getContext()),ro_cook), "ro_cookie");

			// First pass creates duplicate definitions of structs with pointers
			std::vector< StructType * > structs = M.getIdentifiedStructTypes();
			for(auto &def : structs)
			{
				//errs()<<*def<<"\n";
				LLVMContext &GCtx = def->getContext();
				bool flag = false;
				std::vector<Type *> elems_vec;
				for(StructType::element_iterator i = def->element_begin(), end = def->element_end(); i!= end; ++i)
				{
					Type *ty = dyn_cast<Type>(*i);
					//errs()<<*ty<<"\n";
					bool isFnArr = false;
					//if element type is of function pointers then dont make any changes.
					if(dyn_cast<PointerType>(ty)){
						isFnArr = dyn_cast<PointerType>(ty)->getElementType()->isFunctionTy();
					}
					if(ty->isPointerTy())
					{
						//convert normal pointers to i128 leave function pointers.
						//but check each elements of function pointers and if they have pointers then convert them to i128
						if(!isFnArr){
							elems_vec.push_back(Type::getInt128Ty(GCtx));
						}else{
							FunctionType *func_type = dyn_cast<FunctionType>(dyn_cast<PointerType>(ty)->getElementType());
							std::vector<Type*> fParamTypes;

							Type *func_ret_type = func_type->getReturnType();
							Type *fRetType = (func_type->getReturnType()->isPointerTy() && !(dyn_cast<PointerType>(func_ret_type)->getElementType()->isFunctionTy()) ? Type::getInt128Ty(GCtx) : func_ret_type);
							for(FunctionType::param_iterator k = func_type->param_begin(), endp = func_type->param_end(); k != endp; ++k){
								bool argIsFnArr = 0;
								if(dyn_cast<PointerType>(*k)){
									argIsFnArr = dyn_cast<PointerType>(*k)->getElementType()->isFunctionTy();
								}
								if((*k)->isPointerTy() && !argIsFnArr){
									fParamTypes.push_back(Type::getInt128Ty(GCtx));
								}
								else
									fParamTypes.push_back(*k);
							}
							FunctionType *newFuncType = FunctionType::get(fRetType,fParamTypes,func_type->isVarArg());
							//errs() << "new function type : " << *newFuncType->getPointerTo() << "\n" ;
							Type *newType = newFuncType->getPointerTo() ;
							elems_vec.push_back(newType);
						}
						flag = 1;
					}
					/*else if(ty->isArrayTy())
					{
						.
					}*/
					else
					{
						elems_vec.push_back(ty);
					}
				}
				if(flag)
				{
					//errs()<<dyn_cast<StructType>(def)<<"\n";
					ArrayRef<Type *> elems(elems_vec);
					StructType *frm = dyn_cast<StructType>(def);
					StructType *to = StructType::create(elems,frm->getStructName(),frm->isPacked());
					//errs()<<"------------------\nINSERTING "<<*frm<<"\nTO "<<*to<<"\n------------------\n";
					rep_structs.insert(std::make_pair(frm, to));
				}
			}

			for(Module::global_iterator j = M.global_begin(), end = M.global_end(); j != end; ++j)
			{
				GlobalVariable *glob = dyn_cast<GlobalVariable>(j);
				LLVMContext &GCtx = glob->getContext();
				if(!glob->getType()->isPointerTy())
				{
					continue;
					//errs()<<*glob->getType()<<"\n";
				}
				//errs()<<*glob<<" : "<<glob->isConstant()<<"\n";
				if(glob->isConstant())
					continue;
				if(glob->hasInitializer())
				{
					//todo
					if(!glob->getValueType()->isPointerTy())
						continue;
						//errs()<<"HERE"<<*glob<<"\n";
					//glob->mutateType(Type::getInt128Ty(GCtx));
					GlobalVariable *glob2 = new GlobalVariable
					(
						M,
						Type::getInt128Ty(GCtx),
						glob->isConstant(),
						glob->getLinkage(),
						llvm::ConstantInt::get(Type::getInt128Ty(GCtx),0),
						"fpr_"+glob->getName(),
						glob
					);
					//errs()<<"CTXT: "<<&(glob->getContext())<<"\t"<<&(glob2->getContext())<<"\n";
					glob->replaceAllUsesWith(glob2);
					//glob2->setParent(glob->getParent());
					//errs()<<*glob<<"\n"<<*glob2<<"\n";
					j--;
					glob->dropAllReferences();
					glob->eraseFromParent();
				}
				else
				{
					//glob->mutateType(Type::getInt128Ty(GCtx));
					//errs()<<*glob<<"\n";
				}
			}
			#ifdef debug_spass
				errs()<<"First pass done\n";
			#endif

			// Second pass replaces malloc and free
			Value *mallocFunc;
			Value *freeFunc;

			for (auto &F : M)
			{
				modified=false;
				// Get the safemalloc function to call from the new library.
				LLVMContext &Ctx = F.getContext();
				DataLayout *D = new DataLayout(&M);
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
								BitCastInst *nextbc;
								//errs()<<"*******\n"<<*op<<"\n"<<*op->getNextNode()<<"\n-------\n";
								if((nextbc = dyn_cast<BitCastInst>(op->getNextNode())))
								{
									if(op == nextbc->getOperand(0))
									{
										PointerType *bcptr;
										if((bcptr = dyn_cast<PointerType>(nextbc->getDestTy())))
										{
											StructType *st;
											Type* eltype = bcptr->getElementType();
											if((st = dyn_cast<StructType>(eltype)))
											{
												if(rep_structs.find(st) != rep_structs.end())
												{
													//errs()<<"HERE: "<<*(rep_structs.at(st))<<"\n";
													StructType *to = rep_structs.at(st);
													const StructLayout *SL = D->getStructLayout(to);
													unsigned long long sz = SL->getSizeInBytes();
													//errs()<<*op->getOperand(0)<<"\n"<<*nextbc<<"\n"<<sz<<"\n";
													op->setOperand(0,llvm::ConstantInt::get(Type::getInt64Ty(Ctx),sz));
												}
											}
										}

									}
								}
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
			#ifdef debug_spass
				errs()<<"Second pass done\n";
			#endif

			dyn_cast<Function>(mallocFunc)->dropAllReferences();
			dyn_cast<Function>(freeFunc)->dropAllReferences();
			dyn_cast<Function>(mallocFunc)->removeFromParent();
			dyn_cast<Function>(freeFunc)->removeFromParent();//*/

			// Third pass adds stack cookie
			//place the stack cookie below every variable available on stack
			//then in fifth pass when you encounter alloca's then craft fatpointer belwo this 
			Instruction *FPR;
			TruncInst *ptr_to_st_hash;
			PtrToIntInst *ptr_to_st_cook;

			std::map<std::string,std::vector<Instruction *>> mapInst;

			for (auto &F : M)
			{
				if(F.isDeclaration())
					continue;
				Instruction *AI,*RI;
				bool stack_cook_ins = false,flag = false;
				LLVMContext &Ctx = F.getContext();
				Module *m = F.getParent();
				Function *hash = Intrinsic::getDeclaration(m, Intrinsic::riscv_hash);	// get hash intrinsic declaration
				AllocaInst *st_cook;	// pointer to stack cookie
				for (auto &B : F)
				{
					if(&B == &((&F)->front()))
					{	// If BB is first

						AI = &((&B)->front());	// Get first instruction in function
					}

					for (auto &I : B){	// Iterate over instructions to get the last Alloca and Return Inst
						if (dyn_cast<AllocaInst>(&I) && !stack_cook_ins)
						{
							AI = &I;
							flag = true;
						}
						else if (dyn_cast<ReturnInst>(&I))
						{	// If return instruction
							RI = &I;
						}
					}
					if(!stack_cook_ins){
						if(flag)
							FPR = AI->getNextNode();	//that means there is a alloca so go to next instruction and then insert before it
						else
							FPR = AI; //no alloca inst is there. only printf. then insert before printf

						//errs() << *I << "\n" ;
						st_cook = new AllocaInst(Type::getInt64Ty(Ctx), 0,"stack_cookie", FPR);	// alloca stack cookie
						ptr_to_st_cook = new PtrToIntInst(st_cook,Type::getInt32Ty(Ctx), "stack_cookie_32", FPR);

						// Set up intrinsic arguments
						std::vector<Value *> args;
						args.push_back(st_cook);
						ArrayRef<Value *> args_ref(args);

						// Create call to intrinsic
						IRBuilder<> Builder(FPR);
						Builder.SetInsertPoint(FPR);
						Value *hash64 = Builder.CreateCall(hash, args_ref,"stack_hash_long");
						ptr_to_st_hash = new TruncInst(hash64, Type::getInt32Ty(Ctx),"stack_hash", FPR);
						stack_cook_ins = true;
						modified  = true;
						//errs() << "Stack Cookiner Inserted \n" ;
					}
				}
				//set up arguments
				std::vector<Value *> args;
				args.push_back(st_cook);
				ArrayRef<Value *> args_ref(args);

				//Call hash again to burn the cookie
				IRBuilder<> Builder(RI);
				Builder.SetInsertPoint(RI);
				Builder.CreateCall(hash, args_ref,"stack_cookie_burn");
				//errs() << "Adding done in function " << F.getName() << "\n" ;

				std::vector<Instruction *> storeInsPoint;
				storeInsPoint.push_back(FPR);
				storeInsPoint.push_back(ptr_to_st_cook);
				storeInsPoint.push_back(ptr_to_st_hash);
				mapInst.insert(std::pair<std::string,std::vector<Instruction *>>(F.getName(),storeInsPoint));

			}
			#ifdef debug_spass
				errs()<<"Third pass done\n";
			#endif

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
					//errs()<<"\n*****\nFound declaration:\n"<<func<<"\n*****\n";
					continue;
				}
				//errs()<<"\n*****\nFound definition:\n"<<func<<"\n*****\n";

				std::vector<Type*> fParamTypes;
				bool fnHasPtr = false;
				fnHasPtr = (func.getReturnType()->isPointerTy() ? true : false);

				int arg_index = 0;

				bool isFnArr = 0;
				if(dyn_cast<PointerType>(func.getReturnType()))
				{
					isFnArr = dyn_cast<PointerType>(func.getReturnType())->getElementType()->isFunctionTy();
				}

				Type *fRetType = (func.getReturnType()->isPointerTy() && !(isFnArr) ? Type::getInt128Ty(Ctx) : func.getReturnType());
				for(FunctionType::param_iterator k = (func.getFunctionType())->param_begin(), endp = (func.getFunctionType())->param_end(); k != endp; ++k)
				{
					arg_index++;
					bool argIsFnArr = 0;
					if(dyn_cast<PointerType>(*k))
					{
						argIsFnArr = dyn_cast<PointerType>(*k)->getElementType()->isFunctionTy();
					}
					if((*k)->isPointerTy() && !argIsFnArr)
					{
						//j->mutateType(Type::getInt128Ty(Ctx));
						fnHasPtr = true;
						fParamTypes.push_back(Type::getInt128Ty(Ctx));
						//errs()<<**k<<"\n";

						//if the function returns a structure then remove its attributes. 
						//struct return is basically function argument 1 in llvm IR
						if(func.hasStructRetAttr()){
							//errs() << "Struct type detected . so remove all attributes\n" ;
							AttributeList AL = func.getAttributes();
							AttributeList A = AL.removeParamAttributes(Ctx,0);
							func.setAttributes(A);
						}
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
			//errs()<<"HERE\n";

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
				while (!funcx->use_empty())
				{
					if(dyn_cast<CallInst>(funcx->user_back()))
					{
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
					else if(dyn_cast<StoreInst>(funcx->user_back()))
					{
						StoreInst *op = dyn_cast<StoreInst>(funcx->user_back());
						//errs()<<"USER:\n"<<*(funcx->user_back())<<"\n";
						//errs()<<*funcy->getType()->getPointerTo()<<"\n";
						dyn_cast<StoreInst>(funcx->user_back())->setOperand(0,funcy);
						//errs()<<*op->getOperand(1)<<"\n";
						IntToPtrInst *op2;
						if((op2 = dyn_cast<IntToPtrInst>(op->getOperand(1))))
						{
							op2->mutateType(funcy->getType()->getPointerTo());
						}
						//errs()<<*op<<"\n";
					}
					else
					{
						//errs()<<"UNKNOWN USER:\n"<<*(funcx->user_back())<<"\n";
						continue;
					}
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
				StringRef tmp_name = funcx->getName();
				funcx->dropAllReferences();
				funcx->removeFromParent();
				funcy->setName(tmp_name);
			}
			#ifdef debug_spass
				errs()<<"Fourth pass done\n";
			#endif

			// Fifth pass replaces pointers, store and load
			//check the value of ptr_to_st_cook and ptr_to_st_hash
			for (auto &F : M)
			{
				DataLayout *D = new DataLayout(&M);

				LLVMContext &Ctx = F.getContext();
				std::vector<Type*> craftParamTypes = {Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx)};
				Type *craftRetType = Type::getInt128Ty(Ctx);
				FunctionType *craftFuncType = FunctionType::get(craftRetType, craftParamTypes, false);
				Value *craftFunc = F.getParent()->getOrInsertFunction("craft", craftFuncType);

				if(F.isDeclaration())
					continue;

				std::vector<Instruction *> getData = mapInst.find(F.getName())->second;
				if(getData.empty()){
					errs() << "Something is wrong with this function \n"  ;
					continue;
				}
				Instruction *insertPoint = getData[0];
				ptr_to_st_cook = dyn_cast<PtrToIntInst>(getData[1]);
				ptr_to_st_hash = dyn_cast<TruncInst>(getData[2]);

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
						//errs()<<*I<<"\n";

						if (auto *op = dyn_cast<AllocaInst>(I))
						{

							if(rep_structs.find(dyn_cast<StructType>(op->getAllocatedType())) != rep_structs.end())
							{
								op->mutateType(rep_structs.at(dyn_cast<StructType>(op->getAllocatedType()))->getPointerTo());
								op->setAllocatedType(rep_structs.at(dyn_cast<StructType>(op->getAllocatedType())));
								//errs()<<*op->getAllocatedType()<<"\n";
							}
							modified=true;
							if(op->getAllocatedType()->isFunctionTy())
							{
								//errs()<<"Ignoring function pointer "<<*op<<"\n";
								continue;
							}
							//errs()<<"\n-----------\nAlloca:\t"<<*op<<"\n-----------\n";
						 	else if(op->getAllocatedType()->isPointerTy())
							{
								//errs()<<"\n-----------\nptrAlloca:\t"<<*op<<"\n-----------\n";
								bool isFnArr = false;
								isFnArr = dyn_cast<PointerType>(op->getAllocatedType())->getElementType()->isFunctionTy();

								if(isFnArr){

									FunctionType *func_type = dyn_cast<FunctionType>(dyn_cast<PointerType>(op->getAllocatedType())->getElementType());
									std::vector<Type*> fParamTypes;

									//errs() << "function type : " << *func_type << "\n";
									Type *func_ret_type = func_type->getReturnType();
									Type *fRetType = (func_type->getReturnType()->isPointerTy() && !(dyn_cast<PointerType>(func_ret_type)->getElementType()->isFunctionTy()) ? Type::getInt128Ty(Ctx) : func_ret_type);
									for(FunctionType::param_iterator k = func_type->param_begin(), endp = func_type->param_end(); k != endp; ++k){
										//errs() << **k << "\n";
										bool argIsFnArr = 0;
										if(dyn_cast<PointerType>(*k))
										{
											argIsFnArr = dyn_cast<PointerType>(*k)->getElementType()->isFunctionTy();
										}
										if((*k)->isPointerTy() && !argIsFnArr)
										{
											//fnHasPtr = true;
											fParamTypes.push_back(Type::getInt128Ty(Ctx));
										}
										else
											fParamTypes.push_back(*k);
									}
//									FunctionType *fType =       FunctionType::get(fRetType, fParamTypes, func.getFunctionType()->isVarArg());
									FunctionType *newFuncType = FunctionType::get(fRetType,fParamTypes,func_type->isVarArg());
									//errs() << "new function type : " << *newFuncType->getPointerTo() << "\n" ;
									Type *newType = newFuncType->getPointerTo() ;
									op->setAllocatedType(newType);
									op->mutateType(newType);
									continue;
								}
								op->setAllocatedType(Type::getInt128Ty(Ctx));
								op->mutateType(Type::getIntNPtrTy(Ctx,128));
								/*for (auto &U : op->uses())
								*{
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

								bool isFnArr = 0;
								if(dyn_cast<PointerType>(baseTy))
								{
									//errs()<<"Found array: "<<*op<<", baseTy = "<<dyn_cast<PointerType>(baseTy)->getElementType()->isFunctionTy()<<"\n";
									isFnArr = dyn_cast<PointerType>(baseTy)->getElementType()->isFunctionTy();
								}
								if(baseTy->isPointerTy() && !(isFnArr))	//Only do if array is ptr array, and not a fn ptr array
								{
									//errs()<<"\n*********\nALLOCATED TYPE = "<<*op->getAllocatedType()->getArrayElementType()<<"\n\n";
									//errs()<<*(ArrayType::get(Type::getInt128Ty(Ctx),op->getAllocatedType()->getArrayNumElements()))<<"\n";
									op->setAllocatedType(gelType);
									op->mutateType(gelType->getPointerTo());

								}
								//errs()<<*op<<"\n";
							}
							if (op->getName() == "stack_cookie")
							{
								//ptr_to_st_cook = dyn_cast<PtrToIntInst>(op->getNextNode());
								//ptr_to_st_hash = dyn_cast<TruncInst>(op->getNextNode()->getNextNode()->getNextNode());
								continue;
							}

							//do not create a fat pointer if its a normal integer or float or double . 
							if(!op->getAllocatedType()->isIntegerTy(128) && (op->getAllocatedType()->isFloatTy() || op->getAllocatedType()->isDoubleTy() || op->getAllocatedType()->isIntegerTy())){
								continue;
							}

							//PtrToIntInst *trunc = new PtrToIntInst(op, Type::getInt32Ty(Ctx),"pti",op->getNextNode());
							PtrToIntInst *trunc = new PtrToIntInst(op, Type::getInt32Ty(Ctx),"pti",insertPoint);

							std::vector<Value *> args;
							args.push_back(trunc);
							args.push_back(ptr_to_st_cook);
							if(dyn_cast<ConstantInt>(op->getArraySize()))
								args.push_back(
									llvm::ConstantInt::get(
										Type::getInt32Ty(Ctx),
										(D->getTypeAllocSize(op->getAllocatedType()))
									)
								);
							else
							{
								BinaryOperator *total_off =  BinaryOperator::Create(Instruction::Mul, op->getArraySize(), llvm::ConstantInt::get(Type::getInt64Ty(Ctx),(D->getTypeAllocSize(op->getAllocatedType()))) , "off", insertPoint);
								TruncInst *total_off_trunc = new TruncInst(total_off, Type::getInt32Ty(Ctx),"offt", insertPoint);
								args.push_back(total_off_trunc);
							}
							args.push_back(ptr_to_st_hash);
							ArrayRef<Value *> args_ref(args);

							IRBuilder<> Builder(insertPoint);
							//Builder.SetInsertPoint(trunc->getNextNode());
							Builder.SetInsertPoint(insertPoint);
							Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fpr");

							//std::stack <User *> users;
							//std::stack <int> pos;

							// Replace all uses of pointer with fatpointer
							op->replaceAllUsesWith(fpr);
							// except in the ptrtoint instruction that uses the pointer to make a fatpointer
							trunc->setOperand(0,op);

						}

						else if (auto *op = dyn_cast<StoreInst>(I)) {
							//if(op->getParent()->getParent()->getName().contains("compressStream"))
							//	if(op->getPrevNode() != NULL)
							//		if(dyn_cast<CallInst>(op->getPrevNode()))
							//			errs()<<*op<<"\n";
							if(dyn_cast<ConstantPointerNull>(op->getOperand(0)))
							{
								//op->getOperand(0)->mutateType(Type::getInt128Ty(Ctx));
								Value* nullconst = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),0);
								op->setOperand(0, nullconst);
								//errs()<<"FOUND CONSTANT\n"<<*op->getOperand(0)<<"\n"<<op->getOperand(0)->getNumUses()<<"\n";
							}
							//This happens when trying to store fatpointers to pinters inside special structs
							//errs()<<"*********\n"<<*op<<"\n"<<*op->getOperand(1)<<"\n"<<*op->getOperand(1)->getType()<<"\nxxxxxxxx\n";

							if(op->getOperand(1)->getType() != Type::getInt128Ty(Ctx))
							{
								//errs()<<*op->getOperand(0)<<"\n"<<*op->getOperand(0)->getType()<<"\n";
								// if storing non-i128 object to a regular pointer, do nothing
								//if storing non-i128 object to a i128* then create non-128 to i128
								if(op->getOperand(0)->getType() != Type::getInt128Ty(Ctx))
								{
									//means storing a non-i128 object to i128*
									if(dyn_cast<PointerType>(op->getOperand(1)->getType())->getElementType() == Type::getInt128Ty(Ctx)){
										// do something here.
										//craft fpr, store fpr
										PtrToIntInst *trunc = new PtrToIntInst(op->getOperand(0), Type::getInt32Ty(Ctx),"pti1_",op);

										std::vector<Value *> args;
										args.push_back(trunc);//ptr
										PtrToIntInst *ptr32 = new PtrToIntInst(rodata_cookie, Type::getInt32Ty(Ctx),"ptr32_1_",op);
										args.push_back(ptr32);//base
										args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),0));//TODO bound
										args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),ro_hash));//hash
										ArrayRef<Value *> args_ref(args);

										IRBuilder<> Builder(I);
										Builder.SetInsertPoint(op);
										Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fprz");
										op->setOperand(0,fpr);
									}
									if(dyn_cast<PointerType>(op->getOperand(1)->getType())->getElementType()->isFunctionTy()){
										Type *storeptrtype = op->getOperand(1)->getType()->getPointerTo(); 
										//errs() << "store pointer type : " << *storeptrtype << "\n" ;
										op->getOperand(1)->mutateType(storeptrtype);
									}
									continue;
								}
								//else if storing i128 to i128*, do nothing
								else if(dyn_cast<PointerType>(op->getOperand(1)->getType())->getElementType() == Type::getInt128Ty(Ctx))
									continue;

								//errs()<<"<HERE>\n";
								// get here only if op 0 is i128 and operand1 != i128*
								Type *op1Ty = dyn_cast<PointerType>(op->getOperand(1)->getType())->getElementType();
								//validate, truncate and store operand 0
								modified = true;
								TruncInst *tr_lo1 = new TruncInst(op->getOperand(0), Type::getInt64Ty(Ctx),"fpr_lowx", op);	// alloca stack cookie
								Value* shamt1 = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
								BinaryOperator *shifted1 =  BinaryOperator::Create(Instruction::LShr, op->getOperand(0), shamt1 , "fpr_hi_big", op);
								TruncInst *tr_hi1 = new TruncInst(shifted1, Type::getInt64Ty(Ctx),"fpr_hix", op);	// alloca stack cookie

								std::vector<Value *> args1;

								args1.push_back(tr_hi1);
								args1.push_back(tr_lo1);
								ArrayRef<Value *> args_ref1(args1);

								// Create call to intrinsic
								IRBuilder<> Builder(I);
								Builder.SetInsertPoint(I);
								Builder.CreateCall(val, args_ref1,"");

								Value* mask1 = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
								BinaryOperator *ptr32_1 =  BinaryOperator::Create(Instruction::And, tr_lo1, mask1 , "ptr32x", op);
								IntToPtrInst *ptr1 = new IntToPtrInst(ptr32_1,op1Ty,"ptrx",op);

								//new StoreInst(ptr1,op->getOperand(1),op);
								op->setOperand(0,ptr1);

								//i--;
								//i--;
								//op->dropAllReferences();
								//op->removeFromParent();
								continue;
							}

							if(op->getOperand(0)->getType()->isPointerTy())
							{
								if(!(dyn_cast<PointerType>(op->getOperand(0)->getType())->getElementType()->isFunctionTy()))
								{
									if(op->getOperand(1)->getType() == Type::getInt128Ty(Ctx))
									{
										//craft fpr, store fpr
										PtrToIntInst *trunc = new PtrToIntInst(op->getOperand(0), Type::getInt32Ty(Ctx),"pti1_",op);

										std::vector<Value *> args;
										args.push_back(trunc);//ptr
										PtrToIntInst *ptr32 = new PtrToIntInst(rodata_cookie, Type::getInt32Ty(Ctx),"ptr32_1_",op);
										args.push_back(ptr32);//base
										args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),0));//TODO bound
										args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),ro_hash));//hash
										ArrayRef<Value *> args_ref(args);

										IRBuilder<> Builder(I);
										Builder.SetInsertPoint(op);
										Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fprz");
										op->setOperand(0,fpr);
										//errs()<<"***\n"<<*op<<"\n"<<*op->getOperand(0)<<"\n"<<*op->getOperand(1)<<"\nxxx\n";
									}
								}
							}

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
							bool isFnArr = 0;
							if(dyn_cast<PointerType>(storetype))
							{
								isFnArr = dyn_cast<PointerType>(storetype)->getElementType()->isFunctionTy();
							}
							if (storetype->isPointerTy() && !isFnArr)
							{
								storetype = Type::getInt128Ty(Ctx);
								op->mutateType(storetype);
							}
							Type *storeptrtype = storetype->getPointerTo();

							Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
							BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32_", op);
							IntToPtrInst *ptr = new IntToPtrInst(ptr32,storeptrtype,"ptrs",op);

							//new StoreInst(op->getOperand(0),ptr,op);
							op->setOperand(1, ptr);

							//--i;
							//op->dropAllReferences();
							//op->removeFromParent();

						}

						else if (auto *op = dyn_cast<LoadInst>(I))
						{
							//errs()<< *op << "\n";
							if(op->getOperand(0)->getType() == Type::getIntNPtrTy(Ctx,128))
							{
								op->mutateType(Type::getInt128Ty(Ctx));
								continue;
							}
							//errs()<<*op<<"\t"<<*op->getOperand(0)<<"\n";
							if(op->getOperand(0)->getType() != Type::getInt128Ty(Ctx)){

								//errs() << "load inst : " << *op->getOperand(0)->getType() << "\n" ;
								Type *base_type = op->getOperand(0)->getType() ;
								int depth = 0;
								while(dyn_cast<PointerType>(base_type)){
									base_type = base_type->getPointerElementType();
									depth++;
								}
								//errs() << "final base type : " << *base_type << "\n-------------------------\n" ;
								depth--;
								if(base_type->isFunctionTy()){
									while(depth){
										base_type  = base_type->getPointerTo();
										depth--;
									}
									op->mutateType(base_type);
								}
								continue;
							}
							Type *loadtype = op->getType();
							bool isFnArr = 0;
							if(dyn_cast<PointerType>(loadtype))
							{
								isFnArr = dyn_cast<PointerType>(loadtype)->getElementType()->isFunctionTy();
								//errs()<<"LOADTYPE: "<<*loadtype<<"IsFn: "<<isFnArr<<"\n";
							}

							/*if(loadtype->isPointerTy() && !isFnArr && op->getOperand(0)->getName().contains("arrayidx")){

								if(dyn_cast<Instruction>(op->getNextNode())->getOpcode() == Instruction::BitCast){

									op->replaceAllUsesWith(op->getOperand(0));
									--i;
									op->dropAllReferences();
									op->removeFromParent();
									continue;
								}
							}*/



							if (loadtype->isPointerTy())
							{
								if(!isFnArr){
									loadtype = Type::getInt128Ty(Ctx);
									//op->mutateType(loadtype);
								}else{
									//check elements of function type and change any pointer to i128
									FunctionType *func_type = dyn_cast<FunctionType>(dyn_cast<PointerType>(loadtype)->getElementType()) ;
									std::vector<Type*> fParamTypes;

									//errs() << "function type : " << *func_type << "\n";
									Type *func_ret_type = func_type->getReturnType();
									Type *fRetType = (func_type->getReturnType()->isPointerTy() && !(dyn_cast<PointerType>(func_ret_type)->getElementType()->isFunctionTy()) ? Type::getInt128Ty(Ctx) : func_ret_type);
									for(FunctionType::param_iterator k = func_type->param_begin(), endp = func_type->param_end(); k != endp; ++k){
										//errs() << **k << "\n";
										bool argIsFnArr = 0;
										if(dyn_cast<PointerType>(*k))
										{
											argIsFnArr = dyn_cast<PointerType>(*k)->getElementType()->isFunctionTy();
										}
										if((*k)->isPointerTy() && !argIsFnArr)
										{
											//fnHasPtr = true;
											fParamTypes.push_back(Type::getInt128Ty(Ctx));
										}
										else
											fParamTypes.push_back(*k);
									}
//									FunctionType *fType =       FunctionType::get(fRetType, fParamTypes, func.getFunctionType()->isVarArg());
									FunctionType *newFuncType = FunctionType::get(fRetType,fParamTypes,func_type->isVarArg());
									//errs() << "new function type : " << *newFuncType->getPointerTo() << "\n" ;
									Type *newType = newFuncType->getPointerTo() ;
									loadtype = newType;
								}
								//op->mutateType(loadtype);
							}

							/*if(loadtype == Type::getInt128Ty(Ctx) && op->getOperand(0)->getName().contains("arrayidx")){
								if( (dyn_cast<Instruction>(op->getNextNode())->getOpcode() == Instruction::BitCast 
 									|| dyn_cast<Instruction>(op->getNextNode())->getOpcode() == Instruction::Load)
					  				&&!(dyn_cast<Instruction>(op->getNextNode()->getNextNode())->getOpcode() == Instruction::Call)){

									op->replaceAllUsesWith(op->getOperand(0));
									--i;
									op->dropAllReferences();
									op->removeFromParent();
									continue;
								}
							}*/

							Type *loadptrtype = loadtype->getPointerTo();
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

							Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
							BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32_", op);
							IntToPtrInst *ptr = new IntToPtrInst(ptr32,loadptrtype,"ptrl",op);

							op->setOperand(0,ptr);
							op->mutateType(loadtype);

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
							if(op->getOperand(0)->getType()->isPointerTy())
							{
								PointerType *pt1 = dyn_cast<PointerType>(op->getOperand(0)->getType());
								//StructType *st1 = dyn_cast<StructType>(op->getOperand(0)->getType()->getElementType());
								//errs()<<"GETELEMENTPTR name: "<<*pt1->getElementType()<<"\n";
								if(pt1->getElementType()->isStructTy())
								{
									StructType *st1 = dyn_cast<StructType>(pt1->getElementType());
									if(st1->getName().contains("_reent") || st1->getName().contains("spec_fd_t"))
									{
										//errs()<<"STRUCT NAME: "<<st1->getName()<<"\n";
										continue;
									}
								}
							}
							//temp fix
							if(op->getOperand(0)->getName().contains("spec_fd"))
							{
								continue;
							}
							if(op->getOperand(0)->getType()->isPointerTy())
							{
								continue;
							}

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
							//errs()<<"BITCAST: "<<*op<<"\n";
							//errs()<<"\n-----------\n"<<*(op->getSrcTy())<<", "<<*(op->getDestTy())<<"\n-----------\n";

							//if(op->getSrcTy() == Type::getInt128Ty(Ctx) && op->getDestTy()->isPointerTy())
							//{
								//errs()<<"\n-----------\n"<<*(op->getSrcTy())<<", "<<*(op->getDestTy())<<"\n-----------\n";
							if(op->getOperand(0)->getType() == Type::getInt128Ty(Ctx))
							{
								op->replaceAllUsesWith(op->getOperand(0));
								--i;
								op->dropAllReferences();
								op->removeFromParent();
							}
						}
						else if (auto *op = dyn_cast<CallInst>(I))
						{
							//errs()<<*op<<"\n";
							//errs()<<"\n";

							if(op->getCalledFunction() ==  NULL){
								FunctionType *ftp =dyn_cast<FunctionType>(dyn_cast<PointerType>(op->getCalledValue()->getType())->getElementType()); 
								op->mutateFunctionType(ftp);
								//continue;
							}
							for(unsigned int i=0;i<op->getNumOperands()-1;i++)
							{
								if(op->getOperand(i) != NULL)
								{
									if(op->getOperand(i)->getType()->isPointerTy())
									{
										if(dyn_cast<ConstantPointerNull>(op->getOperand(i)))
										{
											op->setOperand(i,llvm::ConstantInt::get(Type::getInt128Ty(Ctx),0));
											continue;
										}
										//check if it is not a declaration and i8* of getelement ptr or not
										if(!op->getCalledFunction()->isDeclaration() ){

											PtrToIntInst *trunc = new PtrToIntInst(op->getOperand(i), Type::getInt32Ty(Ctx),"pti1_",op);
											std::vector<Value *> args;
											args.push_back(trunc);//ptr
											PtrToIntInst *ptr32 = new PtrToIntInst(rodata_cookie, Type::getInt32Ty(Ctx),"ptr32_1_",op);
											args.push_back(ptr32);//base

											args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),0));//TODO bound
											args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),ro_hash));//hash
											ArrayRef<Value *> args_ref(args);

											IRBuilder<> Builder(I);
											Builder.SetInsertPoint(op);
											Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fprz");
											op->setOperand(i,fpr);

										}
									}
								}
							}
							if(op->getCalledFunction() != NULL)
							{
								if(!(op->getCalledFunction()->isDeclaration())) // skip if definition exists in module
								{
									//errs()<<"\n=************************************************\n";

									//if the function 1st paramter type is i128 then remove all attribuetes of the parameter
									//this is a problem for returning structs.
									AttributeList AL = op->getAttributes();
									AttributeList A = AL.removeParamAttributes(Ctx,0);
									op->setAttributes(A);
									continue;
								}
								if(op->getCalledFunction()->getName().contains("safefree"))
								{
									//errs()<<"\n=************************************************\n";
									continue;
								}
							}
							//errs()<<*op<<"\n";
							for(unsigned int i=0;i<op->getNumOperands()-1;i++)
							{
								//this means call parameters match function signature. else break it into required format.
								if(op->getOperand(i)->getType() == op->getFunctionType()->getParamType(i))
									continue;

								//if you are using a global pointer in printf scanf or other system calls then collapse that pointer to i8* or to the required pointer type before calling.
								if(op->getCalledFunction()!=NULL ) { //&& (!op->getCalledFunction()->isIntrinsic()) //&& op->getCalledFunction()->isDeclaration() -- not required as it will readh here only if its a declaration
										if(op->getOperand(i)->getType() == Type::getInt128Ty(Ctx)){
											//errs() <<  "before : " << *op << "\n" ;
											//truncte to i8* and then pass to the function

											//validate the pointer 
											TruncInst *tr_lo = new TruncInst(op->getOperand(i), Type::getInt64Ty(Ctx),"fpr_low", op);
											Value* shamt = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
											BinaryOperator *shifted =  BinaryOperator::Create(Instruction::LShr, op->getOperand(i), shamt , "fpr_hi_big", op);
											TruncInst *tr_hi = new TruncInst(shifted, Type::getInt64Ty(Ctx),"fpr_hi", op);

											// Set up intrinsic arguments
											std::vector<Value *> args;

											args.push_back(tr_hi);
											args.push_back(tr_lo);
											ArrayRef<Value *> args_ref(args);

											// Create call to intrinsic
											IRBuilder<> Builder(I);
											Builder.SetInsertPoint(I);
											Builder.CreateCall(val, args_ref,"");

											//validating the pointer done
											//type cast to i8* 
											Type *ptype = Type::getInt8PtrTy(Ctx);
											if(op->getCalledFunction() != NULL)
											{
												if(!op->getCalledFunction()->isVarArg())
												{
													ptype = op->getCalledFunction()->getFunctionType()->params()[i];
												}
												else if(i < op->getCalledFunction()->getFunctionType()->params().size())
												{
													ptype = op->getCalledFunction()->getFunctionType()->params()[i];
												}
											}

											Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
											BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32_", op);

											IntToPtrInst *ptr = new IntToPtrInst(ptr32,ptype,"ptrc",op);

											op->setOperand(i,ptr);
											op->getOperand(i)->mutateType(ptype);
											continue;
											//errs() <<  "after : " << *op << "\n" ;
										}
								}
								//if(op->getCalledFunction()->getName() == "fprintf")
								//errs()<<"op "<<i<<".\t"<<*op->getOperand(i)<<"\n";
								if(!op->getOperand(i)->getName().contains("arrayidx") && !op->getOperand(i)->getName().contains("fpr") && !op->getOperand(i)->getName().contains("fpld"))
								{
									continue;
								}


								TruncInst *tr_lo = new TruncInst(op->getOperand(i), Type::getInt64Ty(Ctx),"fpr_low", op);	// alloca stack cookie;
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
								Type *ptype = Type::getInt8PtrTy(Ctx);;
								if(op->getCalledFunction() != NULL)
								{
									if(!op->getCalledFunction()->isVarArg())
									{
										ptype = op->getCalledFunction()->getFunctionType()->params()[i];
									}
									else if(i < op->getCalledFunction()->getFunctionType()->params().size())
									{
										ptype = op->getCalledFunction()->getFunctionType()->params()[i];
									}
								}

								//errs()<<i<<".\t"<<ptype<<"\n";

								Value* mask = llvm::ConstantInt::get(Type::getInt64Ty(Ctx),0x7fffffff);
								BinaryOperator *ptr32 =  BinaryOperator::Create(Instruction::And, tr_lo, mask , "ptr32_", op);

								IntToPtrInst *ptr = new IntToPtrInst(ptr32,ptype,"ptrc",op);

								op->setOperand(i,ptr);
								op->getOperand(i)->mutateType(ptype);
								//errs()<<*op<<"\n";
								//errs()<<*(op->getOperand(i)->getType())<<"\n";
							}
							if(op->getFunctionType()->getReturnType() == Type::getInt128Ty(Ctx))
							{
								op->mutateType(Type::getInt128Ty(Ctx));
							}
							//errs()<<"\n=************************************************\n";//*/
						}
						else if (auto *op = dyn_cast<ICmpInst>(I))
						{
							Value *o1 = op->getOperand(0);
							Value *o2 = op->getOperand(1);
							if(o1->getType() != o2->getType())
							{
								//errs()<<"TYPES: "<<*o1<<" "<<*o2<<"\n";
								if(dyn_cast<ConstantPointerNull>(o2))
								{

									//change this value to zero only if operand 0 is not a function pointer
									//else create a null valued function pointer
									if(dyn_cast<PointerType>(o1->getType())){
										if(dyn_cast<PointerType>(o1->getType())->getElementType()->isFunctionTy()){
											Constant *null_val = ConstantPointerNull::getNullValue(op->getOperand(0)->getType());
											op->setOperand(1,null_val);
											continue;
										}
									}
									op->setOperand(1,llvm::ConstantInt::get(Type::getInt128Ty(Ctx),0));
								}
								//if operand(1) is of type i8* and operand(0) of type i128
								else if(op->getOperand(1)->getType()->isPointerTy()){
									//errs() << "Here.....\n" ;
									//errs() << *op->getOperand(1) << "\n" ;
									PtrToIntInst *trunc = new PtrToIntInst(op->getOperand(1), Type::getInt32Ty(Ctx),"pti1_",op);

									std::vector<Value *> args;
									args.push_back(trunc);//ptr
									PtrToIntInst *ptr32 = new PtrToIntInst(rodata_cookie, Type::getInt32Ty(Ctx),"ptr32_1_",op);
									args.push_back(ptr32);//base

									args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),0));//TODO bound
									args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),ro_hash));//hash
									ArrayRef<Value *> args_ref(args);

									IRBuilder<> Builder(I);
									Builder.SetInsertPoint(op);
									Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fprz");
									op->setOperand(1,fpr);
								}
							}
						}
						else if (auto *op = dyn_cast<PtrToIntInst>(I))
						{
							if(op->getOperand(0)->getType() == Type::getInt128Ty(Ctx))
							{
								Value* mask2 = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),0x000000000000000000000000ffffffff);
								BinaryOperator *fpr_addr =  BinaryOperator::Create(Instruction::And, op->getOperand(0), mask2 , "fpr_addr_", op);
								TruncInst *ti_addr = new TruncInst(fpr_addr, op->getDestTy(),"fpr_ptrtoint_", op);
								op->replaceAllUsesWith(ti_addr);
								--i;
								op->dropAllReferences();
								op->removeFromParent();
							}
						}
						//TODO: this is a temporary fix for functions that return pointers to global variables
						else if (auto *op = dyn_cast<ReturnInst>(I))
						{
							//errs()<<*op<<"\n"<<op->getReturnValue();
							if(op->getReturnValue() == NULL)
								continue;

							if(!op->getReturnValue()->getType()->isPointerTy())
								continue;

							//errs()<<"HERE1\n";
							if(op->getParent()->getParent()->getReturnType()->isPointerTy())
								continue;
							//errs()<<"HERE2\n";
							// return type is not pointer, but trying to return a pointer value
							PtrToIntInst *trunc = new PtrToIntInst(op->getReturnValue(), Type::getInt32Ty(Ctx),"ret_pti_",op);

							std::vector<Value *> args;
							args.push_back(trunc);//ptr
							PtrToIntInst *ptr32 = new PtrToIntInst(rodata_cookie, Type::getInt32Ty(Ctx),"glob_cook_pti_",op);
							args.push_back(ptr32);//base

							args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),0));//TODO bound
							args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx),ro_hash));//hash
							ArrayRef<Value *> args_ref(args);

							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(op);
							Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"ret_fpr_");

							op->setOperand(0,fpr);
						}
						//errs()<<*I;
						//errs()<<"\n--------\n";
					}
				}
				//errs()<<"\n*************************************************\n";
				//errs()<<F;
				//errs()<<"\n*************************************************\n";
			}
			#ifdef debug_spass
				errs()<<"Fifth pass done\n";
			#endif

			#ifdef debug_spass_dmodule
				errs()<<"\n--------------\n"<<M<<"\n----------------\n";
			#endif
			return modified;
		}
	};
}

Value* resolveGetElementPtr(GetElementPtrInst *GI,DataLayout *D,LLVMContext &Context)
{
	int offset = 0;
	Value *Offset,*temp;
	int c = 0;
	bool isconstant = true;
	bool isNeg = false;
	if(ConstantInt *CI = dyn_cast<ConstantInt>(GI->getOperand(GI->getNumOperands()-1))){// get the last operand of the getelementptr
		c = CI->getZExtValue ();
		if(c < 0){
			c = c * -1;
			isNeg = true;
		}
	}
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
		if(!isNeg)
			offset+= SL->getElementOffset(c);
		else
			offset+= c*SL->getSizeInBytes() ; 

		if(!isconstant)
		{
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), SL->getSizeInBytes());
			IRBuilder<> builder(GI);
			Offset = builder.CreateBinOp(Instruction::Mul,Offset, temp, "tmp");
		}
		else
			offset+=c*D->getTypeAllocSize(type);
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

		/*Type *gelType = Type::getInt128Ty(Context);
		while(depth)
		{
			int sz = sizes.top();
			sizes.pop();
			gelType = ArrayType::get(gelType,sz);
			depth--;
		}*/
		//errs()<<depth<<": "<<*gelType<<"\n";
		bool isFnArr = 0;
		if(dyn_cast<PointerType>(baseTy))
		{
			isFnArr = dyn_cast<PointerType>(baseTy)->getElementType()->isFunctionTy();
		}

		if(!isconstant)
		{
			//errs()<<"\n------\n"<<*GI<<"\n";
			//temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize((baseTy->isPointerTy() && !isFnArr)?Type::getInt128Ty(Context):baseTy));
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize(t->getElementType()));
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

	if(isNeg)
		offset = offset*-1;

	if(isconstant)
		Offset = llvm::ConstantInt::get(Type::getInt32Ty(Context),offset);


	return Offset;
}


char shaktiPass::ID = 0;
static RegisterPass<shaktiPass> X("t", "Shakti-T transforms");
