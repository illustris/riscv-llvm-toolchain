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

using namespace llvm;

namespace {
	struct cookiePass : public ModulePass {
		static char ID;
		cookiePass() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M) {
			bool modified=false;
			for (auto &F : M) {
				LLVMContext &Ctx = F.getContext();
				Module *m = F.getParent();
				Function *hash = Intrinsic::getDeclaration(m, Intrinsic::riscv_hash);	// get hash intrinsic declaration
				AllocaInst *st_cook;	// pointer to stack cookie
				for (auto &B : F) {
					if(&B == &((&F)->front())) {	// If BB is first
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
					else	// if not first BB
						for (auto &I : B)	// Iterate over instructions
							if (auto *op = dyn_cast<ReturnInst>(&I)) {	// If return instruction

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

			return modified;
		}
	};
}

char cookiePass::ID = 0;
static RegisterPass<cookiePass> X("cookie", "Sample program for checking GetElementPtr instructions ");
