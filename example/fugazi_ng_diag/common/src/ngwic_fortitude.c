/* $Id: ngwic_fortitude.c,v 1.46 2021/08/17 11:50:21 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/ngwic_fortitude.c,v $
 *------------------------------------------------------------------
 *
 * ngwic_fortitude.c - This file contains functions for Fortitude NGWIC.
 *
 * Christine Wen -- Dec. 2011
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
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
#include "ngio.h"

#include <stdlib.h>

#ifdef TACHI
#include "diag_console_util.h"
#include "cetus_gesw_defs.h"
#include "diag_fpga_lib.h"
#endif

#ifdef PHOENIX
#include "dnv_eth_lib.h"
#endif

extern int tftp_get (char *dir, char *file, 
     char *server_ip, char *dest, unsigned int check);
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);
extern int dc_test(void *, unsigned int);
extern int dc_iface_test(void *, unsigned int);
extern int print_dc_slots(struct ngio_intf_t *, char *);
extern int is_curie_1ru(void);
extern int is_curie_2ru(void);

static int ltc4215_register_test (void);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int fortitude_power_off (void);
static int fortitude_pwr_off (void);
static int fortitude_pwr_on (void);
static int fortitude_pwr_cycle (void);
static int fortitude_utils(void);
static int fortitude_console_switch(void);
static int enable_bp_ge_lpbk(void);
static int disable_bp_ge_lpbk(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int fortitude_o2_shell(void);
static int fortitude_o2_command(void);
static int set_ngwic_console(void);
static int set_ngvm_console(void);
static int fortitude_dc_test(int);
static int fortitude_dc_iface_test(int);
static int fortitude_uart_test (void);

static void (*fortitude_saved_diag_exec)(void) = NULL;
static void *oir_if;

static n2g_i2c_if_t *pca_i2c;

static struct ngio_intf_t *fortitude_wic_iface;

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t fortitude_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)fortitude_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off Fortitude NGWIC",     (PFT)fortitude_power_off,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Fortitude NGWIC",      (PFT)fortitude_pwr_on,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Fortitude NGWIC",   (PFT)fortitude_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Enable Backplane GE loopback",  (PFT)enable_bp_ge_lpbk,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Disable Backplane GE loopback", (PFT)disable_bp_ge_lpbk,    0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)fortitude_uart_test,   0,   0,
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
    {"Escape to Shell (debugging only)", (PFT)fortitude_o2_shell,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)fortitude_o2_command,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},   
};

#define FORTITUDE_UTILS_SUBMENU_TABLE_SZ (sizeof(fortitude_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t fort_utils_primary_items[FORTITUDE_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t fort_utils_secondary_items[FORTITUDE_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

char fortitudeutiltitle[50];

menuinfo_t fortitude_util_submenu = {
    fortitudeutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fort_utils_primary_items,
};

menuinfo_t *fortitude_util_submenup = &fortitude_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Fortitude Utilities",           (PFT)fortitude_utils,       0,   0,
     (type_t(*)())0, 0,    (type_t(*)())fortitude_utils, 0},
    {"Fortitude NIM test",            (PFT)fortitude_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Daughter Card Test",            (PFT)fortitude_dc_test, FIRST_SLOT, 
      MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
      (type_t(*)())0, 0,   (type_t(*)())fortitude_dc_test,FIRST_SLOT+MAX_DC},
    {"Daughter Card I/O Test",        (PFT)fortitude_dc_iface_test,FIRST_SLOT, 
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
    "Fortitude Main Menu",	/* title */
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
 *  Function: fortitude_utils
 *
 *  Description: Fortitude Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int fortitude_utils (void)
{
    assert(fortitude_wic_iface);

    sprintf(fortitudeutiltitle, "Fortitude Slot %d Utilities Menu", 
            fortitude_wic_iface->slot);
    build_primary_submenu(fortitude_utils_submenu_table,
                          FORTITUDE_UTILS_SUBMENU_TABLE_SZ,
                          fortitudeutiltitle, &fortitude_util_submenup);

    build_secondary_submenu(fortitude_utils_submenu_table,
                            FORTITUDE_UTILS_SUBMENU_TABLE_SZ,
                            fort_utils_secondary_items);

    menu(fortitude_util_submenup, fort_utils_secondary_items, '\0');

    return (PASSED);
}

/**********************************************************************
 * Function: fortitude_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void
fortitude_cleanup (void)
{
    assert(fortitude_wic_iface);

    if (fortitude_saved_diag_exec) {
        pre_diag_exec = fortitude_saved_diag_exec;
        fortitude_saved_diag_exec = NULL;
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
static int 
set_gpio_db_pins (void)
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

    /* Keep DB in reset, till pass through mode is set on fortitude */
    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }
    data &= ~(DB_RESET_L);
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
        return (FAILED);
    }
    return (PASSED);
}

/*************************************************************************
 * Function: fortitude_uart_test
 *
 * Test the UART connection from the host to Fortitude.
 * Also test the GE0 interface by checking diag image download successful or not.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
fortitude_uart_test ()
{
    const int maxlen = 28;
    char test_if[maxlen];;
    int rv;

    /* 'n\n' for trigger Fortitude side diag sub-item,
     * which will invoke 'uname -a'.
     */
    assert(fortitude_wic_iface);

#ifdef TACHI
    snprintf(test_if, maxlen-1, "/dev/ttyS%d", 
             fortitude_wic_iface->uart_ctrl);
#else
    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d", 
             fortitude_wic_iface->uart_ctrl);
#endif

    prpass(testpass, "Fortitude UART ");

    rv = uart_msg_exh_test(test_if, "\n", "Menuitem>", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"Fortitude UART test failed\n");
    }

    sleep(1);

    rv = uart_msg_exh_test(test_if, "n\n", "Linux", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"Fortitude UART test failed\n");
    }
    return (rv);
}


/*************************************************************************
 * Function: fortitude_iface_test
 *
 * Test entry for Fortitude interface test.
n *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
fortitude_iface_test ()
{
    uchar data;
    int i;
    int wait_time = 3000;

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
	return (FAILED);
    }

    printf("\nWait for Fortitude module side to boot up diag menu.\n");

#ifdef PHOENIX
    /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry 
     * here to workaround Denverton loopback issue*/
    int retry_count = 0;
    int slot = fortitude_wic_iface->slot;
retry:
    if (slot==PHOENIX_NIM0_SLOT && !retry_count) {
        system(PHOENIX_ETH_NIM0_SLOT_DOWN);
        system(PHOENIX_ETH_NIM0_SLOT_UP);
    } else if (slot==PHOENIX_NIM1_SLOT && !retry_count) {
        system(PHOENIX_ETH_NIM1_SLOT_DOWN);
        system(PHOENIX_ETH_NIM1_SLOT_UP);
    }
#endif

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Fortitude module side when the diag menu is up. */
    for (i = 0; i < wait_time; i++) {

	if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
	    cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	    return (FAILED);
	}

	if (data & 0x08)
	    break;

	msleep(200);
    }

#ifdef PHOENIX
    /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry 
     * here to workaround Denverton loopback issue*/
	if (!(data & 0x08) && retry_count<PHOENIX_ETH_RETRY) { 
            retry_count++;
            printf("\nWarning, this is fortitude test retry %d time\n", retry_count);
	    goto retry;
	}
#endif

    if (i == wait_time) {
	cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
	return (FAILED);
    }

    sleep(3);

    /* Testing UART and GE0 interfaces */
    if (fortitude_uart_test()) {
	return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 *
 * Function: fortitude_test().
 *
 * Description: This function is the entry point for Fortitude NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int 
fortitude_test (void *wic)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;

    assert(wic);

    fortitude_wic_iface = (struct ngio_intf_t *)wic;

    slot = fortitude_wic_iface->slot;
    board_id = fortitude_wic_iface->id;

    fortitude_wic_iface->uart_on(wic);    
    
    printf("\nfortitude_test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Fortitude NGWIC ", slot);

    oir_if = (void *)(fortitude_wic_iface->oir);

    pca_i2c = fortitude_wic_iface->pca;

    /* config db_present_l, boot_select, db_reset_l etc per NGIO spec */
    set_gpio_db_pins();

    if (tftp_get(0, "fortitude_diag",
		 0, "/firmware/ngwic_t1e1_fw.img", 1) < 0) {
	cterr('f', 0, "Failed to tftp download firmware to local host");
	return (FAILED);
    }

    system("cp /firmware/ngwic_t1e1_fw.img /firmware/nim_t1e1_fw.img");
    system("cp /firmware/ngwic_t1e1_fw.img /firmware/nim_8cht1e1_fw.img");

    /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
     * not the ability of auto-negotiation between 1G and 10G */
    module_type = fortitude_wic_iface->mod_type;
    new_speed = NGIO_ETH_SPEED_1G;
    ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);

    fortitude_wic_iface->unreset(wic);
    msleep(1000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    /* specify the mod_type as wic dc, 
     * to consider the case about thule, which init dc as sm dc. 
     */
    fortitude_wic_iface->dc->mod_type = WIC_DAUGHTER_CARD;

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    fortitude_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    if (fortitude_wic_iface->test_type == IFACE_TEST) {
	ret_val = fortitude_iface_test();
    } else {
	menu(maindiagp, main_menu_secondary_items, '\0');
    }

    fortitude_cleanup();

    /* Curie 2RU: restore eth configuration */
    ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);

    return (ret_val);
}

/**********************************************************************
 * Function: fortitude_dc_iface_test
 *
 * Description: Call the daughter interface test
 *
 * Input : dc_slot : daughter card slot 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fortitude_dc_iface_test (int dc_slot)
{
    int rv, i, wait_time = 5000;
    const int maxlen = 28;
    uchar data;
    char test_if[maxlen];;
    char ngio_str[32];

    assert(fortitude_wic_iface);

    sprintf(ngio_str, "WIC%d Daughtercard", fortitude_wic_iface->slot);
    if ((rv = print_dc_slots(fortitude_wic_iface->dc, ngio_str)) != PASSED)
        return (rv);

    if (fortitude_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Fortitude NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (fortitude_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Fortitude NGWIC");
        return(FAILED);
    }

    /* DC interface test done in two step
       1. Parent NGIO interface test
       2. Parent - DC interface test */

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }


    /* Test called from Fortitude Main Menu. Please first run the parent
       ngio interface test to verify the GE 0 link and Uart interface. */
    printf("\nWait for Fortitude module side to boot up diag menu.\n");
    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Fortitude module side when the diag menu is up. */
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

#ifdef TACHI
    snprintf(test_if, maxlen-1, "/dev/ttyS%d", 
             fortitude_wic_iface->uart_ctrl);
#else
    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d", 
             fortitude_wic_iface->uart_ctrl);
#endif

    prpass(testpass, "Fortitude UART ");

    rv = uart_msg_exh_test(test_if, "\n", "Menuitem>", TRIG_DIAG_M);
    if (rv == FAILED) {
        cterr('f',0,"Fortitude UART test failed\n");
    }

    sleep(1);

    rv = uart_msg_exh_test(test_if, "m\n", "errors=0", TRIG_DIAG_M);
    if (rv == FAILED) {
        cterr('f',0,"Fortitude UART test failed\n");
    }

    set_ngvm_console();
    return (dc_iface_test(fortitude_wic_iface, dc_slot));
}

/**********************************************************************
 * Function: fortitude_dc_test
 *
 * Description: Call the daughter card test
 *
 * Input : dc_slot : daughter card slot 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int 
fortitude_dc_test (int dc_slot)
{ 
    assert(fortitude_wic_iface);
#ifdef DC_DEBUG
    printf("\n dc_slot = %d ; %p", dc_slot, fortitude_wic_iface);
    printf("\n Set console to ngvm\n");
#endif
    set_ngvm_console();
    return (dc_test(fortitude_wic_iface, dc_slot));
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
static int
ltc4215_register_test (void)
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
static int
ltc4215_reg_write(void)
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
static int
ltc4215_reg_read(void)
{
    return(util_oir_ltc4215_reg_read(oir_if));
}


/**********************************************************************
 *
 * Function: fortitude_pwr_off
 *
 * Description: This function power off Fortitude NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
fortitude_pwr_off (void)
{
    uint8_t data = 0;
    int slot;

    assert(oir_if);
    assert(fortitude_wic_iface);

    slot = fortitude_wic_iface->slot;
    printf("\nPower Off the Fortitude NGWIC.\n");

#if defined(TACHI) || defined(TABEIL) || defined(NANOOK)
    ngiowic_disable_intr (slot, NGIO_FLT_INTR);
#else
    /* disable power interrupt */
    if (fortitude_wic_iface->mod_type == SM_DAUGHTER_CARD) {
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

    fortitude_wic_iface->off(fortitude_wic_iface);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fortitude_power_off
 *
 * Description: This function is a wrapper to power off Fortitude NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
fortitude_power_off (void)
{
    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Fortitude NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    return (fortitude_pwr_off());
}


/**********************************************************************
 *
 * Function: fortitude_pwr_on
 *
 * Description: This function power on Fortitude NGWIC.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
fortitude_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Fortitude NGWIC.\n");

    assert(fortitude_wic_iface);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(fortitude_wic_iface, fortitude_wic_iface->slot, "WIC");

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

    printf("Waiting for Fortitude NGWIC to Power-Up.\n");
    msleep(2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    fortitude_wic_iface->uart_on(fortitude_wic_iface);    

    /* take Fortitude NGWIC out of reset */
    fortitude_wic_iface->unreset(fortitude_wic_iface);

    printf("Fortitude NGWIC is powered up.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fortitude_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
fortitude_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the Fortitude NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Fortitude is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (fortitude_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Fortitude NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (fortitude_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Fortitude NGWIC");
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
static int
pca9557_reg_read (void)
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
static int
pca9557_reg_write (void)
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

static int 
set_ngwic_console ()
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


static int 
set_ngvm_console ()
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


static int
fortitude_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(fortitude_wic_iface);

    set_ngwic_console();

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); 

#ifdef TACHI
    diag_uart_to_nim_cnnt(fortitude_wic_iface->slot); 
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2");
#else
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d", 
             fortitude_wic_iface->uart_ctrl);
#endif


#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}

static int 
fortitude_o2_shell ()
{
    int slot;

    assert(fortitude_wic_iface);
    slot = fortitude_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\nEscaping to Shell from NGWIC Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);    
}


static int 
fortitude_o2_command ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return(PASSED);
}

static int
enable_bp_ge_lpbk ()
{
    int ge_port;

    assert(fortitude_wic_iface);

#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(fortitude_wic_iface->slot, TGT_DEV_NGWIC, 0);
#elif TABEIL
    ge_port = 0;
#elif NANOOK
    ge_port = 0;
#elif PHOENIX
    ge_port = 0;
#else
    if(fortitude_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(fortitude_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(fortitude_wic_iface->slot, TGT_DEV_NGWIC, 0);
#endif
    set_gesw_line_loopback(ge_port, 1);

    return (PASSED);
}

static int
disable_bp_ge_lpbk ()
{
    int ge_port;

    assert(fortitude_wic_iface);

#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(fortitude_wic_iface->slot, TGT_DEV_NGWIC, 0);
#elif TABEIL
    ge_port = 0;
#elif NANOOK
    printf("TBDTBD\n");
    ge_port = 0;
#elif PHOENIX
    ge_port = 0;
#else
    if(fortitude_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(fortitude_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(fortitude_wic_iface->slot, TGT_DEV_NGWIC, 0);
#endif
    set_gesw_line_loopback(ge_port, 0);

    return (PASSED);
}

/******** History ********
$Log: ngwic_fortitude.c,v $
Revision 1.46  2021/08/17 11:50:21  xiaolaya
Fix Fortitude power cycle utility issue when insert in Switzer-Carrier

Revision 1.45  2021/04/15 00:52:04  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.44  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

Revision 1.43  2019/12/11 10:10:22  lucywang
Merged Nanook to main trunk

Revision 1.42  2019/10/17 02:16:15  kehuang2
Collapse Tabei-L into main trunk

Revision 1.41  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.40.2.2  2018/11/22 08:33:28  alpeng
remove delay for curie, since we decrease the duration of link polling on opentftpd

Revision 1.40.2.1  2018/11/13 08:39:59  alpeng
ngvm uboot does not polling link status before downloading fw, add delay to fix racing condition.

Revision 1.40  2018/05/22 02:31:10  alpeng
fixed compiler warning, CSCvj57934

Revision 1.39  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.38  2017/03/30 08:21:05  hondwang
Tachi-L brach merge

Revision 1.37.30.1  2016/12/27 10:28:09  hondwang
Add Fortitude support

Revision 1.37.18.2  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.37.18.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.38  2017/03/30 08:21:05  hondwang
Tachi-L brach merge

Revision 1.37.30.1  2016/12/27 10:28:09  hondwang
Add Fortitude support

Revision 1.37  2015/06/09 08:26:45  alpeng
specify dc type as wic dc

Revision 1.36  2015/06/05 06:13:11  alpeng
fix poll slot issue

Revision 1.35  2014/11/26 07:00:42  alpeng
Support NGSM+NGWIC+NGVM case

Revision 1.34  2014/11/26 04:12:17  alpeng
reverting to version 1.32

Revision 1.32  2014/09/03 22:41:34  ywen
Add support for new Fortitude SKU: 8 channelized T1/E1

Revision 1.31  2014/08/26 08:48:42  bowang3
Make NIM support NGSM carrier card Thule

Revision 1.30  2014/03/03 22:27:45  mcharon
move daughter card init code to fortitude

Revision 1.29  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.28  2013/05/16 18:02:00  ywen
cut delay time during Fortitude IO interface test.

Revision 1.27  2013/03/08 21:11:17  srane
Standard menu for ngwic.

Revision 1.26  2013/02/28 00:36:06  srane
Add support for NGVM testcard.

Revision 1.25  2013/02/07 22:51:42  srane
Add Daughtercard interface test. (Requirement for DF).

Revision 1.24  2013/01/23 23:50:25  ywen
Turn on EN Led when enter NIM submenu.

Revision 1.23  2013/01/15 17:07:02  ywen
more improvement on UART test.

Revision 1.22  2013/01/14 21:44:57  ywen
- Increase delay time for UART test.
- Improve UART test to solve possible test issue.

Revision 1.21  2012/11/12 20:53:40  mcharon
remove printf(.) introduced by mistake

Revision 1.20  2012/11/12 20:35:22  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.19  2012/11/06 17:41:11  ywen
Add NGIO ready bit checking in IO interface test.

Revision 1.18  2012/10/01 23:20:01  ywen
- Add Fortitude UART test.
- Add Fortitude IO interface test.

Revision 1.17  2012/09/18 19:19:54  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.16  2012/09/10 05:59:33  srane
Add diag continuous flag for daughter card test.

Revision 1.15  2012/08/21 23:16:18  ywen
Change image location for tftp download.

Revision 1.14  2012/07/25 00:05:23  srane
-Keep daughter card in reset while running fortitude tests.

Revision 1.13  2012/07/23 06:48:47  srane
- configure db related output pins for GPIO expander.
- set ngvm console during daughter card test.

Revision 1.12  2012/06/28 21:39:04  srane
Add daughtercard support for all NGWIC slots.

Revision 1.11  2012/06/07 18:21:59  ywen
- Fixed all the compile warnings.
- Rename Fortitude module side image from "vmlinux" to "fortitude_diag".

Revision 1.10  2012/05/30 23:40:25  ywen
Fix the console redirect issue after the Fotitude NGWIC power cycle.

Revision 1.9  2012/05/23 22:43:32  ywen
- change nanocom to picocom
- remove tftp directory name hardcoding

Revision 1.8  2012/05/23 17:56:36  ywen
Change parameters to tftp_get function call to avoid hardcoding tftp server IP address.

Revision 1.7  2012/05/21 22:24:36  ywen
Add code to tftp download Fortitude image from host.

Revision 1.6  2012/05/11 20:49:58  ywen
- Change UART baud rate from 115200 to 9600.
- Add utilities to switch console between NGWIC and daughter card.

Revision 1.5  2012/05/04 20:01:45  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.4  2012/04/12 23:09:04  ywen
Add util to execute a shell cmd from menu.
Add util to escape to shell from menu.

Revision 1.3  2012/04/02 17:34:03  ywen
Add Fortitude console redirect utility.

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
