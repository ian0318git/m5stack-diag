/* $Id: plat_defs.h,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Nutella platform defines.
 *
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
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

#define NUTELLA_FPGA_REG_WIDTH    4

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
extern int nutella_show_cpuinfo(void);
extern void nutella_show_meminfo(void);

#endif                          /* _PLAT_DEFS_H_ */
/*-------------------------------------------------
$Log: plat_defs.h,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
