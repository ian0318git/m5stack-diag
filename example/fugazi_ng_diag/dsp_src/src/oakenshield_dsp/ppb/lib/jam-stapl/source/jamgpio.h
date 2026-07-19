/**
 * @file	jamgpio.h
 * @brief	Raspberry Pi GPIO functions for JTAG programming
 */

#define CPLD_MB 1
#define CPLD_DB2 2
#define CPLD_DB3 3

/**
 * gpio_init_jtag() - Initialize the JTAG input and output pins
 */
void gpio_init_jtag(int);

/**
 * gpio_close_jtag() - Puts the JTAG input and output pins back in a safe state (input)
 */
void gpio_close_jtag(int);

void gpio_set_tdi(int);
void gpio_clear_tdi(int);
void gpio_set_tms(int);
void gpio_clear_tms(int);
void gpio_set_tck(int);
void gpio_clear_tck(int);
unsigned int gpio_get_tdo(int);
