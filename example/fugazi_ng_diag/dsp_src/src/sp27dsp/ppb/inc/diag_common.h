/* $Id: diag_common.h,v 1.6 2012/09/10 06:31:42 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/diag_common.h,v $
 *------------------------------------------------------------------
 * diag_common.h
 *      Graffham common defs
 *
 * Mar 2012, Smita Rane
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_COMMON_H__
#define __DIAG_COMMON_H__

/******************************************
 * DSP Data Types for DSS  *
 ******************************************/
#define UINT32 unsigned int
typedef unsigned int	    ulong;	/* System V compatibility */

#define SECTION(x) __attribute__((section(x)))

extern uint8_t *wait_host_msg(int);
extern uint16_t wait_lpbk_msg(int);
extern uint16_t send_host_testmsg(void);
extern uint16_t send_host_readymsg(void);
extern uint16_t send_host_timeoutmsg(void);
extern uint16_t send_host_memmsg(void);
extern int ecc_mem_test(void);

#endif /* __DIAG_COMMON_H__ */

/******** History ********
$Log: diag_common.h,v $
Revision 1.6  2012/09/10 06:31:42  srane
Add defines for dsp memory display pkt and ARM11 CPU1 test.

Revision 1.5  2012/08/15 14:52:17  srane
cleanup code.

Revision 1.4  2012/07/17 20:34:28  srane
cleanup

Revision 1.3  2012/06/07 22:50:11  srane
cleanup

Revision 1.2  2012/05/31 06:40:49  srane
Add define, cleanup.

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

