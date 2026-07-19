/* $Id: diag_dsl_util.h,v 1.3 2019/05/21 07:44:19 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dsl_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_dsl_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int xdsl_util_bcm63168_reset(void);
extern int xdsl_util_init_bcm63168(void);
extern int xdsl_util_config_bcm63168(void);
extern int xdsl_util_get_bcm63168_config(void);
extern int xdsl_util_get_bcm63168_version(void);
extern int xdsl_util_connection_start(void);
extern int xdsl_util_connection_stop(void);
extern int xdsl_util_get_xdsl_mib_info(void);
extern int xdsl_util_get_xtm_bonding_info(void);
extern int xdsl_util_get_xdsl_info(void);
extern int xdsl_util_get_connection_info(void);
extern int xdsl_util_bcm63168_led_test(void);
extern int xdsl_util_bcm63168_show_profile(void);
extern int xdsl_util_ping(void);
extern int xdsl_util_bcm63168_show_spi_flash_reg(void);
extern int xdsl_util_bcm63168_en_spi_flash_reg(void);
extern int xdsl_util_bcm63168_dis_spi_flash_reg(void);
extern int xdsl_util_restore_cfe_param(boolean);
extern int xdsl_util_bcm63168_led(int);
extern int xdsl_util_bcm63168_chk_spi_flash_protect(void);

/*-------------------------------------------------
 * $Log: diag_dsl_util.h,v $
 * Revision 1.3  2019/05/21 07:44:19  wilbhuan
 * Add a new xDSL utility to check the SPI Flash protection.
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
