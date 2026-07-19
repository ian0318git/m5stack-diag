/* $Id: nim_dm_event.c,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_event.c,v $
 *------------------------------------------------------------------
 * NIM DM NGIO Lite - DM event related information
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define BTRACE_APP_MACROS
#include <btrace/btrace.h>
#include <sys/types.h>
#include <evutil/evutil.h>

#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#pragma GCC diagnostic warning "-Wstrict-prototypes"
#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>

#include <msglib/modmgmt/include/modmgmt_msg_lib.h>
#include <ngiolite/ngiolite.h>
#include "nim_dm_cpss_extserv.h"
#include "nim_l2ether_intf.h"
#include "nim_l2ether_drv.h"
#include "nim_l2ether_skuinfo.h"
#include "nim_dm_event.h"

evt_ctx_t evt_ctx;

/****************************************************************
 * nim_dm_event_au_handler
 *
 * Description
 *   AU event handler
 ****************************************************************/
static int
nim_dm_event_au_handler(uint8_t devNum, GT_U32 numOfAu,
                        CPSS_MAC_UPDATE_MSG_EXT_STC *auMsgs)
{
    unsigned int i;

    /* FIXME: how to process AUQ messages TBD */
    INFO("process AUQ msgs (%u, %u)", devNum, (uint32_t)numOfAu);

    for (i = 0; i < numOfAu; i++) {
        DEBUG("AUQ msgs details: \n"
                "updType = %u, macEntryIndex = %u\n"
                "macEntry.key.entryType = %u, macEntry.dstInterface.type = %u\n"
                "dstInterface.devPort.portNum = %u, key.macVlan.vlanId = %u\n"
                "isMoved = %u, oldDstInterface.devPort.portNum = %u\n"
                "Mac address = %x-%x-%x-%x-%x-%x\n",
             (uint32_t)auMsgs[i].updType,
             (uint32_t)auMsgs[i].macEntryIndex,
             (uint32_t)auMsgs[i].macEntry.key.entryType,
             (uint32_t)auMsgs[i].macEntry.dstInterface.type,
             (uint32_t)auMsgs[i].macEntry.dstInterface.devPort.portNum,
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.vlanId,
             (uint32_t)auMsgs[i].isMoved,
             (uint32_t)auMsgs[i].oldDstInterface.devPort.portNum,
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.macAddr.arEther[0],
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.macAddr.arEther[1],
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.macAddr.arEther[2],
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.macAddr.arEther[3],
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.macAddr.arEther[4],
             (uint32_t)auMsgs[i].macEntry.key.key.macVlan.macAddr.arEther[5]);
    }

    return 0;
}

/****************************************************************
 * nim_dm_cpss_event_handler
 *
 * Description
 *   event handler registered with cpss lib to signal evlib thread
 ****************************************************************/
static GT_VOID
nim_dm_cpss_event_handler(GT_UINTPTR hndl __UNUSED,
                          GT_VOID *cookie __UNUSED)
{
    nim_dm_cpss_event_signal();
}

/****************************************************************
 * nim_dm_ngiolite_event_handler
 *
 * Description
 *   event handler running in ngiolite main evlib thread
 ****************************************************************/
static void
nim_dm_ngiolite_event_handler (evContext ctx __UNUSED,
                               void *arg_p,
                               int fd __UNUSED,
                               int mask __UNUSED)
{
    GT_STATUS rc;
    evt_ctx_t *pctx = (evt_ctx_t *)arg_p;
    GT_U32 evBitmapArr[(CPSS_UNI_EVENT_COUNT_E + 31) >> 5];
    GT_U32 evExtData;
    GT_U8  evDev;
    GT_U32 numOfAu = MAX_AU_MSG_NUM;

    DEBUG("Handling cpss event for dreamliner");
    rc = cpssEventWaitingEventsGet(pctx->evHandle, evBitmapArr,
                                   sizeof(evBitmapArr)/sizeof(evBitmapArr[0]));
    if (rc != GT_OK) {
        ERR("failed to get cpss event, rc: %u", rc);
        return;
    }

    if(EV_U32_ARRAY_BITMAP(evBitmapArr, CPSS_PP_EB_AUQ_PENDING_E)) {

        rc = cpssEventRecv(pctx->evHandle, CPSS_PP_EB_AUQ_PENDING_E,
                           &evExtData, &evDev);
        if (rc != GT_OK) {
            ERR("failed to receive AUQ event, rc: %u", rc);
            return;
        }

        /* get the au msg from both primary and secondary AU block */
        rc = cpssDxChBrgFdbAuMsgBlockGet(pctx->devnum, &numOfAu, pctx->auMsgs);
        if (rc != GT_OK && rc != GT_NO_MORE) {
            ERR("failed to get AUQ msg block, rc: %u", rc);
            return;
        }

        if(numOfAu == 0) {
            DEBUG("No AUQ msg is retrieved.");
            return;
        }

        if(nim_dm_event_au_handler(pctx->devnum, numOfAu, pctx->auMsgs)) {
            ERR("failed to process AUQ event, rc: %u", rc);
            return;
        }
    }

    rc = cpssEventTreatedEventsClear(pctx->evHandle);
    if (rc != GT_OK) {
        ERR("failed to clear treated cpss events, rc: %u", rc);
        return;
    }

}

/****************************************************************
 * nim_dm_event_init
 *
 * Description
 *   event handling initialization
 ****************************************************************/
GT_STATUS nim_dm_event_init(uint8_t devnum, evContext evctx)
{
    GT_STATUS rc;
    int ret;
    CPSS_UNI_EV_CAUSE_ENT uniEventArr[] = {
        CPSS_PP_EB_AUQ_PENDING_E
    };

    /*
     * register event handler with evlib,
     * which does the event specific processing
     */
    evt_ctx.devnum = devnum;
    ret = evSelectFD(evctx, sub_fd, EV_READ,
                     nim_dm_ngiolite_event_handler, &evt_ctx,
                     NULL, NULL);
    if (ret) {
        ERR("evSelectFD failed %d", errno);
        return GT_FAIL;
    }

    /*
     * bind the event callback function to our interested events
     */
    rc = cpssEventIsrBind(uniEventArr,
                          sizeof(uniEventArr) / sizeof(uniEventArr[0]),
                          nim_dm_cpss_event_handler, NULL,
                          &evt_ctx.evHandle);
    if (rc != GT_OK) {
        ERR("failed to bind cpss event, rc: %u", rc);
        return rc;
    }

    /*
     * unmask the evnet/interrupt we are interested
     */
    rc = cpssEventDeviceMaskSet(devnum, CPSS_PP_EB_AUQ_PENDING_E,
                                CPSS_EVENT_UNMASK_E);
    if (rc != GT_OK) {
        ERR("failed to unmask cpss event, rc: %u", rc);
        return rc;
    }

    /*
     * clear previously treated events
     */
    rc = cpssEventTreatedEventsClear(evt_ctx.evHandle);
    if (rc != GT_OK) {
        ERR("failed to clear treated cpss events, rc: %u", rc);
        return rc;
    }

    return GT_OK;
}

/****************************************************************
 * nim_dm_event_init
 *
 * Description
 *   event handling resource clean up
 ****************************************************************/
void nim_dm_event_cleanup(void)
{
    if(evt_ctx.evHandle != 0) {
        cpssEventDestroy(evt_ctx.evHandle);
        evt_ctx.evHandle = 0;
    }

    if(evt_ctx.auDescBlock != NULL) {
        osMemCacheDmaFree(evt_ctx.auDescBlock);
        evt_ctx.auDescBlock = NULL;
    }
}
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_event.c,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
