/* $Id: platform_cookie.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/platform_cookie.h,v $
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

#define XDSL_CTRL_TYPE_ANNEX_A      0
#define XDSL_CTRL_TYPE_ANNEX_M      1
#define XDSL_CTRL_TYPE_ANNEX_BJ     2
#define XDSL_CTRL_TYPE_UNKNOWN      3

typedef enum {
    ESW_UNDEFINED = 0,              /* A default value before reading platform cookie */
    ESW_MRVL88E6390,                /* Marvell 88E6390: for 8-port switch PHY */
    ESW_MRVL88E6176,                /* Marvell 88E6176: for 4-port switch PHY */
} PHY_CHIP_TYPE;
#define PLATFORM_PID_LIST_LENGTH 25
struct pid_list {
    uchar   *pid;
    boolean wifi_flag;
    boolean pluggable_flag;
    boolean ge1_flag;
    boolean xdsl_flag;
    boolean sirius_fpga_flag;
    boolean gshdsl_flag;
    int     esw_type;
};

extern uchar *get_current_pid(void);
extern int initial_current_product_id(void);
extern int platform_has_wifi(void);
extern int platform_has_pluggable(void);
extern int platform_has_sirius_fpga(void);
extern int platform_has_2nd_ge(void);
extern int platform_has_xdsl(void);
extern int platform_has_gshdsl(void);
extern int platform_esw_type(void);
extern int xdsl_control_type(void);
extern void show_xdsl_control_type (void);

extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *,
                                    char *);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial (uchar *, char *, uchar);
extern ushort get_mb_id(void);
extern int alter_mb_cookie(void);
extern int alter_wifi_cookie(void);
extern int alter_poe_cookie(void);
extern int alter_plug_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);
extern int tam_act2_reset(int);

#endif /* _PLATFORM_COOKIE_H_ */

/*-------------------------------------------------
 * $Log: platform_cookie.h,v $
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
