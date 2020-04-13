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

#define LWIP_DEBUG 1

#include <stdio.h>

#include "xparameters.h"

#include "netif/xadapter.h"
#include "echo.h"

#include "platform.h"
#include "platform_config.h"
#if defined (__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#include "xqspips_hw.h"
#include "xil_io.h"
#endif

#include "lwip/tcp.h"
#include "xil_cache.h"
#include "xscugic.h"
#include "trust.io.h"

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif

/* defined by each RAW mode application */
void print_app_header();
int start_application();
int transfer_data();
void tcp_fasttmr(void);
void tcp_slowtmr(void);

/* missing declaration in lwIP */
void lwip_init();

#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

extern void monitorInit();
extern void SMC_CALL();
extern void WORLD_SWITCH();
extern void ENABLE_INTERRUPTS();

void
print_ip(char *msg, struct ip_addr *ip) 
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip), 
			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif

// Trust.IO Global for challenge/response
int crypto_response = 0;

#ifdef TRUST_IO
/**
 *  This is small stub TCP recv callback function for receiving our cryptographic response
 *
 */
err_t crypto_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	// Do not read the packet if we are not in ESTABLISHED state
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	// Indicate that the packet has been received
	tcp_recved(tpcb, p->len);
#ifdef DEBUG
	xil_printf("Received packet of length %d\r\n", p->len);
#endif
	// Did we get exactly an integer back, as we'd expect?
	if (p->len == sizeof(ns_challenge)) {
		// Set our response so that the main function can return
		memcpy(&ns_challenge, p->payload, sizeof(ns_challenge));
		crypto_response = 1;
	} else {
		crypto_response = -1;
	}

#ifdef DEBUG
	xil_printf("Crypto Received: %s %d (%d)\n\r", p->payload, p->len, crypto_response);
#endif

	// Free the received pbuf
	pbuf_free(p);

	return ERR_OK;
}

/**
 * This will be called by Trust.IO with the address and the challenge.
 * This function must interact with the authentication device to satisfy the cryptographic challenge, and then return it.
 *
 * This specific instance using the existing TCP connection to send the challenge and receive the response.
 *
 * WARNING: Interrupts are still enabled within this function.
 * WARNING: This function must not try to access any protected memory-mapped I/O
 *
 * ALL PARAMETERS ARE ACTUALLY ENCRYPTED VALUES
 *
 * @return crypto_challenge ^ key or -1
 */
void* crypto_request(int dfar, int dfsr, int value, int counter) {

	ns_challenge.DFAR = dfar;
	ns_challenge.DFSR = dfsr;
	ns_challenge.value = value;
	ns_challenge.counter = counter;

#ifdef TIMING
	// Store our timer value
	CRYPTO_CALL[0] = Xil_In32(0xF8F00200);
	CRYPTO_CALL[1] = Xil_In32(0xF8F00204);
#endif
#ifdef DEBUG
	xil_printf("crypto request called in non-secure world. (0x%08X, 0x%08X, 0x%08X, 0x%08X)\r\n", dfar, dfsr, value, counter);
#endif

	// Make sure our tpcb struct is actually defined.
	if (tpcb_global == NULL) {
		xil_printf("TCP socket is null\r\n");
		return -1;
	}

	// Let's send our challenge to the authentication device over the TCP protocol.
	err_t err;
	if (tcp_sndbuf(tpcb_global) > sizeof(ns_challenge)) {
#ifdef DEBUG
		xil_printf("Sending: %08X%08X%08X%08X\r\n", ns_challenge.DFAR, ns_challenge.DFSR, ns_challenge.value, ns_challenge.counter);
#endif
		err = tcp_write(tpcb_global, (uint8_t*) &ns_challenge, sizeof(ns_challenge), 1);
	} else {
		xil_printf("no space in tcp_sndbuf\n\r");
	}
#ifdef DEBUG
	xil_printf("Sent %d %d\r\n", value, err);
#endif
	// Was there an error?
	if (err != 0) {
		crypto_response = -1;
	} else {
		crypto_response = 0;
	}

	// Hijack the receive callback temporarily
	tcp_recv(tpcb_global, crypto_callback);
	// Receive and process packets
	// TODO: Set a timeout!
	while (crypto_response == 0) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
		transfer_data();
	}
#ifdef DEBUG
	xil_printf("Crypto Response: %d\r\n", crypto_response);
#endif
	// Set our callback back
	tcp_recv(tpcb_global, recv_callback);

#ifdef TIMING
	// Store our timer value
	CRYPTO_RESPONSE[0] = Xil_In32(0xF8F00200);
	CRYPTO_RESPONSE[1] = Xil_In32(0xF8F00204);
#endif
	return &ns_challenge;
}
#endif


int main()
{
	// Trust.IO
//	xil_printf("Setting up monitor...\r\n");
//	monitorInit(&crypto_request);
//	Xil_DCacheFlush();
//	xil_printf("Switching worlds...\r\n");
//	WORLD_SWITCH();


	struct ip_addr ipaddr, netmask, gw;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif = &server_netif;
#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

/* Define this board specific macro in order perform PHY reset on ZCU102 */
#ifdef XPS_BOARD_ZCU102
	IicPhyReset();
#endif

	xil_printf("Initializing platform...\r\n");
	init_platform();


#if LWIP_DHCP==1
    ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#else
	/* initliaze IP addresses to be used */
	IP4_ADDR(&ipaddr,  10, 0,   0, 10);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      10, 0,   0,  1);
#endif	
	print_app_header();

	xil_printf("lwip init...\r\n");
	lwip_init();


	xil_printf("Adding network interface...\r\n");
  	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	xil_printf("Setting default interface...\r\n");
	netif_set_default(echo_netif);


	/* now enable interrupts */
	xil_printf("Programming interrupts...\r\n");
	platform_enable_interrupts();


	/* specify that the network if is up */
	xil_printf("Checking that interface is up...\r\n");
	netif_set_up(echo_netif);

#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	dhcp_start(echo_netif);
	dhcp_timoutcntr = 24;

	while(((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(echo_netif);

	if (dhcp_timoutcntr <= 0) {
		if ((echo_netif->ip_addr.addr) == 0) {
			xil_printf("DHCP Timeout\r\n");
			xil_printf("Configuring default IP of 192.168.1.10\r\n");
			IP4_ADDR(&(echo_netif->ip_addr),  192, 168,   1, 10);
			IP4_ADDR(&(echo_netif->netmask), 255, 255, 255,  0);
			IP4_ADDR(&(echo_netif->gw),      192, 168,   1,  1);
		}
	}

	ipaddr.addr = echo_netif->ip_addr.addr;
	gw.addr = echo_netif->gw.addr;
	netmask.addr = echo_netif->netmask.addr;
#endif



	print_ip_settings(&ipaddr, &netmask, &gw);


	// Trust.IO
#ifdef TRUST_IO
	xil_printf("Setting up monitor...\r\n");
	monitorInit(&crypto_request);
	Xil_DCacheFlush();
	xil_printf("Switching worlds...\r\n");
	WORLD_SWITCH();
	xil_printf("Now in non-secure.\r\n");
	XScuGic_DeviceInitialize(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	xil_printf("SCU configured.\r\n");
	ENABLE_INTERRUPTS();
	xil_printf("Done switching.\r\n");
#endif

	/* start the application (web server, rxtest, txtest, etc..) */
	start_application();

	// Let's try a protected access
//	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR, 0x5555555);


	/* receive and process packets */
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
		transfer_data();
	}
  
	/* never reached */
	cleanup_platform();

	return 0;
}
