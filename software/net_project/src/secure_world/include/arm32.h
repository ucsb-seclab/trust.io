/*
 * arm32.h
 *
 *  Created on: Jun 20, 2017
 *      Author: machiry
 */

#ifndef SRC_SECURE_WORLD_INCLUDE_ARM32_H_
#define SRC_SECURE_WORLD_INCLUDE_ARM32_H_
#include <stdint.h>


static inline uint32_t switch_to_NS() {
	uint32_t scr_reg, old_val;
	asm volatile ("mrc	p15, 0, %[scr], c1, c1, 0" : [scr] "=r" (scr_reg));
	old_val = scr_reg;
	// set the ns bit
	scr_reg |= 0x1;
	asm volatile ("mcr	p15, 0, %[scr], c1, c1, 0" : : [scr] "r" (scr_reg));
	return old_val;
}

static inline void write_scr(uint32_t scr_val) {
	asm volatile ("mcr	p15, 0, %[scr], c1, c1, 0" : : [scr] "r" (scr_val));
}

static inline uint32_t switch_to_S() {
	uint32_t scr_reg, old_val;
	asm volatile ("mrc	p15, 0, %[scr], c1, c1, 0" : [scr] "=r" (scr_reg));
	old_val = scr_reg;
	// clear the ns bit
	scr_reg &= ~(0x1);
	asm volatile ("mcr	p15, 0, %[scr], c1, c1, 0" : : [scr] "r" (scr_reg));
	return old_val;

}

static inline uint32_t read_ns_ifar(void) {
	uint32_t ifar, old_scr;
	old_scr = switch_to_S();
	asm volatile ("mrc	p15, 0, %[ifar], c6, c0, 2" : [ifar] "=r" (ifar));
	write_scr(old_scr);
	return ifar;
}

static inline uint32_t read_ns_dfar(void) {
	uint32_t dfar, old_scr;
	old_scr = switch_to_S();
	asm volatile ("mrc	p15, 0, %[dfar], c6, c0, 0" : [dfar] "=r" (dfar));
	write_scr(old_scr);
	return dfar;
}

static inline uint32_t read_ns_dfsr(void) {
	uint32_t dfsr, old_scr;
	old_scr = switch_to_S();
	asm volatile ("mrc	p15, 0, %[dfsr], c5, c0, 0" : [dfsr] "=r" (dfsr));
	write_scr(old_scr);
	return dfsr;
}

static inline uint32_t read_ns_ifsr(void) {
	uint32_t ifsr, old_scr;
	old_scr = switch_to_S();
	asm volatile ("mrc	p15, 0, %[ifsr], c5, c0, 1" : [ifsr] "=r" (ifsr));
	write_scr(old_scr);
	return ifsr;
}

static inline uint32_t read_ifar(void) {
	uint32_t ifar;

	asm volatile ("mrc	p15, 0, %[ifar], c6, c0, 2" : [ifar] "=r" (ifar));

	return ifar;
}

static inline uint32_t read_dfar(void) {
	uint32_t dfar;

	asm volatile ("mrc	p15, 0, %[dfar], c6, c0, 0" : [dfar] "=r" (dfar));

	return dfar;
}

static inline uint32_t read_dfsr(void) {
	uint32_t dfsr;

	asm volatile ("mrc	p15, 0, %[dfsr], c5, c0, 0" : [dfsr] "=r" (dfsr));

	return dfsr;
}

static inline uint32_t read_ifsr(void) {
	uint32_t ifsr;

	asm volatile ("mrc	p15, 0, %[ifsr], c5, c0, 1" : [ifsr] "=r" (ifsr));

	return ifsr;
}

#endif /* SRC_SECURE_WORLD_INCLUDE_ARM32_H_ */
