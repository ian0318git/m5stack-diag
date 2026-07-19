/* $Id: rbcp_platform.h,v 1.1 2018/05/09 06:53:12 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/rbcp_platform.h,v $
 *------------------------------------------------------------------
 * Filename: rbcp_platform.h
 *
 * Description: Platform dependent code header file
 * Author: Times Huang
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
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


/*------------------------------------------------------------------
 * $Log: rbcp_platform.h,v $
 * Revision 1.1  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
