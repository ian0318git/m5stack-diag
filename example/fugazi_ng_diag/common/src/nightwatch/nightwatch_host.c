/* $Id: nightwatch_host.c,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nightwatch/nightwatch_host.c,v $
 *------------------------------------------------------------------
 *
 * nightwatch_host.c: main source file for Nightwatch host diag.
 *
 * May. 2018 - Mingchun Ding (Ported from Lebowski)
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Ian Chang
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
#include <pthread.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "strings.h"
#include "signals.h"
#include "nvmonvars.h"
#include "linux_api.h"
#include "proto.h"
#include "error.h"
#include "free.h"
#include "cpu.h"
#include "menu.h"
#include "pm_utils.h"
#include "queryflags.h"
#include "mon_plat_defs.h"
#include "slot.h"
#include "sm_slot.h"
#include "cross_platform.h"
#include "linux_pci.h"
#include "dev_object.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "cookie_4.h"
#include "nightwatch_host.h"
#include "sgmii_defs.h"
#include "plat_defs.h"
#include "ethernet.h"
#include "platform_i2c.h"
#include "pca.h"
#include <assert.h>
#include "platform_poe_psu.h"
#include "dash_fpga.h"
#include "bcm_gesw_defs.h"
#include "nightwatch_diag_shell_api.h"
#if defined(STRESS_NGSM_ETH_TRAFFIC)
#include "eth_traf.h"
#endif
/*------------------------------------------------------------------------------
 * globals
 *------------------------------------------------------------------------------
 */
static void (*nightwatch_saved_diag_exec)(void) = NULL;
static int (*savfcn)() = NULL;
static nightwatch_ds_t nightwatch_info;
static nightwatch_ds_t *nightwatch_iface_p;
nightwatch_ds_t nightwatch_iface[MAX_SM+1];
static speed_t slot_uart[MAX_SM] = {
    NIGHTWATCH_B9600,NIGHTWATCH_B9600,
    NIGHTWATCH_B9600,NIGHTWATCH_B9600
};

struct _ngio_pcie_snapshot_ {
    struct pci_dev *root;
    int bus, sec, sub, cnt;
    struct pci_snapshot *p[256];
};

static struct _ngio_pcie_snapshot_  ngio_pcie_snapshot={
    .root = NULL,
    .bus  = 0,
    .sec  = 0,
    .sub  = 0,
    .cnt  = 0,
    .p    = { 0 }
};
/*------------------------------------------------------------------------------
 * prototypes
 *------------------------------------------------------------------------------
 */
static int  util_nightwatch_sm_reset(void);
static int  util_nightwatch_sm_power_off(void);
static int  util_nightwatch_sm_pwr_off(void);
static int  util_nightwatch_sm_pwr_on(void);
static int  util_nightwatch_sm_pwr_cycle(void);
static int  util_nightwatch_uart_baud_rate_set(void);
static int  nightwatch_mc_reset(void);
static void nightwatch_sm_cleanup(void);
static int  nightwatch_sm_dopplerG_console_switch(int);
static int  nightwatch_sm_console_switch(int);
static int  nightwatch_sm_console_switch2(int);
static int  nightwatch_host_utility(int);
static int  nightwatch_nightwatch_sm_test(int);
static int  nightwatch_nightwatch_sm_test2(int);
static int  nightwatch_sm_init(void);
static int  ltc4215_register_test(void);
static int  ltc4215_led_test(void);
static int  ltc4215_oir_test(int);
static int  util_ltc4215_reg_read(void);
static int  util_ltc4215_reg_write(void);
static int  util_nightwatch_sm_disp_pwr(void);
static uint32_t nightwatch_sm_get_current(uint8_t data);
static long nightwatch_configure_i2c_expander(void);
static int nightwatch_i2c_ioe_reg_test(void);
static int setup_uart(void);
static int ngiosm_pcie_cfg_restore(struct _ngio_pcie_snapshot_  *);
static int ngiosm_pci_config_check(struct ngio_intf_t *);
static int nightwatch_loopup_bus_dev_fn(struct ngio_intf_t *, uint32_t *);
static int nightwatch_port_tx_util(int);
static int nightwatch_port_tx_util2(int);
long nightwatch_boot_image(int);

static int nightwatch_sm_lock_flash(int );
#if defined(BCM_ESW_SUPPORT)
static int  nightwatch_switch_gesw_port_mode(int);
static int  util_nightwatch_overlord_gesw(void);
extern int  invoke_bcm_shell(void);
extern void show_gesw_port_assign(void);
extern int  exec_bcm_shell_cmd (int, char *, int);
#endif

/*------------------------------------------------------------------------------
 * constants
 *------------------------------------------------------------------------------
 */
static uart_baud_info nightwatch_uart_baud[] = {
    {"115200",   B115200},
    {"9600",     B9600}
};


/*
 * Sub Menu used for Utility.
 */
static mitem_t nightwatch_util_submenu_table[] = {
    { "LTC4215 Register Read",            0, 0,   (PFT)util_ltc4215_reg_read,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "LTC4215 Register Write",           0, 0,   (PFT)util_ltc4215_reg_write,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "Display Power Meassure",           0, 0,   (PFT)util_nightwatch_sm_disp_pwr,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "Nightwatch SM Switch Reset",       0, 0,   (PFT)util_nightwatch_sm_reset,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "Nightwatch SM Switch Power Off",   0, 0,   (PFT)util_nightwatch_sm_power_off,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "Nightwatch SM Switch Power On",    0, 0,   (PFT)util_nightwatch_sm_pwr_on,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "Nightwatch SM Switch Power Cycle", 0, 0,   (PFT)util_nightwatch_sm_pwr_cycle,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
#if defined(BCM_ESW_SUPPORT)
    { "Nightwatch Switch GESW",  0, 0,            (PFT)util_nightwatch_overlord_gesw,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
    { "Invoke BCM shell",                 0, 0,   (PFT)invoke_bcm_shell,
        (type_t *)&zero, 0, (type_t(*)())0,   0},
    { "Show GESW port assignment",        0, 0,   (PFT)show_gesw_port_assign,
        (type_t *)&zero, 0, (type_t(*)())0,   0},
    { "Switch GESW port mode",        0, 0,       (PFT)nightwatch_switch_gesw_port_mode,
        (type_t *)&zero, 0, (type_t(*)())0,   0},
#endif
    { "Nightwatch UART Baud Rate Set",    0, 0,   (PFT)util_nightwatch_uart_baud_rate_set,
        (type_t *)&zero, 0, NULL_FUNC, 0 },
};

#define NIGHTWATCH_UTIL_SUBMENU_TABLE_SZ \
    (sizeof(nightwatch_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t nightwatch_util_subtest_menu = {
    "Nightwatch Host Utilities Menu",
    (type_t)0,                                        /* title param */
    (PFT)menu_show_dflags,                         /* show diag flags */
    0,
    NIGHTWATCH_UTIL_SUBMENU_TABLE_SZ,
    nightwatch_util_submenu_table,
};

static menuinfo_t *nightwatch_util_submenup = &nightwatch_util_subtest_menu;

/*
 * Sub Menu used for OIR LTC4215 tests.
 */
static submenu_xtable_t oir_submenu_table[] = {
    {"OIR (LTC4215) Register Test",     (PFT)ltc4215_register_test,  0,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
        NULL_FUNC,   0,      NULL_FUNC,   0},
    {"OIR LED Test",                    (PFT)ltc4215_led_test,       0,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
        NULL_FUNC,   0,      NULL_FUNC,   0},
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
 * Sub Menu used for Nightwatch Host side tests.
 */
static submenu_xtable_t nightwatch_submenu_table[] = {
    {"OIR (LTC4215) Test",        (PFT)ltc4215_oir_test,  0,
        MF_CONTINUOUS | MF_DOALL,
        NULL_FUNC, 0,             (PFT)ltc4215_oir_test,  TRUE},
    {"Nightwatch SM Test",        (PFT)nightwatch_nightwatch_sm_test,  NWK_DFLAG_AUTO_RUN,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
        NULL_FUNC, 0,             (PFT)nightwatch_nightwatch_sm_test2, 0},
    {"Nightwatch Host Utilities", (PFT)nightwatch_host_utility, 0,
        0,     NULL_FUNC, 0,      (PFT)nightwatch_host_utility, 0},
    {"Nightwatch DopplerG Console",   (PFT)nightwatch_sm_dopplerG_console_switch, 0,
        0,     NULL_FUNC, 0,       NULL_FUNC, 0},
    {"Nightwatch Switch Console", (PFT)nightwatch_sm_console_switch, 0,
        0,     NULL_FUNC, 0,      (PFT)nightwatch_sm_console_switch2, 0},
    {"BP GE/XE Ports Test",       (PFT)nightwatch_port_tx_util, 0,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
        NULL_FUNC, 0,             (PFT)nightwatch_port_tx_util2, 0},
    {"Firmware Download ",        (PFT)nightwatch_boot_image, 0,
        0,     NULL_FUNC, 0,       NULL_FUNC, 0},
    {"Lock Sboot Golden Image ", (PFT)nightwatch_sm_lock_flash, 0,
        0,     NULL_FUNC, 0,      NULL_FUNC, 0},

};

#define NIGHTWATCH_SUBMENU_TABLE_SIZE (sizeof(nightwatch_submenu_table) / \
        sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t nightwatch_tests_primary_items[NIGHTWATCH_SUBMENU_TABLE_SIZE +
MAX_BASE_ITEMS];
static mitem_t nightwatch_tests_secondary_items[NIGHTWATCH_SUBMENU_TABLE_SIZE +
MAX_BASE_ITEMS];

static menuinfo_t nightwatch_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    nightwatch_tests_primary_items,
};

static menuinfo_t *nightwatch_submenup = &nightwatch_subtest_menu;

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

/**********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_register_test (void)
{
    testname("LTC4215 OIR Register");

    return(oir_ltc4215_register_test(oir));
}

/**********************************************************************
 *
 * Function: ltc4215_led_test
 *
 * Description: A wrapper function for LTC4215 LED test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_led_test (void)
{
    testname("LTC4215 OIR LED");

    return(oir_ltc4215_leds_test(oir));
}

/*------------------------------------------------------------------------------
 *
 * Function: ltc4215_oir_test().
 *
 * This function implements the ltc4215 oir test/menu for main menu.
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int ltc4215_oir_test (int show_menu)
{
    build_primary_submenu(oir_submenu_table,
            OIR_SUBMENU_TABLE_SIZE,
            "LTC4215 OIR", &oir_submenup);
    build_secondary_submenu(oir_submenu_table,
            OIR_SUBMENU_TABLE_SIZE,
            oir_tests_secondary_items);

    if (show_menu) {
        menu(oir_submenup, oir_tests_secondary_items, '\0' );
    } else {
        do_all_menu_items(oir_submenup);
    }

    testname("LTC4215 OIR");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_ltc4215_reg_write (void)
{
    return(util_oir_ltc4215_reg_write(oir));
}

/**********************************************************************
 *
 * Function: util_ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_ltc4215_reg_read (void)
{
    return(util_oir_ltc4215_reg_read((void *)oir));
}

/**********************************************************************
 *
 * Function: util_nightwatch_sm_disp_pwr
 *
 * Description: Display power of SM.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_sm_disp_pwr (void)
{
    uint32_t voltage, current, power;
    uint8_t data = 0;

    printf("\n\nNightwatch SM Power Measure:\n\n");

    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data)) {
        return (FAILED);
    }
    voltage = (data * SINGLE_SM_VOL) / 100;

    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data)) {
        return (FAILED);
    }
    current = nightwatch_sm_get_current(data) / 100;

    power = voltage * current;

    printf("Voltage = %d.%02d V\n", (voltage / 100), (voltage % 100));
    printf("Current = %d.%02d A\n", (current / 100), (current % 100));
    printf("Power = %d.%02d W\n", (power / 10000), ((power % 10000) / 100));

    return (PASSED);
}

/**********************************************************************
 *
 * Function: nightwatch_sm_get_current
 *
 * Description: convert sense register value into current.
 *
 * Input : Sense Register value.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static uint32_t nightwatch_sm_get_current (uint8_t data)
{
    nightwatch_ds_t *nightwatch_info_p = &nightwatch_info;
    ushort cookie_id = nightwatch_info_p->cookie_id;
    uint32_t current;

    if (data) {
        switch (cookie_id) {
            case NIGHTWATCH_48P_ID:
                current = (data - 1) * DWIDE_SM_CURRENT;
                break;
            default:
                current = (data - 1) * SINGLE_SM_CURRENT;
        }
    } else {
        current = 0;
    }

    return (current);
}

/**********************************************************************
 *
 * Function: util_nightwatch_sm_power_off
 *
 * Description: This function is a wrapper of power off Nightwatch SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_sm_power_off (void)
{
    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Nightwatch SM Switch Still Powered On.\n\n");
        return (PASSED);
    }

    return (util_nightwatch_sm_pwr_off());
}

/**********************************************************************
 *
 * Function: util_nightwatch_sm_pwr_off
 *
 * Description: This function power off Nightwatch SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_sm_pwr_off (void)
{
    uint8_t data = 0;
    nightwatch_ds_t *nightwatch_info_p = &nightwatch_info;

    printf("\nPower Off the Nightwatch SM.\n");

    /* disable power fault interrupt */
    ngiosm_disable_intr(nightwatch_info_p->slot, NGIO_FLT_INTR);

    if (util_oir_ltc4215_led(oir, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power off sm module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    printf("\nNightwatch SM is turned off.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_nightwatch_sm_pwr_on
 *
 * Description: This function power on Nightwatch SM.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_sm_pwr_on (void)
{
    uint8_t data = 0;
    struct ngio_intf_t *nwk_sm_iface = (struct ngio_intf_t *)nightwatch_iface_p->ngio_iface;

    printf("\nPower On the Nightwatch SM.\n");

    assert(oir);
    assert(nightwatch_iface_p);

    /* turn on board power and take I2C out of reset */
    if (slot_i2c_unreset(nwk_sm_iface , nwk_sm_iface->slot, "SM")) {
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power on sm module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        return (FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return (FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return (FAILED);
    }

    printf("Waiting for Nightwatch SM to Power-Up.\n");
    /* Release SM from Reset state */
    nwk_sm_iface->unreset(nwk_sm_iface);

    if (nightwatch_sm_init()) {
        nightwatch_sm_cleanup();
        return (FAILED);
    }

    /* turn on the green light status if PSE2 re-init successfully */
    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    printf("\nNightwatch SM is turned on.\n");

    /* disable power fault interrupt */
    ngiosm_enable_intr(nightwatch_iface_p->slot, NGIO_FLT_INTR);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_nightwatch_sm_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_sm_pwr_cycle (void)
{
    uint8_t ix, ans;

    printf("\n");
    testname("Power Cycle the Nightwatch SM");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Nightwatch SM Switch is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (util_nightwatch_sm_pwr_off()) {
        cterr('f', 0, "Failed on Power Off the Nightwatch SM");
        return (FAILED);
    }

    /* msleep for 10 seconds. */
    for (ix = 0; ix < NIGHTWATCH_POWER_CYCLE_SECOND; ix++) {
        printf(".");
        msleep(1000);
    }
    printf("\n");

    if (util_nightwatch_sm_pwr_on()) {
        cterr('f', 0, "Failed on Power On the Nightwatch SM");
        return (FAILED);
    }

    return (PASSED);
}

#if defined(BCM_ESW_SUPPORT)
/**********************************************************************
 *
 * Function: util_nightwatch_overlord_gesw
 *
 * Description: A wrapper function for GE ports loopback setting.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_overlord_gesw (void)
{
    uint8_t ans;
    uint    port;
    int     ge_port, ge_port0, ge_port1;
    nightwatch_ds_t     *nightwatch_info_p;
    nightwatch_info_p       = &nightwatch_info;
    int real_slot = nightwatch_info_p->slot;

    printf("\n");
    testname("Switch the Overlord GESW");

    if (GE0_PNUM == XAUI_PNUM) {
        port = gethex_answer("GESW port GE1/XAUI", GE0_PNUM, GE1_PNUM, GE0_PNUM);
    } else {
        port = gethex_answer("GESW port GE0/GE1",  GE0_PNUM, GE0_PNUM, GE1_PNUM);
    }
    ge_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, port);

    printf("\nEnable the backplane loopback? (y/N) ");
    fflush(0);
    ans = getchar();
    if (ans != 'y' && ans != 'Y') {
        set_gesw_line_loopback(ge_port, FALSE);
    } else {
        set_gesw_line_loopback(ge_port, TRUE);
    }
    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, GE0_PNUM);
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, GE1_PNUM);
    printf("\nSM slot %d, %s : GESW port %d Line Loopback = %s",real_slot,
            GE0_PNUM ? "XAUI" : "GE0",
            ge_port0, get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
    printf("\nSM slot %d, %s : GESW port %d Line Loopback = %s\n",real_slot,
            "GE1",
            ge_port1, get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
    fflush(0);

    return (PASSED);
}
#endif

/*------------------------------------------------------------------------------
 *
 * Function: util_nightwatch_uart_baud_rate_set().
 *
 * This function sets the Yeti3 uart baud rate
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int util_nightwatch_uart_baud_rate_set (void)
{
    const   int maxlen = 128;
    char    tty[maxlen];
    int     fd, slot;
    struct  termios  newtio, ori_conf;
    nightwatch_ds_t   *iface;
    speed_t new_baud = 0, uart_baud_rate = 0;

    iface = nightwatch_iface_p;
    assert(iface);
    slot = nightwatch_iface_p->slot;
    assert((0 < slot) && (slot < MAX_SM));

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",slot-1);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
        perror(tty);
        exit(1);
    }
    tcgetattr(fd, &newtio);

    printf("\n\n Set Yeti3 UART Baud Rate: \n");
    uart_baud_rate = getdec_answer("\nBaudrate (0-115200, 1-9600):", 0, 0, 1);
    slot_uart[slot-1] = uart_baud_rate;

    new_baud = nightwatch_uart_baud[uart_baud_rate].baud_rate;

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
/**********************************************************************
 *
 * Function: util_nightwatch_sm_reset
 *
 * Description: This function resets Nightwatch SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int util_nightwatch_sm_reset (void)
{

    printf("\n\nNightwatch SM Reset.\n\n");

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    nightwatch_mc_reset();

    printf("Reinitialize Nightwatch SM.\n\n");

    if (nightwatch_sm_init()) {
        nightwatch_sm_cleanup();
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: nightwatch_mc_reset
 *
 * Description: This function resets Nightwatch Microcontroller.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
static int nightwatch_mc_reset (void)
{
    struct ngio_intf_t *nwk_sm_iface = (struct ngio_intf_t *)nightwatch_iface_p->ngio_iface;

    /* Pull reset then release the reset*/
    nwk_sm_iface->reset(nwk_sm_iface);

    msleep(500);

    nwk_sm_iface->unreset(nwk_sm_iface);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: setup_uart()
 *
 * Description: Setup UART Interface Parameter
 *
 * Input:  None
 *
 * Output: PASSED/FAILED.
 *
 *****************************************************************
 */
static int setup_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd, slot;
    struct termios oldtio, newtio;
    nightwatch_ds_t   *iface;
    int new_baud = 0;

    iface = nightwatch_iface_p;
    assert(iface);
    slot = nightwatch_iface_p->slot;
    assert((0 < slot) && (slot < MAX_SM));

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",slot-1);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
        perror(tty);
        exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    /* Get current BAUD setting */
    if (0 < slot && slot <= MAX_SM) {
        new_baud = slot_uart[slot-1];
    } else {
        cterr('f',0,"Invalid slot number: %d.", slot);
        close(fd);
        return (FAILED);
    }

    if ( new_baud == NIGHTWATCH_B115200) {
        newtio.c_cflag = B115200|CS8|CLOCAL|CREAD; /* control mode flags */
    } else if (new_baud == NIGHTWATCH_B9600) {
        newtio.c_cflag = B9600|CS8|CLOCAL|CREAD;   /* control mode flags */
    } else {
        cterr('f',0,"Invalid Baud: %d.", new_baud);
        close(fd);
        return (FAILED);
    }

    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    newtio.c_iflag = IGNPAR | ICRNL;  /* input mode flags  */
    newtio.c_oflag = 0;               /* output mode flags */
    newtio.c_lflag = ICANON;          /* local mode flags  */
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    close(fd);
    return (PASSED);
}

/**********************************************************************
 * Function: nightwatch_sm_cleanup()
 *
 * Description: This function perform the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void nightwatch_sm_cleanup (void)
{
    nightwatch_ds_t     *nightwatch_info_p = &nightwatch_info;
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    struct _nightwatch_shell_config_t *nwk_diag_shell_cfg = nwk_get_shell_cfg();
    char **nwk_cmd_exit = nwk_diag_shell_cfg->nwk_cmd_exit;

#if defined(BCM_ESW_SUPPORT)
    int ge_port0, ge_port1;
#endif

    exec_nwk_shell_cmd(0, nwk_cmd_exit, TRUE, TRUE);

    if (ngio_pcie_snapshot.cnt > 0) {
        ngiosm_pcie_cfg_restore(&ngio_pcie_snapshot);
    }

    if (nightwatch_info_p->ngio_iface->pci_rdy) {
        nightwatch_info_p->ngio_iface->pci_rdy(nightwatch_info_p->ngio_iface, FALSE);
    }

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
        cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    data |= ((1 << PCA9555_SYS_RET_REGISTER));
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    msleep(500);

#if defined(BCM_ESW_SUPPORT)
    ge_port0 = ovld_get_ge_sw_port_num(nightwatch_info_p->slot, TGT_DEV_NGSM, GE0_PNUM);
    set_gesw_line_loopback(ge_port0, FALSE);
    ge_port1 = ovld_get_ge_sw_port_num(nightwatch_info_p->slot, TGT_DEV_NGSM, GE1_PNUM);
    set_gesw_line_loopback(ge_port1, FALSE);
#endif

    if (savfcn) {
        savfcn = NULL;
    }

    if (nightwatch_saved_diag_exec) {
        pre_diag_exec = nightwatch_saved_diag_exec;
        nightwatch_saved_diag_exec = NULL;
    }
}

/*------------------------------------------------------------------------------
 *
 * Function: nightwatch_boot_image
 *
 * This function download the image from tftp server
 *
 * Input: None
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
long nightwatch_boot_image (int show_menu)
{
    char cmd[256];
    struct stat sts;
    char dest_img[256] = "/firmware/";
    char *src_img;

    src_img = getenv("SRC_IMG");
    if(!src_img) {
        src_img = "nwkdiag.tar.bz2";
    }
    sprintf(dest_img + strlen(dest_img), "%s", src_img);

    if (stat(dest_img, &sts) == -1) {
        if (tftp_get(0,
                     (unsigned char *)src_img,
                     0,
                     (unsigned char *)dest_img, 0) < 0) {
            sprintf(cmd, "rm -f %s", dest_img);
            system(cmd);
            fflush(stdout);
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);
        }
    } else {
        printf("\nFile image exist...ready to boot !!!\n");
    }

    sprintf(cmd, "tar xf %s", dest_img);
    system(cmd);

    return (PASSED);

}

/*------------------------------------------------------------------------------
 *
 * Function: nightwatch_nightwatch_sm_test().
 *
 * This function implements the nightwatch sm test
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int nightwatch_nightwatch_sm_test (int show_menu)
{
    int rc = PASSED;

    testname("Nightwatch SM");

    rc = nightwatch_sm_console_switch(show_menu);
    if (PASSED != rc) {
        cterr('f', 0, "%d test failed.\n", rc);
    }

    return (rc);
}

static int nightwatch_nightwatch_sm_test2 (int show_menu)
{
    return exec_nwk_shell_cmd(show_menu, NULL, FALSE, FALSE);
}

/*------------------------------------------------------------------------------
 *
 * Function: nightwatch_sm_dopplerG_console_switch().
 *
 * This function provides console redirect for nightwatch sm
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int nightwatch_sm_dopplerG_console_switch (int show_menu)
{
    nightwatch_ds_t   *iface;
    int slot;
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;

    iface = nightwatch_iface_p;
    assert(iface);
    slot = nightwatch_iface_p->slot;
    assert((0 < slot) && (slot < 4));

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
            "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second for the NOTE: */

    new_baud = slot_uart[slot - 1];

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d",
            new_baud ? "b9600" : "b115200",slot-1);

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: nightwatch_sm_console_switch().
 *
 * This function provides console redirect for nightwatch sm
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int nightwatch_sm_console_switch (int show_menu)
{
    nightwatch_ds_t   *iface;
    int slot;
    const int maxlen = 128;
    char cmd[maxlen];
    char *file_ngdiagcfg = "/tmp/ngdiagflag";
    char cmd_save_diagflag[64];
    int rc=0;

    if(nightwatch_boot_image(0) == FAILED) {
        cterr('f', 0, "Archive NIGHTWATCH firmware failed.");
        return (FAILED);
    } else {
        printf("\n ### Archive Firmware Image Successfully\n");
    }

    snprintf(cmd_save_diagflag, sizeof(cmd_save_diagflag)-1,
            "echo 'DIAGFLAG=0x%x' > %s", DIAGFLAG, file_ngdiagcfg);
    system(cmd_save_diagflag);
    iface = nightwatch_iface_p;
    assert(iface);
    slot = nightwatch_iface_p->slot;
    assert((0 < slot) && (slot < 4));

    printf("\n\n ### NOTE: Type exit followed by <Enter> "
            "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second for the NOTE: */

    snprintf(cmd, maxlen-1, "./nwkdiag 0x%x 0x%x %s", show_menu,
            iface->ngio_iface->id, iface->ngio_iface->serial_num);
#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    rc = WEXITSTATUS(system(cmd));

    return (rc);
}

static int nightwatch_sm_console_switch2 (int show_menu)
{
    return exec_nwk_shell_cmd(show_menu, NULL, FALSE, FALSE);
}


static int nightwatch_sm_lock_flash(int params)
{
    struct _nightwatch_shell_config_t *nwk_diag_shell_cfg = nwk_get_shell_cfg();
    int silent = !(DIAGFLAG & D_VERBOSE);
    char **nwk_cmd_exit = nwk_diag_shell_cfg->nwk_cmd_exit;
    char *nwk_cmd_flash[] = {"spiflashlock", NULL};

    printf("SPI FLASH LOCK GOLDEN SECTION(0x20000-0x30000)");

    exec_nwk_shell_cmd(params, nwk_cmd_flash, silent, TRUE);
    exec_nwk_shell_cmd(params, nwk_cmd_flash, FALSE, TRUE);
    exec_nwk_shell_cmd(params, nwk_cmd_exit, TRUE, TRUE);

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: nightwatch_host_utility().
 *
 * This function implements the nightwatch sm utility menu
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int nightwatch_host_utility (int show_menu)
{
    menu(nightwatch_util_submenup, nightwatch_util_submenu_table, '\0');

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: nighwatch_configure_i2c_expander().
 *
 * This function init Nightwatch I2C Expander.
 *
 * Input:  none.
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */

static long nightwatch_configure_i2c_expander (void)
{
    uchar io_port_conf0 = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];

    /* With HW design , Nightwatch only using Port0 */
    if (io_port_8bit_i2c_read(pca, PCA9555_CFG_PORT0_REG,
                &io_port_conf0, TRUE)) {
        return (FAILED);
    }
#ifdef DEBUG
    printf("\n%d, io_port_conf0 = 0x%02x\n", __LINE__, io_port_conf0);
    printf("\n%d, io_port_conf1 = 0x%02x\n", __LINE__, io_port_conf1);
#endif

    /* Set IO 1, 2, 4, 7 to output.  Set IO 3 to input */
    /* Set IO port0 1, 2, 7 to output.  Set IO 3,5 to input */
    io_port_conf0 |= (1 << PCA9555_IN_RDY_REGISTER);
    io_port_conf0 &= ~((1 << PCA9555_BOOT_SE_REGISTER) |
            (1 << PCA9555_SYS_RET_REGISTER) |
            (1 << PCA9555_MODE_REGISTER) |
            (1 << PCA9555_DETECT_REGISTER) );
#ifdef DEBUG
    printf("\n%d, io_port_conf0 = 0x%02x\n", __LINE__, io_port_conf0);
#endif
    if (io_port_8bit_i2c_write(pca, PCA9555_CFG_PORT0_REG,
                &io_port_conf0)) {
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG,
                &io_port_conf0, TRUE)) {
        return (FAILED);
    }

#ifdef DEBUG
    printf("\n%d, io_port_conf0 = 0x%02x\n", __LINE__, io_port_conf0);
#endif
    /* Set IO 2 to 0x1, this will release the Nightwatch Yeti3 CPU from reset */
    /* Set IO 1 to 0x0, this will tell the uboot it's diagnostic */
    /* Set IO 7 to 0x0, this will tell the uboot it's NGIO */

    io_port_conf0 = ((1 << PCA9555_BOOT_SE_REGISTER) |
            (1 << PCA9555_SYS_RET_REGISTER) |
            (1 << PCA9555_MODE_REGISTER));
    io_port_conf0 &= ~((1 << PCA9555_BOOT_SE_REGISTER) |
            (1 << PCA9555_DETECT_REGISTER) );
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG,
                &io_port_conf0)) {
        return (FAILED);
    }

    return (PASSED);
}
/*------------------------------------------------------------------------------
 *
 * Function: nightwatch_sm_init().
 *
 * This function init Nightwatch SM except pse2 fpga.
 *
 * Input:  none.
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int nightwatch_sm_init (void)
{
    nightwatch_ds_t     *nightwatch_info_p;
    nightwatch_info_p       = &nightwatch_info;
#if defined(BCM_ESW_SUPPORT)
    int ge_port0,ge_port1;
#endif
    int real_slot = nightwatch_info_p->slot;
    struct ngio_intf_t *ngio = nightwatch_iface_p->ngio_iface;
    uint host_10gkr_cap;
    int platform_id;
    uint32_t pcie_bus_dev_fn = 0;

    GE0_PNUM = KRXG_PNUM;
    NWK_SET_PARAM(NWK_DFLAG_SET_SLOT, real_slot-1);
    if (is_neptune()) {
        platform_id = NWK_NEPTUNE;
    } else if (is_overlord()) {
        platform_id = NWK_OVERLORD;
        GE0_PNUM = XAUI_PNUM;
    } else if (is_utah() || is_sword()) {
        host_10gkr_cap = host_ngio_10gkr_capability(ngio->mod_type, ngio->slot);
        if (host_10gkr_cap > 0) {
            platform_id = NWK_UTAH_GH;
        } else {
            platform_id = NWK_UTAH;
            GE0_PNUM = XAUI_PNUM;
        }
    } else if (nightwatch_loopup_bus_dev_fn(ngio, &pcie_bus_dev_fn)) {
        printf("General platform on PCIe bus %x:%x.%x.\n",
                pcie_bus_dev_fn & 0xff,
                ((pcie_bus_dev_fn & 0xf00) >> 8) | ((pcie_bus_dev_fn & 0x8000) >> 11),
                ((pcie_bus_dev_fn & 0x7000) >> 12)
            );

        NWK_SET_PARAM(NWK_DFLAG_SET_BUS, pcie_bus_dev_fn);
        platform_id = NWK_NEPTUNE;
    } else {
        cterr('f', 0, "unsupported host platform");
        return (FAILED);
    }
    NWK_SET_PARAM(NWK_DFLAG_SET_HOST, platform_id);

    /* Initial the GPIO */
    if (nightwatch_configure_i2c_expander()) {
        cterr('f', 0, "unable to configure i2c_expander");
        return (FAILED);
    }

#if defined(BCM_ESW_SUPPORT)
    /* setup the backplane loopback here. We will do the loopback test
       from module diag */
    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, GE0_PNUM);
    set_gesw_line_loopback(ge_port0, FALSE);
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, GE1_PNUM);
    set_gesw_line_loopback(ge_port1, FALSE);
#endif
    /* Configure the NGIO GE ports for 1G or 10G-KR. */
    ngio_ge_cfg(ngio);

#if defined(BCM_ESW_SUPPORT)
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nSM slot %d, %s : GESW port %d Line Loopback = %s",real_slot,
                GE0_PNUM ? "XAUI" : "GE0",
                ge_port0, get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
        printf("\nSM slot %d, %s : GESW port %d Line Loopback = %s\n",real_slot,
                "GE1",
                ge_port1, get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
        fflush(0);
    }
#endif

    if (ngio->pci_rdy) {
        /* Make sure Doppler boot up */
        msleep(5000);
        ngio->pci_rdy(ngio, TRUE);
        msleep(200);
    }
    /* FIXME
     * Workaround for Nightwatch
     * Here to restore PCIE configuration
     */
    ngiosm_pci_config_check(nightwatch_info_p->ngio_iface);

    return (PASSED);
}
/*************************************************************************
 * Function: nightwatch_i2c_ioe_reg_test
 *
 * This function toggles the i2c expander Polarity register and verifies it.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int nightwatch_i2c_ioe_reg_test (void)
{
    int ix;
    uchar sav, wval, rval;
    n2g_i2c_if_t *pca= &pca_i2c[0];
    int32_t reg = POLARITY_INV_REG;

    char *name = "Polarity";

    prpass(testpass, "I2C IO Expander Register Test : ");

    /* Save register under test */
    if (io_port_8bit_i2c_read(pca, reg, &sav, FALSE)) {
        return (FAILED);
    }

    /* ripple 1 test */
    for (ix = 0; ix < 8; ix++) {
        wval = 1 << ix;
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }
        printf("%#x ", rval);
        if (rval != wval) {
            cterr ('f', 0, "Ripple one test failed when accessing %s "
                    "register. Expect: %#x, Read: %#x.",
                    name, wval, rval);
            return FAILED;
        }
    }

    /* ripple 0 test */
    for (ix = 0; ix < 8; ix++) {
        wval = ~(1 << ix);
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }
        printf("%#x ", rval);
        if (rval != wval) {
            cterr ('f', 0, "Ripple zero test failed when accessing %s "
                    "register. Expect: %#x, Read: %#x.",
                    name, wval, rval);
            return FAILED;
        }
    }

    /* Restore register under test */
    if (io_port_8bit_i2c_write(pca, reg, &sav)) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 * Function: nightwatch_sm_test()
 *
 * Description: This function is the main program of the Nightwatch test.
 *         This function will be called from platform_sm.h.
 *              Upon entering this function, the slot number will be
 *              checked to see if the user chose to execute all tests
 *              or entering the submenu and executes accordingly.
 *
 * Input:  The slot number of the Nightwatch SM
 *
 * Output: PASSED if all the tests PASSED
 *         FAILED if at least one of them FAILED
 *
 **********************************************************************
 */

int nightwatch_sm_test(void *sm)
{
    int real_slot, ret_val = 0;
    struct ngio_intf_t *nwk_sm_iface = (struct ngio_intf_t *)sm;
    nightwatch_ds_t     *nightwatch_info_p;
    ushort cookie_id = 0;
    nightwatch_info_p       = &nightwatch_info;

    real_slot = nwk_sm_iface->slot;
    cookie_id = nwk_sm_iface->id;
    printf ("\n NIGHTWATCH is in slot %d, Cookie Id is %x \n", real_slot, cookie_id);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = nwk_sm_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    oir = (n2g_i2c_if_t *)nwk_sm_iface->oir;

    /*
     * Initialize an instance of Nightwatch data structure
     */
    nightwatch_iface_p = (nightwatch_ds_t *) &nightwatch_iface[real_slot];
    nightwatch_iface_p->cookie_id    = cookie_id;
    nightwatch_iface_p->slot         = real_slot;
    nightwatch_iface_p->uart         = nwk_sm_iface->uart_ctrl;
    nightwatch_iface_p->ngio_iface   = (struct ngio_intf_t *)sm;
    nightwatch_info_p->slot          = real_slot;
    nightwatch_info_p->ngio_iface    = nwk_sm_iface;

    /* FIXME
     * Workaround for Nightwatch
     * Since Kernel fails to alloc enough BAR space to meet DopplerG's requiement:
     *      1. 512M non-prefetchable for core register
     *      2. 2M prefetchable for DopplerG internal SRAM
     *      3. 2G prefetchable for DopplerG internal DRAM
     * Then PCIe BAR size will be realloc to 2M by default.
     *
     * Here to snapshot PCIE configuration before Kernle handle hotplug
     * then later to restore PCIE configuration after Kernel done hotplug
     */
    ngiosm_pci_config_check(nightwatch_info_p->ngio_iface);
    /*
     * Release Nightwatch SM out of reset.
     */
    /* uart/i2c unreset should be done via function pointer passed into the
       entry point */
    nwk_sm_iface->i2c_unreset(nwk_sm_iface);
    nwk_sm_iface->uart_on(nwk_sm_iface);
    nwk_sm_iface->unreset(nwk_sm_iface);
    msleep(1000);
    assert(sm);
    setup_uart();

    switch (cookie_id) {
        case NIGHTWATCH_24P_ID:
            nwk_diag_shell_cfg_init(real_slot, 20, cookie_id, nwk_sm_iface->serial_num);
            sprintf(nightwatch_iface_p->testname,"Slot%d 24-port Nightwatch",
                    real_slot);
            NWK_SET_PARAM(NWK_DFLAG_SET_MODE, NWK_24_PORT);
            break;
        case NIGHTWATCH_48P_ID:
            nwk_diag_shell_cfg_init(real_slot, 48, cookie_id, nwk_sm_iface->serial_num);
            nightwatch_info_p->num_asic++;
            sprintf(nightwatch_iface_p->testname,"Slot%d 48-port Nightwatch",
                    real_slot);
            NWK_SET_PARAM(NWK_DFLAG_SET_MODE, NWK_48_PORT);
            break;
        default:
            cterr('f', 0, "Invalid Nightwatch cookie id %#.4x in slot %d",
                    cookie_id, real_slot);
            return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER) == FAILED) {
        return (FAILED);
    };

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY) == FAILED) {
        return (FAILED);
    };
    if (nightwatch_sm_init()) {
        nightwatch_sm_cleanup();
        return (FAILED);
    }
    build_primary_submenu(nightwatch_submenu_table,
            NIGHTWATCH_SUBMENU_TABLE_SIZE,
            (char *)nwk_sm_iface->name,
            (menuinfo_t **)&nightwatch_submenup);
    build_secondary_submenu(nightwatch_submenu_table,
            NIGHTWATCH_SUBMENU_TABLE_SIZE,
            nightwatch_tests_secondary_items);

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    nightwatch_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    if (nwk_sm_iface->menu_display == TRUE) {
        menu(nightwatch_submenup, nightwatch_tests_secondary_items, '\0' );
    } else {
        if (nwk_sm_iface->test_type == IFACE_TEST) {
            ret_val |= oir_ltc4215_register_test(oir);
            ret_val |= nightwatch_i2c_ioe_reg_test();
        } else {  /* FULL_TEST */
            do_all_menu_items(nightwatch_submenup);
        }
    }

    ret_val |= util_oir_ltc4215_led(oir, OIR_LED_OFF);

    nightwatch_sm_cleanup();

    return (ret_val);
}

static int ngiosm_pcie_sw_snapshot(struct pci_dev *dev, void *data)
{
    struct _ngio_pcie_snapshot_ *pci_sh = (struct _ngio_pcie_snapshot_*)data;

    if (pci_sh->bus  <= dev->bus && dev->bus <= pci_sh->sub) {
        pci_sh->p[pci_sh->cnt] = pci_snapshot_create(dev);
        pci_sh->cnt++;
    }

    return false;
}

static int ngiosm_pcie_cfg_restore(struct _ngio_pcie_snapshot_  *s)
{
    int i;

    for (i = 0; i < s->cnt; i++) {
        if (!s->p[i])
            continue;

        pci_snapshot_restore(s->p[i]);
        pci_snapshot_destroy(s->p[i]);
        s->p[i] = NULL;
    }

    s->cnt = 0;

    return 0;
}

static int pci_dev_match_root(struct pci_dev *dev, void *data)
{
    int bus = *(int *)data;

    if (dev->secondary <= bus && bus <= dev->subordinate)
        return true;

    return false;
}

static int pci_dev_match_self(struct pci_dev *dev, void *data)
{
    int bus = *(int *)data;

    if (dev->secondary == bus)
        return true;

    return false;
}

static int nightwatch_loopup_bus_dev_fn(
        struct ngio_intf_t *intf,
        uint32_t *bus_dev_fn)
{
    int bus;
    struct pci_dev *dev;

    bus = get_ngio_pcie_dev_bus_num(SM_MODULE, intf->slot);
    dev = pci_dev_find((void*)&bus, pci_dev_match_self);

    if (!dev)
        return false;

    *bus_dev_fn = ((uint32_t)dev->bus << 0) |
                  (((uint32_t)dev->dev & 0x0f) <<  8) |
                  (((uint32_t)dev->dev & 0x10) << 11) |
                  (((uint32_t)dev->func & 0x7) << 12);

    return true;
}

static int ngiosm_pci_config_check(struct ngio_intf_t *intf)
{
    int bus;
    struct pci_dev *dev;

    if (ngio_pcie_snapshot.cnt==0) {
        __pci_rescan();
        bus = get_ngio_pcie_dev_bus_num(SM_MODULE, intf->slot);
        dev = pci_dev_find((void*)&bus, pci_dev_match_root);
        if (!dev)
            return false;

        ngio_pcie_snapshot.root = dev;
        ngio_pcie_snapshot.bus = dev->secondary;
        ngio_pcie_snapshot.sec = dev->secondary+1;
        ngio_pcie_snapshot.sub = dev->subordinate;

        pci_dev_find((void*)&ngio_pcie_snapshot, ngiosm_pcie_sw_snapshot);
        if (DIAGFLAG & D_VERBOSE) {
            printf("\nThe snapshot of PCIe tree on %x:%x.%x with %d nodes is taken\n",
                    dev->bus, dev->dev, dev->func, ngio_pcie_snapshot.cnt);
        }
    } else {
        dev = ngio_pcie_snapshot.root;
        if (DIAGFLAG & D_VERBOSE) {
            printf("\nRestoring configuration of PCIe tree on %x:%x.%x\n",
                    dev->bus, dev->dev, dev->func);
        }
        ngiosm_pcie_cfg_restore(&ngio_pcie_snapshot);
    }

    return true;
}

#if !defined(BCM_ESW_SUPPORT)
static int get_eth_speed(const char *ethName)
{
    char buf[32];
    char str1[12];
    char str2[12];
    char *speedFile = "/tmp/speed";
    int fd;
    int len;
    int speed = 0;

    sprintf(buf, "ethtool %s | grep Speed > %s", ethName, speedFile);
    system(buf);

    fd = open(speedFile, O_RDONLY);
    if (fd < 0) {
        printf("Open %s error!\n", speedFile);
    } else {
        len = read(fd, buf, sizeof(buf));
        if (len > 0) {
            sscanf(buf, "%s %d%s", str1, &speed, str2);
        }
        close(fd);
    }

    return speed;
}

static int curie_get_eth_by_slot_bp(int slot, int ge_port)
{
    int port = get_sgmii_port_num(slot, TYPE_SWITCH);
    int eth_num_off_per_slot = -2;

    /* curie1RU: eth num of GE1 is two more bigger than GE0 */
    if (is_curie_1ru()) {
        eth_num_off_per_slot = 2;
    }

    switch (ge_port) {
    case NGIO_GE0:
        break;

    case NGIO_GE1:
        port += eth_num_off_per_slot;
        break;

    default:
        port = -1;
    }

    return port;
}

#if defined(STRESS_NGSM_ETH_TRAFFIC)
#define NWK_NGSM_ETH_TRAF_UTIL(port, pkt_cnt)                \
        eth_traf_util(port, pkt_cnt)
static int eth_traf_util(int port, int pkt_cnt)
{
    struct eth_traf_tx_task_settings tx_settings;
    struct eth_traf_rx_task_settings rx_settings;
    tx_settings.mode = ETH_TRAF_TX_MODE_FIXED;
    tx_settings.check = ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_settings.len = 64;
    tx_settings.burst = 1;
    tx_settings.interval = 100;
    rx_settings.chk_mode = ETH_TRAF_RX_MODE_CHECK_BIT;
    return eth_traf_util_test(eth_name, eth_name, &tx_settings, &rx_settings, 10);
}
#else
#define NWK_NGSM_ETH_TRAF_UTIL(port, pkt_cnt)               \
        sgmii_lpbk_util(port, pkt_cnt)
#endif

static int __nightwatch_port_tx_util(int ge_port, uint32_t *pkts, uint32_t *burst, time_t timeout, boolean interact)
{
    int rc = FAILED;
    int port = -1;
    int speed;
    int slot = nightwatch_info.slot;
    char eth_name[8];
    uint32_t pkt_cnt = *pkts;

    if (interact) {
        pkt_cnt = getdec_answer("Packet count", pkt_cnt, 0, -1);
        if (pkt_cnt == 0)
            return (PASSED);
    }

    port = curie_get_eth_by_slot_bp(slot, ge_port);
    if (port == -1) {
        return (rc);
    }

    sprintf(eth_name, "eth%d", port);
    speed = get_eth_speed(eth_name);
    printf("[IF:%s link speed:%dMb/s pkt_len:auto burst:1 pkt_interval:0us pkt_count:%d]\n",
            eth_name,
            speed,
            pkt_cnt);
    rc = NWK_NGSM_ETH_TRAF_UTIL(port, pkt_cnt);
    if (rc != PASSED) {
        cterr('f', 0, "%s(): test %s%d error!!!\n",
              __FUNCTION__, "BP Port NGSM-GE", ge_port);
    }

    return (rc);
}
#else
static int __nightwatch_port_tx_on_eth(int ge_port, uint32_t pkt_cnt)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)nightwatch_iface_p->ngio_iface;
    int rc = FAILED;
    int ctrl_plane_sgmii_port = get_sgmii_port_num(ngio->slot, TYPE_SWITCH);

    rc = sgmii_lpbk_util(ctrl_plane_sgmii_port, pkt_cnt);

    if (rc != PASSED) {
        cterr('f', 0, "%s(): test %s%d error!!!\n",
              __FUNCTION__, "BP Port NGSM-GE", ge_port);
    }

    return (rc);
}
static int __nightwatch_port_tx_util(int ge_port, uint32_t *pkts_total, uint32_t *pkts_per_send, time_t timeout, boolean interact)
{
    int rc = PASSED;
    int unit = bcm_uid, port;
    char tx_cmd[128];
    char chk_cmd[32];
    int slot = nightwatch_info.slot;
    uint32_t pkt_cnt = *pkts_total;
    uint32_t burst = *pkts_per_send;
    static uint32_t pkt_len = 68;
    static int pattern = 0xa5a5a5a5;
    char *mac_da_str = "ff:ff:ff:ff:ff:ff";
    char *port_str;
    boolean tx_verbose = interact;
#if defined(STRESS_NGSM_ETH_TRAFFIC)
    boolean stress_mode = TRUE;
#else
    boolean stress_mode = FALSE;
#endif
    char answer;
    static int burst_interval = 100;
    int (*exec_port_cmd)(int, char*, boolean) = exec_bcm_shell_cmd;

    if (interact) {
        answer = getc_answer("Stress mode (Y/N)", "YyNn", stress_mode?'Y':'N');
        if ('Y' == answer || 'y' == answer)
            stress_mode = TRUE;

        pkt_cnt = getdec_answer("Packet count", pkt_cnt, 0, -1);
        if (pkt_cnt == 0)
            return (rc);
        else
            *pkts_total = pkt_cnt;

        if (TRUE == stress_mode) {
            pkt_len = getdec_answer("Packet length", pkt_len, 68, 1518);
            pattern = gethex_answer("Pattern", pattern, 0, 0xffffffff);
            burst = getdec_answer("Burst pkts", burst, 1, pkt_cnt);
            *pkts_per_send = burst;
            burst_interval = getdec_answer("Burst interval (ms)", burst_interval, 0, 10000);
        }
    }

    port = ovld_get_ge_sw_port_num(slot, TGT_DEV_NGSM, ge_port);
    port_str = get_gesw_pname(port);
    /* Clear counter */
    if (DIAGFLAG & D_VERBOSE) {
        printf("Clearing counter before tx ...\n");
    }
    sprintf(chk_cmd, "clear counters %s", port_str);
    exec_port_cmd(unit, chk_cmd, interact);

    /* Execute TX packets */
    printf("Executing tx packets ...\n");

    if ( FALSE == stress_mode ) {
        rc = __nightwatch_port_tx_on_eth(ge_port,pkt_cnt);
        if (DIAGFLAG & D_VERBOSE) {
            printf("Checking counter after tx ...\n");
            /* Check counter */
            sprintf(chk_cmd, "show counter %s", port_str);
            exec_port_cmd(unit, chk_cmd, interact);

            /* Show port status */
            sprintf(chk_cmd, "ps %s", port_str);
            exec_port_cmd(unit, chk_cmd, TRUE);
        }
    } else {
        if (interact)
            printf("Repeat %u times ", pkt_cnt/burst + !!(pkt_cnt%burst));
        for (; pkt_cnt >= burst; pkt_cnt -= burst) {
            sprintf(tx_cmd, "tx %d PBM=%s UBM=%s Length=%d Pattern=%#.8x DM=%s",
                burst, port_str, port_str, pkt_len, pattern, mac_da_str);
            exec_port_cmd(unit, tx_cmd, tx_verbose);
            tx_verbose = FALSE;
            msleep(burst_interval);
        }
        if (pkt_cnt > 0) {
            sprintf(tx_cmd, "tx %d PBM=%s UBM=%s Length=%d Pattern=%#.8x DM=%s",
                pkt_cnt, port_str, port_str, pkt_len, pattern, mac_da_str);
            exec_port_cmd(unit, tx_cmd, tx_verbose);
        }

        msleep(1000);

        if (DIAGFLAG & D_VERBOSE) {
            printf("Checking counter after tx ...\n");
        }
        /* Check counter */
        sprintf(chk_cmd, "show counter %s", port_str);
        exec_port_cmd(unit, chk_cmd, interact);

        /* Show port status */
        sprintf(chk_cmd, "ps %s", port_str);
        exec_port_cmd(unit, chk_cmd, TRUE);
    }

    return (rc);
}
#endif

static int nightwatch_bp_pre_traf(int ge_num, int port, boolean silent, int params)
{
    struct stat st;
    char nwk_cmd_exec[64];
    char nwk_bat_file[32];
    char *nwk_pre_traf_cmds[] = {
        nwk_cmd_exec,
        NULL
    };

    snprintf(nwk_bat_file, sizeof(nwk_bat_file)-1, "ge%d_p%d_pre_traf.bat", ge_num, port);
    if (stat(nwk_bat_file, &st) < 0)
        return (FAILED);

    snprintf(nwk_cmd_exec, sizeof(nwk_cmd_exec)-1, "exec %s -h -s", nwk_bat_file);
    exec_nwk_shell_cmd(params, nwk_pre_traf_cmds, silent, TRUE);

    return (PASSED);
}

static int nightwatch_bp_post_traf(int ge_num, int port, boolean silent, int params)
{
    struct stat st;
    char nwk_cmd_exec[64];
    char nwk_bat_file[32];
    char *nwk_post_traf_cmds[] = {
        nwk_cmd_exec,
        NULL
    };

    snprintf(nwk_bat_file, sizeof(nwk_bat_file)-1, "ge%d_p%d_post_traf.bat", ge_num, port);
    if (stat(nwk_bat_file, &st) < 0)
        return (FAILED);

    snprintf(nwk_cmd_exec, sizeof(nwk_cmd_exec)-1, "exec %s -h -s", nwk_bat_file);
    exec_nwk_shell_cmd(params, nwk_post_traf_cmds, silent, TRUE);

    return (PASSED);
}

static int nightwatch_dump_nwk_bp_counter(int stardust_flag, int port)
{
    char nwk_cmd_tlk_sel_chA[32];
    char nwk_cmd_tlk_sel_chB[33];
    char nwk_cmd_tlk_rdump[32];
    char nwk_cmd_dop_stat[32];
    int tlk_dbg_flag = 0x400;
    char *nwk_error_dump_cmds[] = {
        nwk_cmd_tlk_sel_chA,
        nwk_cmd_tlk_rdump,
        nwk_cmd_tlk_sel_chB,
        nwk_cmd_tlk_rdump,
        nwk_cmd_dop_stat,
        NULL
    };

    snprintf(nwk_cmd_tlk_sel_chA, sizeof(nwk_cmd_tlk_sel_chA)-1,
            "TLK10232Debug %d 0x%x", port, tlk_dbg_flag | 0x80000000);
    snprintf(nwk_cmd_tlk_sel_chB, sizeof(nwk_cmd_tlk_sel_chB)-1,
            "TLK10232Debug %d 0x%x", port, tlk_dbg_flag);
    snprintf(nwk_cmd_tlk_rdump, sizeof(nwk_cmd_tlk_rdump)-1,
            "rd port/%d %s", port, "counter");
    snprintf(nwk_cmd_dop_stat, sizeof(nwk_cmd_dop_stat)-1,
            "statdump %d", port);

    exec_nwk_shell_cmd(stardust_flag, nwk_error_dump_cmds, FALSE, TRUE);

    return (PASSED);
}

static int nightwatch_port_tx_util_do(
        int params,
        boolean interact,
        boolean sysinit,
        boolean end_session)
{
    nightwatch_ds_t     *nightwatch_info_p;
    nightwatch_info_p = &nightwatch_info;
    char *nwk_cmd_sysinit[] = {"sysinit", NULL};
    char nwk_cmd_clr_loopback[32];
    char nwk_cmd_set_loopback[64];
    char nwk_cmd_statclear[32];
    char nwk_cmd_statshow[32];
    char nwk_port_cfg_ngio_ge0[32];
    char nwk_port_init_ngio_ge0[32];
    char *nwk_cmds[] = {
        nwk_cmd_clr_loopback,
        nwk_cmd_set_loopback,
        NULL
    };
    char *nwk_statclear[] = {
        nwk_cmd_statclear,
        NULL
    };
    char *nwk_statshow[] = {
        nwk_cmd_statshow,
        NULL
    };
    char *nwk_extra_cmds[] = {
        nwk_port_cfg_ngio_ge0,
        nwk_port_init_ngio_ge0,
        NULL
    };
    int silent = !(DIAGFLAG & D_VERBOSE);
    int ge_port[3] = {GE0_PNUM, GE1_PNUM, NGIO_GE0};
    time_t timeout[3] = {10, 10, 10};
    struct _nightwatch_shell_config_t *nwk_diag_shell_cfg = nwk_get_shell_cfg();
    int *bp_port_num = nwk_diag_shell_cfg->bp_cap.port;
    uint32_t *bp_port_pkts = nwk_diag_shell_cfg->ge_port_pkts;
    uint32_t *bp_port_burst = nwk_diag_shell_cfg->ge_port_burst;
    char **nwk_cmd_exit = nwk_diag_shell_cfg->nwk_cmd_exit;
    int i, ngio_ge_pnum = 2;

    testname("NGIOSM GE/XE Ports Test");

    if (sysinit)
        exec_nwk_shell_cmd(params, nwk_cmd_sysinit, silent, TRUE);

    snprintf(nwk_cmd_clr_loopback, sizeof(nwk_cmd_clr_loopback)-1,
            "PortCtrl %d,%d -r", bp_port_num[0], bp_port_num[1]);

    snprintf(nwk_cmd_statclear, sizeof(nwk_cmd_statclear)-1,
            "statclear %d,%d", bp_port_num[0], bp_port_num[1]);

    snprintf(nwk_cmd_statshow, sizeof(nwk_cmd_statshow)-1,
            "statshow %d,%d", bp_port_num[0], bp_port_num[1]);

    if (ge_port[0] != NGIO_GE0) {
        ngio_ge_pnum =3;
    }

    exec_nwk_shell_cmd(params, nwk_statclear, silent, TRUE);
    for (i = 0; i < ngio_ge_pnum; i++) {
        printf("Configuring NGIO-GE%d ext loopback on Nightwatch port %d ...\n",
                ge_port[i], bp_port_num[i]);

        if (PASSED != nightwatch_bp_pre_traf(ge_port[i], bp_port_num[i], silent, params)) {
            if (2==i) {
                memset(nwk_port_cfg_ngio_ge0, 0, sizeof(nwk_port_cfg_ngio_ge0));
                snprintf(nwk_port_cfg_ngio_ge0, sizeof(nwk_port_cfg_ngio_ge0)-1,
                        "tlk10232mode %d 9", bp_port_num[i]);
                memset(nwk_port_init_ngio_ge0, 0, sizeof(nwk_port_init_ngio_ge0));
                snprintf(nwk_port_init_ngio_ge0, sizeof(nwk_port_init_ngio_ge0)-1,
                        "sysinit -d:port/%d", bp_port_num[i]);
                exec_nwk_shell_cmd(params, nwk_extra_cmds, silent, TRUE);
            }

            memset(nwk_cmd_set_loopback, 0, sizeof(nwk_cmd_set_loopback));
            snprintf(nwk_cmd_set_loopback, sizeof(nwk_cmd_set_loopback)-1,
                    "PortCtrl %d -L:9", bp_port_num[i]);
            exec_nwk_shell_cmd(params, nwk_cmds, silent, TRUE);
        }

        if (!interact)
            msleep(2000);

        if (PASSED != __nightwatch_port_tx_util(ge_port[i], &bp_port_pkts[i],
                    &bp_port_burst[i], timeout[i], interact)) {
            nightwatch_dump_nwk_bp_counter(params, bp_port_num[i]);
        }

        nightwatch_bp_post_traf(ge_port[i], bp_port_num[i], silent, params);
    }
    exec_nwk_shell_cmd(params, nwk_statshow, !interact, TRUE);

    if (end_session)
        exec_nwk_shell_cmd(params, nwk_cmd_exit, silent, TRUE);

    return (PASSED);
}

static int nightwatch_port_tx_util(int params)
{
    return nightwatch_port_tx_util_do(params, FALSE, TRUE, TRUE);
}

static int nightwatch_port_tx_util2(int params)
{
    return nightwatch_port_tx_util_do(params, TRUE, FALSE, FALSE);
}

#if defined(BCM_ESW_SUPPORT)
static int nightwatch_switch_gesw_port_mode(int show_menu)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)nightwatch_iface_p->ngio_iface;
    struct _nightwatch_shell_config_t *nwk_diag_shell_cfg = nwk_get_shell_cfg();
    int ge_num = 0;
    int port_ability;
    int ngio_ge_port = nwk_diag_shell_cfg->bp_cap.portm[0];
    uint32_t mode = NWK_BP_MODE_10GKR|NWK_BP_MODE_1G;
    uint8_t ans;

    if (ngio_ge_port == NGIO_GE0)
        ge_num = getdec_answer("\nBP Port GE0[0]/GE[1]:", 0, 0, 1);
    else if (ngio_ge_port == NGIO_XAUI)
        ge_num = getdec_answer("\nBP Port XAUI[0]/GE[1]:", 1, 0, 1);

    port_ability = nwk_diag_shell_cfg->bp_cap.ability[ge_num];
    ngio_ge_port = nwk_diag_shell_cfg->bp_cap.portm[ge_num];

    if ((port_ability & mode) != mode) {
        cterr('w', 0, "BP GE%d not able to be changed.\n", ngio_ge_port);
        return (PASSED);
    }

    mode  = nwk_diag_shell_cfg->bp_cap.speed[ge_num];
    printf("\nGE%d is %s, change to %s? (y/N) ", ngio_ge_port,
            (mode&NWK_BP_MODE_10GKR) ? "10GKR" : "1G SGMII",
            (mode&NWK_BP_MODE_10GKR) ? "1G SGMII" : "10GKR");
    fflush(0);
    ans = getchar();
    if (ans != 'y' && ans != 'Y') {
        return (PASSED);
    }
    mode ^= NWK_BP_MODE_10GKR;

    if (mode & NWK_BP_MODE_10GKR) {
        if (cfg_host_10gkr_port(ngio->mod_type, ngio->slot, ngio_ge_port, 1) == FAIL) {
            cterr('f', 0, "GE%d set up 10G-KR port failed.\n", ngio_ge_port);
            return (FAILED);
        }
        cterr('w', 0, "GE%d set up 10G-KR port success.\n", ngio_ge_port);
    } else {
        if (cfg_host_10gkr_port(ngio->mod_type, ngio->slot, ngio_ge_port, 0) == FAIL) {
            cterr('f', 0, "GE%d set up 1G port failed.\n", ngio_ge_port);
            return (FAILED);
        }
        cterr('w', 0, "GE%d set up 1G port success.\n", ngio_ge_port);
    }

    nwk_diag_shell_cfg->bp_cap.speed[ge_num] = mode;

    return (PASSED);
}
#endif

/*-------------------------------------------------
$Log: nightwatch_host.c,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.3  2019/07/25 02:46:34  mingding
CSCvk64124-34: Add sub-menu item to lock golden image on modules' boot-flash

Revision 1.1.2.2  2019/06/11 06:32:40  mingding
CSCvk64124-30: Integrate stardust to ISR menu test

    - Pass DIAGFLAG to stardust from ngdiag for test usage
    - Receive errors count from stardust
    - Report errors for bp test failure on O2/Utah/Neptune
    - Report errors while running test from menu 'Nighwatch SM Test'

Revision 1.1.2.1  2019/05/30 05:33:34  mingding
CSCvk64124-29: Support PCIe-based Nightwatch Server Module

*/
