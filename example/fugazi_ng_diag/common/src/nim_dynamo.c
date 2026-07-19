/* $Id: nim_dynamo.c,v 1.15 2021/04/15 00:52:04 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nim_dynamo.c,v $
 *------------------------------------------------------------------
 *
 * nim_dynamo.c - This file contains functions for Dynamo NGWIC.
 *
 * Karuna Sabnis June, 2013
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"

#include "assert.h"

#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"

#include "router_if.h"
#include "platform_i2c.h"

#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "cross_platform.h"
#include "sgmii_defs.h"

#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"

#include <stdlib.h>

#include "i2c_api.h"
#include "cookie_4.h"
#include "dash_fpga.h"
#include "adapter_fpga.h"
#include "linux_ntwk.h"

#include "platform_fru.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include <stdio.h>
#include <string.h>
#ifdef TACHI
#include "diag_lewis_gesw_test.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_utils.h"
#include "diag_console_util.h"
#include <linux/filter.h>  /* pkt filter */
#include <arpa/inet.h>
#include "diag_fpga_lib.h"
#elif TABEIL
#include "diag_fpga_lib.h"
#include "diag_fpga.h"
#elif PHOENIX
#include "diag_fpga_lib.h"
#include "diag_fpga.h"
#include "dnv_eth_lib.h"
#else
#include "platform_eth_pkt_txrx.h"
#endif


/* #define CTERR_SIM  * */

static struct ngio_intf_t *ngio_ptr;

extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);
extern int ngio_sync_out_enable (void *p, int mask);
extern void display_env(void);

static int ltc4215_register_test (void);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int dynamo_power_off (void);
static int dynamo_pwr_off (void);
static int dynamo_pwr_on (void);
static int dynamo_pwr_cycle (void);
static int dynamo_utils(void);
static int dynamo_console_switch(void);
static int enable_bp_ge_lpbk(void);
static int disable_bp_ge_lpbk(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int dynamo_o2_shell(void);
static int dynamo_o2_command(void);

static int set_ngwic_console(void);

static int dynamo_uart_test (void);

static void (*dynamo_saved_diag_exec)(void) = NULL;
static void *oir_if;
static n2g_i2c_if_t *pca_i2c;

static struct ngio_intf_t *dynamo_wic_iface;

#define DYNAMO_PID_COMMON "NIM-"

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t dynamo_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)dynamo_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off Dynamo NGWIC",     (PFT)dynamo_power_off,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Dynamo NGWIC",      (PFT)dynamo_pwr_on,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Dynamo NGWIC",   (PFT)dynamo_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Enable Backplane GE loopback",  (PFT)enable_bp_ge_lpbk,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Disable Backplane GE loopback", (PFT)disable_bp_ge_lpbk,    0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)dynamo_uart_test,   0,   0,
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
    {"Escape to Shell (debugging only)", (PFT)dynamo_o2_shell,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)dynamo_o2_command,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},   
};

#define DYNAMO_UTILS_SUBMENU_TABLE_SZ (sizeof(dynamo_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t dyn_utils_primary_items[DYNAMO_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t dyn_utils_secondary_items[DYNAMO_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

char dynamoutiltitle[50];

menuinfo_t dynamo_util_submenu = {
    dynamoutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    dyn_utils_primary_items,
};

menuinfo_t *dynamo_util_submenup = &dynamo_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Dynamo Utilities",           (PFT)dynamo_utils,       0,   0,
     (type_t(*)())0, 0,    (type_t(*)())dynamo_utils, 0},
    {"Dynamo NIM test",            (PFT)dynamo_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Dynamo Main Menu",	/* title */
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
 *  Function: cterr_setup
 *
 *  Description: Setup Dynamo specific cterr parameters.
 *
 *  Input: slot
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void
cterr_setup(int slot)
{
    int pid_size;
    char *pid_ptr;
    char dyn_pid[80];	/* Dynamo PID is much less than this */

    /* Setup the PID */
    assert(ngio_ptr);

    memset(&dyn_pid[0], 0, sizeof(dyn_pid)); /* Set the null string */

    if (get_pid(ngio_ptr->cookie , &dyn_pid[0]) == FAILED) {
	cterr('f', 0, "%s: Unable to get PID from cookie", __FUNCTION__);
    }

    pid_size = strlen(dyn_pid);

    /* get the platform PID table */
    /* Assume the platform WIC# are sequential. If not, then use switch */
    fru_table_offset = WIC0 + slot;
    if (strncmp((char *)platform_fru_table[fru_table_offset].pid_string,
		&dyn_pid[0], pid_size)) {
	/* Not the correct PID */
	if (strncmp((char *)platform_fru_table[fru_table_offset].pid_string,
		    DYNAMO_PID_COMMON, sizeof(DYNAMO_PID_COMMON)) == 0) {
	    /* Was Dynamo */
	    free(platform_fru_table[fru_table_offset].pid_string);
	}
	pid_ptr = malloc(pid_size);
	strcpy(pid_ptr, &dyn_pid[0]);
	platform_fru_table[fru_table_offset].pid_string =
					(unsigned char *)pid_ptr;
    }

    /* Environment information */
    cterr_add_env_dump((PFV)display_env);

}

/*
 **********************************************************************
 *
 *  Function: dynamo_utils
 *
 *  Description: Dynamo Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int dynamo_utils (void)
{
    assert(dynamo_wic_iface);

    sprintf(dynamoutiltitle, "Dynamo Slot %d Utilities Menu", 
            dynamo_wic_iface->slot);
    build_primary_submenu(dynamo_utils_submenu_table,
                          DYNAMO_UTILS_SUBMENU_TABLE_SZ,
                          dynamoutiltitle, &dynamo_util_submenup);

    build_secondary_submenu(dynamo_utils_submenu_table,
                            DYNAMO_UTILS_SUBMENU_TABLE_SZ,
                            dyn_utils_secondary_items);

    menu(dynamo_util_submenup, dyn_utils_secondary_items, '\0');

    return (PASSED);
}

/**********************************************************************
 * Function: dynamo_cleanup()
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
dynamo_cleanup (void)
{

    assert(dynamo_wic_iface);

    if (dynamo_saved_diag_exec) {
        pre_diag_exec = dynamo_saved_diag_exec;
        dynamo_saved_diag_exec = NULL;
    }

    /* menu(), do_all, and do_group in menu.c will reset_errmsg_var()
     * which will clean up all the cterr settings */
}

/*************************************************************************
 * Function: dynamo_uart_test
 *
 * Test the UART connection from the host to Dynamo.
 * Also test the GE0 interface by checking diag image download successful or not.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
dynamo_uart_test ()
{
    assert(dynamo_wic_iface);
    char *str, *str1;
    char buf[1024] = {0};
    int port, rx_sz;
    int i;
    char *ptr, *ptr1;
    struct adapter_uart_t *padater_uart;
    if (dynamo_wic_iface->test_type == IFACE_TEST) {
        str = "durt\n";
        str1 = "Dynamo";
    } else {
        str = "m\n";
        str1 = "Linux";
    }

    memset(buf, 0, sizeof(buf));
#ifdef TACHI
    const int maxlen = 28;
    char test_if[maxlen];
    int ret_val;

    diag_uart_to_nim_cnnt(dynamo_wic_iface->slot); 
    snprintf(test_if, maxlen-1, UART_TTYS2_DEV);

    prpass(testpass, "Dynamo UART ");
    ret_val = uart_msg_exh_test(test_if, "diag\n", "Menuitem>",TRIG_DIAG_M);
    if (ret_val == FAILED) {
        cterr('f',0,"Dynamo UART test failed\n");
    }

    sleep(1);
    return (ret_val);
#else
    port = dynamo_wic_iface->uart_ctrl;
#endif

    prpass(testpass, "Dynamo UART ");

    if (dynamo_wic_iface->mod_type == DAUGHTER_CARD) {
        /* Switzer-carrier adapter card has its own control FPGA, use adapter Uart */
        /* utilities when NIM card is inserted into Switzer-carrier adapter card. */
        padater_uart = get_current_adapter_uart();
        padater_uart->adapter_uart_reset(port);
        padater_uart->adapter_uart_tx(port, 9600, str, strlen(str), 0);
    } else {
        dash_uart_reset(port);
        dash_uart_tx(port, 9600, str, strlen(str), 0);
    }

    sleep(1);  /* This may need to be adjusted */
    /*a delay between tx/rx here will cause uart to fail so
     after tx, quickly rx as soon as possible */
    rx_sz = 0;

    if (dynamo_wic_iface->mod_type == DAUGHTER_CARD) {
        /* dynamo inserted in Switzer carrier Adapter card */
        padater_uart->adapter_uart_rx(port, &rx_sz, buf);
    } else {
        dash_uart_rx(port, &rx_sz, buf);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("!!!!!string received=%s; [ sent %s] %s\n",
               buf, str, str1);
    }
    if (dynamo_wic_iface->mod_type == DAUGHTER_CARD) {
        /* dynamo inserted in Switzer carrier Adapter card */
        padater_uart->adapter_uart_reset(port);
    } else {
        dash_uart_reset(port);
    }

    ptr = memchr(buf, str1[0], strlen(str) + strlen(str1));
    ptr1 = ptr;
    if (ptr == NULL) {
        /* Component List */
        cterr_add_component("Router UART", "Dynamo NIM UART");
        cterr_add_debug("Console switch to Dynamo. Check if Diag is running",
                        "Check Dynamo UART on TISoC",
                        "Check router UART");
        cterr('f', 0, "rx buffer missing expect string: expected %s, got %s",
              str1, buf);
        return (FAILED);
    }
    for (i = 0; i < strlen(str1); i++, ptr++) {
        if (*ptr != str1[i]) {
            /* Component List */
            cterr_add_component("Router UART", "Dynamo NIM UART");
            cterr_add_debug("Console switch to Dynamo and check if Diag runs",
                            "Check Dynamo UART on TISoC",
                            "Check router UART");
            cterr('f', 0, "tx/rx strings do not match: expected %s, got %s",
                  str1, ptr1);
            return(FAILED);
        }
    }
    return(PASSED);
}

/*************************************************************************
 * Function: dynamo_iface_test
 *
 * Test entry for Dynamo interface test.
n *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
dynamo_iface_test ()
{
    uchar data;
    int i, rv;
    int wait_time = 5000;

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
	return (FAILED);
    }

    printf("\nWait for Dynamo module side to boot up diag menu.\n");

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Dynamo module side when the diag menu is up. */
    for (i = 0; i < wait_time; i++) {
	rv =io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE);
#ifdef CTERR_SIM
	rv = FAILED;
#endif /* CTERR_SIM */

	if (rv == FAILED) {
	    /* Setup the components and debug for cterr */
	    cterr_add_component("PCA9557 on Dynamo", "LV245 on Dynamo",
				"I2C controller at router");
	    cterr_add_debug("Check I2C components on Dynamo",
			    "Check the I2C controller at the router");

	    cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");

	    /* cterr setup will be cleared back in menu() */

	    return (FAILED);
	}

	if (data & 0x08)
	    break;

	msleep(200);
    }

#ifdef CTERR_SIM
    i = wait_time;
#endif /* CTERR_SIM */

    if (i == wait_time) {
	/* Setup the components and debug for cterr */
	cterr_add_component("TISoC on Dynamo", "PCA9557 on Dynamo");
	cterr_add_debug("Check the NGIO ready pin on Dynamo");
	cterr('f',0,"Primary interface ready pin assert timed out");
	/* cterr setup will be cleared back in menu() */
	return (FAILED);
    }

    sleep(3);

    /* Testing UART and GE0 interfaces */
    if (dynamo_uart_test()) {
	return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 *
 * Function: dynamo_test().
 *
 * Description: This function is the entry point for Dynamo NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int 
dynamo_test (void *wic)
{
    uchar data;
    int i;                  /* */
#ifdef TACHI
    int wait_time = 3000;   /*Tachi needs more time for uart to boot up */
#else
    int wait_time = 1000;   /* */
#endif
    int slot;
    ushort board_id = 0;
    const int maxlen = 28;  /* */
    char test_if[maxlen];   /* */
    int ret_val = PASSED;
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;

    assert(wic);
    dynamo_wic_iface = (struct ngio_intf_t *)wic;
    ngio_ptr = (struct ngio_intf_t *)wic;
    slot = dynamo_wic_iface->slot;
    board_id = dynamo_wic_iface->id;
    cterr_setup(slot);	/* Setup common parameters for new error message */

    dynamo_wic_iface->uart_on(wic);    
    
    printf("\ndynamo_test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Dynamo NGWIC ", slot);

    oir_if = (void *)(dynamo_wic_iface->oir);

    pca_i2c = dynamo_wic_iface->pca;

    /* Setup the components and debug for cterr */
    cterr_add_component("PCA9557 on Dynamo", "LV245 on Dynamo",
			"I2C controller at router");
    cterr_add_debug("Check I2C components on Dynamo",
		    "Check the I2C controller at the router");

    /* set GPIO pin 1 and 2 as output pins, and other pins as input pin */
    /* Pin 0 - DC_PRESENT_L  as input  */
    /* Pin 1 - BOOT_SELECT   as output. Not used by Dynamo HW */
    /* Pin 2 - HOST_DC_RST_L as output */
    /* Pin 3 - PRI_IF_RDY    as input  */
    data = BIT0 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    ret_val = io_port_8bit_i2c_write(pca_i2c, CONFIGURATION_REG, &data);
#ifdef CTERR_SIM_2
    ret_val = FAILED;
#endif /* CTERR_SIM */
    if (ret_val == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
	goto dynamo_test_fail;
    }
    
    /* Read again */
    ret_val = io_port_8bit_i2c_read(pca_i2c,CONFIGURATION_REG,&data,TRUE);
#ifdef CTERR_SIM_3
    ret_val = FAILED;
#endif /* CTERR_SIM */
    if (ret_val == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	goto dynamo_test_fail;
    }

#ifdef CTERR_SIM_4
    data = 0;
#endif /* CTERR_SIM */
    if (data != (BIT0 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)) {
	cterr('f', 0, "Fail to set PCA9557 Configuration Reg\n");
	goto dynamo_test_fail;
    }

    /* take HOST_DC_RST_L out of reset */
    data = BIT2;
    ret_val = io_port_8bit_i2c_write(pca_i2c, OUTPUT_PORT_REG, &data);
    if (ret_val == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
        goto dynamo_test_fail;
    }

    /* Read again */
    ret_val = io_port_8bit_i2c_read(pca_i2c, OUTPUT_PORT_REG, &data, TRUE);
    if (ret_val == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
        goto dynamo_test_fail;
    }

    if (data != BIT2) {
        cterr('f', 0, "Fail to set bit 2 of PCA9557 Output Port Reg\n");
        goto dynamo_test_fail;
    }

    /* cterr_add_... will cterr_clear_..., so no need to clear them */

    if (board_id == DYNAMO_NIM1 || board_id == DYNAMO_NIM2) {
        /*download Dynamo image */    
        ret_val = tftp_get(0, "dsp_analogbri_fw.img", 0,
               "/firmware/dsp_analogbri_fw.img", 1);
    } else {
        /*download Kazirznga image */    
        ret_val = tftp_get(0, "nim_bri_st_fw.img", 0,
	           "/firmware/nim_bri_st_fw.img", 1);
    }

    /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
     * not the ability of auto-negotiation between 1G and 10G */
    module_type = dynamo_wic_iface->mod_type;
    new_speed = NGIO_ETH_SPEED_1G;
    ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);

#ifdef CTERR_SIM_5
    ret_val = -1;
#endif /* CTERR_SIM */
    if (ret_val < 0) {
	/* Setup the components and debug for cterr */
	cterr_add_component("Router network controller",
			    "Router storage device controller");
	cterr_add_debug("Check the network connection at the platform");
	cterr('f', 0, "Failed to tftp download firmware to local host");
	goto dynamo_test_fail;
    }

    /* CSCup55761 & CSCuq12367 */
    /* POR for TISoC is 12P. Dynamo has 20 MHz clock for TISoC.
     * Thus 12 * 50ns = 600ns. Use 2 milliseconds for extras */
    /* FPGA configures with golden, then reconfigure with upgrade.
     * Total time to complete takes about 7 seconds */
    sleep (10); 

#ifdef PHOENIX
    /* CSCvx45420:
     * Base on CSCvn43011, Intel reply there isn't have solution, so we add
     * retry here to workaround Denverton loopback issue.
     */
    int retry_count = 0;
retry:
    if (slot==PHOENIX_NIM0_SLOT && !retry_count) {
        system(PHOENIX_ETH_NIM0_SLOT_DOWN);
        system(PHOENIX_ETH_NIM0_SLOT_UP);
    } else if (slot==PHOENIX_NIM1_SLOT && !retry_count) {
        system(PHOENIX_ETH_NIM1_SLOT_DOWN);
        system(PHOENIX_ETH_NIM1_SLOT_UP);
    }
#endif
    dynamo_wic_iface->unreset(wic);
    if ((diagflag_xram & D_DEBUG_OPTIONS) == 0 ) {
	/* Debug flag not set */
	printf("\nWait for Dynamo module side to boot up diag menu.\n");
	msleep(1000);
	/* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
	   by Dynamo module side when the diag menu is up. */
	for (i = 0; i < wait_time; i++) {
	    ret_val = io_port_8bit_i2c_read(pca_i2c, INPUT_PORT_REG, &data,
					    TRUE);
#ifdef CTERR_SIM_6
	    ret_val = FAILED;
#endif /* CTERR_SIM */

	    if (ret_val == FAILED) {
		/* Setup the components and debug for cterr */
		cterr_add_component("PCA9557 on Dynamo", "LV245 on Dynamo",
				    "I2C controller at router");
		cterr_add_debug("Check I2C components on Dynamo",
				"Check the I2C controller at the router");
		cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
		goto dynamo_test_fail;
	    }

	    if (data & BIT3)
		break;

	    if ((i%5) == 0) {
		printf(" .");fflush(stdout);
	    }
	    msleep(200);
	}

#ifdef CTERR_SIM_7
	i = wait_time;
#endif /* CTERR_SIM */

#ifdef PHOENIX
    /* CSCvx45420:
     * Base on CSCvn43011, Intel reply there isn't have solution, so we add
     * retry here to workaround Denverton loopback issue.
     */
	if (!(data & BIT3) && retry_count<PHOENIX_ETH_RETRY) { 
            retry_count++;
            printf("\nWarning, this is dynamo test retry %d time\n", retry_count);
	    goto retry;
	}
#endif
	if (!(data & BIT3)) {
	    /* Setup the components and debug for cterr */
	    cterr_add_component("TISoC on Dynamo", "PCA9557 on Dynamo");
	    cterr_add_debug("Check the NGIO ready pin on Dynamo");
	    cterr('f',0,"Timeout waiting for primary interface ready pin ",
			"asserted");
	    goto dynamo_test_fail;
	}
    }

    /* Add some delay for the module prompt DM8147 */
    msleep(1000);   /* */
    
    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
    /* Enable the ngio_sync_out_enable   */
    ngio_sync_out_enable (wic, ENABLE_PRE_SCALER_DIV_3125 |   SYNC_OUT_ENABLE );

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    dynamo_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    if (dynamo_wic_iface->test_type == IFACE_TEST) {
	ret_val = dynamo_iface_test();
    } else {
        if ((diagflag_xram & D_DEBUG_OPTIONS) == 0 ) {
            // When Dynamo is in Thule adapter card
#ifdef TACHI
            snprintf(test_if, maxlen-1, UART_TTYS2_DEV);
#else
            snprintf(test_if, maxlen-1, "/dev/ttyDASH%d",
                     dynamo_wic_iface->uart_ctrl);
#endif
            ret_val = uart_msg_exh_test(test_if, "diag\n", "Menuitem>",
                                        TRIG_DIAG_M);
            if (ret_val == FAILED) {
                /* Setup the components and debug for cterr */
                cterr_add_component("Router UART", "Dynamo NIM UART");
                cterr_add_debug("Console switch to Dynamo. Check Diag running",
                                "Check dynamo UART on TISoC",
                                "Check router UART");
                cterr('f',0,"Failed to bring up the Diag menu on Dyanamo module");
                goto dynamo_test_fail;
            }
            /* Add some delay for the diag menu */
            msleep(1000);   /* */
        }

	menu(maindiagp, main_menu_secondary_items, '\0');
    }

    dynamo_cleanup();

    /* Curie 2RU: restore eth configuration */
    ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);

    return (ret_val);

dynamo_test_fail:
    /* cterr setup will be cleared back in menu() */
    return(FAILED);
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
#ifdef CTERR_SIM
    ret = FAILED;
#endif /* CTERR_SIM */
    if (ret == FAILED) {
	/* Setup the components and debug for cterr */
	cterr_add_component("LTC4215 on Dynamo",
			    "I2C controller at router");
	cterr_add_debug("Check LTC4215 on Dynamo",
			"Check the I2C controller at the router");
	cterr('f',0,"LTC4215 register test failed.");
	/* cterr setup will be cleared back in menu() */
    }

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
 * Function: dynamo_pwr_off
 *
 * Description: This function power off Dynamo NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
dynamo_pwr_off (void)
{

    assert(oir_if);
    /* Power off from host to avoid the warning */
    dynamo_wic_iface->off(dynamo_wic_iface);
    printf("\nPower Off the Dynamo NGWIC.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dynamo_power_off
 *
 * Description: This function is a wrapper to power off Dynamo NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
dynamo_power_off (void)
{

    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Dynamo NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    return (dynamo_pwr_off());
}


/**********************************************************************
 *
 * Function: dynamo_pwr_on
 *
 * Description: This function power on Dynamo NGWIC.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dynamo_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Dynamo NGWIC.\n");

    assert(dynamo_wic_iface);
    assert(oir_if);
    dynamo_wic_iface->on(dynamo_wic_iface);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(dynamo_wic_iface, dynamo_wic_iface->slot, "WIC");

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

    printf("Waiting for Dynamo NGWIC to Power-Up.\n");
    msleep(2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    dynamo_wic_iface->uart_on(dynamo_wic_iface);    

    /* take Dynamo NGWIC out of reset */
    dynamo_wic_iface->unreset(dynamo_wic_iface);

    printf("Dynamo NGWIC is powered up.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dynamo_pwr_cycle
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
dynamo_pwr_cycle (void)
{
    int ret_val = PASSED;
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the Dynamo NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Dynamo is not Power Cycled.\n\n");
        return (PASSED);
    }

    ret_val = dynamo_pwr_off();  /* Always return PASSED */
    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
	printf(".");
	msleep(1000);
    }

    ret_val = dynamo_pwr_on();
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */

    if (ret_val == FAILED) {
	/* Setup the components and debug for cterr */
	cterr_add_component("Platform Power controller");
	cterr_add_debug("Check the platform power circuitry");
	cterr('f', 0, "Failed to Power On the Dynamo NGWIC");
	/* cterr setup will be cleared back in menu() */
    }

    return(ret_val);
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
    int offset, ret_val;

    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    ret_val = io_port_8bit_i2c_read(pca, offset, &data, TRUE);
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */

    if (ret_val == FAILED) {
        /* Setup the components and debug for cterr */
        cterr_add_component("PCA9557 on Dynamo", "LV245 on Dynamo",
                            "I2C controller at router");
        cterr_add_debug("Check I2C components on Dynamo",
                        "Check the I2C controller at the router");
        cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", offset);
        /* cterr setup will be cleared back in menu() */
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
    int offset, ret_val;

    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    ret_val = io_port_8bit_i2c_write(pca, offset, &data);
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */

    if (ret_val == FAILED) {
	/* Setup the components and debug for cterr */
	cterr_add_component("PCA9557 on Dynamo", "LV245 on Dynamo",
			    "I2C controller at router");
	cterr_add_debug("Check I2C components on Dynamo",
			"Check the I2C controller at the router");
        cterr('f', 0, "Unable to write PCA9557 register @ %#x\n", offset);
	/* cterr setup will be cleared back in menu() */
	return (FAILED);
    }
    return (PASSED);
}

static int 
set_ngwic_console ()
{
    int ret_val;
    uchar data;
    char err_msg[80];

    ret_val = io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE);
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */

    if (ret_val == FAILED) {
        sprintf(&err_msg[0], "Unable to read PCA9557 register @ 0x03");
	goto set_ngwic_console_fail;
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    ret_val = io_port_8bit_i2c_write(pca_i2c, 0x03, &data);
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */

    if (ret_val == FAILED) {
        sprintf(&err_msg[0], "Unable to write PCA9557 register @ 0x03");
	goto set_ngwic_console_fail;
    }

    ret_val = io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE);
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */
    if (ret_val == FAILED) {
        sprintf(&err_msg[0], "Unable to read PCA9557 register @ 0x01");
	goto set_ngwic_console_fail;
    }

    /* set GPIO pin 4 to 0 for NGWIC console redirect */
    data &= ~0x10;
    ret_val = io_port_8bit_i2c_write(pca_i2c, 0x01, &data);
#ifdef CTERR_SIM
    ret_val = FAILED;
#endif /* CTERR_SIM */
    if (ret_val == FAILED) {
        sprintf(&err_msg[0], "Unable to write PCA9557 register @ 0x01");
	goto set_ngwic_console_fail;
    }

    return (PASSED);

set_ngwic_console_fail:
    /* Setup the components and debug for cterr */
    cterr_add_component("PCA9557 on Dynamo", "LV245 on Dynamo",
                        "I2C controller at router");
    cterr_add_debug("Check I2C components on Dynamo",
                    "Check the I2C controller at the router");

    cterr('f', 0, &err_msg[0]);

    /* cterr setup will be cleared back in menu() */
    return(FAILED);
}

static int
dynamo_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(dynamo_wic_iface);

    set_ngwic_console();

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); 

#ifdef TACHI
    diag_uart_to_nim_cnnt(dynamo_wic_iface->slot);
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2"); 
#else 
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d", 
             dynamo_wic_iface->uart_ctrl); 
#endif

    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}

static int 
dynamo_o2_shell ()
{
    int slot;

    assert(dynamo_wic_iface);
    slot = dynamo_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\nEscaping to Shell from NGWIC Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);    
}


static int 
dynamo_o2_command ()
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

    assert(dynamo_wic_iface);

#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(dynamo_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else
    // When wic card is in sm adapter card
    if (dynamo_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        ge_port = ovld_get_ge_sw_port_num(dynamo_wic_iface->slot, TGT_DEV_NGSM, 0);
    } else {
        ge_port = ovld_get_ge_sw_port_num(dynamo_wic_iface->slot, TGT_DEV_NGWIC, 0);
    }
#endif
    set_gesw_line_loopback(ge_port, 1);

    return (PASSED);
}

static int
disable_bp_ge_lpbk ()
{
    int ge_port;

    assert(dynamo_wic_iface);

#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(dynamo_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else
    // When wic card is in sm adapter card
    if (dynamo_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        ge_port = ovld_get_ge_sw_port_num(dynamo_wic_iface->slot, TGT_DEV_NGSM, 0);
    } else {
        ge_port = ovld_get_ge_sw_port_num(dynamo_wic_iface->slot, TGT_DEV_NGWIC, 0);
    }
#endif
    set_gesw_line_loopback(ge_port, 0);

    return (PASSED);
}

/******** History ********
$Log: nim_dynamo.c,v $
Revision 1.15  2021/04/15 00:52:04  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.14  2020/05/22 02:28:23  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.13  2020/01/09 01:01:52  jiajliu
Merge Curie 2RU to main trunk

Revision 1.12  2019/10/17 02:16:15  kehuang2
Collapse Tabei-L into main trunk

Revision 1.11  2018/05/22 02:31:10  alpeng
fixed compiler warning, CSCvj57934

Revision 1.10  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.9  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.8  2017/03/30 08:21:06  hondwang
Tachi-L brach merge

Revision 1.7  2017/03/16 03:16:37  haohsu
Modiy download image name for kaziranga

Revision 1.6.42.3  2017/02/24 08:38:03  haohsu
Modify NIM Dynamo for TACHI

Revision 1.6.42.2  2017/02/20 08:21:19  haohsu
modify dynamo.c

Revision 1.6.42.1  2017/01/25 07:56:15  haohsu
Add dynamo NIM to Tachi-l

Revision 1.6.30.4  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.6.30.3  2017/04/05 06:40:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.6.30.2  2016/12/09 08:58:16  alpeng
fix dyname uart test for io interface test

Revision 1.6.30.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.9  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.8  2017/03/30 08:21:06  hondwang
Tachi-L brach merge

Revision 1.7  2017/03/16 03:16:37  haohsu
Modiy download image name for kaziranga

Revision 1.6.42.3  2017/02/24 08:38:03  haohsu
Modify NIM Dynamo for TACHI

Revision 1.6.42.2  2017/02/20 08:21:19  haohsu
modify dynamo.c

Revision 1.6.42.1  2017/01/25 07:56:15  haohsu
Add dynamo NIM to Tachi-l

Revision 1.6  2014/10/13 06:31:24  bowang3
Make uart test pass when NIM card is in NGSM carrier card

Revision 1.5  2014/09/30 20:38:19  siyen
Replaced the UART test with Patriot's method (per Huan's change).

Revision 1.4  2014/08/26 08:48:42  bowang3
Make NIM support NGSM carrier card Thule

Revision 1.3  2014/06/17 17:05:27  siyen
Configure Dynamo PCA9557 GPIO pins properly (CSCup41006)

Revision 1.2  2014/03/26 19:29:18  siyen
Initial check in for Dynamo platform supports (CSCun82755)

$Endlog$
*/
