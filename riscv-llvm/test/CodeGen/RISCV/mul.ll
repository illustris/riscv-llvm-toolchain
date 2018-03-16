; RUN: llc -mtriple=riscv32 -verify-machineinstrs < %s | FileCheck %s

define i32 @square(i32 %a) {
; CHECK-LABEL: square:
; CHECK: mul     a0, a0, a0
  %1 = mul i32 %a, %a
  ret i32 %1
}

define i32 @mul(i32 %a, i32 %b) {
; CHECK-LABEL: mul:
; CHECK: mul     a0, a0, a1
  %1 = mul i32 %a, %b
  ret i32 %1
}

define i32 @mul_constant(i32 %a) {
; CHECK-LABEL: mul_constant:
; CHECK: addi    a1, zero, 5
; CHECK: mul     a0, a0, a1
  %1 = mul i32 %a, 5
  ret i32 %1
}

define i32 @mul_pow2(i32 %a) {
; CHECK-LABEL: mul_pow2:
; CHECK: slli a0, a0, 3
; CHECK: jalr zero, ra, 0
  %1 = mul i32 %a, 8
  ret i32 %1
}

define i64 @mul64(i64 %a, i64 %b) {
; CHECK-LABEL: mul64:
; CHECK: mul     a3, a0, a3
; CHECK: mulhu   a4, a0, a2
; CHECK: add     a3, a4, a3
; CHECK: mul     a1, a1, a2
; CHECK: add     a1, a3, a1
; CHECK: mul     a0, a0, a2
  %1 = mul i64 %a, %b
  ret i64 %1
}

define i64 @mul64_constant(i64 %a) {
; CHECK-LABEL: mul64_constant:
; CHECK: addi    a2, zero, 5
; CHECK: mul     a1, a1, a2
; CHECK: mulhu   a3, a0, a2
; CHECK: add     a1, a3, a1
; CHECK: mul     a0, a0, a2
  %1 = mul i64 %a, 5
  ret i64 %1
}
