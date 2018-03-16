//===-- RISCVCallingConv.h - Calling conventions for SystemZ --*- C++ ---*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVCALLINGCONV_H
#define LLVM_LIB_TARGET_RISCV_RISCVCALLINGCONV_H

#include "RISCVSubtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {
namespace RISCV {
  const unsigned NumArgRV64GPRs = 8;
  extern const MCPhysReg RV64ArgGPRs[NumArgRV64GPRs];
  const unsigned NumArgRV32GPRs = 8;
  extern const MCPhysReg RV32ArgGPRs[NumArgRV32GPRs];
  const unsigned NumArgRV32EGPRs = 6;
  extern const MCPhysReg RV32EArgGPRs[NumArgRV32EGPRs];
} // end namespace RISCV

inline ArrayRef<MCPhysReg> getArgRegs(const RISCVSubtarget *STI) {
  if (STI->isRV64())
    return makeArrayRef(RISCV::RV64ArgGPRs);
  if (STI->hasE())
    return makeArrayRef(RISCV::RV32EArgGPRs);

  return makeArrayRef(RISCV::RV32ArgGPRs);
}

// Handle i128 argument types.  These need to be passed by implicit
// reference.
inline bool CC_RISCV_I128Indirect(unsigned &ValNo, MVT &ValVT,
                                  MVT &LocVT,
                                  CCValAssign::LocInfo &LocInfo,
                                  ISD::ArgFlagsTy &ArgFlags,
                                  CCState &State) {
  unsigned Reg, Offset;
  SmallVectorImpl<CCValAssign> &PendingMembers = State.getPendingLocs();
  const RISCVSubtarget* Subtarget =
    &static_cast<const RISCVSubtarget&>(State.getMachineFunction().
                                              getSubtarget());

  ArrayRef<MCPhysReg> ArgRegs = getArgRegs(Subtarget);

  // ArgFlags.isSplit() is true on the first part of a i128 argument;
  // PendingMembers.empty() is false on all subsequent parts.
  if (!ArgFlags.isSplit() && PendingMembers.empty())
    return false;

  // Push a pending Indirect value location for each part.
  LocVT = MVT::i32;
  LocInfo = CCValAssign::Indirect;
  PendingMembers.push_back(CCValAssign::getPending(ValNo, ValVT,
                                                   LocVT, LocInfo));
  if (!ArgFlags.isSplitEnd())
    return true;

  // According to RISCV32 ABI, argument size <= 2xXLEN should pass
  // by register or memory directly.
  if (ArgFlags.isSplitEnd() && PendingMembers.size() <= 2) {
    assert(PendingMembers.size() == 2);
    CCValAssign VA0 = PendingMembers[0];
    CCValAssign VA1 = PendingMembers[1];
    PendingMembers.clear();
    LocInfo = CCValAssign::Full;
    PendingMembers.push_back(CCValAssign::getPending(VA0.getValNo(),
                                                     VA0.getValVT(),
                                                     VA0.getLocVT(),
                                                     LocInfo));
    PendingMembers.push_back(CCValAssign::getPending(VA1.getValNo(),
                                                     VA1.getValVT(),
                                                     VA1.getLocVT(),
                                                     LocInfo));
    for (auto &It : PendingMembers) {
      Reg = State.AllocateReg(ArgRegs);
      Offset = Reg ? 0 : State.AllocateStack(4, 4);
      if (Reg)
        It.convertToReg(Reg);
      else
        It.convertToMem(Offset);
      State.addLoc(It);
    }

    PendingMembers.clear();
    return true;
  }

  // OK, we've collected all parts in the pending list.  Allocate
  // the location (register or stack slot) for the indirect pointer.
  Reg = State.AllocateReg(ArgRegs);
  Offset = Reg ? 0 : State.AllocateStack(4, 4);

  // Use that same location for all the pending parts.
  for (auto &It : PendingMembers) {
    if (Reg)
      It.convertToReg(Reg);
    else
      It.convertToMem(Offset);
    State.addLoc(It);
  }

  PendingMembers.clear();

  return true;
}

} // end namespace llvm

#endif
