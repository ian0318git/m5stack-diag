/* $Id: nim_dm_cpss_trace.h,v 1.2 2015/02/27 10:02:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/nim_dm_cpss_trace.h,v $
 *------------------------------------------------------------------
 * DM CPSS lib external trace service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
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
    IN GT_BOOL     pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
);

GT_STATUS traceHwAccessRead
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN GT_BOOL     pciPexSpace,
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
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 *
 * Revision 1.1.4.2  2015/01/28 22:59:22  iachang
 * Dreamliner-branch2 initial check-in.
 *
 * Revision 1.1.2.1  2014/12/02 08:04:12  iachang
 * Dreamliner Diag initial check-in.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


