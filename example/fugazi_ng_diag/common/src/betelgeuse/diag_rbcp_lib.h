/* $Id: diag_rbcp_lib.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_rbcp_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_rbcp_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef GSHDSL_RBCP_PLATFORM_H_
#define GSHDSL_RBCP_PLATFORM_H_

#define RBCP_WAIT_COUNT 1000000


extern int platform_rbcp_send(uint8_t *, int);
extern int platform_rbcp_recv(uint8_t *, int *);
extern int platform_rbcp_clear_recv(void);
extern int platform_setup_rbcp_ge_env(void);
extern int platform_cleanup_rbcp_ge_env(void);
extern int platform_wait_for_ge_packet(uchar *);

/* common file use */
#define MAX_NUM_SHEDIR_SLOTS	4
#define CPU_SGMII_PORT2         2

#endif /* GSHDSL_RBCP_PLATFORM_H_ */

/*-------------------------------------------------
 * $Log: diag_rbcp_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
