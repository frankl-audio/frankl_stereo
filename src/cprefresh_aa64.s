/*
  This code is the result of lengthy experiments.
    - we prefer store and load over various bit operations (e.g., even
      number of xor)
    - loading from zero register zxr is better than loading literal #0
    - the difference between using an x?? register over a d?? register
      is small, we prefer the latter
    - we prefer to zero a register before writing data to it
*/

/* refresh data in place */
	.arch armv8-a
	.text
	.align	2
	.global	refreshmem_aa64
	.type	refreshmem_aa64, %function
refreshmem_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x5, #1
.LOOP:
        cmp     x1, x5
        blt     .RET
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        add     x4, x4, 8
        add     x5, x5, 1
        b       .LOOP
.RET:
        ret
	.cfi_endproc


/* refresh 64 bit data in place, here we copy values from 24 registers to destination */
	.arch armv8-a
	.text
	.align	2
	.global	refresh64bit_aa64
	.type	refresh64bit_aa64, %function
refresh64bit_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x5, #1
.SLOOP:
        cmp     x1, x5
        blt     .SRET
        eor     v0.8b, v0.8b, v0.8b
        eor     v1.8b, v1.8b, v1.8b
        eor     v2.8b, v2.8b, v2.8b
        eor     v3.8b, v3.8b, v3.8b
        eor     v4.8b, v4.8b, v4.8b
        eor     v5.8b, v5.8b, v5.8b
        eor     v6.8b, v6.8b, v6.8b
        eor     v7.8b, v7.8b, v7.8b
        eor     v16.8b, v16.8b, v16.8b
        eor     v17.8b, v17.8b, v17.8b
        eor     v18.8b, v18.8b, v18.8b
        eor     v19.8b, v19.8b, v19.8b
        eor     v20.8b, v20.8b, v20.8b
        eor     v21.8b, v21.8b, v21.8b
        eor     v22.8b, v22.8b, v22.8b
        eor     v23.8b, v23.8b, v23.8b
        eor     v24.8b, v24.8b, v24.8b
        eor     v25.8b, v25.8b, v25.8b
        eor     v26.8b, v26.8b, v26.8b
        eor     v27.8b, v27.8b, v27.8b
        eor     v28.8b, v28.8b, v28.8b
        eor     v29.8b, v29.8b, v29.8b
        eor     v30.8b, v30.8b, v30.8b
        eor     v31.8b, v31.8b, v31.8b
        ldr     d0, [x4]
        str     xzr, [x4]
        str     xzr, [x4]
        orr     v0.8b, v0.8b, v0.8b
        orr     v1.8b, v0.8b, v0.8b
        orr     v2.8b, v0.8b, v0.8b
        orr     v3.8b, v0.8b, v0.8b
        orr     v4.8b, v0.8b, v0.8b
        orr     v5.8b, v0.8b, v0.8b
        orr     v6.8b, v0.8b, v0.8b
        orr     v7.8b, v0.8b, v0.8b
        orr     v16.8b, v0.8b, v0.8b
        orr     v17.8b, v0.8b, v0.8b
        orr     v18.8b, v0.8b, v0.8b
        orr     v19.8b, v0.8b, v0.8b
        orr     v20.8b, v0.8b, v0.8b
        orr     v21.8b, v0.8b, v0.8b
        orr     v22.8b, v0.8b, v0.8b
        orr     v23.8b, v0.8b, v0.8b
        orr     v24.8b, v0.8b, v0.8b
        orr     v25.8b, v0.8b, v0.8b
        orr     v26.8b, v0.8b, v0.8b
        orr     v27.8b, v0.8b, v0.8b
        orr     v28.8b, v0.8b, v0.8b
        orr     v29.8b, v0.8b, v0.8b
        orr     v30.8b, v0.8b, v0.8b
        orr     v31.8b, v0.8b, v0.8b
        orr     v0.8b, v0.8b, v31.8b
        orr     v1.8b, v1.8b, v0.8b
        orr     v2.8b, v2.8b, v1.8b
        orr     v3.8b, v3.8b, v2.8b
        orr     v4.8b, v4.8b, v3.8b
        orr     v5.8b, v5.8b, v4.8b
        orr     v6.8b, v6.8b, v5.8b
        orr     v7.8b, v7.8b, v6.8b
        orr     v16.8b, v16.8b, v7.8b
        orr     v17.8b, v17.8b, v16.8b
        orr     v18.8b, v18.8b, v17.8b
        orr     v19.8b, v19.8b, v18.8b
        orr     v20.8b, v20.8b, v19.8b
        orr     v21.8b, v21.8b, v20.8b
        orr     v22.8b, v22.8b, v21.8b
        orr     v23.8b, v23.8b, v22.8b
        orr     v24.8b, v24.8b, v23.8b
        orr     v25.8b, v25.8b, v24.8b
        orr     v26.8b, v26.8b, v25.8b
        orr     v27.8b, v27.8b, v26.8b
        orr     v28.8b, v28.8b, v27.8b
        orr     v29.8b, v29.8b, v28.8b
        orr     v30.8b, v30.8b, v29.8b
        orr     v31.8b, v31.8b, v30.8b
        orr     v0.8b, v0.8b, v16.8b
        orr     v1.8b, v1.8b, v17.8b
        orr     v2.8b, v2.8b, v18.8b
        orr     v3.8b, v3.8b, v19.8b
        orr     v4.8b, v4.8b, v20.8b
        orr     v5.8b, v5.8b, v21.8b
        orr     v6.8b, v6.8b, v22.8b
        orr     v7.8b, v7.8b, v23.8b
        orr     v16.8b, v16.8b, v24.8b
        orr     v17.8b, v17.8b, v25.8b
        orr     v18.8b, v18.8b, v26.8b
        orr     v19.8b, v19.8b, v27.8b
        orr     v20.8b, v20.8b, v28.8b
        orr     v21.8b, v21.8b, v29.8b
        orr     v22.8b, v22.8b, v30.8b
        orr     v23.8b, v23.8b, v31.8b
        orr     v24.8b, v24.8b, v0.8b
        orr     v25.8b, v25.8b, v1.8b
        orr     v26.8b, v26.8b, v2.8b
        orr     v27.8b, v27.8b, v3.8b
        orr     v28.8b, v28.8b, v4.8b
        orr     v29.8b, v29.8b, v5.8b
        orr     v30.8b, v30.8b, v6.8b
        orr     v31.8b, v31.8b, v7.8b
        str     xzr, [x4]
        str     d0, [x4]
        str     d1, [x4]
        str     d2, [x4]
        str     d3, [x4]
        str     d4, [x4]
        str     d5, [x4]
        str     d6, [x4]
        str     d7, [x4]
        str     d16, [x4]
        str     d17, [x4]
        str     d18, [x4]
        str     d19, [x4]
        str     d20, [x4]
        str     d21, [x4]
        str     d22, [x4]
        str     d23, [x4]
        str     d24, [x4]
        str     d25, [x4]
        str     d26, [x4]
        str     d27, [x4]
        str     d28, [x4]
        str     d29, [x4]
        str     d30, [x4]
        str     d31, [x4]
        str     d0, [x4]
        str     d1, [x4]
        str     d2, [x4]
        str     d3, [x4]
        str     d4, [x4]
        str     d5, [x4]
        str     d6, [x4]
        str     d7, [x4]
        str     d16, [x4]
        str     d17, [x4]
        str     d18, [x4]
        str     d19, [x4]
        str     d20, [x4]
        str     d21, [x4]
        str     d22, [x4]
        str     d23, [x4]
        str     d24, [x4]
        str     d25, [x4]
        str     d26, [x4]
        str     d27, [x4]
        str     d28, [x4]
        str     d29, [x4]
        str     d30, [x4]
        str     d31, [x4]
        add     x4, x4, 8
        add     x5, x5, 1
        b       .SLOOP
.SRET:
        ret
	.cfi_endproc



/* similar, but writing to different location */
	.text
	.align	2
	.global	cprefresh_aa64
	.type	cprefresh_aa64, %function
cprefresh_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x6, x2
        mov     x5, #1
.CLOOP:
        cmp     x1, x5
        blt     .CRET
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x6]
        str     xzr, [x6]
        str     d11, [x6]
        add     x4, x4, 8
        add     x6, x6, 8
        add     x5, x5, 1
        b       .CLOOP
.CRET:
        ret
	.cfi_endproc

/* copy 64 bit data, here we copy values from 24 registers to destination */
	.arch armv8-a
	.text
	.align	2
	.global	cp64bit_aa64
	.type	cp64bit_aa64, %function
cp64bit_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x6, x2
        mov     x5, #1
.TLOOP:
        cmp     x1, x5
        blt     .TRET
        eor     v0.8b, v0.8b, v0.8b
        eor     v1.8b, v1.8b, v1.8b
        eor     v2.8b, v2.8b, v2.8b
        eor     v3.8b, v3.8b, v3.8b
        eor     v4.8b, v4.8b, v4.8b
        eor     v5.8b, v5.8b, v5.8b
        eor     v6.8b, v6.8b, v6.8b
        eor     v7.8b, v7.8b, v7.8b
        eor     v16.8b, v16.8b, v16.8b
        eor     v17.8b, v17.8b, v17.8b
        eor     v18.8b, v18.8b, v18.8b
        eor     v19.8b, v19.8b, v19.8b
        eor     v20.8b, v20.8b, v20.8b
        eor     v21.8b, v21.8b, v21.8b
        eor     v22.8b, v22.8b, v22.8b
        eor     v23.8b, v23.8b, v23.8b
        eor     v24.8b, v24.8b, v24.8b
        eor     v25.8b, v25.8b, v25.8b
        eor     v26.8b, v26.8b, v26.8b
        eor     v27.8b, v27.8b, v27.8b
        eor     v28.8b, v28.8b, v28.8b
        eor     v29.8b, v29.8b, v29.8b
        eor     v30.8b, v30.8b, v30.8b
        eor     v31.8b, v31.8b, v31.8b
        ldr     d0, [x4]
        str     xzr, [x6]
        str     xzr, [x6]
        orr     v0.8b, v0.8b, v0.8b
        orr     v1.8b, v0.8b, v0.8b
        orr     v2.8b, v0.8b, v0.8b
        orr     v3.8b, v0.8b, v0.8b
        orr     v4.8b, v0.8b, v0.8b
        orr     v5.8b, v0.8b, v0.8b
        orr     v6.8b, v0.8b, v0.8b
        orr     v7.8b, v0.8b, v0.8b
        orr     v16.8b, v0.8b, v0.8b
        orr     v17.8b, v0.8b, v0.8b
        orr     v18.8b, v0.8b, v0.8b
        orr     v19.8b, v0.8b, v0.8b
        orr     v20.8b, v0.8b, v0.8b
        orr     v21.8b, v0.8b, v0.8b
        orr     v22.8b, v0.8b, v0.8b
        orr     v23.8b, v0.8b, v0.8b
        orr     v24.8b, v0.8b, v0.8b
        orr     v25.8b, v0.8b, v0.8b
        orr     v26.8b, v0.8b, v0.8b
        orr     v27.8b, v0.8b, v0.8b
        orr     v28.8b, v0.8b, v0.8b
        orr     v29.8b, v0.8b, v0.8b
        orr     v30.8b, v0.8b, v0.8b
        orr     v31.8b, v0.8b, v0.8b
        orr     v0.8b, v0.8b, v31.8b
        orr     v1.8b, v1.8b, v0.8b
        orr     v2.8b, v2.8b, v1.8b
        orr     v3.8b, v3.8b, v2.8b
        orr     v4.8b, v4.8b, v3.8b
        orr     v5.8b, v5.8b, v4.8b
        orr     v6.8b, v6.8b, v5.8b
        orr     v7.8b, v7.8b, v6.8b
        orr     v16.8b, v16.8b, v7.8b
        orr     v17.8b, v17.8b, v16.8b
        orr     v18.8b, v18.8b, v17.8b
        orr     v19.8b, v19.8b, v18.8b
        orr     v20.8b, v20.8b, v19.8b
        orr     v21.8b, v21.8b, v20.8b
        orr     v22.8b, v22.8b, v21.8b
        orr     v23.8b, v23.8b, v22.8b
        orr     v24.8b, v24.8b, v23.8b
        orr     v25.8b, v25.8b, v24.8b
        orr     v26.8b, v26.8b, v25.8b
        orr     v27.8b, v27.8b, v26.8b
        orr     v28.8b, v28.8b, v27.8b
        orr     v29.8b, v29.8b, v28.8b
        orr     v30.8b, v30.8b, v29.8b
        orr     v31.8b, v31.8b, v30.8b
        orr     v0.8b, v0.8b, v16.8b
        orr     v1.8b, v1.8b, v17.8b
        orr     v2.8b, v2.8b, v18.8b
        orr     v3.8b, v3.8b, v19.8b
        orr     v4.8b, v4.8b, v20.8b
        orr     v5.8b, v5.8b, v21.8b
        orr     v6.8b, v6.8b, v22.8b
        orr     v7.8b, v7.8b, v23.8b
        orr     v16.8b, v16.8b, v24.8b
        orr     v17.8b, v17.8b, v25.8b
        orr     v18.8b, v18.8b, v26.8b
        orr     v19.8b, v19.8b, v27.8b
        orr     v20.8b, v20.8b, v28.8b
        orr     v21.8b, v21.8b, v29.8b
        orr     v22.8b, v22.8b, v30.8b
        orr     v23.8b, v23.8b, v31.8b
        orr     v24.8b, v24.8b, v0.8b
        orr     v25.8b, v25.8b, v1.8b
        orr     v26.8b, v26.8b, v2.8b
        orr     v27.8b, v27.8b, v3.8b
        orr     v28.8b, v28.8b, v4.8b
        orr     v29.8b, v29.8b, v5.8b
        orr     v30.8b, v30.8b, v6.8b
        orr     v31.8b, v31.8b, v7.8b
        str     xzr, [x6]
        str     d0, [x6]
        str     d1, [x6]
        str     d2, [x6]
        str     d3, [x6]
        str     d4, [x6]
        str     d5, [x6]
        str     d6, [x6]
        str     d7, [x6]
        str     d16, [x6]
        str     d17, [x6]
        str     d18, [x6]
        str     d19, [x6]
        str     d20, [x6]
        str     d21, [x6]
        str     d22, [x6]
        str     d23, [x6]
        str     d24, [x6]
        str     d25, [x6]
        str     d26, [x6]
        str     d27, [x6]
        str     d28, [x6]
        str     d29, [x6]
        str     d30, [x6]
        str     d31, [x6]
        str     d0, [x6]
        str     d1, [x6]
        str     d2, [x6]
        str     d3, [x6]
        str     d4, [x6]
        str     d5, [x6]
        str     d6, [x6]
        str     d7, [x6]
        str     d16, [x6]
        str     d17, [x6]
        str     d18, [x6]
        str     d19, [x6]
        str     d20, [x6]
        str     d21, [x6]
        str     d22, [x6]
        str     d23, [x6]
        str     d24, [x6]
        str     d25, [x6]
        str     d26, [x6]
        str     d27, [x6]
        str     d28, [x6]
        str     d29, [x6]
        str     d30, [x6]
        str     d31, [x6]
        add     x4, x4, 8
        add     x6, x6, 8
        add     x5, x5, 1
        b       .TLOOP
.TRET:
        ret
	.cfi_endproc



/* similar, but zero and clean the interval */
	.text
	.align	2
	.global	memclean_aa64
	.type	memclean_aa64, %function
memclean_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x5, #1
.MLOOP:
        cmp     x1, x5
        blt     .MRET
        str     xzr, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        orr     v11.8b, v11.8b, v11.8b
        ldr     d11, [x4]
        str     xzr, [x4]
        str     d11, [x4]
        add     x4, x4, 8
        add     x5, x5, 1
        b       .MLOOP
.MRET:
        ret
	.cfi_endproc


/* similar, but zero and clean the interval */
	.text
	.align	2
	.global	clean64bit_aa64
	.type	clean64bit_aa64, %function
clean64bit_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x5, #1
        eor     v0.8b, v0.8b, v0.8b
        eor     v1.8b, v1.8b, v1.8b
        eor     v2.8b, v2.8b, v2.8b
        eor     v3.8b, v3.8b, v3.8b
        eor     v4.8b, v4.8b, v4.8b
        eor     v5.8b, v5.8b, v5.8b
        eor     v6.8b, v6.8b, v6.8b
        eor     v7.8b, v7.8b, v7.8b
        eor     v16.8b, v16.8b, v16.8b
        eor     v17.8b, v17.8b, v17.8b
        eor     v18.8b, v18.8b, v18.8b
        eor     v19.8b, v19.8b, v19.8b
        eor     v20.8b, v20.8b, v20.8b
        eor     v21.8b, v21.8b, v21.8b
        eor     v22.8b, v22.8b, v22.8b
        eor     v23.8b, v23.8b, v23.8b
        eor     v24.8b, v24.8b, v24.8b
        eor     v25.8b, v25.8b, v25.8b
        eor     v26.8b, v26.8b, v26.8b
        eor     v27.8b, v27.8b, v27.8b
        eor     v28.8b, v28.8b, v28.8b
        eor     v29.8b, v29.8b, v29.8b
        eor     v30.8b, v30.8b, v30.8b
        eor     v31.8b, v31.8b, v31.8b
        orr     v0.8b, v0.8b, v31.8b
        orr     v1.8b, v1.8b, v0.8b
        orr     v2.8b, v2.8b, v1.8b
        orr     v3.8b, v3.8b, v2.8b
        orr     v4.8b, v4.8b, v3.8b
        orr     v5.8b, v5.8b, v4.8b
        orr     v6.8b, v6.8b, v5.8b
        orr     v7.8b, v7.8b, v6.8b
        orr     v16.8b, v16.8b, v7.8b
        orr     v17.8b, v17.8b, v16.8b
        orr     v18.8b, v18.8b, v17.8b
        orr     v19.8b, v19.8b, v18.8b
        orr     v20.8b, v20.8b, v19.8b
        orr     v21.8b, v21.8b, v20.8b
        orr     v22.8b, v22.8b, v21.8b
        orr     v23.8b, v23.8b, v22.8b
        orr     v24.8b, v24.8b, v23.8b
        orr     v25.8b, v25.8b, v24.8b
        orr     v26.8b, v26.8b, v25.8b
        orr     v27.8b, v27.8b, v26.8b
        orr     v28.8b, v28.8b, v27.8b
        orr     v29.8b, v29.8b, v28.8b
        orr     v30.8b, v30.8b, v29.8b
        orr     v31.8b, v31.8b, v30.8b
        orr     v0.8b, v0.8b, v16.8b
        orr     v1.8b, v1.8b, v17.8b
        orr     v2.8b, v2.8b, v18.8b
        orr     v3.8b, v3.8b, v19.8b
        orr     v4.8b, v4.8b, v20.8b
        orr     v5.8b, v5.8b, v21.8b
        orr     v6.8b, v6.8b, v22.8b
        orr     v7.8b, v7.8b, v23.8b
        orr     v16.8b, v16.8b, v24.8b
        orr     v17.8b, v17.8b, v25.8b
        orr     v18.8b, v18.8b, v26.8b
        orr     v19.8b, v19.8b, v27.8b
        orr     v20.8b, v20.8b, v28.8b
        orr     v21.8b, v21.8b, v29.8b
        orr     v22.8b, v22.8b, v30.8b
        orr     v23.8b, v23.8b, v31.8b
        orr     v24.8b, v24.8b, v0.8b
        orr     v25.8b, v25.8b, v1.8b
        orr     v26.8b, v26.8b, v2.8b
        orr     v27.8b, v27.8b, v3.8b
        orr     v28.8b, v28.8b, v4.8b
        orr     v29.8b, v29.8b, v5.8b
        orr     v30.8b, v30.8b, v6.8b
        orr     v31.8b, v31.8b, v7.8b
.ZLOOP:
        cmp     x1, x5
        blt     .ZRET
        str     xzr, [x4]
        str     d0, [x4]
        str     d1, [x4]
        str     d2, [x4]
        str     d3, [x4]
        str     d4, [x4]
        str     d5, [x4]
        str     d6, [x4]
        str     d7, [x4]
        str     d16, [x4]
        str     d17, [x4]
        str     d18, [x4]
        str     d19, [x4]
        str     d20, [x4]
        str     d21, [x4]
        str     d22, [x4]
        str     d23, [x4]
        str     d24, [x4]
        str     d25, [x4]
        str     d26, [x4]
        str     d27, [x4]
        str     d28, [x4]
        str     d29, [x4]
        str     d30, [x4]
        str     d31, [x4]
        str     d0, [x4]
        str     d1, [x4]
        str     d2, [x4]
        str     d3, [x4]
        str     d4, [x4]
        str     d5, [x4]
        str     d6, [x4]
        str     d7, [x4]
        str     d16, [x4]
        str     d17, [x4]
        str     d18, [x4]
        str     d19, [x4]
        str     d20, [x4]
        str     d21, [x4]
        str     d22, [x4]
        str     d23, [x4]
        str     d24, [x4]
        str     d25, [x4]
        str     d26, [x4]
        str     d27, [x4]
        str     d28, [x4]
        str     d29, [x4]
        str     d30, [x4]
        str     d31, [x4]
        add     x4, x4, 8
        add     x5, x5, 1
        b       .ZLOOP
.ZRET:
        ret
	.cfi_endproc


/* multiple refreshs at once */
	.text
	.align	2
	.global	refreshmems_aa64
	.type	refreshmems_aa64, %function
refreshmems_aa64:
	.cfi_startproc
        mov     x4, x0
        mov     x5, #1
        mov     x6, #1
        mov     x7, xzr
        mvn     x7, x7
.COUNT:
        cmp     x2, x6
        blt     .IRET
.ILOOP:
        cmp     x1, x5
        blt     .ITER
        ldr     x3, [x4]
        eor     x3, x3, x7
        eor     x3, x3, x7
        eor     x3, x3, x7
        str     x3, [x4]
        eor     x3, x3, x7
        eor     x3, x3, x7
        eor     x3, x3, x7
        str     x3, [x4]
        add     x4, x4, 8
        add     x5, x5, 1
        b       .ILOOP
.ITER:
        add     x6, x6, 1
        mov     x4, x0
        mov     x5, #1
        mov     x7, xzr
        mvn     x7, x7
        b       .COUNT
.IRET:
        ret
	.cfi_endproc



	.text
	.align	2
	.global cleanregs_aa64	
	.type	cleanregs_aa64, %function
cleanregs_aa64:
	.cfi_startproc
        mov     x0, xzr
        mvn     x0, x0
        fmov   d0, x0
        fsub   d0, d0, d0
        fmov   d0, x0
        fsub   d0, d0, d0
        fmov   d0, x0
        fsub   d0, d0, d0
        fmov   d0, x0
        fsub   d0, d0, d0
        fmov   d1, x0
        fsub   d1, d1, d1
        fmov   d1, x0
        fsub   d1, d1, d1
        fmov   d1, x0
        fsub   d1, d1, d1
        fmov   d1, x0
        fsub   d1, d1, d1
        fmov   d2, x0
        fsub   d2, d2, d2
        fmov   d2, x0
        fsub   d2, d2, d2
        fmov   d2, x0
        fsub   d2, d2, d2
        fmov   d2, x0
        fsub   d2, d2, d2
        fmov   d3, x0
        fsub   d3, d3, d3
        fmov   d3, x0
        fsub   d3, d3, d3
        fmov   d3, x0
        fsub   d3, d3, d3
        fmov   d3, x0
        fsub   d3, d3, d3
        fmov   d4, x0
        fsub   d4, d4, d4
        fmov   d4, x0
        fsub   d4, d4, d4
        fmov   d4, x0
        fsub   d4, d4, d4
        fmov   d4, x0
        fsub   d4, d4, d4
        fmov   d5, x0
        fsub   d5, d5, d5
        fmov   d5, x0
        fsub   d5, d5, d5
        fmov   d5, x0
        fsub   d5, d5, d5
        fmov   d5, x0
        fsub   d5, d5, d5
        fmov   d6, x0
        fsub   d6, d6, d6
        fmov   d6, x0
        fsub   d6, d6, d6
        fmov   d6, x0
        fsub   d6, d6, d6
        fmov   d6, x0
        fsub   d6, d6, d6
        fmov   d7, x0
        fsub   d7, d7, d7
        fmov   d7, x0
        fsub   d7, d7, d7
        fmov   d7, x0
        fsub   d7, d7, d7
        fmov   d7, x0
        fsub   d7, d7, d7
        fmov   d8, x0
        fsub   d8, d8, d8
        fmov   d8, x0
        fsub   d8, d8, d8
        fmov   d8, x0
        fsub   d8, d8, d8
        fmov   d8, x0
        fsub   d8, d8, d8
        fmov   d9, x0
        fsub   d9, d9, d9
        fmov   d9, x0
        fsub   d9, d9, d9
        fmov   d9, x0
        fsub   d9, d9, d9
        fmov   d9, x0
        fsub   d9, d9, d9
        fmov   d10, x0
        fsub   d10, d10, d10
        fmov   d10, x0
        fsub   d10, d10, d10
        fmov   d10, x0
        fsub   d10, d10, d10
        fmov   d10, x0
        fsub   d10, d10, d10
        fmov   d11, x0
        fsub   d11, d11, d11
        fmov   d11, x0
        fsub   d11, d11, d11
        fmov   d11, x0
        fsub   d11, d11, d11
        fmov   d11, x0
        fsub   d11, d11, d11
        fmov   d12, x0
        fsub   d12, d12, d12
        fmov   d12, x0
        fsub   d12, d12, d12
        fmov   d12, x0
        fsub   d12, d12, d12
        fmov   d12, x0
        fsub   d12, d12, d12
        fmov   d13, x0
        fsub   d13, d13, d13
        fmov   d13, x0
        fsub   d13, d13, d13
        fmov   d13, x0
        fsub   d13, d13, d13
        fmov   d13, x0
        fsub   d13, d13, d13
        fmov   d14, x0
        fsub   d14, d14, d14
        fmov   d14, x0
        fsub   d14, d14, d14
        fmov   d14, x0
        fsub   d14, d14, d14
        fmov   d14, x0
        fsub   d14, d14, d14
        fmov   d15, x0
        fsub   d15, d15, d15
        fmov   d15, x0
        fsub   d15, d15, d15
        fmov   d15, x0
        fsub   d15, d15, d15
        fmov   d15, x0
        fsub   d15, d15, d15
        fmov   d16, x0
        fsub   d16, d16, d16
        fmov   d16, x0
        fsub   d16, d16, d16
        fmov   d16, x0
        fsub   d16, d16, d16
        fmov   d16, x0
        fsub   d16, d16, d16
        fmov   d17, x0
        fsub   d17, d17, d17
        fmov   d17, x0
        fsub   d17, d17, d17
        fmov   d17, x0
        fsub   d17, d17, d17
        fmov   d17, x0
        fsub   d17, d17, d17
        fmov   d18, x0
        fsub   d18, d18, d18
        fmov   d18, x0
        fsub   d18, d18, d18
        fmov   d18, x0
        fsub   d18, d18, d18
        fmov   d18, x0
        fsub   d18, d18, d18
        fmov   d19, x0
        fsub   d19, d19, d19
        fmov   d19, x0
        fsub   d19, d19, d19
        fmov   d19, x0
        fsub   d19, d19, d19
        fmov   d19, x0
        fsub   d19, d19, d19
        fmov   d20, x0
        fsub   d20, d20, d20
        fmov   d20, x0
        fsub   d20, d20, d20
        fmov   d20, x0
        fsub   d20, d20, d20
        fmov   d20, x0
        fsub   d20, d20, d20
        fmov   d21, x0
        fsub   d21, d21, d21
        fmov   d21, x0
        fsub   d21, d21, d21
        fmov   d21, x0
        fsub   d21, d21, d21
        fmov   d21, x0
        fsub   d21, d21, d21
        fmov   d22, x0
        fsub   d22, d22, d22
        fmov   d22, x0
        fsub   d22, d22, d22
        fmov   d22, x0
        fsub   d22, d22, d22
        fmov   d22, x0
        fsub   d22, d22, d22
        fmov   d23, x0
        fsub   d23, d23, d23
        fmov   d23, x0
        fsub   d23, d23, d23
        fmov   d23, x0
        fsub   d23, d23, d23
        fmov   d23, x0
        fsub   d23, d23, d23
        fmov   d24, x0
        fsub   d24, d24, d24
        fmov   d24, x0
        fsub   d24, d24, d24
        fmov   d24, x0
        fsub   d24, d24, d24
        fmov   d24, x0
        fsub   d24, d24, d24
        fmov   d25, x0
        fsub   d25, d25, d25
        fmov   d25, x0
        fsub   d25, d25, d25
        fmov   d25, x0
        fsub   d25, d25, d25
        fmov   d25, x0
        fsub   d25, d25, d25
        fmov   d26, x0
        fsub   d26, d26, d26
        fmov   d26, x0
        fsub   d26, d26, d26
        fmov   d26, x0
        fsub   d26, d26, d26
        fmov   d26, x0
        fsub   d26, d26, d26
        fmov   d27, x0
        fsub   d27, d27, d27
        fmov   d27, x0
        fsub   d27, d27, d27
        fmov   d27, x0
        fsub   d27, d27, d27
        fmov   d27, x0
        fsub   d27, d27, d27
        fmov   d28, x0
        fsub   d28, d28, d28
        fmov   d28, x0
        fsub   d28, d28, d28
        fmov   d28, x0
        fsub   d28, d28, d28
        fmov   d28, x0
        fsub   d28, d28, d28
        fmov   d29, x0
        fsub   d29, d29, d29
        fmov   d29, x0
        fsub   d29, d29, d29
        fmov   d29, x0
        fsub   d29, d29, d29
        fmov   d29, x0
        fsub   d29, d29, d29
        fmov   d30, x0
        fsub   d30, d30, d30
        fmov   d30, x0
        fsub   d30, d30, d30
        fmov   d30, x0
        fsub   d30, d30, d30
        fmov   d30, x0
        fsub   d30, d30, d30
        fmov   d31, x0
        fsub   d31, d31, d31
        fmov   d31, x0
        fsub   d31, d31, d31
        fmov   d31, x0
        fsub   d31, d31, d31
        fmov   d31, x0
        fsub   d31, d31, d31
        mvn     x0, x0
        mvn     x0, x0
        mvn     x0, x0
        mvn     x0, x0
        mvn     x1, x1
        mvn     x1, x1
        mvn     x1, x1
        mvn     x1, x1
        mvn     x2, x2
        mvn     x2, x2
        mvn     x2, x2
        mvn     x2, x2
        mvn     x3, x3
        mvn     x3, x3
        mvn     x3, x3
        mvn     x3, x3
        mvn     x4, x4
        mvn     x4, x4
        mvn     x4, x4
        mvn     x4, x4
        mvn     x5, x5
        mvn     x5, x5
        mvn     x5, x5
        mvn     x5, x5
        mvn     x6, x6
        mvn     x6, x6
        mvn     x6, x6
        mvn     x6, x6
        mvn     x7, x7
        mvn     x7, x7
        mvn     x7, x7
        mvn     x7, x7
        mvn     x8, x8
        mvn     x8, x8
        mvn     x8, x8
        mvn     x8, x8
        mvn     x9, x9
        mvn     x9, x9
        mvn     x9, x9
        mvn     x9, x9
        mvn     x10, x10
        mvn     x10, x10
        mvn     x10, x10
        mvn     x10, x10
        mvn     x11, x11
        mvn     x11, x11
        mvn     x11, x11
        mvn     x11, x11
        mvn     x12, x12
        mvn     x12, x12
        mvn     x12, x12
        mvn     x12, x12
        mvn     x13, x13
        mvn     x13, x13
        mvn     x13, x13
        mvn     x13, x13
        mvn     x14, x14
        mvn     x14, x14
        mvn     x14, x14
        mvn     x14, x14
        mvn     x15, x15
        mvn     x15, x15
        mvn     x15, x15
        mvn     x15, x15
        mvn     x16, x16
        mvn     x16, x16
        mvn     x16, x16
        mvn     x16, x16
        mvn     x17, x17
        mvn     x17, x17
        mvn     x17, x17
        mvn     x17, x17
        mvn     x18, x18
        mvn     x18, x18
        mvn     x18, x18
        mvn     x18, x18
        mvn     x19, x19
        mvn     x19, x19
        mvn     x19, x19
        mvn     x19, x19
        mvn     x20, x20
        mvn     x20, x20
        mvn     x20, x20
        mvn     x20, x20
        mvn     x21, x21
        mvn     x21, x21
        mvn     x21, x21
        mvn     x21, x21
        mvn     x22, x22
        mvn     x22, x22
        mvn     x22, x22
        mvn     x22, x22
        mvn     x23, x23
        mvn     x23, x23
        mvn     x23, x23
        mvn     x23, x23
        mvn     x24, x24
        mvn     x24, x24
        mvn     x24, x24
        mvn     x24, x24
        mvn     x25, x25
        mvn     x25, x25
        mvn     x25, x25
        mvn     x25, x25
        mvn     x26, x26
        mvn     x26, x26
        mvn     x26, x26
        mvn     x26, x26
        mvn     x27, x27
        mvn     x27, x27
        mvn     x27, x27
        mvn     x27, x27
        mvn     x28, x28
        mvn     x28, x28
        mvn     x28, x28
        mvn     x28, x28
        mvn     x29, x29
        mvn     x29, x29
        mvn     x29, x29
        mvn     x29, x29
        mvn     x30, x30
        mvn     x30, x30
        mvn     x30, x30
        mvn     x30, x30
        mvn     x30, x30
        mvn     x30, x30
        mvn     x30, x30
        mvn     x30, x30
        mvn     x29, x29
        mvn     x29, x29
        mvn     x29, x29
        mvn     x29, x29
        mvn     x28, x28
        mvn     x28, x28
        mvn     x28, x28
        mvn     x28, x28
        mvn     x27, x27
        mvn     x27, x27
        mvn     x27, x27
        mvn     x27, x27
        mvn     x26, x26
        mvn     x26, x26
        mvn     x26, x26
        mvn     x26, x26
        mvn     x25, x25
        mvn     x25, x25
        mvn     x25, x25
        mvn     x25, x25
        mvn     x24, x24
        mvn     x24, x24
        mvn     x24, x24
        mvn     x24, x24
        mvn     x23, x23
        mvn     x23, x23
        mvn     x23, x23
        mvn     x23, x23
        mvn     x22, x22
        mvn     x22, x22
        mvn     x22, x22
        mvn     x22, x22
        mvn     x21, x21
        mvn     x21, x21
        mvn     x21, x21
        mvn     x21, x21
        mvn     x20, x20
        mvn     x20, x20
        mvn     x20, x20
        mvn     x20, x20
        mvn     x19, x19
        mvn     x19, x19
        mvn     x19, x19
        mvn     x19, x19
        mvn     x18, x18
        mvn     x18, x18
        mvn     x18, x18
        mvn     x18, x18
        mvn     x17, x17
        mvn     x17, x17
        mvn     x17, x17
        mvn     x17, x17
        mvn     x16, x16
        mvn     x16, x16
        mvn     x16, x16
        mvn     x16, x16
        mvn     x15, x15
        mvn     x15, x15
        mvn     x15, x15
        mvn     x15, x15
        mvn     x14, x14
        mvn     x14, x14
        mvn     x14, x14
        mvn     x14, x14
        mvn     x13, x13
        mvn     x13, x13
        mvn     x13, x13
        mvn     x13, x13
        mvn     x12, x12
        mvn     x12, x12
        mvn     x12, x12
        mvn     x12, x12
        mvn     x11, x11
        mvn     x11, x11
        mvn     x11, x11
        mvn     x11, x11
        mvn     x10, x10
        mvn     x10, x10
        mvn     x10, x10
        mvn     x10, x10
        mvn     x9, x9
        mvn     x9, x9
        mvn     x9, x9
        mvn     x9, x9
        mvn     x8, x8
        mvn     x8, x8
        mvn     x8, x8
        mvn     x8, x8
        mvn     x7, x7
        mvn     x7, x7
        mvn     x7, x7
        mvn     x7, x7
        mvn     x6, x6
        mvn     x6, x6
        mvn     x6, x6
        mvn     x6, x6
        mvn     x5, x5
        mvn     x5, x5
        mvn     x5, x5
        mvn     x5, x5
        mvn     x4, x4
        mvn     x4, x4
        mvn     x4, x4
        mvn     x4, x4
        mvn     x3, x3
        mvn     x3, x3
        mvn     x3, x3
        mvn     x3, x3
        mvn     x2, x2
        mvn     x2, x2
        mvn     x2, x2
        mvn     x2, x2
        mvn     x1, x1
        mvn     x1, x1
        mvn     x1, x1
        mvn     x1, x1
        mvn     x0, x0
        mvn     x0, x0
        mvn     x0, x0
        mvn     x0, x0
        ret
	.cfi_endproc


.section .note.GNU-stack,"x",@progbits
