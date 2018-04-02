#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

class GetElementPtr : public ModulePass {
	public:
		static char ID;
		GetElementPtr() : ModulePass(ID) {
		}

		bool runOnModule(Module &M);
		void calculateOffset(Function &F,DataLayout*);
		Value* resolveGetElementPtr(GetElementPtrInst*,DataLayout*,LLVMContext &Context);

};

bool GetElementPtr::runOnModule(Module &M) {
	
	DataLayout *D = new DataLayout(&M);

	for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		
		if (!F->isDeclaration()) {
			calculateOffset(*F,D);
		}
	}
	return true;
}

void GetElementPtr::calculateOffset(Function &F,DataLayout *D) {


	/*
		1. Constant that I am using noew needs to be changed to fatptr 
		2. check for UnionType						//need not be handle i guess
		3. Non constants create a lot of problems. 
		4. check out the final store instruction type

	*/

	LLVMContext &Context = F.getContext();

	Value* offset;
	Value* constant = llvm::ConstantInt::get(Type::getInt128Ty(Context),0); // need to change this to fat_pointer

	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {


		for (BasicBlock::iterator i = BB->begin(), e = BB->end(); i != e; ++i) {			
			

			//%bound = (do %1 + size of struct with padding here)
			//%1_fpr = call i128 @craft(i64* %1, i64* %cookie, i64* %bound, i32 %hash)
			
			if(AllocaInst *AI = dyn_cast<AllocaInst>(i)){
				//convert pointers,arrays and structs to fat pointers.
			}

			if(GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(i)){

				offset = resolveGetElementPtr(GI,D,Context);
				
				//the below instruction should not be a binary add instruction .
				//it should be a getelementptr instruction followed by the offset
				//first zero extend the value and then add
				ZExtInst *zext_binop = new ZExtInst(offset, Type::getInt128Ty(Context), "zextarrayidx", GI);
				BinaryOperator *binop =  BinaryOperator::Create(Instruction::Add, constant, zext_binop , "arrayidx", GI);
				
				errs() << "offset: " << *offset << "\n" ;
				errs() << "binary operation: " << *binop << "\n" ;
				errs() << "zero extended operation: " << *zext_binop << "\n" ;

				//now check where all GI is used


				//the below two instructions are requied for loading the previous value and storing it accordingly
				//may not be required later
				//AllocaInst *pa = new AllocaInst(llvm::Type::getInt32Ty(Context),0, "temp", GI);
				//StoreInst* newstore = new StoreInst(binop, pa, GI);
				

				for (auto &U : GI->uses()) {
					User *user = U.getUser();
					errs() <<"\n****************\nuser:\n"<< *user << "\n****************\n" ; 
					user->setOperand(U.getOperandNo(), binop); //binop - cannot use binop dirctly as it is a i32 value and not a pointer
				 	//break; // break statement should not be here. it should update every uses . but its running to infinite loop so I have added a break.
			    }

			    //now drop references of the Getelementptr instruction and delete the instruction.
			    //cnat delete an instruction from standing at that instruction . so move up.
			    /*--i;
				GI->dropAllReferences();
				GI->removeFromParent();*/

				errs() << "----------------------------------------------------------------------------\n";
			}
		}
	}
}

Value* GetElementPtr::resolveGetElementPtr(GetElementPtrInst *GI,DataLayout *D,LLVMContext &Context){

	int offset = 0;
	Value *Offset;
	int c = 0;

	errs() << *GI << "\n" ;

	if(ConstantInt *CI = dyn_cast<ConstantInt>(GI->getOperand(GI->getNumOperands()-1))){ // get the last operand of the getelementptr
		c = CI->getZExtValue () ;
	}else{
		//suppose the last index was not a constant then set 'c' to some special value and get the last operand.
		c = -999;
		 Instruction *I = dyn_cast<Instruction>(GI->getOperand(GI->getNumOperands()-1));
		 Offset = I->getOperand(0);
	}

	if(c==-999){
		return Offset;
	}
	
	Type *type = GI->getSourceElementType(); //get the type of getelementptr
	if(StructType *t = dyn_cast<StructType>(type) ){ //check for struct type
			
		const StructLayout *SL = D->getStructLayout(t);
		offset+= SL->getElementOffset(c);

	}else if(ArrayType *t = dyn_cast<ArrayType>(type) ) {

		offset+=c*D->getTypeAllocSize(t)/t->getArrayNumElements();
				
	}
	/*else if(UnionType *t = dyn_cast<ArrayType>(type)){
		errs() << "inside union \n" ;
	}
*/

	else{//basic pointer increment or decrements

		offset+=c*D->getTypeAllocSize(type);
	}

	Offset = llvm::ConstantInt::get(Type::getInt32Ty(Context),offset);
	return Offset;
}

char GetElementPtr::ID = 0;
static RegisterPass<GetElementPtr> X("gptr", "Sample program for checking GetElementPtr instructions ");
