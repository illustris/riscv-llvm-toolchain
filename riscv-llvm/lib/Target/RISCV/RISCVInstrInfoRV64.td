//===- RISCVInstrInfoRV64.td - RISCV RV64I instructions -------*- tblgen-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class ALUW_rr<bits<7> funct7, bits<3> funct3, string OpcodeStr,
              SDPatternOperator OpNode> :
      FR<funct7, funct3, 0b0111011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
         OpcodeStr#"\t$rd, $rs1, $rs2",
         [(set GPR:$rd, (OpNode GPR:$rs1, GPR:$rs2))]>;

class ALUW_ri<bits<3> funct3, string OpcodeStr, SDPatternOperator OpNode> :
      FI<funct3, 0b0011011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
         OpcodeStr#"\t$rd, $rs1, $imm12",
         [(set GPR:$rd, (OpNode GPR:$rs1, simm12:$imm12))]>;

class ALU64_rr<bits<7> funct7, bits<3> funct3, string OpcodeStr,
               SDPatternOperator OpNode> :
      FR<funct7, funct3, 0b0110011, (outs GPR64:$rd),
         (ins GPR64:$rs1, GPR64:$rs2),
         OpcodeStr#"\t$rd, $rs1, $rs2",
         [(set GPR64:$rd, (OpNode GPR64:$rs1, GPR64:$rs2))]>;

class SHIFT64_rr<bits<7> funct7, bits<3> funct3, string OpcodeStr,
                 SDPatternOperator OpNode> :
      FR<funct7, funct3, 0b0110011, (outs GPR64:$rd),
         (ins GPR64:$rs1, GPR64:$rs2),
         OpcodeStr#"\t$rd, $rs1, $rs2",
         [(set GPR64:$rd, (OpNode GPR64:$rs1, GPR64:$rs2))]>;

class ALU64_ri<bits<3> funct3, string OpcodeStr, SDPatternOperator OpNode> :
      FI<funct3, 0b0010011, (outs GPR64:$rd), 
         (ins GPR64:$rs1, imm64sx12:$imm12),
         OpcodeStr#"\t$rd, $rs1, $imm12",
         [(set GPR64:$rd, (OpNode GPR64:$rs1, imm64sx12:$imm12))]>;

let DecoderNamespace = "RISCV64_" in {
def ADDW  : ALUW_rr<0b0000000, 0b000, "addw", add>, Requires<[IsRV64]>;
def SUBW  : ALUW_rr<0b0100000, 0b000, "subw", sub>, Requires<[IsRV64]>;
def SLLW  : ALUW_rr<0b0000000, 0b001, "sllw", shl>, Requires<[IsRV64]>;
def SRLW  : ALUW_rr<0b0000000, 0b101, "srlw", srl>, Requires<[IsRV64]>;
def SRAW  : ALUW_rr<0b0100000, 0b101, "sraw", sra>, Requires<[IsRV64]>;

def ADDIW  : ALUW_ri<0b000, "addiw", add>, Requires<[IsRV64]>;

//Standard instructions operating on 64bit values
//Integer arithmetic register-register
def ADD64  : ALU64_rr<0b0000000, 0b000, "add",  add>, Requires<[IsRV64]>;
def SUB64  : ALU64_rr<0b0100000, 0b000, "sub",  sub>, Requires<[IsRV64]>;
def XOR64  : ALU64_rr<0b0000000, 0b100, "xor",  xor>, Requires<[IsRV64]>;
def OR64   : ALU64_rr<0b0000000, 0b110, "or" ,  or>,  Requires<[IsRV64]>;
def AND64  : ALU64_rr<0b0000000, 0b111, "and",  and>, Requires<[IsRV64]>;
def SLT64  : ALU64_rr<0b0000000, 0b010, "slt",  setlt>,  Requires<[IsRV64]>;
def SLTU64 : ALU64_rr<0b0000000, 0b011, "sltu", setult>, Requires<[IsRV64]>;

def SLL64  : SHIFT64_rr<0b0000000, 0b001, "sll", shl>, Requires<[IsRV64]>;
def SRL64  : SHIFT64_rr<0b0000000, 0b101, "srl", srl>, Requires<[IsRV64]>;
def SRA64  : SHIFT64_rr<0b0100000, 0b101, "sra", sra>, Requires<[IsRV64]>;

//Integer arithmetic register-immediate
def ADDI64: ALU64_ri<0b000, "addi", add>, Requires<[IsRV64]>;
def XORI64: ALU64_ri<0b100, "xori", xor>, Requires<[IsRV64]>;
def ORI64 : ALU64_ri<0b110,  "ori",  or>, Requires<[IsRV64]>;
def ANDI64: ALU64_ri<0b111, "andi", and>, Requires<[IsRV64]>;
def SLTI64 : ALU64_ri<0b010, "slti",  setlt>,  Requires<[IsRV64]>;
def SLTIU64: ALU64_ri<0b011, "sltiu", setult>, Requires<[IsRV64]>;

def LUI64 : FU<0b0110111, (outs GPR64:$rd), (ins imm64sxu20:$imm20),
               "lui\t$rd, $imm20", []>;

def AUIPC64 : FU<0b0010111, (outs GPR64:$rd), (ins imm64sxu20:$imm20),
                 "auipc\t$rd, $imm20", []>;
} // End of DecoderNamespace

class SLT_rr<bits<7> funct7, bits<3> funct3, string OpcodeStr,
             SDPatternOperator OpNode> :
      FR<funct7, funct3, 0b0110011, (outs GPR:$rd),
         (ins GPR64:$rs1, GPR64:$rs2),
         OpcodeStr#"\t$rd, $rs1, $rs2",
         [(set GPR:$rd, (OpNode GPR64:$rs1, GPR64:$rs2))]>;

class SLT_ri<bits<3> funct3, string OpcodeStr, SDPatternOperator OpNode> :
      FI<funct3, 0b0010011, (outs GPR:$rd),
         (ins GPR64:$rs1, imm64sx12:$imm12),
         OpcodeStr#"\t$rd, $rs1, $imm12",
         [(set GPR:$rd, (OpNode GPR64:$rs1, imm64sx12:$imm12))]>;

class SHIFT64_32_rr<bits<7> funct7, bits<3> funct3, string OpcodeStr,
                    SDPatternOperator OpNode> :
      FR<funct7, funct3, 0b0110011, (outs GPR64:$rd),
         (ins GPR64:$rs1, GPR:$rs2),
         OpcodeStr#"\t$rd, $rs1, $rs2",
         [(set GPR64:$rd, (OpNode GPR64:$rs1, GPR:$rs2))]>;

let DecoderNamespace = "NO_DECODE_MATCH" in {
def SLL64_32  : SHIFT64_32_rr<0b0000000, 0b001, "sll", shl>, Requires<[IsRV64]>;
def SRL64_32  : SHIFT64_32_rr<0b0000000, 0b101, "srl", srl>, Requires<[IsRV64]>;
def SRA64_32  : SHIFT64_32_rr<0b0100000, 0b101, "sra", sra>, Requires<[IsRV64]>;
def SLT32  : SLT_rr<0b0000000, 0b010, "slt",  setlt>,  Requires<[IsRV64]>;
def SLTU32 : SLT_rr<0b0000000, 0b011, "sltu", setult>, Requires<[IsRV64]>;
def SLTI32 : SLT_ri<0b010, "slti",  setlt>,  Requires<[IsRV64]>;
def SLTIU32: SLT_ri<0b011, "sltiu", setult>, Requires<[IsRV64]>;
} // End of DecoderNamespace


let isReMaterializable = 1, isMoveImm = 1 in
def MOVi64imm : Pseudo<(outs GPR64:$dst), (ins i64imm:$src),
                       [(set GPR64:$dst, (i64 imm:$src))]>;

// signed 12-bit immediate
def : RV64Pat<(imm64sx12:$imm), (ADDI64 X0_64, imm64sx12:$imm)>;

def : RV64Pat<(seteq GPR64:$rs1, GPR64:$rs2),
              (SLTIU64 (XOR64 GPR64:$rs1, GPR64:$rs2), 1)>;
def : RV64Pat<(seteq GPR64:$rs1, GPR64:$rs2),
              (SLTIU32 (XOR64 GPR64:$rs1, GPR64:$rs2), 1)>;
def : RV64Pat<(setne GPR64:$rs1, GPR64:$rs2),
              (SLTU64 X0_64, (XOR64 GPR64:$rs1, GPR64:$rs2))>;
def : RV64Pat<(setne GPR64:$rs1, GPR64:$rs2),
              (SLTU32 X0_64, (XOR64 GPR64:$rs1, GPR64:$rs2))>;
def : RV64Pat<(setugt GPR64:$rs1, GPR64:$rs2),
              (SLTU64 GPR64:$rs2, GPR64:$rs1)>;
def : RV64Pat<(setugt GPR64:$rs1, GPR64:$rs2),
              (SLTU32 GPR64:$rs2, GPR64:$rs1)>;
def : RV64Pat<(setuge GPR64:$rs1, GPR64:$rs2),
              (XORI64 (SLTU64 GPR64:$rs1, GPR64:$rs2), 1)>;
def : RV64Pat<(setuge GPR64:$rs1, GPR64:$rs2),
              (XORI (SLTU32 GPR64:$rs1, GPR64:$rs2), 1)>;
def : RV64Pat<(setule GPR64:$rs1, GPR64:$rs2),
              (XORI64 (SLTU64 GPR64:$rs2, GPR64:$rs1), 1)>;
def : RV64Pat<(setule GPR64:$rs1, GPR64:$rs2),
              (XORI (SLTU32 GPR64:$rs2, GPR64:$rs1), 1)>;
def : RV64Pat<(setgt GPR64:$rs1, GPR64:$rs2),
              (SLT64 GPR64:$rs2, GPR64:$rs1)>;
def : RV64Pat<(setgt GPR64:$rs1, GPR64:$rs2),
              (SLT32 GPR64:$rs2, GPR64:$rs1)>;
def : RV64Pat<(setge GPR64:$rs1, GPR64:$rs2),
              (XORI64 (SLT64 GPR64:$rs1, GPR64:$rs2), 1)>;
def : RV64Pat<(setge GPR64:$rs1, GPR64:$rs2),
              (XORI (SLT32 GPR64:$rs1, GPR64:$rs2), 1)>;
def : RV64Pat<(setle GPR64:$rs1, GPR64:$rs2),
              (XORI64 (SLT64 GPR64:$rs2, GPR64:$rs1), 1)>;
def : RV64Pat<(setle GPR64:$rs1, GPR64:$rs2),
              (XORI (SLT32 GPR64:$rs2, GPR64:$rs1), 1)>;

class SHIFT64_ri<bit arithshift, bits<3> funct3, string OpcodeStr,
                 SDPatternOperator OpNode> :
      ShiftRI<arithshift, funct3, 0b0010011, (outs GPR64:$rd),
              (ins GPR64:$rs1, uimm6:$shamt),
              OpcodeStr#"\t$rd, $rs1, $shamt",
              [(set GPR64:$rd, (OpNode GPR64:$rs1, uimm6:$shamt))]> {
  let Inst{25} = shamt{5};
}

class SHIFTW_ri<bit arithshift, bits<3> funct3, string OpcodeStr,
                SDPatternOperator OpNode> :
      ShiftRI<arithshift, funct3, 0b0011011, (outs GPR:$rd),
              (ins GPR:$rs1, uimm5:$shamt),
              OpcodeStr#"\t$rd, $rs1, $shamt",
              [(set GPR:$rd, (OpNode GPR:$rs1, uimm5:$shamt))]>;

let DecoderNamespace = "RISCV64_" in {
def SLLI64: SHIFT64_ri<0, 0b001 , "slli", shl>, Requires<[IsRV64]>;
def SRLI64: SHIFT64_ri<0, 0b101 , "srli", srl>, Requires<[IsRV64]>;
def SRAI64: SHIFT64_ri<1, 0b101 , "srai", sra>, Requires<[IsRV64]>;

def SLLIW: SHIFTW_ri<0, 0b001 , "slliw", shl>, Requires<[IsRV64]>;
def SRLIW: SHIFTW_ri<0, 0b101 , "srliw", srl>, Requires<[IsRV64]>;
def SRAIW: SHIFTW_ri<1, 0b101 , "sraiw", sra>, Requires<[IsRV64]>;
} // End of DecoderNamespace

// variable shift instructions patterns
def : Pat<(shl GPR64:$rt, (i32 (trunc GPR64:$rs))),
          (SLL64_32 GPR64:$rt, (EXTRACT_SUBREG GPR64:$rs, sub_32))>;
def : Pat<(srl GPR64:$rt, (i32 (trunc GPR64:$rs))),
          (SRL64_32 GPR64:$rt, (EXTRACT_SUBREG GPR64:$rs, sub_32))>;
def : Pat<(sra GPR64:$rt, (i32 (trunc GPR64:$rs))),
          (SRA64_32 GPR64:$rt, (EXTRACT_SUBREG GPR64:$rs, sub_32))>;

//===----------------------------------------------------------------------===//
// Load and Store Instructions
//===----------------------------------------------------------------------===//

let DecoderNamespace = "RISCV64_" in {
  def LWU   : LoadRI<0b110, "lwu", zextloadi32, GPR64>, Requires<[IsRV64]>;
  def LD    : LoadRI<0b011, "ld", load, GPR64>, Requires<[IsRV64]>;
  def LW64  : LoadRI<0b010, "lw", sextloadi32, GPR64>, Requires<[IsRV64]>;
  def LH64  : LoadRI<0b001, "lh", sextloadi16, GPR64>, Requires<[IsRV64]>;
  def LHU64 : LoadRI<0b101, "lhu", zextloadi16, GPR64>, Requires<[IsRV64]>;
  def LB64  : LoadRI<0b000, "lb", sextloadi8, GPR64>, Requires<[IsRV64]>;
  def LBU64 : LoadRI<0b100, "lbu", zextloadi8, GPR64>, Requires<[IsRV64]>;
}

//extended loads
def : Pat<(i64 (extloadi1  addr_reg_imm12s:$addr)), (LBU64 addr_reg_imm12s:$addr)>;
def : Pat<(extloadi1  addr_reg_imm12s:$addr), (LBU64 addr_reg_imm12s:$addr)>;
def : Pat<(i64 (extloadi8  addr_reg_imm12s:$addr)), (LBU64 addr_reg_imm12s:$addr)>;
def : Pat<(extloadi8  addr_reg_imm12s:$addr), (LBU64 addr_reg_imm12s:$addr)>;
def : Pat<(i64 (extloadi16 addr_reg_imm12s:$addr)), (LHU64 addr_reg_imm12s:$addr)>;
def : Pat<(extloadi16 addr_reg_imm12s:$addr), (LHU64 addr_reg_imm12s:$addr)>;
def : Pat<(i64 (extloadi32 addr_reg_imm12s:$addr)), (LW64 addr_reg_imm12s:$addr)>;
def : Pat<(extloadi32 addr_reg_imm12s:$addr), (LW64   addr_reg_imm12s:$addr)>;

let DecoderNamespace = "RISCV64_" in {
  def SD   : StoreRI<0b011, "sd", store, GPR64>, Requires<[IsRV64]>;
  def SW64 : StoreRI<0b010, "sw", truncstorei32, GPR64>, Requires<[IsRV64]>;
  def SH64 : StoreRI<0b001, "sh", truncstorei16, GPR64>, Requires<[IsRV64]>;
  def SB64 : StoreRI<0b000, "sb", truncstorei8, GPR64>, Requires<[IsRV64]>;
}

def : Pat<(store (i64 0), addr_reg_imm12s:$addr),
          (SD X0_64, addr_reg_imm12s:$addr)>;

//===----------------------------------------------------------------------===//
// Branch and Call Instructions
//===----------------------------------------------------------------------===//

let isCall=1, DecoderNamespace = "RISCV64_" in {
def JAL64 : FUJ<0b1101111, (outs GPR64:$rd), (ins simm21_lsb0:$imm20),
                "jal\t$rd, $imm20", []>;
}

let isBranch = 1, isTerminator=1, isBarrier=1, DecoderNamespace = "RISCV64_" in {
def PseudoBR64 : Pseudo<(outs), (ins simm21_lsb0:$imm20), [(br bb:$imm20)]>,
                 PseudoInstExpansion<(JAL64 X0_64, simm21_lsb0:$imm20)>,
                 Requires<[IsRV64]>;
}

let isCall=1, DecoderNamespace = "RISCV64_" in {
def JALR64 : FI<0b000, 0b1100111, (outs GPR64:$rd),
                (ins GPR64:$rs1, simm12:$imm12),
                "jalr\t$rd, $rs1, $imm12", []>;
}

let isBranch = 1, isBarrier = 1, isTerminator = 1, isIndirectBranch = 1,
    DecoderNamespace = "RISCV64_" in {
  def PseudoBRIND64 : Pseudo<(outs), (ins GPR64:$rs1), [(brind GPR64:$rs1)]>,
                      PseudoInstExpansion<(JALR64 X0_64, GPR64:$rs1, 0)>,
                      Requires<[IsRV64]>;
}

class Bcc64<bits<3> funct3, string OpcodeStr, PatFrag CondOp> :
      FSB<funct3, 0b1100011, (outs),
          (ins GPR64:$rs1, GPR64:$rs2, simm13_lsb0:$imm12),
          OpcodeStr#"\t$rs1, $rs2, $imm12",
          [(brcond (i32 (CondOp GPR64:$rs1, GPR64:$rs2)), bb:$imm12)]> {
  let isBranch = 1;
  let isTerminator = 1;
}

let DecoderNamespace = "RISCV64_" in {
def BEQ64   : Bcc64<0b000, "beq",  seteq>;
def BNE64   : Bcc64<0b001, "bne",  setne>;
def BLT64   : Bcc64<0b100, "blt",  setlt>;
def BGE64   : Bcc64<0b101, "bge",  setge>;
def BLTU64  : Bcc64<0b110, "bltu", setult>;
def BGEU64  : Bcc64<0b111, "bgeu", setuge>;
} // End of DecoderNamespace

def : Pat<(brcond (i32 (setne GPR64:$lhs, 0)), bb:$dst),
          (BNE64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (seteq GPR64:$lhs, 0)), bb:$dst),
          (BEQ64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (setlt GPR64:$lhs, 0)), bb:$dst),
          (BLT64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (setge GPR64:$lhs, 0)), bb:$dst),
          (BGE64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (setult GPR64:$lhs, 0)), bb:$dst),
          (BLTU64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (setuge GPR64:$lhs, 0)), bb:$dst),
          (BGEU64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond GPR64:$cond, bb:$dst),
          (BNE64 GPR64:$cond, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (setgt GPR64:$lhs, -1)), bb:$dst),
          (BGE64 GPR64:$lhs, X0_64, bb:$dst)>;
def : Pat<(brcond (i32 (setle GPR64:$lhs, -1)), bb:$dst),
          (BLT64 GPR64:$lhs, X0_64, bb:$dst)>;

class Bcc_SwapPat64<PatFrag CondOp, RISCVInst InstBcc> : Pat<
  (brcond (i32 (CondOp GPR64:$rs1, GPR64:$rs2)), bb:$imm12),
  (InstBcc GPR64:$rs2, GPR64:$rs1, bb:$imm12)>;

// Condition codes that don't have matching RISC-V branch instructions, but
// are trivially supported by swapping the two input operands
def : Bcc_SwapPat64<setgt, BLT64>;
def : Bcc_SwapPat64<setle, BGE64>;
def : Bcc_SwapPat64<setugt, BLTU64>;
def : Bcc_SwapPat64<setule, BGEU64>;

def : Pat<(brcond GPR64:$cond, bb:$imm12),
          (BNE64 GPR64:$cond, X0_64, bb:$imm12)>;

let isCall=1, Defs=[X1_64] in {
  def PseudoCALL64 : Pseudo<(outs), (ins GPR64:$rs1), [(Call GPR64:$rs1)]>,
                     PseudoInstExpansion<(JALR64 X1_64, GPR64:$rs1, 0)>,
                     Requires<[IsRV64]>;
}

let isReturn=1, isTerminator=1, isBarrier=1 in {
  def PseudoRET64 : Pseudo<(outs), (ins), [(RetFlag)]>,
                    PseudoInstExpansion<(JALR64 X0_64, X1_64, 0)>,
                    Requires<[IsRV64]>;
}

// Get i32 value from GPR64 register
def :Pat<(i32 (trunc GPR64:$src)),
         (ADDIW (EXTRACT_SUBREG GPR64:$src, sub_32), 0)>;

// Extension patterns
def : Pat<(i64 (zext GPR:$val)),
          (SRLI64 (SLLI64 (SUBREG_TO_REG (i64 0), GPR:$val, sub_32), 32), 32)>;
def : Pat<(i64 (sext GPR:$val)),
          (SUBREG_TO_REG (i64 0), GPR:$val, sub_32)>;
def : Pat<(i64 (anyext GPR:$val)),
          (SUBREG_TO_REG (i64 0), GPR:$val, sub_32)>;

let usesCustomInserter = 1 in {
  def Select32 : Pseudo<(outs GPR64:$dst),
                        (ins GPR:$lhs, GPR:$rhs, i32imm:$imm, GPR64:$src,
                             GPR64:$src2),
                        [(set i64:$dst,
                         (SelectCC GPR:$lhs, GPR:$rhs, (i32 imm:$imm),
                                   GPR64:$src, GPR64:$src2))]>,
                 Requires<[IsRV64]>;

  def Select64 : Pseudo<(outs GPR64:$dst),
                        (ins GPR64:$lhs, GPR64:$rhs, i32imm:$imm, GPR64:$src,
                             GPR64:$src2),
                        [(set i64:$dst,
                         (SelectCC GPR64:$lhs, GPR64:$rhs, (i32 imm:$imm),
                                   GPR64:$src, GPR64:$src2))]>,
                 Requires<[IsRV64]>;

  def Select64_32 : Pseudo<(outs GPR:$dst),
                           (ins GPR64:$lhs, GPR64:$rhs, i32imm:$imm, GPR:$src,
                                GPR:$src2),
                           [(set i32:$dst,
                            (SelectCC GPR64:$lhs, GPR64:$rhs, (i32 imm:$imm),
                                      GPR:$src, GPR:$src2))]>,
                    Requires<[IsRV64]>;
}

let DecoderNamespace = "RISCV64_" in {
def FENCE64 : FI<0b000, 0b0001111, (outs), (ins uimm4:$pred, uimm4:$succ),
               "fence\t$pred, $succ", []> {
  bits<4> pred;
  bits<4> succ;

  let rs1 = 0;
  let rd = 0;
  let imm12 = {0b0000,pred,succ};
}

def FENCEI64 : FI<0b001, 0b0001111, (outs), (ins), "fence.i", []> {
  let rs1 = 0;
  let rd = 0;
  let imm12 = 0;
}

let rs1=0, rd=0 in {
  def ECALL64  : FI<0b000, 0b1110011, (outs), (ins), "ecall", []> {
    let imm12=0;
  }
  def EBREAK64 : FI<0b000, 0b1110011, (outs), (ins), "ebreak", []> {
    let imm12=1;
  }
}
}

class CSR64_rr<bits<3> funct3, string OpcodeStr> :
      FI<funct3, 0b1110011, (outs GPR64:$rd), (ins uimm12:$imm12, GPR64:$rs1),
         OpcodeStr#"\t$rd, $imm12, $rs1", []>;

class CSR64_ri<bits<3> funct3, string OpcodeStr> :
      FI<funct3, 0b1110011, (outs GPR64:$rd), (ins uimm12:$imm12, uimm5:$rs1),
         OpcodeStr#"\t$rd, $imm12, $rs1", []>;

let DecoderNamespace = "RISCV64_" in {
def CSRRW64 : CSR64_rr<0b001, "csrrw">;
def CSRRS64 : CSR64_rr<0b010, "csrrs">;
def CSRRC64 : CSR64_rr<0b011, "csrrc">;

def CSRRWI64 : CSR64_ri<0b101, "csrrwi">;
def CSRRSI64 : CSR64_ri<0b110, "csrrsi">;
def CSRRCI64 : CSR64_ri<0b111, "csrrci">;
}

def : InstAlias<"nop", (SLLI64 X0_64, X0_64, 0)>, Requires<[IsRV64]>;
