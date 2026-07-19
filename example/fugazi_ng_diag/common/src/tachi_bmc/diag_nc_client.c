/* $Id: diag_nc_client.c,v 1.6 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_nc_client.c,v $
 *------------------------------------------------------------------
 *
 * diag_nc_client.c
 * CSX-Tachi nc client entry
 *
 * Nov 2015, Alan Peng
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "common.h"
#include "proto.h"
#include "plat_defs.h"
#include "diag_nc_common.h"
#include "diag_pem_lib.h"
#include "diag_lewis_gesw_test.h"
#include "nvsysvars.h"
#include "diag_plat_cookie.h"
#include "platform_fru.h"
#include "diag_fpga_lib.h"
#include "diag_i2c_lib.h"
#include "platform_fru.h"
#include "i2c_api.h"
#include "ngio_testcard.h"
#include <fcntl.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "cross_platform.h"
#include "mb_tests.h"

static struct nc_args *head = NULL; 
static struct nc_args *curr = NULL; 

static int display_i350_fiber_reg(void);
static int display_intel_cpu_test_reg(void);
static int display_intel_mem_test_reg(void);
static int display_intel_hdd_test_reg(void);
static int display_intel_usb_test_reg(void);
static int display_intel_ssd_test_reg(void);
static int display_intel_emmc_test_reg(void);
static int display_intel_bmcusb0_test_reg(void);
static int display_intel_bmcusb1_test_reg(void);
static int display_intel_i350_test_reg(void);
static int display_intel_x710_test_reg(void);
static int display_intel_i210_test_reg(void);
static int display_intel_cpu_core_test_reg(void);
static int display_intel_pci_if_test_reg(void);
static int display_intel_tmp20_spi_test_reg(void);
static int display_intel_isp_pci_if_test_reg(void);
static int display_intel_isp_raid_pci_if_test_reg(void);
static int display_intel_isp_crypto_pci_if_test_reg(void);


/**********************************************************************
 *
 * Function: nc_create_arg
 *
 * Description: create head node for nc 
 *
 * Input : arg_str - string to argument
 *
 * Output: ptr - current node .
 *
 **********************************************************************
 */
struct nc_args *nc_create_arg (char *arg_str)
{
    struct nc_args *ptr = (struct nc_args*) malloc(sizeof(struct nc_args)); 
    if (ptr == NULL) {
        printf("Failed to create arg node \n");
        return (NULL);
    }

    strcpy(ptr->arg, arg_str);
    ptr->next = NULL;

    head = curr = ptr;
    return (ptr);
}

/**********************************************************************
 *
 * Function: nc_add_arg
 *
 * Description: add argument into struct arg 
 *
 * Input : arg_str - string to argument 
 *
 * Output: ptr - current node .
 *
 **********************************************************************
 */
struct nc_args *nc_add_args (char *arg_str)
{
    if (head == NULL) {
        return (nc_create_arg(arg_str));
    }

    struct nc_args *ptr = (struct nc_args*) malloc(sizeof(struct nc_args));
    if (ptr == NULL) {
        printf("Failed to create arg node \n");
        return (NULL);
    }

    strcpy(ptr->arg, arg_str);

    ptr->next = NULL;

    curr->next = ptr;
    curr = ptr;

    return (ptr);
}

/**********************************************************************
 *
 * Function: nc_delete_arg
 *
 * Description: delete arg node 
 *
 * Input : head_ptr - head node for arg. 
 *
 * Output: NONE
 *
 **********************************************************************
 */
static void nc_delete_arg (struct nc_args *head_ptr)
{
    struct nc_args *tmp;

    while(head_ptr != NULL)
    {
       tmp = head_ptr;
       head_ptr = head_ptr->next;
       free(tmp);
    }

    return;
}

/**********************************************************************
 * 
 * Function: diag_nc_client_entry
 * 
 * Description: nc client entry for send request to server. 
 *
 * Input : subsystem - server 0: bmc, 1:intel, 2:lewis 
 *         cmd - the command to server subsystem 
 *         parm1 - the parameter to server for cmd 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_client_entry (unsigned int subsystem,
                          char *cmd, char* parm1, char *parm2,
                          char *parm3, char *parm4)
{
    int rc = FAILED;
    struct nc_args *arg; 

    /* prepare cmd, argument into structure nc_args */
    nc_add_args(cmd); 
    nc_add_args(parm1);
    nc_add_args(parm2);
    nc_add_args(parm3);
    nc_add_args(parm4);
    
    arg = head; 

    /* init result file */
    nc_init_result_file();

    /* prepare command, arg to server */
    nc_host_dispatch_comm(subsystem, arg); 

    /* clean up mem */
    nc_delete_arg(head);
    head = NULL;  /* make sure head is NULL for next list head */

    /* check test status from result file */
    rc = nc_check_test_status();
    if (rc == PASSED) {
        printf("nc command %s PASS\n", cmd);
    } else {
        printf("nc command %s FAIL\n", cmd);
    }

    return (rc); 
}

/**********************************************************************
 *
 * Function: diag_nc_client_utility_entry
 *
 * Description: nc client utility entry for send request to server.
 *
 * Input : subsystem - server 0: bmc, 1:intel, 2:lewis
 *         cmd - the command to server subsystem
 *         parm1 - the parameter to server for cmd
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_client_utility_entry(unsigned int subsystem, char *cmd,
        char* parm1, char *parm2, char *parm3, char *parm4) {
    int rc = FAILED;
    struct nc_args *arg;

    /* prepare cmd, argument into structure nc_args */
    nc_add_args(cmd);
    nc_add_args(parm1);
    nc_add_args(parm2);
    nc_add_args(parm3);
    nc_add_args(parm4);

    arg = head;

    /* init result file */
    nc_init_result_file();

    /* prepare command, arg to server */
    nc_host_dispatch_comm(subsystem, arg);

    /* clean up mem */
    nc_delete_arg(head);
    head = NULL; /* make sure head is NULL for next list head */

    /* check test status from result file */
    rc = nc_check_test_status();

    return (rc);
}


/**********************************************************************
 *
 * Function: diag_nc_nim_testcard_test
 *
 * Description: test testcard via nc client 
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_nim_testcard_test (void) 
{
    int rc; 

    fru_table_offset = tc_fru_table_offset;
    platform_fru_table[fru_table_offset].pid_string = testcard_10gkr_pid;
    platform_fru_table[fru_table_offset].location_string = nim_10gkr_loc;
    cterr_add_component("PCIe enumeration test", "10G-KR Testcard NIM PCIe sw");
    cterr_add_debug("PCIe enumeration fail ",
                    "Intel can not recognize NIM PCIe switch ",
                    "Check Intel PCIe switch ",
                    "Using PCI rescan for retry",
                    "Power up NIM via platform FPGA utils before Intel power up",
                    "Check NIM FPGA reg test for verify I2C to NIM");

    testname("Testcard PCIe Test");
    prpass(testpass, "Testcard PCIe Test");

    rc = diag_nc_client_entry(INTEL_SUB, "diag_nim_tc_test", DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);

    if (rc == PASSED) {
        prpass(testpass, "nc command %s PASS\n", __FUNCTION__);
    } else {
        cterr('f', 0, "nc command %s FAIL\n", __FUNCTION__);
    }

    return (rc); 
}

/**********************************************************************
 *
 * Function: diag_nc_nim_dl_test
 *
 * Description: test dreamliner via nc client
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_nim_dl_test (void)
{
    int rc, dummy1, dummy2;
    char arg1[16], arg2[16]; 

    testname("Dreamliner General Test");
    prpass(testpass, "Dreamliner General Test");

    sprintf(arg1, "%d", NGWIC1_SLOT);

    if (check_poe_psu_present(dummy1, dummy2)) {
        sprintf(arg2, "%s", "-e"); 
    } else {
        sprintf(arg2, "%s", " ");  /* empty for dummy */
    }

    rc = diag_nc_client_entry(INTEL_SUB, "diag_nim_dl_test", arg1, arg2,
            DUMMY3, DUMMY4);       

    if (rc == PASSED) {
        prpass(testpass, "nc command %s PASS", __FUNCTION__);
    } else {
        cterr('f', 0, "nc command %s FAIL\n", __FUNCTION__);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_nim_dl_lpbk
 *
 * Description: enable/disable dreamliner loopback 
 *
 * Input : type - loopback type, port - phy port, enable - en/disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_nim_dl_lpbk (int type, int port, int enable ) 
{
    int rc;
    char arg1[16], arg2[16], utils[32]; 

    sprintf(arg1, "%d", type); 
    sprintf(arg2, "%d", port); 

    if (enable == TRUE) {
        sprintf(utils, "%s", "diag_nim_dl_set_lpbk"); 
    } else {
        sprintf(utils, "%s", "diag_nim_dl_clr_lpbk"); 
    }

    rc = diag_nc_client_entry(INTEL_SUB, utils, arg1, arg2, DUMMY3, DUMMY4);

    if (rc == PASSED) {
        printf("Final nc command %s PASS\n", __FUNCTION__);
    } else {
        printf("Final nc command %s FAIL\n", __FUNCTION__);
    }
    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_mb_test
 *
 * Description: Executes all Intel M/B tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_mb_test(void) {
    int rc;

    testname("INTEL mother board test");
    prpass(testpass, "INTEL Mother Board Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_MB_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL Mother Board Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_cpu_test_err_report(void)
{
    fru_table_offset = INTEL_CPU;
    platform_fru_table[INTEL_CPU].pid_string = intel_cpu;
    platform_fru_table[INTEL_CPU].location_string = intel_cpu_loc;
    cterr_add_component("INTEL");
    cterr_add_reg_dump((PFV)display_intel_cpu_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check CPU power and clock working",
    		"Please run memory testing",
    		"Please check environment temperature when testing fail");
}

static int display_intel_cpu_test_reg (void)
{
    cterr_db_print("Test by linux stress tool, no register dump");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_cpu_test
 *
 * Description: Executes Intel CPU tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_cpu_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_cpu_test_err_report();
    }

    testname("INTEL CPU test");
    prpass(testpass, "INTEL CPU Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_CPU_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL CPU Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_mem_test_err_report(void)
{
    fru_table_offset = INTEL_MEM;
    platform_fru_table[INTEL_MEM].pid_string = intel_mem;
    platform_fru_table[INTEL_MEM].location_string = intel_mem_loc;
    cterr_add_component("INTEL", "DDR4 DIMM");
    cterr_add_reg_dump((PFV)display_intel_mem_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the memory interface between INTEL and DDR4",
                        "Replace one DDR4 and redo the test");
}

static int display_intel_mem_test_reg (void)
{
    cterr_db_print("Test by linux memtester tool, no register dump");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_mem_test
 *
 * Description: Executes Intel MEM tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_mem_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_mem_test_err_report();
    }

    testname("INTEL memory Test");
    prpass(testpass, "INTEL Memory Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_MEM_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL Memory Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_hdd_test_err_report(void)
{
    fru_table_offset = INTEL_HDD;
    platform_fru_table[INTEL_HDD].pid_string = intel_hdd;
    platform_fru_table[INTEL_HDD].location_string = intel_hdd_loc;
    cterr_add_component("INTEL", "SATA3", "HDD");
    cterr_add_reg_dump((PFV)display_intel_hdd_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the hdd testing with directly HDD or RAID card",
    		"For RAID card testing, testing HDD must work at JBOD mode"
    		"Replace one HDD and redo the test",
    		"Check the SATA interface between INTEL and HDD",
    		"Check Tsda and Tsdb exit under /dev folder or not");
}

static int display_intel_hdd_test_reg (void)
{
    cterr_db_print("Test by linux blktst tool, no register dump");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_hdd_test
 *
 * Description: Executes Intel HDD tests by mfgdiag.
 * 				HDD include RAID HDD testing
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_hdd_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_hdd_test_err_report();
    }

    testname("INTEL HDD/RAID HDD Test");
    prpass(testpass, "INTEL HDD/RAID HDD Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_HDD_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL HDD/RAID HDD Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_usb_test_err_report(void)
{
    fru_table_offset = INTEL_USB;
    platform_fru_table[INTEL_USB].pid_string = intel_usb;
    platform_fru_table[INTEL_USB].location_string = intel_usb_loc;
    cterr_add_component("INTEL", "USB2.0/USB3.0", "USB stick");
    cterr_add_reg_dump((PFV)display_intel_usb_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Replace one USB stick and redo the test",
    		"Base on different USB stick type to use usb20 and usb30 testing"
    		"Check the USB2.0/USB3.0 interface between INTEL and USB",
    		"For USB2.0 fail check device Tsdd exit under /dev folder or not",
    		"For USB3.0 fail check device Tsde exit under /dev folder or not");
}

static int display_intel_usb_test_reg (void)
{
    cterr_db_print("Test by linux blktst tool, no register dump");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_usb_test
 *
 * Description: Executes Intel USB2.0 stick tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_usb_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_usb_test_err_report();
    }

    testname("USB2.0/USB3.0 stick test");
    prpass(testpass, "USB2.0/USB3.0 stick Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_USB_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "USB2.0/USB3.0 stick Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_usb20_test
 *
 * Description: Executes Intel USB 2.0 stick tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_usb20_test(void) {
    int rc;

    testname("USB2.0 stick test");
    prpass(testpass, "USB2.0 stick Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_USB20_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "USB2.0 stick Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_usb30_test
 *
 * Description: Executes Intel USB 3.0 stick tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_usb30_test(void) {
    int rc;

    testname("USB3.0 stick test");
    prpass(testpass, "USB3.0 stick Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_USB30_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "USB3.0 stick Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_ssd_test_err_report(void)
{
    fru_table_offset = INTEL_SSD;
    platform_fru_table[INTEL_SSD].pid_string = intel_ssd;
    platform_fru_table[INTEL_SSD].location_string = intel_ssd_loc;
    cterr_add_component("INTEL", "SATAx1", "M.2SSD");
    cterr_add_reg_dump((PFV)display_intel_ssd_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Replace one M.2 SSD and redo the test",
    		"Check the SATA interface between INTEL and SSD",
    		"Check device Tsdc exit under /dev folder or not");
}
static int display_intel_ssd_test_reg (void)
{
    cterr_db_print("Test by linux blktst tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_ssd_test
 *
 * Description: Executes Intel SSD tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_ssd_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_ssd_test_err_report();
    }

    testname("M2 SSD Test");
    prpass(testpass, "M2 SSD Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_SSD_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "M2 SSD Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_emmc_test_err_report(void)
{
    fru_table_offset = INTEL_EMMC;
    platform_fru_table[INTEL_EMMC].pid_string = intel_emmc;
    platform_fru_table[INTEL_EMMC].location_string = intel_emmc_loc;
    cterr_add_component("INTEL", "USB2.0", "USB3.0", "FX3S", "eMMC");
    cterr_add_reg_dump((PFV)display_intel_emmc_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check lsscsi command under diag linux"
    		"ensure got Cisco RAID SD0_1 device",
    		"Check the USB2.0 and USB3.0 interface between INTEL and FX3S",
    		"Check device Tsdf exit under /dev folder or not",
    		"Check BMC has mount emmc to intel or not");
}
static int display_intel_emmc_test_reg (void)
{
    cterr_db_print("Test by linux blktst tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_emmc_test
 *
 * Description: Executes Intel eMMC tests by mfgdiag
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_emmc_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_emmc_test_err_report();
    }

    testname("eMMC Test");
    prpass(testpass, "eMMC Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_EMMC30_TEST, DUMMY1, DUMMY2,
        DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "eMMC USB 3.0 Test Failed");
        return (FAILED);
    }

    /* The Tachi-L boards type have both USB2.0 and USB3.0. */
    if ( !fx3_switch_usb() ) {
        rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_EMMC20_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
        if (rc == FAILED) {
            cterr('f', 0, "eMMC USB 2.0 Test Failed");
            return (FAILED);
        }
    }
    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_bmcusb0_test_err_report(void)
{
    fru_table_offset = INTEL_BMCUSB0;
    platform_fru_table[INTEL_BMCUSB0].pid_string = intel_bmcusb0;
    platform_fru_table[INTEL_BMCUSB0].location_string = intel_bmcusb0_loc;
    cterr_add_component("INTEL", "USBCH1", "BMC");
    cterr_add_reg_dump((PFV)display_intel_bmcusb0_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check lsusb command under diag linux"
    		"ensure got Cisco Virtual Keyboard and Mouse",
    		"Check the USBCH1 interface between INTEL and BMC");
}
static int display_intel_bmcusb0_test_reg (void)
{
    cterr_db_print("Test by linux lsusb tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_bmcusb0_test
 *
 * Description: Executes Intel USB interface 0 check between BMC and INTEL
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_bmcusb0_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_bmcusb0_test_err_report();
    }

    testname("INTEL BMC USB interface 0");
    prpass(testpass, "BMC USB Interface 0 Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_BMCUSB0_TEST, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "BMC USB Interface 0 Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_bmcusb1_test_err_report(void)
{
    fru_table_offset = INTEL_BMCUSB1;
    platform_fru_table[INTEL_BMCUSB1].pid_string = intel_bmcusb1;
    platform_fru_table[INTEL_BMCUSB1].location_string = intel_bmcusb1_loc;
    cterr_add_component("INTEL", "USBCH2", "BMC");
    cterr_add_reg_dump((PFV)display_intel_bmcusb1_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check lsusb command under diag linux"
    		"ensure got Cisco Virtual Mass Storage",
    		"Check the USBCH2 interface between INTEL and BMC");
}
static int display_intel_bmcusb1_test_reg (void)
{
    cterr_db_print("Test by linux lsusb tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_bmcusb1_test
 *
 * Description: Executes Intel USB interface 1 check between BMC and INTEL
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_bmcusb1_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_bmcusb1_test_err_report();
    }

    testname("INTEL BMC USB interface 1");
    prpass(testpass, "BMC USB Interface 1 Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_BMCUSB1_TEST, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "BMC USB Interface 1 Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_btb_test
 *
 * Description: Executes Intel BTB X710 ethernet loopback tests
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_btb_test(void) {
    int rc;

    testname("INTEL X710 BTB");
    prpass(testpass, "INTEL X710 BTB card Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_BTB_TEST, DUMMY1, DUMMY2, DUMMY3,
            DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL X710 BTB card Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static void
add_diag_nc_intel_i350_test_err_report(void)
{
    fru_table_offset = INTEL_I350;
    platform_fru_table[INTEL_I350].pid_string = intel_i350;
    platform_fru_table[INTEL_I350].location_string = intel_i350_loc;
    cterr_add_component("INTEL", "PCIE G2x4", "I350");
    cterr_add_reg_dump((PFV)display_intel_i350_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("If enable external flag, please ensure loopback cable inserted",
    		"Replace loopback cable with working one",
    		"Check I350 EEPROM has be program correctly or not"
    		"Check the PCIe interface between INTEL and I350",
    		"Check ethernet interface Teth1 and Teth2 exist or not by ifconfig -a",
    		"Check I350 could be recognizer by celo64e /devices",
    		"Check I350 has program MAC address or not",
    		"If Fiber testing fail, please check I350 Fiber enable in FPGA");
}
static int display_intel_i350_test_reg (void)
{
    cterr_db_print("Test by intel celo64e tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_i350_test
 *
 * Description: Executes Intel I350 testing
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_i350_test(void) {
    int rc;
    unsigned short flag = (NVRAM)->diagflag;
    char val[]=INTEL_NC_STRING_PARAMETER;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_i350_test_err_report();
    }

    testname("INTEL I350");
    prpass(testpass, "INTEL I350 Test");

    /* Enable FPGA I350 Fiber presence ping */
    diag_fpga_reg_write(FPGA_SFP_0_CONFIG_REG, FPGA_SFP_PRESENT_OUTPUT_ENABLE);
    diag_fpga_reg_write(FPGA_SFP_1_CONFIG_REG, FPGA_SFP_PRESENT_OUTPUT_ENABLE);

    if (flag & D_EXT_LOOPBACK) {
        val[0] = '0';
    } else {
        val[0] = '1';
    }

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_I350_TEST, val, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL I350 Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

static int display_i350_fiber_reg(void)
{
    int sfp0,sfp1;
    uchar data;

    cterr_db_print("Dump I350 Fiber test related register\n");

    diag_fpga_reg_read(FPGA_SFP_0_CONFIG_REG, &sfp0);
    cterr_db_print("FPGA SFP 0 Configure Register: 0x%x\n", sfp0);

    diag_fpga_reg_read(FPGA_SFP_1_CONFIG_REG, &sfp1);
    cterr_db_print("FPGA SFP 1 Configure Register: 0x%x\n", sfp1);

    diag_i2c_byte_read(BMC_PCA9543_MUX_BUS, BMC_PCA9543_MUX_ADDRESS, BMC_I350_SFP_I2C_CONTROL_REG, &data);
    cterr_db_print("Switch Mux PCA9543 I2C bus Register: 0x%x\n", data);

    return (PASSED);
}

static void
add_diag_intel_i350_fiber_i2c_test_err_report(void)
{
    fru_table_offset = INTEL_I350;
    platform_fru_table[INTEL_I350].pid_string = intel_i350;
    platform_fru_table[INTEL_I350].location_string = intel_i350_loc;
    cterr_add_component("BMC", "FPGA", "PCA9543 MUX", "I2C", "SFP");
    cterr_add_reg_dump((PFV)display_i350_fiber_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check FPGA has disable SFP presence pin or not",
    		"Have insert SFP module",
    		"Check the I2C interface between Mux PCA9543 and I350 SFP");
}
/**********************************************************************
 *
 * Function: diag_intel_i350_fiber_i2c_test
 *
 * Description: Executes Intel I350 Fiber I2C scan testing
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_intel_i350_fiber_i2c_test(void) {
    int sfp0,sfp1;

    if (get_enhance_err_flag()) {
        add_diag_intel_i350_fiber_i2c_test_err_report();
    }

    testname("INTEL I350 Fiber I2C");
    prpass(testpass, "INTEL I350 Fiber I2C Test");

    /* Check Fiber exist or not */
    diag_fpga_reg_read(FPGA_SFP_0_CONFIG_REG, &sfp0);
    diag_fpga_reg_read(FPGA_SFP_1_CONFIG_REG, &sfp1);
    if (!(FPGA_SFP_PRESENT_MASK & sfp0 & sfp1)) {
    	cterr('f', 0, "INTEL I350 Fiber not insert");
    	return (FAILED);
    }

    /* Disable FPGA I350 Fiber presence ping */
    diag_fpga_reg_write(FPGA_SFP_0_CONFIG_REG, FPGA_SFP_PRESENT_OUTPUT_DISABLE);
    diag_fpga_reg_write(FPGA_SFP_1_CONFIG_REG, FPGA_SFP_PRESENT_OUTPUT_DISABLE);

    /* Switch Mux PCA9543 to I2C bus 1 */
    diag_i2c_byte_write(BMC_PCA9543_MUX_BUS, BMC_PCA9543_MUX_ADDRESS, BMC_I350_SFP_I2C_CONTROL_REG, BMC_PCA9543_MUX_PORT0_MASK);

    /* Run I350 SFP 1 Fiber scan testing */
    if (diag_i2c_ping(BMC_PCA9543_MUX_BUS, BMC_I350_SFP_I2C_ADDRESS, 0)) {
    	cterr('f', 0, "INTEL I350 Fiber 1 ping Failed");
    	return (FAILED);
    }

    /* Switch Mux PCA9543 to I2C bus 2 */
    diag_i2c_byte_write(BMC_PCA9543_MUX_BUS, BMC_PCA9543_MUX_ADDRESS, BMC_I350_SFP_I2C_CONTROL_REG, BMC_PCA9543_MUX_PORT1_MASK);

    /* Run I350 SFP 2 Fiber scan testing */
    if (diag_i2c_ping(BMC_PCA9543_MUX_BUS, BMC_I350_SFP_I2C_ADDRESS, 0)) {
    	cterr('f', 0, "INTEL I350 Fiber 2 ping Failed");
    	return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}



static void
add_diag_nc_intel_x710_test_err_report(void)
{
    fru_table_offset = INTEL_X710;
    platform_fru_table[INTEL_X710].pid_string = intel_x710;
    platform_fru_table[INTEL_X710].location_string = intel_x710_loc;
    cterr_add_component("INTEL", "PCIE G3x8", "X710", "10G-KRx2","LEWIS");
    cterr_add_reg_dump((PFV)display_intel_x710_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check 10G interface between X710 and Lewis"
    		"Check X710 EEPROM has be program correctly or not",
    		"Check the PCIe interface between INTEL and X710",
    		"Check ethernet interface Teth3 and Teth4 exist or not by ifconfig -a",
    		"Check X710 could be recognizer by celo64e /devices",
    		"Check X710 has program MAC address or not");
}
static int display_intel_x710_test_reg (void)
{
    cterr_db_print("Test by intel celo64e tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_x710_test
 *
 * Description: Executes Intel X710 testing
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_x710_test(void) {
    int rc;
    int fail = 0 ;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_x710_test_err_report();
    }

    testname("INTEL X710");
    prpass(testpass, "INTEL X710 Test");

    rc = diag_lewis_gesw_x710_endis_serdes_lpbk(TRUE);
    if (rc == FAILED) {
        printf("Set Lewis Serdes loopback Failed.\n");
        fail++;
    }

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_X710_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        printf("Intel X710 testing Failed.\n");
        fail++;
    }

    rc = diag_lewis_gesw_x710_endis_serdes_lpbk(FALSE);
    if (rc == FAILED) {
        printf("Unset Lewis Serdes loopback Failed.\n");
        fail++;
    }

    if (fail) {
        cterr('f', 0, "INTEL X710 Test Failed");
        return (FAILED);
    }


    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

static void
add_diag_nc_intel_i210_test_err_report(void)
{
    fru_table_offset = INTEL_I210;
    platform_fru_table[INTEL_I210].pid_string = intel_i210;
    platform_fru_table[INTEL_I210].location_string = intel_i210_loc;
    cterr_add_component("INTEL", "PCIE G2x1", "I210");
    cterr_add_reg_dump((PFV)display_intel_i210_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("If enable external flag, please ensure loopback cable inserted",
    		"Replace loopback cable with working one",
    		"Check I210 EEPROM has be program correctly or not",
    		"Check the PCIe interface between INTEL and I210",
    		"Check ethernet interface Teth5 exist or not by ifconfig -a",
    		"Check I210 could be recognizer by celo64e /devices",
    		"Check I210 has program MAC address or not");
}
static int display_intel_i210_test_reg (void)
{
    cterr_db_print("Test by intel celo64e tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_i210_test
 *
 * Description: Executes Intel I210 testing
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_i210_test(void) {
    int rc;
    unsigned short flag = (NVRAM)->diagflag;
    char val[]=INTEL_NC_STRING_PARAMETER;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_i210_test_err_report();
    }

    testname("INTEL I210");
    prpass(testpass, "INTEL I210 Test");

    if (flag & D_EXT_LOOPBACK) {
        val[0] = '0';
    } else {
        val[0] = '1';
    }

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_I210_TEST, val, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL I210 Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_get_i350_mode
 *
 * Description: Check Intel I350 mode
 *
 * Input : None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int diag_nc_intel_get_i350_mode(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_GET_I350_MODE, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    if (rc == DIAG_INTEL_I350_FIBER_MODE) {
        printf("nc command %s I350 is Copper\n", __FUNCTION__);
    } else if (rc == DIAG_INTEL_I350_COPPER_MODE) {
        printf("nc command %s I350 is Fiber\n", __FUNCTION__);
    } else {
        printf("nc command %s I350 mode unknow\n", __FUNCTION__);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_reflash_i350_mode
 *
 * Description: Reflash Intel I350 firmware
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_reflash_i350_mode(void) {
    int rc;
    char val[]=INTEL_NC_STRING_PARAMETER;

    printf("Please select I350 Firmware(0 Fiber, 1 Copper): ");
    val[0] = getchar();
    if ((val[0] != '0') && (val[0] != '1')) {
        printf("*** Wrong input. Only character 0 or 1 allow. ***\n");
        return (FAILED);
    }
    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_REFLASH_I350_MODE, val,
            DUMMY2, DUMMY3, DUMMY4);

    if (val[0] == '0') {
        printf("nc command %s reflash I350 firmware to Fiber\n", __FUNCTION__);
    } else {
        printf("nc command %s reflash I350 firmware to Copper\n", __FUNCTION__);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_force_i350_link
 *
 * Description: Force Intel I350 Teth1 link up
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_force_i350_link(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_FORCE_I350_LINK, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    printf("nc command %s force link Teth1 up\n", __FUNCTION__);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_intel_disable_i350_rx
 *
 * Description: Disable i350 rx
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_disable_i350_rx(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_DISABLE_I350_RX, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    printf("nc command %s disable i350 rx\n", __FUNCTION__);

    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_show_hdd_size
 *
 * Description: Show Intel HDD/eMMC/SSD size
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_show_hdd_size(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_SHOW_HDD_SIZE, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_show_dimm_size
 *
 * Description: Show Intel DIMM size
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_show_dimm_size(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_SHOW_DIMM_SIZE, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_show_teth_interfaces
 *
 * Description: Show Intel Linux Tethx interface info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_show_teth_interfaces(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_SHOW_TETH_INTERFACE, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_show_tsd_devices
 *
 * Description: Show Intel Linux Tsdx device info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_show_tsd_devices(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_SHOW_TSD_DEVICES, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    return (PASSED);
}

static void
add_diag_intel_cpu_core_test_err_report(void)
{
    fru_table_offset = INTEL_CORE;
    platform_fru_table[INTEL_CORE].pid_string = intel_core;
    platform_fru_table[INTEL_CORE].location_string = intel_core_loc;
    cterr_add_component("INTEL");
    cterr_add_reg_dump((PFV)display_intel_cpu_core_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check CPU power and clock working",
    		"Please run memory testing",
    		"Please check environment temperature when testing fail",
    		"Check BMC Platform SKU correct or not");
}
static int display_intel_cpu_core_test_reg (void)
{
    cterr_db_print("Test by linux lscpu tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_intel_cpu_core_test
 *
 * Description: Intel linux cpu core testing
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_intel_cpu_core_test(void) {
    int rc;
    char sku_name[32];
    FILE *fp;
    char val[]=INTEL_NC_STRING_PARAMETER;

    if (get_enhance_err_flag()) {
        add_diag_intel_cpu_core_test_err_report();
    }
    testname("INTEL CPU CORE");
    prpass(testpass, "INTEL CPU Core Test");


    fp = fopen(MB_PID_FILE, "r");
    if (fp != NULL) {
        fscanf(fp, "%s", sku_name);
        fclose(fp);
    } else {
        printf("Failed to open %s \n", MB_PID_FILE);
    }

    printf("Platform SKU: %s\n", sku_name);

    if (strcmp(sku_name,TACHI_SKU1_PID) == 0) {
        val[0] = INTEL_CPU_CORE_6;
    } else if (strcmp(sku_name,TACHI_SKU2_PID) == 0) {
        val[0] = INTEL_CPU_CORE_8;
    } else if (strcmp(sku_name,TACHI_SKU3_PID) == 0) {
        val[0] = INTEL_CPU_CORE_12;
    } else {
        cterr('f', 0, "Could not recognize SKU PID");
        return (FAILED);
    }

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_CPU_CORE_TEST,
            val, DUMMY2, DUMMY3, DUMMY4);

    if (rc == FAILED) {
        cterr('f', 0, "INTEL CPU Core Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);

}

static void
add_diag_intel_pci_if_test_err_report(void)
{
    fru_table_offset = INTEL_PCIE;
    platform_fru_table[INTEL_PCIE].pid_string = intel_pcie;
    platform_fru_table[INTEL_PCIE].location_string = intel_pcie_loc;
    cterr_add_component("INTEL", "PCIe", "I210", "I350", "X710", "PCIe SW");
    cterr_add_reg_dump((PFV)display_intel_pci_if_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the PCIe interface between INTEL and Each devices",
    		"Check PCIe link speed and width in BIOS shell");
}
static int display_intel_pci_if_test_reg (void)
{
    cterr_db_print("Test by linux lspci tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_intel_pci_if_test
 *
 * Description:  Intel linux default PCI bus info
 *               I210, I350, X710 and PCI SW
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_intel_pci_if_test(void) {

    int rc;
    char val[]=INTEL_DF_PCI_BUS;

    if (get_enhance_err_flag()) {
        add_diag_intel_pci_if_test_err_report();
    }

    testname("INTEL PCI BUS");
    prpass(testpass, "INTEL PCI Bus Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_PCI_IF_TEST,
            val, DUMMY2, DUMMY3, DUMMY4);

    if (rc == FAILED) {
        cterr('f', 0, "INTEL PCI Bus Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);

}

static void
add_diag_nc_intel_tpm20_spi_test_err_report(void)
{
    fru_table_offset = INTEL_TPM20;
    platform_fru_table[INTEL_TPM20].pid_string = intel_tpm20;
    platform_fru_table[INTEL_TPM20].location_string = intel_tpm20_loc;
    cterr_add_component("INTEL", "SPI", "TPM20");
    cterr_add_reg_dump((PFV)display_intel_tmp20_spi_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check BIOS has enable TPM function"
    		"Check the SPI interface between INTEL and TPM20",
    		"Double Check by EFI TPM testing Tool");
}
static int display_intel_tmp20_spi_test_reg (void)
{
    cterr_db_print("Test by linux tpm2.0 driver, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_nc_intel_tpm20_spi_test
 *
 * Description: Executes Intel TPM20 SPI interface tests
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_tpm20_spi_test(void) {
    int rc;

    if (get_enhance_err_flag()) {
        add_diag_nc_intel_tpm20_spi_test_err_report();
    }

    testname("TPM2.0 Test");
    prpass(testpass, "TPM2.0 Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_TPM20_SPI_TEST, DUMMY1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "TPM2.0 SPI Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);
}


static void
add_diag_intel_isp_test_pci_if_test_err_report(void)
{
    fru_table_offset = ISP_TEST;
    platform_fru_table[ISP_TEST].pid_string = isp_test;
    platform_fru_table[ISP_TEST].location_string = isp_test_loc;
    cterr_add_component("INTEL", "PCIe G3x4", "ISP TEST");
    cterr_add_reg_dump((PFV)display_intel_isp_pci_if_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the PCIe interface between INTEL and ISP test card",
    		"Check PCIe link speed and width in BIOS shell");
}
static int display_intel_isp_pci_if_test_reg (void)
{
    cterr_db_print("Test by linux lspci tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_intel_isp_test_pci_if_test
 *
 * Description:  Intel linux ISP Test card PCI bus info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_intel_isp_test_pci_if_test(void) {

    int rc;
    char val[]=INTEL_ISP_TEST_PCI_BUS;

    if (get_enhance_err_flag()) {
        add_diag_intel_isp_test_pci_if_test_err_report();
    }

    testname("INTEL ISP TEST CARD PCI BUS");
    prpass(testpass, "INTEL ISP Test card PCI Bus Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_PCI_IF_TEST,
            val, DUMMY2, DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL ISP Test card PCI Bus Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);

}

static void
add_diag_intel_isp_raid_pci_if_test_err_report(void)
{
    fru_table_offset = ISP_RAID;
    platform_fru_table[ISP_RAID].pid_string = isp_raid;
    platform_fru_table[ISP_RAID].location_string = isp_raid_loc;
    cterr_add_component("INTEL", "PCIe G3x4", "ISP RAID");
    cterr_add_reg_dump((PFV)display_intel_isp_raid_pci_if_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the PCIe interface between INTEL and ISP RAID card",
    		"Check PCIe link speed and width in BIOS shell");
}
static int display_intel_isp_raid_pci_if_test_reg (void)
{
    cterr_db_print("Test by linux lspci tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_intel_isp_raid_pci_if_test
 *
 * Description:  Intel linux ISP RAID card PCI bus info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_intel_isp_raid_pci_if_test(void) {

    int rc;
    char val[]=INTEL_ISP_RAID_PCI_BUS;

    if (get_enhance_err_flag()) {
        add_diag_intel_isp_raid_pci_if_test_err_report();
    }

    testname("INTEL ISP RAID CARD PCI BUS");
    prpass(testpass, "INTEL ISP RAID card PCI Bus Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_PCI_IF_TEST,
            val, DUMMY2, DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL ISP RAID card PCI Bus Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);

}

static void
add_diag_intel_isp_crypto_pci_if_test_err_report(void)
{
    fru_table_offset = ISP_CRYPTO;
    platform_fru_table[ISP_CRYPTO].pid_string = isp_crypto;
    platform_fru_table[ISP_CRYPTO].location_string = isp_crypto_loc;
    cterr_add_component("INTEL", "PCIe G3x4", "ISP CRYPTO");
    cterr_add_reg_dump((PFV)display_intel_isp_crypto_pci_if_test_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the PCIe interface between INTEL and ISP Crypto card",
    		"Check PCIe link speed and width in BIOS shell");
}
static int display_intel_isp_crypto_pci_if_test_reg (void)
{
    cterr_db_print("Test by linux lspci tool, no register dump");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: diag_intel_isp_crypto_pci_if_test
 *
 * Description:  Intel linux ISP crypto card PCI bus info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_intel_isp_crypto_pci_if_test(void) {

    int rc;
    char val[]=INTEL_ISP_CRYPTO_PCI_BUS;

    if (get_enhance_err_flag()) {
        add_diag_intel_isp_crypto_pci_if_test_err_report();
    }

    testname("INTEL ISP CRYPTO CARD PCI BUS");
    prpass(testpass, "INTEL ISP CRYPTO card PCI Bus Test");

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_INTEL_PCI_IF_TEST,
            val, DUMMY2, DUMMY3, DUMMY4);
    if (rc == FAILED) {
        cterr('f', 0, "INTEL ISP CRYPTO card PCI Bus Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (rc);

}
/**********************************************************************
 *
 * Function: diag_nc_intel_fw_version
 *
 * Description: Show Intel linux version
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_fw_version(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_INTEL_FW_VERSION, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_nim_fw_version
 *
 * Description: Show NIM version
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_nim_fw_version(void) {

    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_NIM_FW_VERSION, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_nc_intel_shutdown
 *
 * Description: Force Intel power shutdown
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_intel_shutdown(void) {
    int rc;

    rc = diag_nc_client_utility_entry(INTEL_SUB, DIAG_INTEL_SHUTDOWN, DUMMY1,
            DUMMY2, DUMMY3, DUMMY4);

    printf("nc command %s force INTEL shut down\n", __FUNCTION__);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_nc_f2w_i350_test
 *
 * Description: Executes Intel I350 testing
 *
 * Input : intf - f2w ethernet interface name
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_f2w_i350_test (char *intf) {

    int rc;
    char arg1[16], arg2[32], arg3[16];
    unsigned short flag = (NVRAM)->diagflag;
    
    sprintf(arg1, "%d", NGWIC1_SLOT);

    sprintf(arg2, "%s", intf); 

    printf("Testing Eth interface is: %s\n", arg2); 

    if (flag & D_EXT_LOOPBACK) {
        sprintf(arg3, "%d", 0);
    } else {
        sprintf(arg3, "%d", 1);
    }

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_F2W_I350_TEST, arg1, arg2,
            arg3, DUMMY4);
    if (rc == FAILED) {
        /* cterr on module func */
        return (FAILED);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_f2w_i350_relay_test
 *
 * Description: Executes Intel I350 relay loopback testing
 *
* Input : intf - f2w bypass relay ethernet intf
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_f2w_i350_relay_test (char *intf) {

    int rc;
    char arg1[16], arg2[16];

    sprintf(arg1, "%d", NGWIC1_SLOT);

    sprintf(arg2, "%s", intf);

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_F2W_I350_RELAY_TEST, arg1, arg2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        /* cterr on module func */
        return (FAILED);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_f2w_i350_led_test
 *
 * Description: Executes Intel I350 LED testing
 *
* Input : led test type
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_f2w_i350_led_test (char *intf) {

    int rc;
    char arg1[16];

    sprintf(arg1, "%s", intf);

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_F2W_I350_LED_TEST, arg1, DUMMY2,
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        /* cterr on module func */
        return (FAILED);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: diag_nc_f2w_i350_intf_up
 *
 * Description: for utility to bring up i350 intf
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_f2w_i350_intf_up (void) {

    int rc;
    char arg1[16];

    sprintf(arg1, "%d", NGWIC1_SLOT);

    rc = diag_nc_client_entry(INTEL_SUB, DIAG_F2W_I350_INTF_UP, arg1, DUMMY2, 
            DUMMY3, DUMMY4);
    if (rc == FAILED) {
        /* cterr on module func */
        return (FAILED);
    }

    return (rc);
}


/*---------------------------------------------------------------
$Log: diag_nc_client.c,v $
Revision 1.6  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.5  2017/01/25 01:13:13  kodko
Get the Fab Version and PCB Revision cookie field values to distinguish the board type and do the USB3.0/USB2.0 or USB3.0 only test.

Revision 1.4.2.2  2016/11/30 13:32:28  hondwang
Fix build image issue with enhance error message

Revision 1.4  2016/10/19 02:54:42  hondwang
Add I350 I2C and LED test

Revision 1.3  2016/06/04 09:22:20  alpeng
initial check in for f2w

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.24  2016/04/20 08:28:53  jimmyya
Fix X710 test to get testing failing correctly

Revision 1.1.2.23  2016/04/11 14:18:33  hondwang
Add TPM20 testing function

Revision 1.1.2.22  2016/03/28 04:16:46  benchen2
modify i350 lpbk test

Revision 1.1.2.21  2016/03/11 09:45:38  hondwang
Fix NC parameter type issue

Revision 1.1.2.20  2016/03/07 06:22:05  hondwang
Add I350 FPGA set for Tachi P2A board

Revision 1.1.2.19  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.18  2016/03/03 07:55:36  hondwang
Add I350 Fiber I2C testing

Revision 1.1.2.17  2016/03/03 04:55:59  alpeng
 add nim version into bmc util

Revision 1.1.2.16  2016/02/26 09:00:22  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.15  2016/02/24 02:57:33  hondwang
Remove NC checking log

Revision 1.1.2.14  2016/02/20 16:20:17  hondwang
Add CPU and PCI bus testing

Revision 1.1.2.13  2016/02/03 03:18:08  hondwang
to support intel core, pci bus, Tethx and Tsdx check

Revision 1.1.2.12  2016/01/20 07:13:56  hondwang
Modify for INTEL linux and lewis check utility and INTEL NC flag

Revision 1.1.2.11  2016/01/12 00:29:01  uid259484
modify to add INTEL NC utility show HDD, DIMM and linux version.
And add RAID card and BTB testing to daughter card item.

Revision 1.1.2.10  2016/01/06 00:49:36  jimmyya
Add Intel x710 test

Revision 1.1.2.9  2015/12/30 08:37:22  alpeng
 support nc test, check intel and lewis ready before testing

Revision 1.1.2.8  2015/12/29 12:31:06  alpeng
support get_mb_pid for check MB sku

Revision 1.1.2.7  2015/12/28 06:12:30  hondwang
Add and modify files for INTEL NC command support

Revision 1.1.2.6  2015/12/17 03:46:30  alpeng
support dreamliner nc and poe

Revision 1.1.2.5  2015/12/09 10:35:57  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.3  2015/12/01 02:04:36  alpeng
update nc infra structures and support testcard pcie test with nc

Revision 1.1.2.2  2015/11/25 06:12:12  benchen2
add bmc nc comm portion

Revision 1.1.2.1  2015/11/24 12:14:30  alpeng
add nc infrastructure


$Endlog$
*/

