/* $Id: plug_lte_telit_test.h,v 1.3 2020/06/03 08:48:53 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_test.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_test.h - Header File for Pluggable LTE Telit
 *                         Main Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_LTE_TELIT_TEST__
#define __PLUG_LTE_TELIT_TEST__


#define TELIT_SYS_SUPPRESS_PRINTK         "dmesg -n 1"
#define PROBE_LTE_TELIT_USB_TOUT          (500)
#define LTE_TELIT_USB_ENUM_TOUT           (10)
#define MODEM_SERDES_SWITCH_PROBE_TOUT    (500)
#define PLUG_LTE_ACT2_1P5_UNRESET_DELAY   (5000)
#define PLUG_LTE_TELIT_MIN_ACTIVE_SEC     (15)

#define PLUG_LTE_FPGA_I2C_ACK_MUX         (0)
#define PLUG_LTE_ACT2_ADD                 (0xE6 >> 1)
#define PLUG_LTE_FPGA_I2C_ACK_REG_ADD     (0)
#define PLUG_LTE_FPGA_I2C_ACK_SUB_ADD     (1)
#define PLUG_LTE_FPGA_I2C_ACK_DATA_LEN    (1)

extern int plug_lte_telit_main(void *);

#endif

/*-----------------------------------------------
 *$Log:     
 */
