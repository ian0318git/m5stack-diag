/* $Id: diag_cpu_util.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_cpu_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_cpu_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "proto.h"
#include "linux_coretest.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"

/*******************************************************************************
 *                                 Menus
 *******************************************************************************
 */
/*
 * CPU Diag menu
 */
static submenu_xtable_t cpu_sys_util_table[] = {
    {"Show devbus info",      (type_t(*)())diag_cpu_system_show_devbus_info_util,
     TRUE, 0,
     (type_t(*)())0, 0,
     (type_t(*)())0, 0},
    {"ECC error injection",   (type_t(*)())diag_cpu_system_ecc_err_injection_util,
     TRUE, 0,
     (type_t(*)())0, 0,
     (type_t(*)())0, 0},
    {"CPU register Read",     (type_t(*)())diag_cpu_reg_rd_util, 
     0, 0,
     (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"CPU register Write",    (type_t(*)())diag_cpu_reg_wr_util, 
     0, 0,
     (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
};

#define CPU_DIAG_TABLE_SIZE (sizeof(cpu_sys_util_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t cpu_util_menu_pri_items[CPU_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t cpu_util_menu_sec_items[CPU_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static menuinfo_t cpu_util_menu = {
    "%s Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    cpu_util_menu_pri_items,
};

static menuinfo_t *cpu_util_menu_p = &cpu_util_menu;

/*******************************************************************************
 *
 * Function   : diag_cpu_util
 * Description: Entry function of CPU Diag.
 * Inputs     : exec_opt - "TRUE" to execute all MF_DOALL tests in menu
 *                         "FALSE" to show Test Menu
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_cpu_util (void)
{
    build_primary_submenu(cpu_sys_util_table, CPU_DIAG_TABLE_SIZE,
                          "CPU", &cpu_util_menu_p);
    build_secondary_submenu(cpu_sys_util_table, CPU_DIAG_TABLE_SIZE,
                            cpu_util_menu_sec_items);

    menu(&cpu_util_menu, cpu_util_menu_sec_items, 0);
}

/*******************************************************************************
 *
 * Function   : diag_cpu_system_ecc_err_injection_util
 * Description: Function to inject ECC errors
 * Inputs     : N/A
 * Outputs    : N/A
 *
 *******************************************************************************
 */
void diag_cpu_system_ecc_err_injection_util (void)
{
    printf("To stop ECC error injection, power cycle unit is needed.\n");

    if (getc_answer("Enter \"y\" to inject ECC errors.", "y/n", 'n') == 'y') {
        system(ECC_ERR_LOG_CONFIG);
        system(ECC_1BIT_ERR_COUNTER);
        system(ECC_ERR_INFO_0);
        system(ECC_ERR_INFO_1);
        system(INTERRUPT_STATUS_REG);
        system(INTERRUPT_ENABLE_REG);
        system(PHY_REG_FILE_ACCESS_0);
        system(PHY_REG_FILE_ACCESS_1);
    }
}

/*******************************************************************************
 *
 * Function    : diag_cpu_system_show_devbus_info_util
 * Description : Function to show CPU device bus info.
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
void diag_cpu_system_show_devbus_info_util (void)
{
    printf("Platform FPGA(devbus_CS%d) base addr.: 0x%08X\n",
           PLAT_FPGA_DEVBUS_NUM, plat_fpga_reg_baseaddr);
    printf("Aikido FPGA(devbus_CS%d) base addr.: 0x%08X\n",
           PLAT_AIKIDO_DEVBUS_NUM, plat_aikido_reg_baseaddr);
}

/*******************************************************************************
 *
 * Function    : diag_cpu_reg_rd_util
 * Description : Utility to read CPU register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_cpu_reg_rd_util (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffffffff): ",
                               0, 0, 0xffffffff);

    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("0x%08X = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_cpu_reg_wr_util
 * Description : Utility to write CPU register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_cpu_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffffffff): ",
                               0, 0, 0xffffffff);

    if (plat_mem_read32(reg_offset, &orig_val) != PASSED) {
        printf("Failed to read FPGA register 0x%08X.\n", reg_offset);
        return (FAILED);
    }

    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (plat_mem_write32(reg_offset, reg_val) != PASSED) {
        printf("Failed to write CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to CPU register(0x%08X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_cpu_ondie_temp_util
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int diag_cpu_ondie_temp_util (int opt)
{
    int cpu_temp = 0;

    if (plat_get_cpu_ondie_temp(&cpu_temp) != PASSED) {
        printf("%s: Failed to read CPU on-die temperature.\n", __FUNCTION__);
        return (FAILED);
    }
    printf("Current CPU on-die Temp. = %d degree C.\n", cpu_temp);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_cpu_util.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
