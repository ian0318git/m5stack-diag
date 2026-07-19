/* $Id: ngwic_reva.c,v 1.14 2020/05/22 02:28:23 qingcwan Exp $  
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/ngwic_reva.c,v $
 *------------------------------------------------------------------
 *
 * ngwic_reva.c - This file contains functions for Reva NIM.
 *
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
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
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "dash_fpga.h"
#include "adapter_fpga.h"
#include "cross_platform.h"
#include "cookie_4.h"
#include "platform_fru.h"
#include "cli_cmd.h" /* show margining */

#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef TACHI
#include "diag_console_util.h"
#include "diag_fpga_lib.h"
#include "cetus_gesw_defs.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_utils.h"
#include <linux/filter.h>  /* pkt filter */
#include <arpa/inet.h>
#elif TABEIL
#include "diag_fpga_lib.h"
#elif NANOOK
#include "dash_fpga.h"
#else
#include "platform_eth_pkt_txrx.h"
#include "platform_margin_utils.h"
#endif

#define OIR_RES    0.02
#define OIR_SENSE_RES    0.151

#define REVA_CR_STRING            "\015"
#define REVA_ESC_CR_STRING        "\033\015"
#define REVA_RUN_DIAG             "\015/home/apps/reva;"
#define NIM_ASYNC_FW_IMG          "nim_async_fw.img"
#define SM_ASYNC_FW_IMG           "sm_async_fw.img"
#define NIM_ASYNC_FW_PATH         "/firmware/nim_async_fw.img"
#define SM_ASYNC_FW_PATH         "/firmware/sm_async_fw.img"

#define REVA_LOCAL_IP_ADDR     "192.123.123.200"
#define REVA_REQUEST_PORT      2013
#define REVA_STATUS_PORT1       2016
#define DIAG_RTN_PASS_STR        "PASS"
#define DIAG_RTN_STR_LEN         4
#define DIAG_KILL_NC_TMP_FILE    "/tmp/reva_nc_tmp.pid"

/* PCA9557 Definition */
#define PCA9557_IN_PORT_REG             0x00
#define PCA9557_OUT_PORT_REG            0x01
#define PCA9557_POLAR_INV_P_REG         0x02
#define PCA9557_CFG_PORT_REG            0x03

#define PCA9557_PORT_MASK               0xFF
#define PCA9557_PORT_INIT               0x00

extern int tftp_get (unsigned char *dir, unsigned char *file, 
     unsigned char *server_ip, unsigned char *dest, unsigned int check);
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);
extern void port_tx_util(void);
extern int ovld_bcm_check_port_init(void);
extern int get_gesw_line_loopback(int port_num);
extern int do_all_menu_items(struct menuinfo *);
extern void display_env(void);
extern void display_uart_regs_cterr_wrapper(void);

static int ltc4215_util(void);
static int ltc4215_register_test (void);
static int reva_pwr_off (void);
static int reva_pwr_on (void);
static int reva_pwr_off_util (void);
static int reva_pwr_on_util (void);
static int reva_pwr_cycle_util (void);
static int reva_console_switch(void);
static void disable_bp_ge_lpbk (void);
static void enable_bp_ge_lpbk (void);
static int reva_bp_ge_test(void);
static int pca9557_util(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int set_ngwic_console(void);
static int reva_uart_test (void);
static int reva_reset (void);
static int reva_utils (void);
static int reva_nim_test(void);
static int reva_nim_test_double_char(void);
static int nc_cmd_run_reva_diag (int port);
static int reva_check_test_status (void);
static int reva_init_status_file (void);
static void reva_kill_nc (void);
static void reva_get_host_flag(void);
static int reva_send_diag_flag(void);
static int reva_ioe_reg_test(void);
int is_reva_sm (void);
int is_reva_nim (void);

static void (*reva_saved_diag_exec)(void) = NULL;
static void *oir_if;

static n2g_i2c_if_t *pca_i2c;

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

static struct ngio_intf_t *reva_wic_iface;
static int reva_slot;
static uchar ngwic_get_pid[FRU_SIZE] = {0}; 
static uchar ngwic_get_loc[FRU_SIZE] = {0};
static char nim_test_rlt[20] = {0};

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t reva_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)reva_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on reva NGWIC",     (PFT)reva_pwr_on_util,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off reva NGWIC",      (PFT)reva_pwr_off_util,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle reva NGWIC",    (PFT)reva_pwr_cycle_util,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Reva Backplane GE Utility",  (PFT)reva_bp_ge_test,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)reva_uart_test,   0,   
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Hotswap Utility",       (PFT)ltc4215_util, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 GPIO Expander Utility", (PFT)pca9557_util,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Reset Reva NGWIC",            (PFT)reva_reset,         0,    0,
     (type_t(*)())0, 0,    (type_t(*)())0,           0},
};

#define REVA_UTILS_SUBMENU_TABLE_SZ (sizeof(reva_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t reva_utils_primary_items[REVA_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t reva_utils_secondary_items[REVA_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];

char revautiltitle[50];

menuinfo_t reva_util_submenu = {
    revautiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    reva_utils_primary_items,
};

menuinfo_t *reva_util_submenup = &reva_util_submenu;
/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Reva Utilities",            (PFT)reva_utils,       0,   0,
     (type_t(*)())0, 0,    (type_t(*)())reva_utils, 0},

    {"LTC4215 Hotswap test",      (PFT)ltc4215_register_test, 0, MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,    (type_t(*)())ltc4215_util, 0},

    {"PCA9557 GPIO Expander test",(PFT)reva_ioe_reg_test, 0, MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,    (type_t(*)())pca9557_util, 0},

    {"Reva NIM test",      (PFT)reva_nim_test, 0, MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*) ())is_reva_nim, 0,    (type_t(*)())reva_nim_test_double_char, 0},

    {"Reva SM test",      (PFT)reva_nim_test, 0, MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*) ())is_reva_sm, 0,    (type_t(*)())reva_nim_test_double_char, 0},

    {"UART Test",          (PFT)reva_uart_test, 0, MF_CONTINUOUS |  MF_SHOW_ERRCOUNT | MF_DOALL,
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
    "Reva Main Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
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

/*
 **********************************************************************
 *
 *  Function: cterr_pid
 *
 *  Description: Initial Reva specific cterr parameters.
 *
 *  Input: slot
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void cterr_pid(int slot)
{
    uchar num_byte;
    char buf[50];

    /* Setup common parameters for new error message */
    slot_get_bd_pid(reva_wic_iface->cookie, (char*)ngwic_get_pid, &num_byte);
    num_byte = 9; /* P1A2 MFG pads PID with 7 white spaces. */

    if (reva_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        sprintf(buf, " MB/SM Carrier Card Slot%x",reva_slot);
        strcpy((char *)ngwic_get_loc, buf);
    } else if (reva_wic_iface->mod_type == SM_MODULE) {
        sprintf(buf, " MB/SM%x",reva_slot);
        strcpy((char *)ngwic_get_loc, buf);
    } else {
        sprintf(buf, " MB/WIC%x",reva_slot);
        strcpy((char *)ngwic_get_loc, buf);
    }
}

/*
 **********************************************************************
 *
 *  Function: display_no_reg
 *
 *  Description: Print message if item has no reg to display.
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void display_no_reg (void)
{
    cterr_db_print("This item has no reg to display\n"); 
}

/*
 **********************************************************************
 *
 *  Function: display_fpga_reg
 *
 *  Description: Print message if item can use registers read/write/dump 
 *               utilities to read/write/dump the FPGA registers.
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void display_fpga_reg (void)
{
    cterr_db_print("Please use registers read/write/dump utilities in \
FPGA utility to read/write/dump the FPGA registers.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_pwr_cycle_err_report
 *
 *  Description: Enhance error message for power cycle util
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_pwr_cycle_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Platform Power controller");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check platform power circuitry.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_mem_err_report
 *
 *  Description: Enhance error message for memory test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_mem_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA", "Mem DDR3");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check available memory size.", "Please check ECC."); 
}

/*
 **********************************************************************
 *
 *  Function: add_reva_ge_mac_reg_err_report
 *
 *  Description: Enhance error message for MAC register test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_ge_mac_reg_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check GE MAC configuration registers.", 
                    "Please check GE MDIO configuration registers.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_ge_mac_intr_err_report
 *
 *  Description: Enhance error message for MAC intr test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_ge_mac_intr_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check interrupt line.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_ge_mac_lpbk_err_report
 *
 *  Description: Enhance error message for MAC loopback test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_ge_mac_lpbk_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check GE MAC configuration registers.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_ge_dma_reg_err_report
 *
 *  Description: Enhance error message for DMA register test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_ge_dma_reg_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please try to use FPGA utility to check DMA settings is correctly.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_phy_reg_err_report
 *
 *  Description: Enhance error message for PHY register test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_phy_reg_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA","MV88E1512 GE PHY");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check the MDC and MDIO signal between PL and PHY.",
                    "Replace the PHY chip and re-do the test.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_phy_int_lpbk_err_report
 *
 *  Description: Enhance error message for PHY internal loopback test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_phy_int_lpbk_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA","MV88E1512 GE PHY");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check the interrupt signal between PL and PHY.",
                    "Please check interrupt line.",
                    "Replace the PHY chip and re-do the test.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_async_scc_err_report
 *
 *  Description: Enhance error message for UART/PPP ASYNC loopback test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_async_scc_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA","ADM2209E");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether async connector is plugged correctly.",
                    "Please try to replace the async connector with another one.",
                    "Please try to use FPGA utility to check cheerios serial controller registers.");
}

/*
 **********************************************************************
 *
 *  Function: add_interrupt_err_report
 *
 *  Description: Enhance error message for interrupt test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_interrupt_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA","ADM2209E");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether async connector is plugged correctly.",
                    "Please try to replace the async connector with another one.",
                    "Please try to use FPGA utility to check cheerios serial controller registers.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_margin_err_report
 *
 *  Description: Enhance error message for margin test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_margin_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA","DS4424 Voltage Margin");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check I2C components.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_led_err_report
 *
 *  Description: Enhance error message for LED test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_led_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("Xilinx FPGA");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please try to use FPGA utility to check LED settings.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_console_switch_err_report
 *
 *  Description: Enhance error message for console switch
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_console_switch_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("PCA9557","LTC4215");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please make sure the Reva module setup correctly.",
                    "Using PCA9557 GPIO Expander Utility to read/write \
                    the registers to check register is correct or not.",
                    "Using LTC4215 Hotswap Utility to read/write \
                    the registers to check register is correct or not.");                   
}

/*
 **********************************************************************
 *
 *  Function: display_uart_reg
 *
 *  Description: Display UART register
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void display_uart_reg (void)
{
#ifdef TACHI
    return;
#elif TABEIL
    return;
#elif NANOOK
    return;
#else
    display_uart_regs_cterr_wrapper();
#endif
}

/*
 **********************************************************************
 *
 *  Function: add_reva_uart_test_err_report
 *
 *  Description: Enhance error message for UART test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_uart_test_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }

    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("UART");
    cterr_add_reg_dump((PFV)display_uart_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please make sure the module setup correctly.",
                    "Console switch to Reva and check if Diags runs.",
                    "Uart test will trigger diag main menu item on Reva side, please make sure the Reva side stays on Main menu.",
                    "Please try to reset/unreset the module and redo the test.");
}

/*
 **********************************************************************
 *
 *  Function: add_reva_ltc4215_reg_err_report
 *
 *  Description: Enhance error message for ltc4215 register test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_ltc4215_reg_err_report (void)
{
    if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
    } else {
        fru_table_offset = WIC0 + reva_wic_iface->slot -1;
    }
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
    cterr_add_component("LTC4215");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Using LTC4215 Hotswap Utility to read/write the registers.",       
                    "Check the the value is correct or not.",
                    "If not, check the I2C interface between the host and LTC4215.",
                    "If there is no problem on I2C interface, replace one LTC4215 and redo the test.");     
}

/*
 **********************************************************************
 *
 *  Function: add_reva_pca9557_test_err_report
 *
 *  Description: Enhance error message for pca9557 register test
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void add_reva_pca9557_test_err_report (void)
{
   if(reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
       fru_table_offset = SM0_WIC + reva_wic_iface->slot -1;
   } else {
       fru_table_offset = WIC0 + reva_wic_iface->slot -1;
   }

   platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid;
   platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
   cterr_add_component("PCA9557");
   cterr_add_reg_dump((PFV)display_no_reg);
   cterr_add_env_dump((PFV)display_env);
   cterr_add_debug("Using PCA9557 GPIO Expander Utility to read/write the registers.",
                  "Check the the value is correct or not.",
                  "If not, check the path between the host and PCA9557.",
                  "If there is no problem on I2C interface, replace one PCA9557 and redo the test."); 
}


/*
 **********************************************************************
 *
 *  Function: reva_utils
 *
 *  Description: Reva Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int reva_utils (void)
{
    assert(reva_wic_iface);

    sprintf(revautiltitle, "Reva Slot %d Utilities Menu",
            reva_wic_iface->slot);
    build_primary_submenu(reva_utils_submenu_table,
                          REVA_UTILS_SUBMENU_TABLE_SZ,
                          revautiltitle, &reva_util_submenup);

    build_secondary_submenu(reva_utils_submenu_table,
                            REVA_UTILS_SUBMENU_TABLE_SZ,
                            reva_utils_secondary_items);

    menu(reva_util_submenup, reva_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 ***************************************************************************************
 *
 *  Function: reva_nim_test
 *
 *  Description: run Reva nim test automatically by sending a nc client request to the 
 *               nc server listening on Reva side
 *
 *  Input: None 
 *
 *  Returns: PASSED/FAILED
 *
 ****************************************************************************************
 */
static int reva_nim_test(void)
{
    assert(reva_wic_iface);

    if(is_reva_sm()) {
        testname("Reva SM");
    } else {
        testname("Reva NIM");
    }
    printf("\nStarting Reva diag test with nc...\n");

    cterr_pid(reva_slot);
    if (nc_cmd_run_reva_diag(REVA_REQUEST_PORT)) {
        cterr('f', 0, "HOST: NC command failed in run Reva test\n");
        reva_kill_nc();
        return (FAILED);
    }

    reva_kill_nc();
    return (PASSED);
}

/*
 ***************************************************************************************
 *
 *  Function: reva_nim_test_double_char
 *
 *  Description: reva nim test not available double character
 *
 *  Input: None
 *
 *  Returns: PASSED/FAILED
 *
 ****************************************************************************************
 */
static int reva_nim_test_double_char(void)
{
    printf("subtest menu not available for this double character\n");
    printf("Please go to \"Reva Utilities\" > \"Console Redirect\"\n");

    return (PASSED);
}

/*************************************************************************************************
 * Function: nc_cmd_run_reva_diag
 * Description: Start the local nc server for receiving test status and initial the status file
 *              Send a nc client request to the module side nc server.
 *              Check the test status.
 *
 * Input:    port - ruuning diag request port number
 *
 * Return: PASSED / FAILED
 **************************************************************************************************
 */
static int nc_cmd_run_reva_diag (int port)
{
    char cmdbuf[128];
    int check_flag;
    int i;
    uchar data;

    assert(reva_wic_iface);
    assert(oir_if);

    printf("\nWait for Reva module side to boot up diag menu.\n");

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Reva module side when the diag menu is up. */
    for (i = 0; i < 2000; i++) {
        if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
            util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
            cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
            return (FAILED);
        }

        if (data & 0x08)
            break;

        msleep(200);
    }

    if (i == 2000) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }
    sleep(1);

    reva_get_host_flag();
    reva_send_diag_flag();

    sprintf(cmdbuf, "nc %s %d\n", REVA_LOCAL_IP_ADDR, port);
    printf("HOST: nc command: %s\n", cmdbuf);

    if (reva_init_status_file()) {
        printf("Initial status file error.\n");
        return (FAILED);
    }

    for (i = 0; i < 3; i++) {
        if (system(cmdbuf)) {
            printf("Unable to request nc server.\n");
            return (FAILED);
        }

        check_flag = reva_check_test_status();
        if (check_flag != -1) {
            break;
        } else {
            /* Status file is empty */
            printf("Retry...\n");
        }
        msleep(1000);
    }

    if (check_flag == FAILED) {
        prpass('f', 0, "NGWIC REVA-%d test fails\n", reva_wic_iface->slot);
    } else if (check_flag == PASSED) {
        prpass(testpass, "NGWIC REVA-%d test passes, ", reva_wic_iface->slot);
    } else {
        cterr('f', 0, "NC Connection Error. check_flag = %d\n", check_flag);
        return (FAILED);
    }

    return (PASSED);
}

/********************************************************************************
 * Function: reva_init_status_file
 * Description: This function create the status file if it doesn't exist
 *              and listen to the status port
 *
 * Input:  None
 * Output: PASSED/FAILED
 *
 ********************************************************************************
 */
static int reva_init_status_file (void)
{
    char cmd1[84];
    char status_file[32];

    assert(reva_wic_iface);

    sprintf(status_file, "/tmp/ngwic_reva_%d.status", reva_wic_iface->slot);
    /* create or clear the status file */
    /*sprintf(cmd1, "rm -rf %s", status_file);*/
    sprintf(cmd1, "echo ' ' > %s", status_file);
    system(cmd1);

    /* Listen to the command status */
    sprintf(cmd1, "nc -l -l -p %d > %s &", REVA_STATUS_PORT1, status_file);
    /* sprintf(cmd, "nc -l -l -p %d  > /dev/console &", REVA_STATUS_PORT);*/
    printf("HOST: nc command: %s\n", cmd1);

    if (system(cmd1)) {
        return (FAILED);
    }
    return (PASSED);
}

/*****************************************************************
 *
 * Function: reva_check_test_status
 *
 * Description: This function checks the content of status file and
 *              determine whether the test passes or fails.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************
 */
static int reva_check_test_status (void)
{
    FILE *fp;
    char status_file[32];
    char buf[20];
    char *pch;
    
    memset(nim_test_rlt, 0, sizeof(nim_test_rlt));
    sprintf(status_file, "/tmp/ngwic_reva_%d.status", reva_wic_iface->slot);

    if(is_reva_sm()) {
        testname("Reva SM");
    } else {
        testname("Reva NIM");
    }

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    if (fgets(buf, sizeof(buf), fp) != NULL) {
        pch = strchr(buf,'\n');
        if(pch != NULL) {
            *pch = '\0';
        }

        if (strstr(buf, DIAG_RTN_PASS_STR) == NULL) {
            printf("Fail! Return Status is %s test failed.\n", buf);
            strcpy(nim_test_rlt, buf);
            if(strstr(nim_test_rlt, "Memory")) {
	        if (get_enhance_err_flag()) {
                    add_reva_mem_err_report();
                }
                prpass(testpass, "Reva module - Linux memory test, ");
                cterr('f', 0, "Main Memory Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "MAC_reg")) {
                if (get_enhance_err_flag()) {
                    add_reva_ge_mac_reg_err_report();
                }
                prpass(testpass, "Reva module - MAC register test, ");
                cterr('f', 0, "MAC Register Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "MAC_intr")) {
                if (get_enhance_err_flag()) {
                    add_reva_ge_mac_intr_err_report();
                }
                prpass(testpass, "Reva module - MAC interrupt test, ");
                cterr('f', 0, "MAC Interrupt Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "MAC_lpbk")) {
                if (get_enhance_err_flag()) {
                    add_reva_ge_mac_lpbk_err_report();
                }
                prpass(testpass, "Reva module - MAC loopback test, ");
                cterr('f', 0, "MAC Loopback Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "DMA_reg")) {
                if (get_enhance_err_flag()) {
                    add_reva_ge_dma_reg_err_report();
                }
                prpass(testpass, "Reva module - GE DMA register test, ");
                cterr('f', 0, "DMA Register Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "PHY_reg")) {
                if (get_enhance_err_flag()) {
                    add_reva_phy_reg_err_report();
                }
                prpass(testpass, "Reva module - PHY register test, ");
                cterr('f', 0, "PHY Register Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "PHY_int_lpbk")) {
                if (get_enhance_err_flag()) {
                    add_reva_phy_int_lpbk_err_report();
                }
                prpass(testpass, "Reva module - PHY internal loopback test, ");
                cterr('f', 0, "PHY Loopback Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "Margin")) {
                if (get_enhance_err_flag()) {
                    add_reva_margin_err_report();
                }
                prpass(testpass, "Reva module - Margin test, ");
                cterr('f', 0, "Margin Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "SCC")) {
                if (get_enhance_err_flag()) {
                    add_reva_async_scc_err_report();
                }
                prpass(testpass, "Reva module - Async Serial channel test (UART/PPP), ");
                cterr('f', 0, "Async Serial Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "INTERRUPT")) {
                if (get_enhance_err_flag()) {
                    add_interrupt_err_report();
                }
                prpass(testpass, "Reva module - Interrupt test, ");
                cterr('f', 0, "Interrupt Test Fails in run Reva test\n");
            } else if (strstr(nim_test_rlt, "LED")) {
                if (get_enhance_err_flag()) {
                    add_reva_led_err_report();
                }
                prpass(testpass, "Reva module - LED test, ");
                cterr('f', 0, "LED Test Fails in run Reva test\n");
            }
            if (strstr(nim_test_rlt, "ECC")) {
                if (get_enhance_err_flag()) {
                    add_reva_mem_err_report();
                }
                prpass(testpass, "Reva module - ECC occur, ");
                cterr('f', 0, "ECC occur in run Reva test\n");
            }
            fflush(stdout);
            fclose(fp);
            return (FAILED);
         } else {
            fclose(fp);
            return (PASSED);
         }
    }

    printf("Warning: status file is empty.\n");
    fclose(fp);

    return (-1);
}

/***************************************************************************
 *
 * Function: reva_kill_nc
 *
 * Description: This function lists all process and grep nc process,
 *              and dump their pids to a temporary to kill them
 *
 * Input:  None
 *
 * Output: None
 *
 ***************************************************************************
 */
static void reva_kill_nc (void)
{
    char cmd[128];
    char buf[128];
    char pid_file[32];
    char *token;
    int pid;
    FILE *fp;
    struct stat sts;

    fp = fopen(DIAG_KILL_NC_TMP_FILE, "w+");
    if (fp == NULL) {
        printf("%s: Open '%s' Failed\n", __FUNCTION__, DIAG_KILL_NC_TMP_FILE);
        return;
    }

    sprintf(cmd, "ps | grep 'nc 192.123.123 \\| nc -l -l -p' > %s", DIAG_KILL_NC_TMP_FILE);
    system(cmd);
    printf("\nkill cmd: %s\n", cmd);

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        /* separate string of one line, get 1st substring pointer */
        token = strtok(buf, " ");
        pid = atoi(token);
        /* Check if this process is still alive */
        sprintf(pid_file, "/proc/%d", pid);
        if (stat(pid_file, &sts) == -1) {
            /* Process doesn't exist */
            continue;
        }
        printf("Killing a nc process.\n"); 
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }

    fclose(fp);

}

/**********************************************************************
 * Function: reva_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void reva_cleanup (void)
{
    assert(reva_wic_iface);

    disable_bp_ge_lpbk();

    if (reva_saved_diag_exec) {
        pre_diag_exec = reva_saved_diag_exec;
        reva_saved_diag_exec = NULL;
    }
}

/**********************************************************************
 * Function: reva_uart_test
 *
 * Description: This function performs the uart interface test for the 
                NGVM
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int reva_uart_test (void)
{
    int port;
    char *ptr;
    char *tx_str = "n\n";
    char *exp_pattern = "Linux";
    char rx_str[64];
    int tx_len = strlen(tx_str);
    int rx_sz;
    int i;
    struct adapter_uart_t *padater_uart;
#ifdef TACHI
    const int maxlen = 28;
    char test_if[maxlen];
    int ret_val = PASSED;
#endif
    if (get_enhance_err_flag()) {
        add_reva_uart_test_err_report();
    } 

    testname("Reva Uart ");
    prpass(testpass, "Uart Test, ");
#ifdef TACHI
    diag_uart_to_nim_cnnt(reva_wic_iface->slot);
    snprintf(test_if, maxlen-1, UART_TTYS2_DEV);

    ret_val = uart_msg_exh_test(test_if, "diag\n", "Menuitem>",TRIG_DIAG_M);
    if (ret_val == FAILED) {
        cterr('f',0,"Reva UART test failed\n");
    }
    sleep(1);
    return (ret_val);
    printf("Reva UART test passed. \n");
#else
        port = reva_wic_iface->uart_ctrl;
#endif
    printf("UART port %d\n", port);
    memset(rx_str, 0, sizeof(rx_str));
    rx_sz = 0;

    if (reva_wic_iface->mod_type == DAUGHTER_CARD) {
        /* Switzer-carrier adapter card has its own control FPGA, use adapter Uart */
        /* utilities when NIM card is inserted into Switzer-carrier adapter card. */
        padater_uart = get_current_adapter_uart();
        padater_uart->adapter_uart_reset(port);
        padater_uart->adapter_uart_tx(port, 9600, tx_str, tx_len, 0);
        sleep(1);
        padater_uart->adapter_uart_rx(port, &rx_sz, rx_str);
        padater_uart->adapter_uart_reset(port);
    } else {
        dash_uart_reset(port);
        dash_uart_tx(port, 9600, tx_str, tx_len, 0);
        sleep(1);
        dash_uart_rx(port, &rx_sz, rx_str);
        dash_uart_reset(port);
    }
    ptr = memchr(rx_str, exp_pattern[0], strlen(rx_str));
    printf("rx_str = %s\n", rx_str);
    if (ptr == NULL) {
        cterr('f', 0, "tx/rx strings do not match: expected %s, got %s",
                exp_pattern, rx_str);
        return (FAILED);
    }
    for (i = 0; i < strlen(exp_pattern); i++) {
        if (ptr[i] != exp_pattern[i]) {
            cterr('f', 0, ".tx/rx strings do not match: expected %s, got %s",
                     exp_pattern, ptr);
            return (FAILED);
        }
    }

    printf("Reva UART test passed. \n");
    return (PASSED);
}

/*************************************************************************
 * Function: reva_iface_test
 *
 * Test entry for Reva interface test.
 *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int reva_iface_test ()
{
    uchar data;
    int i;

    assert(oir_if);

    /* Setup common parameters for new error message */
    cterr_pid(reva_slot);

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }
    printf("\nWait for Reva module side to boot up diag menu.\n");

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Reva module side when the diag menu is up. */
    for (i = 0; i < 2000; i++) {
	if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
            util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
	    cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	    return (FAILED);
	}

	if (data & 0x08)
	    break;

	msleep(200);
    }

    if (i == 2000) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }

    sleep(3);

    /* Testing UART and GE0 interfaces */
    if (reva_uart_test()) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
	return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 *
 * Function: reva_test().
 *
 * Description: This function is the entry point for Reva NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int reva_test (void *wic)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    assert(wic);
#ifdef TACHI
    const int maxlen = 28;
    char test_if[maxlen];
#endif
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;
    reva_wic_iface = (struct ngio_intf_t *)wic;

    slot = reva_slot = reva_wic_iface->slot;
    board_id = reva_wic_iface->id;

    /* Setup common parameters for new error message */
    cterr_pid(slot);

    reva_wic_iface->uart_on(wic);
    printf("\nreva_test, board_pid = %s, board_id %#x, slot %d\n", ngwic_get_pid, board_id, slot);

    testname("Slot%d Reva NGWIC ", slot);

    oir_if = (void *)(reva_wic_iface->oir);

    pca_i2c = reva_wic_iface->pca;

    if(is_reva_sm()) {
        if (tftp_get(0, (unsigned char *)SM_ASYNC_FW_IMG,
                     0, (unsigned char *)SM_ASYNC_FW_PATH, 1) < 0) {
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);

        }
    } else {
        if (tftp_get(0, (unsigned char *)NIM_ASYNC_FW_IMG,
                     0, (unsigned char *)NIM_ASYNC_FW_PATH, 1) < 0) {
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);

        }

        /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
         * not the ability of auto-negotiation between 1G and 10G */
        module_type = reva_wic_iface->mod_type;
        new_speed = NGIO_ETH_SPEED_1G;
        ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);
    }
    reva_wic_iface->unreset(wic);
    msleep(1000);
    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
#ifdef TACHI
    snprintf(test_if, maxlen-1, UART_TTYS2_DEV);
#endif    

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    reva_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                main_menu_secondary_items);

    if (reva_wic_iface->test_type == IFACE_TEST) {
    ret_val = reva_iface_test();
    } else {
        if (reva_wic_iface->menu_display == TRUE) {
            menu(maindiagp, main_menu_secondary_items, '\0');
        } else {
            do_all_menu_items(maindiagp);
        }
    }

    reva_cleanup();

    if (is_reva_nim()) {
        /* Curie 2RU: restore eth configuration */
        ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);
    }

    return (ret_val);
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

    testname("LTC4215 Hotswap ");
    prpass(testpass, "LTC4215 OIR Register, ");

    if (get_enhance_err_flag()) {
        add_reva_ltc4215_reg_err_report();
    }
    ret = oir_ltc4215_register_test(oir_if);
    if (ret == FAILED) {
        cterr('f',0,"LTC4215 register test failed.");
    }
    return (ret);
}

/**********************************************************************
 *
 * Function: ltc4215_util
 *
 * Description: LTC4215 Register Test/Read/Write utility.
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int ltc4215_util (void)
{
    uchar type = 'd';
    int stop = 0;
    uint8_t  data = 0;
    float current = 0.0;

    assert(oir_if);
    testname("LTC4215 Hotswap ");

    if (get_enhance_err_flag()) {
        add_reva_ltc4215_reg_err_report();
    } 

    printf("\nLTC4215 Hotswap Utility\n"); 

    while (1) {
        printf("\na: LTC4215 Register Read\n");
        printf("b: LTC4215 Register Write\n");
        printf("c: show 12V current consumption\n");
        printf("d: exit\n");
        type = getc_answer("Select an option", "abcd", 'd');
        switch(type) {
        case 'a':
            util_oir_ltc4215_reg_read(oir_if);
            break;
        case 'b':
            util_oir_ltc4215_reg_write(oir_if);
            break;
        case 'c':
            oir_ltc4215_reg_read(oir_if, LTC4215_SENSE_REG, &data);
            current = OIR_SENSE_RES * data / OIR_RES;
            printf("Sense Voltage Register divided by R28: %f mA\n", current);
            break;
        case 'd':
            stop = 1;
            break;
        default:
            break;
        }
        if (stop) {
            break;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: reva_pwr_off
 *
 * Description: This function does all necessary configuration to power off.
 *              reset module, power off, i2c reset.
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int reva_pwr_off (void)
{
    uint8_t data = 0;
    int slot;

    assert(oir_if);
    assert(reva_wic_iface);

    slot = reva_wic_iface->slot;
    printf("\nPower Off the Reva NGWIC.\n");
#ifdef TACHI
    ngiowic_disable_intr (slot, NGIO_FLT_INTR); 
#elif TABEIL
    ngiowic_disable_intr (slot, NGIO_FLT_INTR); 
#elif NANOOK
    ngiowic_disable_intr (slot, NGIO_FLT_INTR);
#else
    /* disable power interrupt */
    if (reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        ngiosm_disable_intr (slot, NGIO_FLT_INTR);
    }
#endif
    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }
    reva_wic_iface->i2c_reset(reva_wic_iface);
    reva_wic_iface->off(reva_wic_iface);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: reva_pwr_off_util
 *
 * Description: This function is called for power-off utility.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int reva_pwr_off_util (void)
{
    uint8_t ans;

    assert(reva_wic_iface);

    testname("Reva power off ");
    if (get_enhance_err_flag()) {
        add_reva_ltc4215_reg_err_report();
    } 

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Reva NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    if (reva_pwr_off()) {
        return (FAILED);
    }

    return PASSED;
}


/***************************************************************************
 *
 * Function: reva_pwr_on
 *
 * Description: This function does all necessary configuration to power on.
 *              enable Reva NGWIC, power it on, take it out of reset.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 ***************************************************************************
 */
static int reva_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Reva NGWIC.\n");

    assert(oir_if);
    assert(reva_wic_iface);

    /* enable ngwic and take I2C out of reset */
    slot_i2c_unreset(reva_wic_iface, reva_wic_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power on NGWIC module */
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
        printf("FET CANNOT be Turned On.\n");
        return (FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return (FAILED);
    }

    printf("Waiting for Reva NGWIC to Power-Up.\n");
    msleep(2000);

    /* take Reva NGWIC out of reset */
    reva_wic_iface->unreset(reva_wic_iface);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
    reva_wic_iface->uart_on(reva_wic_iface);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: reva_pwr_on_util
 *
 * Description: This function is called for power-on utiliy.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int reva_pwr_on_util (void)
{
    assert(reva_wic_iface);

    testname("Reva power on ");
    if (get_enhance_err_flag()) {
        add_reva_ltc4215_reg_err_report();
    } 

    if(reva_pwr_on()) {
        return (FAILED);
    }

    printf("Reva NGWIC is powered up.\n");

   return (PASSED);
}

/**********************************************************************
 *
 * Function: reva_pwr_cycle_util
 *
 * Description: This function is called for Power Cycle utiliy.
 *              Do reva_pwr_off() and reva_pwr_on()
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int reva_pwr_cycle_util (void)
{
    uint8_t i, ans;

    if (get_enhance_err_flag()) {
        add_reva_pwr_cycle_err_report();
    } 

    printf("\n");
    printf("Power Cycle the Reva NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Reva is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (reva_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Reva NGWIC");
        return (FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (reva_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Reva NGWIC");
        return (FAILED);
    }

    return (PASSED);
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

    testname("PCA9557 register ");
    if (get_enhance_err_flag()) {
        add_reva_pca9557_test_err_report();
    }
 
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

    testname("PCA9557 register ");
    if (get_enhance_err_flag()) {
        add_reva_pca9557_test_err_report();
    }

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ %#x\n", offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : reva_ioe_reg_test
 *
 * Description: Wrapped function to do Reva IO Expander register test.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int reva_ioe_reg_test (void)
{
    uint32_t         ctr = 0, test_ctr = 0, total_reg_num = 0;
    uchar            orig_val = 0, test_data = 0, check_data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp = pca_i2c;

    reg_p = &pca9557_reg_tbl[0];
    total_reg_num = (sizeof(pca9557_reg_tbl) / sizeof(reg_info_t));

    testname("PCA9557 Register");
    prpass(testpass, "PCA9557 Register, ");

    if (get_enhance_err_flag()) {
        add_reva_pca9557_test_err_report();
    }

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
 * Function: pca9557_util
 *
 * Description: PCA9557 (GPIO expander) utility menu.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pca9557_util (void)
{
    uint8_t ans;

    assert(reva_wic_iface); 

    testname("PCA9557 register ");
    if (get_enhance_err_flag()) {
        add_reva_pca9557_test_err_report();
    } 

    printf("\nRead or Write pca9557 register? (r/w): ");
    ans = getchar();
    getchar();
    if (ans == 'r' || ans == 'R') {
        if (pca9557_reg_read()){
            return (FAILED);
        }
    } else if (ans == 'w' || ans == 'W') {
         if (pca9557_reg_write()){
            return (FAILED);
         }
    } else {
        printf("ABORT!\n");
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_ngwic_console
 *
 * Description: Set GPIO pin 4 as output pin. Set GPIO pin 4 to 0 
 *              for NGWIC console redirect.
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
 * Function: reva_console_switch
 *
 * Description: Switch Platform's console to reva console using picocom
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int reva_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(reva_wic_iface);
    assert(oir_if);

    if (get_enhance_err_flag()) {
        add_reva_console_switch_err_report();
    } 

    if (set_ngwic_console()) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
    }

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
#ifdef TACHI
    diag_uart_to_nim_cnnt(reva_wic_iface->slot);
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2");
#else
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             reva_wic_iface->uart_ctrl);
#endif
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: disable_bp_ge_lpbk
 *
 * Description: Disable Reva port of Backplane GESW loopback mode
 *
 * Input : None.
 *
 * Output: None.
 *
 **********************************************************************
 */
static void disable_bp_ge_lpbk ()
{
    int ge_port;

    assert(reva_wic_iface);
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        /* Goldbeach platform didn't have GESW */
        printf("\nGoldbeach and Curie 1RU/2RU Didn't Support GESW\n");
        return;
    }

#if defined(TABEIL)
    /* tabei-l platform didn't have gesw*/
    printf("\ntabei-l didn't support gesw\n");
    return;
#endif

#if defined(NANOOK)
    /* Nanook platform didn't have gesw*/
    printf("\nNanook didn't support gesw\n");
    return;
#endif

#ifdef TACHI
     ge_port = tachi_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else

    // When wic card is in sm adapter card or Reva-SM module
    if (reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        ge_port =  ovld_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGSM, 0);
    } else {
        ge_port = ovld_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGWIC, 0);
    }
#endif
    set_gesw_line_loopback(ge_port, 0);
}

/**********************************************************************
 *
 * Function: enable_bp_ge_lpbk
 *
 * Description: Enable Reva port of Backplane GESW loopback mode
 *
 * Input : None.
 *
 * Output: None.
 *
 **********************************************************************
 */
static void enable_bp_ge_lpbk ()
{
    int ge_port;

    assert(reva_wic_iface);
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        /* Goldbeach platform didn't have GESW */
        printf("\nGoldbeach and Curie 1RU/2RU Didn't Support GESW\n");
        return;
    }

#if defined(TABEIL)
    /* tabei-l platform didn't have gesw*/
    printf("\ntabei-l didn't support gesw\n");
    return;
#endif

#if defined(NANOOK)
    /* Nanook platform didn't have gesw*/
    printf("\nNanook didn't support gesw\n");
    return;
#endif

#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else

    // When wic card is in sm adapter card or Reva-SM module
    if (reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        ge_port =  ovld_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGSM, 0);
    } else {
        ge_port = ovld_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGWIC, 0);
    }
#endif
    set_gesw_line_loopback(ge_port, 1);
}

 /**********************************************************************
 *
 * Function: reva_bp_ge_test
 *
 * Description: This function provides tests for Reva port of 
 *              Backplane GESW
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int reva_bp_ge_test ()
{
    int ge_port;
    uchar type = 'e';
    int stop = 0;
    int state = -1;

    assert(reva_wic_iface);

    // When wic card is in sm adapter card or Reva-SM module
#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else
    if (reva_wic_iface->mod_type == SM_DAUGHTER_CARD || reva_wic_iface->mod_type == SM_MODULE) {
        ge_port =  ovld_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGSM, 0);
    } else {
        ge_port = ovld_get_ge_sw_port_num(reva_wic_iface->slot, TGT_DEV_NGWIC, 0);
    }
    
    printf("\nReva Backplane GE Utility\n"); 
#endif
    while (1) {
        printf("\na: enable motherboard line loopback at Reva GESW port\n");
        printf("b: disable motherboard line loopback at Reva GESW port\n");
        printf("c: get Reva GESW port loopback setting\n");
        printf("d: Reva GESW port send package\n");
#ifndef TACHI
        printf("e: check GESW port initialized\n");
#endif
        printf("f: exit\n");
        type = getc_answer("Select an option", "abcdef", 'e');
        switch(type) {
        case 'a':
            enable_bp_ge_lpbk();
            break;
        case 'b':
            disable_bp_ge_lpbk();
            break;
        case 'c':
            state = get_gesw_line_loopback(ge_port);
            if (state) {
                printf("line loopback has been enabled.\n");
            } else {
                printf("line loopback has been disabled.\n");
            }
            break;
        case 'd':
            printf("Reva GE port is %d\n", ge_port);
            port_tx_util();
            break;
#ifndef TACHI
        case 'e':
            ovld_bcm_check_port_init();
            break;
#endif
        case 'f':
            stop = 1;
            break;
        default:
            break;
        }
        if (stop) {
            break;
        }
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: reva_reset
 *
 * Description: This function query for reset or unreset  Reva NGWIC module.
 *              It doesn't reset or unreset i2c.
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int reva_reset (void)
{
    uint8_t ans;

    assert(reva_wic_iface);

    printf("\nReset or Unreset Reva module? (r/u): ");
    ans = getchar();
    putchar(ans);
    printf("\n");
    if (ans == 'r' || ans == 'R') {
        if (reva_wic_iface->reset(reva_wic_iface)){
            printf("Unable to reset Reva Module\n");
            return (FAILED);
        }
        msleep(1000);
    } else if (ans == 'u' || ans == 'U') {
         if (reva_wic_iface->unreset(reva_wic_iface)){
            printf("Unable to unreset Reva Module\n");
            return (FAILED);
         }
        msleep(1000);
    } else {
        printf("ABORT!\n");
    }

    return (PASSED);
}

/**********************************************************************
*
* Function: reva_get_host_flag
*
* Description: Get current Host diag flags
*
* Input : none
*
* Output: none
*
**********************************************************************
*/
static void reva_get_host_flag (void)
{
    char flag_file[32];
    char flags[256];
    char cmd[256];

    /* Write flags to local file */
    sprintf(flag_file, "/tmp/host_flags");
    sprintf(flags, "diagflag=%x\tdiagflag_xram=%x",
        (unsigned int)(NVRAM)->diagflag, (unsigned int)diagflag_xram);
    sprintf(cmd, "echo %s > %s", flags, flag_file);

    system(cmd);
}

/**********************************************************************
*
* Function: reva_send_diag_flag
*
* Description: Send the Host diag flags to module side via nc command
*
* Input : none
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
static int reva_send_diag_flag (void)
{
    char flag_file[32];
    char nc_cmd[84];

    sprintf(flag_file, "/tmp/host_flags");

    /* HOST: send the flag */
    sprintf(nc_cmd, "nc %s 3013 < %s\n", REVA_LOCAL_IP_ADDR, flag_file);
    printf("HOST: nc command: %s\n", nc_cmd);

    if (system(nc_cmd)) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/******************************************************************************
 *
 * Function: is_reva_sm
 *
 * Description: According to sku id to judge whether this is sm sku or not.
 *
 * Inputs      : None
 *
 * Outputs     :TRUE/FALSE
 *
 *****************************************************************************/
int is_reva_sm (void)
{
    if (reva_wic_iface->id == SM_REVA_64A) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/******************************************************************************
 *
 * Function: is_reva_nim
 *
 * Description: According to sku id to judge whether this is sm sku or not.
 *
 * Inputs      : None
 *
 * Outputs     :TRUE/FALSE
 *
 *****************************************************************************/
int is_reva_nim (void)
{
    return (!is_reva_sm());
}

/******** History ********
$Log: ngwic_reva.c,v $
Revision 1.14  2020/05/22 02:28:23  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.13  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

Revision 1.12  2019/12/11 10:10:22  lucywang
Merged Nanook to main trunk

Revision 1.11  2019/10/17 02:16:15  kehuang2
Collapse Tabei-L into main trunk

Revision 1.10  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.9  2018/06/14 01:06:58  haohsu
Modify code for NIM REVA on Tachi platform

Revision 1.8  2018/06/12 01:41:41  haohsu
Add REVA NIM for TACHI platform

Revision 1.6  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.5  2017/03/16 10:55:48  umlin
Reva-SM: Commit Reva-SM platform side code to main trunk. RevaSM controller type is 0x0D77.

Revision 1.4  2016/10/16 12:28:12  iachang
Supported Goldbeach Platform.

Revision 1.3  2016/05/06 03:38:21  umlin
Reva: When Reva module ecc occur, platform side will cterr message

Revision 1.2  2016/04/26 02:15:42  umlin
Initial check-in for Reva.
Merge Reva to maintrunk.

$Endlog$
*/


