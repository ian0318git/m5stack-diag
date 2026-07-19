/* $Id: diag_moka_fpga_util.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_moka_fpga_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_moka_fpga_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/mman.h>
#include <unistd.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include <stdio.h>
#include "proto.h"
#include "diag_moka_fpga_lib.h"
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_cpu_lib.h"
#include "diag_sirius_fpga_lib.h"
#include "diag_sirius_fpga_util.h"
#include "diag_ge_phy_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_util.h"
#include "diag_ge_phy_lib.h"
#include "diag_wifi_lib.h"
#include "diag_moka_fpga_util.h"
#include "diag_moka_fpga_lib.h"

static reg_info_t fpga_reg_dump_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"LPC Scratchpad",                FPGA_LPC_SCRATCHPAD_REG,    FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0xFFFFFF07, 0x0},
    {"LPC Status LED Control",        FPGA_LPC_STAT_LED_CTRL_REG, FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000001, 0x1},
    {"External Device Reset",         FPGA_LPC_EXT_DEV_RST_REG,   FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x0000040C, 0x4},
    {"IRQ Test",                      FPGA_IRQ_TEST_REG,          FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0xFF040000, 0x0},
    {"Board Power Cycle",             FPGA_BOARD_PWR_CYCLE_REG,   FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0xFFFF3FFF, 0x0},
    {"LPC Board Type",                FPGA_LPC_BOARDTYPE_REG,     FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x0000000F, 0x0},
    {"FPGA External Device Reset",    FPGA_EXTER_DEV_RST_REG,     FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x03F83ABA, 0x370FAF2},
    {"Internal Device Reset",         FPGA_INT_DEV_RST_REG,       FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0xFFFFFFFF, 0x0},
    {"Board Type",                    FPGA_BOARD_TYPE_REG,        FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x13},
    {"Master FPGA Revision",          FPGA_MASTER_REV_REG,        FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x800103},
    {"FPGA Revision",                 FPGA_REV_REG,               FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x16041901},
    {"FPGA Debug LED",                FPGA_DBG_LED_REG,           FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000000FF, 0x290FF},
    {"CPU Mux and USB Power Control", FPGA_CPUMUX_AND_USBPWR_REG, FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000003, 0x0},
    {"Status and Control",            FPGA_STAT_AND_CTRL_REG,     FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000003EE, 0xE},
    {"Power Status",                  FPGA_PWR_STAT_REG,          FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0xFFFFFFFF},
    {"Card and Power Present",        FPGA_CARD_AND_PWR_REG,      FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x4},
    {"LED",                           FPGA_LED_REG,               FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00FCFE03, 0x0},
    {"LTE RSSI and LED",              FPGA_LTE_RSSI_LED_REG,      FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x0},
    {"Watchdog Strobe",               FPGA_WATCHDOG_REG,          FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x0},
    {"Ext. Interrupt Pending",        FPGA_EXTER_INT_PENDING_REG, FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x10A4},
    {"Ext. Interrupt Mask",           FPGA_EXT_INTR_MASK_REG,     FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x0001FBFF, 0x0},
    {"Force Ext. Interrupt",          FPGA_FORCE_EXT_INTR_REG,    FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x0001FBFF, 0x0},
    {"SFP Status and Control",        FPGA_SFP_AND_CTRL_REG,      FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000001, 0x0},
    {"LTE Control",                   FPGA_LTE_CTL_REG,           FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000000CE, 0x0},
    {"SIM Status and Control",        FPGA_SIM_STATUS_CTL_REG,    FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000306, 0x0},
    {"xDSL Status and Control",       FPGA_DSL_STATUS_CTL_REG,    FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x0},
    {"I2C Master Control",            FPGA_I2C_CTL_REG,           FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x0F03FFE0, 0x0},
    {"I2C Master Status",             FPGA_I2C_STAT_REG,          FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x0},
    {"I2C Master Status Mask",        FPGA_I2C_STAT_MASK_REG,     FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000000FE, 0x0},
    {"I2C Master Slave Addr.",        FPGA_I2C_SLA_ADDR_REG,      FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000003FF, 0x0},
    {"I2C Master Slave SubAddr.",     FPGA_I2C_SLA_SUBADDR_REG,   FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00FFFFFF, 0x0},
    {"I2C Master Bit-Bang",           FPGA_I2C_BIT_BANG_REG,      FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000003, 0x0},
    {"I2C Byte Count",                FPGA_I2C_BYTE_COUNT_REG,    FPGA_RONLY,
        {(unsigned long)&plat_fpga_reg_ext},   0x00000000, 0x0},
    {"I2C Data FIFO",                 FPGA_I2C_DATA_FIFO_REG,     FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0xFFFE0000, 0x0},
    {"I2C Data FIFO Pointer",         FPGA_I2C_DATA_RW_PTR_REG,   FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000003FF, 0x0},
    {"SPI PROM CONTROL",              FPGA_SPI_CTRL_REG,          FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000087FF, 0x0},
    {"SPI PROM Status",               FPGA_SPI_STAT_REG,          FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x00008001, 0x0},
    {"SPI PROM Read Size",            FPGA_SPI_RD_SIZE_REG,       FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000000FF, 0x0},
    {"SPI PROM R/W Data",             FPGA_SPI_RW_DATA_REG,       FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0x000000FF, 0x0},
    {"SPI PROM OP Code/Addr.",        FPGA_SPI_OP_ADDR_REG,       FPGA_RW,
        {(unsigned long)&plat_fpga_reg_ext},   0xFFFFFFFF, 0x0},
};

/*******************************************************************************
 *
 * Function   : plat_show_fpga_sku_feature_reg
 * Description: Function to show FPGA sku feature register.
 *              This is by reading FPGA Revision Reg(0x00C0).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_show_fpga_sku_feature_reg(int opt)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint fpga_sku_feature = 0;

    if (fpga_read_32_reg(reg_addr, &fpga_sku_feature) != PASSED) {
        printf("Failed to read FPGA SKU Feature Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("FPGA SKU feature register: %08X\n", fpga_sku_feature);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_show_fpga_board_type_reg
 * Description: Function to show FPGA board type register.
 *              This is by reading FPGA Revision Reg(0x0080).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_show_fpga_board_type_reg(int opt)
{
    uint reg_addr = (uint)FPGA_LPC_BOARDTYPE_REG;
    uint fpga_board_type = 0;

    if (fpga_read_32_reg(reg_addr, &fpga_board_type) != PASSED) {
        printf("Failed to read FPGA Board Type Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("FPGA Board Type register: %08X\n", fpga_board_type);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_show_fpga_ver
 * Description: Function to show FPGA version.
 *              This is by reading FPGA Revision Reg(0x108C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_show_fpga_ver (int opt)
{
    uint reg_addr = (uint)FPGA_REV_REG;
    uint fpga_ver = 0;

    if (fpga_read_32_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("FPGA version: %08X\n", fpga_ver);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_reg_rd_util
 * Description : Utility to read FPGA register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_rd_util (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address (0x0 ~ 0x1ffff): ",
                               FPGA_REV_REG, 0, FPGA_MAX_REG_ADDR);

    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("FPGA register(0x%04X) = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_reg_wr_util
 * Description : Utility to write FPGA register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address(0x0 ~ 0x1ffff): ",
                               0, 0, FPGA_MAX_REG_ADDR);

    if (fpga_read_32_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }

    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_reg_dump_util
 * Description : Utility to dump FPGA all registers.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_dump_util (int opt)
{
    uint       reg_val = 0;
    reg_info_t *reg_p = 0;
    int        ctr = 0, total_reg_num = 0;

    reg_p = &fpga_reg_dump_tbl[0];
    total_reg_num = (sizeof(fpga_reg_dump_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        reg_val = 0;
        if (fpga_read_32_reg(reg_p->offset, &reg_val) != PASSED) {
            printf("Failed to read FPGA %s Reg(0x%04X).\n",
                   reg_p->name, reg_p->offset);
            return (FAILED);
        } else {
            printf("%-29s Reg(0x%04X): 0x%08X\n",
                   reg_p->name, reg_p->offset, reg_val);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_status_led_utils
 * Description : Function to turn status LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_status_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    
    printf("\n"); 
    printf("Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Yellow Blink\n");
    printf("2. Yellow\n");
    printf("3. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 3): ", 0, 0, 3);
    
    if (option == 0) { 
        reg_val = (uint)(STAT_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(STAT_LED_YB);
    } else if (option == 2 ) {
        reg_val = (uint)(STAT_LED_Y);
    } else if (option == 3) {
        reg_val = (uint)(STAT_LED_G);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_pwrok_stat_led_utils
 * Description : Function to turn Power OK LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_pwrok_stat_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    
    printf("\n"); 
    printf("Power OK LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. ON\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) { 
        reg_val = (uint)(PWR_OK_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(PWR_OK_LED);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_poestat_led_utils
 * Description : Function to turn POE status LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_poestat_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("POE Present Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Green\n");
    printf("2. Yellow\n");
    option = getdec_answer("Select Toogle (0 ~ 2): ", 0, 0, 2);
    
    if (option == 0) { 
        reg_val = (uint)(POE_PRESENT_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(POE_PRESENT_LED_G);
    } else if (option == 2 ) {
        reg_val = (uint)(POE_PRESENT_LED_Y);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_poeport_led_utils
 * Description : Function to turn POE port LEDs ON/OFF.
 * Inputs      : port - port number.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_poeport_led_utils (int port)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("POE Port%d Status LED utils: \n", port); 
    printf("0. OFF\n");
    printf("1. Yellow\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) {
        if (port == 0) {
            reg_val = (uint)(POE_P0_LED_OFF);
        } else if (port == 1) {
            reg_val = (uint)(POE_P1_LED_OFF);
        } else if (port == 2) {
            reg_val = (uint)(POE_P2_LED_OFF);
        } else if (port == 3) {
            reg_val = (uint)(POE_P3_LED_OFF);
        } else {
            printf("Port not correct\n");
            return (FAILED);
        }
    } else if (option == 1 ) {
        if (port == 0) {
            reg_val = (uint)(POE_P0_LED);
        } else if (port == 1) {
            reg_val = (uint)(POE_P1_LED);
        } else if (port == 2) {
            reg_val = (uint)(POE_P2_LED);
        } else if (port == 3) {
            reg_val = (uint)(POE_P3_LED);
        } else {
            printf("Port not correct\n");
            return (FAILED);
        }
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_aux_led_utils
 * Description : Function to turn AUX LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_aux_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("AUX Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Yellow\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) {
        reg_val = (uint)(AUX_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(AUX_LED);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_microusb_led_utils
 * Description : Function to turn micro usb LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_microusb_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("MicroUSB Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) {
        reg_val = (uint)(MICRO_USB_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(MICRO_USB_LED);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_usb_led_utils
 * Description : Function to turn usb LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_usb_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("USB Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) {
        reg_val = (uint)(USB_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(USB_LED);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_console_led_utils
 * Description : Function to turn console LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_console_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("Console Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) {
        reg_val = (uint)(CONSOLE_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(CONSOLE_LED);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_vpn_led_utils
 * Description : Function to turn vpn LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_vpn_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("VPN OK Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    
    if (option == 0) {
        reg_val = (uint)(VPN_OK_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(VPN_OK_LED);
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_moka_fpga_util.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.3  2021/05/26 04:05:05  harrchan
 * Display FPGA board type and FPGA sku feature in system information utility
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
