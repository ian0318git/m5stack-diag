/* $Id: platform_poe_psu.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_poe_psu.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_poe_psu.h
 *
 * Description: Operation Overlord PoE PSU I2C structs and defines.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_POE_PSU_H__
#define __PLATFORM_POE_PSU_H__


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
//extern boolean has_poe_psu(uint32_t);

#endif /* __PLATFORM_POE_PSU_H__ */

