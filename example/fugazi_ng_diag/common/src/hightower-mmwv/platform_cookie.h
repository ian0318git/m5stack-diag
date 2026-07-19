/* $Id: platform_cookie.h,v 1.1 2020/08/19 09:50:05 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_cookie.h,v $
 *------------------------------------------------------------------
 * 
 * platform_cookie.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

#define HT_V01   "V01"

#define SET_ALREADY                 1
#define NOT_SET                     0
#define PLATFORM_BUFF_SIZE          259
#define CONTROL_TYPE_LEN            20
#define PRODUCT_NAME_LEN            256
#define PRODUCT_SERIAL_LEN          20
#define VID_LEN                     20
#define ACT2_RESET_UNRESET_DELAY    (500)
#define QUACK_RETRY                 8
#define ACT2_UNRESET_DELAY          (5000)

typedef enum {
    ESW_UNDEFINED = 0,              /* A default value before reading platform cookie */
    ESW_MRVL88E6390,                /* Marvell 88E6390: for 8-port switch PHY */
    ESW_MRVL88E6176,                /* Marvell 88E6176: for 4-port switch PHY */
} PHY_CHIP_TYPE;
#define PLATFORM_PID_LIST_LENGTH 25
struct pid_list {
    uchar   *pid;
    boolean pluggable_flag;
};

extern int initial_current_product_id(void);
extern int platform_has_pluggable(void);
extern int xdsl_control_type(void);

extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *,
                                    char *);
extern int get_pcb_serial(uchar *, char *);
extern int alter_plug_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);
extern int tam_act2_reset(int);
extern int get_tlv_serial (uchar *, char *, uchar);

#endif /* _PLATFORM_COOKIE_H_ */

/*-------------------------------------------------
 * $Log: platform_cookie.h,v $
 * Revision 1.1  2020/08/19 09:50:05  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
