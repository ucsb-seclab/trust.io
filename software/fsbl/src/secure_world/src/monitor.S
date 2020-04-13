/*******
Basic secure monitor, ported from ARM's reference.
*******/

//PRESERVE8
.eabi_attribute 25, 1

  //AREA  Monitor_Codeode, CODE, ALIGN=5, READONLY
.section .MonitorCode, "axS"
.balign 32

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


.equ FSR_WRITE,	(1 << 11)
.equ ABT_IMPRECISE_EXT, 0b10000000110
.equ ABT_IMPRECISE_ERR, 0b10000001000

// ------------------------------------------------------------
// Monitor mode vector table
// ------------------------------------------------------------
.global handle_dabort
.type handle_dabort, "function"

.global monitor
monitor:
	// Monitor's Vector Table
	nop								// Reset      - not used by Monitor
	nop								// Undef      - not used by Monitor
	b			SMC_Handler
	b			PREFETCH			// Prefetch   - can by used by Monitor
	b			Interrupt_Handler	// Data abort - can by used by Monitor
	nop			// RESERVED
	b			IRQ					// IRQ        - can by used by Monitor
	b			FIQ					// FIQ        - can by used by Monitor

PREFETCH:
	b PREFETCH
IRQ:
	b IRQ
FIQ:
	b FIQ

.global Interrupt_Handler
Interrupt_Handler:
	// Nice writeup: http://www.ethernut.de/en/documents/arm-exceptions.html

	//srsdb	sp!, #Mode_MON

	// Save our Control Register
	//MRC p15, 0, r11, c1, c0, 0 // Read Control Register
	//ldr r0, =0x08c50078
	//MCR p15, 0, r0, c1, c0, 0 // Write Control Register

	// First, let's save all of our registers
	STMFD sp!, {r0-r12}

	// Go into secure world to access important bitstatus
	MRC     p15, 0, r0, c1, c1, 0
	BIC     r0, r0, #NS_BIT
	MCR     p15, 0, r0, c1, c1, 0
	ISB


	// Test to see if this is an imprecise abort
	// TODO: Figure out a way to handle these better?  Mabye binary modifications
	MRC p15, 0, r0, c5, c0, 0 // Read Data Fault Status Register

	// If it an imprecise abort, let's just ignore it...
	ldr r1, =#ABT_IMPRECISE_EXT
	and r0, r1
	teq r0, r1
	beq int_cleanup

	ldr r1, =#ABT_IMPRECISE_ERR
	and r0, r1
	teq r0, r1
	beq int_cleanup


	// Save our mode
	mrs    r12, spsr

	// Put DFAR in r0, the address that caused the exception
	mrc	p15, 0, r0, c6, c0, 0

	// Put DFSR in r1
	mrc	p15, 0, r1, c5, c0, 0

	// Get the instruction that caused the abort
	ldr r3, [lr, #-8]

	// get the register
	and r3, #0xf000
	lsr r3, #12

	// TODO: Check the instruction type (STR or LDR only)

	// Read or write?
	and r1, #FSR_WRITE
	tst r1, #FSR_WRITE
	bne write_reg
	beq read_reg
write_reg:
	ldr r4, [sp, r3, lsl #2] // SP + R3*4

	str r4, [r0]


	b int_cleanup
read_reg:
	// Let's do our read
	ldr r4, [r0]

	// Update the value on the stack
	str r4, [sp,r3, LSL #2] // SP + R3*4



int_cleanup:

	// Go back into non-secure world
	MRC     p15, 0, r0, c1, c1, 0
	EOR     r0, r0, #NS_BIT
	MCR     p15, 0, r0, c1, c1, 0

	// Restore our contorl register
	//MCR p15, 0, r11, c1, c0, 0 // Write Control Register

	LDMFD sp!, {r0-r12}

	isb
	dsb

	// Subtract 4 from our return address so that we actually go back to t
	//sub lr, #4
	// Perform exception return
  	// -------------------------
 	SUBS    pc, lr, #4


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


   // WTF is this doing?
   //mov r0, #0X0C00
   //MOVT R0, #0X0004
   //MCR p15, 0,R0, c1, c1, 2 // Write Non-secure Access Control Register data


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

  // Disable MMU in monitor mode
  //MRC p15, 0, r1, c1, c0, 0 // Read Control Register
  //EOR r1, r1, #0x1 			// Twiddle the M bit
  //MCR p15, 0, r1, c1, c0, 0 // Write Control Register

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

  MOV     r2,  #Mode_SVC               // Set initial mode for the Normal world as SVC
  LDR     r3,  =ns_image             // Set starting address as location on Normal world binary
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



  // Let's go to monitor mode!
  CPS    #Mode_MON

  // Let's setup our interrupts to all trap to monitor mode and enable CPSR to be modified
  mov r0, #0b011000					// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344h/Babeeidg.html
  MCR p15, 0, r0, c1, c1, 0

  // Let's give Normal world some access!
  ldr r0, =#0b1100000110000000000	// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0388f/CIHGCAFH.html
  //ldr r0, =#0xffffffff				// give them all the things!!
  MCR p15, 0, r0, c1, c1, 2;

  // Enable secure debugging
  ldr r0, =#0b11					// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344k/Babgcaae.html
  MCR p15, 0, r0, c1, c1, 1;

  // Enable the FPU in Non-secure world
  MRC p15, 0, r0, c1, c1, 2
  ORR r0, r0, #0b11<<10 // enable fpu
  MCR p15, 0, r0, c1, c1, 2
  //Set the CPACR for access to CP10 and CP11:
  LDR r0, =(0xF << 20)
  MCR p15, 0, r0, c1, c0, 2
  //Set the FPEXC EN bit to enable the FPU:
  MOV r3, #0x40000000
  VMSR FPEXC, r3

  // Let's give Normal world some access!
  //ldr r0, =#0b1111100110000000000			// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0388f/CIHGCAFH.html
  //MCR p15, 0, r0, c1, c1, 2;

  // ZC702 specific permissions
  // Ref: https://www.xilinx.com/support/documentation/user_guides/ug1019-zynq-trustzone.pdf



  // security2_sdio0
  ldr r1, =0xE0200008
  ldr r2, =0x1
  str r2, [r1]


  // security3_sdio1
  ldr r1, =0xE020000C
  ldr r2, =0x1
  str r2, [r1]

  // security4_qspi
  ldr r1, =0xE0200010
  ldr r2, =0x1
  str r2, [r1]

  // Set APB access to Non-secure (usb, ethernet, spi, can, etc.)
  ldr r1, =0xE0200018
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  // security7_smc
  ldr r1, =0xE020001C
  ldr r2, =0x1
  str r2, [r1]


/* BEGIN DMA -> NS: If these are uncommented (without an SMC call) XDmaPs_Example_W_Intr fails - Chad */
  // DMAC_RST_CTRL
  // Assert
  ldr r1, =0xF800020C
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

    // TZ_DMA_NS
  ldr r1, =0xF8000440
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  // TZ_DMA_IRQ_NS
  ldr r1, =0xF8000444
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  // TZ_DMA_PERIPH_NS
  ldr r1, =0xF8000448
  ldr r2, =0xFFFFFFFF
  str r2, [r1]


  // De-assert
  ldr r2, =0xFFFFFFFe
  str r2, [r1]


  // TZ_OCM_RAM0
  ldr r1, =0xF8000400
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  // TZ_OCM
  ldr r1, =0xF8000408
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  // TZ_OCM_RAM1
  ldr r1, =0xF8000404
  ldr r2, =0xFFFFFFFF
  str r2, [r1]

  // Make all of DDR non-secure
  mov r0, #0XFFFFFFFF
  ldr r2, =0xF8000430
  str r0, [r2]

  // TZ_GEM (Ethernet)
  ldr r2, =0xf8000450
  ldr r0, =0xFFFFFFFF
  str r0, [r2]

  // TZ_SDIO Make SDIO Non-secure
  ldr r2, =0xF8000454
  mov r0, #0X00000003
  str r0, [r2]


  //  Make USB Non-secure
  ldr r2, =0xF8000458
  mov r0, #0X00000003
  str r0, [r2]

  // TZ_FPGA_M should be both secure and non-secure by default

  // TZ_FPGA_AFI should be both secure and non-secure by default

  // Read our security values
  // This crashes the board, but should in theory show the AXPROT secure settings?
  //ldr r2, =0xF890001C
  //ldr r0, [r2]
  //ldr r2, =0xF8900020
  //ldr r1, [r2]

  // security_apb
  //ldr r2, =0xF8900028
  //mov r0, #0b111111
  //str r0, [r2]


  // Give the non-secure world some access to timers
  // Ref: file:///Users/cspensky/Downloads/ug585-Zynq-7000-TRM.pdf page: 1306
  ldr	r7, =0xf8f00054
  ldr	r6, =0b111111111111
  str	r6, [r7]


  // Disable Interrupts
  // ICDICER0 / GIC_DISABLE
  ldr	r7, =0xF8F01180
  ldr	r6, =0xffffffff
  str	r6, [r7]


  // Make them non-pending
  // ICDICPR0 / GIC_PENDING_CLR0
  ldr	r7, =0xF8F01280
  ldr	r6, =0xffffffff
  str	r6, [r7]

  // Set our priority mask
  // ICCPMR / GIC_CPU_PRIOR
  ldr	r7, =0xF8F00104
  ldr	r6, =0x80
  str	r6, [r7]

  // Enable some interrupts for NS
  // ICCICR (Page: 1318)
	/*
	 * If the CPU operates in both security domains, set parameters in the
	 * control_s register.
	 * 1. Set FIQen=1 to use FIQ for secure interrupts,
	 * 2. Program the AckCtl bit
	 * 3. Program the SBPR bit to select the binary pointer behavior
	 * 4. Set EnableS = 1 to enable secure interrupts
	 * 5. Set EnbleNS = 1 to enable non secure interrupts
	 */
  ldr	r7, =0xF8F00100
  ldr	r6, =0b1011
  str	r6, [r7]

  //ICDDCR

   ldr	r7, =0xf8f01000
  ldr	r6, =0b10
  str	r6, [r7]

  // ICDISR0
  ldr	r7, =0xf8f01080
  ldr	r6, =0xffff00ff
  str	r6, [r7]
  // ICDISR1
  ldr	r7, =0xf8f01084
  ldr	r6, =0xffffffff
  str	r6, [r7]
  // ICDISR2
  ldr	r7, =0xf8f01088
  ldr	r6, =0xffffffff
  str	r6, [r7]

  // ICDICFR0 (Page 1349)
  /*
  ldr	r7, =0xF8F01C00
  ldr	r6, =0xffff00ff
  str	r6, [r7]
*/


  // Set scu enable bit in scu
  ldr	r7, =0xf8f00000
  ldr	r0, [r7]
  orr	r0, r0, #0x1
  str	r0, [r7]

  // Invalidate SCU
  ldr	r7, =0xf8f0000c
  ldr	r6, =0xffff
  str	r6, [r7]


  // Drop back to System Mode
  CPS #Mode_SYS


/*
   mcr	 15,0,r0,cr7,cr5,0		// Invalidate Instruction cache
   mcr	 15,0,r0,cr7,cr5,6		// Invalidate branch predictor array

   dsb
   isb					// make sure it completes

   ldr	r4, =0
   mcr	 15,0,r4,cr1,cr0,0		// disable the ICache and MMU

   isb					// make sure it completes

*/

  // Return to the Secure world code
  // --------------------------------
  //b SMC_CALL
  //b ns_image
  //mov r0, #0x100000
  //Bx r0
  bx lr


// ------------------------------------------------------------
// Space reserved for stacks
// ------------------------------------------------------------

  //AREA  Monitor_Data, DATA, ALIGN=8, READWRITE
.section .MonitorData, "awS"
.balign 256

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
