/* $Id: diag_moka_fpga_util.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_moka_fpga_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_moka_fpga_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int plat_show_fpga_ver(int);
extern int fpga_reg_rd_util(int);
extern int fpga_reg_wr_util(int);
extern int fpga_reg_dump_util(int);
extern int plat_status_led_utils(int);
extern int plat_pwrok_stat_led_utils(int);
extern int plat_poestat_led_utils(int);
extern int plat_poeport_led_utils(int);
extern int plat_aux_led_utils(int);
extern int plat_microusb_led_utils(int);
extern int plat_usb_led_utils(int);
extern int plat_console_led_utils(int);
extern int plat_vpn_led_utils(int);

/*-------------------------------------------------
 * $Log: diag_moka_fpga_util.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
