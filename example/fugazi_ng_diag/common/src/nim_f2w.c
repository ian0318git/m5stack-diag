/* $Id: nim_f2w.c,v 1.6 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nim_f2w.c,v $
 *************************************************************************
 *
 * nim_f2w.c - F2W code 
 *
 * Mar 2016, Alan Peng
 *
 * Copyright (c) 2017-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * udevadm test --action=add /sys/class/net/enp12s0f* 
 * celo64e /NIC=NUM /DIAGS /EXTLB
 *
 *************************************************************************
 */
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
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
#include "pca.h"
#include "slot.h"
#include "common_utils.h"
#include "diag_console_util.h"
#include "diag_nc_common.h"
#include "nim_f2w.h"
#include "platform_i2c.h"
#include "linux_api.h"
#include "diag_i2c_api.h"
#include "queryflags.h"
#include "plat_defs.h" /* for hw shawn quick entry */
#include "diag_console_util.h" /* uart marco */

extern int do_all_menu_items(struct menuinfo *);

/* static func */
static int f2w_uart_item(unsigned int);
static int ltc4215_register_test(void);
static int ltc4215_reg_read(void);
static int ltc4215_reg_write(void);
static int pca9555_reg_read(void);
static int pca9555_reg_write(void);
static unsigned int mcu_reg_read(int, uint16_t *);
static int mcu_reg_write(int, uint16_t);
static int mcu_reg_read_wr(void);
static int mcu_reg_write_wr(void);
static int f2w_host_shell(void);
static int f2w_host_cmd(void);
static int f2w_utils(void);
static int gpio_exp_test(void);
static int f2w_i350_test(void);
static int f2w_i350_relay_test(void);
static int f2w_i350_i2c_test(void);
static int f2w_i350_led_test(unsigned int);
static void *f2w_feed_watchdog(void);
static int f2w_iface_test(void); 
static void f2w_cleanup(void);
static void relay_ctrl_util(void);
static void relay_ctrl_wr(unsigned int, unsigned int); /* port, state */
static void request_i2c_bus(unsigned int);
static void f2w_i350_intf_up(void);
static void f2w_mcu_test(void);
static n2g_i2c_if_t *pca_i2c;
static int is_item_available(int);

static n2g_i2c_if_t f2w_i2c =
{
    .dev_name = "F2W_MCU",
    .offset = 0,
    .i2c_bus_type = IOFPGA_I2C,
    .size = sizeof(uint16_t),
    .mux = I2C_MUX_ZERO,
    .sub_addr_len = 0,
    .buf = NULL,
//    .i2c_dev = NIM_F2W_I2C_ADDR_MCU, 
};

static n2g_i2c_if_t nim_f2w_i2c_if;
static struct ngio_intf_t *f2w_iface;
static unsigned int feed_watchdog = FALSE; 
static unsigned int f2w_i2c_occupied = FALSE;

/*=========================================
 * Menu struct
 *=========================================
 */
static void (*f2w_saved_diag_exec)(void) = NULL;

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t f2w_utils_submenu_table[] = {
    {"Console switch to NIM",         (PFT)f2w_uart_item,     FALSE,   0, 
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Read",         (PFT)ltc4215_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Write",        (PFT)ltc4215_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9555 Register Read",         (PFT)pca9555_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9555 Register Write",        (PFT)pca9555_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"MCU Register Read",             (PFT)mcu_reg_read_wr,       0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"MCU Register Write",            (PFT)mcu_reg_write_wr,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Relay Ctrl Utility",            (PFT)relay_ctrl_util,       0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Bring i350 eth intf",           (PFT)f2w_i350_intf_up,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Escape to Shell (debugging only)",        (PFT)f2w_host_shell, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)f2w_host_cmd,   0,   0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},   
    {"Exercise MCU Relay (debugging only)",     (PFT)f2w_mcu_test, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"I350 All LED OFF",    (PFT)f2w_i350_led_test,     F2W_LED_ALL_OFF,   0,
      (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"I350 LED Flash(10M)",    (PFT)f2w_i350_led_test,  F2W_LED_10M_ON,   0,
      (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"I350 LED flash(100M)",    (PFT)f2w_i350_led_test, F2W_LED_100M_ON,   0,
      (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"I350 All LED ON(1G)",    (PFT)f2w_i350_led_test,  F2W_LED_ALL_ON,   0,
      (type_t(*)())0, 0,	   (type_t(*)())0,          0},
};

#define F2W_UTILS_SUBMENU_TABLE_SZ (sizeof(f2w_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t f2w_utils_primary_items[F2W_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t f2w_utils_secondary_items[F2W_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

char f2wutiltitle[50];
menuinfo_t f2w_util_submenu = {
    f2wutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    f2w_utils_primary_items,
};

menuinfo_t *f2w_util_submenup = &f2w_util_submenu;

#define MM_1    (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT)
#define MM_2    (MM_1 | MF_DOALL)


/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t f2w_mainmenu_tbl[] = {
    {"F2W Interface Utilities",            (PFT)f2w_utils,             0,  0,
     (type_t(*)())0, 0, (type_t(*)())f2w_utils, 0},
    {"GPIO Expander (PCA9555) test",       (PFT)gpio_exp_test,         0,  MM_2,
     (type_t(*)())0, 0, (type_t(*)())0,         0},
    {"LTC4215 test",                       (PFT)ltc4215_register_test, 0,  MM_2,
     (type_t(*)())0, 0, (type_t(*)())0,         0},
    {"UART test",                          (PFT)f2w_uart_item,      TRUE,  MM_2,
     (type_t(*)())is_item_available, FALSE, (type_t(*)())0,         0},
    {"I350 test",                          (PFT)f2w_i350_test,         0,  MM_2,
     (type_t(*)())0, 0, (type_t(*)())0,         0},
    {"I350 relay lpbk test",               (PFT)f2w_i350_relay_test,   0,  MM_1,
     (type_t(*)())0, 0, (type_t(*)())0,         0},
    {"I350 I2C bus test",               (PFT)f2w_i350_i2c_test,   0,  MM_2,
     (type_t(*)())0, 0, (type_t(*)())0,         0},
    {"I350 led test",               (PFT)f2w_i350_led_test,   F2W_LED_TEST,  MM_1,
      (type_t(*)())0, 0, (type_t(*)())0,         0},
};

#define F2W_MAINMENU_TBL_SIZE (sizeof(f2w_mainmenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[F2W_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[F2W_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
 
static char nimsubmenutitle[40];

static struct menuinfo maindiag = {
    nimsubmenutitle,	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;
  
/*
 * Function:    is_item_available
 * Description: This function check the item is available for NIM
 * Inputs:      item - item for check
 * Output:      TRUE/FALSE
 */
static int
is_item_available (int item)
{
    if (item) {
        return (TRUE);
    }

    return (FALSE);
}

/*
 **********************************************************************
 *
 *  Function: f2w_utils
 *
 *  Description: F2W Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int f2w_utils (void)
{

    assert(f2w_iface);
    
    sprintf(f2wutiltitle, "F2W NIM%d Host Interface Utilities Menu", 
            f2w_iface->slot);
    build_primary_submenu(f2w_utils_submenu_table,
                          F2W_UTILS_SUBMENU_TABLE_SZ,
                          f2wutiltitle, &f2w_util_submenup);

    build_secondary_submenu(f2w_utils_submenu_table,
                            F2W_UTILS_SUBMENU_TABLE_SZ,
                            f2w_utils_secondary_items);

    menu(f2w_util_submenup, f2w_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: f2w_nim_test
 *
 * Description: This function is the entry point for F2W NGWIC test .
 *
 * Input:  nim - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
int f2w_nim_test (void *nim)
{ 
    struct ngio_intf_t *iface;
    int slot;
    int ret_val = PASSED;
    ushort board_id = 0;
    unsigned char data; 
    unsigned int ia, wait_time = 50; 
    pthread_t threads;

    assert(nim);

    iface = (struct ngio_intf_t *)nim;
    f2w_iface = iface;

    slot = iface->slot;
    board_id = iface->id;

    /* enable uart */
    iface->uart_on(nim);    

    pca_i2c = iface->pca; 
    pca_i2c->i2c_dev = NIM_F2W_I2C_ADDR_GPIO; 

    /* init i2c interface */
    memcpy(&nim_f2w_i2c_if, &f2w_i2c, sizeof(n2g_i2c_if_t)); 
    nim_f2w_i2c_if.i2c_ctrl = f2w_iface->i2c_ctrl;

    /* light on AMBER */
    if (util_oir_ltc4215_led(f2w_iface->oir, OIR_LED_AMBER_ONLY)) {
        printf("LTC4215: Cannot light on ABMER \n");
    }

    printf("\nF2W test, board_id %#x, NIM slot %d\n", board_id, slot);
    sprintf(nimsubmenutitle, "F2W NIM%d Host Main Menu", slot);
    testname("Slot%d F2W NIM", slot);

    /* feed watchdog */
    feed_watchdog = TRUE; 
    ret_val = pthread_create(&threads, NULL, (void *)f2w_feed_watchdog, NULL);
    if (ret_val != 0) {
        cterr('f', 0, "Cannot create thread for watchdog"); 
        return (FAILED);
    }

    for (ia = 0; ia < wait_time; ia++) {
        /* check primary interface ready */ 
        if (io_port_8bit_i2c_read(pca_i2c, INPUT_PORT0_REG, &data, TRUE) == FAILED) {
            cterr('f', 0, "%s(): Unable to read PCA9555 Input P0 register \n",
                  __FUNCTION__);
            return (FAILED);
        }
   
        if (data & REG_BIT(3))  {
            break; 
        }

        msleep(200); 
    }

    if (ia == wait_time) { 
        printf("Primary interface is not ready on bit5 of PCA9555 Input p0=0x%x\n", data); 
    } else {
        printf("Primary interface Ready\n");
        msleep(msleep_delay);
if (skip_init_seq == TRUE)  {

} else {
        iface->pci_rdy(nim, 1);  /* skip init */
}
    }

    /* light on GREEN */
    if (util_oir_ltc4215_led(f2w_iface->oir, OIR_LED_GREEN_ONLY)) {
        printf("LTC4215: Cannot light on GREEN ");
    }

    if (iface->test_type == IFACE_TEST) {
        ret_val = f2w_iface_test();
        feed_watchdog = FALSE; 
        pthread_join(threads, NULL); 
        return (ret_val); 
    }

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    f2w_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(f2w_mainmenu_tbl, F2W_MAINMENU_TBL_SIZE, "F2W Menu",
			  &maindiagp);
    build_secondary_submenu(f2w_mainmenu_tbl, F2W_MAINMENU_TBL_SIZE,
			    main_menu_secondary_items);

    if (iface->menu_display) {
        menu(maindiagp, main_menu_secondary_items, '\0');
    } else {
        do_all_menu_items(maindiagp);
    }

    /* close thread and cleanup */
    feed_watchdog = FALSE; 
    pthread_join(threads, NULL); 
    f2w_cleanup();

    if (ret_val != PASSED) { 
        /* light off  */
        if (util_oir_ltc4215_led(f2w_iface->oir, OIR_LED_OFF)) {
            cterr('f', 0, "Cannot light off LED ");
     //keep going ..       return (FAILED);  
        }
    }

    return (ret_val);
}

/*
 *************************************************************************
 * Function: f2w_feed_watchdog
 *
 * clear bit0 on mcu to keep dog alive
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static void *f2w_feed_watchdog (void)
{
    uint16_t data; 

    sleep(5);   /* wait for mcu working normal */
    while (feed_watchdog) {
        request_i2c_bus(TRUE); 

        mcu_reg_read(0, &data); 
        data &= ~(BIT0); 
        mcu_reg_write(0, data); 
 
        request_i2c_bus(FALSE);
        /* minimal threshold is 1 sec, using 0.1 sec for make up
           penalty time on request i2c bus */
        msleep(NIM_F2W_WATCHDOG_REQ_I2C); 
    }

    pthread_exit(NULL);
}

/*
 *************************************************************************
 * Function: f2w_i350_test
 *
 * Test i350 on F2W
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static int f2w_i350_test (void)
{
    char intf_name[16]; 
    unsigned int ia; 
    int rc;

    testname("F2W i350");
    /* F2W_ETH_INTF = f2w_eth, F2W_ETH_MAX_PORT = 4 */
    for (ia = 0; ia < F2W_ETH_MAX_PORT; ia++) { 
        sprintf(intf_name, "%s%d", F2W_ETH_INTF, ia);
        
        prpass(testpass, "f2w ethernet port%d ", ia);
        rc = diag_nc_f2w_i350_test(intf_name); 
        if (rc == FAILED) {
            cterr('f', 0, "F2W i350 loopback test failed on port%d", ia);
            return (rc);
        }
    }

    return (PASSED);
}

/*
 *************************************************************************
 * Function: f2w_i350_relay_test
 *
 * Test i350 with relay bypass on F2W
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static int f2w_i350_relay_test (void) 
{
    int rc = FAILED; 
    char intf_name[16]; 

    testname("F2W i350 relay ");
    /* testing relay 32 bypass, tx:p1 rx:p0 */
    prpass(testpass, "F2W Relay32 Bypass loopback ");

    sprintf(intf_name, "%d", RELAY32); 
    relay_ctrl_wr(RELAY32, RELAY_BYPASS); 
    rc = diag_nc_f2w_i350_relay_test(intf_name);
    if (rc == FAILED) {
        cterr('f',0,"F2W Relay32 loopback test failed");
        return (rc);
    }
    /* recovery */
    relay_ctrl_wr(RELAY32, RELAY_NORMAL); 

    /* testing relay 10 bypass, tx:p3 rx:p2 */
    prpass(testpass, "F2W Relay10 Bypass loopback ");

    sprintf(intf_name, "%d", RELAY10); 
    relay_ctrl_wr(RELAY10, RELAY_BYPASS); 
    rc = diag_nc_f2w_i350_relay_test(intf_name);

    if (rc == FAILED) {
        cterr('f',0,"F2W Relay10 loopback test failed");
        return (rc);
    }

    /* recovery */
    relay_ctrl_wr(RELAY10, RELAY_NORMAL); 

    return (PASSED);
}

/*
 *************************************************************************
 * Function: f2w_i350_led_test
 *
 * Test/Set F2W LED
 *
 * 10M will flash one time
 * 100M will flash two time
 * 1000M will flash three time
 *
 * LED testing will run OFF/10M/100M/100M each take 5 second
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static int f2w_i350_led_test (unsigned int test)
{
    char intf_name[16];
    int rc;

    testname("F2W i350 LED Test");
    sprintf(intf_name, "%d", test);
    prpass(testpass, "f2w LED Test/Set");

    rc = diag_nc_f2w_i350_led_test(intf_name);
    if (rc == FAILED) {
        cterr('f', 0, "F2W i350 LED test failed ");
        return (rc);
    }

    return (PASSED);
}
/*
 *************************************************************************
 * Function: f2w_uart_test
 *
 * Test the UART connection from the host to F2W.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static int f2w_uart_item (unsigned int test)
{
    const int maxlen = 28;
    char test_if[maxlen], cmd[64];
    int rv, slot;

    /* 'n\n' for trigger F2W side diag sub-item,
     * which will invoke 'uname -a'.
     */
    assert(f2w_iface);
    slot = f2w_iface->slot;

#ifdef TACHI
    diag_uart_to_nim_cnnt(slot); 
    snprintf(test_if, maxlen-1, UART_TTYS2_DEV);
#else 
    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d", f2w_iface->uart_ctrl);
#endif

    if (test == TRUE) {
        testname("F2W UART");
        prpass(testpass, "Read string alive test");

        /* pre HW req, we need to input '3' for 'alive' msg, 
         * after that, we need to input '\n' for disable 'alive' msg */
        rv = uart_msg_exh_test(test_if, "3", "alive", DEFAULT_CASE); 
        if (rv == FAILED) {
            cterr('f', 0, "F2W UART test failed");
            return (FAILED);
        }

        /* for disable, no need to check string */
        uart_msg_exh_test(test_if, "\n", "", DEFAULT_CASE); 
    } else {
        printf("Press Ctrl-AX for exit and back to menu \n");
        snprintf(cmd, PICOCOM_CMD_LENGTH - 1, "picocom %s -d8 -pn -fn %s",
             BAUD9600, test_if);
        printf("cmd=%s\n", cmd);
        system(cmd);
    }

    return (rv);
}

/*
 **********************************************************************
 * Function: gpio_exp_test
 *  
 * Description: excerise  configure register 1 
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 * Note: 10ms delay to avoid exercising register too faster with 'C' flag
 **********************************************************************
 */ 
static int gpio_exp_test (void)
{
    n2g_i2c_if_t  *pca = pca_i2c;     
    uchar data = 0;

    testname("F2W PCA9555");
    prpass(testpass, "PCA9555 GPIO Expander Register test");
    /*
     * 1. Set polarity reg to 0x0
     * 2. Set the config reg to select the output pins
     * 3. read/write to data reg and check the value
     */
    prpass(testpass,"GPIO Expander - ");

    request_i2c_bus(TRUE); 
    data = 0x3; 
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_P1_REG, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9555 Config P1 register \n",
              __FUNCTION__);
        request_i2c_bus(FALSE); 
        return (FAILED);
    }
    msleep(10);
    
    data = 0x14; 
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT1_REG, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9555 Output 1 register \n",
              __FUNCTION__);
        request_i2c_bus(FALSE); 
        return (FAILED);
    }
    msleep(10);

    /* verify */
    if (io_port_8bit_i2c_read(pca, INPUT_PORT0_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "%s(): Unable to read PCA9555 Polarity register \n",
              __FUNCTION__);
        request_i2c_bus(FALSE); 
        return (FAILED);
    }
    msleep(10);

    if (data != NIM_F2W_GPIO_PORT0_REG) {
        cterr('f', 0, "%s(): PCA9555 Verifying failed INPUT P0 reg 0x%x != 0xb\n",
                             __FUNCTION__, data);
        request_i2c_bus(FALSE); 
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca, INPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "%s(): Unable to read PCA9555 Polarity register \n",
              __FUNCTION__);
        request_i2c_bus(FALSE); 
        return (FAILED);
    }
    msleep(10);

    if (data != NIM_F2W_GPIO_PORT1_REG) {
        cterr('f', 0, "%s(): PCA9555 Verifying failed INPUT P1 reg 0x%x != 0x14\n",
              __FUNCTION__, data);
        request_i2c_bus(FALSE); 
        return (FAILED);
    }
    request_i2c_bus(FALSE); 

    return (PASSED);
}   


/*
 **********************************************************************
 * Function: f2w_i350_i2c_test
 *
 * Description: Check F2W I350 I2C bus ACK
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 * Note: 10ms delay to avoid exercising register too faster with 'C' flag
 **********************************************************************
 */
static int f2w_i350_i2c_test (void)
{
    n2g_i2c_if_t i2c_if;
    uchar d32[80];
    unsigned int rc;

    testname("F2W I350");
    prpass(testpass, "F2W I350 I2C Bus ACK test");

    request_i2c_bus(TRUE);
    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = F2W_I350_I2C_BUS_TYPE;
    i2c_if.i2c_ctrl = F2W_I350_I2C_CTRL;
    i2c_if.mux = F2W_I350_I2C_MUX;
    i2c_if.i2c_dev = NIM_F2W_I2C_ADDR_I350;
    i2c_if.offset = F2W_I350_I2C_OFFSET;
    i2c_if.size = F2W_I350_I2C_SIZE;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to read i2c\n");
        request_i2c_bus(FALSE);
        return FAILED;
    }

    request_i2c_bus(FALSE);

    return (PASSED);
}

/*
 *************************************************************************
 * Function: f2w_iface_test
 *
 * Test entry for F2W interface test.
 *      covered: I2C, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int f2w_iface_test (void)
{
    assert(f2w_iface);
    if (gpio_exp_test()) {
        return (FAILED);
    }

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
	return (FAILED);
    }

    /* Testing UART and GE0 interfaces */
    if (f2w_uart_item(TRUE)) {
	return (FAILED);
    }

    if (f2w_i350_test()) {
	return (FAILED);
    }

    if (f2w_i350_i2c_test()) {
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/*
 **********************************************************************
 * Function: f2w_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void f2w_cleanup (void)
{
    assert(f2w_iface);

    if (f2w_saved_diag_exec) {
        pre_diag_exec = f2w_saved_diag_exec;
        f2w_saved_diag_exec = NULL;
    }
}

/*
 **********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 * Note: 10ms delay to avoid exercising register too faster with 'C' flag
 **********************************************************************
 */
static int ltc4215_register_test (void)
{
    int ret;

    testname("F2W LTC4215");
    prpass(testpass, "LTC4215 OIR Register test ");

    request_i2c_bus(TRUE); 
    ret = oir_ltc4215_register_test(f2w_iface->oir);
    if (ret == FAILED) {
	cterr('f',0,"LTC4215 register test failed.");
    }
    request_i2c_bus(FALSE); 

    return (ret);
}

/*
 **********************************************************************
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
static int ltc4215_reg_write (void)
{
    void *oir_if;

    oir_if = (void *)(f2w_iface->oir);
    assert(oir_if);
    return(util_oir_ltc4215_reg_write(oir_if));
}

/*
 **********************************************************************
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
static int ltc4215_reg_read (void)
{
    void *oir_if;

    oir_if = (void *)(f2w_iface->oir);
    assert(oir_if);
    return(util_oir_ltc4215_reg_read(oir_if));
}

/*
 **********************************************************************
 *
 * Function: pca9555_reg_read
 *
 * Description: PCA9555 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9555_reg_read (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 7);

    if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ %#x\n", offset);
	return (FAILED);
    }
    printf("\nRegister @ %#x = %#x\n", offset, data);
    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: pca9555_reg_write
 *
 * Description: PCA9555 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9555_reg_write (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    /* offset 0 cannot be write */
    offset = gethex_answer("Reg offset to write: ", 0, 0, 7);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ %#x\n", offset);
	return (FAILED);
    }
    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: mcu_reg_read
 *
 * Description: MCU Register Read utility.
 *
 * Input : offset - reg offset, data - data pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static unsigned int mcu_reg_read (int offset, uint16_t *data)
{
    int result = FAILED;

    nim_f2w_i2c_if.i2c_dev = NIM_F2W_I2C_ADDR_MCU; 
    nim_f2w_i2c_if.buf = (char *)data;
    nim_f2w_i2c_if.offset = offset;
    nim_f2w_i2c_if.size = sizeof(uint16_t);

    result = n2g_i2c_read(&nim_f2w_i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to read data */
        cterr('f', 0, "%s: Unable to read. rc = 0x%08x", __FUNCTION__, result);
        return (FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: mcu_reg_write
 *
 * Description: MCU Register Write utility.
 *
 * Input : offset - reg offset, data - data for write 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int mcu_reg_write (int offset, uint16_t data)
{
    int result = FAILED;

    nim_f2w_i2c_if.i2c_dev = NIM_F2W_I2C_ADDR_MCU;
    nim_f2w_i2c_if.buf = (char *)&data;
    nim_f2w_i2c_if.offset = offset;
    nim_f2w_i2c_if.size = sizeof(uint16_t);

    result = n2g_i2c_write(&nim_f2w_i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to read data */
        cterr('f', 0, "%s: Unable to write. rc = 0x%08x", __FUNCTION__, result);
        return (FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: mcu_reg_read_wr
 *
 * Description: A wrapper for MCU Register Read utility.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int mcu_reg_read_wr (void)
{
    uint16_t data; 
    int offset;
    
    printf("read mcu register\n");

    offset = gethex_answer("Reg offset to read(0-3): ", 0, 0, 3);
    
    mcu_reg_read(offset, &data); 
    printf("data = 0x%x\n", data);

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: mcu_reg_write_wr
 *
 * Description: A wrapper for MCU Register Wrire utility.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int mcu_reg_write_wr (void)
{
    uint16_t data;
    int offset;

    printf("Write mcu register\n");
    
    offset = gethex_answer("Reg offset to write(0-3): ", 0, 0, 3);
    data = gethex_answer("Enter data :", 0, 0, 0xFFFF);
    
    mcu_reg_write(offset, data);

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: f2w_host_shell
 *
 * Description: Display the platform linux shell prompt
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int f2w_host_shell (void)
{
    int slot;

    assert(f2w_iface);
    slot = f2w_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\nEscaping to Shell from NGWIC Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);    
}

/*
 **********************************************************************
 *
 * Function: f2w_host_cmd
 *
 * Description: Execute linux shell command on the platform linux
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int f2w_host_cmd (void)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return(PASSED);
}

/*
 **********************************************************************
 *
 * Function: relay_ctrl_wr
 *
 * Description: wrapper for relay control 
 *
 * Input : None.
 *
 * Output: None.
 *
 * Note : following HFS for example: 

    normal mode (port32)
    byte0: XX
    byte1: X2

    normal mode (port10)
    byte0: XX
    byte1: X1

    bypass mode (port32 bypass port10 open) 
    byte0: XX
    byte1: 20

    byte0: XX
    byte1: e0

    bypass mode (port10 bypass port32 open) 
    byte0: XX
    byte1: 10
   
    byte0: XX
    byte1: d0
 **********************************************************************
 */
static void relay_ctrl_wr (unsigned int port, unsigned int state)
{
    unsigned short data, set1, set2, mask1, rslt, rslt_st1, rslt_st2, rc = PASSED; 

__mcu_retry:
    /* disable watchdog clearner first */
    request_i2c_bus(TRUE); 

    mcu_reg_read(0, &data);
printf("relay_rd_data - 0x%x\n", data);

    switch (state) {
    case RELAY_NORMAL:
       /* only need to write it once */

       if (port == RELAY32) {
           set1 = REG_BIT(9);
           mask1 = REG_BIT(9);
       } else {  /* RELAY10 */
           set1 = REG_BIT(8);
           mask1 = REG_BIT(8);
       }

       data &= ~(mask1); 
       data |= set1; 
       mcu_reg_write(0, data);
       printf("relay_wr_data - 0x%x\n", data);
    break; 
    case RELAY_BYPASS:
        /* need to write mcu twice */
       if (port == RELAY32) {
           mask1 = REG_BIT(9);
           set1 = REG_BIT(13); 
           set2 = REG_BIT(15); 
       } else { /* RELAY10 */
           mask1 = REG_BIT(8);
           set1 = REG_BIT(12); 
           set2 = REG_BIT(14); 
       }
       
       data &= ~(mask1); 
       data |= set1; 
       mcu_reg_write(0, data);
printf("op1 relay_wr_data - 0x%x\n", data);
       data |= set2; 
       mcu_reg_write(0, data);
printf("op2 relay_wr_data - 0x%x\n", data);
    break; 
    case RELAY_OPEN:
        /* need to write mcu twice */
       if (port == RELAY32) {
           mask1 = REG_BIT(9);
           set1 = REG_BIT(13);
           set2 = REG_BIT(15);
       } else { /* RELAY10 */
           mask1 = REG_BIT(8);
           set1 = REG_BIT(12);
           set2 = REG_BIT(14);
       }

       data &= ~(mask1);
       data &= ~(set1);
       mcu_reg_write(0, data);
printf("op1 relay_wr_data - 0x%x\n", data);
       data |= set2;
       mcu_reg_write(0, data);
printf("op2 relay_wr_data - 0x%x\n", data);
    break; 
    default : 

       printf("%s:Wrong relay number port%d\n", __FUNCTION__, port);
       request_i2c_bus(FALSE);
       return;
    break; 
    }

    msleep(500);
    /* read back for debugging */
    mcu_reg_read(0, &rslt);
printf("relay_rd_data rslt - 0x%x\n", rslt);
    printf("MCU val == 0x%x\n", rslt); 


    if (rslt & REG_BIT(8)) {
        printf("Relay10 is Normal mode\n");
        rslt_st1 = RELAY_NORMAL;
    } else {
       if (rslt & REG_BIT(10)) {
           printf("Relay10 is Bypass mode\n");
           rslt_st1 = RELAY_BYPASS;
       } else {
           printf("Relay10 is Open mode\n");
           rslt_st1 = RELAY_OPEN;
       }
    }

    if (rslt & REG_BIT(9))  {
        printf("Relay32 is Normal mode\n");
        rslt_st2 = RELAY_NORMAL;
    } else {
       if (rslt & REG_BIT(11)) {
           printf("Relay32 is Bypass mode\n");
           rslt_st2 = RELAY_BYPASS;
       } else {
           printf("Relay32 is Open mode\n");
           rslt_st2 = RELAY_OPEN;
       }
    }

    /* enable watchdog */
    request_i2c_bus(FALSE); 

    if (port == RELAY32) {
        if (state != rslt_st2) {
            rc = FAILED;
        }
    } else {
        if (state != rslt_st1) {
            rc = FAILED;
        }
    }

    /* bypass MSB which is self-cleaned after setting is done */
    if (rc) {
        printf("Result not matched ..setup again\n");
        msleep(1000);  /* a delay for updating watchdog */
        rc = PASSED;
        goto __mcu_retry; 
    }


    return;
}


/*
 **********************************************************************
 *
 * Function: relay_ctrl_util
 *
 * Description: write mcu for controlling relay 
 *
 * Input : None.
 *
 * Output: None.
 *
 **********************************************************************
 */
static void relay_ctrl_util (void) 
{
    unsigned int port, state; 
    

    port = getdec_answer("Enter relay1-port(01) relay2-port(23):",
                          1, 1, 2);  

    state = getdec_answer("Enter relay 0:Normal 1:Bypass 2:Open ",
                          0, 1, 2); 

    relay_ctrl_wr(port, state);

    return; 
}

/*
 **********************************************************************
 *
 * Function: request_i2c_bus
 *
 * Description: need to shared i2c bus with feeding watchdog 
 *
 * Input : flag - TRUE for occupy; FALSE for release; 
 *
 * Output: None.
 *
 **********************************************************************
 */
static void request_i2c_bus (unsigned int flag) 
{
    unsigned int ctr = 100000; 

    if (flag) { 
        while (1) { 
            if (f2w_i2c_occupied == TRUE) { 
            /* let's wait..*/
                msleep(10); 
                ctr--; 
                continue; 
            } else {
               f2w_i2c_occupied = TRUE;  /* occupy i2c */
               return; 
            }

            if (ctr == 0) {
                 printf("Cannot get i2c bus for using\n");
                 return; 
            }
        }
    } else { 
        /* release i2c bus */
        f2w_i2c_occupied = FALSE; 
        return;
    }
}

/*
 **********************************************************************
 *
 * Function: f2w_i350_intf_up
 *
 * Description: To bring up i350 eth intf 
 *
 * Input : None
 *
 * Output: None.
 *
 **********************************************************************
 */
static void f2w_i350_intf_up (void)
{

    diag_nc_f2w_i350_intf_up(); 

    return; 
}

/*
 **********************************************************************
 *
 * Function: f2w_mcu_test
 *      
 * Description: an utility for exercise mcu ctrl relay 
 *  
 * Input : None.
 *
 * Output: None.
 *
 **********************************************************************
 */
static void f2w_mcu_test (void) 
{
    int time; 

    printf("Relay states exercise \n");
    time = getdec_answer("Delay time between each state in ms : ", 0, 0, 10000);
    while (1) { 
    printf("Exercise Relay10 Normal, Bypass, Open \n");
    relay_ctrl_wr(1, 0);
    msleep(time);
    relay_ctrl_wr(1, 1);
    msleep(time);
    relay_ctrl_wr(1, 2);
    msleep(time);
    printf("Exercise Relay32 Normal, Bypass, Open \n");
    relay_ctrl_wr(2, 0);
    msleep(time);
    relay_ctrl_wr(2, 1);
    msleep(time);
    relay_ctrl_wr(2, 2);
    msleep(time);
    }

}

/******** History ********
$Log: nim_f2w.c,v $
Revision 1.6  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.5  2018/05/09 03:53:36  hondwang
Fix F2W P2 board issue

Revision 1.4  2018/02/08 02:28:03  hondwang
F2W P1 additional HW change support

Revision 1.3  2017/12/12 03:02:11  hondwang
Modify F2W MCU utility to support offset R/W

Revision 1.2.4.4  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.2.4.3  2017/09/19 10:18:51  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.2.4.2  2017/04/05 08:54:01  leschen
Sync with <ng_diag-tag-032917>

Revision 1.2  2016/10/19 02:52:46  hondwang
Add I350 I2C and LED test

Revision 1.1  2016/06/04 09:22:20  alpeng
initial check in for f2w



$Endlog$
*/
