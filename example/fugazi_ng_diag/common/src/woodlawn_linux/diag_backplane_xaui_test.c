/* $Id: diag_backplane_xaui_test.c,v 1.3 2014/11/12 06:32:59 leschen Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_backplane_xaui_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_backplane_xaui_test.c - Backplane XAUI Loopback Test
 *
 * December 2012, Leslie Chen
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "defs.h"
#include "diag_common_lib.h"
#include "platform_xaui.h"
#include "platform_smi.h"
#include "platform_smi_lib.h"
#include "common_utils.h"
#include "diag_tlk10232_lib.h"
#include "diag_fpga_lib.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "monitor.h"
#include "cross_platform.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_xaui.h"
#include "cvmx-mdio.h"

int xaui_backplane_loopback_test(void);

/******************************************************************************
 *
 * Function: xaui_backplane_loopback_test
 *
 * Description: This function perform the backplane loopback test from Cavium to
 *                    backplane GE Switch.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int xaui_backplane_loopback_test (void)
{
    testname("XAUI backplane loopback");

    int id;
    id = get_sku_id();

    if (id == WOODLAWN_6GE_1XAUI) {
        cterr('f', 0, "This SKU doesn't support TLK10232");
        return (FAILED);
    }

    /* Config xaui0 for backplane loopback test */
    prpass(testpass, "Config backplane XAUI port");
    config_bp_xaui();
    
    /* Perform backplane loopback test */
    prpass(testpass, "Perform XAUI backplane loopback");
    if ((xaui_lpbk_test(LOOP_XAUI_BP)) == FAILED) {
        cterr('f', 0, "XAUI backplane loopback test failed");
        return (FAILED);
    }
    
    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_backplane_xaui_test.c,v $
 * Revision 1.3  2014/11/12 06:32:59  leschen
 * Support Greyhound switch a
 *
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:13  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.8  2013/03/07 12:24:56  leslie
 * Modify for XAUI backplane loopback test.
 *
 * Revision 1.7  2013/02/18 08:00:28  leslie
 * Add prpass to show process message
 *
 * Revision 1.6  2013/01/18 06:22:07  leslie
 * Fix and clean up code.
 *
 * Revision 1.5  2013/01/16 02:30:23  leslie
 * Include diag_fpga_lib.h
 *
 * Revision 1.4  2013/01/16 02:14:32  leslie
 * Add judge SKU type before config TLK10232 to operate in XAUIB <-> XAUIA.
 *
 * Revision 1.3  2013/01/15 23:28:20  leslie
 * Fix coding style of configuration TLK10232 into XAUIB <-> XAUIA.
 *
 * Revision 1.2  2013/01/13 23:06:46  leslie
 * Add configure TLK10232 into XAUI B <-> XAUI A before doing backplane loopback test.
 *
 * Revision 1.1  2012/12/11 02:44:35  leslie
 * Add XAUI backplane loopback test.
 *
 * $Endlog$
 *-------------------------------------------------
 */

