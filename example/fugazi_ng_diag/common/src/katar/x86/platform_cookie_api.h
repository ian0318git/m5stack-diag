/* $Id: platform_cookie_api.h,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_cookie_api.h,v $
 *------------------------------------------------------------------
 *
 * katar_platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define CONTROL_TYPE_LEN            20
#define PRODUCT_SERIAL_LEN          20
#define PRODUCT_NAME_LEN            256

extern int katar_alter_mb_cookie(void);

#endif /* _PLATFORM_COOKIE_H_ */

/*
 *------------------------------------------------------------------
 * $Log: platform_cookie_api.h,v $
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.1  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2018/10/22 08:02:27  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.3  2018/10/22 03:04:32  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.2  2018/06/29 03:40:01  peteteng
 * Add ACT2 utility Menu
 *
 * Revision 1.1.2.1  2018/06/26 06:30:09  peteteng
 * Add Aikido Cookie menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

