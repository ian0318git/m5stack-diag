/* $Id: plug_testcard_phy.c,v 1.4 2019/08/06 06:56:16 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_phy.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : plug_testcard_phy.c
 * Description: PLUG testcard GE PHY(Marvell 88E1112) Diag tests and utilities.
 *
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <ifaddrs.h> /* for using getifaddrs */
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>  /* for including the linux_eth.h */
#include <unistd.h>
#include "monitor.h"
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "nvmonvars.h"
#include "ethernet.h"
#include "common_utils.h"
#include "linux_eth.h"
#include "dev_object.h"
#include "proto.h"
#include "byteswap.h"
#include "plug_slot.h"
#include "plug_common_host_impl.h"
#include "plug_testcard_phy.h"
#include "plug_testcard_host.h"
#include "plug_testcard_host_impl.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/*******************************************************************************
 *                                  Global                                      
 *******************************************************************************
 */
#define SEC_TO_MICROSEC         1000000.0
#define MAX_CHECKTIME_USEC      1000000   /* 1sec */
#define MAX_POLLING_COUNTS      100
#define POLLING_INTRVL          100 /* 100ms */
#define MAX_TRY                 5

#define PLUG_TC_ETH_MAX_RETRY   10

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int plug_tc_i2cphy_reg_rd(int, ushort *);
int plug_tc_i2cphy_reg_wr(int, ushort);

static int plug_tc_gephy_reg_test(int);
static int plug_tc_gephy_utils(int);
static int plug_tc_gephy_set_1000x_default(void);
static int plug_tc_ge_phy_1000x_ext_lpbk_test(int);
static int plug_tc_ge_1000x_lpbk_test(int);
static int plug_tc_gephy_reg_rd_util(int);
static int plug_tc_gephy_reg_wr_util(int);
static int plug_tc_gephy_copper_mac_lpbk_test(int);
extern int plug_curr_i2c_ctrl;


static int plug_tc_gephy_set_test_mode(int);
static int plug_tc_gephy_set_txtype_util(int);
static int plug_tc_set_gephy_txtype(int, ushort);
static int plug_tc_phy_ext_lpbk_test(int, int, int);
static int plug_tc_reset_plug_eth_phy(int);
static int plug_tc_set_phy_1000x_ext_lpbk(int, int);
static int plug_tc_set_media_phy_reg_checks(int);
static int plug_tc_sgmii_lpbk_test(int, int);
static int plug_tc_init_1000x_env(char *, int, int, int);
static int plug_tc_set_phy_stub(int, boolean, boolean);
static int plug_tc_phy_soft_reset(int, boolean);
static int plug_tc_check_link_status(int);
static int plug_tc_sig_pwr_ctrl(int, boolean, boolean);
static int plug_tc_sig_set_port_speed(int, int, boolean);
static int plug_tc_cfg_phy_setting(int, int, int, int, boolean);
static int plug_tc_sig_set_speed(int, int, boolean);
static int plug_tc_set_promisc(int);


/* Packets to be used in xaui port loopback tests
 * we leaave 12 byte for put mac address into the packet
 */
static pktdata_info_t pktdata[] = {
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 5},
  {0xa7, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 5},
  {0xa5, ((ETH_UDP_DATA_MAX_LEN - 1) - 12), H_INCFILL, 5},
  {0xa3, (ETH_UDP_DATA_MAX_LEN - 12), H_INCFILL, 5},
};

extern struct plug_intf_t *plug_test_if;

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
/* GE PHY copper speed table */
static int gephy_copper_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};


static submenu_xtable_t plug_tc_gephy_diag_tbl[] = {
    {"PLUG testcard GE PHY Utilities", (type_t(*)())plug_tc_gephy_utils,   FALSE,
        0, NULL, 0, (type_t(*)())0, 0},
    {"PLUG testcard PHY Register Test", (type_t(*)())plug_tc_gephy_reg_test, PLUG_GE1_ETHNUM,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PLUG testcard PHY MAC Loopback Test",
     (type_t(*)())plug_tc_gephy_copper_mac_lpbk_test,         PLUG_GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PLUG testcard  External Loopback Test", (type_t(*)())plug_tc_ge_phy_1000x_ext_lpbk_test, PLUG_GE1,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define PLUG_TC_GEPHY_DIAG_TBL_SIZE (sizeof(plug_tc_gephy_diag_tbl) / \
                                  sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t plug_tc_gephy_diag_pri_items[PLUG_TC_GEPHY_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t plug_tc_gephy_diag_sec_items[PLUG_TC_GEPHY_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t plug_tc_gephy_diag_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    plug_tc_gephy_diag_pri_items,
};
menuinfo_t *plug_tc_gephy_diag_menup = &plug_tc_gephy_diag_menu;

/* List of PLUG GE PHY Utilities */
static submenu_xtable_t plug_tc_gephy_util_items[] = {
    {"PLUG TC GE PHY register read", (type_t(*)())plug_tc_gephy_reg_rd_util, PLUG_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PLUG TC GE PHY register write", (type_t(*)())plug_tc_gephy_reg_wr_util, PLUG_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Send packet to PLUG TC GE PHY", (type_t(*)())plug_tc_host_ge_send_packet_util, PLUG_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PLUG TC Set PHY 1000BaseT Test mode", (type_t(*)())plug_tc_gephy_set_test_mode, PLUG_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PLUG TC Set PHY Transmitter Type", (type_t(*)())plug_tc_gephy_set_txtype_util, PLUG_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_TC_GEPHY_UTIL_SIZE (sizeof(plug_tc_gephy_util_items) / sizeof(submenu_xtable_t))

/*
 * PLUG TEST GE PHY utility items (filled in from xtable)
 */
static mitem_t plug_tc_gephy_util_pri_items[PLUG_TC_GEPHY_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t plug_tc_gephy_util_sec_items[PLUG_TC_GEPHY_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * PLUG GE PHY Utility Submenu
 */
menuinfo_t plug_tc_gephy_util_menu = {
    "%s Menu",
    0,   
    (PFT)show_endnote,
    0,   
    0,   
    plug_tc_gephy_util_pri_items,
};

menuinfo_t *plug_tc_gephy_util_menup = &plug_tc_gephy_util_menu;

/**********************************************************************
 *
 * Function   : plug_tc_gephy_set_test_mode
 * Description: Utility to set plug testcard GE WAN PHY 1000BaseT test mode.
 * Inputs     : eth_num = ethernet number(eth0, eth1, eth2,...) 
 * Outputs    : PASSED/FAILED
 *
 **********************************************************************
 */
int plug_tc_gephy_set_test_mode (int eth_num)
{
    int    reg_page = 0, reg_addr = 0;
    ushort reg_val = 0, test_mode = 0;
    int    ctr = 0;

    /* Got the current mode. */
    reg_page = (int)PHY_PAGE(0);
    reg_addr = (int)GEPHY_1000T_CNTL_REG;
    if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get test mode(Reg. %d_%d)\n",
               __FUNCTION__, reg_page, reg_addr);
	    return(FAILED);
    }
    test_mode = (ushort)((reg_val & ONEK_CNTL_TESTMODE_MASK) >>
                         ONEK_CNTL_TESTMODE_SHIFT);

    printf("\nTest modes -\n");
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = (ushort)gethex_answer("Enter the test mode: ", test_mode, 0, 4);

    /* Write the new data */
    reg_val &= (ushort)(~ONEK_CNTL_TESTMODE_MASK); /* clear the test mode */
    reg_val |= (ushort)(test_mode << ONEK_CNTL_TESTMODE_SHIFT);

    if (plug_tc_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to set test mode(%d).\n",
	       __FUNCTION__, test_mode);
	return(FAILED);
    }

    /* Recover the page */
    reg_val = (ushort)PHY_PAGE(0);
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_PAGE_ADDR_REG;
    if (plug_tc_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to recover the page(%d).\n",
	       __FUNCTION__, reg_val);
	return(FAILED);
    }

    if (test_mode == OCR_TESTMODE_NORMAL) {
        reg_val = 0;
        reg_page = (int)PHY_PAGE(0);
        reg_addr = (int)MRVL1112_COP_CTRL_REG;
        if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read Reg. %d_%d\n",
                   __FUNCTION__, reg_page, reg_addr);
	    return(FAILED);
        }

        reg_val |= (ushort)COP_CTRL_RESET;
        if (plug_tc_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	    printf("%s: Failed to apply a soft reset.\n", __FUNCTION__);
	    return(FAILED);
        }

        for (ctr = 0; ctr < GEPHY_MAX_RETRY; ctr++) {
            msleep(10);

            reg_val = 0;
            if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr,
                                 &reg_val) != PASSED) {
                printf("%s:%d Failed to read Reg. %d_%d\n",
                       __FUNCTION__, __LINE__, reg_page, reg_addr);
	        return(FAILED);
            }

            if ((reg_val & (ushort)COP_CTRL_RESET) == 0) {
                break;
            }

            if (ctr == (GEPHY_MAX_RETRY - 1)) {
                printf("%s: Time out but PHY still in soft reset process.\n",
                       __FUNCTION__);
	        return(FAILED);
            }
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_gephy_set_txtype_util
 * Description: Utility to set testcard GE WAN PHY Transmitter Type.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_gephy_set_txtype_util (int eth_num)
{
    int    reg_page = (int)REG_PAGE(0); 
    int    reg_addr = (int)MRVL88E1112_CSC_REG2;
    ushort reg_val = 0, tx_type = 0;

    /* To get the current Transmitter Type */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_CSC_REG2;
    if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get current Transmitter Type(Reg. %d_%d)\n",
               __FUNCTION__, reg_addr, reg_page);
    }
    tx_type = (ushort)((reg_val & (ushort)CSCR2_TXTYPE_MASK) >>
                       CSCR2_TXTYPE_OFFSET);

    /* To get user wanted Transmitter Type */
    tx_type = (ushort)gethex_answer("Enter TX Type(0: Class B; 1 - Class A): ",
                                    tx_type, 0, 1);

    if (plug_tc_set_gephy_txtype(eth_num, tx_type) != PASSED) {
        printf("Failed to set GE WAN PHY(eth%d) Transmitter Type.\n", eth_num);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_set_gephy_txtype
 * Description: Function to set GE WAN PHY Transmitter Type.
 * Inputs     : eth_num - ethernet number
 *              tx_type - transmitter type (0: Class B; 1: Class A)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_set_gephy_txtype (int eth_num, ushort tx_type)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)MRVL88E1112_CSC_REG2;
    ushort reg_val = 0, set_type = 0;

    if (tx_type > GEWAN_TXTYPE_A) {
        printf("Plug test card GE WAN PHY(MRVL88E1112) NOT support this type(%d).\n",
               tx_type);
        return (FAILED);
    }

    set_type = (ushort)(tx_type << CSCR2_TXTYPE_OFFSET);

    /* To get the current Transmitter Type */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_CSC_REG2;
    if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get current Transmitter Type(Reg. %d_%d)\n",
               __FUNCTION__, reg_addr, reg_page);
    }

    /* Confirm if Transmitter Type needs to be changed */
    if ((reg_val & set_type) == set_type) {
        return (PASSED);
    }

    reg_val &= (ushort)(~CSCR2_TXTYPE_MASK);
    reg_val |= set_type;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: set_type = 0x%04X; reg_val = 0x%04X.\n",
               __FUNCTION__, set_type, reg_val);
    }

    if (plug_tc_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to set GE WAN PHY(eth%d) Transmitter Type.\n",
	       __FUNCTION__, eth_num);
	return(FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_gephy_reg_rd_util 
 * Description: Utility to read Plug test card GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_gephy_reg_rd_util (int eth_num)
{
    int      reg_page = 0, reg_addr = 0;
    uint16_t reg_val = 0;

    reg_page = getdec_answer("Enter page offset(0 ~ 255): ", 0, 0, 255);
    reg_addr = getdec_answer("Enter register offset(0 ~ 31): ", 0, 0, 31);

    if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("eth%d page%d reg%d: 0x%04X.\n",
                eth_num, reg_page, reg_addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_gephy_reg_wr_util
 * Description: Utility to write plug test card GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_gephy_reg_wr_util (int eth_num)
{
    int      reg_page = 0, reg_addr = 0;
    uint16_t reg_val = 0, w_data = 0;

    reg_page = getdec_answer("Enter page offset(0 ~ 255): ", 0, 0, 255);
    reg_addr = getdec_answer("Enter register offset(0 ~ 31): ", 0, 0, 31);

    if (plug_tc_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }

    w_data = gethex_answer("Enter write in data(0x0 ~ 0xfffff): ",
                            reg_val, 0, 0xffff);

    if (plug_tc_gephy_reg_wr(eth_num, reg_page, reg_addr, w_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%04X to eth%d page%d reg%d.\n",
                w_data, eth_num, reg_page, reg_addr);
    }
    return (PASSED);
}




/******************************************************************************
 *
 * Function   : plug_testcard_sgmii_loopback_test 
 * Description: Entrance of plug GE PHY(88E1112) Diag tests.
 * Inputs     : show_menu - menu option
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************/
int plug_testcard_sgmii_loopback_test (int show_menu)
{

    if (!plug_tc_host_sgmii_present(show_menu)) {
        printf("Host does NOT has SGMII connect to PIM GE PHY.\n");
        printf("Skip the SGMII loopback test...\n");
        return (PASSED);
    }

    build_primary_submenu(plug_tc_gephy_diag_tbl,
                          PLUG_TC_GEPHY_DIAG_TBL_SIZE,
                          "PLUG testcard GE PHY", &plug_tc_gephy_diag_menup);
    build_secondary_submenu(plug_tc_gephy_diag_tbl,
                            PLUG_TC_GEPHY_DIAG_TBL_SIZE,
                            plug_tc_gephy_diag_sec_items);

   
    if (show_menu) {
        menu(&plug_tc_gephy_diag_menu, plug_tc_gephy_diag_sec_items, '\0' );

    } else {
        menu_exec_doall_diags(plug_tc_gephy_diag_menup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plug_tc_gephy_utils
 * Description : Entry point of Plug testcard GE PHY0 utilities.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static int plug_tc_gephy_utils (int opt)
{
    build_primary_submenu(plug_tc_gephy_util_items, PLUG_TC_GEPHY_UTIL_SIZE,
                          "PLUG TC GE PHY Utilities", &plug_tc_gephy_util_menup);
    build_secondary_submenu(plug_tc_gephy_util_items, PLUG_TC_GEPHY_UTIL_SIZE,
                            plug_tc_gephy_util_sec_items);

    menu(plug_tc_gephy_util_menup, plug_tc_gephy_util_sec_items, '\0' );

    return (PASSED);
}

/*******************************************************************************
 *  
 * Function   : plug_tc_gephy_reg_test
 * Description: Function performs Testcard GE PHY(Marvell 88E1112) register test.
 * Inputs     : eth_num - Ethernet number of PHY
 * Outputs    : PASSED / FAILED
 *         
 *******************************************************************************
 */
static int plug_tc_gephy_reg_test (int eth_num)
{

    ushort orig_val = 0, test_pattern = (ushort)REG_PAGE(22);
    ushort read_back = 0;
    char tname[32]; 

    memset(tname, 0, sizeof(tname));

    sprintf(tname, "GE0 PHY register");

    testname(tname);
    prpass(testpass, "%s, ", tname);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        cterr('f', 0, "test - show enhanced error messages");
    }

    if (plug_tc_i2cphy_reg_rd((int)REG_PAGE(22), &orig_val) != PASSED) {
        cterr('f', 0, "Failed to read original value of page reg.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    if (plug_tc_i2cphy_reg_wr((int)REG_PAGE(22), test_pattern) != PASSED) {
        cterr('f', 0, "Failed to write test pattern to page reg.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if (plug_tc_i2cphy_reg_rd((int)REG_PAGE(22), &read_back) != PASSED) {
        cterr('f', 0, "Failed to read page reg.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    if (read_back != test_pattern) {
        cterr('f', 0, "Data mismatched: test_pattern %#x; read_back %#x.\n"
                      "Failed to do page reg.",
                      test_pattern, read_back);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    if (plug_tc_i2cphy_reg_wr((int)REG_PAGE(22), orig_val) != PASSED) {
        cterr('f', 0, "Failed to restore original value of page reg.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}



/*******************************************************************************
 *    
 * Function   : plug_tc_gephy_copper_mac_lpbk_test
 * Description: Function to do PLUG testcard GE PHY MAC internal loopback test.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int plug_tc_gephy_copper_mac_lpbk_test (int ge_num)
{
    struct plug_intf_t *plug;
    int      eth_num = 0;
    int      speed_ctr = 0, speed_tbl_size = 0, test_speed = SPD_1000MBPS;
    int      r_page = 0, r_addr = 0;
    uint16_t w_data = 0;
    char     test_name[32];

    memset(test_name, 0, sizeof(test_name));

    /* Set test name */
    testname(GEPHY_MAC_LPBK_TEST);

    speed_tbl_size = sizeof(gephy_copper_speed_tbl) / sizeof(int);

    /* 
     * Based on Platform Test card mapping with slot
     */
    plug = (struct plug_intf_t *)plug_test_if;
    plug_tc_host_reply_geport_ethnum(plug->slot, &eth_num);

    
    sprintf(test_name, "PLUG TC GE(eth%d) %s", eth_num, GEPHY_MAC_LPBK_TEST);
 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        cterr('f', 0, "test - show enhanced error messages");
    }

    for (speed_ctr = 0; speed_ctr < speed_tbl_size; speed_ctr++) {
        test_speed = gephy_copper_speed_tbl[speed_ctr];
        prpass(testpass, "Test GE at %dMbps ", test_speed);

        /* 1. Config. GE PHY MAC */
        r_page = (int)REG_PAGE(2);
        r_addr = (int)REG_ADDR(0);

        if (test_speed == SPD_10MBPS) {
            w_data = COP_SPD_10Mbps;
        } else if (test_speed == SPD_100MBPS) {
            w_data = COP_SPD_100Mbps;
        } else if (test_speed == SPD_1000MBPS) {
            w_data = COP_SPD_1000Mbps;
        } else {
            cterr('f', 0, "Failed at GE: Got unsupported Testspeed(%d) ",
                           test_speed);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
        
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: Write-in data to GE PHY reg. %d_%d: 0x%04X.\n",
                   __FUNCTION__, r_addr, r_page, w_data);
        }

        if (plug_tc_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
            cterr('f', 0, "Failed to config GE PHY MAC speed to %dMbps ",
                          test_speed);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        /* 2. Config. Platform GE MAC */
        if (plug_tc_host_gephy_set_test_speed(test_speed) != PASSED) {
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        /* 3. Config. Enable GE PHY MAC Loopback */
        r_page = (int)REG_PAGE(0);
        r_addr = (int)REG_ADDR(0);
        w_data = 0;

        if (plug_tc_gephy_reg_rd(eth_num, r_page, r_addr, &w_data) != PASSED) {
            cterr('f', 0, "Failed to read GE PHY reg. %d_%d ",
                          r_addr, r_page);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        w_data |= (uint16_t)COP_CTRL_LPBK;

        if (plug_tc_gephy_reg_wr(eth_num, r_page, r_addr, w_data) != PASSED) {
            cterr('f', 0, "Failed to enable GE PHY MAC Loopback ");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        /* 4. Run SGMII loopback test */
        if (plug_tc_sgmii_lpbk_test(eth_num, test_speed) != PASSED) {
            cterr('f', 0, "Failed at GE ");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }


    /* 5. Disable GE PHY MAC loopback */
    r_page = (int)REG_PAGE(0);
    r_addr = (int)REG_ADDR(0);

    w_data = (uint16_t)(COP_CTRL_AUTONEG |
                        COP_CTRL_DUPLEX_FULL |
                        COP_SPD_1000Mbps);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to GE PHY reg. %d_%d: 0x%04X.\n",
               __FUNCTION__, r_addr, r_page, w_data);
    }

    if (plug_tc_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s: Failed to set GE PHY MAC back to default.\n",
               __FUNCTION__);
        return (FAILED);
    }
   /* 6. Set GE PHY MAC back to default */
    r_page = (int)REG_PAGE(2);
    r_addr = (int)REG_ADDR(0);

    w_data = (uint16_t)(MCR_MAC_AN_EN | MCR_SPD_1000Mbps);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to GE PHY reg. %d_%d: 0x%04X.\n",
               __FUNCTION__, r_addr, r_page, w_data);
    }

    if (plug_tc_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s: Failed to set GE PHY MAC back to default.\n",
               __FUNCTION__);
        return (FAILED);
    }

    if (plug_tc_host_gephy_set_auto_neg() != PASSED) {
        cterr('f', 0, "Failed to set GE back to auto nego");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plug_tc_i2cphy_reg_wr
 * Description : Function implementation of 88E1112 I2C Write
 * Inputs      : eth_num - phy number
 *               reg_addr - register address
 *               w_data - write data
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_tc_i2cphy_reg_wr (int reg_addr, ushort w_data)
{

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write 0x%08X to Reg 0x%08X.\n", __FUNCTION__, 
                __LINE__, w_data, reg_addr);
    }

    w_data = dswap2(w_data); 


    /* Write data to Plug GE phy by I2C interace */
    if (plug_common_host_i2c_wr_2bytes(plug_curr_i2c_ctrl,PLUG_TC_I2C_ADDR_PHY,
                                reg_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write Plug GE phy I2C Reg.(0x%08X).\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_tc_i2cphy_reg_rd 
 * Description : Function implementation of 88E1112 I2C Read
 * Inputs      : eth_num - phy number
 *               reg_addr - register address
 *               buf - point of read buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_tc_i2cphy_reg_rd (int reg_addr, ushort *buf)
{

    /* Read data from Plug GE phy by I2C interface */
    if (plug_common_host_i2c_rd_2bytes(plug_curr_i2c_ctrl, PLUG_TC_I2C_ADDR_PHY,
                                reg_addr, buf) != PASSED) {
        printf("%s:%d Failed to read Plug GE phy I2C Reg.(0x%08X).\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }

    *buf = dswap2((short)(*buf & 0xffff));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d read 0x%08X from Reg 0x%08X.\n", __FUNCTION__, 
                __LINE__, *buf, reg_addr);
    }

    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : plug_tc_ge_phy_1000x_ext_lpbk_test
 * Description: Function to do Plug GE PHY 1000base-X external loopback test.
 * Inputs     : ge_num - phy number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int plug_tc_ge_phy_1000x_ext_lpbk_test (int ge_num)
{

    char tname[32];

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "GE1 1000BASE-X Ext. loopback");

    testname(tname);
    
    if (plug_tc_gephy_set_1000x_default() != PASSED) {
        cterr('f', 0, "Failed to set GE%d to 1000BASE-X default ", ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* Check if Ext. Loopback Flag is ON */
    if (plug_tc_host_check_ext_lpbk_flag() != TRUE) {
        printf("Skip %s test beacuse Ext. Loopback Flag is OFF.\n", tname);
        return (PASSED);
    }

    prpass(testpass, "%s, ", tname);

    if (plug_tc_ge_1000x_lpbk_test(E_1000BASEX_INT_EXT_LPBK) != PASSED) {
        cterr('f', 0, "Failed to test plug testcard 1000x loopback test");
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *    
 * Function   : plug_tc_gephy_set_1000x_default
 * Description: Function to set GE PHY to 1000base-x default.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int plug_tc_gephy_set_1000x_default (void)
{
    struct plug_intf_t *plug;
    int      r_page = 0, r_addr = 0;
    uint16_t w_data = 0;
    int      eth_num = 0;

    /* 
     * Based on Platform Test card mapping with slot
     */
    plug = (struct plug_intf_t *)plug_test_if;
    plug_tc_host_reply_geport_ethnum(plug->slot, &eth_num);

    /* 1. Disable GE PHY MAC loopback */
    r_page = (int)REG_PAGE(0);
    r_addr = (int)REG_ADDR(0);

    w_data = (uint16_t)(COP_CTRL_AUTONEG |
                        COP_CTRL_DUPLEX_FULL |
                        COP_SPD_1000Mbps);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to GE PHY reg. %d_%d: 0x%04X.\n",
                __FUNCTION__, r_addr, r_page, w_data);
    }

    if (plug_tc_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s: Failed to set GE PHY MAC back to default.\n",
                __FUNCTION__);
        return (FAILED);
    }

    /* 2. Set GE PHY MAC back to default */
    r_page = (int)REG_PAGE(2);
    r_addr = (int)REG_ADDR(0);

    w_data = (uint16_t)(MCR_SPD_1000Mbps);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to GE PHY reg. %d_%d: 0x%04X.\n",
                __FUNCTION__,  r_addr, r_page, w_data);
    }

    if (plug_tc_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s: Failed to set GE PHY MAC back to default.\n",
                __FUNCTION__);
        return (FAILED);
    }

    /* 3. Set CPU GE MAC to 1000 speed */
    if (plug_tc_host_gephy_set_1000_speed() != PASSED) {
        printf("%s: Failed to set GE PHY MAC back to 1000.\n",
                __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_ge_1000x_lpbk_test
 * Description: Pluggable test card external 1000base-X loopback test
 * Inputs     : lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 * Outputs    : pass/fail
 *
 *******************************************************************************
 */
static int plug_tc_ge_1000x_lpbk_test (int lpbk_mode)
{

    struct plug_intf_t *plug;
    int rc = 0; 
    int retval = PASSED; 
    int try, retry_limit = 2;
    int test_port, eth_num;
    int test_speed;

    /* 
     * Based on Platform Test card mapping with slot
     */
    plug = (struct plug_intf_t *)plug_test_if;
    plug_tc_host_reply_geport_ethnum(plug->slot, &eth_num);

    test_port = eth_num;
    test_speed = SPD_1000MBPS;
        
    switch(lpbk_mode) {
        case E_1000BASEX_INT_EXT_LPBK:
             /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
              *    if failed, perform Internal loopback test.
              * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
              */
             testname("1000Base-X External Loopback");

             prpass(testpass, "Test GE(eth%d) speed-%d ", eth_num, test_speed);

             for (try = 0; try < retry_limit; try++) {
                 rc = plug_tc_phy_ext_lpbk_test(test_port, test_speed, E_1000BASEX_INT_EXT_LPBK);

                 if ((rc == PASSED) || (try == (retry_limit - 1))) {
                        break;
                 } else {
                        printf("####### retry the test #########\n");
                        break;
                 }
             }

             if (rc != PASSED) {
                    cterr('f', 0, "1000Base-X Failed at GE ");
                    retval = FAILED;
             }
             break;
        default:
            retval = FAILED;
            cterr('f',0," Invalid Loopback mode(0x%08X).\n", lpbk_mode);
            break;
    }

    /* Reset PHY */
    if (plug_tc_reset_plug_eth_phy(eth_num) != PASSED) {
        cterr('f', 0, "Failed to reset eth%d.", eth_num);
        retval = FAILED;
    }

#if DEBUG
    printf("*******End*******\n");
    system("date"); /* real time counter */
    printf("*****************\n");
#endif

    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}


/*******************************************************************************
 *
 * Function   : plug_tc_gephy_config_w_rst 
 * Description: Function to config plug testcard GE PHY(Marvell 88E1112) with PHY reset.
 * Inputs     : eth_num - ethernet number
 *              r_page  - page number of register
 *              r_addr  - offset of wanted register
 *              w_data  - buffer to put value that will be written in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_gephy_config_w_rst (int eth_num, int r_page, int r_addr, ushort w_data)
{
    ushort reg_val = 0;
    int    t_ctr_ms = 0;

    w_data |= (ushort)GEPHY_REG_RESET;
    if (plug_tc_gephy_reg_wr(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write eth%d PHY reg. %d_%d.\n",
                __FUNCTION__, __LINE__, eth_num, r_addr, r_page);
        return (FAILED);
    }

    do {
        reg_val = (ushort)GEPHY_REG_RESET;
        if (plug_tc_gephy_reg_rd(eth_num, r_page, r_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read eth%d PHY reg. %d_%d.\n",
                    __FUNCTION__, __LINE__, eth_num, r_addr, r_page);
            return (FAILED);
        }

        if ((reg_val & (ushort)GEPHY_REG_RESET) == 0) {
            break;
        }
        t_ctr_ms += 10;
    } while(t_ctr_ms <= PLUG_TC_GEPHY_CONFIG_TIME);

    if (t_ctr_ms > PLUG_TC_GEPHY_CONFIG_TIME) {
        return (FAILED);
    }
    msleep(100);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_gephy_reg_rd
 * Description: Function to read plug testcard GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num  - ethernet number(eth0, eth1, eth2,...)
 *              reg_page - page number of register
 *              reg_addr - offset of wanted register
 *              *buf     - buffer to put read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_gephy_reg_rd (int eth_num, int reg_page, int reg_addr, ushort *buf)
{
    ushort reg_val = 0;
    int    page_reg = (int)REG_PAGE(22);

    /* RW Phy by I2C bus */
    /* Read out current value of page address register(0x16) */
    if (plug_tc_i2cphy_reg_rd(page_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to I2C read eth%d reg.0x%08X.\n",
                __FUNCTION__, __LINE__, eth_num, page_reg);
        return (FAILED);
    }

    if (reg_val != (ushort)reg_page) {
        /* Set page to user requested */
        if (plug_tc_i2cphy_reg_wr(page_reg, (ushort)reg_page) != PASSED) {
            printf("%s:%d Failed to I2C write eth%d reg.0x%08X.\n",
                    __FUNCTION__, __LINE__, eth_num, page_reg);
            return (FAILED);
        }

        reg_val = 0;
        if (plug_tc_i2cphy_reg_rd(page_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to I2C read eth%d reg.0x%08X.\n",
                    __FUNCTION__, __LINE__, eth_num, page_reg);
            return (FAILED);
        }

        if (reg_val != (ushort)reg_page) {
            printf("%s:%d Failed to set eth%d to page 0x%02X(%d).\n",
                    __FUNCTION__, __LINE__, eth_num, reg_page, reg_page);
            return (FAILED);
        }
    }

    /* Read Data for user */
    if (plug_tc_i2cphy_reg_rd(reg_addr, buf) != PASSED) {
        printf("%s:%d Failed to I2C read eth%d reg.0x%08X.\n",
                __FUNCTION__, __LINE__, eth_num, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_gephy_reg_wr
 * Description: Function to write plug testcard  GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num  - ethernet number(eth0, eth1, eth2,...)
 *              reg_page - page number of register
 *              reg_addr - offset of wanted register
 *              wr_data  - new value thatwanted to write into register
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_gephy_reg_wr (int eth_num, int reg_page, int reg_addr, ushort w_data)
{
    ushort reg_val = 0;
    int    page_reg = (int)REG_PAGE(22);

    /* Read out current value of page address register(0x16) */
    if (plug_tc_i2cphy_reg_rd(page_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to I2C read eth%d reg.0x%08X.\n",
                __FUNCTION__, __LINE__, eth_num, page_reg);
        return (FAILED);
    }

    if (reg_val != (ushort)reg_page) {
        /* Set page to user requested */
        if (plug_tc_i2cphy_reg_wr(page_reg, (ushort)reg_page) != PASSED) {
            printf("%s:%d Failed to I2C write eth%d reg.0x%08X.\n",
                    __FUNCTION__, __LINE__, eth_num, page_reg);
            return (FAILED);
        }

        reg_val = 0;
        if (plug_tc_i2cphy_reg_rd(page_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to I2C read eth%d reg.0x%08X.\n",
                    __FUNCTION__, __LINE__, eth_num, page_reg);
            return (FAILED);
        }

        if (reg_val != (ushort)reg_page) {
            printf("%s:%d Failed to set eth%d to page 0x%02X(%d).\n",
                    __FUNCTION__, __LINE__, eth_num, reg_page, reg_page);
            return (FAILED);
        }
    }

    if (plug_tc_i2cphy_reg_wr(reg_addr, w_data) != PASSED) {
        printf("%s:%d Failed to I2C write eth%d reg.0x%08X.\n",
                __FUNCTION__, __LINE__, eth_num, reg_addr);
        return (FAILED);
    }
    
    msleep(100);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_phy_ext_lpbk_test
 * Description: This is the entry point for plug testcard external loopback test only.
 * Inputs     : Port = test ethernet port
 *              speed = test speed 
 *              lpbkmode = SGMII mode/E_1000BASAE mode 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_phy_ext_lpbk_test (int port, int speed, int lpbkmode)
{
    switch(lpbkmode) {
        case E_1000BASEX_INT_EXT_LPBK:    
            /* setup loopback information */          
            if (plug_tc_set_phy_1000x_ext_lpbk(port, speed) != PASSED) {
                printf("%s: Failed to set loopback mode of port%d.\n",
                        __FUNCTION__, port);
                return (FAILED);
            }
            plug_tc_set_media_phy_reg_checks(port); 
            /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
            if (plug_tc_sgmii_lpbk_test(port, speed) != PASSED) {
                printf("%s: plug_tc_sgmii_lpbk_test failed\n", __FUNCTION__);
                return (FAILED);
            }
            return (PASSED);
        default:
            cterr('f',0," Invalid Loopback mode(0x%08X).\n", lpbkmode);
            return (FAILED);
    }
}



/*******************************************************************************
 *
 * Function   : plug_tc_set_phy_1000x_ext_lpbk
 * Description: initial and setup plug testcard loopback type on 1000base-X for external lpbk 
 * Inputs     : port - port number
 *              lpbk_typ - internal or external
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_set_phy_1000x_ext_lpbk (int port, int speed)
{
    struct plug_intf_t *plug;
    char eth_ifname[10];
    char pname[10];
    int rc = 0;
    char cmd_str[32];

    /* 
     * Based on Platform Test card mapping with slot
     */
    plug = (struct plug_intf_t *)plug_test_if;
    plug_tc_host_get_eth_interface_info(plug->slot, eth_ifname);
   
    sprintf(pname,"%s%d", eth_ifname, port); 

    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* init environment and set speed for loopback */
    if ((rc = plug_tc_init_1000x_env(pname, speed, port, EXT_LPBK)) != PASSED){
        printf("%s(): %s plug_tc_init_1000x_env failed.\n", __FUNCTION__, pname);
        return (FAILED);
    }

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* 1GMbps external loopback need to setup*/
    if (speed == SPD_1000MBPS) {
        if (plug_tc_set_phy_stub(port, EXT_LPBK, SIG_COPPER) != PASSED){
            printf("%s(): %s set_phy_stub failed.\n", __FUNCTION__, pname);
            return (FAILED);
        }
    }

    if ((rc = plug_tc_phy_soft_reset(port, SIG_COPPER)) != PASSED){
        printf("%s(): %s plug_tc_phy_soft_reset failed\n", __FUNCTION__, pname);
        return (FAILED);
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY * 3);

    /* internal loopback using force_linkup to ensure link stable,
     * external loopback can not use force_linkup, so using check link
     * to ensure the link is stable
     */
    rc = plug_tc_check_link_status(port);
    if ((rc == FAILED)) {
        printf("%s(): %s 1000base-x link up time out\n", __FUNCTION__, pname);
        return FAILED;
    } 

    return (PASSED);
}


/*******************************************************************************
 *
 * Functioni  : plug_tc_init_1000x_env
 * Description: Function to init plug testcard 1000x port.
 *              Link up port, ensure power up,
 *              and turn off other power then set speed.
 * Inputs     : pname - port 
 *              speed: current test speed   
 *              port - port
 *              lpbk_mode - loopback mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_init_1000x_env (char *pname, int speed, int port, int lpbk_mode)
{    
    int autoneg = 0;

    /* Ensure turn off the fiber and turn on copper before test */	
    if (plug_tc_sig_pwr_ctrl(port, DISABLE_SIG, SIG_FIBER) != PASSED) {  
    	printf("Failed to disable FIBER.\n"); 
    	return (FAILED);
    }
    if (plug_tc_sig_pwr_ctrl(port, ENABLE_SIG, SIG_COPPER) != PASSED) {
    	printf("Failed to enable COPPER.\n"); 
    	return (FAILED);
    }

    /* Always init as 10Mbps, duplex full, autoneg off
     * to make it more stable for 1000MPB initialation.
     */
    if (plug_tc_sig_set_port_speed(port, SPD_1000MBPS, SIG_COPPER) != PASSED) {
        printf("%s: Failed to set port speed %d.\n",
                __FUNCTION__, speed);
        return (FAILED);
    }


    /* internal loopback using force_linkup to ensure link stable,
     * external loopback can not use force_linkup, so using check link
     * to ensure the link is stable
     * need to verify this one is necessary or not.
     */

    /* To ensure the test stay on full duplex and set speed */
    autoneg = AUTONEG_ON;

    if (plug_tc_cfg_phy_setting(port, speed, FULL_DUPLEX,
                        autoneg, SIG_COPPER) != PASSED) {
        printf("%s: %s cfg_phy_setting failed speed is %d\n",
                __FUNCTION__, pname, speed);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_sig_pwr_ctrl
 * Description: Enable the power of plug testcard PHY.
 * Inputs     : port - port number 
 *              enable - enable/disable the power up/down register 
 *              signal - select page for copper/fiber
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_sig_pwr_ctrl (int port, boolean enable, boolean signal)
{
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;

    /* Change page based on the signal(Copper/Fiber) */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    /* RW Phy by I2C bus */
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Set SGMII fiber ouput amplitude */
    if ((signal == SIG_FIBER) && (enable == ENABLE_SIG)) {
        reg_addr = (int)FIB_SPEC_CTRL_REG2;
        reg_val = 0;
        if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read Reg%d.\n",
                    __FUNCTION__, __LINE__, reg_addr);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
        }
    
        wr_data = ((reg_val & (ushort)(~FIB_OUTPUT_AMP_MSK)) |
                   (ushort)FIB_OUTPUT_AMP_VAL504);   
        reg_addr = (int)FIB_SPEC_CTRL_REG2;
        if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
            printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                    __FUNCTION__, __LINE__, wr_data);
            return (FAILED);
        }
    }

    reg_addr = (int)PHY_REG(0);   /* reg 0 */
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }
    
    if (enable) {
        wr_data = (reg_val & (ushort)(~PHY_REG_BIT(11))); /* power up */
    } else {
        wr_data = (reg_val | (ushort)PHY_REG_BIT(11));  /* power down */
    } 
    
    /* we do not need to setup the same value */
    if (wr_data == reg_val) {
        return (PASSED);
    }
    
    reg_addr = (int)PHY_REG(0);
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Read back to make sure the write is complete */
    reg_addr = (int)PHY_REG(0);   /* reg 0 */
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if (reg_val != wr_data) {
        printf("%s: Failed because of data mismatch.\n"
               "read back = 0x%04X; but write in = 0x%04X.\n",
                __FUNCTION__, reg_val, wr_data);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_sig_set_port_speed
 * Description: Function to set plug testcard port speed.
 * Inputs     : port - port number
 *              speed - port speed
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_sig_set_port_speed (int port, int speed, boolean signal)
{
    int get_speed;
   
     switch(speed) {
         case SPD_10MBPS:
             get_speed = 0x0;
         break;
         case SPD_100MBPS:
             get_speed = 0x2000;
         break;
         case SPD_1000MBPS:
             get_speed = 0x0040;
         break;   
         default:
             printf("%s: eth%d not support this speed %d\n",
                     __FUNCTION__, port, speed);
             return (FAILED);
         break;
    }
    
    if (plug_tc_sig_set_speed(port, get_speed, signal)) {
        printf("%s: eth%d set Speed %d failed.\n",
                __FUNCTION__, port, get_speed);
        return (FAILED);
    }

    if (plug_tc_set_promisc(port) != PASSED) {
        printf("%s: Failed to set eth%d promisc mode.\n", __FUNCTION__, port);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_sig_set_speed
 * Description: Function to set init speed, autoneg, full duplex full
 *              via ioctl on ethtool.
 * Inputs     : device - device name (ex: "eth1" )
 *              sock - raw socket
 *              speed - select speed
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_sig_set_speed (int port, int speed, boolean signal)
{
    int    reg_addr = 0;
    ushort wr_data = 0;

    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    /* RW Phy by I2C bus */
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }

    wr_data = (((ushort)speed) |
               ((ushort)COP_CTRL_DUPLEX_FULL) |
               ((ushort)COP_CTRL_RESET));

    reg_addr = (int)PHY_REG(0);
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }
    msleep(1000);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_set_promisc
 * Description: Function to set plug testcard promisc mode.
 *              when program exit, this interface will still be promisc mode.
 *              we should disable promisc mode when we exit (ie, use atexit)
 * Inputs     : device
 *              sock
 * Outputs    : PASSED/FAILED
 *
 * Note: Will get the haft of packet from TX if failed to set promisc.
 *
 *******************************************************************************
 */
static int plug_tc_set_promisc (int eth_num)
{
    char cmd[128];

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "ifconfig eth%d promisc", eth_num);
    system(cmd);

    return (PASSED);
}



/*******************************************************************************
 *
 * Function   : plug_tc_cfg_phy_setting
 * Description: Function to config plug testcard PHY register for speed directly,
 *              and switch page to let driver detect the setting.
 *              This function is not like set_port_speed() which is based on 
 *              ethtool and may let other Reg reset.
 * Inputs     : ifname - port name.
 *              speed - setup speed 
 *              duplex - turn full/half duplex
 *              autoneg - turn on/off autoneg        
 *              signal - signal
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_cfg_phy_setting (int port, int speed, int duplex,
                     int autoneg, boolean signal)
{
    int    reg_addr = 0;
    ushort wr_data = 0, reg_val = 0, spdset = 0;

    /* select page 0 or page 1 from signal */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    
    /* RW Phy by I2C bus */
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Read PHY control reg for current speed*/
    /* set speed, Reg [0_2.6, 0_2.13] = value */
    reg_addr = (int)PHY_REG(0);
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    switch(speed) {
    case SPD_10MBPS:
        spdset = 0x0000;
        break;
    case SPD_100MBPS:
        spdset = 0x2000;
        break;
    case SPD_1000MBPS:
        spdset = 0x0040;
        break;
    case 0:  /* use the same speed */
        spdset = reg_val & (ushort)0x2040;
        break;
    }

    /* Set speed bits */
    wr_data = reg_val & (ushort)(~0x2040); /* clear bit 6 and 13 (speed)  */
    wr_data |= spdset;
    
    /* Set duplex mode */
    if (duplex) {
        wr_data |= (ushort)0x100; /* full duplex */
    } else {
        wr_data &= (ushort)(~0x100); /* half duplex */
    }

    /* Set autoneg on or off */
    if (autoneg) {
        wr_data |= (ushort)0x1000; /* enable autoneg */
    } else {
        wr_data &= (ushort)(~0x1000); /* disable autoneg */
    }

    /* Write to the phy and read back immediate to make sure. */
    reg_addr = (int)PHY_REG(0);
    /* RW Phy by I2C bus */
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(0);
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if (wr_data != reg_val) {
        cterr('f', 0, "%s(): register setup failed. wrval = 0x%x rdval = 0x%x",
              __FUNCTION__, wr_data, reg_val);

        sleep(1);
        return (FAILED);
    }

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    /* RW Phy by I2C bus */
    if (speed != 0) {

        reg_addr = (int)PHY_REG(22);
        if(signal) {
            if (plug_tc_i2cphy_reg_wr(reg_addr, (ushort)SIG_COPPER) != PASSED) {
                printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                        __FUNCTION__, __LINE__, wr_data);
                return (FAILED);
            }

            sleep(ETH_DRIVER_DELAY); /* link down here */

            if (plug_tc_i2cphy_reg_wr(reg_addr, (ushort)SIG_FIBER) != PASSED) {
                printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                        __FUNCTION__, __LINE__, wr_data);
                return (FAILED);
            }

        } else {
            if (plug_tc_i2cphy_reg_wr(reg_addr, (ushort)SIG_FIBER) != PASSED) {
                printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                        __FUNCTION__, __LINE__, wr_data);
                return (FAILED);
            }

            sleep(ETH_DRIVER_DELAY); /* link down here */

            if (plug_tc_i2cphy_reg_wr(reg_addr, (ushort)SIG_COPPER) != PASSED) {
                printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                        __FUNCTION__, __LINE__, wr_data);
                return (FAILED);
            }

        }
        sleep(ETH_DRIVER_DELAY); /* link up here */ 
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_set_phy_stub
 * Description: Function to enable plug testcard stub for external loopback test.
 * Inputs     : port - port number 
 *              enable - enable/disable the Enable stub register 
 *              signal - Copper or Fiber
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_set_phy_stub (int port, boolean enable, boolean signal)
{
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;

    /* Based on Marvell88E1112 spec,
     * for 1000BASE-T mode, 16_6:5 must be set to 1
     * to enable the external loopback.
     */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)PHY_PAGE(6);
    /* RW Phy by I2C bus */
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Get current value of 16_6 */
    reg_addr = (int)PHY_REG(16);
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
                __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if ((enable == EXT_LPBK) && (signal == SIG_COPPER)) {
        wr_data = (reg_val | (ushort)(PHY_REG_BIT(5)));
    } else {
        wr_data = (reg_val & (ushort)(~PHY_REG_BIT(5)));
    }
 
    reg_addr = (int)PHY_REG(16);
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);  /* can not mask, effect 1000 external lpbk test */

    reg_val = 0;
    reg_addr = (int)PHY_REG(16);
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if(reg_val != wr_data) {
        printf("%s: Failed because of data mismatch.\n"
               "read back = 0x%04X; but write in = 0x%04X\n",
               __FUNCTION__, reg_val, wr_data);
        return (FAILED);
    } 
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_phy_soft_reset
 * Description: Enable plug testcard PHY reset, add swtich page flow.
 * Inputs     : port - port number 
 *              signal - signal 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_phy_soft_reset (int port, boolean signal)
{
    int    reg_addr = 0;
    ushort wr_data = 0, reg_val = 0;
    int    repeat = 100;

    /* Reset reg 0_0.15=1 */
    /* Use signal to select page for copper or fiber */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    
    /* RW Phy by I2C bus */
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
               __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(0);
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
     }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    wr_data = reg_val | (ushort)PHY_REG_BIT(15);
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
               __FUNCTION__, __LINE__, wr_data);
        return (FAILED);
    }

    /* switch the page is needed*/
    reg_addr = PHY_REG(22);
    if (signal) {
        wr_data = (ushort)SIG_COPPER;
        if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
            printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                   __FUNCTION__, __LINE__, wr_data);
            return (FAILED);
        }

        sleep(ETH_DRIVER_DELAY); /* link down here */

        wr_data = (ushort)SIG_FIBER;
        if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
            printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                   __FUNCTION__, __LINE__, wr_data);
            return (FAILED);
        }

    } else {
        wr_data = (ushort)SIG_FIBER;
         if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
            printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                   __FUNCTION__, __LINE__, wr_data);
            return (FAILED);
        }

        sleep(ETH_DRIVER_DELAY); /* link down here */
        wr_data = (ushort)SIG_COPPER;
        if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
            printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                   __FUNCTION__, __LINE__, wr_data);
            return (FAILED);
        }
     }
    sleep(ETH_DRIVER_DELAY); /* link up here */ 

    /* Read back to check for reset done */
    do {
        msleep(10);
        reg_addr = (int)PHY_REG(0);
        if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
            return (FAILED);
         }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
        }

    } while((repeat-- > 0) && (reg_val & (ushort)PHY_REG_BIT(15)));
    
    if ((repeat == 0) && (reg_val &  (ushort)PHY_REG_BIT(15))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/*******************************************************************************
 *
 * Function   : plug_tc_check_link_status
 * Description: Function to check linux up status from Linux information.
 * Inputs     : port - port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_check_link_status (int port)
{
    struct plug_intf_t *plug;
    char eth_ifname[10];
    int timeout_counter = 100, is_link = FALSE;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];

    /* 
     * Based on Platform Test card mapping with slot
     */
    plug = (struct plug_intf_t *)plug_test_if;
    plug_tc_host_get_eth_interface_info(plug->slot, eth_ifname);
    
    sprintf(pname,"%s%d", eth_ifname, port);

    while(1) {
        /* Get the interface information */
        if (getifaddrs(&if_list) < 0) {
            printf("%s(): %s Failed to get interface information: %s.\n",
                   __FUNCTION__, pname, strerror(errno));
            return (FAILED);
        }
        if (if_list == NULL) {
            printf("%s(): %s No network interfaces were found.\n",
                    __FUNCTION__, pname);
            return (FAILED);
        }

        for (if_info = if_list; if_info; if_info = if_info->ifa_next) {
            /* parse the port name */
            if (strncmp(if_info->ifa_name, pname, IFNAMSIZ) != 0) {
                continue;
            }

             /* printf("%s ", if_info->ifa_name); */

             flags = if_info->ifa_flags;
             if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
               /* printf("up\n"); */
                 fflush(stdout);
                 is_link = TRUE;
                 break;
             } else {
                 /* printf("down\n");  */
                 msleep(10);
                 timeout_counter--;
                 if (timeout_counter == 0) {
                     return (FAILED);
                 }
             }
             fflush(stdout);
        } /*for*/

        freeifaddrs(if_list);

        if (is_link == TRUE) {
            break;
        }
    } /*while */

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_set_media_phy_reg_checks
 * Description: Check PHY setting before testing
 * Inputs     : port
 * Outputs    : none
 *
 *******************************************************************************
 */
static int plug_tc_set_media_phy_reg_checks (int port)
{
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    
    /* Change page to Copper */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)0;
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
         printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                   __FUNCTION__, __LINE__, wr_data);
         return (FAILED);
    }

    /* Read ctrl/status/spec ctrl  reg */
    reg_addr = (int)COP_CTRL_REG0;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 0 Reg 0 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }
    
    reg_addr = (int)COP_STATUS_REG1;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 0 Reg 1 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }

    reg_addr = (int)COP_SPEC_CTRL_REG2;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 0 Reg 26 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* Change page to Fiber */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)1;
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
         printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                   __FUNCTION__, __LINE__, wr_data);
         return (FAILED);
    }

    /* Read ctrl/status/spec ctrl  reg */
    reg_addr = (int)FIB_CTRL_REG0;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 1 Reg 0 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }
    
    reg_addr = (int)FIB_STATUS_REG1;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 1 Reg 1 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }

    reg_addr = (int)FIB_SPEC_CTRL_REG2;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 1 Reg 26 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* Change page to MAC */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)2;
    if (plug_tc_i2cphy_reg_wr(reg_addr, wr_data) != PASSED) {
         printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
                __FUNCTION__, __LINE__, wr_data);
         return (FAILED);
    }

    /* Read ctrl//spec ctrl  reg */
    reg_addr = (int)MAC_CTRL_REG0;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 2 Reg 0 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }

    reg_addr = (int)MAC_SPEC_CTRL_REG2;
    reg_val = 0;
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Page 2 Reg 26 value = 0x%x.\n", __FUNCTION__, __LINE__, reg_val);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_sgmii_lpbk_test
 * Description: Function to do Plug testcard SGMII loopback test.
 * Inputs     : port: current test port   
 *              speed: current test speed   
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_sgmii_lpbk_test (int port, int speed)
{
    struct plug_intf_t *plug;
    char eth_ifname[10];
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0;
    uchar orig_hkpflag = hkeepflags;
    
    /* 
     * Based on Platform Test card mapping with slot
     */
    plug = (struct plug_intf_t *)plug_test_if;
    plug_tc_host_get_eth_interface_info(plug->slot, eth_ifname);
    
    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;
    
    for (typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = pktdata[typ_curr].send_count;
        pkt_len = pktdata[typ_curr].len;
        pkt_val = pktdata[typ_curr].val;
        hkeepflags |= pktdata[typ_curr].hkpflags;

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Test eth%d, speed-%d, pkt-cnt(%#x),"
                   " pkt-len(%#x), pkt-val(%#x)\n",
                   port, speed, pkt_cnt, pkt_len, pkt_val);
            fflush(stdout);                          
        }
                  
        /* To do the tx/rx loopback test */
        rc = plug_tc_host_tx_rx_diag(eth_ifname, port, speed, pkt_cnt, pkt_len, pkt_val);
       
        if (rc == FAILED) {
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                   __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            return (FAILED);
        }
    } /* typ_curr */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("eth%d speed %d PASSED.\n", port, speed);
        fflush(stdout);
    }
    
    hkeepflags = orig_hkpflag;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : plug_tc_reset_plug_eth_phy 
 * Description: plug testcard to reset eth phy by I2C
 * Inputs     : eth_num = GE PHY number  
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_reset_plug_eth_phy (int eth_num)
{
    int    reg_addr = 0;
    ushort w_data = 0, reg_val = 0;
    int    ctr = 0;

    /* Confirm PHY back to normal mode: */
    /* 1. Disable stub mode (16_6:5 = 0) */
    reg_addr = (int)PHY_REG(22);
    w_data = (ushort)PHY_PAGE(6);
    /* reset phy by I2C interface */
    if (plug_tc_i2cphy_reg_wr(reg_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
               __FUNCTION__, __LINE__, w_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(16);   /* 16_6 */
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    /* Disable STUB */
    w_data = (reg_val & (ushort)(~PG_P6R16_STUB_EN));

    reg_addr = (int)PHY_REG(16);
    if (plug_tc_i2cphy_reg_wr(reg_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
               __FUNCTION__, __LINE__, w_data);
        return (FAILED);
    }

    msleep(I2CPHY_REG_WRITE_DELAY);

    /* 2. Reset PHY */
    reg_addr = (int)PHY_REG(22);
    w_data = (ushort)PHY_PAGE(0);
    if (plug_tc_i2cphy_reg_wr(reg_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
               __FUNCTION__, __LINE__, w_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(0);   /* 0_0 */
    if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read Reg%d.\n",
               __FUNCTION__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    /* RESET PHY */
    w_data = (reg_val | (ushort)COP_CTRL_RESET);

    reg_addr = (int)PHY_REG(0);
    if (plug_tc_i2cphy_reg_wr(reg_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write PHY Page Reg.(%d).\n",
               __FUNCTION__, __LINE__, w_data);
        return (FAILED);
    }

    for (ctr = 0; ctr < PLUG_TC_ETH_MAX_RETRY; ctr++) {
        reg_val = 0x8000;
        if (plug_tc_i2cphy_reg_rd(reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read Reg%d.\n",
                   __FUNCTION__, __LINE__, reg_addr);
            return (FAILED);
         }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("[%d]Reg.%d = 0x%04X.\n", ctr, reg_addr, reg_val);
        }

        if ((reg_val & (ushort)COP_CTRL_RESET) == 0) {
            break;
        } else {
            if (ctr == (PLUG_TC_ETH_MAX_RETRY - 1)) {
                printf("%s: Failed to reset eth%d PHY.\n",
                      __FUNCTION__, eth_num);
                return (FAILED);
            } 
        }
        msleep(I2CPHY_REG_WRITE_DELAY);
    }
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: plug_testcard_phy.c,v $
 * Revision 1.4  2019/08/06 06:56:16  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.3  2018/11/23 09:10:40  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.2.62.2  2018/11/15 07:25:53  hondwang
 * fix wrong comment in title
 *
 * Revision 1.2.62.1  2018/10/15 06:50:50  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.2  2018/01/20 05:01:10  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.1.4.4  2017/11/07 09:44:26  hondwang
 * Change test card test with 1000base-X
 *
 * Revision 1.1.4.3  2017/08/15 14:08:38  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
 * add pluggable testcard for star-branch-c9xx
 *
 * Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
 * Reorganize Star Pluggable directory structure
 *
 * Revision 1.1.2.4  2017/06/18 07:52:27  hondwang
 * Modify all phy SMI bus function to support I2C bus
 *
 * Revision 1.1.2.1  2017/06/17 12:09:49  hondwang
 * Add test card phy testing function
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
