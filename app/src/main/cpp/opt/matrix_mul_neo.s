	.globl	matrix_multiply_4x4_neon_asm
matrix_multiply_4x4_neon_asm:
	ldr	q0, [x1]
	ldp	q1, q2, [x0]
	ldp	q3, q4, [x0, #32]
	fmul	v5.4s, v1.4s, v0.s[0]
	fmla	v5.4s, v2.4s, v0.s[1]
	fmla	v5.4s, v3.4s, v0.s[2]
	fmla	v5.4s, v4.4s, v0.s[3]
	str	q5, [x2]
	ldr	q0, [x1, #16]
	fmul	v5.4s, v1.4s, v0.s[0]
	fmla	v5.4s, v2.4s, v0.s[1]
	fmla	v5.4s, v3.4s, v0.s[2]
	fmla	v5.4s, v4.4s, v0.s[3]
	str	q5, [x2, #16]
	ldr	q0, [x1, #32]
	fmul	v5.4s, v1.4s, v0.s[0]
	fmla	v5.4s, v2.4s, v0.s[1]
	fmla	v5.4s, v3.4s, v0.s[2]
	fmla	v5.4s, v4.4s, v0.s[3]
	str	q5, [x2, #32]
	ldr	q0, [x1, #48]
	fmul	v1.4s, v1.4s, v0.s[0]
	fmla	v1.4s, v2.4s, v0.s[1]
	fmla	v1.4s, v3.4s, v0.s[2]
	fmla	v1.4s, v4.4s, v0.s[3]
	str	q1, [x2, #48]
	ret
	.byte   0x79
	.byte   0x6F
	.byte   0x75
	.byte   0x6E
	.byte   0x67
	.byte   0x70
	.byte   0x61
	.byte   0x6B
	.byte   0x5F
	.byte   0x36
	.byte   0x37
	.byte   0x31
	.byte   0x0A
