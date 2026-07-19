/* $Id: testcard_bcm57412.c,v 1.4 2020/12/29 03:10:51 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_bcm57412.c,v $
*-----------------------------------------------------------------------------
* testcard_bcm57412.c - Diags Test for SM test card BCM57412 10G NIC.
*
* Jan 2019, Leschen 
*
* Copyright (c) 2019 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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
#include "queryflags.h" /* for query user functions */  
#include "testcard_bcm57412.h"
#include "dash_fpga.h"
#include "cookie_4.h" 
#include "linux_pciutils.h"


#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int BCM57412_utility(int);
static void BCM57412_fw_mac_program(void);
int sm_bcm57412_internal_loopback_test(int);
int sm_bcm57412_external_loopback_test(int);
int sm_bcm57412_sfp_plus_i2c_test(int);
int sm_bcm57412_pcie_check_test(void); 
static int get_bcm57412_plat_port(int);
static void dump_sfp_eeprom(void);
extern char *strcasestr(char* , char*);
extern int is_bcm57412_sm;
extern int get_pcie_cap_struct_ptr(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_cap(uint32_t, uint16_t, int, uint);
extern int is_curie_1ru(void); 


/******************************************************************************
 *  List of Menu used for Broadcom BCM57412 10G Speed
 *****************************************************************************/
static submenu_xtable_t BCM57412_10G_tests_submenu_table[] = {
   {"SM BCM57412 Port1 Internal Loopback Test", (type_t(*)())sm_bcm57412_internal_loopback_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 Port2 Internal Loopback Test", (type_t(*)())sm_bcm57412_internal_loopback_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 Port1 SFP+ External Loopback Test", (type_t(*)())sm_bcm57412_external_loopback_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 Port2 SFP+ External Loopback Test", (type_t(*)())sm_bcm57412_external_loopback_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 Port1 SFP+ i2c Test", (type_t(*)())sm_bcm57412_sfp_plus_i2c_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 Port2 SFP+ i2c Test", (type_t(*)())sm_bcm57412_sfp_plus_i2c_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 PCIe info Scan Test", (type_t(*)())sm_bcm57412_pcie_check_test,0,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SM BCM57412 Utility", (type_t(*)())BCM57412_utility,   FALSE,
    0, NULL, 0, (type_t(*)())BCM57412_utility,   TRUE},
};

/******************************************************************************
 *  List of Utilities used for BCM57412
 *****************************************************************************/
static submenu_xtable_t BCM57412_util_items[] = {
    {"BCM57412 Firmware Download", (type_t(*)())BCM57412_fw_mac_program, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump External Module EEPROM", (type_t(*)())dump_sfp_eeprom, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM57412_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM57412_10G_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))
#define BCM57412_TESTS_UTIL_SIZE (sizeof(BCM57412_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM57412_tests_primary_items[BCM57412_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t BCM57412_tests_secondary_items[BCM57412_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM57412_tests_primary_util_items[BCM57412_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t BCM57412_tests_secondary_util_items[BCM57412_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * BCM57412 Utils submenu
 *****************************************************************************/
menuinfo_t SM_BCM57412_util_menu = {
    "SM BCM57412 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    BCM57412_tests_primary_util_items,
};
menuinfo_t *SM_BCM57412_util_menup = &SM_BCM57412_util_menu;

menuinfo_t SM_BCM57412_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    BCM57412_tests_primary_items,
};
menuinfo_t *SM_BCM57412_submenup = &SM_BCM57412_subtest_menu;

/******************************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 ******************************************************************************/
int sm_bcm57412_test (int show_menu)
{
    /* To run SM bcm57412 tests */
    if (is_bcm57412_sm == FALSE) {
        printf("platform not support sm testcard w/ bcm57412 \n"); 
        return (PASSED);
    }

    if (is_curie_2ru()) {
        system("echo 1 > /sys/bus/pci/rescan");
    }

    build_primary_submenu(BCM57412_10G_tests_submenu_table,
                          BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                          "SM BCM57412", &SM_BCM57412_submenup);
    build_secondary_submenu(BCM57412_10G_tests_submenu_table,
                            BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                            BCM57412_tests_secondary_items);

    if (show_menu) {
        menu(SM_BCM57412_submenup, BCM57412_tests_secondary_items, '\0' );
    } else {
        exec_doall_menu_items(SM_BCM57412_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : BCM57412_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM57412_utility (int show_menu)
{
    build_primary_submenu(BCM57412_util_items, BCM57412_TESTS_UTIL_SIZE,
                          "SM BCM57412 Utilities Menu", &SM_BCM57412_util_menup);
    build_secondary_submenu(BCM57412_util_items, BCM57412_TESTS_UTIL_SIZE,
                            BCM57412_tests_secondary_util_items);

    menu(SM_BCM57412_util_menup, BCM57412_tests_secondary_util_items, '\0' );

    return (PASSED);
}

/******************************************************************************
 *
 * Function: BCM57412_fw_mac_program
 *
 * Description: This function guides BCM57412 firmware and mac programming.
 *
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void BCM57412_fw_mac_program (void)
{
    printf("This utility uses Broadcom CDiag tool by executing script load.sh\n");
    printf("Please refer to /doc/bcm57412_nic_mac_eeprom_program.doc to operate firmware and mac programming\n"); 
}

/******************************************************************************
 *
 * Function: sm_bcm57412_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to BCM57412 MAC and PHY.
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int sm_bcm57412_internal_loopback_test (int port)
{
    char *tname = "BCM57412 internal loopback";
    char buf[128], tmp[64];
    FILE *fp;
    int retval = PASSED, plat_port = UNKNOWN_BCM57412_PORT;

    system(SUPPRESS_MESG);
    testname("%s port %d", tname, port);

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    plat_port = get_bcm57412_plat_port(port); 
    if (plat_port == UNKNOWN_BCM57412_PORT) {
        cterr('f',0, "Failed to get platfrom eth port num");
        return (FAILED);
    }

    /* online for external lpbk test */
    if (!is_curie_2ru())
        sprintf(tmp, "%s %d offline", BCM57412_PORT_LPBK, plat_port);
    else
        sprintf(tmp, "%s %d offline", CURIE2RU_BCM57412_PORT_LPBK, plat_port);
    system(tmp); 
    msleep(200);

    if (is_curie_2ru()) {
        fp = fopen(CURIE2RU_BCM57412_LPBK_RESULT, "r");
    } else {
        fp = fopen(BCM57412_LPBK_RESULT, "r");
    }
    if (fp == NULL) {
        system(OPEN_MESG);
        cterr('f',0, "Failed to open file bcm57412_lpbk.txt");
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, BCM57412_FAIL) != NULL) {
            cterr('f',0, "port %d failed, ", port);
            retval = FAILED;
            break;
        } else {
            prpass(testpass, "port %d passed, ", port);
            break;
        }
    }

    fclose(fp);
    system(OPEN_MESG);
    return (retval);
}

/******************************************************************************
 *
 * Function: sm_bcm57412_external_loopback_test
 *
 * Description: This function perform the external loopback test from CPU to
 *              BCM57412 SFP+ 
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int sm_bcm57412_external_loopback_test (int port)
{
    char *tname = "BCM57412 external loopback";
    char buf[128], tmp[64];
    FILE *fp;
    int retval = PASSED, plat_port = UNKNOWN_BCM57412_PORT;

    testname("%s port %d", tname, port);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM57412 port %d external loopback test\n", port);
        return (retval);
    }

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    system(SUPPRESS_MESG);

    plat_port = get_bcm57412_plat_port(port); 
    if (plat_port == UNKNOWN_BCM57412_PORT) {
        cterr('f',0, "Failed to get platfrom eth port num");
        return (FAILED);
    }

    /* online for external lpbk test */
    if (!is_curie_2ru())
        sprintf(tmp, "%s %d online", BCM57412_PORT_LPBK, plat_port);
    else
        sprintf(tmp, "%s %d online", CURIE2RU_BCM57412_PORT_LPBK, plat_port);
    system(tmp); 
    msleep(200);

    if (is_curie_2ru()) {
        fp = fopen(CURIE2RU_BCM57412_LPBK_RESULT, "r");
    } else {
        fp = fopen(BCM57412_LPBK_RESULT, "r");
    }
    if (fp == NULL) {
        system(OPEN_MESG);
        cterr('f',0, "Failed to open file bcm57412_lpbk.txt");
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, BCM57412_FAIL) != NULL) {
            cterr('f',0, "port %d failed, ", port);
            retval = FAILED;
            break;
        } else {
            prpass(testpass, "port %d passed, ", port);
            break;
        }
    }

    fclose(fp);
    system(OPEN_MESG);
    return (retval);
}

#define BCM57412_SFP_I2C_TEST_SCRIPT    \
"#!/bin/sh\n" \
"\n" \
"if [ -z \"$1\" ]; then\n" \
"        echo \"Please specify an ethernet interface\"\n" \
"        exit 1\n" \
"fi\n" \
"\n" \
"ether=$1\n" \
"data=$(ethtool -m $ether offset 0 length 512 hex on 2>&1)\n" \
"if echo \"$data\" | grep -q 'Cannot get module EEPROM information:'; then\n" \
"        ifconfig $ether down; ifconfig $ether up\n" \
"        data=$(ethtool -m $ether offset 0 length 512 hex on 2>&1)\n" \
"fi\n" \
"\n" \
"if echo \"$data\" | grep -q 'Cannot get module EEPROM information:'; then\n" \
"        echo \"Error: $ether: No SFP module found, please check\"\n" \
"        exit 2\n" \
"fi\n" \
"\n" \
"if echo \"$data\" | grep -q '^0x0000:\\s\\+03 04'; then\n" \
"        type=$(ethtool -m $ether offset 0 length 512 | grep \"Transceiver type\" | cut -d ':' -f 2-)\n" \
"        echo \"$ether: found transceiver: $type\"\n" \
"        exit 0\n" \
"fi\n" \
"\n" \
"echo \"Error: $ether: unidentified data, SFP unplugged? Need to check\"\n" \
"echo \"$data\" | head -n 3\n" \
"exit 3"

static int curie2ru_sm_bcm57412_sfp_plus_i2c_test(int port)
{
    int fd, rc, ret;
    const char *i2c_script = "/tmp/bcm57412_i2c_test.sh";
    const char *ether = (port == 1) ? "eth12" : "eth13";
    size_t script_len = sizeof(BCM57412_SFP_I2C_TEST_SCRIPT) - 1;
    ssize_t len;
    char cmd[1024];

    snprintf(cmd, sizeof(cmd), "rm -f %s", i2c_script);
    system(cmd);

    fd = open(i2c_script, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        cterr('f', 0, "failed to open %s, ", i2c_script);
        return FAILED;
    }

    len = write(fd, BCM57412_SFP_I2C_TEST_SCRIPT, script_len);
    if (len != script_len) {
        close(fd);
        cterr('f', 0, "failed to write %s: error %d, ", i2c_script, len);
        return FAILED;
    }

    close(fd);

    snprintf(cmd, sizeof(cmd), "sh %s %s", i2c_script, ether);
	ret = system(cmd);
	if (ret < 0) {
		cterr('f', 0, "failed to execute %s\n: error %d, ", i2c_script, ret);
		return FAILED;
	}

	rc = WEXITSTATUS(ret);
	if (rc != 0) {
		cterr('f', 0, "test failed, error %d, ", rc);
        return FAILED;
    }

    prpass(testpass, "port %d passed, ", port);

    return PASSED;
}

/******************************************************************************
 *
 * Function: sm_bcm57412_sfp_plus_i2c_test
 *
 * Description: This function checks SFP+(for Curium)/SFP(for Thallium)
 *              cookie byte 0 and byte 1 to make sure the i2c bus 
 *              between BCM57412 and SFP+/SFP module is good.
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int sm_bcm57412_sfp_plus_i2c_test (int port)
{
    char *tname = "BCM57412 i2c";
    char buf[128];
    FILE *fp;
    int retval = PASSED;

    testname("%s port %d", tname, port);

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM57412 port %d i2c test\n", port);
        return (retval);
    }

    if (is_curie_2ru()) {
        return curie2ru_sm_bcm57412_sfp_plus_i2c_test(port);
    }

    system(SUPPRESS_MESG);
    if (port == BCM57412_PORT1) {
        system(BCM57412_SM_PORT1_I2C);
    } else {
        system(BCM57412_SM_PORT2_I2C);
    }

    fp = fopen(BCM57412_I2C_RESULT, "r");
    if (fp == NULL) {
        system(OPEN_MESG);
        cterr('f',0, "Failed to open file bcm57412_i2c_test.txt");
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, SFP_PLUS_FIXED_ID) == NULL) {
            cterr('f',0, "port %d failed, please check whether has external loopback module", port);
            retval = FAILED;
            break;
        } else {
            prpass(testpass, "port %d passed, ", port);
            break;
        }
    }

    fclose(fp);
    system(OPEN_MESG);
    return (retval);
}

static void curie2u_dump_sfp_eeprom(void)
{
    printf("\n----Display Port 1 SFP+ EEPROM----\n");
    system("ethtool -m eth12 offset 0 length 512 hex on");
    printf("\n----Display Port 2 SFP+ EEPROM----\n");
    system("ethtool -m eth13 offset 0 length 512 hex on");
}

static void curie1ru_dump_sfp_eeprom(void)
{
    printf("\n----Display Port 1 SFP+ EEPROM----\n");
    system("ethtool -m eth10 offset 0 length 512 hex on");
    printf("\n----Display Port 2 SFP+ EEPROM----\n");
    system("ethtool -m eth11 offset 0 length 512 hex on");
}

static void dump_sfp_eeprom (void)
{
    if (is_curie_2ru()) {
        curie2u_dump_sfp_eeprom();
        return;
    }

    if (is_curie_1ru()) {
        curie1ru_dump_sfp_eeprom();
        return;
    }
}

/**********************************************************************
 *
 * Function: sm_bcm57412_pcie_check_test  
 *
 * Description: This function perform the pcie lane/speed check test from CPU to
 *              BCM57412 
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************/
int sm_bcm57412_pcie_check_test (void)
{
    char *tname = "BCM57412 pcie check";
    int result = PASSED;
    uint32_t bus, dev, func, reg_val, sta_val;
    uint32_t sta_s, sta_w;

    testname("%s ", tname);
        
    /* root port for SM TC BCM PHY is 00:03.2 */
    if (is_curie_1ru()) { 
        bus = PCI_BUS_0; 
        dev = PCI_DEV_3;
        func = PCI_FUN_2; 
    } else if (is_curie_2ru()) {
        bus = 0x5e;
        dev = 2;
        func = 0;
    } else {
        cterr('f',0, " must apply root port info from plafrom ");
        return (FAILED); 
    }

    prpass(testpass, "%s", "able to get bus num of PLX PCIe switch");
    reg_val = get_pcie_cap_struct_ptr(bus, dev, func, PCI_CAP_PTR_OFFSET);
    if (reg_val == FAILED) {
        cterr('f',0, "Can't get PCI cap pointer");
        return (FAILED);
    }

    sta_val = get_pcie_link_status(bus, dev, func, reg_val);

    /* Speed - bit 0~3 */
    sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
    /* Width - bit 4~9 */
    sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

    /* check speed */
    if (sta_s == PCI_EXP_LINK_STA_SPD_8GT) {
        prpass(testpass, "Link speed is 8G ");
    } else {
        cterr('f',0, "Link speed is not 8G");
        result = FAILED;  /* fail through */
    }

    /* check width */
    if (sta_w == PCI_EXP_LINK_STA_WID_4) {
        prpass(testpass, "Link width is x4 ");
    } else {
        cterr('f',0, "Link width should be x4, please check the width");
        result = FAILED;  /* fail through */
    }

    if (result != FAILED) {
        prpass(testpass, "PCIe lane scan success. ");
    }

    return (result);
}


/**********************************************************************
 *
 * Function: get_bcm57412_plat_port
 *
 * Description: return bcm57412 platform eth port number for testing.
 *
 * Inputs      : port - BCM57412_PORT1, BCM57412_PORT2.
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************/
static int get_bcm57412_plat_port (int port) {

    if (port == BCM57412_PORT1) {
        if (is_curie_1ru()) {
            return (CURIE_BCM57412_PORT1); 
        } else if (is_neptune()) {
            return (NEPTUNE_BCM57412_PORT1);
        } else if (is_curie_2ru()) {
            return (CURIE2RU_BCM57412_PORT1);
        } else {
            printf("%d,%s,%s : unknown platform to detect bcm port\n",
                  __LINE__, __FUNCTION__, __FILE__); 
            return (UNKNOWN_BCM57412_PORT); 
        }
    } else { /* BCM57412_PORT2 */
        if (is_curie_1ru()) {
            return (CURIE_BCM57412_PORT2); 
        } else if (is_neptune()) {
            return (NEPTUNE_BCM57412_PORT2);
        } else if (is_curie_2ru()) {
            return (CURIE2RU_BCM57412_PORT1);
        } else {
            printf("%d,%s,%s : unknown platform to detect bcm port\n",
                  __LINE__, __FUNCTION__, __FILE__); 
            return (UNKNOWN_BCM57412_PORT); 
        }
    }
}

/*-------------------------------------------------
$Log: testcard_bcm57412.c,v $
Revision 1.4  2020/12/29 03:10:51  leschen
Remove bnxt_en operations.

Revision 1.3  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.2  2019/07/22 00:52:20  alpeng
 support sm testcard w/ bcm57412


$Endlog$
*/
