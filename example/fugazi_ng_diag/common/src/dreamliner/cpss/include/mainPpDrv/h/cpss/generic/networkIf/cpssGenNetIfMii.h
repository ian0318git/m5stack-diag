/* $Id: cpssGenNetIfMii.h,v 1.1 2015/02/13 11:33:57 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/networkIf/cpssGenNetIfMii.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssGenNetIfTypes.h
*
* DESCRIPTION:    generic network interface MII types definitions
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssGenNetIfMiih
#define __cpssGenNetIfMiih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>

/* size of single internal TX buffer in Mii CPU mode */
#define CPSS_GEN_NETIF_MII_TX_INTERNAL_BUFF_SIZE_CNS 32

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __cpssGenNetIfTypesh */


/*
 *------------------------------------------------------------------
 * $Log: cpssGenNetIfMii.h,v $
 * Revision 1.1  2015/02/13 11:33:57  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
