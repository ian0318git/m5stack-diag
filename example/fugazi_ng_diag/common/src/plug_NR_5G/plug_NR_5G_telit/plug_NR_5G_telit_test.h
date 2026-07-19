/*
 * $Id: plug_NR_5G_telit_test.h,v 1.3 2021/07/15 18:23:23 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_test.h,v $
 *------------------------------------------------------------------
 *
 * plug_NR_5G_telit_test.h - Header File for Pluggable NR_sub6 Telit
 *                         Main Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_NR_5G_TELIT_TEST__
#define __PLUG_NR_5G_TELIT_TEST__


#define TELIT_SYS_SUPPRESS_PRINTK         "dmesg -n 1"
#define PROBE_NR_5G_TELIT_USB_TOUT        (500)
#define PLUG_MODULE_ACT2_1P5_UNRESET_DELAY   (5000)

#define PLUG_MODULE_FPGA_I2C_ACK_MUX      (0)
#define PLUG_MODULE_ACT2_ADD              (0xE6 >> 1)
#define PLUG_MODULE_FPGA_I2C_ACK_REG_ADD  (0)
#define PLUG_MODULE_FPGA_I2C_ACK_SUB_ADD  (1)
#define PLUG_MODULE_FPGA_I2C_ACK_DATA_LEN (1)

#define MODEM_TX_POWER_LOW                 (18)
#define MODEM_TX_POWER_HIGH                (28)

#define MODEM_ANT_WAIT_TIME              (120)
#define MODEM_GPS_WAIT_TIME               (90)

#define MODEM_NUM_OF_ANT                  (5)
#define MODEM_NUM_CELLULAR_ANT            (4)

#define FC_OFFSET                         (2500)

extern int plug_NR_5g_telit_main(void *);

#endif
/*********************************************************************
 * $Log: plug_NR_5G_telit_test.h,v $
 * Revision 1.3  2021/07/15 18:23:23  tshanmug
 * Sears PIM Rx test and Power ON pin test updated to fix the issues
 *
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.2  2020/12/02 03:57:22  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */
