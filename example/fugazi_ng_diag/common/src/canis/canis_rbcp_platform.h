/* $Id: canis_rbcp_platform.h,v 1.2 2012/10/11 07:28:20 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/canis_rbcp_platform.h,v $
 *------------------------------------------------------------------
 * Filename: canis_rbcp_platform.h
 *
 * Description: Platform dependent code header file
 * Author: Times Huang
 *
 * Copyright (c) 2012 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef CANIS_RBCP_PLATFORM_H_
#define CANIS_RBCP_PLATFORM_H_

extern int canis_rbcp_send(uint8_t *, int);
extern int canis_rbcp_recv(uint8_t *, int *);

#define MAX_NUM_CANIS_SLOTS	4

#endif /* CANIS_RBCP_PLATFORM_H_ */


/*------------------------------------------------------------------
 * $Log: canis_rbcp_platform.h,v $
 * Revision 1.2  2012/10/11 07:28:20  hondwang
 * porting multi card insert issue fix from G2. CSCua22608
 *
 * Revision 1.1  2012/03/29 18:46:42  ksabzwar
 * Initial check in into ng_diag
 *
 * Revision 1.1.4.2  2012/03/10 01:18:29  ksabzwar
 * First check-in for Canis user menu for Overloard platform
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
