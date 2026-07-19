/* $Id: prvCpssDxChBrg.h,v 1.1 2015/02/13 11:31:32 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h,v $
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
*
* prvCpssDxChBrg.h
*
* DESCRIPTION:
*       Common private bridge declarations.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChBrgh
#define __prvCpssDxChBrgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

/*******************************************************************************
* prvCpssDxChBrgCpuPortBmpConvert
*
* DESCRIPTION:
*       Convert port bitmap according to physical CPU port connection.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Xcat; Xcat2.
*
* INPUTS:
*       devNum              - device number
*       isWriteConversion   - direction of conversion
*                             GT_TRUE - write conversion
*                             GT_FALSE - read conversion
*       portBitmapPtr       - (pointer to) bmp of ports members in vlan
*                             CPU port supported
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error.
*       GT_HW_ERROR     - on hardware error
*       GT_BAD_PARAM    - wrong devNum
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChBrgCpuPortBmpConvert
(
    IN GT_U8                devNum,
    IN GT_BOOL              isWriteConversion,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgh */
/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChBrg.h,v $
 * Revision 1.1  2015/02/13 11:31:32  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
