//===-- RISCVCallingConv.cpp - Calling conventions for RISCV --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RISCVCallingConv.h"

using namespace llvm;

const MCPhysReg RISCV::RV64ArgGPRs[RISCV::NumArgRV64GPRs] = {
    RISCV::X10_64, RISCV::X11_64, RISCV::X12_64, RISCV::X13_64,
    RISCV::X14_64, RISCV::X15_64, RISCV::X16_64, RISCV::X17_64
};
const MCPhysReg RISCV::RV32ArgGPRs[RISCV::NumArgRV32GPRs] = {
    RISCV::X10_32, RISCV::X11_32, RISCV::X12_32, RISCV::X13_32,
    RISCV::X14_32, RISCV::X15_32, RISCV::X16_32, RISCV::X17_32
};
const MCPhysReg RISCV::RV32EArgGPRs[RISCV::NumArgRV32EGPRs] = {
    RISCV::X10_32, RISCV::X11_32, RISCV::X12_32, RISCV::X13_32,
    RISCV::X14_32, RISCV::X15_32
};
