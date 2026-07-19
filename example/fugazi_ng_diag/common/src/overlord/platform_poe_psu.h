/* $Id: platform_poe_psu.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_poe_psu.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_poe_psu.h
 *
 * Description: Operation Overlord PoE PSU I2C structs and defines.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_POE_PSU_H__
#define __PLATFORM_POE_PSU_H__

#include "dev_at24c0n.h"

/* Common defines */
#define POE_PSU_BUF_SIZE   7
#define POE_PSU_MAX_RETRY  3

/* PoE PSU related device */
typedef enum {
    POE_PSU1_EEPROM = 0,     /* PoE PSU1 EEPROM */
    POE_PSU1_MCNTRL,         /* PoE PSU1 Microcontroller */
    POE_PSU2_EEPROM,         /* PoE PSU2 EEPROM */
    POE_PSU2_MCNTRL,         /* PoE PSU2 Microcontroller */
} OVLD_IOFPGA_POE_PSU_DEVICE;

/* PoE PSU No. */
typedef enum {
    POE_PSU_ONE = 1,     /* PoE PSU 1 */
    POE_PSU_TWO,         /* PoE PSU 2 */
} OVLD_IOFPGA_POE_PSU_NUM;

/* Registers offset */
#define CNTRL_REG_OFFSET    0x00
#define ALTER_REG_OFFSET    0x01
#define STATUS_REG_OFFSET   0x02
#define FAULT_REG_OFFSET    0x03
#define SENSE_REG_OFFSET    0x04
#define SOURCE_REG_OFFSET   0x05
#define ADIN_REG_OFFSET     0x06

/* 0x03: Fault Register definitions */
#define FET_SHORT_OCCUR     0x20
#define EN_CHANGED_STAT     0x10
#define POWER_BAD_OCCUR     0x08
#define OVERCUR_OCCUR       0x04
#define UNDERVOLT_OCCUR     0x02
#define OVERVOLT_OCCUR      0x01
#define POE_PSU_NO_FAULT    0x00

/* since the 12V PoE PSU register is only 8 bits,
 * the test pattern is from common.h
#define POWER_BAD_OCCUR     0x08
#define OVERCUR_OCCUR       0x04
 * #define PATTERN   0x5ADBA56C
 */
#define POE_PSU_PATTERN 0x5A

/* Waiting time for FPGA to pick up PoE PSU power status change (msec),
 * the value we used now is just a experiential one.
 */
#define POE_PSU_WAIT_TIME   1000


/* Functions prototype */
extern boolean has_poe_psu(uint32_t);

#endif /* __PLATFORM_POE_PSU_H__ */

/*------------------------------------------------------------------
$Log: platform_poe_psu.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.7  2012/05/04 08:03:32  alpeng
skip Max1617, check PoE and PoE PSU is present before I2C scan test

Revision 1.6  2012/04/25 02:01:52  palin2
Add code to clean-up FAULT register before first read based on HW's request.

Revision 1.5  2012/04/17 16:37:47  palin2
Add 12V PoE PSU power enable test.

Revision 1.4  2012/04/17 14:14:06  palin2
Add 12V PoE PSU cookie utility support.

Revision 1.3  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
