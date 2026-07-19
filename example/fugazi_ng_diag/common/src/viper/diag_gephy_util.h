 /* $Id: diag_gephy_util.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_gephy_util.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_util.h
 * Description: Header file of GE PHY Utility
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
 * $Log: diag_gephy_util.h,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/04/26 08:14:16  lucywang
 * Added utility to set 88E1514 EEE
 *
 * Revision 1.1.2.2  2018/03/16 01:59:55  olin2
 * Support GE PHY testmode util
 *
 * Revision 1.1.2.1  2018/02/27 08:06:43  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

