/* $Id: diag_esw_util.h,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_esw_util.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_esw_util.h 
 * Description: Diagnostic ethernet switch utility header
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_ESW_UTIL__
#define __DIAG_ESW_UTIL__

extern int diag_esw_pcie_config_rd_util (void);
extern int diag_esw_pcie_config_wr_util (void);
extern int diag_esw_xcat3_reg_rd_util (void);
extern int diag_esw_xcat3_reg_wr_util (void);
extern int diag_esw_xcat3_internal_reg_rd_util (void);
extern int diag_esw_xcat3_internal_reg_wr_util (void);
extern int diag_esw_phy_reg_rd_util (void);
extern int diag_esw_phy_reg_wr_util (void);
extern int diag_esw_phy_led_util (void);
extern int diag_esw_phy_test_mode_util (void);
extern int diag_esw_set_ixia_snake_config_util (uint);
extern int diag_esw_xcat3_10g_kr_test_mode_util (void);
extern int diag_esw_xcat3_serdes_tx_config_read_util(void);
extern int diag_esw_xcat3_serdes_tx_config_write_util (void);
extern int diag_esw_xcat3_phy_tx_config_read_util (void);
extern int diag_esw_xcat3_phy_tx_config_write_util (void);
extern int diag_esw_set_ixia_speed_config_util (uint);


#endif   /* __DIAG_ESW_UTIL__ */

/*-------------------------------------------------
 * $Log: diag_esw_util.h,v $
 * Revision 1.2  2019/12/11 10:10:29  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
