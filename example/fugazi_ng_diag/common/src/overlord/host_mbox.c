/* $Id: host_mbox.c,v 1.1 2013/05/09 05:42:36 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/host_mbox.c,v $
 *------------------------------------------------------------------
 * host_mbox.c - Host side mailbox
 *
 * Paul Tong, July 2011
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "common.h"
#include "error.h"
#include "strings.h"
#include "proto.h"
#include "../cavium/host2dp_mbox.h"
#include "octeon-remote.h" /* in cvmx2.2/host/remote-lib/ */

/* Host side mailbox pointer
 */
mbox_t *host_out_mbxp;
mbox_t *host_in_mbxp;


/**** Octeon remote PCI read-write SPI *********************************/

static uint32_t mbx_rd32(uint64_t physical_address)
{
    return(octeon_remote_read_mem32(physical_address));
}

static void mbx_wr32(uint64_t physical_address, uint32_t data)
{
    octeon_remote_write_mem32(physical_address, data);
}

/**** mailbox host code *********************************/

int host_mbox_init (void)
{
    uint64_t namedblock_phy_base, namedblock_size;
    int rv;

    if (host_in_mbxp != 0) {
        printf("HOST: Mail box has already been setup. host_in_mbxp= %p\n", host_in_mbxp);
	return(PASS);
    }

    printf("HOST: Initialize pointers to DP mailbox\n");

    /* Open a connection */
    if (octeon_remote_open("PCI", 0)) {
	printf("HOST: octeon_remote_open() failed\n");
        return FAIL;
    }
    printf("HOST: octeon_remote_open() passed\n");
    
    rv = octeon_remote_named_block_find(OVERLORD_DP_MBOX_NAMED_BLOCK,
					&namedblock_phy_base, 
					&namedblock_size);
    printf("HOST: %s ", __FUNCTION__);
    if (rv) { /* success return */
	host_out_mbxp = (mbox_t *)namedblock_phy_base;
	host_in_mbxp = (mbox_t *)(namedblock_phy_base + OVERLORD_DP_MBOX_SIZE);
    
	mbx_wr32((uint64_t)&host_out_mbxp->msg_ctl, 0);
        printf("passed. host_in_mbxp= %p\n", host_in_mbxp);
	return(PASS);
    }
    else {
        printf("failed\n");
	return(FAIL);
    }
}

void host_mbox_close(void)
{
    if (host_in_mbxp != 0) {
        octeon_remote_close();
    }
    host_out_mbxp = 0;
    host_in_mbxp = 0;
}

int is_mbox_empty (mbox_t *mbxp)
{
    uint32_t msg_ctl;

    msg_ctl = mbx_rd32((uint64_t)&mbxp->msg_ctl);
    return((msg_ctl & MSG_VALID) == 0); 
}

void ack_msg (void)
{
    uint32_t msg_ctl;

    msg_ctl = mbx_rd32((uint64_t)&host_in_mbxp->msg_ctl);
    msg_ctl &= ~MSG_VALID;
    mbx_wr32((uint64_t)&host_in_mbxp->msg_ctl, msg_ctl);
}

uint get_msg (void)
{
    uint32_t msg_ctl;

    msg_ctl = mbx_rd32((uint64_t)&host_in_mbxp->msg_ctl);
    return((uint) msg_ctl);
}

uint get_msg_id (uint msg_ctl)
{
    return(msg_ctl & MSG_ID_MASK);
}

int is_msg_pass (uint msg)
{
    uint32_t msg_ctl;

    msg_ctl = mbx_rd32((uint64_t)&host_in_mbxp->msg_ctl);

    if ((msg == get_msg_id(msg_ctl)) &&
	(msg_ctl & (MSG_VALID | MSG_PASS))) {
        return(TRUE);
    }
    else {
        return(FALSE);
    }
}

void send_msg(uint msg)
{
    mbx_wr32((uint64_t)&host_out_mbxp->msg_ctl, (MSG_VALID | msg));
}


/******** History ******** 
$Log: host_mbox.c,v $
Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.10  2012/12/05 22:05:22  ptong
Fixed data plane boot up process

Revision 1.9  2012/12/03 22:08:48  ptong
Impove the remote boot steps when booting the Octeon

Revision 1.8  2012/11/29 22:30:25  ptong
Print more message when host is booting the data plane

Revision 1.7  2012/11/14 01:47:33  ptong
Move BCM and CVMX vendor src tree to /auto/overlord/ovld-vendor-src

Revision 1.6  2012/06/07 02:11:21  palin2
Clean up compiler warnings.

Revision 1.5  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.4  2012/04/17 22:01:27  ptong
Added more utility to run DP test from host.

Revision 1.3  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
