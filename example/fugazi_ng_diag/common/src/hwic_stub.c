/* $Id: hwic_stub.c,v 1.2 2012/03/28 00:38:13 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hwic_stub.c,v $
 *------------------------------------------------------------------
 *
 *
 * 5/2008
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <assert.h>

extern void fixme(void);
int goofy_debug_flag;

void fixme(void)
{
    printf("FIX ME %s %s \n", __FILE__, __FUNCTION__);
    assert(0);
}

void cable_show_tx_buffer (unsigned char *tx_buffer)
{
    fixme();
}
int cable_check_rbcp_buffer (unsigned char *rx_buffer)
{
    fixme();
    return 0;
}
void cable_swap_mac_addrs (unsigned char *tx_buffer)
{
    fixme();
    return;
}

/******** History *********
$Log: hwic_stub.c,v $
Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
