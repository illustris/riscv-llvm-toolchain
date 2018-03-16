# RUN: llvm-mc %s -triple=riscv64imac -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv64imac < %s \
# RUN:     | llvm-objdump -d - | FileCheck -check-prefix=CHECK-INST %s

.LBB0_3:
# CHECK-INST: c.ldsp  ra, 8(sp)
# CHECK: encoding: [0xa2,0x60]
c.ldsp  ra, 8(sp)
# CHECK-INST: c.sdsp  ra, 8(sp)
# CHECK: encoding: [0x06,0xe4]
c.sdsp  ra, 8(sp)
# CHECK-INST: c.lw    a3, 8(a2)
# CHECK: encoding: [0x14,0x46]
c.lw    a3, 8(a2)
# CHECK-INST: c.ld    a4, 16(a3)
# CHECK: encoding: [0x98,0x6a]
c.ld    a4, 16(a3)
# CHECK-INST: c.sw    a3, 8(a4)
# CHECK: encoding: [0x14,0xc7]
c.sw    a3, 8(a4)
# CHECK-INST: c.sd    a5, 24(a3)
# CHECK: encoding: [0x9c,0xee]
c.sd    a5, 24(a3)
# CHECK-INST: c.addiw a5, 24
# CHECK: encoding: [0xe1,0x27]
c.addiw a5, 24
# CHECK-INST: c.addw  a1, a2
# CHECK: encoding: [0xb1,0x9d]
c.addw a1, a2
# CHECK-INST: c.subw  a3, a4
# CHECK: encoding: [0x99,0x9e]
c.subw a3, a4
# CHECK-INST: c.lwsp  a1, 4(sp)
# CHECK: encoding: [0x92,0x45]
c.lwsp  a1, 4(sp)
# CHECK-INST: c.swsp  a4, 12(sp)
# CHECK: encoding: [0x3a,0xc6]
c.swsp  a4, 12(sp)

.LBB0_2:
# CHECK-INST: c.jalr  s0
# CHECK: encoding: [0x02,0x94]
c.jalr  s0
# CHECK: encoding: [0bAAAAAA01,0b101AAAAA]
# CHECK:   fixup A - offset: 0, value: .LBB0_3, kind: fixup_riscv_rvc_jump
c.j     .LBB0_3
# CHECK-INST: c.j     -12
# CHECK: encoding: [0xed,0xbf]
c.j     -12
# CHECK-INST: c.jr    a7
# CHECK: encoding: [0x82,0x88]
c.jr    a7
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
# CHECK-INST: c.lui   s0, 30
# CHECK: encoding: [0x79,0x64]
c.lui   s0, 30
