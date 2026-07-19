/* $Id: platform_pcie_sw.c,v 1.4 2017/07/14 02:51:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pcie_sw.c,v $
 *------------------------------------------------------------------
 * Filename   : platform_pcie_sw.c
 *
 * Description: Overlord PCIe switch, IDT PES16NT16G2,
 *              related diag tests and utilities.
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
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
#include "i2c_address.h"
#include "slot.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "proto.h"
#include "defs.h"
#include "platform_pcie_sw.h"

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_pcie_sw_menu(int);
int  ovld_pcie_10prbs_cdr_int_lpbk_test(void);

static void build_pcie_sw_utils(int);
static int  pes16nt16g2_get_baseaddr(uint32_t, uint32_t *);
static int  pes16nt16g2_set_reg_by_mask(uint32_t, uint32_t, uint32_t, boolean);


/*******************************************************************************
 *                                    Externs                                  *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);

extern int pcie_conf_read_util(void);
extern int pcie_conf_write_util(void);
extern uint32_t get_ngio_pcie_bus_num(void);
extern boolean is_not_plx(void);


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * PCIe switch Tests and Utilities Main Menu
 */
static submenu_xtable_t pcie_sw_diag_table[] = {
    {"PCIe switch Utilities",
     (PFT)build_pcie_sw_utils,                  TRUE,
     0,                                         (PFT)0, 0,
     (PFT)build_pcie_sw_utils,                  TRUE},
};

#define PCIE_SW_DIAG_TABLE_SIZE (sizeof(pcie_sw_diag_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pcie_sw_diag_pri_items[PCIE_SW_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t pcie_sw_diag_sec_items[PCIE_SW_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo pcie_sw_diag = {
    "PCIe switch SubMenu",         /* title */
    0,                             /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,         /* shows major flags */
    0,                             /* generic prompt */
    0,                             /* size -- bumped by add_menu_item() */
    pcie_sw_diag_pri_items,
};

static struct menuinfo *pcie_sw_diag_p = &pcie_sw_diag;


/*
 * PCIe switch Utilities SubMenu
 */
static submenu_xtable_t pcie_sw_utils_tbl[] = {
    {"Read PCIe config. space register",  (PFT)pcie_conf_read_util,    FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Alter PCIe config. space register", (PFT)pcie_conf_write_util,   FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
};

#define PCIE_SW_UTILS_TBL_SIZE (sizeof(pcie_sw_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pcie_sw_utils_pri_items[PCIE_SW_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t pcie_sw_utils_sec_items[PCIE_SW_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo pcie_sw_utils = {
    "PCIe switch Utils SubMenu",     /* title */
    0,                               /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,           /* shows major flags */
    0,                               /* generic prompt */
    0,                               /* size -- bumped by add_menu_item() */
    pcie_sw_utils_pri_items,
};

static struct menuinfo *pcie_sw_utils_p = &pcie_sw_utils;


/*******************************************************************************
 *
 * Function   : build_pcie_sw_menu
 * Description: Build Overlord PCIe switch Diag Tests and Utilities SubMenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_pcie_sw_menu (int submenu)
{
    build_primary_submenu(pcie_sw_diag_table, PCIE_SW_DIAG_TABLE_SIZE,
                          "PCIe switch SubMenu", &pcie_sw_diag_p);
    build_secondary_submenu(pcie_sw_diag_table, PCIE_SW_DIAG_TABLE_SIZE,
                            pcie_sw_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&pcie_sw_diag, pcie_sw_diag_sec_items, 0);
    } else {
        do_all_menu_items(pcie_sw_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   : build_pcie_sw_utils
 * Description: Build Overlord PCIe switch related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_pcie_sw_utils (int submenu)
{
    build_primary_submenu(pcie_sw_utils_tbl, PCIE_SW_UTILS_TBL_SIZE,
                          "PCIe switch Utils SubMenu", &pcie_sw_utils_p);
    build_secondary_submenu(pcie_sw_utils_tbl, PCIE_SW_UTILS_TBL_SIZE,
                            pcie_sw_utils_sec_items);

    menu(&pcie_sw_utils, pcie_sw_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_global_addr_rd
 * Description:	Function to read Overlord PCIe switch (IDT PES16NT16G2)
 *              register by using global address space access.
 * Inputs     :	port_num - Number of the accessed port
 *              reg_off  - Offset of the register that want to read
 *              reg_val  - buffer to put read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_global_addr_rd (uint32_t port_num, uint32_t reg_off,
                                       uint32_t *reg_val)
{
    uint32_t base_addr = 0, data = 0;
    uint32_t bus;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s: port_num = %d, reg_off = %#x.\n",
               __FUNCTION__, port_num, reg_off);
    }

    /* Get base address by port */
    if (pes16nt16g2_get_baseaddr(port_num, &base_addr) != PASSED) {
        printf("%s: Failed to get port %d's base address.\n",
               __FUNCTION__, port_num);
        return (FAILED);
    }

    data = (base_addr + reg_off);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: data = %#x.\n", __FUNCTION__, data);
    }

    /* 1. Write Address to Global Address Space Access Address Reg (0xFF8) */
    bus = get_ngio_pcie_bus_num();
    pcie_config_write(0, bus, 0, 0, GASAADDR_OFF, data);

    /* 2. Read data from Global Address Space Access Data Reg (0xFFC) */
    *reg_val = pcie_config_read(0, bus, 0, 0, GASADATA_OFF);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_global_addr_wr
 * Description:	Function to write Overlord PCIe switch (IDT PES16NT16G2)
 *              register by using global address space access.
 * Inputs     :	port_num - Number of the accessed port
 *              reg_off   - Offset of the register that want to read
 *              data_in   - value of data that want to write in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_global_addr_wr (uint32_t port_num, uint32_t reg_off,
                                       uint32_t data_in)
{
    uint32_t base_addr = 0;
    uint32_t bus;

    /* Get base address by port */
    if (pes16nt16g2_get_baseaddr(port_num, &base_addr) != PASSED) {
        printf("%s: Failed to get port %d's base address.\n",
               __FUNCTION__, port_num);
        return (FAILED);
    }

    bus = get_ngio_pcie_bus_num();

    /* 1. Write Address to Global Address Space Access Address Reg (0xFF8) */
    pcie_config_write(0, bus, 0, 0, GASAADDR_OFF, (base_addr + reg_off));

    /* Read data back to confirm the write action is completed successfully */
    pcie_config_read(0, bus, 0, 0, GASAADDR_OFF);

    /* 2. Write data to Global Address Space Access Data Reg (0xFFC) */
    pcie_config_write(0, bus, 0, 0, GASADATA_OFF, data_in);

    /* Read data back to confirm the write action is completed successfully */
    pcie_config_read(0, bus, 0, 0, GASADATA_OFF);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_get_baseaddr
 * Description:	Function to get base address of each port.
 * Inputs     :	port_num  - the number of port
 *              base_addr - buffer to put the get back base address 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_get_baseaddr (uint32_t port_num, uint32_t *base_addr)
{
    switch (port_num) {
    case PCIE_SW_P0:
        *base_addr = P0_CFG_ADDR;
        break;
    case PCIE_SW_P1:
        *base_addr = P1_CFG_ADDR;
        break;
    case PCIE_SW_P2:
        *base_addr = P2_CFG_ADDR;
        break;
    case PCIE_SW_P3:
        *base_addr = P3_CFG_ADDR;
        break;
    case PCIE_SW_P4:
        *base_addr = P4_CFG_ADDR;
        break;
    case PCIE_SW_P5:
        *base_addr = P5_CFG_ADDR;
        break;
    case PCIE_SW_P6:
        *base_addr = P6_CFG_ADDR;
        break;
    case PCIE_SW_P7:
        *base_addr = P7_CFG_ADDR;
        break;
    case PCIE_SW_P8:
        *base_addr = P8_CFG_ADDR;
        break;
    case PCIE_SW_P9:
        *base_addr = P9_CFG_ADDR;
        break;
    case PCIE_SW_P10:
        *base_addr = P10_CFG_ADDR;
        break;
    case PCIE_SW_P11:
        *base_addr = P11_CFG_ADDR;
        break;
    case PCIE_SW_P12:
        *base_addr = P12_CFG_ADDR;
        break;
    case PCIE_SW_P13:
        *base_addr = P13_CFG_ADDR;
        break;
    case PCIE_SW_P14:
        *base_addr = P14_CFG_ADDR;
        break;
    case PCIE_SW_P15:
        *base_addr = P15_CFG_ADDR;
        break;
    case PCIE_SW_P16:
        *base_addr = P16_CFG_ADDR;
        break;
    case PCIE_SW_P17:
        *base_addr = P17_CFG_ADDR;
        break;
    case PCIE_SW_P18:
        *base_addr = P18_CFG_ADDR;
        break;
    case PCIE_SW_P19:
        *base_addr = P19_CFG_ADDR;
        break;
    default:
        printf("%s:%d Invalid Port number: %d.\n",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_enter_test_mode
 * Description:	Function to let Overlord PCIe switch (IDT PES16NT16G2)
 *              to enter Test Mode.
 * Inputs     :	port_num - the number of port that will be set to Test Mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_enter_test_mode (uint32_t port_num, uint32_t lpbk_type)
{
    uint32_t reg_val = 0;

    if (lpbk_type == PCIE_SW_EXT_TEST) {
        /* Try to let Port enter Test Mode by set STMCTL (0xE54h) */
        reg_val = ((GEN1_2_5GTS << STMCTL_SPEED_OFF) | ENT_TEST_MOD);
    } else if (lpbk_type == SER_CDR_PMA_LPBK) {
        reg_val = ((GEN2_5_0GTS << STMCTL_SPEED_OFF) | ENT_TEST_MOD);
    } else {
        printf("%s:%d Unknown loopback type = %d.\n",
               __FUNCTION__, __LINE__, lpbk_type);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s: reg_val = 0x%08X.\n\n", __FUNCTION__, reg_val);
    }

    if (pes16nt16g2_global_addr_wr(port_num, STMCTL_OFF, reg_val) != PASSED) {
        printf("%s:%d Failed to write 0x%04X to port%2d, reg. 0x%03X.\n",
               __FUNCTION__, __LINE__, reg_val, port_num, STMCTL_OFF);
        return (FAILED);
    }

    msleep(100);

    /* Confirm Port state by check STMSTS (0xE58h) */
    reg_val = 0;     
    if (pes16nt16g2_global_addr_rd(port_num, STMSTS_OFF, &reg_val) == PASSED) {
        if (reg_val & STMSTS_CC) {
            if (((reg_val & STMSTS_PSTATE_MSK) >> STMSTS_PSTATE_OFF) !=
                SERDES_TEST_MOD) {
                printf("%s: Not in the Test Mode stat. (0x%01X)",
                       __FUNCTION__,
                       ((reg_val & STMSTS_PSTATE_MSK) >> STMSTS_PSTATE_OFF));
            }
        } else {
            printf("%s: Can't Complete to enter Test Mode.\n", __FUNCTION__);

            /* Abort Command */
            reg_val = ABORT_CMD;
            if (pes16nt16g2_global_addr_wr(port_num, STMCTL_OFF,
                                           reg_val) != PASSED) {
                printf("%s:%d Failed to write 0x%04X to port%2d, reg. 0x%03X.\n",
                       __FUNCTION__, __LINE__, reg_val, port_num, STMCTL_OFF);
                return (FAILED);
            }
            return (FAILED);
        }
    } else {
        printf("%s: Failed to read port%2d, reg. 0x%03X.\n",
               __FUNCTION__, port_num, STMSTS_OFF);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_exit_test_mode
 * Description:	Function to let Overlord PCIe switch (IDT PES16NT16G2)
 *              to enter Test Mode.
 * Inputs     :	port_num - the number of port that will exit Test Mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_exit_test_mode (uint32_t port_num)
{
    uint32_t reg_val = 0;

    reg_val = EXT_TEST_MOD;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s: reg_val = 0x%08X.\n\n", __FUNCTION__, reg_val);
    }

    /* Try to let Port enter Test Mode by set STMCTL (0xE54h) */
    if (pes16nt16g2_global_addr_wr(port_num, STMCTL_OFF, reg_val) != PASSED) {
        printf("%s:%d Failed to write 0x%04X to port%2d, reg. 0x%03X.\n",
               __FUNCTION__, __LINE__, reg_val, port_num, STMCTL_OFF);
        return (FAILED);
    }

    msleep(100);

    /* Confirm Port state by check STMSTS (0xE58h) */
    reg_val = 0;     
    if (pes16nt16g2_global_addr_rd(port_num, STMSTS_OFF, &reg_val) == PASSED) {
        if (reg_val & STMSTS_CC) {
            if (((reg_val & STMSTS_PSTATE_MSK) >> STMSTS_PSTATE_OFF) !=
                NORMAL_MOD) {
                printf("%s: Not in the Normal Mode stat. (0x%01X)",
                       __FUNCTION__,
                       ((reg_val & STMSTS_PSTATE_MSK) >> STMSTS_PSTATE_OFF));
            }
        } else {
            printf("%s: Can't Complete to exit Test Mode.\n", __FUNCTION__);

            /* Abort Command */
            reg_val = ABORT_CMD;
            if (pes16nt16g2_global_addr_wr(port_num, STMCTL_OFF,
                                           reg_val) != PASSED) {
                printf("%s:%d Failed to write 0x%04X to port%2d, reg. 0x%03X.\n",
                       __FUNCTION__, __LINE__, reg_val, port_num, STMCTL_OFF);
                return (FAILED);
            }
            return (FAILED);
        }
    } else {
        printf("%s: Failed to read port%2d, reg. 0x%03X.\n",
               __FUNCTION__, port_num, STMSTS_OFF);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_set_int_lpbk
 * Description:	Function to set the port of IDT PES16NT16G2 to chosen
 *              internal loopback type.
 * Inputs     :	port_num  - number of port want to change
 *              lpbk_type - the chosen type of internal loopback
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_set_int_lpbk (uint32_t port_num, uint32_t lpbk_type)
{
    uint32_t reg_val = 0;

    if (pes16nt16g2_global_addr_rd(port_num, SERDESCFG_OFF, &reg_val) != PASSED) {
        printf("%s: Failed to read port%2d, reg. 0x%03X.\n",
               __FUNCTION__, port_num, SERDESCFG_OFF);
        return (FAILED);
    }

    reg_val &= (~(SERDESCFG_ILPBK_MSK));
    reg_val |= (lpbk_type << SERDESCFG_ILPBK_OFF);

    if (pes16nt16g2_global_addr_wr(port_num, SERDESCFG_OFF, reg_val) != PASSED) {
        printf("%s: Failed to select loopback test type.\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_disable_int_lpbk
 * Description:	Function to disable the chosen port's loopback type.
 * Inputs     :	port_num  - number of port want to change
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_disable_int_lpbk (uint32_t port_num)
{
    uint32_t reg_val = 0;

    if (pes16nt16g2_global_addr_rd(port_num, SERDESCFG_OFF, &reg_val) != PASSED) {
        printf("%s: Failed to read port%2d, reg. 0x%03X.\n",
               __FUNCTION__, port_num, SERDESCFG_OFF);
        return (FAILED);
    }

    reg_val &= (~(SERDESCFG_ILPBK_MSK));
    reg_val |= (LPBK_DIS << SERDESCFG_ILPBK_OFF);

    if (pes16nt16g2_global_addr_wr(port_num, SERDESCFG_OFF, reg_val) != PASSED) {
        printf("%s: Failed to disable port%d loopback.\n", __FUNCTION__, port_num);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_lpbk_test_sel
 * Description:	Function to select Overlord PCIe switch (IDT PES16NT16G2)
 *              PCIe loopback test type.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_lpbk_test_sel (uint32_t port_num, uint32_t test_sel)
{
    uint32_t reg_val = 0;

    reg_val = ((TSYNCP_100US << STMTCTL_TSYNCP_OFF) | test_sel);
    if (pes16nt16g2_global_addr_wr(port_num, STMTCTL_OFF, reg_val) != PASSED) {
        printf("%s: Failed to select loopback test type.\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_start_test
 * Description:	Function to start test.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_start_test (uint32_t port_num, uint32_t lpbk_type)
{
    uint32_t reg_val = 0;

    /* Enter Test Mode to start test */
    if (pes16nt16g2_enter_test_mode(port_num, lpbk_type) != PASSED) {
        printf("%s: Failed to let port%2d enter Test mode.\n",
               __FUNCTION__, port_num);
        return (FAILED);
    }

    /* Check bit 31, Test Running bit, of STMTSTS register
     * to make sure the Test is running.
     */
    if (pes16nt16g2_global_addr_rd(port_num, STMTSTS_OFF, &reg_val) != PASSED) {
        printf("%s: Failed to read port%2d, register 0x%03X.\n",
               __FUNCTION__, port_num, STMTSTS_OFF);
        return (FAILED);
    } else {
        if (!(reg_val & STMTSTS_TR)) {
            printf("%s: Test is not running.\n", __FUNCTION__);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_check_test_status
 * Description:	Function to check test status.
 * Inputs     : port_num - number of tested port
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_check_test_status (uint32_t port_num, uint8_t sync_status)
{
    uint32_t reg_val = 0, need_check = 0, ctr = 0, reg_off = 0, checker[8];

    for (ctr = 0; ctr < sizeof(sync_status); ctr++) {
        if ((sync_status >> ctr) & 0x1) {
            checker[need_check] = ctr;
            need_check++;
        }
    }

    for (ctr = 0; ctr < need_check; ctr++) {
        reg_val = 0;
        reg_off = (STMECNT0_OFF + 4 * checker[ctr]);

        if (pes16nt16g2_global_addr_rd(port_num, reg_off, &reg_val) != PASSED) {
            printf("%s: Failed to read port%2d, register 0x%03X.\n",
                   __FUNCTION__, port_num, reg_off);
            return (FAILED);
        }

        if (reg_val & STMECNT_ERR_DET) {
            printf("%s: Test on Line %d is failed.\n",
                   __FUNCTION__, ctr);
            return (FAILED);
        }
    } 

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_set_reg_by_mask
 * Description:	Function to set/unset bit(s) of Overlord PCIe switch
 *              (IDT PES16NT16G2).
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_set_reg_by_mask (uint32_t port_num, uint32_t reg_off,
                                        uint32_t reg_mask, boolean opt)
{
    uint32_t data_in = 0;

    if (pes16nt16g2_global_addr_rd(port_num, reg_off, &data_in) != PASSED) {
        printf("%s: Failed to read Reg. 0x%08X out.\n", __FUNCTION__, reg_off);
        return (FAILED);
    }

    if (opt == ENABLE) {
         data_in |= reg_mask;
    } else {
         data_in &= (~reg_mask);
    }

    if (pes16nt16g2_global_addr_wr(port_num, reg_off, data_in) != PASSED) {
        printf("%s: Failed to write 0x%08X to Reg. 0x%08X out.\n",
               __FUNCTION__, data_in, reg_off);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pes16nt16g2_pcie_lpbk_test
 * Description:	Function to do Overlord PCIe switch (IDT PES16NT16G2)
 *              PCIe loopback test.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pes16nt16g2_pcie_lpbk_test (uint32_t port_num, uint32_t test_sel,
                                       uint32_t lpbk_type)
{
    uint32_t reg_val = 0;
    uint16_t reg_16bit = 0;
    uint8_t  sync_status = 0;

    /* 1. Set internal loopback mode if needed. */
    if (lpbk_type != PCIE_SW_EXT_TEST) {
        if (pes16nt16g2_set_int_lpbk(port_num, lpbk_type) != PASSED) {
            printf("%s: Failed to set port%2d to internal loopback(0x%X).\n",
                   __FUNCTION__, port_num, lpbk_type);
            return (FAILED);
        }
    }

    if (lpbk_type == PCIE_SW_EXT_TEST) {
        /* Based on IDT FAE's suggestion, before do external loopback test,
         * we need to retrain the Link of the tested port to make sure
         * the link up state is correct. 
         */

        /* 1. Set Self Cross Link Enable(SCLINKEN, bit 13) bit of
         *    PHY Link Configuration 0(PHYLCFG0, reg. 0x530) Register
         */
        if (pes16nt16g2_set_reg_by_mask(port_num, PHYLCFG0_OFF,
                                        PHYLCFG0_SCLINKEN, ENABLE) != PASSED) {
            printf("%s: Failed to set Self Cross Link Enable"
                   "(Reg. 0x530, bit 13) on port%2d.\n",
                   __FUNCTION__, port_num);
            return (FAILED);
        }

        /* 2. Set Register Unlock(REGUNLOCK, bit 3) bit of
         *    Switch Control(SWCTL, reg. 0x3E000) Register
         */
        if (pes16nt16g2_set_reg_by_mask(port_num,
                                        (SWCONF_STS_BASE + SWCTL_OFF),
                                        SWCTL_REGUNLOCK, ENABLE) != PASSED) {
            printf("%s: Failed to set Register Unlock"
                   "(Reg. 0x3E000, bit 3) on port%2d.\n",
                   __FUNCTION__, port_num);
            return (FAILED);
        }

        /* 3. Set Link Retrain(LRET, bit 5) bit of 
         *    PCI Express Link Control(PCIELCTL, reg. 0x050) Register
         */
        if (pes16nt16g2_set_reg_by_mask(port_num, PCIELCTL_OFF,
                                        PCIELCTL_LRET, ENABLE) != PASSED) {
            printf("%s: Failed to set Link Retrain"
                   "(Reg. 0x050, bit 5) on port%2d.\n",
                   __FUNCTION__, port_num);
            return (FAILED);
        }

        sleep(1);

        /* 4. Check Data Link Layer Link Active(DLLLA, bit 13) bit of 
         *    PCI Express Link Status(PCIELSTS, reg. 0x052) Register
         */
        if (pes16nt16g2_global_addr_rd(port_num, PCIELSTS_OFF,
                                       &reg_val) != PASSED) {
            printf("%s: Failed to read Reg. 0x%08X out.\n",
                   __FUNCTION__, PCIELSTS_OFF);
            return (FAILED);
        }

        reg_16bit = (uint16_t)((reg_val & 0xFFFF0000) >> 16);

        if (((reg_16bit & PCIELSTS_DLLLA) >> PCIELSTS_DLLLA_OFF) != ENABLE) {
            printf("%s: Data Link Layer Link Active bit is not set.\n",
                   __FUNCTION__);
            return (FAILED);
        }
    }

    /* 2. Set Loopbak test Type */
    if (pes16nt16g2_lpbk_test_sel(port_num, test_sel) != PASSED) {
        printf("%s: Failed to select Loopback Test type.\n", __FUNCTION__);
        return (FAILED);
    }

    /* 3. Enter Test Mode to start test */
    if (pes16nt16g2_start_test(port_num, lpbk_type) != PASSED) {
        printf("%s: Failed to start Loopback Test.\n", __FUNCTION__);
        return (FAILED);
    }

    /* 4. Get Sync Status by reading bit[7:0] of STMTSTS register */
    reg_val = 0;

    if (pes16nt16g2_global_addr_rd(port_num, STMTSTS_OFF, &reg_val) != PASSED) {
        printf("%s: Failed to read port%2d, STMTSTS reg.(0x%03X)\n",
               __FUNCTION__, port_num, STMTSTS_OFF);
        return (FAILED);
    }
    sync_status = (uint8_t)(reg_val & STMTSTS_SYNC_MSK);

    sleep (OVLD_PCIE_LPBK_TEST_TIME);

    /* 5. Stop test and check result */
    if (pes16nt16g2_exit_test_mode(port_num) != PASSED) {
        printf("%s: Failed to let port%2d exit Test mode.\n",
               __FUNCTION__, port_num);
        return (FAILED);
    }

    if (pes16nt16g2_check_test_status(port_num, sync_status) != PASSED) {
        printf("%s: Port%2d is failed at loopback test.\n",
               __FUNCTION__, port_num);
        return (FAILED);
    }

    /* 6. Clear internal loopback mode if needed. */
    if (lpbk_type != PCIE_SW_EXT_TEST) {
        if (pes16nt16g2_disable_int_lpbk(port_num) != PASSED) {
            printf("%s: Failed to disable port%2d internal loopback.\n",
                   __FUNCTION__, port_num);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ovld_pcie_8prbs_ext_lpbk_test
 * Description:	Function to test PCIe switch ports externally by running
 *              8bit PRBS Master loopback test.
 * Inputs     :	test_port - Number of the tested port
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_pcie_8prbs_ext_lpbk_test (uint32_t test_port)
{
    if (pes16nt16g2_pcie_lpbk_test(test_port, LPBK_8PRBS_MST,
                                   PCIE_SW_EXT_TEST) != PASSED) {
        cterr('f', 0, "port %d", test_port);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	ovld_pcie_10prbs_cdr_int_lpbk_test
 * Description:	Function to test Ovld PCIe switch ports internally by running
 *              10bit PRBS Master Serializer/CDR internal loopback test.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_pcie_10prbs_cdr_int_lpbk_test (void)
{
    /* HW verified the chip is not stable and this test adds no value */
    /* Just return pass */
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	pcie_dump_port_regs_util
 * Description:	Function to dump specific PCIe port all registers.
 * Inputs     :	sw_port - Switch port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pcie_dump_port_regs_util (uint32_t sw_port)
{
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

    return (PASSED);
}

/*------------------------------------------------------------------
$Log: platform_pcie_sw.c,v $
Revision 1.4  2017/07/14 02:51:39  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.3  2014/01/27 23:53:39  ptong
Obsolete the ovld_pcie_10prbs_cdr_int_lpbk_test

Revision 1.2  2013/12/18 09:33:59  hroni
plx does not support prbs internal loopback, hide the test item in pcie switch util

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.8  2013/02/23 07:31:56  ptong
Fixed the problem due to new ROMMON changed the PCIe bus numbering and bumped diag version to 6.3

Revision 1.7  2012/12/21 01:28:30  palin2
1.Add support for Overlord Diag RDT version.
2.Add utility to dump specific PCIe port's all registers for debugging purpose.

Revision 1.6  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.5  2012/11/05 17:49:28  palin2
To fix PCIe loopback(at TestCard) test on NGWIC 3 by using Gen 1 speed
based on IDT FAE's suggestion.

Revision 1.4  2012/10/10 16:57:01  palin2
Fixed NGWIC TestCard PCIe loopback test based on IDT FAE's suggestion.

Revision 1.3  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.2  2012/09/24 17:37:42  palin2
1. Use "Internal loopback test" as default test for TestCard.
2. Unify all tests print out format for TestCard.

Revision 1.1  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

$Endlog$
*/

