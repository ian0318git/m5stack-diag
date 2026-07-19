/* $Id: nim_kalamata.c,v 1.6 2019/11/25 08:55:50 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/kalamata/nim_kalamata.c,v $
 *------------------------------------------------------------------------------
 *
 * nim_kalamata.c: This file contains functions for NIM Kalamata
 *
 * June 2018 - Kody Ko
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
#include "nim_kalamata.h"
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
#include "kalamata_rbcp_main.h"
#include "rbcp_lib.h"
#include "rbcp_platform.h"
#include "bcm_gesw_defs.h"
#include "kalamata_uart.h"

#define ENHANCED_ERR_MSG_EXAMPLE 1
/*******************************************************************************
 * Extern function prototypes
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int getdec_answer(char *,uint ,uint ,uint);
extern void show_margins_cterr_wrapper(void);
extern int rbcp_recv_msgs(char *, ushort, int *);
#ifdef TABEIL
n2g_i2c_if_t pca_i2c[32];
#else
extern n2g_i2c_if_t pca_i2c[];
#endif
extern int dash_tx_uart(char *, char *);
extern int dash_rx_polling_uart(char *, char *, int);

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
static int kalamata_power_off(void);
static int pwr_off(void);
static int kalamata_pwr_on(void);
static int kalamata_pwr_cycle(void);
static int kalamata_utils(void);
static int kalamata_ioe_reg_test(void);
static int kalamata_o2_shell(void);
static int kalamata_o2_command(void);
static void show_oir_ltc4215_regs(void);
static long util_show_oir_ltc4215_regs(void);
static void (*kalamata_saved_diag_exec)(void) = NULL;
static char pca_buff0[256];
static void *oir_if;
static int kalamata_set_rbcp_mac_add(void);
static long kalamata_boot_image(int);


void set_kalamata_bp_loopback(int, int, int);
int kalamata_nim_test(void *);

static void display_i2c_regs_cter(void);
static void display_i2c_regs(void);

long kalamata_rbcp_picocom_switch (void);
long kalamata_show_ioe_regs(char *);

static speed_t slot1_uart = KALAMATA_B9600;

int kalamata_test_slot = 1;
ushort kalamata_board_id;
boolean pca9557;

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
kalamata_ds_t kalamata_iface[MAX_WIC+1];
static kalamata_ds_t *kalamata_iface_p;
extern int do_all_menu_items(struct menuinfo *);

struct ngio_intf_t *kalamata_nim_iface;

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
     0,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR reg. Write util",     (PFT)ltc4215_reg_write,      0,
     0,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR Register Test",       (PFT)ltc4215_register_test,  0,
     MM_2,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR LED Test",            (PFT)ltc4215_led_test,       0,
     MM_2,
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
static submenu_xtable_t kalamata_utils_submenu_table[] = {
    {"Power off kalamata NIM",      (PFT)kalamata_power_off,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"Power on kalamata NIM",       (PFT)kalamata_pwr_on,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"Power cycle kalamata NIM",    (PFT)kalamata_pwr_cycle,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Test",           (PFT)ltc4215_register_test,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Read",           (PFT)ltc4215_reg_read,          0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Write",          (PFT)ltc4215_reg_write,         0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register show",          (PFT)util_show_oir_ltc4215_regs, 0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"IO Expander Reg. Dump",  (PFT)pca9557_reg_dump_util,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0}, 
    {"IO Expander Reg. Read",  (PFT)pca9557_reg_read_util,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0}, 
    {"IO Expander Reg. Write", (PFT)pca9557_reg_write_util,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},  
    {"Escape to Shell (debugging only)", (PFT)kalamata_o2_shell,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)kalamata_o2_command,0,0,
      (type_t(*)())0, 0,        (type_t(*)())0,          0},
    {"Picocom Console redirect Link", (PFT)kalamata_rbcp_picocom_switch,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define KALAMATA_UTILS_SUBMENU_TABLE_SZ (sizeof(kalamata_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t kalamata_utils_primary_items[KALAMATA_UTILS_SUBMENU_TABLE_SZ + 
                                        MAX_BASE_ITEMS];
static mitem_t kalamata_utils_secondary_items[KALAMATA_UTILS_SUBMENU_TABLE_SZ + 
                                          MAX_BASE_ITEMS];

char kalamatautiltitle[50];

menuinfo_t kalamata_util_submenu = {
    kalamatautiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    kalamata_utils_primary_items,
};

menuinfo_t *kalamata_util_submenup = &kalamata_util_submenu;


/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Kalamata Utilities",      (PFT)kalamata_utils,        0,   0,
     (type_t(*)())0, 0,         (type_t(*)())kalamata_utils,    0},
    {"Boot Kalamata Image",       (PFT)kalamata_boot_image, 0, MM_2,
     (long(*)())0, 0, (long(*)())0, 0 },
    {"LTC4215 OIR Test",            (PFT)oir_ltc4215_tests,         0, MM_2,
     (type_t(*)())0, 0,         (PFT)oir_ltc4215_tests,             TRUE},
    {"I2C IO Expander Register Test",       (PFT)kalamata_ioe_reg_test, 0, MM_2,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0}, 
    {"RBCP Tests",            (PFT)build_kalamata_rbcp_menu, 0, MM_2,
     (type_t(*)())0, 0, 	(PFT)build_kalamata_rbcp_menu, TRUE},
    { "Kalamata Console redirect Test",  (PFT)kalamata_rbcp_picocom_switch, 0, 0,
     (type_t(*)())0, 0, 		(type_t(*)())0, 0 },
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
        
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Kalamata Main Menu",   /* title */
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
 * Function: kalamata_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 *******************************************************************************
 */
static void kalamata_cleanup (void)
{
    assert(kalamata_nim_iface);

    if (kalamata_saved_diag_exec) {
        pre_diag_exec = kalamata_saved_diag_exec;
        kalamata_saved_diag_exec = NULL;
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
    int fd;
    struct termios oldtio, newtio;
    int new_baud = 0;

    assert(kalamata_nim_iface);

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", kalamata_nim_iface->uart_ctrl);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    new_baud = slot1_uart;

    newtio.c_cflag = CS8 | CLOCAL | CREAD; /* control mode flags */
   
    if (new_baud == KALAMATA_B115200) {
        newtio.c_cflag |= B115200;
    } else if (new_baud == KALAMATA_B9600) {
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


/*******************************************************************************
 *
 * Function: kalamata_iface_test
 *
 * Description: Test entry for kalamata interface test.
 *              covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int kalamata_iface_test ()
{
    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }

    prcomplete (testpass, errcount, 0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: kalamata_nim_test().
 *
 * Description: This function is the main entrance for kalamata NIM test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int kalamata_nim_test (void *nim)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    char board_pid[20] = {0};
    uchar num_byte;

    assert(nim);

    kalamata_nim_iface = (struct ngio_intf_t *)nim;

    slot = kalamata_nim_iface->slot;
    kalamata_test_slot = kalamata_nim_iface->slot;
    board_id = kalamata_nim_iface->id;
    kalamata_board_id=kalamata_nim_iface->id;

    slot_get_bd_pid(kalamata_nim_iface->cookie, board_pid, &num_byte);
    num_byte = 9; /* P1A2 MFG pads PID with 9 white spaces. */

    kalamata_nim_iface->uart_on(nim); 
    
    printf ("\n%s, board_id %#x, slot %d.\n", board_pid, board_id, slot);

    testname ("Slot%d kalamata NIM ", slot);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = kalamata_nim_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = 0x1C;
    pca_i2c[0].buf   = pca_buff0;

    oir_if = (void *)(kalamata_nim_iface->oir);

    /*
     * Initialize an instance of kalamata data structure
     */
    kalamata_iface_p = (kalamata_ds_t *) &kalamata_iface[slot];
    kalamata_iface_p->board_id = board_id;
    kalamata_iface_p->slot = slot;
    kalamata_iface_p->uart = kalamata_nim_iface->uart_ctrl;
    kalamata_iface_p->kalamata_nim_iface = (struct ngio_intf_t *)nim;

    /*
     * Initialize an instance of kalamata data structure
     */
     
    kalamata_nim_iface->unreset(nim);

    /* Set kalamata boot up parameters */
    setup_uart();
    
    msleep (1000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    kalamata_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    /* Setup RBCP GE switch environment and Mac address */
    if (kalamata_setup_rbcp_ge_env(kalamata_iface_p->slot) == FAILED) {
            return (FAILED);
    }
    kalamata_set_rbcp_mac_add();

    clear_kalamata_regis_done_flag(kalamata_test_slot);

    /*
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    kalamata_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    ret_val = PASS;
    if (kalamata_nim_iface->menu_display == TRUE) {
    	menu(maindiagp, main_menu_secondary_items, '\0');
    } else {
    	if (kalamata_nim_iface->test_type == IFACE_TEST) {
        	ret_val = kalamata_iface_test();
    	} else {
            do_all_menu_items(maindiagp);
    	}
    }

    if (kalamata_saved_diag_exec) {
    	pre_diag_exec = kalamata_saved_diag_exec;
    	kalamata_saved_diag_exec = NULL;
    }

    if (kalamata_cleanup_rbcp_ge_env(kalamata_iface_p->slot) == FAILED) {
        return (FAILED);
    }

    kalamata_cleanup ();

    return (ret_val);
}

/*******************************************************************************
 *
 *  Function: kalamata_utils
 *
 *  Description: Kalamata Utilities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int kalamata_utils (void)
{
    assert(kalamata_nim_iface);

    sprintf(kalamatautiltitle, "Kalamata Slot %d Utilities Menu", 
            kalamata_nim_iface->slot);
    build_primary_submenu(kalamata_utils_submenu_table,
                          KALAMATA_UTILS_SUBMENU_TABLE_SZ,
                          kalamatautiltitle, &kalamata_util_submenup);

    build_secondary_submenu(kalamata_utils_submenu_table,
                            KALAMATA_UTILS_SUBMENU_TABLE_SZ,
                            kalamata_utils_secondary_items);

    menu(kalamata_util_submenup, kalamata_utils_secondary_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : display_i2c_regs_cter
 *
 * Description: calling display_i2c_regs function to dump i2c io expander registers
 *
 * Inputs     : None
 *
 * Outputs    : None
 *
 *******************************************************************************
 */

static void display_i2c_regs_cter (void)
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
    kalamata_show_ioe_regs("\nI2C IO Expander config");
}

/*******************************************************************************
 *
 * Function   : kalamata_ioe_reg_test
 *
 * Description: Wrapped function to do Kalamata IO Expander register test.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int kalamata_ioe_reg_test (void)
{
    testname("I2C IO Expander Register");
    int ix;
    uchar sav, wval, rval;
    n2g_i2c_if_t *pca= &pca_i2c[0];
    int32_t reg = POLARITY_INV_REG;
    char *name = "Polarity";

    #ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar nim_get_pid[FRU_SIZE] = {0};
    uchar nim_get_loc[FRU_SIZE] = {0};
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

    // get_mb_pid((char *)mb_get_pid);
    memcpy(nim_get_pid,(char*)&kalamata_board_id,2);
    strcpy((char *)nim_get_loc, "I2C IO Expander Register Test");
    platform_fru_table[fru_table_offset].pid_string = nim_get_pid;
    platform_fru_table[fru_table_offset].location_string = nim_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("I2C");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)display_i2c_regs_cter);


    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Go to the Kalamata Utilities menu",
    		 "Use the IO Expander Reg. Read Utilities to check whether it can or can't dump the register value ");
   #endif


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

        if (diagflag_xram & D_VERBOSE) {
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
    for (ix = 0; ix < 8; ix++) {
        wval = ~(1 << ix);
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }

        if (diagflag_xram & D_VERBOSE) {
            printf("%#x ", rval);
        }

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

    prcomplete (testpass, errcount, 0);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: kalamata_show_ioe_regs
 *
 * utility to dump i2c io expander registers
 *
 * Input : title - title to display
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long kalamata_show_ioe_regs (char *title)
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
    uchar nim_get_pid[FRU_SIZE] = {0};
    uchar nim_get_loc[FRU_SIZE] = {0};
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

    //get_mb_pid((char *)nim_get_pid);
    memcpy(nim_get_pid,(char*)&kalamata_board_id,2);
    strcpy((char *)nim_get_loc, "LTC4215 OIR Register Test");
    platform_fru_table[fru_table_offset].pid_string = nim_get_pid ;
    platform_fru_table[fru_table_offset].location_string = nim_get_loc;

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
 * Description: Entry function of Kalamata OIR(LTC4215)
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
 * Function: pwr_off
 *
 * Description: This function power off kalamata NIM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_off (void)
{
    uint8_t data = 0;

    assert (oir_if);

    printf ("\nPower Off the kalamata NIM.\n");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power off NIM module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: kalamata_power_off
 *
 * Description: This function is a wrapper to power off kalamata NIM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int kalamata_power_off (void)
{
    uint8_t ans;

    printf ("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar (ans);
    printf("\n\n");
    
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! kalamata NIM Still Power On.\n\n");
        return (PASSED);
    }

    return (pwr_off());
}

/*******************************************************************************
 *
 * Function: kalamata_pwr_on
 *
 * Description: This function power on kalamata NIM.
 *
 * Input :    None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int kalamata_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the kalamata NIM.\n");

    assert(kalamata_nim_iface);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(kalamata_nim_iface, kalamata_nim_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power on NIM module */
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

    printf ("Waiting for kalamata NIM to Power-Up.\n");
    msleep (2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    kalamata_nim_iface->uart_on(kalamata_nim_iface);    

    /* take kalamata NIM out of reset */
    kalamata_nim_iface->unreset(kalamata_nim_iface);

    printf("Kalamata NIM is powered up.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: kalamata_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int kalamata_pwr_cycle (void)
{
    uint8_t ix, ans;

    printf("\n");
    printf("Power Cycle the kalamata NIM");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar ();
    putchar (ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "kalamata is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (pwr_off()) {
        printf("Failed to Power Off the kalamata NIM");
        return (FAILED);
    }

    /* msleep for 10 seconds. */
    for (ix = 0; ix < ROUND; ix++) {
        printf (".");
        msleep (1000);
    }

    if (kalamata_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Kalamata NIM");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : kalamata_o2_shell
 *
 * Description: 
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int kalamata_o2_shell ()
{
    int slot;

    assert(kalamata_nim_iface);
    slot = kalamata_nim_iface->slot;

    printf("\nEscaping to Shell from NIM Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return (PASSED);    
}

/*******************************************************************************
 *
 * Function   : kalamata_o2_command
 *
 * Description: 
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int kalamata_o2_command ()
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

    printf("\nKalamata IO Expander(PCA9557) registers dump:\n");
    for (ctr = 0; ctr < total_reg_num; ctr++) {
        reg_p = &pca9557_reg_tbl[ctr];
        printf("%-25s Reg.(0x%01X) = 0x%02X.\n",
               reg_p->name, reg_p->offset, reg_data[ctr]);
    }

    return (PASSED);
}

/*************************************************************************
 * Function: kalamata_rbcp_picocom_switch
 *
 * Switch uart console without RBCP
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
long kalamata_rbcp_picocom_switch (void)
{
    kalamata_ds_t *iface;
    const int maxlen = 128;
    char cmd[maxlen];

    iface = kalamata_iface_p;
    assert(iface);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); /* pause a second to display above NOTE */

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             kalamata_nim_iface->uart_ctrl);

    if (diagflag_xram & D_VERBOSE) {
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
 *  Function: kalamata_set_rbcp_mac_add
 *
 *  Get ge mac address
 *
 *  Input: none
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
static int kalamata_set_rbcp_mac_add (void)
{
    uchar slot = 0;

    slot = kalamata_nim_iface->slot;

    if (diagflag_xram & D_VERBOSE) {
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
        printf(" Failed to read Kalamata LTC4215 CONTROL register\n");
    } else {
        printf(" LTC4215 CONTROL: 0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_ALERT_REG, &data)) {
        printf(" Failed to read Kalamatta LTC4215 ALERT register\n");
    } else {
        printf(" LTC4215 ALERT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        printf(" Failed to read Kalamata LTC4215 STATUS register\n");
    } else {
        printf(" LTC4215 STATUS:  0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_FAULT_REG, &data)) {
        printf(" Failed to read Kalamata LTC4215 FAULT register\n");
    } else {
        printf(" LTC4215 FAULT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_SENSE_REG, &data)) {
        printf(" Failed to read Kalamata LTC4215 SENSE register\n");
    } else {
        printf(" LTC4215 SENSE:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_SOURCE_REG, &data)) {
        printf(" Failed to read Kalamata LTC4215 SOURCE register\n");
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
  * Function: kalamata_boot_image
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
 static long kalamata_boot_image (int show_menu)
 {
    int boot_timeout, ping_timeout, go_timeout;
    const int maxlen = 128;
    char tty_dev[maxlen];

     /* Power up Kalamata NIM now */
     kalamata_pwr_on();

     msleep(KALAMATA_POWER_UP_DELAY);

     /* Setup UART toward Kalamata NIM */
     printf("Setup UART...");
     sprintf(tty_dev, "/dev/ttyDASH%d", kalamata_nim_iface->uart_ctrl);

     /* Polling if bootloader is up, we need to fire tftpdnld command
      * through UART interface since Kalamata bootloader doesn't boot
      * linux by default. (It boots up SE instead)
      */
     printf("Looking for bootloader prompt (1)...");
     fflush(stdout);

     boot_timeout = KALAMATA_BL_PROMPT_TOUT;
     do {
         /* Transmit New Line */
         dash_tx_uart(tty_dev, KALAMATA_CR_STRING);

         if (dash_rx_polling_uart(tty_dev, KALAMATA_BL_PROMPT, 100) == PASSED) {
             printf("OK\n");
             fflush(stdout);
             break;
         }
     } while (boot_timeout--);

     if (boot_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to get '%s' bootloader prompt", KALAMATA_BL_PROMPT);
         return (FAILED);
     }
     msleep(500);

     /* since autoboot is disabled by sending CR on previous stage,
      * we are able to spend time to download image now. 
      * To avoid the timeout for start autoboot automatically */
     if (tftp_get(0, (unsigned char *)"kalamata.SSA",
                  0, (unsigned char *)"/firmware/kalamata.SSA", 1) < 0) {
         cterr('f', 0, "Failed to tftp download firmware to local host");
         return(FAILED);
     }

     /* Now, we can do tftp download in Uboot prompt */
     printf("Set ENV ...");
     dash_tx_uart(tty_dev, KALAMATA_CR_STRING);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_IPADDR);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_NETMASK);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_GETWAY);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_SERVERIP);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_ETH1);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_ETH2);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_ETH);
     msleep(500);
     dash_tx_uart(tty_dev, KALAMATA_SET_ETHACT);
     msleep(500);

     /* Now ping the server ip */
     printf("Ping TFTP Server from Backplane ...");
     fflush(stdout);

     dash_tx_uart(tty_dev, KALAMATA_PING_SERVER);
     dash_tx_uart(tty_dev, KALAMATA_CR_STRING);
     ping_timeout = KALAMATA_PING_TOUT;
     do{
         if (dash_rx_polling_uart(tty_dev, KALAMATA_PING_ALIVE, 1000) == PASSED) {
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
     printf("Loading image ...");
     fflush(stdout);

     dash_tx_uart(tty_dev, KALAMATA_SET_FILENAME);
     dash_tx_uart(tty_dev, KALAMATA_CR_STRING);
     dash_tx_uart(tty_dev, KALAMATA_BOOT_UP_CMD);

     boot_timeout = KALAMATA_BOOT_TOUT;
     do{
         if (dash_rx_polling_uart(tty_dev, KALAMATA_BOOT_PASSED, 1000) == PASSED) {
             printf("OK\n");
             fflush(stdout);
             break;
         }
     } while (boot_timeout--);

     if (boot_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to load image");
         return (FAILED);
     }


     printf("Booting image ...");
     fflush(stdout);

     dash_tx_uart(tty_dev, KALAMATA_GO_ADDR);

     go_timeout = KALAMATA_GO_TOUT;
     do{
         if (dash_rx_polling_uart(tty_dev, KALAMATA_RBCP_LOOP, 1000) == PASSED) {
             printf("OK\n");
             fflush(stdout);
             break;
         }
     } while (go_timeout--);

     if (go_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to bootup image");
         return (FAILED);
     }
    return (PASSED);
 }



/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: nim_kalamata.c,v $
 * Revision 1.6  2019/11/25 08:55:50  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.5  2018/05/18 09:24:50  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.4.2.3  2018/04/20 09:24:45  alpeng
 * move tftp_get() right after stop autoboot, otherwise it will fail to stop autoboot
 *
 * Revision 1.4.2.2  2018/04/20 08:45:41  alpeng
 * support kalamata on Neptune
 *
 * Revision 1.4  2018/04/19 09:15:38  letsai
 * Add LED utility of single color control
 *
 * Revision 1.3  2018/03/30 03:24:09  letsai
 * Change auto boot function to common code and can do all NIM tests automatically.
 *
 * Revision 1.2  2018/02/24 07:36:25  letsai
 * Collapse Kalamata-branch to Main Trunk.
 *
 * Revision 1.1.4.5  2017/09/21 02:07:25  kodko
 * Turn on the green light when enter the kalamata menu.
 *
 * Revision 1.1.4.4  2017/09/21 01:38:01  kodko
 * Support ISR4K platform slot2 and slot3.
 *
 * Revision 1.1.4.3  2017/08/17 13:01:51  kodko
 * Automation test bring up for Kalamata P1A.
 *
 * Revision 1.1.4.2  2017/06/16 07:17:03  kodko
 * Initial platform code commit for Kalamata project.
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
