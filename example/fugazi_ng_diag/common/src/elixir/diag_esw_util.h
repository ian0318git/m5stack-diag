/* $Id: diag_esw_util.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_esw_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int diag_esw_pcie_config_rd_util (void);
extern int diag_esw_pcie_config_wr_util (void);
extern int diag_esw_xcat5_internal_reg_rd_util (void);
extern int diag_esw_xcat5_internal_reg_wr_util (void);
extern int diag_esw_xcat5_reg_rd_util (void);
extern int diag_esw_xcat5_reg_wr_util (void);
extern int diag_esw_phy_reg_rd_util (void);
extern int diag_esw_phy_reg_wr_util (void);
extern int diag_esw_phy_test_mode_util (void);

/*-------------------------------------------------
 * $Log: diag_esw_util.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.7  2021/05/31 10:47:29  illiu
 * Remove function diag_esw_xcat5_phy_tx_config_read/write_util
 *
 * Revision 1.1.2.6  2020/12/04 08:37:14  illiu
 * Add 1680 PHY Test Mode Utility
 *
 * Revision 1.1.2.5  2020/10/06 02:06:32  illiu
 * Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
 *
 * Revision 1.1.2.4  2020/09/28 10:36:17  illiu
 * Add below utility items:
 * 1. ESW PHY Register Read Utility
 * 2. ESW PHY Register Write Utility
 * 3. ESW 88E1680 Tx Config Read Utility
 * 4. ESW 88E1680 Tx Config Write Utility
 *
 * Revision 1.1.2.3  2020/09/26 03:34:23  illiu
 * Add below Utilities items:
 *     ESW PCI Config Read Utility
 *     ESW PCI Config Write Utility
 *     ESW xCat3 Internal Register Write Utility
 *     ESW xCat3 PP Register Read Utility
 *     ESW xCat3 PP Register Write Utility
 *     Print All PHY Counter Utility
 *     Clear All PHY Counter Utility
 *     Print xCat3 Counter Utility
 *     Clear xCat3 Counter Utility
 *     ESW Reset Default Utility
 *
 * Revision 1.1.2.2  2020/09/10 09:52:31  illiu
 * Delete 88E6390/88E6176 Switch related code
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
