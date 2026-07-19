/* $Id: diag_ge_phy.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_ge_phy.h,v $
 *------------------------------------------------------------------
 * Filename: diag_ge_phy.h
 *
 * Description: Header File for GE PHY utils and tests.
 * Apr 2014, Xiaoying Zhang
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef DIAG_GE_PHY_H_
#define DIAG_GE_PHY_H_

#define DIAG_DO_ALL_PORT_BASE                    (2015)
#define DIAG_DO_ALL_INT_PORT_BASE                (2017)
#define DIAG_IP_ADDR                             "192.123.123.150"

#define INT_LPBK_TEST_LOG   "/tmp/int_lpbk_reg_dump"
#define SFP_LPBK_TEST_LOG   "/tmp/sfp_lpbk_reg_dump"
#define CU_LPBK_TEST_LOG    "/tmp/cu_lpbk_reg_dump"
#define BP_LPBK_TEST_LOG    "/tmp/bp_lpbk_reg_dump"
#define PTP_LPBK_TEST_LOG   "/tmp/ptp_lpbk_reg_dump"

#define FIBER_MODE  0
#define COPPER_MODE 1

extern int wallander_phy_init(boolean, int);
extern int phy_default_config(boolean, int);

extern int ge_phy_test(int);
extern int ge_phy_do_all_wrapper(void);

extern int get_num_ports(void);
extern int phy_int_lpbk_test();
extern int phy_sfp_ext_lpbk_test();
extern int phy_cu_ext_lpbk_test();
extern int phy_bp_ext_lpbk_test();
extern int phy_ptp_ext_lpbk_test();
extern int phy_lpbk_test();
extern int phy_lpbk_config();
extern int phy_cu_ext_lpbk_config();

extern int phy_reg_rd_util();
extern int phy_reg_wr_util();
extern int phy_reg_dp_util();
extern int phy_1588_reg_rd_util();
extern int phy_1588_reg_wr_util();
extern int phy_1588_reg_dp_util();
extern int phy_1588_tod_util();
extern int setup_phy_test_mode_util();
extern int fpga_1588_reg_dp_util();
extern int show_port_ctrl_stat_util();
extern int show_phy_test_log_util();
extern int smi_ctrl_reg_dp_util();

#endif /* DIAG_GE_PHY_H_ */


/*------------------------------------------------------------------
 * $Log: diag_ge_phy.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
