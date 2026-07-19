/* $Id: SerdesRxOptDeviceIf.h,v 1.1 2015/02/13 11:33:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/SerdesRxOptDeviceIf.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* SerdesRxOptDeviceIf.h
*
* DESCRIPTION:
*		Serdes device interface definition
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __serdesRxOptDevIf_H
#define __serdesRxOptDevIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <serdesOpt/SerdesRxOptimizer.h>
#include <serdesOpt/private/serdesOptPrivate.h>

/*******************************************************************************
* mvSerdesLaneRegistration
*
* DESCRIPTION:
*       Init all global variables for tunning algorithm.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*******************************************************************************/
int mvSerdesLaneRegistration
(
    unsigned int    devNum, 
    MV_ACCESS_FUNCS *accFuncs
);

#ifdef __cplusplus
}
#endif

#endif /* __serdesRxOptDevIf_H */


/*
 *------------------------------------------------------------------
 * $Log: SerdesRxOptDeviceIf.h,v $
 * Revision 1.1  2015/02/13 11:33:35  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
