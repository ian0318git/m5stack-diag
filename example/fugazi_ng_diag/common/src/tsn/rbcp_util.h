/* $Id: rbcp_util.h,v 1.2 2018/05/09 06:53:12 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/rbcp_util.h,v $
 *------------------------------------------------------------------
 * Filename: rbcp_util.h
 *
 * Description: Platform dependent code header file
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef TSN_RBCP_PLATFORM_H_
#define TSN_RBCP_PLATFORM_H_

#define ETH_MAX_LEN    1518
#define XAUI_PKT_BUF_LEN   10240 + 60  /* 60 extra bytes */


extern int tsn_rbcp_send(uint8_t *, int);
extern int tsn_rbcp_recv(uint8_t *, int *);
extern int tsn_rbcp_clear_recv(void);
extern int tsn_setup_rbcp_ge_env(void);
extern int tsn_cleanup_rbcp_ge_env(void);
extern int tsn_wait_for_ge_packet(uchar *);

#endif /* TSN_RBCP_PLATFORM_H_ */


/*------------------------------------------------------------------
 * $Log: rbcp_util.h,v $
 * Revision 1.2  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
