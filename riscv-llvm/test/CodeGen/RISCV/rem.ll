; RUN: llc -mtriple=riscv32 -verify-machineinstrs < %s | FileCheck %s

define i32 @urem(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: urem:
; CHECK: remu    a0, a0, a1
  %1 = urem i32 %a, %b
  ret i32 %1
}

define i32 @srem(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: srem:
; CHECK: rem     a0, a0, a1
  %1 = srem i32 %a, %b
  ret i32 %1
}
