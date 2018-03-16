//===- RISCVInstrInfoA.td - Atomic RISCV instructions -*- tblgen-*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//RV32
//TODO: add LR/SC and acq/rel

def AMOSWAP_W : InstA<"amoswap.w" , 0b0101111, 0b00000, 0b010, atomic_swap,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOADD_W  : InstA<"amoadd.w"  , 0b0101111, 0b00001, 0b010, atomic_load_add,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOXOR_W  : InstA<"amoxor.w"  , 0b0101111, 0b00100, 0b010, atomic_load_xor,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOAND_W  : InstA<"amoand.w"  , 0b0101111, 0b01100, 0b010, atomic_load_and,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOOR_W   : InstA<"amoor.w"   , 0b0101111, 0b01000, 0b010, atomic_load_or,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOMIN_W  : InstA<"amomin.w"  , 0b0101111, 0b10000, 0b010, atomic_load_min,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOMAX_W  : InstA<"amomax.w"  , 0b0101111, 0b10100, 0b010, atomic_load_max,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOMINU_W : InstA<"amominu.w" , 0b0101111, 0b11000, 0b010, atomic_load_umin,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;
def AMOMAXU_W : InstA<"amomaxu.w" , 0b0101111, 0b11100, 0b010, atomic_load_umax,
                      GPR, memreg>, Requires<[IsRV32, HasA]>;

def LR_W : InstLR<"lr.w", 0b010, GPR, memreg>, Requires<[HasA]>;
def SC_W : InstSC<"sc.w", 0b010, GPR, memreg>, Requires<[HasA]>;

//RV64A
//TODO: add LR/SC and acq/rel

//TODO add gr32 operations
let DecoderNamespace = "RISCV64_" in {
def AMOSWAP_D   : InstA<"amoswap.d", 0b0101111, 0b00000, 0b011, atomic_swap,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOADD_D    : InstA<"amoadd.D" , 0b0101111, 0b00001, 0b011, atomic_load_add,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOXOR_D    : InstA<"amoxor.d" , 0b0101111, 0b00100, 0b011, atomic_load_xor,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOAND_D    : InstA<"amoand.d" , 0b0101111, 0b01100, 0b011, atomic_load_and,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOOR_D     : InstA<"amoor.d"  , 0b0101111, 0b01000, 0b011, atomic_load_or,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMIN_D    : InstA<"amomin.d" , 0b0101111, 0b10000, 0b011, atomic_load_min,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMAX_D    : InstA<"amomax.d" , 0b0101111, 0b10100, 0b011, atomic_load_max,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMINU_D   : InstA<"amominu.d", 0b0101111, 0b11000, 0b011, atomic_load_umin,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMAXU_D   : InstA<"amomaxu.d", 0b0101111, 0b11100, 0b011, atomic_load_umax,
                        GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def AMOSWAP_W64 : InstA<"amoswap.w", 0b0101111, 0b00000, 0b010, atomic_swap,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOADD_W64  : InstA<"amoadd.w" , 0b0101111, 0b00001, 0b010, atomic_load_add,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOXOR_W64  : InstA<"amoxor.w" , 0b0101111, 0b00100, 0b010, atomic_load_xor,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOAND_W64  : InstA<"amoand.w" , 0b0101111, 0b01100, 0b010, atomic_load_and,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOOR_W64   : InstA<"amoor.w"  , 0b0101111, 0b01000, 0b010, atomic_load_or,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMIN_W64  : InstA<"amomin.w" , 0b0101111, 0b10000, 0b010, atomic_load_min,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMAX_W64  : InstA<"amomax.w" , 0b0101111, 0b10100, 0b010, atomic_load_max,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMINU_W64 : InstA<"amominu.w", 0b0101111, 0b11000, 0b010, atomic_load_umin,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;
def AMOMAXU_W64 : InstA<"amomaxu.w", 0b0101111, 0b11100, 0b010, atomic_load_umax,
                        GPR, memreg64>, Requires<[IsRV64, HasA]>;

def LR_W64 : InstLR<"lr.w", 0b010, GPR,   memreg64>, Requires<[IsRV64, HasA]>;
def SC_W64 : InstSC<"sc.w", 0b010, GPR,   memreg64>, Requires<[IsRV64, HasA]>;
def LR_D   : InstLR<"lr.d", 0b011, GPR64, memreg64>, Requires<[IsRV64, HasA]>;
def SC_D   : InstSC<"sc.d", 0b011, GPR64, memreg64>, Requires<[IsRV64, HasA]>;
} // End of DecoderNamespace
