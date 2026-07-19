/* $Id: host2dp_mbox.c,v 1.9 2018/05/18 09:24:52 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/host2dp_mbox.c,v $
 *------------------------------------------------------------------
 * host2dp_mbox.c - Overload host cpu to data plane cpu mailbox messaging code.
 * 
 * March 2011 ptong
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "types.h"
#include "common.h"
#include "host2dp_mbox.h"

extern int memops_mmap(ulong phyaddr, ulong length,
                       ulong *viraddr_p, ulong *pg_viraddr_p);

/* Virtual address as pointers to mailbox
 */
mbox_t *in_mbxp, *out_mbxp;

/*
 * Function: mlbx_vir_addr_get
 *
 * Description: Use mmap to get a virtual address for the mail box
 *
 * Input: physical_address - physical address of the mail box
 *        length - length of the mail box
 *
 * Return: virtual address value as a pointer to void
 */
static void *mlbx_vir_addr_get(ulong physical_address, int length)
{
    ulong viraddr, phyaddr, pg_viraddr;

    phyaddr = physical_address;
    if (memops_mmap(phyaddr, length, &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(0);
    }

    return((void *)viraddr);
}

/*
 * Function: dp_mbox_init
 *
 * Description: Init the data plane mail box virtual address
 *
 * Input: cpu - Cavium 72XX or 63XX
 *
 * Return: none
 */
void dp_mbox_init (int cpu)
{
    ulong in_mlbx_phy_addr, out_mlbx_phy_addr;

    if (cpu == CVMX_OVERLORD) {  /* o2/juno */
       printf("Initial mbox as O2/Juno \n");
       in_mlbx_phy_addr = OVERLORD_DP_IN_MBOX_PHY_ADDR;
       out_mlbx_phy_addr = OVERLORD_DP_OUT_MBOX_PHY_ADDR;
    } else {   /* octeon3 is neptune series, CVMX_NEPTUNE */
       printf("Initial mbox as Neptune series \n");
       in_mlbx_phy_addr = NEPTUNE_DP_IN_MBOX_PHY_ADDR;
       out_mlbx_phy_addr = NEPTUNE_DP_OUT_MBOX_PHY_ADDR;
    }

    in_mbxp  = (mbox_t *)mlbx_vir_addr_get(in_mlbx_phy_addr, sizeof(mbox_t));
    out_mbxp = (mbox_t *)mlbx_vir_addr_get(out_mlbx_phy_addr, sizeof(mbox_t));
    out_mbxp->msg_ctl = 0;
}

/*
 * Function: is_mbox_empty
 *
 * Description: Check is the mail box has a valid message from host
 *
 * Input: mbxp - ptr to the mail box
 *
 * Return: TRUE/FALSE
 */
int is_mbox_empty (mbox_t *mbxp)
{
    return((mbxp->msg_ctl & MSG_VALID) == 0); 
}

/*
 * Function: ack_msg
 *
 * Description: Acknowledge a the reception of a valid message
 *
 * Input: void
 *
 * Return: void
 */
void ack_msg (void)
{
    in_mbxp->msg_ctl &= ~MSG_VALID;
}

/*
 * Function: get_msg
 *
 * Description: Return the message control field of the message
 *
 * Input: void
 *
 * Return: message code
 */
uint get_msg (void)
{
    return(in_mbxp->msg_ctl);
}

/*
 * Function: get_msg_id
 *
 * Description: Return the message ID in the message
 *
 * Input: msg_ctl - message control field value
 *
 * Return: message ID value
 */
uint get_msg_id (uint msg_ctl)
{
    return(msg_ctl & MSG_ID_MASK);
}

/*
 * Function: is_msg_pass
 *
 * Description: Check if the message in the in-box is valid 
 * and indicate the result is pass.
 *
 * Input: msg - the message ID
 *
 * Return: PASS/FAIL
 */
int is_msg_pass (uint msg)
{
    uint msg_ctl = in_mbxp->msg_ctl;

    if ((msg == get_msg_id(msg_ctl)) &&
	(msg_ctl & (MSG_VALID | MSG_PASS))) {
        return(TRUE);
    }
    else {
        return(FALSE);
    }
}

/*
 * Function: send_msg
 *
 * Description: Write a valid message to the mailbox
 *
 * Input: msg - message
 *
 * Return: void
 */
void send_msg(uint msg)
{
    out_mbxp->msg_ctl = (MSG_VALID | msg);
}

/*
 * Function: rtn_pass_msg
 *
 * Description: Return to host the message with pass status
 *
 * Input: msg - message to send to host
 *
 * Return: void
 */
void rtn_pass_msg(uint msg)
{
    out_mbxp->msg_ctl = (msg | MSG_PASS | MSG_VALID);
}

/*
 * Function: rtn_fail_msg
 *
 * Description: Return to host the message with fail status
 *
 * Input: msg - message to send to host
 *
 * Return: void
 */
void rtn_fail_msg(uint msg)
{
    out_mbxp->msg_ctl = (msg | MSG_VALID);
}

/*-------------------------------------------------
$Log: host2dp_mbox.c,v $
Revision 1.9  2018/05/18 09:24:52  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.8.72.2  2016/11/15 07:16:52  alpeng
resolve cvm 2nd test issue

Revision 1.8.72.1  2016/11/03 08:26:54  alpeng
merge octeon_test.c with o2

Revision 1.8  2012/12/05 22:05:22  ptong
Fixed data plane boot up process

Revision 1.7  2012/11/02 00:55:51  ptong
Add comment and clean-up

Revision 1.6  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.5  2012/04/17 22:01:26  ptong
Added more utility to run DP test from host.

Revision 1.4  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.3  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
