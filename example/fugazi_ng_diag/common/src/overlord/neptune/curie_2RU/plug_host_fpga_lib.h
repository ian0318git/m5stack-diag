/* $Id: plug_host_fpga_lib.h,v 1.1 2020/01/09 01:02:05 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/plug_host_fpga_lib.h,v $
 *------------------------------------------------------------------
 * Filename: plug_host_fpga_lib.h
 * Description: Header file for plug host functions for various fpga components
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_HOST_FPGA_LIB__
#define __PLUG_HOST_FPGA_LIB__

#include "dash_fpga.h"
#include "ngio.h"

#define BIT(x)  (1 << (x))
#define BIT_VAL(a,b)  ((a & BIT(b)) >> b)

typedef enum
{
    FPGA_BIT_OPS_ON,
    FPGA_BIT_OPS_OFF
} fpga_bit_ops;

#define PLUG_TESTCARD_GPS_SLOT1         (0x001)
#define PLUG_TESTCARD_GPS_SLOT2         (0x002)
#define MAX_PLUG_SLOT_NUMBER            (1)
#define MAX_PLUG_USB_MASS_STORE_ON_SYS  (5)

typedef enum {
    PLUG_SLOT_1 = 1,
    PLUG_SLOT_2
} plug_slot_no;

#define FPGA_PLUG_OFFSET_BY_SLOT(offset, slot)  (offset + ((slot - 1) * 0x10))
#define PLUG_UART_CONTROL_OFFSET_BY_SLOT(offset, slot) (offset + ((slot - 1) * 0x100))

#define PLUG_FPGA_I2C_OP_DELAY                      (3)
#define PLUG_FPGA_I2C_OP_TOUT                       (500)
#define PLUG_FPGA_I2C_IDLE_TIMEOUT                   30
#define PLUG_FPGA_REG_WRITE_DELAY                   1000

#define SCL_DRIVE_TIMES   100
#define PLUG_FPGA_DBG_LED_ADDR_REG      PLUG_MISCELLANEOUS_REG
#define PLUG_UART_CONTROLLER_OFFSET     PLUG_UART_CTRL6_OFFSET

/* PLUG I2C Controller Offset */
#define PLUG_I2C_CTRL_OFFSET            PLUG_I2C_CTRL20_OFFSET
#define PLUG_FPGA_I2C_OFFSET            (0x100)

/*
 * Debug LED register bit
 */
#define PLUG_LTE_GPS_SYNC_STATUS_0        0x000
#define PLUG_LTE_GPS_SYNC_STATUS_1        0x002

#define FPGA_EXTER_DEV_RST_REG       0x0004
#define FPGA_USB_HUB_RESET_BIT      (27)

#define PLUG_TC_USB_3P0_BUS_NUMBER    (4)
#define PLUG_TC_USB_3P0_LEV_NUMBER    (1)
#define PLUG_TC_USB_3P0_PRNT_NUMBER   (1)
#define PLUG_TC_USB_3P0_PORT_NUMBER   (5)
#define PLUG_TC_USB_2P0_BUS_NUMBER    (2)
#define PLUG_TC_USB_2P0_LEV_NUMBER    (2)
#define PLUG_TC_USB_2P0_PRNT_NUMBER   (2)
#define PLUG_TC_USB_2P0_PORT_NUMBER   (3)

#define PLUG_TC_USB_3P0_BUS_NUMBER_2RU  (2)
#define PLUG_TC_USB_3P0_LEV_NUMBER_2RU  (1)
#define PLUG_TC_USB_3P0_PRNT_NUMBER_2RU (1)
#define PLUG_TC_USB_3P0_PORT_NUMBER_2RU (9)
#define PLUG_TC_USB_2P0_BUS_NUMBER_2RU  (1)
#define PLUG_TC_USB_2P0_LEV_NUMBER_2RU  (1)
#define PLUG_TC_USB_2P0_PRNT_NUMBER_2RU (1)
#define PLUG_TC_USB_2P0_PORT_NUMBER_2RU (3)

#endif
/*
 *-----------------------------------------------------------------------------
$Log: plug_host_fpga_lib.h,v $
Revision 1.1  2020/01/09 01:02:05  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
