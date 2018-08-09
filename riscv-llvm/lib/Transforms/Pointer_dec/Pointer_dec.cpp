#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/Debug.h"
#include <set>

using namespace llvm;

class Pointer_dec : public ModulePass {
	public:
		static char ID;
		Pointer_dec() : ModulePass(ID) {
		}

		bool runOnModule(Module &M);
		int checkPointerDec(Function &F);

};

bool Pointer_dec::runOnModule(Module &M) {
	int tot_cnt = 0;
	for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		if (!F->isDeclaration()) {
			tot_cnt += checkPointerDec(*F);
		}
	}
	errs() << "Total no of Pointer decrements : " << tot_cnt << "\n" ;
	return true;
}

int Pointer_dec::checkPointerDec(Function &F) {
	std::set<std::string> pointers;

	int count_ptr_dec = 0 ;
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
		for (BasicBlock::iterator i = BB->begin(), e = BB->end(); i != e; ++i) {			
			
			if(GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(i)){
				//if we are accessing array elements then we have 3 parameters in the GetElementPtrInst . 
				//array : [10 x i32]* %a, 
				//start index : i64 0,
				//index we are accessing : i64 5

				//But if we are performing pointer increment or decrement operations then it has two parametes
				//1st : the pointer 
				//2nd : the increment or decrement

				/*
				Example 1 : Constants gt direct reflected in the statement
					p++ :   
					%6 = load i32*, i32** %p, align 8
  					%incdec.ptr = getelementptr inbounds i32, i32* %6, i32 1

  					p--:
					%7 = load i32*, i32** %p, align 8
  					%incdec.ptr3 = getelementptr inbounds i32, i32* %7, i32 -1

					p-=3;
  					%8 = load i32*, i32** %p, align 8
  					%add.ptr = getelementptr inbounds i32, i32* %8, i64 -3

  				Example 2 : Some variable value

					p+=n
  					%6 = load i32, i32* %n, align 4
  					%7 = load i32*, i32** %p, align 8
  					%idx.ext = sext i32 %6 to i64
  					%add.ptr = getelementptr inbounds i32, i32* %7, i64 %idx.ext

  					p-=n
  					%8 = load i32, i32* %n, align 4
  					%9 = load i32*, i32** %p, align 8
  					%idx.ext3 = sext i32 %8 to i64
  					%idx.neg = sub i64 0, %idx.ext3
  					%add.ptr4 = getelementptr inbounds i32, i32* %9, i64 %idx.neg
  
				Example 3: Some varibale value that turns out to be negative and we are doing p+=n where 'n' is negative.
							This we cant handle .

				*/

				//errs() << *(GI->getOperand(0)) << "\n" ;
				//errs() << *(GI->getOperand(1)) << "\n" ; //returns the 2nd parameter . i.e. i32 -1 or i64 %idx.ext 


				//getting line number for Getelementptr
				DILocation *Loc = GI->getDebugLoc();
				unsigned int Line = Loc->getLine();


				if(ConstantInt *CI = dyn_cast<ConstantInt>(GI->getOperand(1))){
					if(CI->isNegative()){
						//errs() << "Pointer Decrement \n" ;
						errs() << "!!!!!!!!!!!Warning Pointer Decrement in line no : " << Line << "!!!!!!!!!!!!!!!!!!!\n" ;
						count_ptr_dec++;
					}
				}else{
					if(Instruction *I = dyn_cast<Instruction>(GI->getOperand(1))){
						if(I->getOpcode() == Instruction::Sub){
							//errs() << "Pointer decrement \n" ;
							errs() << "!!!!!!!!!!!!!!! Warning Pointer Decrement in line no : " << Line-1 << "!!!!!!!!!!!!!!\n" ;
							count_ptr_dec++;
						}
					}
				}
				

			}
		}
	}
	//errs() << "No of pointer Decrements : " << count_ptr_dec << "\n" ;
	return count_ptr_dec;
}



char Pointer_dec::ID = 0;
static RegisterPass<Pointer_dec> X("pd", "Sample program for checking Pointer Decrement ");
