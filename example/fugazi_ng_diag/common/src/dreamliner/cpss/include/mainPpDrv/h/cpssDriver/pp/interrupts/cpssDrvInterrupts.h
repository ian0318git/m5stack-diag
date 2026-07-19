/* $Id: cpssDrvInterrupts.h,v 1.1 2015/02/13 11:35:32 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpssDriver/pp/interrupts/cpssDrvInterrupts.h,v $
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
* cpssDrvInterrupts.h
*
* DESCRIPTION:
*       Includes general definitions for the interrupts handling unit.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDrvInterruptsh
#define __cpssDrvInterruptsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>

/*******************************************************************************
* cpssDrvInterruptsTreeGet
*
* DESCRIPTION:
*       function return :
*       1. the root to the interrupts tree info of the specific device
*       2. the interrupt registers that can't be accesses before 'Start Init'
*
* APPLICABLE DEVICES: All devices --> Packet Processors (not Fa/Xbar)
*
* INPUTS:
*       devNum - the device number
*
* OUTPUTS:
*       numOfElementsPtr - (pointer to) number of elements in the tree.
*       treeRootPtrPtr - (pointer to) pointer to root of the interrupts tree info.
*       numOfInterruptRegistersNotAccessibleBeforeStartInitPtr - (pointer to)
*                           number of interrupt registers that can't be accessed
*                           before 'Start init'
*       notAccessibleBeforeStartInitPtrPtr (pointer to)pointer to the interrupt
*                           registers that can't be accessed before 'Start init'
*
* RETURNS:
*       GT_OK - on success
*       GT_BAD_PARAM             - wrong devNum
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_INITIALIZED       - the driver was not initialized for the device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS   cpssDrvInterruptsTreeGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *numOfElementsPtr,
    OUT const CPSS_INTERRUPT_SCAN_STC        **treeRootPtrPtr,
    OUT GT_U32  *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    OUT GT_U32  **notAccessibleBeforeStartInitPtrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDrvInterruptsh */


/*
 *------------------------------------------------------------------
 * $Log: cpssDrvInterrupts.h,v $
 * Revision 1.1  2015/02/13 11:35:32  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
