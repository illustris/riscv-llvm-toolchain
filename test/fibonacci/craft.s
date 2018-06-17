	.text
	.file	"craft.c"
	.globl	craft                   # -- Begin function craft
	.p2align	2
	.type	craft,@function
craft:                                  # @craft
# BB#0:
	andi a0, a0, 0xffffffff
	andi a1, a1, 0xffffffff
	slli a2, a2, 32
	or a1, a2, a1
	slli a3, a3, 32
	or a0, a3, a0
	jalr	zero, ra, 0
.Lfunc_end0:
	.size	craft, .Lfunc_end0-craft
                                        # -- End function

	.ident	"clang version 5.0.0 "
	.section	".note.GNU-stack","",@progbits

