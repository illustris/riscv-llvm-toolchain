//===- RISCVInstrInfoM.td - Multiply Divide RISCV instructions -*-tblgen-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//RV32
def MUL   : ALU_rr<0b0000001, 0b000, "mul",   mul>, Requires<[IsRV32, HasM]>;
def MULH  : ALU_rr<0b0000001, 0b001, "mulh",  mulhs>, Requires<[HasM]>;
def MULHU : ALU_rr<0b0000001, 0b011, "mulhu", mulhu>, Requires<[HasM]>;
def DIV   : ALU_rr<0b0000001, 0b100, "div",   sdiv>, Requires<[IsRV32, HasM]>;
def DIVU  : ALU_rr<0b0000001, 0b101, "divu",  udiv>, Requires<[IsRV32, HasM]>;
def REM   : ALU_rr<0b0000001, 0b110, "rem",   srem>, Requires<[IsRV32, HasM]>;
def REMU  : ALU_rr<0b0000001, 0b111, "remu",  urem>, Requires<[IsRV32, HasM]>;

//RV64
let DecoderNamespace = "RISCV64_" in {
def MUL64    : ALU64_rr<0b0000001, 0b000, "mul",   mul>,
               Requires<[IsRV64, HasM]>;
def MULH64   : ALU64_rr<0b0000001, 0b001, "mulh",  mulhs>,
               Requires<[IsRV64, HasM]>;
def MULHU64  : ALU64_rr<0b0000001, 0b011, "mulhu", mulhu>,
               Requires<[IsRV64, HasM]>;
def DIV64    : ALU64_rr<0b0000001, 0b100, "div",   sdiv>,
               Requires<[IsRV64, HasM]>;
def DIVU64   : ALU64_rr<0b0000001, 0b101, "divu",  udiv>,
               Requires<[IsRV64, HasM]>;
def REM64    : ALU64_rr<0b0000001, 0b110, "rem",   srem>,
               Requires<[IsRV64, HasM]>;
def REMU64   : ALU64_rr<0b0000001, 0b111, "remu",  urem>,
               Requires<[IsRV64, HasM]>;

def MULW  : ALUW_rr<0b0000001, 0b000, "mulw",  mul>,  Requires<[IsRV64, HasM]>;
def DIVW  : ALUW_rr<0b0000001, 0b100, "divw",  sdiv>, Requires<[IsRV64, HasM]>;
def DIVUW : ALUW_rr<0b0000001, 0b101, "divuw", udiv>, Requires<[IsRV64, HasM]>;
def REMW  : ALUW_rr<0b0000001, 0b110, "remw",  srem>, Requires<[IsRV64, HasM]>;
def REMUW : ALUW_rr<0b0000001, 0b111, "remuw", urem>, Requires<[IsRV64, HasM]>;
} // End of DecoderNamespace

//TODO: no corresponding llvm ir instruction
//def MULHSU : ALU64_rr<0b0000001, 0b010, "mulh", mulhs>, Requires<[IsRV64, HasM]>;
