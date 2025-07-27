
	.globl	refreshmem_x8664
	.type	refreshmem_x8664, @function
refreshmem_x8664:
.LFB0:
	.cfi_startproc
        xorq    %rcx, %rcx
        xorq    %rdx, %rdx
        not     %rdx
        movq    %rdx, %xmm2
        jmp    .fin
.L1:
        movsd   (%rdi), %xmm0
        xorpd   %xmm1, %xmm1
        movsd   %xmm1, (%rdi)
        xorpd   %xmm2, %xmm1
        movsd   %xmm1, (%rdi)
        xorpd   %xmm2, %xmm1
        movsd   %xmm1, (%rdi)
        xorpd   %xmm2, %xmm0
        xorpd   %xmm2, %xmm0
        xorpd   %xmm2, %xmm0
        xorpd   %xmm2, %xmm0
        xorpd   %xmm2, %xmm0
        xorpd   %xmm2, %xmm0
        movsd   %xmm0, (%rdi)
        decq    %rsi
        addq    $8, %rdi
.fin:
        cmpq    %rsi, %rcx
        jl      .L1
        ret
	.cfi_endproc


	.globl	refresh64_x8664
	.type	refresh64_x8664, @function
refresh64_x8664:
	.cfi_startproc
        xorq    %rcx, %rcx
        jmp    .fin2
.L2:
        movq    %rcx, %xmm0
        movq    %xmm0, %xmm1
        movq    %xmm0, %xmm2
        movq    %xmm0, %xmm3
        movq    %xmm0, %xmm4
        movq    %xmm0, %xmm5
        movq    %xmm0, %xmm6
        movq    %xmm0, %xmm7
        movq    %xmm0, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm0, %xmm10
        movq    %xmm0, %xmm11
        movq    %xmm0, %xmm12
        movq    %xmm0, %xmm13
        movq    %xmm0, %xmm14
        movq    %xmm0, %xmm15
        movsd   (%rdi), %xmm0
        movsd   %xmm1, (%rdi)
        movq    %xmm0, %xmm1
        movq    %xmm0, %xmm2
        movq    %xmm0, %xmm3
        movq    %xmm0, %xmm4
        movq    %xmm0, %xmm5
        movq    %xmm0, %xmm6
        movq    %xmm0, %xmm7
        movq    %xmm0, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm0, %xmm10
        movq    %xmm0, %xmm11
        movq    %xmm0, %xmm12
        movq    %xmm0, %xmm13
        movq    %xmm0, %xmm14
        movq    %xmm0, %xmm15
        movq    %xmm7, %xmm0
        movq    %xmm8, %xmm1
        movq    %xmm9, %xmm2
        movq    %xmm10, %xmm3
        movq    %xmm11, %xmm4
        movq    %xmm12, %xmm5
        movq    %xmm13, %xmm6
        movq    %xmm14, %xmm7
        movq    %xmm15, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm1, %xmm10
        movq    %xmm2, %xmm11
        movq    %xmm3, %xmm12
        movq    %xmm4, %xmm13
        movq    %xmm5, %xmm14
        movq    %xmm6, %xmm15
        movsd   %xmm0, (%rdi)
        movsd   %xmm1, (%rdi)
        movsd   %xmm2, (%rdi)
        movsd   %xmm3, (%rdi)
        movsd   %xmm4, (%rdi)
        movsd   %xmm5, (%rdi)
        movsd   %xmm6, (%rdi)
        movsd   %xmm7, (%rdi)
        movsd   %xmm8, (%rdi)
        movsd   %xmm9, (%rdi)
        movsd   %xmm10, (%rdi)
        movsd   %xmm11, (%rdi)
        movsd   %xmm12, (%rdi)
        movsd   %xmm13, (%rdi)
        movsd   %xmm14, (%rdi)
        movsd   %xmm15, (%rdi)
        decq    %rsi
        addq    $8, %rdi
.fin2:
        cmpq    %rsi, %rcx
        jl      .L2
        ret
	.cfi_endproc


	.globl	cp64_x8664
	.type	cp64_x8664, @function
cp64_x8664:
	.cfi_startproc
        xorq    %rcx, %rcx
        jmp    .fin3
.L3:
        movq    %rcx, %xmm0
        movq    %xmm0, %xmm1
        movq    %xmm0, %xmm2
        movq    %xmm0, %xmm3
        movq    %xmm0, %xmm4
        movq    %xmm0, %xmm5
        movq    %xmm0, %xmm6
        movq    %xmm0, %xmm7
        movq    %xmm0, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm0, %xmm10
        movq    %xmm0, %xmm11
        movq    %xmm0, %xmm12
        movq    %xmm0, %xmm13
        movq    %xmm0, %xmm14
        movq    %xmm0, %xmm15
        movsd   (%rdi), %xmm0
        movsd   %xmm1, (%rdx)
        movq    %xmm0, %xmm1
        movq    %xmm0, %xmm2
        movq    %xmm0, %xmm3
        movq    %xmm0, %xmm4
        movq    %xmm0, %xmm5
        movq    %xmm0, %xmm6
        movq    %xmm0, %xmm7
        movq    %xmm0, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm0, %xmm10
        movq    %xmm0, %xmm11
        movq    %xmm0, %xmm12
        movq    %xmm0, %xmm13
        movq    %xmm0, %xmm14
        movq    %xmm0, %xmm15
        movq    %xmm7, %xmm0
        movq    %xmm8, %xmm1
        movq    %xmm9, %xmm2
        movq    %xmm10, %xmm3
        movq    %xmm11, %xmm4
        movq    %xmm12, %xmm5
        movq    %xmm13, %xmm6
        movq    %xmm14, %xmm7
        movq    %xmm15, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm1, %xmm10
        movq    %xmm2, %xmm11
        movq    %xmm3, %xmm12
        movq    %xmm4, %xmm13
        movq    %xmm5, %xmm14
        movq    %xmm6, %xmm15
        movsd   %xmm0, (%rdx)
        movsd   %xmm1, (%rdx)
        movsd   %xmm2, (%rdx)
        movsd   %xmm3, (%rdx)
        movsd   %xmm4, (%rdx)
        movsd   %xmm5, (%rdx)
        movsd   %xmm6, (%rdx)
        movsd   %xmm7, (%rdx)
        movsd   %xmm8, (%rdx)
        movsd   %xmm9, (%rdx)
        movsd   %xmm10, (%rdx)
        movsd   %xmm11, (%rdx)
        movsd   %xmm12, (%rdx)
        movsd   %xmm13, (%rdx)
        movsd   %xmm14, (%rdx)
        movsd   %xmm15, (%rdx)
        decq    %rsi
        addq    $8, %rdi
        addq    $8, %rdx
.fin3:
        cmpq    %rsi, %rcx
        jl      .L3
        ret
	.cfi_endproc


	.globl	memclean64_x8664
	.type	memclean64_x8664, @function
memclean64_x8664:
	.cfi_startproc
        xorq    %rcx, %rcx
        movq    %rcx, %xmm0
        movq    %xmm0, %xmm1
        movq    %xmm0, %xmm2
        movq    %xmm0, %xmm3
        movq    %xmm0, %xmm4
        movq    %xmm0, %xmm5
        movq    %xmm0, %xmm6
        movq    %xmm0, %xmm7
        movq    %xmm0, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm0, %xmm10
        movq    %xmm0, %xmm11
        movq    %xmm0, %xmm12
        movq    %xmm0, %xmm13
        movq    %xmm0, %xmm14
        movq    %xmm0, %xmm15
        movq    %xmm7, %xmm0
        movq    %xmm8, %xmm1
        movq    %xmm9, %xmm2
        movq    %xmm10, %xmm3
        movq    %xmm11, %xmm4
        movq    %xmm12, %xmm5
        movq    %xmm13, %xmm6
        movq    %xmm14, %xmm7
        movq    %xmm15, %xmm8
        movq    %xmm0, %xmm9
        movq    %xmm1, %xmm10
        movq    %xmm2, %xmm11
        movq    %xmm3, %xmm12
        movq    %xmm4, %xmm13
        movq    %xmm5, %xmm14
        movq    %xmm6, %xmm15
        jmp    .fin4
.L4:
        movsd   %xmm0, (%rdi)
        movsd   %xmm1, (%rdi)
        movsd   %xmm2, (%rdi)
        movsd   %xmm3, (%rdi)
        movsd   %xmm4, (%rdi)
        movsd   %xmm5, (%rdi)
        movsd   %xmm6, (%rdi)
        movsd   %xmm7, (%rdi)
        movsd   %xmm8, (%rdi)
        movsd   %xmm9, (%rdi)
        movsd   %xmm10, (%rdi)
        movsd   %xmm11, (%rdi)
        movsd   %xmm12, (%rdi)
        movsd   %xmm13, (%rdi)
        movsd   %xmm14, (%rdi)
        movsd   %xmm15, (%rdi)
        decq    %rsi
        addq    $8, %rdi
.fin4:
        cmpq    %rsi, %rcx
        jl      .L4
        ret
	.cfi_endproc


/* r:=["rax","rbx","rcx","rdx","rsi","rdi","rbp","rsp","r8","r10","r11","r12","r13","r14","r15"];  */

	.globl	cleanregs_x8664
	.type	cleanregs_x8664, @function
cleanregs_x8664:
	.cfi_startproc
        xorq    %r9, %r9
        not     %r9
        movq    %r9, %xmm15
        xorq    %r9, %rax
        xorq    %r9, %rax
        xorq    %r9, %rax
        xorq    %r9, %rax
        xorq    %r9, %rax
        xorq    %r9, %rax
        xorq    %r9, %rbx
        xorq    %r9, %rbx
        xorq    %r9, %rbx
        xorq    %r9, %rbx
        xorq    %r9, %rbx
        xorq    %r9, %rbx
        xorq    %r9, %rcx
        xorq    %r9, %rcx
        xorq    %r9, %rcx
        xorq    %r9, %rcx
        xorq    %r9, %rcx
        xorq    %r9, %rcx
        xorq    %r9, %rdx
        xorq    %r9, %rdx
        xorq    %r9, %rdx
        xorq    %r9, %rdx
        xorq    %r9, %rdx
        xorq    %r9, %rdx
        xorq    %r9, %rsi
        xorq    %r9, %rsi
        xorq    %r9, %rsi
        xorq    %r9, %rsi
        xorq    %r9, %rsi
        xorq    %r9, %rsi
        xorq    %r9, %rdi
        xorq    %r9, %rdi
        xorq    %r9, %rdi
        xorq    %r9, %rdi
        xorq    %r9, %rdi
        xorq    %r9, %rdi
        xorq    %r9, %rbp
        xorq    %r9, %rbp
        xorq    %r9, %rbp
        xorq    %r9, %rbp
        xorq    %r9, %rbp
        xorq    %r9, %rbp
        xorq    %r9, %rsp
        xorq    %r9, %rsp
        xorq    %r9, %rsp
        xorq    %r9, %rsp
        xorq    %r9, %rsp
        xorq    %r9, %rsp
        xorq    %r9, %r8
        xorq    %r9, %r8
        xorq    %r9, %r8
        xorq    %r9, %r8
        xorq    %r9, %r8
        xorq    %r9, %r8
        xorq    %r9, %r10
        xorq    %r9, %r10
        xorq    %r9, %r10
        xorq    %r9, %r10
        xorq    %r9, %r10
        xorq    %r9, %r10
        xorq    %r9, %r11
        xorq    %r9, %r11
        xorq    %r9, %r11
        xorq    %r9, %r11
        xorq    %r9, %r11
        xorq    %r9, %r11
        xorq    %r9, %r12
        xorq    %r9, %r12
        xorq    %r9, %r12
        xorq    %r9, %r12
        xorq    %r9, %r12
        xorq    %r9, %r12
        xorq    %r9, %r13
        xorq    %r9, %r13
        xorq    %r9, %r13
        xorq    %r9, %r13
        xorq    %r9, %r13
        xorq    %r9, %r13
        xorq    %r9, %r14
        xorq    %r9, %r14
        xorq    %r9, %r14
        xorq    %r9, %r14
        xorq    %r9, %r14
        xorq    %r9, %r14
        xorq    %r9, %r15
        xorq    %r9, %r15
        xorq    %r9, %r15
        xorq    %r9, %r15
        xorq    %r9, %r15
        xorq    %r9, %r15
        xorpd   %xmm15, %xmm0
        xorpd   %xmm15, %xmm0
        xorpd   %xmm15, %xmm0
        xorpd   %xmm15, %xmm0
        xorpd   %xmm15, %xmm0
        xorpd   %xmm15, %xmm0
        xorpd   %xmm15, %xmm1
        xorpd   %xmm15, %xmm1
        xorpd   %xmm15, %xmm1
        xorpd   %xmm15, %xmm1
        xorpd   %xmm15, %xmm1
        xorpd   %xmm15, %xmm1
        xorpd   %xmm15, %xmm2
        xorpd   %xmm15, %xmm2
        xorpd   %xmm15, %xmm2
        xorpd   %xmm15, %xmm2
        xorpd   %xmm15, %xmm2
        xorpd   %xmm15, %xmm2
        xorpd   %xmm15, %xmm3
        xorpd   %xmm15, %xmm3
        xorpd   %xmm15, %xmm3
        xorpd   %xmm15, %xmm3
        xorpd   %xmm15, %xmm3
        xorpd   %xmm15, %xmm3
        xorpd   %xmm15, %xmm4
        xorpd   %xmm15, %xmm4
        xorpd   %xmm15, %xmm4
        xorpd   %xmm15, %xmm4
        xorpd   %xmm15, %xmm4
        xorpd   %xmm15, %xmm4
        xorpd   %xmm15, %xmm5
        xorpd   %xmm15, %xmm5
        xorpd   %xmm15, %xmm5
        xorpd   %xmm15, %xmm5
        xorpd   %xmm15, %xmm5
        xorpd   %xmm15, %xmm5
        xorpd   %xmm15, %xmm6
        xorpd   %xmm15, %xmm6
        xorpd   %xmm15, %xmm6
        xorpd   %xmm15, %xmm6
        xorpd   %xmm15, %xmm6
        xorpd   %xmm15, %xmm6
        xorpd   %xmm15, %xmm7
        xorpd   %xmm15, %xmm7
        xorpd   %xmm15, %xmm7
        xorpd   %xmm15, %xmm7
        xorpd   %xmm15, %xmm7
        xorpd   %xmm15, %xmm7
        xorpd   %xmm15, %xmm8
        xorpd   %xmm15, %xmm8
        xorpd   %xmm15, %xmm8
        xorpd   %xmm15, %xmm8
        xorpd   %xmm15, %xmm8
        xorpd   %xmm15, %xmm8
        xorpd   %xmm15, %xmm9
        xorpd   %xmm15, %xmm9
        xorpd   %xmm15, %xmm9
        xorpd   %xmm15, %xmm9
        xorpd   %xmm15, %xmm9
        xorpd   %xmm15, %xmm9
        xorpd   %xmm15, %xmm10
        xorpd   %xmm15, %xmm10
        xorpd   %xmm15, %xmm10
        xorpd   %xmm15, %xmm10
        xorpd   %xmm15, %xmm10
        xorpd   %xmm15, %xmm10
        xorpd   %xmm15, %xmm11
        xorpd   %xmm15, %xmm11
        xorpd   %xmm15, %xmm11
        xorpd   %xmm15, %xmm11
        xorpd   %xmm15, %xmm11
        xorpd   %xmm15, %xmm11
        xorpd   %xmm15, %xmm12
        xorpd   %xmm15, %xmm12
        xorpd   %xmm15, %xmm12
        xorpd   %xmm15, %xmm12
        xorpd   %xmm15, %xmm12
        xorpd   %xmm15, %xmm12
        xorpd   %xmm15, %xmm13
        xorpd   %xmm15, %xmm13
        xorpd   %xmm15, %xmm13
        xorpd   %xmm15, %xmm13
        xorpd   %xmm15, %xmm13
        xorpd   %xmm15, %xmm13
        xorpd   %xmm15, %xmm14
        xorpd   %xmm15, %xmm14
        xorpd   %xmm15, %xmm14
        xorpd   %xmm15, %xmm14
        xorpd   %xmm15, %xmm14
        xorpd   %xmm15, %xmm14
        ret
	.cfi_endproc


.section .note.GNU-stack,"x",@progbits
