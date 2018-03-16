//===-- RISCVMCExpr.cpp - RISCV specific MC expression classes ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the assembly expression modifiers
// accepted by the RISCV architecture (e.g. ":lo12:", ":gottprel_g1:", ...).
//
//===----------------------------------------------------------------------===//

#include "RISCVMCExpr.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "riscvmcexpr"

const RISCVMCExpr *RISCVMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                       MCContext &Ctx) {
  return new (Ctx) RISCVMCExpr(Expr, Kind);
}

void RISCVMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  bool HasVariant = getKind() != VK_RISCV_None;
  if (HasVariant)
    OS << '%' << getVariantKindName(getKind()) << '(';
  Expr->print(OS, MAI);
  if (HasVariant)
    OS << ')';
}

bool RISCVMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                            const MCAsmLayout *Layout,
                                            const MCFixup *Fixup) const {
  if (!getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  if (Res.getRefKind() != MCSymbolRefExpr::VK_None)
    return false;

  if (Res.isAbsolute() && Fixup == nullptr) {
    int64_t AbsVal = Res.getConstant();
    switch (Kind) {
    case VK_RISCV_None:
    case VK_RISCV_Invalid:
      llvm_unreachable("MEK_None and MEK_Special are invalid");
    case VK_RISCV_HI:
    case VK_RISCV_PCREL_HI:
      AbsVal = ((AbsVal + 0x800) >> 12) & 0xfffff;
      break;
    case VK_RISCV_LO:
      AbsVal = SignExtend64<12>(AbsVal & 0xfff);
      break;
    }
    Res = MCValue::get(AbsVal);
    return true;
  }

  Res = MCValue::get(Res.getSymA(), Res.getSymB(), Res.getConstant(),
                     getKind());
  return true;
}

void RISCVMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

RISCVMCExpr::VariantKind RISCVMCExpr::getVariantKindForName(StringRef name) {
  return StringSwitch<RISCVMCExpr::VariantKind>(name)
      .Case("lo", VK_RISCV_LO)
      .Case("hi", VK_RISCV_HI)
      .Case("pcrel_hi", VK_RISCV_PCREL_HI)
      .Default(VK_RISCV_None);
}

StringRef RISCVMCExpr::getVariantKindName(VariantKind Kind) {
  switch (Kind) {
  case VK_RISCV_LO:
    return "lo";
  case VK_RISCV_HI:
    return "hi";
  case VK_RISCV_PCREL_HI:
    return "pcrel_hi";
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  }
}
