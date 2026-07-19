/*------------------------------------------------------------------
 * Filename:    platform_psu.h
 *
 * Description: Informers PSU I2C structs and defines.
 *
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PSU_H__
#define __PLATFORM_PSU_H__

/* Common defines */
 
/* PSU related device */
typedef enum {
    PSU0_EEPROM = 0,     /* PSU0 EEPROM */
    PSU0_MCNTRL,         /* PSU0 Microcontroller */
    PSU1_EEPROM,         /* PSU1 EEPROM */
    PSU1_MCNTRL,         /* PSU1 Microcontroller */
} IOFPGA_PSU_DEVICE;

/* PSU No. */
typedef enum {
    PSU_ZERO = 0,     /* PSU0 */
    PSU_ONE,          /* PSU1 */
    MAX_NUM_PSU
} IOFPGA_PSU_NUM;

/* PSU register data structure */
typedef struct psu_reg_info_t_ {
    uint16_t    code;       /* Command Code */
    char        *name;      /* Text */
    uint32_t    data_len;   /* Register Data Length */
    uint8_t     type;       /* Access Type */
} psu_reg_info_t;

/* Functions prototype */
extern void show_psu_type(void);
extern int show_psu_cookie(int);
void build_psu_util_menu(uint32_t);
void build_psu_test_menu(uint32_t);
extern uint32_t check_psu_present(uint32_t);
extern uint32_t check_psu_stat(uint32_t);
void psu_show_env_info(void);

#endif
