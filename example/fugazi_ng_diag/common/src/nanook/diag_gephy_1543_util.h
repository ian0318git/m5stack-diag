/* $Id: diag_gephy_1543_util.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_gephy_1543_util.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_1543_util.h
 * Description: Header file of GE PHY Utility
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_GE_PHY_1543_UTIL_H__
#define __DIAG_GE_PHY_1543_UTIL_H__

#define SENDUTIL_PKT_CNT                    (3)

extern int diag_gephy_1543_read_reg_util(void);
extern int diag_gephy_1543_alter_reg_util(void);
extern int diag_gephy_1543_dump_reg_util(void);
extern int diag_gephy_1543_send_pkt_util(void);
extern int diag_gephy_1543_testmode_util(void);
extern int diag_gephy_1543_eee_util(void);
extern int diag_gephy_1543_ping_config_util(int);
extern int diag_gephy_1543_sfp_send_pkt_util (void);
extern int diag_gephy_1543_force_phy_speed_util(void);

#endif


/*-------------------------------------------------
$Log: diag_gephy_1543_util.h,v $
Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
