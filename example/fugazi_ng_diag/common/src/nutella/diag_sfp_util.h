/* $Id: diag_sfp_util.h,v 1.1 2020/08/07 09:02:35 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_sfp_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_sfp_util.h
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DIAG_SFP_UTIL_H_
#define _DIAG_SFP_UTIL_H_

extern int igb_read_sfp_phy_util(void);
extern int igb_write_sfp_phy_util(void);
extern int igb_read_sfp_phy(int, ushort, ushort *);
extern int igb_write_sfp_phy(int, ushort, ushort);

#endif /* DIAG_SFP_UTIL_H_ */

/*-------------------------------------------------
$Log: diag_sfp_util.h,v $
Revision 1.1  2020/08/07 09:02:35  alicehua
CSCvv24244: Add SFP PHY read/write utilities.


$Endlog$
*/
