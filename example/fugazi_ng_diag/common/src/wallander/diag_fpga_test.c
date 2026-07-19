/* $Id: diag_fpga_test.c,v 1.2 2015/02/26 08:15:43 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_fpga_test.c,v $ 
 *-----------------------------------------------------------------------------
 * diag_fpga_test.c - Wallander FPGA Test Menu
 *
 * Apr 2014, Xiaoying Zhang
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "diag_fpga_lib.h"
#include "diag_fpga_fw.h"
#include "diag_fpga_test.h"
#include "diag_common_drv.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "platform_sfp_cookie.h"

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

// int fpga_test(int);
// int fpga_do_all_wrapper(void);

extern uint32_t get_gpio_rx_dat_bits(uint32_t);
extern void msleep(unsigned long);

extern int get_num_ports(void);
extern int check_offset (ushort, reg_info_t*);
extern ulong get_reg_size (ushort, reg_info_t*);
extern void reg_dump(ulong, reg_info_t*);


static int fpga_register_test(void);
// static int fpga_force_interrupt_test(void);
// static int program_fpga(void);
/*static*/ int display_fpga_version(void);
// static int set_fpga_boot_flag(void);
static int diag_fpga_read32_fn(ulong, int, ulong *, void *);
static int diag_fpga_write32_fn(ulong, int, ulong, void *);
// static int display_fpga_flash_content(void);
// static int erase_fpga_flash_sector(void);

int fpga_burst_mode_register_test(void);
int fpga_local_bus_read(void);
int fpga_local_bus_write(void);

/* Sub Menu used for FPGA tests.*/
static submenu_xtable_t fpga_tests_submenu_table[] = {
    {"FPGA Local Bus Register Test", (type_t(*)()) fpga_register_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define FPGA_TESTS_SUBMENU_TABLE_SIZE (sizeof(fpga_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_tests_primary_items[FPGA_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t fpga_tests_secondary_items[FPGA_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

menuinfo_t fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fpga_tests_primary_items,
};
menuinfo_t *fpga_submenup = &fpga_subtest_menu;


static reg_info_t_ext fpga_reg32_ext = {{4}, diag_fpga_read32_fn,
                                      diag_fpga_write32_fn, 0};

static reg_info_t fpga_regs[] = {
    /* NAME                     OFFSET TYPE          SIZE  MASK  RESET VAL*/
    /* Gerneral Purpose Registers */
    {"FPGA Create Hour",                    0x0000, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"FPGA Create Day ",                    0x0001, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"FPGA Create Month",                   0x0002, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"FPGA Create Year",                    0x0003, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Board ID",                            0x0004, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"CPU Attached Individual Device Rest", 0x0005, READ_ONLY | SAVE_RESTORE,
        {1},   0x07, 0x0},
    {"CPU Reset Ctrl",                      0x0006, READ_ONLY | SAVE_RESTORE,
        {1},   0x01, 0x0},
    {"SFP LED Ctrl",                        0x0007, READ_WRITE | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"PHY LED Ctrl",                        0x0008, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"GPIO Expander Low",                   0x0009, READ_WRITE | SAVE_RESTORE,
        {1},   0x08, 0x23},
    {"GPIO Expander High",                  0x000A, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Sync In Clock MUX Ctrl",              0x000D, READ_WRITE | SAVE_RESTORE,
        {1},   0x01, 0x2},
    {"Sync Trigger In Clock MUX Ctrl",      0x000E, READ_WRITE | SAVE_RESTORE,
        {1},   0x01, 0x2},
    {"Sync Out Clock Selection Ctrl",       0x000F, READ_ONLY | SAVE_RESTORE,
        {1},   0x07, 0x0},
    {"Sync Out Clock Ctrl Intr",            0x0010, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Sync Out Clock Ctrl Intr Enable",     0x0011, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Boot Flash Write Protect Ctrl",       0x0012, READ_WRITE | SAVE_RESTORE,
        {1},   0x01, 0x06},
    {"SFP Port0 Intr Enable and Ctrl",      0x0013, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"SFP Port0 Intr Status",               0x0014, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"SFP Port2 Intr Enable and Ctrl",      0x0015, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"SFP Port2 Intr Status",               0x0016, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Device Status",                       0x0017, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Device Status Intr",                  0x0018, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Device Status Intr Enable",           0x0019, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"FPGA Intr Source Ctrl",               0x001A, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"PHY and ZL30254 Status Ctrl",         0x001B, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},

    /* 1588 Timestamp Control Registers */
    {"Timestamp Ctrl",                      0x1000, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Timestamp Interrupt Ctrl",            0x1001, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Timestamp Data Counter",              0x1002, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"Timestamp Status",                    0x1003, READ_ONLY | SAVE_RESTORE,
        {1},   0xFF, 0x0},
    {"END",              0x00,  0,          {0},   0x0,  0x0},
};

static reg_info_t fpga_regs32[] = {
    /* FPGA Multi-boot Registers */
    {"Multiboot Ctrl",                      0x8100, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"Multiboot Status",                    0x8104, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"Multiboot Header ID",                 0x8108, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"Multiboot Header Data",               0x810C, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"Multiboot Header Flag",               0x8110, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"Multiboot Header Magic Number",       0x8114, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"Multiboot State History",             0x8118, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},

    /* FPGA SPI Controller Registers */
    {"SPI Ctrl",                            0x9040, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0x00000076, 0x0},
    {"SPI Status",                          0x9044, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"SPI Read Size",                       0x9048, READ_WRITE | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0x000000FF, 0x0},
    {"SPI Data",                            0x904C, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0x000000FF, 0x0},
    {"SPI Address",                         0x9050, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0x00FFFFFF, 0x0},

    /* Secure Boot Registers */
    {"S-Boot Core Status Golden",           0x9060, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Check Status Golden",          0x9064, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Core Signature Addr Golden",   0x9068, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Core Signature Size Golden",   0x906C, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Core Status Upgrade",          0x9070, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Check Status Upgrade",         0x9074, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Signature Addr Upgrade",       0x9078, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"S-Boot Core Signature Size Upgrade",  0x907C, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},

    /* GPIO Registers */
    {"GPIO_O",                              0x9080, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"GPIO_OE",                             0x9084, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"GPIO_I",                              0x9088, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"GPIO_INHB_RST",                       0x908c, READ_ONLY | REG_ACCESS,
        {(unsigned long)&fpga_reg32_ext},     0xFFFFFFFF, 0x0},
    {"END",              0x00,  0,          {0},   0x0,  0x0},
};


/*
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 */

int fpga_test (int show_menu)
{
    build_primary_submenu(fpga_tests_submenu_table,
                          FPGA_TESTS_SUBMENU_TABLE_SIZE,
                          "FPGA", &fpga_submenup);
    build_secondary_submenu(fpga_tests_submenu_table,
                            FPGA_TESTS_SUBMENU_TABLE_SIZE,
                            fpga_tests_secondary_items);

    if (show_menu) {
        menu(fpga_submenup, fpga_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(fpga_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_do_all_wrapper
 * Description : Wrapper for FPGA do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int fpga_do_all_wrapper (void)
{
    int rc = PASSED;

    if (fpga_register_test() == FAILED) {
        rc = FAILED;
    }

    return (rc);
}


/******************************************************************************
 *
 * Function: fpga_sfp_led_test
 *
 * Description: This function performs the FPGA SFP LED test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_sfp_led_test (void)
{
    int max_port = get_num_ports() / 2;
    int port;
    int led_on = 1;
    int led_off = 0;
    int mode = 0;

    testname("FPGA SFP LED");

    /* turn all leds off */
    printf("Turn all leds off\n");
    for (port = 0; port < max_port; port++) {
        if (fpga_ctrl_sfp_led(port, FPGA_SFP_LED_Y, led_off) ||
            fpga_ctrl_sfp_led(port, FPGA_SFP_LED_G, led_off) ||
            fpga_ctrl_sfp_led(port, FPGA_SFP_LED_SPD, led_off)) {
                return (FAILED);
            }
    }
    sleep(1);

    /* turn yellow leds on */
    printf("Turn leds yellow\n");
    mode = 1;
    for (port = 0; port < max_port; port++) {
        if(fpga_ctrl_sfp_led(port, FPGA_SFP_LED_Y, led_on)) {
            return (FAILED);
        }
    }
    sleep(1);
    for (port = 0; port < max_port; port++) {
        if (fpga_ctrl_sfp_led(port, FPGA_SFP_LED_Y, led_off)) {
                return (FAILED);
            }
    }
    sleep(1);

    /* turn green leds on then off */
    printf("Turn leds green\n");
    for (port = 0; port < max_port; port++) {
        if(fpga_ctrl_sfp_led(port, FPGA_SFP_LED_G, led_on)) {
            return (FAILED);
        }
    }
    sleep(1);
    for (port = 0; port < max_port; port++) {
        if (fpga_ctrl_sfp_led(port, FPGA_SFP_LED_G, led_off)) {
                return (FAILED);
            }
    }
    sleep(1);

    /* set speed mode */
    printf("Turn on speed leds\n");
    for (mode = 1; mode < 4; mode++) {
        for (port = 0; port < max_port; port++) {
            if(fpga_ctrl_sfp_led(port, FPGA_SFP_LED_SPD, mode)) {
                return (FAILED);
            }
        }
        msleep(2500);
    }

    /* turn all leds off */
    printf("Turn leds off\n");
    mode = 0;
    for (port = 0; port < max_port; port++) {
        if (fpga_ctrl_sfp_led(port, FPGA_SFP_LED_Y, mode) ||
            fpga_ctrl_sfp_led(port, FPGA_SFP_LED_G, mode) ||
            fpga_ctrl_sfp_led(port, FPGA_SFP_LED_SPD, mode)) {
                return (FAILED);
            }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_register_test
 *
 * Description: This function performs the FPGA register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int fpga_register_test (void)
{
    ulong fpga_offset;

    testname("FPGA Register");

    fpga_offset = (ulong)fpga_get_local_bus_addr();
    if (register_tests(fpga_offset, fpga_regs) == FAILED) {
        cterr('f', 0, "FPGA Register Test Failed");
        return (FAILED);
    }
    if (register_tests(0, fpga_regs32) == FAILED) {
        cterr('f', 0, "FPGA Register Test Failed");
        return (FAILED);
    }

    printf("FPGA Register Test Passed\n");
    return (PASSED);
}


/******************************************************************************
 *
 * Function: show_fpga_version
 *
 * Description: This function supports to display the FPGA version.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int show_fpga_version()
{
    ushort version_offset = 4;
    char reg_data;
    ushort i;

    printf("The FPGA version is ");
    for (i = 0; i < version_offset; i++) {
        if (fpga_reg_read(i, &reg_data)) {
            printf("Failed to read FPGA Reg Offset %d.", i);
            return (FAILED);
        } else {
            printf("%x", reg_data);
            if (i < version_offset - 1) {
                printf("/");
            }
        }
    }
    printf("\n");

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_reg_rd_util
 *
 * Description: Utility for users to read FPGA registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int fpga_reg_rd_util()
{
    int offset = 0;
    char reg_data = 0;
    ulong reg_data32 = 0;

    offset = gethex_answer("\nEnter register offset[0 to 0x908c]:",
               0, 0, 0x908c);

    if (check_offset(offset, fpga_regs) && check_offset(offset, fpga_regs32)) {
        perror("\n Offset is invalid.\n ");
        return (FAILED);
    } else if (offset < FPGA_MB_REG_BASE) {
        if (fpga_reg_read(offset, &reg_data)) {
            printf("Failed to read FPGA Reg Offset %d.", offset);
            return (FAILED);
        } else {
            printf("\n Register value %#x ", (uchar)reg_data);
            return (PASSED);
        }
    } else {
        if (fpga_reg_read32(offset, &reg_data32)) {
            printf("Failed to read FPGA Reg Offset %d.", offset);
            return (FAILED);
        } else {
            printf("\n Register value %#x ", (unsigned int)reg_data32);
            return (PASSED);
        }
    }
}

/******************************************************************************
 *
 * Function: fpga_reg_wr_util
 *
 * Description: Utility for users to write FPGA registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int fpga_reg_wr_util()
{
    int offset;
    char reg_data = 0;
    ulong reg_data32 = 0;

    offset = gethex_answer("\nEnter register offset[0 to 0x908c]:",
               0, 0, 0x908c);

    if (check_offset(offset, fpga_regs) && check_offset(offset, fpga_regs32)) {
        perror("\n Offset is invalid.\n ");
        return (FAILED);
    } else if (offset < FPGA_MB_REG_BASE) {
        /* Display original value */
        if (fpga_reg_read((int)offset, &reg_data) == FAILED) {
            printf("Read FPGA register %#.4x failed\n", offset);
            return (FAILED);
        } else {
            printf("Original data of register %#.4x = %#.2x\n", offset, reg_data);
        }

        /* Alter register with new value */
        reg_data = gethex_answer("Enter the new data (hex): ", reg_data, 0,
                                0xFF);

        if (fpga_reg_write((int)offset, reg_data) == FAILED) {
            printf("Write data %#.2x to register %#.4x failed\n", reg_data, offset);
            return (FAILED);
        }

        /* Display the value again */
        if (fpga_reg_read((int)offset, &reg_data)) {
            printf("Read FPGA register %#.4x failed\n", offset);
            return (FAILED);
        } else {
            printf("New data of register %#.4x = %#.2x\n", offset, reg_data);
            return (PASSED);
        }

    } else {
        /* Display original value */
        if (fpga_reg_read32((int)offset, &reg_data32) == FAILED) {
            printf("Read FPGA register %#.4x failed\n", offset);
            return (FAILED);
        } else {
            printf("Original data of register %#.4x = %#lx\n", offset, reg_data32);
        }

        /* Alter register with new value */
        reg_data32 = gethex_answer("Enter the new data (hex): ", reg_data32, 0,
                                0xFFFFFFFF);

        if (fpga_reg_write32((int)offset, reg_data32) == FAILED) {
            printf("Write data %#lx to register %#.4x failed\n", reg_data32, offset);
            return (FAILED);
        }

        /* Display the value again */
        if (fpga_reg_read32((int)offset, &reg_data32)) {
            printf("Read FPGA register %#.4x failed\n", offset);
            return (FAILED);
        } else {
            printf("New data of register %#.4x = %#lx\n", offset, reg_data32);
            return (PASSED);
        }
    }
}

int fpga_reg_dp_util()
{
    int ix;
    int reg_num = sizeof(fpga_regs) / sizeof(fpga_regs[0]);

    for (ix = 0; ix < reg_num - 1; ix++) {
        uchar buf = 0;
        fpga_reg_read(fpga_regs[ix].offset, (char *)&buf);
        printf("%-36s reg %#.4x = %#.2x\n",
            fpga_regs[ix].name, fpga_regs[ix].offset, (uchar)buf);
    }

    reg_num = sizeof(fpga_regs32) / sizeof(fpga_regs32[0]);
    for (ix = 0; ix < reg_num - 1; ix++) {
        ulong buf = 0;
        fpga_reg_read32(fpga_regs32[ix].offset, (ulong *)&buf);
        printf("%-36s reg %#.4x = %#.8lx\n", 
            fpga_regs32[ix].name, fpga_regs32[ix].offset, buf);
    }

    return PASSED;
}

int fpga_1588_reg_dp_util()
{
    int ix;
    int reg_num = sizeof(fpga_regs) / sizeof(fpga_regs[0]);

    /* Last four 8-bit FPGA registers */
    for (ix = reg_num - 5; ix < reg_num - 1; ix++) {
        uchar buf = 0;
        fpga_reg_read(fpga_regs[ix].offset, (char *)&buf);
        printf("%-36s reg %#.4x = %#.2x\n",
            fpga_regs[ix].name, fpga_regs[ix].offset, (uchar)buf);
    }

    printf("Timestamp Data:\n");
    for (ix = 0x1100; ix < 0x1200; ix++) {
        uchar buf = 0;
        if ((ix & 0xF) == 0) {
            printf("\n%#.2x:\t", ix);
        }
        fpga_reg_read(ix, (char *)&buf);
        printf("%.2x ", (uchar)buf);
    }
    printf("\n");

    return PASSED;
}


/******************************************************************************
 *
 * Function: fpga_local_bus_read
 *
 * Description: This function supports to read the FPGA registers through local bus.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_local_bus_read (void)
{
    unsigned char reg_addr;
    uchar *fpga_offset; 

    printf("Read FPGA register through local bus\n");

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    } 

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xff): ", 0, 0, 0xff);
    fpga_offset += reg_addr;
    printf("Read FPGA register %#.8x, val = %#.8x\n", reg_addr, *fpga_offset);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_local_bus_write
 *
 * Description: This function supports to write the FPGA registers through local bus.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_local_bus_write (void)
{
    unsigned char reg_addr, reg_val;
    uchar *fpga_offset;

    printf("Alter FPGA register through local bus\n");

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    } 
        
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xff): ", 0, 0, 0xff);
    fpga_offset += reg_addr;
    printf("Read register %#.8x, original val = %#.8x\n", reg_addr, *fpga_offset);
    
    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", *fpga_offset, 0, 0xff);
    *fpga_offset = reg_val;
    printf("Read register %#.8x, new val = %#.8x\n", reg_addr, *fpga_offset);

    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : spi_flash_read
 * Description: Read the FPGA SPI Flash
 * Inputs     : opcode
 *              addr    - read address
 *              size    - read size
 *              rd_data - buffer for read data
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int spi_flash_read (int opcode, ulong addr, int size, uint8_t *rd_data)
{
    uchar spi_data;
    char reg_data;
    ulong reg_data32;
    int i;

    /* clear fpga_spi_stat register and wait for read FIFO empty. */
    fpga_reg_read32(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, &reg_data32);
    fpga_reg_write32(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, reg_data32);

    for (i = 0; i < 1000; i++) {
        fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, &reg_data);
        if (reg_data & FPGA_SPI_RD_FIFO_EMPTY)
            break;
        msleep(1);
    }

    if (i == 1000) {
        printf("Time out waiting for read FIFO empty.\n");
        return (FAILED);
    }

    /* set read data size */
    reg_data = size - 1;
    fpga_reg_write(FPGA_SPI_REG_BASE + FPGA_SPI_RD_SIZE_OFFSET, reg_data);

    /* set read address and opcode */
    reg_data32 = (opcode << 24) | (addr & 0xFFFFFF);
    fpga_reg_write32(FPGA_SPI_REG_BASE + FPGA_SPI_ADDR_OFFSET, reg_data32);

    /* set bit 0 and bit 2 of fpga_spi_ctrl register per
       requirement of the opcode used. */
    fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_CTRL_OFFSET, &reg_data);
    spi_data = reg_data;

    if (opcode == RD_DATA_BYTES) {
        spi_data |= FPGA_SPI_ADDR_EN;
    } else {
        spi_data &= (~FPGA_SPI_ADDR_EN);
    }

    /* write 0 to bit 1 for read */
    spi_data &= /*0xf9*/0xfd;
    fpga_reg_write(FPGA_SPI_REG_BASE + FPGA_SPI_CTRL_OFFSET, spi_data);

    /* wait for spi flash read to complete by polling DONE bit
       in fpga_spi_stat register */
    for (i = 0; i < 600; i++) {
        fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET + 1, &reg_data);
        if ((reg_data & FPGA_SPI_DONE) == FPGA_SPI_DONE) {
            break;
        }
        msleep(1);
    }

    if (i == 600) {
        printf("Time out waiting for read to complete.\n");
        return (FAILED);
    }

    /* read data */
    for (i = 0; i < size; i++) {
        fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_DATA_OFFSET, &reg_data);
        *rd_data++ = reg_data;
/*        printf("Read FPGA_SPI_DATA @%#x = %#x\n", 
            FPGA_SPI_REG_BASE + FPGA_SPI_DATA_OFFSET, reg_data);*/
    }

    fpga_reg_write(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET + 1, FPGA_SPI_DONE);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : spi_flash_write
 * Description: Write the FPGA SPI Flash
 * Inputs     : opcode
 *              addr    - write address
 *              size    - write size
 *              rd_data - buffer of write data
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int spi_flash_write (int opcode, ulong addr, int size, uint8_t *wr_data)
{
    uchar status_reg;
    char reg_data;
    ulong reg_data32;
    int i;
    fpga_reg_read32(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, &reg_data32);
    fpga_reg_write32(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, reg_data32);

    /* poll "Write In Progress" bit by reading SPI flash status 
       register and wait bit 0 to be 0. */
    for (i = 0; i < 3000; i++) {
        if (spi_flash_read(RD_STATUS, 0, 1, &status_reg) == FAILED) {
            printf("Failed to read SPI flash status register.\n");
            return (FAILED);
        }
        if (!(status_reg & RDSR_WIP))
            break;
        msleep(1);
    }

    if (i == 3000) {
        printf("Time out waiting for SPI flash ready.\n");
        return (FAILED);
    }

    /* check for write enable bit in SPI flash status register */
    if (opcode != WRITE_ENABLE) {
        if (!(status_reg & RDSR_WEL)) {
            printf("SPI flash write is not enabled, status = %#x.\n", status_reg);
            return (FAILED);
        }
    }

    /* set write address and opcode */
    reg_data32 = opcode << 24;

    if ((opcode == SECTOR_ERASE) || (opcode == PAGE_PROGRAM)) {
        reg_data32 |= (addr & 0xFFFFFF);
    }
//     printf("%s: reg_data32 = %#x\n", __FUNCTION__, reg_data32);
    fpga_reg_write32(FPGA_SPI_REG_BASE + FPGA_SPI_ADDR_OFFSET, reg_data32);

    if ((opcode == PAGE_PROGRAM) || (opcode == WR_STATUS)) {
        /* write the data to the write FIFO */
        for (i = 0; i < size; i++) {
            fpga_reg_write(FPGA_SPI_REG_BASE + FPGA_SPI_DATA_OFFSET, *wr_data++);
        }
    }

    /* set bit 1 of fpga_spi_ctrl register to 1 for write operation 
       and set bit 0 and bit 2 per requirement of the opcode used. */
    fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_CTRL_OFFSET, &reg_data);
    if ((opcode == SECTOR_ERASE) || (opcode == PAGE_PROGRAM)) {
        reg_data |= FPGA_SPI_ADDR_EN;
    } else {
        reg_data &= (~FPGA_SPI_ADDR_EN);
    }
    reg_data |= FPGA_SPI_WRITE;
    fpga_reg_write(FPGA_SPI_REG_BASE + FPGA_SPI_CTRL_OFFSET, reg_data);

    /* wait for spi flash write to complete by polling DONE bit
       in fpga_spi_stat register */
    for (i = 0; i < 6000; i++) {
        fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET + 1, &reg_data);
        if ((reg_data & FPGA_SPI_DONE) == FPGA_SPI_DONE) {
            break;
        }
        msleep(1);
    }

    if (i == 6000) {
        printf("Time out waiting for write to complete.\n");
        return (FAILED);
    }

    /* check bit 2 of fpga_spi_stat register to make sure 
       write FIFO is empty. If not, error has occured. */
    fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, &reg_data);
    if (!(reg_data & FPGA_SPI_WR_FIFO_EMPTY)) {
        printf("Error: SPI flash write FIFO is not empty.\n");
        return (FAILED);
    }

    /* clear fpga_spi_stat register. */
    fpga_reg_read(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, &reg_data);
    if ((reg_data & FPGA_SPI_WR_FIFO_ORUN) 
            == FPGA_SPI_WR_FIFO_ORUN) {
        fpga_reg_write(FPGA_SPI_REG_BASE + FPGA_SPI_STAT_OFFSET, FPGA_SPI_WR_FIFO_ORUN);
    }

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : peek_spi_flash
 * Description: Utility to peek the SPI flash
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int peek_spi_flash ()
{
    int opcode, size, i;
    ulong addr;
    uint8_t data[4];

    opcode = gethex_answer("\nEnter opcode to access SPI flash[03, 05, 9F]:",
               0x03, 0x03, 0x9f);
    if ((opcode != RD_IDENTIFICATION) && (opcode != RD_STATUS)
        && (opcode != RD_DATA_BYTES)) {
        printf("\nWrong opcode to access SPI flash!\n");
        return (FAILED);
    }

    if (opcode == RD_IDENTIFICATION) {
        size = 4;
    } else {
        size = 1;
    }

    if ((opcode == RD_DATA_BYTES) || (opcode == RD_DATA_BYTES_HIGH_SPEED)) {
        addr = gethex_answer("\nEnter address to access SPI flash"
                    "[0x100000 - 0x200000]:", 
                    0x100000, 0x100000, 0x200000);
    } else {
        addr = 0;
    }

    if (spi_flash_read(opcode, addr, size, data) == FAILED) {
        printf("\nFailed to read from SPI flash.\n");
        return (FAILED);
    }

    for (i = 0; i < size; i++) {
        printf("\nData = %#x", data[i]);
    }

    return (PASSED);
}


/*****************************************************************
 *
 * Function: fpga_upgrade_secondary()
 *
 * Description: This function upgrades secondary Wallander PFGA image in
  *             the SPI flash.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_upgrade_secondary (void)
{
    uint i, write_size;
    unsigned char *fpga_image_fw, *fpga_data;
    int fpga_image_fw_size;
    ulong mb_ctrl_addr, mb_ctrl_data, fpga_addr;
    unsigned char wr_status;

    printf("\nStart secondary FPGA upgrade!\n");

    fpga_image_fw = (unsigned char *)fpga_normal_image_fw_array;
    fpga_image_fw_size = fpga_normal_image_fw_size;

    printf("Image size : %#x\n", fpga_image_fw_size);
    mb_ctrl_addr = FPGA_MB_REG_BASE + MB_CTRL_OFFSET;
    if (fpga_reg_read32(mb_ctrl_addr, &mb_ctrl_data) == FAILED) {
        return (FAILED);
    }

    /* 1. before writing the upgrade image to spi flash, reset bit 0 
       of mb_ctrl_reg to invalidate the header register. */
    mb_ctrl_data &= 0xffffffe;
    if (fpga_reg_write32(mb_ctrl_addr, mb_ctrl_data) == FAILED) {
        return (FAILED);
    }

    /* enable SPI flash write */
    printf("1. Enable SPI flash write.\n");
    fpga_addr = SECONDARY_FPGA_IMAGE_START_ADDR;
    if (spi_flash_write(WRITE_ENABLE, /*SECONDARY_FPGA_IMAGE_START_ADDR*/0, 0, NULL) == FAILED) {
        printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* 2. Before programming, set status register BP3:BP0=000 to make 
       all secotrs unprotected. */
    printf("2. Write to SPI flash status register.\n");
    wr_status = 0x00;
    if (spi_flash_write(WR_STATUS, /*SECONDARY_FPGA_IMAGE_START_ADDR*/0, 1, &wr_status) == FAILED) {
        printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    /* 3. write the upgrade image to spi flash */
    printf("3. Write the upgrade image to spi flash.\n");
    fpga_addr = SECONDARY_FPGA_IMAGE_START_ADDR;
    fpga_data = fpga_image_fw;
    for (i = fpga_image_fw_size; i > 0; ) {
#ifdef DEBUG
        printf("i = %#x, fpga_addr = %#x\n", i, fpga_addr);
#endif
        if (i >= FPGA_SPI_PAGE_SIZE)
            write_size = FPGA_SPI_PAGE_SIZE;
        else
            write_size = i;

        if (!(fpga_addr & 0xffff)) {
#ifdef DEBUG
            printf("sector erase. fpga_addr = %#x\n", fpga_addr);
#endif
            /* enable SPI flash write */
            if (spi_flash_write(WRITE_ENABLE, /*fpga_addr & 0x1f0000*/0, 0, NULL) == FAILED) {
                printf("Failed to enable SPI flash write.\n");
            return (FAILED);
            }

            /* erase sector (64KB) */
            printf("   Erase sector (64KB) fpga_addr = %#lx\n", fpga_addr);
            if (spi_flash_write(SECTOR_ERASE, fpga_addr & 0x1f0000, 0, NULL) 
            == FAILED) {
                printf("Failed to erase sector.\n");
                return (FAILED);
            }
        }

        /* enable SPI flash write */
        if (spi_flash_write(WRITE_ENABLE, /*fpga_addr & 0x1f0000*/0, 0, NULL) == FAILED) {
            printf("Failed to enable SPI flash write.\n");
            return (FAILED);
        }

        if (spi_flash_write(PAGE_PROGRAM, fpga_addr, write_size,
                    fpga_data) == FAILED) {
            printf("Failed to write to SPI flash.\n");
            return (FAILED);
        }
        i -= FPGA_SPI_PAGE_SIZE;
        fpga_addr += write_size;
        fpga_data += write_size;
    }

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* 4. After programming, set status register BP3:BP0=1010 to protect 
    golden sectors(lower half, sectors 0-15th).
     * BP3:BP0 are at write status register bits 5:2.
     */
    printf("4. set status register BP3:BP0=1010 to protect golden sectors(0th-15th).\n");
    wr_status = 0x28;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
        printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    printf("\nFinish secondary FPGA upgrade!\n");

#ifdef DEBUG
    /* 5. set bit 1 of mb_ctrl_reg to reset the Reconfiguration FSM. 
    We can choose to reset automatically or let users to do the reset
    after the FPGA upgrade. */
    mb_ctrl_data |= 0x3;
    if (fpga_reg_write32((mb_ctrl_addr, 4, mb_ctrl_data, 0) == FAILED) {
        return (FAILED);
    }
#endif

    return (PASSED);
}

/*****************************************************************
 *
 * Function: fpga_upgrade_golden()
 *
 * Description: This function upgrades golden Wallander PFGA image in
  *             the SPI flash.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_upgrade_golden (void)
{
    uint i, write_size;
    unsigned char *fpga_image_fw, *fpga_data;
    int fpga_image_fw_size;
//     ulong mb_ctrl_addr, mb_ctrl_data;
    ulong fpga_addr;
    unsigned char wr_status;

    printf("\nStart golden FPGA upgrade!\n");

    fpga_image_fw = (unsigned char *)fpga_normal_image_fw_array;
    fpga_image_fw_size = fpga_normal_image_fw_size;

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* 1. Before programming, set status register BP3:BP0=0101 to make 
       lower half(16 secotrs) unprotected. */
    wr_status = 0x14;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
        printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    /* 2. write the golden image to spi flash */
    fpga_addr = GOLDEN_FPGA_IMAGE_START_ADDR;
    fpga_data = fpga_image_fw;
    for (i = fpga_image_fw_size; i > 0; ) {
#ifdef DEBUG
        printf("i = %#x, fpga_addr = %#x\n", i, fpga_addr);
#endif
        if (i >= FPGA_SPI_PAGE_SIZE)
            write_size = FPGA_SPI_PAGE_SIZE;
        else
            write_size = i;

        if (!(fpga_addr & 0xffff)) {
#ifdef DEBUG
            printf("sector erase. fpga_addr = %#x\n", fpga_addr);
#endif
            /* enable SPI flash write */
            if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
            printf("Failed to enable SPI flash write.\n");
            return (FAILED);
            }

            /* erase sector (64KB) */
            if (spi_flash_write(SECTOR_ERASE, fpga_addr & 0x1f0000, 0, NULL) 
            == FAILED) {
            printf("Failed to erase sector.\n");
            return (FAILED);
            }
        }

        /* enable SPI flash write */
        if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
            printf("Failed to enable SPI flash write.\n");
            return (FAILED);
        }

        if (spi_flash_write(PAGE_PROGRAM, fpga_addr, write_size,
                    fpga_data) == FAILED) {
            printf("Failed to write to SPI flash.\n");
            return (FAILED);
        }
        i -= FPGA_SPI_PAGE_SIZE;
        fpga_addr += write_size;
        fpga_data += write_size;
    }

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* 3. After programming, set status register BP2:BP0=111 to protect 
    all sectors. */
    wr_status = 0x1c;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
        printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    printf("\nFinish golden FPGA upgrade!\n");

    return (PASSED);
}

/*****************************************************************
 *
 * Function: fpga_lock_golden()
 *
 * Description: This function locks golden Wallander PFGA sectors in
  *             the SPI flash.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_lock_golden (void)
{
    unsigned char wr_status;

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* set status register BP3:BP0=1010 to protect golden sectors
        (lower half, sectors 0-15th).
     * BP3:BP0 are at write status register bits 5:2. 
     */
    wr_status = 0x28;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
        printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    printf("Lock sectors 0~15.\n");
    return (PASSED);
}

/*****************************************************************
 *
 * Function: fpga_unlock_golden()
 *
 * Description: This function unlocks golden Wallander PFGA sectors in
  *             the SPI flash.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_unlock_golden (void)
{
    unsigned char wr_status;

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* set status register BP3:BP0=0000 to unprotect all sectors
     * BP3:BP0 are at write status register bits 5:2. 
     */
    wr_status = 0x0;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
        printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    printf("Unlock all sectors.\n");
    return (PASSED);
}

/*****************************************************************
 *
 * Function: fpga_lock_test()
 *
 * Description: This function checks whether golden PFGA sectors are locked.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_lock_test (void)
{

    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_fpga_read32_fn
 *
 * Description: FPGA register read called by register_display
 *
 * Inputs      : addr - FPGA register offset
 *               size - FPGA register size
 *               *buf - pointer to the data buf
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_fpga_read32_fn (ulong addr, int size, ulong *buf, void *param)
{
    ulong reg_data32 = 0;
    fpga_reg_read32(addr, &reg_data32);

    *buf = reg_data32;
//     printf("%s: addr %#x data %#x *buf=%#x\n", __FUNCTION__, addr, reg_data32, *buf);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_fpga_write32_fn
 *
 * Description: handoff FPGA register write called by register_display
 *
 * Inputs      : addr - FPGA register offset
 *               size - FPGA register size
 *               data - the write data value
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_fpga_write32_fn (ulong addr, int size, ulong data, void *param)
{
    fpga_reg_write32((int)addr, data);
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: diag_fpga_test.c,v $
 * Revision 1.2  2015/02/26 08:15:43  xiaoyizh
 * Remove unused routines.
 *
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
