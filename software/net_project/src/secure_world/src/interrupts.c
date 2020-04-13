
#include "lwip/tcp.h"
#include "aes.h"
#include "aes_tio.h"
#include "../../trust.io.h"
//#include "dh.h"

#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif

#ifdef TRUST_IO

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


__monitor_data extern int crypto_callback_addr = 0;
__monitor_data extern int updating_key = 0;
__monitor_data static int GLOBAL_TIMER = 0xF8F00200;
__monitor_data static int BUTTON_ADDR = 0x41210000;
__monitor_data static int CRYPTO_KEY = 0x01020304;
__monitor_data static struct crypto_challenge challenge;
__monitor_data static struct AES_ctx ctx;
__monitor_data static int AES_initialized = 0;
__monitor_data static int counter = 0;
__monitor_data static int dh_sent = 0;


//__monitor_data2 DH_KEY local_private = {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76};
//__monitor_data2 DH_KEY local_public = {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0};
//__monitor_data static struct crypto_challenge remote_public;

__monitor_data int protect_array[] = {0x41200000};



__monitor_code static INLINE u32 read_addr(UINTPTR Addr)
{
	return *(volatile u32 *) Addr;
}

__monitor_code int get_key() {
	return CRYPTO_KEY & 0x7fffffff;
}

__monitor_code void imprecise_abort(int dfar, int lr) {
#ifdef DEBUG
		xil_printf("ERROR: Imprecise data abort %08X @ %08X!!\r\n", dfar, lr);
#endif
//	while(1);
}

__monitor_code void secure_irq() {
	xil_printf("ERROR: Secure IRQ!!\r\n");

	while(1);
}

__monitor_code int handle_data_abort(int dfar, int dfsr, struct tcp_pcb *tpcb) {
#ifdef DEBUG
		xil_printf("Data Abort: %08X %08x\r\n", dfar, dfsr);
#endif

	for (int i = 0; i < sizeof(protect_array); i++) {
		if (dfar == protect_array[i]) {
			return 1;
		}
	}
	return 0;
}

__monitor_code int check_response(int dfar, int dfsr, int value, int counter) {

	int response = 0;

#ifdef NOCRYPTO
	return 1;
#endif
#ifdef DEBUG
	xil_printf("Checking response...\r\n");
#endif
//	if (updating_key == 3) {
//		xil_printf("Generating new AES key...\r\n");
//		remote_public.DFAR = dfar;
//		remote_public.DFSR = dfsr;
//		remote_public.challenge = challenge_val;
//		remote_public.counter = counter;
//		DH_generate_key_secret(key, local_private, &remote_public);
//		updating_key = 4;
//		AES_initialized = 0;
//		return 0;
//	} else if (updating_key == 1) {
//		if (dfar == 0xffffffff &&
//				dfsr == 0xffffffff &&
//				challenge_val == 0xffffffff &&
//				counter == 0xffffffff) {
//
//			xil_printf("Generating DH keys...\r\n");
//			/*Alice generate her private key and public key */
//			DH_generate_key_pair(local_public, local_private);
//
//			memcpy(&ns_challenge, local_public, sizeof(ns_challenge));
//
//
//			updating_key = 2;
//			return 0;
//		}
//	}

	ns_challenge.DFAR = dfar;
	ns_challenge.DFSR = dfsr;
	ns_challenge.value = value;
	ns_challenge.counter = counter;

	if (updating_key) {
		AES_CBC_decrypt_buffer(&ctx, (uint8_t*) &ns_challenge, sizeof(ns_challenge));
		memcpy(key, &ns_challenge, sizeof(key));
		xil_printf("Key updated! %08X%08X%08X%08X\r\n", ns_challenge.DFAR,
				ns_challenge.DFSR,
				ns_challenge.value,
				ns_challenge.counter);

		updating_key = 2;
		AES_initialized = 0;
	}

	AES_CBC_decrypt_buffer(&ctx, (uint8_t*) &ns_challenge, sizeof(ns_challenge));

	response = (ns_challenge.DFAR == challenge.DFAR &&
			ns_challenge.DFSR == challenge.DFSR &&
			ns_challenge.value == challenge.value &&
			ns_challenge.counter == challenge.counter+1);
#ifdef DEBUG
	xil_printf("Crypto response: %d\r\n", response);
#endif
	return response;
}


//__monitor_code void* verified_read(unsigned int dfar, unsigned int read_value, int* verify_value) {
//#ifdef NOCRYPTO
//	return verify_value;
//#endif
//	if (!AES_initialized) {
//		AES_init_ctx_iv(&ctx, key, init_vector);
//		AES_initialized = 1;
//	}
//
//	// Check to make sure there was a successful auth already, for the same memory region
//	if (!(ns_challenge.DFAR == challenge.DFAR &&
//			ns_challenge.DFSR == challenge.DFSR &&
//			ns_challenge.value == challenge.value &&
//			ns_challenge.counter == challenge.counter+1) ||
//			dfar != ns_challenge.DFAR) {
//		verify_value[0] = 0xfffffff;
//		verify_value[1] = 0xfffffff;
//		verify_value[2] = 0xfffffff;
//		verify_value[3] = 0xfffffff;
//		return verify_value;
//	}
//
//	struct crypto_challenge read_response;
//
//	read_response.DFAR = dfar;
//	read_response.DFSR = read_value;
//	read_response.value = *verify_value;
//	read_response.counter = counter++;
//
//	AES_CBC_encrypt_buffer(&ctx, (uint8_t*) &read_response, sizeof(read_response));
//	memcpy(verify_value, &read_response, sizeof(read_response));
//
//
//	// TODO: DON'T CALL NORMAL WORLD CODE FROM SECURE WORLD!!
////	Xil_DCacheFlush();
//
//	return verify_value;
//}


__monitor_code void * get_crypto_challenge(unsigned int dfar, unsigned int dfsr, unsigned int value) {
#ifdef NOCRYPTO
	return &ns_challenge;
#endif

//	if (updating_key >= 4) {
//		xil_printf("Key update complete...\r\n");
//		updating_key = 0;
//	} else if (updating_key == 2) {
//		xil_printf("Sending DH public key...\r\n");
//		updating_key = 3;
//		// ns_challenge was already set to be our DH public key
//		return &ns_challenge;
//	}


	int btn = read_addr(BUTTON_ADDR);
	if (updating_key >= 2) {
			updating_key = 0;
	} else if (btn == 0x1) {
#ifdef DEBUG
		xil_printf("Start key exchange...\r\n");
#endif
		ns_challenge.DFAR = 0xffffffff;
		ns_challenge.DFSR = 0xffffffff;
		ns_challenge.value = 0xffffffff;
		ns_challenge.counter = 0xffffffff;
		updating_key = 1;
		dh_sent = 0;
		return &ns_challenge;
	}

	if (!AES_initialized) {
		AES_init_ctx_iv(&ctx, key, init_vector);
		AES_initialized = 1;
#ifdef DEBUG
		xil_printf("Initialized with: %08X%08X%08X%08X\r\n", *((int *)key), *((int *)key+1), *((int *)key+2), *((int *)key+3));
#endif
	}

	dfsr |= read_addr(GLOBAL_TIMER) << 16 ;
	challenge.DFAR = dfar;
	challenge.DFSR = dfsr;
	challenge.value = value;
	challenge.counter = counter;
	counter += 2; // Prevent replays of the reply

#ifdef DEBUG
	xil_printf("Crypto challenge: %08X %08X %08X %08X\r\n", challenge.DFAR,
			challenge.DFSR,
			challenge.value,
			challenge.counter);
#endif

	memcpy(&ns_challenge, &challenge, sizeof(challenge));
	AES_CBC_encrypt_buffer(&ctx, (uint8_t*) &ns_challenge, sizeof(ns_challenge));

	return &ns_challenge;
}
#endif
