/* $Id: diag.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Wallander main menu and supporting wrappers.
 *
 * Xiaoying Zhang -- Feb. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdint.h>
//#include <defs.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvsysvars.h"
// #include "nvmonvars.h"
#include "common_utils.h"
// #include "pcmap.h"
#include "i2c_api.h"
// #include "module_fru.h"
#include "platform_i2c.h"
#include "diag_bootflash_test.h"
#include "diag_fpga_lib.h"
#include "diag_fpga_test.h"
#include "diag_ge_phy.h"
#include "cvmx.h"
#include "cvmx-pexp-defs.h"
#include "cvmx-gpio.h"

#define DIAG_RTN_STS_TMP_FILE                    "/tmp/wallander.status"
#define DIAG_RTN_STS_OUT_PORT_BASE               (2016)
#define BP_GE_IP_ADDR                            "192.123.123.1"
#define BP_PASS_STR                              "PASS"
#define BP_FAIL_STR                              "FAIL"

/* for main menu */
extern int module_mem_test(void);

/* Function prototype */
extern int alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int memtest(), memloop(), addrloop(), find_mem(), memdebug();

extern int fpga_test();
extern int ge_phy_test();
extern int fpga_sfp_led_test();

extern int sfp_eeprom_dp_util();

static int wallander_uart_msg_exh_test(void);
static int gpio_rd_util();
static int gpio_set_util();
static int led_test();

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/* FRU PID and Location Strings */
uchar io_pid[] = "IO-PID";
uchar dimm_pid[] = "DIMM-PID";

uchar io_loc[] = "IO";
uchar dimm0_loc[] = "IO/DIMM0";

fru_table_t platform_fru_table[] = {
    { io_pid,        io_loc },
    { dimm_pid,      dimm0_loc },
};

/* =========================================
 *  DDR3 Memory Access Utilities
 * ========================================= */
static struct mitem ddr3_mem_items[] = {
    {"alter memory",            0,      0,
     (PFT)alt_mem,              &one,   0, (type_t(*)())0, 0},
    {"compare memory block",    0,      0,
     (PFT)cmp_mem,              &one,   0, (type_t(*)())0, 0},
    {"display memory",          0,      0,
     (PFT)dis_mem,              &one,   0, (type_t(*)())0, 0},
    {"move memory block",       0,      0,
     (PFT)mov_mem,              &one,   0, (type_t(*)())0, 0},
    {"fill memory",             0,      0,
    (PFT)fil_mem,               &one,   0, (type_t(*)())0, 0},
    {"memory test",             0,      0,
     (PFT)memtest,              &one,   0, (type_t(*)())0, 0},
    {"memory read or write loop",   0,      0,
     (PFT)memloop,              &one,   0, (type_t(*)())0, 0},
    {"memory debug loop",       0,      0,
     (PFT)memdebug,             &one,   0, (type_t(*)())0, 0},
    {"address loop",            0,      0,
     (PFT)addrloop,             &one,   0, (type_t(*)())0, 0},
};

static struct menuinfo ddr3_mem_menu = {
    "DDR3 Memory Access Utilities Menu",
    0,
    0,
    0,
    sizeof(ddr3_mem_items)/sizeof(struct mitem),
    ddr3_mem_items,
};
static struct menuinfo *ddr3_mem_menup = &ddr3_mem_menu;

/* =========================================
 *  FPGA Utilities
 * ========================================= */
static struct mitem fpga_util_items[] = {
    {"Show Board type/FPGA version",    0,      0,
     (PFT)show_fpga_version,         &one,  0,  (type_t(*)())0, 0},
    {"FPGA system register read",       0,      0, 
     (PFT)fpga_reg_rd_util,         &one,   0,  (type_t(*)())0, 0},
    {"FPGA system register write",      0,      0, 
     (PFT)fpga_reg_wr_util,         &one,   0,  (type_t(*)())0, 0},
    {"FPGA system registers dump",      0,      0, 
     (PFT)fpga_reg_dp_util,         &one,   0,  (type_t(*)())0, 0},
    {"Peek SPI flash",                  0,      0,
     (PFT)peek_spi_flash,           &one,   0,  (type_t(*)())0, 0},
    {"Secondary FPGA image upgrade",    0,      0,
     (PFT)fpga_upgrade_secondary,   &one,   0,  (type_t(*)())0, 0},
    {"Golden FPGA image lock",          0,      0,
     (PFT)fpga_lock_golden,         &one,   0,  (type_t(*)())0, 0},
    {"Golden FPGA image unlock",        0,      0,
     (PFT)fpga_unlock_golden,         &one,   0,  (type_t(*)())0, 0},
};

static struct menuinfo fpga_util_menu = {
    "FPGA Utilities Menu",
    0,
    0,
    0,
    sizeof(fpga_util_items)/sizeof(struct mitem),
    fpga_util_items,
};
static struct menuinfo *fpga_util_menup = &fpga_util_menu;

/* =========================================
 *  PHY Utilities
 * ========================================= */
static struct mitem phy_util_items[] = {
    {"PHY register read",               0,  0,
     (PFT)phy_reg_rd_util,              &one,   0, (type_t(*)())0, 0},
    {"PHY register write",              0,  0,
     (PFT)phy_reg_wr_util,              &one,   0, (type_t(*)())0, 0},
    {"PHY register dump",               0,  0,
     (PFT)phy_reg_dp_util,              &one,   0, (type_t(*)())0, 0},
    {"Show port control and status",    0,  0,
     (PFT)show_port_ctrl_stat_util,     &one,   0, (type_t(*)())0, 0},
    {"Show test log",                   0,  0,
     (PFT)show_phy_test_log_util,       &one,   0, (type_t(*)())0, 0},
    {"SMI Control register dump",       0,  0,
     (PFT)smi_ctrl_reg_dp_util,         &one,   0, (type_t(*)())0, 0},
    {"Setup PHY test mode",             0,  0,
     (PFT)setup_phy_test_mode_util,     &one,   0, (type_t(*)())0, 0},
    {"PHY 1588 register read",          0,  0,
     (PFT)phy_1588_reg_rd_util,         &one,   0, (type_t(*)())0, 0},
    {"PHY 1588 register write",         0,  0,
     (PFT)phy_1588_reg_wr_util,         &one,   0, (type_t(*)())0, 0},
    {"PHY 1588 register dump",          0,  0,
     (PFT)phy_1588_reg_dp_util,         &one,   0, (type_t(*)())0, 0},
    {"FPGA 1588 register dump",         0,  0,
     (PFT)fpga_1588_reg_dp_util,        &one,   0, (type_t(*)())0, 0},
    {"PHY 1588 Tod",                    0,  0,
     (PFT)phy_1588_tod_util,            &one,   0, (type_t(*)())0, 0},
    {"PHY 1588 register Stat",          0,  0,
     (PFT)phy_reg_dp_util,              &one,   0, (type_t(*)())0, 0},
};

static struct menuinfo phy_util_menu = {
    "PHY Utilities Menu",
    0,
    0,
    0,
    sizeof(phy_util_items)/sizeof(struct mitem),
    phy_util_items,
};
static struct menuinfo *phy_util_menup = &phy_util_menu;

/* =========================================
 *  Voltage Margining Utilities
 * ========================================= */
static struct mitem margin_util_items[] = {
    {"Margin all voltages to high", 0,0, (PFT)voltage_margin_high,
                                    &one,  0, (type_t(*)())0, 0},
    {"Margin all voltages to low",  0,0, (PFT)voltage_margin_low,
                                    &one,  0, (type_t(*)())0, 0},
    {"Set all voltages to nominal", 0,0, (PFT)voltage_margin_nom,
                                    &one,  0, (type_t(*)())0, 0},
    {"Set all voltages to no margin", 0,0, (PFT)voltage_no_margin,
                                    &one,  0, (type_t(*)())0, 0},
    {"Margin a specific voltage",   0,0, (PFT)voltage_margin_specific,
                                    &one,  0, (type_t(*)())0, 0},
    {"Display current margins", 0,0, (PFT)voltage_margin_display,
                            &one,  0, (type_t(*)())0, 0},
};

static struct menuinfo margin_util_menu = {
    "Voltage Margin Utilities Menu",
    0,
    0,
    0,
    sizeof(margin_util_items)/sizeof(struct mitem),
    margin_util_items,
};
static struct menuinfo *margin_util_menup = &margin_util_menu;

/* =========================================
 *  I2c Utilities
 * ========================================= */
static struct mitem i2c_util_items[] = {
    {"I2c Read", 0,0, (PFT)i2c_rd_util,
                                    &one,  0, (type_t(*)())0, 0},
    {"I2c Write",  0,0, (PFT)i2c_wr_util,
                                    &one,  0, (type_t(*)())0, 0},
};

static struct menuinfo i2c_util_menu = {
    "I2c Utilities Menu",
    0,
    0,
    0,
    sizeof(i2c_util_items)/sizeof(struct mitem),
    i2c_util_items,
};
static struct menuinfo *i2c_util_menup = &i2c_util_menu;

/* =========================================
 *  Bootflash Utilities
 * ========================================= */
static struct mitem bootflash_util_items[] = {
    {"Bootflash Get Information",       0,0,    (PFT)get_bootflash_info,
                                    (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"Bootflash Lock Golden Image",     0,0,    (PFT)bootflash_lock_golden,
                                    (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"Bootflash Golden Lock Test",      0,0,    (PFT)bootflash_golden_lock_test,
                                    (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"Bootflash Unlock Golden Image",   0,0,    (PFT)bootflash_all_ppb_earse,
                                    (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"Bootflash Test",                  0,0,    (PFT)bootflash_test,
                                    (type_t *)&zero,       0,    (type_t(*)())0, 0},
};

static struct menuinfo bootflash_util_menu = {
    "Bootflash Utilities Menu",
    0,
    0,
    0,
    sizeof(bootflash_util_items)/sizeof(struct mitem),
    bootflash_util_items,
};
static struct menuinfo *bootflash_util_menup = &bootflash_util_menu;

/* =========================================
 *  GPIO Utilities
 * ========================================= */
static struct mitem gpio_util_items[] = {
    {"GPIO Read",       0,      0,
     (PFT)gpio_rd_util,         &one,   0, (type_t(*)())0, 0},
    {"GPIO Set",        0,      0, 
     (PFT)gpio_set_util,        &one,   0, (type_t(*)())0, 0},
};

static struct menuinfo gpio_util_menu = {
    "GPIO Utilities Menu",
    0,
    0,
    0,
    sizeof(gpio_util_items)/sizeof(struct mitem),
    gpio_util_items,
};
static struct menuinfo *gpio_util_menup = &gpio_util_menu;


/* =========================================
 *   Basic utilities
 * ========================================= */
static struct mitem utilmenuitems[] = {
    {"DDR3 Memory Access",      0,      0,
     (PFT)menu,     (type_t *)&ddr3_mem_menup,      0, (type_t(*)())0,0},
    {"FPGA Utilities",          0,      0,
     (PFT)menu,     (type_t *)&fpga_util_menup,     0, (type_t(*)())0,0},
    {"PHY Utilities",           0,      0,
     (PFT)menu,     (type_t *)&phy_util_menup,      0, (type_t(*)())0,0},
    {"Margin Utilities",        0,      0,
     (PFT)menu,     (type_t *)&margin_util_menup,   0, (type_t(*)())0,0},
    {"I2c Utilities",           0,      0,
     (PFT)menu,     (type_t *)&i2c_util_menup,      0, (type_t(*)())0,0},
    {"Flash Utilities",         0,      0,
     (PFT)menu,     (type_t *)&bootflash_util_menup,0, (type_t(*)())0,0},
    {"SFP Utilities",           0,      0,
     (PFT)sfp_eeprom_dp_util,   (type_t *)&one,     0, (type_t(*)())0,0},
    {"GPIO Utilities",          0,      0,
     (PFT)menu,     (type_t *)&gpio_util_menup,     0, (type_t(*)())0,0},
};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems)/sizeof(struct mitem),
    utilmenuitems,
 };
struct menuinfo *utilmenup = &utilmenu;

/* =========================================
 *  Main menu items
 * ========================================= */
static submenu_xtable_t main_menu_table[] = {
    {"Linux memory test",   (PFT)module_mem_test,  0,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (type_t(*)())0,     0},
    {"FPGA test",           (PFT)fpga_test,    FALSE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0, (type_t(*)())fpga_test, TRUE},
    {"GE PHY test",         (PFT)ge_phy_test,    FALSE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0, (type_t(*)())ge_phy_test, TRUE},
/*    {"Bootflash test",      (PFT)bootflash_test,    0,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0, (type_t(*)())bootflash_test,   0},*/
    {"LED test",            (PFT)led_test,    FALSE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0, (type_t(*)())ge_phy_test, TRUE},
    {"Dummy item to send string to UART",
     (PFT)wallander_uart_msg_exh_test,    0,          0,
     (type_t(*)())0, 0,     (PFT)0,                   0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",      /* title */
    0,              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,  /* shows major flags */
    0,              /* generic prompt */
    0,              /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*********************************************************************
 * Function: diag_menu
 * Description: This is the main entry to diag menu interface.
 * Inputs: argc
 *         argv
 * Outputs: None
 *********************************************************************
 */
void diag_menu(int argc, char *argv[]) 
{
    char arg;

    if(argc > 1) {
        arg = *argv[1]; 
    } else { 
        arg = 0;
    }

    testname("Wallander NGWIC");

    (NVRAM)->pollcon = 1;       /* poll the console */

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
        &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
        main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

static int led_test(void)
{
    if (fpga_sfp_led_test()) {
        return (FAILED);
    }
    return (PASSED);
}

static int gpio_rd_util(void)
{
    printf("Read GPIO: %#llx\n", (long long unsigned int)cvmx_gpio_read());
    return (PASSED);
}

static int gpio_set_util(void)
{
    uint64_t set_mask = 0;

    set_mask = gethex_answer("\nEnter GPIO Set Mask:",
                0, 0, 0xFFFFFFFF);

    cvmx_gpio_set(set_mask);

    /* Read Back */
    printf("Current GPIO: %#llx\n", (long long unsigned int)cvmx_gpio_read());

    return (PASSED);
}

int cvmx_csr_read_util(void)
{
    uint64_t addr64;
    uint64_t data;

    addr64 = gethex_answer("\nEnter 64-bit Addr:",
                0, 0, 0xFFFFFFFFFFFFFFFFULL);

    printf("cvmx_read_csr(%#llx)\n", (long long unsigned int)addr64);
    data = cvmx_read_csr(addr64);
    printf("%#llx\n", (long long unsigned int)data);

    return (PASSED);
}

/* Check whether the register offset is valid */
int check_offset (ushort offset, reg_info_t* reg_table_p)
{
    for (; reg_table_p->size.size != 0; reg_table_p++) {
        if (reg_table_p->offset == offset) {
            return (PASSED);
        }
    }
    return (FAILED);
}

/* Get the register size */
ulong get_reg_size (ushort offset, reg_info_t* reg_table_p)
{
    for (; reg_table_p->size.size != 0; reg_table_p++) {
        if (reg_table_p->offset == offset) {
            return (reg_table_p->size.size);
        }
    }
    return (0);
}

void reg_dump(ulong base_addr, reg_info_t* reg_table_p)
{
    uchar *reg_p;

    for (; reg_table_p->size.size != 0; reg_table_p++) {
        reg_p = (uchar *)(base_addr + reg_table_p->offset);

        switch (reg_table_p->size.size) {
        case 1:
            printf("\n %s @%#x = %#x ", reg_table_p->name, 
                (unsigned int)(base_addr + reg_table_p->offset), *reg_p);
        break;
        case 2:
            printf("\n %s @%#x = %#x ", reg_table_p->name, 
                (unsigned int)(base_addr + reg_table_p->offset), *(unsigned int *)reg_p);
        break;
        case 4:
            printf("\n %s @%#x = %#x ", reg_table_p->name, 
                (unsigned int)(base_addr + reg_table_p->offset), *(unsigned int *)reg_p);
        break;
        }
    }
}

/*
 * Function: wallander_uart_msg_exh_test()
 *
 * 'uname' to generate strings for the host to catch via UART.
 *  This is to respond to overlord side UART test
 *
 * Input : NONE
 * Output: PASSED
 */
static int wallander_uart_msg_exh_test (void)
{
    /* using 'uname' to dispay system info as a string.
     * x86 side will compare string for uart test 
     */ 
    system("uname");
    return PASSED;
}

/**********************************************************************
 *
 * Function: diag_report_status_host
 *
 * This function reports the pass/fail status to host through nc
 *
 * Input : str - status string
 *
 * Output: none
 *
 **********************************************************************
 */
static void diag_report_status_host (char *str)
{
    char cmd[128];

    /* Sanity check */
    if (str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return;
    }

    sprintf(cmd, "echo %s > result", str);
    printf("%s\n", cmd);
    system(cmd);
    sleep(1);
    sprintf(cmd, "nc %s %d < result", BP_GE_IP_ADDR,
            DIAG_RTN_STS_OUT_PORT_BASE);
    printf("Wallander nc command: %s\n", cmd);
    system(cmd);
    sleep(1);
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
 * Function: diag_do_all
 *
 * Description: This function performs all tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int diag_do_all (void)
{
    doall_print_head("Memory");
    if (module_mem_test() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "Main Memory Test Fails");
        return (FAILED);
    }
    doall_print_tail("Memory");

    doall_print_head("FPGA");
    if (fpga_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "FPGA Test Fails");
        return (FAILED);
    }
    doall_print_tail("FPGA");

    doall_print_head("PHY");
    if (ge_phy_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "PHY Test Fails");
        return (FAILED);
    }
    doall_print_tail("PHY");

    doall_print_head("LED");
    if (led_test() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "LED Test Fails");
        return (FAILED);
    }
    doall_print_tail("LED");

    diag_report_status_host(BP_PASS_STR);

    return (PASSED);
}


/******** History ******** 
$Log: diag.c,v $
Revision 1.1  2015/02/26 07:18:29  xiaoyizh
Initial check in for Wallander.


$Endlog$
*/
