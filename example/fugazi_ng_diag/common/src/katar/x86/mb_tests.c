/* $Id: mb_tests.c,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "slot.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "mon_plat_defs.h"
#include "setjmps.h"
#include "platform_env.h"
#include "platform_pwr_seq.h"
#include "proto.h"
#include "platform_psu.h"
#include "linux_usb_test.h"
#include "dash_fpga.h"
#include "linux_api.h"
#include "cli_cmd.h"
#include "plat_defs.h"
#include "linux_pciutils.h"
#include "queryflags.h"

/* M/B test flag defines */
#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/*
 * Global extern functions
 */
extern void display_uart_regs_cterr_wrapper(void);
extern int display_multiboot(int);
extern void show_margins_cterr_wrapper(void);
extern void show_temp_cterr_wrapper(void);
extern int get_mb_pid(char *);
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern int get_pcie_cap_struct_ptr(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_cap(uint32_t, uint16_t, int, uint);

/* #define BYPASS_ENV  * */
extern int linux_memory_tester(int);
extern int linux_memory_tester_with_ecc_check(int);
extern int test_not_avail_yet (int);
extern int do_all_menu_items(struct menuinfo *);
extern int mainmem_test(void);
extern int spi_flash_test(boolean menu_option);
extern int compactflash_main(int);

extern int katar_x86_i2c_scan_test(int);
extern int katar_dash_rd_wr_test(int);
extern int katar_mb_ge_phy_test (int dummy);
extern int katar_mb_ge_phy_cross_port_test (int dummy);
extern int katar_mb_ge10_phy_cross_port_test (int dummy);
extern int katar_mb_ge2p5_phy_cross_port_test (int dummy);
extern int aqr_register_read_write_util (void);
extern int katar_2p5_phy_cross_test_rdt(void);
extern int check_menu_flag(uint);
extern int pcie_rd_reg (uint32_t reg_addr);
extern int rtc_tests (int dummy);
extern int katar_mb_fan_high_test(int dummy);
extern int katar_mb_fan_low_test (int dummy);
extern int katar_mb_force_intr_test (int dummy);
extern int aikido_reg_test (void);

//static int aux_loopback_test(int dummy);
static int usb_tests(int);
static int emmc_tests(int);
int katar_pcie_lane_scan_test(void);
int usb_exist(int);
int katar_is_sfp_sku(void);
int cpu_core_test (int do_more_test);

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";
uchar psu_pid[] = "PSU-PID";
uchar rps_pid[] = "RPS-PID";
uchar pvdm_pid[] = "PVDM-PID";
uchar backplane_pid[] = "Backplane-PID";
uchar risercard_pid[] = "RiserCard-PID";
uchar mb_emmc_loc[] = "MB/eMMC";
uchar mb_eusb_loc[] = "MB/eUSB";
uchar mb_msata_loc[] = "MB/mSATA";
uchar sm_pid[] = "SM-PID";
uchar wic_pid[] = "WIC-PID";
uchar dc_pid[] = "DC-PID";
uchar mb_loc[] = "MB";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar sfp0_loc[] = "MB/SFP0";
uchar sfp1_loc[] = "MB/SFP1";
uchar sfp2_loc[] = "MB/SFP2";
uchar sfp3_loc[] = "MB/SFP3";
uchar psu0_loc[] = "MB/PSU0";
uchar psu1_loc[] = "MB/PSU1";
uchar rps_loc[] = "MB/RPS";
uchar pvdm0_loc[] = "MB/PVDM0";
uchar backplane_loc[] = "MB/Backplane";
uchar risercard_loc[] = "MB/RiserCard";
uchar sm0_loc[] = "MB/SM0";
uchar sm1_loc[] = "MB/SM1";
uchar wic0_loc[] = "MB/WIC0";
uchar wic1_loc[] = "MB/WIC1";
uchar wic2_loc[] = "MB/WIC2";
uchar sm0wic_loc[] = "SM0/WIC";
uchar sm1wic_loc[] = "SM1/WIC";
uchar sm0pvdm0_loc[] = "SM0/PVDM0";
uchar sm0pvdm1_loc[] = "SM0/PVDM1";
uchar sm0pvdm2_loc[] = "SM0/PVDM2";
uchar sm1pvdm0_loc[] = "SM1/PVDM0";
uchar sm1pvdm1_loc[] = "SM1/PVDM1";
uchar sm1pvdm2_loc[] = "SM1/PVDM2";
uchar sm0wic0dc_loc[] = "SM0/WIC0/DC";
uchar sm1wic0dc_loc[] = "SM1/WIC0/DC";
uchar sm0wic1dc_loc[] = "SM0/WIC1/DC";
uchar sm1wic1dc_loc[] = "SM1/WIC1/DC";
uchar sm0dc_loc[] = "SM0/DC";
uchar sm1dc_loc[] = "SM1/DC";


fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { mb_pid,        mb_emmc_loc},
    { mb_pid,        mb_eusb_loc},
    { mb_pid,        mb_msata_loc},
    { dimm_pid,      dimm0_loc },
    { dimm_pid,      dimm1_loc },
    { sfp_pid,       sfp0_loc },
    { sfp_pid,       sfp1_loc },
    { sfp_pid,       sfp2_loc },
    { sfp_pid,       sfp3_loc },
    { psu_pid,       psu0_loc },
    { psu_pid,       psu1_loc },
    { rps_pid,       rps_loc },
    { pvdm_pid,      pvdm0_loc },
    { backplane_pid, backplane_loc },
    { risercard_pid, risercard_loc },
    { sm_pid,        sm0_loc },
    { sm_pid,        sm1_loc },
    { wic_pid,       wic0_loc },
    { wic_pid,       wic1_loc },
    { wic_pid,       wic2_loc },
    { wic_pid,       sm0wic_loc },
    { wic_pid,       sm1wic_loc },
    { pvdm_pid,      sm0pvdm0_loc },
    { pvdm_pid,      sm0pvdm1_loc },
    { pvdm_pid,      sm0pvdm2_loc },
    { pvdm_pid,      sm1pvdm0_loc },
    { pvdm_pid,      sm1pvdm1_loc },
    { pvdm_pid,      sm1pvdm2_loc },
    { dc_pid,        sm0wic0dc_loc },
    { dc_pid,        sm1wic0dc_loc },
    { dc_pid,        sm0wic1dc_loc },
    { dc_pid,        sm1wic1dc_loc },
    { dc_pid,        sm0dc_loc },
    { dc_pid,        sm1dc_loc },
};


/* 
 * Sub Menu used for Motherboard tests.
 */
submenu_xtable_t mb_tests_submenu_table[] = {

	{"CPU core test",
     (PFT)cpu_core_test,   FALSE,          MF_3,
     (type_t(*)())0, 0,         (PFT)cpu_core_test,        TRUE},

    {"Main memory test with cache on and ECC checking",
     (PFT)linux_memory_tester_with_ecc_check,   FALSE,          MF_3,
     (type_t(*)())0, 0,         (PFT)linux_memory_tester_with_ecc_check,        TRUE},

    {"Boot flash test",
     (PFT) build_boot_flash_menu,       FALSE,          MF_3,
     (type_t(*)())0, 0,         (PFT) build_boot_flash_menu,    TRUE},

    {"USB test (need USB3.0 drive)",
        (PFT)usb_tests, 1,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0, 0},

    {"eMMC 0 test",
        (PFT)emmc_tests,  0,                      MF_3,
                (type_t(*)())0,         0,
        (PFT)0,  0},

    {"PCIe register test",
        (PFT) pcie_rd_reg,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"PCIe lane scan test",
        (PFT)katar_pcie_lane_scan_test,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"I2C scan test",
        (PFT)katar_x86_i2c_scan_test,   0,       MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},
    
    {"Platform logic FPGA register test",
        (PFT)katar_dash_rd_wr_test,      0,           MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"Interrupt auto test",
        (PFT)katar_mb_force_intr_test,      0,           MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"Aikido FPGA register test",
        (PFT)aikido_reg_test,            0,           MF_4,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"GE phy cross-port tests",
       (PFT)katar_mb_ge_phy_cross_port_test,   0,      MF_3,
       (type_t(*)())0, 0,
       (type_t(*)())0,   0},

    {"10G phy (RJ45/SFP) cross-port tests",
       (PFT)katar_mb_ge10_phy_cross_port_test,   0,      MF_3,
       (type_t(*)())0, 0,
       (type_t(*)())0,   0},

    {"2.5G phy cross-port tests",
       (PFT)katar_mb_ge2p5_phy_cross_port_test,   0,      MF_3,
       (type_t(*)())0, 0,
       (type_t(*)())0,   0},

    {"2.5G phy register tests",
       (PFT)aqr_register_read_write_util,   0,      MF_3,
       (type_t(*)())0, 0,
       (type_t(*)())0,   0},

	{"Fan force high speed test",
        (PFT)katar_mb_fan_high_test,  0,                      MF_3,
            (type_t(*)())0,     0,
        (PFT)0,  0},

	{"RTC test",
        (PFT)rtc_tests,  0,                      MF_3,
            (type_t(*)())0,     0,
        (PFT)0,  0},

	{"Fan low speed test",
        (PFT)katar_mb_fan_low_test,  0,                      MF_3,
        (type_t(*)())0,     0,
        (PFT)0,  0},


    {"2.5G phy cross-port tests for RDT",
       (PFT)katar_2p5_phy_cross_test_rdt,   0,      MF_3,
       (type_t(*)())0, 0,
       (type_t(*)())0,   0},

};

#define MB_TESTS_SUBMENU_TABLE_SIZE (sizeof(mb_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mb_tests_primary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];
static mitem_t mb_tests_secondary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];

menuinfo_t mb_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mb_tests_primary_items,
};
menuinfo_t *mb_submenup = &mb_subtest_menu;

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
int 
mb_tests (boolean mb_test_items_executed)
{
    build_primary_submenu(mb_tests_submenu_table, MB_TESTS_SUBMENU_TABLE_SIZE,
                            "Motherboard", &mb_submenup);
    build_secondary_submenu(mb_tests_submenu_table,
                            MB_TESTS_SUBMENU_TABLE_SIZE,
                            mb_tests_secondary_items);

    if (mb_test_items_executed) {
        do_all_menu_items(&mb_subtest_menu);
    } else {
        menu(&mb_subtest_menu, mb_tests_secondary_items, '\0');
    }
    return PASSED;
}

// End of extended error reporting functions

/*
 * Function: usb_exist
 *
 * Description : Check USB device is available. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
int usb_exist (int slot)
{
    char check_usbdrv[50];
    int devfd, ix;
    sprintf(check_usbdrv, "/dev/usbdrv%d", slot);

    for (ix = 0; ix < 10; ix++) {
        devfd = open(check_usbdrv, O_RDWR);
        if(devfd < 0) {
            sleep(1);
            close(devfd);
            continue;
        } else {
            break;
        }
    }
    if (devfd < 0) {
        close(devfd);
        perror("there is no device file descriptor available. ");
        printf("Can not access device at USB slot%d. is slot vacant?", slot);
        return (FAILED);
    } else {
        close(devfd);
        return (PASSED);
    }
}

/*
 * Function: usb_tests
 *
 * Description : usb r/w tests. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
static int usb_tests (int slot) 
{
    int retval = PASSED;
    char *tname = "USB slot";
    char buf[128] = "NULL";
    FILE *fp;
    char *check_usb3_file = "/nep-diag/usb_speed.txt";
    char check_usb2_spd[] = "480";
    int ix, is_usb2_stick;

    testname("%s%d access", tname, slot);
    /* 
     * D_EXT_LOOPBACK = 0, enable ext. loopback 
     * D_EXT_LOOPBACK = 1, disable ext. loopback 
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return(PASSED);
    }

    /* Temporary disable showing kernel messages because unbind and bind XHCI controller to XHCI driver */
    system(SUPPRESS_MESG);

    /* Check USB stick is 2.0 or 3.0, return fail if is USB 2.0 stick */
    for (ix = 0; ix < 5; ix++) {
        is_usb2_stick = 0;
        if (slot) {
            if (usb_exist(slot) != PASSED) {
                cterr('f',0, "Can't find USB slot-%d device node.", slot);
                return (FAILED);
            }
            system(GET_USB1_SPEED);
        } else {
            if (usb_exist(slot) != PASSED) {
                cterr('f',0, "Can't find USB slot-%d device node.", slot);
                return (FAILED);
            }
            system(GET_USB0_SPEED);
        }

        fp = fopen(check_usb3_file, "r");
        if (fp == NULL) {
            return (FALSE);
        }

        while (!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, check_usb2_spd) != NULL) {
                if (ix == 4) {
                    fclose(fp);
                    system("lsusb -tv");
                    cterr('f',0, "Please plug USB3 stick into USB slot-%d.", slot);
                    system(REMOVE_USBSPD_FILE);
                    /* Enable kernel message */
                    system(OPEN_MESG);
                    return (FAILED);
                } else {
                    is_usb2_stick = 1;
                    break;
                }
            } 
        }

        if (is_usb2_stick == 1) {
            printf("Detect USB2 stick at slot-%d\n", slot);
            system(UNBIND_XHCI_CONTROLLER);
            msleep(100);
            system(BIND_XHCI_CONTROLLER);
            msleep(500);
            system(UDEVTRIGGER);
            msleep(100);
            fclose(fp);
            system(REMOVE_USBSPD_FILE);
        } else {
            fclose(fp);
            goto out;
        }
    }

out:
    /* First time USB test */
    prpass(testpass, "USB slot%d host XHCI controller default run\n", slot);
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller default run failed.", slot);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d host XHCI controller disable super speed run\n", slot);

    /* Disable USB 3.0 super speed */
    system(DISABLE_USB3_SS);

    for (ix = 0; ix < 5; ix++) {
        /* Unbind and bind XHCI controller to XHCI driver to make disable super speed setting active */
        system(UNBIND_XHCI_CONTROLLER);
        msleep(100);
        system(BIND_XHCI_CONTROLLER);
        msleep(500);
        system(UDEVTRIGGER);
        msleep(100);

        /* Check USB device available after disable USB3 super speed */
        if (usb_exist(slot) != PASSED) {
            if (ix == 4) {
                cterr('f',0, "USB slot%d not available after disable USB3 super speed.", slot);
                return (FAILED);
            } 
        } else {
            goto done;
        }
    }
done:

    /* Second time USB test - disable super speed run */
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller disable super speed run failed.", slot);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d switch USB3 from XHCI to EHCI controller run\n", slot);
    /* Route USB ports from XHCI controller to EHCI controller */
    system(ROUTE_USB2_TO_EHCI);
    msleep(500);
    system(UDEVTRIGGER);
    msleep(100);

    /* Check USB device available after route USB from XHCI to EHCI controller */
    if (usb_exist(slot) != PASSED) {
        cterr('f',0, "USB slot%d not available after route USB from XHCI to EHCI controller.", slot);
        return (FAILED);
    }

    /* Final USB test  */
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d switch USB3 from XHCI to EHCI controller run failed.", slot);
    }

    /* Enable super speed and route USB port from EHCI controller to XHCI controller */
    system(ENABLE_USB3_SS);
    msleep(100);
    system(ROUTE_USB2_TO_XHCI);
    msleep(1000);
    system(UDEVTRIGGER);
    msleep(100);

    for (ix = 0; ix < 5; ix++) {
        /* Check USB device available after route USB port from EHCI to XHCI controller */
        if (usb_exist(slot) != PASSED) {
            if (ix == 4) {
                cterr('f',0, "USB slot%d not available after route USB port from EHCI to XHCI controller.", slot);
                return (FAILED);
            } else {
                /* Unbind and bind XHCI controller to XHCI driver */
                system(UNBIND_XHCI_CONTROLLER);
                msleep(100);
                system(BIND_XHCI_CONTROLLER);
                msleep(500);
                system(UDEVTRIGGER);
                msleep(100);
            }
        } else {
            goto exit;
        }
    }
exit:
    system(REMOVE_USBSPD_FILE);
    /* Enable kernel message */
    system(OPEN_MESG);
    return (retval);
}

/*
 * Function: display_env
 *
 * Description: Display the Environment information
 *
 * Inputs: None
 *
 * Output: None
 *
 */
void
display_env(void)
{
	return;
}

/*
 * Function: emmc_tests
 *
 * Description : emmc r/w tests.
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int emmc_tests (int dummy) 
{
    int rc = FAILED;
    char *tname = "emmc0";
    
    /* we only have one emmc */
    testname("%s access", tname);

    /* testname is printed on usb_slot_tests */
    rc = emmc_slot_tests(dummy);
    if (rc == FAILED) {
        cterr('f',0,"emmc0 test failed.");
    }else
		prpass(testpass, NULL);

    return(rc);
}

/*******************************************************************************
 * Function: cpu_core_test
 *
 * Description : Test for stress of cpu core & memory.
 * Default command: "stress -c 400 -m 2 -t 1".
 *
 * Inputs: test option - auto or user assign
 *
 * Output: PASSED/FAILED
 *******************************************************************************
 */
#define CPU_STRESS_LOG   "/tmp/cpu_core_log"
#define CPU_STRESS_RLT   "/tmp/cpu_core_rlt"
int cpu_core_test (int do_more_test)
{
    char cmd[100], line[100], *ptr;
    unsigned int cpu_count = 16, mem_count = 2, sec = 5;
    int rc = FAILED;
    char *tname = "CPU core";
    FILE *fp;

    memset(line, 0x0, sizeof(line));

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    prpass(testpass, "%s stress, ", tname);

    if (do_more_test) {
        /* Spawn N workers spining on sqrt()*/
        cpu_count =
            getdec_answer("\nEnter CPU-bound processes:", 400, 1, 400);
        /* Spawn N workers spinning on malloc()/free() */
        mem_count =
            getdec_answer("Enter memory allocator process:", 2, 1, 6);
        sec = getdec_answer("Enter test time (seconds):", 1, 1, 60);
    }
    prpass(testpass, "Starting stress test, ");
    prpass(testpass, "Stress testing (times: %d, process: %d, duration: %d sec.), ",
           cpu_count, mem_count, sec);
    sprintf(cmd, "stress -c %d -m %d -t %d > %s 2>&1",
            cpu_count, mem_count, sec, CPU_STRESS_LOG);
    system(cmd);

    sprintf(cmd, "cat %s | grep run > %s", CPU_STRESS_LOG, CPU_STRESS_RLT);
    system(cmd);

    fp = fopen(CPU_STRESS_RLT, "r");
    if (fp != NULL) {
        fgets(line, sizeof(line), fp);
        if (do_more_test) {
            printf("\n%s \n", line);
        }
        if (line[0] == '\0') {
            printf("Line[0] == 0\n");
            rc = FAILED;
        }
        ptr = strstr(line, "successful");
        if (ptr != NULL) {
            rc = PASSED;
        }
        fclose(fp);
    } else {
        cterr('f', 0, "%s test failed.");
        rc = FAILED;
    }

    /* delete the log file */
    unlink(CPU_STRESS_LOG);
    unlink(CPU_STRESS_RLT);

    if ((rc == PASSED)) {
        prpass(testpass, "%s test passed, ", tname);
    }

    return (rc);
}


uint32_t diag_pci_get_device_bus(ushort vendor, ushort device, uint32_t *bus, uint32_t device_num)
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    uint32_t found_dev = 0; 

    pacc = pci_alloc(); /* Get the pci_access structure */
    pci_init(pacc);     /* Initialize the PCI library */
    pci_scan_bus(pacc); /* We want to get the list of devices */
    for (dev=pacc->devices; dev; dev=dev->next) /* Iterate over all devices */
    {
        /* Fill in header info we need */
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS); 

        /* Read config register directly */
        pci_read_byte(dev, PCI_INTERRUPT_PIN);

        if ((dev->vendor_id == vendor) && (dev->device_id == device)) {
            if(found_dev < device_num){
                                if(bus != NULL)
                                        bus[found_dev] = dev->bus;
                found_dev++;
            }
                        if(found_dev == device_num)
                        {
                                pci_cleanup(pacc);
                                return found_dev;
                        }
        }
    }
    pci_cleanup(pacc);   /* Close everything */
        return found_dev;
}

int katar_get_pcie_cap_struct_ptr (uint32_t bus, uint16_t dev, int fn, uint reg)
{
    uint32_t val;
    uint32_t offset = reg;
    int get_pci_cap_cnt;
    int max_cnt = 15;

    val = pci_config_read(bus, dev, fn, offset);
    val = val & PCI_EXP_CAP_ID_MASK;
    if (val == PCI_EXP_CAP_ID) {
        return offset;
    } else {
        offset = val;
        val = pci_config_read(bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        if (val == PCI_EXP_CAP_ID) {
            return offset;
        }
    }

    /*
     * PCI cap pointer should be found within few steps, to avoid incorrect
     * bus number to casue infinite loop, set max loop number is 15
     */
    for (get_pci_cap_cnt = 1; get_pci_cap_cnt <= max_cnt; get_pci_cap_cnt++) {
        offset += PCI_EXP_CAP_NEXTPTR_OFFSET;
        val = pci_config_read(bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        offset = val;
        val = pci_config_read(bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        if (val == PCI_EXP_CAP_ID) {
            break;
        }

        if (get_pci_cap_cnt == max_cnt) {
            return (FAILED);
        }
    }

    return offset;
}

/*
 * Function: pcie_lane_scan_test
 *
 * Description : PCI interface scan/check
 *
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int katar_pcie_lane_scan_test (void)
{
#define PCIE_SCAN_NUM   2

    int ix,iy;
    char rj45_dev_name[PCIE_SCAN_NUM][20] = {"AQC107","I211"};
    uint32_t rj45_dev_vid[PCIE_SCAN_NUM] = {0x1d6a,0x8086};
    uint32_t rj45_dev_did[PCIE_SCAN_NUM] = {0x07b1,0x1539};
        char sfp_dev_name[PCIE_SCAN_NUM][20] = {"AQC100","I211"};
        uint32_t sfp_dev_vid[PCIE_SCAN_NUM] = {0x1d6a,0x8086};
    uint32_t sfp_dev_did[PCIE_SCAN_NUM] = {0x0001,0x1539};
        uint32_t sfp1_dev_did[PCIE_SCAN_NUM] = {0xd100,0x1539};
    char (*dev_name)[20];
    uint32_t *dev_vid;
    uint32_t *dev_did;
        uint32_t bus[2];
    uint32_t id_val,offset_val,cap_val,sta_val;
        uint32_t cap_s, cap_w, sta_s, sta_w;
    uint32_t vend_id;
    uint32_t dev_id;
    char *tname = "PCI lane scan";

    testname("%s", tname);
    switch(katar_get_plat_sku())
    {
        case KATAR_RJ45_SKU:
                        dev_name = rj45_dev_name;
                        dev_vid = rj45_dev_vid;
                        dev_did = rj45_dev_did;
            break;
        case KATAR_SFP_SKU:
            dev_name = sfp_dev_name;
            dev_vid = sfp_dev_vid;
            dev_did = sfp_dev_did;
                        break;
        case KATAR_SFP1_SKU:
            dev_name = sfp_dev_name;
            dev_vid = sfp_dev_vid;
            dev_did = sfp1_dev_did;
            break;
        case UNKNOWN_SKU:
            cterr('f', 0, "Failed to get board SKU");
            return (FAILED);
            break;
    }
        for (iy = 0 ; iy < PCIE_SCAN_NUM ; iy++) {
                if(diag_pci_get_device_bus(dev_vid[iy],dev_did[iy],bus,2)!=2)
                {
                        cterr('f', 0, "Get VID/DID bus error");
            return (FAILED);
                }
                for (ix = 0 ; ix < 2 ; ix++) {
                        prpass(testpass, "%s-%d ", dev_name[iy],ix);
                        id_val = pci_config_read(bus[ix], PCI_DEV_0, PCI_FUN_0, 0x00);
                dev_id = (id_val & 0xFFFF0000) >> 16;
                vend_id = (id_val & 0x0000FFFF);

                if((dev_id!= dev_did[iy])||(vend_id!=dev_vid[iy]))
                {
                    cterr('f',0, "DID/VID is not correct");
                                return (FAILED);
                }
                        offset_val = katar_get_pcie_cap_struct_ptr(bus[ix], PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
                    if (offset_val == FAILED) {
                    cterr('f',0, "Can't get PCI cap pointer");
                    return (FAILED);
                }
                        cap_val = get_pcie_link_cap(bus[ix], PCI_DEV_0, PCI_FUN_0, offset_val);
                        sta_val = get_pcie_link_status(bus[ix], PCI_DEV_0, PCI_FUN_0, offset_val);
                /* Speed - bit 0~3 */
                cap_s = cap_val & PCI_EXP_LINK_STA_SPD_MASK;
                sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
                /* Width - bit 4~9 */
                cap_w = (cap_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;
                sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;
                        if ((cap_s == PCI_EXP_LINK_STA_SPD_2DOT5) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
                        prpass(testpass, "Link speed is 2.5G ");
                } else if ((cap_s == PCI_EXP_LINK_STA_SPD_5GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
                    prpass(testpass, "Link speed is 5G ");
                } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_8GT)) {
                    prpass(testpass, "Link speed is 8G ");
                } else {
                    cterr('f',0, "Link speed is not correct");
                    return (FAILED);
                }
                if ((cap_w == PCI_EXP_LINK_STA_WID_1) && (sta_w == PCI_EXP_LINK_STA_WID_1)) {
                    prpass(testpass, "Link width is x1 ");
                } else if ((cap_w == PCI_EXP_LINK_STA_WID_2) &&(sta_w == PCI_EXP_LINK_STA_WID_2)) {
                    prpass(testpass, "Link width is x2 ");
                } else if ((cap_w == PCI_EXP_LINK_STA_WID_4) && (sta_w == PCI_EXP_LINK_STA_WID_4)) {
                    prpass(testpass, "Link width is x4 ");
                } else if ((cap_w == PCI_EXP_LINK_STA_WID_8) && (sta_w == PCI_EXP_LINK_STA_WID_8)) {
                    prpass(testpass, "Link width is x8 ");
                } else {
                    cterr('f',0, "Link width is not correct");
                    return (FAILED);
                }
                }
        }
	prpass(testpass, NULL);
    return (PASSED);
}

//AQC107
#define RJ45_TEST_PCIE_SW_VID   0x1d6a
#define RJ45_TEST_PCIE_SW_DID   0x07b1
//AQC100
#define SFP_TEST_PCIE_SW_VID   0x1d6a
#define SFP_TEST_PCIE_SW_DID   0x0001
#define SFP1_TEST_PCIE_SW_DID  0xD100
int katar_get_plat_sku(void)
{
        if(diag_pci_get_device_bus(RJ45_TEST_PCIE_SW_VID,RJ45_TEST_PCIE_SW_DID,NULL,1))
                return KATAR_RJ45_SKU;
        else if(diag_pci_get_device_bus(SFP_TEST_PCIE_SW_VID,SFP_TEST_PCIE_SW_DID,NULL,1))
                return KATAR_SFP_SKU;
        else if(diag_pci_get_device_bus(SFP_TEST_PCIE_SW_VID,SFP1_TEST_PCIE_SW_DID,NULL,1))
        return KATAR_SFP1_SKU;
        else
                return UNKNOWN_SKU;
}

int katar_is_sfp_sku(void)
{
        if(katar_get_plat_sku() != KATAR_RJ45_SKU)
                return TRUE;
        else
                return FALSE;
}

int pcie_rd_reg (uint32_t reg_addr)
{
        uint32_t reg_val;
        uint32_t vend_id;
        uint32_t dev_id;
        uint32_t port0_bus_num;
        uint32_t test_did;
        uint32_t test_vid;

        char *tname = "PCI read register";
        testname("%s", tname);

        switch(katar_get_plat_sku())
        {
                case KATAR_RJ45_SKU:
                        test_did = RJ45_TEST_PCIE_SW_DID;
                        test_vid = RJ45_TEST_PCIE_SW_VID;
                        break;
                case KATAR_SFP_SKU:
            test_did = SFP_TEST_PCIE_SW_DID;
            test_vid = SFP_TEST_PCIE_SW_VID;
            break;
        case KATAR_SFP1_SKU:
            test_did = SFP1_TEST_PCIE_SW_DID;
            test_vid = SFP_TEST_PCIE_SW_VID;
            break;
                case UNKNOWN_SKU:
                        cterr('f', 0, "Failed to get board SKU");
                        return (FAILED);
                        break;
        }

        port0_bus_num = get_pcie_bus_num (test_vid, test_did);
        /* read config register 00h */
        reg_val = pcie_config_read(0, port0_bus_num, 0, 0, 0x00);
        dev_id = reg_val;
        dev_id = (dev_id & 0xFFFF0000) >> 16;
        vend_id = reg_val;
        vend_id = (vend_id & 0x0000FFFF);

        if ((dev_id == test_did) && (vend_id == test_vid)) {
                prpass(testpass, "PCIe device id %x, vendor id %x. ", dev_id, vend_id);
                return (PASSED);
        } else {
                cterr('f', 0, "Failed to get PCIe device id %x vendor id %x", dev_id, vend_id);
                return (FAILED);
        }
}

/*
 *------------------------------------------------------------------
 * $Log: mb_tests.c,v $
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.19  2019/06/10 03:47:18  mikech2
 * Remove platform_fru.h base on PRRQ#4685780 Comment#6
 *
 * Revision 1.1.2.18  2019/06/10 02:26:44  mikech2
 * Remove skip test function base on PRRQ#4685780 Comment#4
 *
 * Revision 1.1.2.17  2019/05/29 05:59:17  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.16  2019/05/24 13:45:52  benlu
 * Add 2.5G cross-port test for RDT in MB diag
 *
 * Revision 1.1.2.15  2019/04/30 06:06:59  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.14  2019/04/23 07:10:20  peteteng
 * Add bootflash switch test
 *
 * Revision 1.1.2.13  2019/03/07 02:51:08  mikech2
 * Move reset button/SFP present test to utility
 *
 * Revision 1.1.2.12  2019/03/04 00:45:14  mikech2
 * Clean up codes and remove unnecessary files
 *
 * Revision 1.1.2.11  2019/02/26 03:51:38  mikech2
 * Add internal loopback support for mb test
 *
 * Revision 1.1.2.10  2019/01/31 03:53:52  mikech2
 * Add cpu core test and change mb test order
 *
 * Revision 1.1.2.9  2018/12/27 03:49:00  mikech2
 * Modify prpass usage
 *
 * Revision 1.1.2.8  2018/11/14 08:59:49  benlu
 * Move 2.5G PHY register test from util to mbtest
 *
 * Revision 1.1.2.7  2018/11/14 08:14:58  peteteng
 * Add Aikido FPGA register test
 *
 * Revision 1.1.2.6  2018/11/13 07:50:12  mikech2
 * Fix pcie bus scan issue
 *
 * Revision 1.1.2.5  2018/11/08 06:00:15  mikech2
 * Add fan low and interrupt test in mb test and remove intr utility
 *
 * Revision 1.1.2.4  2018/11/01 07:25:16  mikech2
 * Add fan speed test in mb test
 *
 * Revision 1.1.2.3  2018/10/26 03:11:16  mikech2
 * update comment & rollback mb_tests.c
 *
 * Revision 1.1.2.1  2018/10/22 08:02:33  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.14  2018/10/22 03:04:33  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.13  2018/10/18 02:32:25  mikech2
 * Add RTC test in mb_tests.c
 *
 * Revision 1.1.2.12  2018/10/16 08:51:08  benlu
 * Add AQR412c cross-port test
 *
 * Revision 1.1.2.11  2018/10/08 03:36:17  mikech2
 * Modify pcie scan for different AQC100 FW
 *
 * Revision 1.1.2.10  2018/09/21 08:52:12  mikech2
 * Add cross-port & internal lpbk test util
 *
 * Revision 1.1.2.9  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.8  2018/08/27 08:28:47  mikech2
 * Fix I2C & pcie scan test
 *
 * Revision 1.1.2.7  2018/07/10 09:58:03  benlu
 * add interanl/external loopback in menu
 *
 * Revision 1.1.2.6  2018/07/04 03:28:47  mikech2
 * Add GE phy mb test function
 *
 * Revision 1.1.2.5  2018/06/29 07:17:31  mikech2
 * Remove compile warning and unused files
 *
 * Revision 1.1.2.4  2018/06/27 07:45:16  mikech2
 * remove unused test in mb_test
 *
 * Revision 1.1.2.3  2018/06/21 08:24:09  mikech2
 * remove unused menu, add scratchpad reg test
 *
 * Revision 1.1.2.2  2018/06/11 07:05:53  peteteng
 * add bootflash test from viper
 *
 * Revision 1.1.2.1  2018/06/07 01:19:22  peteteng
 * add project katar - based on neptune
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

