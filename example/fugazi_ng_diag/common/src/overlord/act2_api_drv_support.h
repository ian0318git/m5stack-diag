/* $Id: act2_api_drv_support.h,v 1.2 2013/11/26 08:40:35 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * act2_api_drv_support.h  Support for ACT2/Ruby API code.
 *
 * May 2011: Alan O'Sullivan  
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __ACT2_API_DRV_SUPPORT_H__
#define __ACT2_API_DRV_SUPPORT_H__

#define TOKEN_CERT_SIZE  256
#define TOKEN_SIG_SIZE   256
#define CLIIP_SIZE       128

extern int act2_drv_read (void *module, char *status_buf, unsigned int length);
extern int act2_drv_write (void *module, char *send_buf, unsigned int length);
//extern int test_lib_api_call (void);

#endif  /* __ACT2_API_DRV_SUPPORT_H__ */
/******** History ********
$Log: act2_api_drv_support.h,v $
Revision 1.2  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
