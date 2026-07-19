/* $Id: nim_grimlock.c,v 1.4 2020/12/22 14:30:19 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nim_grimlock.c,v $
 *------------------------------------------------------------------
 *
 * nim_grimlock.c - This file contains functions for Grimlock NIM.
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"

#include "cookie_4.h"

#include <stdlib.h>
#include <string.h>

extern int tftp_get(char *dir, char *file, 
     char *server_ip, char *dest, unsigned int check);
extern int ovld_get_ge_sw_port_num(int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);
extern int dc_test(void *, unsigned int);
extern int dc_iface_test(void *, unsigned int);
extern int print_dc_slots(struct ngio_intf_t *, char *);
extern int get_cookie_pid(int slot, int type, unsigned char *eeprom_data, char *pid);

static int ltc4215_register_test(void);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int grimlock_power_off(void);
static int grimlock_pwr_off(void);
static int grimlock_pwr_on(void);
static int grimlock_pwr_cycle(void);
static int grimlock_utils(void);
static int grimlock_console_switch(void);
static int enable_bp_ge_lpbk(void);
static int disable_bp_ge_lpbk(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int set_ngwic_console(void);
static int set_ngvm_console(void);
static int grimlock_dc_test(int);
static int grimlock_dc_iface_test(int);
static int grimlock_uart_test(void);
static int check_grimlock_dc_pids_wrap(void);
static int check_grimlock_dc_pids(void);

static void (*grimlock_saved_diag_exec)(void) = NULL;
static void *oir_if;

static n2g_i2c_if_t *pca_i2c;

static struct ngio_intf_t *grimlock_wic_iface;

/*
 *
 * The following staucture and table are only used for the Grimlock NIM
 * to check the the PIDs of Grimlock NIM and its associated
 * PVDM (32/64/128/256) match.
 */

typedef struct grimlock_pvdm_pid_pair
{
    char *grimlock_pid;
    char *pvdm_pid;
} grimlock_pvdn_pid_pair_t;

static grimlock_pvdn_pid_pair_t grimlock_pvdm_pids_table[] =
{
    {"NIM-PVDM-32",     "PVDM4-32"},
    {"NIM-PVDM-64",     "PVDM4-64"},
    {"NIM-PVDM-128",    "PVDM4-128"},
    {"NIM-PVDM-256",    "PVDM4-256"},
    {NULL,              NULL}           /* end of table */
};

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t grimlock_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)grimlock_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off Grimlock NGWIC",     (PFT)grimlock_power_off,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Grimlock NGWIC",      (PFT)grimlock_pwr_on,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Grimlock NGWIC",   (PFT)grimlock_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Enable Backplane GE loopback",  (PFT)enable_bp_ge_lpbk,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Disable Backplane GE loopback", (PFT)disable_bp_ge_lpbk,    0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)grimlock_uart_test,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Test",         (PFT)ltc4215_register_test, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Read",         (PFT)ltc4215_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Write",        (PFT)ltc4215_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 Register Read",         (PFT)pca9557_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 Register Write",        (PFT)pca9557_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Set NGWIC Console",             (PFT)set_ngwic_console,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Set Daughtercard Console",      (PFT)set_ngvm_console,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Check Grimlock with PVDM PID pair", (PFT)check_grimlock_dc_pids_wrap, 0,0,
        (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define GRIMLOCK_UTILS_SUBMENU_TABLE_SZ (sizeof(grimlock_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t grimlock_utils_primary_items[GRIMLOCK_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t grimlock_utils_secondary_items[GRIMLOCK_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

char grimlock_util_title[50];

menuinfo_t grimlock_util_submenu = {
    grimlock_util_title,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    grimlock_utils_primary_items,
};

menuinfo_t *grimlock_util_submenup = &grimlock_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Grimlock Utilities",           (PFT)grimlock_utils,       0,   0,
     (type_t(*)())0, 0,    (type_t(*)())grimlock_utils, 0},
    {"Grimlock NIM test",            (PFT)grimlock_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Daughter Card Test",            (PFT)grimlock_dc_test, FIRST_SLOT, 
      MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
      (type_t(*)())0, 0,   (type_t(*)())grimlock_dc_test,FIRST_SLOT+MAX_DC},
    {"Daughter Card I/O Test",        (PFT)grimlock_dc_iface_test,FIRST_SLOT, 
      MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
      (type_t(*)())0, 0,   (type_t(*)())0,         0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Grimlock Main Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;
  
/*
 **********************************************************************
 *
 *  Function: grimlock_utils
 *
 *  Description: Grimlock Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int grimlock_utils (void)
{
    assert(grimlock_wic_iface);

    sprintf(grimlock_util_title, "Grimlock Slot %d Utilities Menu", 
            grimlock_wic_iface->slot);
    build_primary_submenu(grimlock_utils_submenu_table,
                          GRIMLOCK_UTILS_SUBMENU_TABLE_SZ,
                          grimlock_util_title, &grimlock_util_submenup);

    build_secondary_submenu(grimlock_utils_submenu_table,
                            GRIMLOCK_UTILS_SUBMENU_TABLE_SZ,
                            grimlock_utils_secondary_items);

    menu(grimlock_util_submenup, grimlock_utils_secondary_items, '\0');

    return (PASSED);
}

/**********************************************************************
 * Function: grimlock_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void grimlock_cleanup (void)
{
    assert(grimlock_wic_iface);

    if (grimlock_saved_diag_exec) {
        pre_diag_exec = grimlock_saved_diag_exec;
        grimlock_saved_diag_exec = NULL;
    }
}

/**********************************************************************
 * Function: set_gpio_db_pins.
 *
 * Description: This function will set the config register of the 
 *              PCA9557 I2C device (GPIO expander) for the 
 *              daughterboard related bits.
 *
 * Input:  none
 *
 * Output: PASSED/FAILED.
 **********************************************************************
 */
static int set_gpio_db_pins (void)
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~(DB_RESET_L | UART_MUX);
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
        return (FAILED);
    }

    /* Keep DB in reset, till pass through mode is set on Grimlock */
    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
        return (FAILED);
    }
    data &= ~(DB_RESET_L);
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
        return (FAILED);
    }
    return (PASSED);
}

/*************************************************************************
 * Function: grimlock_uart_test
 *
 * Test the UART connection from the host to Grimlock.
 * Also test the GE0 interface by checking diag image download successful or not.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int grimlock_uart_test ()
{
    const int maxlen = 28;
    char test_if[maxlen];;
    int rv;

    /* 'n\n' for trigger Grimlock side diag sub-item,
     * which will invoke 'uname -a'.
     */
    assert(grimlock_wic_iface);

    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d", 
             grimlock_wic_iface->uart_ctrl);

    prpass(testpass, "Grimlock UART ");

    rv = uart_msg_exh_test(test_if, "\n", "Menuitem>", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"Grimlock UART test failed\n");
    }

    sleep(1);

    rv = uart_msg_exh_test(test_if, "n\n", "Linux", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"Grimlock UART test failed\n");
    }
    return (rv);
}


/*************************************************************************
 * Function: grimlock_iface_test
 *
 * Test entry for Grimlock interface test.
n *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int grimlock_iface_test ()
{
    uchar data;
    int ix;
    int wait_time = 3000;

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
	return (FAILED);
    }

    printf("\nWait for Grimlock module side to boot up diag menu.\n");

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Grimlock module side when the diag menu is up. */
    for (ix = 0; ix < wait_time; ix++) {

	if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
	    cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	    return (FAILED);
	}

	if (data & 0x08)
	    break;

	msleep(200);
    }

    if (ix == wait_time) {
	cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
	return (FAILED);
    }

    sleep(3);

    /* Testing UART and GE0 interfaces */
    if (grimlock_uart_test()) {
	return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 *
 * Function: grimlock_test().
 *
 * Description: This function is the entry point for Grimlock NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int grimlock_test (void *wic)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;

    assert(wic);

    grimlock_wic_iface = (struct ngio_intf_t *)wic;

    slot = grimlock_wic_iface->slot;
    board_id = grimlock_wic_iface->id;

    grimlock_wic_iface->uart_on(wic);    
    
    printf("\ngrimlock_test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Grimlock NGWIC ", slot);

    oir_if = (void *)(grimlock_wic_iface->oir);

    pca_i2c = grimlock_wic_iface->pca;

    /* config db_present_l, boot_select, db_reset_l etc per NGIO spec */
    set_gpio_db_pins();

    if (tftp_get(0, "grimlock_diag",
		 0, "/firmware/ngwic_t1e1_fw.img", 1) < 0) {
	cterr('f', 0, "Failed to tftp download firmware to local host");
	return (FAILED);
    }

    system("cp /firmware/ngwic_t1e1_fw.img /firmware/nim_t1e1_fw.img");
    system("cp /firmware/ngwic_t1e1_fw.img /firmware/nim_8cht1e1_fw.img");

    /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
     * not the ability of auto-negotiation between 1G and 10G */
    module_type = grimlock_wic_iface->mod_type;
    new_speed = NGIO_ETH_SPEED_1G;
    ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);

    grimlock_wic_iface->unreset(wic);
    msleep(1000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    /* specify the mod_type as wic dc, 
     * to consider the case about thule, which init dc as sm dc. 
     */
    grimlock_wic_iface->dc->mod_type = WIC_DAUGHTER_CARD;

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    grimlock_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    if (grimlock_wic_iface->test_type == IFACE_TEST) {
	ret_val = grimlock_iface_test();
    } else {
	menu(maindiagp, main_menu_secondary_items, '\0');
    }

    grimlock_cleanup();

    /* Curie 2RU: restore eth configuration */
    ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);

    return (ret_val);
}

/**********************************************************************
 * Function: grimlock_dc_iface_test
 *
 * Description: Call the daughter interface test
 *
 * Input : dc_slot : daughter card slot 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int grimlock_dc_iface_test (int dc_slot)
{
    int rv, i, wait_time = 5000;
    const int maxlen = 28;
    uchar data;
    char test_if[maxlen];;
    char ngio_str[32];

    assert(grimlock_wic_iface);

    sprintf(ngio_str, "WIC%d Daughtercard", grimlock_wic_iface->slot);
    if ((rv = print_dc_slots(grimlock_wic_iface->dc, ngio_str)) != PASSED)
        return (rv);

    if (grimlock_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Grimlock NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (grimlock_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Grimlock NGWIC");
        return(FAILED);
    }

    /* DC interface test done in two step
       1. Parent NGIO interface test
       2. Parent - DC interface test */

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }


    /* Test called from Grimlock Main Menu. Please first run the parent
       ngio interface test to verify the GE 0 link and Uart interface. */
    printf("\nWait for Grimlock module side to boot up diag menu.\n");
    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Grimlock module side when the diag menu is up. */
    for (i = 0; i < wait_time; i++) {

        if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
            cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
            return (FAILED);
        }

        if (data & 0x08)
            break;

        msleep(200);
    }

    if (i == wait_time) {
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }

    sleep(3);

    /* Testing UART interfaces */
    set_ngwic_console();

    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d", 
             grimlock_wic_iface->uart_ctrl);

    prpass(testpass, "Grimlock UART ");

    rv = uart_msg_exh_test(test_if, "\n", "Menuitem>", TRIG_DIAG_M);
    if (rv == FAILED) {
        cterr('f',0,"Grimlock UART test failed\n");
    }

    sleep(1);

    rv = uart_msg_exh_test(test_if, "m\n", "errors=0", TRIG_DIAG_M);
    if (rv == FAILED) {
        cterr('f',0,"Grimlock UART test failed\n");
    }

    set_ngvm_console();
    return (dc_iface_test(grimlock_wic_iface, dc_slot));
}

/**********************************************************************
 * Function: grimlock_dc_test
 *
 * Description: Call the daughter card test
 *
 * Input : dc_slot : daughter card slot 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int grimlock_dc_test (int dc_slot)
{ 
    assert(grimlock_wic_iface);
    set_ngvm_console();
    return (dc_test(grimlock_wic_iface, dc_slot));
}

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
    int ret;

    prpass(testpass, "LTC4215 OIR Register test ");

    ret = oir_ltc4215_register_test(oir_if);
    if (ret == FAILED)
	cterr('f',0,"LTC4215 register test failed.");

    return (ret);
}


/**********************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_reg_write(void)
{
    return(util_oir_ltc4215_reg_write(oir_if));
}

/**********************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_reg_read(void)
{
    return(util_oir_ltc4215_reg_read(oir_if));
}

/**********************************************************************
 *
 * Function: grimlock_pwr_off
 *
 * Description: This function power off Grimlock NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int grimlock_pwr_off (void)
{
    uint8_t data = 0;
    int slot;

    assert(oir_if);
    assert(grimlock_wic_iface);

    slot = grimlock_wic_iface->slot;
    printf("\nPower Off the Grimlock NGWIC.\n");

#ifdef TABEIL
    ngiowic_disable_intr (slot, NGIO_FLT_INTR);
#else
    /* disable power interrupt */
    if (grimlock_wic_iface->mod_type == DAUGHTER_CARD) {
        ;
    } else if (grimlock_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        ngiosm_disable_intr (slot, NGIO_FLT_INTR);
    } else {
        ngiowic_disable_intr (slot, NGIO_FLT_INTR);
    }
#endif

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: grimlock_power_off
 *
 * Description: This function is a wrapper to power off Grimlock NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int grimlock_power_off (void)
{
    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Grimlock NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    return (grimlock_pwr_off());
}


/**********************************************************************
 *
 * Function: grimlock_pwr_on
 *
 * Description: This function power on Grimlock NGWIC.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int grimlock_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Grimlock NGWIC.\n");

    assert(grimlock_wic_iface);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    /* put slot_i2c_unreset after power on NGWIC module for Switzer Carrier */
    if (grimlock_wic_iface->mod_type != DAUGHTER_CARD)
        slot_i2c_unreset(grimlock_wic_iface, grimlock_wic_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power on NGWIC module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
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

    printf("Waiting for Grimlock NGWIC to Power-Up.\n");
    msleep(2000);

    if (grimlock_wic_iface->mod_type == DAUGHTER_CARD)
        slot_i2c_unreset(grimlock_wic_iface, grimlock_wic_iface->slot, "WIC");

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    grimlock_wic_iface->uart_on(grimlock_wic_iface);    

    /* take Grimlock NGWIC out of reset */
    grimlock_wic_iface->unreset(grimlock_wic_iface);

    printf("Grimlock NGWIC is powered up.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: grimlock_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int grimlock_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the Grimlock NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Grimlock is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (grimlock_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Grimlock NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (grimlock_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Grimlock NGWIC");
        return(FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: pca9557_reg_read
 *
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pca9557_reg_read (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    printf("\nRegister @ %#x = %#x\n", offset, data);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: pca9557_reg_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pca9557_reg_write (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_ngwic_console
 *
 * Description: Set console.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int set_ngwic_console ()
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    /* set GPIO pin 4 to 0 for NGWIC console redirect */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: set_ngvm_console
 *
 * Description: Set console.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int set_ngvm_console ()
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    /* set GPIO pin 4 to 1 for daughtercard console redirect */
    data |= 0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: grimlock_console_switch
 *
 * Description: Set console.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int grimlock_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(grimlock_wic_iface);

    set_ngwic_console();

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); 

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d", 
             grimlock_wic_iface->uart_ctrl);

    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: enable_bp_ge_lpbk
 *
 * Description: Enable loopback.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int enable_bp_ge_lpbk ()
{
    int ge_port;

    assert(grimlock_wic_iface);

#ifdef TABEIL
    ge_port = 0;
#else
    if(grimlock_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(grimlock_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(grimlock_wic_iface->slot, TGT_DEV_NGWIC, 0);
#endif
    set_gesw_line_loopback(ge_port, 1);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: enable_bp_ge_lpbk
 *
 * Description: Disable loopback.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int disable_bp_ge_lpbk ()
{
    int ge_port;

    assert(grimlock_wic_iface);

#ifdef TABEIL
    ge_port = 0;
#else
    if(grimlock_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(grimlock_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(grimlock_wic_iface->slot, TGT_DEV_NGWIC, 0);
#endif
    set_gesw_line_loopback(ge_port, 0);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: check_grimlock_dc_pids (only for Grimlock NIM)
 *
 * Description: Get PIDs of Grimlock NIM and associated PVDM and check 
 *              both match or not
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

static int check_grimlock_dc_pids(void)
{
    struct ngio_intf_t *wic, *dc;
    char wic_pid[COOKIE_SIZE_32], dc_pid[COOKIE_SIZE_32]; 
    grimlock_pvdn_pid_pair_t *gp;
    int ret = FAILED;
    
    assert(grimlock_wic_iface);
    wic = grimlock_wic_iface;
    assert(wic->dc);
    dc = wic->dc;
    memset(wic_pid, 0, COOKIE_SIZE_32);
    memset(dc_pid, 0, COOKIE_SIZE_32);

    printf("\nChecking Grimlock with associated PVDM PID pair (32/64/128/256)...\n");
    get_cookie_pid(wic->slot, wic->mod_type, wic->cookie, wic_pid);

    /* Platform could get the PID of Daughter Card under Grimlock NIM. 
     * The code structure located in platform_cookie.c or platform_tam_cookie.c,
     * depends on platforms. */
    print_dc_slots(dc, "Grimlock DC");
    get_cookie_pid(wic->slot, dc->mod_type, dc->cookie, dc_pid);

    printf("NIM PID:%s\n", wic_pid);
    printf("DC PID:%s\n", dc_pid);

    printf("Found Grimlock PID:%s with PVDM PID:%s\n", wic_pid, dc_pid);

    for (gp = grimlock_pvdm_pids_table; gp->grimlock_pid; gp++) {
        if (!strncmp(wic_pid, gp->grimlock_pid, strlen(gp->grimlock_pid))) {
            /* found Grimlock PID in table */
            if (!strncmp(dc_pid, gp->pvdm_pid, strlen(gp->pvdm_pid))) {
                ret = PASSED;
            }
            break;
        }
    }

    return (ret);
}

static int check_grimlock_dc_pids_wrap(void)
{
    int ret = check_grimlock_dc_pids();

    if (ret == PASSED) {
        printf("Passed. Grimlock with associated PVDM PID pair (32/64/128/256) match!\n");
    } else {
        printf("Failed. Grimlock with associated PVDM PID pair (32/64/128/256) do NOT match!\n");
    }

    return (ret);
}

/******** History ********
$Log: nim_grimlock.c,v $
Revision 1.4  2020/12/22 14:30:19  jiajliu
CSCvu31200-5: Switer Carrier: Add support for Grimlock and DC Graffham

Revision 1.3  2020/04/01 06:30:39  letsai
Fix PID getting issue(CSCvt60494)

Revision 1.2  2020/03/13 12:12:17  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.3  2020/02/05 03:01:45  shhuang
use strlen of PID as length for the strncmp to compare two PIDs.

Revision 1.1.4.2  2020/01/14 06:46:53  wilbhuan
[GRIMLOCK DEVELOP] Supported Grimlock NIM test menu on platform side.

$Endlog$
*/
