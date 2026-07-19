/* $Id: diag_tam_api_drv_support.h,v 1.2 2016/04/20 11:25:24 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_tam_api_drv_support.h,v $
 *------------------------------------------------------------------
 *
 * diag_tam_api_drv_support.h - Header file for TAM ACT2 Library
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_TAM_API_DRV_SUPPORT__
#define __DIAG_TAM_API_DRV_SUPPORT__

#define SEGMENT_I2C_READ                                    (511)
#define ACT2_RW_RETRY                                       (50)
#define ACT_RETRY                                           (30)
#define ACT_DELAY                                           (200)

extern int act2_drv_write(void *, char *, unsigned int);
extern int act2_drv_read(void *, char *, unsigned int);

#endif /* __DIAG_TAM_API_DRV_SUPPORT__ */

/*---------------------------------------------------------------
$Log: diag_tam_api_drv_support.h,v $
Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/
