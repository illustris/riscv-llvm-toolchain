//===-- RISCVAnalyzeImmediate.cpp - Analyze Immediates --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "RISCVAnalyzeImmediate.h"
#include "RISCV.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

RISCVAnalyzeImmediate::Inst::Inst(unsigned O, unsigned I) : Opc(O), ImmOpnd(I) {}

// Add I to the instruction sequences.
void RISCVAnalyzeImmediate::AddInstr(InstSeqLs &SeqLs, const Inst &I) {
  // Add an instruction seqeunce consisting of just I.
  if (SeqLs.empty()) {
    SeqLs.push_back(InstSeq(1, I));
    return;
  }

  for (InstSeqLs::iterator Iter = SeqLs.begin(); Iter != SeqLs.end(); ++Iter)
    Iter->push_back(I);
}

void RISCVAnalyzeImmediate::GetInstSeqLsADDI(uint64_t Imm, unsigned RemSize,
                                             InstSeqLs &SeqLs) {
  GetInstSeqLs((Imm + 0x800ULL) & 0xfffffffffffff000ULL, RemSize, SeqLs);
  AddInstr(SeqLs, Inst(ADDI, Imm & 0xfffULL));
}

void RISCVAnalyzeImmediate::GetInstSeqLsSLL(uint64_t Imm, unsigned RemSize,
                                            InstSeqLs &SeqLs) {
  unsigned Shamt = countTrailingZeros(Imm);
  GetInstSeqLs(Imm >> Shamt, RemSize - Shamt, SeqLs);
  AddInstr(SeqLs, Inst(SLL, Shamt));
}

void RISCVAnalyzeImmediate::GetInstSeqLs(uint64_t Imm, unsigned RemSize,
                                        InstSeqLs &SeqLs) {
  uint64_t MaskedImm = Imm & (0xffffffffffffffffULL >> (64 - Size));

  // Do nothing if Imm is 0.
  if (!MaskedImm)
    return;

  // A single ADDI will do if RemSize <= 12.
  if (RemSize <= 12) {
    AddInstr(SeqLs, Inst(ADDI, MaskedImm));
    return;
  }

  // Shift if the lower 12-bit is cleared.
  if (!(Imm & 0xfff)) {
    GetInstSeqLsSLL(Imm, RemSize, SeqLs);
    return;
  }

  GetInstSeqLsADDI(Imm, RemSize, SeqLs);
}

// Replace ADDI & SLL with a LUI.
void RISCVAnalyzeImmediate::ReplaceADDISLLWithLUI(InstSeq &Seq) {
  // Check if the first two instructions are ADDI and SLL and the shift amount
  // is at least 12.
  if ((Seq.size() < 2) || (Seq[0].Opc != ADDI) ||
      (Seq[1].Opc != SLL) || (Seq[1].ImmOpnd < 12))
    return;

  // Sign-extend and shift operand of ADDI.
  int64_t Imm = SignExtend64<12>(Seq[0].ImmOpnd);
  int64_t LUIImm = (uint64_t)Imm << (Seq[1].ImmOpnd - 12);

  if (!isInt<20>(LUIImm))
    return;

  // Replace the first instruction and erase the second.
  // In this point we transfer ADDI/SLLI pair to LUI
  Seq[0].Opc = LUI;
  Seq[0].ImmOpnd = (unsigned)(LUIImm & 0xfffff);
  Seq.erase(Seq.begin() + 1);

  // Try to transfer LUI/ADDI/SLLI to LUI
  // LUI   a1, 11
  // ADDI  a1, a1, -1365
  // SLLI  a1, a1, 12
  // are replaced with
  // LUI   a1, 0xaaab
  if ((Seq.size() > 2) && (Seq[0].Opc == LUI) &&
      (Seq[1].Opc == ADDI) && (Seq[2].Opc == SLL) &&
      (Seq[2].ImmOpnd >= 12)) {

    // Sign-extend and shift operand of ADDI.
    Imm = SignExtend64<12>(Seq[1].ImmOpnd);
    int64_t ADDIresult = (LUIImm << 12) + Imm;
    LUIImm = ((uint64_t)ADDIresult << (Seq[2].ImmOpnd - 12));

    if (!isInt<20>(LUIImm))
      return;

    Seq[0].ImmOpnd = (unsigned)(LUIImm & 0xfffff);
    Seq.erase(Seq.begin() + 1);
    Seq.erase(Seq.begin() + 1);
  }
}

void RISCVAnalyzeImmediate::GetShortestSeq(InstSeqLs &SeqLs, InstSeq &Insts) {
  InstSeqLs::iterator ShortestSeq = SeqLs.end();
  // The length of an instruction sequence is at most 10.
  unsigned ShortestLength = 10;

  InstSeqLs::iterator S = SeqLs.begin();
  ReplaceADDISLLWithLUI(*S);
  for (; S != SeqLs.end(); ++S) {
    assert(S->size() <= 10);

    if (S->size() < ShortestLength) {
      ShortestSeq = S;
      ShortestLength = S->size();
    }
  }

  Insts.clear();
  Insts.append(ShortestSeq->begin(), ShortestSeq->end());
}

const RISCVAnalyzeImmediate::InstSeq
&RISCVAnalyzeImmediate::Analyze(uint64_t Imm, unsigned Size,
                                bool LastInstrIsADDI) {
  this->Size = Size;

  if (Size == 32) {
    ADDI = RISCV::ADDI;
    ORI = RISCV::ORI;
    SLL = RISCV::SLLI;
    LUI = RISCV::LUI;
  } else {
    ADDI = RISCV::ADDI64;
    ORI = RISCV::ORI64;
    SLL = RISCV::SLLI64;
    LUI = RISCV::LUI64;
  }

  InstSeqLs SeqLs;

  // Get the list of instruction sequences.
  GetInstSeqLs(Imm, Size, SeqLs);

  // Set Insts to the shortest instruction sequence.
  GetShortestSeq(SeqLs, Insts);

  return Insts;
}
