//===-- RISCVInstrInfo.h - RISCV Instruction Information --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the RISCV implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVINSTRINFO_H
#define LLVM_LIB_TARGET_RISCV_RISCVINSTRINFO_H

#include "RISCVRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "RISCVGenInstrInfo.inc"

namespace llvm {

class RISCVSubtarget;
class RISCVInstrInfo : public RISCVGenInstrInfo {
  const RISCVRegisterInfo RI;
  RISCVSubtarget &STI;
  unsigned UncondBranch, LUI, CLUI, ADDI, CMV;
  unsigned CADDI16SP, CADDI, ADD, CLI;

public:
  explicit RISCVInstrInfo(RISCVSubtarget &STI);

  const RISCVRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
                   const DebugLoc &DL, unsigned DestinationRegister,
                   unsigned SourceRegister, bool KillSource) const override;

  // Get the load and store opcodes for a given register class.
  void getLoadStoreOpcodes(const TargetRegisterClass *RC,
                           unsigned &LoadOpcode, unsigned &StoreOpcode) const;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, unsigned SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI, unsigned DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  // Adjust SP by Amount bytes.
  void adjustStackPtr(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const;

  void loadImmediate(unsigned ScratchReg, int64_t Imm, MachineBasicBlock &MBB,
                     MachineBasicBlock::iterator II, const DebugLoc &DL) const;

  unsigned basePlusImmediate(unsigned DstReg, unsigned BaseReg, int64_t Imm,
                             MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator II,
                             const DebugLoc &DL) const;

  unsigned basePlusImmediateStripOffset(unsigned BaseReg, int64_t &Imm,
                                        MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator II,
                                        const DebugLoc &DL) const;

  unsigned getOppositeBranchOpc(unsigned Opc) const;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  /// \returns true if a branch from an instruction with opcode \p BranchOpc
  ///  bytes is capable of jumping to a position \p BrOffset bytes away.
  bool isBranchOffsetInRange(unsigned BranchOpc,
                             int64_t BrOffset) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  unsigned insertIndirectBranch(MachineBasicBlock &MBB,
                                MachineBasicBlock &NewDestBB,
                                const DebugLoc &DL,
                                int64_t BrOffset,
                                RegScavenger *RS = nullptr) const override;

private:
  void BuildCondBr(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                   const DebugLoc &DL, ArrayRef<MachineOperand> Cond) const;

  void AnalyzeCondBr(const MachineInstr *Inst, unsigned Opc,
                     MachineBasicBlock *&BB,
                     SmallVectorImpl<MachineOperand> &Cond) const;
};
}

#endif
