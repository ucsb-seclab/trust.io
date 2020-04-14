#ifndef __TRUSTIO_TEST_H__
#define __TRUSTIO_TEST_H__
#include <drivers/pl061_gpio.h>

extern struct pl061_data glob_trustio_pd;
extern int is_trustio_init;

//void perform_trustio_test(void);

void write_to_trustio_pin(long val);

void init_trustio_gpio(void);

#endif
