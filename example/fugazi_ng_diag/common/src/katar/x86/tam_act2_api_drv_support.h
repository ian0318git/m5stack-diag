/* $Id: tam_act2_api_drv_support.h,v 1.2 2019/06/14 05:24:52 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/tam_act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * katar_tam_act2_api_drv_support.h  Support for ACT2 API code.
 *
 * Jan 2015: Kody Ko
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef TAM_ACT2_API_DRV_SUPPORT_H_
#define TAM_ACT2_API_DRV_SUPPORT_H_

extern int katar_diagact2_lib_initialize(char *, int);
extern void tam_lib_platform_debug(void *platform_opaque_handle,boolean bSetting);
extern void tam_lib_platform_add_delay(void *platform_opaque_handle,int delayms);

#define SUCCEED             0
#define I2C_SLAVE           0x0703
#define SEGMENT_I2C_READ    511
#define ACT_RETRY           100
#define ACT_DELAY           200

#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */


/*
 *------------------------------------------------------------------
 * $Log: tam_act2_api_drv_support.h,v $
 * Revision 1.2  2019/06/14 05:24:52  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.4  2019/03/14 03:58:52  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.3  2019/02/12 08:06:31  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2018/12/06 08:32:25  mikech2
 * Fine-tune Aikido I2C r/w and fix Aikido update FW utility
 *
 * Revision 1.1.2.1  2018/10/22 08:02:31  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.2  2018/07/12 08:02:08  peteteng
 * add tam_lib_platform_write/read
 *
 * Revision 1.1.2.1  2018/06/26 06:30:09  peteteng
 * Add Aikido Cookie menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

