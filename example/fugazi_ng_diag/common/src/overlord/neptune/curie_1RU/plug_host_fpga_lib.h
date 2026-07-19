/* $Id: plug_host_fpga_lib.h,v 1.2 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/plug_host_fpga_lib.h,v $
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
#endif
/*-------------------------------------------------
$Log: plug_host_fpga_lib.h,v $
Revision 1.2  2019/08/06 06:56:15  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.5  2018/11/29 10:09:05  meho
Corrected LTE GPS SYNC STATUS register.

Revision 1.1.2.4  2018/11/29 01:54:05  meho
corrected lte gps sync status bit

Revision 1.1.2.3  2018/11/06 07:16:20  meho
sync pluggable test-card common code

Revision 1.1.2.2  2018/10/18 01:41:41  meho
code clean up

Revision 1.1.2.1  2018/10/16 09:05:39  meho
Pluggable re-structured


$Endlog$
*/

