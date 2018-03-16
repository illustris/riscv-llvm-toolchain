//===-- RISCVRegisterInfo.h - RISCV Register Information Impl ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the RISCV implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVREGISTERINFO_H
#define LLVM_LIB_TARGET_RISCV_RISCVREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "RISCVGenRegisterInfo.inc"

namespace llvm {

class RISCVSubtarget;

struct RISCVRegisterInfo : public RISCVGenRegisterInfo {
public:
  const RISCVSubtarget &Subtarget;

  RISCVRegisterInfo(const RISCVSubtarget &STI);

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  const uint32_t *getNoPreservedMask() const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override;

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override;

  bool requiresVirtualBaseRegisters(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  unsigned getFrameRegister(const MachineFunction &MF) const override;

  // Let ptr_rc could transfer to correct register class
  const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF,
                                                unsigned Kind) const override;
};
}

#endif
