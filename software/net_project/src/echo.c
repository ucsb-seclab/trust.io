/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#if defined (__arm__) || defined (__aarch64__)
#include "xil_printf.h"
#include "xil_io.h"
#endif

#include "echo.h"

// Trust.IO: Added to make sure we can re-use the same socket.
struct tcp_pcb *tpcb_global;
int transfer_data() {
	return 0;
}

void print_app_header()
{
	xil_printf("\n\r\n\r-----lwIP TCP echo server ------\n\r");
	xil_printf("TCP packets sent to port 6001 will be echoed back\n\r");
}

void print_time(unsigned int time_array[2]) {
	xil_printf("0x%08x%08x\r\n", time_array[1], time_array[0]);
}

err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	// Trust.IO: Added to make sure we can re-use the same socket.
	tpcb_global = tpcb;

	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);

#ifdef DEBUG
	xil_printf("Received: %s %d\n\r", p->payload, p->len);
#endif

	// MODIFICATION: Making TCP Server do something interesting
	if (p->len >= 4 && strncmp(p->payload,"on", p->len-2) == 0) {
#ifndef SILENT
		xil_printf("Turning lights ON...\n\r");
#endif

#ifdef TIMING
		// Store our timer value
		START_TIME[0] = Xil_In32(0xF8F00200);
		START_TIME[1] = Xil_In32(0xF8F00204);
#endif

		Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR, 0xffffff);

#ifdef TIMING
		// Store our timer value
		TIO_RETURN[0] = Xil_In32(0xF8F00200);
		TIO_RETURN[1] = Xil_In32(0xF8F00204);

		xil_printf("--- on ---\r\n");
		print_time(START_TIME);
		print_time(CRYPTO_CALL);
		print_time(CRYPTO_RESPONSE);
		print_time(TIO_RETURN);
#endif

	} else if (p->len >= 5 && strncmp(p->payload,"off", p->len-2) == 0) {
#ifndef SILENT
		xil_printf("Turning lights OFF...\n\r");
#endif
#ifdef TIMING
		// Store our timer value
		START_TIME[0] = Xil_In32(0xF8F00200);
		START_TIME[1] = Xil_In32(0xF8F00204);
#endif
		Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR, 0);
#ifdef TIMING
		// Store our timer value
		TIO_RETURN[0] = Xil_In32(0xF8F00200);
		TIO_RETURN[1] = Xil_In32(0xF8F00204);

		xil_printf("--- off ---\r\n");
		print_time(START_TIME);
		print_time(CRYPTO_CALL);
		print_time(CRYPTO_RESPONSE);
		print_time(TIO_RETURN);

#endif
	} else if (p->len >= 6 && strncmp(p->payload,"read", p->len-2) == 0) {
#ifndef SILENT
		xil_printf("Performing a verified read...\n\r");
#endif

		int read[5] = {0,0,0,0,0};
#ifdef TIMING
		// Store our timer value
		START_TIME[0] = Xil_In32(0xF8F00200);
		START_TIME[1] = Xil_In32(0xF8F00204);
#endif

		read[0] = Xil_In32(XPAR_AXI_GPIO_0_BASEADDR);
#ifdef TRUST_IO
//		int *verify_val = (int *) get_read_verification();
//		read[1] = verify_val[0];
//		read[2] = verify_val[1];
//		read[3] = verify_val[2];
//		read[4] = verify_val[3];

#endif
#ifdef TIMING
		// Store our timer value
		TIO_RETURN[0] = Xil_In32(0xF8F00200);
		TIO_RETURN[1] = Xil_In32(0xF8F00204);

		xil_printf("--- read ---\r\n");
		print_time(START_TIME);
		print_time(CRYPTO_CALL);
		print_time(CRYPTO_RESPONSE);
		print_time(TIO_RETURN);
#endif

#ifdef DEBUG
		xil_printf("Read: %d \n\r", read[0]);
//		xil_printf("verify value: %08X%08X%08X%08X.\n\r", read[1], read[2], read[3], read[4]);
		xil_printf("Sending %d bytes\r\n", sizeof(read)-sizeof(int));
#endif



//#ifdef TRUST_IO
//
//		err = tcp_write(tpcb, read, sizeof(read), 1);
//#else
		err = tcp_write(tpcb, read, sizeof(int), 1);
//#endif
		/* free the received pbuf */
		pbuf_free(p);

		return ERR_OK;
	}

	/* echo back the payload */
	/* in this case, we assume that the payload is < TCP_SND_BUF */
	if (tcp_sndbuf(tpcb) > p->len) {
		// MODIFICATION: Removing echo functionality
		err = tcp_write(tpcb, p->payload, p->len, 1);
	} else
		xil_printf("no space in tcp_sndbuf\n\r");

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);

	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)(UINTPTR)connection);

	/* increment for subsequent accepted connections */
	connection++;

	return ERR_OK;
}


int start_application()
{
	struct tcp_pcb *pcb;

	err_t err;
	unsigned port = 7;

	/* create new TCP PCB structure */
	pcb = tcp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP echo server started @ port %d\n\r", port);

	return 0;
}
