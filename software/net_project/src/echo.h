/*
 * echo.h
 *
 *  Created on: Aug 22, 2017
 *      Author: cspensky
 */

#ifndef ECHO_H_
#define ECHO_H_

struct tcp_pcb *tpcb_global;
err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err);

unsigned int START_TIME[2];
unsigned int CRYPTO_CALL[2];
unsigned int CRYPTO_RESPONSE[2];
unsigned int TIO_RETURN[2];


#endif /* ECHO_H_ */
