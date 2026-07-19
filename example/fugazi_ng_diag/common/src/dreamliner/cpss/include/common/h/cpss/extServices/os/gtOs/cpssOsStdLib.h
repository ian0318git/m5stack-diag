/* $Id: cpssOsStdLib.h,v 1.1 2015/02/13 11:31:05 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/common/h/cpss/extServices/os/gtOs/cpssOsStdLib.h,v $
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
* cpssOsStdLib.h
*
* DESCRIPTION:
*       Operating System wrapper. stdlib functions
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssOsStdLibh
#define __cpssOsStdLibh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/************* Typedefs *******************************************************/
/*
 * Typedef: COMPARE_ITEMS_FUNC_PTR
 *
 * Description: Comparison function type
 *
 * Inputs:
 *      a - pointer to one item
 *      b - pointer to another item
 *
 * Outputs:
 *      None
 *
 * Returns:
 *      Less than zero if a < b
 *      Zero if a == b
 *      greather than zero if a > b
 *
 */
typedef int(*CPSS_OS_COMPARE_ITEMS_FUNC_PTR)
(
    const void *a,
    const void *b
);


/************* Functions ******************************************************/

/*******************************************************************************
* osQsort
*
* DESCRIPTION:
*       Sort an array
*
* INPUTS:
*       array      - Address of first element in array
*       nItems     - Number of items in array
*       itemSize   - Size of one memner
*       comparFunc - Function which compares two items
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_STDLIB_QSORT_FUNC)
(
    IN  void *                          array,
    IN  GT_U32                          nItems,
    IN  GT_U32                          itemSize,
    IN  CPSS_OS_COMPARE_ITEMS_FUNC_PTR  comparFunc
);

/*******************************************************************************
* osBsearch
*
* DESCRIPTION:
*       binary search of a sorted array
*
* INPUTS:
*       key        - key to search
*       array      - Address of first element in array
*       nItems     - Number of items in array
*       itemSize   - Size of one memner
*       comparFunc - Function which compares two items
*
* OUTPUTS:
*       result     - pointer to item found or NULL
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_STDLIB_BSEARCH_FUNC)
(
    IN  const void *                    key,
    IN  const void *                    array,
    IN  GT_U32                          nItems,
    IN  GT_U32                          itemSize,
    IN  CPSS_OS_COMPARE_ITEMS_FUNC_PTR  comparFunc,
    OUT GT_VOID_PTR                     *result
);


/* CPSS_OS_STDLIB_BIND_STC -
    structure that hold the "os stdlib" functions needed be bound to cpss.

*/
typedef struct{
    CPSS_OS_STDLIB_QSORT_FUNC           osQsortFunc;
    CPSS_OS_STDLIB_BSEARCH_FUNC         osBsearchFunc;
}CPSS_OS_STDLIB_BIND_STC;

#ifdef __cplusplus
}
#endif

#endif  /* __cpssOsStdLibh */
/* Do Not Add Anything Below This Line */

/*
 *------------------------------------------------------------------
 * $Log: cpssOsStdLib.h,v $
 * Revision 1.1  2015/02/13 11:31:05  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
