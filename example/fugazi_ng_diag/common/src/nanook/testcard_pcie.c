/* $Id: testcard_pcie.c,v 1.2 2019/12/11 10:10:36 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/testcard_pcie.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_pcie.c
 *
 * Description: Testcard PCIe related diag tests and utilities.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteswap.h"
#include "common.h"
#include "common_utils.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "slot.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "queryflags.h"

#include "defs.h"
#include "goofy_i2c.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "testcard_pcie.h"
#include "linux_api.h"
#include "dash_fpga.h"
#include "linux_pciutils.h" /* added for get_pcie_bus_num() function */

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_pcie_menu(int);
int  tc_read_pcie_redriver_reg(void);
int  tc_pcie_redrv_alter_util(void);
int  tc_pcie_8prbs_lpbk_test(void);

static void build_tc_pcie_utils(int);
static int  dump_pcie_redriver_reg(void);
static int  dump_tc_pcie_regs_util(void);


/*******************************************************************************
 *                                  Externs                                    *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int pcie_conf_read_util(void);
extern int pcie_conf_write_util(void);
extern int ovld_pcie_8prbs_ext_lpbk_test(uint32_t);
extern int plx_pcie_utp_ext_lpbk_test (uint32_t);

extern uint32_t tc_real_pcie_port;
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);
/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static n2g_i2c_if_t tc_pcie_i2c_if;

static reg_info_t pi2eqx5964_reg_tbl[]=
{
    {"Signal Detect (SIG)",                              SIG_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x00, 0x00},
    {"Receiver Detect Output (RX50)",                    RX50_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x00, 0x5A},
    {"Loopback and Emphasis Control (LBEC)",             LBEC_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFC, 0xFC},
    {"Channel Input Disable (INDIS)",                    INDIS_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0x00},
    {"Channel Output Disable (OUTDIS)",                  OUTDIS_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0x00},
    {"Channel Reset (RESET)",                            RESET_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0xFF},
    {"Power Down Control (PWR)",                         PWR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0xFF},
    {"Receiver Detect Enable (RXDETEN)",                 RXDETEN_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0xFF},
    {"A-Channels Equalizer and Output Control (AEOC)",   AEOC_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0xFF},
    {"B-Channels Equalizer and Output Control (BEOC)",   BEOC_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0xFF},
    {"Reserved",                                         RESV_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0x00},
    {"Idle Detect Threshold Control",                    IDL_DET_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF, 0xEF},
};


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * PCIe Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_pcie_diag_table[] = {
    {"PCIe Utilities",
     (PFT)build_tc_pcie_utils,      TRUE,
     0,                             (PFT)0, 0, (PFT)build_tc_pcie_utils, TRUE},
    {"PCIe 8-bit PRBS Master loopback test(loopback at TestCard)",
     (PFT)tc_pcie_8prbs_lpbk_test, FALSE,
     (MF_CONTINUOUS | MF_DOALL),    (PFT)0, 0, (PFT)0,                   0},
    {"PCIe UTP Master loopback test(loopback at TestCard)",
     (PFT)tc_pcie_8prbs_lpbk_test, FALSE,
     (MF_CONTINUOUS | MF_DOALL),    (PFT)0, 0, (PFT)0,                   0},
};

#define TC_PCIE_DIAG_TABLE_SIZE (sizeof(tc_pcie_diag_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_pcie_diag_pri_items[TC_PCIE_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_pcie_diag_sec_items[TC_PCIE_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_pcie_diag = {
    "TestCard PCIe SubMenu",       /* title */
    0,                             /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,         /* shows major flags */
    0,                             /* generic prompt */
    0,                             /* size -- bumped by add_menu_item() */
    tc_pcie_diag_pri_items,
};

static struct menuinfo *tc_pcie_diag_p = &tc_pcie_diag;


/*
 * TestCard PCIe Utilities SubMenu
 */
static submenu_xtable_t pcie_utils_tbl[] = {
    {"Read PCIe config. space register",  (PFT)pcie_conf_read_util,        FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Alter PCIe config. space register", (PFT)pcie_conf_write_util,       FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Dump PCIe all registers",           (PFT)dump_tc_pcie_regs_util,     FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Read TestCard FPGA register",       (PFT)tc_read_fpga_reg_wrap,      FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Alter TestCard FPGA register",      (PFT)tc_alter_fpga_reg_wrap,     FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Read PCIe ReDriver register",       (PFT)tc_read_pcie_redriver_reg,  FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Alter PCIe ReDriver register",      (PFT)tc_pcie_redrv_alter_util,   FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Dump all PCIe ReDriver registers",  (PFT)dump_pcie_redriver_reg,     FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
};

#define PCIE_UTILS_TBL_SIZE (sizeof(pcie_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_pcie_utils_pri_items[PCIE_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_pcie_utils_sec_items[PCIE_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_pcie_utils = {
    "TestCard PCIe Utilities",       /* title */
    0,                               /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,           /* shows major flags */
    0,                               /* generic prompt */
    0,                               /* size -- bumped by add_menu_item() */
    tc_pcie_utils_pri_items,
};

static struct menuinfo *tc_pcie_utils_p = &tc_pcie_utils;


/*******************************************************************************
 *
 * Function   : build_tc_pcie_menu
 * Description: Build TestCard PCIe Tests and Utilities SubMenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_pcie_menu (int submenu)
{
    build_primary_submenu(tc_pcie_diag_table, TC_PCIE_DIAG_TABLE_SIZE,
                          "TestCard PCIe SubMenu", &tc_pcie_diag_p);
    build_secondary_submenu(tc_pcie_diag_table, TC_PCIE_DIAG_TABLE_SIZE,
                            tc_pcie_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_pcie_diag, tc_pcie_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_pcie_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   : build_tc_pcie_utils
 * Description: Build TestCard PCIe related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_pcie_utils (int submenu)
{
    build_primary_submenu(pcie_utils_tbl, PCIE_UTILS_TBL_SIZE,
                          "TestCard PCIe Utils SubMenu", &tc_pcie_utils_p);
    build_secondary_submenu(pcie_utils_tbl, PCIE_UTILS_TBL_SIZE,
                            tc_pcie_utils_sec_items);

    menu(&tc_pcie_utils, tc_pcie_utils_sec_items, 0);
}



/*******************************************************************************
 *
 * Function   : pcie_dump_port_regs_util
 * Description: Function to dump specific PCIe port all registers.
 * Inputs     : sw_port - Switch port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pcie_dump_port_regs_util (uint32_t sw_port)
{
#if 0
    uint32_t reg_val = 0, reg_off = 0;

    printf("\nPCIe Switch Port %d registers:\n", sw_port);

    for (reg_off = 0; reg_off < 0xFFF; reg_off += sizeof(uint32_t)) {
        reg_val = 0;
        if (pes16nt16g2_global_addr_rd(sw_port, reg_off, &reg_val) != PASSED) {
            printf("%s: Failed to read Register 0x%03X.\n",
                   __FUNCTION__, reg_off);
            return (FAILED);
        } else {
            printf("Offset 0x%03X: 0x%08X.\n", reg_off, reg_val);
        }
    }
#endif

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	pi2eqx5964_reg_rd
 * Description:	Utility to read TestCard PCIe ReDriver, PI2EQX5964, register.
 * Inputs     :	data - to put the read back register value.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pi2eqx5964_reg_rd (char *data)
{
    int result = FAILED;

    /* Setup I2C API parameter struct */
    /* 1. To get TestCard common I2C structure */
    get_tc_i2c_struct(&tc_pcie_i2c_if);

    /* 2. To set TestCard FPGA read specific parameters */
    tc_pcie_i2c_if.i2c_dev = TC_PCIE_REDRIVER_I2C_ADDR;
    tc_pcie_i2c_if.buf = data;
    tc_pcie_i2c_if.offset = 0;
    tc_pcie_i2c_if.size = PI2EQX5964_REG_SIZE;

    result = n2g_i2c_read(&tc_pcie_i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to read data */
        printf("%s: Unable to read. rc = 0x%08x", __FUNCTION__, result);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pi2eqx5964_reg_wr
 * Description:	Utility to write TestCard PCIe ReDriver, PI2EQX5964, register.
 * Inputs     :	data - to put the read back register value.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pi2eqx5964_reg_wr (char *data)
{
    int result = FAILED;

    /* Setup I2C API parameter struct */
    /* 1. To get TestCard common I2C structure */
    get_tc_i2c_struct(&tc_pcie_i2c_if);

    /* 2. To set TestCard FPGA read specific parameters */
    tc_pcie_i2c_if.i2c_dev = TC_PCIE_REDRIVER_I2C_ADDR;
    tc_pcie_i2c_if.buf = data;
    tc_pcie_i2c_if.offset = 0;
    tc_pcie_i2c_if.size = PI2EQX5964_REG_SIZE;

    result = n2g_i2c_write(&tc_pcie_i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to write data */
        printf("%s: Unable to write. rc = 0x%08x", __FUNCTION__, result);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_read_pcie_redriver_reg
 * Description:	Utility to read TestCard PCIe ReDriver register.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_read_pcie_redriver_reg (void)
{
    char      reg_val[PI2EQX5964_REG_SIZE];
    char      *reg_val_p;
    uint       choice = 0;
    reg_info_t *reg_ptr;


    /* Get Register byte number from user */
    choice = getdec_answer("Enter the Byte number of Reg. you want to read:",
                           SIG_REG_OFFSET, SIG_REG_OFFSET, IDL_DET_REG_OFFSET);

    reg_ptr = &pi2eqx5964_reg_tbl[choice];

    /* Read Registers value back */
    reg_val_p = &reg_val[0];

    if (pi2eqx5964_reg_rd(reg_val_p) != PASSED) {
        return (FAILED);
    }

    /* Display value of the choosen register */
    printf("Byte%2d: %s Reg.: 0x%02X\n",
           choice, reg_ptr->name, reg_val[choice]);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_alter_pcie_redriver_reg
 * Description: Function to alter specific TestCard PCIe ReDriver register.
 * Inputs     :	reg_off - offset of register want to change
 *              data_in - the value want to write in
 *              msg_opt - parameter to determine print message or not
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_alter_pcie_redriver_reg (uint reg_off, uint8_t data_in, boolean msg_opt)
{
    char      reg_val[PI2EQX5964_REG_SIZE], val_now = 0;
    char      *reg_val_p;
    reg_info_t *reg_ptr;


    /* Got the expected reg. num, check if it writable again */
    reg_ptr = &pi2eqx5964_reg_tbl[reg_off];

    if (reg_ptr->type & READ_ONLY) {
        /* read only */
        printf("Sorry !!! Byte%2d: %s is a Read-Only Register.\n",
               reg_off, reg_ptr->name);
        return (FAILED);
    }

    /* Read Registers value back */
    reg_val_p = &reg_val[0];

    memset(reg_val_p, 0, PI2EQX5964_REG_SIZE);

    if (pi2eqx5964_reg_rd(reg_val_p) != PASSED) {
        printf("%s:%d Failed to read PCIe ReDriver registers.\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    val_now = reg_val[reg_off];

    data_in &= reg_ptr->mask;

    if (msg_opt == ENABLE) {
        printf("Mask of Byte%2d: %s Reg. is 0x%02X,\n",
               reg_off, reg_ptr->name, reg_ptr->mask);
        printf("so the actual write-in data = 0x%02X.\n", data_in);
    }

    reg_val[reg_off] = data_in;
    if (pi2eqx5964_reg_wr(reg_val_p) != PASSED) {
        printf("%s:%d Failed to wrote 0x%02X to Test Card "
               "PCIe ReDriver Reg Byte%2d.\n",
               __FUNCTION__, __LINE__, data_in, reg_off);
    }

    /* Re-read Registers value back */
    memset(reg_val_p, 0, PI2EQX5964_REG_SIZE);

    if (pi2eqx5964_reg_rd(reg_val_p) != PASSED) {
        printf("%s:%d Failed to re-read PCIe ReDriver registers for confirm.\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    val_now = reg_val[reg_off];

    if (val_now != data_in) {
        printf("%s:%d Failed !! Write-in data is not match to Read-back.\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if (msg_opt == ENABLE) {
        printf("Successfully to alter register !!\n");

        /* Display value of the choosen register */
        printf("Now Byte%2d: %s Reg.: 0x%02X\n",
               reg_off, reg_ptr->name, reg_val[reg_off]);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_pcie_redrv_alter_util
 * Description:	Utility to alter TestCard PCIe ReDriver register.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_pcie_redrv_alter_util (void)
{
    uint8_t    write_in = 0;
    uint       byte_ctr = 0, choice = 0, ref_val = 0;
    uint       start_num = PI2EQX5964_REG_SIZE, end_num = 0;
    reg_info_t *reg_ptr;


    /* List all read/writable Registers */
    printf("\nAll Read/Writable Reg. list:\n");
    for (byte_ctr = 0, reg_ptr = &pi2eqx5964_reg_tbl[0];
         byte_ctr < PI2EQX5964_REG_SIZE;
         byte_ctr++, reg_ptr++) {
        if (!(reg_ptr->type & READ_ONLY)) {
            if (byte_ctr <= start_num) {
                start_num = byte_ctr;
            }

            if (byte_ctr > end_num) {
                end_num = byte_ctr;
            }

            /* Read writeable */
            printf("Byte%2d: %-46s Reg.\n", byte_ctr, reg_ptr->name);
        }
    }

    if (end_num < start_num) {
        printf("\nSorry !! There's no Read/Writable Reg. here.\n");
        return (PASSED);
    }

    choice = getdec_answer("Enter byte number of register want to alter:",
                            start_num, start_num, end_num);

    ref_val = pi2eqx5964_reg_tbl[choice].reset_val;
    write_in = gethex_answer("Enter the 8-bit data:", ref_val, 0, 0xFF);

    /* Alter the register with the value that user defined */
    if (tc_alter_pcie_redriver_reg(choice, write_in, ENABLE) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	dump_pcie_redriver_reg
 * Description:	Utility to dump TestCard PCIe ReDriver all registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_pcie_redriver_reg (void)
{
    char      reg_val[PI2EQX5964_REG_SIZE];
    char      *reg_val_p;
    uint       byte_ctr = 0;
    reg_info_t *reg_ptr;


    /* Read Registers value back */
    reg_val_p = &reg_val[0];

    if (pi2eqx5964_reg_rd(reg_val_p) != PASSED) {
        return (FAILED);
    }

    /* Dump PCIe ReDriver, PI2EQX5964, all registers */
    printf("\nPCIe ReDriver, PI2EQX5964, registers:\n");

    for (byte_ctr = 0, reg_ptr = &pi2eqx5964_reg_tbl[0];
         byte_ctr < PI2EQX5964_REG_SIZE;
         byte_ctr++, reg_ptr++) {
        printf("Byte%2d: %-46s Reg.: 0x%02X\n",
               byte_ctr, reg_ptr->name, reg_val[byte_ctr]);
    }

    return (PASSED);
}


#if 0
/*******************************************************************************
 *
 * Function   :	set_pcie_redrv_lpbk
 * Description:	Function to set PCIe interface of TestCard, here will do
 *              10-bit PRBS Master loopback test (Host->TestCard->Host).
 * Inputs     :	lane_no - number of lane
 *              opt     - to determine ENABLE/DISABLE loopback mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int set_pcie_redrv_lpbk (uint8_t lane_no, boolean opt)
{
    char      reg_val[PI2EQX5964_REG_SIZE];
    char      *reg_val_p;

    /* Read Registers value back */
    reg_val_p = &reg_val[0];

    if (pi2eqx5964_reg_rd(reg_val_p) != PASSED) {
        return (FAILED);
    }

    switch (lane_no) {
    case PCIE_LANE0:
        if (opt == ENABLE) {
            reg_val[LBEC_REG_OFFSET] &= (~LB_A0B0);
            reg_val[INDIS_REG_OFFSET] &= (~INDIS_A0);
            reg_val[INDIS_REG_OFFSET] |= INDIS_B0;
            reg_val[OUTDIS_REG_OFFSET] |= ODIS_A0;
            reg_val[OUTDIS_REG_OFFSET] &= (~ODIS_B0);
        } else {
            reg_val[LBEC_REG_OFFSET] |= LB_A0B0;
            reg_val[INDIS_REG_OFFSET] &= (~(INDIS_A0 | INDIS_B0));
            reg_val[OUTDIS_REG_OFFSET] &= (~(ODIS_A0 | ODIS_B0));
        }
        break;
    case PCIE_LANE1:
        if (opt == ENABLE) {
            reg_val[LBEC_REG_OFFSET] &= (~LB_A1B1);
            reg_val[INDIS_REG_OFFSET] &= (~INDIS_A1);
            reg_val[INDIS_REG_OFFSET] |= INDIS_B1;
            reg_val[OUTDIS_REG_OFFSET] |= ODIS_A1;
            reg_val[OUTDIS_REG_OFFSET] &= (~ODIS_B1);
        } else {
            reg_val[LBEC_REG_OFFSET] |= LB_A1B1;
            reg_val[INDIS_REG_OFFSET] &= (~(INDIS_A1 | INDIS_B1));
            reg_val[OUTDIS_REG_OFFSET] &= (~(ODIS_A1 | ODIS_B1));
        }
        break;
    case PCIE_LANE2:
        if (opt == ENABLE) {
            reg_val[LBEC_REG_OFFSET] &= (~LB_A2B2);
            reg_val[INDIS_REG_OFFSET] &= (~INDIS_A2);
            reg_val[INDIS_REG_OFFSET] |= INDIS_B2;
            reg_val[OUTDIS_REG_OFFSET] |= ODIS_A2;
            reg_val[OUTDIS_REG_OFFSET] &= (~ODIS_B2);
        } else {
            reg_val[LBEC_REG_OFFSET] |= LB_A2B2;
            reg_val[INDIS_REG_OFFSET] &= (~(INDIS_A2 | INDIS_B2));
            reg_val[OUTDIS_REG_OFFSET] &= (~(ODIS_A2 | ODIS_B2));
        }
        break;
    case PCIE_LANE3:
        if (opt == ENABLE) {
            reg_val[LBEC_REG_OFFSET] &= (~LB_A3B3);
            reg_val[INDIS_REG_OFFSET] &= (~INDIS_A3);
            reg_val[INDIS_REG_OFFSET] |= INDIS_B3;
            reg_val[OUTDIS_REG_OFFSET] |= ODIS_A3;
            reg_val[OUTDIS_REG_OFFSET] &= (~ODIS_B3);
        } else {
            reg_val[LBEC_REG_OFFSET] |= LB_A3B3;
            reg_val[INDIS_REG_OFFSET] &= (~(INDIS_A3 | INDIS_B3));
            reg_val[OUTDIS_REG_OFFSET] &= (~(ODIS_A3 | ODIS_B3));
        }
        break;
    default:
        printf("%s:%d Invalid Lane number = %d.\n",
               __FUNCTION__, __LINE__, lane_no);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (opt == ENABLE) {
            printf("\nSet ");
        } else {
            printf("\nUnset ");
        }
        printf("Lane %d Loopback mode:\n", lane_no);
        printf("LB_A%dB%d = %d\n", lane_no, lane_no, 
               ((reg_val[LBEC_REG_OFFSET] & (LB_A0B0 >> lane_no)) 
                >> (LB_A0B0_SFT - lane_no)));
        printf("INDIS_A%d = %d\n", lane_no,
               ((reg_val[INDIS_REG_OFFSET] & (INDIS_A0 >> (lane_no * 2)))
                >> (INDIS_A0 - (lane_no * 2))));
        printf("OUTDIS_A%d = %d\n", lane_no,
               ((reg_val[OUTDIS_REG_OFFSET] & (ODIS_A0 >> (lane_no * 2)))
                >> (ODIS_A0 - (lane_no * 2))));
        printf("INDIS_B%d = %d\n", lane_no,
               ((reg_val[INDIS_REG_OFFSET] & (INDIS_B0 >> (lane_no * 2)))
                >> (INDIS_B0 - (lane_no * 2))));
        printf("OUTDIS_B%d = %d\n", lane_no,
               ((reg_val[OUTDIS_REG_OFFSET] & (ODIS_B0 >> (lane_no * 2)))
                >> (ODIS_B0 - (lane_no * 2))));
    }

    if (pi2eqx5964_reg_wr(reg_val_p) != PASSED) {
        return (FAILED); 
    }

    return (PASSED); 
}
#endif

#if 0
/*****************************************************************************
 *
 * Function   : is_tc_pcie_tested
 * Description: Check if the NGIO test card PCIe PRBS test has been run.
 *              This test can only be run once due to IDT PCIe switch
 *              PRBS issue. See HW CDET CSCud23263 for details.
 *
 * Inputs     : tc_name - NGIO test card name
 *              slot - NGIO slot number
 *              
 * Outputs    : TRUE/FALSE
 *
 *****************************************************************************/
static int
is_tc_pcie_tested (char *tc_name, int slot)
{
    char fname[32], cmd[32];
    size_t size = 0;

    sprintf(fname, "%s%d_pcie_tested", tc_name, slot);
    if (file_exist(fname, &size)) {
        return(TRUE);
    }
    else {
        sprintf(cmd, "echo > %s;", fname);
	system(cmd);
	return(FALSE);
    }
}

#endif

/*******************************************************************************
 *
 * Function   :	tc_pcie_8prbs_lpbk_test
 * Description:	Function to verifiy PCIe interface of TestCard, here will do
 *              8-bit PRBS Master loopback test (Host->TestCard->Host).
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_pcie_8prbs_lpbk_test (void)
{
#if 0
    uint8_t test_lane = 0;
    char    reg_val[PI2EQX5964_REG_SIZE];
    char    *reg_val_p;
    int     main_result = FAILED;
    uint32_t port0_bus_num;
    int retry = 0;

    testname("TestCard PCIe 8-bit PRBS Master Loopback (at TestCard)");

    /* Fix for CSCud23263: IDT PCIe PRBS loopback test can only be run
     * once after each power up.
     */
    if (is_tc_pcie_tested(testcard_if_p->type_name, testcard_if_p->slot) == TRUE) {
        prpass(testpass, "%s%d", testcard_if_p->type_name, testcard_if_p->slot);
        printf("skipped after ran once");
        return (PASSED);
    }

    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    /* Assign bus number of PLX port 0 for different platform  */
    if (is_utah_plx()) {
        port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8618);
    } else if (is_sword()) {
        port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8617);
    } else if (is_dagger()) {
        port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8604);
    } else if (is_ntpn_machines() || is_vg450()) { 
        printf("\nNeptune Pericom PCIe switch cannot generate PRBS signal \n"); 
        return (PASSED);
    }

    /* force pcie speed to Gen1 2.5Gbps 
     * to fix sm testcard pcie lpbk test error 
     */
    if (is_utah_plx() || is_sword() || is_dagger()) {
        pcie_config_write(0, port0_bus_num + 1, tc_real_pcie_port, 0, PLX_LNK_STA_CTRL, 0x1);
    }

    /* Read PCIe reDriver setting for recovery after test */
    reg_val_p = &reg_val[0];

    if (pi2eqx5964_reg_rd(reg_val_p) != PASSED) {
        cterr('f', 0, "%s: Failed to read PCIe reDriver registers.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* 1. Put the PCIe re-Driver chip in analog loopback mode
     *    by setting bit 0 of TestCard FPGA PCIe loopback Reg.(0x12h)
     */
    if (tc_set_fpga_reg(PCIE_LPBK_REG_OFFSET, PCIE_HOST_LPBK, ENABLE)
        != PASSED) {
        cterr('f', 0, "%s: Failed to set PCIe reDriver "
                      "in analog loopback mode.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* 2. Set the mini. Equalization and De-Emphasis and
     *    keep the output voltage swing at 1.0V for loopback mode.
     *    (by setting Byte 8 and 9 of PCIe reDriver to 0x03h)
     */
    if (tc_alter_pcie_redriver_reg(AEOC_REG_OFFSET, TC_PCIE_LPBK_OVSWING,
                                   DISABLE) != PASSED) {
        cterr('f', 0, "%s: Failed to set PCIe reDriver reg. "
                      "Byte%2d to 0x%02X.",
                      __FUNCTION__, AEOC_REG_OFFSET, TC_PCIE_LPBK_OVSWING);
        return (FAILED);
    }

    if (tc_alter_pcie_redriver_reg(BEOC_REG_OFFSET, TC_PCIE_LPBK_OVSWING,
                                   DISABLE) != PASSED) {
        cterr('f', 0, "%s: Failed to set PCIe reDriver reg. "
                      "Byte%2d to 0x%02X.",
                      __FUNCTION__, BEOC_REG_OFFSET, TC_PCIE_LPBK_OVSWING);
        return (FAILED);
    }

    /* 3. Set all tested PCIe re-Driver lane(s) into Loopback mode 
     *    by setting related registers of PCIe re-Driver.
     *    (bit[7:4] of byte2, LBEC; bit[7:0] of byte3, INDIS 
     *     and byte 4, OUTDIS)
     */
    if (testcard_if_p->type == TC_NGWIC) {
        /* Based on HW's info, NGWIC TestCard is a x1 model,
         * and Lane 0 of PCIe Swicth NGWIC port(port 8 & 10) is
         * connected to NGWIC TestCard Lane 1.
         * So only Enable Lane 1 loopback mode of PCIe reDriver.
         */
        if (set_pcie_redrv_lpbk(PCIE_LANE1, ENABLE) != PASSED) {
            cterr('f', 0, "%s: Failed to set PCIe reDriver Lane %d "
                          "to loopback mode.",
                          __FUNCTION__, PCIE_LANE1);
            return (FAILED);
        }
    } else if (testcard_if_p->type == TC_NGSM) {
        for (test_lane = 0; test_lane < TC_NGSM_PCIE_LANE_NUM; test_lane++) {
            if (set_pcie_redrv_lpbk(test_lane, ENABLE) != PASSED) {
                cterr('f', 0, "%s: Failed to set PCIe reDriver Lane %d "
                              "to loopback mode.",
                              __FUNCTION__, test_lane);
                return (FAILED);
            }
        }
    } else {
        cterr('f', 0, "%s: Unknown TestCard Type.", __FUNCTION__);
        return (FAILED);
    }

    /* 4. Run PCIe 8bit PRBS Master loopback test from Host side. */
    /* check swtich id */
    if (is_juno_plx() || is_utah_plx() || is_sword() || is_dagger()) {
       /* 0x10b5 for vendor id (PLX)
        * 0x8618 for device id (PEX 8618) */
        main_result = plx_pcie_utp_ext_lpbk_test (tc_real_pcie_port); 
    } else {
        main_result = ovld_pcie_8prbs_ext_lpbk_test(tc_real_pcie_port);
    }

    if (is_utah_plx() || is_sword() || is_dagger()) {
        if (main_result != PASSED) {
            for (retry = 0; retry < 4 ; retry++) {
                printf("\nReset PLX...\n");
                reset_platform_ext_dev(0x4);
                sleep(5);
                unreset_platform_ext_dev(0x4);
                sleep(5);

                /* after reset plx pcie switch, rescan pcie bus 
                 * to fix the issue that once plx pcie switch is reset,
                 * the New NIM test card pcie linkup will fail.
                 */
                system("echo 1 > /sys/bus/pci/rescan");       
                sleep(1);

                printf("\nRerun loopback test...\n");
                main_result = plx_pcie_utp_ext_lpbk_test (tc_real_pcie_port);
                if (main_result == PASSED) {
                    break ;
                }
            }
            if (main_result != PASSED) {
                cterr('f', 0, "%s: Failed to run 10 PRBS external loopback test.",
                              __FUNCTION__);
            }
        }
    } else {
        if (main_result != PASSED) {
            cterr('f', 0, "%s: Failed to run 10 PRBS external loopback test.",
                          __FUNCTION__);
        }
    }

    /* 5. Recover the setting of PCIe re-Driver. */
    if (pi2eqx5964_reg_wr(reg_val_p) != PASSED) {
        if (main_result != PASSED) {
            printf("%s: Failed to recover PCIe reDriver registers.\n",
                   __FUNCTION__);
        } else {
            cterr('f', 0, "%s: Failed to recover PCIe reDriver registers.",
                          __FUNCTION__);

            main_result = FAILED;
        }
    }

    /* 6. Put the PCIe re-Driver chip back to normal mode
     *    by unset bit 0 of TestCard FPGA PCIe loopback Reg.(0x12h)
     */
    if (tc_set_fpga_reg(PCIE_LPBK_REG_OFFSET, PCIE_HOST_LPBK, DISABLE)
        != PASSED) {
        if (main_result != PASSED) {
            printf("%s: Failed to set PCIe reDriver "
                   "back to normal mode.",
                   __FUNCTION__);
        } else {
            cterr('f', 0, "%s: Failed to set PCIe reDriver "
                          "back to normal mode.",
                          __FUNCTION__);
            main_result = FAILED;
        }
    }

    return (main_result);
#endif
    return 0;
}


/*******************************************************************************
 *
 * Function   : dump_tc_pcie_regs_util
 * Description: Function to dump TestCard related PCIe port registers.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_tc_pcie_regs_util (void)
{
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: %s%d is connected to PCIe Switch port %d.\n",
               __FUNCTION__, testcard_if_p->type_name,
               testcard_if_p->slot, tc_real_pcie_port);
    }

    return (pcie_dump_port_regs_util(tc_real_pcie_port));
}


/*------------------------------------------------------------------
$Log: testcard_pcie.c,v $
Revision 1.2  2019/12/11 10:10:36  lucywang
Merged Nanook to main trunk


$Endlog$
*/

