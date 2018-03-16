//===- RISCVInstrInfoC.td - Compressed RISCV instructions -*- tblgen-*-----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Stack-Pointer-Based Loads and Stores
//===----------------------------------------------------------------------===//

class Stack_Load<bits<3> funct3, string OpcodeStr,
                 RegisterClass cls, DAGOperand opnd> :
      CI<funct3, 0b10, (outs cls:$rd), (ins opnd:$imm),
         OpcodeStr#"\t$rd, $imm", []> {
  let mayLoad = 1;
}

def CLWSP  : Stack_Load<0b010, "c.lwsp", GPR, addr_sp_imm6u_word>,
             Requires<[HasC]> {
  let Inst{6-4} = imm{4-2};
  let Inst{3-2} = imm{7-6};
}

class Stack_Store<bits<3> funct3, string OpcodeStr,
                  RegisterClass cls, DAGOperand opnd> :
      CSS<funct3, 0b10, (outs), (ins cls:$rs2, opnd:$offset),
          OpcodeStr#"\t$rs2, $offset", []> {
  let mayStore = 1;
}

def CSWSP  : Stack_Store<0b110, "c.swsp", GPR, addr_sp_imm6u_word>,
             Requires<[HasC]> {
  let Inst{12-9} = offset{5-2};
  let Inst{8-7}  = offset{7-6};
}

//===----------------------------------------------------------------------===//
// Register-Based Loads and Stores
//===----------------------------------------------------------------------===//

class Reg_Load<bits<3> funct3, string OpcodeStr, SDPatternOperator Op,
               RegisterClass cls, DAGOperand opnd> :
      CL<funct3, 0b00, (outs cls:$rd), (ins opnd:$addr),
         OpcodeStr#"\t$rd, $addr",
         [(set cls:$rd, (Op opnd:$addr))]> {
  let mayLoad = 1;
}

def CLW  : Reg_Load<0b010, "c.lw", load, GPRC, addr_reg_imm5u_word>,
           Requires<[HasC]>;

class Reg_Store<bits<3> funct3, string OpcodeStr, SDPatternOperator Op,
                RegisterClass cls, DAGOperand opnd> :
      CS<funct3, 0b00, (outs), (ins cls:$rs2, opnd:$addr),
         OpcodeStr#"\t$rs2, $addr",
         [(Op cls:$rs2, opnd:$addr)]> {
  let mayStore = 1;
  bits<8> addr;
  let Inst{12-5} = addr;
}

def CSW  : Reg_Store<0b110, "c.sw", store, GPRC, addr_reg_imm5u_word>,
           Requires<[HasC]>;

//===----------------------------------------------------------------------===//
// Control Transfer Instructions
//===----------------------------------------------------------------------===//


class Jump_Imm : CJ<0b101, 0b01, (outs), (ins simm12_lsb0:$offset),
                    "c.j\t$offset",
                    [(br bb:$offset)]> {
  let isBranch = 1;
  let isTerminator=1;
  let isBarrier=1;
}

def CJ : Jump_Imm, Requires<[HasC]>;

class Call_Imm : CJ<0b001, 0b01, (outs), (ins simm12_lsb0:$offset),
                    "c.jal\t$offset", []> {
  let isCall = 1;
}

def CJAL : Call_Imm, Requires<[HasC, IsRV32]>;

class Jump_Reg<RegisterClass cls> :
      CR<0b1000, 0b10, (outs), (ins cls:$rs1),
         "c.jr\t$rs1",
         [(brind cls:$rs1)]> {
  let isBranch = 1;
  let isBarrier = 1;
  let isTerminator = 1;
  let isIndirectBranch = 1;
  let rs2 = 0;
}

def CJR : Jump_Reg<GPR>, Requires<[HasC]>;

let isCall=1, Defs=[X1_32], rs2 = 0 in {
def CJALR : CR<0b1001, 0b10, (outs), (ins GPR:$rs1),
               "c.jalr\t$rs1",
               [(Call GPR:$rs1)]>, Requires<[HasC]>;
}

class Bcz<bits<3> funct3, string OpcodeStr, PatFrag CondOp,
          RegisterClass cls> :
      CB<funct3, 0b01, (outs), (ins cls:$rs1, simm9_lsb0:$imm),
         OpcodeStr#"\t$rs1, $imm",
         [(brcond (i32 (CondOp cls:$rs1, 0)), bb:$imm)]> {
  let isBranch = 1;
  let isTerminator = 1;
  let Inst{12} = imm{8};
  let Inst{11-10} = imm{4-3};
  let Inst{6-5} = imm{7-6};
  let Inst{4-3} = imm{2-1};
  let Inst{2} = imm{5};
}

def CBEQZ   : Bcz<0b110, "c.beqz",  seteq, GPRC>, Requires<[HasC]>;
def CBNEZ   : Bcz<0b111, "c.bnez",  setne, GPRC>, Requires<[HasC]>;

//===----------------------------------------------------------------------===//
// Integer Computational Instructions
//===----------------------------------------------------------------------===//

class Move_Imm<RegisterClass cls, Operand ImmOpnd> :
      CI<0b010, 0b01, (outs cls:$rd), (ins ImmOpnd:$imm),
         "c.li\t$rd, $imm",
         [(set cls:$rd, ImmOpnd:$imm)]> {
  let Inst{6-2} = imm{4-0};
}

def CLI   : Move_Imm<GPR, simm6>, Requires<[HasC]>;

class Move_High<RegisterClass cls, Operand ImmOpnd> :
      CI<0b011, 0b01, (outs cls:$rd), (ins ImmOpnd:$imm),
         "c.lui\t$rd, $imm", []> {
  let Inst{6-2} = imm{4-0};
}

def CLUI   : Move_High<GPR, uimm6>, Requires<[HasC]>;

class Add_Imm<RegisterClass cls, Operand ImmOpnd> :
      CI<0b000, 0b01, (outs cls:$rd_wb), (ins cls:$rd, ImmOpnd:$imm),
         "c.addi\t$rd, $imm",
         [(set cls:$rd_wb, (add cls:$rd, ImmOpnd:$imm))]> {
  let Constraints = "$rd = $rd_wb";
  let Inst{6-2} = imm{4-0};
}

def CADDI   : Add_Imm<GPR, simm6>, Requires<[HasC, IsRV32]>;

class ADDI_16SP<RegisterClass cls> :
      CI<0b011, 0b01, (outs cls:$rd_wb),
         (ins cls:$rd, simm10_4lsb0:$imm),
         "c.addi16sp\t$rd, $imm", []> {
  let Constraints = "$rd = $rd_wb";
  let Inst{12} = imm{9};
  let Inst{11-7} = 2;
  let Inst{6} = imm{4};
  let Inst{5} = imm{6};
  let Inst{4-3} = imm{8-7};
  let Inst{2} = imm{5};
}

def CADDI16SP : ADDI_16SP<SP>, Requires<[HasC]>;

class ADDI_4SPN<RegisterClass cls, RegisterClass spcls> :
      CIW<0b000, 0b00, (outs cls:$rd),
          (ins spcls:$rs1, uimm10_2lsb0:$imm),
          "c.addi4spn\t$rd, $rs1, $imm", []> {
  bits<5> rs1;
  let Inst{12-11} = imm{5-4};
  let Inst{10-7} = imm{9-6};
  let Inst{6} = imm{2};
  let Inst{5} = imm{3};
}

def CADDI4SPN : ADDI_4SPN<GPRC, SP>, Requires<[HasC]>;

class Shift_left<RegisterClass cls, Operand ImmOpnd, SDPatternOperator PF> :
      CI<0b000, 0b10, (outs cls:$rd_wb),
         (ins cls:$rd, ImmOpnd:$imm),
         "c.slli\t$rd, $imm",
         [(set cls:$rd_wb, (shl cls:$rd, PF:$imm))]> {
  let Constraints = "$rd = $rd_wb";
  let Inst{6-2} = imm{4-0};
}

def CSLLI   : Shift_left<GPR,   uimm5, immZExt5>, Requires<[HasC, IsRV32]>;

class Shift_right<bits<2> funct2, string OpcodeStr, SDPatternOperator OpNode,
                  RegisterClass cls, Operand ImmOpnd, SDPatternOperator PF> :
      CB<0b100, 0b01, (outs cls:$rs1_wb), (ins cls:$rs1, ImmOpnd:$imm),
         OpcodeStr#"\t$rs1, $imm",
         [(set cls:$rs1_wb, (OpNode cls:$rs1, PF:$imm))]> {
  let Constraints = "$rs1 = $rs1_wb";
  let Inst{12} = imm{5};
  let Inst{11-10} = funct2;
  let Inst{6-2} = imm{4-0};
}

def CSRLI   : Shift_right<0b00, "c.srli",  srl, GPRC,   uimm5, immZExt5>,
              Requires<[HasC, IsRV32]>;

def CSRAI   : Shift_right<0b01, "c.srai",  sra, GPRC,   uimm5, immZExt5>,
              Requires<[HasC, IsRV32]>;


class And_Imm<RegisterClass cls, Operand ImmOpnd> :
      CB<0b100, 0b01, (outs cls:$rs1_wb), (ins cls:$rs1, ImmOpnd:$imm),
               "c.andi\t$rs1, $imm",
               [(set cls:$rs1_wb, (and cls:$rs1, ImmOpnd:$imm))]> {
  let Constraints = "$rs1 = $rs1_wb";
  let Inst{12} = imm{5};
  let Inst{11-10} = 0b10;
  let Inst{6-2} = imm{4-0};
}

def CANDI  : And_Imm<GPRC, simm6>, Requires<[HasC]>;

class Move_Reg<RegisterClass cls> :
      CR<0b1000, 0b10, (outs cls:$rs1), (ins cls:$rs2),
         "c.mv\t$rs1, $rs2", []>;

def CMV    : Move_Reg<GPR>, Requires<[HasC]>;

class Add_Reg<RegisterClass cls> :
      CR<0b1001, 0b10, (outs cls:$rs1_wb), (ins cls:$rs1, cls:$rs2),
              "c.add\t$rs1, $rs2",
              [(set cls:$rs1_wb, (add cls:$rs1, cls:$rs2))]> {
  let Constraints = "$rs1 = $rs1_wb";
}

def CADD   : Add_Reg<GPR>, Requires<[HasC, IsRV32]>;

class CS_ALU<bits<2> funct2, string OpcodeStr, SDPatternOperator OpNode,
             RegisterClass cls, bit RV64only> :
      CS<0b100, 0b01, (outs cls:$rd_wb), (ins cls:$rd, cls:$rs2),
         OpcodeStr#"\t$rd, $rs2",
         [(set cls:$rd_wb, (OpNode cls:$rd, cls:$rs2))]> {
  bits<3> rd;
  let Constraints = "$rd = $rd_wb";
  let Inst{12} = RV64only;
  let Inst{11-10} = 0b11;
  let Inst{9-7} = rd;
  let Inst{6-5} = funct2;
}

def CAND   : CS_ALU<0b11, "c.and",  and, GPRC, 0>, Requires<[HasC]>;
def COR    : CS_ALU<0b10, "c.or" ,   or, GPRC, 0>, Requires<[HasC]>;
def CXOR   : CS_ALU<0b01, "c.xor",  xor, GPRC, 0>, Requires<[HasC]>;
def CSUB   : CS_ALU<0b00, "c.sub",  sub, GPRC, 0>, Requires<[HasC, IsRV32]>;

let rd = 0, imm = 0 in
def CNOP : CI<0b000, 0b01, (outs), (ins), "c.nop", []>, Requires<[HasC]>;

let rs1 = 0, rs2 = 0 in
def CEBREAK : CR<0b1001, 0b10, (outs), (ins), "c.ebreak", []>, Requires<[HasC]>;
