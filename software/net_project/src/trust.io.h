/*
 * trust.io.h
 *
 *  Created on: Apr 17, 2018
 *      Author: cspensky
 */

#ifndef SRC_TRUST_IO_H_
#define SRC_TRUST_IO_H_


struct crypto_challenge {
	unsigned int DFAR;
	unsigned int DFSR;
	unsigned int value;
	unsigned int counter;
};

struct crypto_challenge ns_challenge;

#endif /* SRC_TRUST_IO_H_ */
