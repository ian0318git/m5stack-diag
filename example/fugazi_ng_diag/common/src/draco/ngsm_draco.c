/* $Id: ngsm_draco.c,v 1.6 2019/01/25 06:46:28 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/draco/ngsm_draco.c,v $
 *------------------------------------------------------------------------------
 *
 * ngsm_draco.c: This file contains functions for NGSM Draco
 *
 * May 2015 - Times Huang
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "router_if.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "ngsm_draco.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "cookie_4.h"
#include "plat_defs.h"
#include "dash_fpga.h"
#include "common_utils.h"
#include "linux_api.h"
#include "hwic_slot.h"
#include "queryflags.h"
#include "cross_platform.h" /* board_type */
#include "module_fru.h"
#include "platform_margin_utils.h"
#include "draco_rbcp_main.h"
#include "rbcp_lib.h"
#include "rbcp_platform.h"
#include "bcm_gesw_defs.h"

#define ENHANCED_ERR_MSG_EXAMPLE 1
/*******************************************************************************
 * Extern function prototypes
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int getdec_answer(char *,uint ,uint ,uint);
extern void show_margins_cterr_wrapper(void);
extern int rbcp_recv_msgs(char *, ushort, int *);
/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
static int ltc4215_register_test (void);
static int ltc4215_led_test(void);
static int oir_ltc4215_tests(int);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int pca9557_reg_dump_util(void);
static int pca9557_reg_read_util(void);
static int pca9557_reg_write_util(void);
static int draco_power_off (void);
static int draco_pwr_off (void);
static int draco_pwr_on (void);
static int draco_pwr_cycle (void);
static void draco_check_bmc_ready(void);
static void (*wait_draco_bmc_ready_func)(void) = draco_check_bmc_ready;
static int draco_utils(void);
static int draco_ioe_reg_test(void);
static int draco_o2_shell(void);
static int draco_o2_command(void);
static long util_debug_o2_power_fault(void);
static void show_oir_ltc4215_regs(void);
static long util_show_oir_ltc4215_regs(void);
static void (*draco_saved_diag_exec)(void) = NULL;
static char pca_buff0[256];
static void *oir_if;
static int draco_set_rbcp_mac_add(void);

int draco_rbcp_bmc_con_switch(void);
int draco_rbcp_intel_con_switch(void);
void set_draco_bp_loopback(int, int, int);

static void display_i2c_regs_cter(void);
static void display_i2c_regs(void);

long draco_rbcp_picocom_switch (void);
long draco_show_ioe_regs(char *);
long draco_gesw_lpbk_set(void);

static speed_t slot1_uart = DRACO_B9600;
static speed_t slot2_uart = DRACO_B9600;
static speed_t slot3_uart = DRACO_B9600;

/* addr of 8bit 0x38H >> 1; 16bit 0x48H >> 1 */
static n2g_i2c_if_t pca_i2c[] = {
    {
        .i2c_dev = 0x1C,  
    },
    {
        .i2c_dev = 0x24,
    },
};

static reg_info_t pca9557_reg_tbl[]=
{
    {"Input port",                  PCA9557_IN_PORT_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0x00, 0x00},
    {"Output port",                 PCA9557_OUT_PORT_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
    {"Polarity Inversion port",     PCA9557_POLAR_INV_P_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0x00},
    {"Configuration port",          PCA9557_CFG_PORT_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
};

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
int draco_test_slot = 1;
ushort draco_board_id;
boolean pca9557;

draco_ds_t draco_iface[MAX_SM+1];
static draco_ds_t *draco_iface_p;
extern uint bcm_op_mode;
extern uint bcm_idle_listen_params;
extern uint bcm_line_id;
extern int do_all_menu_items(struct menuinfo *);


static struct ngio_intf_t *draco_ngsm_iface;
static int debug_o2_power_fault_flag = 0;

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)
#define FLAGS   MF_CONTINUOUS

/*=========================================
 * LTC4215 OIR menu items
 *=========================================
 */
static submenu_xtable_t oir_submenu_table[] = {
    {"LTC4215 OIR reg. Read util",      (PFT)ltc4215_reg_read,       0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR reg. Write util",     (PFT)ltc4215_reg_write,      0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR Register Test",       (PFT)ltc4215_register_test,  0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR LED Test",            (PFT)ltc4215_led_test,       0,
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

/*=========================================
 * Utilities menu items
 *=========================================
 */
static submenu_xtable_t draco_utils_submenu_table[] = {
    {"Power off draco SM",      (PFT)draco_power_off,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"Power on draco SM",       (PFT)draco_pwr_on,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"Power cycle draco SM",    (PFT)draco_pwr_cycle,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Test",           (PFT)ltc4215_register_test,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Read",           (PFT)ltc4215_reg_read,          0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Write",          (PFT)ltc4215_reg_write,         0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register show",          (PFT)util_show_oir_ltc4215_regs, 0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"IO Expander(PCA9557) Reg. Dump",  (PFT)pca9557_reg_dump_util,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0}, 
    {"IO Expander(PCA9557) Reg. Read",  (PFT)pca9557_reg_read_util,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0}, 
    {"IO Expander(PCA9557) Reg. Write", (PFT)pca9557_reg_write_util,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},  
    {"Escape to Shell (debugging only)", (PFT)draco_o2_shell,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)draco_o2_command,0,0,
      (type_t(*)())0, 0,        (type_t(*)())0,          0},
    {"Set Power Fault Debug Flag", (PFT)util_debug_o2_power_fault,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Picocom Console redirect Link", (PFT)draco_rbcp_picocom_switch,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Platform GE Switch Line Loopback Setup", (PFT)draco_gesw_lpbk_set,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define DRACO_UTILS_SUBMENU_TABLE_SZ (sizeof(draco_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t draco_utils_primary_items[DRACO_UTILS_SUBMENU_TABLE_SZ + 
                                        MAX_BASE_ITEMS];
static mitem_t draco_utils_secondary_items[DRACO_UTILS_SUBMENU_TABLE_SZ + 
                                          MAX_BASE_ITEMS];

char dracoutiltitle[50];

menuinfo_t draco_util_submenu = {
    dracoutiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    draco_utils_primary_items,
};

menuinfo_t *draco_util_submenup = &draco_util_submenu;


/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Draco Utilities",      (PFT)draco_utils,        0,   0,
     (type_t(*)())0, 0,         (type_t(*)())draco_utils,    0},
    {"LTC4215 OIR Test",            (PFT)oir_ltc4215_tests,         0,  MM_3,
     (type_t(*)())0, 0,         (PFT)oir_ltc4215_tests,             TRUE},
    {"I2C IO Expander Register Test",       (PFT)draco_ioe_reg_test, 0,  MM_3,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0}, 
    {"RBCP Tests",            (PFT)build_draco_rbcp_menu, 0, 	MM_3,
     (type_t(*)())0, 0, 	(PFT)build_draco_rbcp_menu, TRUE},
    { "BMC Console redirect Test",  (PFT)draco_rbcp_bmc_con_switch, 0, 0,
     (type_t(*)())0, 0, 		(type_t(*)())0, 0 },
    { "INTEL Console redirect Test", (PFT)draco_rbcp_intel_con_switch, 0, 0,
     (type_t(*)())0, 0,		 	(type_t(*)())0, 0 },
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
        
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Draco Main Menu",   /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*******************************************************************************
 *
 * Function: slot_get_bd_pid
 *
 * Description: This function will return the Product ID(PID) of NIM module.
 *
 * Inputs : eeprom_data - pointer to eeprom data.
 *          board_pid - pointer to NIM product id.
 *
 * Returns : PID of board.
 *******************************************************************************
 */
static int slot_get_bd_pid (uchar *eeprom_data, char *board_pid, uchar *num_byte)
{
    uchar *data_ptr;

    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
        if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
            (eeprom_data, PRODUCT_ID, num_byte, FALSE)) == NULL) {
            sprintf((char *)board_pid, (char *)"NO PID");
            return (FAILED);

        } else {
            memcpy(board_pid, data_ptr, *num_byte);
        }
        return (PASSED);
    } else {
        sprintf((char *)board_pid, (char *)"NO PID");
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function: draco_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 *******************************************************************************
 */
static void draco_cleanup (void)
{
    assert(draco_ngsm_iface);

    if (draco_saved_diag_exec) {
        pre_diag_exec = draco_saved_diag_exec;
        draco_saved_diag_exec = NULL;
    }
}

/*******************************************************************************
 *
 * Function: setup_uart
 *
 * Description: Setup UART Interface Parameter
 *  
 * Input:  None
 *
 * Output: None 
 *
 *******************************************************************************
 */
static void setup_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd, slot;
    struct termios oldtio, newtio;
    int new_baud = 0;

    assert(draco_ngsm_iface);
    slot = draco_ngsm_iface->slot;

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", draco_ngsm_iface->uart_ctrl);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    /* Get current BAUD setting */
    if (slot == DRACO_NGSM_SLOT1) {
        new_baud = slot1_uart;
    } else if (slot == DRACO_NGSM_SLOT2) {
        new_baud = slot2_uart;
    } else if (slot == DRACO_NGSM_SLOT3) {
        new_baud = slot3_uart;
    } else {
        cterr('f',0,"Invalid slot number: %d.", slot);
        close(fd);
        return;
    }
    
    newtio.c_cflag = CS8 | CLOCAL | CREAD; /* control mode flags */
   
    if (new_baud == DRACO_B115200) {
        newtio.c_cflag |= B115200;
    } else if (new_baud == DRACO_B9600) {
        newtio.c_cflag |= B9600;
    } else {
        cterr('f',0,"Invalid Baud: %d.", new_baud);
        close(fd);
        return;
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
    return;
}

/**********************************************************************
 *
 *  Function: draco_gesw_lpbk_set
 *
 *  Description: This function is the wrapper to enables or disable the line
 *		 loopback of the GE switch port connected to the draco.
 *
 *  Input  : none
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
long draco_gesw_lpbk_set (void)
{
    int port, slot, enable;

    slot = draco_iface_p->slot;

    port = getdec_answer("GE Backplane port number", 0, 0, 1); 

    if (getc_answer("(e)nable or (d)isable the Platform GE switch lineloopback",
                    "ed", 'e') == 'e') {
        /* User request to enable the line loopback */
        enable = TRUE;
    } else {
        /* User request to disable the line loopback */
        enable = FALSE;
    }

    set_draco_bp_loopback(slot, port, enable);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: draco_iface_test
 *
 * Description: Test entry for draco interface test.
 *              covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int draco_iface_test ()
{
    /* Testing the I2C interface */
    if (ltc4215_register_test() == FAILED) {
        return (FAILED);
    }

    prcomplete (testpass, errcount, 0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: draco_ngsm_test().
 *
 * Description: This function is the main entrance for draco NGSM test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int draco_ngsm_test (void *ngsm)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    char board_pid[20];
    uchar num_byte;

    assert(ngsm);

    draco_ngsm_iface = (struct ngio_intf_t *)ngsm;

    slot = draco_ngsm_iface->slot;
    draco_test_slot = draco_ngsm_iface->slot;
    board_id = draco_ngsm_iface->id;
    draco_board_id=draco_ngsm_iface->id;

    slot_get_bd_pid(draco_ngsm_iface->cookie, board_pid, &num_byte);
    num_byte = 9; /* P1A2 MFG pads PID with 7 white spaces. */

    draco_ngsm_iface->uart_on(ngsm); 
    
    printf ("\n%s, board_id %#x, slot %d.\n", board_pid, board_id, slot);

    testname ("Slot%d draco NGSM ", slot);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = draco_ngsm_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT1;
    pca_i2c[0].buf   = pca_buff0;

    oir_if = (void *)(draco_ngsm_iface->oir);

    /*
     * Initialize an instance of draco data structure
     */
    draco_iface_p = (draco_ds_t *) &draco_iface[slot];
    draco_iface_p->board_id = board_id;
    draco_iface_p->slot = slot;
    draco_iface_p->uart = draco_ngsm_iface->uart_ctrl;
    draco_iface_p->draco_ngsm_iface = (struct ngio_intf_t *)ngsm;

    draco_ngsm_iface->unreset(ngsm);

    /* Set draco boot up parameters */
    setup_uart();
    
    msleep (1000);

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    draco_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    /* Setup RBCP GE switch environment and Mac address */
    if (platform_setup_rbcp_ge_env() == FAILED) {
            return (FAILED);
    }
    draco_set_rbcp_mac_add();

    /* wait for BMC ready */
    (*wait_draco_bmc_ready_func)();

    /* to help debug O2 power fault noise only */
    if (debug_o2_power_fault_flag) {
        show_oir_ltc4215_regs();
    }

    clear_draco_regis_done_flag(draco_test_slot);

    /* Enable BP loopback */
    set_draco_bp_loopback(draco_test_slot, DRACO_BP_GE1, TRUE);

    /*
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    draco_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    ret_val = PASS;
    if (draco_ngsm_iface->menu_display == TRUE) {
    	menu(maindiagp, main_menu_secondary_items, '\0');
    } else {
    	if (draco_ngsm_iface->test_type == IFACE_TEST) {
        	ret_val = draco_iface_test();
    	} else {
            menu (maindiagp, main_menu_secondary_items, '\0');
    	}
    }

    if (draco_saved_diag_exec) {
    	pre_diag_exec = draco_saved_diag_exec;
    	draco_saved_diag_exec = NULL;
    }

    if (platform_cleanup_rbcp_ge_env() == FAILED) {
        return (FAILED);
    }

    /* Disable BP loopback */
    set_draco_bp_loopback(draco_test_slot, DRACO_BP_GE1, FALSE);

    draco_cleanup ();

    return (ret_val);
}

/*******************************************************************************
 *
 *  Function: draco_utils
 *
 *  Description: Draco Utilities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int draco_utils (void)
{
    assert(draco_ngsm_iface);

    sprintf(dracoutiltitle, "Draco Slot %d Utilities Menu", 
            draco_ngsm_iface->slot);
    build_primary_submenu(draco_utils_submenu_table,
                          DRACO_UTILS_SUBMENU_TABLE_SZ,
                          dracoutiltitle, &draco_util_submenup);

    build_secondary_submenu(draco_utils_submenu_table,
                            DRACO_UTILS_SUBMENU_TABLE_SZ,
                            draco_utils_secondary_items);

    menu(draco_util_submenup, draco_utils_secondary_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : display_i2c_regs_cter
 *
 * Description: calling display_i2c_regs function
 *
 * Inputs     : None
 *
 * Outputs    : None
 *
 *******************************************************************************
 */

static void
display_i2c_regs_cter(void)
{
   display_i2c_regs();
}
/*******************************************************************************
 *
 * Function   : display_i2c_regs
 *
 * Description: Display error message
 *
 * Inputs     : None
 *
 * Outputs    : None
 *
 *******************************************************************************
 */

static void display_i2c_regs (void)
{
  draco_show_ioe_regs("\nI2C IO Expander config");
}

/*******************************************************************************
 *
 * Function   : draco_ioe_reg_test
 *
 * Description: Wrapped function to do Draco IO Expander register test.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_ioe_reg_test (void)
{
    uint32_t         ctr = 0, test_ctr = 0, total_reg_num = 0;
    uchar            orig_val = 0, test_data = 0, check_data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    #ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
    #endif
    io_exp = &pca_i2c[0];

    reg_p = &pca9557_reg_tbl[0];
    total_reg_num = (sizeof(pca9557_reg_tbl) / sizeof(reg_info_t));


    #ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    // get_mb_pid((char *)mb_get_pid);
    memcpy(ngsm_get_pid,(char*)&draco_board_id,2);
    strcpy((char *)ngsm_get_loc, "I2C IO Expander Register Test");
    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("I2C");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)display_i2c_regs_cter);


    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Go to the Draco Utilities menu",
    		 "Use the IO Expander(PCA9557) Reg. Read Utilities to check whether it can or can't dump the register value ");
   #endif

    testname("PCA9557 Register");

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        /* Skip Input port registers & Output port registers
         * Based on PCA9557 datasheet, Input port registers are input-only,
         * writes to these registers have no effect.
         * And skip Output port registers to avoid to change the system set-ups.
         * Like cause ShrinkRay alien sub-module be put in reset(GPIO[2] = 0).
         */
        if ((reg_p->offset == PCA9557_IN_PORT_REG) ||
            (reg_p->offset == PCA9557_OUT_PORT_REG)) {
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

    return (PASSED);
}


/**********************************************************************
 *
 * Function: draco_show_ioe_regs
 *
 * utility to dump i2c io expander registers
 *
 * Input : title - title to display
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long draco_show_ioe_regs (char *title)
{
    uint8_t data;
    n2g_i2c_if_t *pca = &pca_i2c[0];

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

/*******************************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_register_test (void)
{

    #if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
    #endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    //get_mb_pid((char *)ngsm_get_pid);
    memcpy(ngsm_get_pid,(char*)&draco_board_id,2);
    strcpy((char *)ngsm_get_loc, "LTC4215 OIR Register Test");
    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ;
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO","LTC4215");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)display_rbcp_regs_cter);


    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Please follow the below steps and try to debug",
    		 "1.Using LTC4215 Register Write utility to write value 0x55 into offset 0x1 register in LTC4215",
    		 "2.Using LTC4215 Register Read to check the value in offset 0x1 register is 0x55. If not, check the I2C interface between the host and LTC4215.",
    		 "3.Back to step1 and use value 0xaa redo the debugging step.",
    		 "4.If there is no problem on I2C interface, replace one LTC4215 and redo the test.");
#endif


    testname("LTC4215 OIR Register");
    return (oir_ltc4215_register_test(oir_if));
}


/*******************************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_reg_write(void)
{
    return (util_oir_ltc4215_reg_write(oir_if));
}

/*******************************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_reg_read(void)
{
    return (util_oir_ltc4215_reg_read(oir_if));
}

/*******************************************************************************
 *
 * Function   : ltc4215_led_test
 *
 * Description: Wrapped function to do LTC4215 LED test.
 *
 * Inputs     : None 
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_led_test (void)
{
    testname("LTC4215 OIR LED");
    return (oir_ltc4215_leds_test(oir_if));
}

/*******************************************************************************
 *
 * Function   : oir_ltc4215_tests
 *
 * Description: Entry function of Draco OIR(LTC4215)
 *              Diag tests and utilities.
 *
 * Inputs     : show menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int oir_ltc4215_tests (int show_menu)
{
    testname("LTC4215 OIR");

    build_primary_submenu(oir_submenu_table, 
                          OIR_SUBMENU_TABLE_SIZE,
                          "LTC4215 OIR", &oir_submenup);
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
 * Function: draco_pwr_off
 *
 * Description: This function power off draco NIM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_pwr_off (void)
{
    uint8_t data = 0;

    assert (oir_if);

    printf ("\nPower Off the draco NGSM.\n");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power off NGSM module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: draco_power_off
 *
 * Description: This function is a wrapper to power off draco NIM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_power_off (void)
{
    uint8_t ans;

    printf ("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar (ans);
    printf("\n\n");
    
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Draco SM Still Power On.\n\n");
        return (PASSED);
    }

    return (draco_pwr_off());
}

/*******************************************************************************
 *
 * Function: draco_pwr_on
 *
 * Description: This function power on draco NIM.
 *
 * Input :    None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the draco NGSM.\n");

    assert(draco_ngsm_iface);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(draco_ngsm_iface, draco_ngsm_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power on NGSM module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return (FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf ("FET CANNOT be Turned On.\n");
        return (FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf ("Power CANNOT be Turned On.\n");
        return (FAILED);
    }

    printf ("Waiting for draco NGSM to Power-Up.\n");
    msleep (2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    draco_ngsm_iface->uart_on(draco_ngsm_iface);    

    /* take draco NGSM out of reset */
    draco_ngsm_iface->unreset(draco_ngsm_iface);

    printf("Draco NGSM is powered up.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: draco_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the draco NGSM");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar ();
    putchar (ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "draco is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (draco_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the draco NGSM");
        return (FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf (".");
        msleep (1000);
    }

    if (draco_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Draco NIM");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : draco_rbcp_bmc_con_switch
 *
 * Description: A utility to Switch uart console to bmc with RBCP.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int draco_rbcp_bmc_con_switch (void)
{
    int slot;
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;

    draco_rbcp_bmc_console_switch();

    assert (draco_ngsm_iface);
    slot = draco_ngsm_iface->slot;

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
                          "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); // pause a second for the NOTE:

    if (slot == DRACO_NGSM_SLOT1) {
        new_baud = slot1_uart;
    } else if (slot == DRACO_NGSM_SLOT2){
        new_baud = slot2_uart;
    } else {
        new_baud = slot3_uart;
    }    

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d",
             new_baud ? "b9600" : "b115200", draco_ngsm_iface->uart_ctrl);
#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : draco_rbcp_intel_con_switch
 *
 * Description: A utility to Switch uart console to intel with RBCP.
 *
 * Inputs     : NONE
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int draco_rbcp_intel_con_switch (void)
{
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;

    draco_rbcp_intel_console_switch();

    assert (draco_ngsm_iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
                          "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second for the NOTE: */

    new_baud = DRACO_B9600;
    
    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d",
             new_baud ? "b9600" : "b115200", draco_ngsm_iface->uart_ctrl);
#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : draco_o2_shell
 *
 * Description: 
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_o2_shell ()
{
    int slot;

    assert(draco_ngsm_iface);
    slot = draco_ngsm_iface->slot;

    printf("\nEscaping to Shell from NGSM Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return (PASSED);    
}

/*******************************************************************************
 *
 * Function   : draco_o2_command
 *
 * Description: 
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int draco_o2_command ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9557_reg_dump_util
 *
 * Description: Wrap utility to dump all registers of PCA9557
 *              IO Expander(PCA9557).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pca9557_reg_dump_util (void)
{
    uint32_t         ctr = 0, total_reg_num = 0;
    uchar            data = 0, reg_data[8];
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    memset((uchar *)reg_data, 0, (sizeof(reg_data)/sizeof(uchar))); 
    total_reg_num = (sizeof(pca9557_reg_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++) {
        data = 0;

        if (io_port_8bit_i2c_read(io_exp, ctr, &data, TRUE)) {
            printf("\n\nFailed to read IO Expander(PCA9557)"
                   " register 0x%02X.\n\n", ctr);
            return (FAILED);
        }
        reg_data[ctr] = data;
    }

    printf("\nDraco IO Expander(PCA9557) registers dump:\n");
    for (ctr = 0; ctr < total_reg_num; ctr++) {
        reg_p = &pca9557_reg_tbl[ctr];
        printf("%-25s Reg.(0x%01X) = 0x%02X.\n",
               reg_p->name, reg_p->offset, reg_data[ctr]);
    }

    return (PASSED);
}

/*************************************************************************
 * Function: draco_rbcp_picocom_switch
 *
 * Switch uart console without RBCP
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long draco_rbcp_picocom_switch (void)
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert (draco_ngsm_iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second to display above NOTE */

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             draco_ngsm_iface->uart_ctrl);

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("cmd=%s\n", cmd);
    }

    system(cmd);

    return (PASSED);

}

/*******************************************************************************
 *
 * Function   : pca9557_reg_read_util
 *
 * Description: Wrap utility to read specific register of PCA9557
 *              IO Expander(PCA9557).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pca9557_reg_read_util (void)
{
    uint32_t         offset = 0;
    uchar            data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    offset = (uint32_t)gethex_answer("Enter offset of register:", 0, 0, 0x7);

    if (io_port_8bit_i2c_read(io_exp, offset, &data, TRUE)) {
        printf("\n\nFailed to read IO Expander(PCA9557) register 0x%02X.\n\n",
               offset);
        return (FAILED);
    }

    reg_p = &pca9557_reg_tbl[offset];
    printf("\nIO Expander(PCA9557) %s Reg.(0x%01X): 0x%02X.\n\n",
           reg_p->name, reg_p->offset, data);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9557_reg_write_util
 *
 * Description: Wrap utility to write specific register of PCA9557
 *              IO Expander(PCA9557).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pca9557_reg_write_util (void)
{
    uint32_t         offset = 0;
    uchar            data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    offset = (uint32_t)gethex_answer("Enter offset of register:", 0, 0, 0x7);

    reg_p = &pca9557_reg_tbl[offset];

    if (reg_p->offset == PCA9557_IN_PORT_REG) {
        printf("\n\n %s Reg.(0x%01X) is an input-only register"
               " it's prohibited to write this register.\n\n",
               reg_p->name, offset);
        return (PASSED);
    }

    data = (uchar)gethex_answer("Enter write-in Data:", 0, 0, 0xFF);

    if (io_port_8bit_i2c_write(io_exp, offset, &data)) {
        printf("\n\nFailed to write 0x%02X to IO Expander(PCA9557)"
               " register 0x%02X.\n\n", data, offset);
        return (FAILED);
    }

    printf("\nDone write 0x%02X to IO Expander(PCA9557) %s Reg.(0x%01X).\n\n",
           data, reg_p->name, reg_p->offset);

    return (PASSED);
}


/**********************************************************************
 *
 *  Function: draco_set_rbcp_mac_add
 *
 *  Get ge mac address
 *
 *  Input: none
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
static int draco_set_rbcp_mac_add (void)
{
    uchar slot=0;

    slot = draco_ngsm_iface->slot;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n slot number = %d",slot);
    }

    rbcp_get_mac(slot);

    return (PASSED);
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
    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        printf(" Failed to read Canis LTC4215 CONTROL register\n");
    } else {
        printf(" LTC4215 CONTROL: 0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_ALERT_REG, &data)) {
        printf(" Failed to read Canis LTC4215 ALERT register\n");
    } else {
        printf(" LTC4215 ALERT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        printf(" Failed to read Canis LTC4215 STATUS register\n");
    } else {
        printf(" LTC4215 STATUS:  0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_FAULT_REG, &data)) {
        printf(" Failed to read Canis LTC4215 FAULT register\n");
    } else {
        printf(" LTC4215 FAULT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_SENSE_REG, &data)) {
        printf(" Failed to read Canis LTC4215 SENSE register\n");
    } else {
        printf(" LTC4215 SENSE:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_SOURCE_REG, &data)) {
        printf(" Failed to read Canis LTC4215 SOURCE register\n");
    } else {
        printf(" LTC4215 SOURCE:  0x%02x\n", data);
    }
}

/**********************************************************************
 *
 * Function: util_show_oir_ltc4215_regs
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

    return PASSED;
}

/**********************************************************************
 *
 * Function: util_debug_o2_power_fault
 *
 * Showing debug message when flag is turn on
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

static long util_debug_o2_power_fault (void)
{
    if (getc_answer("Turn on power fault debug flag? (y/n)", "yn", 'n')
        == 'y') {
        debug_o2_power_fault_flag = 1;
    } else {
        debug_o2_power_fault_flag = 0;
    }

    return PASSED;
}




/**********************************************************************
 *
 * Function: set_draco_bp_loopback
 *
 * This function is used by Draco diag to set the line loopback of the
 * GE switch port that it connect to for loopback testing.
 *
 * Input :  slot - NGSM slot number
 *          local_port - GE0 or GE1
 *          mode - TRUE for setting the line loopback mode
 *
 * Output: void
 *
 **********************************************************************
 */

void set_draco_bp_loopback (int slot, int local_port, int mode)
{
    int port, onoff;

    port = ovld_get_ge_sw_port_num(slot, TGT_DEV_NGSM, local_port);
    onoff = (mode == TRUE); /* set or clear */
    set_gesw_line_loopback(port, onoff);
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
void draco_check_bmc_ready (void)
{
    char ix, rc = 0, recv_retry = 50;
    char buf[RBCP_MSG_BUF_SIZE];
    int pkt_len = 0;

    printf("Waiting for Draco BMC to boot up\n");
    
    for (ix = 0; ix < recv_retry; ix++) {
        printf(".");
        fflush(stdout);
        rc = rbcp_recv_msgs(buf, CISCO_SCP_OP_REG, &pkt_len);
        if (!rc) break;
    }
    if (ix >= recv_retry) {
        printf("Warning: Draco BMC did not ready, not receive RBCP packet\n");
    }


}


/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngsm_draco.c,v $
 * Revision 1.6  2019/01/25 06:46:28  alpeng
 * CSCvo11021 - fixed bug aquila and shedir will skip slot3
 *
 * Revision 1.5  2018/05/22 02:31:11  alpeng
 * fixed compiler warning, CSCvj57934
 *
 * Revision 1.4  2018/05/18 09:24:49  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.3  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2.8.2  2018/05/17 10:50:21  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.2.8.1  2016/12/05 06:37:00  alpeng
 * fixed the uart ctrl num for ngio; change is approved on prrq
 *
 * Revision 1.2  2016/01/21 01:50:03  olin2
 * Collapse Draco-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2015/07/27 02:05:51  olin2
 * Initial commit code for Draco
 *
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
