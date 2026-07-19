/* $Id: platform_psu.h,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_psu.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_psu.h
 *
 * Description: Informers PSU I2C structs and defines.
 *		This file is based on EDCS-483504 and vendors datasheets.
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PSU_H__
#define __PLATFORM_PSU_H__

/* Common defines */

/* PSU related device */
typedef enum {
    PSU1_EEPROM = 0,     /* PSU1 EEPROM */
    PSU1_MCNTRL,         /* PSU1 Microcontroller */
    PSU2_EEPROM,         /* PSU2 EEPROM */
    PSU2_MCNTRL,         /* PSU2 Microcontroller */
    PEM0_EEPROM,         /* PEM0 EEPROM for JUNO*/
    PEM0_MCNTRL,         /* PEM0 Microcontroller for JUNO*/
    PEM1_EEPROM,         /* PEM1 EEPROM for JUNO*/
    PEM1_MCNTRL,         /* PEM1 Microcontroller for JUNO*/
} FUGAZI_IOFPGA_PSU_DEVICE;

/* PSU No. */
typedef enum {
    PSU_ONE = 1,     /* PSU 1 */
    PSU_TWO,         /* PSU 2 */
} FUGAZI_IOFPGA_PSU_NUM;

/* PSU register data structure */
typedef struct psu_reg_info_t_ {
    uint16_t    code;       /* Command Code */
    char        *name;      /* Text */
    uint32_t    data_len;   /* Register Data Length */
    uint8_t     type;       /* Access Type */
} psu_reg_info_t;


/* PSU Cookie Controler IDs */
#define PSU_AC_DPS450VB    0x0492 /* Delta AC DPS-450VB A */
#define PSU_AC_DPS1000KB   0x0493 /* Delta AC DPS-1000KB A */


/* Functions prototype */
extern void show_psu_type(void);
extern int show_psu_cookie(int);
extern void build_psu_test_menu(int submenu);
extern uint32_t check_psu_present(uint32_t);
extern uint32_t check_psu_stat(uint32_t);
extern int psu_i2c_test_warp (void);
extern void build_psu_menu (uint32_t option);

#endif /* __PLATFORM_PSU_H__ */


/*-------------------------------------------------
 * $Log: platform_psu.h,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:28  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 */
