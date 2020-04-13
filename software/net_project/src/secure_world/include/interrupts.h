/*
 * interrupts.h
 *
 *  Created on: Jun 20, 2017
 *      Author: machiry
 */

#ifndef SRC_SECURE_WORLD_INCLUDE_INTERRUPTS_H_
#define SRC_SECURE_WORLD_INCLUDE_INTERRUPTS_H_

#define FSR_WRITE (1 << 11)
int is_write_abort(uint32_t dfsr);

inline int is_write_abort(uint32_t dfsr) {
	return dfsr & FSR_WRITE;
}

#endif /* SRC_SECURE_WORLD_INCLUDE_INTERRUPTS_H_ */
