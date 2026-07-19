/* $Id: shrinkray_utils.c,v 1.2 2014/03/03 06:33:51 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shrinkray_utils.c,v $
 *******************************************************************************
 * File Name: shrinkray_utils.c
 *
 * Description: Shrinkray utilities source file
 *
 * Author: Sofian Teja
 *
 * Copyright (c)2013 ~ 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "menu.h"
#include "proto.h"
#include "queryflags.h"
#include "strings.h"
#include "shrinkray_host.h"
#include "sgmii_defs.h"

static int socket_gl;


/***********************************************************************
 * Name: shrinkray_setup_ge_env
 *
 * Description:
 *      This test will set up GE operation environment.
 *
 * Input: iface - Shrinkray data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int
shrinkray_setup_ge_env (shrinkray_ds_t *iface)
{
    int sgmii_port = 0;
    int status = PASSED;
    char eth_name[10];

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
        cterr('f', 0, "Setup: Failed to get sgmii port number.");
        return (FAILED);
    }

    sprintf (eth_name, "eth%d", sgmii_port);
    status = setup_eth_dev(eth_name, &socket_gl);

    if (set_promisc(eth_name, socket_gl) == -1) {
        return(FAILED);
    }
#ifdef DEBUG
    printf("\nsocket_gl = 0x%02x\n", socket_gl);
#endif
    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: shrinkray_wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets
 *
 *  Input: pak - received packet buffer
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int
shrinkray_wait_for_ge_packet(uchar *pak)
{
    int wait_count = 10000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    /* clear buffer before use */
    memset((uchar *)pak, 0, sizeof(fe_packet_t));
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count;
    rx_pkt_p->socket = socket_gl;

    /* now wait for */
    status = eth_pkt_rx(rx_pkt_p);

    if (status) {
	return (FAILED); /* retry is provided by caller */
    };
    /* copy received to user pak */
    memcpy ((char *)pak, (uchar *)recv_buffer, sizeof(fe_packet_t));

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nRx data :\n");
        dismem((unsigned char *)(pak), 0x40,
            (unsigned long)(pak), 1);
    }

    return (PASSED);
}


/***********************************************************************
 * Name: shrinkray_cleanup_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: iface - Shrinkray data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int
shrinkray_cleanup_ge_env(shrinkray_ds_t *iface)
{
    int sgmii_port = 0;
    int status = PASSED;
    char eth_name[10];

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
        cterr('f', 0, "cleanup: Failed to get sgmii port number.");
        return (FAILED);
    }
    sprintf (eth_name, "eth%d", sgmii_port);
    status = cleanup_eth_dev(eth_name, socket_gl);

    if (status) {
        cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
        return (FAILED);
    }
    close(socket_gl);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 * $Log: shrinkray_utils.c,v $
 * Revision 1.2  2014/03/03 06:33:51  palin2
 * -Initial check-in ShrinkRay host side Diag.
 *
 * Revision 1.1.4.2  2014/02/27 07:09:58  steja
 * Fix compile error after get update from latest code main trunk
 *
 * Revision 1.1.4.1  2014/02/26 11:08:59  palin2
 * -To support ShrinkRay host side tests on O2.
 * -This branch is created to pick up O2 main tunk code changes.
 *
 * Revision 1.1.2.2  2014/01/27 08:51:07  steja
 * Code clean up
 *
 * Revision 1.1.2.1  2013/08/17 03:27:00  steja
 * add code command and respond ( Host <->GE <-> TILE CPU#0) for O2 platform
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
