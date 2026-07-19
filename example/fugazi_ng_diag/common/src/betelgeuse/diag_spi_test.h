/* $Id: diag_spi_test.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_spi_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_spi_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define PLAT_BF_BUSNUM   0

extern int quiet_launch;
extern int show_plat_curr_temps(void);

extern int spi_slot_tests(int);
extern int diag_bootflash_test(int);

/*-------------------------------------------------
 * $Log: diag_spi_test.h,v $
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
