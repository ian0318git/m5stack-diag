/* $Id: gtVersion.h,v 1.1 2015/02/13 11:31:19 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/common/h/cpss/generic/version/gtVersion.h,v $
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
* gtVersion.h
*
* DESCRIPTION:
*       Includes definitions for math function.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __gtVersionh
#define __gtVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>


#define VERSION_MAX_LEN 30
/*
 * Typedef: struct GT_VERSION
 *
 * Description: This struct holds the package version.
 *
 * Fields:
 *      version - string array holding the version.
 *
 */
typedef struct
{
    GT_U8   version[VERSION_MAX_LEN];
}GT_VERSION;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtVersionh */

/*
 *------------------------------------------------------------------
 * $Log: gtVersion.h,v $
 * Revision 1.1  2015/02/13 11:31:19  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
