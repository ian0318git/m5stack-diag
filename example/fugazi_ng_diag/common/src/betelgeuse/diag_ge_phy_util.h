/* $Id: diag_ge_phy_util.h,v 1.2 2019/01/10 06:36:26 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_ge_phy_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_ge_phy_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int diag_util_ge_rd_reg(int);
extern int diag_util_ge_wr_reg(int);
extern int diag_util_ge_set_test_mode(int);
extern int diag_util_ge_set_tx_type(int);
extern int diag_util_ge_set_vod(int);
extern int diag_util_ge_send_packet_util (int);
extern int diag_util_ge_led(int);
extern int gephy_set_txtype_util(int);

/*-------------------------------------------------
 * $Log: diag_ge_phy_util.h,v $
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
