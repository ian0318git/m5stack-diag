/* $Id:
 * $Source:
 *------------------------------------------------------------------
 * 
 * platform_cookie.h
 *
 * Copyright (c) 2019 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

#define HR_V01   "V01"
#define HR_v01   "v01"
#define PLATFORM_BUFF_SIZE          259
#define CONTROL_TYPE_LEN            20
#define PRODUCT_NAME_LEN            256
#define PRODUCT_SERIAL_LEN          20
#define VID_LEN                     20


extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *, char *);
extern int get_pcb_serial(uchar *, char *);
extern int alter_plug_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);
extern int get_tlv_serial (uchar *, char *, uchar);

#endif /* _PLATFORM_COOKIE_H_ */

/*-------------------------------------------------
 * $Log: platform_cookie.h,v $
 * Revision 1.1  2020/08/19 09:49:35  markzha
 * *** empty log message ***
 *
 *
 *-------------------------------------------------
 */
