//===-- RISCVInstrInfo.td - Target Description for RISCV ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the RISC-V instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "RISCVOperands.td"
include "RISCVInstrFormats.td"

//===----------------------------------------------------------------------===//
// Predicates for Subtargets
//===----------------------------------------------------------------------===//

 def IsRV32 :    Predicate<"Subtarget.isRV32()">,
                 AssemblerPredicate<"FeatureRV32">;
 def IsRV64 :    Predicate<"Subtarget.isRV64()">,
                 AssemblerPredicate<"FeatureRV64">;
 def HasM   :    Predicate<"Subtarget.hasM()">,
                 AssemblerPredicate<"FeatureM">;
 def HasF   :    Predicate<"Subtarget.hasF()">,
                 AssemblerPredicate<"FeatureF">;
 def HasD   :    Predicate<"Subtarget.hasD()">,
                 AssemblerPredicate<"FeatureD">;
 def HasA   :    Predicate<"Subtarget.hasA()">,
                 AssemblerPredicate<"FeatureA">;
 def HasE   :    Predicate<"Subtarget.hasE()">,
                 AssemblerPredicate<"FeatureE">;
 def HasC   :    Predicate<"Subtarget.hasC()">,
                 AssemblerPredicate<"FeatureC">;

// RV32Pat - Same as Pat<>, but requires has RISCV32 ISA support.
class RV32Pat<dag pattern, dag result> : Pat<pattern, result> {
    list<Predicate> Predicates = [IsRV32];
}
// RV64Pat - Same as Pat<>, but requires has RISCV64 ISA support.
class RV64Pat<dag pattern, dag result> : Pat<pattern, result> {
    list<Predicate> Predicates = [IsRV64];
}
// RVCPat - Same as Pat<>, but requires has RISCV C ISA support.
class RVCPat<dag pattern, dag result> : Pat<pattern, result> {
    list<Predicate> Predicates = [HasC];
}

//===----------------------------------------------------------------------===//
// Type Profiles.
//===----------------------------------------------------------------------===//

def SDT_RISCVCallSeqStart : SDCallSeqStart<[SDTCisVT<0, iPTR>,
                                            SDTCisVT<1, iPTR>]>;
def SDT_RISCVCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, iPTR>,
                                          SDTCisVT<1, iPTR>]>;
def SDT_RISCVCall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_RISCVSelectCC     : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>,
                                                 SDTCisSameAs<0, 4>,
                                                 SDTCisSameAs<4, 5>]>;

def SDT_RISCVStructByVal : SDTypeProfile<0, 4,
                                         [SDTCisVT<0, i32>, SDTCisVT<1, i32>,
                                          SDTCisVT<2, i32>, SDTCisVT<3, i32>]>;

//===----------------------------------------------------------------------===//
// RISCV Specific Node Definitions.
//===----------------------------------------------------------------------===//

def Call             : SDNode<"RISCVISD::CALL", SDT_RISCVCall,
                              [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                               SDNPVariadic]>;
def RetFlag          : SDNode<"RISCVISD::RET_FLAG", SDTNone,
                              [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def CallSeqStart     : SDNode<"ISD::CALLSEQ_START", SDT_RISCVCallSeqStart,
                              [SDNPHasChain, SDNPSideEffect, SDNPOutGlue]>;
def CallSeqEnd       : SDNode<"ISD::CALLSEQ_END", SDT_RISCVCallSeqEnd,
                              [SDNPHasChain, SDNPSideEffect, SDNPOptInGlue,
                               SDNPOutGlue]>;
def SelectCC         : SDNode<"RISCVISD::SELECT_CC", SDT_RISCVSelectCC,
                              [SDNPInGlue]>;

//===----------------------------------------------------------------------===//
// C Subtarget feature
//===----------------------------------------------------------------------===//

include "RISCVInstrInfoC.td"
include "RISCVInstrInfoRV64C.td"

//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// RISCV32 I Instructions
//===----------------------------------------------------------------------===//

def LUI : FU<0b0110111, (outs GPR:$rd), (ins uimm20:$imm20),
             "lui\t$rd, $imm20", []>;

let isReMaterializable = 1, isMoveImm = 1 in
def MOVi32imm : Pseudo<(outs GPR:$dst), (ins i32imm:$src),
                       [(set GPR:$dst, (i32 imm:$src))]>;

def AUIPC : FU<0b0010111, (outs GPR:$rd), (ins uimm20:$imm20),
               "auipc\t$rd, $imm20", []>;

let isCall=1 in {
def JAL : FUJ<0b1101111, (outs GPR:$rd), (ins simm21_lsb0:$imm20),
              "jal\t$rd, $imm20", []>;
}

let isBranch = 1, isTerminator=1, isBarrier=1 in {
def PseudoBR : Pseudo<(outs), (ins simm21_lsb0:$imm20), [(br bb:$imm20)]>,
               PseudoInstExpansion<(JAL X0_32, simm21_lsb0:$imm20)>,
               Requires<[IsRV32]>;
}

let isCall=1 in {
def JALR : FI<0b000, 0b1100111, (outs GPR:$rd),
              (ins GPR:$rs1, simm12:$imm12),
              "jalr\t$rd, $rs1, $imm12", []>;
}

let isBranch = 1, isBarrier = 1, isTerminator = 1, isIndirectBranch = 1 in {
  def PseudoBRIND : Pseudo<(outs), (ins GPR:$rs1), [(brind GPR:$rs1)]>,
                    PseudoInstExpansion<(JALR X0_32, GPR:$rs1, 0)>,
                    Requires<[IsRV32]>;
}

let isCall=1, Defs=[X1_32] in {
  def PseudoCALL : Pseudo<(outs), (ins GPR:$rs1), [(Call GPR:$rs1)]>,
                   PseudoInstExpansion<(JALR X1_32, GPR:$rs1, 0)>,
                   Requires<[IsRV32]>;
}

let isReturn=1, isTerminator=1, isBarrier=1 in {
  def PseudoRET : Pseudo<(outs), (ins), [(RetFlag)]>,
                  PseudoInstExpansion<(JALR X0_32, X1_32, 0)>,
                  Requires<[IsRV32]>;
}

// Pessimstically assume the stack pointer will be clobbered
let Defs = [X2_32], Uses = [X2_32], hasSideEffects = 1 in {
  def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                 [(CallSeqStart timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                [(CallSeqEnd timm:$amt1, timm:$amt2)]>;
}

class Bcc<bits<3> funct3, string OpcodeStr, PatFrag CondOp> :
      FSB<funct3, 0b1100011, (outs),
          (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12),
          OpcodeStr#"\t$rs1, $rs2, $imm12",
          [(brcond (i32 (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm12)]> {
  let isBranch = 1;
  let isTerminator = 1;
}

def BEQ   : Bcc<0b000, "beq",  seteq>;
def BNE   : Bcc<0b001, "bne",  setne>;
def BLT   : Bcc<0b100, "blt",  setlt>;
def BGE   : Bcc<0b101, "bge",  setge>;
def BLTU  : Bcc<0b110, "bltu", setult>;
def BGEU  : Bcc<0b111, "bgeu", setuge>;

def : Pat<(brcond (i32 (setne GPR:$lhs, 0)), bb:$dst),
          (BNE GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (seteq GPR:$lhs, 0)), bb:$dst),
          (BEQ GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (setlt GPR:$lhs, 0)), bb:$dst),
          (BLT GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (setge GPR:$lhs, 0)), bb:$dst),
          (BGE GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (setult GPR:$lhs, 0)), bb:$dst),
          (BLTU GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (setuge GPR:$lhs, 0)), bb:$dst),
          (BGEU GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond GPR:$cond, bb:$dst),
          (BNE GPR:$cond, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (setgt GPR:$lhs, -1)), bb:$dst),
          (BGE GPR:$lhs, X0_32, bb:$dst)>;
def : Pat<(brcond (i32 (setle GPR:$lhs, -1)), bb:$dst),
          (BLT GPR:$lhs, X0_32, bb:$dst)>;

class Bcc_SwapPat<PatFrag CondOp, RISCVInst InstBcc> : Pat<
  (brcond (i32 (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm12),
  (InstBcc GPR:$rs2, GPR:$rs1, bb:$imm12)>;

// Condition codes that don't have matching RISC-V branch instructions, but
// are trivially supported by swapping the two input operands
def : Bcc_SwapPat<setgt,  BLT>;
def : Bcc_SwapPat<setle,  BGE>;
def : Bcc_SwapPat<setugt, BLTU>;
def : Bcc_SwapPat<setule, BGEU>;

def : Pat<(brcond GPR:$cond, bb:$imm12),
          (BNE GPR:$cond, X0_32, bb:$imm12)>;

let usesCustomInserter = 1 in {
  def Select : Pseudo<(outs GPR:$dst),
                      (ins GPR:$lhs, GPR:$rhs, i32imm:$imm, GPR:$src, GPR:$src2),
                      [(set i32:$dst,
                       (SelectCC GPR:$lhs, GPR:$rhs, (i32 imm:$imm), GPR:$src,
                                 GPR:$src2))]>;
}

def LB   : LoadRI<0b000, "lb",  sextloadi8,  GPR>;
def LH   : LoadRI<0b001, "lh",  sextloadi16, GPR>;
def LW   : LoadRI<0b010, "lw",  load,        GPR>;
def LBU  : LoadRI<0b100, "lbu", zextloadi8,  GPR>;
def LHU  : LoadRI<0b101, "lhu", zextloadi16, GPR>;

// load patterns
def : Pat<(i32 (extloadi16 addr_reg_imm12s:$src)), (LHU addr_reg_imm12s:$src)>;
def : Pat<(i32 (extloadi8  addr_reg_imm12s:$src)), (LBU addr_reg_imm12s:$src)>;
def : Pat<(i32 (extloadi1  addr_reg_imm12s:$src)), (LBU addr_reg_imm12s:$src)>;

// Operands for stores are in the order srcreg, base, offset rather than
// reflecting the order these fields are specified in the instruction
// encoding.

def SB  : StoreRI<0b000, "sb", truncstorei8,  GPR>;
def SH  : StoreRI<0b001, "sh", truncstorei16, GPR>;
def SW  : StoreRI<0b010, "sw", store,         GPR>;

def : Pat<(store (i32 0), addr_reg_imm12s:$addr),
          (SW X0_32, addr_reg_imm12s:$addr)>;

class ALU_ri<bits<3> funct3, string OpcodeStr, SDPatternOperator OpNode> :
      FI<funct3, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
         OpcodeStr#"\t$rd, $rs1, $imm12",
         [(set GPR:$rd, (OpNode GPR:$rs1, simm12:$imm12))]>;

def ADDI  : ALU_ri<0b000, "addi",  add>, Requires<[IsRV32]>;
def SLTI  : ALU_ri<0b010, "slti",  setlt>;
def SLTIU : ALU_ri<0b011, "sltiu", setult>;
def XORI  : ALU_ri<0b100, "xori",  xor>;
def ORI   : ALU_ri<0b110, "ori",   or>;
def ANDI  : ALU_ri<0b111, "andi",  and>;

class SHIFT32_ri<bit arithshift, bits<3> funct3, string OpcodeStr,
                 SDPatternOperator OpNode> :
      ShiftRI<arithshift, funct3, 0b0010011, (outs GPR:$rd),
              (ins GPR:$rs1, uimm5:$shamt),
              OpcodeStr#"\t$rd, $rs1, $shamt",
              [(set GPR:$rd, (OpNode GPR:$rs1, uimm5:$shamt))]>;

def SLLI : SHIFT32_ri<0, 0b001, "slli", shl>, Requires<[IsRV32]>;
def SRLI : SHIFT32_ri<0, 0b101, "srli", srl>, Requires<[IsRV32]>;
def SRAI : SHIFT32_ri<1, 0b101, "srai", sra>, Requires<[IsRV32]>;

class ALU_rr<bits<7> funct7, bits<3> funct3, string OpcodeStr,
             SDPatternOperator OpNode> :
      FR<funct7, funct3, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
         OpcodeStr#"\t$rd, $rs1, $rs2",
         [(set GPR:$rd, (OpNode GPR:$rs1, GPR:$rs2))]>;

def ADD  : ALU_rr<0b0000000, 0b000, "add",  add>, Requires<[IsRV32]>;
def SUB  : ALU_rr<0b0100000, 0b000, "sub",  sub>, Requires<[IsRV32]>;
def SLL  : ALU_rr<0b0000000, 0b001, "sll",  shl>, Requires<[IsRV32]>;
def SRL  : ALU_rr<0b0000000, 0b101, "srl",  srl>, Requires<[IsRV32]>;
def SRA  : ALU_rr<0b0100000, 0b101, "sra",  sra>, Requires<[IsRV32]>;
def SLT  : ALU_rr<0b0000000, 0b010, "slt",  setlt>;
def SLTU : ALU_rr<0b0000000, 0b011, "sltu", setult>;
def XOR  : ALU_rr<0b0000000, 0b100, "xor",  xor>;
def OR   : ALU_rr<0b0000000, 0b110, "or",   or>;
def AND  : ALU_rr<0b0000000, 0b111, "and",  and>;

// Define pattern expansions for setcc operations that aren't directly
// handled by a RISC-V instruction

def : Pat<(seteq  GPR:$rs1, GPR:$rs2), (SLTIU (XOR GPR:$rs1, GPR:$rs2), 1)>;
def : Pat<(setne  GPR:$rs1, GPR:$rs2), (SLTU X0_32, (XOR GPR:$rs1, GPR:$rs2))>;
def : Pat<(setugt GPR:$rs1, GPR:$rs2), (SLTU GPR:$rs2, GPR:$rs1)>;
def : Pat<(setuge GPR:$rs1, GPR:$rs2), (XORI (SLTU GPR:$rs1, GPR:$rs2), 1)>;
def : Pat<(setule GPR:$rs1, GPR:$rs2), (XORI (SLTU GPR:$rs2, GPR:$rs1), 1)>;
def : Pat<(setgt  GPR:$rs1, GPR:$rs2), (SLT GPR:$rs2, GPR:$rs1)>;
def : Pat<(setge  GPR:$rs1, GPR:$rs2), (XORI (SLT GPR:$rs1, GPR:$rs2), 1)>;
def : Pat<(setle  GPR:$rs1, GPR:$rs2), (XORI (SLT GPR:$rs2, GPR:$rs1), 1)>;

def FENCE : FI<0b000, 0b0001111, (outs), (ins uimm4:$pred, uimm4:$succ),
               "fence\t$pred, $succ", []> {
  bits<4> pred;
  bits<4> succ;

  let rs1 = 0;
  let rd = 0;
  let imm12 = {0b0000,pred,succ};
}

def FENCEI : FI<0b001, 0b0001111, (outs), (ins), "fence.i", []> {
  let rs1 = 0;
  let rd = 0;
  let imm12 = 0;
}

let rs1=0, rd=0 in {
  def ECALL  : FI<0b000, 0b1110011, (outs), (ins), "ecall", []> {
    let imm12=0;
  }
  def EBREAK : FI<0b000, 0b1110011, (outs), (ins), "ebreak", []> {
    let imm12=1;
  }
}

class CSR_rr<bits<3> funct3, string OpcodeStr> :
      FI<funct3, 0b1110011, (outs GPR:$rd), (ins uimm12:$imm12, GPR:$rs1),
         OpcodeStr#"\t$rd, $imm12, $rs1", []>;

def CSRRW : CSR_rr<0b001, "csrrw">;
def CSRRS : CSR_rr<0b010, "csrrs">;
def CSRRC : CSR_rr<0b011, "csrrc">;

class CSR_ri<bits<3> funct3, string OpcodeStr> :
      FI<funct3, 0b1110011, (outs GPR:$rd), (ins uimm12:$imm12, uimm5:$rs1),
         OpcodeStr#"\t$rd, $imm12, $rs1", []>;

def CSRRWI : CSR_ri<0b101, "csrrwi">;
def CSRRSI : CSR_ri<0b110, "csrrsi">;
def CSRRCI : CSR_ri<0b111, "csrrci">;

// signed 12-bit immediate
def : Pat<(simm12:$imm), (ADDI X0_32, simm12:$imm)>;

def : InstAlias<"nop", (SLLI X0_32, X0_32, 0)>, Requires<[IsRV32]>;

//===----------------------------------------------------------------------===//
// Subtarget features
//===----------------------------------------------------------------------===//

include "RISCVInstrInfoRV64.td"
include "RISCVInstrInfoM.td"
include "RISCVInstrInfoA.td"
