/* $Id: octeon_test.c,v 1.10 2018/05/18 09:24:51 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/octeon_test.c,v $
 *------------------------------------------------------------------
 * octeon_test.c - Octeon tests on overlord platform
 *
 * Paul Tong, July 2011
 *
 * Copyright (c) 2014 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "common.h"
#include "error.h"
#include "proto.h"
#include "menu.h"
#include "nvmonvars.h"
#include "linux_api.h"
#include "../cavium/host2dp_mbox.h"
#include "bcm_gesw_defs.h"
#include "octeon_test.h"
#include "plat_defs.h"
#include "linux_ntwk.h"
#include "dash_fpga.h"
#include "platform_cookie.h"
#include "../timingcard/vm_timingcard_zl3036x_lib.h"
#include "linux_pciutils.h"
/* can be remove afrer baud rate change back to 9600 */
#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <pthread.h>
#include "types.h"
#include "common.h"
#include "proto.h"
#include "error.h"
#include "linux_api.h"
/*end of  can be remove afrer baud rate change back to 9600 */

#undef DEBUG

#define F_GRP	     (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E	     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL	     (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

extern int host_mbox_init(void);
extern long clock_direction_lib(uint);
extern int timingcard_init_seq(void);
extern void clear_timingcard_init_flag(void);

static int host_chk_dp_diag_up(int);
static int run_dp_main_test(int);
static int run_dp_hello_test(int);
static int dp_uart_loopback_test(void);
static int host_probe_dp(void);
static int nc_cmd_run_oct_diag(int);
static int run_dp_test(uint, char *, int);
static int discover_octeon(ushort, ushort);
static int ovld_check_cavium_eeprom (void);
static int nep_check_cavium_eeprom (void);
int timingcard_clk_trig_verify_test(int);

extern unsigned int brd_ver;
boolean cavium_init_3036x = FALSE;

/* 
 * Sub Menu used for Octeon tests.
 */
submenu_xtable_t octeon_tests_submenu_table[] = {
    /* DP MFG diag is bundled in the DP Linux rootfs.
     * The nc command is 
     * "nc -l -l -p 50 -e /ovld-diag/cvmx_diag 1"
     */
    {"Run DP main test from CP via nc command",
     (type_t(*)())run_dp_main_test,       0,    F_ALL_E,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"(For debug use) Switch to DP console to show DP test menu",
     (type_t(*)())switch_to_dp_console,   0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"(For debug use) Boot Octeon Linux and diag from CP",
     (type_t(*)())oct_remote_bootlinux,   0,	0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"(For debug use) Boot Octeon uboot",
     (type_t(*)())oct_remote_bootuboot,   0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"(For debug use) Check Octeon DDR",
     (type_t(*)())oct_remote_memcheck,    0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"(For debug use) Hello message to DP test", 
     (type_t(*)())run_dp_hello_test,      0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"(For debug use) DP UART(/dev/ttyDASH7) loopback test", 
     (type_t(*)())dp_uart_loopback_test,  0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"Timingcard CLK/TRIG verification", 
     (type_t(*)())timingcard_clk_trig_verify_test,      0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

#if DEBUG /* may need these later */
    /* DP lab diag is mounted on the pxe server.
     * The nc command is 
     * "nc -l -l -p 51 -e /linux_diag/cavium/cvmx_diag 1"
     */
    {"(For LAB only) Run DP main test from CP via nc command",
     (type_t(*)())run_dp_main_test,   1,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"Remote check Octeon core 0",
     (type_t(*)())oct_remote_core,   0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},

    {"Remote reset Octeon",
     (type_t(*)())oct_remote_reset,   0,    0,
     (type_t(*)())0,	0,
     (type_t(*)())0,   	0},
#endif

};

#define OCTEON_TESTS_SUBMENU_TABLE_SIZE (sizeof(octeon_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t octeon_tests_primary_items[OCTEON_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t octeon_tests_secondary_items[OCTEON_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t octeon_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    octeon_tests_primary_items,
};
menuinfo_t *octeon_submenup = &octeon_subtest_menu;

/*
 * Function: discover_octeon
 * Check if the Cavium CPU has been discovered in the host PCI bus
 *
 * Input:
 * venid - vendor ID of the CN66XX CPU
 * devid - device ID of the CN66XX CPU
 *
 * Return: PASS/FAIL
 */
static int discover_octeon (ushort venid, ushort devid)
{
    struct pci_dev *dev;

    dev = diag_pci_get_device(venid, devid, NULL);
    if (dev == NULL) {
        return (FAIL);
    } else {
        dev = NULL;
        return (PASS);
    }
}

/*
 * Function: octeon_tests
 * Test entry function for the Octeon based data plane.
 *
 * Input:
 * run_tests - flag to either run the tests or enter the submenu
 *
 * Return: PASS/FAIL
 */
int octeon_tests (boolean run_tests)
{
    unsigned int vid, did;
    char buf[16];
    int rv = PASS;

    if (is_ntpn_machines() || is_vg450()) {
        vid = CVMX_VENID;
        did = CN7300_DEVID;
        sprintf(buf, "CN73XX");
    } else {
        vid = CVMX_VENID;
        did = CN6600_DEVID;
        sprintf(buf, "CN66XX");
    }

    if (discover_octeon(vid, did) != PASS) {
        cterr('f',0,"ERROR: Octeon %s 0x%08x not discovered\n",
               buf, ((vid << 16) | did));
        return(FAIL);
    }

    build_primary_submenu(octeon_tests_submenu_table, OCTEON_TESTS_SUBMENU_TABLE_SIZE,
			    "Data Plane Octeon", &octeon_submenup);
    build_secondary_submenu(octeon_tests_submenu_table,
			    OCTEON_TESTS_SUBMENU_TABLE_SIZE,
			    octeon_tests_secondary_items);
    if (is_overlord() || is_juno()) {
        if (brd_ver >= OVLD_PILOT_REV) {
            ovld_check_cavium_eeprom();
        } else {
            printf("\nSystem is earlier than pilot version, \
                      cavium eeprom check is not supported!!\n");
        }
    }
    else if (is_ntpn_machines() || is_vg450()) {
        nep_check_cavium_eeprom();
    }

    if (run_tests) {
        rv = run_dp_main_test(0);
    }
    else {
	rv = PASS;
        menu(&octeon_subtest_menu, octeon_tests_secondary_items, '\0');
    }

    return(rv);
}

/*
 * Function: is_dp_lnx_up
 * Check if data plane CPU Linux is up by sending ping packet via the GESW 
 *
 * Input: verbose - flag to control message printing
 *
 * Return: PASS/FAIL
 */
int is_dp_lnx_up (int verbose)
{
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline;
    char *result_file;
    FILE *fp;
    int tx_cnt, rx_cnt;
    int rv = FAIL;

    pktcnt = 2;
    deadline = 5;
    result_file = "dp_ping_result";

    fp = fopen(result_file, "r");
    if (fp != NULL) {
        fclose(fp);
	sprintf(cmdbuf, "rm %s", result_file);
	system(cmdbuf);
    }

    sprintf(cmdbuf, "ping -c %d -w %d -I eth1 %s > %s",
	    pktcnt, deadline, OCTEON_XAUI0_IP_ADDR,
	    result_file);
    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        if (verbose) {
	    printf("HOST: Ping DP %s was not created\n", result_file);
	}
	goto  is_dp_lnx_up_exit;
    }
    
    /* Check the result
     */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);

	if (strstr(buf, "received") != NULL) {
#if DEBUG
	    printf("HOST: Ping DP result: %s", buf);
#endif
	    break;
	}
    }
    fclose(fp);
    sprintf(cmdbuf, "rm %s", result_file);
    system(cmdbuf);

    /* Read the string 
     */
    sscanf(buf, "%d %s %s %d", &tx_cnt, dum_char, dum_char, &rx_cnt);

    if (rx_cnt < pktcnt) {
        if (verbose) {
	    printf("HOST: Ping DP packet count mismatch. "
		   "Expected= %d, Actual: tx= %d rx= %d\n",
		   pktcnt, tx_cnt, rx_cnt);
	}
	goto  is_dp_lnx_up_exit;
    }

    rv = PASS;

 is_dp_lnx_up_exit:

    if (rv == PASS) {
        if (verbose) {
	    printf("HOST: Ping DP via GESW passed.\n");
	}
	return(TRUE);
    }
    else {
        if (verbose) {
	    printf("HOST: Ping DP via GESW failed.\n");
	}
	return(FALSE);
    }
}

/*-----------------------------------------------------------------*/
/*----  Octeon remote PCIe utilities  -----------------------------*/

#define REMOTE_SCRIPT_PATH  "./oct-remote-script/"

/*
 * Function: run_remote_script
 * Use the Linux "system" command to execute a script 
 *
 * Input:
 * script_path - The directory path to the script
 * script_name - The script name
 * arg_str - argument string to pass to the script
 *
 * Return: PASS/FAIL
 */
int run_remote_script (char *script_path, char *script_name, char *arg_str)
{
    char cmdbuf[128];
    int rv;

    if (arg_str == NULL) {
      sprintf(cmdbuf, "%s%s\n", script_path, script_name);
    }
    else {
      sprintf(cmdbuf, "%s%s %s\n", script_path, script_name, arg_str);
    }

    rv = system (cmdbuf);
    return(rv);
}

/*
 * Function: oct_remote_bootlinux
 * Run the Octeon remote script to boot uboot, Linux, and the data
 * plane diag images on the Octeon CPU from the host CPU via the PCIe
 * interface. If any of those 3 images is not already in the
 * /o2-diag/cavium directory, the host will download it from
 * the TFTP directory specified in the TFTPDIR environment var.
 *
 * Input: 
 * flag - To indicate if the console switch util is available
 *
 * Return: PASS/FAIL
 */
int oct_remote_bootlinux (int flag)
{
    int rv;
    int plat_sku, arg_val;
    char arg_str[16];
    int loopcnt, looplimit;
    char uname[32], iname[32], kname[32], path[128], buf1[128];
    char buf2[128], cmd[32];
    size_t size = 0;

    if(!chk_plat_sku(&plat_sku)) {
        cterr('f',0,"HOST: SKU number on FPGA and cookie are different.\n");
        printf("Using cookie util to program MB cookie first\n");
        return(FAIL);
    }

    if (plat_sku == SKU_INVALID) {
        printf("\nWarning: In %s, platform SKU is unknown. Assume it is ISR4451.\n", __FUNCTION__);
        plat_sku = SKU_ISR4451;
    }

    sprintf(path, "cavium/");

    /* 0 for Neptune - 16 cores for cavium */
    if (is_neptune() || is_vg450()) {
       if (is_vg450()) {
           plat_sku = SKU_VG450;  
       } else {
           plat_sku = SKU_ISR4461;  /* fixme, will have sku in the future */
       }
       arg_val = 0;
       sprintf(uname, "u-boot-octeon_cisco_nep.bin");
       sprintf(iname, "nepcvm_diag");
       sprintf(kname, "nepcvm_vmlinux.64");
       sprintf(buf1, "/nep-diag/%s", path);
       sprintf(cmd, "oct_bootlinux");
    } else if (is_triton()) { /* 10 cores */
       plat_sku = SKU_ISR4452E;
       arg_val = 1;
       sprintf(uname, "u-boot-octeon_cisco_nep.bin");
       sprintf(iname, "nepcvm_diag");
       sprintf(kname, "nepcvm_vmlinux.64");
       sprintf(buf1, "/nep-diag/%s", path);
       sprintf(cmd, "oct_bootlinux");
    } else if (is_neso() || is_proteus()) { /* proteus and neso, 6 cores */
       if (is_proteus()) {
           plat_sku = SKU_ISR4452;
       } else {
           /* NESO */
           plat_sku = SKU_ISR4432;
       } 
       arg_val = 2;
       sprintf(uname, "u-boot-octeon_cisco_nep.bin");
       sprintf(iname, "nepcvm_diag");
       sprintf(kname, "nepcvm_vmlinux.64");
       sprintf(buf1, "/nep-diag/%s", path);
       sprintf(cmd, "oct_bootlinux");
    } else if (is_overlord()) {
       arg_val = 0; /* 10 cores  */
       sprintf(uname, "u-boot-octeon_cisco_ovld.bin");
       sprintf(iname, "o2cvm_lnx");
       sprintf(kname, "vmlinux.64");
       sprintf(buf1, "/o2-diag/%s", path);
       sprintf(cmd, "oct_bootlinux");
    } else if (is_juno()) {
       arg_val = 1;  /* 6 cores */
       sprintf(uname, "u-boot-octeon_cisco_ovld.bin");
       sprintf(iname, "o2cvm_lnx");
       sprintf(kname, "vmlinux.64");
       sprintf(buf1, "/o2-diag/%s", path);
       sprintf(cmd, "oct_bootlinux");
    } else {
       cterr('f',0,"Unknown platform type, not O2/Juno and Neptune series.\n");
       return (FAILED);
    }

    if (is_ntpn_machines() || is_vg450()) { 
        /* neptune bundle cvm uboot and kernel into x86 rootfs, but
         * let us still call tftp_get to check the existence of uboot and kernel.
	 */
        sprintf(buf2, "%s%s", buf1, uname);
        printf("HOST: Check Cavium u-boot is bundled in host rfs.\n");
	if ( ! (file_exist(buf2, &size) && (size > 0))) {
            cterr('f', 0, "Cavium u-boot is missing in the host rfs");
            return (FAILED);
	}

        sprintf(buf2, "%s%s", buf1, kname);
        printf("HOST: Check Cavium vmlinux.64 is bundled in host rfs.\n");
	if ( ! (file_exist(buf2, &size) && (size > 0))) {
            cterr('f', 0, "Cavium vmlinux is missing in the host rfs");
            return (FAILED);
        }
    } else { 
        sprintf(buf2, "%s%s", buf1, uname);
        printf("HOST: TFTP Cavium u-boot to host rfs if not already done.\n");
        if (tftp_get(0, uname, 0, buf2, 1) < 0) {
            cterr('f', 0, "Failed to tftp download Cavium u-boot to host rfs");
            return (FAILED);
        }

        sprintf(buf2, "%s%s", buf1, kname);
        printf("HOST: TFTP Cavium vmlinux.64 to host rfs if not already done.\n");
        if (tftp_get(0, kname, 0, buf2, 1) < 0) {
            cterr('f', 0, "Failed to tftp download Cavium vmlinux to host rfs");
            return (FAILED);
        }
    }
    
    sprintf(buf2, "%s%s", buf1, iname);
    printf("HOST: TFTP data plane diag image from network if not already done.\n");
    if (tftp_get(0, iname, 0, buf2, 1) < 0) {
        cterr('f', 0, "Failed to tftp download nepcvm_diag to host rfs");
        return (FAILED);
    }

    printf("\nList Cavium uboot, kernel and diag image \n"); 

    sprintf(buf2, "ls -l --color=never %s%s", buf1, uname);
    system(buf2);
    sprintf(buf2, "ls -l --color=never %s%s", buf1, kname);
    system(buf2);
    sprintf(buf2, "ls -l --color=never %s%s", buf1, iname);
    system(buf2);

    printf("\nHOST: Remote boot DP CPU ...\n");
    sprintf(arg_str, "%d", arg_val);
    rv = run_remote_script(REMOTE_SCRIPT_PATH, cmd, arg_str);
    if (rv == 0) {
        printf("\nHOST: Waiting for DP Linux to come up...\n");
	loopcnt = 0;
	looplimit = 10;
	while (!is_dp_lnx_up(0) && (loopcnt < looplimit)) {
	    msleep(1);
	    loopcnt++;
	}

	if (!is_dp_lnx_up(1)) {
	     printf("Possible causes of problem:\n"
		    "1. Cavium DRAM may be bad. (Run Check Octeon DDR test in submenu)\n"
		    "2. Cavium CPU may be bad or has soldering problem.\n"
		    "3. Intel to Cavium PCIe interface may be bad\n");
	     cterr('f',0,"HOST: DP Linux boot error.\n");
	     return(FAIL);
	}
	else {
	    printf("HOST: DP Linux is up!\n");
	}

	/* Init mail box to communicate with DP diag
	 */
	host_mbox_init();
	if (host_chk_dp_diag_up(plat_sku) == FAIL) {
	    cterr('f',0,"HOST: DP diag boot error.\n");
	    return(FAIL);
	}
	else {
	    printf("\nHOST: DP diag is up\n");
	    if (flag == 0) {
	        printf("This diag submenu provides util to switch to the DP console.\n");
	    }
	    /* Ethernet drivers need some time to settle
	     */
	    sleep(1);
	    return(PASS);
	}
    }
    else {
        cterr('f',0,"HOST: Boot Octeon via PCIe script failed\n");
	return(FAIL);
    }
}

/*
 * Function: oct_remote_memcheck
 * Run the Octeon remote script to check the first 8MB of the Octeon
 * DRAM from the host CPU via the PCIe interface. This is provided as
 * a utility in the diag menu as a quick sanity check of the data
 * plane DRAM
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int oct_remote_memcheck (void)
{
    int rv;
    char cmd[32]; 

    if (is_overlord() || is_juno()) {  /* remote_memcheck test 8MB */
        sprintf(cmd, "oct_remote_memcheck"); 
    } else { /* Neptune testing  100MB */
        sprintf(cmd, "oct_ddr 100"); 
    }
    printf("HOST: Remote check DP CPU low memory ... (DP CPU halted)\n");
    rv = run_remote_script(REMOTE_SCRIPT_PATH, cmd, NULL);
    printf("HOST: Remote soft reset DP CPU ... wait\n");
    rv += run_remote_script(REMOTE_SCRIPT_PATH, "oct_remote_reset", NULL);
    printf("HOST: Done\n");
    return(rv);
}

/*
 * Function: oct_remote_bootuboot
 * Run the Octeon remote script to boot uboot on the Octeon CPU from the
 * host CPU via the PCIe interface. If the uboot image is not already
 * in the /o2-diag/cavium directory, the host will download it from
 * the TFTP directory specified in the TFTPDIR environment var.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int oct_remote_bootuboot (void)
{
    int rv;
 
    printf("HOST: TFTP Cavium u-boot to host rfs if not already done.\n");
    if (is_overlord() || is_juno()) { 
        if (tftp_get(0, "u-boot-octeon_cisco_ovld.bin", 0, 
	    "/o2-diag/cavium/u-boot-octeon_cisco_ovld.bin", 1) < 0) {
	    cterr('f', 0, "Failed to tftp download Cavium u-boot to host rfs");
	    return (FAILED);
        }
    } else if (is_neptune() || is_triton() || is_neso() || is_vg450()) {
        printf("Neptune Cavium Uboot is already bundle into rootfs \n"); 
        printf("Prepare to Boot Uboot ...\n"); 
    } else { 
        cterr('f', 0, "Platform is not O2/Juno/Neptune/Triton/Neso.. Abort"); 
        return (FAILED); 
    }

    rv = run_remote_script(REMOTE_SCRIPT_PATH, "oct_bootuboot", NULL);
    if (rv != 0) {
        printf("Possible causes of problem:\n"
	       "1. Cavium DRAM may be bad. (Run Check Octeon DDR test in submenu)\n"
	       "2. Cavium CPU may be bad or has soldering problem.\n"
	       "3. Intel to Cavium PCIe interface may be bad\n");
        cterr('f',0,"Boot Octeon via PCIe failed\n");
	return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: oct_remote_core
 * Run the Octeon remote script to display the Octeon CPU core 0 from the
 * host CPU via the PCIe interface.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int oct_remote_core (void)
{
    return(run_remote_script(REMOTE_SCRIPT_PATH, "oct_remote_core", "0"));
}

/*
 * Function: oct_remote_reset
 * Run the Octeon remote script to reset the Octeon CPU from the
 * host CPU via the PCIe interface.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int oct_remote_reset (void)
{
    return(run_remote_script(REMOTE_SCRIPT_PATH, "oct_remote_reset", NULL));
}

/*
 * Function: switch_to_dp_console
 * A utility provide in the diag menu to allow user to switch the
 * system constol from the host console port to the data plane console
 * port. The data plance console is connected to UART 7 in the
 * DASH FPGA.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int switch_to_dp_console (void)
{   
    /*  for using picocom and nanocom to monitor cavium console
     *  system("./nanocom /dev/ttyDASH7 -b9600 -pn -s1 -d8 -fn -en");
     *  system("./picocom /dev/ttyDASH7");
     */

    /* flush the buffer before executing picocom */
    char disp[64];
    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);

    if (is_overlord() || is_juno()) { 
        sprintf(disp, "picocom /dev/ttyDASH7");
    } else { 
        sprintf(disp, "picocom /dev/ttyDASH7 -b115200");
    }
    printf("\n %s\n\n\n", disp);
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(disp);

    return (PASSED);
}

/*
 * Function: host_chk_dp_diag_up
 * The host checks that if the data plane has boot up Linux and
 * the data plane diagnostics application. The data plane diag
 * returns a message which also indicate if the correct Cavium
 * CPU is installed on the mother board.
 *
 * Input:
 * plat_sku - SKU value for either Overlord or Juno. The value can
 *            be SKU_ISR4451, SKU_ISR4431
 *
 * Return: PASS/FAIL
 */
static int host_chk_dp_diag_up (int plat_sku)
{
    mbox_t *mbxp = host_in_mbxp;
    uint msg_ctl, expected_msg;
    int loopcnt, looplimit;

    /* Wait for mail box not empty
     */
    loopcnt = 0;
    looplimit = 30; /* for 30 second count */
    while (is_mbox_empty(mbxp) && (loopcnt < looplimit)) {
        sleep(1);
	loopcnt++;
    }

    if (loopcnt >= looplimit) {
        if (is_mbox_empty(mbxp)) {
	    printf("HOST: Mail message timed out. DP diag failed to come up on time.\n");
	    return(FAIL);
	}
    }

    msg_ctl = get_msg();
    ack_msg(); /* clear the valid bit */

    if ((plat_sku == SKU_ISR4461) || (plat_sku == SKU_VG450)) {  /* neptune/vg450 */
        expected_msg = (MBOX_MSG_DP_CN7260R | MSG_VALID);
    } else if (plat_sku == SKU_ISR4452E) { /* triton */
        expected_msg = (MBOX_MSG_DP_CN7245R | MSG_VALID);
    } else if (plat_sku == SKU_ISR4452) { /* proteus */
        expected_msg = (MBOX_MSG_DP_CN7235R | MSG_VALID);
    } else if (plat_sku == SKU_ISR4432) { /* neso */
        expected_msg = (MBOX_MSG_DP_CN7235R | MSG_VALID);
    } else if (plat_sku == SKU_ISR4451) { /* O2 */
        expected_msg = (MBOX_MSG_DP_CN6645 | MSG_VALID);
    } else {  /* Juno */
        expected_msg = (MBOX_MSG_DP_CN6635 | MSG_VALID);
    }

    if (msg_ctl == expected_msg) {
	printf("HOST: DP reported matching CPU of ");
        if (plat_sku == SKU_ISR4461) {
            printf("CN7260R for SKU Neptune \n");
        } else if (plat_sku == SKU_VG450) {
            printf("CN7260R for SKU VG450 \n");
        } else if (plat_sku == SKU_ISR4452E) {
            printf("CN7245 for SKU Triton\n");
        } else if (plat_sku == SKU_ISR4452) {
            printf("CN7235 for SKU Proteus\n");
        } else if (plat_sku == SKU_ISR4432) {
            printf("CN7235 for SKU Neso \n");
        } else if (plat_sku == SKU_ISR4451) {
	    printf("CN6645 for SKU Overlord\n");
	} else {
	    printf("CN6635 for SKU Juno\n");
	}
        return(PASS);
    } else {
        printf("HOST: DP reported unknown CPU for ");
        if (plat_sku == SKU_ISR4461) {
            printf("SKU Neptune \n");
        } else if (plat_sku == SKU_VG450) {
            printf("SKU VG450 \n");
        } else if (plat_sku == SKU_ISR4452E) {
            printf("SKU Triton\n");
        } else if (plat_sku == SKU_ISR4452) {
            printf("SKU Proteus\n");
        } else if (plat_sku == SKU_ISR4432) {
            printf("SKU Neso \n");
        } else if (plat_sku == SKU_ISR4451) {
            printf("SKU Overlord\n");
        } else {
            printf("SKU Juno\n");
        }

	printf("HOST: Expected valid msg (%#.8x), received (%#.8x)\n",
	       expected_msg, msg_ctl);
	return(FAIL);
    }
}

/*-----------------------------------------------------------------*/
/*----  nc command to run cavium diag from Intel  -----------------*/

/*
 * Function: nc_cmd_run_oct_diag
 * Use the Linux "system" command to send a nc client request to
 * the data plance nc server.
 *
 * Input:
 * port - the port numner of the nc server
 *
 * Return: PASS/FAIL
 */
static int nc_cmd_run_oct_diag (int port)
{
    char cmdbuf[128];

    sprintf(cmdbuf, "nc %s %d\n", OCTEON_XAUI0_IP_ADDR, port);
    printf("HOST: nc command: %s\n", cmdbuf);

    return(system(cmdbuf));
}

/*
 * Function: host_chk_dp_test_result
 * Host check the test result from data plane by reading the
 * message returned in the mailbox by data plane.
 *
 * Input:
 * test_msg - The test message which result is being checked
 *
 * Return: PASS/FAIL
 */
static int host_chk_dp_test_result (uint test_msg)
{
    mbox_t *mbxp = host_in_mbxp;
    uint msg_ctl, msg_id;

    if (is_mbox_empty(mbxp)) {
        printf("HOST: Mail message timed out. DP did not return test result.\n");
	return(FAIL);
    }

    msg_ctl = get_msg();
    msg_id = get_msg_id(msg_ctl);
    ack_msg(); /* clear the valid bit */

    if ((msg_id == test_msg) && (is_msg_pass(msg_id))) {
        return(PASS);
    }
    else {
        printf("HOST: DP returned fail message.\n"
	       "Expected pass msg (%#.8x), received (%#.8x)\n",
	       (test_msg | MSG_VALID | MSG_PASS), msg_ctl);
        return(FAIL);
    }
}

/*
 * Function: host_probe_dp
 * Host sending ping via the GESW to data plane to check if
 * data plane has booted up. If data plane is not up and running,
 * the host will boot it up across the PCIe interface.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
static int host_probe_dp (void)
{
    int rv;

    if (!is_dp_lnx_up(1)) {
        printf("HOST: Boot DP Linux and diag...\n");

        rv = oct_remote_bootlinux(1);
        printf("HOST: Boot DP Linux and diag ");
	if (rv == FAIL) {
	    printf("failed\n");
	    return(rv);
	}
	else {
	  printf("passed\n");
	}

	/* Check again to see if DP can be reached via GESW
	 */
	printf("HOST: Ping DP...\n");
	if (!is_dp_lnx_up(1)) {
	    printf("HOST: Ping DP again...\n");
	    if (!is_dp_lnx_up(1)) {
		printf("HOST: Failed to contact DP via GESW. "
		       "Please check the GESW and DP remote boot.\n");
		return(FAIL);
	    }
	}
    }

    rv = host_mbox_init();
    printf("HOST: Mail box init ");
    if (rv == PASS) {
        printf("passed\n");
    }
    else {
        printf("failed\n");
    }

    return(rv);
}

/*
 * Function: run_dp_test
 * Send a test message to data plane mail box. The test starts
 * by sending a nc client request to the nc server running on
 * the data plane Linux.
 *
 * Input:
 * test_msg - the test message send to data plane
 * t_name - test name matches the test message
 * flag - flag to choose which port to use for the nc command.
 *        Port 50 is for ordinary use and port 51 is only for
 *        early lab bringup use.
 *
 * Return: PASS/FAIL
 */
static int run_dp_test (uint test_msg, char *t_name, int flag)
{
    int rv, nc_port;

    rv = host_probe_dp();
    if (rv == FAIL) {
	goto run_dp_test_exit;
    }
    
    send_msg(test_msg);

    nc_port = (flag) ? NC_CMD_PORT_51 : NC_CMD_PORT_50;

    if (nc_cmd_run_oct_diag(nc_port) == 0) {
	rv = host_chk_dp_test_result(test_msg);
    }
    else {
        printf("HOST: NC command failed in run DP test\n");
        rv = FAIL;
    }

 run_dp_test_exit:
    if (rv == PASS) {
        /* Note: Do not change this print statement.
	 * Klemtest script checks this for test pass or fail.
	 */
        printf("HOST: %s test passed\n", t_name);
    }
    else {
        cterr('f',0,"HOST: %s test failed (See detailed error messages above)\n", t_name);
    }

    return(rv);
}

/*
 * Function: run_dp_hello_test
 * This test is a quick sanity check if the data plane CPU
 * is up and can receive message.
 *
 * Input:
 * flag - This flag is passed to run_dp_test
 *
 * Return: PASS/FAIL
 */
static int run_dp_hello_test (int flag)
{
    int rv;
    char *t_name = "Run DP hello";

    testname("%s", t_name);
    prpass(testpass, "");
    printf("\n");
    rv = run_dp_test(MBOX_MSG_DP_HELLO_TEST, t_name, flag);
    return(rv);
}

/*
 * Function: run_dp_main_test
 * Test that runs all the data plane diag.
 *
 * Input:
 * flag - This flag is passed to run_dp_test
 *
 * Return: PASS/FAIL
 */
static int run_dp_main_test (int flag)
{
    char *t_name = "Run DP main";
    uint t_msg_id;
    int rv;

    if (dp_uart_loopback_test() == FAIL) {
        return (FAIL);
    }

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        testname("%s %s", t_name, "(external loopback off)");
	t_msg_id = MBOX_MSG_DP_MAIN_NOEXT_TEST;
    }
    else {
        /* Note: Do not change the t_name string.
	 * Klemtest script checks this for test pass or fail.
	 */
        testname("%s %s", t_name, "(external loopback on)");
	t_msg_id = MBOX_MSG_DP_MAIN_TEST;
    }

    if (diagflag_xram & D_GE_INT_LOOPBACK) {
        t_msg_id |= MBOX_FLAG_MSG_DP_GE_INT_LPBK;
    } else {
        t_msg_id &= ~MBOX_FLAG_MSG_DP_GE_INT_LPBK;
    }

    prpass(testpass, "");
    printf("\n");
    rv = run_dp_test(t_msg_id, t_name, flag);
    return(rv);
}


static void
* read_aux (void *u)
{
    int timeout = 5; /*in secs */
    int size = 0; /* when size= 0, read all bytes from uart controller */

    s_uart *uart = (s_uart *)u;

    if (rx_uart(uart->dev, size, (char *)uart->buf, timeout, uart->tst_typ) < 0) {

    }
    pthread_exit(NULL);
}

static int tmp_uart_msg_exh_test (char *dev, const char *send_str, const char *exp_msg, uint trig_typ)
{
    struct termios config;
    int uart_fd;
    char sd_str[100], exp_str[100];
    char *sd_pattern, *exp_pattern;
    pthread_t threads;
    speed_t test_speed;

    s_uart uart;

    uart.dev =  dev;

    memset(uart.buf, '\0', sizeof(uart.buf));

    /* the expect strings are from diag menu
     * this is for set exit state on rx_uart
     */
    uart.tst_typ = trig_typ;


    sd_pattern = (char *)send_str;
    exp_pattern = (char *)exp_msg;

    uart_fd = open(dev, O_WRONLY);
    if (uart_fd < 0) {
        perror("\nuart_msg_exh_test(): open tty failed.");
        return uart_fd;
    }
    if (tcgetattr(uart_fd, &config) < 0) {
        perror("\nuart_msg_exh_test(): Failed in tcgetattr()\n");
        return (FAILED);
    }

    test_speed = B115200; 
    if (cfsetospeed(&config, test_speed) < 0) {
        tcsetattr(uart_fd, TCSAFLUSH, &config);
        close(uart_fd);
        cterr('f', 0, "uart_intf_test(): Failed to set output speed.");
        return (FAILED);
    }

    if (cfsetispeed(&config, test_speed) < 0) {
        tcsetattr(uart_fd, TCSAFLUSH, &config);
        close(uart_fd);
        cterr('f', 0, "uart_intf_test(): Failed to set intput speed.");
        return (FAILED);
    }

    config.c_lflag &= ~(ICANON|IEXTEN|ISIG|ECHO);
    config.c_iflag |= IGNCR;
    config.c_oflag &= ~(OPOST);
    if (tcsetattr(uart_fd, TCSAFLUSH, &config) < 0) {
        perror("\nuart_msg_exh_test(): Failed in tcsetattr()\n");
        return (FAILED);
    }
    close(uart_fd);

    if(pthread_create(&threads, NULL, read_aux, (void *)&uart)) {
        perror("pthread_create failed.");
        printf("%s: pthread_create failed.\n", __FUNCTION__);
        return FAILED;
    }

    msleep(500);

    tx_uart(dev, sd_pattern, 1);

    pthread_join(threads, NULL);

    if (!strlen(uart.buf)) {
        printf("%s: Failed to receive data.\n", __FUNCTION__);
       return FAILED;

    }
    /* don't compare carriage return */
    if (!strstr(uart.buf, exp_pattern)) {
        sprintf(sd_str, sd_pattern);
        sprintf(exp_str, exp_pattern);

        printf("%s: failed. send/expect/reply string ", __FUNCTION__);
        printf("[sd = %s] [exp = %s] [rp = %s].\n",
               sd_str, exp_str, uart.buf);
        return FAILED;
    }

    return (PASSED);
}



/*
 * Function: dp_uart_loopback_test
 * UART loopback test for testing the UART connected to Cavium console port
 *
 * Input: None
 *
 * Return: PASS/FAIL
 */
static int dp_uart_loopback_test (void)
{
    char *t_name, *test_if;
    const char *pattern = "j\n";
    const char *msg = "Linux";
    int rv;

    /* 'j\n' for trigger cavium side diag sub-item,
     * which will invoke 'uname -a' on cavium. 
     * msg for expect string "Linux" since cavium is 
     * Linux based CP.
     */

    t_name = "Run DP UART loopback";
    test_if = "/dev/ttyDASH7";
    testname("%s", t_name);
    prpass(testpass, "");
    printf("\n");

    rv = host_probe_dp();
    if (rv == PASS) {
        
    /*  Before Rommon is ready, fixed baud rate to 115200 
     */
        if (is_ntpn_machines() || is_vg450()) { 
           printf("%s:---- FIX baud rate to 115200 ----\n", __FUNCTION__); 
           rv = tmp_uart_msg_exh_test(test_if, pattern, msg, TRIG_DIAG_M); 
        } else { 
           rv = uart_msg_exh_test(test_if, pattern, msg, TRIG_DIAG_M); 
        }
    }

    if (rv == PASS) {
        printf("HOST: %s test passed\n", t_name);
	return (PASSED);
    }
    else {
        cterr('f',0,"HOST: %s test failed\n", t_name);
        return (FAILED);
    }

}

/*
 * Function   : ovld_check_cavium_eeprom
 * Description: Function to check cavium eeprom loaded content
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 */
static int
ovld_check_cavium_eeprom (void)
{
    system("sh /overlord/bin/cavium_eeprom_check.sh");
    char cavium_eeprom_info[80];
    FILE  *fp;

    fp = fopen("/ovld_cavium_eeprom_check.txt", "r");
    if (fp == NULL) {

        printf("Failed to open /ovld_cavium_eeprom_check.txt");
        return (FAILED);

    }

    fgets(cavium_eeprom_info, sizeof(cavium_eeprom_info), fp);
    cavium_eeprom_info[strlen(cavium_eeprom_info)-1] = '\0';

    if (strstr(cavium_eeprom_info, "Region 4:") &&
        strstr(cavium_eeprom_info ,"64-bit, prefetchable") &&
        ( strstr(cavium_eeprom_info, "size=100000000") ||
          strstr(cavium_eeprom_info, "size=4G") )) {
        printf("Output message from lspci:%s\n Cavium EEPROM load check passed.\n",
        cavium_eeprom_info);
    } else {
        cterr('f', 0, "Output message from lspci:%s\n Cavium EEPROM load check failed\n",
        cavium_eeprom_info);
    }

    printf("\n");

    fclose(fp);

    return (PASSED);
}

/*
 * Function   : nep_check_cavium_eeprom
 * Description: Function to check cavium eeprom loaded content
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 */
static int
nep_check_cavium_eeprom (void)
{
    char cavium_eeprom_info[200];
    FILE  *fp;

    system("rm -f nep_cavium_eeprom_check.txt");
    system("touch nep_cavium_eeprom_check.txt");
    system("lspci -vv -d 177d: | grep 'Region 4:' | cut -c 2- >> nep_cavium_eeprom_check.txt");

    fp = fopen("nep_cavium_eeprom_check.txt", "r");
    if (fp == NULL) {
        printf("Failed to open nep_cavium_eeprom_check.txt");
        return (FAILED);
    }

    fgets(cavium_eeprom_info, sizeof(cavium_eeprom_info), fp);
    cavium_eeprom_info[strlen(cavium_eeprom_info)-1] = '\0';

    if (strstr(cavium_eeprom_info, "Region 4:") &&
        strstr(cavium_eeprom_info ,"64-bit, prefetchable") &&
        strstr(cavium_eeprom_info, "size=8G")) {
        printf("Output message from lspci:%s\n Cavium EEPROM load check passed.\n",
        cavium_eeprom_info);
    } else {
        cterr('f', 0, "Output message from lspci:%s\n Cavium EEPROM load check failed\n",
        cavium_eeprom_info);
    }

    printf("\n");

    fclose(fp);

    return (PASSED);
}

/*
 * Function: timingcard_clk_trig_verify_test
 * This test check if timingcard clk/trig are fed into
 * Cavium PHY properly.
 *
 * Input:
 * flag - This flag is passed to run_dp_test
 *
 * Return: PASS/FAIL
 */
int timingcard_clk_trig_verify_test (int flag)
{
    int rv;
    char *t_name = "Run DP CLK/TRIG verification test";

    testname("%s", t_name);
    prpass(testpass, "");
    printf("\n");

    if (cavium_init_3036x == FALSE) {
        /* Clear the timing card initialized flag */
        clear_timingcard_init_flag();

        /* Initialize the timing card */
        if (timingcard_init_seq() == FAILED) {
            cterr('f', 0, "Initialize the timing card fail");
            return (FAILED);
        }
        cavium_init_3036x = TRUE;
    }

    /* Set up the clock path - Overlord -> Timing card -> O2 Cavium */
    if (clock_direction_lib((uint)OVERLORD_CAVIUM) == FAILED) {
        return (FAILED);
    }

    rv = run_dp_test(MBOX_MSG_DP_CLK_TRIG_VERIFY, t_name, flag);
    return(rv);
}

/******** History ******** 
$Log: octeon_test.c,v $
Revision 1.10  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.9.6.17  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.9.6.16  2017/09/28 08:16:25  leschen
Support VG450

Revision 1.9.6.15  2017/08/10 21:21:05  ptong
Add nep_check_cavium_eeprom for Neptune

Revision 1.9.6.14  2017/08/02 09:59:41  leschen
Update Neptune PID from ISR4462 to ISR4461.

Revision 1.9.6.13  2017/07/13 23:12:56  ptong
Check if Cavium uboot and vmlinux exist in /nep-diag/cavium

Revision 1.9.6.12  2017/03/13 08:27:30  leschen
Support latest Neptune/Triton/Proteus/Neso PID.

Revision 1.9.6.11  2017/03/13 07:43:31  leschen
Support Triton system.

Revision 1.9.6.10  2017/03/07 02:07:36  alpeng
update ISR num to 4462 for neptune

Revision 1.9.6.9  2017/01/06 09:15:43  alpeng
fix boot uboot on neptune

Revision 1.9.6.8  2017/01/06 07:07:12  alpeng
neptune don't need to dl uboot and kernel for cvm

Revision 1.9.6.7  2017/01/04 08:32:38  alpeng
update sku num for neptune

Revision 1.9.6.6  2016/12/27 02:06:17  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.9.6.5  2016/12/09 08:59:15  alpeng
move the order of timing card

Revision 1.9.6.4  2016/12/07 02:20:20  alpeng
update cavium path for supporting kernel dir change

Revision 1.9.6.3  2016/11/09 00:14:46  alpeng
fix compiler err

Revision 1.9.6.2  2016/11/08 05:35:47  alpeng
update the uart test for neptune as baudrate 115200

Revision 1.9.6.1  2016/11/03 08:26:54  alpeng
merge octeon_test.c with o2

Revision 1.9  2016/03/04 19:19:15  ptong
Clean up obsolete ISR platfrom PID

Revision 1.8  2015/02/14 12:48:41  kodko
Collapse timing card branch code into main trunk.

Revision 1.7  2014/12/12 23:10:41  ptong
Add oct_remote_reset in oct_remote_memcheck. Bump version to 11.1.0

Revision 1.6.4.2  2014/03/11 07:02:24  leschen
Add 1588 clk/trig verification item.

Revision 1.6.4.1  2014/03/11 02:36:14  leschen
Add 1588 clk/trig verification function on O2 cavium.

Revision 1.6  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.5  2013/08/19 01:53:19  alpeng
using both FPGA and MB cookie to get/check board type

Revision 1.4  2013/07/10 06:20:19  alpeng
including platform_cookie.h to fix compile issue

Revision 1.3  2013/05/31 12:51:27  danchung
Add checking board type for Juno.

Revision 1.2  2013/05/16 11:37:39  danchung
Add cavium eeprom check on intel side.

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.34  2013/02/19 19:01:25  ptong
Add more info in error message

Revision 1.33  2013/01/24 01:10:02  ptong
Make ISR4451 the default SKU

Revision 1.32  2012/12/05 22:05:22  ptong
Fixed data plane boot up process

Revision 1.31  2012/12/03 22:08:48  ptong
Impove the remote boot steps when booting the Octeon

Revision 1.30  2012/11/29 22:31:11  ptong
Print more message when host is booting the data plane

Revision 1.29  2012/11/07 10:58:15  alpeng
remove useless file and clean up code

Revision 1.28  2012/10/04 00:32:24  ptong
Change the DP main test name to make klemtest happy

Revision 1.27  2012/10/01 22:22:57  ptong
Better message printing between host and data plane

Revision 1.26  2012/09/24 05:58:25  alpeng
add argument for rx_uart(), for getting last character on rx

Revision 1.25  2012/09/17 08:28:56  alpeng
revert uart_intf_test(), add uart_msg_exh_test()

Revision 1.24  2012/09/14 06:43:12  alpeng
move uart_loopback_test() to secondary item to avoid main test will invoke this item

Revision 1.23  2012/09/12 23:44:59  ptong
Code cleanup and add comments

Revision 1.22  2012/09/12 02:34:08  alpeng
do cavium uart loopback test with trigger cavium diag item

Revision 1.21  2012/09/04 23:53:18  ptong
Set DP UART test as a group test in the menu

Revision 1.20  2012/08/31 05:07:00  ptong
Add data plane uart test in the DP main test

Revision 1.19  2012/08/29 07:45:37  alpeng
add cterr for fail case on uart_intf_test()

Revision 1.18  2012/08/29 06:20:09  alpeng
fixed fail case for return value of uart_intf_test()

Revision 1.17  2012/08/22 09:32:40  alpeng
supporting uart test for cavium

Revision 1.16  2012/08/18 00:01:13  ptong
Use official SKU for PID checking

Revision 1.15  2012/08/17 22:10:54  ptong
Add comments

Revision 1.14  2012/08/13 07:06:22  alpeng
replacing nanocom to picocom

Revision 1.13  2012/07/27 02:05:19  ptong
Use tftp_get to download cavium u-boot

Revision 1.12  2012/07/19 20:09:10  ptong
Fix test did not stop on error issue

Revision 1.11  2012/06/25 07:02:15  alpeng
revert method for storing diag flag

Revision 1.10  2012/06/19 23:20:15  ptong
Check correct Octeon model is used on the platform

Revision 1.9  2012/06/06 02:12:33  ptong
Call tftp_get to download Cavium images

Revision 1.8  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.7  2012/05/27 22:33:25  ptong
Modified oct_remote_bootlinux to support Overlord and Omaha SKU

Revision 1.6  2012/05/12 00:00:07  ptong
Added DP_MAIN_NOEXT_TEST to cavium

Revision 1.5  2012/04/27 01:03:58  ptong
Minor changes

Revision 1.4  2012/04/17 22:01:27  ptong
Added more utility to run DP test from host.

Revision 1.3  2012/04/11 21:27:17  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
