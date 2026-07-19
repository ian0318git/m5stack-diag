/* $Id: nim_dm_cpss_trace.h,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_cpss_trace.h,v $
 *------------------------------------------------------------------
 * DM CPSS lib external trace service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *---------------------------------------------------------------------------
 */

#ifndef __NIM_DM_CPSS_TRACE_H__
#define __NIM_DM_CPSS_TRACE_H__



/*
 * typedef: enum TRACE_HW_ACCESS_TYPE_ENT
 *
 * Description: PP access type enumeration
 *
 * Fields:
 *      TRACE_HW_ACCESS_TYPE_READ_E  - PP access type is read.
 *      TRACE_HW_ACCESS_TYPE_WRITE_E - PP access type is write
 *      TRACE_HW_ACCESS_TYPE_DELAY_E - PP access type is (write/)delay
 */
typedef enum
{
    TRACE_HW_ACCESS_TYPE_READ_E,
    TRACE_HW_ACCESS_TYPE_WRITE_E,
    TRACE_HW_ACCESS_TYPE_DELAY_E
} TRACE_HW_ACCESS_TYPE_ENT;

/*
 * typedef: enum  APP_DEMO_TRACE_OUTPUT_MODE_ENT
 *
 * Description: PP access type enumeration
 *
 * Fields:
 *      TRACE_OUTPUT_MODE_DIRECT_E         - use osPrintf.
 *      TRACE_OUTPUT_MODE_DIRECT_SYNC_E    - use osPrintSync need for ISR debug
 *      TRACE_OUTPUT_MODE_DB_E             - store the data in db
 *      TRACE_OUTPUT_MODE_FILE_E           - store the data in file
 */

typedef enum
{
    TRACE_OUTPUT_MODE_DIRECT_E,
    TRACE_OUTPUT_MODE_DIRECT_SYNC_E,
    TRACE_OUTPUT_MODE_DB_E,
    TRACE_OUTPUT_MODE_FILE_E
} TRACE_OUTPUT_MODE_ENT;


GT_STATUS traceHwAccessWrite
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    /* Modified for CPSS 4.1 */
    //IN GT_BOOL     pciPexSpace,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
);

GT_STATUS traceHwAccessRead
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    /* Modified for CPSS 4.1 */
    //IN GT_BOOL     pciPexSpace,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
);

GT_STATUS traceHwAccessDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);

#endif /*__NIM_DM_CPSS_TRACE_H__*/
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_trace.h,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


