 /*------------------------------------------------------------------
 *
 * diag_lte_telit_util.h - Header File for LTE Telit
 *                         Utility
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __LTE_TELIT_UTIL_H__
#define __LTE_TELIT_UTIL_H__

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

extern int diag_lte_telit_util(void);
extern int diag_lte_telit_modem_temp_util(int);

#endif /* __LTE_TELIT_UTIL_H__ */

/*------------------------------------------------------------------
$Log: diag_lte_telit_util.h,v $
Revision 1.1  2020/08/19 09:49:35  markzha
*** empty log message ***

$Endlog$
*/
