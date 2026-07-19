/* $Id: plug_testcard_usb_lib.h,v 1.2 2018/11/23 09:10:40 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_usb_lib.h,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_usb_lib.h - head file for PLUGGABLE Test Card USB Functions
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_TESTCARD_USB__
#define __PLUG_TESTCARD_USB__

#define PLUG_TESTCARD_USB2P0_SPEED        (480)
#define PLUG_TESTCARD_USB3P0_SPEED        (5000)
#define PLUG_TC_USB_IGNORE                (5000)
#define SUPPRESS_MESG "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG "echo 4 > /proc/sys/kernel/printk"

extern int plug_testcard_usb_hub_test(int);
extern int plug_tc_usb_parse_info(void);
extern int plug_tc_usb_mass_stor_present_index(int , int *, int , int );
extern int plug_tc_usb_get_speed(int);

#endif
/*-------------------------------------------------
$Log: plug_testcard_usb_lib.h,v $
Revision 1.2  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.5  2018/11/16 06:40:49  hondwang
modify PRRQ suggest with CSCvn17216 pluggable re-instruct

Revision 1.1.2.4  2018/11/14 07:40:11  hondwang
Modify code for no serial number fix

Revision 1.1.2.3  2018/11/01 12:59:34  hondwang
Modify pluggable testcard USB Hub testing with random port

Revision 1.1.2.2  2018/11/01 06:25:06  hondwang
Add plug testcard USB HUB testing function

Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
pluggable common code re-instruct add and remove files


 
*/
