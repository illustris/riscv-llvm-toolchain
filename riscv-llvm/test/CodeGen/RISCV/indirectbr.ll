; RUN: llc -mtriple=riscv32 -verify-machineinstrs < %s | FileCheck %s

define i32 @indirectbr(i8* %target) nounwind {
; CHECK-LABEL: indirectbr:
; CHECK: jalr zero, a0, 0
; CHECK: .LBB0_1:
; CHECK: addi a0, zero, 0
; CHECK: jalr zero, ra, 0
  indirectbr i8* %target, [label %test_label]
test_label:
  br label %ret
ret:
  ret i32 0
}
