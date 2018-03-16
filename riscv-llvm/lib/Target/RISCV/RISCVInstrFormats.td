//===-- RISCVInstrFormats.td - RISCV Instruction Formats ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
//  These instruction format definitions are structured to match the
//  description in the RISC-V User-Level ISA specification as closely as
//  possible. For instance, the specification describes instructions with the
//  MSB (31st bit) on the left and the LSB (0th bit) on the right. This is
//  reflected in the order of parameters to each instruction class.
//
//  One area of divergence is in the description of immediates. The
//  specification describes immediate encoding in terms of bit-slicing
//  operations on the logical value represented. The immediate argument to
//  these instruction formats instead represents the bit sequence that will be
//  inserted into the instruction. e.g. although JAL's immediate is logically
//  a 21-bit value (where the LSB is always zero), we describe it as an imm20
//  to match how it is encoded.
//
//===----------------------------------------------------------------------===//

// Format specifies the encoding used by the instruction. This is used by
// RISCVMCCodeEmitter to determine which form of fixup to use. These
// definitions must be kept in-sync with RISCVBaseInfo.h.
class Format<bits<4> val> {
  bits<4> Value = val;
}
def FrmPseudo : Format<0>;
def FrmR      : Format<1>;
def FrmI      : Format<2>;
def FrmS      : Format<3>;
def FrmSB     : Format<4>;
def FrmU      : Format<5>;
def FrmOther  : Format<6>;
def FrmC      : Format<7>;

class RISCVInst<dag outs, dag ins, string asmstr, list<dag> pattern, Format format>
    : Instruction {
  let Namespace = "RISCV";

  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let TSFlags{3-0} = format.Value;
}

class RISCV32Inst<dag outs, dag ins, string asmstr, list<dag> pattern, Format format>
    : RISCVInst<outs, ins, asmstr, pattern, format>
{
  field bits<32> Inst;
  let Size = 4;
  bits<7> Opcode = 0;
  let Inst{6-0} = Opcode;
}

class RISCV16Inst<dag outs, dag ins, string asmstr, list<dag> pattern, Format format>
    : RISCVInst<outs, ins, asmstr, pattern, format>
{
  field bits<16> Inst;
  let Size = 2;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, list<dag> pattern>
    : RISCVInst<outs, ins, "", pattern, FrmPseudo> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

class FR<bits<7> funct7, bits<3> funct3, bits<7> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern> : RISCV32Inst<outs, ins, asmstr, pattern, FrmR>
{
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode;
}

class FI<bits<3> funct3, bits<7> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : RISCV32Inst<outs, ins, asmstr, pattern, FrmI>
{
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = imm12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode;
}

class LoadRI<bits<3> funct3, string OpcodeStr, SDPatternOperator Op,
             RegisterClass cls> :
      FI<funct3, 0b0000011, (outs cls:$rd), (ins addr_reg_imm12s:$addr),
         OpcodeStr#"\t$rd, $addr",
	 [(set cls:$rd, (Op addr_reg_imm12s:$addr))]> {
  bits<17> addr;
  let Inst{31-15} = addr;
  let mayLoad = 1;
}

class ShiftRI<bit arithshift, bits<3> funct3, bits<7> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : RISCV32Inst<outs, ins, asmstr, pattern, FrmI>
{
  bits<6> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31} = 0;
  let Inst{30} = arithshift;
  let Inst{29-25} = 0;
  let Inst{24-20} = shamt{4-0};
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode;
}

class FS<bits<3> funct3, bits<7> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : RISCV32Inst<outs, ins, asmstr, pattern, FrmS>
{
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = imm12{11-5};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = imm12{4-0};
  let Opcode = opcode;
}

class StoreRI<bits<3> funct3, string OpcodeStr, SDPatternOperator Op,
              RegisterClass cls> :
      FS<funct3, 0b0100011, (outs), (ins cls:$rs2, addr_reg_imm12s:$addr),
         OpcodeStr#"\t$rs2, $addr",
	 [(Op cls:$rs2, addr_reg_imm12s:$addr)]> {
  let mayStore = 1;
  bits<17> addr;
  let Inst{31-25} = addr{16-10};
  let Inst{19-15} = addr{9-5};
  let Inst{11-7} = addr{4-0};
}

class FSB<bits<3> funct3, bits<7> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : RISCV32Inst<outs, ins, asmstr, pattern, FrmSB>
{
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31} = imm12{11};
  let Inst{30-25} = imm12{9-4};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-8} = imm12{3-0};
  let Inst{7} = imm12{10};
  let Opcode = opcode;
}

class FU<bits<7> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : RISCV32Inst<outs, ins, asmstr, pattern, FrmU>
{
  bits<20> imm20;
  bits<5> rd;

  let Inst{31-12} = imm20;
  let Inst{11-7} = rd;
  let Opcode = opcode;
}

class FUJ<bits<7> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : RISCV32Inst<outs, ins, asmstr, pattern, FrmU>
{
  bits<20> imm20;
  bits<5> rd;

  let Inst{31} = imm20{19};
  let Inst{30-21} = imm20{9-0};
  let Inst{20} = imm20{10};
  let Inst{19-12} = imm20{18-11};
  let Inst{11-7} = rd;
  let Opcode = opcode;
}

//A-Type
class InstA<string mnemonic, bits<7> op, bits<5> funct5, bits<3> funct3,
            SDPatternOperator operator, RegisterClass cls1,
            Operand cls2>
  : RISCV32Inst<(outs cls1:$dst), (ins cls1:$src1, cls2:$src2),
               mnemonic#"\t$dst, $src1, $src2",
               [(set cls1:$dst, (operator regaddr:$src2, cls1:$src1))], FrmOther> {
  field bits<32> Inst;

  bits<5> RD;
  bits<5> RS1;
  bits<5> RS2;

  let Inst{31-27} = funct5;
  let Inst{26} = 0;//aq
  let Inst{25} = 0;//rl
  let Inst{24-20} = RS2;
  let Inst{19-15} = RS1;
  let Inst{14-12} = funct3;
  let Inst{11- 7} = RD;
  let Inst{6 - 0} = op;
}

//LR/SC
class InstLR<string mnemonic, bits<3> funct3,
             RegisterClass cls1, Operand cls2>
  : RISCV32Inst<(outs cls1:$dst), (ins cls2:$src2),
              mnemonic#"\t$dst, $src2", 
              [], FrmOther> {
  field bits<32> Inst;

  bits<5> RD;
  bits<5> RS1;

  let Inst{31-27} = 0b00010;
  let Inst{26} = 0;//aq
  let Inst{25} = 0;//rl
  let Inst{24-20} = 0b00000;
  let Inst{19-15} = RS1;
  let Inst{14-12} = funct3;
  let Inst{11- 7} = RD;
  let Inst{6 - 0} = 0b0101111;
}

class InstSC<string mnemonic, bits<3> funct3,
             RegisterClass reg, Operand memOp>
  : RISCV32Inst<(outs reg:$dst), (ins reg:$src2, memOp:$src1),
              mnemonic#"\t$dst, $src2, $src1", 
              [], FrmOther> {
  field bits<32> Inst;

  bits<5> RD;
  bits<5> RS1;
  bits<5> RS2;

  let Inst{31-27} = 0b00011;
  let Inst{26} = 0;//aq
  let Inst{25} = 0;//rl
  let Inst{24-20} = RS2;
  let Inst{19-15} = RS1;
  let Inst{14-12} = funct3;
  let Inst{11- 7} = RD;
  let Inst{6 - 0} = 0b0101111;
}

class CR<bits<4> funct4, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<5> rs1;
  bits<5> rs2;

  let Inst{15-12} = funct4;
  let Inst{11-7} = rs1;
  let Inst{6-2} = rs2;
  let Inst{1-0} = opcode;
}

class CI<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<10> imm;
  bits<5> rd;

  let Inst{15-13} = funct3;
  let Inst{12} = imm{5};
  let Inst{11-7} = rd;
  let Inst{1-0} = opcode;
}

class CSS<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<10> offset;
  bits<5> rs2;

  let Inst{15-13} = funct3;
  let Inst{6-2} = rs2;
  let Inst{1-0} = opcode;
}

class CIW<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<10> imm;
  bits<3> rd;

  let Inst{15-13} = funct3;
  let Inst{4-2} = rd;
  let Inst{1-0} = opcode;
}

class CL<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<3> rd;
  bits<8> addr;

  let Inst{15-13} = funct3;
  let Inst{12-5} = addr;
  let Inst{4-2} = rd;
  let Inst{1-0} = opcode;
}

class CS<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<3> rs2;

  let Inst{15-13} = funct3;
  let Inst{4-2} = rs2;
  let Inst{1-0} = opcode;
}

class CB<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<9> imm;
  bits<3> rs1;

  let Inst{15-13} = funct3;
  let Inst{9-7} = rs1;
  let Inst{1-0} = opcode;
}

class CJ<bits<3> funct3, bits<2> opcode, dag outs, dag ins,
         string asmstr, list<dag> pattern>
    : RISCV16Inst<outs, ins, asmstr, pattern, FrmC>
{
  bits<12> offset;

  let Inst{15-13} = funct3;
  let Inst{12} = offset{11};
  let Inst{11} = offset{4};
  let Inst{10-9} = offset{9-8};
  let Inst{8} = offset{10};
  let Inst{7} = offset{6};
  let Inst{6} = offset{7};
  let Inst{5-3} = offset{3-1};
  let Inst{2} = offset{5};
  let Inst{1-0} = opcode;
}
