//===-- RISCVAnalyzeImmediate.h - Analyze Immediates -----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_RISCV_RISCVANALYZEIMMEDIATE_H
#define LLVM_LIB_TARGET_RISCV_RISCVANALYZEIMMEDIATE_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {

  class RISCVAnalyzeImmediate {
  public:
    struct Inst {
      unsigned Opc, ImmOpnd;
      Inst(unsigned Opc, unsigned ImmOpnd);
    };
    typedef SmallVector<Inst, 10> InstSeq;

    /// Analyze - Get an instruction sequence to load immediate Imm. The last
    /// instruction in the sequence must be an ADDI if LastInstrIsADDI is
    /// true;
    const InstSeq &Analyze(uint64_t Imm, unsigned Size, bool LastInstrIsADDI);
  private:
    typedef SmallVector<InstSeq, 5> InstSeqLs;

    /// AddInstr - Add I to all instruction sequences in SeqLs.
    void AddInstr(InstSeqLs &SeqLs, const Inst &I);

    /// GetInstSeqLsADDI - Get instruction sequences which end with an ADDI to
    /// load immediate Imm
    void GetInstSeqLsADDI(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);

    /// GetInstSeqLsSLL - Get instruction sequences which end with a SLL to
    /// load immediate Imm
    void GetInstSeqLsSLL(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);

    /// GetInstSeqLs - Get instruction sequences to load immediate Imm.
    void GetInstSeqLs(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);

    /// ReplaceADDISLLWithLUI - Replace an ADDI & SLL pair with a LUI.
    void ReplaceADDISLLWithLUI(InstSeq &Seq);

    /// GetShortestSeq - Find the shortest instruction sequence in SeqLs and
    /// return it in Insts.
    void GetShortestSeq(InstSeqLs &SeqLs, InstSeq &Insts);

    unsigned Size;
    unsigned ADDI, ORI, SLL, LUI;
    InstSeq Insts;
  };
}

#endif
