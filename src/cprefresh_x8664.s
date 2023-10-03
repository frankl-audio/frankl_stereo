
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
