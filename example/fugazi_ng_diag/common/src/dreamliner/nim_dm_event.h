/* $Id: nim_dm_event.h,v 1.2 2015/02/27 10:02:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/nim_dm_event.h,v $
 *------------------------------------------------------------------
 * NIM DM NGIO Lite - event information
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef _NIM_DM_EVENT_H_
#define _NIM_DM_EVENT_H_

#define EV_U32_ARRAY_BITMAP(array, event)    \
    (array[(event) >> 5] & (1 << ((event) & 0x1F)))

#define MAX_AU_MSG_NUM   1024
typedef struct evt_ctx_s {
    uint8_t devnum;
    CPSS_PP_DEVICE_TYPE device_type;

    GT_UINTPTR evHandle;

    GT_U8   *auDescBlock;
    CPSS_MAC_UPDATE_MSG_EXT_STC auMsgs[MAX_AU_MSG_NUM];

} evt_ctx_t;

extern evt_ctx_t evt_ctx;

GT_STATUS nim_dm_event_init(uint8_t devnum, evContext evctx);
void nim_dm_event_cleanup(void);

#endif
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_event.h,v $
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 *
 * Revision 1.1.4.2  2015/01/28 22:59:22  iachang
 * Dreamliner-branch2 initial check-in.
 *
 * Revision 1.1.2.1  2014/12/02 08:04:13  iachang
 * Dreamliner Diag initial check-in.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
