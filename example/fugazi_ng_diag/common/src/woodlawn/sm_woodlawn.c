/* $Id: sm_woodlawn.c,v 1.16 2020/01/09 01:03:02 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn.c,v $
 *******************************************************************************
 * File Name: sm_woodlawn.c
 *
 * Description: Woodlawn SM main source file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2014 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "strings.h"
#include "sm_slot.h"
#include "menu.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_slot.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "sm_woodlawn.h"
#include "ngio.h"
#include "plat_defs.h"
#include "linux_ntwk.h"

#include "pca.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>

#include "sm_woodlawn_uart.h"
#include "sm_woodlawn_fpga_lib.h"
#include "sm_woodlawn_comm.h"
#include "dash_fpga.h"
/* timing card has NOT been tested on Utah yet */
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
#include "vm_timingcard.h"
#include "vm_timingcard_zl3036x_lib.h"
#endif

/**************************************
 *  Global Variables
 **************************************
 */
static struct ngio_intf_t *woodlawn_sm_iface;

woodlawn_ds_t woodlawn_iface[MAX_SM+1];
static woodlawn_ds_t *woodlawn_iface_p;
static void (*woodlawn_saved_diag_exec)(void) = NULL;
static long woodlawn_ltc_reg_read(void);
static long woodlawn_ltc_reg_write(void);
static long woodlawn_ltc_reg_test(void);
static long woodlawn_gesw_lpbk_set(void);
static int woodlawn_sm_console_switch(int show_menu);
static long xaui_lpbk_test(void);
static int woodlawn_power_on(struct ngio_intf_t *);
long woodlawn_fpga_reg_read(void);
long woodlawn_fpga_reg_write(void);
static long woodlawn_boot_image(int);
static long woodlawn_run_sm_test(int);
static long woodlawn_pcie_test(int);
static long ge_bp_lpbk_test(int);
static int is_woodlawn_up(int);
static int woodlawn_discover_pcie(ushort, ushort);
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
static long woodlawn_boot_up_other_slot(int);
static long woodlawn_power_off_other_slot(int);
static long timingcard_power_on(int);
static long timingcard_power_off(int);
#endif
long woodlawn_voltage_margin_status(void);
long woodlawn_voltage_margin_ctrl(void);
long woodlawn_show_diag_version(void);
long woodlawn_show_power_status(void);
long power_on_woodlawn_card(void);
long power_off_woodlawn_card(void);
long led_all_test(void);
long led_seq_test(void);
long led_single_test(void);
long led_multiple_test(void);
int get_sku_id(void);
long print_common_led(void);
long print_four_ge_port_led(void);
long woodlawn_1340_reg_read(void);
long woodlawn_1340_reg_write(void);
long woodlawn_1548_reg_read(void);
long woodlawn_1548_reg_write(void);
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
long woodlawn_1548_ptp_reg_read(void);
long woodlawn_1548_ptp_reg_write(void);
#endif
long woodlawn_2222_reg_read(void);
long woodlawn_2222_reg_write(void);
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
long woodlawn_2222_ptp_reg_read(void);
long woodlawn_2222_ptp_reg_write(void);
#endif
long woodlawn_sm_fpga_reg_read(void);
long woodlawn_sm_fpga_reg_write(void);
long woodlawn_10232_reg_read(void);
long woodlawn_10232_reg_write(void);
long woodlawn_cavium_sgmii_status(void);
long woodlawn_cavium_sgmii_config(void);
long woodlawn_cavium_sgmii_reg_read(void);
long woodlawn_cavium_xaui_status(void);
long woodlawn_cavium_xaui_gmx_reg_read(void);
long woodlawn_cavium_xaui_pcs_reg_read(void);
long woodlawn_bootflash_get_info(void);
long woodlawn_bootflash_otp_test(void);
int woodlawn_boot_select(void);
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
long woodlawn_enable_1548_ptp_engine(void);
long woodlawn_clk_direction(uint);
long ptp_clk_trigger_verify(void);
long mrvl1548_clk_trig_in_verify(void);
long mrvl1548_clk_trig_out_verify(void);
long mrvl2222_clk_trig_in_verify(void);
long mrvl2222_clk_trig_out_verify(void);
#endif
static int has_88x2222(void);
static int has_10gkr_feature(void);

boolean pca9557;

int woodlawn_test_slot = 0;
boolean woodlawn_init_3036x = FALSE;
static int woodlawn_uart_ctrl = 0; 
/* 
 * Extern function prototypes 
 */
extern int getdec_answer(char *,uint ,uint ,uint);
extern int do_all_menu_items(struct menuinfo *);
extern int sgmii_lpbk_util(int, int);
extern int get_ctrl_plane_sgmii_port(void); 
extern int is_plat_10gkr_capable(void);
/* 
 * Function prototype 
 */
int woodlawn_sm_test(void *);
long woodlawn_utility_submenu(int);
void woodlawn_get_sm_ip_addr(char *);

/* 
 * Woodlawn SM main menu on Overlord platform
 */
static submenu_xtable_t woodlawn_submenu_tbl[] = {
    {"Woodlawn Switch Console", (PFT)woodlawn_sm_console_switch, 0,
     0,     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Firmware Download",  woodlawn_boot_image, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"LTC4215 Register Test",  woodlawn_ltc_reg_test, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"FPGA Register Test",  woodlawn_fpga_reg_test, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"Run SM Tests",  woodlawn_run_sm_test, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"GE0 Backplane Loopback Test",  ge_bp_lpbk_test, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"GE1 Backplane Loopback Test",  ge_bp_lpbk_test, 1,
      MF_CONTINUOUS | MF_DOALL, (long(*)())has_10gkr_feature, 0, (long(*)())0, 0 },
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
    {"MRVL1548 Clock/Trigger In Verification",  mrvl1548_clk_trig_in_verify, 0,
      MF_CONTINUOUS, (long(*)())0, 0, (long(*)())0, 0 },
    {"MRVL1548 Clock/Trigger Out Verification",  mrvl1548_clk_trig_out_verify, 0,
      MF_CONTINUOUS, (long(*)())0, 0, (long(*)())0, 0 },
    {"MRVL2222 Clock/Trigger In Verification",  mrvl2222_clk_trig_in_verify, 0,
      MF_CONTINUOUS, (long(*)())has_88x2222, 0, (long(*)())0, 0 },
    {"MRVL2222 Clock/Trigger Out Verification",  mrvl2222_clk_trig_out_verify, 0,
      MF_CONTINUOUS, (long(*)())has_88x2222, 0, (long(*)())0, 0 },
#endif
    { "Woodlawn utility",      woodlawn_utility_submenu, 0,
     0,                   (long(*)())0, 0, (long(*)())0, 0 },
};

#define WOODLAWN_SUBMENU_TABLE_SZ \
                (sizeof(woodlawn_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t woodlawn_primary_items[WOODLAWN_SUBMENU_TABLE_SZ +
                                   MAX_BASE_ITEMS];
static mitem_t woodlawn_secondary_items[WOODLAWN_SUBMENU_TABLE_SZ +
                                     MAX_BASE_ITEMS];

static menuinfo_t woodlawn_menu = {
    "Woodlawn Main Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    woodlawn_primary_items,
};
static menuinfo_t *woodlawn_menup = &woodlawn_menu;


/* 
 * Woodlawn SM utilities menu on Overlord platform
 */
static mitem_t woodlawn_util_submenu_table[] = {
    { "LTC4215 Register Read util", 0, 0, woodlawn_ltc_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "LTC4215 Register Write util", 0, 0, woodlawn_ltc_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Platform GE Switch Line Loopback Setup", 0, 0, woodlawn_gesw_lpbk_set,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Platform XAUI Switch Line Loopback Setup", 0, 0, xaui_lpbk_test,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "FPGA Register Read util", 0, 0, woodlawn_fpga_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "FPGA Register Write util", 0, 0, woodlawn_fpga_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Voltage Margin Show Status util", 0, 0, woodlawn_voltage_margin_status,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Voltage Margin Ctrl util", 0, 0, woodlawn_voltage_margin_ctrl,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Display Diagnostic Version", 0, 0, woodlawn_show_diag_version,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "PCIe Test", 0, 0,  woodlawn_pcie_test, 
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Display SM Card Power Status", 0, 0, woodlawn_show_power_status,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Power on Woodlawn Card", 0, 0, power_on_woodlawn_card,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Power off Woodlawn Card", 0, 0, power_off_woodlawn_card,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Turn multiple LEDs On/Off", 0, 0, led_multiple_test,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Turn single LED On/Off", 0, 0, led_single_test,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Turn LEDs On/Off", 0, 0, led_all_test,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "LEDs Sequential On/Off", 0, 0, led_seq_test,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "88E1340 Read Reg Util", 0, 0, woodlawn_1340_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "88E1340 Alter Reg Util", 0, 0, woodlawn_1340_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "88E1548 Read Reg Util", 0, 0, woodlawn_1548_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "88E1548 Alter Reg Util", 0, 0, woodlawn_1548_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
    { "88E1548 Read PTP Reg Util", 0, 0, woodlawn_1548_ptp_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "88E1548 Alter PTP Reg Util", 0, 0, woodlawn_1548_ptp_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
#endif
    { "88X2222 Read Reg Util", 0, 0, woodlawn_2222_reg_read,
      (long *)&zero, 0, (type_t(*)())has_88x2222, 0 },
    { "88X2222 Alter Reg Util", 0, 0, woodlawn_2222_reg_write,
      (long *)&zero, 0, (type_t(*)())has_88x2222, 0 },
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
    { "88X2222 Read PTP Reg Util", 0, 0, woodlawn_2222_ptp_reg_read,
      (long *)&zero, 0, (type_t(*)())has_88x2222, 0 },
    { "88X2222 Alter PTP Reg Util", 0, 0, woodlawn_2222_ptp_reg_write,
      (long *)&zero, 0, (type_t(*)())has_88x2222, 0 },
#endif
    { "SM FPGA Read Reg Util", 0, 0, woodlawn_sm_fpga_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "SM FPGA Alter Reg Util", 0, 0, woodlawn_sm_fpga_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "TLK10232 Read Reg Util", 0, 0, woodlawn_10232_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "TLK10232 Alter Reg Util", 0, 0, woodlawn_10232_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Show SM Cavium SGMII Port Status", 0, 0, woodlawn_cavium_sgmii_status,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "SM Cavium SGMII Port Config", 0, 0, woodlawn_cavium_sgmii_config,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Read SM Cavium SGMII Reg", 0, 0, woodlawn_cavium_sgmii_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Show SM Cavium XAUI Port Status", 0, 0, woodlawn_cavium_xaui_status,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Read SM Cavium XAUI GMX Reg", 0, 0, woodlawn_cavium_xaui_gmx_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Read SM Cavium XAUI PCS Reg", 0, 0, woodlawn_cavium_xaui_pcs_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Bootflash Get Info Util", 0, 0, woodlawn_bootflash_get_info,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Bootflash OTP Verification Util", 0, 0, woodlawn_bootflash_otp_test,
      (long *)&zero, 0, (type_t(*)())0, 0 },
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
    { "Enable 88E1548 PTP Engine", 0, 0, woodlawn_enable_1548_ptp_engine,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "PTP Clk/Trigger Verification", 0, 0, ptp_clk_trigger_verify,
      (long *)&zero, 0, (type_t(*)())0, 0 },
#endif
};

#define WOODLAWN_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(woodlawn_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t woodlawn_util_subtest_menu = {
    "Woodlawn Utilities Menu",
    0,                                        /* title param */
    0,                         /* show diag flags */
    0,
    WOODLAWN_UTIL_SUBMENU_TABLE_SZ,
    woodlawn_util_submenu_table,
};

static menuinfo_t *woodlawn_util_submenup = &woodlawn_util_subtest_menu;

static n2g_i2c_if_t *oir;
n2g_i2c_if_t woodlawn_fpga_i2c;

/******************************************************************************
 *
 * Function: has_10gkr_feature
 *
 * Description: Judge this is Greyhound switch or not.
 *
 * Inputs      : None
 * Outputs     :TRUE/FALSE
 *
 *****************************************************************************/
static int has_10gkr_feature (void)
{
    if (is_plat_10gkr_capable()) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/******************************************************************************
 *
 * Function: has_88x2222
 *
 * Description: According to sku id to judge whether this is 10G sku or not.
 *
 * Inputs      : None
 * Outputs     :TRUE/FALSE
 *
 *****************************************************************************/
static int has_88x2222 (void)
{
    int id;

    id = get_sku_id();

    if (id == WOODLAWN_6GE) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/******************************************************************************
 *
 * Function: woodlawn_boot_select 
 *
 * Description: Define Woodlawn boot from upgrade setcor.
 *
 * Inputs      : None
 * Outputs     : None 
 *
 *****************************************************************************/
int woodlawn_boot_select (void)
{
    n2g_i2c_if_t *i2c_p = &woodlawn_fpga_i2c;    
    unsigned int reg_addr;
    int boot_val = 2;

    reg_addr = FPGA_GPIO_EXP0_OUT_REG; 
    i2c_p->offset = reg_addr;
    i2c_p->buf = (char *)&boot_val;


    if (n2g_i2c_write(i2c_p) != PASSED) {
        cterr('f', 0, "Unable to write fpga reg #x bit 2\n", reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: get_sku_id
 *
 * Description: This function read fpga id to distinguish SKU type.
 *
 * Inputs      : None
 * Outputs     : WOODLAWN_4GE_1XAUI/WOODLAWN_6GE/WOODLAWN_6GE_1XAUI
 *
 *****************************************************************************/
int get_sku_id (void)
{
    char id;
    n2g_i2c_if_t *i2c_p = &woodlawn_fpga_i2c;    
    uchar fpga_buf[1];
    unsigned int reg_addr;
    int rc = PASSED;

    reg_addr = FPGA_HIGH_VER_REG; 
    i2c_p->offset = reg_addr;
    i2c_p->buf = (char *)fpga_buf;

    rc = n2g_i2c_read(i2c_p); 

    if (rc != PASSED) {
        cterr('f', 0, "Unable to read FPGA id, ret_code = %#x\n", rc);
        rc = FAILED;
    }

    /* Read fpga id, bit4 - fpga id1, bit5 - fpga id2 */
    id = (fpga_buf[0] & (~FPGA_ID_MASK)) >> 4;
   
    /* SKU1 ID = 01(4 port sfp, 1 port sfp+), SKU2 ID = 00(6 port sfp). */
    if (id == FPGA_ID_SKU1) {
        return (WOODLAWN_4GE_1XAUI);
    } else if (id == FPGA_ID_SKU2) {
        return (WOODLAWN_6GE);
    } else {
        return (WOODLAWN_6GE_1XAUI);
    }
}

/**********************************************************************
 *
 * Function: woodlawn_sm_test
 *
 * This function is the main entrance for Woodlawn SM testing.
 *
 * Input : sm - pointer to sm ngio interface
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int woodlawn_sm_test (void *sm)
{
    int real_slot;
    int ret_val;
    n2g_i2c_if_t *fpga = &woodlawn_fpga_i2c;
    ushort board_id = 0;
    char tty_dev[32];
    int ge_bp_port;
    uint8_t data = 0;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n%s: sm=%#lx ", __FUNCTION__, (long)sm);
    }

    woodlawn_sm_iface = (struct ngio_intf_t *)sm;

    assert(sm);

    real_slot = woodlawn_sm_iface->slot;
    board_id = woodlawn_sm_iface->id;

    woodlawn_test_slot = woodlawn_sm_iface->slot;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf ("slot=%d board_id=%#x ", real_slot, board_id);
    }

    /* Turn on GE BP line loopback */
    if (is_plat_10gkr_capable() == FALSE) {
        ge_bp_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1);
        set_gesw_line_loopback(ge_bp_port, 1);
    }

    testname(" (SM Slot %d Woodlawn)", real_slot);
    
    fpga_init_i2c(real_slot, (void *)fpga);
    
    woodlawn_fpga_i2c.i2c_ctrl = woodlawn_sm_iface->i2c_ctrl; 
    woodlawn_uart_ctrl = woodlawn_sm_iface->uart_ctrl; 

    oir = (n2g_i2c_if_t *)woodlawn_sm_iface->oir;

    /*
     * Initialize an instance of Woodlawn data structure
     */
    woodlawn_iface_p = (woodlawn_ds_t *) &woodlawn_iface[real_slot];
    woodlawn_iface_p->board_id = board_id;
    woodlawn_iface_p->slot = real_slot;
    woodlawn_iface_p->uart = woodlawn_sm_iface->uart_ctrl;
    woodlawn_iface_p->woodlawn_sm_iface = (struct ngio_intf_t *)sm;

    /* Setup UART toward Woodlawn board */
    sprintf(tty_dev, "/dev/ttyDASH%d", woodlawn_uart_ctrl);
    if (woodlawn_uart_setup(tty_dev) == FAILED) {
        printf("\nFailed to setup UART\n");
    }

    /* Support Utah Greyhound switch 10g-kr */
    ngio_ge_cfg(woodlawn_sm_iface);

    /* Woodlawn menu's title need a fixed name via cookie_id */
    build_primary_submenu(woodlawn_submenu_tbl, WOODLAWN_SUBMENU_TABLE_SZ,
                          "Woodlawn", &woodlawn_menup);
    build_secondary_submenu(woodlawn_submenu_tbl,
                            WOODLAWN_SUBMENU_TABLE_SZ, woodlawn_secondary_items);

    /*
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    woodlawn_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    ret_val = PASS;
    if (woodlawn_sm_iface->menu_display == TRUE) {
        menu(woodlawn_menup, woodlawn_secondary_items, '\0');
    } else {
        if (woodlawn_sm_iface->test_type == IFACE_TEST) {
        } else {  /* FULL_TEST */
            do_all_menu_items(woodlawn_menup);
        }
    }

    if (woodlawn_saved_diag_exec) {
        pre_diag_exec = woodlawn_saved_diag_exec;
        woodlawn_saved_diag_exec = NULL;
    }

    if (is_plat_10gkr_capable() == FALSE) {
        /* Turn off GE BP line loopback */
        set_gesw_line_loopback(ge_bp_port, 0);
    }

    if (!((NVRAM)->diagflag & D_POWER_ON)) {
        /* Toggle bit 7 of LTC4215 */
        if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
            ret_val = FAILED;
        }
        data |= 0x80;

        if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
            ret_val = FAILED;
        }
    }
    return (ret_val);
}

/*------------------------------------------------------------------------------
 *
 * Function: woodlawn_utility_submenu().
 *
 * This function implements the Woodlawn sm test/menu
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
long woodlawn_utility_submenu (int menu_option)
{
    menu(woodlawn_util_submenup, woodlawn_util_submenu_table, '\0');

    return (PASSED);
}


/*****************************************************************
 *
 * Function: woodlawn_get_sm_ip_addr
 *
 * Description: This function returns IP Address of SM card based
 *              on slot number
 *
 * Input:  ip_addr - Buffer to put ip address
 *
 * Output: None
 *
 *****************************************************************
 */
void woodlawn_get_sm_ip_addr (char *ip_addr)
{
    /* Sanity check */
    if (ip_addr == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    sprintf(ip_addr, "%s.%d", WOODLAWN_DIAG_IP_ADDR_SUBNET,
            WOODLAWN_DIAG_IP_ADDR_BASE + woodlawn_test_slot);
}

/**********************************************************************
 *
 * Function: woodlawn_ltc_reg_read
 *
 * Wrapper for LTC4215 Register Read utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_ltc_reg_read (void)
{
    return util_oir_ltc4215_reg_read(oir);
}

/**********************************************************************
 *
 * Function: woodlawn_ltc_reg_write
 *
 * Wrapper for LTC4215 Register write utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_ltc_reg_write (void)
{
    return util_oir_ltc4215_reg_write(oir);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_reg_read
 *
 * Wrapper for FPGA Register Read utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_fpga_reg_read (void)
{
    return util_oir_fpga_reg_read(&woodlawn_fpga_i2c);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_reg_write
 *
 * Wrapper for FPGA Register write utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_fpga_reg_write (void)
{
    return util_oir_fpga_reg_write(&woodlawn_fpga_i2c);
}

/**********************************************************************
 *
 * Function: woodlawn_voltage_margin_status
 *
 * Show voltage margin status utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_voltage_margin_status (void)
{ 
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VOLTAGE_MARGIN_GET);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_voltage_margin_ctrl
 *
 * Control voltage margin  utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_voltage_margin_ctrl (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VOLTAGE_MARGIN_SET);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_1340_reg_read 
 *
 * Read 1340 reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_1340_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_1340_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_1340_reg_write 
 *
 * Write 1340 reg utility.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_1340_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_1340_REG_WRITE);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_1548_reg_read
 *
 * Read 1548 reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_1548_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_1548_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_1548_reg_write
 *
 * Write 1548 reg utility.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_1548_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_1548_REG_WRITE);

    return (PASSED);
}

#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
/**********************************************************************
 *
 * Function: woodlawn_1548_ptp_reg_read
 *
 * Read 1548 ptp reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_1548_ptp_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_1548_PTP_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_1548_ptp_reg_write
 *
 * Write 1548 ptp reg utility.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_1548_ptp_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_1548_PTP_REG_WRITE);

    return (PASSED);
}
#endif

/**********************************************************************
 *
 * Function: woodlawn_2222_reg_read
 *
 * Read 2222 reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_2222_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_2222_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_2222_reg_write
 *
 * Write 2222 reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_2222_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_2222_REG_WRITE);

    return (PASSED);
}

#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
/**********************************************************************
 *
 * Function: woodlawn_2222_ptp_reg_read
 *
 * Read 2222 ptp reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_2222_ptp_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_2222_PTP_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_2222_ptp_reg_write
 *
 * Write 2222 ptp reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_2222_ptp_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_2222_PTP_REG_WRITE);

    return (PASSED);
}
#endif

/**********************************************************************
 *
 * Function: woodlawn_sm_fpga_reg_read
 *
 * Read SM FPGA reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_sm_fpga_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_SM_FPGA_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_sm_fpga_reg_write
 *
 * Write SM FPGA reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_sm_fpga_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_SM_FPGA_REG_WRITE);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_10232_reg_read
 *
 * Read 10232 reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_10232_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_10232_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_10232_reg_write
 *
 * Write 10232 reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_10232_reg_write (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_10232_REG_WRITE);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_cavium_sgmii_status 
 *
 * Show SM cavium sgmii port status.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_cavium_sgmii_status (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CAVIUM_SGMII_PORT_STATUS);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_cavium_sgmii_config
 *
 * SM cavium sgmii port config.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_cavium_sgmii_config (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CAVIUM_SGMII_PORT_CONFIG);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_cavium_sgmii_reg_read
 *
 * Read SM cavium sgmii reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_cavium_sgmii_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CAVIUM_SGMII_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_cavium_xaui_status
 *
 * Show SM cavium xaui port status.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_cavium_xaui_status (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CAVIUM_XAUI_PORT_STATUS);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_cavium_xaui_gmx_reg_read
 *
 * Read SM cavium xaui gmx reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_cavium_xaui_gmx_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CAVIUM_XAUI_GMX_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_cavium_xaui_pcs_reg_read
 *
 * Read SM cavium xaui pcs reg utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_cavium_xaui_pcs_reg_read (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CAVIUM_XAUI_PCS_REG_READ);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_bootflash_get_info
 *
 * Get bootflash info utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_bootflash_get_info (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_BOOTFLASH_GET_INFO);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_bootflash_otp_test
 *
 * Verify bootflash OTP area utility.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long woodlawn_bootflash_otp_test (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_BOOTFLASH_OTP_TEST);

    return (PASSED);
}

#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
/**********************************************************************
 *
 * Function: woodlawn_enable_1548_ptp_engine
 *
 * Enable 88E1548 PTP Engine.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long woodlawn_enable_1548_ptp_engine (void)
{
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_ENABLE_1548_PTP_ENGINE);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: ptp_clk_trigger_verify
 *
 * Through Woodlawn PHY 1548 to verify whether the host side are provide clock
 * and trigger successfully.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long ptp_clk_trigger_verify (void)
{
    if (woodlawn_init_3036x == FALSE) {
        /* Clear the timing card initialized flag */
        clear_timingcard_init_flag();

        /* Initialize the timing card */
        if (timingcard_init_seq() == FAILED) {
            cterr('f', 0, "Initialize the timing card fail");
            return (FAILED);
        }
        woodlawn_init_3036x = TRUE;
    }

    /* Set up the clock path - Overlord -> Timing card -> Woodlawn */
    if (clock_direction_lib((uint)woodlawn_test_slot) == FAILED) {
        return (FAILED);
    }

    /* 
     *  Enable 1pps drift adjustment mode script and to 
     *  verify whether 1548 clock and trigger are work 
     *  successfully through nc command 
     */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_1548_PTP);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl1548_clk_trig_in_verify
 *
 * Through Woodlawn PHY 1548 to verify whether the host side are provide clock
 * and trigger successfully.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long mrvl1548_clk_trig_in_verify (void)
{
    int test_slot = woodlawn_test_slot;

    /* Check if firmware has been downloaded to the SM card */
    if (is_woodlawn_up(FALSE) == FALSE) {
        printf("*** Need to download the firmware prior to run this test!\n");
        return (FAILED);
    }
    
    /* Power up timing card */
    if (woodlawn_init_3036x == FALSE) {
        if (timingcard_power_on(FIRST_SLOT) == FAILED) {
            printf("*** Power on timing card failed\n");
            return (FAILED);
        }

        /* Clear the timing card initialized flag */
        clear_timingcard_init_flag();

        /* Initialize the timing card */
        if (timingcard_init_seq() == FAILED) {
            printf("*** Initialize the timing card fail\n");
            return (FAILED);
        }
        woodlawn_init_3036x = TRUE;
    }

    /* Set up the clock path - Overlord -> Timing card -> Woodlawn */
    if (clock_direction_lib((uint)test_slot) == FAILED) {
        printf("*** Setup clock path failed\n");
        return (FAILED);
    }

    /* 
     *  Enable 1pps drift adjustment mode script and to 
     *  verify whether 1548 clock and trigger are work 
     *  successfully through nc command 
     */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_1548_CLK_TRIG_IN);

    /* Power-off timing card */
    if (timingcard_power_off(FIRST_SLOT) == FAILED) {
        printf("*** Power-off timing card failed\n");
        return (FAILED);
    }

    woodlawn_init_3036x = FALSE;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl1548_clk_trig_out_verify
 *
 * Through Woodlawn PHY 1548 to verify whether the host side are provide clock
 * and trigger successfully.
 *
 * Note: Need to upgrade FPGA firmware version first. After FPGA firmware
 *       upgrade, please power cycle Woodlawn and check FPGA version
 *       06.09.9b for 4GE ports.
 *       06.09.8b for 6GE ports.
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long mrvl1548_clk_trig_out_verify (void)
{
    int slot_id, test_slot, verify_slot, temp_slot;
    int ge_num, ge_cnt;
    char *clk_mux;

    /* Check if firmware has been downloaded to the SM card */
    if (is_woodlawn_up(FALSE) == FALSE) {
        printf("*** Need to download the firmware prior to run this test!\n");
        return (FAILED);
    }

    slot_id = get_sku_id();

    test_slot = woodlawn_test_slot;

    /* Verify slot is SM2 if the current slot is SM1, and vice versa */
    verify_slot = (MAX_SM + FIRST_SLOT) - (test_slot);

    /* Get the number of 88E1548P based on SKU ID */
    if (slot_id == WOODLAWN_6GE) {
        ge_cnt = 2;
    } else if (slot_id == WOODLAWN_4GE_1XAUI) {
        ge_cnt = 1;
    }

    /* Power up Woodlawn at the other slot and boot up image */
    if (woodlawn_boot_up_other_slot(verify_slot) == FAILED) {
        printf("*** Boot up on slot %d failed\n", verify_slot);
        return (FAILED);
    }
    
    /* Setup O2 FPGA clock and trigger routing */
    if (woodlawn_clk_direction(verify_slot) == FAILED) {
        printf("*** O2 FPGA routing setup failed\n");
        return (FAILED);
    }

    /* Config test slot to generate 8k clock out,
     * and verify at other slot.
     */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CONFIG_1548_GEN_CLK_OUT);

    for (ge_num = 0; ge_num < ge_cnt; ge_num++) {
        printf("\nVerifying Clock out on 88E1548P PHY-%d...\n", ge_num);
        /* Config Woodlawn FPGA Clock Mux to seclect GE0/GE1 */
        if (ge_num == 0) {
            clk_mux = DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_GE0;
        } else {
            clk_mux = DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_GE1;
        }
        woodlawn_nc_dispatch_comm(clk_mux);

        /* Verify the clock out in other slot 
         * FPGA DEV_STATUS_REG(0x0E)
         */
        temp_slot = woodlawn_test_slot;
        woodlawn_test_slot = verify_slot;
        woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_FPGA_SYNC_CLK_OUT);
        woodlawn_test_slot = temp_slot;
    }

    /* Config test slot to generate 1PPS trigger out,
     * and verify at other slot.
     */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CONFIG_1548_GEN_TRIG_OUT);

    for (ge_num = 0; ge_num < ge_cnt; ge_num++) {
        printf("\nVerifying Trigger out on 88E1548P PHY-%d...\n", ge_num);
        /* Config Woodlawn FPGA Trigger Mux to seclect GE0/GE1 */
        if (ge_num == 0) {
            clk_mux = DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_GE0;
        } else {
            clk_mux = DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_GE1;
        }
        woodlawn_nc_dispatch_comm(clk_mux);

        /* Verify the clock out in other slot 
         * FPGA TRIG_STATUS_REG(0xA1)
         */
        temp_slot = woodlawn_test_slot;
        woodlawn_test_slot = verify_slot;
        woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_FPGA_SYNC_TRIG_OUT);
        woodlawn_test_slot = temp_slot;
    }

    /* Power-off slot2 Woodlawn */
    if (woodlawn_power_off_other_slot(verify_slot) == FAILED ) {
        printf("*** Power-off Woodlawn on slot %d failed\n", verify_slot);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl2222_clk_trig_in_verify
 *
 * Through Woodlawn PHY 2222 to verify whether the host side are provide clock
 * and trigger successfully.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long mrvl2222_clk_trig_in_verify (void)
{
    int test_slot = woodlawn_test_slot;

    /* Check if firmware has been downloaded to the SM card */
    if (is_woodlawn_up(FALSE) == FALSE) {
        printf("*** Need to download the firmware prior to run this test!\n");
        return (FAILED);
    }

    /* Power up timing card */
    if (woodlawn_init_3036x == FALSE) {
        if (timingcard_power_on(FIRST_SLOT) == FAILED) {
            printf("*** Power on timing card failed\n");
            return (FAILED);
        }

        /* Clear the timing card initialized flag */
        clear_timingcard_init_flag();

        /* Initialize the timing card */
        if (timingcard_init_seq() == FAILED) {
            printf("*** Initialize the timing card fail");
            return (FAILED);
        }
        woodlawn_init_3036x = TRUE;
    }

    /* Set up the clock path - Overlord -> Timing card -> Woodlawn */
    if (clock_direction_lib((uint)test_slot) == FAILED) {
        printf("*** Setup clock path failed\n");
        return (FAILED);
    }

    /* 
     *  Enable 1pps drift adjustment mode script and to 
     *  verify whether 1548 clock and trigger are work 
     *  successfully through nc command 
     */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_2222_CLK_TRIG_IN);

    /* Power-off timing card */
    if (timingcard_power_off(FIRST_SLOT) == FAILED) {
        printf("*** Power-off timing card failed\n");
        return (FAILED);
    }

    woodlawn_init_3036x = FALSE;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl2222_clk_trig_out_verify
 *
 * Through Woodlawn PHY 2222 to verify whether the host side are provide clock
 * and trigger successfully.
 *
 * Note: Need to upgrade FPGA firmware version first. After FPGA firmware
 *       upgrade, please power cycle Woodlawn and check FPGA version
 *       06.09.9b for 4GE ports.
 *       06.09.8b for 6GE ports.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long mrvl2222_clk_trig_out_verify (void)
{
    int test_slot, verify_slot, temp_slot;

    /* Check if firmware has been downloaded to the SM card */
    if (is_woodlawn_up(FALSE) == FALSE) {
        printf("*** Need to download the firmware prior to run this test!\n");
        return (FAILED);
    }

    test_slot = woodlawn_test_slot;

    /* Verify slot is SM2 if the current slot is SM1, and vice versa */
    verify_slot = (MAX_SM + FIRST_SLOT) - (test_slot);

    /* Power up Woodlawn at the other slot and boot up image */
    if (woodlawn_boot_up_other_slot(verify_slot) == FAILED) {
        printf("*** Boot up on slot %d failed\n", verify_slot);
        return (FAILED);
    }
    
    /* Setup O2 FPGA clock and trigger routing */
    if (woodlawn_clk_direction(verify_slot) == FAILED) {
        printf("*** O2 FPGA routing setup failed\n");
        return (FAILED);
    }

    /* Config test slot to generate 8kHz clock out */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CONFIG_2222_GEN_CLK_OUT);

    printf("\nVerifying Clock out on 88X2222P...\n");

    /* Config Woodlawn FPGA Clock Mux to select 88X2222P */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_X2222P);

    /* Verify the clock out in other slot 
     * FPGA DEV_STATUS_REG(0x0E)
     */
    temp_slot = woodlawn_test_slot;
    woodlawn_test_slot = verify_slot;
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_FPGA_SYNC_CLK_OUT);
    woodlawn_test_slot = temp_slot;

    /* Config test slot to generate 1PPS trigger out,
     * and verify at other slot.
     */
    /* Config test slot to generate 1PPS trigger out */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CONFIG_2222_GEN_TRIG_OUT);

    printf("\nVerifying 1PPS Trigger out on 88X2222P...\n");

    /* Config Woodlawn FPGA Trigger Mux to select 88X2222P */
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_X2222P);

    /* Verify the trigger out in other slot 
     * FPGA TRIG_STATUS_REG(0xA1)
     */
    temp_slot = woodlawn_test_slot;
    woodlawn_test_slot = verify_slot;
    woodlawn_nc_dispatch_comm(DIAG_COMMAND_VERIFY_FPGA_SYNC_TRIG_OUT);
    woodlawn_test_slot = temp_slot;

    /* Power-off Woodlawn in other slot */
    if (woodlawn_power_off_other_slot(verify_slot) == FAILED ) {
        printf("*** Power-off Woodlawn on slot %d failed\n", verify_slot);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_clk_direction
 *
 * Set up the clock path - Woodlawn test slot -> Dash FPGA -> Woodlawn verify slot
 * Configures the O2 dash fpga SYNC/TRIG Control Register
 * to select the clock frequency that outputs to NGSM card. 
 * Input : clk_dest - destination ngsm slot number
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long woodlawn_clk_direction (uint clk_dest)
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out_trig_out;
    uint32_t value, source_clk = 0, source_trig = 0;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    if (clk_dest == NGSM_SLOT_ONE) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGSM1_SYNC_OUT_SYNC_TRIG_OUT);
        source_clk = DASH_FPGA_SYNC_OUT_NGSM2;
        source_trig = DASH_FPGA_SYNC_OUT_NGSM2;
    } else if (clk_dest == NGSM_SLOT_TWO) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGSM2_SYNC_OUT_SYNC_TRIG_OUT);
        source_clk = DASH_FPGA_SYNC_OUT_NGSM1;
        source_trig = DASH_FPGA_SYNC_OUT_NGSM1;
    } else {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             QUAD_PHY_SYNC_TRIG_CTRL_REG);
    }

    
    /* Set Dash FPGA SYNC/TRIG Control Register as NGVM output.
     * Enable the SYNC_OUT, NGSM1/NGSM2 
     */
    value = (DASH_FPGA_TRIG_OUTPUT_EN | (source_trig << 16) |
             DASH_FPGA_SYNC_OUTPUT_EN | source_clk);
    
    *sync_out_trig_out = value;

    return (PASSED);
}
#endif

/**********************************************************************
 *
 * Function: woodlawn_show_diag_version
 *
 * Display diagnostic version.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_show_diag_version (void)
{
    int rc, ix, ping_timeout;    

    /* Reach SM card through ping */
    rc = FAILED;
    ping_timeout = WOODLAWN_DIAG_PROMPT_TOUT;
    for (ix = 0; ix < ping_timeout; ix++) {
        /* Start pinging SM card */
        if (is_woodlawn_up(FALSE) == TRUE) {
            printf("\nWoodlawn Image is up!\n");
            fflush(stdout);
            rc = PASSED;
            break;
        }
        sleep(1);
    }

    if (rc != PASSED) {
        printf("\nUnable to reach Woodlawn firmware! do firmware download now...\n");
        fflush(stdout);
        /* do firmware download */
        rc = woodlawn_boot_image(0);
    } else {
        /* Display version */
        printf("Diagnostic version:\n");
        woodlawn_transmit_nc_request(DIAG_SEND_DIAG_VER_PORT_BASE);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: power_on_woodlawn_card
 *
 * Description: This function power on Woodlawn NGSM.
 *
 * Input : none 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long power_on_woodlawn_card (void)
{
    int real_slot;
    struct ngio_intf_t *ngio;
    uint8_t data = 0;

    real_slot = woodlawn_sm_iface->slot;
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(real_slot);

    ngio->on(ngio);
    ngio->i2c_unreset(ngio);
    ngio->uart_on(ngio);

    printf("Powering Woodlawn SM Slot-%d...", woodlawn_test_slot);
    fflush(stdout);

    /* Pull high GPIO3 on LTC4215 */
    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }
    data &= ~(0x80);

    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    msleep(100);

    ngio->unreset(ngio);

    printf("OK\n");
    fflush(stdout);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: power_off_woodlawn_card
 *
 * Description: This function power off Woodlawn NGSM.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long power_off_woodlawn_card (void)
{
    int real_slot;
    struct ngio_intf_t *ngio;
    uint8_t data = 0;

    real_slot = woodlawn_sm_iface->slot;
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(real_slot);

    printf("Power off Woodlawn SM Slot-%d...", woodlawn_test_slot);
    fflush(stdout);

    /* Pull high GPIO3 on LTC4215 */
    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    data |= 0x80;

    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    ngio->off(ngio);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_show_power_status
 *
 * Display SM card power status.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_show_power_status (void)
{
    int real_slot, rv;
    struct ngio_intf_t *intf, *ngio;
    ngio_t *ngio_pwr = (ngio_t *)(dash_fpga + NGIO_BASE);
    ng_t *io_pwr_en;
    uint8_t data = 0, show_data;

    real_slot = woodlawn_sm_iface->slot;
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(real_slot);

    intf = ngio;
    assert(intf->slot);
    msleep(10);

    io_pwr_en = (ng_t *)&ngio_pwr->sm[intf->slot-FIRST_SLOT];

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    show_data = data;
    data &= (0x80);

    if ((io_pwr_en->ctrl & NGIO_PWR_EN) && (io_pwr_en->ctrl & NGIO_PWR_OK) &&
        (io_pwr_en->ctrl & NGIO_UART_TX) && (!(io_pwr_en->ctrl & NGIO_RESET)) &&
        data == 0) {
        rv = PASSED;
    } else {
        rv = FAILED;
    }

    printf("Power of SM Slot-%d is ", real_slot);
    if (rv == PASSED) {
        printf("ON\n");
    } else {
        printf("OFF\n");
        printf("NGIO status/ctrl reg=%#.8x\n", io_pwr_en->ctrl);
        printf("LTC4215 FAULT REG val=%x\n", show_data); 
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: led_all_test
 *
 * Description: This test allows the operator to turn on and off all the
 *                    LEDs at the same time
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************
 */
long led_all_test (void)
{
    int led_display;
    char cmd[64];
    int on = 1, off = 0, ix, sku_id; 
    uint8_t data = 0;
    int four_ge_sku[] = {DIAG_LED_SET_GE0_SPD,
                         DIAG_LED_SET_GE0_LINK,
                         DIAG_LED_SET_SFP2_EN, 
                         DIAG_LED_SET_SFP2_S,     
                         DIAG_LED_SET_GE1_SPD,
                         DIAG_LED_SET_GE1_LINK,
                         DIAG_LED_SET_SFP3_EN,
                         DIAG_LED_SET_SFP3_S,
                         DIAG_LED_SET_GE2_SPD,
                         DIAG_LED_SET_GE2_LINK,
                         DIAG_LED_SET_SFP4_EN,
                         DIAG_LED_SET_SFP4_S,
                         DIAG_LED_SET_GE3_SPD,
                         DIAG_LED_SET_GE3_LINK,
                         DIAG_LED_SET_SFP5_EN,
                         DIAG_LED_SET_SFP5_S};
#if 0
    int four_ge_sku[] = {0, 1, 10, 11, 4, 5, 14, 15,  
                    8, 9, 18, 19, 12, 13, 22, 23};  
#endif

    sku_id = get_sku_id();

    led_display = getdec_answer("Turn LEDs ON = 1, Off = 0", 0, 0, 1);
     
    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    if (led_display) {
        data |= 0x40;

        if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
            printf("Fail\n");
            fflush(stdout);
            return (FAILED);
        }        
    } else {
        data &= 0xbf;
        if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
            printf("Fail\n");
            fflush(stdout);
            return (FAILED);
        }
    }

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        if (led_display) {
            for (ix = DIAG_LED_SET_GE0_SPD; ix <= DIAG_LED_SET_SFP3_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, four_ge_sku[ix], on);
                woodlawn_nc_dispatch_comm(cmd);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
            }
        } else {
            for (ix = DIAG_LED_SET_GE0_SPD; ix <= DIAG_LED_SET_SFP3_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, four_ge_sku[ix], off);
                woodlawn_nc_dispatch_comm(cmd);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {  
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
            }
        }  
    } else if (sku_id == WOODLAWN_6GE) {
        if (led_display) {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
            }
        } else {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
            }
        }
    } else {
        if (led_display) {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {    
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
            }
        } else {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {                     
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
            }
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: led_seq_test
 *
 * Description: This test turns on/off each LED sequentially.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************
 */
long led_seq_test (void)
{
    int sku_id, delay_time, led_display;
    int ix, on = 1, off = 0;
    char cmd[64];
    uint8_t data = 0;
    int four_ge_sku[] = {DIAG_LED_SET_GE0_SPD,
                         DIAG_LED_SET_GE0_LINK, 
                         DIAG_LED_SET_SFP2_EN,  
                         DIAG_LED_SET_SFP2_S,      
                         DIAG_LED_SET_GE1_SPD,
                         DIAG_LED_SET_GE1_LINK,
                         DIAG_LED_SET_SFP3_EN, 
                         DIAG_LED_SET_SFP3_S, 
                         DIAG_LED_SET_GE2_SPD,
                         DIAG_LED_SET_GE2_LINK, 
                         DIAG_LED_SET_SFP4_EN, 
                         DIAG_LED_SET_SFP4_S,
                         DIAG_LED_SET_GE3_SPD,
                         DIAG_LED_SET_GE3_LINK, 
                         DIAG_LED_SET_SFP5_EN, 
                         DIAG_LED_SET_SFP5_S};

    delay_time = getdec_answer("Select delay time(unit:ms)", 0, 0, 100000);
    led_display = getdec_answer("Sequential turn LEDs ON = 1, Off = 0", 0, 0, 1);

    sku_id = get_sku_id();

    if (led_display) {
        data |= 0x40;

        if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
            printf("Fail\n");
            fflush(stdout);
            return (FAILED);
        }
    } else {
        data &= 0xbf;
        if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
            printf("Fail\n");
            fflush(stdout);
            return (FAILED);
        }
    }

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        if (led_display) {
            for (ix = DIAG_LED_SET_GE0_SPD; ix <= DIAG_LED_SET_SFP3_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, four_ge_sku[ix], on);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }
        } else {
            for (ix = DIAG_LED_SET_GE0_SPD; ix <= DIAG_LED_SET_SFP3_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, four_ge_sku[ix], off);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }
        }
    } else if (sku_id == WOODLAWN_6GE) {
        if (led_display) {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }
        } else {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }
        }
    } else {
        if (led_display) {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, on);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }
        } else {
            for (ix = 0; ix <= DIAG_LED_SET_SFP5_S; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }

            for (ix = DIAG_LED_SET_SFP_PLUS_EN; ix <= DIAG_LED_SET_SFP_PLUS_SPD; ix++) {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, ix, off);
                woodlawn_nc_dispatch_comm(cmd);
                msleep(delay_time);
            }
        }
    } 

    return (PASSED);
}

/**********************************************************************
 *
 * Function: led_multiple_test
 *
 * Description: This test allows the operator to turn on different
 *                   combinations of the LEDs
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************
 */
long led_multiple_test (void)
{
    int ix = 0, sku_id, led_flag[53] = {0};
    int led_display, index;
    char cmd[64];
    int on = 1, off = 0;
    uint8_t data = 0;

    led_display = getdec_answer("Turn multiple LEDs ON = 1, Off = 0", 0, 0, 1);

    sku_id = get_sku_id();

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        print_four_ge_port_led();
        printf("LED_SFP_PLUS_EN   => 50\n");
        printf("LED_SFP_PLUS_SPD  => 51\n");

        do {
            led_flag[ix] = getdec_answer("\nSelect number to ctrl LED :", 0, 0, 52);
            index = ix;
            ix ++;
        } while (getc_answer("Continue - (y/n)", "yn", 'y') == 'y');

        if (led_display) {
            for (ix = 0; ix <= index; ix++) {
                if (led_flag[ix] == 52) {
                    data |= 0x40;
                    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                        printf("Fail\n");
                        fflush(stdout);
                        return (FAILED);
                    }
                } else {
                    sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag[ix], on);
                    woodlawn_nc_dispatch_comm(cmd);
                }
            }
        } else {
            for (ix = 0; ix <= index; ix++) {
                if (led_flag[ix] == 52) {
                    data &= 0xbf;
                    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                        printf("Fail\n");
                        fflush(stdout);
                        return (FAILED);
                    }
                } else { 
                    sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag[ix], off);
                    woodlawn_nc_dispatch_comm(cmd);
                }
            }
        }
    } else if (sku_id == WOODLAWN_6GE) {
        print_common_led();
        printf("LED_GE4_SPD       => 16\n");
        printf("LED_GE4_LINK      => 17\n");
        printf("LED_SFP4_EN       => 18\n");
        printf("LED_SFP4_S        => 19\n");
        printf("LED_GE5_SPD       => 20\n");
        printf("LED_GE5_LINK      => 21\n");
        printf("LED_SFP5_EN       => 22\n");
        printf("LED_SFP5_S        => 23\n");

        do {
            led_flag[ix] = getdec_answer("\nSelect number to ctrl LED :", 0, 0, 52);
            index = ix;
            ix++;
        } while (getc_answer("Continue - (y/n)", "yn", 'y') == 'y');

        if (led_display) {
            for (ix = 0; ix <= index; ix++) {
                if (led_flag[ix] == 52) {
                    data |= 0x40;
                    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                        printf("Fail\n");
                        fflush(stdout);
                        return (FAILED);
                    }
                } else {
                    sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag[ix], on);
                    woodlawn_nc_dispatch_comm(cmd);
                }
            }
        } else {
            for (ix = 0; ix <= index; ix++) {
                if (led_flag[ix] == 52) {
                    data &= 0xbf;
                    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                        printf("Fail\n");
                        fflush(stdout);
                        return (FAILED);
                    }
                } else {
                    sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag[ix], off);
                    woodlawn_nc_dispatch_comm(cmd);
                }
            }
        }
    } else {
        print_common_led();
        printf("LED_GE4_SPD       => 16\n");
        printf("LED_GE4_LINK      => 17\n");
        printf("LED_SFP4_EN       => 18\n");
        printf("LED_SFP4_S        => 19\n");
        printf("LED_GE5_SPD       => 20\n");
        printf("LED_GE5_LINK      => 21\n");
        printf("LED_SFP5_EN       => 22\n");
        printf("LED_SFP5_S        => 23\n");
        printf("LED_SFP_PLUS_EN   => 50\n");
        printf("LED_SFP_PLUS_SPD  => 51\n");

        do {
            led_flag[ix] = getdec_answer("\nSelect number to ctrl LED :", 0, 0, 52);
            index = ix;
            ix++;
        } while (getc_answer("Continue - (y/n)", "yn", 'y') != 'y');

        if (led_display) {
            for (ix = 0; ix <= index; ix++) {
                if (led_flag[ix] == 52) {
                    data |= 0x40;
                    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                        printf("Fail\n");
                        fflush(stdout);
                        return (FAILED);
                    }
                } else {
                    sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag[ix], on);
                    woodlawn_nc_dispatch_comm(cmd);
                }
            }
        } else {
            for (ix = 0; ix <= index; ix++) {
                if (led_flag[ix] == 52) {
                    data &= 0xbf;
                    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                        printf("Fail\n");
                        fflush(stdout);
                        return (FAILED);
                    }
                } else {
                    sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag[ix], off);
                    woodlawn_nc_dispatch_comm(cmd);
                }
            }
        }
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: led_single_test
 *
 * Description: This test allows the operator to turn on different
 *                   combinations of the LEDs
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************
 */
long led_single_test (void)
{
    int sku_id, led_flag = 0;
    int led_display, on = 1, off = 0;
    char cmd[64];
    uint8_t data = 0;

    led_display = getdec_answer("Turn multiple LEDs ON = 1, Off = 0", 0, 0, 1);

    sku_id = get_sku_id();

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        print_four_ge_port_led();
        printf("DIAG_LED_SET_SFP_PLUS_EN => 50\n");
        printf("DIAG_LED_SET_SFP_PLUS_SPD => 51\n");

        led_flag = getdec_answer("\nSelect number to ctrl LED :", 0, 0, 52);

        if (led_display) {
            if (led_flag == 52) {
                data |= 0x40;
                if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                    printf("Fail\n");
                    fflush(stdout);
                    return (FAILED);
                }
            } else {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag, on);
                woodlawn_nc_dispatch_comm(cmd);
            }
        } else {
            if (led_flag == 52) {
                data &= 0xbf;
                if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                    printf("Fail\n");
                    fflush(stdout);
                    return (FAILED);
                }
            } else {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag, off);
                woodlawn_nc_dispatch_comm(cmd);
            }
        }
    } else if (sku_id == WOODLAWN_6GE) {
        print_common_led();
        printf("LED_GE4_SPD       => 16\n");
        printf("LED_GE4_LINK      => 17\n");
        printf("LED_SFP4_EN       => 18\n");
        printf("LED_SFP4_S        => 19\n");
        printf("LED_GE5_SPD       => 20\n");
        printf("LED_GE5_LINK      => 21\n");
        printf("LED_SFP5_EN       => 22\n");
        printf("LED_SFP5_S        => 23\n");

        led_flag = getdec_answer("\nSelect number to ctrl LED :", 0, 0, 23);

        if (led_display) {
            if (led_flag == 52) {
                data |= 0x40;
                if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                    printf("Fail\n");
                    fflush(stdout);
                    return (FAILED);
                }
            } else {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag, on);
                woodlawn_nc_dispatch_comm(cmd);
            }
        } else {
            if (led_flag == 52) {
                data &= 0xbf;
                if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                    printf("Fail\n");
                    fflush(stdout);
                    return (FAILED);
                }
            } else {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag, off);
                woodlawn_nc_dispatch_comm(cmd);
            }
        }
    } else {
        print_common_led();
        printf("LED_GE4_SPD       => 16\n");
        printf("LED_GE4_LINK      => 17\n");
        printf("LED_SFP4_EN       => 18\n");
        printf("LED_SFP4_S        => 19\n");
        printf("LED_GE5_SPD       => 20\n");
        printf("LED_GE5_LINK      => 21\n");
        printf("LED_SFP5_EN       => 22\n");
        printf("LED_SFP5_S        => 23\n");
        printf("LED_SFP_PLUS_EN   => 50\n");
        printf("LED_FP_PLUS_SPD   => 51\n");

        led_flag = getdec_answer("\nSelect number to ctrl LED :", 0, 0, 25);

        if (led_display) {
            if (led_flag == 52) {
                data |= 0x40;
                if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                    printf("Fail\n");
                    fflush(stdout);
                    return (FAILED);
                }
            } else {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag, on);
                woodlawn_nc_dispatch_comm(cmd);
            }
        } else {
             if (led_flag == 52) {
                data &= 0xbf;
                if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
                    printf("Fail\n");
                    fflush(stdout);
                    return (FAILED);
                }
            } else {
                sprintf(cmd, "%s,%d:%d", DIAG_COMMAND_LED_SET, led_flag, off);
                woodlawn_nc_dispatch_comm(cmd);
            }
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: print_common_led
 *
 * Display common LED options.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long print_common_led (void)
{
    printf("LED_GE0_SPD       => 0\n");
    printf("LED_GE0_LINK      => 1\n");
    printf("LED_SFP0_EN       => 2\n");
    printf("LED_SFP0_S        => 3\n");
    printf("LED_GE1_SPD       => 4\n");
    printf("LED_GE1_LINK      => 5\n");
    printf("LED_SFP1_EN       => 6\n");
    printf("LED_SFP1_S        => 7\n");
    printf("LED_GE2_SPD       => 8\n");
    printf("LED_GE2_LINK      => 9\n");
    printf("LED_SFP2_EN       => 10\n");
    printf("LED_SFP2_S        => 11\n");
    printf("LED_GE3_SPD       => 12\n");
    printf("LED_GE3_LINK      => 13\n");
    printf("LED_SFP3_EN       => 14\n");
    printf("LED_SFP3_S        => 15\n");
    printf("LED_EN            => 52\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: print_four_ge_port_led
 *
 * Display LED options for four ge port sku.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long print_four_ge_port_led (void)
{
    printf("LED_GE0_SPD       => 0\n");
    printf("LED_GE0_LINK      => 1\n");
    printf("LED_SFP0_EN       => 10\n");
    printf("LED_SFP0_S        => 11\n");
    printf("LED_GE1_SPD       => 4\n");
    printf("LED_GE1_LINK      => 5\n");
    printf("LED_SFP1_EN       => 14\n");
    printf("LED_SFP1_S        => 15\n");
    printf("LED_GE2_SPD       => 8\n");
    printf("LED_GE2_LINK      => 9\n");
    printf("LED_SFP2_EN       => 18\n");
    printf("LED_SFP2_S        => 19\n");
    printf("LED_GE3_SPD       => 12\n");
    printf("LED_GE3_LINK      => 13\n");
    printf("LED_SFP3_EN       => 22\n");
    printf("LED_SFP3_S        => 23\n");
    printf("LED_EN            => 52\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_ltc_reg_test
 *
 * Wrapper for LTC4215 Register test.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_ltc_reg_test (void)
{
    int rc;

    testname("LTC4215 OIR Register");

    rc = oir_ltc4215_register_test(oir);

    prcomplete(testpass, errcount, (char *)0);

    return rc? FAILED: PASSED;
}

/**********************************************************************
 *
 * Function: xaui_lpbk_test
 *
 * Setup GE switch to line loopback mode
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long xaui_lpbk_test (void)
{
    int port, onoff, slot, local_port;
    int tgt_device = TGT_DEV_NGSM;
    uchar cin = 's'; /* setup line loopback */

    slot = woodlawn_test_slot;
    local_port = 2; /* xaui port */

    cin = getc_answer("Set or clear line loopback setting(enter s or c)", "sc", 'c');
    port = ovld_get_ge_sw_port_num(slot, tgt_device, local_port);
    onoff = (cin == 's');
    
    set_gesw_line_loopback(port, onoff);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_gesw_lpbk_set
 *
 * Description: This function is the wrapper to enables or disable the line
 *              loopback of the GE switch port connected to the Canis.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_gesw_lpbk_set (void)
{
    uint8_t ans;
    uint port;
    int ge_port, ge_port0, ge_port1;
    int real_slot = woodlawn_iface_p->slot;

    printf("\n");
    testname("Switch the Overlord GESW");

    port = gethex_answer("Overlord GESW port E0/E1", 0, 0, 1);
    if (port) {
        ge_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1); //BP PHY
    } else {
        ge_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0); //Serdes Mux
    }
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
    printf("\nSM slot %d, E0 : GESW port %d = %s",real_slot, ge_port0 ,
            get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
    printf("\nSM slot %d, E1 : GESW port %d = %s\n",real_slot, ge_port1 ,
            get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
    fflush(0);

    return(PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: woodlawn_sm_console_switch().
 *
 * This function provides console redirect for woodlawn sm
 *
 * Input: show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int woodlawn_sm_console_switch (int show_menu)
{
    woodlawn_ds_t *iface;
    const int maxlen = 128;
    char cmd[maxlen];

    iface = woodlawn_iface_p;
    assert(iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); // pause a second for the NOTE:

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             woodlawn_uart_ctrl);

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return(PASSED);
}


/**********************************************************************
 *
 * Function: woodlawn_power_on
 *
 * Description: This function power on Woodlawn NGSM.
 *
 * Input :  sm_iface - NGSM data structure
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int woodlawn_power_on (struct ngio_intf_t *sm_iface)
{
    uint8_t  data = 0;

    sm_iface->on(sm_iface);
    sm_iface->i2c_unreset(sm_iface);
    sm_iface->uart_on(sm_iface);

    printf("Powering Woodlawn SM Slot-%d now ...", woodlawn_test_slot);
    fflush(stdout);

    /* Pull high GPIO3 on LTC4215 */
    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }
    data &= ~(0x80);

    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
        printf("Fail\n");
        fflush(stdout);
        return (FAILED);
    }
    
    msleep(100);

    sm_iface->unreset(sm_iface);

    printf("OK\n");
    fflush(stdout);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_pcie_test
 *
 * This function checks if Octeon is discovered by PCIe enumeration
 *
 * Input : show_menu - Not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_pcie_test (int show_menu)
{
    testname("Woodlawn PCIe");
    prpass(testpass, "Check Cavium PCIe");
    if (woodlawn_discover_pcie(WOODLAWN_CVMX_VEND_ID, WOODLAWN_CVMX_DEV_ID) ==
                               FAILED) {
        cterr('f', 0, "Woodlawn PCIe is not enumerated");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
*
* Function: ge_bp_lpbk_test
*
* Perform GE loopback test to verify GE0/GE1 backplane connectivity between
* host and SM card
*
* Input : none
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
static long ge_bp_lpbk_test (int port)
{
    int rc = PASSED;
    int packet_no = WOODLAWN_GE_BP_PACKET_NO;
    int ix;
    char tty_dev[32];
    int ctrl_plane_sgmii_port;

    /* Setup UART toward Woodlawn board */
    sprintf(tty_dev, "/dev/ttyDASH%d", woodlawn_uart_ctrl); 

    /* Escape to default shell by typing 'ESC' and 'Enter' */
    for (ix = 0; ix < 5; ix++) {
        woodlawn_tx_uart(tty_dev, WOODLAWN_ESC_CR_STRING);
    }

    msleep(500);

    if (port == 0) {
        testname(" GE0 Backplane Loopback");
        prpass(testpass, "Setting TLK10232 loopback bit");
    } else {
        testname(" GE1 Backplane Loopback");
        prpass(testpass, "Setting Marvell_1112 loopback bit");
    }

    /* Turn on lpbk bit */
    if (port == 0) {
        woodlawn_tx_uart(tty_dev, WOODLAWN_TURN_GE_LPBK);
    } else {
        woodlawn_tx_uart(tty_dev, WOODLAWN_TURN_ON_GE1_LPBK);
    }

    prpass(testpass, "Running GE%d loopback test now", port);

    ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port();

    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, packet_no) != PASSED) {
        cterr('f', 0, "GE%d loopback from Host side %d fails.",
        port, ctrl_plane_sgmii_port);
        rc = (FAILED);
    } else {
        if (port == 0) {
            prpass(testpass, "GE0 backplane loopback pass");
        } else {
            prpass(testpass, "GE1 backplane loopback pass");
        }
    }

    if (port == 0) {
        /* Turn off GE0 lpbk bit */
        woodlawn_tx_uart(tty_dev, WOODLAWN_TURN_OFF_GE0_LPBK);
    } else {
        /* Turn off GE1 lpbk bit */
        woodlawn_tx_uart(tty_dev, WOODLAWN_TURN_OFF_GE1_LPBK);
    } 

    return (rc);
}

/**********************************************************************
 *
 * Function: woodlawn_run_sm_test
 *
 * This function runs all SM tests
 *
 * Input : show_menu - Not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_run_sm_test (int show_menu)
{
    return (woodlawn_do_all());
}

/**********************************************************************
 *
 * Function: woodlawn_boot_image
 *
 * This function interrupts Uboot, download image through TFTP, and
 * load the image
 *
 * Input : show_menu - Not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_boot_image (int show_menu)
{
    int ix, rc, boot_timeout, ping_timeout;
    char tty_dev[32];
    char load_linux_str[256];
    char cmd[32];
    struct stat sts;
    char ext_lpbk_flag[16], stop_on_err_flag[16];
    char min_test_flag[16], verbose_flag[16];

    /* Boot from woodlawn upgrad sector */
    woodlawn_boot_select();

    /* Download image from the network for the first time  */
    if (stat(WOODLAWN_DEST_DIAG_IMG, &sts) == -1) {
        if (tftp_get(0, WOODLAWN_SRC_DIAG_IMG, 0, WOODLAWN_DEST_DIAG_IMG, 0) < 0) {
            sprintf(cmd, "rm -f %s", WOODLAWN_DEST_DIAG_IMG);
            system(cmd);
            fflush(stdout);
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);
        }
    }

    /* Power up Woodlawn board now */
    if (woodlawn_power_on(woodlawn_sm_iface) == FAILED) {
        cterr('f', 0, "Failed to power up Woodlawn board");
        return (FAILED);
    }

    msleep(WOODLAWN_POWER_UP_DELAY);
    
    /* Setup UART toward Woodlawn board */
    sprintf(tty_dev, "/dev/ttyDASH%d", woodlawn_uart_ctrl); 

    /* Polling if bootloader is up, we need to fire tftpdnld command
     * through UART interface since Woodlawn bootloader doesn't boot
     * linux by default. (It boots up SE instead)
     */
    printf("Looking for bootloader prompt (1)...");
    fflush(stdout);

    boot_timeout = WOODLAWN_BL_PROMPT_TOUT;
    do {
        /* Transmit New Line */
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

        if (woodlawn_rx_polling_uart(tty_dev, WOODLAWN_BL_PROMPT, 100) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "Failed to get '%s' bootloader prompt", WOODLAWN_BL_PROMPT);
        return (FAILED);
    }

    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

    printf("Removing memory environment ...");
    fflush(stdout);

    /* Remove this environment, otherwise diag image won't boot up */
    woodlawn_tx_uart(tty_dev, WOODLAWN_REMOVE_MEM_ENV);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(100);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SAVE_ENV);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(WOODLAWN_POWER_UP_DELAY);
    printf("OK\n");
    fflush(stdout);

    /* Now, reset uboot to take effect */
    printf("Resetting Uboot...\n");
    fflush(stdout);
    woodlawn_tx_uart(tty_dev, WOODLAWN_RESET_UBOOT);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(WOODLAWN_POWER_UP_DELAY);

    /* Polling if bootloader is up, we need to fire tftpdnld command
     * through UART interface since Woodlawn bootloader doesn't boot
     * linux by default. (It boots up SE instead)
     */
    printf("Looking for bootloader prompt (2)...");
    fflush(stdout);

    boot_timeout = WOODLAWN_BL_PROMPT_TOUT;
    do{
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

        if (woodlawn_rx_polling_uart(tty_dev, WOODLAWN_BL_PROMPT, 100) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "Failed to get '%s' bootloader prompt", WOODLAWN_BL_PROMPT);
        return (FAILED);
    }

    /* Now, we can do tftp download in Uboot prompt */
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_IPADDR);
    msleep(500);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_NETMASK);
    msleep(500);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_SERVERIP);
    msleep(500);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_ETHACT);
    msleep(500);

    /* Now ping the server ip */
    printf("Ping TFTP Server from Backplane ...");
    fflush(stdout);

    woodlawn_tx_uart(tty_dev, WOODLAWN_PING_SERVERIP);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    ping_timeout = WOODLAWN_PING_TOUT;
    do {
        if (woodlawn_rx_polling_uart(tty_dev, WOODLAWN_PING_ALIVE, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
        
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "Failed to ping TFTP Server");
        return (FAILED);
    }

    /* Now, boot image using TFTP download */
    printf("Booting image ...\n");
    fflush(stdout);

    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_FILENAME);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(100);

    /* Pass Ext Loopback flag to SM card */
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        sprintf(ext_lpbk_flag, "FALSE");
    } else {
        sprintf(ext_lpbk_flag, "TRUE");
    }

    /* Pass Stop on error flag to SM card */
    if (!((NVRAM)->diagflag & D_STOPONERR)) {
        sprintf(stop_on_err_flag, "FALSE");
    } else {
        sprintf(stop_on_err_flag, "TRUE");
    }

    /* Pass Min test time flag to SM card */
    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        sprintf(min_test_flag, "FALSE");
    } else {
        sprintf(min_test_flag, "TRUE");
    }

    /* Pass Verbose flag to SM card */
    if (!((NVRAM)->diagflag & D_VERBOSE)) {
        sprintf(verbose_flag, "FALSE");
    } else {
        sprintf(verbose_flag, "TRUE");
    }

    sprintf(load_linux_str, "%s sm_slot=%d ext_lpbk=%s stop_on_err=%s min_test=%s verbose=%s", WOODLAWN_SET_LOAD_LINUX,
                            woodlawn_test_slot, ext_lpbk_flag, stop_on_err_flag, min_test_flag, verbose_flag);
    woodlawn_tx_uart(tty_dev, load_linux_str);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(100);

    woodlawn_tx_uart(tty_dev, WOODLAWN_BOOT_UP_CMD);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

    /* Now, we wait until we can reach SM card through ping */
    rc = FAILED;
    ping_timeout = WOODLAWN_DIAG_PROMPT_TOUT;
    for (ix = 0; ix < ping_timeout; ix++) {
        printf(".");
        fflush(stdout);
        /* Start pinging SM card */
        if (is_woodlawn_up(FALSE) == TRUE) {
            printf("\nWoodlawn Image is up!\n");
            fflush(stdout);
            rc = PASSED;
            break;
        }
        sleep(1);
    }

    if (rc != PASSED) {
        cterr('f', 0, "\nUnable to reach Woodlawn firmware!");
        fflush(stdout);
    } else {
        /* Display version */
        printf("Diagnostic version:\n");
        woodlawn_transmit_nc_request(DIAG_SEND_DIAG_VER_PORT_BASE);
    }

    return (rc);
}


/**************************************************************************
 *
 * Function: is_woodlawn_up
 *
 * Check if Woodlawn SM card is up by sending ping packet via the GESW
 *
 * Input: verbose - flag to control message printing
 *
 * Return: TRUE/FALSE
 *
 * *************************************************************************
 */
static int is_woodlawn_up (int verbose)
{
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline;
    char *result_file = "/tmp/woodlawn_ping_result";;
    FILE *fp;
    int tx_cnt, rx_cnt;
    int rv = FAILED;
    char sm_ip[16];
    int eth;

    pktcnt = 2;
    deadline = 5;

    fp = fopen(result_file, "r");
    if (fp != NULL) {
        fclose(fp);
        sprintf(cmdbuf, "rm %s", result_file);
        system(cmdbuf);
    }

    eth = get_ctrl_plane_sgmii_port();

    /* 192.123.123.101 for slot 1, 192.123.123.102 for slot 2 */
    sprintf(sm_ip, "%s.%d", WOODLAWN_DIAG_IP_ADDR_SUBNET,
                   WOODLAWN_DIAG_IP_ADDR_BASE + woodlawn_test_slot);

    sprintf(cmdbuf, "ping -c %d -w %d -I eth%d %s > %s",
            pktcnt, deadline, eth, sm_ip, result_file);
    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        if (verbose) {
            printf("HOST: Ping DP %s was not created\n", result_file);
        }
        goto is_woodlawn_up_exit;
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
        goto  is_woodlawn_up_exit;
    }

    rv = PASSED;

is_woodlawn_up_exit:

    if (rv == PASSED) {
        if (verbose) {
            printf("HOST: Ping DP via GESW passed.\n");
        }
        return (TRUE);
    } else {
        if (verbose) {
            printf("HOST: Ping DP via GESW failed.\n");
        }
        return (FALSE);
    }
}


/*
 * Function: woodlawn_discover_pcie
 * Check if the Cavium CPU has been discovered in the host PCI bus
 *
 * Input:
 * venid - vendor ID of the CN68xx CPU
 * devid - device ID of the CN68xx CPU
 *
 * Return: PASSED/FAILED
 */
static int woodlawn_discover_pcie (ushort venid, ushort devid)
{
    FILE *in;
    uint32_t bus;
    uint32_t devfn;
    uint32_t pci_id;
    uint32_t ven_dev_id = (venid << 16 | devid);
    int rv = FAILED;
    int count;
    uint irq;
    uint64_t bar0;
    uint64_t bar1;
    uint64_t siz0;
    uint64_t siz1;
    uint64_t unused;
    char rest_of_line[256];

    /* Open the list of all PCI/PCIe devices */
    in = fopen(PCI_DEVICE_FILENAME, "r");
    if (in == NULL) {
        printf("Unable to open %s\n", PCI_DEVICE_FILENAME);
        return (rv);
    }

    /* Each line in this file represents a PCI/PCIe device */
    while (!feof(in)) {
        count = fscanf(in, "%2x%2x %8x %x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx",
                           &bus, &devfn, &pci_id, &irq,
                           &bar0, &unused, &bar1, &unused, &unused, &unused, &unused,
                           &siz0, &unused, &siz1);
        if (count != 14)
        {
            if (count == -1) {
                printf("fscanf returned %d instead of 14\n", count);
                printf("fscanf failed to read the format");
                break;
            }
        }

        /* The fscanf doesn't read the whole line. Read the rest and
         * throw it away
         */
        if (fgets(rest_of_line, sizeof(rest_of_line), in) == NULL) {
            printf("fgets failed to read the rest of the line");
            break;
        }

#ifdef DEBUG
        printf("pci_id=%#.8x ven_dev_id=%#.8x\n", pci_id, ven_dev_id);
#endif
        if (pci_id != ven_dev_id) {
            continue;
        }

        rv = PASSED;
        break;
    }
    fclose(in);

    return (rv);
}

#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
/**********************************************************************
 *
 * Function: woodlawn_boot_up_other_slot
 *
 * This function boots up Woodlawn on the other slot
 *
 * Input : verify_slot - slot number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_boot_up_other_slot (int verify_slot)
{
    struct ngio_intf_t *sm_ptr;
    n2g_i2c_if_t *sm_oir;
    int status = PASSED;
    uint8_t data = 0;
    char err[80];
    char tty_dev[32];
    char load_linux_str[256];
    char cmd[32];
    struct stat sts;
    char ext_lpbk_flag[16], stop_on_err_flag[16];
    char min_test_flag[16], verbose_flag[16];
    int ix, rc, boot_timeout, ping_timeout, retry;
    int temp_slot;

    sm_ptr = slot_get_ngiosm(verify_slot);
    if (sm_ptr == NULL) {
        printf("%s: Null Pointer!\n", __FUNCTION__);
        return (FAILED);
    }

    sm_oir = (n2g_i2c_if_t *)sm_ptr->oir;

    /* Check if SM is present or not */
    if (!sm_ptr->is_present((void *)sm_ptr)) {
        printf("SM Slot-%d is Vacant! Exiting...\n", verify_slot);
        return (FAILED);
    }

    /* NGIO Power On and read ID */
    for (retry = 0, *err = '\0'; retry < 3; retry++) {
        if (slot_i2c_unreset(sm_ptr, sm_ptr->slot, "SM") == FAILED) {
            return (FAILED);
        }

        /* Read Controller Type */
        if ((status = sm_ptr->get_id((void *)sm_ptr, err)) == FAILED) {
            if (sm_ptr->off) {
                sm_ptr->off(sm_ptr);
            }
            sleep(2);
            continue;
        }
        break;
    }

    /* Make sure the SM card is Woodlawn */
    if (sm_ptr->id == NGSM_WOODLAWN_6G ||
        sm_ptr->id == NGSM_WOODLAWN_10G4G) {
        printf("Woodlawn SM Card is detected!\n");
    } else {
        printf("Woodlawn SM Card is required for the test! Exiting...\n");
        return (FAILED);
    }

    /* Download image from the network for the first time  */
    if (stat(WOODLAWN_DEST_DIAG_IMG, &sts) == -1) {
        if (tftp_get(0, WOODLAWN_SRC_DIAG_IMG, 0, WOODLAWN_DEST_DIAG_IMG, 0) < 0) {
            sprintf(cmd, "rm -f %s", WOODLAWN_DEST_DIAG_IMG);
            system(cmd);
            fflush(stdout);
            printf("Failed to tftp download firmware to local host! Exiting...\n");
            return (FAILED);
        }
    }

    /* Power up the card */
    printf("Powering up SM-%d now...\n", sm_ptr->slot);
    fflush(stdout);

    sm_ptr->on(sm_ptr);
    sm_ptr->uart_on(sm_ptr);
    /* Pull high GPIO3 on LTC4215 */
    if (oir_ltc4215_reg_read(sm_oir, LTC4215_FAULT_REG, &data)) {
        printf("Read LTC4215 register fail!\n");
        return (FAILED);
    }
    data &= ~ (0x80);
    if (oir_ltc4215_reg_write(sm_oir, LTC4215_FAULT_REG, &data)) {
        printf("Write LTC4215 register fail!\n");
        return (FAILED);
    }

    msleep(100);

    sm_ptr->unreset(sm_ptr);

    msleep(WOODLAWN_POWER_UP_DELAY);

    /* Setup UART */
    sprintf(tty_dev, "/dev/ttyDASH%d", woodlawn_uart_ctrl);
    if (woodlawn_uart_setup(tty_dev) == FAILED) {
        printf("Failed to setup UART on SM-%d! Exiting...\n", sm_ptr->slot);
        return (FAILED);
    }

    /* Polling if bootloader is up, we need to fire tftpdnld command
     * through UART interface since Woodlawn bootloader doesn't boot
     * linux by default. (It boots up SE instead)
     */
    printf("Looking for bootloader prompt (1)...");
    fflush(stdout);

    boot_timeout = WOODLAWN_BL_PROMPT_TOUT;
    do {
        /* Transmit New Line */
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

        if (woodlawn_rx_polling_uart(tty_dev, WOODLAWN_BL_PROMPT, 100) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        printf("Failed to get '%s' bootloader prompt\n", WOODLAWN_BL_PROMPT);
        return (FAILED);
    } 

    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

    printf("Removing memory environment ...");
    fflush(stdout);

    /* Remove this environment, otherwise diag image won't boot up */
    woodlawn_tx_uart(tty_dev, WOODLAWN_REMOVE_MEM_ENV);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(100);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SAVE_ENV);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(WOODLAWN_POWER_UP_DELAY);
    printf("OK\n");
    fflush(stdout);

    /* Now, reset uboot to take effect */
    printf("Resetting Uboot...\n");
    fflush(stdout);
    woodlawn_tx_uart(tty_dev, WOODLAWN_RESET_UBOOT);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(WOODLAWN_POWER_UP_DELAY);

    /* Polling if bootloader is up, we need to fire tftpdnld command
     * through UART interface since Woodlawn bootloader doesn't boot
     * linux by default. (It boots up SE instead)
     */
    printf("Looking for bootloader prompt (2)...");
    fflush(stdout);

    boot_timeout = WOODLAWN_BL_PROMPT_TOUT;
    do {
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
        woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

        if (woodlawn_rx_polling_uart(tty_dev, WOODLAWN_BL_PROMPT, 100) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        printf("Failed to get '%s' bootloader prompt\n", WOODLAWN_BL_PROMPT);
        return (FAILED);
    }

    /* Now, we can do tftp download in Uboot prompt */
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_IPADDR);
    msleep(500);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_NETMASK);
    msleep(500);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_SERVERIP);
    msleep(500);
    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_ETHACT);
    msleep(500);

    /* Now ping the server ip */
    printf("Ping TFTP Server from Backplane ...");
    fflush(stdout);

    woodlawn_tx_uart(tty_dev, WOODLAWN_PING_SERVERIP);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    ping_timeout = WOODLAWN_PING_TOUT;
    do {
        if (woodlawn_rx_polling_uart(tty_dev, WOODLAWN_PING_ALIVE, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        printf("Failed to ping TFTP Server\n");
        return (FAILED);
    }

    /* Now, boot image using TFTP download */
    printf("Booting image ...\n");
    fflush(stdout);

    woodlawn_tx_uart(tty_dev, WOODLAWN_SET_FILENAME);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(100);

    /* Pass Ext Loopback flag to SM card */
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        sprintf(ext_lpbk_flag, "FALSE");
    } else {
        sprintf(ext_lpbk_flag, "TRUE");
    }

    /* Pass Stop on error flag to SM card */
    if (!((NVRAM)->diagflag & D_STOPONERR)) {
        sprintf(stop_on_err_flag, "FALSE");
    } else {
        sprintf(stop_on_err_flag, "TRUE");
    }

    /* Pass Min test time flag to SM card */
    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        sprintf(min_test_flag, "FALSE");
    } else {
        sprintf(min_test_flag, "TRUE");
    }

    /* Pass Verbose flag to SM card */
    if (!((NVRAM)->diagflag & D_VERBOSE)) {
        sprintf(verbose_flag, "FALSE");
    } else {
        sprintf(verbose_flag, "TRUE");
    }

    sprintf(load_linux_str, "%s sm_slot=%d ext_lpbk=%s stop_on_err=%s min_test=%s verbose=%s", WOODLAWN_SET_LOAD_LINUX,
                            sm_ptr->slot, ext_lpbk_flag, stop_on_err_flag, min_test_flag, verbose_flag);
    woodlawn_tx_uart(tty_dev, load_linux_str);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);
    msleep(100);

    woodlawn_tx_uart(tty_dev, WOODLAWN_BOOT_UP_CMD);
    woodlawn_tx_uart(tty_dev, WOODLAWN_CR_STRING);

    /* Now, we wait until we can reach SM card through ping */
    rc = FAILED;
    ping_timeout = WOODLAWN_DIAG_PROMPT_TOUT;
    temp_slot = woodlawn_test_slot;
    woodlawn_test_slot = sm_ptr->slot;
    for (ix = 0; ix < ping_timeout; ix++) {
        printf(".");
        fflush(stdout);
        /* Start pinging SM card */
        if (is_woodlawn_up(FALSE) == TRUE) {
            printf("\nWoodlawn Image is up!\n");
            fflush(stdout);
            rc = PASSED;
            break;
        }
        sleep(1);
    }
    woodlawn_test_slot = temp_slot;

    if (rc != PASSED) {
        printf("\nUnable to reach Woodlawn firmware!\n");
        fflush(stdout);
    } else {
        /* Display version */
        printf("Diagnostic version for SM-%d:\n", sm_ptr->slot);
        woodlawn_test_slot = sm_ptr->slot;
        woodlawn_transmit_nc_request(DIAG_SEND_DIAG_VER_PORT_BASE);
        woodlawn_test_slot = temp_slot;
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: woodlawn_power_off_other_slot
 *
 * This function turns off Woodlawn on second slot
 *
 * Input : verify_slot
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long woodlawn_power_off_other_slot (int verify_slot)
{
    struct ngio_intf_t *sm_ptr;
    n2g_i2c_if_t *sm_oir;
    uint8_t data;

    sm_ptr = slot_get_ngiosm(verify_slot);
    if (sm_ptr == NULL) {
        printf("%s: Null Pointer!\n", __FUNCTION__);
        return (FAILED);
    }

    sm_oir = (n2g_i2c_if_t *)sm_ptr->oir;

    /* Toggle bit 7 of LTC4215 */
    if (oir_ltc4215_reg_read(sm_oir, LTC4215_FAULT_REG, &data)) {
        printf("Read LTC4215 Register Failed!\n");
        return (FAILED);
    }

    data |= 0x80;

    if (oir_ltc4215_reg_write(sm_oir, LTC4215_FAULT_REG, &data)) {
        printf("Write LTC4215 Register Failed!\n");
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: timingcard_power_on
 *
 * This function power-on timing card
 *
 * Input : test_slot - slot number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_power_on (int test_slot)
{
    struct ngio_intf_t *vm_ptr;
    char err[80];
    int retry;
    int status = PASSED;
    
    vm_ptr = slot_get_ngiovm(test_slot);
    if (vm_ptr == NULL) {
        printf("%s: Null Pointer!\n", __FUNCTION__);
        return (FAILED);
    }

    /* Check if SM is present or not */
    if (!vm_ptr->is_present((void *)vm_ptr)) {
        printf("VM Slot-%d is Vacant! Exiting...\n", test_slot);
        return (FAILED);
    }

    /* NGIO Power On and read ID */
    for (retry = 0, *err = '\0'; retry < 3; retry++) {
        if (slot_i2c_unreset(vm_ptr, vm_ptr->slot, "VM") == FAILED) {
            return (FAILED);
        }

        /* Read Controller Type */
        if ((status = vm_ptr->get_id((void *)vm_ptr, err)) == FAILED) {
            if (vm_ptr->off) {
                vm_ptr->off(vm_ptr);
            }
            sleep(2);
            continue;
        }
        break;
    }

    /* Make sure the VM card is Timing Card */
    if (vm_ptr->id == TIMINGCARD_VM) {
        printf("Timing Card is detected!\n");
    } else {
        printf("VM Card Controller Type: 0x%x\n", vm_ptr->id);
        printf("Timing Card is required for the test! Exiting...\n");
        return (FAILED);
    }

    /* Power up the VM card */
    printf("Powering up VM-%d now...\n", vm_ptr->slot);
    fflush(stdout);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_power_off
 *
 * This function power-off timing card
 *
 * Input : test_slot - slot number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_power_off (int test_slot)
{
    struct ngio_intf_t *vm_ptr;

    vm_ptr = slot_get_ngiovm(test_slot);
    if (vm_ptr == NULL) {
        printf("%s: Null Pointer!\n", __FUNCTION__);
        return (FAILED);
    }

    /* Power off the VM card */
    printf("Powering off VM-%d now...\n", vm_ptr->slot);
    vm_ptr->off(vm_ptr);
    vm_ptr->reset(vm_ptr);
    return (PASSED);
}

#endif
/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: sm_woodlawn.c,v $
 * Revision 1.16  2020/01/09 01:03:02  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.15  2019/08/06 06:56:18  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.14.2.1  2018/06/28 07:38:26  alpeng
 * remove data plane and timing card portions for curie 1RU;
 *
 * Revision 1.14  2018/05/18 09:25:01  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.12.20.3  2018/05/17 10:51:00  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.12.20.2  2017/06/13 09:28:18  leschen
 * Fix for UART kernel panic on Neptune system. This problem doesn't be found on USD or O2 systems before, will verify the changes on these platforms afterwards.
 *
 * Revision 1.12.20.1  2016/11/07 02:30:57  leschen
 * Modify Woodlawn uart and i2c controller for Neptune.
 *
 * Revision 1.13  2017/07/14 02:51:39  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.12  2015/03/31 07:35:23  leschen
 * Fix for get prompt.
 *
 * Revision 1.11  2015/02/20 08:14:34  kwochan
 * TimingCard codes commit broke utah_lnx daily-built, but O2 x86_lnx daily-built is
 * ok. TimingCard is an uncommitted ISC project, which is used to verify project
 * that needs to test 1588, eg. Woodlawn SM and Wallendar NIM.
 * It is believe that Woodlawn SM and Wallendar NIM verify 1588 on O2 only, so
 * #ifndef UTAH to commit the codes in sm_woodlawn.c so that other projects
 * can build the utah-lnx successful for them to release the image.
 * Kody needs to confirm if the codes change of this sm_woodlawn.c are done
 * correctly ...
 *
 * Revision 1.10  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.9  2014/11/12 06:05:06  leschen
 * Support Greyhound GE1 BP lpbk.
 *
 * Revision 1.8  2014/10/17 07:38:55  leschen
 * Support Greyhound switch
 *
 * Revision 1.7  2014/09/06 00:53:21  ptong
 * Remove ngio_ge_cfg from slot.c and add to ngio_testcard.c and sm_woodlawn.c
 *
 * Revision 1.6  2014/02/18 09:11:12  alpeng
 * CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
 *
 * Revision 1.5  2014/02/11 04:17:51  leschen
 * Suppotr on Utah platform.
 *
 * Revision 1.4.4.3  2014/04/30 13:47:20  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.4.4.2  2014/03/11 02:32:41  leschen
 * Add 1588 clk/trig verification item.
 *
 * Revision 1.4.4.1  2014/01/03 06:38:45  leschen
 * Add utility to verify clk/trigger are fed properly into woodlawn phy
 *
 * Revision 1.4  2013/11/26 08:40:38  hroni
 * fix compiler warning
 *
 * Revision 1.3  2013/11/11 21:18:41  mcharon
 * pass string instead of number in first argum of host_send_packet ; add xaui support
 *
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.22  2013/07/02 07:54:49  leschen
 * Add get bootflash info and OTP verification utility
 *
 * Revision 1.1.2.21  2013/06/18 01:43:13  leschen
 * If not turn on ngio pwr after test flag then power off SM card when leave Woodlawn main menu
 *
 * Revision 1.1.2.20  2013/06/17 10:30:40  leschen
 * Move sm utility to host side through nc command
 *
 * Revision 1.1.2.19  2013/06/17 03:19:26  leschen
 * Add utility to ctrl front panel leds from host side
 *
 * Revision 1.1.2.18  2013/06/13 11:38:20  tirawan
 * Implement NC dispatch command
 *
 * Revision 1.1.2.17  2013/06/06 12:08:20  leschen
 * Add SM power utility
 *
 * Revision 1.1.2.16  2013/05/30 08:41:58  leschen
 * Add show diag ver utility, fix firmware download procedure and pass more flags to SM card
 *
 * Revision 1.1.2.15  2013/05/17 03:32:30  leschen
 * Extend delay time to 500ms to fix the environment variables formate is not right when do firmware download
 *
 * Revision 1.1.2.14  2013/04/25 07:00:45  kodko
 * Check FPGA is in upgrade mode when power on SM card
 *
 * Revision 1.1.2.13  2013/04/24 11:12:52  tirawan
 * Fix GE BP Loopback issue which sets up loopback bit through UART
 *
 * Revision 1.1.2.12  2013/04/24 07:27:38  tirawan
 * Fix intermittent boot up issue
 *
 * Revision 1.1.2.11  2013/04/18 06:46:09  tirawan
 * Provide Voltage Margin Utility from O2
 *
 * Revision 1.1.2.10  2013/04/18 02:30:14  tirawan
 * Hit CTRL-C to interrupt Uboot instead of using CR
 *
 * Revision 1.1.2.9  2013/04/10 10:23:35  tirawan
 * Pass External Loopback flag to SM card
 *
 * Revision 1.1.2.8  2013/04/10 03:33:27  tirawan
 * Add GE backplane loopback test to verify GE0 connectivity
 *
 * Revision 1.1.2.7  2013/04/09 09:27:11  tirawan
 * Change powering up module action to firmware download test item
 *
 * Revision 1.1.2.6  2013/04/08 02:38:11  tirawan
 * Download the image through tftp before powering up the module
 *
 * Revision 1.1.2.5  2013/04/03 05:46:40  tirawan
 * Add auto boot by UART function, and auto run by nc utility
 *
 * Revision 1.1.2.4  2013/03/08 07:16:03  tirawan
 * Correct slot number when configuring XAUI loopback mode on BCM Switch
 *
 * Revision 1.1.2.3  2013/03/07 12:55:02  tirawan
 * Update FPGA I2C Address, and add power on sequence for Woodlawn SM
 *
 * Revision 1.1.2.2  2013/02/28 03:00:53  tirawan
 * Update baud rate to 9600
 *
 * Revision 1.1.2.1  2013/02/06 03:04:55  tirawan
 * Woodlawn Support on O2
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

