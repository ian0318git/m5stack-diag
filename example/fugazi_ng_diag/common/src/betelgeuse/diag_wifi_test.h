/* $Id: diag_wifi_test.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_wifi_test.h,v $
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
extern void diag_wifi_test(boolean);
extern int diag_wifi_module_bootup_test(void);
extern int diag_wifi_memory_test(void);
extern int diag_wifi_nor_flash_test(void);
extern int diag_wifi_temp_sensor_reg_test(void);
extern int diag_wifi_led_test(void);

#define WIFI_KERNEL_FILE "/var/lib/tftpboot/tsn_wifi_diag_kernel.img.SSA" 
#endif   /* __DIAG_WIFI_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_wifi_test.h,v $
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
