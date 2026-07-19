/* $Id: platform_cookie.h,v 1.3 2018/02/09 09:56:54 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 


#define PLATFORM_BUFF_SIZE          259
#define CONTROL_TYPE_LEN            20
#define PRODUCT_NAME_LEN            256
#define PRODUCT_SERIAL_LEN          20
#define VID_LEN                     20
#define ACT2_RESET_UNRESET_DELAY    (500)
#define QUACK_RETRY                 8
#define ACT2_UNRESET_DELAY          (5000)


extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *,
                                    char *);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial (uchar *, char *, uchar);
extern ushort get_mb_id(void);
extern int alter_mb_cookie(void);
extern int alter_wifi_cookie(void);
extern int alter_poe_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);

#endif /* _PLATFORM_COOKIE_H_ */

/******** History ********
$Log: platform_cookie.h,v $
Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 07:21:47  hondwang
Fix some merge branch issue

Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3  2016/07/01 08:33:30  steja
Fix the unsupported old cookie format

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/24 10:35:04  steja
Add Cookie and Act2 programming



$Endlog$
*/
