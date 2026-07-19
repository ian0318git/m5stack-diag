 /* $Id: plat_defs.h,v 1.3 2018/08/31 03:59:30 chieyang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Viper platform defines.
 *
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
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

#define VIPER_FPGA_REG_WIDTH    4

/* define sleep seconds */
#define LENGTH100             100
#define LENGTH1000            1000
#define LENGTH1024            1024

#define LENGTH32               32
#define LENGTH64               64 

/* define usb register */
#define USB_PORT_CTRL			"0480"
#define USB_PORT_CTRL_TEST		"0484"

/* Extern */
extern int usb_dump_x(int);
extern int get_i2c_fd(int);
extern int usb_utils(int);
extern int diag_extend_feature(boolean);
extern int viper_show_cpuinfo(void);
extern void viper_show_meminfo(void);

#endif                          /* _PLAT_DEFS_H_ */
/*-------------------------------------------------
 * $Log: plat_defs.h,v $
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.4  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.3  2018/05/10 09:04:37  lucywang
 * Added USB 2.0 test mode utility
 *
 * Revision 1.1.2.2  2018/04/09 02:34:50  lucywang
 * Added System Intermation
 *
 * Revision 1.1.2.1  2018/02/27 08:06:51  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
