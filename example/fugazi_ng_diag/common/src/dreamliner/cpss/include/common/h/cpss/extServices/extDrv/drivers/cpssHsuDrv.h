/* $Id: cpssHsuDrv.h,v 1.1 2015/02/13 11:31:00 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/common/h/cpss/extServices/extDrv/drivers/cpssHsuDrv.h,v $
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
* cpssHsuDrv.h
*
* DESCRIPTION:
*       Includes HSU function for HSU functionality
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssHsuDrvh
#define __cpssHsuDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/*******************************************************************************
* extDrvHsuMemBaseAddrGet
*
* DESCRIPTION:
*       Get start address of HSU area
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       hsuPtr      - Pointer to beginning of HSU area
*
* RETURNS:
*       GT_OK if successful, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None
*
*******************************************************************************/

typedef GT_STATUS (*CPSS_EXTDRV_HSU_MEM_BASE_ADDR_GET_FUNC)
(
    OUT  GT_U32  **hsuPtr
);

/*******************************************************************************
* extDrvHsuWarmRestart
*
* DESCRIPTION:
*       Performs warm restart of the 8548 cpu
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       No return
*
* COMMENTS:
*       None
*
*******************************************************************************/

typedef GT_VOID (*CPSS_EXTDRV_HSU_WARM_RESTART_FUNC)
(
    GT_VOID
);

/*******************************************************************************
* extDrvHsuInboundSdmaEnable
*
* DESCRIPTION:
*       This routine enables inbound sdma access on 8548 cpu.
*
* INPUTS:
*       none.
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/

typedef GT_STATUS (*CPSS_EXTDRV_HSU_INBOUND_SDMA_ENABLE_FUNC)
(
    GT_VOID
);


/*******************************************************************************
* extDrvHsuInboundSdmaDisable
*
* DESCRIPTION:
*       This routine disables inbound sdma access on 8548 cpu.
*
* INPUTS:
*       none.
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_EXTDRV_HSU_INBOUND_SDMA_DISABLE_FUNC)
(
    GT_VOID
);


/*******************************************************************************
* extDrvHsuInboundSdmaStateGet
*
* DESCRIPTION:
*       This routine gets the state of inbound sdma access on 8548 cpu.
*
* INPUTS:
*       none.
*
* OUTPUTS:
*       enablePtr  - (pointer to) inbound sdma status .
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/

typedef GT_STATUS (*CPSS_EXTDRV_HSU_INBOUND_SDMA_STATE_GET_FUNC)
(
     OUT GT_BOOL *enablePtr
);

/* CPSS_EXT_DRV_HSU_DRV_STC -
    structure that hold the "external driver HSU functions" that are
    bound to cpss.
*/
typedef struct{
    CPSS_EXTDRV_HSU_MEM_BASE_ADDR_GET_FUNC         extDrvHsuMemBaseAddrGetFunc;
    CPSS_EXTDRV_HSU_WARM_RESTART_FUNC              extDrvHsuWarmRestartFunc;
    CPSS_EXTDRV_HSU_INBOUND_SDMA_ENABLE_FUNC       extDrvHsuInboundSdmaEnableFunc;
    CPSS_EXTDRV_HSU_INBOUND_SDMA_DISABLE_FUNC      extDrvHsuInboundSdmaDisableFunc;
    CPSS_EXTDRV_HSU_INBOUND_SDMA_STATE_GET_FUNC    extDrvHsuInboundSdmaStateGetFunc;
}CPSS_EXT_DRV_HSU_DRV_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssHsuDrvh */




/*
 *------------------------------------------------------------------
 * $Log: cpssHsuDrv.h,v $
 * Revision 1.1  2015/02/13 11:31:00  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
