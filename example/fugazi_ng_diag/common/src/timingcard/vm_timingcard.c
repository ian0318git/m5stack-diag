/* $Id: vm_timingcard.c,v 1.3 2015/02/18 06:08:26 bowang3 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard.c,v $
 *******************************************************************************
 * File Name: vm_timingcard.c
 *
 * Description: Timing Card NGVM main source file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
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
#include "menu.h"
#include "router_if.h"
#include "platform_slot.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "plat_defs.h"
#include "linux_ntwk.h"
#include "vm_timingcard_zl3036x_lib.h"

#include "pca.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "vm_timingcard.h"
#include "vm_timingcard_cpld_diag.h"
#include "vm_timingcard_zl3036x_diag.h"
#include "vm_timingcard_cpld_lib.h"
#include "vm_timingcard_pca9557_diag.h"
#include "dash_fpga.h"

/**************************************
 *  Functions Declaration
 **************************************/
n2g_i2c_if_t *get_timingcard_i2c_device(void);

/**************************************
 *  Static Function Declaration
 **************************************/
static int timingcard_reset_en(void);
static int timingcard_unreset_en(void);
static int timingcard_get_pid(char *, void *);

/**************************************
 *  Global Variables
 **************************************
 */
static struct ngio_intf_t *timingcard_vm_iface;
timingcard_ds_t timingcard_iface[MAX_SM + 1];
static timingcard_ds_t *timingcard_iface_p;
static void (*timingcard_saved_diag_exec)(void) = NULL;

static int sku_id = 0;
int timingcard_test_slot = 0;

/**************************************
 * Extern function prototypes 
 **************************************/
extern int do_all_menu_items(struct menuinfo *);

/***************************************
 * Function prototype 
 ***************************************/
int timingcard_vm_test(void *);
int timingcard_init_seq(void);
int get_timingcard_sku_id(void);
void clear_timingcard_init_flag(void);
void set_timingcard_i2c_addr (void);

/***************************************
 * Timing Card NGVM main menu on Overlord platform
 ***************************************/
static submenu_xtable_t timingcard_submenu_tbl[] = {
    {"NGVM reset", (PFT)timingcard_reset_en,  0,0,(type_t(*)())0,
      0, (type_t(*)())0},
    {"NGVM unreset", (PFT)timingcard_unreset_en,0,0,(type_t(*)())0,
      0, (type_t(*)())0},
    {"CPLD Tests", build_timingcard_cpld_menu, 0, MF_CONTINUOUS | MF_DOALL,
      (long(*)())0, 0, build_timingcard_cpld_menu, TRUE},
    {"ZL3036X Tests", build_timingcard_3036x_menu, 0, MF_CONTINUOUS | MF_DOALL,
      (long(*)())0, 0, build_timingcard_3036x_menu, TRUE},
    {"PCA9557 Tests", build_timingcard_pca9557_menu, 0, MF_CONTINUOUS | MF_DOALL,
      (long(*)())0, 0, build_timingcard_pca9557_menu, TRUE},
};

#define TIMINGCARD_SUBMENU_TABLE_SZ \
                (sizeof(timingcard_submenu_tbl)/sizeof(submenu_xtable_t))

/***************************************
 * Primary & secondary submenu items (filled in from xtable)
 ***************************************/
static mitem_t timingcard_primary_items[TIMINGCARD_SUBMENU_TABLE_SZ +
                                   MAX_BASE_ITEMS];
static mitem_t timingcard_secondary_items[TIMINGCARD_SUBMENU_TABLE_SZ +
                                     MAX_BASE_ITEMS];

static menuinfo_t timingcard_menu = {
    "Timing Card Main Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    timingcard_primary_items,
};
static menuinfo_t *timingcard_menup = &timingcard_menu;

static n2g_i2c_if_t zl3036x =
{
    .dev_name = "Timing Card ZL3036X",
    .offset = 0,
    .i2c_bus_type = IOFPGA_I2C,
    .size = 0x1,
    .mux = I2C_MUX_ZERO,
    .buf = NULL,
    /* Dash FPGA I2C Controller 15 is used for NGVM, refer to O2 HFS. */
    .i2c_ctrl = I2C_CTRL_FIFTEEN,
    /* Timing Card ZL3036X I2C 7-bits address */
    .i2c_dev = TIMING_CARD_ZL3036X_I2C_ADDR,
};

static boolean init_3036x;

/**********************************************************************
 *
 * Function: timingcard_vm_test
 *
 * This function is the main entrance for Timing Card NGVM testing.
 *
 * Input : vm - pointer to ngvm ngio interface
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int timingcard_vm_test (void *ngvm)
{
    int real_slot;
    int ret_val;
    char timingcard_pid[30];
    ushort board_id = 0;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n%s: ngvm=%#lx ", __FUNCTION__, (long)ngvm);
    }

    timingcard_vm_iface = (struct ngio_intf_t *)ngvm;

    assert(ngvm);

    /* Get PID */
    timingcard_get_pid(timingcard_pid, timingcard_vm_iface);

    real_slot = timingcard_vm_iface->slot;
    board_id = timingcard_vm_iface->id;

    timingcard_test_slot = timingcard_vm_iface->slot;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf ("slot=%d board_id=%#x ", real_slot, board_id);
    }

    testname(" (NGVM Slot %d Timing Card)", real_slot);

    /* Initialize the ZL3036X interface flag to false. */
    init_3036x = FALSE;

    /*
     * Initialize an instance of Timing Card data structure
     */
    timingcard_iface_p = (timingcard_ds_t *) &timingcard_iface[real_slot];
    timingcard_iface_p->board_id = board_id;
    timingcard_iface_p->slot = real_slot;
    timingcard_iface_p->timingcard_vm_iface = (struct ngio_intf_t *)ngvm;

    /* 30361 I2C slave address is 0x58 */
    if (get_timingcard_sku_id() == SKU_30361) {
        zl3036x.i2c_dev = TIMING_CARD_ZL30361_I2C_ADDR;
    }

    /* Timing Card menu's title need a fixed name via cookie_id */
    build_primary_submenu(timingcard_submenu_tbl, TIMINGCARD_SUBMENU_TABLE_SZ,
                          "Timing Card", &timingcard_menup);
    build_secondary_submenu(timingcard_submenu_tbl,
                            TIMINGCARD_SUBMENU_TABLE_SZ, timingcard_secondary_items);

    /*
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    timingcard_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    ret_val = PASSED;
    if (timingcard_vm_iface->menu_display == TRUE) {
        menu(timingcard_menup, timingcard_secondary_items, '\0');
    } else {
        if (timingcard_vm_iface->test_type == IFACE_TEST) {
        } else {  /* FULL_TEST */
            do_all_menu_items(timingcard_menup);
        }
    }

    if (timingcard_saved_diag_exec) {
        pre_diag_exec = timingcard_saved_diag_exec;
        timingcard_saved_diag_exec = NULL;
    }

    sku_id = 0;

    return (ret_val);
}

/**********************************************************************
 *
 * Function: timingcard_reset_en
 *
 * This function reset the TimingCard NGVM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *
 **********************************************************************
 */
static int timingcard_reset_en (void)
{
    int ret;

    assert(timingcard_vm_iface);

    printf("\nTimingCard NGVM Enabling Reset\n ");
    /* Pull reset enable */
    ret = timingcard_vm_iface->reset(timingcard_vm_iface);
    msleep(50);

    return (ret);

}

/**********************************************************************
 *
 * Function: timingcard_unreset_en
 *
 * This function unresets the TimingCard NGVM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *
 **********************************************************************
 */
static int timingcard_unreset_en (void)
{
    int ret;

    assert(timingcard_vm_iface);

    printf("\nTimingCard NGVM unreset");
    /* release the reset*/
    ret = timingcard_vm_iface->unreset(timingcard_vm_iface);
    msleep(50);

    return (ret);

}

/**********************************************************************
 *
 * Function: timingcard_init_seq
 *
 * This function initializes the TimingCard NGVM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *
 **********************************************************************
 */
int timingcard_init_seq (void)
{
    /* If the initialize ZL3036X sequence is not performed before,
     * perform the initialization. */
    if (init_3036x == FALSE) {
        if (!timingcard_vm_iface) {
            timingcard_vm_iface = slot_get_ngiovm(FIRST_SLOT);
        }
        
        /* release the reset*/
        if (timingcard_vm_iface->unreset(timingcard_vm_iface) == FAILED) {
            cterr('f', 0, "Unable to un-reset the timing card.");
            return (FAILED);
        }
      
        msleep(200);

        /* Do the timingcard initialization sequence. */
        if (init_timingcard() == FAILED) {
            return (FAILED);
        }
    }

    init_3036x = TRUE;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: clear_timingcard_init_flag
 *
 * This function clear the init zl3036X flag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void clear_timingcard_init_flag (void)
{
    init_3036x = FALSE;
}

/*
 **********************************************************************
 *
 *  Function: timingcard_get_pid
 *
 *  Description: This function returns the PID string
 *
 *  Input: char *
 *
 *  Returns: TRUE/FALSE
 *
 **********************************************************************
 */
static int timingcard_get_pid (char *pid, void *timingcard_ngvm)
{
    uchar i, num_byte, *data_ptr;
    char *cookie_pid = "30361TCXO";
    boolean flag = FALSE;
    struct ngio_intf_t *timingcard_ngvm_ptr;

    timingcard_ngvm_ptr= (struct ngio_intf_t *) timingcard_ngvm;

    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
        (timingcard_ngvm_ptr->cookie, (uchar) PRODUCT_ID,
        &num_byte, FALSE)) == (uchar *) NULL) {
            /*Search CONTROLLER_TYPE failed. */
            pid[0] = 0;                /* illegal code */
            return (FAILED);
    } else {
            for (i = 0; i < num_byte; i++) {
                pid[i] = *data_ptr++;
            }
    }

    for (i = 0; i < num_byte; i++) {
        if (pid[i] == cookie_pid[i]) {
            flag = TRUE;
            continue;
        } else {
            flag = FALSE;
            break;
        }
    }

    if (flag == TRUE) {
        /* 30361 SKU */
        sku_id = SKU_30361;
    } else {
        /* 30363 SKU */
        sku_id = SKU_30363;
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: get_timingcard_sku_id
 *
 *  Description: This function returns the PID SKU
 *
 *  Input: None
 *
 *  Returns: sku_id
 *
 **********************************************************************
 */
int get_timingcard_sku_id (void)
{
    struct ngio_intf_t *vm_ptr;
    char timingcard_pid[30];

    vm_ptr = slot_get_ngiovm(FIRST_SLOT);
    if (sku_id == 0) {
        timingcard_get_pid(timingcard_pid, vm_ptr);
    }
    /* Return the SKU id */
    return (sku_id);
}

/*
 **********************************************************************
 *
 *  Function: get_timingcard_i2c_device
 *
 *  Description: This function returns the I2C device pointer
 *
 *  Input: None
 *
 *  Returns: i2c device pointer
 *
 **********************************************************************
 */
n2g_i2c_if_t *get_timingcard_i2c_device (void)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&zl3036x;

    return (i2c_p);
}

/*
 **********************************************************************
 *
 *  Function: set_timingcard_zl30361_i2c_addr
 *
 *  Description: This function returns the I2C addrss of Timing card zl30361
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
void set_timingcard_i2c_addr (void)
{
    /* 30361 I2C slave address is 0x58 */
    if (get_timingcard_sku_id() == SKU_30361) {
        zl3036x.i2c_dev = TIMING_CARD_ZL30361_I2C_ADDR;
    }
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard.c,v $
 * Revision 1.3  2015/02/18 06:08:26  bowang3
 * Support Wallander NIM 1588 test with timing card
 *
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.6  2014/04/30 13:47:20  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.1.2.5  2014/04/22 06:06:02  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.4  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.3  2014/01/14 01:27:51  kodko
 * Add the ngvm unreset while enter the build menu function.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:05  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

