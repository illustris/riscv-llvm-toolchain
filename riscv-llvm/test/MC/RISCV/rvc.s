# RUN: llvm-mc %s -triple=riscv32imac -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv32imac < %s \
# RUN:     | llvm-objdump -d - | FileCheck -check-prefix=CHECK-INST %s

.LBB0_3:
# CHECK-INST: c.lwsp  ra, 12(sp)
# CHECK: encoding: [0xb2,0x40]
c.lwsp  ra, 12(sp)
# CHECK-INST: c.swsp  ra, 12(sp)
# CHECK: encoding: [0x06,0xc6]
c.swsp  ra, 12(sp)
# CHECK-INST: c.lw    a2, 4(a0)
# CHECK: encoding: [0x50,0x41]
c.lw    a2, 4(a0)
# CHECK-INST: c.sw    a5, 8(a3)
# CHECK: encoding: [0x9c,0xc6]
c.sw    a5, 8(a3)
# CHECK: encoding: [0bAAAAAA01,0b101AAAAA]
# CHECK:   fixup A - offset: 0, value: .LBB0_3, kind: fixup_riscv_rvc_jump
c.j     .LBB0_3
# CHECK-INST: c.j     -16
# CHECK: encoding: [0xe5,0xbf]
c.j     -16

.LBB0_2:
# CHECK-INST: c.jr    a7
# CHECK: encoding: [0x82,0x88]
c.jr    a7
# CHECK-INST: c.jalr  a1
# CHECK: encoding: [0x82,0x95]
c.jalr  a1
# CHECK: encoding: [0bAAAAAA01,0b110AAAAA]
# CHECK:   fixup A - offset: 0, value: .LBB0_2, kind: fixup_riscv_rvc_branch
c.beqz  a3, .LBB0_2
# CHECK: encoding: [0bAAAAAA01,0b111AAAAA]
# CHECK:   fixup A - offset: 0, value: .LBB0_2, kind: fixup_riscv_rvc_branch
c.bnez  a5, .LBB0_2
# CHECK-INST: c.beqz  a3, -8
# CHECK: encoding: [0xf5,0xde]
c.beqz  a3, -8
# CHECK-INST: c.bnez  a5, -12
# CHECK: encoding: [0xed,0xff]
c.bnez  a5, -12

# CHECK-INST: c.li  a7, 31
# CHECK: encoding: [0xfd,0x48]
c.li    a7, 31
# CHECK-INST: c.addi  a3, 15
# CHECK: encoding: [0xbd,0x06]
c.addi  a3, 15
# CHECK-INST: c.addi16sp  sp, -16
# CHECK: encoding: [0x7d,0x71]
c.addi16sp  sp, -16
# CHECK-INST: c.addi4spn  a3, sp, 16
# CHECK: encoding: [0x14,0x08]
c.addi4spn      a3, sp, 16
# CHECK-INST: c.slli  a1, 2
# CHECK: encoding: [0x8a,0x05]
c.slli  a1, 2
# CHECK-INST: c.srli  a3, 3
# CHECK: encoding: [0x8d,0x82]
c.srli  a3, 3
# CHECK-INST: c.srai  a4, 2
# CHECK: encoding: [0x09,0x87]
c.srai  a4, 2
# CHECK-INST: c.andi  a5, 15
# CHECK: encoding: [0xbd,0x8b]
c.andi  a5, 15
# CHECK-INST: c.mv    a7, s0
# CHECK: encoding: [0xa2,0x88]
c.mv    a7, s0
# CHECK-INST: c.and   a1, a2
# CHECK: encoding: [0xf1,0x8d]
c.and   a1, a2
# CHECK-INST: c.or    a2, a3
# CHECK: encoding: [0x55,0x8e]
c.or    a2, a3
# CHECK-INST: c.xor   a3, a4
# CHECK: encoding: [0xb9,0x8e]
c.xor   a3, a4
# CHECK-INST: c.sub   a4, a5
# CHECK: encoding: [0x1d,0x8f]
c.sub   a4, a5
# CHECK-INST: c.nop
# CHECK: encoding: [0x01,0x00]
c.nop
# CHECK-INST: c.ebreak
# CHECK: encoding: [0x02,0x90]
c.ebreak
# CHECK: encoding: [0bAAAAAA01,0b001AAAAA]
# CHECK:   fixup A - offset: 0, value: func1, kind: fixup_riscv_rvc_jump
c.jal   func1
# CHECK-INST: c.jal   256
# CHECK: encoding: [0x41,0x20]
c.jal   256
# CHECK-INST: c.lui   s0, 30
# CHECK: encoding: [0x79,0x64]
c.lui   s0, 30
