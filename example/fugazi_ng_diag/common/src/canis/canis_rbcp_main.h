/* $Id: canis_rbcp_main.h,v 1.2 2012/06/08 06:45:05 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/canis_rbcp_main.h,v $
 *------------------------------------------------------------------
 * Filename: canis_rbcp_main.h
 *
 * Description: The RBCP main code header file
 * Author: Times Huang
 *
 * Copyright (c) 2012 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef CANIS_RBCP_MAIN_H_
#define CANIS_RBCP_MAIN_H_

extern int canis_rbcp_menu(void);
extern int canis_rbcp_con_sw_bmc(void);
extern int canis_rbcp_con_sw_intel(void);

#endif /* CANIS_RBCP_MAIN_H_ */

/*------------------------------------------------------------------
 * $Log: canis_rbcp_main.h,v $
 * Revision 1.2  2012/06/08 06:45:05  hondwang
 * Fix canis complier warning on O2 x86
 *
 * Revision 1.1  2012/03/29 18:46:42  ksabzwar
 * Initial check in into ng_diag
 *
 * Revision 1.1.4.2  2012/03/10 01:18:28  ksabzwar
 * First check-in for Canis user menu for Overloard platform
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
