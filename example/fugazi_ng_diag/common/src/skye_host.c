/* $Id: skye_host.c,v 1.6 2019/08/06 06:56:06 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_host.c,v $
 *------------------------------------------------------------------
 *
 * skye_host.c: Main file for Skye host side Diag.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
 
/*------------------------------------------------------------------------------
 * includes
 *------------------------------------------------------------------------------
 */
#include <sys/time.h>
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
#include "router_if.h"
#include "pca.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "act2_utils.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include <assert.h>
#include "defs.h"
#include "common_utils.h"
#include "skye_host.h"

/*------------------------------------------------------------------------------
 * globals
 *------------------------------------------------------------------------------
 */
#define ENABLE_DELAY   600   /* Ported from /overlord/ngio.c */
ushort cpu_cookie = 0;
int cpu_id = CPU0;
static short linepos;
int skye_test_slot = 0;
boolean skye_e0_10g_cap = FALSE;
static void           (*skye_saved_diag_exec)(void) = NULL;
static int (*savfcn)() = NULL;
static skye_ds_t skye_info;
static skye_ds_t *skye_iface_p;
skye_ds_t        skye_iface[MAX_SM+1];
static speed_t        slot1_uart = 1;   /* set default baud rate to 9600 */
static speed_t        slot2_uart = 1;
boolean freq_margin = FALSE;

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
static int skye_dual_cpu_xaui_test(int);
static int  skye_ioe_reg_test(void);
static int  skye_ioe_init(void);
static int  skye_ioe_reg_dump_util(void);
static int  skye_ioe_reg_rd_util(void);
static int  skye_ioe_reg_wr_util(void);
static long ge_bp_lpbk_test(void);
static int  pwr_off_skye_sm_wrap(void);
static int  pwr_off_skye_sm(void);
static int  pwr_on_skye_sm(void);
static int  pwr_cycle_skye_sm(void);
static int  skye_submodule_reset(void);
static int  unreset_skye_sm(void);
static int  ltc4215_register_test(void);
static int  ltc4215_led_test(void);
static int  oir_ltc4215_tests(int);
static int  util_ltc4215_reg_read(void);
static int  util_ltc4215_reg_write(void);
static int  set_skye_uart_baud(void);
static int  show_skye_sm_pwr(void);
static int  skye_host_utility(int);
static int  skye_sm_cpu0_console(int);
static int  skye_sm_cpu1_console(int);
static int  skye_sm_init(void);
static void skye_sm_cleanup(void);
static void skye_setup_uart(void);
static int  act2lite_tests(int);
static uint32_t get_skye_sm_current(uint8_t);
static int  skye_hipwr_ctrl_util(int);
static long skye_gesw_lpbk_set(void);
static long skye_boot_image(int);
static int is_skye_up(boolean, int, boolean);
static long skye_run_sm_test(int);
int set_skye_hipwr(boolean);
int skye_tx_uart(int, char *);
int skye_rx_polling_uart(int, char *, int);
int is_skye_uart(int);
boolean has_10GKR_cap(void);
static int skye_exec_apps(int, char *);
boolean skye_one_cpu(void);
boolean skye_two_cpu(void);
int check_skye_ready(void);
static int skye_set_tlk_1gkx_lpbk_bit(boolean);
static int skye_o2_shell(void);
static int skye_o2_command(void);
static long ge_bp_tlk_lpbk_test(void);
static int pre_skye_test(int, char *);
static int ping_cpu1_xaui_test(int);
static int ping_cpu0_10g_test(int);
boolean no_10GKR_cap(void);
boolean mask_out(void);

/*------------------------------------------------------------------------------
 * Externs
 *------------------------------------------------------------------------------
 */
extern int act2_version(int);  
extern int sgmii_lpbk_util(int, int);
extern int tftp_get (char *, char *, char *, char *, int);
extern int skye_cpu_alive_check(int);
extern int skye_do_all(int);
extern void skye_clrline(char *);
extern int main_thread_wait_time;
extern void ngio_ge_cfg(struct ngio_intf_t *);
extern int save_to_log_utils(void);
extern int get_skye_vm_setup_util(void);
extern int skye_exec_test(int, char *);
extern int get_skye_fm_setup_util(void);
extern int resend_skye_kernel_boot_cmd(void);

/* Added these for Overlord GE API
 */
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);

/*------------------------------------------------------------------------------
 * constants
 *------------------------------------------------------------------------------
 */
static uart_baud_info skye_uart_baud[] = {
    {"115200",   B115200},
    {"9600",     B9600}
};

/* 
 * Sub Menu used for Utility.
 */
static mitem_t skye_util_submenu_table[] = {
    { "LTC4215 Register Read",           0, 0,   (PFT)util_ltc4215_reg_read,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "LTC4215 Register Write",          0, 0,   (PFT)util_ltc4215_reg_write,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "IO Expander(PCA9555) Reg. Dump",  0, 0,   (PFT)skye_ioe_reg_dump_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "IO Expander(PCA9555) Reg. Read",  0, 0,   (PFT)skye_ioe_reg_rd_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "IO Expander(PCA9555) Reg. Write", 0, 0,   (PFT)skye_ioe_reg_wr_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Enable Skye High Power",     0, 0,   (PFT)skye_hipwr_ctrl_util,
      (type_t *)&one, 0,    (type_t(*)())0, 0 },
    { "Disable Skye High Power",    0, 0,   (PFT)skye_hipwr_ctrl_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Display Skye SM Power",      0, 0,   (PFT)show_skye_sm_pwr,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Skye Sub-Module Reset",       0, 0,   (PFT)skye_submodule_reset,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Power Off Skye SM",          0, 0,   (PFT)pwr_off_skye_sm_wrap,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Power On Skye SM",           0, 0,   (PFT)pwr_on_skye_sm,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Power Cycle Skye SM",        0, 0,   (PFT)pwr_cycle_skye_sm,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Set Skye UART Baud Rate",    0, 0,   (PFT)set_skye_uart_baud,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Platform GE Switch Line Loopback Setup", 0, 0, (PFT)skye_gesw_lpbk_set,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Skye CPU0 alive check",           0, 0,   (PFT)skye_cpu_alive_check,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Skye CPU1 alive check",           0, 0,   (PFT)skye_cpu_alive_check,
      (type_t *)&one, 0,   (type_t(*)())skye_two_cpu, 0 },
    { "Dump Skye_error_log",           0, 0,   (PFT)save_to_log_utils,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Check Skye ready pin",           0, 0,   (PFT)check_skye_ready,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Set Skye voltage margin by NC",   0, 0,  (PFT)get_skye_vm_setup_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Escape to Shell (debugging only)",  0, 0,   (PFT)skye_o2_shell,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Execute a Shell command (debugging only)",   0, 0,  (PFT)skye_o2_command,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Test 10G-KR loopback from host",   0, 0,  (PFT)ge_bp_tlk_lpbk_test,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Set Skye frequency margin by NC",   0, 0,  (PFT)get_skye_fm_setup_util,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Test Dual CPU XAUI for debug",   0, 0,  (PFT)skye_dual_cpu_xaui_test,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
};

#define SKYE_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(skye_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t skye_util_subtest_menu = {
    "Skye Host Utilities Menu",
    (type_t)0,                             /* title param */
    (PFT)menu_show_dflags,                 /* show diag flags */
    0,
    SKYE_UTIL_SUBMENU_TABLE_SZ,
    skye_util_submenu_table,
};

static menuinfo_t *skye_util_submenup = &skye_util_subtest_menu;

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
 * Sub Menu used for Skye Host side tests.
 */
static submenu_xtable_t skye_submenu_table[] = {
    {"Skye Host Utilities",         (PFT)skye_host_utility,       0,
     0,
     (type_t(*)())0, 0,                  (PFT)skye_host_utility,       0},
    {"ACT2 Lite Utilities",                   (PFT)act2lite_tests,               0,
     MF_CONTINUOUS,
     (type_t(*)())0, 0,                  (PFT)act2lite_tests,               TRUE},
    {"Skye IO Expander Reg. Test",  (PFT)skye_ioe_reg_test,       0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0, 0,                  (type_t(*)())0,                    0},
    {"OIR(LTC4215) Test",                (PFT)oir_ltc4215_tests,            0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0, 0,                  (PFT)oir_ltc4215_tests,            TRUE},
    {"Skye CPU0 Console",           (PFT)skye_sm_cpu0_console,    0,
     0,
     (type_t(*)())0, 0,       (type_t(*)())0,                    0},
    {"Skye CPU1 Console",           (PFT)skye_sm_cpu1_console,    0,
     0,
     (type_t(*)())skye_two_cpu, 0,       (type_t(*)())0,                    0},
    { "Run Skye CPU0 SM Test", (PFT)skye_run_sm_test, 0,
      MF_CONTINUOUS | MF_DOALL,
      (type_t(*)())0, 0,      (type_t(*)())0,                    0 },
    { "Run Skye CPU1 SM Test", (PFT)skye_run_sm_test, 1,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())skye_two_cpu, 0,       (type_t(*)())0,                    0 },
    { "Run Skye Dual CPU XAUI ping Test", (PFT)ping_cpu1_xaui_test, 0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())skye_two_cpu, 0,       (type_t(*)())0,                    0 },
    { "TLK10232 Loopback Test From Host side GESW", (PFT)ge_bp_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())no_10GKR_cap, 0,       (type_t(*)())0,                    0},
    { "Run Ping Test 10G-KR interface", (PFT)ping_cpu0_10g_test, 0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())mask_out, 0,       (type_t(*)())0,                    0},
    {"Firmware Download CPU 0",                (PFT)skye_boot_image,         0,
     MF_CONTINUOUS,
     (type_t(*)())0, 0,       (type_t(*)())0,                    0},
    {"Firmware Download CPU 1",                (PFT)skye_boot_image,         1,
     MF_CONTINUOUS,
     (type_t(*)())skye_two_cpu, 0,       (type_t(*)())0,                    0},
};

#define SKYE_SUBMENU_TABLE_SIZE (sizeof(skye_submenu_table) / \
                                      sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t skye_tests_primary_items[SKYE_SUBMENU_TABLE_SIZE + 
                                            MAX_BASE_ITEMS];
static mitem_t skye_tests_secondary_items[SKYE_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

static menuinfo_t skye_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    skye_tests_primary_items,
};

static menuinfo_t *skye_submenup = &skye_subtest_menu;

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
 * Function: skye_gesw_lpbk_set
 *
 * Description: This function is the wrapper to enables or disable the line
 *              loopback of the GE switch port connected to the Skye.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long skye_gesw_lpbk_set (void)
{
    uint8_t ans;
    uint port;
    int ge_port, ge_port0, ge_port1, ge_port2;
    int real_slot = skye_iface_p->slot;

    printf("\n");
    testname("Switch the Overlord GESW");

    if (skye_e0_10g_cap == FALSE) {
        port = gethex_answer("Overlord GESW port E0/E1/E2(XAUI)", 0, 0, 2);
    } else {
        port = gethex_answer("Overlord GESW port E0/E1", 0, 0, 1);
    }
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
    if (skye_e0_10g_cap == FALSE) {
        ge_port2 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 2);
    }
    printf("\nSM slot %d, E0 : GESW port %d = %s",real_slot, ge_port0 ,
            get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
    printf("\nSM slot %d, E1 : GESW port %d = %s",real_slot, ge_port1 ,
            get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
    if (skye_e0_10g_cap == FALSE) {
        printf("\nSM slot %d, E2 : GESW port %d = %s\n",real_slot, ge_port2 ,
            get_gesw_line_loopback(ge_port2) ? "ENABLE" : "DISABLE");
    }
    fflush(0);

    return(PASSED);
}


/**********************************************************************
*
* Function: ge_bp_lpbk_test
*
* Description: Perform GE loopback test to verify GE0 backplane
*               connectivity between host and SM card
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
    int packet_no = SKYE_GE_BP_PACKET_NO;
    int ctrl_plane_sgmii_port;

    testname(" GE Backplane Loopback");
    prpass(testpass, "Setting TLK10232 loopback bit");

    if (skye_set_tlk_1gkx_lpbk_bit(TRUE) == FAILED) {
        cterr('f', 0, "Setup GE loopback on module side fails.");
        return (FAILED);
    }
    /* To wait setup done */
    prpass(testpass, "Running loopback test now");

    ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port();
    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, packet_no) != PASSED) {
        cterr('f', 0, "GE loopback from Host side GE%d fails.",
                ctrl_plane_sgmii_port);
        rc = (FAILED);
    } else {
        printf("Loopback test pass.\n");
    }

    if (skye_set_tlk_1gkx_lpbk_bit(FALSE) == FAILED) {
        cterr('f', 0, "Disable GE loopback on module side fails.");
        return (FAILED);
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
    return (util_oir_ltc4215_reg_write((void *)oir));
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
 * Function   : show_skye_sm_pwr
 * Description: Display power of Skye.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
show_skye_sm_pwr (void)
{
    uint32_t voltage, current, power;
    uint8_t data = 0;

    printf("\n\nSkye SM Power Measure:\n\n");

    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data)) {
        return (FAILED);
    }
    voltage = (data * SINGLE_SM_VOL) / 100;

    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data)) {
        return (FAILED);
    }
    current = get_skye_sm_current(data) / 100;

    power = voltage * current;

    printf("Voltage = %d.%02d V\n", (voltage / 100), (voltage % 100));
    printf("Current = %d.%02d A\n", (current / 100), (current % 100));
    printf("Power = %d.%02d W\n", (power / 10000), ((power % 10000) / 100));

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : get_skye_sm_current
 * Description: convert sense register value into current.
 * Inputs     : Sense Register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t
get_skye_sm_current (uint8_t data)
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
 * Function   : pwr_off_skye_sm_wrap
 * Description: Warpped function to power-off Skye SM.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_off_skye_sm_wrap (void)
{
    uint8_t ans;

    printf("\n\nReally want to Power off Skye? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nUser Abort! Skye SM will stay in power ON.\n\n");
        return (PASSED);
    }
    return (pwr_off_skye_sm());
}

/*******************************************************************************
 *
 * Function   : pwr_off_skye_sm
 * Description: Function to power off Skye SM.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_off_skye_sm (void)
{
    uint8_t data = 0;

    printf("\nPower Off the Skye SM.\n");

    /* disable power fault interrupt */
    ngiosm_disable_intr(skye_iface_p->slot, NGIO_FLT_INTR);

    if (util_oir_ltc4215_led(oir, OIR_LED_OFF)) {
        printf("Failed to turn OIR LED OFF.\n");
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        printf("Failed to read LTC4215 Control Reg.(0x%02X)\n",
               LTC4215_CONTROL_REG);
        return (FAILED);
    }

    /* power off SM module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        printf("Failed to power off SM module by setting LTC4215 Control"
               " Reg.(0x%02X) bit %d to 0.\n",
               LTC4215_CONTROL_REG, LTC4215_FET_ON_CONTROL);
        return (FAILED);
    }
    msleep(200);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pwr_on_skye_sm
 * Description: Function to power on Skye SM.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_on_skye_sm (void)
{
    uint8_t data = 0;

    printf("\nPower On the Skye SM.\n");

    /* Enable NGSM and take I2C out of reset */
    printf("Enable NGSM and take its I2C out of reset.\n");
    if (slot_i2c_unreset(skye_iface_p->skye_sm_iface,
                         skye_iface_p->slot, "SM") != PASSED) {
        printf("FAILED to Eable NGSM and take its I2C out of rest.\n");
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* Power on SM module */
    printf("Power on Skye module by set bit%d of Reg. 0x%#X.\n",
           LTC4215_FET_ON_CONTROL, LTC4215_CONTROL_REG);
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        printf("Failed to power on Skye module"
               "(by set bit%d of Reg. 0x%#X).\n",
               LTC4215_FET_ON_CONTROL, LTC4215_CONTROL_REG);
        return (FAILED);
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

    printf("Waiting for Skye SM to Power-Up.\n");
    /* Based Current FPGA Design, we need to unreset skye CPU within 500 ms after
     * confirm with Skye power good signal */
    msleep(500);

    /* Release Skye SM from Reset state */
    unreset_skye_sm();

    /* disable power fault interrupt */
    ngiosm_enable_intr(skye_iface_p->slot, NGIO_FLT_INTR);
    msleep(ENABLE_DELAY);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pwr_cycle_skye_sm
 * Description: Wrapped function to power cycle Skye SM.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
pwr_cycle_skye_sm (void)
{
    uint8_t i, ans;

    printf("\n");
    testname("Power Cycle Skye SM");

    printf("\n\nReally want to Power Cycle Skye? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nUser Abort! "
                "Stop power-cycle Skye process.\n\n");
        return (PASSED);
    }

    if (pwr_off_skye_sm() != PASSED) {
        cterr('f', 0, "Failed to Power OFF the Skye SM");
        return (FAILED);
    }

    /* msleep for 10 seconds ?? Check if Skye needs. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }
    printf("\n");

    if (pwr_on_skye_sm() != PASSED) {
        cterr('f', 0, "Failed to Power ON the Skye SM");
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_skye_uart_baud
 * Description: Function to set Skye uart baud rate.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
set_skye_uart_baud (void)
{
    const   int maxlen = 128;
    char    tty[maxlen];
    int     fd, slot;
    struct  termios  newtio, ori_conf;
    skye_ds_t   *iface;
    speed_t new_baud = 0, uart_baud_rate = 0;

    iface = skye_iface_p;
    assert(iface);
    slot = skye_iface_p->slot;

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", skye_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }
    tcgetattr(fd, &newtio);

    printf("\n\n Set Skye UART Baud Rate: \n");
    uart_baud_rate = getdec_answer("\nBaudrate (0-115200, 1-9600):", 0, 0, 1);
    if ( slot == SKYE_SLOT1) {
        slot1_uart = uart_baud_rate;
    } else {
        slot2_uart = uart_baud_rate;
    }    
    
    new_baud = skye_uart_baud[uart_baud_rate].baud_rate;

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
 * Function   : skye_submodule_reset
 * Description: Function to reset Skye SubModule.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_submodule_reset (void)
{
    int user_ch = 4;

    printf("\nUtility to Reset Skye SM:\n");
    printf("1. Put Skye SM into Reset state.\n");
    printf("2. Unreset Skye SM.\n");
    printf("3. Perform completed Reset Skye SM process.\n");
    printf("4. Exit.\n");
    user_ch = getdec_answer("Please enter your choice: ", 4, 1, 4);

    /* Exit utility */
    if (user_ch == 4) {
        printf("\nExit based on user's request.\n");
        return (PASSED);
    }

    /* Reset Skye SM */
    if ((user_ch == 1) || (user_ch == 3)) {
        if (user_ch == 1) {
            printf("\nStart to put Skye SM into Reset state... ");
        } else {
            printf("\nStart to Reset Skye SM... ");
        }

        /* Turn OIR LED to Amber */
        if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
            printf("Failed to change OIR LED to Amber.\n");
            return (FAILED);
        }

        /* Put Skye module into Reset */
        skye_iface_p->skye_sm_iface->reset(skye_iface_p->skye_sm_iface);
        if (user_ch == 1) {
            printf("Done successfully.\n");
            return (PASSED);
        }

        /* delay time for processing, need check */
        msleep(500);
    }

    /* Unreset Skye SM */
    if (user_ch == 2) {
        printf("\nStart to Unreset Skye SM... ");
    }

    if (unreset_skye_sm() == PASSED) {
        printf("Done successfully.\n");
        return (PASSED);
    }
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : unreset_skye_sm
 * Description: Function to release Skye SubModule from Reset state.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
unreset_skye_sm (void)
{
    /* Release Skye module from Reset */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Release Skye SM from Reset state.\n");
    }
    skye_iface_p->skye_sm_iface->uart_on(skye_iface_p->skye_sm_iface);
    skye_iface_p->skye_sm_iface->unreset(skye_iface_p->skye_sm_iface);

    /* After unreset Skye, needs to wait seconds for FPGA to get ready. */
    msleep(SKYE_FPGA_READY_TIME);

    /* Configure the NGIO GE ports for 1G or 10G-KR.
     */
    ngio_ge_cfg(skye_iface_p->skye_sm_iface);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Setup Skye SM UART.\n");
    }
    skye_setup_uart();

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reinitialize Skye SM.\n");
    }
    if (skye_sm_init()) {
        printf("Failed to init Skye SM.\n");
        skye_sm_cleanup();
        return (FAILED);
    }

    /* Turn on the Green light */
    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        printf("Failed to turn OIR LED to Green.\n");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_setup_uart
 * Description: Function to setup Skye UART interface parameter.
 * Inputs     : NONE
 * Outputs    : NONE 
 *
 *******************************************************************************
 */
static void
skye_setup_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd;
    struct termios oldtio, newtio;

    assert(skye_iface_p); 

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", skye_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = B9600|CS8|CLOCAL|CREAD;
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
 * Function   : skye_sm_cleanup
 * Description: This function perform the cleanup task before exiting
 *              the test.
 * Inputs     : NONE
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static void
skye_sm_cleanup (void)
{
    skye_ds_t     *skye_info_p = &skye_info;
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    char cmd[CMD_LENGTH];

    int ge_port0, ge_port1, ge_port2;

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    data |= SKYE_SUBMOD_RESET;
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    msleep(500);

    /* Mask host side slot interrupt */

    ge_port0 = ovld_get_ge_sw_port_num(skye_info_p->slot, TGT_DEV_NGSM, 0);
    set_gesw_line_loopback(ge_port0, 0);
    ge_port1 = ovld_get_ge_sw_port_num(skye_info_p->slot, TGT_DEV_NGSM, 1);
    set_gesw_line_loopback(ge_port1, 0);
    if (skye_e0_10g_cap == FALSE) {
        ge_port2 = ovld_get_ge_sw_port_num(skye_info_p->slot, TGT_DEV_NGSM, 2);
        set_gesw_line_loopback(ge_port2, 0);
    }

    skye_e0_10g_cap = FALSE;
    freq_margin = FALSE;

    /* Remove the route table for nc communicate with CPU 1 */
    /* to remove the route after exit the program */
    /* route del default gw 192.123.123.101 */
    if (cpu_id == CPU1) {
        sprintf(cmd, "route del default gw %s.%d", SKYE_CPU0_IP_ADDR_SUBNET,
            SKYE_DIAG_IP_ADDR_BASE + skye_test_slot);
        system(cmd);
        cpu_id = CPU0;
    }

    if (savfcn) {
        savfcn = NULL;
    }

    if (skye_saved_diag_exec) {
        pre_diag_exec = skye_saved_diag_exec;
        skye_saved_diag_exec = NULL;
    }
}

/*******************************************************************************
 *
 * Function   : skye_sm_cpu0_console
 * Description: Function to redirect console to Skye SM CPU0.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int
skye_sm_cpu0_console (int show_menu)
{
    skye_ds_t   *iface;
    int slot;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;
    uint8_t data = 0;

    iface = skye_iface_p;
    assert(iface);
    slot = skye_iface_p->slot;
    assert((slot == 1) || (slot == 2));

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
    data &= ~(SKYE_UART_MUX_SEL);

    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    if (slot == SKYE_SLOT1) {
        new_baud = slot1_uart;
    } else {
        new_baud = slot2_uart;
    }    

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d", 
             new_baud ? "b9600" : "b115200", skye_iface_p->uart);

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_sm_cpu1_console
 * Description: Function to redirect console to Skye SM CPU1.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int
skye_sm_cpu1_console (int show_menu)
{
    skye_ds_t   *iface;
    int slot;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;
    uint8_t data = 0;

    iface = skye_iface_p;
    assert(iface);
    slot = skye_iface_p->slot;
    assert((slot == 1) || (slot == 2));

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
    data |= SKYE_UART_MUX_SEL;

    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    if (slot == SKYE_SLOT1) {
        new_baud = slot1_uart;
    } else {
        new_baud = slot2_uart;
    }    

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d", 
             new_baud ? "b9600" : "b115200", skye_iface_p->uart);

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_host_utility
 * Description: Function to build Skye SM host side utilities submenu.
 * Inputs     : show menu option
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_host_utility (int show_menu)
{
    menu(skye_util_submenup, skye_util_submenu_table, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_ioe_init
 * Description: Function to init Skye I2C IO Expander.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_ioe_init (void)
{
    uchar io_port_conf0 = 0, io_port_conf1 = 0, io_port_output0 = 0;
    uchar io_port_output1 = 0;
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

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT1_REG,
                              &io_port_output1, TRUE)) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s:%d, default io_port_conf0 = 0x%02X\n",
               __FUNCTION__, __LINE__, io_port_conf0);
        printf("%s:%d, default io_port_conf1 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_conf1);
        printf("%s:%d, default io_port_output0 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_output0);
        printf("%s:%d, default io_port_output1 = 0x%02X\n\n",
               __FUNCTION__, __LINE__, io_port_output1);
    }

    /* Based on Skye HFS, we need to configure -
     * IO port 0.0, 0.3 as input(bit set to 1),
     * and IO port 0.1, 0.2, 0.4, 0.5, 1.0, 1.1 as output(bit set to 0).
     */
    io_port_conf0 &= (SKYE_DB_PRESENT | SKYE_PRI_INF_RDY);
    io_port_conf0 |= (SKYE_DB_PRESENT | SKYE_PRI_INF_RDY);

    /* Configure the NGIO GE ports for 1G or 10G-KR.
     */
    ngio_ge_cfg(skye_iface_p->skye_sm_iface);

    /* Check 10G Capability */
    if (io_port_output1 & SKYE_E0_10G_CAP) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("*** DBG: Test 10GKR check\n");
        }
        io_port_conf1 &= (SKYE_E0_10G_CAP | SKYE_E1_10G_CAP);
        io_port_conf1 |= SKYE_E0_10G_CAP;
        skye_e0_10g_cap = TRUE;
    }

    io_port_output0 |= (SKYE_SUBMOD_RESET | SKYE_RESET_CONF);

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
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
        cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* Based on ShinkRay HFS, GPIO Expander Definition,
     * GPIO[2] Alien Sub-module reset_l
     * 0 = Place alien sub-module in reset.
     * 1 = Release alien sub-module from reset.
     */
    data &= ~(SKYE_SUBMOD_RESET);
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    /* delay time for processing, need check */
    msleep(500);

    data |= SKYE_SUBMOD_RESET;
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
 * Function   : skye_sm_init
 * Description: Function to init Skye SM.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_sm_init (void)
{
    skye_ds_t     *skye_info_p;
    skye_info_p = &skye_info;
    int ge_port0, ge_port1, ge_port2;
    int real_slot = skye_info_p->slot;
    
    skye_e0_10g_cap = FALSE;
    freq_margin = FALSE;
    /* Initial the GPIO */
    if (skye_ioe_init() != PASSED) {
        cterr('f', 0, "%s: Unable to configure Skye IO Expander",
                      __FUNCTION__);
        return (FAILED);
    }

    /* setup the backplane loopback here. We will do the loopback test
       from Tile CPU */
    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0);
    /* Set 10G or 1G capability */
    if (skye_e0_10g_cap == TRUE) {
        /* Default disable the loopback */
        printf("Skye E0-10GKR-Capability.\n");
        set_gesw_line_loopback(ge_port0, 0);
    } else {
        printf("Skye E0-1GKX-Capability.\n");
        set_gesw_line_loopback(ge_port0, 0);
    }
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1);
    set_gesw_line_loopback(ge_port1, 0);
    /* Non-Greyhound use XAUI to test full data path TLK */
    if (skye_e0_10g_cap == FALSE) {
        ge_port2 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 2);
        set_gesw_line_loopback(ge_port2, 1);
    }

    if (diagflag_xram & D_VERBOSE) {
        printf("\nSM slot %d, E0 : GESW port %d = %s",real_slot, ge_port0 ,
                get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
        printf("\nSM slot %d, E1 : GESW port %d = %s",real_slot, ge_port1 ,
                get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
        if (skye_e0_10g_cap == FALSE) {
            printf("\nSM slot %d, E2 : GESW port %d = %s\n",real_slot, ge_port2 ,
                get_gesw_line_loopback(ge_port2) ? "ENABLE" : "DISABLE");
        }
        fflush(0);
    }

    return (PASSED);
}


/**********************************************************************
 * Function: skye_sm_test
 *
 * Description: This function is the main program of the Skye test.
 *         This function will be called from platform_sm.h.
 *              Upon entering this function, the slot number will be
 *              checked to see if the user chose to execute all tests
 *              or entering the submenu and executes accordingly.
 *
 * Input:  The slot number of the Skye SM
 *
 * Output: PASSED if all the tests PASSED
 *         FAILED if at least one of them FAILED
 *
 **********************************************************************
 */
int
skye_sm_test (void *sm)
{
    int               real_slot, ret_val = PASSED;
    struct ngio_intf_t *skye_sm_iface = (struct ngio_intf_t *)sm;
    skye_ds_t     *skye_info_p;
    ushort cookie_id = 0;
    skye_info_p       = &skye_info;

    real_slot = skye_sm_iface->slot;
    cookie_id = skye_sm_iface->id;
    printf ("\n SKYE is in slot %d, Cookie Id is %x \n", real_slot, cookie_id);

    skye_test_slot = skye_sm_iface->slot;
    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = skye_sm_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    oir = (n2g_i2c_if_t *)skye_sm_iface->oir;

    /*
     * Initialize an instance of Skye data structure
     */
    skye_iface_p = (skye_ds_t *) &skye_iface[real_slot];
    skye_iface_p->cookie_id            = cookie_id;
    skye_iface_p->slot                = real_slot;
    skye_iface_p->uart                = skye_sm_iface->uart_ctrl;
    skye_iface_p->skye_sm_iface = (struct ngio_intf_t *)sm;
    skye_info_p->slot = real_slot;

   /*
     * Release Skye SM out of reset.
     */
    /* uart/i2c unreset should be done via function pointer passed into the
       entry point */
    skye_sm_iface->i2c_unreset(skye_sm_iface);
    skye_sm_iface->uart_on(skye_sm_iface);
    skye_sm_iface->unreset(skye_sm_iface);

    /* After unreset Skye, needs to wait seconds for FPGA to get ready. */
    msleep(SKYE_FPGA_READY_TIME);

    /* Configure the NGIO GE ports for 1G or 10G-KR.
     */
    ngio_ge_cfg(skye_sm_iface);
    assert(sm);
    skye_setup_uart();

    cpu_cookie = cookie_id;
    switch (cookie_id) {
    case SKYE_1CPU_ID:
        sprintf((char *)skye_iface_p->testname, "Slot%d Skye 1CPU SM", real_slot);
        break;
    case SKYE_2CPU_ID:
        sprintf((char *)skye_iface_p->testname, "Slot%d Skye 2CPUs SM", real_slot);
        break;
    default:
        cterr('f', 0, "Invalid Skye cookie id %#.4x in slot %d", 
                                          cookie_id, real_slot);
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER) == FAILED) {
        skye_sm_cleanup();
        return (FAILED);
    };

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY) == FAILED) {
        skye_sm_cleanup();
        return (FAILED);
    };

    if (skye_sm_init()) {
        skye_sm_cleanup();
        return(FAILED);
    }

    /* Enable Hi-Power mode is to power up CPU1.
     * So only need it when we want to test both CPU.
     */
    if (cpu_cookie == SKYE_2CPU_ID) {
        if (set_skye_hipwr(ENABLE) != PASSED) {
            printf("Failed to Enable Hi-Power mode.\n");
            skye_sm_cleanup();
            return (FAILED);
        }
        printf("Enabled Hi-Power mode.\n");
    }

    build_primary_submenu(skye_submenu_table, 
                          SKYE_SUBMENU_TABLE_SIZE,
                          (char *)skye_sm_iface->name,
                          (menuinfo_t **)&skye_submenup);
    build_secondary_submenu(skye_submenu_table,
                            SKYE_SUBMENU_TABLE_SIZE,
                            skye_tests_secondary_items);

    /*
    * pm_subtest_menu now built.  Display and interact with user until
    * <ESC><RET> back to main menu.
    *
    * To prevent freeing up allocated memory prematurely,
    * save the pre_diag_exec function and set it to NULL.
    * This will prevent menu() marking the needed memory freed.
    */
    skye_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    if (skye_sm_iface->menu_display == TRUE) {
        menu(skye_submenup, skye_tests_secondary_items, '\0' );
    } else {
        if (skye_sm_iface->test_type == IFACE_TEST) {
            ret_val = oir_ltc4215_register_test(oir);
            ret_val |= skye_ioe_reg_test();        
        } else {  /* FULL_TEST */    
            do_all_menu_items(skye_submenup);
        }
    }

    ret_val |= util_oir_ltc4215_led(oir, OIR_LED_OFF);

    skye_sm_cleanup();

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
    skye_ds_t *skye_info_p = &skye_info;
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
                                       skye_info_p->slot,
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
 * Function   : skye_ioe_reg_test
 * Description: Wrapped function to do Skye IO Expander register test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_ioe_reg_test (void)
{
    uint32_t         ctr = 0, test_ctr = 0, total_reg_num = 0;
    uchar            orig_val = 0, test_data = 0, check_data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    reg_p = &pca9555_reg_tbl[0];
    total_reg_num = (sizeof(pca9555_reg_tbl) / sizeof(reg_info_t));

    testname("Skye IO Expander Registers");

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        /* Skip Input port registers & Output port registers
         * Based on PCA9555 datasheet, Input port registers are input-only,
         * writes to these registers have no effect.
         * And skip Output port registers to avoid to change the system set-ups.
         * Like cause Skye alien sub-module be put in reset(GPIO[2] = 0).
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
 * Function   : set_skye_hipwr
 * Description: Function to Enable/Disable Skye High Power mode.
 * Inputs     : opt - Enable/Disable
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
set_skye_hipwr (boolean opt)
{
    uint8_t data = 0;

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("%s: Failed to read LTC4215 register(0x%02X).\n",
               __FUNCTION__, LTC4215_FAULT_REG);
        return (FAILED);
    }

    if (opt == DISABLE) {
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
 * Function   : skye_hipwr_ctrl_util
 * Description: Wrapped utility to control Skye High Power mode.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_hipwr_ctrl_util (int opt)
{
    uint8_t data = 0;

    printf("Starting to %s Skye High Power mode... ",
           (opt == ENABLE) ? "Enable" : "Disable");

    if (set_skye_hipwr(opt) != PASSED) {
        printf("\n%s: Failed to %s Skye High Power mode.\n",
               __FUNCTION__, (opt == ENABLE) ? "Enable" : "Disable");
        return (FAILED);
    }

    printf("Done.\n");

    if (oir_ltc4215_reg_read(oir, LTC4215_FAULT_REG, &data)) {
        printf("%s: Failed to read LTC4215 register(0x%02X).\n",
               __FUNCTION__, LTC4215_FAULT_REG);
        return (FAILED);
    }

    printf("Skye High Power mode is %s now.\n",
           (data & LTC4215_GPIO3_OUTPUT)? "Disabled" : "Enabled");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_ioe_reg_dump_util
 * Description: Wrap utility to dump all registers of Skye
 *              IO Expander(PCA9555).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_ioe_reg_dump_util (void)
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

    printf("\nSkye IO Expander(PCA9555) registers dump:\n");
    for (ctr = 0; ctr < total_reg_num; ctr++) {
        reg_p = &pca9555_reg_tbl[ctr];
        printf("%-25s Reg.(0x%01X) = 0x%02X.\n",
               reg_p->name, reg_p->offset, reg_data[ctr]);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_ioe_reg_rd_util
 * Description: Wrap utility to read specific register of Skye
 *              IO Expander(PCA9555).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_ioe_reg_rd_util (void)
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
 * Function   : skye_ioe_reg_wr_util
 * Description: Wrap utility to write specific register of Skye
 *              IO Expander(PCA9555).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
skye_ioe_reg_wr_util (void)
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


/**********************************************************************
 *
 * Function: skye_boot_image
 *
 * This function interrupts Mboot, download image through TFTP, and
 * load the image
 *
 * Input : cpu  - CPU 0 or CPU 1
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long skye_boot_image (int cpu)
{
    int ix;
    static char tty_dev[32];
    int rc, boot_timeout, ping_timeout;
    char cmd[CMD_LENGTH];
    char ipconfig[256];
    struct stat sts;
    uint retryp = 0;
    int uart_fd;

    rc = PASSED;
#ifdef ALWAYS_TFTP
    /* Download image from the network for the first time  */
    /* Always Get the latest image -- Debug purpose
     * For CPU1 no need to download the firmware again.*/
    if (cpu == CPU0) {
        sprintf(cmd, "rm -f %s", SKYE_DEST_DIAG_IMG);
        system(cmd);
        fflush(stdout);
    }
#endif
    if (stat(SKYE_DEST_DIAG_IMG, &sts) == -1) {
        if (tftp_get(0, SKYE_SRC_DIAG_IMG, 0, SKYE_DEST_DIAG_IMG, 0) < 0) {
            sprintf(cmd, "rm -f %s", SKYE_DEST_DIAG_IMG);
            system(cmd);
            fflush(stdout);
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);
        }
    } else {
        printf("\nFile image exist...ready to boot !!!\n");
    }

    if (cpu == CPU0) {
        printf("-- Power off skye module...");
        /* Power Cycle before firmware download */
        if (pwr_off_skye_sm() != PASSED) {
            cterr('f', 0, "Failed to Power OFF the Skye SM");
            return (FAILED);
        }

        /* msleep for 10 seconds ?? Check if Skye needs. */
        for (ix = 0; ix < 10; ix++) {
            printf(".");
            msleep(1000);
        }
        printf("\n");
        printf("-- Power on skye module...");
        if (pwr_on_skye_sm() != PASSED) {
            cterr('f', 0, "Failed to Power ON the Skye SM");
            return(FAILED);
        }

        msleep(SKYE_POWER_UP_DELAY);
    }

    /* CPU 0 */
    if (cpu == CPU0) {
        if (is_skye_uart(0) == FAILED) {
            printf("Failed to switch uart 0\n");
            return (FAILED);
        }
    } else { /* CPU 1 */
        if (is_skye_uart(1) == FAILED) {
            printf("Failed to switch uart 1\n");
            return (FAILED);
        }
    }

    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Polling if bootloader is up, we need to fire tftpdnld command
     * through UART interface since Skye bootloader doesn't boot
     * linux by default.
     */
    printf("CPU%d -- Looking for mboot prompt (1)...", cpu);
    fflush(stdout);

    /* wait for mboot prompt come up*/
    msleep(2000);
    /* Transmit New Line */
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);

    boot_timeout = SKYE_BL_PROMPT_TOUT * 10;
    do {
        /* Transmit New Line */
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);

        if (skye_rx_polling_uart(uart_fd, SKYE_BL_PROMPT, 100) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU%d -- Failed to get '%s' bootloader prompt", cpu, SKYE_BL_PROMPT);
        close(uart_fd);
        return (FAILED);
    }

    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);

    printf("Removing memory environment ...");
    fflush(stdout);

    /* Remove this environment, otherwise diag image won't boot up */
    skye_tx_uart(uart_fd, SKYE_REMOVE_MEM_ENV);
    skye_tx_uart(uart_fd, SKYE_YES_STRING);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    msleep(100);
    printf("OK\n");
    fflush(stdout);
    /* Polling if bootloader is up, we need to fire tftpdnld command
     * through UART interface since Woodlawn bootloader doesn't boot
     * linux by default. (It boots up SE instead)
     */
    printf("CPU%d -- Looking for mboot prompt (2)...", cpu);
    fflush(stdout);
    /* wait for mboot prompt come up*/
    msleep(2000);
    /* Transmit New Line */
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);

    boot_timeout = SKYE_BL_PROMPT_TOUT * 10;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);

        if (skye_rx_polling_uart(uart_fd, SKYE_BL_PROMPT, 100) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU%d -- Failed to get '%s' bootloader prompt", cpu, SKYE_BL_PROMPT);
        close(uart_fd);
        return (FAILED);
    }

    /* Setup IP config on Mboot */
    printf("CPU%d -- Setup IP config on Mboot...", cpu);
    fflush(stdout);
    /* wait for mboot prompt come up*/
    msleep(1000);

    if (cpu == CPU0) {
        /* Setup IP based on SM slot */
        sprintf(ipconfig, "ifconfig gbe4 %s.%d -mask 255.255.255.0 -up",
                    SKYE_CPU0_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE +  skye_test_slot);
        skye_tx_uart(uart_fd, ipconfig);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
    } else {
        /* Setup IP based on SM slot */
        sprintf(ipconfig, "ifconfig xgbe1 %s.%d -mask 255.255.255.0 -up",
                SKYE_CPU1_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE +  skye_test_slot + cpu);
        skye_tx_uart(uart_fd, ipconfig);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        msleep(500);
        /* for CPU1 to ping TFTPserver backplane */
        skye_tx_uart(uart_fd, SKYE_CPU1_SET_ROUTE_MBOOT);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
    }
    msleep(500);
    printf("OK\n");
    fflush(stdout);

    /* Now ping the server ip */
    printf("CPU%d -- Ping TFTP Server from Backplane ...", cpu);
    fflush(stdout);

    ping_timeout = SKYE_PING_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_PING_SERVERIP);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_PING_ALIVE, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU%d -- Failed to ping TFTP Server", cpu);
        close(uart_fd);
        return (FAILED);
    }

    /* Now, boot image using TFTP download */
    printf("CPU%d -- Booting image ...", cpu);
    fflush(stdout);

    skye_tx_uart(uart_fd, SKYE_BOOT_CMD);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    msleep(100);

    /* Check linux prompt "#" */
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }

        /*
         * We had tried to fix the problem of Skye sometimes can't boot up
         * it's kernel on Curie by adding delay time before sending
         * kernel boot up command and extend the polling time of checking 
         * Linux prompt but in vain.
         * 
         * The solution here is creating weak function "resend_skye_kernel_boot_cmd" 
         * for Curie to resend kernel boot command for the duration of the polling time,
         * other platforms such as Neptune will not be impacted.
         *
         * This solution had been verified by running regression.
         */
        if (resend_skye_kernel_boot_cmd()) {
            skye_tx_uart(uart_fd, SKYE_BOOT_CMD);
        } 
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU%d -- Failed to check linux prompt", cpu);
        close(uart_fd);
        return (FAILED);
    }
    /* keep initial retry value */
    retryp = 0;
retry_ping :
    /* Setup IP config after linux come up */
    printf("CPU%d -- Setup IP config after linux prompt...", cpu);
    if (cpu == CPU0) {
        /* Setup IP based on SM slot */
        sprintf(ipconfig, "ifconfig gbe4 %s.%d netmask 255.255.255.0",
            SKYE_CPU0_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE +  skye_test_slot);
        /* To avoid linux "press any key to continue" after linux boot up, just press enter twice */
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, ipconfig);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        msleep(500);
        printf("OK\n");
    } else {
        sprintf(ipconfig, "ifconfig xgbe1 %s.%d netmask 255.255.255.0",
            SKYE_CPU1_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE +  skye_test_slot + cpu);
        /* To avoid linux "press any key to continue" after linux boot up, just press enter twice */
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, ipconfig);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        msleep(500);
        /* for CPU1 to ping TFTPserver backplane */
        skye_tx_uart(uart_fd, SKYE_CPU1_SET_ROUTE);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        msleep(500);
        printf("OK\n");
    }

     /* to ping CPU1 need to add this */
    //route add default gw 192.123.123.101
    if (cpu == CPU1) {
        sprintf(cmd, "route add default gw %s.%d", SKYE_CPU0_IP_ADDR_SUBNET,
            SKYE_DIAG_IP_ADDR_BASE + skye_test_slot);
        system(cmd);
        cpu_id = CPU1;
    }

    /* Now, we wait until we can reach SM card through ping */
    printf("CPU%d -- Check module card can be reach through ping...", cpu);
    rc = FAILED;
    ping_timeout = SKYE_DIAG_PROMPT_TOUT;
    for (ix = 0; ix < ping_timeout; ix++) {
        printf(".");
        fflush(stdout);
        /* Start pinging SM card */
        if (is_skye_up(FALSE, cpu, FALSE) == TRUE) {
            printf("\nSkye CPU%d is up!\n", cpu);
            fflush(stdout);
            rc = PASSED;
            break;
        }
        sleep(1);
        if (ping_timeout > 11) {
            retryp++;
            if (retryp > 11) {
                /* retry to 10 times */
                break;
            }
            goto retry_ping;
        }
    }

    /* Debug */
    printf("retry to ping = %d\n", retryp);

    if (rc != PASSED) {
        cterr('f', 0, "\nUnable to reach Skye firmware!");
        fflush(stdout);
    }

    close(uart_fd);
    return (rc);
}


/*****************************************************************
 *
 * Function: skye_tx_uart
 *
 * Description: This function transmits strings into tty
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         out_str: compared string
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */

int skye_tx_uart (int uart_fd, char *out_str)
{
    int cnt;
    int rc = PASSED;

    /* Sanity check */
    if (out_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    cnt = write(uart_fd, out_str, strlen(out_str));

    if (cnt < 0) {
        perror("tx_uart: write failed\n");
        rc = FAILED;
    }

    return (rc);
}


/*****************************************************************
 *
 * Function: skye_rx_polling_uart
 *
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         comp_str: compared string
 *         timeout: timeout value (ms)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int skye_rx_polling_uart (int uart_fd, char *comp_str, int timeout)
{
    int cnt;
    struct timeval read_timeout;
    fd_set set;
    char buf[1024];
    char *search_str;
    int rc;
    struct timeval start_time, curr_time;
    int elapsed_time_in_ms;

    memset(buf, 0 , 1024);

    /* Sanity check */
    if (comp_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    gettimeofday(&start_time, NULL);

    do {
        /* Set timeout on file descriptor */
        FD_ZERO(&set);
        FD_SET(uart_fd, &set);

        read_timeout.tv_sec  = SKYE_UART_READ_TIMEOUT;
        read_timeout.tv_usec = 0;

        rc = select(uart_fd + 1, &set, NULL, NULL, &read_timeout);

        if (rc == -1) { /* Error occured */
            perror("select");
            fflush(stdout);
            goto exit_poll_uart;
        }

        if (FD_ISSET(uart_fd, &set)) {
            /* Now, we read the buffer */
            cnt = read(uart_fd, buf, 255);
            if (cnt < 0) {
                perror("Read error");
                fflush(stdout);
                goto exit_poll_uart;
            }

            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                //close(uart_fd);
                return (PASSED);
            }
        }

        /* Now check if elapsed time exceeds timeout value */
        gettimeofday(&curr_time, NULL);

        elapsed_time_in_ms = (curr_time.tv_sec - start_time.tv_sec) * 1000;
        elapsed_time_in_ms += (curr_time.tv_usec - start_time.tv_usec) / 1000;

        if (elapsed_time_in_ms > timeout) {
            goto exit_poll_uart;
        }

        msleep(1);
    } while (1);

exit_poll_uart:

    return (FAILED);
}


/**************************************************************************
 *
 * Function: is_skye_up
 *
 * Check if Skye SM card is up by sending ping packet via the GESW
 *
 * Input: verbose - flag to control message printing
 *        cpu     - cpu 0 or cpu 1
 *        xg      - flag to control 10G or 1G
 *
 * Return: TRUE/FALSE
 *
 * *************************************************************************
 */
static int is_skye_up (boolean verbose, int cpu, boolean xg)
{
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline;
    char *result_file = SKYE_PING_TEST;
    FILE *fp;
    int tx_cnt, rx_cnt;
    int rv = FAILED;
    char sm_ip[16];
    static boolean result = FALSE;

    pktcnt = 2;
    deadline = 5;

    sprintf(cmdbuf, "rm %s", SKYE_PING_TEST);
    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp != NULL) {
        fclose(fp);
        sprintf(cmdbuf, "rm %s", result_file);
        system(cmdbuf);
    }

    if (cpu == CPU0) {
        /* Not 10GKR */
        if (xg == FALSE) {
            /* 192.123.123.101 for slot 1, 192.123.123.102 for slot 2 */
            sprintf(sm_ip, "%s.%d", SKYE_CPU0_IP_ADDR_SUBNET,
            SKYE_DIAG_IP_ADDR_BASE + skye_test_slot);
        } else {
            /* 10G xgbe2 */
            /* 192.123.123.201 for slot 1, 192.123.123.202 for slot 2 */
            sprintf(sm_ip, "%s.%d", SKYE_CPU0_IP_ADDR_SUBNET,
                    SKYE_DIAG_IP_ADDR_BASE_XG + skye_test_slot);
        }
    } else {
        /* 192.168.1.102 for slot 1, 192.168.1.103 for slot 2 */
        sprintf(sm_ip, "%s.%d", SKYE_CPU1_IP_ADDR_SUBNET,
        SKYE_DIAG_IP_ADDR_BASE + skye_test_slot + cpu);
    }

    sprintf(cmdbuf, "ping -c %d -w %d %s > %s",
                pktcnt, deadline, sm_ip, result_file);
    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        if (verbose) {
            printf("HOST: Ping DP %s was not created\n", result_file);
        }
        goto is_skye_up_exit;
    }

    /* Check the result
     */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);

        if (strstr(buf, "received") != NULL) {
#if DEBUG
            printf("HOST: Ping DP result: %s", buf);
#endif
            result = TRUE;
            break;
        }
    }
    fclose(fp);
    if (result == FALSE) {
        goto  is_skye_up_exit;
    }
    /* Read the string
     */
    sscanf(buf, "%d %s %s %d", &tx_cnt, dum_char, dum_char, &rx_cnt);

    if (rx_cnt < pktcnt) {
        if (verbose) {
            printf("HOST: Ping DP packet count mismatch. "
                   "Expected= %d, Actual: tx= %d rx= %d\n",
                   pktcnt, tx_cnt, rx_cnt);
        }
        goto  is_skye_up_exit;
    } else {
        rv = PASSED;
    }

is_skye_up_exit:

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


/**************************************************************************
 *
 * Function: is_skye_uart
 *
 * Config skye UART Mux
 *
 * Input: uart - cpu uart
 *
 * Return: TRUE/FALSE
 *
 * *************************************************************************
 */
int is_skye_uart (int uart)
{
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    uint8_t data = 0;
    int cpu1 = 1;
    /* GPIO Bit4 UART Mux Select:
     * 0 = Connect the primary UART(CPU #0) to the host;
     * 1 = Connect the secondary UART(CPU #1) to the host.
     */
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
        cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    if (uart == cpu1)
        data |= SKYE_UART_MUX_SEL;
    else
        data &= ~(SKYE_UART_MUX_SEL);

    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        cterr('f', 0, "GPIO write failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_get_ip_addr
 * Description: This function returns IP Address
 * Inputs     : ip_addr - Buffer to put IP address
 *              cpu     - cpu id
 * Outputs    : void
 *
 *******************************************************************************
 */
void skye_get_ip_addr (char *ip_addr, int cpu)
{
    /* Sanity check */
    if (ip_addr == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    if (cpu == CPU0)
        sprintf(ip_addr, "%s.%d", SKYE_CPU0_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE + skye_test_slot);
    else {
        sprintf(ip_addr, "%s.%d", SKYE_CPU1_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE + skye_test_slot + cpu);
    }
    return;
}


/**********************************************************************
 *
 * Function: skye_run_sm_test
 *
 * This function runs all SM tests
 *
 * Input : cpu - cpu 0 or 1
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long skye_run_sm_test (int cpu)
{
    testname("Skye CPU%d Do All Test", cpu);
    prpass(testpass, "Firmware boot up");
    if (skye_boot_image(cpu) == FAILED) {
        cterr('f', 0, "Skye CPU%d firmware download failed.", cpu);
        return (FAILED);
    }

    if (pre_skye_test(cpu, DIAG_DO_MEM_TEST) == FAILED) {
        cterr('f', 0, "Skye CPU%d Skye Memory test failed .", cpu);
        return (FAILED);
    }

    if (pre_skye_test(cpu, DIAG_DO_I2CDEV_TEST) == FAILED) {
        cterr('f', 0, "Skye CPU%d Skye I2C Device test failed .", cpu);
        return (FAILED);
    }

    if (pre_skye_test(cpu, DIAG_DO_FPGA_TEST) == FAILED) {
        cterr('f', 0, "Skye CPU%d Skye FPGA test failed .", cpu);
        return (FAILED);
    }

    if (pre_skye_test(cpu, DIAG_DO_SPIROM_TEST) == FAILED) {
        cterr('f', 0, "Skye CPU%d Skye SPIROM test failed .", cpu);
        return (FAILED);
    }
    
    if (cpu == CPU0) {
        if (pre_skye_test(cpu, DIAG_DO_TLK_TEST) == FAILED) {
            cterr('f', 0, "Skye CPU%d Skye TLK test failed .", cpu);
            return (FAILED);
        }   
    }

    /* Skye PCIe lanes Scan Test is needed only in CPU1 of 2-CPUs Skye */
    if (cpu == CPU1) {
        if (pre_skye_test(cpu, DIAG_DO_PCIE_TEST) == FAILED) {
            cterr('f', 0, "Skye CPU%d Skye PCIe test failed .", cpu);
            return (FAILED);
        }
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: skye_rx_uart
 *
 * This function receive uart and print out.
 *
 * Input : fd - file descriptor for UART
 *
 * Output: none
 *
 **********************************************************************
 */
void *skye_rx_uart (void *fd)
{
    int cnt;
    char buf[1024];

    /* Sanity check */
    if (fd == NULL) {
        perror("Null pointer\n");
        fflush(stdout);
        goto exit_poll_uart;
    }

    /*
     * we are probing status every 2 sec (min) and the timeout
     * is in second.
     */
    while(1) {
        cnt = read(*(int *)fd, buf, 255);
        if (cnt < 0) {
            perror("Read error");
            fflush(stdout);
            goto exit_poll_uart;
        }

            if (cnt > 1) {
                skye_clrline(buf);
                if (diagflag_xram & D_TRACE) {
                    printf("(%d)%s\n", cnt, buf);
                }
                /* update the main thread wait time */
                main_thread_wait_time = 5000;
            }

    }
exit_poll_uart:
    pthread_exit(NULL);
}


/**********************************************************************
 *
 * Function: skye_clrline()
 *
 * Description: This function checks input string for newline, clears it
 *
 * Input:  string to check for newline
 *
 * Output: void
 *
 **********************************************************************
 */
void skye_clrline (char *string)
{
    int     ix;  /* get line position */
    char    *cptr;
    int     length;

    length = strlen(string);
    ix = linepos;
    cptr = strchr(string, '\n');  /* does the string have a newline? */
    if(cptr) {
        *cptr = '\0';  /* replace the first one with NULL */
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nlength(%d)", length);
            dismem((unsigned char *)(string), length + 4, (unsigned long)(string), 1);
        }
    }
    putchar('\r');
    ix -= printf(string); /* ix = strlen(previous) - strlen(current) */
    fflush(0);
    linepos = length;
    if(ix > 0) {  /* last line was longer - wipe the rest of it */
        while(ix --) {
            putchar(' '); /* clear rest of line */
        }
    }
    printf("\r");
}


/**********************************************************************
 *
 * Function: skye_run_sm_dual_cpu_xaui_test
 *
 * This function runs dual cpu xaui test
 *
 * Input : show_menu - Not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int skye_dual_cpu_xaui_test (int show_menu)
{
    char tty_dev[32];
    int ping_timeout;
    int timedelay = 500; /* 0.5 seconds */
    int uart_fd;

    if (diagflag_xram & D_SET_OPTIONS) {
        timedelay = getdec_answer("delay for how long?(500milliseconds Default)", 500, 0, 1000);
        printf("timedelay %d\n", timedelay);
    }

    /* setup skye uart for CPU 1 first */
    if (is_skye_uart(1) == FAILED) {
        printf("Failed to switch uart 1\n");
        return (FAILED);
    }
    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Check linux prompt "#" */
    printf("CPU1 -- Check linux prompt # ...");
    fflush(stdout);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU1 -- Failed to check linux prompt");
        close(uart_fd);
        return (FAILED);
    }

    /* execute skye_lnx -k "for cpu1 xaui test waiting packet from cpu0" */
    skye_tx_uart(uart_fd, SKYE_LINUX_TEST_CPU1_XAUI);
    /* Delay 0.5 seconds for waiting CPU 1 ready to Receive */
    msleep(timedelay);
    /* setup skye uart for CPU 0 secondly */
    if (is_skye_uart(0) == FAILED) {
        printf("Failed to switch uart 0\n");
        close(uart_fd);
        return (FAILED);
    }

    close(uart_fd);

    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Check linux prompt "#" */
    printf("CPU0 -- Check linux prompt # ...");
    fflush(stdout);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do {
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU0 -- Failed to check linux prompt");
        close(uart_fd);
        return (FAILED);
    }

    /* execute skye_lnx -j "for cpu0 xaui test waiting packet from cpu1" */
    skye_tx_uart(uart_fd, SKYE_LINUX_TEST_CPU0_XAUI);
    /* Delay 0.5 seconds for waiting CPU 0 ready to check results */
    msleep(timedelay);
    /* Check linux prompt "#" */
    printf("Dual CPU XAUI Test Results: ...");
    /* Check results */
    ping_timeout = SKYE_DUAL_CPU_XAUI_TOUT;
    do{
        if (skye_rx_polling_uart(uart_fd, PASS_RESULTS, 1000) == PASSED) {
            printf("PASS\n");
            fflush(stdout);
            break;
        }
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "Dual CPU XAUI test failed");
        close(uart_fd);
        return (FAILED);
    }

    close(uart_fd);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: no_10GKR_cap
 *
 * This function for checking 10GKR capability
 *
 * Input : None
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean no_10GKR_cap (void)
{
    if (skye_e0_10g_cap == TRUE)
        return (FALSE);  /* Hide the menu */
    else
        return (TRUE);
}


/**********************************************************************
 *
 * Function: skye_exec_apps
 *
 * This function for executing the linux apps by uart
 *
 * Input : cpu - cpu0 or cpu1
 *         comp_str - linux apps
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
static int skye_exec_apps (int cpu, char *comp_str)
{
    int ping_timeout;
    char tty_dev[32];
    int uart_fd;

    /* CPU 0 */
    if (cpu == CPU0) {
        if (is_skye_uart(0) == FAILED) {
            printf("Failed to switch uart 0\n");
            return (FAILED);
        }
    } else { /* CPU 1 */
        if (is_skye_uart(1) == FAILED) {
            printf("Failed to switch uart 1\n");
            return (FAILED);
        }
    }

    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    printf("Linux Prompt ...");
    /* Check linux prompt "#" */
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "Failed to check linux prompt");
        close(uart_fd);
        return (FAILED);
    }

    /* Execute Skye Diag Linux */
    printf("Executing diag linux ...");
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    msleep(100);
    skye_tx_uart(uart_fd, comp_str);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    printf("OK\n");
    fflush(stdout);

    close(uart_fd);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: skye_one_cpu
 *
 * This function for checking one cpu
 *
 * Input : None
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean skye_one_cpu (void)
{
    if (cpu_cookie == SKYE_1CPU_ID)
        return (TRUE);
    else
        return (FALSE); /* Hide the menu */
}


/**********************************************************************
 *
 * Function: skye_two_cpu
 *
 * This function for checking two cpus
 *
 * Input : None
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean skye_two_cpu (void)
{
    if (cpu_cookie == SKYE_2CPU_ID)
        return (TRUE);
    else
        return (FALSE); /* Hide the menu */
}


/**********************************************************************
 *
 * Function: check_skye_ready
 *
 * This function for checking sm interface ready by reading bit 3
 *
 * Input : None
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
int check_skye_ready (void)
{
    uchar io_port_conf0 = 0;
    n2g_i2c_if_t *pca;
    int          ctr = 0, max_retry = 2000;

    pca = &pca_i2c[0];

    for (ctr = 1; ctr <= max_retry; ctr++) {
        if (io_port_8bit_i2c_read(pca, PCA9555_IN_PORT0_REG,
                                  &io_port_conf0, TRUE)) {
            cterr('f', 0, "%s: Can't read IO Expander Reg.(0x%02X)",
                          __FUNCTION__, PCA9555_IN_PORT0_REG);
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:[%d]IO Expander Reg.(0x%02X) = 0x%02X\n",
                   __FUNCTION__, ctr, PCA9555_IN_PORT0_REG, io_port_conf0);
            fflush(stdout);
        }

        /* Based on Skye HFS, we need to configure -
         * IO port 0.3 as input(bit set to 1),
         */
        if ((io_port_conf0 & SKYE_PRI_INF_RDY) == SKYE_PRI_INF_RDY) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\n%s: Retry counter = %d.\n", __FUNCTION__, ctr);
                fflush(stdout);
            }
            break;
        }
        msleep(200);
    }

    if (ctr > max_retry) {
        cterr('f', 0, "Time out & Primary Interface is NOT Ready");
        return (FAILED);
    }

    printf("Primary Interface is Ready");
    fflush(stdout);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: skye_set_tlk_1gkx_lpbk_bit
 *
 * This function set tlk module 1gkx and 10gkr bit for host loopback
 *
 * Input : enable / disable loopback bit
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int skye_set_tlk_1gkx_lpbk_bit (boolean enable)
{
    char tty_dev[32];
    int ping_timeout;
    int check_count = 3; /* check results max 3 times */
    int ix;
    int uart_fd;

    /* setup skye uart for CPU 0 */
    if (is_skye_uart(0) == FAILED) {
        printf("Failed to switch uart 0\n");
        return (FAILED);
    }
    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Escape to default shell by typing 'ESC' and 'Enter' */
    for (ix = 0; ix < 5; ix++) {
        skye_tx_uart(uart_fd, SKYE_ESC_CR_STRING);
    }

    msleep(500);

    /* Check linux prompt "#" */
    printf("CPU0 -- Check linux prompt # ...");
    fflush(stdout);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do {
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU0 -- Failed to check linux prompt");
        close(uart_fd);
        return (FAILED);
    }

    if (enable == TRUE) {
        check_count = 10;
        /* execute skye_lnx -l "for cpu0 setup tlk loopback bit " */
        skye_tx_uart(uart_fd, SKYE_SET_TLK_1GKX_LBPK);
        printf("Enable TLK loopback bit ...");
retry_chck_set:
        skye_tx_uart(uart_fd, "cat /diag/skye_check_set_tlk_bit.txt\012");
        /* Check results */
        ping_timeout = SKYE_TLK_LPBKBIT_TOUT;
        do{
            if (skye_rx_polling_uart(uart_fd, PASS_RESULTS, 1000) == PASSED) {
                printf("PASS\n");
                fflush(stdout);
                break;
            }
            skye_tx_uart(uart_fd, SKYE_CR_STRING);
            skye_tx_uart(uart_fd, SKYE_CR_STRING);
        } while (ping_timeout--);

        if (ping_timeout <= 0) {
            if (check_count == 0) {
                printf("FAIL\n");
                fflush(stdout);
                cterr('f', 0, "Set TLK loopback bit failed");
                close(uart_fd);
                return (FAILED);
            } else {
                check_count--;
                printf("retry = %d\n", check_count);
                fflush(stdout);
                goto retry_chck_set;
            }
        }
    } else {
        check_count = 10;
        /* execute skye_lnx -m "for cpu0 disable tlk loopback bit " */
        skye_tx_uart(uart_fd, SKYE_DISABLE_TLK_1GKX_LBPK);
        printf("Disable TLK loopback bit ...");
retry_chck_clr:
        skye_tx_uart(uart_fd, "cat /diag/skye_check_clr_tlk_bit.txt\012");
        /* Check results */
        ping_timeout = SKYE_TLK_LPBKBIT_TOUT;
        do{
            if (skye_rx_polling_uart(uart_fd, PASS_RESULTS, 1000) == PASSED) {
                printf("PASS\n");
                fflush(stdout);
                break;
            }
            skye_tx_uart(uart_fd, SKYE_CR_STRING);
            skye_tx_uart(uart_fd, SKYE_CR_STRING);
        } while (ping_timeout--);

        if (ping_timeout <= 0) {
            if (check_count == 0) {
                printf("FAIL\n");
                fflush(stdout);
                cterr('f', 0, "Set TLK loopback bit failed");
                close(uart_fd);
                return (FAILED);
            } else {
                check_count--;
                printf("retry = %d\n", check_count);
                fflush(stdout);
                goto retry_chck_clr;
            }
        }
    }

    close(uart_fd);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: skye_o2_shell
 *
 * This function to escaping to shell bash.
 *
 * Input : None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int skye_o2_shell (void)
{
    int slot;

    assert(skye_test_slot);
    slot = skye_test_slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\nEscaping to Shell from SM Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);
}


/**********************************************************************
 *
 * Function: skye_o2_command
 *
 * This function enter shell command
 *
 * Input : void
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int skye_o2_command (void)
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
* Function: ge_bp_tlk_lpbk_test
*
* Perform GE loopback test to verify GE0 backplane connectivity between
* host and SM card. Need to setup the skye module config tlk to setup
* loopback bit.
*
* Input : none
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
static long ge_bp_tlk_lpbk_test (void)
{
    int rc = PASSED;
    int packet_no = SKYE_GE_BP_PACKET_NO;
    int ctrl_plane_sgmii_port;

    testname(" 10G-KR Backplane Loopback");
    prpass(testpass, "Setting TLK10232 loopback bit");

    prpass(testpass, "Running loopback test now");

    ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port();
    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, packet_no) != PASSED) {
        cterr('f', 0, "GE loopback from Host side %d fails.",
                ctrl_plane_sgmii_port);
        rc = (FAILED);
    } else {
        printf("Loopback test pass.\n");
    }
    return (rc);
}


/**********************************************************************
*
* Function: pre_skye_test
*
* Perform Execute Linux App before send NC command
*
* Input : cpu - cpu number
*         tstname - testname to send to skye module
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
static int pre_skye_test (int cpu, char *tstname)
{
    testname("%s ", __FUNCTION__);
    prpass(testpass, "Execute Linux");
    if (skye_exec_apps(cpu, SKYE_LINUX_NC) == FAILED) {
        cterr('f', 0, "Skye CPU%d executing linux app failed.", cpu);
        return (FAILED);
    }
    /* check Skye SM side was ready */
    if (cpu == CPU0) {
        if (check_skye_ready() == FAILED) {
            cterr('f', 0, "Skye CPU%d is not ready", cpu);
            return (FAILED);
        } else {
            printf("\nSkye CPU%d ready...", cpu);fflush(stdout);
        }
    }
    /* delay 2 second to let the module is ready to run the test */
    msleep(2000);

    prpass(testpass, "Run %s ", tstname);
    if (skye_exec_test(cpu, tstname) == FAILED) {
        cterr('f', 0, "Skye CPU%d do %s test failed.", cpu, tstname);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: ping_cpu1_xaui_test
 *
 * This function runs ping from CPU 1 to Host BP through CPU 0
 *
 * Input : show_menu - Not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ping_cpu1_xaui_test (int show_menu)
{
    char tty_dev[32];
    int ping_timeout;
    int uart_fd;
    uint check_count = 0;

    /* setup skye uart for CPU1 */
    if (is_skye_uart(1) == FAILED) {
        printf("Failed to switch uart 0\n");
        return (FAILED);
    }
    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Check linux prompt "#" */
    printf("CPU1 -- Check linux prompt # ...");
    fflush(stdout);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        cterr('f', 0, "CPU1 -- Failed to check linux prompt");
        close(uart_fd);
        return (FAILED);
    }

    /* execute skye_lnx -n "for cpu1 ping xaui test through cpu0" */
    skye_tx_uart(uart_fd, SKYE_CPU1_XAUI_PING_TEST);
    /* Delay 0.5 seconds for waiting CPU 1 ready to check results */
    msleep(500);
    printf("Dual CPU XAUI ping test results: ...");
    check_count = 5; /* check results max 5 times */
retry_chck:
    skye_tx_uart(uart_fd, CHECK_PING_OUTPUT);
    /* Check results */
    ping_timeout = SKYE_DUAL_CPU_XAUI_TOUT;
    do{
        if (skye_rx_polling_uart(uart_fd, PASS_RESULTS, 1000) == PASSED) {
            printf("PASS\n");
            fflush(stdout);
            break;
        }
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        if (check_count == 0) {
            printf("FAIL\n");
            fflush(stdout);
            cterr('f', 0, "Dual CPU XAUI ping test failed");
            close(uart_fd);
            return (FAILED);
        } else {
            check_count--;
            printf("retry = %d\n", check_count);
            fflush(stdout);
            goto retry_chck;
        }
    }

    close(uart_fd);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: ping_cpu0_10g_test
 *
 * This function runs ping from Host BP 10G-KR through CPU 0
 *
 * Input : show_menu - Not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ping_cpu0_10g_test (int show_menu)
{
    int rc, ping_timeout;
    char tty_dev[32];
    uint retryp = 0;
    int cpu = 0;
    int ix;
    char ipconfig[256];
    char config_gbe4[256];
    char config_xgbe2[256];
    skye_ds_t     *skye_info_p;
    skye_info_p = &skye_info;
    int ge_port0;
    int real_slot = skye_info_p->slot;
    uint check_count = 0;
    uint check_prompt = 0;
    int uart_fd;

    testname(" 10G-KR Ping test");

    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0);
    /* Disable the loopback */
    set_gesw_line_loopback(ge_port0, 0);
    /* keep initial retry value */
    retryp = 0;
    check_count = 5; /* check results max 5 times */
    check_prompt = 3; /* check results max 3 times */
retry_ping :
    /* setup skye uart for CPU0 */
    if (is_skye_uart(cpu) == FAILED) {
        printf("Failed to switch uart 0\n");
        return (FAILED);
    }
    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_iface_p->uart);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Escape to default shell by typing 'ESC' and 'Enter' */
    for (ix = 0; ix < 5; ix++) {
        skye_tx_uart(uart_fd, SKYE_ESC_CR_STRING);
    }

    msleep(500);

    /* Check linux prompt "#" */
    printf("CPU%d -- Check linux prompt # ...", cpu);
    fflush(stdout);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, SKYE_LINUX_PROMPT, 1000) == PASSED) {
            printf("OK\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        if (check_prompt == 0) {
            printf("Fail\n");
            fflush(stdout);
            cterr('f', 0, "CPU%d -- Failed to check linux prompt", cpu);
            close(uart_fd);
            return (FAILED);
        } else {
            check_prompt--;
            printf("retry check prompt = %d\n", check_prompt);
            fflush(stdout);
            goto retry_ping;
        }
    }

    /* execute skye_lnx -o "for setup 10G-KR config script" */
    skye_tx_uart(uart_fd, SKYE_CPU0_XG_PING_TEST);
    /* Delay 0.5 seconds for waiting CPU 0 ready to check results */
    msleep(500);
    /* Check 10G-KR script deploy successfully */
    prpass(testpass, "Setting 10G-KR script");
    printf("CPU%d -- 10G-KR Script configure ...", cpu);
    fflush(stdout);

    skye_tx_uart(uart_fd, CHECK_XG_OUTPUT);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    ping_timeout = SKYE_LINUX_PROMPT_TOUT;
    do{
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        if (skye_rx_polling_uart(uart_fd, PASS_RESULTS, 1000) == PASSED) {
            printf("Pass\n");
            fflush(stdout);
            break;
        }
    } while (ping_timeout--);

    if (ping_timeout <= 0) {
        if (check_count == 0) {
            printf("Fail\n");
            fflush(stdout);
            cterr('f', 0, "CPU%d -- 10G-KR Script failed configured", cpu);
            close(uart_fd);
            return (FAILED);
        } else {
            check_count--;
            printf("retry 10G config = %d\n", check_count);
            fflush(stdout);
            goto retry_ping;
        }
    }

    /* Config gbe4 down */
    sprintf(config_gbe4, CONFIG_GBE4_DOWN);
    skye_tx_uart(uart_fd, config_gbe4);

    /* Setup IP config after linux come up */
    printf("CPU%d -- Setup IP config after linux prompt...", cpu);
    /* Setup IP based on SM slot */
    sprintf(ipconfig, "ifconfig xgbe2 %s.%d netmask 255.255.255.0 up",
        SKYE_CPU0_IP_ADDR_SUBNET, SKYE_DIAG_IP_ADDR_BASE_XG +  skye_test_slot);
    /* To avoid linux "press any key to continue" after linux boot up, just press enter twice */
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    skye_tx_uart(uart_fd, ipconfig);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    msleep(500);
    printf("OK\n");

    /* Now, we wait until we can reach SM card through ping */
    prpass(testpass, "Running Ping test ...");
    rc = FAILED;
    ping_timeout = SKYE_DIAG_PROMPT_TOUT;
    for (ix = 0; ix < ping_timeout; ix++) {
        printf(".");
        fflush(stdout);
        /* Start pinging SM card */
        if (is_skye_up(FALSE, cpu, PING10GKR) == TRUE) {
            printf("Pass\n");
            fflush(stdout);
            rc = PASSED;
            break;
        }
        sleep(1);
        if (ping_timeout > 11) {
            retryp++;
            if (retryp > 11) {
                /* retry to 10 times */
                break;
            }
            goto retry_ping;
        }
    }

    if (rc != PASSED) {
        /* Config xgbe2 down first, then gbe4 up*/
        sprintf(config_xgbe2, CONFIG_XGBE2_DOWN);
        skye_tx_uart(uart_fd, config_xgbe2);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        msleep(1000);
        sprintf(config_gbe4, CONFIG_GBE4_UP);
        skye_tx_uart(uart_fd, config_gbe4);
        skye_tx_uart(uart_fd, SKYE_CR_STRING);
        msleep(1000);
        /* Enable the loopback */
        set_gesw_line_loopback(ge_port0, 1);
        cterr('f', 0, "Fail, retry = %d\n", retryp);
        fflush(stdout);
        close(uart_fd);
        return (FAILED);
    }
    /* Config xgbe2 down first, then gbe4 up*/
    sprintf(config_xgbe2, CONFIG_XGBE2_DOWN);
    skye_tx_uart(uart_fd, config_xgbe2);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    msleep(1000);
    sprintf(config_gbe4, CONFIG_GBE4_UP);
    skye_tx_uart(uart_fd, config_gbe4);
    skye_tx_uart(uart_fd, SKYE_CR_STRING);
    msleep(1000);

    /* Enable the loopback */
    set_gesw_line_loopback(ge_port0, 1);

    prcomplete(testpass, errcount, (char *)0);

    close(uart_fd);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: has_10GKR_cap
 *
 * This function for checking 10GKR capability
 *
 * Input : None
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean has_10GKR_cap (void)
{
    if (skye_e0_10g_cap == TRUE)
        return (TRUE);
    else
        return (FALSE);/* Hide the menu */
}


/**********************************************************************
 *
 * Function: mask_out
 *
 * This function for masking out the un-use test
 *
 * Input : None
 *
 * Output: FALSE
 *
 **********************************************************************
 */
boolean mask_out (void)
{
    return (FALSE);
}

/*------------------------------------------------------------------
 * $Log: skye_host.c,v $
 * Revision 1.6  2019/08/06 06:56:06  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.5.2.3  2018/11/22 06:07:10  leschen
 * Fix Skye intermittent boot kernel failure on Curie platform.
 *
 * Revision 1.5.2.2  2018/11/09 06:02:10  leschen
 * Add 100ms delay before and after sending boot command to make sure module is ready to receive it.
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
 * Revision 1.2.18.3  2018/05/17 10:50:20  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.2.18.2  2017/06/29 01:53:21  leschen
 * Open UART device node codes are moved out of the UART tx and rx functions.
 *
 * Revision 1.2.18.1  2016/12/05 06:36:59  alpeng
 * fixed the uart ctrl num for ngio; change is approved on prrq
 *
 * Revision 1.3  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2  2015/05/25 03:56:16  steja
 * Add support Skye SM
 *
 * Revision 1.1.4.8  2015/05/20 09:43:07  steja
 * Fix TLK missing code after code review <CDETS CSCuu01237>
 *
 * Revision 1.1.4.7  2015/05/11 14:09:16  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.6  2015/05/06 09:08:14  steja
 * Minor fix
 *
 * Revision 1.1.4.5  2015/05/05 11:53:01  steja
 * CDETS[CSCuu01237] Solving TLK intermittent loopback issue on GH platform.
 *
 * Revision 1.1.4.4  2015/04/30 08:35:23  steja
 * Clean up code
 *
 * Revision 1.1.4.3  2015/04/29 13:30:15  steja
 * 1.Based Current FPGA Design, we need to unreset skye CPU within 500 ms after
 *   confirm with Skye power good signal
 * 2.Update TLK 10G-KR test path
 *
 * Revision 1.1.4.2  2015/04/29 11:48:02  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.50  2015/03/26 08:33:33  steja
 * Debug edvt found issue on 2CPU skye Dual CPU Xaui Test
 *
 * Revision 1.1.2.49  2015/03/24 02:40:34  palin2
 * Enhanced Reset Skye SM utility:
 * To support "Reset Skye SM", "Unreset Skye SM",
 * and "Perform completed Reset Skye SM process" individually.
 *
 * Revision 1.1.2.48  2015/03/09 09:02:34  steja
 * Fix boot up process failed on skye 2 CPU, 2nd CPU ping failed.(CSCut26710)
 *
 * Revision 1.1.2.47  2015/02/13 05:29:25  palin2
 * 1. Added "SKYE_FPGA_READY_TIME" definition for Skye common usage.
 * 2. Updated UART funtion error print out message to make it more clear.
 *
 * Revision 1.1.2.46  2015/02/12 12:46:40  steja
 * add ngio GE config for unreset skye sm
 *
 * Revision 1.1.2.45  2015/02/04 11:28:26  steja
 * Mask TLK test on CPU1 testing
 *
 * Revision 1.1.2.44  2015/01/30 03:00:04  steja
 * Fix code init when detect 10G mode
 *
 * Revision 1.1.2.43  2015/01/26 01:13:48  steja
 * 1. Add function for frequency margin to host side menu utilities through NC
 * 2. Remove "ifconfig gbe4" after retry ping to prevent mischeck through UART
 *
 * Revision 1.1.2.42  2015/01/22 09:07:43  palin2
 * Moved up I2C scan test ordering in Skye Diag main tests.
 *
 * Revision 1.1.2.41  2015/01/22 08:59:50  palin2
 * Updated code to fix CDETs CSCus59933:
 * 1. Removed unnecessary Diag app. ready check.
 * 2. Changed primary interface ready bit check function to polling mode.
 *
 * Revision 1.1.2.40  2015/01/13 08:30:54  steja
 * Mask GESW message when is Greyhound platform
 *
 * Revision 1.1.2.39  2014/11/27 15:04:18  palin2
 * 1. For bring up purpose, temporarily masked out console utilities
 *    auto boot to Diag functionality.
 * 2. Double the waiting time for mboot prompt to make sure it will come up,
 *    will tune this time later.
 *
 * Revision 1.1.2.38  2014/11/27 13:38:48  palin2
 * Fixed Skye UART function, skye_exec_apps, lost read back data issue.
 *
 * Revision 1.1.2.37  2014/11/27 09:29:48  palin2
 * Added statement to let PCIe lanes Scan Test only run in CPU1 of 2-CPUs Skye.
 *
 * Revision 1.1.2.36  2014/11/27 07:24:37  palin2
 * 1. Added PCIe lanes Scan test to 2-CPUs Skye default tests.
 * 2. Fixed CPU1 console utility of 2-CPUs Skye.
 *
 * Revision 1.1.2.35  2014/11/27 02:31:05  steja
 * Fix the intermittent failure to run do all test(CSCur27613)
 *
 * Revision 1.1.2.34  2014/11/21 09:37:27  steja
 * Support Full data path loopback for 10G-KR by ping test
 *
 * Revision 1.1.2.33  2014/11/12 09:30:39  steja
 * Update setup TLK loopback bit
 *
 * Revision 1.1.2.32  2014/11/11 08:00:06  steja
 * Debug the host loopback tlk test
 *
 * Revision 1.1.2.31  2014/11/10 09:43:43  steja
 * Update code for 10G-KR loopback test
 *
 * Revision 1.1.2.30  2014/10/29 08:10:03  steja
 * Fix skye boot ifconfig for different slot number
 *
 * Revision 1.1.2.29  2014/10/27 13:43:21  steja
 * minor fix
 *
 * Revision 1.1.2.27  2014/10/24 06:49:08  steja
 * 1. Add utility for switch host console and run system cmd
 * 2. Add Retry Ping
 *
 * Revision 1.1.2.26  2014/10/07 13:52:57  steja
 * Modify the skye is up function
 *
 * Revision 1.1.2.25  2014/10/07 06:04:21  palin2
 * Added to set Skye voltages margin through NC command.
 *
 * Revision 1.1.2.24  2014/10/01 08:13:19  palin2
 * Merged NC command related code(skye_comm_lib.c) to skye_utils.c
 *
 * Revision 1.1.2.23  2014/09/30 08:26:52  steja
 * Add Flag compiler for Always TFTP and fix the ping function.
 *
 * Revision 1.1.2.22  2014/09/29 18:33:01  palin2
 * Added controller type check to enable high power mode only in 2CPUs Skye.
 *
 * Revision 1.1.2.21  2014/09/26 09:05:48  steja
 * (CSCuq98591)Fix GBE4 link issue
 *
 * Revision 1.1.2.20  2014/09/24 08:04:14  steja
 * Minor fix remove "==="
 *
 * Revision 1.1.2.19  2014/09/23 07:03:09  steja
 * Update code for checking Primary Interface Ready (GPIO3)
 *
 * Revision 1.1.2.18  2014/09/22 14:00:32  steja
 * Update testname for do all test
 *
 * Revision 1.1.2.17  2014/09/22 13:13:56  steja
 * Update NC for enhanced error log message to host dblog.txt and errlog.txt
 *
 * Revision 1.1.2.16  2014/09/18 07:17:28  steja
 * 1.Update for 2 CPU cookie
 * 2.Update NC command to print the module errlog.
 * 3.Update switch consoel to module menu
 *
 * Revision 1.1.2.15  2014/09/18 03:39:02  palin2
 * Updated Power on/off Skye SM function to fix CSCuq91311.
 *
 * Revision 1.1.2.14  2014/09/16 15:26:25  steja
 * 1. move the firmware download to the run do all test
 * 2. remove *** replace ===
 * 3. arrange firmware download menu item
 *
 * Revision 1.1.2.13  2014/09/12 14:38:04  steja
 * Update NC and Uart firmware download for CPU1
 *
 * Revision 1.1.2.12  2014/09/09 09:02:13  steja
 * Add skye rx uart to print the test progress.
 *
 * Revision 1.1.2.11  2014/09/04 12:51:56  steja
 * Enhanced 10GKR support capability
 *
 * Revision 1.1.2.10  2014/09/02 13:09:10  steja
 * Update firmware download code
 *
 * Revision 1.1.2.8  2014/08/28 02:54:01  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.7  2014/08/25 11:57:09  steja
 * Update code for support BST testing
 *
 * Revision 1.1.2.6  2014/08/21 03:44:19  palin2
 * Update Skye module reset utility based on its HW design change.
 *
 * Revision 1.1.2.5  2014/08/21 02:29:29  palin2
 * Fixed misused of checking VERBOSE flag option.
 *
 * Revision 1.1.2.4  2014/08/15 03:26:55  palin2
 * Initial check-in to support NC command on Skye.
 *
 * Revision 1.1.2.3  2014/08/15 02:11:57  palin2
 * Changed Skye default baud rate to 9600.
 *
 * Revision 1.1.2.2  2014/08/08 08:32:36  steja
 * Add the Run Skye SM test
 *
 * Revision 1.1.2.1  2014/07/17 06:32:22  palin2
 * Initial check-in Skye host side code.
 *
 *------------------------------------------------------------------
 * shrinkray_host.c:
 * Revision 1.2.8.10  2014/07/08 20:49:49  palin2
 * Changed to use 9600 as Shrinkray Diag default baud rate based on SW team's BIB setup.
 *
 * Revision 1.2.8.9  2014/06/27 15:22:03  steja
 * Coding for config iptable
 *
 * Revision 1.2.8.8  2014/06/25 13:11:43  steja
 * Add Tftpdownload Shrinkray Firmware
 *
 * Revision 1.2.8.7  2014/06/05 08:37:59  palin2
 * Enable Hi-Power mode when module is up to simplify EDVT test process.
 *
 * Revision 1.2.8.6  2014/05/28 02:09:09  steja
 * minor changes
 *
 * Revision 1.2.8.4  2014/05/26 00:29:51  palin2
 * To fix CSCun50457 and support ShrinkRay host side power on/off/cycle utilities.
 *
 * Revision 1.2.8.3  2014/05/14 14:31:55  palin2
 * Update Menu to put test items together.
 *
 * Revision 1.2.8.2  2014/05/12 08:37:52  steja
 * Add DO_ALL flag to Host GE 0 to TLK Loopback test
 *
 * Revision 1.2.8.1  2014/05/11 07:53:29  steja
 * Update code for future use
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

