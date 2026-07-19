/* $Id: sm_canis.c,v 1.32 2018/05/22 02:31:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/sm_canis.c,v $
 *******************************************************************************
 * File Name: sm_canis.c
 *
 * Description: Canis SM main source file
 *
 * Author: Khalid Sabzwari
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
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
#include "sm_canis.h"
#include "ngio.h"

#include "pca.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#define DEBUG_O2_POWER_NOISE    1

/**************************************
 *  Global Variables
 **************************************
 */
static int debug_o2_power_fault_flag = 0;

canis_ds_t canis_iface[MAX_SM+1];
static canis_ds_t *canis_iface_p;
static void (*canis_saved_diag_exec)(void) = NULL;
static long canis_ltc_reg_read(void);
static long canis_ltc_reg_write(void);
static long canis_ltc_reg_test(void);
static long util_canis_power_on(void);
static long util_canis_power_off(void);
static long util_canis_power_cycle(void);
static long canis_pwrup_onoff(int onoff);
static long switch_wait_canis_bmc_ready(void);
static void canis_check_bmc_ready(void);
static void canis_check_bmc_ready_wa(void);
static void (*wait_canis_bmc_ready_func)(void) = canis_check_bmc_ready;
static void show_oir_ltc4215_regs(void);
static long util_show_oir_ltc4215_regs(void);
static long util_debug_o2_power_fault(void);

boolean pca9557;

int canis_test_slot = 0;

/* 
 * Extern function prototypes 
 */
extern long build_canis_rbcp_menu(int);
extern int canis_rbcp_bmc_console_switch(void);
extern int canis_rbcp_intel_console_switch(void);
extern int canis_setup_rbcp_ge_env(void);
extern int do_all_menu_items(struct menuinfo *);
extern int canis_cleanup_rbcp_ge_env(void);
extern int canis_get_mac(uchar);
extern int getdec_answer(char *,uint ,uint ,uint);
extern void set_canis_loopback(int ,int);
extern int canis_rbcp_heartbeat_test(void);     /* part of interface test */
extern int canis_rbcp_registration_test(int);   /* part of interface test */
extern void clear_regis_done_flag(int);

/* 
 * Function prototype 
 */
int canis_sm_test(void *);
int canis_iface_test(canis_ds_t *);
long canis_utility_submenu(int);
long canis_i2c_ioe_reg_test(void);

long configure_i2c_expander(int);
long canis_i2c_port_reg_read(void);
long canis_i2c_port_reg_write(void);
long canis_o2_shell(canis_ds_t *);
long canis_o2_command(canis_ds_t *);
long canis_rbcp_bmc_con_switch(void);
long canis_rbcp_intel_con_switch (void);
int canis_set_rbcp_mac_add(void);
long canis_gesw_lpbk_set(void);

long canis_rbcp_picocom_switch (void);
long canis_rbcp_set_link_bmc(void);
long canis_rbcp_set_link_intel(void);
long canis_show_ioe_regs(char *);

/* 
 * Canis SM main menu on Overlord platform 
 */
static submenu_xtable_t canis_submenu_tbl[] = {
    {"I2C IO Expander Register Test",   canis_i2c_ioe_reg_test, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"LTC4215 Register Test",  canis_ltc_reg_test, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, (long(*)())0, 0 },
    {"RBCP Tests",                      build_canis_rbcp_menu, 0,
      MF_CONTINUOUS | MF_DOALL, (long(*)())0, 0, build_canis_rbcp_menu, TRUE},
    { "BMC Console redirect Test",      canis_rbcp_bmc_con_switch, 0,
     0,                   (long(*)())0, 0, (long(*)())0, 0 },
    { "INTEL Console redirect Test",    canis_rbcp_intel_con_switch, 0,
     0,                   (long(*)())0, 0, (long(*)())0, 0 },
    { "Canis utility",                  canis_utility_submenu, 0,
     0,                   (long(*)())0, 0, (long(*)())0, 0 },
};

#define CANIS_SUBMENU_TABLE_SZ \
                (sizeof(canis_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t canis_primary_items[CANIS_SUBMENU_TABLE_SZ +
                                   MAX_BASE_ITEMS];
static mitem_t canis_secondary_items[CANIS_SUBMENU_TABLE_SZ +
                                     MAX_BASE_ITEMS];

static menuinfo_t canis_menu = {
    "Canis Main Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    canis_primary_items,
};
static menuinfo_t *canis_menup = &canis_menu;


/* 
 * Canis SM utilities menu on Overlord platform 
 */
static mitem_t canis_util_submenu_table[] = {
    { "I2C IO Port Register Read util", 0, 0, canis_i2c_port_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "I2C IO Port Register Write util", 0, 0, canis_i2c_port_reg_write,
      (long *)&one, 0, (long(*)())0, 0 },
    { "Init I2C IO Expander Registers", 0, 0, configure_i2c_expander,
      (long *)&one, 0, (type_t(*)())0, 0 },
    { "Show I2C IO Expander Registers", 0, 0, canis_show_ioe_regs,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "LTC4215 Register Read util", 0, 0, canis_ltc_reg_read,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "LTC4215 Register Write util", 0, 0, canis_ltc_reg_write,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Escape to Shell (debugging only)", 0, 0, canis_o2_shell,
      (long *)&canis_iface_p, 0, (long(*)())0, 0 },
    { "Execute a Shell command (debugging only)", 0, 0, canis_o2_command,
      (long *)&canis_iface_p, 0, (long(*)())0, 0 },
    { "Picocom Console redirect Link", 0, 0, canis_rbcp_picocom_switch,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Send RBCP to set link with BMC", 0, 0, canis_rbcp_set_link_bmc,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Send RBCP to set link with INTEL", 0, 0, canis_rbcp_set_link_intel,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Platform GE Switch Line Loopback Setup", 0, 0, canis_gesw_lpbk_set,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Power Off Canis SM", 0, 0, util_canis_power_off,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Power On Canis SM", 0, 0, util_canis_power_on,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Power Cycle Canis SM", 0, 0, util_canis_power_cycle,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Switch Wait Canis BMC Ready Function", 0, 0, switch_wait_canis_bmc_ready,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Set Power Fault Debug Flag", 0, 0, util_debug_o2_power_fault,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Show LTC4215 Registers", 0, 0, util_show_oir_ltc4215_regs,
      (long *)&zero, 0, (type_t(*)())0, 0 },
};

#define CANIS_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(canis_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t canis_util_subtest_menu = {
    "Canis Utilities Menu",
    0,                                        /* title param */
    0,                         /* show diag flags */
    0,
    CANIS_UTIL_SUBMENU_TABLE_SZ,
    canis_util_submenu_table,
};

static menuinfo_t *canis_util_submenup = &canis_util_subtest_menu;


static n2g_i2c_if_t pca_i2c;
static char pca_buff[256];
static n2g_i2c_if_t *oir;


/**********************************************************************
 *
 * Function: canis_sm_test
 *
 * This function is the main entrance for Canis SM testing.
 *
 * Input : sm - pointer to sm ngio interface
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int canis_sm_test (void *sm)
{
    int real_slot;
    int ret_val;
    n2g_i2c_if_t *pca = &pca_i2c;
    struct ngio_intf_t *canis_sm_iface = (struct ngio_intf_t *)sm;
    ushort board_id = 0;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n%s: sm=%#lx ", __FUNCTION__, (long)sm);
    }

    real_slot = canis_sm_iface->slot;
    board_id = canis_sm_iface->id;

    canis_test_slot = canis_sm_iface->slot;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf ("slot=%d board_id=%#x ", real_slot, board_id);
    }

    testname(" (SM Slot %d Canis)", real_slot);

    pca_init_i2c((void *)pca);
    pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;
    pca->i2c_ctrl = canis_sm_iface->i2c_ctrl;
    pca->buf = pca_buff;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("i2c_ctrl=%d i2c_dev=%#x i2c_mux=%d i2c_size=%d\n", 
                pca->i2c_ctrl, pca->i2c_dev, pca->mux, pca->size);
    }

    oir = (n2g_i2c_if_t *)canis_sm_iface->oir;

    /*
     * Initialize an instance of Canis data structure
     */
    canis_iface_p = (canis_ds_t *) &canis_iface[real_slot];
    canis_iface_p->board_id = board_id;
    canis_iface_p->slot = real_slot;
    canis_iface_p->uart = canis_sm_iface->uart_ctrl;
    canis_iface_p->canis_sm_iface = (struct ngio_intf_t *)sm;

    canis_sm_iface->uart_on(canis_sm_iface);
    canis_sm_iface->unreset(canis_sm_iface);
    msleep(1000);

    assert(sm);

    if (configure_i2c_expander(TRUE)) {
        cterr('w', 0, "unable to configure i2c_expander");
    }

    if (canis_pwrup_onoff(ENABLE)) {
        cterr('w', 0, "unable to set GPIO3 pin to power up Intel side");
    }

    /* Canis menu's title need a fixed name via cookie_id */
    build_primary_submenu(canis_submenu_tbl, CANIS_SUBMENU_TABLE_SZ,
                          "Canis", &canis_menup);
    build_secondary_submenu(canis_submenu_tbl,
                            CANIS_SUBMENU_TABLE_SZ, canis_secondary_items);

    /* Setup RBCP GE switch environment and Mac address */
    if (canis_setup_rbcp_ge_env() == FAILED) {
        return (FAILED);
    }
    canis_set_rbcp_mac_add();

    /* setup the backplane loopback here. We will do the loopback test
           from Intel CPU */
    set_canis_loopback(real_slot,TRUE);

    /* wait for BMC ready */
    (*wait_canis_bmc_ready_func)();

    /* to help debug O2 power fault noise only */
    if (debug_o2_power_fault_flag) {
        show_oir_ltc4215_regs();
    }

    clear_regis_done_flag(canis_test_slot);

    /*
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    canis_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    ret_val = PASS;
    if (canis_sm_iface->menu_display == TRUE) {
        menu(canis_menup, canis_secondary_items, '\0');
    } else {
        if (canis_sm_iface->test_type == IFACE_TEST) {
            ret_val = canis_iface_test(canis_iface_p);
        } else {  /* FULL_TEST */
            do_all_menu_items(canis_menup);
        }
    }

    if (canis_saved_diag_exec) {
        pre_diag_exec = canis_saved_diag_exec;
        canis_saved_diag_exec = NULL;
    }

    /* disable the backplane loopback here */
    set_canis_loopback(real_slot,FALSE);

    if (canis_cleanup_rbcp_ge_env() == FAILED) {
        return (FAILED);
    }

    return (ret_val);
}

/*************************************************************************
 * Function: canis_iface_test
 *
 * Test entry for canis interface test.
 *      covered: I2C, GE0.
 *  not covered: console UART to Intel/BMC, GE1, XAUI.
 *
 * Input : canis_ifp - pointer to canis specfic data structure
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int canis_iface_test (canis_ds_t *canis_ifp)
{
    if (canis_i2c_ioe_reg_test() ||
        canis_ltc_reg_test() ||
        canis_rbcp_registration_test(FALSE) ||
        canis_rbcp_heartbeat_test()) {
        return FAILED;
    }
    return PASSED;
}

/*************************************************************************
 * Function: canis_rbcp_picocom_switch
 *
 * Switch uart console without RBCP
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long canis_rbcp_picocom_switch (void)
{
    canis_ds_t *iface;
    const int maxlen = 128;
    char cmd[maxlen];

    iface = canis_iface_p;
    assert(iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second to display above NOTE */

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             iface->uart);

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("cmd=%s\n", cmd);
    }

    system(cmd);

    return(PASSED);

}

/*************************************************************************
 * Function: canis_rbcp_bmc_con_switch
 *
 * Switch uart console to bmc with RBCP
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long canis_rbcp_bmc_con_switch (void)
{
    canis_ds_t *iface;
    const int maxlen = 128;
    char cmd[maxlen];

    canis_rbcp_bmc_console_switch();

    iface = canis_iface_p;
    assert(iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second to display above NOTE */

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             iface->uart); 

    if (diagflag_xram & D_SET_OPTIONS) {
       printf("cmd=%s\n", cmd);
    }

    system(cmd);

    return(PASSED);
}

/*************************************************************************
 * Function: canis_rbcp_intel_con_switch
 *
 * Switch uart console to intel with RBCP
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long canis_rbcp_intel_con_switch (void)
{
    canis_ds_t *iface;
    const int maxlen = 128;
    char cmd[maxlen];

    canis_rbcp_intel_console_switch();

    iface = canis_iface_p;
    assert(iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second to display above NOTE */

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             iface->uart);

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("cmd=%s\n", cmd);
    }

    system(cmd);

    return(PASSED);
}

/*************************************************************************
 * Function: canis_rbcp_set_link_intel
 *
 * Switch uart console to intel
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long canis_rbcp_set_link_intel (void)
{

    canis_rbcp_intel_console_switch();

    return(PASSED);
}

/*************************************************************************
 * Function: canis_rbcp_set_link_bmc
 *
 * Switch uart console to bmc
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long canis_rbcp_set_link_bmc (void)
{

    canis_rbcp_bmc_console_switch();

    return(PASSED);
}

/*************************************************************************
 * Function: canis_i2c_ioe_reg_test
 *
 * This function toggles the i2c expander Polarity register and verifies it.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long canis_i2c_ioe_reg_test (void)
{
    int i;
    uchar sav, wval, rval;
    n2g_i2c_if_t *pca= &pca_i2c;
    int32_t reg = POLARITY_INV_REG;
    char *name = "Polarity";

    prpass(testpass, "I2C IO Expander Register");

    if (diagflag_xram & D_SET_OPTIONS) {
        canis_show_ioe_regs("\nBefore IOE test");
    }

    /* Save register under test */
    if (io_port_8bit_i2c_read(pca, reg, &sav, FALSE)) {
        return (FAILED);
    }

    /* ripple 1 test */
    for (i = 0; i < 8; i++) {
        wval = 1 << i;
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }

        if (diagflag_xram & D_SET_OPTIONS) {
            printf("%#x ", rval);
        }

        if (rval != wval) {
            cterr ('f', 0, "Ripple one test failed when accessing %s "
                           "register. Expect: %#x, Read: %#x.",
                           name, wval, rval);
            return FAILED;
        }
    } 

    /* ripple 0 test */
    for (i = 0; i < 8; i++) {
        wval = ~(1 << i);
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }

        if (diagflag_xram & D_SET_OPTIONS) {
            printf("%#x ", rval);
        }

        if (rval != wval) {
            cterr ('f', 0, "Ripple zero test failed when accessing %s "
                           "register. Expect: %#x, Read: %#x.",
                           name, wval, rval);
            return FAILED;
        }
    } 

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n");
    }

    /* Restore register under test */
    if (io_port_8bit_i2c_write(pca, reg, &sav)) {
        return (FAILED);
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        canis_show_ioe_regs("After IOE test");
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 *  Function: canis_set_rbcp_mac_add
 *
 *  Get ge mac address
 *
 *  Input: none
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int canis_set_rbcp_mac_add(void)
{
    uchar slot=0;

    slot = canis_iface_p->slot;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n slot number = %d",slot);
    }

    canis_get_mac(slot);

    return(PASSED);
}

/**********************************************************************
 *
 *  Function: canis_gesw_lpbk_set
 *
 *  Description: This function is the wrapper to enables or disable the line
 *		 loopback of the GE switch port connected to the Canis.
 *
 *  Input  : none
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
long canis_gesw_lpbk_set (void)
{
    int slot, enable;

    slot = canis_iface_p->slot;

    if (getc_answer("(e)nable or (d)isable the Platform GE switch lineloopback",
                    "ed", 'e') == 'e') {
        /* User request to enable the line loopback */
        enable = TRUE;
    } else {
        /* User request to disable the line loopback */
        enable = FALSE;
    }

    set_canis_loopback(slot, enable);

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: pse2_utility_submenu().
 *
 * This function implements the Canis sm test/menu
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
long canis_utility_submenu (int menu_option)
{
    menu(canis_util_submenup, canis_util_submenu_table, '\0');

    return (PASSED);
}

/******************************************************************************
 * Function: configure_i2c_expander
 * 
 * Refer to I2C IO Expnader pins table in sm_canis.h
 * 
 * Input : dump - if TRUE will dump ioe regs before and after config
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long configure_i2c_expander (int dump)
{
    uint8_t data;
    n2g_i2c_if_t *pca = &pca_i2c;

    pca9557 = TRUE;

    /* set up IO pin direction in configure register */
    data = IOE_ALL_IN; /* all as input */

    /* Set IO bits 1,2,4,5 as output */
    data &= ~(IOE_BOOT_SEL | IOE_DB_RESET_L |
              IOE_UART_MUX_SEL | IOE_RESET_CFG_L);

    if (dump) {
        canis_show_ioe_regs("\nBefore I2C IO Expander config");
    }

    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &data)) {
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &data, TRUE)) {
        return (FAILED);
    }

    /* set up input IO pin polarity -- not inverted */
    data = 0;
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &data)) {
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, POLARITY_INV_REG, &data, TRUE)) {
        return (FAILED);
    }

    sleep(1);

    if (dump) {
        canis_show_ioe_regs("After I2C IO Expander config");
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: canis_i2c_port_reg_read
 *
 * This function reads a register on I2C IO port chip
 *
 * Input : none 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long canis_i2c_port_reg_read (void)
{

    uchar offset = 0, value = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c;

    offset = getdec_answer("Read I2C Port Register Offset: ", 0, 0, 3);

    if (io_port_8bit_i2c_read(pca, offset, &value, FALSE)) {
        return (FAILED);
    }

    printf("\nI2C Port Register Value: 0x%02x", value);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: canis_i2c_port_reg_write
 *
 * This function writes a value to a register on I2C IO port chip
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long canis_i2c_port_reg_write (void)
{
    uchar offset = 0, value = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c;

    offset = getdec_answer("Write I2C Port Register Offset: ", 0, 0, 3);

    value = gethex_answer("Write I2C Port Register Offset value: ", 0, 0, 0xFF);

    if (io_port_8bit_i2c_write(pca, offset, &value)) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: canis_o2_shell
 *
 * Debug utility to escape to shell from Canis SM submenu
 *
 * Input : iface - Canis data structure info pointer
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long canis_o2_shell (canis_ds_t *iface)
{
    assert(iface);

    printf("\nEscaping to Shell from SM Slot %d Menu,\n", iface->slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);
}

/**********************************************************************
 *
 * Function: canis_o2_command
 *
 * Debug utility to issue a shell command from Canis SM submenu
 *
 * Input : iface - Canis data structure info pointer
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long canis_o2_command (canis_ds_t *iface)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return(PASSED);
}

/**********************************************************************
 *
 * Function: canis_show_ioe_regs
 *
 * utility to dump i2c io expander registers
 *
 * Input : title - title to display
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long canis_show_ioe_regs (char *title)
{
    uint8_t data;
    n2g_i2c_if_t *pca = &pca_i2c;

    if (title) {
        printf("%s:\n", title);
    }

    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE)) {
        printf("failed to read Input Port register\n");
        return (FAILED);
    } else {
        printf(" %-30s : %#x\n", "Input Port register", data);
    }

    if (io_port_8bit_i2c_read(pca, OUTPUT_PORT_REG, &data, TRUE)) {
        printf("failed to read Output Port register\n");
        return (FAILED);
    } else {
        printf(" %-30s : %#x\n", "Output Port register", data);
    }

    if (io_port_8bit_i2c_read(pca, POLARITY_INV_REG, &data, TRUE)) {
        printf("failed to read Polarity Inversion register\n");
        return (FAILED);
    } else {
        printf(" %-30s : %#x\n", "Polarity Inversion register", data);
    }

    if (io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &data, TRUE)) {
        printf("failed to read Configuration register\n");
        return (FAILED);
    } else {
        printf(" %-30s : %#x\n", "Configuration register", data);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: canis_check_bmc_ready
 *
 * check for BMC ready with timeout
 *
 * Input : none
 *
 * Output: none
 *
 **********************************************************************
 */
void canis_check_bmc_ready (void)
{
    n2g_i2c_if_t *pca = &pca_i2c;
    int timeout;
    uint8_t data = 0;

    /* polling BMC ready bit */
    printf("Waiting up to %d minutes for Canis BMC to boot up",
            BMC_RDY_BIT_MINS);
    for (timeout = BMC_RDY_BIT_SECS; timeout; timeout--) {
        if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE)) {
            ; /* I2C read error -- re-check next time */
        } else if (data & PRIM_INTF_READY) {
            /* BMC ready bit is set */
            break;
        }
        WAIT_ONE_SEC();
        printf(".");
        fflush(stdout);
    }
    printf("\n");
    if (timeout > 0) {
        printf("Canis BMC ready (bit 3) is on, INP=%#x\n", data);
    } else {
        printf("Warning: Canis BMC ready (bit 3) still off, INP=%#x\n", data);
    }
    
    /*
     * Canis Phase-2 after 4/2013 (CSCug49316)
     * Wait extra POST_BMC_WAIT_SECS to ensure BCM activates RBCPd.
     */
    printf("Waiting %d minutes for Canis BMC to boot to login prompt",
            POST_BMC_RDY_MINS);
    for (timeout = POST_BMC_RDY_SECS; timeout; timeout--) {
        WAIT_ONE_SEC();
        printf(".");
        fflush(stdout);
    }
    printf("\n");

    return;
}

/**********************************************************************
 *
 * Function: canis_check_bmc_ready_wa
 *
 * check for BMC ready (work-around version)
 *
 *   Use this version before CPLD HW and BMC Foundation SW
 *      are finalized to support primary interface raedy
 *      (ie, BMC ready for Canis) as required by O2 NGIO architecture.
 *
 * Input : none
 *
 * Output: none
 *
 **********************************************************************
 */
void canis_check_bmc_ready_wa (void)
{
    n2g_i2c_if_t *pca = &pca_i2c;
    int timeout;
    uint8_t data = 0;

    /* wait for BMC to boot up */
    printf("Waiting %d minutes for Canis BMC to boot up", PRE_BMC_RDY_MINS);
    for (timeout = PRE_BMC_RDY_SECS; timeout; timeout--) {
        WAIT_ONE_SEC();
        printf(".");
        fflush(stdout);
    }
    printf("\n");

    /* check BMC ready bit */
    printf("Waiting up to %d minute for Canis BMC to be ready", BMC_RDY_MINS);
    for (timeout = BMC_RDY_SECS; timeout; timeout--) {
        if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE)) {
            ; /* I2C read error -- re-check next time */
        } else if (data & PRIM_INTF_READY) {
            /* BMC ready bit is set */
            break;
        }
        WAIT_ONE_SEC();
        printf(".");
        fflush(stdout);
    }
    printf("\n");
    if (timeout > 0) {
        printf("Canis BMC ready (bit 3) is on, INP=%#x\n", data);
    } else {
        printf("Warning: Canis BMC ready (bit 3) still off, INP=%#x\n", data);
    }

    /* wait for BMC to activate RBCPd */
    printf("Waiting %d minute for Canis BMC to activate RBCPd",
            POST_BMC_RDY_MINS);
    for (timeout = POST_BMC_RDY_SECS; timeout; timeout--) {
        WAIT_ONE_SEC();
        printf(".");
        fflush(stdout);
    }
    printf("\n");

    return;
}

/**********************************************************************
 *
 * Function: canis_ltc_reg_read
 *
 * Wrapper for LTC4215 Register Read utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long canis_ltc_reg_read (void)
{
    return util_oir_ltc4215_reg_read(oir);
}

/**********************************************************************
 *
 * Function: canis_ltc_reg_write
 *
 * Wrapper for LTC4215 Register write utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long canis_ltc_reg_write (void)
{
    return util_oir_ltc4215_reg_write(oir);
}

/**********************************************************************
 *
 * Function: canis_ltc_reg_test
 *
 * Wrapper for LTC4215 Register test.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long canis_ltc_reg_test (void)
{
    int rc;

    prpass(testpass, "LTC4215 OIR Register");

    rc = oir_ltc4215_register_test(oir);

    prcomplete(testpass, errcount, (char *)0);
    return rc? FAILED: PASSED;
}

/**********************************************************************
 *
 * Function: canis_power
 *
 * utility to power on/off Canis SM
 *
 * Input : on - ENABLE/DISABLE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long canis_power (long on)
{
    uint8_t data = 0;
    struct ngio_intf_t *canis_sm_iface;

    printf("\nPower %s Canis SM...\n", on? "On": "Off");

    assert(oir);
    assert(canis_iface_p);

    canis_sm_iface = canis_iface_p->canis_sm_iface;
    assert(canis_sm_iface);

    if (on) {
        /* turn on board power and take I2C out of reset */
        if (slot_i2c_unreset(canis_sm_iface, canis_iface_p->slot, "SM")) {
            return FAILED;
        }

        if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
            return FAILED;
        }

        if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
            return FAILED;
        }

        /* FET on */
        data |= LTC4215_FET_ON_CONTROL;
        if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
            return FAILED;
        }
        msleep(200);

        /* make sure the power is output good */
        if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
            return FAILED;
        }
        if (!(data & LTC4215_FET_ON_STATUS)) {
            printf("FET CANNOT be Turned On.\n");
            return FAILED;
        }
        if (data & LTC4215_POWER_BAD_STATUS) {
            printf("Power CANNOT be Turned On.\n");
            return FAILED;
        }

        printf("Waiting Canis SM to power up...\n");
        msleep(2000);

        /* turn on the green light */
        if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
            return FAILED;
        }

        /* turn on UART interface */
        canis_sm_iface->uart_on(canis_sm_iface);

        /* take Canis out of reset */
        canis_sm_iface->unreset(canis_sm_iface);
        msleep(1000);

        if (configure_i2c_expander(TRUE)) {
            cterr('w', 0, "unable to configure i2c_expander");
        }

        /* wait for BMC ready */
        (*wait_canis_bmc_ready_func)();

        printf("Canis SM is powered up.\n");
    } else {
        if (util_oir_ltc4215_led(oir, OIR_LED_OFF)) {
            return FAILED;
        }
        if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
            return FAILED;
        }
        /* FET off */
        data &= ~LTC4215_FET_ON_CONTROL;
        if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
            return FAILED;
        }
        printf("Canis SM is powered off.\n");
    }
    return PASSED;
}

/**********************************************************************
 *
 * Function: util_canis_power_on
 *
 * wrapper to power on Canis SM
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long util_canis_power_on (void)
{
    if (getc_answer("Proceed with Power On Canis SM? (y/n)", "yn", 'n')
                    == 'n') {
        return PASSED;
    }
    return canis_power(ENABLE);
}

/**********************************************************************
 *
 * Function: util_canis_power_off
 *
 * wrapper to power off Canis SM
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long util_canis_power_off (void)
{
    if (getc_answer("Proceed with Power Off Canis SM? (y/n)", "yn", 'n')
                    == 'n') {
        return PASSED;
    }
    return canis_power(DISABLE);
}

/**********************************************************************
 *
 * Function: util_canis_power_cycle
 *
 * wrapper to power cycle Canis SM
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long util_canis_power_cycle (void)
{
    int seconds = 10;

    if (getc_answer("Proceed with Power Cycle Canis SM? (y/n)", "yn", 'n')
                    == 'n') {
        return PASSED;
    }

    if (canis_power(DISABLE)) {
        printf("Failed to Power Off Canis SM.\n");
        return FAILED;
    }

    printf("Waiting %d seconds before power up", seconds);
    while (seconds--) {
        printf(".");
        fflush(stdout);
        WAIT_ONE_SEC();
    }
    printf("\n");

    if (canis_power(ENABLE)) {
        printf("Failed to Power On Canis SM.\n");
        return FAILED;
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: canis_pwrup_onoff
 *
 * enable/disable power up Intel side
 *
 * Input : on/off
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long canis_pwrup_onoff(int onoff)
{
    uint8_t data;

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("Failed to read Canis LTC4215 OIR fault register\n");
        return FAILED;
    }

    /* GPIO3 is a low true pin */
    if (onoff) {
        data &= ~LTC4215_GPIO3_OUT;
    } else {
        data |= LTC4215_GPIO3_OUT;
    }

    if (oir_ltc4215_reg_write(oir, LTC4215_FAULT_REG, &data)) {
        printf("Failed to write Canis LTC4215 OIR fault register\n");
        return FAILED;
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        printf("Failed to read Canis LTC4215 OIR status register\n");
        return FAILED;
    }

    sleep(1);

    printf("Canis LTC4215 OIR status: %#x, GPIO3 input (bit 2) is %d\n",
          data, ((data & LTC4215_GPIO3_IN)? 1: 0));

    return PASSED;
}

/**********************************************************************
 *
 * Function: switch_wait_canis_bmc_ready
 *
 * utility to select which function to wait for Canis BMC ready
 *
 * Input : none
 *
 * Output: none
 *
 **********************************************************************
 */
static long switch_wait_canis_bmc_ready (void)
{
    int ans;

    printf("select which function to wait for Canis BMC ready\n");
    printf("enter '1' to select ORIGINAL one\n");
    printf("enter '2' to select WORKAROUND one\n");
    ans = getdec_answer("enter 1 or 2", 1, 1, 2);

    if (ans == 1) {
        wait_canis_bmc_ready_func = canis_check_bmc_ready;
    } else {
        wait_canis_bmc_ready_func = canis_check_bmc_ready_wa;
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: show_oir_ltc4215_regs
 *
 * Display LTC4215 registers
 *
 * Input : none
 *
 * Output: none
 *
 **********************************************************************
 */
static void show_oir_ltc4215_regs (void)
{
    uint8_t data;

    printf("\nLTC4215 registers:\n");
    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        printf(" Failed to read Canis LTC4215 CONTROL register\n");
    } else {
        printf(" LTC4215 CONTROL: 0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir, LTC4215_ALERT_REG, &data)) {
        printf(" Failed to read Canis LTC4215 ALERT register\n");
    } else {
        printf(" LTC4215 ALERT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        printf(" Failed to read Canis LTC4215 STATUS register\n");
    } else {
        printf(" LTC4215 STATUS:  0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf(" Failed to read Canis LTC4215 FAULT register\n");
    } else {
        printf(" LTC4215 FAULT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data)) {
        printf(" Failed to read Canis LTC4215 SENSE register\n");
    } else {
        printf(" LTC4215 SENSE:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data)) {
        printf(" Failed to read Canis LTC4215 SOURCE register\n");
    } else {
        printf(" LTC4215 SOURCE:  0x%02x\n", data);
    }
}

/**********************************************************************
 *
 * Function: show_oir_ltc4215_regs
 *
 * Utility to display LTC4215 registers
 *
 * Input : none
 *
 * Output: none
 *
 **********************************************************************
 */
static long util_show_oir_ltc4215_regs (void)
{
    show_oir_ltc4215_regs();

    return PASS;
}

static long util_debug_o2_power_fault (void)
{
    if (getc_answer("Turn on power fault debug flag? (y/n)", "yn", 'n')
        == 'y') {
        debug_o2_power_fault_flag = 1;
    } else {
        debug_o2_power_fault_flag = 0;
    }

    return PASS;
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: sm_canis.c,v $
 * Revision 1.32  2018/05/22 02:31:11  alpeng
 * fixed compiler warning, CSCvj57934
 *
 * Revision 1.31  2018/05/18 09:24:49  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.30.48.1  2016/12/05 06:36:59  alpeng
 * fixed the uart ctrl num for ngio; change is approved on prrq
 *
 * Revision 1.30  2014/02/18 09:11:12  alpeng
 * CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
 *
 * Revision 1.29  2013/11/26 08:40:34  hroni
 * fix compiler warning
 *
 * Revision 1.28  2013/11/11 21:18:39  mcharon
 * pass string instead of number in first argum of host_send_packet ; add xaui support
 *
 * Revision 1.27  2013/05/14 23:32:47  shhuang
 * Changed post BMC ready wait to 5 minutes per O2/Canis RDT meeting.
 *
 * Revision 1.26  2013/05/09 19:25:18  mcharon
 * remove unused header files. fixed dependancy compile problem
 *
 * Revision 1.25  2013/05/09 18:07:04  shhuang
 * Add power fault debug hook.
 *
 * Revision 1.24  2013/05/02 21:35:57  shhuang
 * Add Canis BMC ready bit set too early work-around function. (CSCug49316)
 * Add utility to switch to the work-around function.
 *
 * Revision 1.23  2013/03/14 17:19:40  shhuang
 * Extend BMC Linux boot up ready time check for Canis phase-2. (CSCuf28643)
 *
 * Revision 1.22  2013/02/21 19:20:43  shhuang
 * Set LTC4215 GPIO3 pin for Canis BMC to power up Intel side.
 *
 * Revision 1.21  2012/12/20 06:24:17  hondwang
 * Fill matrix valuse. Print debug info and increase retry to six
 *
 * Revision 1.20  2012/11/12 20:35:23  mcharon
 * add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting
 *
 * Revision 1.19  2012/10/12 00:22:20  shhuang
 * Minor debugging message clean up.
 *
 * Revision 1.18  2012/10/11 07:28:20  hondwang
 * porting multi card insert issue fix from G2. CSCua22608
 *
 * Revision 1.17  2012/09/18 20:40:39  shhuang
 * Added canis interface test. Cleaned up.
 *
 * Revision 1.16  2012/08/02 18:49:41  shhuang
 * Added LTC4215 register r/w test and utilities.
 * Added Canis SM power off/on/cycle utilities.
 * Minor fixes of testname/prpass strings.
 *
 * Revision 1.15  2012/07/10 00:43:39  shhuang
 * Added dummy canis_saved_diag_exec function.
 *
 * Revision 1.14  2012/07/09 19:27:48  shhuang
 * Added function to check BMC ready during Canis menu init.
 * Added i2c io expander register r/w test.
 * Added utilities to init and show i2c io expander regs.
 * Cleaned up other i2c io expander code.
 *
 * Revision 1.13  2012/06/27 09:39:46  hondwang
 * revise canis source code for proper indent
 *
 * Revision 1.12  2012/06/27 07:22:38  hondwang
 * Add slot variable for set canis loopback function
 *
 * Revision 1.11  2012/06/26 12:44:01  hondwang
 * add Canis GE switch line loopback function
 *
 * Revision 1.10  2012/06/23 20:29:05  ksabzwar
 * Added i2c expander (PCA9557) test
 *
 * Revision 1.9  2012/06/08 06:45:05  hondwang
 * Fix canis complier warning on O2 x86
 *
 * Revision 1.8  2012/06/07 06:50:06  shhuang
 * Fixed compile warning: implicit declaration of function: system.
 *
 * Revision 1.7  2012/05/04 20:01:46  mcharon
 * use void* instead of int as argument to to func ptrs in ngio_intf
 *
 * Revision 1.6  2012/04/24 08:30:56  hondwang
 * Add RBCP for Canis
 *
 * Revision 1.5  2012/04/12 18:32:13  shhuang
 * Minor clean-up.
 *
 * Revision 1.4  2012/04/10 15:52:17  ksabzwar
 * clean up I2C expander functions
 *
 * Revision 1.3  2012/04/10 06:10:46  hondwang
 * fix warning message
 *
 * Revision 1.2  2012/04/06 18:52:46  shhuang
 * Added console redirect via picocom.
 * Added util to execute a shell command from menu.
 * Added util to escape to shell from menu.
 *
 * Revision 1.1  2012/03/29 18:46:42  ksabzwar
 * Initial check in into ng_diag
 *
 * Revision 1.1.2.3  2012/03/27 17:18:18  ksabzwar
 * add BMC console switch diag option
 *
 * Revision 1.1.2.2  2012/03/10 02:00:36  ksabzwar
 * added some debugging
 *
 * Revision 1.1.2.1  2012/03/10 01:18:29  ksabzwar
 * First check-in for Canis user menu for Overloard platform
 *
 * Revision 1.1.2.1  2012/02/14 21:00:47  ksabzwar
 * Added Canis SM card entry function for sanity test on Overlord
 *
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

