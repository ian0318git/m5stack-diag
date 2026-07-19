/* $Id: plug_lte_test.h,v 1.10 2020/01/17 03:06:05 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_test.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_test.h - Header file for Pluggable LTE 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_TEST__
#define __PLUG_LTE_TEST__

#define LTE_SIM_MUX_SWITCH_DELAY                    (500)

#define RSSI_B8_FREQ                                "944.5"
#define RSSI_B4_FREQ                                "2134.5"
#define RSSI_B1_FREQ                                "2142"
#define RSSI_AMP                                    "-70"

#define SYS_SUPPRESS_PRINTK                         "dmesg -n 1"

#define PROBE_LTE_USB_TOUT                          (60000)
#define MODEM_USB_RENUM_DELAY                       (1000)
#define MODEM_USB_RESET_DELAY                       (2000)
#define PLUG_MODULE_I2C_UNRESET_DELAY               (1000)
#define LTE_USB_ENUM_TOUT                           (10000)
#define LTE_USB_SWITCH_MODE_TOUT                    (1500)
#define GPS_FIXES_MAX_RETRY_TIME                    (6000)
#define MAX_RESET_TIME                              (3)
#define USB2P0_MAX_RETRY_TIME                       (4)

#define PLUG_LTE_FPGA_I2C_ACK_MUX                   (0)
#define PLUG_LTE_ACT2_ADD                           (0xE6 >> 1)
#define PLUG_LTE_FPGA_I2C_ACK_REG_ADD               (0)
#define PLUG_LTE_FPGA_I2C_ACK_SUB_ADD               (1)
#define PLUG_LTE_FPGA_I2C_ACK_DATA_LEN              (1)

extern int plug_lte_main(void *);
extern int modem_is_shutdown;

#endif

/*-------------------------------------------------
$Log: plug_lte_test.h,v $
Revision 1.10  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.9  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.8  2019/06/14 05:48:11  shjung
Supported WP7605 modules

Revision 1.7  2018/11/23 09:15:08  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.6.40.3  2018/11/21 01:02:50  shjung
Added GPIO expander test register table and modified RF test macro name based on test RF band

Revision 1.6.40.2  2018/10/24 06:23:21  shjung
Added I2C reset pin test in I/O interface test, and modified kernel debug message level

Revision 1.6.40.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.6  2018/04/13 09:35:00  shjung

1. Fix CSCvh79986 and CSCvh79979: Added modem tty device file descriptor
   slef test to ensure communication between host and modem is good
2. Modified code based on Pluggable LTE WP7601/03 ER code review
3. Put all cterr functions to the outer file
4. Modified modem USB device enumeration timeout and GPS pin vaule polling
   timeout

Revision 1.5  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.2  2018/03/23 06:53:39  shjung
Modified USB2.0 detetcion test reset and retry mechanism

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:46  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.4  2018/02/01 23:41:02  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.3  2018/01/22 01:09:02  shjung
Code clean up

Revision 1.1.4.12  2018/01/22 01:07:12  shjung
Code clean up

Revision 1.1.4.11  2018/01/09 06:10:00  shjung
Added test criteria for the hold-up time of super caps, which are used for dying gasp feature

Revision 1.1.4.10  2017/12/15 08:16:52  shjung
Set longer polling time for modem reset test

Revision 1.1.4.9  2017/12/13 15:14:51  shjung
Added dying gasp test for pluggable LTE-EM module

Revision 1.1.4.8  2017/11/08 02:56:07  shjung
Modified the timeout mechanism of GPS pin test

Revision 1.1.4.7  2017/10/30 14:15:14  shjung
Added GPS pin test for LTE-WP module

Revision 1.1.4.6  2017/10/25 04:40:51  shjung
Modified pluggable module USB interface power-on/off sequence and USB interface mode configuration

Revision 1.1.4.5  2017/09/06 01:37:27  shjung
Code clean up.

Revision 1.1.4.4  2017/08/30 02:03:46  shjung
Update AT command for modem reset and ensure modem finish reset test

Revision 1.1.4.3  2017/08/28 07:53:27  shjung
Added pluggable I/O interface test

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.3  2017/07/24 22:51:25  tirawan
Add Pluggable AT command functions

Revision 1.1.2.2  2017/07/20 17:22:50  tirawan
Add USB 2.0 test and Debug port, and host implementation function prototype

Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.2  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

