//===- RISCVInstrInfoC.td - Compressed RISCV64 instructions -*- tblgen-*---===//
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

let DecoderNamespace = "RISCV64_" in
def CLWSP64: Stack_Load<0b010, "c.lwsp", GPR64, addr_sp_imm6u_word>,
             Requires<[HasC]> {
  let Inst{6-4} = imm{4-2};
  let Inst{3-2} = imm{7-6};
}

let DecoderNamespace = "RISCV64_" in
def CSWSP64: Stack_Store<0b110, "c.swsp", GPR64, addr_sp_imm6u_word>,
             Requires<[HasC]> {
  let Inst{12-9} = offset{5-2};
  let Inst{8-7}  = offset{7-6};
}

let DecoderNamespace = "RISCV64_" in
def CLDSP  : Stack_Load<0b011, "c.ldsp", GPR64, addr_sp_imm6u_double>,
             Requires<[HasC, IsRV64]> {
  let Inst{6-5} = imm{4-3};
  let Inst{4-2} = imm{8-6};
}

let DecoderNamespace = "RISCV64_" in
def CSDSP  : Stack_Store<0b111, "c.sdsp", GPR64, addr_sp_imm6u_double>,
             Requires<[HasC, IsRV64]> {
  let Inst{12-10} = offset{5-3};
  let Inst{9-7}   = offset{8-6};
}

//===----------------------------------------------------------------------===//
// Register-Based Loads and Stores
//===----------------------------------------------------------------------===//

let DecoderNamespace = "RISCV64_" in
def CLW64: Reg_Load<0b010, "c.lw", sextloadi32, GPR64C, addr_reg_imm5u_word>,
           Requires<[HasC, IsRV64]>;

let DecoderNamespace = "RISCV64_" in
def CLD  : Reg_Load<0b011, "c.ld", load, GPR64C, addr_reg_imm5u_double>,
           Requires<[HasC, IsRV64]>;

let DecoderNamespace = "RISCV64_" in
def CSW64: Reg_Store<0b110, "c.sw", truncstorei32, GPR64C, addr_reg_imm5u_word>,
           Requires<[HasC, IsRV64]>;

let DecoderNamespace = "RISCV64_" in
def CSD  : Reg_Store<0b111, "c.sd", store, GPR64C, addr_reg_imm5u_double>,
           Requires<[HasC, IsRV64]>;

//===----------------------------------------------------------------------===//
// Control Transfer Instructions
//===----------------------------------------------------------------------===//

let isCall=1, Defs=[X1_64], rs2 = 0, DecoderNamespace = "RISCV64_" in {
def CJALR64 : CR<0b1001, 0b10, (outs), (ins GPR64:$rs1),
                 "c.jalr\t$rs1",
                 [(Call GPR64:$rs1)]>, Requires<[HasC]>;
}

let DecoderNamespace = "RISCV64_" in {
def CJ64  : Jump_Imm, Requires<[HasC]>;
def CJR64 : Jump_Reg<GPR64>, Requires<[HasC]>;
def CBEQZ64 : Bcz<0b110, "c.beqz",  seteq, GPR64C>, Requires<[HasC]>;
def CBNEZ64 : Bcz<0b111, "c.bnez",  setne, GPR64C>, Requires<[HasC]>;
}

//===----------------------------------------------------------------------===//
// Integer Computational Instructions
//===----------------------------------------------------------------------===//

let DecoderNamespace = "RISCV64_" in
def CADDIW : CI<0b001, 0b01, (outs GPR:$rd_wb), (ins GPR:$rd, simm6:$imm),
                "c.addiw\t$rd, $imm",
                [(set GPR:$rd_wb, (add GPR:$rd, simm6:$imm))]>,
             Requires<[HasC, IsRV64]> {
  let Constraints = "$rd = $rd_wb";
  let Inst{6-2} = imm{4-0};
}

// Get i32 value from GPR64 register
def :RVCPat<(i32 (trunc GPR64:$src)),
            (CADDIW (EXTRACT_SUBREG GPR64:$src, sub_32), 0)>;

let DecoderNamespace = "RISCV64_" in {
def CLI64    : Move_Imm<GPR64, imm64sx6>, Requires<[HasC]>;
def CLUI64   : Move_High<GPR64, imm64sxu6>, Requires<[HasC]>;
def CADDI64  : Add_Imm<GPR64, imm64sx6>, Requires<[HasC, IsRV64]>;

def CADDI16SP64 : ADDI_16SP<SP64>, Requires<[HasC]>;
def CADDI4SPN64 : ADDI_4SPN<GPR64C, SP64>, Requires<[HasC]>;

def CSLLI64  : Shift_left<GPR64, uimm6, immZExt6>, Requires<[HasC, IsRV64]>;

def CSRLI64  : Shift_right<0b00, "c.srli",  srl, GPR64C, uimm6, immZExt6>,
               Requires<[HasC, IsRV64]>;

def CSRAI64  : Shift_right<0b01, "c.srai",  sra, GPR64C, uimm6, immZExt6>,
               Requires<[HasC, IsRV64]>;

def CANDI64  : And_Imm<GPR64C, imm64sx6>, Requires<[HasC]>;
def CADD64   : Add_Reg<GPR64>, Requires<[HasC, IsRV64]>;

def CMV64    : Move_Reg<GPR64>, Requires<[HasC]>;

def CAND64   : CS_ALU<0b11, "c.and",  and, GPR64C, 0>, Requires<[HasC]>;
def COR64    : CS_ALU<0b10, "c.or" ,   or, GPR64C, 0>, Requires<[HasC]>;
def CXOR64   : CS_ALU<0b01, "c.xor",  xor, GPR64C, 0>, Requires<[HasC]>;
def CSUB64   : CS_ALU<0b00, "c.sub",  sub, GPR64C, 0>, Requires<[HasC]>;
def CSUBW    : CS_ALU<0b00, "c.subw", sub, GPRC, 1>, Requires<[HasC, IsRV64]>;
def CADDW    : CS_ALU<0b01, "c.addw", add, GPRC, 1>, Requires<[HasC, IsRV64]>;
}

let rd = 0, imm = 0, DecoderNamespace = "RISCV64_" in
def CNOP64 : CI<0b000, 0b01, (outs), (ins), "c.nop", []>, Requires<[HasC]>;

let rs1 = 0, rs2 = 0, DecoderNamespace = "RISCV64_" in
def CEBREAK64 : CR<0b1001, 0b10, (outs), (ins), "c.ebreak", []>, Requires<[HasC]>;
