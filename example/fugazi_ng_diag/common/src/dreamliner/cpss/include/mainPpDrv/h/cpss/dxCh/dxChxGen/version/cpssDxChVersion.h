/* $Id: cpssDxChVersion.h,v 1.1 2015/02/13 11:33:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/version/cpssDxChVersion.h,v $
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
* cpssDxChVersion.h
*
* DESCRIPTION:
*       Includes software CPSS DxCh version information.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDxChVersionh
#define __cpssDxChVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/version/cpssGenVersion.h>

/*******************************************************************************
* cpssDxChVersionGet
*
* DESCRIPTION:
*       This function returns CPSS DxCh version.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       versionPtr     - (pointer to)CPSS DxCh version info.
*
* RETURNS:
*       GT_OK       - on success
*       GT_BAD_PTR  - one parameter is NULL pointer
*       GT_BAD_SIZE - the version name is too long
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChVersionGet
(
    OUT CPSS_VERSION_INFO_STC   *versionPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChVersionh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChVersion.h,v $
 * Revision 1.1  2015/02/13 11:33:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
