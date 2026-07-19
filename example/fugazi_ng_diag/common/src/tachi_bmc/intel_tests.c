/* $Id: intel_tests.c,v 1.3 2017/03/30 08:30:54 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel_tests.c,v $
 *------------------------------------------------------------------
 *
 * intel_tests.c - INTEL test wraps.
 *
 * Jan 2016, Honda Wang adapted from Tachi.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "intel_tests.h"
#include "mon_plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_nc_common.h"
#include "diag_i2c_test.h"
#include "diag_geswitch_test.h"

/* INTEL test flag defines */
#define IF_1	(MF_CONTINUOUS | MF_DOGRP)
#define IF_2	(IF_1 | MF_DOALL)
#define IF_3	(IF_2 | MF_SHOW_ERRCOUNT)
#define IF_4	(IF_1 | MF_SHOW_ERRCOUNT)

/*
 * Global variables
 */

/*
 * Global extern functions
 */

/*
 * Local static functions
 */



int check_lewis_alive(int);
int check_intel_linux_alive(int);
int diag_ncsi_i350_test(void);

/* #define BYPASS_ENV  * */
extern int do_all_menu_items(struct menuinfo *);
extern int check_menu_flag(uint);

/* 
 * Sub Menu used for INTEL tests.
 */
submenu_xtable_t intel_tests_submenu_table[] = {
        { "I2c_Test", (PFT) diag_intel_i2c_scan_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "CPU_Test", (PFT) diag_nc_intel_cpu_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "MEM_Test", (PFT) diag_nc_intel_mem_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "HDD_Test", (PFT) diag_nc_intel_hdd_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "USB_Test", (PFT) diag_nc_intel_usb_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "USB20_Test", (PFT) diag_nc_intel_usb20_test, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "USB30_Test", (PFT) diag_nc_intel_usb30_test, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "SSD_Test", (PFT) diag_nc_intel_ssd_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "EMMC_Test", (PFT) diag_nc_intel_emmc_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "BMCUSB0_Test", (PFT) diag_nc_intel_bmcusb0_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "BMCUSB1_Test", (PFT) diag_nc_intel_bmcusb1_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "BMC I350 NCSI lpbk Test", (PFT) diag_ncsi_i350_test, 0, IF_4,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "I350_Test", (PFT) diag_nc_intel_i350_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "I350 Fiber I2c Test", (PFT) diag_intel_i350_fiber_i2c_test, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "X710_Test", (PFT) diag_nc_intel_x710_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "I210_Test", (PFT) diag_nc_intel_i210_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "GET_I350_Mode", (PFT) diag_nc_intel_get_i350_mode, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Reflash_I350_Mode", (PFT) diag_nc_intel_reflash_i350_mode, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Force_I350_Link", (PFT) diag_nc_intel_force_i350_link, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Check HDD/SSD size", (PFT) diag_nc_show_hdd_size, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Check DIMM size", (PFT) diag_nc_show_dimm_size, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Show Tethx interface info", (PFT) diag_show_teth_interfaces, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Show Tsdx device info", (PFT) diag_show_tsd_devices, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "CPU Core test", (PFT) diag_intel_cpu_core_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "PCI interface test", (PFT) diag_intel_pci_if_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "TPM20 test", (PFT) diag_nc_intel_tpm20_spi_test, 0, IF_3,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Check linux Version", (PFT) diag_nc_intel_fw_version, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
        { "Intel_Shutdown", (PFT) diag_nc_intel_shutdown, 0, IF_1,
          (type_t(*)()) 0, 0, (PFT) 0, 0 },
};
#define INTEL_TESTS_SUBMENU_TABLE_SIZE (sizeof(intel_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t intel_tests_primary_items[INTEL_TESTS_SUBMENU_TABLE_SIZE
        + MAX_BASE_ITEMS];
static mitem_t intel_tests_secondary_items[INTEL_TESTS_SUBMENU_TABLE_SIZE
        + MAX_BASE_ITEMS];

menuinfo_t intel_subtest_menu = { "%s Subtest Menu", 0, /* mtparam added by init_empty_menu */
(PFT) show_endnote, /* notes missing WICs in combos */
0, /* use generic prompt */
0, /* size (bumped by add_menu_item() */
intel_tests_primary_items, };
menuinfo_t *intel_submenup = &intel_subtest_menu;

/*-------------------------------------------------------------------
 *
 * Function: mb_tests()
 *
 * First build the primary & secondary submenus for the INTEL
 * diags based on the _xtable_ intel_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int intel_tests(boolean intel_test_items_executed) {
    sighandler_t old_handler;

    build_primary_submenu(intel_tests_submenu_table,
            INTEL_TESTS_SUBMENU_TABLE_SIZE, "INTEL", &intel_submenup);
    build_secondary_submenu(intel_tests_submenu_table,
            INTEL_TESTS_SUBMENU_TABLE_SIZE, intel_tests_secondary_items);

    testname("Check INTEL linux Ready");
    prpass(testpass, "Check INTEL linux Ready");
    /* Backup SIGNAL before system call  */
    old_handler = signal(SIGCHLD, SIG_DFL);
    if (check_intel_linux_ready()) {
        /* recover system call SIGNAL  */
        signal(SIGCHLD, old_handler);
        cterr('f', 0, "Check INTEL linux ready Failed");
        return (FAILED);
    }
    /* Display INTEL fw version */
    diag_nc_intel_fw_version();

    /* recover system call SIGNAL  */
    signal(SIGCHLD, old_handler);
    prcomplete(testpass, errcount, 0);

    if (intel_test_items_executed) {
        do_all_menu_items(&intel_subtest_menu);
    } else {
        menu(&intel_subtest_menu, intel_tests_secondary_items, '\0');
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: check_intel_linux_ready
 *
 * Description: Check INTEL linux ready for testing or not
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int check_intel_linux_ready(void) {

    /* INTEL linux alive */
    if (check_intel_linux_alive(PRE_PING_CHECK_LOOP) == PASSED) {
        printf("INTEL linux Ready.\n");
        return (PASSED);
    } else {
        printf("INTEL linux NOT Ready.\n");
        return (FAILED);
    }
}

/**********************************************************************
 *
 * Function: check_lewis_alive
 *
 * Description: Check INTEL power up or not
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int check_lewis_alive(int setloop) {
    char buf[128];
    int loop = 0;

    sprintf(
            buf,
            "%s",
            "ping 192.123.123.11 -c 3 -W 5 | grep -e packet | awk \'{print $7}\' | grep -e 100%");

    do {
        if (system(buf) == 0) {
            loop++;
        } else {
            return (PASSED);
        }

    } while (loop < setloop);

    return (FAILED);
}

/**********************************************************************
 *
 * Function: check_intel_linux_alive
 *
 * Description: Check INTEL power up or not
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int check_intel_linux_alive(int setloop) {
    char buf[128];
    int loop = 0;

    sprintf(
            buf,
            "%s",
            "ping 192.123.123.2 -c 1 -W 5 | grep -e packet | awk \'{print $7}\' | grep -e 100%");

    do {
        if (system(buf) == 0) {
            loop++;
        } else {
            return (PASSED);
        }

    } while (loop < setloop);

    return (FAILED);
}
/**********************************************************************
 *
 * Function: intel_linux_alive
 *
 * Description: Check INTEL linux alive or not
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int intel_linux_alive(void) {
    /* INTEL linux alive */
        if (check_intel_linux_alive(PRE_PING_CHECK_LOOP) == PASSED) {
            printf("INTEL linux Ready.\n");
        } else {
            printf("INTEL linux Not Ready.\n");
        }
        return (PASSED);
}

/**********************************************************************
 *
 * Function: intel_lewis_alive
 *
 * Description: Check INTEL linux alive or not
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int intel_lewis_alive(void) {
    /* INTEL linux alive */
        if (check_lewis_alive(PING_CHECK_LOOP) == PASSED) {
            printf("INTEL Lewis Ready.\n");
        } else {
            printf("INTEL Lewis Not Ready.\n");
        }
        return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_ncsi_i350_test
 *
 * Description: do ncsi lpbk test between bmc and i350
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************/
int diag_ncsi_i350_test(void) {
    int retval = PASSED;
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (retval);
    }
    
    if (diag_geswitch_i350_eth1_lpbk_test() == FAILED) {
        retval = FAILED;
    }
    return (retval);
}

/*---------------------------------------------------------------
 $Log: intel_tests.c,v $
 Revision 1.3  2017/03/30 08:30:54  hondwang
 Tachi-L brach merge

 Revision 1.2.14.1  2017/03/07 03:44:10  hondwang
 Move NCSI testing from do all

 Revision 1.2  2016/04/20 11:25:26  benchen2
 add tachi fru portion

 Revision 1.1.2.23  2016/04/11 14:18:33  hondwang
 Add TPM20 testing function

 Revision 1.1.2.22  2016/04/08 07:33:30  benchen2
 move i350 ncsi lpbk to intel

 Revision 1.1.2.21  2016/03/29 08:54:36  hondwang
 Add ping test loop to 3 and remove recover function

 Revision 1.1.2.19  2016/03/03 07:55:36  hondwang
 Add I350 Fiber I2C testing

 Revision 1.1.2.18  2016/02/26 09:00:22  hondwang
 add intel enhance error message, pci bus scan

 Revision 1.1.2.17  2016/02/25 04:11:56  jimmyya
 Export check_lewis & check_intel function

 Revision 1.1.2.16  2016/02/20 16:20:18  hondwang
 Add CPU and PCI bus testing

 Revision 1.1.2.15  2016/02/03 03:18:08  hondwang
 to support intel core, pci bus, Tethx and Tsdx check

 Revision 1.1.2.14  2016/01/20 07:13:56  hondwang
 Modify for INTEL linux and lewis check utility and INTEL NC flag

 Revision 1.1.2.13  2016/01/18 06:55:06  benchen2
 separate i2c scan test

 Revision 1.1.2.12  2016/01/12 07:37:37  jimmyya
 Fix error messages of TFTP_MVL_FILE

 Revision 1.1.2.11  2016/01/12 00:29:02  uid259484
 modify to add INTEL NC utility show HDD, DIMM and linux version.
 And add RAID card and BTB testing to daughter card item.

 Revision 1.1.2.10  2016/01/11 10:28:22  jimmyya
 Add lewis check functions

 Revision 1.1.2.9  2016/01/08 03:42:42  hondwang
 modify INTEL linux check funciton for shortly time

 Revision 1.1.2.8  2016/01/07 01:22:21  hondwang
 Set intel boot order to CDROM not CDROM:Virtual-CD

 Revision 1.1.2.7  2016/01/07 01:14:09  hondwang
 Add fail check with check_intel_linux_ready

 Revision 1.1.2.6  2016/01/06 01:36:52  jimmyya
 Add intel x710 test

 Revision 1.1.2.5  2015/12/31 08:44:49  uid259484
 Check in enforce BMC NCSI interface cycle code

 Revision 1.1.2.4  2015/12/30 12:10:12  hondwang
 Default will run RAID not X710 BTB testing

 Revision 1.1.2.3  2015/12/30 07:51:44  hondwang
 add power cycle function check

 Revision 1.1.2.2  2015/12/30 06:27:21  hondwang
 Add downlaod server path env require for image download

 Revision 1.1.2.1  2015/12/28 06:12:30  hondwang
 Add and modify files for INTEL NC command support


 $Endlog$
 */
