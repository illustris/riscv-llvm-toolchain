//===-- RISCVBaseInfo.h - Top level definitions for RISCV MC ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone enum definitions for the RISCV target
// useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVBASEINFO_H
#define LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVBASEINFO_H

#include "RISCVMCTargetDesc.h"

namespace llvm {

// RISCVII - This namespace holds all of the target specific flags that
// instruction info tracks. All definitions must match RISCVInstrFormats.td.
namespace RISCVII {
enum {
  Pseudo = 0,
  FrmR = 1,
  FrmI = 2,
  FrmS = 3,
  FrmSB = 4,
  FrmU = 5,
  FrmOther = 6,

  FormMask = 15
};
enum {
  MO_None,
  MO_LO,
  MO_HI,
  MO_PCREL_HI,
};
}
}

#endif
