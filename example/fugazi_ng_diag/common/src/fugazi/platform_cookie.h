/* $Id: platform_cookie.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

#define QUACK_RETRY                   (8)
#define PLATFORM_BUFF_SIZE          (259)
#define CONTROL_TYPE_LEN             (20)
#define PRODUCT_NAME_LEN            (256)
#define PRODUCT_SERIAL_LEN           (20)
#define VID_LEN                      (20)

#define SUPPORT_DISCRETE_AIKIDO_ACT2  (1)

extern boolean pcb_for_sudi;

extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *,
                                    char *);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial (uchar *, char *, uchar);
extern ushort get_mb_id(void);
extern int alter_mb_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);


#endif /* _PLATFORM_COOKIE_H_ */


/*-------------------------------------------------
 * $Log: platform_cookie.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:50  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
