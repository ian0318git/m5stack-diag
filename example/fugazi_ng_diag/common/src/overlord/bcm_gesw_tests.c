/* $Id: bcm_gesw_tests.c,v 1.9 2018/05/18 09:24:50 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/bcm_gesw_tests.c,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw_tests.c - BCM56321 tests. This is leveraged from the
 *                    BCM SDK diag shell diagnostics.
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <linux/types.h>

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "menu.h"
#include "proto.h"
#include "error.h"
#include "queryflags.h"
#include "router_if.h"
#include "bcm_gesw_defs.h"
#include "plat_defs.h"
#include "eth_pkt_utils.h"
#include "dash_fpga.h"

extern int exec_bcm_shell_cmd (int unit, char *cmd, int print_cmd);
extern void detach_bcm_driver(void);
extern int ovld_gesw_init(void);
extern void gesw_tests(void);
extern void l2_forward_setup(void);
extern void l2_forward_cleanup(void);
extern void port_tx_util(void);
extern void cavecreek_sgmii_macsa_declare(void);
extern void ctrl_plane_sgmii_macsa_declare(void);
extern int invoke_bcm_shell(void);
extern int gesw_configed;
extern int host_send_pkt_util(void);
extern void pfix_loopback_util(void); /* pfix-dbg */
extern void pfix_loopback_util_1(void); /* pfix-dbg */

static int reg_reset_default_test (void);
static int register_test (void);
static int pci_compliance_test (void);
static int pci_s_channel_buf_test (void);
static int bcm_cpu_loopback_test (void);
static int bcm_cpu_benchmarks_test (void);
static int bcm_ge_port_phy_lpbk_test (void);
static int bcm_ge_port_snake_test (void);
static void loopback_util(void);
static void show_counter_util(void);

extern void set_ctrl_plane_sgmii_for_ge_test(int up);

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t gesw_tests_submenu_table[] = {
   {"Register reset default test", (type_t(*)())reg_reset_default_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Register read-write test", (type_t(*)())register_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PCI Compliance test", (type_t(*)())pci_compliance_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PCI S-Channel Bufferr test", (type_t(*)())pci_s_channel_buf_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"CPU Benchmarks test", (type_t(*)())bcm_cpu_benchmarks_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"CPU port loopback test", (type_t(*)())bcm_cpu_loopback_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"GE and XAUI internal PHY loopback test", (type_t(*)())bcm_ge_port_phy_lpbk_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"GE port snake test", (type_t(*)())bcm_ge_port_snake_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},

   /*----  Below are utilities ----*/
   {"Invoke BCM shell", (type_t(*)())invoke_bcm_shell,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Init BCM util", (type_t(*)())ovld_gesw_init,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"GESW port loopback util", (type_t(*)())loopback_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"GESW port TX util", (type_t(*)())port_tx_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"GESW Show counter util", (type_t(*)())show_counter_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Host SGMII port send pkt util", (type_t(*)())host_send_pkt_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},

#ifdef UTAH
   {"Host SGMII declare MAC address", (type_t(*)())ctrl_plane_sgmii_macsa_declare,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
#else /* Overord and Neptune */
   {"Host SGMII declare MAC address", (type_t(*)())cavecreek_sgmii_macsa_declare,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
#endif
   {"(for debug use) bcm_port_phy_control API check", (type_t(*)())pfix_loopback_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"(for debug use) bcm_port_loopback API check", (type_t(*)())pfix_loopback_util_1,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GESW_TESTS_SUBMENU_TABLE_SIZE (sizeof(gesw_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gesw_tests_primary_items[GESW_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t gesw_tests_secondary_items[GESW_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t gesw_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    gesw_tests_primary_items,
};
menuinfo_t *gesw_submenup = &gesw_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: gesw_run_all_tests
 * Run all the GE switch standalone tests
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int gesw_run_all_tests(void)
{
    if (reg_reset_default_test() == FAIL) {
        return (FAIL);
    }

    if (register_test() == FAIL) {
        return (FAIL);
    }

    if (pci_compliance_test() == FAIL) {
        return (FAIL);
    }

    if (pci_s_channel_buf_test() == FAIL) {
        return (FAIL);
    }

    if (bcm_cpu_benchmarks_test() == FAIL) {
        return (FAIL);
    }

    if (bcm_cpu_loopback_test() == FAIL) {
        return (FAIL);
    }

    if (bcm_ge_port_phy_lpbk_test() == FAIL) {
        return (FAIL);
    }

    if (bcm_ge_port_snake_test() == FAIL) {
        return (FAIL);
    }

    return(PASS);
}

/*------------------------------------------------------------------
 *
 * Function: gesw_test_main
 *	This is the entry point for the GESW main test.
 *
 * Input:  show_menu = 0 show submenu, !=0 perform all tests
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
gesw_test_main (int show_menu)
{
    build_primary_submenu(gesw_tests_submenu_table,
			  GESW_TESTS_SUBMENU_TABLE_SIZE,
                          "BCM GE switch", &gesw_submenup);
    build_secondary_submenu(gesw_tests_submenu_table,
                            GESW_TESTS_SUBMENU_TABLE_SIZE,
                            gesw_tests_secondary_items);

    if (show_menu) {
        return(gesw_run_all_tests());
    } else {
        menu(gesw_submenup, gesw_tests_secondary_items, '\0' );
    }

    return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: run_this_test
 * This function is used to run the tests provided by the BCM diag
 * shell. 
 *
 * Input: 
 * t_name - char string of the test name
 * t_num - BCM diag shell test number 
 * cmd_args - specific arguments for that test
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int run_this_test(char *t_name, int t_num, char *cmd_args)
{
    int unit = bcm_uid;
    char cmd[64];
    int rv;
    int runcnt_a, runcnt_b, passcnt_a, passcnt_b;

    /* The cavecreek port will affect the linkscan of the GESW.
     * Debugging with broadcom FAE. Turn the port off for now.
     */
    //    printf("pfix-0 %s\n", __FUNCTION__);
    set_ctrl_plane_sgmii_for_ge_test(0); /* shut down the ports */

    testname("BCM GESW");
    prpass(testpass, "%s, ", t_name);

    if (cmd_args != 0) {
        sprintf(cmd, "testrun %d %s;",t_num, cmd_args);
    }
    else {
        sprintf(cmd, "testrun %d;",t_num);
    }

    /* The following tests calls the rc.soc to re-init the bcm
     * drivers. Need to detach the current driver from memory to avoid
     * double drivers running and cause memory leak.
     */
    switch(t_num) {
    case 17:
    case 39:
    case 49:
	detach_bcm_driver();
	break;
    default:
        break;
    }

    get_bcm_shell_test_result(t_num, &runcnt_b, &passcnt_b);
    printf("\n");
    rv = exec_bcm_shell_cmd(unit, cmd, TRUE);
    get_bcm_shell_test_result(t_num, &runcnt_a, &passcnt_a);

    /* Make sure the GESW is init after test since internal tests
     * destroys the chip setup for system support and
     * bringup the cavecreek SGMII ports after the test
     */
    //    printf("pfix-1 %s\n", __FUNCTION__);
    ovld_gesw_init(); // Init the GESW before setting cavecreek
    msleep(10);
    printf("Re-init BCM after internal test\n");

    //    printf("pfix-2 %s\n", __FUNCTION__);
    set_ctrl_plane_sgmii_for_ge_test(1);

    if (!((rv == PASS) && (runcnt_a > runcnt_b) && (passcnt_a > passcnt_b))) {
        cterr('f', 0, "BCM %s test failed.\n", t_name);
	rv = FAIL;
    }

    printf("\n");
    return(rv);
}

/*------------------------------------------------------------------
 *
 * Function: 
 * Run the GE switch register reset default test provide by the BCM
 * diag shell.
 * It resets the switch and verifies the reset values of the
 * registers. CMIC and PHY registers are not included.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int reg_reset_default_test (void)
{
    int testnum = 1;
    return(run_this_test("Register reset defaults", testnum, (char *)0));
}

/*------------------------------------------------------------------
 *
 * Function: register_test
 * Run the GE switch register r/w test provide by the BCM diag shell.
 * It writes specific patterns to register and verifies them. Patterns
 * used are 0x00000000, 0xffffffff, 0x55555555, 0xaaaaaaaa.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int register_test (void)
{
    int testnum = 3;
    return(run_this_test("Register read-write", testnum, (char *)0));
}

/*------------------------------------------------------------------
 *
 * Function: 
 * Run the GE switch PCI compliance test provide by the BCM diag
 * shell.
 * It tests the PCIe configuration read/write operations.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int pci_compliance_test (void)
{
    int testnum = 2;
    return(run_this_test("PCI Compliance", testnum, (char *)0));
}

/*------------------------------------------------------------------
 *
 * Function: 
 * Run the GE switch PCI S channel test provide by the BCM diag
 * shell. 
 * It writes and verifies patterns in CMIC_SCHAN_MSG registers
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int pci_s_channel_buf_test (void)
{
    int testnum = 4;
    return(run_this_test("PCI S-Channel Buf", testnum, (char *)0));
}

/*------------------------------------------------------------------
 *
 * Function: bcm_cpu_loopback_test
 * Run the GE switch cpu loopback test provide by the BCM diag shell.
 * It is a simple interrupt-driven DMA test. For each DMA channel,
 * transmits packets from CMIC to CMiC and verifies the pacekt
 * counters. 
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int bcm_cpu_loopback_test (void)
{
    int testnum = 17;
    return(run_this_test("CPU port loopback", testnum, (char *)0));
}

/*------------------------------------------------------------------
 *
 * Function: bcm_cpu_benchmarks_test
 * Run the GE switch cpu benchmark test provide by the BCM diag shell.
 * It includes system memory test, DMA memory test, PCI r/w test,
 * mutex lock & unlock test, SOC register r/w test, FP r/w test, PHY
 * register r/w test, L2 insert/delete test, L2 table DMA test, L2
 * lookup test.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int bcm_cpu_benchmarks_test (void)
{
    int testnum = 21;
    return(run_this_test("CPU Benchmarks", testnum, (char *)0));
}

/*------------------------------------------------------------------
 *
 * Function: bcm_ge_port_phy_lpbk_test
 * Run the GE switch PHY loopback test provide by the BCM diag shell
 * It verifies internal PHY loopback by transmitting packets of
 * variable length and receiving them on the CPU.
 * 
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int bcm_ge_port_phy_lpbk_test (void)
{
    char *t_name = "GE and XAUI internal PHY loopback";
    int testnum = 49; /* This is PHY loopback mark 2 test in the bcm
			 diag shell */
    char cmd_arg[64];
    uint64 port_mask;

    get_gesw_pbmp(&port_mask, BCM_PTYPE_ALL);

    sprintf(cmd_arg, "PortBitMap=%#.16llx", port_mask);
    return(run_this_test(t_name, testnum, cmd_arg));
}

/*------------------------------------------------------------------
 *
 * Function: bcm_ge_port_snake_test
 * Run the GE switch snake test provide by the BCM diag shell.
 * Packets are sent from CPU and forwarded from port ge0 to ge23 and
 * returned back to CPU in a snake fashion to check each port's
 * forwarding and receiving function.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int bcm_ge_port_snake_test (void)
{
    char *t_name = "GE port snake";
    int testnum = 39;
    char cmd_arg[64];
    uint64 port_mask;

    get_gesw_pbmp(&port_mask, (BCM_PTYPE_GE | BCM_PTYPE_10GKR));

    sprintf(cmd_arg, "PortBitMap=%#.16llx Count=1", port_mask);
    return(run_this_test(t_name, testnum, cmd_arg));
}


/*------------------------------------------------------------------
 *
 * Function: set_gesw_mac_loopback
 * Set the mac loopback of the port.
 * The port_num uses the GESW_PBMP_CMIC_START numbering order.
 * User should use the ovld_get_ge_sw_port_num to get the port_num.
 *
 * Input: 
 * port_num - the GE port number
 * onoff - 1 for on, 0 for off
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
int set_gesw_mac_loopback(int port_num, int onoff)
{
    int port_type = get_gesw_ptype(port_num);

    if (port_type == 0) {
        printf("%s: port_num unknown. No mac loopback is set\n",
	       __FUNCTION__);
	return(FAIL);
    }

    if (port_type != BCM_PTYPE_XE) {
        if (onoff) {
	    return(bcm_gesw_ge_lpbk_set(bcm_uid, 
					port_num, 
					GESW_MAC_LOOPBACK));
	}
	else {
	    return(bcm_gesw_ge_lpbk_set(bcm_uid, 
					port_num, 
					GESW_LOOPBACK_NONE));
	}
    }
    else {
        if (onoff) {
	    return(bcm_gesw_xaui_lpbk_set(bcm_uid, 
					  port_num, 
					  GESW_MAC_LOOPBACK));
	}
	else {
	    return(bcm_gesw_xaui_lpbk_set(bcm_uid, 
					  port_num, 
					  GESW_LOOPBACK_NONE));
	}
    }
}

/*------------------------------------------------------------------
 *
 * Function: set_gesw_line_loopback
 * Set the line loopback of the port.
 * The port_num uses the GESW_PBMP_CMIC_START numbering order.
 * User should use the ovld_get_ge_sw_port_num to get the port_num.
 *
 * Input: 
 * port_num - the GE port number
 * onoff - 1 for on, 0 for off
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
int set_gesw_line_loopback(int port_num, int onoff)
{
    int port_type = get_gesw_ptype(port_num);

    if (port_type == 0) {
        printf("%s: port_num unknown. No line loopback is set\n",
	       __FUNCTION__);
	return(FAIL);
    }

    if (port_type != BCM_PTYPE_XE) {
        if (onoff) {
	    return(bcm_gesw_ge_lpbk_set(bcm_uid, 
					port_num, 
					GESW_LINE_LOOPBACK));
	}
	else {
	    return(bcm_gesw_ge_lpbk_set(bcm_uid, 
					port_num, 
					GESW_LOOPBACK_NONE));
	}
    }
    else {
        if (onoff) {
	    return(bcm_gesw_xaui_lpbk_set(bcm_uid, 
					  port_num, 
					  GESW_LINE_LOOPBACK));
	}
	else {
	    return(bcm_gesw_xaui_lpbk_set(bcm_uid, 
					  port_num, 
					  GESW_LOOPBACK_NONE));
	}
    }
}

/*------------------------------------------------------------------
 *
 * Function: get_gesw_mac_loopback
 * Return the state of the mac loopback setting of the port
 *
 * Input: port_num - the GE port number
 *
 * Return: 1 for set, 0 for clear, -1 error
 *
 *------------------------------------------------------------------
 */
int get_gesw_mac_loopback(int port_num)
{
    int state = -1;
    int port_type = get_gesw_ptype(port_num);

    if (port_type == 0) {
        printf("%s: port_num unknown. No mac loopback get\n",
	       __FUNCTION__);
    }
    else if (port_type != BCM_PTYPE_XE) {
        bcm_gesw_ge_lpbk_get(bcm_uid, port_num, GESW_MAC_LOOPBACK, &state);
    }
    else {
        bcm_gesw_xaui_lpbk_get(bcm_uid, port_num, GESW_MAC_LOOPBACK, &state);
    }
    return(state);
}

/*------------------------------------------------------------------
 *
 * Function: get_gesw_line_loopback
 * Return the state of the line loopback setting of the port
 *
 * Input: port_num - the GE port number
 *
 * Return: 1 for set, 0 for clear, -1 error
 *
 *------------------------------------------------------------------
 */
int get_gesw_line_loopback(int port_num)
{
    int state = -1;
    int port_type = get_gesw_ptype(port_num);

    if (port_type == 0) {
        printf("%s: port_num unknown. No line loopback get\n",
               __FUNCTION__);
    }
    else if (port_type != BCM_PTYPE_XE) {
        bcm_gesw_ge_lpbk_get(bcm_uid, port_num, GESW_LINE_LOOPBACK, &state);
    }
    else {
        bcm_gesw_xaui_lpbk_get(bcm_uid, port_num, GESW_LINE_LOOPBACK, &state);
    }
    return(state);
}

/*------------------------------------------------------------------
 *
 * Function: gesw_port_query
 * This is a general query funstion to get the port number of the GE
 * switch.
 *
 * Input: 
 * rtn_port - pointer of buffer for returned port value
 * rtn_port_str - pointer of buffer for returned port str
 *
 * Return: void
 *
 *------------------------------------------------------------------
 */
void gesw_port_query(int *rtn_port, char *rtn_port_str)
{
    uchar iotype = 's'; /* ngio type */
    int port, slot, tgt_device, local_port, minslot, maxslot;
    char query[128];
    char port_str[8];
    char *name_p;

    iotype = getc_answer("Port connected to SM, WIC, VM, DP, or CP (enter s, w, v, d, or c)", "swvdc", 's');

    switch (iotype) {
    case 's': /* SM */
        minslot = NGSM1_SLOT;

	if (is_overlord() || is_utah() || is_triton() || is_proteus()) {
            maxslot = NGSM2_SLOT;
	}
	else if (is_neptune() || is_vg450()) {
            maxslot = NGSM3_SLOT;
	}
	else if (is_sword() || is_neso()) {
            maxslot = NGSM1_SLOT;
	}
	else {
	    printf("SM is not supported on this platform.\n");
	    *rtn_port = 0;
	    return;
	}

	tgt_device = TGT_DEV_NGSM;

	sprintf(query, "NGSM slot number (%d to %d)", minslot, maxslot);
	slot = getdec_answer(query, minslot, minslot, maxslot);

	sprintf(query, "NGSM local port (0 to %d for ge0,1,xaui)", MAX_NGIO_LOCAL_ETH_PORTS-1);
	local_port = getdec_answer(query, 0, 0, MAX_NGIO_LOCAL_ETH_PORTS-1);
	break;
    case 'w': /* WIC */
        minslot = NGWIC1_SLOT;

	if (is_overlord() || is_juno() || is_utah() || is_triton() || is_proteus() || is_neptune() || is_vg450()) {
	    maxslot = NGWIC3_SLOT;
	}
	else { /* sword, dagger, neso */
	    maxslot = NGWIC2_SLOT;
	}

	tgt_device = TGT_DEV_NGWIC;

	sprintf(query, "NGWIC slot number (%d to %d)", minslot, maxslot);
	slot = getdec_answer(query, minslot, minslot, maxslot);

	sprintf(query, "NGWIC local port (0 to %d for ge0,1)", MAX_NGIO_LOCAL_GE_PORTS-1);
	local_port = getdec_answer(query, 0, 0, MAX_NGIO_LOCAL_GE_PORTS-1);
	break;
    case 'v':
        if (is_ntpn_machines() || is_vg450()) {
	    printf("ISC module is not supported on this platform.\n");
	    *rtn_port = 0;
	    return;
	}

        minslot = maxslot = NGVM1_SLOT;      
	tgt_device = TGT_DEV_NGVM;
	slot = minslot;

	sprintf(query, "NGVM local port (0 to %d for ge0,1)", MAX_NGIO_LOCAL_GE_PORTS-1);
	local_port = getdec_answer(query, 0, 0, MAX_NGIO_LOCAL_GE_PORTS-1);
	break;
    case 'c':
	slot = 0;
	tgt_device = TGT_DEV_CPU;
        minslot = CPU_SGMII_PORT1;

	if (is_overlord() || is_juno()) {
	    maxslot = CPU_SGMII_PORT3;
	}
	else if (is_usd_machines()) {
	    minslot = CPU_SGMII_PORT3;
	    maxslot = CPU_SGMII_PORT3;
	}
 	if (is_ntpn_machines() || is_vg450()) {
	    maxslot = CPU_SGMII_PORT2;
	}

	sprintf(query, "CPU SGMII port number (%d to %d)", minslot, maxslot);
	local_port = getdec_answer(query, minslot, minslot, maxslot);
	break;
    case 'd':
        minslot = maxslot = 1;      
	tgt_device = TGT_DEV_DP;
	slot = minslot;
	local_port = 0;
	break;
    default:
	printf("No such IO type.\n");
	*rtn_port = 0;
	return;
    }

    port = ovld_get_ge_sw_port_num(slot, tgt_device, local_port);

    name_p = get_gesw_pname(port);
    if (name_p != 0) {
        sprintf(port_str, "%s", name_p);
    }

    *rtn_port = port;
    sprintf(rtn_port_str, "%s", port_str);
}

/*------------------------------------------------------------------
 *
 * Function: loopback_util
 * This utility is used to set or clear either the line or MAC
 * loopback of the GE switch port.
 *
 * Input: void
 *
 * Return: void
 *
 *------------------------------------------------------------------
 */
void loopback_util(void)
{
    int port, onoff;
    char port_str[8];
    uchar lptype = 'l';  /* loopback type */
    uchar cin = 'c'; /* set or clear */

    gesw_port_query(&port, port_str);
    if (port == 0) {
        return;
    }

    lptype = getc_answer("Line or MAC loopback (enter l or m)", "lm", 'l');

    cin = getc_answer("Set or clear line loopback (enter s or c)", "sc", 'c');
    onoff = (cin == 's');

    if (lptype == 'l') {
        set_gesw_line_loopback(port, onoff);
	printf("GESW port %s line loopacket is set to %d\n",
	       port_str, get_gesw_line_loopback(port));
    }
    else {
        set_gesw_mac_loopback(port, onoff);
	printf("GESW port %s mac loopacket is set to %d\n",
	       port_str, get_gesw_mac_loopback(port));
    }
}

/*------------------------------------------------------------------
 *
 * Function: set_canis_loopback
 * This function is used by Canis diag to set the line loopback of the
 * GE swtich port that it connect to for loopback testing.
 *
 * Input: 
 * slot - NGIO SM slot number
 * mode - TRUE for setting the line loopback mode
 *
 * Return: void
 *
 *------------------------------------------------------------------
 */
void set_canis_loopback(int slot, int mode)
{
    int port, onoff, local_port, tgt_device;
    char port_str[8];
    char *name_p;

    local_port = 1; /* port will be 1 for bcm5719 P1GE */
    tgt_device = TGT_DEV_NGSM;

    port = ovld_get_ge_sw_port_num(slot, tgt_device, local_port);

    name_p = get_gesw_pname(port);
    if (name_p != 0) {
        sprintf(port_str, "%s", name_p);
    }

    onoff = (mode == TRUE); /* set or clear */

    set_gesw_line_loopback(port, onoff);
	printf("GESW port %s line loopacket is set to %d\n",
	       port_str, get_gesw_line_loopback(port));

}

/*------------------------------------------------------------------
 *
 * Function: port_tx_util
 * This utility use the tx command provided in the BCM shell to
 * transmit a packet out of a port. This is a very useful utility to
 * help NGIO module diag development.
 *
 * Input: void
 *
 * Return: void
 *
 *------------------------------------------------------------------
 */
void port_tx_util(void)
{
    int unit = bcm_uid, port;
    char port_str[8];
    uint pkt_cnt, pkt_len;
    uint pattern;
    char cmd[64];
    mac_addr_t mac_da = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char mac_da_str[32];

    gesw_port_query(&port, port_str);
    if (port == 0) {
        return;
    }

    pkt_cnt = getdec_answer("Packet count", 1,1,100);
    pkt_len = getdec_answer("Packet length", 68, 68, 1518);
    pattern = gethex_answer("Pattern", 0xa5a5a5a5, 0, 0xffffffff);

    mac_addr_query("Default MAC_DA is (ff:ff:ff:ff:ff:ff)\n",
		   &mac_da);
    sprintf(mac_da_str, "%02x:%02x:%02x:%02x:%02x:%02x",
	    (uchar)mac_da[0], (uchar)mac_da[1], (uchar)mac_da[2], 
	    (uchar)mac_da[3], (uchar)mac_da[4], (uchar)mac_da[5]);

    sprintf(cmd, "tx %d U=yes PBM=%s UBM=%s Length=%d Pattern=%#.8x DM=%s;",
	    pkt_cnt, port_str, port_str, pkt_len, pattern, mac_da_str);
    
    exec_bcm_shell_cmd(unit, cmd, TRUE);
}

/*------------------------------------------------------------------
 *
 * Function: show_counter_util
 * This funciton just provide the instruction how to use the BCM shell
 * to display the counter and L2 information
 *
 * Input: void
 *
 * Return: void
 *
 *------------------------------------------------------------------
 */
void show_counter_util(void)
{
    printf("Invoke the BCM shell and use the following example to access the chip\n");
    printf("show c ge0-ge9 (to dump the changed counters)\n");
    printf("vlan show (to list the existing vlans)\n");
    printf("l2 show (to list the existing L2 info)\n");
    printf("help (list all the commands)\n");
    printf("quit (exit the BCM shell)\n");
}


/******** History ******** 
$Log: bcm_gesw_tests.c,v $
Revision 1.9  2018/05/18 09:24:50  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.8  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.7.32.7  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.7.32.6  2017/11/27 06:08:40  leschen
Initial check in to support VG450.

Revision 1.7.32.5  2017/02/22 00:58:04  ptong
Fixed the BCM TX util

Revision 1.7.32.4  2016/10/27 21:49:40  ptong
Completed BCM self tests on Neptune

Revision 1.7.32.3  2016/10/06 01:33:25  ptong
Init Greyhound switch properly for Neptune

Revision 1.7.32.2  2016/06/22 21:25:35  ptong
Add temporary util for neptune bringup

Revision 1.7.32.1  2016/06/18 00:43:54  ptong
Support Neptune

Revision 1.7  2014/08/22 00:27:16  ptong
Fixed line loopback set up for Greyhound 10gKR ports

Revision 1.6  2014/04/02 21:47:51  ptong
Use port mapping between BCM and platform to support the new BCM chip

Revision 1.5  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.4  2013/09/09 05:58:01  ptong
Replace set_cavecreek_sgmii_for_ge_test with set_ctrl_plane_sgmii_for_ge_test. Replace #ifdef UTAH with is_overlord() for platform dependent code

Revision 1.3  2013/09/06 22:56:19  ptong
Support Utah with ctrl_plane_sgmii_macsa_declare

Revision 1.2  2013/07/16 01:32:30  ptong
Make file sharable between overlord and utah

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.16  2013/01/08 01:18:14  ptong
Added function headers

Revision 1.15  2012/09/07 22:49:34  ptong
Minor fix on a printf and code clean-up

Revision 1.14  2012/08/23 22:44:24  ptong
Replace the MAC loopback test with internal PHY loopback

Revision 1.13  2012/06/27 07:22:15  hondwang
Add slot variable for set canis loopback function

Revision 1.12  2012/06/26 12:43:56  hondwang
add Canis GE switch line loopback function

Revision 1.11  2012/06/07 00:09:47  palin2
Clean up compiler warnings.

Revision 1.10  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.9  2012/06/02 00:29:50  ptong
Change ge0-ge2 to an=on

Revision 1.8  2012/05/16 07:20:51  ptong
Add GESW port TX util

Revision 1.7  2012/05/10 21:59:58  ptong
Return to not use miim_intr_enable=0

Revision 1.6  2012/05/09 18:52:58  ptong
BCM FAE recommended to use miim_intr_enable=0 to avoid the link scan intr time out issue

Revision 1.5  2012/05/04 23:48:57  ptong
Improve BCM init process and test message printing

Revision 1.4  2012/04/28 00:40:40  ptong
Change port setup to not auto-neg

Revision 1.3  2012/04/03 01:46:18  ptong
Replace loading rc.soc with ovld_gesw_init() to avoid re-init the bcm drivers

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
