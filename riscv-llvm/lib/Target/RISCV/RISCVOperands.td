//===-- RISCVOperands.td - RISCV instruction operands --------*- tblgen-*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

def ptr_rc_c : PointerLikeRegClass<1>;

class ImmAsmOperand<string prefix, int width, string suffix> : AsmOperandClass {
  let Name = prefix # "Imm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

class SImmAsmOperand<int width, string suffix = "">
  : ImmAsmOperand<"S", width, suffix> {
}

class UImmAsmOperand<int width, string suffix = "">
  : ImmAsmOperand<"U", width, suffix> {
}

def uimm4 : Operand<i32> {
  let ParserMatchClass = UImmAsmOperand<4>;
  let DecoderMethod = "decodeUImmOperand<4>";
}

def uimm5 : Operand<i32>, ImmLeaf<i32, [{return isUInt<5>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<5>;
  let DecoderMethod = "decodeUImmOperand<5>";
}

def uimm6 : Operand<i32>, ImmLeaf<i32, [{return isUInt<6>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<6>;
  let DecoderMethod = "decodeUImmOperand<6>";
}

def uimm12 : Operand<i32> {
  let ParserMatchClass = UImmAsmOperand<12>;
  let DecoderMethod = "decodeUImmOperand<12>";
}

def uimm20 : Operand<i32> {
  let ParserMatchClass = UImmAsmOperand<20>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<20>";
}

def simm6 : Operand<i32>, ImmLeaf<i32, [{return isInt<6>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<6>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<6>";
}

def simm12 : Operand<i32>, ImmLeaf<i32, [{return isInt<12>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<12>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<12>";
}

def uimm10_2lsb0 : Operand<i32>,
                   ImmLeaf<i32, [{return isUInt<10>(Imm) &&
                                         ((Imm % 4) == 0);}]> {
  let ParserMatchClass = UImmAsmOperand<10, "_2Lsb0">;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<10>";
}

def simm10_4lsb0 : Operand<i32>,
                   ImmLeaf<i32, [{return isInt<10>(Imm) &&
                                         ((Imm % 16) == 0);}]> {
  let ParserMatchClass = SImmAsmOperand<10, "_4Lsb0">;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<10>";
}

// A 9-bit signed immediate where the least significant bit is zero.
def simm9_lsb0 : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<9, "Lsb0">;
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<9>";
}

def simm12_lsb0 : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<12, "Lsb0">;
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<12>";
}

// A 13-bit signed immediate where the least significant bit is zero.
def simm13_lsb0 : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<13, "Lsb0">;
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<13>";
}

// A 21-bit signed immediate where the least significant bit is zero.
def simm21_lsb0 : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<21, "Lsb0">;
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<21>";
}

// shamt field must fit in 5 bits.
def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;

// shamt field must fit in 5 bits.
def immZExt6 : ImmLeaf<i32, [{return Imm == (Imm & 0x3f);}]>;

// Extract least significant 12 bits from an immediate value and sign extend
// them.
def LO12Sext : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<12>(N->getZExtValue()),
                                   SDLoc(N), MVT::i32);
}]>;

// Extract the most significant 20 bits from an immediate value. Add 1 if bit
// 11 is 1, to compensate for the low 12 bits in the matching immediate addi
// or ld/st being negative.
def HI20 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(((N->getZExtValue()+0x800) >> 12) & 0xfffff,
                                   SDLoc(N), MVT::i32);
}]>;

//===----------------------------------------------------------------------===//
// Immediate asm operands.
//===----------------------------------------------------------------------===//

class ImmediateAsmOperand<string name>
  : AsmOperandClass {
  let Name = name;
  //let RenderMethod = "addImmOperands";
}

def S12Imm : ImmediateAsmOperand<"S12Imm">;

//===----------------------------------------------------------------------===//
// 64-bit immediates
//===----------------------------------------------------------------------===//

// Extract least significant 12 bits from an immediate value and sign extend
// them.
def LO12SextI64 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<12>(N->getZExtValue()),
                                   SDLoc(N), MVT::i64);
}]>;

// Extract the most significant 20 bits from an immediate value. Add 1 if bit
// 11 is 1, to compensate for the low 12 bits in the matching immediate addi
// or ld/st being negative.
def HI20I64 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(((N->getZExtValue()+0x800) >> 12) & 0xfffff,
                                   SDLoc(N), MVT::i64);
}]>;

// Constructs both a DAG pattern and instruction operand for an immediate
// of type VT.  PRED returns true if a node is acceptable and XFORM returns
// the operand value associated with the node.  ASMOP is the name of the
// associated asm operand, and also forms the basis of the asm print method.
class Immediate<ValueType vt, code pred, SDNodeXForm xform, string asmop>
  : PatLeaf<(vt imm), pred, xform>, Operand<vt> {
  let PrintMethod = "print"##asmop##"Operand";
  //let ParserMatchClass = !cast<AsmOperandClass>(asmop);
}

//sign-extended 6 bit immediate
def imm64sx6 : Immediate<i64, [{
  return isInt<6>(N->getSExtValue());
}], NOOP_SDNodeXForm, "S6Imm"> {
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<6>";
}

//sign-extended 12 bit immediate
def imm64sx12 : Immediate<i64, [{
  return isInt<12>(N->getSExtValue());
}], NOOP_SDNodeXForm, "S12Imm"> {
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<12>";
}

def imm64sxu6 : Immediate<i64, [{
  return isUInt<6>(N->getSExtValue());
}], NOOP_SDNodeXForm, "U6Imm"> {
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<6>";
}

def imm64sxu20 : Immediate<i64, [{
  return isUInt<20>(N->getSExtValue());
}], NOOP_SDNodeXForm, "U20Imm"> {
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<20>";
}

//sign-extended 32bit immediate for LUI/ADDIW
def imm64sxu32 : Immediate<i64, [{
  return isUInt<32>(N->getSExtValue());
}], NOOP_SDNodeXForm, "U32Imm">;

//sign-extended 32bit immediate for LUI/ADDI
def imm64sx32 : Immediate<i64, [{
  return isInt<32>(N->getSExtValue()) && (N->getSExtValue() < 0);
}], NOOP_SDNodeXForm, "U32Imm">;

//===----------------------------------------------------------------------===//
// Load/Store Operands
//

class MemOperand : Operand<i32> { let OperandType = "OPERAND_MEMORY"; }

def memreg : Operand<i32> {
  let MIOperandInfo = (ops GPR);
  //let EncoderMethod = "getMemRegEncoding";
  let OperandType = "OPERAND_MEMORY";
  let PrintMethod = "printMemRegOperand";
}

def memreg64 : Operand<i64> {
  let MIOperandInfo = (ops GPR64);
  //let EncoderMethod = "getMemRegEncoding";
  let OperandType = "OPERAND_MEMORY";
  let PrintMethod = "printMemRegOperand";
}

def regaddr : ComplexPattern<iPTR, 1, "selectRegAddr">;

// addr_reg_imm12s := [reg + imm12s]
def AddrRegImm12sAsmOperand : AsmOperandClass { let Name = "AddrRegImm12s"; }
def addr_reg_imm12s : MemOperand,
  ComplexPattern<iPTR, 2, "SelectAddrRegImm12s", []> {
  // 12-bit signed immediate operand.
  // Immediate access range would be +/- (imm12s << 1).

  let MIOperandInfo = (ops ptr_rc, iPTR:$offsimm);
  let PrintMethod = "printAddrRegImmOperand";
  let ParserMatchClass = AddrRegImm12sAsmOperand;
  let EncoderMethod = "getAddrRegImmEncoding";
  let DecoderMethod = "decodeAddrRegImm";
}

// addr_sp_imm6u_word := [sp + imm6u << 2]
def AddrSpImm6uWordAsmOperand : AsmOperandClass { let Name = "AddrSpImm6uWord"; }
def addr_sp_imm6u_word : MemOperand {
  // 6-bit unsigned immediate operand.
  // Immediate access range would be (imm6u << 2).

  let MIOperandInfo = (ops ptr_rc, iPTR:$offsimm);
  let PrintMethod = "printAddrRegImmOperand";
  let ParserMatchClass = AddrSpImm6uWordAsmOperand;
  let EncoderMethod = "getAddrSpImm6uWordEncoding";
  let DecoderMethod = "decodeAddrSpImm6uWord";
}

// addr_sp_imm6u_double := [sp + imm6u << 3]
def AddrSpImm6uDoubleAsmOperand : AsmOperandClass { let Name = "AddrSpImm6uDouble"; }
def addr_sp_imm6u_double : MemOperand {
  // 6-bit unsigned immediate operand.
  // Immediate access range would be (imm6u << 3).

  let MIOperandInfo = (ops ptr_rc, iPTR:$offsimm);
  let PrintMethod = "printAddrRegImmOperand";
  let ParserMatchClass = AddrSpImm6uDoubleAsmOperand;
  let EncoderMethod = "getAddrSpImm6uDoubleEncoding";
  let DecoderMethod = "decodeAddrSpImm6uDouble";
}

// addr_reg_imm5u_word := [reg + imm5u << 2]
def AddrRegImm5uWordAsmOperand : AsmOperandClass { let Name = "AddrRegImm5uWord"; }
def addr_reg_imm5u_word : MemOperand,
    ComplexPattern<iPTR, 2, "SelectAddrRegImm5uWord", []> {
  // 5-bit unsigned immediate operand.
  // Immediate access range would be (imm5u << 2).

  let MIOperandInfo = (ops ptr_rc_c:$base, iPTR:$offsimm);
  let PrintMethod = "printAddrRegImmOperand";
  let ParserMatchClass = AddrRegImm5uWordAsmOperand;
  let EncoderMethod = "getAddrRegImm5uWordEncoding";
  let DecoderMethod = "decodeAddrRegImm5uWord";
}

// addr_reg_imm5u_double := [reg + imm5u << 3]
def AddrRegImm5uDoubleAsmOperand : AsmOperandClass { let Name = "AddrRegImm5uDouble"; }
def addr_reg_imm5u_double : MemOperand,
    ComplexPattern<iPTR, 2, "SelectAddrRegImm5uDouble", []> {
  // 5-bit unsigned immediate operand.
  // Immediate access range would be (imm5u << 3).

  let MIOperandInfo = (ops GPR64C:$base, iPTR:$offsimm);
  let PrintMethod = "printAddrRegImmOperand";
  let ParserMatchClass = AddrRegImm5uDoubleAsmOperand;
  let EncoderMethod = "getAddrRegImm5uDoubleEncoding";
  let DecoderMethod = "decodeAddrRegImm5uDouble";
}
