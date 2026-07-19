/* $Id: mb_tests.c,v 1.10 2019/06/14 09:59:28 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * March 2016, Sofian Teja adapted from Xformers.
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
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
#include "mb_tests.h"
#include "mon_plat_defs.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_plug_fpga.h"
#include "plug_host_fpga_lib.h"
#include "plug_common_host_impl.h"
#include "diag_reset_button.h"
#include "usb_dongle_common_test.h"
#include "usb_dongle_common_host_impl.h"

/*
 * Global variables
 */
fru_table_t platform_fru_table[];
static boolean flag_ecc = FALSE;

/* Local functions */

int usb_storage_rd_wr_tests(int);
int emmc_tests(int);
int bootflash_tests(int);
boolean tsn_has_2nd_ge(int);
int ecc_tests(void);

/*
 * Global extern functions
 */
extern int esw_diag_test(int);

extern int linux_memory_tester(int);
extern int usb_get_info(void);
extern int usb_slot_tests(int);
extern int emmc_slot_tests(int);
extern int spi_slot_tests(int);
extern int tsn_x64_i2c_scan_test(int);
extern int tsn_i2c_temp_reg_rw_test(int);
extern void time_validity_test_wrapper(void);
extern int build_rtc_menu(boolean);
extern int do_all_menu_items(struct menuinfo *);
extern int quiet_launch;
extern int diag_fpga(int);
extern int fpga_reset_32_api(uint, uint, uint, uint);
extern int fpga_read_32_reg(uint, uint *);
extern int check_dsl_sku(uint *);
extern int cpu_core_test(int);
extern int build_cpu_test_menu(int);
extern int tsn_leds_test(int);
extern int build_led_test_menu(int);
extern int tsn_gephy0_diag(int);
extern int tsn_gephy1_diag(int);
extern int tsn_mem_read32(uint, uint *);
extern int tsn_mem_write32(uint, uint);
extern int build_snsr_menu(boolean);
extern void build_wifi_snsr_menu(void);
extern int tsn_display_temp_errormsg(void);
extern int usb_get_speed(int);
extern boolean has_wifi_temp(int);

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar mb_loc[] = "MB";

fru_table_t platform_fru_table[] = {
    {mb_pid, mb_loc},
};

/*
 * Sub Menu used for "Main menu -> motherboard test"
 */
submenu_xtable_t mb_tests_submenu_table[] = {
    {"Main memory test with cache on",
     (PFT) linux_memory_tester, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) linux_memory_tester, TRUE},

    {"External USB test",
    (PFT) usb_dongle_test_entry, USB_SLOT0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) usb_dongle_test_entry, USB_SLOT0 + USB_MAX_SLOT_NO},
    
    {"eMMC test",
    (PFT) emmc_tests, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},
    
    {"Bootflash test",
    (PFT) bootflash_tests, TSN_BF_BUSNUM,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},
    
    {"I2C scan test",
    (PFT) tsn_x64_i2c_scan_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (type_t(*)())0, 0},

    {"M/B Temperature test",
    (PFT) build_snsr_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_snsr_menu, TRUE},

    {"WiFi Temperature test",
    (PFT) build_wifi_snsr_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_wifi_temp, 0,
    (PFT) build_wifi_snsr_menu, TRUE},

    {"RTC test",
    (PFT) time_validity_test_wrapper, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_rtc_menu, 0},

    {"FPGA test",
     (type_t(*)())diag_fpga, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())diag_fpga, TRUE},

    {"GE PHY 0 test",
     (type_t(*)())tsn_gephy0_diag, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())tsn_gephy0_diag, TRUE},

    {"GE PHY 1 test",
     (type_t(*)())tsn_gephy1_diag, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())tsn_has_2nd_ge, 0,
     (type_t(*)())tsn_gephy1_diag, TRUE},

    {"Ethernet Switch test",
     (type_t(*)())esw_diag_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())esw_diag_test, TRUE},

    {"CPU core test",
    (PFT) cpu_core_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_cpu_test_menu, FALSE},

    {"LED test",
    (PFT) tsn_leds_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_led_test_menu, FALSE},

    {"Reset button test",
    (PFT)tsn_reset_button_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)0,         0},

    {"Pluggabble FPGA test",
     (type_t(*)())diag_plug_fpga, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())has_plug_slot, PLUG_SLOT_1,
     (type_t(*)())diag_plug_fpga, TRUE},
};

#define MB_TESTS_SUBMENU_TABLE_SIZE (sizeof(mb_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "Main menu -> motherboard test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mb_tests_primary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t mb_tests_secondary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t mb_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    mb_tests_primary_items,
};

menuinfo_t *mb_submenup = &mb_subtest_menu;


/**********************************************************************
 *
 * Function   : tsn_has_2nd_ge
 * Description: Function to check if this board has GE1 feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean tsn_has_2nd_ge (int opt)
{
    uint b_type = 0;

    if (tsn_get_boardtype(&b_type) != PASSED) {
        printf("%s: Failed to get Board Type.\n", __FUNCTION__);
        return (FALSE);
    }

    if ((b_type & TSN_W_GE1) == TSN_W_GE1) {
        return (TRUE);
    }
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function: mb_tests()
 *
 * First build the primary & secondary submenus for the motherboard
 * diags based on the _xtable_ mb_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int mb_tests (boolean mb_test_items_executed)
{
    int rc = FAILED;

    build_primary_submenu(mb_tests_submenu_table,
                          MB_TESTS_SUBMENU_TABLE_SIZE, "Motherboard",
                          &mb_submenup);

    build_secondary_submenu(mb_tests_submenu_table,
                            MB_TESTS_SUBMENU_TABLE_SIZE,
                            mb_tests_secondary_items);

    if (mb_test_items_executed) {
        do_all_menu_items(&mb_subtest_menu);
    } else {
        menu(&mb_subtest_menu, mb_tests_secondary_items, '\0');
    }

    return (rc);
}

/*
 * Function: init_hotplug_sock
 *
 * Description : Open a NETLINK socket.
 *
 * Inputs: N/A
 *
 * Output: socket id
 */
static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));

    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1) {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
 
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }

    return hotplug_sock;
}

/*
 * Function: usb_storage_rd_wr_tests
 *
 * Description : usb r/w tests.
 *
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */

int usb_storage_rd_wr_tests (int slot)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB3.0", "USB3.0 Port");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the USB port.",
                    "If there is no problem for these interfaces, "
                    "replace one USB device and redo the test.");
#endif
    int usb_speed = 0;
    int rc = FAILED;
    char *tname = "";
    int ret = -1;
    int hotplug_sock = -1;
    fd_set fdSocks;
    struct timeval  tTimeout;
    int numReaders = 0;

    if ( slot == USB_SLOT0 ) {
        tname = "External USB 0";
    } else if (slot == USB_SLOT1) {
        tname = "External USB 1";
    } else if (slot == USB_SLOT2) {
        tname = "External USB 2";
    } else {
        cterr('f', 0, "Unsupported slot!");
        return (FAILED);
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* External loopback only for external USB */
    if ((slot == USB_SLOT0) || (this_is_star()) || (this_is_supernova())) {
        /*
         * D_EXT_LOOPBACK = 0, enable ext. loopback
         * * D_EXT_LOOPBACK = 1, disable ext. loopback
         */
        if (check_menu_flag(D_EXT_LOOPBACK)) {
            prpass(testpass, "\n External loopback flag is off, skip '%s' \
                external loopback test. ", tname);
	        prcomplete(testpass, errcount, (char *)0);
            return (PASSED);
        }
    }

    /*
     * testname is printed on usb_slot_tests
     */
    /* Test USB Auto(3.0) mode */
    if (usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }

    usb_speed = usb_get_speed(slot);
    if (usb_speed != USB3) {
        cterr('f', 0, "USB 3.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = usb_slot_tests(slot);
        if (rc == PASSED) {
            prpass(testpass, "%s 3.0 read/write test passed, \n", tname);
        } else {
            cterr('f', 0, "USB 3.0 read/write test failed (%d)", usb_speed);
            return (FAILED);
        }
    }

    //Hotplug Socket Initialization
    hotplug_sock = init_hotplug_sock();
    
    if (slot == USB_SLOT0) {
    /* Test USB 2.0 mode */
    msleep(DELAY_USBCMD);
    system(USB_POWER_OFF_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_20_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_20_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_20_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_POWER_ON_CMD);
    msleep(DELAY_USBCMD); 
    } else if (slot == USB_SLOT1) {
        /* Test USB 2.0 mode */
        msleep(DELAY_USBCMD);
        system(USB_POWER_OFF_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_20_CUSTOM_REG1_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_20_CUSTOM_REG2_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_MISC_CTRL_1_REG_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_20_CTRL_REG_CMD);
        msleep(DELAY_SYSCMD);
        system(USB_POWER_ON_CMD);
        msleep(DELAY_USBCMD); 
    } else {
        cterr('f', 0, "Wrong slot! for test USB 2.0 mode");
            if (hotplug_sock >= 0) {
                close(hotplug_sock);
            }
        return (FAILED);
    } 
    
    //Listen to Kernel hotplug uevent
    while (hotplug_sock) {
        char buf[UEVENT_BUFFER_SIZE] = {0};
        FD_ZERO(&fdSocks);
        FD_SET(hotplug_sock, &fdSocks);
        tTimeout.tv_sec  = 1;	/* kSocketTimeOut */
        tTimeout.tv_usec = 0;
        numReaders = select(hotplug_sock+1, &fdSocks, NULL, NULL, &tTimeout);
        /* if timeout, report it */
        if (0 == numReaders) {
            printf("socket receive uevent timeout!\n");
            break;
        }
        ret = recv(hotplug_sock, &buf, sizeof(buf), 0); 
        if (ret) {
            if (strstr(buf, "add@/devices") && strstr(buf, "usb") && strstr(buf, "/block/sd")) {
                break;
            }
        }
    }
    
    if (hotplug_sock >= 0) {
        close(hotplug_sock);
    }
    
    if (usb_get_info() == FAILED) {
        printf("usb_get_info() failed\n");
    }

    usb_speed = usb_get_speed(slot);
    if (usb_speed != USB2) {
        cterr('f', 0, "USB 2.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = usb_slot_tests(slot);
        if (rc == PASSED) {
            prpass(testpass, "%s 2.0 read/write test passed, ", tname);
        } else {
            cterr('f', 0, "USB 2.0 read/write test failed (%d)", usb_speed);
            return (FAILED);
        }
    }

    if (slot == USB_SLOT0) {
    /* Recover to USB Auto(3.0) mode */
    msleep(DELAY_USBCMD); 
    system(USB_POWER_OFF_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_30_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_30_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_30_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_POWER_ON_CMD);
    msleep(DELAY_USBCMD); 
    } else if (slot == USB_SLOT1) {
        /* Recover to USB Auto(3.0) mode */
        msleep(DELAY_USBCMD); 
        system(USB_POWER_OFF_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_30_CUSTOM_REG1_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_30_CUSTOM_REG2_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_MISC_CTRL_1_REG_CMD);
        msleep(DELAY_SYSCMD);
        system(USB1_30_CTRL_REG_CMD);
        msleep(DELAY_SYSCMD);
        system(USB_POWER_ON_CMD);
        msleep(DELAY_USBCMD); 
    } else {
        cterr('f', 0, "Wrong slot! for test USB 3.0 mode");
        return (FAILED);
    } 

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/*
 * Function: emmc_tests
 *
 * Description : emmc r/w tests.
 *
 * Inputs: option for future use
 *
 * Output: PASSED/FAILED
 */
int emmc_tests (int option)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "SDIO/MMC", "eMMC Storage Flash");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the eMMC.",
                    "If there is no problem for these interfaces, "
                    "replace one eMMC and redo the test.");
#endif

    int rc = FAILED;
    char *tname = "eMMC";

    /*
     * we only have one emmc
     */
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (!quiet_launch) {
        prpass(testpass, "%s read/write, ", tname);
    }
    /*
     * testname is printed on usb_slot_tests
     */
    rc = emmc_slot_tests(option);
    if (rc == FAILED) {
        cterr('f', 0, "%s test failed.", tname);
        return (rc);
    }
    if (!quiet_launch) {
        prpass(testpass, "%s read/write test passed, ", tname);
    }
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/*
 * Function: insert_spi_module
 *
 * Description : insert spi module driver.
 *
 * Inputs: mode TRUE (Insert)
 *              FALSE (Removed) 
 *
 * Output: PASSED/FAILED
 */
void insert_spi_module (boolean mode)
{
     if (mode == TRUE) {
         system(INSMOD);
     } else {
         system(RMMOD);
     }
}


/*
 * Function: bootflash_tests
 *
 * Description : bootflash r/w tests.
 *
 * Inputs: slot - bootflash slot num
 *
 * Output: PASSED/FAILED
 */
int bootflash_tests (int slot)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
    */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "SPI", "SPI UEFI Flash");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Boot up is OK means interfaces between Host SoC and flash is OK.",
                    "If there is no problem for these interfaces, "
                    "replace one flash and redo the test.");
#endif

    char *tname = "Bootflash";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /*
     * testname is printed on spi_slot_tests
     */
    /*
     * tsn uses spi flash
     */
    if (spi_slot_tests(slot) != PASSED) {
        cterr('f', 0, "SPI bootflash test failed.");
        return (FAILED);
    }
    return (PASSED);
}


/*
 * Function: xdsl_eth2_linkup 
 *
 * Description : Disable Auto Nego and enable eth2 force link up
 *
 * Inputs: none
 *
 * Output: PASSED/FAILED
 */
int xdsl_eth2_linkup (void)
{
    int rc = PASSED;
    uint reg_offset = 0, orig_val = 0, reg_val = 0;
    reg_offset = GIGA_PORT_3_ADDR;
    /* Read the oringnal value form CPU Register */
    if (tsn_mem_read32(reg_offset, &orig_val) != PASSED) {
        printf("Failed to read FPGA register 0x%08X.\n", reg_offset);
        return (FAILED);
    }

    reg_val = orig_val & RESET_VAL;
    if (this_is_tsn_gshdsl_sku() == TRUE) {
        /* Disable Auto Nego and Force in 1Gbps */
        reg_val = reg_val | DIS_AN_1G_SPD; 
    } else {
        /* Disable Auto Nego and Enable Force Link Up in 1Gbps */
        reg_val = reg_val | DIS_AN_EN_FORCE_1G_SPD; 
    }
    /* WR CPU register*/
    if (tsn_mem_write32(reg_offset, reg_val) != PASSED) {
        printf("Failed to write CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
             printf("Done writing 0x%08X to CPU register(0x%08X).\n",
               reg_val, reg_offset);
        }
    }
    
    if (this_is_tsn_gshdsl_sku() == TRUE) {
        reg_offset = GIGA_PORT_3_MAC_CTL_REG2;
        /* RD CPU Register */
        if (tsn_mem_read32(reg_offset, &orig_val) != PASSED) {
            printf("Failed to read FPGA register 0x%08X.\n", reg_offset);
            return (FAILED);
        }
        /* Clear Port MAC Control Register 2 bits 9:7 to 0 to default value. */
        reg_val = orig_val & CLEAR_RESERVED_BITS;
        /* WR CPU register*/
        if (tsn_mem_write32(reg_offset, reg_val) != PASSED) {
            printf("Failed to write CPU register 0x%08X.\n", reg_offset);
            return (FAILED);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Done writing 0x%08X to CPU register(0x%08X).\n",
                   reg_val, reg_offset);
            }
        }
    }

    return (rc); 
}

/*
 * Function: init_eth2
 *
 * Description : Initialize eth2 interface to force linkup.
 *
 * Inputs: N/A
 *
 * Output: None
 */ 
void init_eth2 (void)
{
    if (tsn_has_2nd_ge(0) == FALSE) {
        if (xdsl_eth2_linkup() != PASSED) {
            cterr('f', 0, "Failed to config GE Force link up.");
        }
    }
    return;
}

/*
 * Function: ecc_tests
 *
 * Description : ddr ecc tests (not verified yet)
 *
 * Inputs: N/A
 *
 * Output: PASSED/FAILED
 */ 
int ecc_tests (void)
{
    int rc = PASSED;
    char *tname = "ecc_tests";

    testname("%s", tname);

    if (flag_ecc) {
        flag_ecc = FALSE;
        rc = system("rmmod ecc_timer.ko");
        printf("DDR ECC disabled\n");
    }
    else {
        flag_ecc = TRUE;
        rc = system("insmod /diag/ecc_timer.ko");
        printf("DDR ECC enabled\n");
    }

    return (rc);
}


/*-------------------------------------------------
$Log: mb_tests.c,v $
Revision 1.10  2019/06/14 09:59:28  steja
Supported Cooper usb dongle LTE

Revision 1.9  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.8  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.7.38.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.7  2018/05/09 06:53:12  letsai
Add TSN GSHDSL portion

Revision 1.6  2018/02/27 03:53:30  hondwang
Modify for follow TSN MB test item order

Revision 1.5  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.4  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.3.6.3  2018/02/08 07:16:05  lucywang
Merged LTE USB2.0 detect test from trunk

Revision 1.3.6.2  2018/02/07 10:23:01  lucywang
Followed coding rule

Revision 1.3.6.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.3  2017/12/01 13:45:20  palin2
Added support RESET button test (CSCvg96921).

Revision 1.2.4.4  2017/12/05 02:47:09  lucywang
Sync from TSN trunk : Added support RESET button test (CSCvg96921).

Revision 1.2.4.3  2017/10/16 02:56:36  lucywang
Wait USB attached event then get storage information to avoid empty device name that caused USB test failed

Revision 1.2.4.2  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:46  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/25 08:31:55  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.15.2.9  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.15.2.8  2017/07/18 06:10:36  steja
Code cleanup

Revision 1.1.4.15.2.7  2017/07/05 14:05:24  steja
Enhance code readability

Revision 1.1.4.15.2.6  2017/05/17 01:17:53  palin2
Updated GE WAN mapping number with team's decision.
(GE0: GE WAN with SFP; GE1: 2nd GE WAN)

Revision 1.1.4.15.2.5  2017/04/14 00:52:15  steja
Fix USB switch mode 3.0/2.0 issue (CSCvd89346)

Revision 1.1.4.15.2.4  2017/04/13 13:10:28  palin2
Updated to support TSN-M E2E SKU.

Revision 1.1.4.15.2.3  2017/04/05 08:22:02  steja
Temporarily workaround for EDVT test (CSCvd04737) keep investigating with Marvell

Revision 1.1.4.15.2.2  2017/03/08 14:50:27  steja
Fix External USB test for USB3.0Hub

Revision 1.1.4.15.2.1  2017/02/23 11:03:16  palin2
Updated code based on FPGA changes. These updates are verified on P2A TSN.

Revision 1.1.4.15  2017/02/10 15:10:44  petteng
Modify USB 3.0 & 2.0 test

Revision 1.1.4.14  2016/12/08 13:54:51  steja
Support ECC kernel module timer interrupt

Revision 1.1.4.13  2016/11/25 08:11:42  steja
Fix CSCvc13983: TSN-M VDSL bootup fail randomly during EEDVT

Revision 1.1.4.12  2016/11/01 07:29:20  petteng
Add enhanced error message

Revision 1.1.4.11  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.10  2016/09/13 08:14:23  palin2
Added CPU to GE PHY MAC loopback test.

Revision 1.1.4.9  2016/08/29 13:18:44  palin2
Update GE0 check function for TSN-M.

Revision 1.1.4.8  2016/08/23 08:14:17  steja
Add MB Temperature interrupt test

Revision 1.1.4.7  2016/07/25 09:32:06  steja
Add Wlan DC present or not present

Revision 1.1.4.6  2016/07/18 13:14:29  steja
1. Move M/B Temperature sensor register test to run as default test.
2. Move M/B Temperature utilities under basic utilities.

Revision 1.1.4.5  2016/07/17 11:15:16  palin2
Added function to distinguish bwteen TSN-H and TSN-M.

Revision 1.1.4.4  2016/07/15 14:39:28  steja
Add code for DSL sku to force link up eth2 in diag.

Revision 1.1.4.3  2016/07/10 10:29:33  steja
Add LED test

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.23  2016/06/17 12:30:20  steja
Add prpass

Revision 1.1.2.22  2016/05/25 13:59:29  steja
Support eMMC test

Revision 1.1.2.21  2016/05/16 06:44:55  palin2
Add function to get TSN board type, and config Diag test items in menu for
different SKUs based on its board type info.

Revision 1.1.2.20  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.19  2016/05/05 04:54:41  steja
Workaround for SPI bootflash test

Revision 1.1.2.18  2016/05/03 16:01:58  palin2
Added GE PHY register test.

Revision 1.1.2.17  2016/04/29 11:24:04  steja
Temp remove bootflash

Revision 1.1.2.16  2016/04/29 10:14:56  palin2
Updated code and added support ext. loopback test after bring up Switch.

Revision 1.1.2.15  2016/04/29 10:07:20  steja
skip RTC and eMMC test

Revision 1.1.2.14  2016/04/23 15:00:29  steja
Check in for fix SPD Read RAW

Revision 1.1.2.13  2016/04/22 12:28:36  palin2
Updated code after bring up GE PHY external loopback test.

Revision 1.1.2.12  2016/04/22 11:34:00  steja
check-in for first release

Revision 1.1.2.11  2016/04/14 06:09:49  palin2
1. Removed cpld.c and cpld.h because TSN don't have CPLD.
2. Linked related function to correct FPGA one.

Revision 1.1.2.10  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.9  2016/03/25 08:05:40  steja
Add CPU Stress tools

Revision 1.1.2.8  2016/03/24 03:58:26  steja
Add LTE test

Revision 1.1.2.7  2016/03/23 03:31:11  palin2
Added FPGA Diag.

Revision 1.1.2.6  2016/03/20 14:06:28  steja
Add RTC test and utilities

Revision 1.1.2.5  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility

Revision 1.1.2.4  2016/03/16 13:36:56  steja
Add bootflash test, need FPGA function

Revision 1.1.2.3  2016/03/16 10:24:50  steja
Add EMMC test

Revision 1.1.2.2  2016/03/16 08:57:54  steja
add usb test

Revision 1.1.2.1  2016/03/14 14:32:03  steja
Add memory test


*/


