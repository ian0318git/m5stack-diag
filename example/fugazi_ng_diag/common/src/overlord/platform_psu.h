/* $Id: platform_psu.h,v 1.5 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_psu.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_psu.h
 *
 * Description: Informers PSU I2C structs and defines.
 *		This file is based on EDCS-483504 and vendors datasheets.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PSU_H__
#define __PLATFORM_PSU_H__

#include "dev_at24c0n.h"

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
} OVLD_IOFPGA_PSU_DEVICE;

/* PSU No. */
typedef enum {
    PSU_ONE = 1,     /* PSU 1 */
    PSU_TWO,         /* PSU 2 */
} OVLD_IOFPGA_PSU_NUM;

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

/*------------------------------------------------------------------
$Log: platform_psu.h,v $
Revision 1.5  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.4  2013/07/16 08:02:08  alpeng
support i2c read and read cookie for juno psu

Revision 1.3  2013/06/14 09:50:48  hroni
1. support to PSU mux
2. temporarily uses #ifdef MUX124 to turn off/on PCA9545 mux support

Revision 1.2  2013/05/31 12:51:28  danchung
Add checking board type for Juno.

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.5  2013/03/29 15:18:47  palin2
Add utilities to dump PSU MicroController registers for debug purpose.

Revision 1.4  2012/06/05 11:44:37  palin2
Clean up compiler warnings.

Revision 1.3  2012/05/04 08:03:33  alpeng
skip Max1617, check PoE and PoE PSU is present before I2C scan test

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
