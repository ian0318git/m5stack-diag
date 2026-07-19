/* $Id: diag_i350_test.c,v 1.6 2020/08/07 09:02:35 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_i350_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_i350_test.c - Diag Test for Intel I350.
 *
 * July 2018, Leschen 
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

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
#include "ethernet.h"
#include "linux_eth.h"
#include "queryflags.h" /* for query user functions */  
#include "diag_i350_test.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_gephy_test.h"
#include "dnv_eth_lib.h"
#include "diag_sfp_util.h"


#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int i350_utility(int);
static void i350_dump_sfp_present_pin_status(void);
static void i350_enable_disable_sfp(void);
int i350_check_result(int, char*);
int diag_i350_single_port_reg_test(char*, int, char*);
int diag_i350_single_port_internal_test(char*, int, char*); 
int diag_i350_single_port_ext_test(char*, char*, int, char*);
int i350_register_test(int);
int i350_internal_loopback_test(int);
int i350_external_sfp_loopback_test(int);
static void i350_get_nic_number(void);
static void i350_show_running_port(void);
static void i350_display_cap(void);
int i350_read_ctrl_reg(ulong, uint *, uint);
int i350_write_ctrl_reg(ulong, uint, uint);

/******************************************************************************
 *  List of Menu used for I350
 *****************************************************************************/
static submenu_xtable_t I350_tests_submenu_table[] = {
   {"I350 Utility", (type_t(*)())i350_utility,   FALSE,
    0, NULL, 0, (type_t(*)())i350_utility,   TRUE},
   {"I350 Register Test", (type_t(*)())i350_register_test, 0,
     MF_3, (PFT)0, 0, (type_t(*)())0,   0},
   {"I350 Internal Loopback Test", (type_t(*)())i350_internal_loopback_test, 0,
     MF_3, (PFT)0, 0, (type_t(*)())0,   0},
   {"I350 External SFP Loopback Test", (type_t(*)())i350_external_sfp_loopback_test, 0,
     MF_3, (PFT)0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for I350
 *****************************************************************************/
static submenu_xtable_t I350_util_items[] = {
    {"Get I350 NIC Number", (type_t(*)())i350_get_nic_number, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display I350 ports capability", (type_t(*)())i350_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display Currently Running ETH Port", (type_t(*)())i350_show_running_port, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"I350 check test result", (type_t(*)())i350_check_result, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump I350 SFP ports present pin", (type_t(*)())i350_dump_sfp_present_pin_status, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Assert SFP DISABLE", (type_t(*)())i350_enable_disable_sfp, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read I350 Specific SFP PHY", (type_t(*)())igb_read_sfp_phy_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write I350 Specific SFP PHY", (type_t(*)())igb_write_sfp_phy_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define I350_TESTS_SUBMENU_TABLE_SIZE (sizeof(I350_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))

#define I350_TESTS_UTIL_SIZE (sizeof(I350_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t I350_tests_primary_items[I350_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t I350_tests_secondary_items[I350_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t I350_tests_primary_util_items[I350_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t I350_tests_secondary_util_items[I350_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 *I350 Utils submenu
 *****************************************************************************/
menuinfo_t I350_util_menu = {
    "I350 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    I350_tests_primary_util_items,
};
menuinfo_t *I350_util_menup = &I350_util_menu;

menuinfo_t I350_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    I350_tests_primary_items,
};
menuinfo_t *I350_submenup = &I350_subtest_menu;

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
int diag_i350_test (int show_menu)
{
    build_primary_submenu(I350_tests_submenu_table,
                          I350_TESTS_SUBMENU_TABLE_SIZE,
                          "I350 ", &I350_submenup);
    build_secondary_submenu(I350_tests_submenu_table,
                            I350_TESTS_SUBMENU_TABLE_SIZE,
                            I350_tests_secondary_items);

    if (show_menu) {
        menu(I350_submenup, I350_tests_secondary_items, '\0' );
    } else {
        exec_doall_menu_items(I350_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : i350_utility
 * Description : I350 Utility menu
 * Inputs      : show_menu - for future use
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int i350_utility (int show_menu)
{
    build_primary_submenu(I350_util_items, I350_TESTS_UTIL_SIZE,
                          "I350 Utilities Menu", &I350_util_menup);
    build_secondary_submenu(I350_util_items, I350_TESTS_UTIL_SIZE,
                            I350_tests_secondary_util_items);

    menu(I350_util_menup, I350_tests_secondary_util_items, '\0' );

    return (PASSED);
}

/*******************************************************************************
 * Function: i350_get_nic_number
 *
 * Description: To get I350 four port's NIC number 
 *
 * Input: none
 *
 * Return: none
 *******************************************************************************/
static void i350_get_nic_number (void)
{
    system(I350_GET_NIC);
}

/*******************************************************************************
 * Function: i350_show_running_port
 *
 * Description: To display all the running ETH ports 
 *
 * Input: none
 *
 * Return: none
 *******************************************************************************/
static void i350_show_running_port (void)
{
    system(I350_SHOW_RUNNING_ETH);
}

/*******************************************************************************
 * 
 * Function: i350_display_cap
 *
 * Description: Display I350 ports capability
 *
 * Input: none
 *
 * Return: none
 *
 ******************************************************************************/
static void i350_display_cap (void)
{
    system(DISPLAY_I350_PORT1_CAP);
    system(DISPLAY_I350_PORT2_CAP);
}

/*
 * Function: i350_dump_sfp_present_pin_status
 *
 * Description: Dump I350 SFP present PIN
 *
 * Input: none
 *
 * Return: none
 */
static void i350_dump_sfp_present_pin_status (void)
{
    uint i350_ctrl_val = 0;

    i350_read_ctrl_reg(I350_PORT1, &i350_ctrl_val, NUTELLA_I350_CTRL);
    if (!(i350_ctrl_val & NUTELLA_I350_SFP_PRESENT)) {
        printf("I350 Port: %d, Detect SFP Present\n", (int)I350_PORT1);
    } else {
        printf("I350 Port: %d, Not Detect SFP Present\n", (int)I350_PORT1);
    }

    i350_read_ctrl_reg(I350_PORT2, &i350_ctrl_val, NUTELLA_I350_CTRL);
    if (!(i350_ctrl_val & NUTELLA_I350_SFP_PRESENT)) {
        printf("I350 Port: %d, Detect SFP Present\n", (int)I350_PORT2);
    } else {
        printf("I350 Port: %d, Not Detect SFP Present\n", (int)I350_PORT2);
    }

}

/*
 * Function: i350_enable_disable_sfp
 *
 * Description: Enable/disable I350 SFP
 *
 * Input: none
 *
 * Return: none
 */
static void i350_enable_disable_sfp (void)
{
    uint i350_ctrl_val = 0;
    int set_value = 0, which_sfp = 0;

    which_sfp = getdec_answer("\nEnter I350 Port 1/2 SFP ", 1, 1, 2);
    set_value = gethex_answer("\nDisable SFP: 1; Enable SFP: 0 ", 0, 0, 1);

    if (set_value == 1) {
        i350_read_ctrl_reg(which_sfp, &i350_ctrl_val, NUTELLA_I350_CTRL);
        i350_ctrl_val |= NUTELLA_I350_SFP_DISABLE;
        i350_write_ctrl_reg(which_sfp, i350_ctrl_val, NUTELLA_I350_CTRL);
        printf("I350 Port: %d, Disable SFP\n", which_sfp);

    } else {
        i350_read_ctrl_reg(which_sfp, &i350_ctrl_val, NUTELLA_I350_CTRL);
        i350_ctrl_val &= ~(NUTELLA_I350_SFP_DISABLE);
        i350_write_ctrl_reg(which_sfp, i350_ctrl_val, NUTELLA_I350_CTRL);
        printf("I350 Port: %d, Normal SFP\n", which_sfp);
    }

}

/******************************************************************************
 *
 * Function: I350_check_result
 *
 * Description: This function performs uses to check test result
 *
 * Inputs      : port - I350 port number 
                 test_name - what kind of test be checked
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_check_result (int port, char *test_name)
{
    FILE *fp;
    int retval = FAILED;
    char buf[512] = {0};

    if (port == 0){
        printf("Checking I350 test result\n");
    } else{
        printf("Checking I350 port %d %s test result\n", port, test_name);
    }
    
    fp = fopen(I350_TEST_RESULT, "r");
    if (fp == NULL) {
        if (port == 0){
            cterr('f',0, "can't open file i350_test_result.txt");
        } else {
            cterr('f',0, "Port %d can't open file i350_test_result.txt", port);
        }
        
        return (retval);
    }
    
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, I350_FAIL) != NULL) {
            if (port == 0){
                cterr('f',0, "Test failed");
            } else{
                cterr('f',0, "port %d %s test failed, ", port, test_name);
            }
            fclose(fp);
            printf("\n");
            system("cat /nutella-diag/i350_test_result.txt");
            printf("\n");
            system("cat /nutella-diag/i350_test.LOG");
            printf("\n");
            return (retval);
        } else {
            if (port == 0){
                printf("Test passed");
            } else{
                printf("port %d test passed, \n", port);
            }
            printf("\n");
            retval = PASSED;
            break;
        }
    }

    fclose(fp);
    return (retval);
}

/*******************************************************************************
 *
 * Function    : diag_i350_single_port_reg_test
 * Description : This function performs single port register test from CPU to
 *               I350 SFP module 
 * Inputs      : script_num - base on port number to give different parameter
 *                            to script
 *               port_num   - I350 port number
 *               tname      - I350 test name
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_i350_single_port_reg_test(char* script_num, int port_num, char* tname)
{
    int retval = FAILED;

    system(script_num);
    msleep(WAIT_I350_TEST);
    retval = i350_check_result(port_num, tname);
    if (retval == FAILED) {
        cterr('f', 0, "%s: Failed to do register test (from I350 port %d)",
              __FUNCTION__, port_num);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function    : diag_i350_single_port_internal_test
 * Description : This function performs single port internal loopback test
 *               from CPU to I350 SFP module 
 * Inputs      : script_num - base on port number to give different parameter
 *                            to script
 *               port_num   - I350 port number
 *               tname      - I350 test name
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_i350_single_port_internal_test(char* script_num , int port_num, char* tname)
{
    int retval = FAILED;
    
    system(script_num);
    msleep(WAIT_I350_TEST);
    retval = i350_check_result(port_num, tname);
    if (retval == FAILED) {
        cterr('f', 0, "%s: Failed to do internal loopback test (from I350 port %d)",
              __FUNCTION__, port_num);
    }
    
    return (retval);
} 

/*******************************************************************************
 *
 * Function    : diag_i350_single_port_ext_test
 * Description : This function performs single port external loopback test
 *               from CPU to I350 SFP module 
 * Inputs      : interface_name - I350 interface name
 *               interface_up        - link up command
 *               port_num       - I350 port number
 *               interface_down      - link down command
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_i350_single_port_ext_test(char* interface_name, char* interface_up,
                                   int port_num, char* interface_down)
{
    int retval = FAILED;
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;
    char iface_name_t[16];
    char *iface_name = iface_name_t;
    uint buf;
    
    /* Create socket for ioctl calls */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sk < 0) {
        cterr('f', 0, "%s() Error Creating RX Socket", __FUNCTION__);
    }
    
    sprintf(iface_name, interface_name);
    
    sprintf(ethreq.ifr_name, iface_name);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    /* detect phy media type from SFP module.
     * In addition to using the standardized calls,
     * each interface can define its own ioctl commands.
     * The plip interface, for example,
     * allows the interface to modify its internal timeout values via ioctl.
     * The ioctl implementation for sockets recognizes 16 commands
     * as private to the interface: SIOCDEVPRIVATE through SIOCDEVPRIVATE+15. */
    if (ioctl(sk, I350_IOCTL_COMMAND3, &ethreq) < 0) {
        cterr('f', 0, "%s() Error do IOCTL %s", __FUNCTION__, I350_IOCTL_COMMAND3);
        close(sk);
        return (FAILED);
    } 
    buf = miip->val_out;
    
    /* Set up SFP module FCLF-8521-3 register to link up */
    if (buf == e1000_media_type_copper) {
        if (ioctl(sk, I350_IOCTL_COMMAND2, &ethreq) < 0) {
            cterr('f', 0, "%s() Error do IOCTL %s", __FUNCTION__, I350_IOCTL_COMMAND2);
            close(sk);
            return (FAILED);
        } 
    }
    close(sk);

    system(interface_up);

    msleep(WAIT_I350_ETH_TEST);

    /* check Linux ethernet interface status */
    if (chk_linux_eth_linkup(port_num, TRUE) == FAILED) {
        cterr('f',0, "Ethernet %s cannot link up", iface_name);
    }

    retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);
    if (retval != PASSED) {
        cterr('f', 0, "%s: Failed To do loopback test (from I350 SFP port %s)",
              __FUNCTION__, iface_name);
    } 
    system(interface_down);

    return (retval);
}

/******************************************************************************
 *
 * Function: I350_register_test
 *
 * Description: This function performs the register test
 *
 * Inputs      : dummy 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_register_test (int dummy)
{
    char *tname = "I350 register";
    int retval1 = FAILED, retval2 = FAILED;

    testname("%s", tname);
    
    prpass(testpass, "%s, Test NIC 1, ", tname);
    retval1 = diag_i350_single_port_reg_test(I350_PORT1_REG_TEST, I350_PORT1, tname);

    prpass(testpass, "%s, Test NIC 2, ", tname);
    retval2 = diag_i350_single_port_reg_test(I350_PORT2_REG_TEST, I350_PORT2, tname);

    prcomplete(testpass, errcount, (char *)0);
    return (retval1 | retval2);
}

/******************************************************************************
 *
 * Function: I350_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to I350 MAC and PHY.
 *
 * Inputs      : dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_internal_loopback_test (int dummy)
{
    char *tname = "I350 internal lpbk";
    int retval1 = FAILED, retval2 = FAILED;

    testname("%s", tname);

    prpass(testpass, "%s, Test NIC 1, ", tname);
    retval1 = diag_i350_single_port_internal_test(I350_PORT1_INT_LPBK,
                                                  I350_PORT1, tname);

    prpass(testpass, "%s, Test NIC 2, ", tname);
    retval2 = diag_i350_single_port_internal_test(I350_PORT2_INT_LPBK,
                                                  I350_PORT2, tname);

    prcomplete(testpass, errcount, (char *)0);
    return (retval1 | retval2);
}

/******************************************************************************
 *
 * Function: I350_external_sfp_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I350 SFP module
 *
 * Inputs      : dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_external_sfp_loopback_test (int dummy)
{
    char *tname = "I350 external SFP lpbk";
    int retval1 = FAILED, retval2 = FAILED;

    testname("%s", tname);

    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    }

    prpass(testpass, "%s, Test NIC 1, ", tname);
    retval1 = diag_i350_single_port_ext_test(I350_INTERFACE_NAME_PORT1, I350_PORT1_UP,
                                             NUTELLA_I350_SFP_PORT1, I350_PORT1_DOWN);

    prpass(testpass, "%s, Test NIC 2, ", tname);
    retval2 = diag_i350_single_port_ext_test(I350_INTERFACE_NAME_PORT2, I350_PORT2_UP,
                                             NUTELLA_I350_SFP_PORT2, I350_PORT2_DOWN);
    
    prcomplete(testpass, errcount, (char *)0);
    return (retval1 | retval2);
}

/*******************************************************************************
 *
 * Function    : i350_read_ctrl_reg
 *
 * Description : Function to read Device Control Register in I350
 *               
 * Inputs      : which_port - 1/2
 *               *val   - data buffer 
 *               offset - register offset
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int i350_read_ctrl_reg (ulong which_port, uint *val, uint offset)
{   
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size;
    char     ethaddr[128];

    map_size = MAP_SIZE;
 
    if (which_port == I350_PORT1) {
        sprintf(ethaddr, "%s%s", NUTELLA_I350_PCIE_BUS, NUTELLA_I350_PCIE_PORT1);
    } else if (which_port == I350_PORT2) {
        sprintf(ethaddr, "%s%s", NUTELLA_I350_PCIE_BUS, NUTELLA_I350_PCIE_PORT2);
    } else {
        printf("Wrong port number: %d\n", (int)which_port);
        return (FAILED);
    }

    fd = open(ethaddr, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
        fflush(stdout);
    }

    map_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                    target & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
         close(fd);
         return (FAILED);
    }
    virt_addr = map_base + offset;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I350 Port :%d, Device Control Register - CTRL: %x\n", 
               (int)which_port, *(volatile uint32_t*)virt_addr);
    }

    *val = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);



    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : i350_write_ctrl_reg
 *
 * Description : Function to write Device Control Register in I350
 *               
 * Inputs      : which_port - 1/2
 *               val   - data buffer 
 *               offset - register offset
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int i350_write_ctrl_reg (ulong which_port, uint val, uint offset)
{   
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size;
    char     ethaddr[128];

    map_size = MAP_SIZE;
 
    if (which_port == I350_PORT1) {
        sprintf(ethaddr, "%s%s", NUTELLA_I350_PCIE_BUS, NUTELLA_I350_PCIE_PORT1);
    } else if (which_port == I350_PORT2) {
        sprintf(ethaddr, "%s%s", NUTELLA_I350_PCIE_BUS, NUTELLA_I350_PCIE_PORT2);
    } else {
        printf("Wrong port number: %d\n", (int)which_port);
        return (FAILED);
    }

    fd = open(ethaddr, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
        fflush(stdout);
    }

    map_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                    target & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
         close(fd);
         return (FAILED);
    }
    virt_addr = map_base + offset;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I350 Port :%d, Device Control Register - CTRL: %x\n", 
               (int)which_port, *(volatile uint32_t*)virt_addr);
    }

    *(volatile uint32_t*)virt_addr = val;

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);

    return (PASSED);
}
/*-------------------------------------------------
$Log: diag_i350_test.c,v $
Revision 1.6  2020/08/07 09:02:35  alicehua
CSCvv24244: Add SFP PHY read/write utilities.

Revision 1.5  2019/12/19 07:16:51  harrchan
1.Add utility to dump SFP present pin status 2.Add utility to enable/disable SFP module(CSCvs46746)

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
