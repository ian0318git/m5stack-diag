/* $Id: plug_lte_util.h,v 1.6 2018/11/23 09:15:08 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_util.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_util.h - Header file for Pluggable LTE Utilities
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_UTIL__
#define __PLUG_LTE_UTIL__

/* Todo: Need to consider 2nd pluggable Slot */

#define AT_COMMAND_UTIL_DELAY               (1000)

typedef enum {
    OPT_READ,
    OPT_WRITE
} reg_util_opt_t;

typedef enum {
    OPT_DISABLE,
    OPT_ENABLE 
} debug_usb_util_opt_t;

typedef enum {
    OPT_OFF,
    OPT_ON 
} led_util_opt_t;

typedef enum {
    OPT_GREEN = 0,
    OPT_YELLOW 
} led_color_opt_t;

extern int plug_lte_util(void);
extern int plug_lte_modem_temp_util(int);

#endif

/*-------------------------------------------------
$Log: plug_lte_util.h,v $
Revision 1.6  2018/11/23 09:15:08  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.5.46.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.5  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.1  2018/03/02 03:29:33  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:46  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:56:37  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:09  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.3  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.2  2017/08/08 07:42:14  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.5  2017/07/11 18:29:34  tirawan
Add AT command utility and change the RSSI frequency to 944.5 Mhz

Revision 1.1.2.4  2017/06/28 00:46:18  shjung
Add pluggable LTE debug usb utility

Revision 1.1.2.3  2017/06/27 22:45:24  shjung
Add pluggable LTE LED utility

Revision 1.1.2.2  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

