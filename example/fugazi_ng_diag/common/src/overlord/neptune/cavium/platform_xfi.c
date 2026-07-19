/* $Id: platform_xfi.c,v 1.2 2018/05/18 09:24:57 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_xfi.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base xfi port loopback test
 * 
 * June 2016 Mecca Ho
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdio.h>
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

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "proto.h"
#include "queryflags.h"

#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"
#include "diag_common_lib.h"

#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_xfi.h"
#include "platform_ext_lpbk.h"

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/

/* XFI */
int xfi_mapping_qlm_num[] = {CVMX_GMX2_INF_ID, CVMX_GMX2_INF_ID};

/******************************************************************************
 *
 * Function: enable_xfi1_interface
 *
 * Description: This function enables the xfi0 interface.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int enable_xfi1_interface(void)
{
    char *ifconfig_all = "ifconfig -a";
    char *enable_xfi1= "ifconfig xfi1 19.19.19.19 netmask 255.255.0.0 promisc -arp -allmulti -multicast";
    char xfi_test[INFO_LEN];


    /* Clear the system command log buffer */
    memset(xfi_test, '\0', INFO_LEN);

    /* if xfi0 is not existed, enable the xfi0 */
    if (exec_cmd(ifconfig_all, xfi_test, INFO_LEN) == FAILED) {
        cterr('f', 0, "Execute ifconfig command failed");
        return (FAILED);
    } else if(strstr(xfi_test, "xfi1") == NULL) {
        if (exec_cmd(enable_xfi1, xfi_test, INFO_LEN) == FAILED) {
            cterr('f', 0, "Enable xfi1 command failed");
            return (FAILED);
        } else if(strstr(xfi_test, "xfi1") == NULL) {
            cterr('f', 0, "Failed to enalbe xfi1 interface");
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: cavium_is_xfi_linkup
 *
 * Description:
 * Check if the XFI port link status is up (for cavium internal lopback)
 * 
 * Input: bgx_num
 *        xfi_num - ethernet port number
 *
 * Return: true/false
 */
boolean cavium_is_xfi_linkup (int eth_num)
{
    cvmx_bgxx_spux_status1_t spu_status1;
    int index = eth_mapping_xfi_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];
    int repeat = 1000;
 
    do {
        msleep(10);
        spu_status1.u64 = cvmx_read_csr(CVMX_BGXX_SPUX_STATUS1(index, interface));
    } while ((repeat-- > 0) && (spu_status1.s.rcv_lnk == 0));

    return(spu_status1.s.rcv_lnk);
}

/**********************************************************************
 *
 * Function: set_xfi_int_lpbk
 *
 * Description:
 * Set or clear the internal loopback bit of the cavium xfi port
 * 
 * Input: portnum - ethernet port number
 *        onoff - true or false to tur on or off the internal loopback
 *
 * Return: void
 */
void set_xfi_int_lpbk (int eth_num, boolean onoff)
{
#define XFI_DRIVER_DELAY    1 // Kernel need time to bring up link
    int index = eth_mapping_xfi_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];
    cvmx_bgxx_cmrx_config_t cmr_config;
    cvmx_bgxx_spux_control1_t spu_control1;

    /* Set autoneg, clear the loopback just in case other test set it */
    spu_control1.u64 = cvmx_read_csr(CVMX_BGXX_SPUX_CONTROL1(index, interface));
    if (onoff) {
        spu_control1.s.loopbck = 1;
    } else {
        spu_control1.s.loopbck = 0;
    }
    cvmx_write_csr(CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);

    cmr_config.u64 = cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(index, interface));
    cmr_config.s.enable = 1;
    cvmx_write_csr(CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
    /* Give time for HW to settle when loopback is set */
    sleep(XFI_DRIVER_DELAY);

    spu_control1.u64 = cvmx_read_csr(CVMX_BGXX_SPUX_CONTROL1(index, interface));
}

/*-------------------------------------------------
 * $Log: platform_xfi.c,v $
 * Revision 1.2  2018/05/18 09:24:57  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.10  2016/11/29 07:56:10  meho
 * Fixed SGMII and XFI MAC loopback bug.
 *
 * Revision 1.1.2.9  2016/11/29 06:27:52  meho
 * Changed submenu name and code clean up.
 *
 * Revision 1.1.2.8  2016/11/28 03:43:55  meho
 * 1. Fixed GE phy Mac/Int/Ext loopback test bugs.
 * 2. Added 10G FW download.
 *
 * Revision 1.1.2.7  2016/08/24 06:55:53  meho
 * Added dump Cavium sgmii tx/rx statistic register utility.
 *
 * Revision 1.1.2.6  2016/08/18 06:57:49  meho
 * Code clean up.
 *
 * Revision 1.1.2.5  2016/08/12 10:12:19  meho
 * Clean up code.
 *
 * Revision 1.1.2.4  2016/07/26 10:09:43  meho
 * Added 10G PHY PTP1588 loopback test skeleton.
 *
 * Revision 1.1.2.3  2016/07/20 01:45:00  meho
 * Added GE PHY loopback debug utilities.
 *
 * Revision 1.1.2.2  2016/07/14 09:17:41  meho
 * Added internal/SFP-external loopback for BCM82752.
 *
 * Revision 1.1.2.1  2016/07/07 09:04:30  meho
 * 1. Added BCM54194 RDB register r/w utility.
 * 2. Added GE PHY internal/external loopback skeleton.
 * 3. Added 10GE PHY internal/external loopback skeleton.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
