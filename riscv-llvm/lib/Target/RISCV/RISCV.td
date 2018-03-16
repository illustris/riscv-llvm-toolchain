//===-- RISCV.td - Describe the RISCV Target Machine -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// RISCV subtarget features
//===----------------------------------------------------------------------===//

def FeatureM : SubtargetFeature<"m", "HasM", "true",
                                "Supports Integer Multiplication and Division.">;
def FeatureA : SubtargetFeature<"a", "HasA", "true",
                                "Supports Atomic Instructions.">;
def FeatureF : SubtargetFeature<"f", "HasF", "true",
                                "Supports Single-Precision Floating-Point.">;
def FeatureD : SubtargetFeature<"d", "HasD", "true",
                                "Supports Double-Precision Floating-Point.">;
def FeatureE : SubtargetFeature<"e", "HasE", "true",
                                "Supports RV32E.">;
def FeatureC : SubtargetFeature<"c", "HasC", "true",
                                "Supports RVC.">;

def FeatureRV32 : SubtargetFeature<"rv32", "RISCVArchVersion", "RV32", 
                                   "RV32 ISA Support">;
def FeatureRV64 : SubtargetFeature<"rv64", "RISCVArchVersion", "RV64", 
                                   "RV64 ISA Support">;

def FeatureSoftFloat : SubtargetFeature<"soft-float", "UseSoftFloat", "true",
                                        "Use software floating point features.">;

//===----------------------------------------------------------------------===//
// Register file description
//===----------------------------------------------------------------------===//

include "RISCVRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling convention description
//===----------------------------------------------------------------------===//

include "RISCVCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction descriptions
//===----------------------------------------------------------------------===//

include "RISCVInstrInfo.td"


def RISCVInstrInfo : InstrInfo;


def : ProcessorModel<"generic", NoSchedModel, []>;

def : ProcessorModel<"generic-rv32", NoSchedModel, [FeatureRV32]>;

def : ProcessorModel<"generic-rv64", NoSchedModel, [FeatureRV64]>;

def : ProcessorModel<"rv32ema", NoSchedModel, [FeatureRV32, FeatureE,
                                               FeatureM, FeatureA]>;

def : ProcessorModel<"rv32emac", NoSchedModel, [FeatureRV32, FeatureE,
                                                FeatureM, FeatureA, FeatureC]>;

def : ProcessorModel<"rv32imac", NoSchedModel, [FeatureRV32, FeatureM,
                                                FeatureA, FeatureC]>;

def : ProcessorModel<"rv64imac", NoSchedModel, [FeatureRV64, FeatureM,
                                                FeatureA, FeatureC]>;

def RISCVAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def RISCV : Target {
  let InstructionSet = RISCVInstrInfo;
  let AssemblyParsers = [RISCVAsmParser];
}
