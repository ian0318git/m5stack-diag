/* $Id: plat_defs.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Fugazi platform defines.
 *
 *
 * Copyright (c) 2016 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_

#include "types.h"

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)

#define FUGAZI_FPGA_REG_WIDTH    4

/* define sleep seconds */
#define LENGTH100             100
#define LENGTH1000            1000
#define LENGTH1024            1024

#define LENGTH32               32
#define LENGTH64               64 

/* define usb register */
#define USB_PORT_CTRL			"0480"
#define USB_PORT_CTRL_TEST		"0484"

#define USB0_MASK                   (1 << 1)
#define USB1_MASK                   (1 << 2)
#define AUX_EXT_MASK                (1 << 3)
#define MSATA_MASK                  (1 << 4)
#define EUSB_MASK                   (1 << 5)

/* Fugazi CPU critical info */
#define FUGAZI_CORE_NUM             12
#define FUGAZI_PROC_NUM             (FUGAZI_CORE_NUM*2)
#define SKYLAKE_XEON_2_0GHZ_CPU     "Intel(R) Xeon(R) D-2168NT CPU @ 2.60GHz"
#define FUGAZI_BUF_SIZE             256
#define FUGAZI_DIMM_NUM             2
#define FUGAZI_SYS_PRESS_THRE       70


/* Extern */
extern int usb_dump_x(int);
extern int get_i2c_fd(int);
extern int usb_utils(int);
extern int diag_extend_feature(boolean);

/* Fugazi CPU PCIe bus to NGIO (For common file)*/
#define FUGAZI_NGIO_PCIE_BUS_NUM  (0x78)

#define MAX_NUM_PSU             2
#define FUGAZI_MENU_OPT_MSK    0x1



#endif                          /* _PLAT_DEFS_H_ */

/*-------------------------------------------------
 * $Log: plat_defs.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:50  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.3  2019/03/28 06:31:52  iachang
 * Check SKYLAKE CPU model name and speed
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

