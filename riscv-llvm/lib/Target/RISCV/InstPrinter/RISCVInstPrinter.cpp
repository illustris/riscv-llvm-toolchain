//===-- RISCVInstPrinter.cpp - Convert RISCV MCInst to asm syntax ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an RISCV MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "RISCVInstPrinter.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#include "RISCVGenAsmWriter.inc"

void RISCVInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                 StringRef Annot, const MCSubtargetInfo &STI) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void RISCVInstPrinter::printRegName(raw_ostream &O, unsigned RegNo) const {
  O << markup("<reg:") << getRegisterName(RegNo) << markup(">");
}

void RISCVInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O, const char *Modifier) {
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void RISCVInstPrinter::printMemRegOperand(const MCInst *MI, int opNum,
                                          raw_ostream &O) {
  O << "0"; //No offset for this ever
  O << "(";
  O << getRegisterName(MI->getOperand(opNum).getReg());
  O << ")";
}

// Print Imm(Reg) addressing mode
void RISCVInstPrinter::printAddrRegImmOperand(const MCInst *MI, unsigned OpNum,
                                              raw_ostream &O) {
  printOperand(MI, OpNum+1, O);
  O << "(";
  printOperand(MI, OpNum, O);
  O << ")";
}

void RISCVInstPrinter::printS6ImmOperand(const MCInst *MI, int OpNum,
                                           raw_ostream &O) {
  if(MI->getOperand(OpNum).isImm()){
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(isInt<6>(Value) && "Invalid s6imm argument");
    O << Value;
  }else
    printOperand(MI, OpNum, O);
}

void RISCVInstPrinter::printS12ImmOperand(const MCInst *MI, int OpNum,
                                           raw_ostream &O) {
  if(MI->getOperand(OpNum).isImm()){
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(isInt<12>(Value) && "Invalid s12imm argument");
    O << Value;
  }else
    printOperand(MI, OpNum, O);
}

void RISCVInstPrinter::printU6ImmOperand(const MCInst *MI, int OpNum,
                                           raw_ostream &O) {
  if(MI->getOperand(OpNum).isImm()){
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(isUInt<6>(Value) && "Invalid u6imm argument");
    O << Value;
  }else
    printOperand(MI, OpNum, O);
}

void RISCVInstPrinter::printU20ImmOperand(const MCInst *MI, int OpNum,
                                           raw_ostream &O) {
  if(MI->getOperand(OpNum).isImm()){
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(isUInt<20>(Value) && "Invalid u20imm argument");
    O << Value;
  }else
    printOperand(MI, OpNum, O);
}
