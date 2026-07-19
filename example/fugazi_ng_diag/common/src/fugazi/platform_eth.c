/* $Id: platform_eth.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_eth.c,v $
 *------------------------------------------------------------------
 *
 * Platform specific code for Linux base ethernet port loopback test
 * 
 * May 2016, Xiaoying Zhang
 * Jan 2019, Letsai modified for Fugazi.
 *
 * Copyright (c) 2016-2020 by Cisco Systems, Inc.
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
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
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
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  
#include "diag_bcm54194_api.h"
#include "diag_bcm54194_test.h"
#include "platform_ext_lpbk.h"

#define F_GRP	     (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E	     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL	     (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/*****************************************************************************
 ***************************  Globals Variables   ****************************
 *****************************************************************************/
/* PHY addr mapping of front panel port */
/* 1G PHY address interface to SFP */
int ge_port_mapping_phy_addr_down[] = {0x0, 0x0, 0x0, 0x0, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB,};
/* 1G PHY address interface to BCM57412 MAC */
int ge_port_mapping_phy_addr_up[] = {0x0, 0x0, 0x0, 0x0, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,};
/* from PCI mapping to BCM57412 MAC, 1st 1G PHYs start at 3rd MAC */
int ge_phy_mapping_phy_num[] = {2, 3, 4, 5};
int te_port_mapping_phy_addr[] = {0x0, 0x1};

/* disable non support items */
int is_item_available()
{
    return 0;
}



/*------------------------------------------------------------------
 * Function: check_ext_lpbk_flag
 *
 * Check if external loopback flag is set in diag
 *
 * Input:  NONE
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int check_ext_lpbk_flag(void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);  /* EXT loopback flag is not set */
    } else {
        return (TRUE);   /* EXT loopback flag is set */
    }
}

/*------------------------------------------------------------------
 * Function: check_ge_int_lpbk_flag
 *
 * Check if ge internal loopback flag is set in diag
 * Please turn on this flag while no external stub plug in.
 *
 * Input:  NONE
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int check_ge_int_lpbk_flag(void)
{
    /* according to menu_show_dflags(), D_GE_INT_LOOPBACK is inverse flag */ 
    if (diagflag_xram & D_GE_INT_LOOPBACK) {
        return (TRUE);   /* D_GE_INT_LOOPBACK flag is not set */
    } else {
        return (FALSE);  /* D_GE_INT_LOOPBACK flag is set */
    }
}



/*-------------------------------------------------
 * $Log: platform_eth.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.10  2020/08/06 04:27:03  pdoong
 * clean code for BCM54194 1G PHY
 *
 * Revision 1.1.6.9  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.8  2019/10/16 06:12:31  letsai
 * Modify file name
 *
 * Revision 1.1.6.7  2019/04/18 01:21:30  letsai
 * 1. Clean up code
 * 2. Modify 1G phy address mapping
 * 3. Modify print message of MCU FW opgrade
 *
 * Revision 1.1.6.6  2019/04/11 22:32:29  letsai
 * 1. Replace the sign "*" to "-" when doing FPGA interrupt test
 * 2. Fix M.2 combo test when slot is empty.
 * 3. Make "check link utility" easy to use.
 * 4. When USB console detected, check the corresponding FPGA register bit.
 *
 * Revision 1.1.6.5  2019/04/09 16:10:40  letsai
 * 1. Support all BCM54194 PHY (0~3) Register Test.
 * 2. Let utilities can dump each phy registers.
 * 3. Check link status for each phy and each port(upstream and downstream).
 *
 * Revision 1.1.6.4  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.3  2019/03/25 18:37:36  letsai
 * Modified eth and port number
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

