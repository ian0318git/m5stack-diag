#ifndef __JAM_LINUX_GPIO_H__
#define __JAM_LINUX_GPIO_H__
int gpio_init_jtag(void);
int gpio_free_jtag(void);
void gpio_set_tdi(int tdi);
void gpio_set_tms(int tms);
void gpio_set_tck(int tlk);
unsigned int gpio_get_tdo(void);
#endif
