/* $Id: platform_fpga.c,v 1.16 2019/03/07 09:51:32 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_fpga.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_fpga.c
 * Description: TSN FPGA Library.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
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
#include "plat_defs.h"
#include "tsn_comm.h"
#include "platform_fpga.h"
#include "platform_esw.h"

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_cpu.h"
#include "plug_host_fpga_lib.h"
#include "plug_host_fpga_prog.h"
#include "plug_common_host_impl.h"
#include "dsl_tests.h"
#include "platform_wifi.h"

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int tsn_all_ge_leds_on(int);
extern int tsn_all_ge_leds_off(int);
extern boolean has_xdsl(int);
extern int program_reggio_spi_prom(void);
extern int program_spi_update_version(void);
extern boolean tsn_has_2nd_ge(int);
extern int tsn_display_temp_errormsg(void);
extern unsigned int tsn_gfast_sku;
extern int tsn_esw_force_led_onoff_util(int);
extern int tsn_ge_led_utils(int);
extern int wrap_bcm63168_led_utils(int);
extern int tsn_wifi_led_control(int);
extern boolean has_fpga_sku_check(int);
extern boolean has_poe(int);
extern boolean has_aux(int);
extern boolean has_sfp(int);
extern boolean has_usb_console(int);
extern boolean has_wifi_temp(int);
extern boolean has_power_ok_led(int);
extern boolean has_console_stat_led(int);
extern boolean has_lte_gps(int opt);

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int has_plug_slot(int);
int        tsn_fpga_utils(int);
int        fpga_reset_32_api(uint, uint, uint, uint);
int        fpga_read_32_reg(uint, uint *);
int        fpga_write_32_reg(uint, uint);
int        aikido_read_32_reg(uint, uint *);
int        aikido_write_32_reg(uint, uint);
static int fpga_reg_test_read_fn(ulong, int, ulong *, void *);
static int fpga_reg_test_write_fn(ulong, int, ulong, void *);
int fpga_reg_rd_util(int);
int fpga_reg_wr_util(int);
int aikido_reg_rd_util(int);
int aikido_reg_wr_util(int);
static int fpga_reg_dump_util(int);
int        tsn_show_fpga_ver(int);
int        this_is_tsn_h_sku(void);
int        this_is_tsn_dsl_annex_sku(void);
int        aikido_reg_test (void);
int        tsn_all_leds_off(int);
int        aikido_mailbox_test(void);
boolean show_sirius_fpga_upgd_without_hdr(void);
boolean show_sirius_fpga_upgd_flag(void);
int tsn_status_led_utils(int);
int tsn_pwrok_stat_led_utils(int);
int tsn_poestat_led_utils(int);
int tsn_poeport_led_utils(int);
int tsn_aux_led_utils(int);
int tsn_microusb_led_utils(int);
int tsn_usb_led_utils(int);
int tsn_console_led_utils(int);
int tsn_vpn_led_utils(int);
int tsn_sim_stat_led_utils(int);
int tsn_gps_stat_led_utils(int);
int tsn_rssi_led_utils(int);
int diag_check_ge_ext_intr_no_pending(int);
int diag_check_ge__ext_intr_pending(int);
int diag_check_esw_ext_intr_pending(void);
int diag_check_esw_ext_no_intr_pending(void);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
reg_info_t_ext tsn_fpga_reg_ext = {TSN_FPGA_REG_WIDTH,
                                   fpga_reg_test_read_fn,
                                   fpga_reg_test_write_fn,
                                   0};

static fpga_platform_sku_info fpag_platform_sku_tbl[] = {
    {"C1101_4P",       C1101_4P_CONTROL_TYPE,     FALSE, FALSE, STAR_SKUFEATURE_C1101E2E},
    {"C1101_4PLTEP",   C1101_4PL_CONTROL_TYPE,    TRUE,  FALSE, STAR_SKUFEATURE_C1101P},
    {"C1101_4PLTEPW",  C1101_4PLW_CONTROL_TYPE,   TRUE,  TRUE,  STAR_SKUFEATURE_C1101P},
    {"C1109_2PGB",     C1109_2PGB_CONTROL_TYPE,   TRUE,  FALSE, STAR_SKUFEATURE_C1109_2P},
    {"C1109_2PNA",     C1109_2PNA_CONTROL_TYPE,   TRUE,  FALSE, STAR_SKUFEATURE_C1109_2P},
    {"C1109_2PVZ",     C1109_2PVZ_CONTROL_TYPE,   TRUE,  FALSE, STAR_SKUFEATURE_C1109_2P},
    {"C1109_2PJN",     C1109_2PJN_CONTROL_TYPE,   TRUE,  FALSE, STAR_SKUFEATURE_C1109_2P},
    {"C1109_2PAU",     C1109_2PAU_CONTROL_TYPE,   TRUE,  FALSE, STAR_SKUFEATURE_C1109_2P},
    {"C1109_2PIN",     C1109_2PIN_CONTROL_TYPE,   TRUE,  FALSE, STAR_SKUFEATURE_C1109_2P},
    {"C1109-4P2LTEP",  C1109_4PL_CONTROL_TYPE,    TRUE,  FALSE, STAR_SKUFEATURE_C1109_4P},
    {"C1109_4P2LTEPW", C1109_4PLW_CONTROL_TYPE,   TRUE,  TRUE,  STAR_SKUFEATURE_C1109_4P},
    {"C1118-8P",       C1118_8P_CONTROL_TYPE,     FALSE, FALSE, FPGA_SKUID_TSN_GSHDSL},
    {"C1111X-8P",      C1111X_8P_CONTROL_TYPE,    FALSE, FALSE, 0},
    {"C959_2PUS",      C959_2PUS_CONTROL_TYPE,  TRUE,  FALSE, SUPERNOVA_SKUFEATURE_C959_2P},
    {"C959_2PGB",      C959_2PGB_CONTROL_TYPE,  TRUE,  FALSE, SUPERNOVA_SKUFEATURE_C959_2P},
    {"C959_2PVZ",      C959_2PVZ_CONTROL_TYPE,  TRUE,  FALSE, SUPERNOVA_SKUFEATURE_C959_2P},
    {"C959_2PIN",      C959_2PIN_CONTROL_TYPE,  TRUE,  FALSE, SUPERNOVA_SKUFEATURE_C959_2P},
    {"C951_4P",        C951_4P_CONTROL_TYPE,    FALSE, FALSE, SUPERNOVA_SKUFEATURE_C951_4P},
    { NULL, 0x0000, FALSE, FALSE, 0x0000}
};
static reg_info_t fpga_reg_dump_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"LPC Scratchpad",                FPGA_LPC_SCRATCHPAD_REG,    FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0xFFFFFF07, 0x0},
    {"LPC Status LED Control",        FPGA_LPC_STAT_LED_CTRL_REG, FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000001, 0x1},
    {"External Device Reset",         FPGA_LPC_EXT_DEV_RST_REG,   FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x0000040C, 0x4},
    {"IRQ Test",                      FPGA_IRQ_TEST_REG,          FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0xFF040000, 0x0},
    {"Board Power Cycle",             FPGA_BOARD_PWR_CYCLE_REG,   FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0xFFFF3FFF, 0x0},
    {"LPC Board Type",                FPGA_LPC_BOARDTYPE_REG,     FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x0000000F, 0x0},
    {"FPGA External Device Reset",    FPGA_EXTER_DEV_RST_REG,     FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x03F83ABA, 0x370FAF2},
    {"Internal Device Reset",         FPGA_INT_DEV_RST_REG,       FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0xFFFFFFFF, 0x0},
    {"Board Type",                    FPGA_BOARD_TYPE_REG,        FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x13},
    {"Master FPGA Revision",          FPGA_MASTER_REV_REG,        FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x800103},
    {"FPGA Revision",                 FPGA_REV_REG,               FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x16041901},
    {"FPGA Debug LED",                FPGA_DBG_LED_REG,           FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000000FF, 0x290FF},
    {"CPU Mux and USB Power Control", FPGA_CPUMUX_AND_USBPWR_REG, FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000003, 0x0},
    {"Status and Control",            FPGA_STAT_AND_CTRL_REG,     FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000003EE, 0xE},
    {"Power Status",                  FPGA_PWR_STAT_REG,          FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0xFFFFFFFF},
    {"Card and Power Present",        FPGA_CARD_AND_PWR_REG,      FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x4},
    {"LED",                           FPGA_LED_REG,               FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00FCFE03, 0x0},
    {"LTE RSSI and LED",              FPGA_LTE_RSSI_LED_REG,      FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x0},
    {"Watchdog Strobe",               FPGA_WATCHDOG_REG,          FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x0},
    {"Ext. Interrupt Pending",        FPGA_EXTER_INT_PENDING_REG, FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x10A4},
    {"Ext. Interrupt Mask",           FPGA_EXT_INTR_MASK_REG,     FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x0001FBFF, 0x0},
    {"Force Ext. Interrupt",          FPGA_FORCE_EXT_INTR_REG,    FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x0001FBFF, 0x0},
    {"SFP Status and Control",        FPGA_SFP_AND_CTRL_REG,      FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000001, 0x0},
    {"LTE Control",                   FPGA_LTE_CTL_REG,           FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000000CE, 0x0},
    {"SIM Status and Control",        FPGA_SIM_STATUS_CTL_REG,    FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000306, 0x0},
    {"xDSL Status and Control",       FPGA_DSL_STATUS_CTL_REG,    FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x0},
    {"I2C Master Control",            FPGA_I2C_CTL_REG,           FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x0F03FFE0, 0x0},
    {"I2C Master Status",             FPGA_I2C_STAT_REG,          FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x0},
    {"I2C Master Status Mask",        FPGA_I2C_STAT_MASK_REG,     FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000000FE, 0x0},
    {"I2C Master Slave Addr.",        FPGA_I2C_SLA_ADDR_REG,      FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000003FF, 0x0},
    {"I2C Master Slave SubAddr.",     FPGA_I2C_SLA_SUBADDR_REG,   FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00FFFFFF, 0x0},
    {"I2C Master Bit-Bang",           FPGA_I2C_BIT_BANG_REG,      FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000003, 0x0},
    {"I2C Byte Count",                FPGA_I2C_BYTE_COUNT_REG,    FPGA_RONLY,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00000000, 0x0},
    {"I2C Data FIFO",                 FPGA_I2C_DATA_FIFO_REG,     FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0xFFFE0000, 0x0},
    {"I2C Data FIFO Pointer",         FPGA_I2C_DATA_RW_PTR_REG,   FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000003FF, 0x0},
    {"SPI PROM CONTROL",              FPGA_SPI_CTRL_REG,          FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000087FF, 0x0},
    {"SPI PROM Status",               FPGA_SPI_STAT_REG,          FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x00008001, 0x0},
    {"SPI PROM Read Size",            FPGA_SPI_RD_SIZE_REG,       FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000000FF, 0x0},
    {"SPI PROM R/W Data",             FPGA_SPI_RW_DATA_REG,       FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0x000000FF, 0x0},
    {"SPI PROM OP Code/Addr.",        FPGA_SPI_OP_ADDR_REG,       FPGA_RW,
        {(unsigned long)&tsn_fpga_reg_ext},   0xFFFFFFFF, 0x0},
};

/*
 * FPGA Utilities
 */
static submenu_xtable_t fpga_utils_tbl[] = {
    {"Show System FPGA version",   (type_t(*)())tsn_show_fpga_ver,  0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Show Pluggable FPGA version",   (type_t(*)())show_plug_fpga_ver,  0, 0,
     (type_t(*)())this_is_star_with_sirius_fpga, 0,     (type_t(*)())0, 0},
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
    {"CPU register Read",   (type_t(*)())tsn_cpureg_rd_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPU register Write",  (type_t(*)())tsn_cpureg_wr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
#if 0   /* TSN_DIAG: Temporarily masked them, will implement later. */
    {"Display FPGA Flash content", (type_t(*)())display_fpga_flash_content, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Erase FPGA Flash", (type_t(*)())erase_fpga_flash_sector, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
    {"Program FPGA SPI PROM image",  (type_t(*)())program_reggio_spi_prom, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    /* Avoid to confuse users, combine item o and p(with/without header). 
     * Temporarily masked out "Program Pluggable FPGA SPI PROM image without 
     * header" item from menu. 
     */
    {"Program Pluggable FPGA SPI PROM image without header", (type_t(*)())plug_fpga_spi_prog, 0, 0,
     (type_t(*)())show_sirius_fpga_upgd_without_hdr, 0, (type_t(*)())0,   0},
    {"Program Pluggable FPGA SPI PROM image", (type_t(*)())plug_fpga_spi_prog, 0, 0,
     (type_t(*)())this_is_star_with_sirius_fpga, 0, (type_t(*)())0,   0},
    {"Pluggable FPGA Erase/Program Image Upgrade Header", (type_t(*)())plug_fpga_erase_header, 1, 0,
     (type_t(*)())this_is_star_with_sirius_fpga, 0, (type_t(*)())0,   0},
    {"Set Pluggable FPGA revision and date", (type_t(*)())plug_fpga_set_date_revision, 1, 0,
     (type_t(*)())this_is_star_with_sirius_fpga, 0, (type_t(*)())0,   0},
    {"Set Pluggable FPGA update flag", (type_t(*)())plug_fpga_set_update_flag, 1, 0,
     (type_t(*)())show_sirius_fpga_upgd_flag, 0, (type_t(*)())0,   0},
    {"Display Pluggable FPGA PROM sector", (type_t(*)())plug_fpga_display_sector, 1, 0,
     (type_t(*)())this_is_star_with_sirius_fpga, 0, (type_t(*)())0, 0},
    {"Modify SPI Directory Table",  (type_t(*)())program_spi_update_version, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"xDSL annex type",  (type_t(*)())this_is_tsn_dsl_annex_sku, 0, 0,
     (type_t(*)())has_xdsl, 0,     (type_t(*)())0, 0},
    {"Compare FPGA SKU info with cookie record",  (type_t(*)())check_fpga_sku_info, 0, 0,
     (type_t(*)())has_fpga_sku_check, 0,     (type_t(*)())0, 0},
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
    {"Status LED utils", (type_t(*)())tsn_status_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Power OK stat LED utils", (type_t(*)())tsn_pwrok_stat_led_utils, TRUE,
     0,
      (type_t(*)())has_power_ok_led, 0, (type_t(*)())0, 0},
    {"POE present stat LED utils", (type_t(*)())tsn_poestat_led_utils, TRUE,
     0,
      (type_t(*)())has_poe, 0, (type_t(*)())0, 0},
    {"POE port0 stat LED utils", (type_t(*)())tsn_poeport_led_utils, 0,
     0,
      (type_t(*)())has_poe, 0, (type_t(*)())0, 0},
    {"POE port1 stat LED utils", (type_t(*)())tsn_poeport_led_utils, 1,
     0,
      (type_t(*)())has_poe, 0, (type_t(*)())0, 0},
    {"POE port2 stat LED utils", (type_t(*)())tsn_poeport_led_utils, 2, 
     0,
      (type_t(*)())this_is_tsn_h_sku, 0, (type_t(*)())0, 0},
    {"POE port3 stat LED utils", (type_t(*)())tsn_poeport_led_utils, 3,
     0,
      (type_t(*)())this_is_tsn_h_sku, 0, (type_t(*)())0, 0},
    {"AUX stat LED utils", (type_t(*)())tsn_aux_led_utils, TRUE,
     0,
      (type_t(*)())has_aux, 0, (type_t(*)())0, 0},
    {"Micro USB stat LED utils", (type_t(*)())tsn_microusb_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB stat LED utils", (type_t(*)())tsn_usb_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Console stat LED utils", (type_t(*)())tsn_console_led_utils, TRUE,
     0,
      (type_t(*)())has_console_stat_led, 0, (type_t(*)())0, 0},
    {"VPN stat LED utils", (type_t(*)())tsn_vpn_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE0 stat LED utils", (type_t(*)())tsn_ge_led_utils, 0,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE1 stat LED utils", (type_t(*)())tsn_ge_led_utils, 1,
     0,
      (type_t(*)())tsn_has_2nd_ge, 0, (type_t(*)())0, 0},
    {"GE Switch port LED utils",  (type_t(*)())tsn_esw_force_led_onoff_util,
     0, 0, 
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"XDSL LED OFF",     (type_t(*)())wrap_bcm63168_led_utils, LED_OFF, 0,
      (type_t(*)())has_xdsl, 0,   (type_t(*)())0,          0},
    {"XDSL LED CD ON",   (type_t(*)())wrap_bcm63168_led_utils, LED_CD_ON, 0,
      (type_t(*)())has_xdsl, 0,   (type_t(*)())0,          0},
    {"XDSL LED Data ON", (type_t(*)())wrap_bcm63168_led_utils, LED_DATA_ON, 0,
      (type_t(*)())has_xdsl, 0,   (type_t(*)())0,          0},
    {"WLAN LED ON in RED",  (type_t(*)())tsn_wifi_led_control, WIFI_LED_RED, 0,
     (type_t(*)())tsn_fpga_check_dev_present, FPGA_CPP_WLAN_PRESENT,
     (type_t(*)())0,          0},
    {"WLAN LED ON in GREEN", (type_t(*)())tsn_wifi_led_control, 
        WIFI_LED_GREEN, 0,
     (type_t(*)())tsn_fpga_check_dev_present, FPGA_CPP_WLAN_PRESENT,
     (type_t(*)())0,          0},
    {"WLAN LED ON in AMBER",  (type_t(*)())tsn_wifi_led_control, 
        WIFI_LED_AMBER, 0,
     (type_t(*)())tsn_fpga_check_dev_present, FPGA_CPP_WLAN_PRESENT,
     (type_t(*)())0,          0},
    {"WLAN LED OFF",  (type_t(*)())tsn_wifi_led_control, WIFI_LED_OFF, 0,
     (type_t(*)())tsn_fpga_check_dev_present, FPGA_CPP_WLAN_PRESENT,
     (type_t(*)())0,          0},
    {"LTE SIM 0 stat LED utils", (type_t(*)())tsn_sim_stat_led_utils, 0,
     0,
     (type_t(*) ())tsn_fpga_check_dev_present, FPGA_CPP_LTE0_PRESENT,
     (type_t(*)())0, 0},
    {"LTE SIM 1 stat LED utils", (type_t(*)())tsn_sim_stat_led_utils, 1,
     0,
     (type_t(*) ())tsn_fpga_check_dev_present, FPGA_CPP_LTE0_PRESENT,
     (type_t(*)())0, 0},
    {"LTE GPS stat LED utils", (type_t(*)())tsn_gps_stat_led_utils, TRUE,
     0,
     (type_t(*) ())tsn_fpga_check_dev_present, FPGA_CPP_LTE0_PRESENT,
     (type_t(*)())0, 0},
    {"LTE RSSI stat LED utils", (type_t(*)())tsn_rssi_led_utils, TRUE,
     0,
     (type_t(*) ())tsn_fpga_check_dev_present, FPGA_CPP_LTE0_PRESENT,
     (type_t(*)())0, 0},
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
 * Function   : has_plug_slot
 * Description: Function to check whether this platform has pluggable or not
 * Inputs     : slot_num - Pluggable slot number (Start from 1)
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_plug_slot (int slot)
{
    if (this_is_star_c1101p() || this_is_star_c1109_4p()) {
        return (TRUE);
    }
    return (FALSE);
}


/*******************************************************************************
 *
 * Function    : usb_console_to_uart
 * Description : Function to check UART link withUSB console or not.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int usb_console_to_uart (int opt)
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
 * Function    : tsn_all_yellow_leds_on
 * Description : Function to turn TSN all Yellow LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_all_yellow_leds_on (int opt)
{
    uint reg_offset = 0, reg_val = 0;

    /* Turn all LEDs OFF */
    if (tsn_all_leds_off(0) != PASSED) {
        printf("%s: Failed to turn all LEDs OFF.\n", __FUNCTION__);
        return (FAILED);
    }

    if (tsn_has_2nd_ge(0) == TRUE) {
    if (has_xdsl(0) == FALSE) { 
        if (tsn_all_ge_leds_off(TSN_GE1_ETHNUM) != PASSED) {
            printf("%s: Failed to turn all GE1 LEDs ON.\n", __FUNCTION__);
        }
    }
    }

    /* Turn Status LED to Yellow */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = STAT_LED_Y;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    reg_offset = (uint)FPGA_LTE_RSSI_LED_REG;
    reg_val = (uint)(LTE_MOD_2G3G_SIGNAL_AMBER | LTE_MOD_HIGH_RSSI); 
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%08X to FPGA Reg0x%#X.\n",
               __FUNCTION__, reg_val, reg_offset);
    }

    if (tsn_all_ge_leds_off(TSN_GE0_ETHNUM) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs ON.\n", __FUNCTION__);
    }

    reg_offset = (uint)FPGA_LED_REG;
    if (this_is_star() || this_is_supernova()) {
        reg_val = (uint)(LTE_SIM1_STAT_LED_Y | LTE_SIM0_STAT_LED_Y |
                     POE_PRESENT_LED_Y | POE_P0_LED |
                     POE_P1_LED | CONSOLE_LED);
        if (has_lte_gps(0)) {
            reg_val |= (uint)(LTE_GPS_STAT_LED_Y);
        }
    } else {
    reg_val = (uint)(LTE_SIM1_STAT_LED_Y | LTE_SIM0_STAT_LED_Y |
                     LTE_GPS_STAT_LED_Y | POE_PRESENT_LED_Y | POE_P0_LED |
                     POE_P1_LED | POE_P2_LED | POE_P3_LED | AUX_LED);
    }
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    if ((this_is_star() || this_is_supernova()) && (this_is_star_c1109_2p()==FALSE) && (this_is_supernova_c959_2p()==FALSE)) {
    reg_offset = (uint)FPGA_LTE_RSSI_LED_REG;
    reg_val = 0x5050;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_all_green_leds_on
 * Description : Function to turn TSN all Green LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_all_green_leds_on (int opt)
{
    uint reg_offset = 0, reg_val = 0;

    /* Turn all LEDs OFF */
    if (tsn_all_leds_off(0) != PASSED) {
        printf("%s: Failed to turn all LEDs OFF.\n", __FUNCTION__);
        return (FAILED);
    }

    reg_offset = (uint)FPGA_LED_REG;
    if (this_is_star() || this_is_supernova()) {
        reg_val = (uint)(LTE_SIM1_STAT_LED_G | LTE_SIM0_STAT_LED_G | 
                     POE_PRESENT_LED_G | POE_P3_LED |
                     POE_P2_LED | POE_STAT_LED | AUX_LED | MICRO_USB_LED | 
                     USB_LED | VPN_OK_LED);
        if (has_lte_gps(0)) {
            reg_val |= (uint)(LTE_GPS_STAT_LED_G);
        }
    } else {
    reg_val = (uint)(LTE_SIM1_STAT_LED_G | LTE_SIM0_STAT_LED_G | 
                     LTE_GPS_STAT_LED_G | POE_PRESENT_LED_G | POE_STAT_LED |
                     MICRO_USB_LED | USB_LED | VPN_OK_LED | CONSOLE_LED);
    }
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    reg_offset = (uint)FPGA_LTE_RSSI_LED_REG;
    reg_val = 0xB0B0;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    /* Turn Status LED to Green */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = STAT_LED_G;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    reg_offset = (uint)FPGA_LTE_RSSI_LED_REG;
    reg_val = (uint)(LTE_MOD_LTE_SIGNAL_GREEN | LTE_MOD_HIGH_RSSI); 
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%08X to FPGA Reg0x%#X.\n",
               __FUNCTION__, reg_val, reg_offset);
    }

    if (tsn_all_ge_leds_on(TSN_GE0_ETHNUM) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs ON.\n", __FUNCTION__);
    }

    if (tsn_has_2nd_ge(0) == TRUE) {
    if (has_xdsl(0) == FALSE) { 
        if (tsn_all_ge_leds_on(TSN_GE1_ETHNUM) != PASSED) {
            printf("%s: Failed to turn all GE1 LEDs ON.\n", __FUNCTION__);
        }
    }
    }

    if (tsn_esw_force_led_onoff((int)ALL_ESW_LEDS,
                                (boolean)ESW_LED_F_ON) != PASSED) {
            printf("%s: Failed to turn all ESW LEDs ON.\n", __FUNCTION__);
    }

    /* If this is Star with pluggable slot, turn on pluggable FPGA debug LEDs */
    if (has_plug_slot(PLUG_SLOT_1)) {
        reg_offset = (uint) PLUG_FPGA_DBG_LED_ADDR_REG;
        reg_val = 0x00;
        reg_val = PLUG_DBG_LED_ON;
        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_all_leds_off
 * Description : Function to turn TSN all LEDs OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_all_leds_off (int opt)
{
    uint reg_offset = 0, reg_val = 0;

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = 0x000000;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    reg_offset = (uint)FPGA_LTE_RSSI_LED_REG;
    reg_val = 0x0000;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    /* Turn Power OK and Status LED off */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = (uint)(PWR_OK_LED_OFF | STAT_LED_OFF);
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    if (tsn_all_ge_leds_off(TSN_GE0_ETHNUM) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs OFF.\n", __FUNCTION__);
    }

    if (tsn_has_2nd_ge(0) == TRUE) {
    if (has_xdsl(0) == FALSE) { 
        if (tsn_all_ge_leds_off(TSN_GE1_ETHNUM) != PASSED) {
            printf("%s: Failed to turn all GE1 LEDs OFF.\n", __FUNCTION__);
        }
    }
    }

    if (tsn_esw_force_led_onoff((int)ALL_ESW_LEDS,
                                (boolean)ESW_LED_F_OFF) != PASSED) {
            printf("%s: Failed to turn all ESW LEDs OFF.\n", __FUNCTION__);
    }

    /* If this is Star with pluggable slot, turn off pluggable FPGA debug LEDs */
    if (has_plug_slot(PLUG_SLOT_1)) {
        reg_offset = (uint) PLUG_FPGA_DBG_LED_ADDR_REG;
        reg_val = PLUG_DBG_LED_OFF;
        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_leds_test
 * Description : Function to turn TSN all LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_leds_test (int opt)
{
    uint reg_offset = 0, reg_val = 0;

    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If any failure occurs in this test, "
                    "try the Debugging Steps to narrow down the issue.",
                    "Check the LED GPIO to see if its' "
                    "implementation is identical to FPGA specification.",
                    "If step a is OK, check the GPIO interface "
                    "between MB and FPGA.");
#endif

    char *tname = "TSN LEDS";
    int rc = PASSED;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Turn all LEDs OFF*/
    printf("Turn all LEDs Off\n");
    tsn_all_leds_off(0); 
    sleep(1);   

    /* Turn LED Green ON */
    printf("Need Visual testing!!\n");
    printf("Turn LED Green ON\n");
    tsn_all_green_leds_on(0); 
    sleep(1);

    /* Turn LED Green OFF*/
    printf("Turn LED Green OFF\n");
    tsn_all_leds_off(0); 
    sleep(1);   

    /* Turn LED Yellow ON */
    printf("Turn LED Yellow ON\n");
    tsn_all_yellow_leds_on(0); 
    sleep(1);   

    /* Turn LED Yellow OFF*/
    printf("Turn LED Yellow OFF\n");
    tsn_all_leds_off(0); 
    sleep(1);   

    /* Turn Power OK LED */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    if (this_is_star() || this_is_supernova()) {
        reg_val = (uint)(PWR_OK_LED | STAT_LED_G);
    } else {
        reg_val = (uint)(PWR_OK_LED);
    }
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*******************************************************************************
 *
 * Function    : tsn_fpga_utils
 * Description : Function to show TSN FPGA utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_fpga_utils (int opt)
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
 * Inputs     : *result - buffer to put the check result(TRUE/FALSE)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int is_sfp_present (int *result)
{
    uint reg_offset = (uint)FPGA_SFP_AND_CTRL_REG;
    uint reg_val = 0;

    /* Read SFP status and control register */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA SFP status and Control Reg(0x%04X).\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if ((reg_val & (uint)SFP_SC_MODULE_DEF) == (uint)SFP_SC_MODULE_DEF) {
        *result = TRUE;
    } else {
        *result = FALSE;
    }

    return (PASSED);
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
 * Function    : fpga_reg_rd_util
 * Description : Utility to read TSN FPGA register.
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
 * Function    : aikido_reg_rd_util
 * Description : Utility to read TSN Aikido register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_reg_rd_util (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffff): ",
                               0x2008, 0, 0xffff);

    if (aikido_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("FPGA register(0x%04X) = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reg_dump_util
 * Description : Utility to dump TSN FPGA all registers.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int fpga_reg_dump_util (int opt)
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
 * Function    : fpga_read_32_reg
 * Description : Function to read TSN FPGA register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_read_32_reg (uint reg_offset, uint *buf)
{
    uint offset = 0;

    offset = (uint)(tsn_fpga_reg_baseaddr + reg_offset);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Device Bus address 0x%08X\n", offset);
    }
    if (tsn_mem_read32(offset, buf) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}
/*******************************************************************************
 *
 * Function    : aikido_read_32_reg
 * Description : Function to read TSN Aikido register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_read_32_reg (uint reg_offset, uint *buf)
{
    uint offset = 0;

    offset = (uint)(tsn_aikido_reg_baseaddr + reg_offset);
    if ((NVRAM)->diagflag & D_VERBOSE) {
    printf("Device Bus address 0x%08X\n", offset);
    }
    if (tsn_mem_read32(offset, buf) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_reg_wr_util
 * Description : Utility to write TSN FPGA register.
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
 * Function    : aikido_reg_wr_util
 * Description : Utility to write TSN Aikido register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffff): ",
                               0, 0, 0xffff);

    if (aikido_read_32_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }

    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (aikido_write_32_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               reg_val, reg_offset);
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

    offset = (uint)(tsn_fpga_reg_baseaddr + reg_offset);

    if (tsn_mem_write32(offset, wr_data) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}
/*******************************************************************************
 *
 * Function    : aikido_write_32_reg
 * Description : Function performs Aikido register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_write_32_reg (uint reg_offset, uint wr_data)
{
    uint offset = 0;

    offset = (uint)(tsn_aikido_reg_baseaddr + reg_offset);
    if ((NVRAM)->diagflag & D_VERBOSE) {
    printf("Device Bus address 0x%08X\n", offset);
    }
    if (tsn_mem_write32(offset, wr_data) != PASSED) {
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
        printf("%s: Failed to read TSN/STAR FPGA Reg(0x%lx).\n",
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
 * Function   : tsn_get_boardtype
 * Description: Function to get TSN board type.
 *              This is by reading FPGA LPC Board Type Reg(0x80).
 * Inputs     : *b_type - buffer to put the read back board type value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_get_boardtype (uint *b_type)
{
    if (fpga_read_32_reg((uint)FPGA_LPC_SKUFEATURE_REG, b_type) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_show_fpga_ver
 * Description: Function to show FPGA version.
 *              This is by reading TSN FPGA Revision Reg(0x108C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_show_fpga_ver (int opt)
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
 * Function   : this_is_tsn
 * Description: Function to distinguish this TSN
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_tsn (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (((btype & FPGA_BTYPE_SUB_HIGH_MASK) >> FPGA_BTYPE_SUB_HIGH_SHIFT) 
              | (btype & FPGA_BTYPE_SUB_LOW_MASK));
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_TSN) {
        return (TRUE);
    }
    return (FALSE);
}


/*******************************************************************************
 *
 * Function   : this_is_star
 * Description: Function to distinguish this is Star
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_star (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (((btype & FPGA_BTYPE_SUB_HIGH_MASK) >> FPGA_BTYPE_SUB_HIGH_SHIFT) 
              | (btype & FPGA_BTYPE_SUB_LOW_MASK));
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_STAR) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_supernova
 * Description: Function to distinguish this is Supernova
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_supernova (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (((btype & FPGA_BTYPE_SUB_HIGH_MASK) >> FPGA_BTYPE_SUB_HIGH_SHIFT) 
              | (btype & FPGA_BTYPE_SUB_LOW_MASK));
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_SUPERNOVA) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_star_c1101p
 * Description: Function to distinguish sku feature with Star
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_star_c1101p (void)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0;
     
    if (this_is_not_star()) {
        return (FALSE);
    }

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (btype & STAR_SKUFEATURE_MASK); 
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }

    if (btype == STAR_SKUFEATURE_C1101P) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_star_c1101e2e
 * Description: Function to distinguish sku feature with Star
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_star_c1101e2e (void)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0;
    
    if (this_is_not_star()) {
        return (FALSE);
    }

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (btype & STAR_SKUFEATURE_MASK); 
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }

    if (btype == STAR_SKUFEATURE_C1101E2E) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_supernova_c951_4p
 * Description: Function to distinguish sku feature with Supernova
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_supernova_c951_4p (void)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0;
    
    if (this_is_not_supernova()) {
        return (FALSE);
    }

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (btype & STAR_SKUFEATURE_MASK); 
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }

    if (btype == SUPERNOVA_SKUFEATURE_C951_4P) {
        return (TRUE);
    }
    return (FALSE);
}


/*******************************************************************************
 *
 * Function   : this_is_star_c1109_4p
 * Description: Function to distinguish sku feature with Star
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_star_c1109_4p (void)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0;
    
    if (this_is_not_star()) {
        return (FALSE);
    }

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (btype & STAR_SKUFEATURE_MASK); 
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }

    if (btype == STAR_SKUFEATURE_C1109_4P) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_star_c1109_2p
 * Description: Function to distinguish sku feature with Star
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_star_c1109_2p (void)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0;
    
    if (this_is_not_star()) {
        return (FALSE);
    }

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (btype & STAR_SKUFEATURE_MASK); 
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }

    if (btype == STAR_SKUFEATURE_C1109_2P) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_not_star
 * Description: Function this is not Star
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_not_star (void)
{
    return (!this_is_star());
}

/*******************************************************************************
 *
 * Function   : this_is_not_supernova
 * Description: Function this is not Supernova
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_not_supernova (void)
{
    return (!this_is_supernova());
}

/*******************************************************************************
 *
 * Function   : this_is_supernova_c959_2p
 * Description: Function to distinguish sku feature with Supernova
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_supernova_c959_2p (void)
{
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0;
    
    if (this_is_not_supernova()) {
        return (FALSE);
    }

    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = (btype & STAR_SKUFEATURE_MASK); 
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }

    if (btype == SUPERNOVA_SKUFEATURE_C959_2P) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_star_with_sirius_fpga
 * Description: Function to distinguish whether this star platform is with 
 *              Sirius FPGA or not.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_star_with_sirius_fpga (void)
{
    if ((this_is_star_c1109_4p() || this_is_star_c1101p()) == TRUE) {
        return (TRUE);
    }

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : star_io_interface_show
 * Description: Function to check need to show IO interface or not
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int star_io_interface_show (void)
{
    if ((this_is_star_with_sirius_fpga() || this_is_tsn()) == TRUE) {
        return (TRUE);
    }

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_tsn_gshdsl_sku
 * Description: Function to distinguish TSN-GSHDSL.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_tsn_gshdsl_sku (void)
{
    uint reg_addr = (uint)FPGA_REV_REG;
    uint fpga_ver = 0, board_type;

    if (fpga_read_32_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("%s: Failed to read FPGA Revision Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FALSE);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA version %08X.\n", __FUNCTION__, fpga_ver);
    }

    if (fpga_ver >= TSN_M_P1A_FPGA_VER) {
        reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;

        if (fpga_read_32_reg(reg_addr, &board_type) != PASSED) {
            printf("%s: Failed to read FPGA Board Type Reg(0x%04X).\n",
                   __FUNCTION__, reg_addr);
            return (FALSE);
        }

        if (board_type  == FPGA_SKUID_TSN_GSHDSL) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is TSN-GSHDSL.\n", __FUNCTION__);
            }
            return (TRUE);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is Non TSN-GSHDSL.\n", __FUNCTION__);
            }
            return (FALSE);
        }
    }

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_tsn_h_sku
 * Description: Function to distinguish TSN-H and TSN-M.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_tsn_h_sku (void)
{
    uint reg_addr = (uint)FPGA_REV_REG;
    uint fpga_ver = 0, board_type;

    if (fpga_read_32_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("%s: Failed to read FPGA Revision Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA version %08X.\n", __FUNCTION__, fpga_ver);
    }

    if (fpga_ver >= TSN_M_P1A_FPGA_VER) {
        reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;

        if (fpga_read_32_reg(reg_addr, &board_type) != PASSED) {
            printf("%s: Failed to read FPGA Board Type Reg(0x%04X).\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        if ((board_type & (uint)FPGA_SKUID_TSN_H) == (uint)FPGA_SKUID_TSN_H) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is TSH-H.\n", __FUNCTION__);
            }
            return (TRUE);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is TSH-M.\n", __FUNCTION__);
            }
            return (FALSE);
        }
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : tsn_fpga_check_dev_present
 * Description: Function to check if TSN module is present.
 *              This is by read FPGA Card and Power present Reg.(0x1118).
 * Inputs     : mod_type - TSN device type(WLAN, LTE, PoE)
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
boolean tsn_fpga_check_dev_present (uint mod_type)
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
 * Function   : this_is_tsn_dsl_annex_sku
 * Description: Function to distinguish dsl_annex A/B/M/J.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_tsn_dsl_annex_sku (void)
{
    uint reg_addr = (uint)FPGA_REV_REG;
    uint fpga_ver = 0, board_type;
    struct GFAST_CID gfast_annex_cid_map[15] = {{(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A), GF_Annex_A},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A), GF_Annex_A_LTE},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A), GF_Annex_A_WLAN},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A), GF_Annex_A_LTE_WLAN},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_M), GF_Annex_M},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_M), GF_Annex_M_LTE},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_M), GF_Annex_M_WLAN},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B), GF_Annex_B},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B), GF_Annex_B_LTE},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B), GF_Annex_B_WLAN},
                                               {(DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B), GF_Annex_B_LTE_WLAN}};

    int gfast_cid_map_size = sizeof(gfast_annex_cid_map)/sizeof(GFAST_CID);
    int ix;
    int cid = 0;
    int cid_val = 0;
    int annex_btype;    

    if (fpga_read_32_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("%s: Failed to read FPGA Revision Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA version %08X.\n", __FUNCTION__, fpga_ver);
    }

    if (fpga_ver >= TSN_M_P1A_FPGA_VER) {
        reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;

        if (fpga_read_32_reg(reg_addr, &board_type) != PASSED) {
            printf("%s: Failed to read FPGA Board Type Reg(0x%04X).\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }
        if (((board_type & ((uint)FPGA_SKUID_TSN_H))) == ((uint)FPGA_SKUID_TSN_H) &&  /* TSN-G.Fast */
            ((board_type & (1 << BOARD_TYPE_DSL_SHIFT)) == SUPPORT_DSL_SKU))  {    
            /* get control type to distinguish annex mode of G.Fast */
                cid = get_mb_id();

                /* Get corresponding value according to controller type */
                for (ix = 0; ix < gfast_cid_map_size; ix++) {
                    if (gfast_annex_cid_map[ix].cid == cid) {
                        cid_val = gfast_annex_cid_map[ix].val;
                        break;
                        }
                    }
                if (cid_val == 0) {
                    printf("%s: Unknown controller type (%#x)\n", __func__, cid);
                    return (FALSE);
                }

                annex_btype = (board_type >> BOARD_TYPE_ANNEX_SHIFT) & 0x3;
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("%s: Annex value %d  from board type\n", __func__, annex_btype);
                }
                /* Check whether FPGA Annex setting matches with Controller Type */
                if (((annex_btype == BOARD_TYPE_ANNEX_A) &&
                     (cid_val & DSL138_SKU_ANNEX_A)) ||
                    ((annex_btype == BOARD_TYPE_ANNEX_M) &&
                     (cid_val & DSL138_SKU_ANNEX_M)) ||
                    ((annex_btype == BOARD_TYPE_ANNEX_BJ) &&
                     (cid_val & DSL138_SKU_ANNEX_B))) {
                    tsn_gfast_sku = TRUE;
                    return (cid_val);
                }
                printf("%s:  Failed to match CID with FPGA Board Type Reg(0x%4X).\n",
                        __func__, reg_addr);
                return (FALSE);
        } else /* TSN-M */
        if (((board_type & ((uint)FPGA_SKUID_ANNEX_A)) == ((uint)FPGA_SKUID_ANNEX_A)) &&
            ((board_type & ((uint)FPGA_SKUID_ANNEX_M)) == ((uint)FPGA_SKUID_ANNEX_M &
            ((uint)~FPGA_SKUID_ANNEX_M)))) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is xDSL Annex A.\n", __FUNCTION__);
                }
                tsn_gfast_sku = FALSE;
            return (DSL_SKU_ANNEX_A);
       } else
        if (((board_type & ((uint)FPGA_SKUID_ANNEX_M)) == ((uint)FPGA_SKUID_ANNEX_M)) &&
            ((board_type & ((uint)FPGA_SKUID_ANNEX_A)) == ((uint)FPGA_SKUID_ANNEX_A &
            ((uint)~FPGA_SKUID_ANNEX_A)))) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is xDSL Annex M.\n", __FUNCTION__);
                }
                tsn_gfast_sku = FALSE;
            return (DSL_SKU_ANNEX_M);
        } else
         if (((board_type & ((uint)FPGA_SKUID_ANNEX_M)) == ((uint) FPGA_SKUID_ANNEX_M &
             ((uint)~FPGA_SKUID_ANNEX_M))) &&
             ((board_type & ((uint)FPGA_SKUID_ANNEX_A)) == ((uint) FPGA_SKUID_ANNEX_A &
             ((uint)~FPGA_SKUID_ANNEX_A)))) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: This is xDSL Annex B.\n", __FUNCTION__);
                }
                tsn_gfast_sku = FALSE;
            return (DSL_SKU_ANNEX_B);
        } else { 
                if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: Unknown xDSL Annex Type.\n", __FUNCTION__);
                }
            return (FALSE); 
        }
    }
    return (TRUE);
}


/*******************************************************************************
 *  
 * Function    : aikido_reg_test
 * Description : Utility to test TSN aikido register.
 * Inputs      : NONE
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_reg_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If any failure occurs in this test, "
                    "check the GPIO interface between MB and FPGA.");
#endif

    char *tname = "Aikido Register";

    uint reg_offset = FPGA_AIKIDO_REG, rd_reg_val = 0x0, wr_reg_val = 0x1;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    // 1. write 0x1
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               wr_reg_val, reg_offset);
    }

    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if (rd_reg_val != 0x1) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x1 instead of 0x%x",
              __FUNCTION__, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    // 2. write 0x0
    wr_reg_val = 0x0;

    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               wr_reg_val, reg_offset);
    }

    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if (rd_reg_val == 0x0) {
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    } else {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x0 instead of 0x%x",
              __FUNCTION__, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
}


/*******************************************************************************
 *
 * Function    : check_fpga_sku_infop 
 * Description : Check FGPA provide SKU with platform cookie.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int check_fpga_sku_info (void)
{
    fpga_platform_sku_info *fpgap;
    uint16_t cookid = 0;
    uint reg_addr = (uint)FPGA_LPC_SKUFEATURE_REG;
    uint btype = 0, mbtype = 0;
    boolean ret;


    cookid = (uint16_t)get_mb_id();
    fpgap = fpag_platform_sku_tbl;

    /* Search match platform cookie table */
    while (fpgap->platform_name != NULL) {
        if (fpgap->cook_contype == cookid) {
            printf("%s   cookie = 0x%04x.\n", fpgap->platform_name, cookid);
            break;
        }
        fpgap++;
    }
    if (fpgap->platform_name == NULL) {
        printf("*** WARNING: Could not find correct FPGA SKU info.\n");
        return (FAILED);
    }

    /* Check LTE, WIFI and Platform */
    if (fpga_read_32_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Board skufeature Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA skufeature %08X.\n", __FUNCTION__, btype);
    }
    
    ret = (btype & TSN_W_WIFI) ? TRUE : FALSE;
    if (ret != fpgap->haswifi) {
        printf("*** WARNING: FPGA show SKU WIFI info not match with cookie record.\n");
        return (FAILED);
    }

    if (cookid != C1118_8P_CONTROL_TYPE) {
        mbtype = (btype & STAR_SKUFEATURE_MASK); 
        if (mbtype != fpgap->platform) {
            printf("*** WARNING: FPGA show platfrom SKU not match with cookie record.\n");
            return (FAILED);
        } 
    }
    printf("%s FPGA SKU info match with cookie.\n", fpgap->platform_name);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : show_sirius_fpga_upgd_without_hdr 
 * Description : Function to determine whether to show Sirius FPGA upgrade 
 *               without header utility or not.
 * Inputs      : None
 * Outputs     : TRUE/FALSE
 *
 *******************************************************************************
 */
boolean show_sirius_fpga_upgd_without_hdr (void)
{
    return (FALSE);
}


/*******************************************************************************
 *
 * Function    : show_sirius_fpga_upgd_flag 
 * Description : Function to determine whether to show set Sirius FPGA upgrade 
 *               utility or not.
 * Inputs      : None
 * Outputs     : TRUE/FALSE
 *
 *******************************************************************************
 */
boolean show_sirius_fpga_upgd_flag (void)
{
    return (FALSE);
}
/*******************************************************************************
 *  
 * Function    : aikido_mailbox_test
 * Description : Utility to test Aikido mailbox.
 * Inputs      : NONE
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_mailbox_test (void)
{
    char *tname = "Aikido Mailbox";
    uint reg_offset = 0x0, rd_reg_val = 0x0, wr_reg_val = 0x0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Step 1. Set MBX enable */
    reg_offset = FPGA_AIKIDO_MBX_REG;
    wr_reg_val = MBX_EN_FLAGS_OR;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "\n%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    /* Step 2. Read MBX enable reg */
    reg_offset = FPGA_AIKIDO_MBX_REG;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_EN_FLAGS_OR) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_EN_FLAGS_OR, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 3. write Get SCC ID cmd */
    reg_offset = FPGA_AIKIDO_MBX_DPRAM_REG;
    wr_reg_val = MBX_GET_SCC_ID_CMD;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    /* Step 4. Send interrupt to TAM FW */
    reg_offset = FPGA_AIKIDO_MBX_INTSTAT_REG;
    wr_reg_val = MBX_H2M_FLAGS;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    /* Step 5. Read status bit, 0x08 means data is ready */
    reg_offset = FPGA_AIKIDO_MBX_INTCRTL_REG;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_FLAGS_OR) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_FLAGS_OR, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 6.  Read actual SCC ID 1 */
    reg_offset = FPGA_AIKIDO_MBX_DPRAM_REG;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_SCC_ID_1) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_SCC_ID_1, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 7.  Read actual SCC ID 2 */
    reg_offset = FPGA_AIKIDO_MBX_DPRAM_REG + MBX_DPRAM_OFFSET_FOUR ;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_SCC_ID_2) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_SCC_ID_2, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 8.  Clear Flag, ack FW */
    reg_offset = FPGA_AIKIDO_MBX_H2M_FLAGS_REG;
    wr_reg_val = MBX_M2H_FLAGS;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    printf("\n%s test PASSED ", tname);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_status_led_utils
 * Description : Function to turn TSN status LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_status_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    
    printf("\n"); 
    printf("TSN Status LED utils: \n"); 
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
 * Function    : tsn_pwrok_stat_led_utils
 * Description : Function to turn TSN Power OK LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_pwrok_stat_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    
    printf("\n"); 
    printf("TSN Power OK LED utils: \n"); 
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
 * Function    : tsn_sim_stat_led_utils
 * Description : Function to turn TSN SIM LEDs ON/OFF.
 * Inputs      : slot - Sim slots 
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_sim_stat_led_utils (int slot)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN LTE SIM %d Status LED utils: \n", slot); 
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 2): ", 0, 0, 2);
    
    if (option == 0) {
        if (slot == 0) {
            reg_val = (uint)(LTE_SIM0_STAT_LED_OFF);
        } else {
            reg_val = (uint)(LTE_SIM1_STAT_LED_OFF);
        }
    } else if (option == 1 ) {
        if (slot == 0) {
            reg_val = (uint)(LTE_SIM0_STAT_LED_Y);
        } else {
            reg_val = (uint)(LTE_SIM1_STAT_LED_Y);
        }
    } else if (option == 2 ) {
        if (slot == 0) {
            reg_val = (uint)(LTE_SIM0_STAT_LED_G);
        } else {
            reg_val = (uint)(LTE_SIM1_STAT_LED_G);
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
 * Function    : tsn_gps_stat_led_utils
 * Description : Function to turn TSN GPS LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_gps_stat_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    if (this_is_supernova()) {
        printf("Supernova doesn't support GPS.\n");
        return (FAILED);
    }

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN LTE GPS Status LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 2): ", 0, 0, 2);
    
    if (option == 0) { 
        reg_val = (uint)(LTE_GPS_STAT_LED_OFF);
    } else if (option == 1 ) {
        reg_val = (uint)(LTE_GPS_STAT_LED_Y);
    } else if (option == 2 ) {
        reg_val = (uint)(LTE_GPS_STAT_LED_G);
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
 * Function    : tsn_poestat_led_utils
 * Description : Function to turn TSN POE status LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_poestat_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN POE Present Status LED utils: \n"); 
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
 * Function    : tsn_poeport_led_utils
 * Description : Function to turn TSN POE port LEDs ON/OFF.
 * Inputs      : port - port number.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_poeport_led_utils (int port)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN POE Port%d Status LED utils: \n", port); 
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
 * Function    : tsn_aux_led_utils
 * Description : Function to turn TSN AUX LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_aux_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN AUX Status LED utils: \n"); 
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
 * Function    : tsn_microusb_led_utils
 * Description : Function to turn TSN micro usb LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_microusb_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN MicroUSB Status LED utils: \n"); 
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
 * Function    : tsn_usb_led_utils
 * Description : Function to turn TSN usb LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_usb_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN USB Status LED utils: \n"); 
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
 * Function    : tsn_console_led_utils
 * Description : Function to turn TSN console LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_console_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN Console Status LED utils: \n"); 
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
 * Function    : tsn_vpn_led_utils
 * Description : Function to turn TSN vpn LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_vpn_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LED_REG;
    
    printf("\n"); 
    printf("TSN VPN OK Status LED utils: \n"); 
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

/*******************************************************************************
 *
 * Function    : tsn_rssi_led_utils
 * Description : Function to turn TSN RSSI LEDs ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_rssi_led_utils (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    uint option = 0;

    reg_offset = (uint)FPGA_LTE_RSSI_LED_REG;
    
    printf("\n"); 
    printf("TSN LTE RSSI Status LED utils: \n"); 
    printf("0. 2G/3G & LTE RSSI OFF\n");
    printf("1. 2G/3G RSSI\n");
    printf("2. 2G/3G RSSI Low\n");
    printf("3. 2G/3G RSSI Medium\n");
    printf("4. 2G/3G RSSI High\n");
    printf("5. LTE RSSI\n");
    printf("6. LTE RSSI Low\n");
    printf("7. LTE RSSI Medium\n");
    printf("8. LTE RSSI High\n");
    option = getdec_answer("Select Toogle (0 ~ 8): ", 0, 0, 8);
    
    if (option == 0) {
        reg_val = (uint)(LTE_MOD_NO_SERV|LTE_MOD_NO_RSSI);
    } else if (option == 1 ) {
        reg_val = (uint)(LTE_MOD_2G3G_SIGNAL_AMBER|LTE_MOD_RSSI);
    } else if (option == 2 ) {
        reg_val = (uint)(LTE_MOD_2G3G_SIGNAL_AMBER|LTE_MOD_LOW_RSSI);
    } else if (option == 3 ) {
        reg_val = (uint)(LTE_MOD_2G3G_SIGNAL_AMBER|LTE_MOD_MEDIUM_RSSI);
    } else if (option == 4 ) {
        reg_val = (uint)(LTE_MOD_2G3G_SIGNAL_AMBER|LTE_MOD_HIGH_RSSI);
    } else if (option == 5 ) {
        reg_val = (uint)(LTE_MOD_LTE_SIGNAL_GREEN|LTE_MOD_RSSI);
    } else if (option == 6 ) {
        reg_val = (uint)(LTE_MOD_LTE_SIGNAL_GREEN|LTE_MOD_LOW_RSSI);
    } else if (option == 7 ) {
        reg_val = (uint)(LTE_MOD_LTE_SIGNAL_GREEN|LTE_MOD_MEDIUM_RSSI);
    } else if (option == 8 ) {
        reg_val = (uint)(LTE_MOD_LTE_SIGNAL_GREEN|LTE_MOD_HIGH_RSSI);
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
 * Function    : tsn_ge_led_utils
 * Description : Function to turn TSN GE LEDs ON/OFF.
 * Inputs      : port - GE0 or GE1 
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_ge_led_utils (int port)
{
    uint geport = 0;
    uint option = 0;

    printf("\n"); 
    printf("TSN GE%d Status LED utils: \n", port); 
    printf("0. OFF\n");
    printf("1. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    if (port == 0) {
        geport = TSN_GE0_ETHNUM;
    } else {
        geport = TSN_GE1_ETHNUM;
    }
    if (option == 0) {
        if (tsn_all_ge_leds_off(geport) != PASSED) {
            printf("%s: Failed to turn all GE%d LEDs ON.\n", __FUNCTION__, port);
            return (FAILED);
        }
    } else if (option == 1 ) {
        if (tsn_all_ge_leds_on(geport) != PASSED) {
            printf("%s: Failed to turn all GE%d LEDs ON.\n", __FUNCTION__, port);
            return (FAILED);
        }
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_led_ctrl_utils
 * Description : Function to show TSN LED Control utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_led_ctrl_utils (int opt)
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
 * Function    : diag_check_ge_ext_intr_no_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "No Interrupt Pending" state
 * Inputs      : fpga_pending_bit
 * Outputs     : TRUE / FALSE
 *
 *******************************************************************************
 */
int diag_check_ge_ext_intr_no_pending (int fpga_pending_bit)
{
    uint fpga_offset, fpga_rd_data, fpga_wr_data;
    fpga_offset = FPGA_EXTER_INT_PENDING_REG; /* 0x1128 */

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FALSE);
    }

    /* Write “1” to clear when forced by the force interrupt register.*/
    /* set FPGA EIPR GE interrupt pending bit as 1(in order to read interrupt pin)*/
    fpga_wr_data = fpga_rd_data | fpga_pending_bit;
    if (fpga_write_32_reg(fpga_offset, fpga_wr_data) != PASSED) {
        printf("%s:%d: Failed to write FPGA reg:0x%x with data:0x%x\n",
                __FUNCTION__, __LINE__, fpga_offset, fpga_wr_data);
        return (FALSE);
    }

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FALSE);
    }

    /* checking pending bit is low */
    if ((fpga_rd_data & fpga_pending_bit) != 0) {
        printf("%s:%d: The EIPR(0x1128) pending bit is still high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function    : diag_check_ge_ext_intr_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "Interrupt Pending" state
 * Inputs      : fpga_pending_bit
 * Outputs     : TRUE / FALSE
 *
 *******************************************************************************
 */
int diag_check_ge_ext_intr_pending (int fpga_pending_bit)
{
    uint fpga_offset, fpga_rd_data;
    fpga_offset = FPGA_EXTER_INT_PENDING_REG; /* 0x1128 */

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FALSE);
    }

    /* checking pending bit is high */
    if ((fpga_rd_data & fpga_pending_bit) != fpga_pending_bit) {
        printf("%s:%d: The EIPR(0x1128) pending bit is not high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function    : diag_check_esw_ext_no_intr_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "No Interrupt Pending" state
 * Inputs      : NONE
 * Outputs     : TRUE / FALSE
 *
 *******************************************************************************
 */
int diag_check_esw_ext_no_intr_pending (void)
{
    uint fpga_offset, fpga_rd_data, fpga_wr_data;
    int fpga_pending_bit = PENDING_BIT_ESW; 
    fpga_offset = FPGA_EXTER_INT_PENDING_REG;
    
    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FALSE);
    }
    
    /* Write “1” to clear when forced by the force interrupt register.*/
    /* set FPGA EIPR GE interrupt pending bit as 1(in order to read interrupt pin)*/
    fpga_wr_data = fpga_rd_data | fpga_pending_bit;
    if (fpga_write_32_reg(fpga_offset, fpga_wr_data) != PASSED) {
        printf("%s:%d: Failed to write FPGA reg:0x%x with data:0x%x\n",
                __FUNCTION__, __LINE__, fpga_offset, fpga_wr_data);
        return (FALSE);
    }

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FALSE);
    }

    /* checking pending bit is low */
    if ((fpga_rd_data & fpga_pending_bit) != 0) {
        printf("%s:%d: The EIPR(0x1128) pending bit is still high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function    : diag_check_esw_ext_intr_pending
 * Description : Function to check External Interrupt Pending Register(0x1128)
 *               is getting to "Interrupt Pending" state
 * Inputs      : NONE
 * Outputs     : TRUE / FALSE
 *
 *******************************************************************************
 */
int diag_check_esw_ext_intr_pending (void)
{
    uint fpga_offset, fpga_rd_data;
    int fpga_pending_bit = PENDING_BIT_ESW;
    fpga_offset = FPGA_EXTER_INT_PENDING_REG;

    /* read FPGA EIPR original data */
    if (fpga_read_32_reg(fpga_offset, &fpga_rd_data) != PASSED) {
        printf("%s:%d: Failed to read FPGA reg:0x%x\n", __FUNCTION__, __LINE__, fpga_offset);
        return (FALSE);
    }
    
    /* checking pending bit is high */
    if ((fpga_rd_data & fpga_pending_bit) != fpga_pending_bit) {
        printf("%s:%d: The EIPR(0x1128) pending bit is not high!!\n", __FUNCTION__, __LINE__);
        printf("%s:%d: The EIPR(0x1128) data = 0x%x\n", __FUNCTION__, __LINE__, fpga_rd_data);
        return (FALSE);
    }
    return (TRUE);
}


/*-------------------------------------------------
 * $Log: platform_fpga.c,v $
 * Revision 1.16  2019/03/07 09:51:32  lucywang
 * [Supernova] PID changed : C1101L-4P --> C951-4P, C1109L-2P --> C959-2P
 *
 * Revision 1.15  2019/01/24 03:30:48  letsai
 * Update Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.14  2019/01/24 01:07:22  letsai
 * Add Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.13  2019/01/18 05:54:46  yungchen
 * Merge Supernova branch to the main trunk (CSCvn79871)
 *
 * Revision 1.12  2018/11/23 08:49:51  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.11.30.1  2018/10/15 06:53:07  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.11  2018/06/05 09:54:08  lucywang
 * Merge Star branch star-branch-c110x to main trunk
 *
 * Revision 1.10  2018/06/01 08:16:10  letsai
 * Added Check flash protection utility
 *
 * Revision 1.9  2018/05/15 09:37:32  steja
 * CSCvj38863: Enhanced LED single test utility
 *
 * Revision 1.8  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 * Revision 1.7  2018/04/15 22:03:30  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.6.2.1  2018/04/02 09:14:26  palin2
 * Added Vulcan controller type and SKU info to platform SKU table.
 *
 * Revision 1.6  2018/03/27 12:46:38  hondwang
 * Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER
 *
 * Revision 1.5  2018/02/27 07:16:31  hondwang
 * Remove Star FPGA check LTE info, Star LTE module is movable
 *
 * Revision 1.4.2.2  2018/02/24 06:22:39  iachang
 * Support Aikido Mailbox Test
 *
 * Revision 1.4  2018/02/09 09:56:55  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.3  2018/01/23 11:38:19  steja
 * Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)
 *
 * Revision 1.2.20.2  2018/02/02 13:34:45  hondwang
 * Fix this_is_star_cxx function may return PASS with TSN platform
 *
 * Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.2.12.2  2017/12/19 06:48:55  shjung
 * Updated the SKUs for new PID
 *
 * Revision 1.2.12.1  2017/10/20 11:42:40  steja
 * Sync Gfast  with the latest main trunk
 *
 * Revision 1.2.4.10  2017/12/08 11:20:27  hondwang
 * Add console link with USB or RJ45 utility
 *
 * Revision 1.2.4.9  2017/11/22 09:45:46  hondwang
 * Fix demo SKU and menu show
 *
 * Revision 1.2.4.8  2017/11/20 07:54:32  lucywang
 * Changed PID to C1101/C1109-2P/C1109-4P
 *
 * Revision 1.2.4.7  2017/11/06 11:28:10  shjung
 * Check SPI transaction to ensure Sirius FPGA programming is completed and combine FPGA upgrade utilities(with/without header)
 *
 * Revision 1.2.4.6  2017/10/24 03:06:47  lucywang
 * Turned on green Power LED after LED test
 *
 * Revision 1.2.4.5  2017/10/16 10:18:26  lucywang
 * modified SKU ID table for C949-2P
 *
 * Revision 1.2.4.4  2017/10/07 02:12:40  hondwang
 * Add FPGA SKU check function to double confirm FPGA info
 *
 * Revision 1.2.4.3  2017/08/28 06:52:32  shjung
 * Code cleanup.
 *
 * Revision 1.2.4.2  2017/08/28 03:34:13  lucywang
 * modified for C949-2P
 *
 * Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:48  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:19  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.4  2017/07/25 08:31:55  steja
 * 1. Remove unused code.
 * 2. Verified before check-in
 *
 * Revision 1.1.6.3  2017/07/24 14:14:10  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.16.2.6  2017/07/17 13:54:44  palin2
 * Code cleanup.
 *
 * Revision 1.1.4.16.2.5  2017/06/03 08:07:18  steja
 * Update LED test based on latest FPGA register
 *
 * Revision 1.1.4.16.2.4  2017/05/17 01:17:53  palin2
 * Updated GE WAN mapping number with team's decision.
 * (GE0: GE WAN with SFP; GE1: 2nd GE WAN)
 *
 * Revision 1.1.4.16.2.3.4.8  2017/07/26 14:04:53  hondwang
 * Add USB console check function
 *
 * Revision 1.1.4.16.2.3.4.7  2017/07/08 00:48:55  shjung
 * Add Sirius FPGA firmware upgrade
 *
 * Revision 1.1.4.16.2.3.4.6  2017/07/04 04:47:41  hondwang
 * Implement has_plug_slot function
 *
 * Revision 1.1.4.16.2.3.4.5  2017/07/03 13:16:39  hondwang
 * fix E2E LED, I2C and GE phy testing fail
 *
 * Revision 1.1.4.16.2.3.4.4  2017/06/30 13:37:55  hondwang
 * Fix Star platform I2c scan issue and add this_is_star function
 *
 * Revision 1.1.4.16.2.3.4.3  2017/06/28 22:34:12  shjung
 * Add pluggable FPGA debug LEDs turn on/off
 *
 * Revision 1.1.4.16.2.3.4.2  2017/06/19 14:42:33  hondwang
 * Add Plug FPGA utility support
 *
 * Revision 1.1.4.16.2.3.4.1  2017/06/16 06:52:38  tirawan
 * Foxconn Pluggable FPGA I2C Read/Write function correction during the bring up
 *
 * Revision 1.1.4.16.2.3  2017/04/10 10:53:47  palin2
 * Added RSSI related LEDs into LED test.
 *
 * Revision 1.1.4.16.2.2  2017/03/29 13:59:50  steja
 * Fix xDSL sku type feature based new FPGA registers
 *
 * Revision 1.1.4.16.2.1  2017/02/23 11:03:16  palin2
 * Updated code based on FPGA changes. These updates are verified on P2A TSN.
 *
 * Revision 1.1.4.16  2016/12/30 09:12:44  steja
 * Enhanced io interface test
 *
 * Revision 1.1.4.15  2016/12/05 09:00:18  petteng
 * Add aikido register test
 *
 * Revision 1.1.4.14  2016/11/29 02:54:39  palin2
 * Dynamically getting device bus window base from CPU register.
 *
 * Revision 1.1.4.13  2016/11/01 07:29:21  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.12  2016/10/07 13:07:56  steja
 * 1. Add Check xDSL sku type
 * 2. Support Annex B
 *
 * Revision 1.1.4.11  2016/10/04 06:39:08  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.10  2016/09/13 14:35:47  steja
 * Commit Aikido / TAM Mailbox code
 *
 * Revision 1.1.4.9  2016/08/30 01:56:33  steja
 * Update led test code for TSN-M
 *
 * Revision 1.1.4.8  2016/08/09 09:47:54  iachang
 * Supported FPGA/Aikido firmware upgrade.
 *
 * Revision 1.1.4.7  2016/07/22 13:04:37  palin2
 * Added function to check DC present.
 *
 * Revision 1.1.4.6  2016/07/17 11:15:16  palin2
 * Added function to distinguish bwteen TSN-H and TSN-M.
 *
 * Revision 1.1.4.5  2016/07/13 11:09:39  iachang
 * Provided Aikido register r/w utilities.
 *
 * Revision 1.1.4.4  2016/07/10 10:29:34  steja
 * Add LED test
 *
 * Revision 1.1.4.3  2016/07/05 14:26:51  palin2
 * Added utililty to force ON/OFF TSN Switch port LED(s).
 *
 * Revision 1.1.4.2  2016/06/30 06:22:50  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.10  2016/05/26 11:53:03  palin2
 * Added utilities to turn TSN all Green/Yellow LEDs ON.
 *
 * Revision 1.1.2.9  2016/05/16 06:44:55  palin2
 * Add function to get TSN board type, and config Diag test items in menu for
 * different SKUs based on its board type info.
 *
 * Revision 1.1.2.8  2016/04/29 10:14:56  palin2
 * Updated code and added support ext. loopback test after bring up Switch.
 *
 * Revision 1.1.2.7  2016/04/26 20:48:49  palin2
 * Updated code after bring up SFP external loopback test.
 *
 * Revision 1.1.2.6  2016/04/24 12:42:56  palin2
 * 1. Updated FPGA registers map.
 * 2. Fixed FPGA force interrupt test.
 * 3. Added FPGA registers dump utility.
 *
 * Revision 1.1.2.5  2016/04/21 20:45:40  palin2
 * Fixed FPGA function, "fpga_reset_32_api".
 *
 * Revision 1.1.2.4  2016/04/19 07:39:50  palin2
 * Updated FPGA register r/w utilities.
 *
 * Revision 1.1.2.3  2016/04/14 13:03:02  palin2
 * Fixed FPGA reset external devices function.
 *
 * Revision 1.1.2.2  2016/04/14 06:12:17  palin2
 * Updated FPGA register read/write function and register map after bring up.
 *
 * Revision 1.1.2.1  2016/03/23 03:31:11  palin2
 * Added FPGA Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */

