/* $Id: platform_smi.c,v 1.2 2013/10/08 08:48:31 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_smi.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base SMI port register R/W test
 * 
 * Mat 2012 Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
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

#include "nvsysvars.h"
#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "menu.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"

#include "platform_smi_lib.h"

int read_ten_g_phy_reg(ulong, int, uint *, void *);
int write_ten_g_phy_reg(ulong, int, uint, void *);

/*******************************************************************
 *
 * Function    : read_ten_g_phy_reg
 * Description : SMI read funtion for ten_g_phy reg test.
 * Input       : addr  - register offset.
 *               size  - read data size
 *               buf   - read buffer
 *               param - parameter
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
 int read_ten_g_phy_reg (ulong addr, int size, uint *buff, void *addr_info)
{
    ten_g_phy_t *phy_addr_info = (ten_g_phy_t *)addr_info;
    uint mii_value;
    int phy_id, bus_id, dev_id;

    bus_id = (phy_addr_info->port_id) >> 4;
    phy_id = (phy_addr_info->port_id) & 0xF;
    dev_id = phy_addr_info->device_id;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, addr);

    if (mii_value < 0) {
        cterr('f', 0, "Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s() bus_id is %d phy_id is %d dev_id is %d mii_value is %x\n",
                    __FUNCTION__, bus_id, phy_id, dev_id, mii_value);
        }
        *buff = mii_value;
        return (PASSED);
    }
}

/*******************************************************************
 *
 * Function    : write_ten_g_phy_reg
 * Description : SMI write funtion for ten_g_phy reg test.
 * Input       : addr  - register offset.
 *               size  - read data size
 *               value - data to be written.
 *               param - parameter
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
int write_ten_g_phy_reg (ulong addr, int size, uint val, void *addr_info)
{
    ten_g_phy_t *phy_addr_info = (ten_g_phy_t *)addr_info;
    int status;
    int phy_id, bus_id, dev_id;

    bus_id = (phy_addr_info->port_id) >> 4;
    phy_id = (phy_addr_info->port_id) & 0xF;
    dev_id = phy_addr_info->device_id;

    status = cvmx_mdio_45_write(bus_id, phy_id, dev_id, (int)addr, val);

    if (status < 0) {
        cterr('f', 0, "Write error to device %d(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s() bus_id is %d phy_id is %d dev_id is %d val is %x\n",
                    __FUNCTION__, bus_id, phy_id, dev_id, val);
        }
        return(PASSED);
    }

}

/*-------------------------------------------------
 * $Log: platform_smi.c,v $
 * Revision 1.2  2013/10/08 08:48:31  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:43:02  kuangik
 * Add for the first time
 *
 * Revision 1.10  2013/01/18 06:48:35  leslie
 * Fix and clean up code.
 *
 * Revision 1.9  2012/10/24 10:46:09  leslie
 * Fix and clean up code.
 *
 * Revision 1.8  2012/09/21 11:54:00  kody
 * Clean up the code.
 *
 * Revision 1.7  2012/09/05 22:50:27  leslie
 * Modify variable type.
 *
 * Revision 1.6  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/07/19 06:24:23  leslie
 * Modify cterr first argument phy_id to dev_id
 *
 * Revision 1.3  2012/05/16 03:32:14  leslie
 * Modify variable type
 *
 * Revision 1.2  2012/05/15 01:26:53  leslie
 * Include header file platform_lib.h
 *
 * Revision 1.1  2012/05/11 04:43:58  leslie
 * Add platform code for SMI port register R/W
 *
 * $Endlog$
 *-------------------------------------------------
 */

