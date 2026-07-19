 /* $Id: diag_gephy_util.h,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_gephy_util.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_util.h
 * Description: Header file of GE PHY Utility
 * 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_GE_PHY_UTIL_H__
#define __DIAG_GE_PHY_UTIL_H__

#define SENDUTIL_PKT_CNT                    (3)
#define TABEI_SGMII_AMP_VAL                 0x6
#define WAITING_FOR_1514_INIT               100

extern int diag_gephy_read_reg_util(void);
extern int diag_gephy_alter_reg_util(void);
extern int diag_gephy_dump_reg_util(void);
extern int diag_gephy_send_pkt_util(void);
extern int diag_gephy_testmode_util(void);
extern int diag_gephy_eee_util(void);
extern int diag_gephy_init_util(void);
extern int diag_gephy_1514_init(int);
extern int diag_gephy_set_sgmii_amp(int);

#endif


/*-------------------------------------------------
 * $Log: diag_gephy_util.h,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.3  2019/03/12 07:24:09  olin2
 * Adjust SGMII output amp and revise GE PHY init sequence
 *
 * Revision 1.1.2.2  2018/12/25 06:38:40  olin2
 * Support initial GE PHY util
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */

