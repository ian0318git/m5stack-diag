/* $Id: nim_dm_cpss_trace.c,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_cpss_trace.c,v $
 *------------------------------------------------------------------
 * DM CPSS lib external trace service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

/* Modified for CPSS 4.1 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "nim_dm_cpss_extserv.h"

/******************************************************************************
* appDemoTraceHwAccessAction
*
* DESCRIPTION:
*       Trace HW Access action: print or store.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       isrContext  - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                                       cpssDrvPpHwIsrRead)
*                    GT_FALSE: called from not ISR context.
*       pciPexSpace - GT_TRUE: called for PCI/PEX registers address
*                     space (cpssDrvPpHwInternalPciRegWrite,
*                            cpssDrvPpHwInternalPciRegRead)
*                     GT_FALSE - called for usual  (not PCI/PEX) address space
*       addr        - start address that the access was made to
*       length      - length of the data that was written in words
*       dataPtr     - (pointer to) data that was written
*       outputMode  - output mode: print, synchronious print and store
*       accessType  - access type: read or write
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
******************************************************************************/
static GT_STATUS traceHwAccessAction
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroupId,
    IN GT_BOOL                     isrContext,
    IN GT_BOOL                     pciPexSpace,
    IN GT_U32                      addr,
    IN GT_U32                      length,
    IN GT_U32                      *dataPtr,
    IN TRACE_OUTPUT_MODE_ENT       outputMode,
    IN TRACE_HW_ACCESS_TYPE_ENT    accessType
)
{
    GT_STATUS ret = GT_OK;
    GT_U32 i;
    char *buf = NULL;
    int len = 0;

    buf = malloc(256 + length * 8 + length / 4);
    if(buf == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to alloc memory for "
            "hardware access trace.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    INFO("Hardware Access Trace:");

    switch (outputMode) {
        case TRACE_OUTPUT_MODE_DIRECT_E:
        case TRACE_OUTPUT_MODE_DIRECT_SYNC_E:
            if (accessType == TRACE_HW_ACCESS_TYPE_DELAY_E) {
                INFO("Delay \t%u \t%u \t%8u",(unsigned int)devNum,
                         (unsigned int)portGroupId, (unsigned int)*dataPtr);
            } else {
                if (accessType == TRACE_HW_ACCESS_TYPE_READ_E) {
                    len += sprintf(buf+len, "Read ");
                } else {
                    len += sprintf(buf+len, "Write ");
                }

                len += sprintf(buf+len, "\t%u \t%u ", (unsigned int)devNum,
                               (unsigned int)portGroupId);

                if (isrContext) {
                    len += sprintf(buf+len, "\tISR ");
                } else {
                    len += sprintf(buf+len, "\tTSK ");
                }

                if (pciPexSpace) {
                    len += sprintf(buf+len, "\tPEX ");
                } else {
                    len += sprintf(buf+len, "\tREG ");
                }

                INFO("%s", buf);
                INFO("ADDR:  %-#8x", (unsigned int)addr);

                len = 0;
                for (i = 0; i < length; i++) {
                    len += sprintf(buf+len, "%-#8x  ", (unsigned int)dataPtr[i]);

                    if (((i+1) % 4 == 0) && (i != (length -1)))
                        len += sprintf(buf+len, "\n");
                }

                INFO("%s", buf);
            }

            break;

        case TRACE_OUTPUT_MODE_DB_E:
        case TRACE_OUTPUT_MODE_FILE_E:
            ret = GT_NOT_SUPPORTED;
            break;

        default:
            ret = GT_BAD_STATE;
            break;
    }

    free(buf);

    return ret;
}

/******************************************************************************
* traceHwAccessWrite
*
* DESCRIPTION:
*       Trace HW write access information.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       isrContext  - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                                       cpssDrvPpHwIsrRead)
*                    GT_FALSE: called from not ISR context.
*       pciPexSpace - GT_TRUE: called for PCI/PEX registers address
*                     space (cpssDrvPpHwInternalPciRegWrite,
*                            cpssDrvPpHwInternalPciRegRead)
*                     GT_FALSE - called for usual  (not PCI/PEX) address space
*       addr        - start address that the access was made to
*       length      - length of the data that was written in words
*       dataPtr     - (pointer to) data that was written
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on fail
*
* COMMENTS:
*       None.
*
******************************************************************************/
GT_STATUS traceHwAccessWrite
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    /* Modified for CPSS 4.1 */
    //IN GT_BOOL     pciPexSpace,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT      pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
)
{
    return traceHwAccessAction(devNum,
                               portGroupId,
                               isrContext,
                               pciPexSpace,
                               addr,
                               length,
                               dataPtr,
                               TRACE_OUTPUT_MODE_DIRECT_E,
                               TRACE_HW_ACCESS_TYPE_WRITE_E);
}

/******************************************************************************
* traceHwAccessRead
*
* DESCRIPTION:
*       Trace HW read access information.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       isrContext  - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                                       cpssDrvPpHwIsrRead)
*                     GT_FALSE: called from not ISR context.
*       pciPexSpace - GT_TRUE: called for PCI/PEX registers address
*                     space (cpssDrvPpHwInternalPciRegWrite,
*                            cpssDrvPpHwInternalPciRegRead)
*                     GT_FALSE - called for usual  (not PCI/PEX) address space
*       addr        - start address that the access was made to
*       length      - length of the data that was written in words
*       dataPtr     - (pointer to) data that was read
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on fail
*
* COMMENTS:
*       None.
*
******************************************************************************/
GT_STATUS traceHwAccessRead
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    /* Modified for CPSS 4.1 */
    //IN GT_BOOL     pciPexSpace,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT      pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
)
{
    return traceHwAccessAction(devNum,
                               portGroupId,
                               isrContext,
                               pciPexSpace,
                               addr,
                               length,
                               dataPtr,
                               TRACE_OUTPUT_MODE_DIRECT_E,
                               TRACE_HW_ACCESS_TYPE_READ_E);
}


/******************************************************************************
* traceHwAccessDelay
*
* DESCRIPTION:
*       Trace HW write access information.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       millisec   -  the delay in millisec
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on fail
*
* COMMENTS:
*       None.
*
******************************************************************************/
GT_STATUS traceHwAccessDelay
(
       IN GT_U8       devNum,
       IN GT_U32      portGroupId,
       IN GT_U32      millisec
)
{
    return traceHwAccessAction(devNum,
                               ((portGroupId==0xFFFFFFFF)?0:portGroupId),
                               GT_FALSE,
                               GT_FALSE,
                               0,
                               1,
                               &millisec,
                               TRACE_OUTPUT_MODE_DIRECT_E,
                               TRACE_HW_ACCESS_TYPE_DELAY_E);
}
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_trace.c,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
