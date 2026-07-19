/* $Id: sp_ppb_icmp.h,v 1.2 2017/07/28 07:58:49 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/sp_ppb_icmp.h,v $
 *------------------------------------------------------------------
 * sp_ppb_icmp.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

//#include "lsi_mg_std.h"

/* ARP REQUEST packet size */
#define ARP_REQ_PACK_SIZE 42

/* ARP REPLY packet size */
#define ARP_REP_PACK_SIZE 60

/* ICMP packet size */
#define ICMP_IPV4_SIZE 34
#define ICMP_HD_SIZE 8
#define ICMP_DT_SIZE 32
#define ICMP_PACK_SIZE ( ICMP_IPV4_SIZE + ICMP_HD_SIZE + ICMP_DT_SIZE )

/* In IPV4 Header */
#define ICMP_IPV4_MAC_SIZE 12 /* Size of MAC addresses field */
#define ICMP_IPV4_TYPE_SIZE 2
#define ICMP_HD_SIZE 8
#define ICMP_DT_SIZE 32

void ICMP_ReqtoReplyCon(uint8_t *incoming_buf, uint8_t *target_buf, uint32_t data_len) __attribute__ ((noinline));
void ARP_ReqtoReplyCon(uint8_t *incoming_buf, uint8_t *target_buf, uint32_t interface)  __attribute__ ((noinline));

void TIMER_WFI(void);

void countLED(uint32_t count);

void loc_error(uint32_t errnum);

/******** History ********
$Log: sp_ppb_icmp.h,v $
Revision 1.2  2017/07/28 07:58:49  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:37  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

