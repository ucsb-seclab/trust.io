//#include <arm32.h>
//#include <interrupts.h>
//
//void handle_dabort() {
//	asm("push {r0-r10}");
//	// handle the data abort
//	uint32_t dfar, dfsr;
//	dfar = read_ns_dfar();
//	dfsr = read_ns_dfsr();
//	if(is_write_abort(dfsr)) {
//		// This is write fault
//	} else {
//		// read fault.
//	}
//	//TODO: finish this.
//
//	// Subtract 4 from our link register before returning
//	asm("ldr r3, [sp, #12]");
//	asm("sub r3, r3, #0x4");
//	asm("str r3, [sp, #12]");
//
//	// Restore our registers
//	asm("pop {r0-r10}");
//}
