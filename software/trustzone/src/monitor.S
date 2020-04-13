/*******
Basic secure monitor, ported from ARM's reference.
*******/

//PRESERVE8
.eabi_attribute 25, 1

  //AREA  Monitor_Codeode, CODE, ALIGN=5, READONLY
.section .text, "x"
.balign 4

// ------------------------------------------------------------
// Define some values
// ------------------------------------------------------------

// - Standard definitions of mode bits and interrupt (I&F) flags in PSRs
.equ Mode_USR,	0x10
.equ Mode_FIQ,	0x11
.equ Mode_IRQ,	0x12
.equ Mode_SVC,	0x13
.equ Mode_ABT,	0x17
.equ Mode_UNDEF,0x1B
.equ Mode_SYS,	0x1F
.equ Mode_MON,	0x16
.equ I_Bit,		0x80 // when I bit is set, IRQ is disabled
.equ F_Bit,		0x40 // when F bit is set, FIQ is disabled


.equ TTB_COHERENT,		0x00014c06  // Template descriptor for coherent memory
.equ TTB_NONCOHERENT,	0x00000c1e  // Template descriptor for non-coherent memory
.equ TTB_DEVICE,		0x00000c06  // Template descriptor for device memory

.equ NS_BIT,	0x1


// ------------------------------------------------------------
// Monitor mode vector table
// ------------------------------------------------------------

.global monitor
monitor:
	// Monitor's
	nop			// Reset      - not used by Monitor
	nop			// Undef      - not used by Monitor
	b			SMC_Handler
	nop			// Prefetch   - can by used by Monitor
	nop			// Data abort - can by used by Monitor
	nop			// RESERVED
	nop			// IRQ        - can by used by Monitor
	nop			// FIQ        - can by used by Monitor

// ------------------------------------------------------------
// SMC Handler
//
// - Detect which world executed SMC
// - Saves state to appropiate stack
// - Restores other worlds state
// - Switches world
// - Performs exception return
// ------------------------------------------------------------

//  EXPORT switchocm
//switchocm
//	push {r7}
//	ldr	r7, =0xf8000404
//	ldr	r0, [r7]
//	orr	r0, r0, #0xffff0000
//	str	r0, [r7]
//	pop {r7}
//	BX lr


.global SMC_CALL
.type SMC_CALL, "function"
SMC_CALL:
	SMC #0
	BX lr

.global SMC_Handler
.type SMC_Handler, "function"
SMC_Handler: //PROC {r4-r12}
  PUSH   {r0-r3}                       // r0-r3 contain args to be passed between worlds
                                       // Temporarily stack, so can be used as scratch regs

  // Which world have we come from
  // ------------------------------
  MRC     p15, 0, r0, c1, c1, 0        // Read Secure Configuration Register data
  TST     r0, #NS_BIT                  // Is the NS bit set?
  PUSH    {r0}                         // Save a copy of the SCR on entry
  BIC     r0, r0, #NS_BIT              // Make sure the SCR.NS bit is now clear
  MCR     p15, 0, r0, c1, c1, 0        // Write Secure Configuration Register data
  ISB

  // Load save to pointer
  // ---------------------
  LDREQ   r0, =S_STACK_SP             // If NS bit set, was in Normal world.  So restore Secure state
  LDRNE   r0, =NS_STACK_SP
  LDR     r2, [r0]

  // Load restore from pointer
  // --------------------------
  LDREQ   r1, =NS_STACK_SP
  LDRNE   r1, =S_STACK_SP
  LDR     r3, [r1]

  // r2  <-- save to
  // r3  <-- restore from

  // Save general purpose registers, SPSR, LR and SP
  // ------------------------------------------------
  STMFD   r2!, {r4-r12}               // Save r4 to r12
  MRS     r4, spsr                    // Also get a copy of the SPSR_mon
  STMFD   r2!, {r4, lr}               // Save original SPSR_mon and LR_mon

  CPS     #Mode_SVC                   // Switch into Supervisor mode temporarily
  MOV     r4, sp                      // Temp for SP_svc
  STMFD   r2!, {r4, lr}               // Save SP_svc and LR_svc

  STR     r2, [r0]                    // Save updated pointer back, r0 and r2 now free

  // Restore other world's registers, SPSR and LR
  // ---------------------------------------------
  LDMFD   r3!, {r4, lr}               // Restore SP_svc and LR_svc
  MOV     sp, r4                      // Temp for SP_svc
  CPS     #Mode_MON                   // Switch back into Monitor mode

  LDMFD   r3!, {r0, lr}               // Get SPSR_mon and LR_mon (return address)
  MSR     spsr_cxsf, r0               // Restore SPSR_mon
  LDMFD   r3!, {r4-r12}               // Restore registers r4 to r12

  STR     r3, [r1]                    // Save updated pointer back, r1 and r3 now free

  // Clear local monitor
  // --------------------
  CLREX                               // Not strictly required in this example, as not using LDREX/STREX
                                      // However, architecturally should execute CLREX on a context switch

  // Set SCR.NS bit correctly for world we're entering
  // --------------------------------------------------
  POP     {r0}
  EOR     r0, r0, #NS_BIT              // Toggle NS bit
  MCR     p15, 0, r0, c1, c1, 0        // Write Secure Configuration Register data


  // Now restore args (r0-r3)
  // -------------------------
  POP     {r0-r3}

//switch reg stuff
  mov r0, #0XFFFFFFFF
  ldr r2, =0xF8000430
  str r0, [r2]


//tz apb gem stuff

  //#set APB access to Non-secure
  ldr r1, =0xE0200018
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  //GEM
  mov r0, #0X00000003
  ldr r2, =0xf8000450
  str r0, [r2]

 // 	/*set scu enable bit in scu*/
	ldr	r7, =0xf8f00000
	ldr	r0, [r7]
	orr	r0, r0, #0x1
	str	r0, [r7]

//	/*invalidate scu*/
	ldr	r7, =0xf8f0000c
	ldr	r6, =0xffff
	str	r6, [r7]


  mov r0, #0X0C00
  MOVT R0, #0X0004
  MCR p15, 0,R0, c1, c1, 2 // Write Non-secure Access Control Register data


  // disable IRQ


  // Perform exception return
  // -------------------------
  MOVS    pc, lr

  //ENDP

// ------------------------------------------------------------
// Monitor Initialization
//
// This is called the first time the Secure world wishes to
// move to the Normal world.
// ------------------------------------------------------------

.global monitorInit
.type monitorInit, "function"
monitorInit: //PROC

  // Install Secure Monitor
  // -----------------------
  LDR     r0, =monitor                 // Get address of Monitor's vector table
  MCR     p15, 0, r0, c12, c0, 1       // Write Monitor Vector Base Address Register

  // Initialize the Monitor mode stack pointer
  .global Image$$MON_STACK$$ZI$$Limit
  CPS    #Mode_MON

  LDR    sp, =MON_STACK_LIMIT
  CPS    #Mode_SVC

  // Create and save a "dummy" Normal world state
  // This will be used for the first entry to the Normal world
  LDR     r1, =NS_STACK_LIMIT          // Get top of Normal state stack (assuming FD model)

  MOV     r2,  #0
  MOV     r3,  #0

  STMFD   r1!, {r2-r3}                 // Save off "dummy" values for r11-r12
  STMFD   r1!, {r2-r3}                 // Save off "dummy" values for r9-r10
  STMFD   r1!, {r2-r3}                 // Save off "dummy" values for r7-r8
  STMFD   r1!, {r2-r3}                 // Save off "dummy" values for r5-r6
  STMFD   r1!, {r2}                    // Save off "dummy" values for r4
  // NOT saving value for r0-r3, as we are using these registers to pass information between worlds

  .global  ns_image
  MOV     r2,  #Mode_SVC               // Set initial mode for the Normal world as SVC
  LDR     r3,  =0x100000              // Set starting address as location on Normal world binary
  STMFD   r1!, {r2-r3}
  MOV     r2,  #0                      // Set initial SP_svc for the Normal world
  MOV     r3,  #0                      // Set initial LR_svc for the Normal world
  STMFD   r1!, {r2,r3}

  LDR     r0, =NS_STACK_SP
  STR     r1, [r0]                     // Save a pointer to the top of the Normal world context


  // Set up the pointer to the Secure world memory saved state
  // This buffer is currently empty, as the Secure world is running
  LDR     r0, =S_STACK_LIMIT           // Get address of memory for saving Secure state
  LDR     r1, =S_STACK_SP              // Get address of the pointer to saved Secure state
  STR     r0, [r1]                     // Buffer currently empty, so pointer points to the top of the stack (FD stack model)

  // Return to the Secure world code
  // --------------------------------
  BX      lr


// ------------------------------------------------------------
// Space reserved for stacks
// ------------------------------------------------------------

  //AREA  Monitor_Data, DATA, ALIGN=8, READWRITE
.section .data, "aw"
.balign 8

  // Reserving some memory to save Normal world state information
NS_STACK_BASE:
  .word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NS_STACK_LIMIT:

  // Reserving some memory to save Secure world state information
S_STACK_BASE:
  .word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
S_STACK_LIMIT:

  // Reserving some memory to save Secure world state information
MON_STACK_BASE:
  .word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
MON_STACK_LIMIT:


  // This will be used as a pointer to the top of the saved Normal world state
NS_STACK_SP:
  .word     0

  // This will be used as a pointer to the top of the saved Secure world state
S_STACK_SP:
  .word     0

MON_STACK_SP:
  .word     0

.end


// ------------------------------------------------------------
// End
// ------------------------------------------------------------
