/* $Id: diag_lte_util.h,v 1.4 2019/07/11 12:31:29 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_lte_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_lte_util.h
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DIAG_LTE_UTIL_H_
#define _DIAG_LTE_UTIL_H_


#define LTE_TESTMSG_BUFSZ  32   /* 32 bytes */


enum sim_stat {
   SIM_NOT_PRESENT = 0,
   SIM_PRESENT,
};

extern void build_lte_utils_menu(void);



#endif /* _DIAG_LTE_UTIL_H_ */

/*-------------------------------------------------
$Log: diag_lte_util.h,v $
Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
