//===-- RISCVTargetStreamer.h - RISCV Target Streamer ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVTARGETSTREAMER_H
#define LLVM_LIB_TARGET_RISCV_RISCVTARGETSTREAMER_H

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class RISCVTargetStreamer : public MCTargetStreamer {
public:
  RISCVTargetStreamer(MCStreamer &S);
};

// This part is for ELF object output
class RISCVTargetELFStreamer : public RISCVTargetStreamer {
  const MCSubtargetInfo &STI;

public:
  MCELFStreamer &getStreamer();
  RISCVTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
};
}
#endif
