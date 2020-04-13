/*******
Non-secure ASM code
*******/

#ifdef TRUST_IO
//PRESERVE8
.eabi_attribute 25, 1

.section .NonsecureCode, "axS"
.balign 32

.global CRYPTO_CALL
.type CRYPTO_CALL, "function"
CRYPTO_CALL:
	push {lr}
	blx r10
	ldr r0, [r9]
	ldr r1, [r9, #+4]
	ldr r2, [r9, #+8]
	ldr r3, [r9, #+12]
	pop {lr}
	smc #0


.global ENABLE_INTERRUPTS
.type ENABLE_INTERRUPTS, "function"
ENABLE_INTERRUPTS:
  push {r0,r1}
  //ICDISER0
  ldr	r0, =0xF8F01100
  ldr	r1, =0xffffffff
  str	r1, [r0]
  //ICDISER1
  ldr	r0, =0xF8F01104
  ldr	r1, =0xffffffff
  str	r1, [r0]
  //ICDISER2
  ldr	r0, =0xF8F01108
  ldr	r1, =0xffffffff
  str	r1, [r0]
  pop {r0,r1}
  bx lr


.global get_read_verification
.type get_read_verification, "function"
get_read_verification:
  ldr r0, =READ_VERIFY
  //ldr r0, [r0]
  bx lr


.global READ_VERIFY
READ_VERIFY:
  .word 	0, 0, 0, 0

#endif
