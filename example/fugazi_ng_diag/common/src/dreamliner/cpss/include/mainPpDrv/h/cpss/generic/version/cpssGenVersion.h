/* $Id: cpssGenVersion.h,v 1.1 2015/02/13 11:35:20 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/version/cpssGenVersion.h,v $
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
* cpssGenVersion.h
*
* DESCRIPTION:
*       Includes software CPSS version information .
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssGenVersionh
#define __cpssGenVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

/* the max length for the string to define the CPSS version */
#define CPSS_VERSION_MAX_LEN_CNS 30
/*
 * Typedef: struct CPSS_VERSION_INFO_STC
 *
 * Description: This struct holds the CPSS for specific device family
 *              version info. (for example : ExMx / DxCh /DxSal ...)
 *
 * Fields:
 *      version - string array holding the version ID.
 *
 */
typedef struct
{
    GT_U8   version[CPSS_VERSION_MAX_LEN_CNS];
}CPSS_VERSION_INFO_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenVersionh */

/*
 *------------------------------------------------------------------
 * $Log: cpssGenVersion.h,v $
 * Revision 1.1  2015/02/13 11:35:20  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
