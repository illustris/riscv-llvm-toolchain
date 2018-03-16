//===-- RISCVMCCodeEmitter.cpp - Convert RISCV code to machine code -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the RISCVMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/RISCVBaseInfo.h"
#include "MCTargetDesc/RISCVFixupKinds.h"
#include "MCTargetDesc/RISCVMCExpr.h"
#include "MCTargetDesc/RISCVMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of MC fixups created.");

namespace {
class RISCVMCCodeEmitter : public MCCodeEmitter {
  RISCVMCCodeEmitter(const RISCVMCCodeEmitter &) = delete;
  void operator=(const RISCVMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  RISCVMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  ~RISCVMCCodeEmitter() override {}

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  void EmitByte(unsigned char C, raw_ostream &OS) const;

  void EmitInstruction(uint64_t Val, unsigned Size, const MCSubtargetInfo &STI,
                       raw_ostream &OS) const;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;

  unsigned getExprOpValue(const MCInst &MI, const MCExpr *Expr,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  bool isLoad(unsigned int Opc) const;

  bool useGPRC(unsigned int Opc) const;

  unsigned getAddrRegImmEncoding(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getAddrSpImm6uWordEncoding(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const;

  unsigned getAddrSpImm6uDoubleEncoding(const MCInst &MI, unsigned OpNo,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const;

  unsigned getAddrRegImm5uWordEncoding(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const;

  unsigned getAddrRegImm5uDoubleEncoding(const MCInst &MI, unsigned OpNo,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const;
};
} // end anonymous namespace

MCCodeEmitter *llvm::createRISCVMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new RISCVMCCodeEmitter(Ctx, MCII);
}

void RISCVMCCodeEmitter::EmitByte(unsigned char C, raw_ostream &OS) const {
  OS << (char)C;
}

void RISCVMCCodeEmitter::EmitInstruction(uint64_t Val, unsigned Size,
                                         const MCSubtargetInfo &STI,
                                         raw_ostream &OS) const {
  // Output the instruction encoding in big endian byte order.
  for (unsigned i = 0; i < Size; ++i) {
    unsigned Shift = i * 8;
    EmitByte((Val >> Shift) & 0xff, OS);
  }
}

void RISCVMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  MCInst TmpInst = MI;

  uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);

  const MCInstrDesc &Desc = MCII.get(TmpInst.getOpcode());

  // Get byte count of instruction
  unsigned Size = Desc.getSize();
  if (!Size)
    llvm_unreachable("Desc.getSize() returns 0");

  EmitInstruction(Binary, Size, STI, OS);

  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned
RISCVMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {

  if (MO.isReg()) {
    if (useGPRC(MI.getOpcode()))
      return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg()) - 8;

    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  }

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  // MO must be an Expr.
  assert(MO.isExpr());
  return getExprOpValue(MI, MO.getExpr(),Fixups, STI);
}

unsigned
RISCVMCCodeEmitter::getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm()) {
    unsigned Res = MO.getImm();
    assert((Res & 1) == 0 && "LSB is non-zero");
    return Res >> 1;
  }

  return getImmOpValue(MI, OpNo, Fixups, STI);
}

unsigned RISCVMCCodeEmitter::getImmOpValue(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {

  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, there is nothing to do
  if (MO.isImm())
    return MO.getImm();

  // MO must be an Expr.
  assert(MO.isExpr());
  return getExprOpValue(MI, MO.getExpr(),Fixups, STI);
}

unsigned RISCVMCCodeEmitter::
getExprOpValue(const MCInst &MI, const MCExpr *Expr,
               SmallVectorImpl<MCFixup> &Fixups,
               const MCSubtargetInfo &STI) const {
  MCInstrDesc const &Desc = MCII.get(MI.getOpcode());
  unsigned MIFrm = Desc.TSFlags & RISCVII::FormMask;
  MCExpr::ExprKind Kind = Expr->getKind();
  RISCV::Fixups FixupKind;
  if (Kind == MCExpr::Target) {
    const RISCVMCExpr *RVExpr = cast<RISCVMCExpr>(Expr);

    switch (RVExpr->getKind()) {
    case RISCVMCExpr::VK_RISCV_None:
    case RISCVMCExpr::VK_RISCV_Invalid:
      llvm_unreachable("Unhandled fixup kind!");
      break;
    case RISCVMCExpr::VK_RISCV_LO:
      FixupKind = MIFrm == RISCVII::FrmI ? RISCV::fixup_riscv_lo12_i
                                         : RISCV::fixup_riscv_lo12_s;
      break;
    case RISCVMCExpr::VK_RISCV_HI:
      FixupKind = RISCV::fixup_riscv_hi20;
      break;
    case RISCVMCExpr::VK_RISCV_PCREL_HI:
      FixupKind = RISCV::fixup_riscv_pcrel_hi20;
      break;
    }
  } else if (Kind == MCExpr::SymbolRef &&
             cast<MCSymbolRefExpr>(Expr)->getKind() == MCSymbolRefExpr::VK_None) {
    if (Desc.getOpcode() == RISCV::JAL || Desc.getOpcode() == RISCV::JAL64) {
      FixupKind = RISCV::fixup_riscv_jal;
    } else if (MIFrm == RISCVII::FrmSB) {
      FixupKind = RISCV::fixup_riscv_branch;
    } else if (Desc.getOpcode() == RISCV::CJ ||
               Desc.getOpcode() == RISCV::CJ64 ||
               Desc.getOpcode() == RISCV::CJAL) {
      FixupKind = RISCV::fixup_riscv_rvc_jump;
    } else if (Desc.getOpcode() == RISCV::CBEQZ ||
               Desc.getOpcode() == RISCV::CBNEZ ||
               Desc.getOpcode() == RISCV::CBEQZ64 ||
               Desc.getOpcode() == RISCV::CBNEZ64) {
      FixupKind = RISCV::fixup_riscv_rvc_branch;
    } else {
      llvm_unreachable("Unhandled expression!");
    }
  } else {
    llvm_unreachable("Unhandled expression!");
  }
  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  ++MCNumFixups;

  return 0;
}

bool RISCVMCCodeEmitter::isLoad(unsigned Opc) const {
  if (Opc == RISCV::LB  || Opc == RISCV::LB64  ||
      Opc == RISCV::LBU || Opc == RISCV::LBU64 ||
      Opc == RISCV::LD  || Opc == RISCV::LH    ||
      Opc == RISCV::LHU || Opc == RISCV::LHU64 ||
      Opc == RISCV::LW  || Opc == RISCV::LW64 ||
      Opc == RISCV::LWU || Opc == RISCV::LH64)
    return true;
  return false;
}

bool RISCVMCCodeEmitter::useGPRC(unsigned Opc) const {
  if (Opc == RISCV::CLW       || Opc == RISCV::CSW ||
      Opc == RISCV::CBEQZ     || Opc == RISCV::CBNEZ ||
      Opc == RISCV::CADDI4SPN || Opc == RISCV::CSRLI ||
      Opc == RISCV::CSRAI     || Opc == RISCV::CANDI ||
      Opc == RISCV::CAND      || Opc == RISCV::COR ||
      Opc == RISCV::CSUB      || Opc == RISCV::CXOR ||
      Opc == RISCV::CLW64     || Opc == RISCV::CLD ||
      Opc == RISCV::CSW64     || Opc == RISCV::CSD ||
      Opc == RISCV::CSRLI64   || Opc == RISCV::CSRAI64 ||
      Opc == RISCV::CBEQZ64   || Opc == RISCV::CBNEZ64 ||
      Opc == RISCV::CSUBW     || Opc == RISCV::CADDW ||
      Opc == RISCV::CAND64    || Opc == RISCV::COR64 ||
      Opc == RISCV::CSUB64    || Opc == RISCV::CXOR64 ||
      Opc == RISCV::CANDI64   || Opc == RISCV::CADDI4SPN64)
    return true;
  return false;
}

// Encoding Reg + Imm addressing mode
unsigned
RISCVMCCodeEmitter::getAddrRegImmEncoding(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  const MCOperand &MO1 = MI.getOperand(OpNo + 1);
  unsigned Imm12 = getMachineOpValue(MI, MO1, Fixups, STI);

  // It's a load instruction.
  if (isLoad(MI.getOpcode())) {
    return getMachineOpValue(MI, MO,  Fixups, STI) | Imm12 << 5;
  // It's a store instruction.
  } else {
    return getMachineOpValue(MI, MO,  Fixups, STI) << 5 |
           (Imm12 & 0x1f) | (Imm12 >> 5) << 10;
  }
}

// Encoding Sp + Imm6u << 2 addressing mode
unsigned
RISCVMCCodeEmitter::getAddrSpImm6uWordEncoding(const MCInst &MI, unsigned OpNo,
                                               SmallVectorImpl<MCFixup> &Fixups,
                                               const MCSubtargetInfo &STI) const {
  const MCOperand &MO1 = MI.getOperand(OpNo + 1);
  unsigned Imm = getMachineOpValue(MI, MO1, Fixups, STI);

  return Imm;
}

// Encoding Sp + Imm6u << 3 addressing mode
unsigned
RISCVMCCodeEmitter::getAddrSpImm6uDoubleEncoding(const MCInst &MI, unsigned OpNo,
                                                 SmallVectorImpl<MCFixup> &Fixups,
                                                 const MCSubtargetInfo &STI) const {
  const MCOperand &MO1 = MI.getOperand(OpNo + 1);
  unsigned Imm = getMachineOpValue(MI, MO1, Fixups, STI);

  return Imm;
}

// Encoding Reg + Imm5u << 2 addressing mode
unsigned
RISCVMCCodeEmitter::getAddrRegImm5uWordEncoding(const MCInst &MI, unsigned OpNo,
                                                SmallVectorImpl<MCFixup> &Fixups,
                                                const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  const MCOperand &MO1 = MI.getOperand(OpNo + 1);
  unsigned Imm = getMachineOpValue(MI, MO1, Fixups, STI);
  unsigned Imm2Encode = 0;

  if (Imm & (1 << 2)) Imm2Encode = Imm2Encode + 2;
  if (Imm & (1 << 6)) Imm2Encode = Imm2Encode + 1;

  return Imm2Encode | ((Imm >> 3) << 5) |
         (getMachineOpValue(MI, MO, Fixups, STI)) << 2;
}

// Encoding Reg + Imm5u << 3 addressing mode
unsigned
RISCVMCCodeEmitter::getAddrRegImm5uDoubleEncoding(const MCInst &MI, unsigned OpNo,
                                                  SmallVectorImpl<MCFixup> &Fixups,
                                                  const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  const MCOperand &MO1 = MI.getOperand(OpNo + 1);
  unsigned Imm = getMachineOpValue(MI, MO1, Fixups, STI);

  return (Imm >> 6) | ((Imm >> 3) << 5) |
         (getMachineOpValue(MI, MO, Fixups, STI)) << 2;
}

#include "RISCVGenMCCodeEmitter.inc"
