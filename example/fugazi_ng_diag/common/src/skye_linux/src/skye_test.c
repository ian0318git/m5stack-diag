/* $Id: skye_test.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_test.c,v $
 *******************************************************************************
 * File Name: skye_test.c
 *
 * Description: This file is for test functions
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c)2013~2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include "types.h"
#include "common.h"
#include "skye_main.h"
#include "common_utils.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "skye_comm_lib.h"
#include "nvmonvars.h"

extern int cpu0_ge_bp_lp_test(void);
extern int spirom_test(void);
extern int usb_slot_tests(int);
extern int eusb_slot_tests(int);
extern int skye_led_test(int);
extern int skye_i2c_scan_test(int);
extern boolean check_cpu0(void);
extern void diag_report_status_host(char *);
extern int memtest_do_all_wrapper(void);
extern int tlk10232_do_all_wrapper(void);
extern int phy_88E1514_do_all_wrapper(void);
extern int fpga_do_all_wrapper(void);
extern boolean cpu_id;
extern void clrerrlog(void);
extern void clrdblog(void);
extern int skye_check_pcie_lanes(void);

/**********************************************************************
 *
 * Function: skye_cpu_alive_test
 * This function test to make sure the CPU is alive and can response
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
skye_cpu_alive_test (void)
{
    printf("\nskye_cpu_alive_test completed\n");fflush(0);
    /* For this test, only need to send back the ACK */

    return (TO_HOST_CPU_ALIVE_TEST_OK);
}


/******************************************************************************
 *
 * Function: doall_print_head
 *
 * Description: This function prints out testname at the beginning of test
 *
 * Inputs      : teststr - Test String
 * Outputs     : None
 *
 *****************************************************************************/
static void doall_print_head (char *teststr)
{
    printf("\n--- Running %s Test ---\n", teststr);
}


/******************************************************************************
 *
 * Function: doall_print_tail
 *
 * Description: This function prints out testname at the end of test
 *
 * Inputs      : teststr - Test String
 * Outputs     : None
 *
 *****************************************************************************/
static void doall_print_tail (char *teststr)
{
    printf("\n--- %s Test PASS ---\n", teststr);
}


/******************************************************************************
 *
 * Function: skye_diag_do_all
 *
 * Description: This function performs all tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_do_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();
    
    /* For development stage */
    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        goto min_test;
    }
    doall_print_head("Memory");
    if (memtest_do_all_wrapper() == FAILED) {
        cterr('f', 0, "Main Memory Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("Memory");

#ifdef SKYE_P1A
    doall_print_head("88E1514");
    if (phy_88E1514_do_all_wrapper() == FAILED) {
        cterr('f', 0, "88E1514 Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("88E1514");
#endif   /* SKYE_P1A */

    doall_print_head("I2C Device Scan");
    if (skye_i2c_scan_test(0) == FAILED) {
        cterr('f', 0, "I2C Device Scan Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("I2C Device Scan");

    /* FPGA Interrupt test use gxio that need to fix also in the menu */
    doall_print_head("FPGA");
    if (fpga_do_all_wrapper() == FAILED) {
        cterr('f', 0, "FPGA Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("FPGA");

    doall_print_head("SPI-ROM");
    if (spirom_test() == FAILED) {
        cterr('f', 0, "SPI-ROM Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("SPI-ROM");

min_test: // DEBUG
    /* CPU 0 test only */
    if (cpu_id == MASTER_CPU) {
        doall_print_head("TLK 10232");
        if (tlk10232_do_all_wrapper() == FAILED) {
            cterr('f', 0, "TLK 10232 Test Fails");
            diag_report_status_host(DIAG_RTN_FAIL_STR);
            return (FAILED);
        }
        doall_print_tail("TLK 10232");
    }

    /* For Skye 2-CPUs version, only do PCIe device scan test in CPU1 */
    if (cpu_id == SLAVE_CPU) {
        doall_print_head("PCIe lanes Scan");
        if (skye_check_pcie_lanes() != PASSED) {
            cterr('f', 0, "PCIe Device Scan Test Fails");
            diag_report_status_host(DIAG_RTN_FAIL_STR);
            return (FAILED);
        }
        doall_print_tail("PCIe lanes Scan");
    }

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: skye_diag_mem_test_all
 *
 * Description: This function performs mem tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_mem_test_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();

    doall_print_head("Memory");
    if (memtest_do_all_wrapper() == FAILED) {
        cterr('f', 0, "Main Memory Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("Memory");

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: skye_diag_fpga_test_all
 *
 * Description: This function performs fpga tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_fpga_test_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();

    /* FPGA Interrupt test use gxio that need to fix also in the menu */
    doall_print_head("FPGA");
    if (fpga_do_all_wrapper() == FAILED) {
        cterr('f', 0, "FPGA Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("FPGA");

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: skye_diag_spirom_test_all
 *
 * Description: This function performs spirom tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_spirom_test_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();

    doall_print_head("SPI-ROM");
    if (spirom_test() == FAILED) {
        cterr('f', 0, "SPI-ROM Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("SPI-ROM");

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: skye_diag_spirom_test_all
 *
 * Description: This function performs i2c dev tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_i2c_dev_test_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();

    doall_print_head("I2C Device Scan");
    if (skye_i2c_scan_test(0) == FAILED) {
        cterr('f', 0, "I2C Device Scan Test Fails");
        diag_report_status_host(DIAG_RTN_FAIL_STR);
        return (FAILED);
    }
    doall_print_tail("I2C Device Scan");

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: skye_diag_tlk_test_all
 *
 * Description: This function performs tlk dev tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_tlk_test_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();

    if (cpu_id == MASTER_CPU) {
        doall_print_head("TLK 10232");
        if (tlk10232_do_all_wrapper() == FAILED) {
            cterr('f', 0, "TLK 10232 Test Fails");
            diag_report_status_host(DIAG_RTN_FAIL_STR);
            return (FAILED);
        }
        doall_print_tail("TLK 10232");
    }

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: skye_diag_pcie_test_all
 *
 * Description: This function performs PCIe lanes scan test
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
long skye_diag_pcie_test_all (char *opt)
{
    /* Remove dblog and errlog before run the test */
    clrerrlog();
    clrdblog();

    if (cpu_id == SLAVE_CPU) {
        doall_print_head("PCIe lanes Scan");
        if (skye_check_pcie_lanes() != PASSED) {
            cterr('f', 0, "PCIe Device Scan Test Fails");
            diag_report_status_host(DIAG_RTN_FAIL_STR);
            return (FAILED);
        }
        doall_print_tail("PCIe lanes Scan");
    }

    diag_report_status_host(DIAG_RTN_PASS_STR);

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * $Log: skye_test.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:36  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.12  2015/01/22 09:07:47  palin2
 * Moved up I2C scan test ordering in Skye Diag main tests.
 *
 * Revision 1.1.2.11  2014/11/27 07:25:20  palin2
 * 1. Fixed PCIe lanes Scan test.
 * 2. Added PCIe lanes Scan test to 2-CPUs Skye default tests.
 * 3. Added SKYE_P1A compile flag to tell difference between P1A and P1B.
 *
 * Revision 1.1.2.10  2014/11/27 02:32:50  steja
 * 1.Fix the intermittent failure to run do all test(CSCur27613)
 * 2.Update TLK full data path by ping test.
 *
 * Revision 1.1.2.9  2014/11/21 09:37:34  steja
 * Support Full data path loopback for 10G-KR by ping test
 *
 * Revision 1.1.2.8  2014/09/24 08:04:58  steja
 * Minor fix remove "***" and arrange the TLK to the last test item for do all
 *
 * Revision 1.1.2.7  2014/09/18 07:18:43  steja
 * 1.Update NC command codei
 * 2.Update enhanced error message
 *
 * Revision 1.1.2.6  2014/09/16 15:32:54  steja
 * for debug purpose to add min test time for nc do all test
 *
 * Revision 1.1.2.5  2014/09/12 14:38:43  steja
 * Update code for CPU do all test
 *
 * Revision 1.1.2.4  2014/08/28 02:54:26  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.3  2014/08/14 12:24:25  steja
 * Remove debug message and add printf info for internal loopback
 *
 * Revision 1.1.2.2  2014/08/08 08:34:33  steja
 * Add Do all test
 *
 * Revision 1.1.2.1  2014/07/21 01:56:56  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------------------
 * skye_test.c:
 * Revision 1.2  2014/02/27 15:01:44  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.3  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.4.2  2013/09/13 07:00:09  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/08/15 11:30:33  steja
 * Add code command and respond ( Host <->GE <-> TILE CPU#0) for G2 (PPC & MIPS) platform
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 */
