/* $Id: plug_host_fpga_lib.h,v 1.2 2019/10/17 02:16:27 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_host_fpga_lib.h,v $
 *------------------------------------------------------------------
 * Filename: plug_host_fpga_lib.h
 * Description: Header file for plug host functions for various fpga components
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_HOST_FPGA_LIB__
#define __PLUG_HOST_FPGA_LIB__

#include "diag_fpga.h"
#include "ngio.h"

#define PLUG_TESTCARD_GPS_SLOT1           (0x001)
#define PLUG_TESTCARD_GPS_SLOT2           (0x002)
#define MAX_PLUG_SLOT_NUMBER              (1)
#define MAX_PLUG_USB_MASS_STORE_ON_SYS    (5)

#define PLUG_FPGA_I2C_OP_DELAY            (3)
#define PLUG_FPGA_I2C_OP_TOUT             (500)
#define PLUG_FPGA_I2C_IDLE_TIMEOUT        30
#define PLUG_FPGA_REG_WRITE_DELAY         1000

#define SCL_DRIVE_TIMES                   100
#define PLUG_FPGA_DBG_LED_ADDR_REG        PLUG_MISCELLANEOUS_REG
#define PLUG_UART_CONTROLLER_OFFSET       PLUG_UART_CTRL6_OFFSET

/* PLUG I2C Controller Offset */
#define PLUG_I2C_CTRL_OFFSET              PLUG_I2C_CTRL20_OFFSET
#define PLUG_FPGA_I2C_OFFSET              (0x100)

/*
 * Debug LED register bit
 */
#define PLUG_LTE_GPS_SYNC_STATUS_0        0
#define PLUG_LTE_GPS_SYNC_STATUS_1        9

#define CISCO_FPGA_EXTER_DEV_RST_REG      0x0004
#define FPGA_USB_HUB_RESET_BIT            (27)

#define PLUG_TC_USB_3P0_BUS_NUMBER        (2)
#define PLUG_TC_USB_3P0_LEV_NUMBER        (1)
#define PLUG_TC_USB_3P0_PORT_NUMBER       (1)
#define PLUG_TC_USB_2P0_BUS_NUMBER        (2)
#define PLUG_TC_USB_2P0_LEV_NUMBER        (2)
#define PLUG_TC_USB_2P0_PORT_NUMBER       (3)

#define BIT(x)                            (1 << (x))
#define BIT_VAL(a,b)                      ((a & BIT(b)) >> b)

#define FPGA_PLUG_OFFSET_BY_SLOT(offset, slot)  (offset + ((slot - 1) * 0x10))
#define PLUG_UART_CONTROL_OFFSET_BY_SLOT(offset, slot) (offset + ((slot - 1) * 0x100))

typedef enum
{
    FPGA_BIT_OPS_ON,
    FPGA_BIT_OPS_OFF
} fpga_bit_ops;

typedef enum {
    PLUG_SLOT_1 = 1,
    PLUG_SLOT_2,
} plug_slot_no;

#endif

/*-------------------------------------------------
$Log: plug_host_fpga_lib.h,v $
Revision 1.2  2019/10/17 02:16:27  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.3  2019/07/29 06:13:52  kodko
Clean up code based on off-line code review

Revision 1.1.2.2  2018/11/29 12:05:23  kodko
Corrected LTE GPS SYNC STATUS register bit definition

Revision 1.1.2.1  2018/10/26 08:40:50  kodko
Add support for PIM LTE and test card modules.

$Endlog$
*/

