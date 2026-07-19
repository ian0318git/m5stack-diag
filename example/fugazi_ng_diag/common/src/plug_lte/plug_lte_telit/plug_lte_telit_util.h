/* $Id: plug_lte_telit_util.h,v 1.2 2019/05/14 08:48:37 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_util.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_util.h - Header File for Pluggable LTE Telit
 *                         Utility
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_LTE_TELIT_UTIL_H__
#define __PLUG_LTE_TELIT_UTIL_H__

#define AT_COMMAND_UTIL_DELAY                 (1000)
#define LTE_TESTMSG_BUFSZ                     (256)

typedef enum {
    OPT_READ,
    OPT_WRITE
} reg_util_opt_t;

typedef enum {
    OPT_OFF,
    OPT_ON 
} led_util_opt_t;

typedef enum {
    OPT_GREEN = 0,
    OPT_YELLOW 
} led_color_opt_t;

typedef enum {
    OPT_DISABLE,
    OPT_ENABLE
} debug_usb_util_opt_t;

typedef enum {
    SIM_NOT_PRESENT,
    SIM_PRESENT
} simdet_pin_stat_t;

typedef enum {
    OPERATION_MODE,
    TEST_MODE
} testmode_stat_t;

extern int plug_lte_telit_util(void);
extern int plug_lte_telit_modem_temp_util(int);

#endif /* __PLUG_LTE_TELIT_UTIL_H__ */

/*------------------------------------------------------------------
$Log: plug_lte_telit_util.h,v $
Revision 1.2  2019/05/14 08:48:37  sherliu2
Support hyperloop

Revision 1.1.2.3  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.2  2019/01/18 13:40:28  shjung
Added utility to control all LEDs

Revision 1.1.2.1  2018/12/14 00:50:16  shjung
Initial check-in for Hyperloop



$Endlog$
*/
