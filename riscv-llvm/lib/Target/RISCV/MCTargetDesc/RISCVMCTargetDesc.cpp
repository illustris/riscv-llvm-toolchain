//===-- RISCVMCTargetDesc.cpp - RISCV Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// This file provides RISCV-specific target descriptions.
///
//===----------------------------------------------------------------------===//

#include "RISCVMCTargetDesc.h"
#include "RISCVMCAsmInfo.h"
#include "RISCVTargetStreamer.h"
#include "InstPrinter/RISCVInstPrinter.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "RISCVGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "RISCVGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "RISCVGenSubtargetInfo.inc"

using namespace llvm;

/// Select the RISCV CPU for the given triple and cpu name.
StringRef RISCV_MC::selectRISCVCPU(const Triple &TT, StringRef CPU) {
  if (CPU.empty()) {
    StringRef MArch = TT.getArchName();
    bool Is64Bit = TT.getArch() == llvm::Triple::riscv64;

    if (MArch.startswith("riscv32ema"))
      return "rv32ema";
    if (MArch.startswith("riscv32emac"))
      return "rv32emac";
    if (MArch.startswith("riscv32imac"))
      return "rv32imac";
    if (MArch.startswith("riscv64imac"))
      return "rv64imac";

    if (Is64Bit)
      CPU = "generic-rv64";
    else
      CPU = "generic-rv32";
  }
  return CPU;
}

static MCSubtargetInfo *createRISCVMCSubtargetInfo(const Triple &TT,
                                                   StringRef CPU,
                                                   StringRef FS) {
  std::string CPUName = RISCV_MC::selectRISCVCPU(TT, CPU);
  return createRISCVMCSubtargetInfoImpl(TT, CPUName, FS);
}

static MCInstrInfo *createRISCVMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitRISCVMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createRISCVMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitRISCVMCRegisterInfo(X, RISCV::X1_32);
  return X;
}

static MCAsmInfo *createRISCVMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT) {
  return new RISCVMCAsmInfo(TT);
}

static MCTargetStreamer *
createRISCVObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new RISCVTargetELFStreamer(S, STI);
}

static MCInstPrinter *createRISCVMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new RISCVInstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeRISCVTargetMC() {
  for (Target *T : {&getTheRISCV32Target(), &getTheRISCV64Target()}) {
    TargetRegistry::RegisterMCAsmInfo(*T, createRISCVMCAsmInfo);
    TargetRegistry::RegisterMCInstrInfo(*T, createRISCVMCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createRISCVMCRegisterInfo);
    TargetRegistry::RegisterMCAsmBackend(*T, createRISCVAsmBackend);
    TargetRegistry::RegisterMCCodeEmitter(*T, createRISCVMCCodeEmitter);
    TargetRegistry::RegisterMCInstPrinter(*T, createRISCVMCInstPrinter);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createRISCVMCSubtargetInfo);
    TargetRegistry::RegisterObjectTargetStreamer(
        *T, createRISCVObjectTargetStreamer);
  }
}
