/* $Id: rbcp_platform.h,v 1.2 2015/05/25 03:58:21 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shedir/rbcp_platform.h,v $
 *------------------------------------------------------------------
 * Filename: rbcp_platform.h
 *
 * Description: Platform dependent code header file
 * Author: Times Huang
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef SHEDIR_RBCP_PLATFORM_H_
#define SHEDIR_RBCP_PLATFORM_H_

extern int platform_rbcp_send(uint8_t *, int);
extern int platform_rbcp_recv(uint8_t *, int *);
extern int platform_rbcp_clear_recv(void);
extern int platform_setup_rbcp_ge_env(void);
extern int platform_cleanup_rbcp_ge_env(void);
extern int platform_wait_for_ge_packet(uchar *);

#define MAX_NUM_SHEDIR_SLOTS	4

#endif /* SHEDIR_RBCP_PLATFORM_H_ */


/*------------------------------------------------------------------
 * $Log: rbcp_platform.h,v $
 * Revision 1.2  2015/05/25 03:58:21  steja
 * Fix merge conflict issue
 *
 * Revision 1.1.2.2  2015/05/22 15:42:31  steja
 * Sync skye-branch2 with Maintrunk
 *
 * Revision 1.1  2015/05/14 05:32:26  hondwang
 * Merge Shedir NIM to maintrunk
 *
 * Revision 1.1.2.1  2014/08/29 03:18:46  hondwang
 * shedir project
 *
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
