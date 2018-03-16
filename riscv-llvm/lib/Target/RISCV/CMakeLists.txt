set(LLVM_TARGET_DEFINITIONS RISCV.td)

tablegen(LLVM RISCVGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM RISCVGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM RISCVGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM RISCVGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM RISCVGenCallingConv.inc -gen-callingconv)
tablegen(LLVM RISCVGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM RISCVGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM RISCVGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM RISCVGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM RISCVGenDisassemblerTables.inc -gen-disassembler)

add_public_tablegen_target(RISCVCommonTableGen)

add_llvm_target(RISCVCodeGen
  RISCVAsmPrinter.cpp
  RISCVCallingConv.cpp
  RISCVFrameLowering.cpp
  RISCVInstrInfo.cpp
  RISCVISelDAGToDAG.cpp
  RISCVISelLowering.cpp
  RISCVMCInstLower.cpp
  RISCVRegisterInfo.cpp
  RISCVSubtarget.cpp
  RISCVTargetMachine.cpp
  RISCVMachineFunctionInfo.cpp
  RISCVAnalyzeImmediate.cpp
  RISCVExpandPseudoInsts.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
