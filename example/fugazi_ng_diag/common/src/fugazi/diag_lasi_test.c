/* $Id: diag_lasi_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_lasi_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_lasi_test.c - Fugazi LASI Diag test.
 *
 * Mar. 2020, Ian Chang <iachang@cisco.com>
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <stdio.h>
#include <linux/mii.h>

#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "dash_fpga.h"
#include "cookie_4.h"
#include "platform_fru.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "diag_miura_reg.h"
#include "diag_bcm_lib.h"
#include "ethernet.h"
#include "diag_eth_traf.h"
#include "diag_bcm82757_test.h"
#include "platform_ext_lpbk.h"
#include "diag_bcm57412_utils.h"
#include "platform_synce_pll_utils.h"
#include "diag_bcm54194_api.h"
#include "diag_lasi_test.h"


#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)


int fugazi_bcm82757_set_sfp_present(fugazi_lane_t);
int bcm82752_emphasis_setting(void);
long bcm82757_reset(int);
int  bcm82757_recover_clock(int, int);
int bcm54194_interrupt_test(int, uint16_t *);
int bcm54194_lasi_test_f(int, uint16_t *);
int ge_port_mapping_phy_addr[] = {0x0, 0x0, 0x0, 0x0, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB,};

static long bcm82757_lasi_test(void);
static long bcm54194_lasi_test(void);
static long bcm54194_interrupt_lasi_utility_f (void);
static long bcm82757_interrupt_lasi_utility_f (void);

    
/******************************************************************************
 *  List of Utilities used for LASI
 *****************************************************************************/
static submenu_xtable_t lasi_util_items[] = {
    {"BCM82757 Register Read", bcm82757_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Write", bcm82757_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 link status", bcm82757_link_status, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Interrupt utility", bcm82757_interrupt, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 initialization", bcm82757_init_f, 0,
     0, NULL, 0, NULL, 0},
    {"Reset and Init All BCM54194 PHY", (type_t(*)())bcm54194_reset, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM82757 Interrupt LASI utility", bcm82757_interrupt_lasi_utility_f, 0,
     0, NULL, 0, NULL, 0},
	{"BCM54194 Interrupt LASI utility", bcm54194_interrupt_lasi_utility_f, 0,
	 0, NULL, 0, NULL, 0},
};
#define LASI_TESTS_UTIL_SIZE (sizeof(lasi_util_items) / sizeof(submenu_xtable_t))
static mitem_t lasi_tests_primary_util_items[LASI_TESTS_UTIL_SIZE +
                                             MAX_BASE_ITEMS];
static mitem_t lasi_tests_secondary_util_items[LASI_TESTS_UTIL_SIZE +
                                               MAX_BASE_ITEMS];

menuinfo_t lasi_util_menu = {
    "LASI Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    lasi_tests_primary_util_items,
};
menuinfo_t *lasi_util_menup = &lasi_util_menu;

/*******************************************************************************
 *
 * Function    : lasi_utility
 * Description :
 * Inputs      : menu_option - display utility for lasi test
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int lasi_utility (int show_menu)
{
    build_primary_submenu(lasi_util_items, LASI_TESTS_UTIL_SIZE,
                          "LASI Utilities Menu", &lasi_util_menup);
    build_secondary_submenu(lasi_util_items, LASI_TESTS_UTIL_SIZE,
                            lasi_tests_secondary_util_items);

    menu(lasi_util_menup, lasi_tests_secondary_util_items, '\0' );

    return (PASSED);
}


/* LASI submenu items */
static submenu_xtable_t lasi_submenu_table[] = {
    {"BCM82757 10G PHY LASI test", bcm82757_lasi_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM54194 1G PHY LASI test", bcm54194_lasi_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"LASI Utility", (type_t(*)())lasi_utility, FALSE,
     0, NULL, 0, (type_t(*)())lasi_utility, TRUE},
};

#define LASI_SUBMENU_TABLE_SZ (sizeof(lasi_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t lasi_submenu_primary_items[LASI_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t lasi_submenu_secondary_items[LASI_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char lasi_submenu_title[] = "Fugazi LASI Subtest Menu";

static menuinfo_t lasi_submenu = {
    lasi_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    lasi_submenu_primary_items,
};

static menuinfo_t *lasi_submenup = &lasi_submenu;

/*
 * Function: fugazi_lasi_test
 *
 * Description: Build the LASI main menu
 *
 * Inputs      : show_menu - display menu instead of running all LASI tests.
 * Outputs     : PASSED / FAILED
 */
int fugazi_lasi_test (int show_menu)
{
    testname("LASI");

    if ((fugazi_bcm82757_init(fugazi_struct))) {
        cterr('f', 0, "fugazi_bcm82757_init failed");
    }

    build_primary_submenu(lasi_submenu_table, LASI_SUBMENU_TABLE_SZ,
                          lasi_submenu_title, &lasi_submenup);
    build_secondary_submenu(lasi_submenu_table, LASI_SUBMENU_TABLE_SZ,
                            lasi_submenu_secondary_items);

    /* Emphasis compliance setting */
    bcm82752_emphasis_setting();

    if (show_menu) {
        menu_exec_doall_diags(lasi_submenup);
    } else {
        menu(lasi_submenup, lasi_submenu_secondary_items, '\0');
    }
    return (PASSED);
}

/*
 * Function: bcm82757_lasi_test_f 
 *
 * Description: BCM82757 LASI Interrupt Test
 *  -  Step1 : HOST MIDO BUS Acquire 
 *  -  Step2 : Initialize bcm82757 10G PHY 
 *  -  Step3 : Clear bcm82757 10G PHY system side interrupt 
 *  -  Step4 : Enable bcm82757 10G PHY system side interrupt
 *  -  Step5 : Clear LASI counter via IOCTL
 *  -  Step6 : Generate LASI interrupt by enable PHY’s Shallow Line-Side loopback
 *  -  Step7 : HOST MIDO BUS Release 
 *  -  Step8 : IOCTL read LASI event counter  (ASYNC_EVENT_CMPL_EVENT_ID_LINK_STATUS_CHANGE)
 *  -  Step9 : Disable bcm82757 10G PHY system side interrupt
 *  -  Step10: Disable PHY’s Shallow Line-Side loopback
 * Inputs      : lane - PHY number.
 * Outputs     : PASSED / FAILED
 */
int bcm82757_lasi_test_f (fugazi_lane_t lane)
{
    fugazi_if_side_t if_side;
    unsigned int enable = 0, intr_type, intr_data = 0;
	int lb_mode ;
    int result = PASSED;

    struct ifreq ifr;
    int sockfd;
    char devname[32];
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr.ifr_data;

    /* Step3 : Clear bcm82757 10G PHY system side interrupt */
    if_side = FUGAZI_IF_SIDE_SYS;
    intr_type = BCM82757_LINK_DOWN_INT; /* Link Down Interrupt*/
    if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                      BCM82757_INT_CLEAR)) {
        cterr('f', 0, "fugazi_bcm82757_interrupt_set failed");
        result = FAILED;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nPort %x Interrupt is cleared",lane);
    }
    msleep(BCM82757_LASI_WAIT_TIME); /* wait for interrupt clear */

    /* Step4 : Enable bcm82757 10G PHY system side interrupt  */
    if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                      BCM82757_INT_ENABLE)) {
        cterr('f', 0, "fugazi_bcm82757_interrupt_set failed");
        result = FAILED;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        fugazi_bcm82757_interrupt_get(fugazi_struct, lane, if_side, intr_type,
                                      &intr_data);
        if (intr_type == intr_data) {
            printf("\nPort %x Interrupt mode enable",lane);
        } else {
            printf("\nPort %x Interrupt mode disable",lane);
        }
    }
    msleep(BCM82757_LASI_WAIT_TIME); /* wait for interrupt enable */
    
    /* Step5 : Clear LASI counter via IOCTL */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "socket set up failed");
        return (FAILED);
    }
    /* clear LASI counter */
    sprintf(devname, "eth%d", lane);
    strcpy(ifr.ifr_name, devname);
    ioctl(sockfd, SIOCSMIILASI, &ifr); /* clear LASI counter */
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nClear LASI counter %x",mdio->val_out);
    }

    /* Step6 : Generate LASI interrupt by enable PHY’s Shallow Line-Side loopback */
    if_side = FUGAZI_IF_SIDE_LINE;  /* Shallow Line-Side loopback*/
    lb_mode = REMOTE_PMD;
    fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, lb_mode, ENABLE);
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        fugazi_bcm82757_loopback_get(fugazi_struct, lane, if_side, lb_mode, 
                                     &enable);
        if (enable) {
            printf("\nPort %x Loopback mode enable", lane);
        } else {
            printf("\nPort %x Loopback mode disable", lane);
        }
    }
    /* Step7 : HOST MIDO BUS Release */
    if (bcm57412_mdio_bus_release(TRUE)) {
        cterr('f', 0, "Fugazi MDIO bus relase failed");
        result = FAILED;
    }
    /* BRCM firmware requires wait 250ms for firmware to send ASYN event to
     *  kernel driver after received LASI interrupt */
    msleep(BCM82757_LASI_WAIT_TIME); 
    /* Step8 : IOCTL read LASI event counter */
    sprintf(devname, "eth%d", lane);
    strcpy(ifr.ifr_name, devname);
    if (ioctl(sockfd, SIOCSMIILASI, &ifr) < 0) {
        cterr('f', 0, "eth port get current setting failed");
        result = FAILED;
    }
    close(sockfd);
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nPort %x LASI contuer %x",lane, mdio->val_out);
    }
    /* CSCvo59196-18 : Fix couldn't access PHY register after LASI test failure */
    /* HOST MIDO BUS Acquire */
    if (bcm57412_mdio_bus_acquire(TRUE)) {
        cterr('f', 0, "Fugazi MDIO bus acquire failed");
        result = FAILED;
    }
    if (mdio->val_out != LASI_EVENT) { /* Check LASI event counter */
        printf("\nPort %x LASI contuer %x",lane, mdio->val_out);
        cterr('f', 0, "Port %x LASI test Failed",lane);
        result = FAILED;
    }
    /* Step9 : Disable bcm82757 10G PHY system side interrupt */
    if_side = FUGAZI_IF_SIDE_SYS;
    if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                      BCM82757_INT_DISABLE)) {
        cterr('f', 0, "fugazi_bcm82757_interrupt_set Failed");
        result = FAILED;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        fugazi_bcm82757_interrupt_get(fugazi_struct, lane, if_side, intr_type,
                                      &intr_data);
        if (intr_type == intr_data) {
            printf("\nPort %x Interrupt mode enable",lane);
        } else {
            printf("\nPort %x Interrupt mode disable",lane);
        }
    }
    /* Step10 : Disable PHY’s Shallow Line-Side loopback (label 2) */
    msleep(BCM82757_LASI_WAIT_TIME); /* Need delay more than 250ms */
    if_side = FUGAZI_IF_SIDE_LINE;  /* Shallow Line-Side loopback*/
    if (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, lb_mode, 
                                     DISABLE)) {
        cterr('f', 0, "fugazi_bcm82757_loopback_disable failed");
        result = FAILED;
    }
    return (result);
}
/*
 * Function: bcm82757_lasi_test 
 *
 * Description: BCM82757 Network Link from up to down LASI Interrupt signal Test 
 *  -  Step1 : HOST MIDO BUS Acquire 
 *  -  Step2 : Initialize bcm82757 10G PHY 
 *  -  Step3 : Clear bcm82757 10G PHY system side interrupt 
 *  -  Step4 : Enable bcm82757 10G PHY system side interrupt
 *  -  Step5 : Clear LASI counter via IOCTL
 *  -  Step6 : Generate LASI interrupt by enable PHY’s Shallow Line-Side loopback
 *  -  Step7 : HOST MIDO BUS Release 
 *  -  Step8 : IOCTL read LASI event counter  (ASYNC_EVENT_CMPL_EVENT_ID_LINK_STATUS_CHANGE)
 *  -  Step9 : Disable bcm82757 10G PHY system side interrupt
 *  -  Step10: Disable PHY’s Shallow Line-Side loopback
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_lasi_test (void)
{
    fugazi_lane_t lane;
	int lane_start, lane_end, rc, ix ;
    int result = PASSED;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip BCM82757 LASI test\n");
        return (FAILED);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        system("dmesg -n 7");
    } else {
        system("dmesg -n 1");
    }

    testname("BCM82757 LASI");
    lane_start = 0;
   	lane_end = MAX_NR_FUGAZI_LANE;
    /* Step1 : HOST MIDO BUS Acquire */
    if (bcm57412_mdio_bus_acquire(TRUE)) {
        cterr('f', 0, "Fugazi MDIO bus acquire failed");
        result = FAILED;
    }

    /* Step2 : Initialize bcm82757 10G PHY */
    for (lane = lane_start; lane < lane_end; lane++) {
    	printf("\n\rPHY port %d initialization ... ", lane);
		rc |= bcm82757_PHY_init(lane, FUGAZI_PORT_SPEED_10G);
	    if (rc < 0) {
	    	printf("failed.");
	    }
	    else {
	    	printf("successful.");
	    }
	}
    
    /* Test two times to make sure LASI will not stuck at low */
    for (ix = 0; ix < 2; ix++) {
        for (lane = lane_start; lane < lane_end; lane++) {
            /* BCM82757 LASI Interrupt Test step3 to step10 */
            if (bcm82757_lasi_test_f(lane) ) {
                cterr('f', 0, "PHY %d Test FAILED at loop %d,", lane, ix + 1);
                result = FAILED;
            } else {
                prpass(testpass, "PHY %d Test PASSED at loop %d,", lane, ix + 1);
            }
        }
    }
    return (result);
}
/*
 * Function: bcm54194_lasi_test_f
 *
 * Description:
 *   BCM54194 LASI LASI Interrupt test function
 *
 *  -  Step1 : Reset BCM54194_1G PHY 
 *  -  Step2 : HOST MIDO BUS Acquire 
 *  -  Step3 : Configure PHY core interrupts on the INTRP 
 *  -  Step4 : Clear BCM54194 Link change Interrupt  
 *  -  Step5 : Enable BCM54194 Link change Interrupt 
 *  -  Step6 : Clear LASI counter via IOCTL 
 *  -  Step7 : Power off fiber - Link up to Down 
 *  -  Step8 : HOST MIDO BUS Release  
 *  -  Step9 : IOCTL read LASI event counter 
 *  -  Step10 : HOST MIDO BUS Acquire 
 *  -  Step11 : Disable BCM54194 Link change Interrupt 
 *  -  Step12 : Read BCM54194 interrupt status 
 *  -  Step13 : Power on Fiber - Link down to Up 
 * Input: ethnum_index : port number
 *        int_status   : interrupt status.
 *
 * Return: PASSED / FAILED
 */
int bcm54194_lasi_test_f(int ethnum_index, uint16_t *int_status)
{
    int rc = PASSED;
    int phy_addr, phy_num, phy_port;
    uint16_t status = 0;
    struct ifreq ifr;
    int sockfd;
    char devname[32];
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr.ifr_data;
    int *ge_port_phy_addr = ge_port_mapping_phy_addr;

    phy_addr = ge_port_phy_addr[ethnum_index];
    phy_num  = (int) (ethnum_index / 2);
    phy_port = (int) (ethnum_index % phy_num);
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nethnum_index=%d, phy_num=%d, phy_port=%d, phy_addr=0x%02x\n",
                 ethnum_index, phy_num, phy_port, phy_addr);
    } 
    /* Step2 : HOST MIDO BUS Acquire */
    if (bcm57412_mdio_bus_acquire(TRUE)) {
        cterr('f', 0, "Fugazi MDIO bus acquire failed");
        rc = FAILED;
    }
 
    /* Step3 : Configure PHY core interrupts on the INTRP */
    if (bcm54194_config_interrupt(phy_num, phy_port, ENABLE)) {
        cterr('f', 0, "Failed to bcm54194_config_interrupt() GE PHY %d, phy_port %d, "
               "phy addr:0x%x\n", phy_num - 2, phy_port, phy_addr);
        rc = FAILED;
    }
 
    /* Step4 : Clear BCM54194 Link change Interrupt */
    if (bcm54194_interrupt_clear(phy_addr, phy_num)) {
        cterr('f', 0, "Failed to bcm54194_interrupt_clear() GE PHY %d, phy addr:0x%x\n",
              phy_num - 2, phy_addr);
        rc = FAILED;
    }
    /* Step5 : Enable BCM54194 SERDES LINK Status Change interrupt */
    if (bcm54194_interrupt_set(phy_addr, phy_num, ENABLE)) {
        cterr('f', 0, "Failed to bcm54194_interrupt_set(ENABLE) GE PHY %d, phy addr:"
               "0x%x\n", phy_num - 2, phy_addr);
        rc = FAILED;
    }
    msleep(10);


    /* Step6 : Clear LASI counter via IOCTL */
 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "socket set up failed");
        return (FAILED);
    }
 
    /* clear LASI counter */
    sprintf(devname, "eth%d", ethnum_index);
    strcpy(ifr.ifr_name, devname);
    ioctl(sockfd, SIOCSMIILASI, &ifr); /* clear LASI counter */
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nClear LASI counter %x\n",mdio->val_out);
    }
 
    /* Step7 : Power off fiber - Link up to Down on Network side */
    if ((rc = bcm54194_sig_pwr_ctrl(phy_num, phy_addr, DISABLE, 
                                    BCM54194_FIBER_INTF)) != PASSED) {
        cterr('f',0, "GE port %d: disable fiber failed", phy_addr);
        return(FAILED);
    }
    /* Step8 : HOST MIDO BUS Release */
    if (bcm57412_mdio_bus_release(TRUE)) {
        cterr('f', 0, "Fugazi MDIO bus relase failed");
        rc = FAILED;
    }
    /* BRCM firmware requires wait 300ms for firmware to send ASYN event to 
     * kernel driver after received LASI interrupt */
    msleep(BCM82757_LASI_WAIT_TIME); 

    /* Step9 : IOCTL read LASI event counter */
    sprintf(devname, "eth%d", ethnum_index);
    strcpy(ifr.ifr_name, devname);
    if (ioctl(sockfd, SIOCSMIILASI, &ifr) < 0) {
        cterr('f', 0, "eth port get current setting failed");
        rc = FAILED;
    }
    close(sockfd);
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nPort %d LASI counter %x\n",ethnum_index, mdio->val_out);
    }
    /* CSCvo59196-18 : Fix couldn't access PHY register after LASI test failure */
    /* Step10 : HOST MIDO BUS Acquire */
    if (bcm57412_mdio_bus_acquire(TRUE)) {
        cterr('f', 0, "Fugazi MDIO bus acquire failed");
        rc = FAILED;
    }
 
    if (mdio->val_out != LASI_EVENT) { /* Check LASI event counter */
        printf("\nPort %d LASI contuer %x", ethnum_index, mdio->val_out);
        cterr('f', 0, "PHY %d LASI test Failed", ethnum_index);
        rc = FAILED;
    }  
 
    /* Step11 : Disable BCM54194 SERDES LINK Status Change interrupt */
    if (bcm54194_interrupt_set(phy_addr, phy_num, DISABLE)) {
        cterr('f', 0, "Failed to bcm54194_interrupt_set() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc = FAILED;
    }
    /* Step12 : Read BCM54194 interrupt status */
    if (bcm54194_interrupt_get(phy_addr, phy_num, &status)) {
        cterr('f', 0, "Failed to bcm54194_interrupt_get() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc = FAILED;
    } else {
        *int_status = status;
    }
    /* Step13 : Power on Fiber - Link down to Up */
    if ((rc = bcm54194_sig_pwr_ctrl(phy_num, phy_addr, ENABLE, 
                                    BCM54194_FIBER_INTF)) != PASSED) {
        cterr('f',0, "GE port %d: disable fiber failed", phy_addr);
        return(FAILED);
    }
 
    msleep(BCM82757_LASI_WAIT_TIME); /* Waite for PHY stable */
 
    return (rc);
}
/*
 * Function: bcm54194_lasi_test 
 *
 * Description: BCM54194 network side link change LASI Interrupt Test
 *  -  Step1 : Reset BCM54194_1G PHY 
 *  -  Step2 : HOST MIDO BUS Acquire 
 *  -  Step3 : Configure PHY core interrupts on the INTRP 
 *  -  Step4 : Clear BCM54194 Link change Interrupt  
 *  -  Step5 : Enable BCM54194 SERDES LINK Status Change interrupt 
 *  -  Step6 : Clear LASI counter via IOCTL 
 *  -  Step7 : Power off fiber - Link up to Down on Network side 
 *  -  Step8 : HOST MIDO BUS Release  
 *  -  Step9 : IOCTL read LASI event counter 
 *  -  Step10 : HOST MIDO BUS Acquire 
 *  -  Step11 : Disable BCM54194 SERDES LINK Status Change interrupt 
 *  -  Step12 : Read BCM54194 interrupt status 
 *  -  Step13 : Power on Fiber - Link down to Up 
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm54194_lasi_test (void)
{
    int rc = PASSED;
	int ethnum_index, eth_start, eth_end, ix;
    uint16_t int_status=0;


    eth_start = FUGAZI_1G_eth_4;
    eth_end = MAX_FUGAZI_1G_ETH;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip BCM54194 LASI test\n");
        return (FAILED);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        system("dmesg -n 7");
    } else {
        system("dmesg -n 1");
    }
    testname("BCM54194 LASI");
    /* Step1 : Reset BCM54194_1G PHY */
    bcm54194_reset(0);
    msleep(BCM82757_LASI_WAIT_TIME); /* Waite for PHY initial */
    /* Test two times to make sure LASI will not stuck at low */
    for (ix = 0; ix < 2; ix++) {
        for (ethnum_index = eth_start; ethnum_index < eth_end; ethnum_index++) {
            int_status = 0;
            if (bcm54194_lasi_test_f(ethnum_index, &int_status) ) {
                cterr('f', 0, "PHY %d Test FAILED at loop %d,", ethnum_index, 
                       ix + 1);
                rc = FAILED;
            } else {
                prpass(testpass, "PHY %d Test PASSED at loop %d,", ethnum_index, 
                       ix + 1);
                printf("\n");
            }
        }
    }
	return (rc);
}

static int bcm54194_interrupt_lasi_test_util(int ethnum_index, uint16_t *int_status)
{
    int rc = PASSED;
    int phy_addr, phy_num, phy_port;
    uint16_t status = 0;
    struct ifreq ifr;
    int sockfd;
    char devname[32];
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr.ifr_data;
    int *ge_port_phy_addr = ge_port_mapping_phy_addr;

    phy_addr = ge_port_phy_addr[ethnum_index];
    phy_num  = (int) (ethnum_index/2);
    phy_port = (int) (ethnum_index%phy_num);
    printf("\nethnum_index=%d, phy_num=%d, phy_port=%d, phy_addr=0x%02x\n",
            ethnum_index, phy_num, phy_port, phy_addr);
   
    /* Create IOCTL */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "socket set up failed");
        return (FAILED);
    }
   
    /* Configure PHY core interrupts on the INTRP */
    if ( bcm54194_config_interrupt(phy_num, phy_port, ENABLE) ) {
        printf("Failed to bcm54194_config_interrupt() GE PHY %d, phy_port %d," 
               "phy addr:0x%x\n", phy_num-2, phy_port, phy_addr);
        rc |= FAILED;
    }
   
    /* Clear BCM54194 Link change Interrupt */
    if ( bcm54194_interrupt_clear(phy_addr, phy_num) ) {
        printf("Failed to bcm54194_interrupt_clear() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }
   
    /* Enable BCM54194 Network Side Link Change Interrupt */
    if ( bcm54194_interrupt_set(phy_addr, phy_num, ENABLE) ) {
        printf("Failed to bcm54194_interrupt_set(ENABLE) GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }
    msleep(10);
   
    /* Clear BCM54194 Link change Interrupt */
    if ( bcm54194_interrupt_clear(phy_addr, phy_num) ) {
        printf("Failed to bcm54194_interrupt_clear() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }
    msleep(10);

    /* clear LASI counter */
    sprintf(devname, "eth%d", ethnum_index);
    strcpy(ifr.ifr_name, devname);
    ioctl(sockfd, SIOCSMIILASI, &ifr); /* clear LASI counter */
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nClear LASI counter %d\n",mdio->val_out);
    }

    printf("\npress any key to enable loopback...");
    getchar();

    /* Generate interrupts by enable System loopback */
    if ( bcm54194_interrupt_generate(phy_addr, phy_num, ENABLE) ) {
        printf("Failed to bcm54194_interrupt_generate(ENABLE) GE PHY %d, "
               "phy addr:0x%x\n", phy_num-2, phy_addr);
        rc |= FAILED;
    }
    /* BRCM firmware requires max wait time 300ms for firmware to send ASYN event
     * to kernel driver after received LASI interrupt */
    msleep(BCM82757_LASI_WAIT_TIME);  
   
    /* IOCTL read LASI event counter */
    sprintf(devname, "eth%d", ethnum_index);
    strcpy(ifr.ifr_name, devname);
    if (ioctl(sockfd, SIOCSMIILASI, &ifr) < 0) {
        cterr('f', 0, "eth port get current setting failed");
        rc = FAILED;
        close(sockfd);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nAfter enable loopback Port %d LASI counter %d\n",ethnum_index, 
                mdio->val_out);
    }
   
    if (mdio->val_out == 2) { /* The link status change min. 2 times */
        printf("\nPHY %d test Passed, ", ethnum_index);
        printf("\n");
    } else {
        printf("\nPort %d LASI contuer %x", ethnum_index, mdio->val_out);
        cterr('f', 0, "PHY %d LASI test Failed", ethnum_index);
        rc |= FAILED;
    }
   
    /* Disable System loopback */
    if ( bcm54194_interrupt_generate(phy_addr, phy_num, DISABLE) ) {
        printf("Failed to bcm54194_interrupt_generate(DISABLE) GE PHY %d, "
               "phy addr:0x%x\n", phy_num-2, phy_addr);
        rc |= FAILED;
    }
   
    /* Read BCM54194 interrupt status */
    if ( bcm54194_interrupt_get(phy_addr, phy_num, &status) ) {
        printf("Failed to bcm54194_interrupt_get() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    } else {
        *int_status = status;
    }
       
    /* Disable BCM54194 Link change Interrupt */
    if ( bcm54194_interrupt_set(phy_addr, phy_num, DISABLE) ) {
        printf("Failed to bcm54194_interrupt_set() GE PHY %d, phy addr:0x%x\n",
        		phy_num-2, phy_addr);
        rc |= FAILED;
    }
   
    /* IOCTL read LASI event counter */
    sprintf(devname, "eth%d", ethnum_index);
    strcpy(ifr.ifr_name, devname);
    if (ioctl(sockfd, SIOCSMIILASI, &ifr) < 0) {
        cterr('f', 0, "eth port get current setting failed");
        rc = FAILED;
    }
    close(sockfd);
   
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nAfter disavle interrupt, Port %d LASI counter %d\n", 
                ethnum_index, mdio->val_out);
    }
   
    return (rc);
}


/*
 * Function: bcm54194_interrupt_lasi_utility_f
 *
 * Description: BCM54194 LASI Interrupt Test for HW debug
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm54194_interrupt_lasi_utility_f (void)
{
    int rc = PASSED;
    int ethnum, ethnum_index, eth_start, eth_end;
    uint16_t int_status=0;


    printf("\nPort number 0 - 7 = ETH4 - ETH11");
    ethnum = gethex_answer("\nEnter eth num (0x4 - 0xB; 0xff-all ports)", 0xff, 
                            4, 0xff);
   
    if ( ethnum == 0xff) {
        eth_start = FUGAZI_1G_eth_4;
        eth_end = MAX_FUGAZI_1G_ETH;
    } else {
        eth_start = ethnum;
        eth_end = ethnum + 1;
    }
   
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        system("dmesg -n 7");
    } else {
        system("dmesg -n 1");
    }
   
    for (ethnum_index=eth_start; ethnum_index<eth_end; ethnum_index++) {
        testname("BCM54194 PHY eth%d LASI test", ethnum_index);
        int_status = 0;
        if ( bcm54194_interrupt_lasi_test_util(ethnum_index, &int_status) ) {
            rc |= FAILED;
            prpass(testpass, "- FAILED! (int_status=0x%04X)\n", int_status);
        } else {
            prpass(testpass, "- PASSED\n");
        }
    }
   
    return (rc);
}

/*
 * Function: bcm82757_interrupt_lasi_utility_f
 *
 * Description: BCM82757 LASI Interrupt test for HW
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_interrupt_lasi_utility_f (void)
{
    int result = PASSED;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int intr_type;
    int lane_start, lane_end, lb_mode;
    struct ifreq ifr;
    int sockfd;
    char devname[32];
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr.ifr_data;


    lane = gethex_answer("Enter Port(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    if ( lane == 0xff) {
        lane_start = 0;
        lane_end = MAX_NR_FUGAZI_LANE;
    }
    else {
        lane_start = lane;
        lane_end = lane + 1;
    }

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        system("dmesg -n 7");
    } else {
        system("dmesg -n 1");
    }


    for (lane=lane_start; lane<lane_end; lane++) {
        /* Create IOCTL */
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            cterr('f', 0, "socket set up failed");
            return (FAILED);
        }

        if_side = FUGAZI_IF_SIDE_SYS;
        intr_type = BCM82757_LINK_DOWN_INT; /* Link Down Interrupt*/

        if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                          BCM82757_INT_CLEAR)) {
            cterr('f', 0, "fugazi_bcm82757_interrupt_set failed");
            result |= FAILED;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nPort %x Interrupt is cleared",lane);
        }
        msleep(100); /* wait for interrupt clear */

        /* Step3 : Enable bcm82757 10G PHY system side interrupt  */
        if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                          BCM82757_INT_ENABLE)) {
            cterr('f', 0, "fugazi_bcm82757_interrupt_set failed");
            result |= FAILED;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nPort %x Interrupt mode enable",lane);
        }
        msleep(250); /* wait for interrupt enable */

        if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                          BCM82757_INT_CLEAR)) {
            cterr('f', 0, "fugazi_bcm82757_interrupt_set failed");
            result |= FAILED;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nPort %x Interrupt is cleared",lane);
        }
        msleep(250); /* wait for interrupt clear */

        /* clear LASI counter */
        sprintf(devname, "eth%d", lane);
        strcpy(ifr.ifr_name, devname);
        ioctl(sockfd, SIOCSMIILASI, &ifr); /* clear LASI counter */
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nClear LASI counter %d\n",mdio->val_out);
        }


        printf("\npress any key to enable loopback...");
        getchar();

        /* Step5 : Generate LASI interrupt by enable PHY’s Deep/Shallow System-Side loopback */
        if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            /* External loopback flag is Off: Deep system-side loopback */
            printf("\nExternal loopback flag is Off.\n");
            if_side = FUGAZI_IF_SIDE_LINE;
            lb_mode = DIGITAL_PMD;
            /* eable it to make link up first */
            if (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, 
                                             lb_mode, ENABLE)) {
                cterr('f', 0, "fugazi_bcm82757_loopback_enable failed");
                result |= FAILED;
            }
            msleep(300); /* LASI interrupt is level triggered */

            /* then, diable it to make link down */
            if (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, 
                                             lb_mode, DISABLE)) {
                cterr('f', 0, "fugazi_bcm82757_loopback_enable failed");
                result |= FAILED;
            }
            msleep(300); /* LASI interrupt is level triggered */
        } else {
            /* External loopback flag is On: Shallow System-Side loopback  */
            printf("\nExternal loopback flag is On.\n");
            if_side = FUGAZI_IF_SIDE_SYS;
            lb_mode = REMOTE_PMD;
            if (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, 
                                             lb_mode, ENABLE)) {
                cterr('f', 0, "fugazi_bcm82757_loopback_enable failed");
                result |= FAILED;
            }
            msleep(300); /* LASI interrupt is level triggered */
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nPort %x Loopback mode enable", lane);
        }

        /* IOCTL read LASI event counter */
        sprintf(devname, "eth%d", lane);
        strcpy(ifr.ifr_name, devname);
        mdio->val_out = 0;
        if (ioctl(sockfd, SIOCSMIILASI, &ifr) < 0) {
            cterr('f', 0, "eth port get current setting failed");
            result |= FAILED;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nAfter enable loopback, Port %d LASI counter %d\n", lane,
                   mdio->val_out);
        }

        /* Step7 : Disable bcm82757 10G PHY system side interrupt */
        if_side = FUGAZI_IF_SIDE_SYS;
        if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, 
                                          intr_type, DISABLE)) {
            cterr('f', 0, "fugazi_bcm82757_interrupt_set Failed");
            result |= FAILED;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nPort %x Interrupt mode disable",lane);
        }

        /* Step8 : Disable PHY Deep/Shallow System-Side loopback (label 2) */
        msleep(10); /* Need delay more than 250ms */
        if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            if_side = FUGAZI_IF_SIDE_LINE;
            lb_mode = DIGITAL_PMD;
        } else {
            if_side = FUGAZI_IF_SIDE_SYS;
            lb_mode = REMOTE_PMD;
        }
        if (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, lb_mode, 
                                         DISABLE)) {
            cterr('f', 0, "fugazi_bcm82757_loopback_disable failed");
            result |= FAILED;
       }

        msleep(10); /* Need delay more than 250ms for link up ASYN event */
        /* IOCTL read LASI event counter */
        sprintf(devname, "eth%d", lane);
        strcpy(ifr.ifr_name, devname);
        mdio->val_out = 0;
        if (ioctl(sockfd, SIOCSMIILASI, &ifr) < 0) {
            cterr('f', 0, "eth port get current setting failed");
            result |= FAILED;
        }
        printf("\nAfter disable interrupt, Port %d LASI counter %d\n", lane,
                mdio->val_out);

        close(sockfd);

    } /*for (lane=lane_start; lane<lane_end; lane++) { */

    return (result);
}


/*
 *------------------------------------------------------------------
 * $Log: diag_lasi_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.3  2020/10/08 02:40:48  iachang
 * CSCvo59196-18 : Fix couldn't access PHY register after LASI test failure
 *
 * Revision 1.1.4.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.16  2020/08/24 00:06:17  pdoong
 * Clean code for ER.
 *
 * Revision 1.1.2.15  2020/08/21 03:39:17  iachang
 * Change cterr() to printf() inside bcm57412_mdio_bus_acquire/release() function ,
 * avoid BCM57412 FW didn't support MIDO bus control and get cterr() message when platform initial
 *
 * Revision 1.1.2.14  2020/08/20 11:29:49  iachang
 * PRRQ CSCvo59196-8 : LASI and Side Band code review
 *
 * Revision 1.1.2.13  2020/08/06 02:43:30  iachang
 * Code clean up.
 *
 * Revision 1.1.2.12  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.2.11  2020/07/20 09:27:49  iachang
 * Skip LASI test when didn't plug-in SFP and loopback
 *
 * Revision 1.1.2.10  2020/07/16 01:59:18  iachang
 * Test two times to make sure LASI pin release
 *
 * Revision 1.1.2.9  2020/07/13 08:13:49  iachang
 * BCM54194 1G PHY LASI test with loopback.
 *
 * Revision 1.1.2.8  2020/07/10 08:56:23  iachang
 * BCM82757 10G PHY LASI test with loopback.
 *
 * Revision 1.1.2.7  2020/07/03 07:34:20  iachang
 * Support bcm57412_mdio_bus_release() and bcm57412_mdio_bus_acquire()
 * Move those funcitons from diag_bcm57412_test.c to diag_bcm57412_utils.c
 *
 * Revision 1.1.2.6  2020/04/24 23:35:32  pdoong
 * Fix build error.
 *
 * Revision 1.1.2.5  2020/04/24 02:17:30  pdoong
 * Removed I changed from bcm54194_lasi_test.
 *
 * Revision 1.1.2.4  2020/04/23 23:47:49  pdoong
 * Add BCM82757/BCM54194 LASI Interrupt test utility
 *
 * Revision 1.1.2.3  2020/04/16 01:56:44  iachang
 * Add BCM54194 1G PHY LASI test.
 *
 * Revision 1.1.2.2  2020/04/06 07:05:10  iachang
 * Add BCM54194 LASI Interrupt Test
 *
 * Revision 1.1.2.1  2020/03/18 06:51:44  iachang
 * Create independent file for LASI test
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
