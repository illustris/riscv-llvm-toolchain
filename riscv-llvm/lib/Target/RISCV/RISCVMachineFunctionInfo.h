//=== RISCVMachineFunctionInfo.h - RISCV machine function info -*- C++ -*---==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares RISCV-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_RISCV_RISCVMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// RISCVMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private RISCV target-specific information for each MachineFunction.
class RISCVMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

  /// CalleeSavedFrameSize - Size of the callee-saved register portion of the
  /// stack frame in bytes.
  unsigned CalleeSavedFrameSize;

  /// ReturnAddrIndex - FrameIndex for return slot.
  int ReturnAddrIndex;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

  /// True if function has a byval argument.
  bool HasByvalArg;

  /// Size of incoming argument area.
  unsigned IncomingArgSize;

public:
  RISCVMachineFunctionInfo() : CalleeSavedFrameSize(0) {}

  explicit RISCVMachineFunctionInfo(MachineFunction &MF)
    : CalleeSavedFrameSize(0), ReturnAddrIndex(0) {}

  unsigned getCalleeSavedFrameSize() const { return CalleeSavedFrameSize; }
  void setCalleeSavedFrameSize(unsigned bytes) { CalleeSavedFrameSize = bytes; }

  int getRAIndex() const { return ReturnAddrIndex; }
  void setRAIndex(int Index) { ReturnAddrIndex = Index; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex;}
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  void setFormalArgInfo(unsigned Size, bool HasByval) {
    IncomingArgSize = Size;
    HasByvalArg = HasByval;
  }

  unsigned getIncomingArgSize() const { return IncomingArgSize; }
};

} // End llvm namespace

#endif
