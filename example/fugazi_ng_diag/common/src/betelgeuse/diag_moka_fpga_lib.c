/* $Id: diag_moka_fpga_lib.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_moka_fpga_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_moka_fpga_lib.c
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
#include "diag_dsl_util.h"
#include "diag_dsl_test.h"
#include "diag_ge_phy_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_util.h"
#include "diag_ge_phy_lib.h"
#include "diag_wifi_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_moka_fpga_util.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"
#include "diag_aikido_fpga_lib.h"
#include "diag_aikido_fpga_util.h"
#include "diag_aikido_fpga_test.h"


/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int program_reggio_spi_prom(void);
extern int program_spi_update_version(void);
extern int show_plat_curr_temps(void);
extern unsigned int plat_gfast_sku;

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int        fpga_reset_32_api(uint, uint, uint, uint);
int        fpga_read_32_reg(uint, uint *);
int        fpga_write_32_reg(uint, uint);
static int fpga_reg_test_read_fn(ulong, int, ulong *, void *);
static int fpga_reg_test_write_fn(ulong, int, ulong, void *);
int fpga_reg_rd_util(int);
int fpga_reg_wr_util(int);
int        plat_all_leds_off(int);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
reg_info_t_ext plat_fpga_reg_ext = {PLAT_FPGA_REG_WIDTH,
                                   fpga_reg_test_read_fn,
                                   fpga_reg_test_write_fn,
                                   0};


/*
 * FPGA Utilities
 */
static submenu_xtable_t fpga_utils_tbl[] = {
    {"Show System FPGA version",   (type_t(*)())plat_show_fpga_ver,  0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Show Pluggable FPGA version",   (type_t(*)())show_plug_fpga_ver,  0, 0,
     (type_t(*)())platform_has_sirius_fpga, 0,     (type_t(*)())0, 0},
    {"FPGA register Read",  (type_t(*)())fpga_reg_rd_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Write", (type_t(*)())fpga_reg_wr_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Aikido register Read",  (type_t(*)())aikido_reg_rd_util,    0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Aikido register Write", (type_t(*)())aikido_reg_wr_util,    0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA registers Dump", (type_t(*)())fpga_reg_dump_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPU register Read",   (type_t(*)())diag_cpu_reg_rd_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPU register Write",  (type_t(*)())diag_cpu_reg_wr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Program FPGA SPI PROM image",  (type_t(*)())program_reggio_spi_prom, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    /* Avoid to confuse users, combine item o and p(with/without header). 
     * Temporarily masked out "Program Pluggable FPGA SPI PROM image without 
     * header" item from menu. 
     */
    {"Program Pluggable FPGA SPI PROM image", (type_t(*)())plug_fpga_spi_prog, 0, 0,
     (type_t(*)())platform_has_sirius_fpga, 0, (type_t(*)())0,   0},
    {"Pluggable FPGA Erase/Program Image Upgrade Header", (type_t(*)())plug_fpga_erase_header, 1, 0,
     (type_t(*)())platform_has_sirius_fpga, 0, (type_t(*)())0,   0},
    {"Set Pluggable FPGA revision and date", (type_t(*)())plug_fpga_set_date_revision, 1, 0,
     (type_t(*)())platform_has_sirius_fpga, 0, (type_t(*)())0,   0},
    {"Display Pluggable FPGA PROM sector", (type_t(*)())plug_fpga_display_sector, 1, 0,
     (type_t(*)())platform_has_sirius_fpga, 0, (type_t(*)())0, 0},
    {"Modify SPI Directory Table",  (type_t(*)())program_spi_update_version, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Aikido Mail Box Test",  (type_t(*)())aikido_mailbox_test, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
};

#define FPGA_UTILS_TBL_SIZE (sizeof(fpga_utils_tbl) / sizeof(submenu_xtable_t))
#define ENHANCE_ERROR_MSG_RDY 1

/* FPGA Utils items (filled in from xtable) */
static mitem_t fpga_utils_pri_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_utils_sec_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* FPGA Utils submenu */
menuinfo_t fpga_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_utils_pri_items,
};
menuinfo_t *fpga_utils_menup = &fpga_utils_menu;

/*
 * LED Control 
 */
static submenu_xtable_t fpga_led_ctrl_tbl[] = {
    {"Status LED utils", (type_t(*)())plat_status_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Power OK stat LED utils", (type_t(*)())plat_pwrok_stat_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"POE present stat LED utils", (type_t(*)())plat_poestat_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"POE port0 stat LED utils", (type_t(*)())plat_poeport_led_utils, 0,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"POE port1 stat LED utils", (type_t(*)())plat_poeport_led_utils, 1,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"POE port2 stat LED utils", (type_t(*)())plat_poeport_led_utils, 2, 
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"POE port3 stat LED utils", (type_t(*)())plat_poeport_led_utils, 3,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"AUX stat LED utils", (type_t(*)())plat_aux_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Micro USB stat LED utils", (type_t(*)())plat_microusb_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB stat LED utils", (type_t(*)())plat_usb_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Console stat LED utils", (type_t(*)())plat_console_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"VPN stat LED utils", (type_t(*)())plat_vpn_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE0 stat LED utils", (type_t(*)())diag_util_ge_led, 0,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE1 stat LED utils", (type_t(*)())diag_util_ge_led, 1,
     0,
      (type_t(*)())platform_has_2nd_ge, 0, (type_t(*)())0, 0},
    {"GE Switch port LED utils",  (type_t(*)())diag_esw_force_led_onoff_util,
     0, 0, 
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"XDSL LED OFF",     (type_t(*)())xdsl_util_bcm63168_led, LED_OFF, 0,
      (type_t(*)())platform_has_xdsl, 0,   (type_t(*)())0,          0},
    {"XDSL LED CD ON",   (type_t(*)())xdsl_util_bcm63168_led, LED_CD_ON, 0,
      (type_t(*)())platform_has_xdsl, 0,   (type_t(*)())0,          0},
    {"XDSL LED Data ON", (type_t(*)())xdsl_util_bcm63168_led, LED_DATA_ON, 0,
      (type_t(*)())platform_has_xdsl, 0,   (type_t(*)())0,          0},
    {"WLAN LED ON in RED",  (type_t(*)())wifi_led_control, WIFI_LED_RED, 0,
     (type_t(*)())platform_has_wifi, 0,
     (type_t(*)())0,          0},
    {"WLAN LED ON in GREEN", (type_t(*)())wifi_led_control, WIFI_LED_GREEN, 0,
     (type_t(*)())platform_has_wifi, 0,
     (type_t(*)())0,          0},
    {"WLAN LED ON in AMBER",  (type_t(*)())wifi_led_control, WIFI_LED_AMBER, 0,
     (type_t(*)())platform_has_wifi, 0,
     (type_t(*)())0,          0},
    {"WLAN LED OFF",  (type_t(*)())wifi_led_control, WIFI_LED_OFF, 0,
     (type_t(*)())platform_has_wifi, 0,
     (type_t(*)())0,          0},
};

#define FPGA_LED_CTRL_TBL_SIZE (sizeof(fpga_led_ctrl_tbl) / sizeof(submenu_xtable_t))

/* LED Control Utils items (filled in from xtable) */
static mitem_t fpga_led_ctrl_pri_items[FPGA_LED_CTRL_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_led_ctrl_sec_items[FPGA_LED_CTRL_TBL_SIZE + MAX_BASE_ITEMS];

/* LED Control Utils submenu */
menuinfo_t fpga_led_ctrl_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_led_ctrl_pri_items,
};
menuinfo_t *led_ctrl_menup = &fpga_led_ctrl_menu;

/*******************************************************************************
 *
 * Function    : show_usb_console_to_uart_connectivity
 * Description : Function to check UART link with USB console or not.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int show_usb_console_to_uart_connectivity (int opt)
{
    uint reg_offset = (uint)FPGA_STAT_AND_CTRL_REG;
    uint reg_val = 0;

    /* Read FPGA status and control register */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA status and Control Reg(0x%04X).\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if ((reg_val & (uint)FPGA_USB_AND_RJ45_CON_MUX) == (uint)FPGA_USB_AND_RJ45_CON_MUX) {
        printf("%s: USB console link with UART.\n", __FUNCTION__);
        return (PASSED);
    } else {
        printf("%s: RJ45 console link with UART.\n", __FUNCTION__);
        return (FAILED);
    }

}

/*******************************************************************************
 *
 * Function    : diag_moka_fpga_util
 * Description : Function to show FPGA utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_moka_fpga_util (int opt)
{
    build_primary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                          "FPGA Utilities", &fpga_utils_menup);
    build_secondary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                            fpga_utils_sec_items);

    menu(fpga_utils_menup, fpga_utils_sec_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : is_sfp_present
 * Description: Function to see if SFP module is present.
 *              This is by checking Module definition 0(bit1) of
 *              FPGA SFP status and Control Reg(0x1134).
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int is_sfp_present (void)
{
    uint reg_offset = (uint)FPGA_SFP_AND_CTRL_REG;
    uint reg_val = 0;

    /* Read SFP status and control register */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA SFP status and Control Reg(0x%04X).\n",
               __FUNCTION__, reg_offset);
        return (FALSE);
    }

    if ((reg_val & (uint)SFP_SC_MODULE_DEF) == (uint)SFP_SC_MODULE_DEF) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : enable_sfp_tx_transmit
 * Description: Function to turn on(Enable)/off(Disable) SFP TX.
 * Inputs     : opt - To ENABLE/DISABLE SFP TX
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sfp_tx_enable_switch (int opt)
{
    uint reg_offset = (uint)FPGA_SFP_AND_CTRL_REG;
    uint reg_val = 0;

    if ((opt != ENABLE) & (opt != DISABLE)) {
        printf("%s: Unknown option(%d).\n", __FUNCTION__, opt);
        return (FAILED);
    }

    /* Read SFP status and control register */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA SFP status and Control Reg(0x%04X).\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if (opt == ENABLE) {
        reg_val |= (uint)SFP_SC_TX_DIS;
    } else {
        reg_val &= (uint)(~SFP_SC_TX_DIS);
    }

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    reg_val = 0;
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA SFP status and Control Reg(0x%04X).\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if (((opt == ENABLE) & ((reg_val & SFP_SC_TX_DIS) != SFP_SC_TX_DIS)) ||
        ((opt == DISABLE) & ((reg_val & SFP_SC_TX_DIS) != 0))) {

        printf("%s: Failed to %s SFP TX.\n",
               __FUNCTION__, (opt == ENABLE) ? "Enable" : "Disable");
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reset_32_api
 * Description : Function of FPGA to reset/unreset interface.
 * Inputs      : r_offset  - register offset
 *               r_bit     - reset bit of register
 *               r_opt     - reset(TRUE)/un-reset(FALSE)
 *               r_time_ms - the reset time interval(millisecond)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reset_32_api (uint r_offset, uint r_bit, uint r_opt, uint r_time_ms)
{
    uint reg_val = 0;

    /* Read FPGA interface reset register. */
    if (fpga_read_32_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set the Reset bit. */
        reg_val |= r_bit;
    } else if (r_opt == FALSE) {
        /* Clear the reset bit. */
        reg_val &= (uint)(~r_bit);
    } else {
        printf("%s: Invalid Reset option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }
 
    /* Write the reset/un-reset into the corresponding register bit. */
    if (fpga_write_32_reg(r_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    /* Delay milliseconds after reset/un-reset */
    msleep(r_time_ms);

    /* Confirm the change to FPGA interface reset register. */
    reg_val = 0;
    if (fpga_read_32_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (((r_opt == TRUE) && ((reg_val & r_bit) != r_bit)) ||
        ((r_opt == FALSE) && ((reg_val & r_bit) != 0))) {
        printf("%s: Failed to %s reset bit in FPGA reg.(0x%04X).\n",
               __FUNCTION__, (r_opt == TRUE) ? "set" : "clear", r_offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_read_32_reg
 * Description : Function to read FPGA register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_read_32_reg (uint reg_offset, uint *buf)
{
    uint offset = 0;

    offset = (uint)(plat_fpga_reg_baseaddr + reg_offset);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Device Bus address 0x%08X\n", offset);
    }
    if (plat_mem_read32(offset, buf) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_write_32_reg
 * Description : Function performs FPGA register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_write_32_reg (uint reg_offset, uint wr_data)
{
    uint offset = 0;

    offset = (uint)(plat_fpga_reg_baseaddr + reg_offset);

    if (plat_mem_write32(offset, wr_data) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fpga_reg_test_read_fn
 * Description: FPGA register read function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              *buf   - pointer to read buffer
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
static int fpga_reg_test_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    if (fpga_read_32_reg((uint)addr, (uint *)buf) != PASSED) {
        printf("%s: Failed to read FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fpga_reg_test_write_fn
 * Description: FPGA register write function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              data   - write in data
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
static int fpga_reg_test_write_fn (ulong addr, int size, ulong data, void *param)
{
    if (fpga_write_32_reg((uint)addr, (uint)data) != PASSED) {
        printf("%s: Failed to write FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_get_boardtype
 * Description: Function to get board type.
 *              This is by reading FPGA LPC Board Type Reg(0x80).
 * Inputs     : *b_type - buffer to put the read back board type value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_get_boardtype (uint *b_type)
{
    if (fpga_read_32_reg((uint)FPGA_LPC_SKUFEATURE_REG, b_type) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_fpga_check_dev_present
 * Description: Function to check if module is present.
 *              This is by read FPGA Card and Power present Reg.(0x1118).
 * Inputs     : mod_type - device type(WLAN, LTE, PoE)
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
boolean plat_fpga_check_dev_present (uint mod_type)
{
    uint reg_addr = (uint)FPGA_CARD_AND_PWR_REG;
    uint reg_val = 0;


    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA Card and Power Present Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA @0x%04X: 0x%08X.\n", __func__, reg_addr, reg_val);
        printf("%s: mod_type = 0x%08X.\n", __func__, mod_type);
    }

    if ((reg_val & mod_type) != mod_type) {
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function    : diag_led_ctrl_util
 * Description : Function to show LED Control utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_ctrl_util (int opt)
{
    build_primary_submenu(fpga_led_ctrl_tbl, FPGA_LED_CTRL_TBL_SIZE,
                          "LED Control Utilities", &led_ctrl_menup);
    build_secondary_submenu(fpga_led_ctrl_tbl, FPGA_LED_CTRL_TBL_SIZE,
                            fpga_led_ctrl_sec_items);

    menu(led_ctrl_menup, fpga_led_ctrl_sec_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_check_ext_intr_no_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "No Interrupt Pending" state
 * Inputs      : fpga_pending_bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_check_ext_intr_no_pending (int fpga_pending_bit)
{
    uint fpga_offset, fpga_rd_data, fpga_wr_data;
    fpga_offset = FPGA_EXTER_INT_PENDING_REG; /* 0x1128 */

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FAILED);
    }

    /* set FPGA EIPR GE interrupt pending bit as 1(in order to read interrupt pin)*/
    fpga_wr_data = fpga_rd_data | fpga_pending_bit;
    if (fpga_write_32_reg(fpga_offset, fpga_wr_data) != PASSED) {
        printf("%s:%d: Failed to write FPGA reg:0x%x with data:0x%x\n",
                __FUNCTION__, __LINE__, fpga_offset, fpga_wr_data);
        return (FAILED);
    }

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FAILED);
    }

    /* checking pending bit is low */
    if ((fpga_rd_data & fpga_pending_bit) != 0) {
        printf("%s:%d: The EIPR(0x1128) pending bit is still high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_check_ext_intr_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "Interrupt Pending" state
 * Inputs      : fpga_pending_bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_check_ext_intr_pending (int fpga_pending_bit)
{
    uint fpga_offset, fpga_rd_data;
    fpga_offset = FPGA_EXTER_INT_PENDING_REG; /* 0x1128 */

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FAILED);
    }

    /* checking pending bit is high */
    if ((fpga_rd_data & fpga_pending_bit) != fpga_pending_bit) {
        printf("%s:%d: The EIPR(0x1128) pending bit is not high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_check_esw_ext_intr_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "Interrupt Pending" state
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_check_esw_ext_intr_pending (void)
{
    uint fpga_offset, fpga_rd_data, fpga_wr_data;
    int fpga_pending_bit = PENDING_BIT_ESW; 
    fpga_offset = FPGA_EXTER_INT_PENDING_REG;

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FAILED);
    }

    /* set FPGA EIPR GE interrupt pending bit as 1(in order to read interrupt pin)*/
    fpga_wr_data = fpga_rd_data | fpga_pending_bit;
    if (fpga_write_32_reg(fpga_offset, fpga_wr_data) != PASSED) {
        printf("%s:%d: Failed to write FPGA reg:0x%x with data:0x%x\n",
                __FUNCTION__, __LINE__, fpga_offset, fpga_wr_data);
        return (FAILED);
    }

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FAILED);
    }

    /* checking pending bit is low */
    if ((fpga_rd_data & fpga_pending_bit) != 0) {
        printf("%s:%d: The EIPR(0x1128) pending bit is still high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_check_esw_ext_no_intr_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "No Interrupt Pending" state
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_check_esw_ext_no_intr_pending (void)
{
    uint fpga_offset, fpga_rd_data;
    int fpga_pending_bit = PENDING_BIT_ESW;
    fpga_offset = FPGA_EXTER_INT_PENDING_REG;

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FAILED);
    }

    /* checking pending bit is high */
    if ((fpga_rd_data & fpga_pending_bit) != fpga_pending_bit) {
        printf("%s:%d: The EIPR(0x1128) pending bit is not high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_moka_fpga_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
