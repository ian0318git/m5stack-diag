/* $Id: bcm_epdm_id.h,v 1.1 2020/01/09 01:01:48 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/bcm_epdm_id.h,v $
 *------------------------------------------------------------------
 *
 * bcm_epdm_id.c - Broadcom EPDM id allocation APIs
 *
 * May 10, 2019, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __BCM_EPDM_ID__
#define __BCM_EPDM_ID__

extern int bcm_epdm_alloc_id(void);
extern void bcm_epdm_free_id(int id);

#endif

/*
 *-----------------------------------------------------------------------------
$Log: bcm_epdm_id.h,v $
Revision 1.1  2020/01/09 01:01:48  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
