//===-- RISCVRegisterInfo.td - RISC-V Register defs --------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the RISC-V register file
//===----------------------------------------------------------------------===//

let Namespace = "RISCV" in {
def sub_32     : SubRegIndex<32>;
}

// We have banks of 32 registers each.
class RISCVReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "RISCV";
}

class RISCVRegWithSubRegs<bits<16> Enc, string n, list<Register> subregs>
  : RegisterWithSubRegs<n, subregs> {
  let HWEncoding = Enc;
  let Namespace = "RISCV";
}

// RISCV CPU Registers
class RISCVGPRReg<bits<16> Enc, string n> : RISCVReg<Enc, n>;

// RISCV 64-bit CPU Registers
class RISCV64GPRReg<bits<16> Enc, string n, list<Register> subregs>
  : RISCVRegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_32];
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

let Namespace = "RISCV" in {
  // General Purpose Registers
  def X0_32   : RISCVGPRReg< 0,  "zero">, DwarfRegNum<[0]>;
  def X1_32   : RISCVGPRReg< 1,  "ra">,   DwarfRegNum<[1]>;
  def X2_32   : RISCVGPRReg< 2,  "sp">,   DwarfRegNum<[2]>;
  def X3_32   : RISCVGPRReg< 3,  "gp">,   DwarfRegNum<[3]>;
  def X4_32   : RISCVGPRReg< 4,  "tp">,   DwarfRegNum<[4]>;
  def X5_32   : RISCVGPRReg< 5,  "t0">,   DwarfRegNum<[5]>;
  def X6_32   : RISCVGPRReg< 6,  "t1">,   DwarfRegNum<[6]>;
  def X7_32   : RISCVGPRReg< 7,  "t2">,   DwarfRegNum<[7]>;
  def X8_32   : RISCVGPRReg< 8,  "s0">,   DwarfRegNum<[8]>;
  def X9_32   : RISCVGPRReg< 9,  "s1">,   DwarfRegNum<[9]>;
  def X10_32  : RISCVGPRReg< 10, "a0">,   DwarfRegNum<[10]>;
  def X11_32  : RISCVGPRReg< 11, "a1">,   DwarfRegNum<[11]>;
  def X12_32  : RISCVGPRReg< 12, "a2">,   DwarfRegNum<[12]>;
  def X13_32  : RISCVGPRReg< 13, "a3">,   DwarfRegNum<[13]>;
  def X14_32  : RISCVGPRReg< 14, "a4">,   DwarfRegNum<[14]>;
  def X15_32  : RISCVGPRReg< 15, "a5">,   DwarfRegNum<[15]>;
  def X16_32  : RISCVGPRReg< 16, "a6">,   DwarfRegNum<[16]>;
  def X17_32  : RISCVGPRReg< 17, "a7">,   DwarfRegNum<[17]>;
  def X18_32  : RISCVGPRReg< 18, "s2">,   DwarfRegNum<[18]>;
  def X19_32  : RISCVGPRReg< 19, "s3">,   DwarfRegNum<[19]>;
  def X20_32  : RISCVGPRReg< 20, "s4">,   DwarfRegNum<[20]>;
  def X21_32  : RISCVGPRReg< 21, "s5">,   DwarfRegNum<[21]>;
  def X22_32  : RISCVGPRReg< 22, "s6">,   DwarfRegNum<[22]>;
  def X23_32  : RISCVGPRReg< 23, "s7">,   DwarfRegNum<[23]>;
  def X24_32  : RISCVGPRReg< 24, "s8">,   DwarfRegNum<[24]>;
  def X25_32  : RISCVGPRReg< 25, "s9">,   DwarfRegNum<[25]>;
  def X26_32  : RISCVGPRReg< 26, "s10">,  DwarfRegNum<[26]>;
  def X27_32  : RISCVGPRReg< 27, "s11">,  DwarfRegNum<[27]>;
  def X28_32  : RISCVGPRReg< 28, "t3">,   DwarfRegNum<[28]>;
  def X29_32  : RISCVGPRReg< 29, "t4">,   DwarfRegNum<[29]>;
  def X30_32  : RISCVGPRReg< 30, "t5">,   DwarfRegNum<[30]>;
  def X31_32  : RISCVGPRReg< 31, "t6">,   DwarfRegNum<[31]>;

  // General Purpose 64-bit Registers
  def X0_64   : RISCV64GPRReg< 0,  "zero", [X0_32]>,  DwarfRegNum<[0]>;
  def X1_64   : RISCV64GPRReg< 1,  "ra",   [X1_32]>,  DwarfRegNum<[1]>;
  def X2_64   : RISCV64GPRReg< 2,  "sp",   [X2_32]>,  DwarfRegNum<[2]>;
  def X3_64   : RISCV64GPRReg< 3,  "gp",   [X3_32]>,  DwarfRegNum<[3]>;
  def X4_64   : RISCV64GPRReg< 4,  "tp",   [X4_32]>,  DwarfRegNum<[4]>;
  def X5_64   : RISCV64GPRReg< 5,  "t0",   [X5_32]>,  DwarfRegNum<[5]>;
  def X6_64   : RISCV64GPRReg< 6,  "t1",   [X6_32]>,  DwarfRegNum<[6]>;
  def X7_64   : RISCV64GPRReg< 7,  "t2",   [X7_32]>,  DwarfRegNum<[7]>;
  def X8_64   : RISCV64GPRReg< 8,  "s0",   [X8_32]>,  DwarfRegNum<[8]>;
  def X9_64   : RISCV64GPRReg< 9,  "s1",   [X9_32]>,  DwarfRegNum<[9]>;
  def X10_64  : RISCV64GPRReg< 10, "a0",   [X10_32]>, DwarfRegNum<[10]>;
  def X11_64  : RISCV64GPRReg< 11, "a1",   [X11_32]>, DwarfRegNum<[11]>;
  def X12_64  : RISCV64GPRReg< 12, "a2",   [X12_32]>, DwarfRegNum<[12]>;
  def X13_64  : RISCV64GPRReg< 13, "a3",   [X13_32]>, DwarfRegNum<[13]>;
  def X14_64  : RISCV64GPRReg< 14, "a4",   [X14_32]>, DwarfRegNum<[14]>;
  def X15_64  : RISCV64GPRReg< 15, "a5",   [X15_32]>, DwarfRegNum<[15]>;
  def X16_64  : RISCV64GPRReg< 16, "a6",   [X16_32]>, DwarfRegNum<[16]>;
  def X17_64  : RISCV64GPRReg< 17, "a7",   [X17_32]>, DwarfRegNum<[17]>;
  def X18_64  : RISCV64GPRReg< 18, "s2",   [X18_32]>, DwarfRegNum<[18]>;
  def X19_64  : RISCV64GPRReg< 19, "s3",   [X19_32]>, DwarfRegNum<[19]>;
  def X20_64  : RISCV64GPRReg< 20, "s4",   [X20_32]>, DwarfRegNum<[20]>;
  def X21_64  : RISCV64GPRReg< 21, "s5",   [X21_32]>, DwarfRegNum<[21]>;
  def X22_64  : RISCV64GPRReg< 22, "s6",   [X22_32]>, DwarfRegNum<[22]>;
  def X23_64  : RISCV64GPRReg< 23, "s7",   [X23_32]>, DwarfRegNum<[23]>;
  def X24_64  : RISCV64GPRReg< 24, "s8",   [X24_32]>, DwarfRegNum<[24]>;
  def X25_64  : RISCV64GPRReg< 25, "s9",   [X25_32]>, DwarfRegNum<[25]>;
  def X26_64  : RISCV64GPRReg< 26, "s10",  [X26_32]>, DwarfRegNum<[26]>;
  def X27_64  : RISCV64GPRReg< 27, "s11",  [X27_32]>, DwarfRegNum<[27]>;
  def X28_64  : RISCV64GPRReg< 28, "t3",   [X28_32]>, DwarfRegNum<[28]>;
  def X29_64  : RISCV64GPRReg< 29, "t4",   [X29_32]>, DwarfRegNum<[29]>;
  def X30_64  : RISCV64GPRReg< 30, "t5",   [X30_32]>, DwarfRegNum<[30]>;
  def X31_64  : RISCV64GPRReg< 31, "t6",   [X31_32]>, DwarfRegNum<[31]>;
}

// The order of registers represents the preferred allocation sequence.
// Registers are listed in the order caller-save, callee-save, specials.
def GPR : RegisterClass<"RISCV", [i32], 32, (add
  (sequence "X%u_32", 10, 17),
  (sequence "X%u_32", 5, 7),
  (sequence "X%u_32", 28, 31),
  (sequence "X%u_32", 8, 9),
  (sequence "X%u_32", 18, 27),
  (sequence "X%u_32", 0, 4)
)>;

def GPR64 : RegisterClass<"RISCV", [i64], 64, (add
  (sequence "X%u_64", 10, 17),
  (sequence "X%u_64", 5, 7),
  (sequence "X%u_64", 28, 31),
  (sequence "X%u_64", 8, 9),
  (sequence "X%u_64", 18, 27),
  (sequence "X%u_64", 0, 4)
)>;

def GPRC : RegisterClass<"RISCV", [i32], 32, (add
  (sequence "X%u_32", 10, 15),
  (sequence "X%u_32", 8, 9)
)>;

def GPR64C : RegisterClass<"RISCV", [i64], 64, (add
  (sequence "X%u_64", 10, 15),
  (sequence "X%u_64", 8, 9)
)>;

def SP   : RegisterClass<"RISCV", [i32], 32, (add X2_32)>;
def SP64 : RegisterClass<"RISCV", [i64], 64, (add X2_64)>;
