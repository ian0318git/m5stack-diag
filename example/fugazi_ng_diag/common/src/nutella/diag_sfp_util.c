/* $Id: diag_sfp_util.c,v 1.1 2020/08/07 09:02:35 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_sfp_util.c,v $
 *------------------------------------------------------------------
 * Platform specific code for SFP PHY read/write 
 * 
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
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
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "linux_eth.h"
#include "queryflags.h" /* for query user functions */  
#include "dnv_eth_lib.h"
#include "diag_i350_test.h"
#include "diag_sfp_util.h"

/***************************************************************************
 *
 * Function   : igb_write_sfp_phy
 * Description: Write SFP PHY 
 * Inputs     : which_sfp - which I350 SFP port
 *              regnum - PHY register
 *              write_data - write data
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_write_sfp_phy (int which_sfp, ushort regnum, ushort write_data)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT1) {
        sprintf(ethreq.ifr_name, NUTELLA_I350_SFP_P1_IFACE_NAME);
    } else if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, NUTELLA_I350_SFP_P2_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        close(sk);
        return (FAILED);
    }

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;
    miip->val_in = write_data;

    if (ioctl(sk, SIOCDEVPRIVATE+5, &ethreq) != 0) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }
    close(sk);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: write PHY reg %d = %#.4x\n", ethreq.ifr_name, miip->reg_num, miip->val_in);
    }

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_write_sfp_phy_util
 * Description: Read SFP PHY util
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_write_sfp_phy_util (void)
{
    int which_sfp;
    ushort regnum, regval;

    which_sfp = getdec_answer("\nEnter I350 Port 1/2 SFP ", 1, 1, 2);

    regnum = getdec_answer("\nEnter PHY reg: ", 0, 0, 100);

    regval = gethex_answer("\nEnter data: ", 0, 0, 0xffff);

    printf("I350 port %d: write PHY reg %d = %#.4x\n", which_sfp, regnum, regval);
    if (getdec_answer("\nWrite Data? Yes 1; No 0", 0, 0, 1)) {
        igb_write_sfp_phy (which_sfp, regnum, regval);
    }

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_read_sfp_phy
 * Description: Read SFP PHY 
 * Inputs     : which_sfp - which I350 SFP port
 *              regnum - PHY register
 *              read_data - read data
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_read_sfp_phy (int which_sfp, ushort regnum, ushort *read_data)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT1) {
        sprintf(ethreq.ifr_name, NUTELLA_I350_SFP_P1_IFACE_NAME);
    } else if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, NUTELLA_I350_SFP_P2_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        close(sk);
        return (FAILED);
    }

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;

    if (ioctl(sk, SIOCDEVPRIVATE+4, &ethreq) != 0) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }
    close(sk);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: read PHY reg %d = %#.4x\n", ethreq.ifr_name, miip->reg_num, miip->val_out);
    }

    *read_data = miip->val_out;

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_read_sfp_phy_util
 * Description: Read SFP PHY util
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_read_sfp_phy_util (void)
{
    int which_sfp;
    ushort regnum, read_data;

    which_sfp = getdec_answer("\nEnter I350 Port 1/2 SFP ", 1, 1, 2);

    regnum = getdec_answer("\nEnter PHY reg: ", 0, 0, 100);

    igb_read_sfp_phy(which_sfp, regnum, &read_data);

    printf("I350 port %d: read PHY reg %d = %#.4x\n", which_sfp, regnum, read_data);


    return (PASSED);
}
/*-------------------------------------------------
$Log: diag_sfp_util.c,v $
Revision 1.1  2020/08/07 09:02:35  alicehua
CSCvv24244: Add SFP PHY read/write utilities.


$Endlog$
*/
