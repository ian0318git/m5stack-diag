/* $Id: diag_lte_util.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_lte_util.h,v $ 
 *------------------------------------------------------------------
 * 
 * diag_lte_util.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
 * $Log: diag_lte_util.h,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.1  2018/07/09 08:27:34  olin2
 * CSCvk17781: Support util to verify SIM Detect pin
 *
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
