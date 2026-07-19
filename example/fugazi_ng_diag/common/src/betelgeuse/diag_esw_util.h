/* $Id: diag_esw_util.h,v 1.2 2019/01/10 06:36:26 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_esw_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int esw_send_packet_util(int);

extern int diag_esw_set_allports_forward_util(void);
extern int diag_esw_reg_rd_util(void);
extern int diag_esw_reg_wr_util(void);
extern int diag_esw_phy_reg_rd_util(void);
extern int diag_esw_phy_reg_wr_util(void);
extern int diag_esw_smi_c45_rd_util(void);
extern int diag_esw_smi_c45_wr_util(void);
extern int diag_esw_set_1k_testmode_util(void);
extern int diag_esw_force_led_onoff_util(void);
extern int diag_esw_port_vod_adjust_util(void);

/*-------------------------------------------------
 * $Log: diag_esw_util.h,v $
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
