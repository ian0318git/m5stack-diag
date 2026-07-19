/* $Id: platform_cookie.h,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * diag_platform_cookie.h - Header file for platform cookie
 *
 * June 2015, Times Huang ported from Overlord
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_PLATFORM_COOKIE__
#define __DIAG_PLATFORM_COOKIE__

#include "cli_cmd.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "diag_plat_cookie.h"

#define QUACK_RETRY                                     8

#define ACT2_RESET_UNRESET_DELAY                        (500)
#define ACT2_UNRESET_DELAY                              (5000)

#endif /* __DIAG_PLATFORM_COOKIE__ */

/*---------------------------------------------------------------
$Log: platform_cookie.h,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.6  2016/01/18 07:02:28  alpeng
update cookie info for read mac

Revision 1.1.2.5  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.4  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.3  2015/08/11 07:44:28  meho
Added f35 nim tests.

Revision 1.1.2.2  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project


$Endlog$
*/
