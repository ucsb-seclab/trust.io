
#include <drivers/pl061_gpio.h>
#include <hikey_peripherals.h>
#include <io.h>
#include <kernel/tee_time.h>
#include <mm/core_memprot.h>
#include <stdint.h>
#include <trace.h>
#include <util.h>
#include <trustio_test.h>

#define TRUSTIO_PIN_ID 17

struct pl061_data glob_trustio_pd = { 0 };
int is_trustio_init = 0;

long trust_io_hash = 0;
long read_from_trustio_pin(void);

void init_trustio_gpio(void) {
    if(!is_trustio_init) {
        vaddr_t trustio_gpio_base = core_mmu_get_va(TRUSTIO_GPIO_BASE, MEM_AREA_IO_NSEC);
    
    	
	    DMSG("Initializing Trust.IO GPIO using base:%lx\n", trustio_gpio_base);
    
        pl061_init(&glob_trustio_pd);
        pl061_register(trustio_gpio_base, 2);
    
        pl061_set_mode_control(TRUSTIO_PIN_ID, PL061_MC_SW);
    
        glob_trustio_pd.chip.ops->set_interrupt(TRUSTIO_PIN_ID, GPIO_INTERRUPT_DISABLE);
    
        //glob_trustio_pd.chip.ops->set_direction(TRUSTIO_PIN_ID, GPIO_DIR_IN);
        glob_trustio_pd.chip.ops->set_direction(TRUSTIO_PIN_ID, GPIO_DIR_OUT);
        is_trustio_init = 1;
    }
}

/*void perform_trustio_test(void) {
    struct pl061_data pd;
    vaddr_t trustio_gpio_base = core_mmu_get_va(TRUSTIO_GPIO_BASE, MEM_AREA_IO_NSEC);
    
    	
	DMSG("Turning ON TrustIO Pin 17 new got:%lx\n", trustio_gpio_base);
    
    pl061_init(&pd);
    pl061_register(trustio_gpio_base, 2);
    
    pl061_set_mode_control(TRUSTIO_PIN_ID, PL061_MC_SW);
    
    pd.chip.ops->set_interrupt(TRUSTIO_PIN_ID, GPIO_INTERRUPT_DISABLE);
    
    pd.chip.ops->set_direction(TRUSTIO_PIN_ID, GPIO_DIR_OUT);
    
    pd.chip.ops->set_value(TRUSTIO_PIN_ID, GPIO_LEVEL_HIGH);    
    
}*/

long read_from_trustio_pin(void) {
    return glob_trustio_pd.chip.ops->get_value(TRUSTIO_PIN_ID);
}

void write_to_trustio_pin(long val) {
    if(val) {
        glob_trustio_pd.chip.ops->set_value(TRUSTIO_PIN_ID, GPIO_LEVEL_HIGH); 
    } else {
        glob_trustio_pd.chip.ops->set_value(TRUSTIO_PIN_ID, GPIO_LEVEL_LOW); 
    }
}
