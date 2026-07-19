/* $Id: diag_wifi_test.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_wifi_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_wifi_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_WIFI_TEST_H__
#define __DIAG_WIFI_TEST_H__

/* Extern */
extern int diag_wifi_test(boolean);
extern int diag_wifi_module_bootup_test(void);
extern int diag_wifi_memory_test(void);
extern int diag_wifi_nor_flash_test(void);
extern int diag_wifi_temp_sensor_reg_test(void);
extern int diag_wifi_led_test(void);

#define WIFI_KERNEL_FILE "/var/lib/tftpboot/diag-wifi-kernel-image.bin"

#endif   /* __DIAG_WIFI_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_wifi_test.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.5  2021/03/05 07:13:47  illiu
 * Rename wifi kernel image name
 *
 * Revision 1.1.2.4  2021/03/03 01:33:39  illiu
 * Modify wifi bootup kernel from foxconn u-boot version to cisco u-boot version
 *
 * Revision 1.1.2.3  2020/11/12 06:37:09  illiu
 * 1. Add WiFi module Bootup Test item
 * 2. Add WiFi module Memory Test item
 * 3. Add WiFi module NOR flash Test item
 * 4. Fix WiFi LED control Util item
 *
 * Revision 1.1.2.2  2020/11/06 06:29:49  harrchan
 * Add port pve configuration before testing wifi
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
