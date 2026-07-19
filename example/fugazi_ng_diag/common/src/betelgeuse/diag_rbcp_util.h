/* $Id: diag_rbcp_util.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_rbcp_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_rbcp_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef PLAT_RBCP_PLATFORM_H_
#define PLAT_RBCP_PLATFORM_H_

#define ETH_MAX_LEN    1518
#define XAUI_PKT_BUF_LEN   10240 + 60  /* 60 extra bytes */


extern int plat_rbcp_send(uint8_t *, int);
extern int plat_rbcp_recv(uint8_t *, int *);
extern int plat_rbcp_clear_recv(void);
extern int plat_setup_rbcp_ge_env(void);
extern int plat_cleanup_rbcp_ge_env(void);
extern int plat_wait_for_ge_packet(uchar *);

#endif /* PLAT_RBCP_PLATFORM_H_ */

/*-------------------------------------------------
 * $Log: diag_rbcp_util.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
