//===-- RISCVTargetStreamer.cpp - RISCV Target Streamer Methods -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides RISCV specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "RISCVMCExpr.h"
#include "RISCVMCTargetDesc.h"
#include "RISCVTargetStreamer.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

RISCVTargetStreamer::RISCVTargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {}

// This part is for ELF object output.
RISCVTargetELFStreamer::RISCVTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : RISCVTargetStreamer(S), STI(STI) {
  MCAssembler &MCA = getStreamer().getAssembler();

  const FeatureBitset &Features = STI.getFeatureBits();

  unsigned EFlags = MCA.getELFHeaderEFlags();

  // Architecture
  if (Features[RISCV::FeatureE])
    EFlags |= ELF::EF_RISCV_RVE;
  else if (Features[RISCV::FeatureC])
    EFlags |= ELF::EF_RISCV_RVC;
  else if (Features[RISCV::FeatureSoftFloat])
    EFlags |= ELF::EF_RISCV_FLOAT_ABI_SOFT;

  MCA.setELFHeaderEFlags(EFlags);
}

MCELFStreamer &RISCVTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
