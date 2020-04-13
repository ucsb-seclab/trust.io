//  AREA NORMAL_IMAGE, CODE, READONLY
//.eabi_attribute 25, 1
.section .normal_world, "axS"

.set PSS_L2CC_BASE_ADDR, 0xF8F02000
.set EFUSEStaus,        (0xF800D000 + 0x10)            /*(XPS_EFUSE_BASEADDR + EFUSE_STATUS_OFFSET)*/
.set L2CCCrtl,		(PSS_L2CC_BASE_ADDR + 0x0100)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CNTRL_OFFSET)*/
.set CRValMmuCac,	0b01000000000101	/* Enable IDC, and MMU */


.global ns_image
.balign 32
ns_image:

	// SMC call
// 	SMC #0
//	BX lr

  //.incbin "../../gpio_test/gpio_test.mem"
//  ;INCBIN server.bin

// 	mov r2, #0xff
//	ldr r1, =#0x41200000
//	str r2, [r1]


	/* Enable mmu, icahce and dcache */
//	ldr	r0,=CRValMmuCac
//	mcr	p15,0,r0,c1,c0,0		/* Enable cache and MMU */
//	dsb					/* dsb	allow the MMU to start up */
//	isb					/* isb	flush prefetch buffer */

//	smc #0

//	mov r1, #0b1010
//	ldr r2, =#0x41200000
//	str r1, [r2]
//	nop
//	nop
//	nop
//	nop


//    ldr r0,=EFUSEStaus
//    ldr r1,[r0]                             /* Read eFuse setting */
//    ands r1,r1,#0x80                        /* Check whether device is having single core */

	// This completely screws us!!!
//    ldr	r0,=L2CCCrtl			/* Load L2CC base address base + control register */
//	mov	r1, #0				/* force the disable bit */
//	str	r1, [r0]			/* disable the L2 Caches */

.end
