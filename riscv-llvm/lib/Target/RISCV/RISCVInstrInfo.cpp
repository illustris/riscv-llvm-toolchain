//===-- RISCVInstrInfo.cpp - RISCV Instruction Information ------*- C++ -*-===//
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

#include "RISCVInstrInfo.h"
#include "RISCV.h"
#include "RISCVSubtarget.h"
#include "RISCVTargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "RISCVGenInstrInfo.inc"

using namespace llvm;

RISCVInstrInfo::RISCVInstrInfo(RISCVSubtarget &sti)
  : RISCVGenInstrInfo(RISCV::ADJCALLSTACKDOWN, RISCV::ADJCALLSTACKUP),
    RI(sti), STI(sti) {
  if (sti.isRV32()) {
    UncondBranch = RISCV::PseudoBR;
    CADDI16SP = RISCV::CADDI16SP;
    CADDI = RISCV::CADDI;
    CLUI = RISCV::CLUI;
    ADDI = RISCV::ADDI;
    ADD = RISCV::ADD;
    LUI = RISCV::LUI;
    CMV = RISCV::CMV;
    CLI = RISCV::CLI;
  } else {
    UncondBranch = RISCV::PseudoBR64;
    CADDI16SP = RISCV::CADDI16SP64;
    CADDI = RISCV::CADDI64;
    CLUI = RISCV::CLUI64;
    ADDI = RISCV::ADDI64;
    ADD = RISCV::ADD64;
    LUI = RISCV::LUI64;
    CMV = RISCV::CMV64;
    CLI = RISCV::CLI64;
  }

  if (sti.hasC())
    UncondBranch = RISCV::CJ;
}

void RISCVInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator Position,
                                 const DebugLoc &DL,
                                 unsigned DestinationRegister,
                                 unsigned SourceRegister,
                                 bool KillSource) const {
  if (STI.isRV64() && RISCV::GPRRegClass.contains(DestinationRegister)) {
    BuildMI(MBB, Position, DL, get(RISCV::ADDIW), DestinationRegister)
      .addReg(SourceRegister, getKillRegState(KillSource))
      .addImm(0);
    return;
  }

  if (STI.hasC()) {
    BuildMI(MBB, Position, DL, get(CMV), DestinationRegister)
      .addReg(SourceRegister, getKillRegState(KillSource));
  } else {
    BuildMI(MBB, Position, DL, get(ADDI), DestinationRegister)
      .addReg(SourceRegister, getKillRegState(KillSource))
      .addImm(0);
  }
}

void RISCVInstrInfo::getLoadStoreOpcodes(const TargetRegisterClass *RC,
                                         unsigned &LoadOpcode,
                                         unsigned &StoreOpcode) const {
  if (RC == &RISCV::GPRRegClass || RC == &RISCV::GPRCRegClass) {
    LoadOpcode = RISCV::LW;
    StoreOpcode = RISCV::SW;
  } else if (RC == &RISCV::GPR64RegClass ||
             RC == &RISCV::GPR64CRegClass) {
    LoadOpcode = RISCV::LD;
    StoreOpcode = RISCV::SD;
  } else
    llvm_unreachable("Unsupported regclass to load or store");
}


void RISCVInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator I,
                                         unsigned SrcReg, bool IsKill, int FI,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  unsigned LoadOpcode, StoreOpcode;
  getLoadStoreOpcodes(RC, LoadOpcode, StoreOpcode);

  BuildMI(MBB, I, DL, get(StoreOpcode))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FI)
      .addImm(0);
}

void RISCVInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          unsigned DestReg, int FI,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  unsigned LoadOpcode, StoreOpcode;
  getLoadStoreOpcodes(RC, LoadOpcode, StoreOpcode);

  BuildMI(MBB, I, DL, get(LoadOpcode), DestReg).addFrameIndex(FI).addImm(0);
}

/// Adjust SP by Amount bytes.
void RISCVInstrInfo::adjustStackPtr(unsigned SP, int64_t Amount,
                                    MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator I) const {
  DebugLoc DL;
  if (Amount == 0)
    return;

  if (isInt<12>(Amount)) {
    if (isInt<10>(Amount) && ((Amount % 16) == 0) && STI.hasC()) {
      // c.addi16sp amount
      BuildMI(MBB, I, DL, get(CADDI16SP), SP).
        addReg(SP).addImm(Amount);
    } else if (isInt<6>(Amount) && STI.hasC()) {
      // c.addi sp amount
      BuildMI(MBB, I, DL, get(CADDI), SP).
        addReg(SP).addImm(Amount);
    } else {
      // addi sp, sp, amount
      BuildMI(MBB, I, DL, get(ADDI), SP).
        addReg(SP).addImm(Amount);
    }
  } else {
    basePlusImmediate (SP, SP, Amount, MBB, I, DL);
  }
}

void RISCVInstrInfo::loadImmediate(unsigned ScratchReg,
                                   int64_t Imm, MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator II,
                                   const DebugLoc &DL) const {
  MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
  const TargetRegisterClass *RC = STI.isRV64() ?
    &RISCV::GPR64RegClass : &RISCV::GPRRegClass;
  unsigned ZeroReg = STI.isRV64() ? RISCV::X0_64 : RISCV::X0_32;

  if (Imm == 0)
    return;

  int64_t LuiImm = ((Imm + 0x800) >> 12) & 0xfffff;
  int64_t LowImm = SignExtend64<12>(Imm);
  unsigned LuiOpcode = LUI;

  if (isUInt<5>(LuiImm) && STI.hasC())
    LuiOpcode = CLUI;

  if ((LuiImm != 0) && (LowImm == 0)) {
    BuildMI(MBB, II, DL, get(LuiOpcode), ScratchReg)
      .addImm(LuiImm);
  } else if ((LuiImm == 0) && (LowImm != 0)) {
    if (isInt<6>(LowImm) && STI.hasC()) {
      // c.li
      BuildMI(MBB, II, DL, get(CLI), ScratchReg)
        .addImm(LowImm);
    } else {
      BuildMI(MBB, II, DL, get(ADDI), ScratchReg)
        .addReg(ZeroReg)
        .addImm(LowImm);
    }
  } else {
    // Create TempReg here because Virtual register expect as SSA form.
    // So ADDI ScratchReg, ScratchReg, Imm is not allow.
    unsigned TempReg = RegInfo.createVirtualRegister(RC);

    BuildMI(MBB, II, DL, get(LuiOpcode), TempReg)
      .addImm(LuiImm);

    BuildMI(MBB, II, DL, get(ADDI), ScratchReg)
      .addReg(TempReg, getKillRegState (true))
      .addImm(LowImm);
  }
}

unsigned
RISCVInstrInfo::basePlusImmediate(unsigned DstReg, unsigned BaseReg,
                                  int64_t Imm, MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator II,
                                  const DebugLoc &DL) const {
  MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
  const TargetRegisterClass *RC = STI.isRV64() ?
    &RISCV::GPR64RegClass : &RISCV::GPRRegClass;
  unsigned ScratchReg = RegInfo.createVirtualRegister(RC);

  if (DstReg == 0)
    DstReg = ScratchReg;

  loadImmediate (ScratchReg, Imm, MBB, II, DL);
  BuildMI(MBB, II, DL, get(ADD), DstReg)
    .addReg(ScratchReg)
    .addReg(BaseReg);

  return ScratchReg;
}

unsigned
RISCVInstrInfo::basePlusImmediateStripOffset(unsigned BaseReg, int64_t &Imm,
                                             MachineBasicBlock &MBB,
                                             MachineBasicBlock::iterator II,
                                             const DebugLoc &DL) const {
  MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
  const TargetRegisterClass *RC = STI.isRV64() ?
    &RISCV::GPR64RegClass : &RISCV::GPRRegClass;
  unsigned ScratchReg1 = RegInfo.createVirtualRegister(RC);
  unsigned ScratchReg2 = RegInfo.createVirtualRegister(RC);

  int64_t LuiImm = ((Imm + 0x800) >> 12) & 0xfffff;
  unsigned LuiOpcode = LUI;

  if (isUInt<5>(LuiImm) && STI.hasC())
    LuiOpcode = CLUI;

  BuildMI(MBB, II, DL, get(LuiOpcode), ScratchReg1)
    .addImm(LuiImm);

  BuildMI(MBB, II, DL, get(ADD), ScratchReg2)
    .addReg(ScratchReg1, getKillRegState (true))
    .addReg(BaseReg);

  Imm = SignExtend64<12>(Imm);

  return ScratchReg2;
}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//

static bool isUncondBranch(unsigned Opcode) {
  if (Opcode == RISCV::PseudoBR ||
      Opcode == RISCV::PseudoBR64 ||
      Opcode == RISCV::CJ ||
      Opcode == RISCV::CJ64)
    return true;
  return false;
}

static bool isIndirectBranch(unsigned Opcode) {
  if (Opcode == RISCV::PseudoBRIND ||
      Opcode == RISCV::PseudoBRIND64 ||
      Opcode == RISCV::CJR ||
      Opcode == RISCV::CJR64)
    return true;
  return false;
}

static bool isCondBranch(unsigned Opcode) {

  if (Opcode == RISCV::BEQ ||
      Opcode == RISCV::BNE ||
      Opcode == RISCV::BLT ||
      Opcode == RISCV::BGE ||
      Opcode == RISCV::BLTU ||
      Opcode == RISCV::BGEU ||
      Opcode == RISCV::BEQ64 ||
      Opcode == RISCV::BNE64 ||
      Opcode == RISCV::BLT64 ||
      Opcode == RISCV::BGE64 ||
      Opcode == RISCV::BLTU64 ||
      Opcode == RISCV::BGEU64 ||
      Opcode == RISCV::CBEQZ ||
      Opcode == RISCV::CBNEZ ||
      Opcode == RISCV::CBEQZ64 ||
      Opcode == RISCV::CBNEZ64)
    return true;
  return false;
}

void RISCVInstrInfo::BuildCondBr(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                                 const DebugLoc &DL,
                                 ArrayRef<MachineOperand> Cond) const {
  unsigned Opc = Cond[0].getImm();
  const MCInstrDesc &MCID = get(Opc);
  MachineInstrBuilder MIB = BuildMI(&MBB, DL, MCID);

  for (unsigned I = 1; I < Cond.size(); ++I) {
    if (Cond[I].isReg())
      MIB.addReg(Cond[I].getReg());
    else if (Cond[I].isImm())
      MIB.addImm(Cond[I].getImm());
    else
       assert(false && "Cannot copy operand");
  }
  MIB.addMBB(TBB);
}

unsigned RISCVInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                      MachineBasicBlock *TBB,
                                      MachineBasicBlock *FBB,
                                      ArrayRef<MachineOperand> Cond,
                                      const DebugLoc &DL,
                                      int *BytesAdded) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");

  // Two-way Conditional branch.
  if (FBB) {
    BuildCondBr(MBB, TBB, DL, Cond);
    BuildMI(&MBB, DL, get(UncondBranch)).addMBB(FBB);

    if (BytesAdded)
      *BytesAdded = 8;

    return 2;
  }

  // One way branch.
  // Unconditional branch.
  if (Cond.empty())
    BuildMI(&MBB, DL, get(UncondBranch)).addMBB(TBB);
  else // Conditional branch.
    BuildCondBr(MBB, TBB, DL, Cond);

  if (BytesAdded)
    *BytesAdded = 4;

  return 1;
}

// removeBranch will remove last branch or last two
// conditional and unconditional branches which let BranchFolding
// could do the right thing.
unsigned RISCVInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                      int *BytesRemoved) const {
  int RemovedSize = 0;
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();

  if (I == MBB.end())
    return 0;

  if (!isUncondBranch(I->getOpcode()) &&
      !isCondBranch(I->getOpcode()))
    return 0;

  RemovedSize += getInstSizeInBytes(*I);
  // Remove the branch.
  I->eraseFromParent();

  I = MBB.end();

  if (I == MBB.begin()) {
    if (BytesRemoved)
      *BytesRemoved = RemovedSize;
    return 1;
  }
  --I;
  if (!isCondBranch(I->getOpcode())) {
    if (BytesRemoved)
      *BytesRemoved = RemovedSize;
    return 1;
  }

  RemovedSize += getInstSizeInBytes(*I);
  // Remove the branch.
  I->eraseFromParent();
  if (BytesRemoved)
    *BytesRemoved = RemovedSize;

  return 2;
}

void RISCVInstrInfo::AnalyzeCondBr(const MachineInstr *Inst, unsigned Opc,
                                   MachineBasicBlock *&BB,
                                   SmallVectorImpl<MachineOperand> &Cond) const {
  int NumOp = Inst->getNumExplicitOperands();

  // for both int and fp branches, the last explicit operand is the
  // MBB.
  BB = Inst->getOperand(NumOp-1).getMBB();
  Cond.push_back(MachineOperand::CreateImm(Opc));

  for (int i=0; i<NumOp-1; i++)
    Cond.push_back(Inst->getOperand(i));
}

bool RISCVInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const {
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(*I))
      break;

    // A terminator that isn't a branch can't easily be handled
    // by this analysis.
    if (!I->isBranch())
      return true;

    // Cannot handle indirect branches.
    if (isIndirectBranch (I->getOpcode()))
      return true;

    // Handle unconditional branches.
    if (isUncondBranch (I->getOpcode())) {
      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a J, delete them.
      while (std::next(I) != MBB.end())
        std::next(I)->eraseFromParent();
      Cond.clear();
      FBB = nullptr;

      // Delete the J if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
        TBB = nullptr;
        I->eraseFromParent();
        I = MBB.end();
        continue;
      }

      // TBB is used to indicate the unconditinal destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }

    // Handle conditional branches.
    if (isCondBranch (I->getOpcode())) {
      // Working from the bottom, handle the first conditional branch.
      if (!Cond.empty())
        return true;

      FBB = TBB;
      MachineInstr *LastInst = &*I;
      AnalyzeCondBr(LastInst, I->getOpcode(), TBB, Cond);
      return false;
    }

    return true;
  }

  return false;
}

/// getOppositeBranchOpc - Return the inverse of the specified
/// opcode, e.g. turning BEQ to BNE.
unsigned RISCVInstrInfo::getOppositeBranchOpc(unsigned Opc) const {
  switch (Opc) {
  default:             llvm_unreachable("Illegal opcode!");
  case RISCV::BEQ:     return RISCV::BNE;
  case RISCV::BNE:     return RISCV::BEQ;
  case RISCV::BLT:     return RISCV::BGE;
  case RISCV::BGE:     return RISCV::BLT;
  case RISCV::BLTU:    return RISCV::BGEU;
  case RISCV::BGEU:    return RISCV::BLTU;
  case RISCV::BEQ64:   return RISCV::BNE64;
  case RISCV::BNE64:   return RISCV::BEQ64;
  case RISCV::BLT64:   return RISCV::BGE64;
  case RISCV::BGE64:   return RISCV::BLT64;
  case RISCV::BLTU64:  return RISCV::BGEU64;
  case RISCV::BGEU64:  return RISCV::BLTU64;
  case RISCV::CBEQZ:   return RISCV::CBNEZ;
  case RISCV::CBNEZ:   return RISCV::CBEQZ;
  case RISCV::CBEQZ64: return RISCV::CBNEZ64;
  case RISCV::CBNEZ64: return RISCV::CBEQZ64;
  }
}

bool RISCVInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert( (Cond.size() && Cond.size() <= 3) &&
          "Invalid RISCV branch condition!");
  Cond[0].setImm(getOppositeBranchOpc(Cond[0].getImm()));
  return false;
}

static unsigned getBranchDisplacementBits(unsigned Opc) {
  switch (Opc) {
    default:
    llvm_unreachable("unexpected opcode!");
  case RISCV::CBEQZ:
  case RISCV::CBNEZ:
  case RISCV::CBEQZ64:
  case RISCV::CBNEZ64:
    return 9;
  case RISCV::CJ:
  case RISCV::CJ64:
    return 12;
  case RISCV::BEQ:
  case RISCV::BNE:
  case RISCV::BLT:
  case RISCV::BGE:
  case RISCV::BLTU:
  case RISCV::BGEU:
  case RISCV::BEQ64:
  case RISCV::BNE64:
  case RISCV::BLT64:
  case RISCV::BGE64:
  case RISCV::BLTU64:
  case RISCV::BGEU64:
    return 13;
  case RISCV::JAL:
  case RISCV::JAL64:
  case RISCV::PseudoBR:
  case RISCV::PseudoBR64:
    return 21;
  }
}

bool RISCVInstrInfo::isBranchOffsetInRange(unsigned BranchOp,
                                           int64_t BrOffset) const {
  unsigned Bits = getBranchDisplacementBits(BranchOp);
  return isIntN(Bits, BrOffset);
}

MachineBasicBlock *RISCVInstrInfo::getBranchDestBlock(
  const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected opcode!");
  case RISCV::CJ:
  case RISCV::CJ64:
  case RISCV::PseudoBR:
  case RISCV::PseudoBR64:
    return MI.getOperand(0).getMBB();
  case RISCV::JAL:
  case RISCV::JAL64:
  case RISCV::CBEQZ:
  case RISCV::CBNEZ:
  case RISCV::CBEQZ64:
  case RISCV::CBNEZ64:
    return MI.getOperand(1).getMBB();
  case RISCV::BEQ:
  case RISCV::BNE:
  case RISCV::BLT:
  case RISCV::BGE:
  case RISCV::BLTU:
  case RISCV::BGEU:
  case RISCV::BEQ64:
  case RISCV::BNE64:
  case RISCV::BLT64:
  case RISCV::BGE64:
  case RISCV::BLTU64:
  case RISCV::BGEU64:
    return MI.getOperand(2).getMBB();
  }
}

unsigned RISCVInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  unsigned NumBytes = 0;
  const MCInstrDesc &Desc = MI.getDesc();

  switch (Desc.getOpcode()) {
  default:
    NumBytes = Desc.Size;
    break;
  case RISCV::PseudoRET:
  case RISCV::PseudoRET64:
  case RISCV::PseudoCALL:
  case RISCV::PseudoCALL64:
  case RISCV::PseudoBR:
  case RISCV::PseudoBR64:
  case RISCV::PseudoBRIND:
  case RISCV::PseudoBRIND64:
    NumBytes = 4;
    break;
  }

  return NumBytes;
}

unsigned RISCVInstrInfo::insertIndirectBranch(MachineBasicBlock &MBB,
                                              MachineBasicBlock &DestBB,
                                              const DebugLoc &DL,
                                              int64_t BrOffset,
                                              RegScavenger *RS) const {
  // Relax CJ to JAL instruction
  if (isInt<21>(BrOffset)) {
    unsigned Br = STI.isRV64() ? RISCV::PseudoBR64 : RISCV::PseudoBR;
    BuildMI(&MBB, DL, get(Br)).addMBB(&DestBB);
    return 4;
  }
  llvm_unreachable("Not support relax to indirect branch yet");
}
