//===-- RISCVISelLowering.cpp - RISCV DAG Lowering Implementation  --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that RISCV uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "RISCVISelLowering.h"
#include "RISCVCallingConv.h"
#include "RISCV.h"
#include "RISCVMachineFunctionInfo.h"
#include "RISCVRegisterInfo.h"
#include "RISCVSubtarget.h"
#include "RISCVTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "riscv-lower"

RISCVTargetLowering::RISCVTargetLowering(const TargetMachine &TM,
                                         const RISCVSubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  MVT PtrVT = Subtarget->isRV64() ? MVT::i64 : MVT::i32;
  // Set up the register classes.
  addRegisterClass(MVT::i32, &RISCV::GPRRegClass);

  if(Subtarget->isRV64())
    addRegisterClass(MVT::i64,  &RISCV::GPR64RegClass);

  LUI = Subtarget->isRV64() ? RISCV::LUI64 : RISCV::LUI;
  ADDI = Subtarget->isRV64() ? RISCV::ADDI64 : RISCV::ADDI;

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  if(Subtarget->isRV64())
    setStackPointerRegisterToSaveRestore(RISCV::X2_64);
  else
    setStackPointerRegisterToSaveRestore(RISCV::X2_32);

  // Load extented operations for i1 types must be promoted
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD,  VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1,  Promote);
  }

  // Custom lower UMULO/SMULO to correct the argument passing to __muldi3
  if(Subtarget->isRV32()) {
    setOperationAction(ISD::UMULO, MVT::i32, Custom);
    setOperationAction(ISD::SMULO, MVT::i32, Custom);
  }

  // Handle integer types.
  for (unsigned I = MVT::FIRST_INTEGER_VALUETYPE;
       I <= MVT::LAST_INTEGER_VALUETYPE;
       ++I) {
    MVT VT = MVT::SimpleValueType(I);
    if (isTypeLegal(VT)) {
      if(Subtarget->hasM()) {
        if(Subtarget->isRV64() && VT==MVT::i32)
          setOperationAction(ISD::MUL  , VT, Promote);
        if(Subtarget->isRV32() && VT==MVT::i64)
          setOperationAction(ISD::MUL  , VT, Expand);
        setOperationAction(ISD::MUL  , VT, Legal);
        setOperationAction(ISD::MULHS, VT, Legal);
        setOperationAction(ISD::MULHU, VT, Legal);
        setOperationAction(ISD::SDIV , VT, Legal);
        setOperationAction(ISD::UDIV , VT, Legal);
        setOperationAction(ISD::SREM , VT, Legal);
        setOperationAction(ISD::UREM , VT, Legal);
      }else{
        setOperationAction(ISD::MUL  , VT, Expand);
        setOperationAction(ISD::MULHS, VT, Expand);
        setOperationAction(ISD::MULHU, VT, Expand);
        setOperationAction(ISD::SDIV , VT, Expand);
        setOperationAction(ISD::UDIV , VT, Expand);
        setOperationAction(ISD::SREM , VT, Expand);
        setOperationAction(ISD::UREM , VT, Expand);
      }

      //No support at all
      setOperationAction(ISD::SDIVREM, VT, Expand);
      setOperationAction(ISD::UDIVREM, VT, Expand);
      //RISCV doesn't support  [ADD,SUB][E,C]
      setOperationAction(ISD::ADDE, VT, Expand);
      setOperationAction(ISD::SUBE, VT, Expand);
      setOperationAction(ISD::ADDC, VT, Expand);
      setOperationAction(ISD::SUBC, VT, Expand);
      //RISCV doesn't support s[hl,rl,ra]_parts
      setOperationAction(ISD::SHL_PARTS, VT, Expand);
      setOperationAction(ISD::SRL_PARTS, VT, Expand);
      setOperationAction(ISD::SRA_PARTS, VT, Expand);
      //RISCV doesn't support rotl
      setOperationAction(ISD::ROTL, VT, Expand);
      setOperationAction(ISD::ROTR, VT, Expand);
      setOperationAction(ISD::SMUL_LOHI, VT, Expand);
      setOperationAction(ISD::UMUL_LOHI, VT, Expand);

      // Expand ATOMIC_LOAD and ATOMIC_STORE using ATOMIC_CMP_SWAP.
      // FIXME: probably much too conservative.
      setOperationAction(ISD::ATOMIC_LOAD,  VT, Expand);
      setOperationAction(ISD::ATOMIC_STORE, VT, Expand);

      // No special instructions for these.
      setOperationAction(ISD::CTPOP,           VT, Expand);
      setOperationAction(ISD::CTTZ,            VT, Expand);
      setOperationAction(ISD::CTLZ,            VT, Expand);
      setOperationAction(ISD::CTTZ_ZERO_UNDEF, VT, Expand);
      setOperationAction(ISD::CTLZ_ZERO_UNDEF, VT, Expand);

    }
  }

  //Some Atmoic ops are legal
  if(Subtarget->hasA()) {
    if(Subtarget->isRV64()) {
      //push 32 bits up to 64
      setOperationAction(ISD::ATOMIC_SWAP,      MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_ADD,  MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_AND,  MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_OR,   MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_XOR,  MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_MIN,  MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_MAX,  MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_UMIN, MVT::i32, Promote);
      setOperationAction(ISD::ATOMIC_LOAD_UMAX, MVT::i32, Promote);
      //Legal in RV64A
      setOperationAction(ISD::ATOMIC_SWAP,      MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_ADD,  MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_AND,  MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_OR,   MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_XOR,  MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_MIN,  MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_MAX,  MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_UMIN, MVT::i64, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_UMAX, MVT::i64, Legal);
      //These are not native instructions
      setOperationAction(ISD::ATOMIC_CMP_SWAP,  MVT::i32, Expand);
      setOperationAction(ISD::ATOMIC_CMP_SWAP,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_NAND, MVT::i32, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_SUB,  MVT::i32, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_NAND, MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_SUB,  MVT::i64, Expand);
    } else {
      //Legal in RV32A
      setOperationAction(ISD::ATOMIC_SWAP,      MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_ADD,  MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_AND,  MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_OR,   MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_XOR,  MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_MIN,  MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_MAX,  MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_UMIN, MVT::i32, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_UMAX, MVT::i32, Legal);
      //Expand 64 bit into 32?
      setOperationAction(ISD::ATOMIC_SWAP,      MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_ADD,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_AND,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_OR,   MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_XOR,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_MIN,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_MAX,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_UMIN, MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_UMAX, MVT::i64, Expand);
      //These are not native instructions
      setOperationAction(ISD::ATOMIC_CMP_SWAP,  MVT::i32, Expand);
      setOperationAction(ISD::ATOMIC_CMP_SWAP,  MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_NAND, MVT::i32, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_SUB,  MVT::i32, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_NAND, MVT::i64, Expand);
      setOperationAction(ISD::ATOMIC_LOAD_SUB,  MVT::i64, Expand);
    }
  } else {
    //No atomic ops so expand all
    setOperationAction(ISD::ATOMIC_SWAP,      MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_ADD,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_AND,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_OR,   MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_XOR,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_MIN,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_MAX,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_UMIN, MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_UMAX, MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_SWAP,      MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_ADD,  MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_AND,  MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_OR,   MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_XOR,  MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_MIN,  MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_MAX,  MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_UMIN, MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_UMAX, MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_CMP_SWAP,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_CMP_SWAP,  MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_NAND, MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_SUB,  MVT::i32, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_NAND, MVT::i64, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_SUB,  MVT::i64, Expand);
  }

  // TODO: add all necessary setOperationAction calls

  setOperationAction(ISD::JumpTable, PtrVT, Custom);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

  setOperationAction(ISD::ROTR, MVT::i32, Expand);
  setOperationAction(ISD::ROTL, MVT::i32, Expand);
  setOperationAction(ISD::ROTR, MVT::i64, Expand);
  setOperationAction(ISD::ROTL, MVT::i64, Expand);

  setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::BR_CC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  if(Subtarget->isRV64())
    setOperationAction(ISD::SELECT_CC, MVT::i64, Custom);
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::i64, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);

  setOperationAction(ISD::VASTART,          MVT::Other, Custom);
  setOperationAction(ISD::VAARG,            MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,           MVT::Other, Expand);
  setOperationAction(ISD::VAEND,            MVT::Other, Expand);

  setOperationAction(ISD::DYNAMIC_STACKALLOC, PtrVT, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Expand);

  setOperationAction(ISD::STACKSAVE,        MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE,     MVT::Other, Expand);

  setOperationAction(ISD::BSWAP,            MVT::i32,   Expand);
  setOperationAction(ISD::BSWAP,            MVT::i64,   Expand);

  setOperationAction(ISD::GlobalAddress, PtrVT, Custom);
  setOperationAction(ISD::BlockAddress,  PtrVT, Custom);
  setOperationAction(ISD::ConstantPool,  PtrVT, Custom);

  setBooleanContents(ZeroOrOneBooleanContent);

  // Function alignments (log2)
  setMinFunctionAlignment(Subtarget->hasC() ? 1 : 2);
  setPrefFunctionAlignment(2);

  // inline memcpy() for kernel to see explicit copy
  MaxStoresPerMemset = MaxStoresPerMemsetOptSize = 128;
  MaxStoresPerMemcpy = MaxStoresPerMemcpyOptSize = 128;
  MaxStoresPerMemmove = MaxStoresPerMemmoveOptSize = 128;
}

void
RISCVTargetLowering::LowerOperationWrapper(SDNode *N,
                                           SmallVectorImpl<SDValue> &Results,
                                           SelectionDAG &DAG) const {
  SDValue Res = LowerOperation(SDValue(N, 0), DAG);

  for (unsigned I = 0, E = Res->getNumValues(); I != E; ++I)
    Results.push_back(Res.getValue(I));
}

void
RISCVTargetLowering::ReplaceNodeResults(SDNode *N,
                                        SmallVectorImpl<SDValue> &Results,
                                        SelectionDAG &DAG) const {
  return LowerOperationWrapper(N, Results, DAG);
}

SDValue RISCVTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return lowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:
    return lowerBlockAddress(Op, DAG);
  case ISD::JumpTable:
    return lowerJumpTable(Op, DAG);
  case ISD::SELECT_CC:
    return lowerSELECT_CC(Op, DAG);
  case ISD::VASTART:
    return lowerVASTART(Op, DAG);
  case ISD::RETURNADDR:
    return LowerRETURNADDR(Op, DAG);
  case ISD::FRAMEADDR:
    return LowerFRAMEADDR(Op, DAG);
  case ISD::ConstantPool:
    return LowerConstantPool(Op, DAG);
  case ISD::UMULO:
  case ISD::SMULO:
    return lowerUMULO_SMULO(Op, DAG);
  default:
    report_fatal_error("unimplemented operand");
  }
}

SDValue RISCVTargetLowering::lowerGlobalAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = N->getGlobal();
  int64_t Offset = N->getOffset();

//  if (!isPositionIndependent()) {
    SDValue GAHi =
        DAG.getTargetGlobalAddress(GV, DL, Ty, Offset, RISCVII::MO_HI);
    SDValue GALo =
        DAG.getTargetGlobalAddress(GV, DL, Ty, Offset, RISCVII::MO_LO);
    SDValue MNHi = SDValue(DAG.getMachineNode(LUI, DL, Ty, GAHi), 0);
    SDValue MNLo =
        SDValue(DAG.getMachineNode(ADDI, DL, Ty, MNHi, GALo), 0);
    return MNLo;
//  } else {
//    llvm_unreachable("Unable to lowerGlobalAddress");
//  }
}

SDValue RISCVTargetLowering::lowerBlockAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  BlockAddressSDNode *BASDN = cast<BlockAddressSDNode>(Op);
  const BlockAddress *BA = BASDN->getBlockAddress();

//  if (!isPositionIndependent()) {
    SDValue BAHi =
        DAG.getTargetBlockAddress(BA, Ty, 0, RISCVII::MO_HI);
    SDValue BALo =
        DAG.getTargetBlockAddress(BA, Ty, 0, RISCVII::MO_LO);
    SDValue MNHi = SDValue(DAG.getMachineNode(LUI, DL, Ty, BAHi), 0);
    SDValue MNLo =
        SDValue(DAG.getMachineNode(ADDI, DL, Ty, MNHi, BALo), 0);
    return MNLo;
//  } else {
//    llvm_unreachable("Unable to lowerBlockAddress");
//  }
}

SDValue RISCVTargetLowering::lowerExternalSymbol(SDValue Op,
                                                 SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  ExternalSymbolSDNode *N = cast<ExternalSymbolSDNode>(Op);
  const char *Sym = N->getSymbol();

  // TODO: should also handle gp-relative loads

//  if (!isPositionIndependent()) {
    SDValue GAHi = DAG.getTargetExternalSymbol(Sym, Ty, RISCVII::MO_HI);
    SDValue GALo = DAG.getTargetExternalSymbol(Sym, Ty, RISCVII::MO_LO);
    SDValue MNHi = SDValue(DAG.getMachineNode(LUI, DL, Ty, GAHi), 0);
    SDValue MNLo =
        SDValue(DAG.getMachineNode(ADDI, DL, Ty, MNHi, GALo), 0);
    return MNLo;
//  } else {
//    llvm_unreachable("Unable to lowerExternalSymbol");
//  }
}

SDValue RISCVTargetLowering::lowerJumpTable(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  JumpTableSDNode *N = cast<JumpTableSDNode>(Op);

//  if (!isPositionIndependent()) {
    SDValue GAHi =
        DAG.getTargetJumpTable(N->getIndex(), Ty, RISCVII::MO_HI);
    SDValue GALo =
        DAG.getTargetJumpTable(N->getIndex(), Ty, RISCVII::MO_LO);
    SDValue MNHi = SDValue(DAG.getMachineNode(LUI, DL, Ty, GAHi), 0);
    SDValue MNLo =
        SDValue(DAG.getMachineNode(ADDI, DL, Ty, MNHi, GALo), 0);
    return MNLo;
//  } else {
//    llvm_unreachable("Unable to lowerGlobalAddress");
//  }
}

SDValue RISCVTargetLowering::lowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueV = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc DL(Op);

  switch (CC) {
  default:
    break;
  case ISD::SETGT:
  case ISD::SETLE:
  case ISD::SETUGT:
  case ISD::SETULE:
    CC = ISD::getSetCCSwappedOperands(CC);
    std::swap(LHS, RHS);
    break;
  }

  SDValue TargetCC = DAG.getConstant(CC, DL, MVT::i32);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SDValue Ops[] = {LHS, RHS, TargetCC, TrueV, FalseV};

  return DAG.getNode(RISCVISD::SELECT_CC, DL, VTs, Ops);
}

SDValue RISCVTargetLowering::lowerVASTART(SDValue Op,
                                          SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  RISCVMachineFunctionInfo *FuncInfo = MF.getInfo<RISCVMachineFunctionInfo>();
  auto PtrVT = getPointerTy(DAG.getDataLayout());

  // Frame index of first vararg argument
  SDValue FrameIndex =
      DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(), PtrVT);
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // Create a store of the frame index to the location operand
  return DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue
RISCVTargetLowering::getReturnAddressFrameIndex(SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  RISCVMachineFunctionInfo *FuncInfo = MF.getInfo<RISCVMachineFunctionInfo>();
  int ReturnAddrIndex = FuncInfo->getRAIndex();
  auto PtrVT = getPointerTy(MF.getDataLayout());

  if (ReturnAddrIndex == 0) {
    // Set up a frame object for the return address.
    uint64_t SlotSize = MF.getDataLayout().getPointerSize();
    ReturnAddrIndex = MF.getFrameInfo().CreateFixedObject(SlotSize, -SlotSize,
                                                          true);
    FuncInfo->setRAIndex(ReturnAddrIndex);
  }

  return DAG.getFrameIndex(ReturnAddrIndex, PtrVT);
}

SDValue RISCVTargetLowering::LowerRETURNADDR(SDValue Op,
                                             SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MachineFunction &MF = DAG.getMachineFunction();
  MFI.setReturnAddressIsTaken(true);

  if (verifyReturnAddressArgumentIsConstant(Op, DAG))
    return SDValue();

  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDLoc dl(Op);
  auto PtrVT = getPointerTy(DAG.getDataLayout());
  unsigned RA = Subtarget->isRV64() ? RISCV::X1_64 : RISCV::X1_32;

  // We don't support Depth > 0
  if (Depth > 0) {
    return DAG.getConstant(0, dl, PtrVT);
  }

  unsigned Reg = MF.addLiveIn(RA, getRegClassFor(PtrVT));
  return DAG.getCopyFromReg(DAG.getEntryNode(), dl, Reg, PtrVT);
}

SDValue RISCVTargetLowering::LowerFRAMEADDR(SDValue Op,
                                            SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  unsigned FP = Subtarget->isRV64() ? RISCV::X8_64 : RISCV::X8_32;

  EVT VT = Op.getValueType();
  SDLoc dl(Op);
  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), dl,
                                         FP, VT);
  while (Depth--)
    FrameAddr = DAG.getLoad(VT, dl, DAG.getEntryNode(), FrameAddr,
                            MachinePointerInfo());
  return FrameAddr;
}

SDValue RISCVTargetLowering::LowerConstantPool(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
  const Constant *C = N->getConstVal();

//  if (!isPositionIndependent()) {
    uint8_t OpFlagHi = RISCVII::MO_HI;
    uint8_t OpFlagLo = RISCVII::MO_LO;

    SDValue Hi = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
                                           N->getOffset(), OpFlagHi);
    SDValue Lo = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
                                           N->getOffset(), OpFlagLo);

    SDValue MNHi = SDValue(DAG.getMachineNode(LUI, DL, Ty, Hi), 0);
    SDValue MNLo =
        SDValue(DAG.getMachineNode(ADDI, DL, Ty, MNHi, Lo), 0);
    return MNLo;
//  } else {
//    llvm_unreachable("Unable to LowerConstantPool");
//  }
}

// Custom lower UMULO/SMULO to pass the test case 
// c-c++-common/torture/builtin-arith-overflow-1.c UMULO testing.
// UMULO: unsigned multiplication overflow (__builtin_mul_overflow)
// UMULO will return true if the overflow occur.
// Generic code will expand to wider length multiplication
// and testing signed bit.
// E.g. i32: UMULO will expand to call __muldi3 and testing signed bit.
// However, generic call expand __muldi3 will passing arguments in wrong order.
// So we customize the UMULO/SMULO lowering.
SDValue RISCVTargetLowering::lowerUMULO_SMULO(SDValue Op,
                                              SelectionDAG &DAG) const {
  unsigned opcode = Op.getOpcode();
  assert((opcode == ISD::UMULO || opcode == ISD::SMULO) && "Invalid Opcode.");

  bool isSigned = (opcode == ISD::SMULO);
  EVT VT = MVT::i32;
  EVT WideVT = MVT::i64;
  SDLoc dl(Op);
  SDValue LHS = Op.getOperand(0);

  if (LHS.getValueType() != VT)
    return Op;

  SDValue ShiftAmt = DAG.getConstant(31, dl, VT);

  SDValue RHS = Op.getOperand(1);
  SDValue HiLHS, HiRHS;

  if (isSigned) {
    HiLHS = DAG.getNode(ISD::SRA, dl, VT, LHS, ShiftAmt);
    HiRHS = DAG.getNode(ISD::SRA, dl, MVT::i32, RHS, ShiftAmt);
  } else {
    HiLHS = HiRHS = DAG.getConstant(0, dl, VT);
  }
  // Arguments passing order
  SDValue Args[] = { LHS, HiLHS, RHS, HiRHS };

  SDValue MulResult = makeLibCall(DAG,
                                  RTLIB::MUL_I64, WideVT,
                                  Args, isSigned, dl).first;
  SDValue BottomHalf = DAG.getNode(ISD::EXTRACT_ELEMENT, dl, VT,
                                   MulResult, DAG.getIntPtrConstant(0, dl));
  SDValue TopHalf = DAG.getNode(ISD::EXTRACT_ELEMENT, dl, VT,
                                MulResult, DAG.getIntPtrConstant(1, dl));
  if (isSigned) {
    SDValue Tmp1 = DAG.getNode(ISD::SRA, dl, VT, BottomHalf, ShiftAmt);
    TopHalf = DAG.getSetCC(dl, MVT::i32, TopHalf, Tmp1, ISD::SETNE);
  } else {
    TopHalf = DAG.getSetCC(dl, MVT::i32, TopHalf, DAG.getConstant(0, dl, VT),
                           ISD::SETNE);
  }
  // MulResult is a node with an illegal type. Because such things are not
  // generally permitted during this phase of legalization, ensure that
  // nothing is left using the node. The above EXTRACT_ELEMENT nodes should have
  // been folded.
  assert(MulResult->use_empty() && "Illegally typed node still in use!");

  SDValue Ops[2] = { BottomHalf, TopHalf } ;
  return DAG.getMergeValues(Ops, dl);
}

MachineBasicBlock *
RISCVTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                 MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  bool Is64RV = Subtarget->isRV64();

  // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = ++BB->getIterator();

  // ThisMBB:
  // ...
  //  TrueVal = ...
  //  jmp_XX r1, r2 goto Copy1MBB
  //  fallthrough --> Copy0MBB
  MachineBasicBlock *ThisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *Copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *Copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, Copy0MBB);
  F->insert(I, Copy1MBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  Copy1MBB->splice(Copy1MBB->begin(), BB,
                   std::next(MachineBasicBlock::iterator(MI)), BB->end());
  Copy1MBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(Copy0MBB);
  BB->addSuccessor(Copy1MBB);

  // Insert Branch if Flag
  unsigned LHS = MI.getOperand(1).getReg();
  unsigned RHS = MI.getOperand(2).getReg();
  int CC = MI.getOperand(3).getImm();
  unsigned Opcode = -1;
  switch (CC) {
  case ISD::SETEQ:
    Opcode = Is64RV ? RISCV::BEQ64 : RISCV::BEQ;
    break;
  case ISD::SETNE:
    Opcode = Is64RV ? RISCV::BNE64 : RISCV::BNE;
    break;
  case ISD::SETLT:
    Opcode = Is64RV ? RISCV::BLT64 : RISCV::BLT;
    break;
  case ISD::SETGE:
    Opcode = Is64RV ? RISCV::BGE64 : RISCV::BGE;
    break;
  case ISD::SETULT:
    Opcode = Is64RV ? RISCV::BLTU64 : RISCV::BLTU;
    break;
  case ISD::SETUGE:
    Opcode = Is64RV ? RISCV::BGEU64 : RISCV::BGEU;
    break;
  default:
    report_fatal_error("unimplemented select CondCode " + Twine(CC));
  }

  BuildMI(BB, DL, TII.get(Opcode))
    .addReg(LHS)
    .addReg(RHS)
    .addMBB(Copy1MBB);

  // Copy0MBB:
  //  %FalseValue = ...
  //  # fallthrough to Copy1MBB
  BB = Copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(Copy1MBB);

  // Copy1MBB:
  //  %Result = phi [ %FalseValue, Copy0MBB ], [ %TrueValue, ThisMBB ]
  // ...
  BB = Copy1MBB;
  BuildMI(*BB, BB->begin(), DL, TII.get(RISCV::PHI), MI.getOperand(0).getReg())
      .addReg(MI.getOperand(5).getReg())
      .addMBB(Copy0MBB)
      .addReg(MI.getOperand(4).getReg())
      .addMBB(ThisMBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return BB;
}

//===----------------------------------------------------------------------===//
//                       RISCV Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
TargetLowering::ConstraintType
RISCVTargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return C_RegisterClass;
    default:
      break;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
RISCVTargetLowering::getRegForInlineAsmConstraint(
    const TargetRegisterInfo *TRI, StringRef Constraint, MVT VT) const {
  if (Constraint.size() == 1) {
    // GCC Constraint Letters
    switch (Constraint[0]) {
    default: break;
    case 'r':   // GENERAL_REGS
      if (Subtarget->isRV64())
        return std::make_pair(0U, &RISCV::GPR64RegClass);
      else
        return std::make_pair(0U, &RISCV::GPRRegClass);
    }
  }

  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}


//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "RISCVGenCallingConv.inc"

// addLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned
addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
  unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
  MF.getRegInfo().addLiveIn(PReg, VReg);
  return VReg;
}

static CCAssignFn *getCCAssignFn(const RISCVSubtarget *STI, bool IsVarArg) {
  if (STI->hasE())
    return CC_RISCV32E;
  if (STI->isRV64())
    return (IsVarArg ? CC_RISCV64_VAR : CC_RISCV64);

  return (IsVarArg ? CC_RISCV32_VAR : CC_RISCV32);
}

// RestoreVarArgRegs - Store VarArg register to the stack
void RISCVTargetLowering::RestoreVarArgRegs(std::vector<SDValue> &OutChains,
                                            SDValue Chain, const SDLoc &DL,
                                            SelectionDAG &DAG,
                                            CCState &State) const {
  ArrayRef<MCPhysReg> ArgRegs = getArgRegs(Subtarget);
  unsigned Idx = State.getFirstUnallocated(ArgRegs);
  unsigned RegSizeInBytes = Subtarget->isRV64() ? 8 : 4;
  MVT RegTy = MVT::getIntegerVT(RegSizeInBytes * 8);
  const TargetRegisterClass *RC = getRegClassFor(RegTy);
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  RISCVMachineFunctionInfo *RISCVFI = MF.getInfo<RISCVMachineFunctionInfo>();
  // VaArgOffset is the va_start offset from stack pointer
  int VaArgOffset = 0;

  // All ArgRegs use to pass arguments
  if (ArgRegs.size() == Idx) {
    VaArgOffset = State.getNextStackOffset();
  //
  // ---------------- <-- va_start
  // | outgoing args|
  // ---------------- <-- sp
  // |   A0 ~ A7    |
  // ----------------
  }
  else {
    VaArgOffset = -(int)(RegSizeInBytes * (ArgRegs.size() - Idx));
  // E.g Idx = A3 which means f (a, b, c, ...)
  //
  // ---------------- <-- sp
  // |   A3 ~ A7    |
  // ---------------- <-- va_start
  // |   A0 ~ A2    |
  // ----------------
  }

  // Record the frame index of the first variable argument
  // which is a value necessary to VASTART.
  int VA_FI = MFI.CreateFixedObject(RegSizeInBytes, VaArgOffset, true);
  RISCVFI->setVarArgsFrameIndex(VA_FI);

  // Copy the integer registers that have not been used for argument passing
  // to the argument register save area. The save area is allocated in the
  // callee's stack frame.
  // For above case Idx = A3, generate store to push A3~A7 to callee stack
  // frame.
  for (unsigned I = Idx; I < ArgRegs.size();
       ++I, VaArgOffset += RegSizeInBytes) {
    unsigned Reg = addLiveIn(MF, ArgRegs[I], RC);
    SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegTy);
    int FI = MFI.CreateFixedObject(RegSizeInBytes, VaArgOffset, true);
    SDValue PtrOff = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
    SDValue Store =
        DAG.getStore(Chain, DL, ArgValue, PtrOff, MachinePointerInfo());
    cast<StoreSDNode>(Store.getNode())->getMemOperand()->setValue(
        (Value *)nullptr);
    OutChains.push_back(Store);
  }
}

void RISCVTargetLowering::copyByValRegs(
    SDValue Chain, const SDLoc &DL, std::vector<SDValue> &OutChains,
    SelectionDAG &DAG, const ISD::ArgFlagsTy &Flags,
    SmallVectorImpl<SDValue> &InVals, const Argument *FuncArg,
    const CCValAssign &VA, CCState &CCInfo) const {
  ArrayRef<MCPhysReg> ArgRegs = getArgRegs(Subtarget);
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  unsigned FirstReg, LastReg;
  unsigned ByValIdx = CCInfo.getInRegsParamsProcessed();
  unsigned ByValArgsCount = CCInfo.getInRegsParamsCount();

  // The ByVal Argument pass by register
  if (ByValIdx < ByValArgsCount) {
    CCInfo.getInRegsParamInfo(ByValIdx, FirstReg, LastReg);
  // The ByVal Argument pass by stack
  } else {
    unsigned MaxArgReg = Subtarget->hasE() ? 5 : 7;
    unsigned FirstRegIdx = CCInfo.getFirstUnallocated(ArgRegs);
    FirstReg = FirstRegIdx == MaxArgReg + 1 ? MaxArgReg : FirstRegIdx;
    LastReg = MaxArgReg;
  }

  unsigned GPRSizeInBytes = Subtarget->isRV64() ? 8 : 4;
  unsigned NumRegs = LastReg - FirstReg;
  unsigned RegAreaSize = NumRegs * GPRSizeInBytes;
  unsigned FrameObjSize = std::max(Flags.getByValSize(), RegAreaSize);
  int FrameObjOffset;
  ArrayRef<MCPhysReg> ByValArgRegs = getArgRegs(Subtarget);

  if (RegAreaSize)
    FrameObjOffset =
        - (int)((ByValArgRegs.size() - FirstReg) * GPRSizeInBytes);
  else
    FrameObjOffset = VA.getLocMemOffset();

  // Create frame object.
  EVT PtrTy = getPointerTy(DAG.getDataLayout());
  int FI = MFI.CreateFixedObject(FrameObjSize, FrameObjOffset, true);
  SDValue FIN = DAG.getFrameIndex(FI, PtrTy);
  InVals.push_back(FIN);

  if (!NumRegs)
    return;

  // Copy arg registers.
  MVT RegTy = MVT::getIntegerVT(GPRSizeInBytes * 8);
  const TargetRegisterClass *RC = getRegClassFor(RegTy);

  for (unsigned I = 0; I < NumRegs; ++I) {
    unsigned ArgReg = ByValArgRegs[FirstReg + I];
    unsigned VReg = addLiveIn(MF, ArgReg, RC);
    unsigned Offset = I * GPRSizeInBytes;
    SDValue StorePtr = DAG.getNode(ISD::ADD, DL, PtrTy, FIN,
                                   DAG.getConstant(Offset, DL, PtrTy));
    SDValue Store = DAG.getStore(Chain, DL, DAG.getRegister(VReg, RegTy),
                                 StorePtr, MachinePointerInfo(FuncArg, Offset));
    OutChains.push_back(Store);
  }
}

static SDValue UnpackFromArgumentSlot(SDValue Val, const CCValAssign &VA,
                                      EVT ArgVT, const SDLoc &DL,
                                      SelectionDAG &DAG) {
  MVT LocVT = VA.getLocVT();
  EVT ValVT = VA.getValVT();

  // Shift into the upper bits if necessary.
  switch (VA.getLocInfo()) {
  default:
    break;
  case CCValAssign::AExtUpper:
  case CCValAssign::SExtUpper:
  case CCValAssign::ZExtUpper: {
    unsigned ValSizeInBits = ArgVT.getSizeInBits();
    unsigned LocSizeInBits = VA.getLocVT().getSizeInBits();
    unsigned Opcode =
        VA.getLocInfo() == CCValAssign::ZExtUpper ? ISD::SRL : ISD::SRA;
    Val = DAG.getNode(
        Opcode, DL, VA.getLocVT(), Val,
        DAG.getConstant(LocSizeInBits - ValSizeInBits, DL, VA.getLocVT()));
    break;
  }
  }

  // If this is an value smaller than the argument slot size (32-bit for RISCV32,
  // 64-bit for RISCV64), it has been promoted in some way to the argument slot
  // size. Extract the value and insert any appropriate assertions regarding
  // sign/zero extension.
  switch (VA.getLocInfo()) {
  default:
    llvm_unreachable("Unknown loc info!");
  case CCValAssign::Full:
  case CCValAssign::Indirect:
    break;
  case CCValAssign::AExtUpper:
  case CCValAssign::AExt:
    Val = DAG.getNode(ISD::TRUNCATE, DL, ValVT, Val);
    break;
  case CCValAssign::SExtUpper:
  case CCValAssign::SExt:
    Val = DAG.getNode(ISD::AssertSext, DL, LocVT, Val, DAG.getValueType(ValVT));
    Val = DAG.getNode(ISD::TRUNCATE, DL, ValVT, Val);
    break;
  case CCValAssign::ZExtUpper:
  case CCValAssign::ZExt:
    Val = DAG.getNode(ISD::AssertZext, DL, LocVT, Val, DAG.getValueType(ValVT));
    Val = DAG.getNode(ISD::TRUNCATE, DL, ValVT, Val);
    break;
  case CCValAssign::BCvt:
    Val = DAG.getNode(ISD::BITCAST, DL, ValVT, Val);
    break;
  }

  return Val;
}

// Transform physical registers into virtual registers
SDValue RISCVTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  RISCVMachineFunctionInfo *FI = MF.getInfo<RISCVMachineFunctionInfo>();
  EVT PtrVT = getPointerTy(DAG.getDataLayout());

  FI->setVarArgsFrameIndex(0);
 
  // Used with vargs to accumulate store chains.
  std::vector<SDValue> OutChains;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;

  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  const Function *Func = DAG.getMachineFunction().getFunction();
  Function::const_arg_iterator FuncArg = Func->arg_begin();

  CCAssignFn *CC = getCCAssignFn (Subtarget, IsVarArg);

  CCInfo.AnalyzeFormalArguments(Ins, CC);
  FI->setFormalArgInfo(CCInfo.getNextStackOffset(),
                       CCInfo.getInRegsParamsCount() > 0);

  unsigned CurArgIdx = 0;

  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    unsigned offset = 0;
    CCValAssign &VA = ArgLocs[I];
    if (Ins[I].isOrigArg()) {
      std::advance(FuncArg, Ins[I].getOrigArgIndex() - CurArgIdx);
      CurArgIdx = Ins[I].getOrigArgIndex();
    }
    ISD::ArgFlagsTy Flags = Ins[I].Flags;

    if (Flags.isByVal()) {
      assert(Ins[I].isOrigArg() && "Byval arguments cannot be implicit");

      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");

      copyByValRegs(Chain, DL, OutChains, DAG, Flags, InVals, &*FuncArg,
                    VA, CCInfo);
      CCInfo.nextInRegsParam();

      continue;
    }

    bool IsRegLoc = VA.isRegLoc();
    SDValue ArgValue;
    // Arguments stored on registers
    if (IsRegLoc) {
      MVT RegVT = VA.getLocVT();
      unsigned ArgReg = VA.getLocReg();
      const TargetRegisterClass *RC = getRegClassFor(RegVT);

      // Transform the arguments stored on
      // physical registers into virtual ones
      unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
      ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

      // All integral types are promoted to the GPR width in RISCV Clang,
      // So we have to handle the integer argument smaller then GPR width.
      // See gcc.c-torture/execute/20000205-1.c testcase argument passing
      // of f for RISCV64 target.
      ArgValue = UnpackFromArgumentSlot(ArgValue, VA, Ins[I].ArgVT, DL, DAG);
    } else { // VA.isRegLoc()
      MVT LocVT = VA.getLocVT();

      // sanity check
      assert(VA.isMemLoc());

      // The stack pointer offset is relative to the caller stack frame.
      int FrameIdx = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8,
                                      VA.getLocMemOffset(), true);
      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(FrameIdx,
                                      getPointerTy(DAG.getDataLayout()));
      ArgValue = DAG.getLoad(
          VA.getValVT(), DL, Chain, FIN,
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(),
                                            FrameIdx));
      OutChains.push_back(ArgValue.getValue(1));
    }
    // Convert the value of the argument register into the value that's
    // being passed.
    if (VA.getLocInfo() == CCValAssign::Indirect) {
      InVals.push_back(DAG.getLoad(VA.getValVT(), DL, Chain, ArgValue,
                                   MachinePointerInfo()));
      // If the original argument was split (e.g. i128), we need
      // to load all parts of it here (using the same address).
      unsigned ArgIndex = Ins[I].OrigArgIndex;
      assert (Ins[I].PartOffset == 0);
      while (I + 1 != E && Ins[I + 1].OrigArgIndex == ArgIndex) {
        CCValAssign &PartVA = ArgLocs[I + 1];
        unsigned PartOffset = Ins[I + 1].PartOffset;
        SDValue Address = DAG.getNode(ISD::ADD, DL, PtrVT, ArgValue,
                                      DAG.getIntPtrConstant(PartOffset, DL));
        InVals.push_back(DAG.getLoad(PartVA.getValVT(), DL, Chain, Address,
                                     MachinePointerInfo()));
        ++I;
      }
    } else
      InVals.push_back(ArgValue);
  }

  // Push VarArg Registers to the stack
  if (IsVarArg)
    RestoreVarArgRegs(OutChains, Chain, DL, DAG, CCInfo);

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens when on varg functions
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
  }

  return Chain;
}

/// LowerMemOpCallTo - Store the argument to the stack.
SDValue RISCVTargetLowering::LowerMemOpCallTo(SDValue Chain, SDValue StackPtr,
                                              SDValue Arg, const SDLoc &dl,
                                              SelectionDAG &DAG,
                                              const CCValAssign &VA,
                                              ISD::ArgFlagsTy Flags) const {
  unsigned LocMemOffset = VA.getLocMemOffset();
  SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset, dl);
  PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()),
                       StackPtr, PtrOff);
  return DAG.getStore(
      Chain, dl, Arg, PtrOff,
      MachinePointerInfo::getStack(DAG.getMachineFunction(), LocMemOffset));
}

// Copy byVal arg to registers and stack.
void RISCVTargetLowering::passByValArg(
    SDValue Chain, const SDLoc &DL,
    RegsToPassVector &RegsToPass,
    SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
    MachineFrameInfo &MFI, SelectionDAG &DAG, SDValue Arg, unsigned FirstReg,
    unsigned LastReg, const ISD::ArgFlagsTy &Flags, bool isLittle,
    const CCValAssign &VA) const {
  unsigned ByValSizeInBytes = Flags.getByValSize();
  unsigned OffsetInBytes = 0; // From beginning of struct
  unsigned RegSizeInBytes = Subtarget->isRV64() ? 8 : 4;
  unsigned Alignment = std::min(Flags.getByValAlign(), RegSizeInBytes);

  EVT PtrTy = getPointerTy(DAG.getDataLayout()),
      RegTy = MVT::getIntegerVT(RegSizeInBytes * 8);
  unsigned NumRegs = LastReg - FirstReg;

  if (NumRegs) {
    ArrayRef<MCPhysReg> ArgRegs = getArgRegs(Subtarget);

    bool LeftoverBytes = (NumRegs * RegSizeInBytes > ByValSizeInBytes);
    unsigned I = 0;

    // Copy words to registers.
    for (; I < NumRegs - LeftoverBytes; ++I, OffsetInBytes += RegSizeInBytes) {
      SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
                                    DAG.getConstant(OffsetInBytes, DL, PtrTy));
      SDValue LoadVal = DAG.getLoad(RegTy, DL, Chain, LoadPtr,
                                    MachinePointerInfo(), Alignment);
      MemOpChains.push_back(LoadVal.getValue(1));
      unsigned ArgReg = ArgRegs[FirstReg + I];
      RegsToPass.push_back(std::make_pair(ArgReg, LoadVal));
    }

    // Return if the struct has been fully copied.
    if (ByValSizeInBytes == OffsetInBytes)
      return;

    // Copy the remainder of the byval argument with sub-word loads and shifts.
    if (LeftoverBytes) {
      SDValue Val;

      for (unsigned LoadSizeInBytes = RegSizeInBytes / 2, TotalBytesLoaded = 0;
           OffsetInBytes < ByValSizeInBytes; LoadSizeInBytes /= 2) {
        unsigned RemainingSizeInBytes = ByValSizeInBytes - OffsetInBytes;

        if (RemainingSizeInBytes < LoadSizeInBytes)
          continue;

        // Load subword.
        SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
                                      DAG.getConstant(OffsetInBytes, DL,
                                                      PtrTy));
        SDValue LoadVal = DAG.getExtLoad(
            ISD::ZEXTLOAD, DL, RegTy, Chain, LoadPtr, MachinePointerInfo(),
            MVT::getIntegerVT(LoadSizeInBytes * 8), Alignment);
        MemOpChains.push_back(LoadVal.getValue(1));

        // Shift the loaded value.
        unsigned Shamt;

        if (isLittle)
          Shamt = TotalBytesLoaded * 8;
        else
          Shamt = (RegSizeInBytes - (TotalBytesLoaded + LoadSizeInBytes)) * 8;

        SDValue Shift = DAG.getNode(ISD::SHL, DL, RegTy, LoadVal,
                                    DAG.getConstant(Shamt, DL, MVT::i32));

        if (Val.getNode())
          Val = DAG.getNode(ISD::OR, DL, RegTy, Val, Shift);
        else
          Val = Shift;

        OffsetInBytes += LoadSizeInBytes;
        TotalBytesLoaded += LoadSizeInBytes;
        Alignment = std::min(Alignment, LoadSizeInBytes);
      }

      unsigned ArgReg = ArgRegs[FirstReg + I];
      RegsToPass.push_back(std::make_pair(ArgReg, Val));
      return;
    }
  }

  // Copy remainder of byval arg to it with memcpy.
  unsigned MemCpySize = ByValSizeInBytes - OffsetInBytes;
  SDValue Src = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
                            DAG.getConstant(OffsetInBytes, DL, PtrTy));
  SDValue Dst = DAG.getNode(ISD::ADD, DL, PtrTy, StackPtr,
                            DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));
  Chain = DAG.getMemcpy(Chain, DL, Dst, Src,
                        DAG.getConstant(MemCpySize, DL, PtrTy),
                        Alignment, /*isVolatile=*/false, /*AlwaysInline=*/false,
                        /*isTailCall=*/false,
                        MachinePointerInfo(), MachinePointerInfo());
  MemOpChains.push_back(Chain);
}

// Lower a call to a callseq_start + CALL + callseq_end chain, and add input 
// and output parameter nodes.
SDValue
RISCVTargetLowering::LowerCall(CallLoweringInfo &CLI,
                               SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CLI.IsTailCall = false;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  const TargetFrameLowering *TFL = Subtarget->getFrameLowering();
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  EVT PtrVT = getPointerTy(MF.getDataLayout());

  // Analyze the operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState ArgCCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCAssignFn *CC = getCCAssignFn (Subtarget, IsVarArg);
  ArgCCInfo.AnalyzeCallOperands(Outs, CC);


  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NextStackOffset = ArgCCInfo.getNextStackOffset();

  // Chain is the output chain of the last Load/Store or CopyToReg node.
  // ByValChain is the output chain of the last Memcpy node created for copying
  // byval arguments to the stack.
  unsigned StackAlignment = TFL->getStackAlignment();
  NextStackOffset = alignTo(NextStackOffset, StackAlignment);
  SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, DL, true);

  if (!CLI.IsTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, NextStackOffset, 0, DL);

  // Copy argument values to their designated locations.
  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  unsigned SP = Subtarget->isRV64() ? RISCV::X2_64 : RISCV::X2_32;
  SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, SP,
                                        getPointerTy(DAG.getDataLayout()));

  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    CCValAssign &VA = ArgLocs[I];
    SDValue Arg = OutVals[I];
    ISD::ArgFlagsTy Flags = Outs[I].Flags;

    if (VA.getLocInfo() == CCValAssign::Indirect) {
      // Store the argument in a stack slot and pass its address.
      SDValue SpillSlot = DAG.CreateStackTemporary(Outs[I].ArgVT);
      int FI = cast<FrameIndexSDNode>(SpillSlot)->getIndex();
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, Arg, SpillSlot,
                       MachinePointerInfo::getFixedStack(MF, FI)));
      // If the original argument was split (e.g. i128), we need
      // to store all parts of it here (and pass just one address).
      unsigned ArgIndex = Outs[I].OrigArgIndex;
      assert (Outs[I].PartOffset == 0);
      while (I + 1 != E && Outs[I + 1].OrigArgIndex == ArgIndex) {
        SDValue PartValue = OutVals[I + 1];
        unsigned PartOffset = Outs[I + 1].PartOffset;
        SDValue Address = DAG.getNode(ISD::ADD, DL, PtrVT, SpillSlot,
                                      DAG.getIntPtrConstant(PartOffset, DL));
        MemOpChains.push_back(
            DAG.getStore(Chain, DL, PartValue, Address,
                         MachinePointerInfo::getFixedStack(MF, FI)));
        ++I;
      }
      Arg = SpillSlot;
    } else if (Flags.isByVal()) {
      unsigned offset = 0;
      unsigned FirstByValReg, LastByValReg;
      unsigned ByValIdx = ArgCCInfo.getInRegsParamsProcessed();
      unsigned ByValArgsCount = ArgCCInfo.getInRegsParamsCount();

      if (ByValIdx < ByValArgsCount) {
        ArgCCInfo.getInRegsParamInfo(ByValIdx, FirstByValReg, LastByValReg);

        assert(Flags.getByValSize() &&
               "ByVal args of size 0 should have been ignored by front-end.");
        assert(ByValIdx < ArgCCInfo.getInRegsParamsCount());
        assert(!CLI.IsTailCall &&
               "Do not tail-call optimize if there is a byval argument.");
        passByValArg(Chain, DL, RegsToPass, MemOpChains, StackPtr, MFI, DAG, Arg,
                     FirstByValReg, LastByValReg, Flags, true, VA);
        ArgCCInfo.nextInRegsParam();

        // If parameter size outsides register area, "offset" value
        // helps us to calculate stack slot for remained part properly.
        offset = LastByValReg - FirstByValReg;
      }
      if (Flags.getByValSize() > 4*offset) {
        auto PtrVT = getPointerTy(DAG.getDataLayout());
        unsigned LocMemOffset = VA.getLocMemOffset();
        SDValue StkPtrOff = DAG.getIntPtrConstant(LocMemOffset, DL);
        SDValue Dst = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr, StkPtrOff);
        SDValue SrcOffset = DAG.getIntPtrConstant(4*offset, DL);
        SDValue Src = DAG.getNode(ISD::ADD, DL, PtrVT, Arg, SrcOffset);
        SDValue SizeNode = DAG.getConstant(Flags.getByValSize() - 4*offset, DL,
                                           MVT::i32);

        Chain = DAG.getMemcpy(Chain, DL, Dst, Src, SizeNode,
                              Flags.getByValAlign(), /*isVolatile=*/false,
                              /*AlwaysInline=*/false, /*isTailCall=*/false,
                              MachinePointerInfo(), MachinePointerInfo());
        MemOpChains.push_back(Chain);
      }
      continue;
    }

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Indirect:
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::BCvt:
      Arg = DAG.getNode(ISD::BITCAST, DL, VA.getLocVT(), Arg);
      break;
    }

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());
      MemOpChains.push_back(LowerMemOpCallTo(Chain, StackPtr, Arg,
                                             DL, DAG, VA, Flags));
    }
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, DL, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  if (isa<GlobalAddressSDNode>(Callee)) {
    Callee = lowerGlobalAddress(Callee, DAG);
  } else if (isa<ExternalSymbolSDNode>(Callee)) {
    Callee = lowerExternalSymbol(Callee, DAG);
  }

  // The first call operand is the chain and the second is the target address.
  std::vector<SDValue> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  // Add a register mask operand representing the call-preserved(callee saved)
  // registers. Let register allocator could get correct register live time
  // cross call.
  const TargetRegisterInfo *TRI = Subtarget->getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(DAG.getMachineFunction(), CLI.CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(RISCVISD::CALL, DL, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain, NextStackOffsetVal,
                             DAG.getIntPtrConstant(0, DL, true), InFlag, DL);

  InFlag = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCAssignFn *RetCC = Subtarget->isRV64() ? RetCC_RISCV64 : RetCC_RISCV32;
  CCInfo.AnalyzeCallResult(Ins, RetCC);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned I = 0; I != RVLocs.size(); ++I) {
    CCValAssign &VA = RVLocs[I];
    SDValue Val = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(),
                                     VA.getLocVT(), InFlag);
    Chain = Val.getValue(1);
    InFlag = Val.getValue(2);

    // Generate extension node for getting i32 return value from X10_64
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::BCvt:
      Val = DAG.getNode(ISD::BITCAST, DL, VA.getValVT(), Val);
      break;
    case CCValAssign::AExt:
    case CCValAssign::AExtUpper:
      Val = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Val);
      break;
    case CCValAssign::ZExt:
    case CCValAssign::ZExtUpper:
      Val = DAG.getNode(ISD::AssertZext, DL, VA.getLocVT(), Val,
                        DAG.getValueType(VA.getValVT()));
      Val = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Val);
      break;
    case CCValAssign::SExt:
    case CCValAssign::SExtUpper:
      Val = DAG.getNode(ISD::AssertSext, DL, VA.getLocVT(), Val,
                        DAG.getValueType(VA.getValVT()));
      Val = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Val);
      break;
    }

    InVals.push_back(Val);
  }

  return Chain;
}

bool
RISCVTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                    MachineFunction &MF, bool isVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    LLVMContext &Context) const
{
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  CCAssignFn *RetCC = Subtarget->isRV64() ? RetCC_RISCV64 : RetCC_RISCV32;
  return CCInfo.CheckReturn(Outs, RetCC);
}


SDValue
RISCVTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                 bool IsVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &DL, SelectionDAG &DAG) const {

  // Stores the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // Info about the registers and stack slot
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCAssignFn *RetCC = Subtarget->isRV64() ? RetCC_RISCV64 : RetCC_RISCV32;
  CCInfo.AnalyzeReturn(Outs, RetCC);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    SDValue Val = OutVals[i];
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    // Return i32 type by X10_64 for riscv64,
    // we need to promote the value to i64 first.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::BCvt:
      Val = DAG.getNode(ISD::BITCAST, DL, VA.getLocVT(), Val);
      break;
    case CCValAssign::AExtUpper:
      LLVM_FALLTHROUGH;
    case CCValAssign::AExt:
      Val = DAG.getNode(ISD::ANY_EXTEND, DL, VA.getLocVT(), Val);
      break;
    case CCValAssign::ZExtUpper:
      LLVM_FALLTHROUGH;
    case CCValAssign::ZExt:
      Val = DAG.getNode(ISD::ZERO_EXTEND, DL, VA.getLocVT(), Val);
      break;
    case CCValAssign::SExtUpper:
      LLVM_FALLTHROUGH;
    case CCValAssign::SExt:
      Val = DAG.getNode(ISD::SIGN_EXTEND, DL, VA.getLocVT(), Val);
      break;
    }

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Flag);

    // Guarantee that all emitted copies are stuck together
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode()) {
    RetOps.push_back(Flag);
  }

  return DAG.getNode(RISCVISD::RET_FLAG, DL, MVT::Other, RetOps);
}

const char *RISCVTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((RISCVISD::NodeType)Opcode) {
  case RISCVISD::FIRST_NUMBER:
    break;
  case RISCVISD::RET_FLAG:
    return "RISCVISD::RET_FLAG";
  case RISCVISD::CALL:
    return "RISCVISD::CALL";
  case RISCVISD::SELECT_CC:
    return "RISCVISD::SELECT_CC";
  }
  return nullptr;
}

EVT RISCVTargetLowering::getSetCCResultType(const DataLayout &, LLVMContext &,
                                            EVT VT) const {
  if (!VT.isVector())
    return MVT::i32;
  return VT.changeVectorElementTypeToInteger();
}

bool
RISCVTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  // The RISCV target isn't yet aware of offsets.
  return false;
}
