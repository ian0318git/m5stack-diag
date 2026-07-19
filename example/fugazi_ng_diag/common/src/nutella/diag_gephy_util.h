/* $Id: diag_gephy_util.h,v 1.4 2019/07/11 12:31:28 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_gephy_util.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_util.h
 * Description: Header file of GE PHY Utility
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_GE_PHY_UTIL_H__
#define __DIAG_GE_PHY_UTIL_H__

#define SENDUTIL_PKT_CNT                    (3)

extern int diag_gephy_read_reg_util(void);
extern int diag_gephy_alter_reg_util(void);
extern int diag_gephy_dump_reg_util(void);
extern int diag_gephy_send_pkt_util(void);
extern int diag_gephy_testmode_util(void);
extern int diag_gephy_eee_util(void);

#endif


/*-------------------------------------------------
$Log: diag_gephy_util.h,v $
Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
