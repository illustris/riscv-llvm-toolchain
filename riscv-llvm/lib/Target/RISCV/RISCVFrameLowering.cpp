//===-- RISCVFrameLowering.cpp - RISCV Frame Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the RISCV implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "RISCVFrameLowering.h"
#include "RISCVSubtarget.h"
#include "RISCVCallingConv.h"
#include "RISCVMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"

using namespace llvm;

RISCVFrameLowering::RISCVFrameLowering(const RISCVSubtarget &sti)
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/sti.hasE() ? 4: 16,
                            /*LocalAreaOffset=*/0), STI(sti) {}


bool RISCVFrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return (MF.getTarget().Options.DisableFramePointerElim(MF) ||
          MF.getFrameInfo().hasVarSizedObjects() ||
          MFI.isFrameAddressTaken() ||
          TRI->needsStackRealignment(MF));
}

bool RISCVFrameLowering::hasBP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return MFI.hasVarSizedObjects() && TRI->needsStackRealignment(MF);
}

void RISCVFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  unsigned SP = STI.isRV64() ? RISCV::X2_64 : RISCV::X2_32;
  unsigned FP = STI.isRV64() ? RISCV::X8_64 : RISCV::X8_32;
  unsigned ZERO = STI.isRV64() ? RISCV::X0_64 :RISCV::X0_32;
  unsigned ADDI = STI.isRV64() ? RISCV::ADDI64 :RISCV::ADDI;
  unsigned AND = STI.isRV64() ? RISCV::AND64 :RISCV::AND;

  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
  MachineFrameInfo &MFI = MF.getFrameInfo();

  const RISCVInstrInfo &TII =
      *static_cast<const RISCVInstrInfo *>(STI.getInstrInfo());
  const RISCVRegisterInfo &TRI =
      *static_cast<const RISCVRegisterInfo *>(STI.getRegisterInfo());
  const TargetRegisterClass *RC = STI.isRV64() ?
    &RISCV::GPR64RegClass : &RISCV::GPRRegClass;

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL;

  // First, compute final stack size.
  uint64_t StackSize = MFI.getStackSize();
  uint64_t AlignMask = getStackAlignment() - 1;

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI.adjustsStack()) return;

  // Align the stack
  StackSize = (StackSize + AlignMask) & ~AlignMask;
  MFI.setStackSize(StackSize);

  MachineModuleInfo &MMI = MF.getMMI();
  const MCRegisterInfo *MRI = MMI.getContext().getRegisterInfo();

  // Adjust stack.
  TII.adjustStackPtr(SP, -StackSize, MBB, MBBI);

  // emit ".cfi_def_cfa_offset StackSize"
  unsigned CFIIndex = MF.addFrameInst(
      MCCFIInstruction::createDefCfaOffset(nullptr, -StackSize));
  BuildMI(MBB, MBBI, DL, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex);

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

  if (CSI.size()) {
    // Find the instruction past the last instruction that saves a callee-saved
    // register to the stack.
    for (unsigned i = 0; i < CSI.size(); ++i)
      ++MBBI;

    // Iterate over list of callee-saved registers and emit .cfi_offset
    // directives.
    for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
           E = CSI.end(); I != E; ++I) {
      int64_t Offset = MFI.getObjectOffset(I->getFrameIdx());
      unsigned Reg = I->getReg();
      unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createOffset(
            nullptr, MRI->getDwarfRegNum(Reg, 1), Offset));
      BuildMI(MBB, MBBI, DL, TII.get(TargetOpcode::CFI_INSTRUCTION))
          .addCFIIndex(CFIIndex);
    }
  }

  // if framepointer enabled, set it to point to the stack pointer.
  if (hasFP(MF)) {
    if (isInt<12>(StackSize)) {
      BuildMI(MBB, MBBI, DL, TII.get(ADDI), FP).addReg(SP).addImm(StackSize)
        .setMIFlag(MachineInstr::FrameSetup);
    } else {
      TII.loadImmediate (FP, StackSize, MBB, MBBI, DL);

      BuildMI(MBB, MBBI, DL, TII.get(RISCV::ADD), FP).addReg(FP).addReg(SP)
        .setMIFlag(MachineInstr::FrameSetup);
    }
    // emit ".cfi_def_cfa_register $fp"
    unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createDefCfaRegister(
        nullptr, MRI->getDwarfRegNum(FP, true)));
    BuildMI(MBB, MBBI, DL, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);

    // Realignment the stack for structure alignment
    // E.g. pass structure by stack and the structure need 16 byte alignment.
    // See the test case c-c++-common/torture/vector-shift2.c
    // myfunc2 function will return structure by stack.
    if (TRI.needsStackRealignment(MF)) {
      // ADDI $Reg, $zero, -MaxAlignment
      // AND  $sp, $sp, $Reg
      unsigned VR = MF.getRegInfo().createVirtualRegister(RC);
      assert(isInt<16>(MFI.getMaxAlignment()) &&
             "Function's alignment size requirement is not supported.");
      int MaxAlign = -(int)MFI.getMaxAlignment();

      BuildMI(MBB, MBBI, DL, TII.get(ADDI), VR).addReg(ZERO) .addImm(MaxAlign);
      BuildMI(MBB, MBBI, DL, TII.get(AND), SP).addReg(SP).addReg(VR);

      // We need another base register when there are many allocas,
      // LLVM will generate SP alignment adjustment instructions
      // which will modify SP and lost stack base tracking.
      //
      // See testcase gcc.dg/vla-24.c for RV32E.
      // RV32E stack alignment is 4 byte. Therefore, it will generate
      // multiple sp adjustment in this case.
      if (hasBP(MF)) {
        // move BP, SP
        unsigned BP = STI.isRV64() ? RISCV::X9_64 : RISCV::X9_32;
        BuildMI(MBB, MBBI, DL, TII.get(ADDI), BP)
          .addReg(SP)
          .addImm(0);
      }
    }
  }
}

void RISCVFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  unsigned SP = STI.isRV64() ? RISCV::X2_64 : RISCV::X2_32;
  unsigned FP = STI.isRV64() ? RISCV::X8_64 : RISCV::X8_32;
  unsigned ZERO = STI.isRV64() ? RISCV::X0_64 :RISCV::X0_32;
  unsigned ADDI = STI.isRV64() ? RISCV::ADDI64 :RISCV::ADDI;

  MachineFrameInfo &MFI = MF.getFrameInfo();
  const RISCVInstrInfo &TII =
      *static_cast<const RISCVInstrInfo *>(STI.getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI->getDebugLoc();

  // Get the number of bytes from FrameInfo
  uint64_t StackSize = MFI.getStackSize();

  // Restore the stack pointer if framepointer enabled
  // and the stack have been realign or have variable length object
  if (hasFP(MF)) {
    // Find the first instruction that restores a callee-saved register.
    MachineBasicBlock::iterator I = MBBI;

    for (unsigned i = 0; i < MFI.getCalleeSavedInfo().size(); ++i)
      --I;

    // Insert instruction "addi $sp, $fp, 0" at this location.
    if (isInt<12>(-StackSize)) {
      BuildMI(MBB, I, DL, TII.get(ADDI), SP).addReg(FP).addImm(-StackSize);
    } else {
      TII.loadImmediate (SP, -StackSize, MBB, I, DL);
      BuildMI(MBB, I, DL, TII.get(RISCV::ADD), SP).addReg(FP).addReg(SP);
    }
  }

  if (!StackSize)
    return;
  else
    // Adjust stack.
    TII.adjustStackPtr(SP, StackSize, MBB, MBBI);
}

bool RISCVFrameLowering::
spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI,
                          const std::vector<CalleeSavedInfo> &CSI,
                          const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return false;

  MachineFunction &MF = *MBB.getParent();
  MachineBasicBlock *EntryBlock = &(&MF)->front();
  const TargetInstrInfo &TII = *STI.getInstrInfo();

  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    // Add the callee-saved register as live-in. Do not add if the register is
    // RA and return address is taken, because it has already been added in
    // method RISCVTargetLowering::lowerRETURNADDR.
    // It's killed at the spill, unless the register is RA and return address
    // is taken.
    unsigned Reg = CSI[i].getReg();
    bool IsRAAndRetAddrIsTaken = (Reg == RISCV::X1_32 || Reg == RISCV::X1_64)
        && MF.getFrameInfo().isReturnAddressTaken();
    if (!IsRAAndRetAddrIsTaken)
      MBB.addLiveIn(Reg);

    // Insert the spill to the stack frame.
    bool IsKill = !IsRAAndRetAddrIsTaken;
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(*EntryBlock, MI, Reg, IsKill,
                            CSI[i].getFrameIdx(), RC, TRI);
  }

  return true;
}

bool RISCVFrameLowering::
restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            const std::vector<CalleeSavedInfo> &CSI,
                            const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return false;

  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();

  const TargetInstrInfo &TII = *STI.getInstrInfo();

  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    unsigned Reg = CSI[i].getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.loadRegFromStackSlot(MBB, MI, Reg, CSI[i].getFrameIdx(), RC, TRI);
  }

  return true;
}

/// Mark \p Reg and all registers aliasing it in the bitset.
static void setAliasRegs(MachineFunction &MF, BitVector &SavedRegs,
                         unsigned Reg) {
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  for (MCRegAliasIterator AI(Reg, TRI, true); AI.isValid(); ++AI)
    SavedRegs.set(*AI);
}

uint64_t RISCVFrameLowering::estimateStackSize(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo &TRI = *STI.getRegisterInfo();

  int64_t Offset = 0;

  // Iterate over fixed sized objects.
  for (int I = MFI.getObjectIndexBegin(); I != 0; ++I)
    Offset = std::max(Offset, -MFI.getObjectOffset(I));

  // Conservatively assume all callee-saved registers will be saved.
  for (const MCPhysReg *R = TRI.getCalleeSavedRegs(&MF); *R; ++R) {
    unsigned Size = TRI.getSpillSize(*TRI.getMinimalPhysRegClass(*R));
    Offset = alignTo(Offset + Size, Size);
  }

  unsigned MaxAlign = MFI.getMaxAlignment();

  // Check that MaxAlign is not zero if there is a stack object that is not a
  // callee-saved spill.
  assert(!MFI.getObjectIndexEnd() || MaxAlign);

  // Iterate over other objects.
  for (unsigned I = 0, E = MFI.getObjectIndexEnd(); I != E; ++I)
    Offset = alignTo(Offset + MFI.getObjectSize(I), MaxAlign);

  // Call frame.
  if (MFI.adjustsStack() && hasReservedCallFrame(MF))
    Offset = alignTo(Offset + MFI.getMaxCallFrameSize(),
                     std::max(MaxAlign, getStackAlignment()));

  return alignTo(Offset, getStackAlignment());
}

void RISCVFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                              BitVector &SavedRegs,
                                              RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  unsigned FP = STI.isRV64() ? RISCV::X8_64 : RISCV::X8_32;
  unsigned BP = STI.isRV64() ? RISCV::X9_64 : RISCV::X9_32;
  // Mark X8 as used if function has dedicated frame pointer.
  if (hasFP(MF))
    setAliasRegs(MF, SavedRegs, FP);

  // Mark X9 as used if function has dedicated base pointer.
  if (hasBP(MF))
    setAliasRegs(MF, SavedRegs, BP);

  // Set scavenging frame index if necessary.
  uint64_t MaxSPOffset = MF.getInfo<RISCVMachineFunctionInfo>()->getIncomingArgSize() +
    estimateStackSize(MF);

  if (isInt<12>(MaxSPOffset))
    return;

  const TargetRegisterClass RC =
      STI.isRV64() ? RISCV::GPR64RegClass : RISCV::GPRRegClass;
  int FI = MF.getFrameInfo().CreateStackObject(TRI->getSpillSize(RC),
                                               TRI->getSpillAlignment(RC),
                                               false);
  RS->addScavengingFrameIndex(FI);
}

int RISCVFrameLowering::getFrameIndexReference(const MachineFunction &MF,
                                               int FI,
                                               unsigned &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  unsigned SP = STI.isRV64() ? RISCV::X2_64 : RISCV::X2_32;
  unsigned FP = STI.isRV64() ? RISCV::X8_64 : RISCV::X8_32;
  unsigned BP = STI.isRV64() ? RISCV::X9_64 : RISCV::X9_32;

  if (MFI.isFixedObjectIndex(FI))
    FrameReg = hasFP(MF) ? FP : SP;
  else
    FrameReg = hasBP(MF) ? BP : SP;

  return MFI.getObjectOffset(FI) + MFI.getStackSize() -
         getOffsetOfLocalArea() + MFI.getOffsetAdjustment();
}

bool RISCVFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator RISCVFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  unsigned SP = STI.isRV64() ? RISCV::X2_64 : RISCV::X2_32;

  if (!hasReservedCallFrame(MF)) {
    // If we have alloca, convert as follows:
    // ADJCALLSTACKDOWN -> sub, sp, sp, amount
    // ADJCALLSTACKUP   -> add, sp, sp, amount
    //
    // To avoid alloca area poison by outgoing arguments.
    // See the test case gcc.dg/pr31507-1.c.
    int64_t Amount = I->getOperand(0).getImm();

    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      Amount = alignSPAdjust(Amount);

      if (I->getOpcode() == RISCV::ADJCALLSTACKDOWN)
        Amount = -Amount;

      STI.getInstrInfo()->adjustStackPtr(SP, Amount, MBB, I);
    }
  }

  return MBB.erase(I);
}
