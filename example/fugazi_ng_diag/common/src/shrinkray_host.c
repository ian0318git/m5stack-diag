/* $Id: shrinkray_host.c,v 1.6 2019/08/06 06:56:06 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shrinkray_host.c,v $
 *------------------------------------------------------------------
 *
 * shrinkray_host.c: Main file for ShrinkRay host side Diag.
 *
 * May 2013 - Paul Lin(palin2) ported from Lebowski
 *
 * Original Author: Ian Chang
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
 
/*------------------------------------------------------------------------------
 * includes
 *------------------------------------------------------------------------------
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <pthread.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "strings.h"
#include "signals.h"
#include "linux_api.h"
#include "proto.h"
#include "error.h"
#include "menu.h"
#include "queryflags.h"
#include "cross_platform.h"
#include "pci.h"
#include "dev_object.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "nvmonvars.h"
#include "slot.h"
#include "shrinkray_host.h"
#include "sgmii_defs.h"
#include "pca.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "act2_utils.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include <assert.h>
#include "defs.h"
#include "common_utils.h"

/*------------------------------------------------------------------------------
 * globals
 *------------------------------------------------------------------------------
 */
int shrinkray_test_slot = 0;
static void           (*shrinkray_saved_diag_exec)(void) = NULL;
static int (*savfcn)() = NULL;
static shrinkray_ds_t shrinkray_info;
static shrinkray_ds_t *shrinkray_iface_p;
shrinkray_ds_t        shrinkray_iface[MAX_SM+1];
static speed_t        slot1_uart = 0;
static speed_t        slot2_uart = 0;

static reg_info_t pca9555_reg_tbl[]=
{
    {"Input port 0",                PCA9555_IN_PORT0_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0x00, 0x00},
    {"Input port 1",                PCA9555_IN_PORT1_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0x00, 0x00},
    {"Output port 0",               PCA9555_OUT_PORT0_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
    {"Output port 1",               PCA9555_OUT_PORT1_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
    {"Polarity Inversion port 0",   PCA9555_POLAR_INV_P0_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0x00},
    {"Polarity Inversion port 1",   PCA9555_POLAR_INV_P1_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0x00},
    {"Configuration port 0",        PCA9555_CFG_PORT0_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
    {"Configuration port 1",        PCA9555_CFG_PORT1_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
};

/*------------------------------------------------------------------------------
 * prototypes
 *------------------------------------------------------------------------------
 */
static int  shrinkray_ioe_reg_test(void);
static long ge_bp_lpbk_test(void);
static int  pwr_off_shrinkray_sm_wrap(void);
static int  pwr_off_shrinkray_sm(void);
static int  pwr_on_shrinkray_sm(void);
static int  pwr_cycle_shrinkray_sm(void);
static int  shrinkray_submodule_reset(void);
static int  ltc4215_register_test(void);
static int  ltc4215_led_test(void);
static int  oir_ltc4215_tests(int);
static int  util_ltc4215_reg_read(void);
static int  util_ltc4215_reg_write(void);
static int  set_shrinkray_uart_baud(void);
static int  show_shrinkray_sm_pwr(void);
static int  shrinkray_host_utility(int);
static int  shrinkray_sm_cpu0_console(int);
static int  shrinkray_sm_cpu1_console(int);
static int  shrinkray_sm_init(void);
static void shrinkray_sm_cleanup(void);
static void shrinkray_setup_uart(void);
static int  act2lite_tests(int);
static uint32_t get_shrinkray_sm_current(uint8_t);
static int  shrinkray_hipwr_ctrl_util(int);
static int  shrinkray_ioe_init(void);
static long shrinkray_gesw_lpbk_set(void);
int set_shrinkray_hipwr(boolean);
int pca9555_reg_dump_util(void);
int pca9555_reg_read_util(void);
int pca9555_reg_write_util(void);

/*------------------------------------------------------------------------------
 * Externs
 *------------------------------------------------------------------------------
 */
extern int act2_version(int); 
extern int sgmii_lpbk_util(int, int);

/*------------------------------------------------------------------------------
 * constants
 *------------------------------------------------------------------------------
 */
static uart_baud_info shrinkray_uart_baud[] = {
    {"115200",   B115200},
    {"9600",     B9600}
};

/* 
 * Sub Menu used for Utility.
 */
static mitem_t shrinkray_util_submenu_table[] = {
    { "LTC4215 Register Read",           0, 0,   (PFT)util_ltc4215_reg_read,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "LTC4215 Register Write",          0, 0,   (PFT)util_ltc4215_reg_write,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "IO Expander(PCA9555) Reg. Dump",  0, 0,   (PFT)pca9555_reg_dump_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "IO Expander(PCA9555) Reg. Read",  0, 0,   (PFT)pca9555_reg_read_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "IO Expander(PCA9555) Reg. Write", 0, 0,   (PFT)pca9555_reg_write_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Enable ShrinkRay High Power",     0, 0,   (PFT)shrinkray_hipwr_ctrl_util,
      (type_t *)&one, 0,    (type_t(*)())0, 0 },
    { "Disable ShrinkRay High Power",    0, 0,   (PFT)shrinkray_hipwr_ctrl_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Display Shrinkray SM Power",      0, 0,   (PFT)show_shrinkray_sm_pwr,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Shrinkray Sub-Module Reset",       0, 0,   (PFT)shrinkray_submodule_reset,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Power Off Shrinkray SM",          0, 0,   (PFT)pwr_off_shrinkray_sm_wrap,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Power On Shrinkray SM",           0, 0,   (PFT)pwr_on_shrinkray_sm,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Power Cycle Shrinkray SM",        0, 0,   (PFT)pwr_cycle_shrinkray_sm,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Set Shrinkray UART Baud Rate",    0, 0,   (PFT)set_shrinkray_uart_baud,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Platform GE Switch Line Loopback Setup", 0, 0, (PFT)shrinkray_gesw_lpbk_set,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "TLK10232 Loopback Test From Host E0", 0, 0, (PFT)ge_bp_lpbk_test,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
};

#define SHRINKRAY_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(shrinkray_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t shrinkray_util_subtest_menu = {
    "Shrinkray Host Utilities Menu",
    (type_t)0,                             /* title param */
    (PFT)menu_show_dflags,                 /* show diag flags */
    0,
    SHRINKRAY_UTIL_SUBMENU_TABLE_SZ,
    shrinkray_util_submenu_table,
};

static menuinfo_t *shrinkray_util_submenup = &shrinkray_util_subtest_menu;

/* 
 * Sub Menu used for OIR(LTC4215) tests.
 */
static submenu_xtable_t oir_submenu_table[] = {
    {"OIR(LTC4215) reg. Read util",     (PFT)util_ltc4215_reg_read,  0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"OIR(LTC4215) reg. Write util",    (PFT)util_ltc4215_reg_write, 0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"OIR(LTC4215) Register Test",      (PFT)ltc4215_register_test,  0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"OIR(LTC4215) LED Test",           (PFT)ltc4215_led_test,       0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
};

#define OIR_SUBMENU_TABLE_SIZE (sizeof(oir_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t oir_tests_primary_items[OIR_SUBMENU_TABLE_SIZE + 
                                       MAX_BASE_ITEMS];
static mitem_t oir_tests_secondary_items[OIR_SUBMENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static menuinfo_t oir_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    oir_tests_primary_items,
};

static menuinfo_t *oir_submenup = &oir_subtest_menu;

/*
 * Sub Menu used for Shrinkray Host side tests.
 */
static submenu_xtable_t shrinkray_submenu_table[] = {
    {"Shrinkray Host Utilities",         (PFT)shrinkray_host_utility,       0,
     0,
     (type_t(*)())0, 0,                  (PFT)shrinkray_host_utility,       0},
    {"ShrinkRay IO Expander Reg. Test",  (PFT)shrinkray_ioe_reg_test,       0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0, 0,                  (type_t(*)())0,                    0},
    {"OIR(LTC4215) Test",                (PFT)oir_ltc4215_tests,            0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0, 0,                  (PFT)oir_ltc4215_tests,            TRUE},
    {"ACT2 Lite Test",                   (PFT)act2lite_tests,               0,
     MF_CONTINUOUS,
     (type_t(*)())0, 0,                  (PFT)act2lite_tests,               TRUE},
    {"Shrinkray CPU0 Console",           (PFT)shrinkray_sm_cpu0_console,    0,
     0,
     (type_t(*)())0, 0,                  (type_t(*)())0,                    0},
    {"Shrinkray CPU1 Console",           (PFT)shrinkray_sm_cpu1_console,    0,
     0,
     (type_t(*)())0, 0,                  (type_t(*)())0,                    0},
};

#define SHRINKRAY_SUBMENU_TABLE_SIZE (sizeof(shrinkray_submenu_table) / \
                                      sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t shrinkray_tests_primary_items[SHRINKRAY_SUBMENU_TABLE_SIZE + 
                                            MAX_BASE_ITEMS];
static mitem_t shrinkray_tests_secondary_items[SHRINKRAY_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

static menuinfo_t shrinkray_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    shrinkray_tests_primary_items,
};

static menuinfo_t *shrinkray_submenup = &shrinkray_subtest_menu;

static n2g_i2c_if_t pca_i2c[] = {
    {
        .i2c_dev = SM_I2C_ADDR_IO_PORT,
    },
    {
        .i2c_dev = PCA9555_I2C_ADDRESS,
    },
};

static char pca_buff0[256];
static n2g_i2c_if_t *oir;

/*------------------------------------------------------------------------------
 * functions
 *------------------------------------------------------------------------------
 */

/**********************************************************************
 *
 * Function: shrinkray_gesw_lpbk_set
 *
 * Description: This function is the wrapper to enables or disable the line
 *              loopback of the GE switch port connected to the Shrinkray.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long shrinkray_gesw_lpbk_set (void)
{
    uint8_t ans;
    uint port;
    int ge_port, ge_port0, ge_port1, ge_port2;
    int real_slot = shrinkray_iface_p->slot;

    printf("\n");
    testname("Switch the Overlord GESW");

    port = gethex_answer("Overlord GESW port E0/E1/E2(XAUI)", 0, 0, 2);
    /* 0 = Serdex Mux,  1 = BP, 2 = Xaui */
    ge_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, port);
    printf("\nEnable the backplane loopback? (y/n) ");
    fflush(0);
    ans = getchar();
    if (ans != 'y' && ans != 'Y') {
        set_gesw_line_loopback(ge_port, 0);
    } else {
        set_gesw_line_loopback(ge_port, 1);
    }
    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0);
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1);
    ge_port2 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 2);
    printf("\nSM slot %d, E0 : GESW port %d = %s",real_slot, ge_port0 ,
            get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
    printf("\nSM slot %d, E1 : GESW port %d = %s",real_slot, ge_port1 ,
            get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
    printf("\nSM slot %d, E2 : GESW port %d = %s\n",real_slot, ge_port2 ,
            get_gesw_line_loopback(ge_port2) ? "ENABLE" : "DISABLE");
    fflush(0);

    return(PASSED);
}


/**********************************************************************
*
* Function: ge_bp_lpbk_test
*
* Perform GE loopback test to verify GE0 backplane connectivity between
* host and SM card
*
* Input : none
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
static long ge_bp_lpbk_test (void)
{
    int rc = PASSED;
    int packet_no = SHRINKRAY_GE_BP_PACKET_NO;
    int ctrl_plane_sgmii_port;

    testname(" GE Backplane Loopback");
    prpass(testpass, "Setting TLK10232 loopback bit");

    prpass(testpass, "Running loopback test now");

    ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port();
    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, packet_no) != PASSED) {
        cterr('f', 0, "GE loopback from Host side %d fails.",
        CPU_SGMII_PORT1);
        rc = (FAILED);
    } else {
        printf("Loopback test pass.\n");
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : ltc4215_register_test
 * Description: Wrapped function to do LTC4215 register test.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ltc4215_register_test (void)
{
    testname("OIR(LTC4215) Register");
    return (oir_ltc4215_register_test(oir));
}

/*******************************************************************************
 *
 * Function   : ltc4215_led_test
 * Description: Wrapped function to do LTC4215 LED test.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ltc4215_led_test (void)
{
    testname("OIR(LTC4215) LED");
    return (oir_ltc4215_leds_test(oir));
}

/*******************************************************************************
 *
 * Function   : oir_ltc4215_tests
 * Description: Entry function of ShinkRay OIR(LTC4215)
 *              Diag tests and utilities.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
oir_ltc4215_tests (int show_menu)
{
    testname("OIR(LTC4215)");

    build_primary_submenu(oir_submenu_table, 
                          OIR_SUBMENU_TABLE_SIZE,
                          "OIR(LTC4215)", &oir_submenup);
    build_secondary_submenu(oir_submenu_table,
                            OIR_SUBMENU_TABLE_SIZE,
                            oir_tests_secondary_items);

    if (show_menu) {
        menu(oir_submenup, oir_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(oir_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_ltc4215_reg_write
 * Description: LTC4215 Register Write utility.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_ltc4215_reg_write (void)
{
    return (util_oir_ltc4215_reg_write(oir));
}

/*******************************************************************************
 *
 * Function   : util_ltc4215_reg_read
 * Description: LTC4215 Register Read utility.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_ltc4215_reg_read (void)
{
    return (util_oir_ltc4215_reg_read((void *)oir));
}

/*******************************************************************************
 *
 * Function   : show_shrinkray_sm_pwr
 * Description: Display power of ShrinkRay.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
show_shrinkray_sm_pwr (void)
{
    uint32_t voltage, current, power;
    uint8_t data = 0;

    printf("\n\nShrinkray SM Power Measure:\n\n");

    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data)) {
        return (FAILED);
    }
    voltage = (data * SINGLE_SM_VOL) / 100;

    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data)) {
        return (FAILED);
    }
    current = get_shrinkray_sm_current(data) / 100;

    power = voltage * current;

    printf("Voltage = %d.%02d V\n", (voltage / 100), (voltage % 100));
    printf("Current = %d.%02d A\n", (current / 100), (current % 100));
    printf("Power = %d.%02d W\n", (power / 10000), ((power % 10000) / 100));

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : get_shrinkray_sm_current
 * Description: convert sense register value into current.
 * Inputs     : Sense Register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t
get_shrinkray_sm_current (uint8_t data)
{
    uint32_t       current = 0;
    if (data) {
        current = (data - 1) * SINGLE_SM_CURRENT;
    } else {
        current = 0;
    }
    return (current);
}

/*******************************************************************************
 *
 * Function   : pwr_off_shrinkray_sm_wrap
 * Description: Warpped function to power-off Shrinkray SM.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_off_shrinkray_sm_wrap (void)
{
    uint8_t ans;

    printf("\n\nReally want to Power off ShrinkRay? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nUser Abort! Shrinkray SM will stay in power ON.\n\n");
        return (PASSED);
    }
    return (pwr_off_shrinkray_sm());
}

/*******************************************************************************
 *
 * Function   : pwr_off_shrinkray_sm
 * Description: Function to power off Shrinkray SM.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_off_shrinkray_sm (void)
{
    uint8_t data = 0;

    printf("\nPower Off the Shrinkray SM.\n");

    if (util_oir_ltc4215_led(oir, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off SM module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is turned off */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (data & LTC4215_FET_ON_STATUS) {
        printf("FET CANNOT be Turned Off.\n");
        return(FAILED);
    }
    if (data & LTC4215_FET_SHORT_PRESENT) {
        printf("FET Shortage Detected.\n");
        return(FAILED);
    }
    if (!(data & LTC4215_POWER_BAD_STATUS)) {
        printf("Power CANNOT be Turned Off.\n");
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pwr_on_shrinkray_sm
 * Description: Function to power on Shrinkray SM.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_on_shrinkray_sm (void)
{
    uint8_t data = 0;

    printf("\nPower On the Shrinkray SM.\n");

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    // power on sm module
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return(FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return(FAILED);
    }

    printf("Waiting for Shrinkray SM to Power-Up.\n");
    msleep(2000);

    printf("Reinitializing the Shrinkray SM.\n");

    if (shrinkray_sm_init()) {
        shrinkray_sm_cleanup();
        return(FAILED);
    }

    // turn on the green light status if PSE2 re-init successfully
    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pwr_cycle_shrinkray_sm
 * Description: Wrapped function to power cycle ShrinkRay SM.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_cycle_shrinkray_sm (void)
{
    uint8_t i, ans;

    printf("\n");
    testname("Power Cycle Shrinkray SM");

    printf("\n\nReally want to Power Cycle ShrinkRay? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nUser Abort! "
                "Stop power-cycle ShrinkRay process.\n\n");
        return (PASSED);
    }

    if (pwr_off_shrinkray_sm() != PASSED) {
        cterr('f', 0, "Failed to Power OFF the Shrinkray SM");
        return (FAILED);
    }

    /* msleep for 10 seconds ?? Check if ShrinkRay needs. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }
    printf("\n");

    if (pwr_on_shrinkray_sm() != PASSED) {
        cterr('f', 0, "Failed to Power ON the Shrinkray SM");
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_shrinkray_uart_baud
 * Description: Function to set ShrinkRay uart baud rate.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
set_shrinkray_uart_baud (void)
{
    const   int maxlen = 128;
    char    tty[maxlen];
    int     fd, slot;
    struct  termios  newtio, ori_conf;
    shrinkray_ds_t   *iface;
    speed_t new_baud = 0, uart_baud_rate = 0;

    iface = shrinkray_iface_p;
    assert(iface);
    slot = shrinkray_iface_p->slot;

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", shrinkray_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }
    tcgetattr(fd, &newtio);

    printf("\n\n Set Shrinkray UART Baud Rate: \n");
    uart_baud_rate = getdec_answer("\nBaudrate (0-115200, 1-9600):", 0, 0, 1);
    if ( slot == SHRINKRAY_SLOT1) {
        slot1_uart = uart_baud_rate;
    } else {
        slot2_uart = uart_baud_rate;
    }    
    
    new_baud = shrinkray_uart_baud[uart_baud_rate].baud_rate;

    /* Backup default config for recover after test */
    memcpy(&ori_conf, &newtio, sizeof(newtio));

    if ((newtio.c_cflag & CBAUD) != new_baud) {
        if (cfsetospeed(&newtio, new_baud) < 0) {
            tcsetattr(fd, TCSAFLUSH, &ori_conf);
            close(fd);
            cterr('f', 0, "Failed to set output speed.");
            return (FAILED);
        }

        if (cfsetispeed(&newtio, new_baud) < 0) {
            tcsetattr(fd, TCSAFLUSH, &ori_conf);
            close(fd);
            cterr('f', 0, "Failed to set intput speed.");
            return (FAILED);
        }
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_submodule_reset
 * Description: Function to reset ShrinkRay SubModule.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
shrinkray_submodule_reset (void)
{
    uint8_t data = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    printf("Starting to Reset Shrinkray SubModule... ");

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(io_exp, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
        cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* Based on ShinkRay HFS, GPIO Expander Definition,
     * GPIO[2] Alien Sub-module reset_l
     * 0 = Place alien sub-module in reset.
     * 1 = Release alien sub-module from reset.
     */
    data &= ~(SRINKRAY_SUBMOD_RESET);
    if (io_port_8bit_i2c_write(io_exp, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* delay time for processing, need check */
    msleep(500);

    data |= SRINKRAY_SUBMOD_RESET;
    if (io_port_8bit_i2c_write(io_exp, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* delay time for processing, need check */
    msleep(500);

    printf("Done.\n");
    printf("Reinitialize Shrinkray SM... ");

    if (shrinkray_sm_init()) {
        /* shrinkray_sm_cleanup(); */
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    printf("Done.\n\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_setup_uart
 * Description: Function to setup Shrinkray UART interface parameter.
 * Inputs     : NONE
 * Outputs    : NONE 
 *
 *******************************************************************************
 */
static void
shrinkray_setup_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd;
    struct termios oldtio, newtio;

    assert(shrinkray_iface_p); 

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", shrinkray_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = B115200|CS8|CLOCAL|CREAD;
    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    newtio.c_iflag = IGNPAR | ICRNL; 
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    close(fd);
    return;
}

/*******************************************************************************
 *
 * Function   : shrinkray_sm_cleanup
 * Description: This function perform the cleanup task before exiting
 *              the test.
 * Inputs     : NONE
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static void
shrinkray_sm_cleanup (void)
{
    shrinkray_ds_t     *shrinkray_info_p = &shrinkray_info;
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];

    int ge_port0, ge_port1, ge_port2;

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    data |= SRINKRAY_SUBMOD_RESET;
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    msleep(500);

    /* Mask host side slot interrupt */

    ge_port0 = ovld_get_ge_sw_port_num(shrinkray_info_p->slot, TGT_DEV_NGSM, 0);
    set_gesw_line_loopback(ge_port0, 0);
    ge_port1 = ovld_get_ge_sw_port_num(shrinkray_info_p->slot, TGT_DEV_NGSM, 1);
    set_gesw_line_loopback(ge_port1, 0);
    ge_port2 = ovld_get_ge_sw_port_num(shrinkray_info_p->slot, TGT_DEV_NGSM, 2);
    set_gesw_line_loopback(ge_port2, 0);

    if (savfcn) {
        savfcn = NULL;
    }

    if (shrinkray_saved_diag_exec) {
        pre_diag_exec = shrinkray_saved_diag_exec;
        shrinkray_saved_diag_exec = NULL;
    }
}

/*******************************************************************************
 *
 * Function   : shrinkray_sm_cpu0_console
 * Description: Function to redirect console to ShrinkRay SM CPU0.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int
shrinkray_sm_cpu0_console (int show_menu)
{
    shrinkray_ds_t   *iface;
    int slot;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;
    uint8_t data = 0;

    iface = shrinkray_iface_p;
    assert(iface);
    slot = shrinkray_iface_p->slot;

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
			           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); // pause a second for the NOTE:

    /* GPIO Bit4 UART Mux Select:
     * 0 = Connect the primary UART(CPU #0) to the host;
     * 1 = Connect the secondary UART(CPU #1) to the host.
     */
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }
    data &= ~(SRINKRAY_UART_MUX_SEL);

    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    if (slot == SHRINKRAY_SLOT1) {
        new_baud = slot1_uart;
    } else {
        new_baud = slot2_uart;
    }    

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d", 
             new_baud ? "b9600" : "b115200", shrinkray_iface_p->uart);
#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_sm_cpu1_console
 * Description: Function to redirect console to ShrinkRay SM CPU1.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int
shrinkray_sm_cpu1_console (int show_menu)
{
    shrinkray_ds_t   *iface;
    int slot;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;
    uint8_t data = 0;

    iface = shrinkray_iface_p;
    assert(iface);
    slot = shrinkray_iface_p->slot;

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
			           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); // pause a second for the NOTE:

    /* GPIO Bit4 UART Mux Select:
     * 0 = Connect the primary UART(CPU #0) to the host;
     * 1 = Connect the secondary UART(CPU #1) to the host.
     */
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }
    data |= SRINKRAY_UART_MUX_SEL;

    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    if (slot == SHRINKRAY_SLOT1) {
        new_baud = slot1_uart;
    } else {
        new_baud = slot2_uart;
    }    

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d",
             new_baud ? "b9600" : "b115200", shrinkray_iface_p->uart);
#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_host_utility
 * Description: Function to build Shrinkray SM host side utilities submenu.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
shrinkray_host_utility (int show_menu)
{
    menu(shrinkray_util_submenup, shrinkray_util_submenu_table, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_ioe_init
 * Description: Function to init Shrinkray I2C IO Expander.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
shrinkray_ioe_init (void)
{
    uchar io_port_conf0 = 0, io_port_conf1 = 0, io_port_output0 = 0;
    n2g_i2c_if_t *pca;
    uint8_t data = 0;
    pca = &pca_i2c[0];

    if (io_port_8bit_i2c_read(pca, PCA9555_CFG_PORT0_REG,
                              &io_port_conf0, TRUE)) {
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca, PCA9555_CFG_PORT1_REG,
                              &io_port_conf1, TRUE)) {
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG,
                              &io_port_output0, TRUE)) {
        return (FAILED);
    }

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n%s:%d, default io_port_conf0 = 0x%02X\n",
               __FUNCTION__, __LINE__, io_port_conf0);
        printf("%s:%d, default io_port_conf1 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_conf1);
        printf("%s:%d, default io_port_output0 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_output0);
    }

    /* Based on ShrinkRay HFS, we need to configure -
     * IO port 0.0, 0.3 as input(bit set to 1),
     * and IO port 0.1, 0.2, 0.4, 0.5, 1.0, 1.1 as output(bit set to 0).
     */
    io_port_conf0 &= (SRINKRAY_DB_PRESENT | SRINKRAY_PRI_INF_RDY);
    io_port_conf0 |= (SRINKRAY_DB_PRESENT | SRINKRAY_PRI_INF_RDY);
    io_port_conf1 &= (SRINKRAY_E0_10G_CAP | SRINKRAY_E1_10G_CAP);
    io_port_conf1 &= ~(SRINKRAY_E0_10G_CAP | SRINKRAY_E1_10G_CAP);

    io_port_output0 |= (SRINKRAY_SUBMOD_RESET | SRINKRAY_RESET_CONF);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n%s:%d, configured io_port_conf0 = 0x%02X\n",
               __FUNCTION__, __LINE__, io_port_conf0);
        printf("%s:%d, configured io_port_conf1 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_conf1);
        printf("%s:%d, configured io_port_output0 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_output0);
    }

    if (io_port_8bit_i2c_write(pca, PCA9555_CFG_PORT0_REG,
                               &io_port_conf0)) {
        return (FAILED);
    }

    if (io_port_8bit_i2c_write(pca, PCA9555_CFG_PORT1_REG,
                               &io_port_conf1)) {
        return (FAILED);
    }

    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG,
                               &io_port_output0)) {
        return (FAILED);
    }

    /* Reset and unreset
     * NOTE: in case to debug GE switch just comment out the reset and unreset
     */
    printf("Starting to Reset Shrinkray SubModule... ");

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
        cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* Based on ShinkRay HFS, GPIO Expander Definition,
     * GPIO[2] Alien Sub-module reset_l
     * 0 = Place alien sub-module in reset.
     * 1 = Release alien sub-module from reset.
     */
    data &= ~(SRINKRAY_SUBMOD_RESET);
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* delay time for processing, need check */
    msleep(500);

    data |= SRINKRAY_SUBMOD_RESET;
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* delay time for processing, need check */
    msleep(500);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_sm_init
 * Description: Function to init Shrinkray SM.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
shrinkray_sm_init (void)
{
    shrinkray_ds_t     *shrinkray_info_p;
    shrinkray_info_p = &shrinkray_info;
    int ge_port1, ge_port2;
    int real_slot = shrinkray_info_p->slot;

    /* Initial the GPIO */
    if (shrinkray_ioe_init() != PASSED) {
        cterr('f', 0, "%s: Unable to configure ShrinkRay IO Expander",
                      __FUNCTION__);
        return (FAILED);
    }

    /* setup the backplane loopback here. We will do the loopback test
       from Tile CPU */
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1);
    set_gesw_line_loopback(ge_port1, 1);
    ge_port2 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 2);
    set_gesw_line_loopback(ge_port2, 1);

    if (diagflag_xram & D_VERBOSE) {
        printf("\nSM slot %d, E1 : GESW port %d = %s",real_slot, ge_port1 ,
                get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
        printf("\nSM slot %d, E2 : GESW port %d = %s\n",real_slot, ge_port2 ,
                get_gesw_line_loopback(ge_port2) ? "ENABLE" : "DISABLE");
        fflush(0);
    }

    return (PASSED);
}


/**********************************************************************
 * Function: shrinkray_sm_test
 *
 * Description: This function is the main program of the Shrinkray test.
 *         This function will be called from platform_sm.h.
 *              Upon entering this function, the slot number will be
 *              checked to see if the user chose to execute all tests
 *              or entering the submenu and executes accordingly.
 *
 * Input:  The slot number of the Shrinkray SM
 *
 * Output: PASSED if all the tests PASSED
 *         FAILED if at least one of them FAILED
 *
 **********************************************************************
 */
int
shrinkray_sm_test (void *sm)
{
    int               real_slot, ret_val = PASSED;
    struct ngio_intf_t *shrinkray_sm_iface = (struct ngio_intf_t *)sm;
    shrinkray_ds_t     *shrinkray_info_p;
    ushort cookie_id = 0;
    shrinkray_info_p       = &shrinkray_info;

    real_slot = shrinkray_sm_iface->slot;
    cookie_id = shrinkray_sm_iface->id;
    printf ("\n SHRINKRAY is in slot %d, Cookie Id is %x \n", real_slot, cookie_id);

    shrinkray_test_slot = shrinkray_sm_iface->slot;
    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = shrinkray_sm_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    oir = (n2g_i2c_if_t *)shrinkray_sm_iface->oir;

    /*
     * Initialize an instance of Shrinkray data structure
     */
    shrinkray_iface_p = (shrinkray_ds_t *) &shrinkray_iface[real_slot];
    shrinkray_iface_p->cookie_id            = cookie_id;
    shrinkray_iface_p->slot                = real_slot;
    shrinkray_iface_p->uart                = shrinkray_sm_iface->uart_ctrl;
    shrinkray_iface_p->shrinkray_sm_iface = (struct ngio_intf_t *)sm;
    shrinkray_info_p->slot = real_slot;

   /*
     * Release Shrinkray SM out of reset.
     */
    /* uart/i2c unreset should be done via function pointer passed into the
       entry point */
    shrinkray_sm_iface->i2c_unreset(shrinkray_sm_iface);
    shrinkray_sm_iface->uart_on(shrinkray_sm_iface);
    shrinkray_sm_iface->unreset(shrinkray_sm_iface);
    msleep(1000);
    assert(sm);
    shrinkray_setup_uart();

    switch (cookie_id) {
    case SHRINKRAY_ID:
        sprintf((char *)shrinkray_iface_p->testname, "Slot%d Shrinkray SM", real_slot);
        break;
    default:
        cterr('f', 0, "Invalid Shrinkray cookie id %#.4x in slot %d", 
                                          cookie_id, real_slot);
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER) == FAILED) {
        return (FAILED);
    };

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY) == FAILED) {
        shrinkray_sm_cleanup();
        return (FAILED);
    };
    if (shrinkray_sm_init()) {
        shrinkray_sm_cleanup();
        return(FAILED);
    }
    build_primary_submenu(shrinkray_submenu_table, 
                          SHRINKRAY_SUBMENU_TABLE_SIZE,
                          (char *)shrinkray_sm_iface->name,
                          (menuinfo_t **)&shrinkray_submenup);
    build_secondary_submenu(shrinkray_submenu_table,
                            SHRINKRAY_SUBMENU_TABLE_SIZE,
                            shrinkray_tests_secondary_items);

    /*
    * pm_subtest_menu now built.  Display and interact with user until
    * <ESC><RET> back to main menu.
    *
    * To prevent freeing up allocated memory prematurely,
    * save the pre_diag_exec function and set it to NULL.
    * This will prevent menu() marking the needed memory freed.
    */
    shrinkray_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    if (shrinkray_sm_iface->menu_display == TRUE) {
        menu(shrinkray_submenup, shrinkray_tests_secondary_items, '\0' );
    } else {
        if (shrinkray_sm_iface->test_type == IFACE_TEST) {
            ret_val = oir_ltc4215_register_test(oir);
            ret_val |= shrinkray_ioe_reg_test();        
        } else {  /* FULL_TEST */    
            do_all_menu_items(shrinkray_submenup);
        }
    }

    ret_val |= util_oir_ltc4215_led(oir, OIR_LED_OFF);

    shrinkray_sm_cleanup();

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : act2lite_tests
 * Description: Entry function of ShinkRay ACT2 Lite Diag tests and utilities.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
act2lite_tests (int show_menu)
{
    shrinkray_ds_t *shrinkray_info_p = &shrinkray_info;
    uchar          cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    int dummy = 0; 
 
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    testname("ACT2 Lite");

    if (plat_init_smart_eeprom_context(con,
                                       SM_MODULE,
                                       shrinkray_info_p->slot,
                                       cookie) == FAILED) {
        cterr('f', 0, "%s: Failed to init smart EEPROM context.", __FUNCTION__);
        return (FAILED);
    }
    
    act2_init_cont((void*)con);

    /* is_act2() is obeseleted, using act2_version() on tam_lib.c */
    if (act2_version(dummy)) { 
        act2_prog(0);
    } else {
        cterr('f', 0, "%s: This is not ACT2.", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : shrinkray_ioe_reg_test
 * Description: Wrapped function to do ShrinkRay IO Expander register test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
shrinkray_ioe_reg_test (void)
{
    uint32_t         ctr = 0, test_ctr = 0, total_reg_num = 0;
    uchar            orig_val = 0, test_data = 0, check_data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    reg_p = &pca9555_reg_tbl[0];
    total_reg_num = (sizeof(pca9555_reg_tbl) / sizeof(reg_info_t));

    testname("ShrinkRay IO Expander Registers");

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        /* Skip Input port registers & Output port registers
         * Based on PCA9555 datasheet, Input port registers are input-only,
         * writes to these registers have no effect.
         * And skip Output port registers to avoid to change the system set-ups.
         * Like cause ShrinkRay alien sub-module be put in reset(GPIO[2] = 0).
         */
        if ((reg_p->offset == PCA9555_IN_PORT0_REG) ||
            (reg_p->offset == PCA9555_IN_PORT1_REG) ||
            (reg_p->offset == PCA9555_OUT_PORT0_REG) ||
            (reg_p->offset == PCA9555_OUT_PORT1_REG)) {
            continue;
        }

        if ((reg_p->type & SAVE_RESTORE) == SAVE_RESTORE) {
            /* Backup Original value */
            if (io_port_8bit_i2c_read(io_exp, ctr, &orig_val, TRUE)) {
                cterr('f', 0, "%s: Failed to read IO Expander Reg %#x"
                              " as restore value.",
                              __FUNCTION__, reg_p->offset);
                return (FAILED);
            }

            /*
             * Ripple 1 test
             */
            prpass(testpass, "Ripple 1 ");
            for (test_ctr = 0; test_ctr < (sizeof(test_data) * 8); test_ctr++) {
                test_data = ((1 << test_ctr) & reg_p->mask);
                if (!test_data) {
                    continue;
                }

                /* Write Test Data in */
                if (io_port_8bit_i2c_write(io_exp, ctr, &test_data)) {
                    cterr('f', 0, "%s: Failed to wrote 0x%02X "
                                  "to IO Expander Reg. %#x in Ripple 1 test.",
                                  __FUNCTION__, test_data, reg_p->offset);
                    return (FAILED);
                }

                /* Read the register value back for double check */
                if (io_port_8bit_i2c_read(io_exp, ctr, &check_data, TRUE)) {
                    cterr('f', 0, "%s: Failed to read IO Expander Reg. %#x "
                                  "in Ripple 1 test",
                                  __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple 1 test FAILED, "
                                  "read back = 0x%02X and expected = 0x%02X.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 1 Test */

            check_data = 0;
            test_data = 0;

            /*
             * Ripple 0 test
             */
            prpass(testpass, "Ripple 0 ");
            for (test_ctr = 0; test_ctr < (sizeof(test_data) * 8); test_ctr++) {
                test_data = (1 << test_ctr);
                if (!test_data) {
                    continue;
                }

                test_data = ((uchar)(~(1 << test_ctr)) & reg_p->mask);

                /* Write Test Data in */
                if (io_port_8bit_i2c_write(io_exp, ctr, &test_data)) {
                    cterr('f', 0, "%s: Failed to wrote 0x%02X "
                                  "to IO Expander Reg. %#x in Ripple 0 test.",
                                  __FUNCTION__, test_data, reg_p->offset);
                    return (FAILED);
                }

                /* Read the register value back for double check */
                if (io_port_8bit_i2c_read(io_exp, ctr, &check_data, TRUE)) {
                    cterr('f', 0, "%s: Failed to read IO Expander Reg. %#x "
                                  "in Ripple 0 test.",
                          __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple 0 test FAILED, "
                                  "read back = 0x%02X and expected = 0x%02X.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 0 Test */

            /* Restore the value before test */
            if (io_port_8bit_i2c_write(io_exp, ctr, &orig_val)) {
                cterr('f', 0, "%s: Failed to write the restore value 0x%02X "
                              "back to IO Expander Reg. %#x.",
                              __FUNCTION__, test_data, reg_p->offset);
                return (FAILED);
            }
        }
    }
    prpass(testpass, "Done ");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_shrinkray_hipwr
 * Description: Function to Enable/Disable ShrinkRay High Power mode.
 * Inputs     : opt - Enable/Disable
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
set_shrinkray_hipwr (boolean opt)
{
    uint8_t data = 0;

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("%s: Failed to read LTC4215 register(0x%02X).\n",
               __FUNCTION__, LTC4215_FAULT_REG);
        return (FAILED);
    }

    if (opt == ENABLE) {
        data |= LTC4215_GPIO3_OUTPUT;
    } else {
        data &= (uint8_t)(~LTC4215_GPIO3_OUTPUT);
    }

    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
        printf("%s: Failed to write 0x%01X to LTC4215 register(0x%02X).\n",
               __FUNCTION__, data, LTC4215_FAULT_REG);
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : shrinkray_hipwr_ctrl_util
 * Description: Wrapped utility to control ShrinkRay High Power mode.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
shrinkray_hipwr_ctrl_util (int opt)
{
    uint8_t data = 0;

    printf("Starting to %s ShrinkRay High Power mode... ",
           (opt == ENABLE) ? "Enalbe" : "Disable");

    if (set_shrinkray_hipwr(opt) != PASSED) {
        printf("\n%s: Failed to %s ShrinkRay High Power mode.\n",
               __FUNCTION__, (opt == ENABLE) ? "Enable" : "Disable");
        return (FAILED);
    }

    printf("Done.\n");

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("%s: Failed to read LTC4215 register(0x%02X).\n",
               __FUNCTION__, LTC4215_FAULT_REG);
        return (FAILED);
    }

    printf("ShrinkRay High Power mode is %s now.\n",
           (data & LTC4215_GPIO3_OUTPUT)? "Enabled" : "Disabled");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9555_reg_dump_util
 * Description: Wrap utility to dump all registers of ShrinkRay
 *              IO Expander(PCA9555).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
pca9555_reg_dump_util (void)
{
    uint32_t         ctr = 0, total_reg_num = 0;
    uchar            data = 0, reg_data[8];
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    memset((uchar *)reg_data, 0, (sizeof(reg_data)/sizeof(uchar))); 
    total_reg_num = (sizeof(pca9555_reg_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++) {
        data = 0;

        if (io_port_8bit_i2c_read(io_exp, ctr, &data, TRUE)) {
            printf("\n\nFailed to read IO Expander(PCA9555)"
                   " register 0x%02X.\n\n", ctr);
            return (FAILED);
        }
        reg_data[ctr] = data;
    }

    printf("\nShrinkRay IO Expander(PCA9555) registers dump:\n");
    for (ctr = 0; ctr < total_reg_num; ctr++) {
        reg_p = &pca9555_reg_tbl[ctr];
        printf("%-25s Reg.(0x%01X) = 0x%02X.\n",
               reg_p->name, reg_p->offset, reg_data[ctr]);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9555_reg_read_util
 * Description: Wrap utility to read specific register of ShrinkRay
 *              IO Expander(PCA9555).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
pca9555_reg_read_util (void)
{
    uint32_t         offset = 0;
    uchar            data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    offset = (uint32_t)gethex_answer("Enter offset of register:", 0, 0, 0x7);

    if (io_port_8bit_i2c_read(io_exp, offset, &data, TRUE)) {
        printf("\n\nFailed to read IO Expander(PCA9555) register 0x%02X.\n\n",
               offset);
        return (FAILED);
    }

    reg_p = &pca9555_reg_tbl[offset];
    printf("\nIO Expander(PCA9555) %s Reg.(0x%01X): 0x%02X.\n\n",
           reg_p->name, reg_p->offset, data);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9555_reg_write_util
 * Description: Wrap utility to write specific register of ShrinkRay
 *              IO Expander(PCA9555).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
pca9555_reg_write_util (void)
{
    uint32_t         offset = 0;
    uchar            data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    offset = (uint32_t)gethex_answer("Enter offset of register:", 0, 0, 0x7);

    reg_p = &pca9555_reg_tbl[offset];

    if ((reg_p->offset == PCA9555_IN_PORT0_REG) ||
        (reg_p->offset == PCA9555_IN_PORT1_REG)) {
        printf("\n\n Based on PCA955 datasheet, %s Reg.(0x%01X) is an input-only"
               " port, writes to this register have no effect.\n\n",
               reg_p->name, offset);
        return (PASSED);
    }

    data = (uchar)gethex_answer("Enter write-in Data:", 0, 0, 0xFF);

    if (io_port_8bit_i2c_write(io_exp, offset, &data)) {
        printf("\n\nFailed to write 0x%02X to IO Expander(PCA9555)"
               " register 0x%02X.\n\n", data, offset);
        return (FAILED);
    }

    printf("\nDone write 0x%02X to IO Expander(PCA9555) %s Reg.(0x%01X).\n\n",
           data, reg_p->name, reg_p->offset);

    return (PASSED);
}

/*------------------------------------------------------------------
 * $Log: shrinkray_host.c,v $
 * Revision 1.6  2019/08/06 06:56:06  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.5.2.1  2018/09/27 09:46:23  alpeng
 * support tam lib and aikido for curie
 *
 * Revision 1.5  2018/05/22 02:31:11  alpeng
 * fixed compiler warning, CSCvj57934
 *
 * Revision 1.4  2018/05/18 09:24:48  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.3  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2.48.2  2018/05/17 10:50:20  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.2.48.1  2016/12/05 06:36:59  alpeng
 * fixed the uart ctrl num for ngio; change is approved on prrq
 *
 * Revision 1.3  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2  2014/03/03 06:33:51  palin2
 * -Initial check-in ShrinkRay host side Diag.
 *
 * Revision 1.1.4.4  2014/02/28 08:31:55  steja
 * Fix the GE Backplane loopback fail
 *
 * Revision 1.1.4.2  2014/02/27 00:13:31  palin2
 * Sync up name of Sub-Module reset utility with ShrinkRay G2 Host side.
 *
 * Revision 1.1.4.1  2014/02/26 11:08:59  palin2
 * -To support ShrinkRay host side tests on O2.
 * -This branch is created to pick up O2 main tunk code changes.
 *
 * Revision 1.1.2.16  2014/02/25 04:15:33  steja
 * 1. Fix Reset and unreset shrinkray module
 * 2. Add setup Backplane loopback in init steps
 *
 * Revision 1.1.2.15  2014/01/27 08:51:07  steja
 * Code clean up
 *
 * Revision 1.1.2.14  2014/01/27 06:03:57  palin2
 * Add message for ShirnkRay IO expander registers test based on review comment.
 *
 * Revision 1.1.2.13  2013/12/24 11:59:16  iachang
 * Get Shrinkray SM Current
 *
 * Revision 1.1.2.12  2013/12/24 11:46:04  iachang
 * Setup ShrinkRay uart baud rate
 *
 * Revision 1.1.2.11  2013/10/15 02:47:46  steja
 * Support Shrinkray Utility to set platform GE/XAUI Switch line loopback setup
 *
 * Revision 1.1.2.10  2013/10/11 02:39:38  palin2
 * 1. Update ShrinkRay IO Expander init function.
 * 2. Fix ShrinkRay Sub-Module Reset utility.
 *
 * Revision 1.1.2.9  2013/10/10 19:55:17  palin2
 * 1. Code update after bring up ShrinkRay Host side I2C IO Expander.
 * 2. Simple code clean up.
 *
 * Revision 1.1.2.8  2013/09/24 00:36:12  palin2
 * Updated ShrinkRay IO expander init function.
 *
 * Revision 1.1.2.7  2013/08/17 03:27:00  steja
 * add code command and respond ( Host <->GE <-> TILE CPU#0) for O2 platform
 *
 * Revision 1.1.2.6  2013/08/13 10:03:50  palin2
 * Add ShrinkRay High Power mode control utility support.
 *
 * Revision 1.1.2.5  2013/07/24 02:26:39  iachang
 * Support Console Switch with CPU0 & CPU1
 *
 * Revision 1.1.2.4  2013/07/08 10:12:04  steja
 * 1. Fix test name for GE loopback test
 * 2. Add dummy function for Szalinski FPGA test
 *
 * Revision 1.1.2.3  2013/07/08 08:49:16  steja
 * Add GE backplane loopback test (Host <->TLK10232)
 *
 * Revision 1.1.2.2  2013/05/30 00:38:06  palin2
 * Add OIR tests/utilities, ACT2 Lite tests/utilities support.
 *
 * Revision 1.1.2.1  2013/05/22 02:44:02  palin2
 * Initial check-in to add ShrinkRay host side Diag support on O2.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

