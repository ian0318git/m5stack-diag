/* $Id: platform_synce_pll_utils.c,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_synce_pll_utils.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename: platform_synce_pll_utils.c
 *
 * Description: SyncE PLL, IDT8A3xxxx utilities.
 *
 * Copyright (c) 2019 - 2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "proto.h"
#include "common.h"
#include "queryflags.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "cli_cmd.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "platform_i2c.h"
#include "platform_synce_pll_utils.h"
#include "dash_fpga.h" 
#include "diag_bcm54194_api.h"
#include "diag_bcm82757_test.h"
#include "diag_bcm_lib.h"
#include "nvmonvars.h"
#include "platform_eth.h"


/*
 * Functional prototype
 */
int fugazi_idt8a3_set_page( uint8_t *page );
int fugazi_idt8a3_read( uint32_t offset, uint8_t *data, uint32_t *re_addr );
int fugazi_idt8a3_write( uint32_t offset, uint8_t *data, uint32_t *re_addr );
int fugazi_idt8a3_setup( void );
int fugazi_idt8a3_eeprom_read( uint8_t dev, uint32_t offset, uint8_t *data, uint32_t data_len);
int fugazi_idt8a3_eeprom_write( uint8_t dev, uint32_t offset, uint8_t *data, uint32_t data_len );
int fugazi_get_idt8a3_eeprom_data( uint8_t *eeprom_buf );

int idt8a3_read( uint32_t offset, uint8_t *data, uint32_t *re_addr );
int idt8a3_write( uint32_t offset, uint8_t *data, uint32_t *re_addr );
int idt8a3_reg_dump( uint8_t id_param, int non_zero_param );
int idt8a3_status_dump( void );
int idt8a3_reg_info( void );
int idt8a3_read_fw_version( uint8_t *, uint8_t *, uint8_t *, uint8_t *);
int idt8a3_show_fw_version( void);
int idt8a3_set_freq_margin( uint32_t freq_margin_N, uint32_t freq_margin_ppm, uint32_t freq_margin_high);
int idt8a3_show_freq( void );
int idt8a3_input_clock_check( uint32_t clock_mask, int mode, int time, int duration );
int idt8a3_sw_reset( void );
int idt8a3_gen_int( int enable );
int idt8a3_input_clock_rate( int phy_clk_index );
int idt8a3_input_ref_monitor_cfg( int phy_clk_index );
int idt8a3_check_eeprom_config_status( void );
int idt8a3_check_dpll_lock_status( void );

int idt8a3_reg_test( void );
int idt8a3_pll_intr_test (void);
int idt8a3_recovered_clock_test (void);

int idt8a3_eeprom_read( uint32_t offset, uint32_t count);
int idt8a3_eeprom_write( uint32_t addr, uint8_t data );
int idt8a3_eeprom_prog( uint32_t mode );

int bcm82752_emphasis_setting(void);


/* for utilities submenu. */
static int  idt8a3_debug_flag_f(void);
static int  idt8a3_reg_read_f(void);
static int  idt8a3_reg_write_f(void);
static int  idt8a3_reg_dump_f(void);
static int  idt8a3_status_dump_f(void);
static int  idt8a3_input_clock_check_f(void);
static int  idt8a3_show_fw_version_f( void);
static int  idt8a3_sw_reset_f( void);
static int  idt8a3_reg_test_f( void);
static int  idt8a3_pll_intr_test_f(void);
static int  idt8a3_recovered_clock_test_f(void);
static int  idt8a3_eeprom_read_f( void);
static int  idt8a3_eeprom_write_f( void);
static int  idt8a3_eeprom_read_file_f( void);
static int  idt8a3_eeprom_prog_f( void);

static int  idt8a3_phy_clk_f (void);
static int  idt8a3_clear_all_sticky_f( void);
static int  idt8a3_get_freq_margin( uint8_t, unsigned long long *);
static int  idt9a3_update_in_freq(uint8_t, int );
static int  idt9a3_in_freq_init( void );

static int idt8a3_eeprom_dev_offset( uint32_t addr, uint8_t *dev, uint32_t *offset );
static void build_synce_pll_utility_menu(int dummy);


#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/*
 *  Externs
 */

extern int utility_get_rtc(int);
extern void fugazi_cleanup_bcm82757_macsec(fugazi_lane_t);

/*
 *  Local variables
 */


/*
 *  Globals variables
 */

uint8_t idt8a3_data = 0;
uint8_t idt8a3_debug_flag = 0;
int idt8a3_freq_status = FREQ_MARGIN_NORMAL;

idt8a3_block_t idt_block_info[] = {
    /* block_id                              base */
    { IDT_BLOCK_HW_REVISION                , 0x8180, "HW_REVISION" },
    { IDT_BLOCK_RESET_CTRL                 , 0xc000, "RESET_CTRL" },
    { IDT_BLOCK_GENERAL_STATUS             , 0xc014, "GENERAL_STATUS" },
    { IDT_BLOCK_STATUS                     , 0xc03c, "STATUS" },
    { IDT_BLOCK_GPIO_USER_CONTROL          , 0xc160, "GPIO_USER_CONTROL" },
    { IDT_BLOCK_STICKY_STATUS_CLEAR        , 0xc164, "STICKY_STATUS_CLEAR" },
    { IDT_BLOCK_GPIO_TOD_NOTIFICATION_CLEAR, 0xc16c, "GPIO_TOD_NOTIFICATION_CLEAR" },
    { IDT_BLOCK_ALERT_CFG                  , 0xc188, "ALERT_CFG" },
    { IDT_BLOCK_SYS_DPLL_XO                , 0xc194, "SYS_DPLL_XO" },
    { IDT_BLOCK_SYS_APLL                   , 0xc19c, "SYS_APLL" },
    { IDT_BLOCK_INPUT_0                    , 0xc1b0, "INPUT" },
    { IDT_BLOCK_REF_MON_0                  , 0xc2e0, "REF_MON" },
    { IDT_BLOCK_DPLL_0                     , 0xc3b0, "DPLL" },
    { IDT_BLOCK_SYS_DPLL                   , 0xc5b8, "SYS_DPLL" },
    { IDT_BLOCK_DPLL_CTRL_0                , 0xc600, "DPLL_CTRL" },
    { IDT_BLOCK_SYS_DPLL_CTRL              , 0xc800, "SYS_DPLL_CTRL" },
    { IDT_BLOCK_DPLL_PHASE_0               , 0xc818, "DPLL_PHASE" },
    { IDT_BLOCK_DPLL_FREQ_0                , 0xc838, "DPLL_FREQ" },
    { IDT_BLOCK_DPLL_PHASR_PULL_IN_0       , 0xc880, "DPLL_PHASR_PULL_IN" },
    { IDT_BLOCK_GPIO_CFG                   , 0xc8c0, "GPIO_CFG" },
    { IDT_BLOCK_GPIO_0                     , 0xc8c2, "GPIO" },
    { IDT_BLOCK_OUT_DIV_MUX                , 0xca12, "OUT_DIV_MUX" },
    { IDT_BLOCK_OUTPUT_0                   , 0xca14, "OUTPUT" },
    { IDT_BLOCK_SERIAL                     , 0xcae0, "SERIAL" },
    { IDT_BLOCK_PWM_ENCODER_0              , 0xcb00, "PWM_ENCODER" },
    { IDT_BLOCK_PWM_DECODER_0              , 0xcb40, "PWM_DECODER" },
    { IDT_BLOCK_PWM_USER_DATA              , 0xcbc8, "PWM_USER_DATA" },
    { IDT_BLOCK_TOD_0                      , 0xcbcc, "TOD" },
    { IDT_BLOCK_TOD_WRITE_0                , 0xcc00, "TOD_WRITE" },
    { IDT_BLOCK_TOD_READ_PRIMARY_0         , 0xcc40, "TOD_READ_PRIMARY" },
    { IDT_BLOCK_TOD_READ_SECONDARY_0       , 0xcc90, "TOD_READ_SECONDARY" },

    { IDT_BLOCK_OUTPUT_TDC_CFG             , 0xccd0, "OUTPUT_TDC_CFG" },
    { IDT_BLOCK_OUTPUT_TDC_0               , 0xcd00, "OUTPUT_TDC" },
    { IDT_BLOCK_INPUT_TDC                  , 0xcd08, "INPUT_TDC" },

    { IDT_BLOCK_SCRATCH                    , 0xcf50, "SCRATCH" },
    { IDT_BLOCK_EEPROM                     , 0xcf68, "EEPROM" },
    { IDT_BLOCK_OTP                        , 0xcf70, "OTP" },
    { IDT_BLOCK_BYTE                       , 0xcf80, "BYTE" },
    { IDT_BLOCK_MAX                        , 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_HW_REVISION_reg_info[] = {
    { 0x007a, "REV_ID" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_RESET_CTRL_reg_info[] = {
    { 0x0012, "SOFT_RESET" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_GENERAL_STATUS_reg_info[] = {
    { 0x0004, "OTP_STATUS_7:0" },
    { 0x0005, "OTP_STATUS_15:8" },
    { 0x0006, "OTP_STATUS_23:16" },
    { 0x0007, "OTP_STATUS_31:24" },
    { 0x0008, "EEPROM_STATUS_7:0" },
    { 0x0009, "EEPROM_STATUS_15:8" },
    { 0x0010, "FIRMWARE_MAJ_REL" },
    { 0x0011, "FIRMWARE_MIN_REL" },
    { 0x0012, "FIRMWARE_HOTFIX_REL" },
    { 0x001C, "JTAG_DEVICE_ID_7:0" },
    { 0x001D, "JTAG_DEVICE_ID_15:8" },
    { 0x001E, "PRODUCT_ID_7:0" },
    { 0x001F, "PRODUCT_ID_15:8" },
    { 0x0020, "TEMPERATURE_7:0" },
    { 0x0021, "TEMPERATURE_15:8" },
    { 0x0022, "OTP_SCSR_CONFIG_SELECT" },
    { 0x0023, "OTP_CONFIG_STATUS" },
    { 0x0024, "OTP_CSR_CONFIG_STATUS" },
    { 0x0026, "EEPROM_CONFIG_STATUS" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_STATUS_reg_info[] = {
    { 0x0000, "I2CM" },
    { 0x0002, "SER0" },
    { 0x0003, "SER0_SPI" },
    { 0x0004, "SER0_I2C" },
    { 0x0005, "SER1" },
    { 0x0006, "SER1_SPI" },
    { 0x0007, "SER1_I2C" },
    { 0x0008, "IN0_MON" },
    { 0x0009, "IN1_MON" },
    { 0x000A, "IN2_MON" },
    { 0x000B, "IN3_MON" },
    { 0x000C, "IN4_MON" },
    { 0x000D, "IN5_MON" },
    { 0x000E, "IN6_MON" },
    { 0x000F, "IN7_MON" },
    { 0x0010, "IN8_MON" },
    { 0x0011, "IN9_MON" },
    { 0x0012, "IN10_MON" },
    { 0x0013, "IN11_MON" },
    { 0x0014, "IN12_MON" },
    { 0x0015, "IN13_MON" },
    { 0x0016, "IN14_MON" },
    { 0x0017, "IN15_MON" },
    { 0x0018, "DPLL0" },
    { 0x0019, "DPLL1" },
    { 0x001A, "DPLL2" },
    { 0x001B, "DPLL3" },
    { 0x001C, "DPLL4" },
    { 0x001D, "DPLL5" },
    { 0x001E, "DPLL6" },
    { 0x001F, "DPLL7" },
    { 0x0020, "DPLL_SYS" },
    { 0x0021, "SYS_APLL" },
    { 0x0022, "DPLL0_REF_STAT" },
    { 0x0023, "DPLL1_REF_STAT" },
    { 0x0024, "DPLL2_REF_STAT" },
    { 0x0025, "DPLL3_REF_STAT" },
    { 0x0026, "DPLL4_REF_STAT" },
    { 0x0027, "DPLL5_REF_STAT" },
    { 0x0028, "DPLL6_REF_STAT" },
    { 0x0029, "DPLL7_REF_STAT" },
    { 0x002A, "DPLL_SYS_REF_STAT" },

    { 0x0044, "DPLL0_FILTER_7:0" },
    { 0x0045, "DPLL0_FILTER_15:8" },
    { 0x0046, "DPLL0_FILTER_23:16" },
    { 0x0047, "DPLL0_FILTER_31:24" },
    { 0x0048, "DPLL0_FILTER_39:32" },
    { 0x0049, "DPLL0_FILTER_47:40" },

    { 0x004c, "DPLL1_FILTER_7:0" },
    { 0x004d, "DPLL1_FILTER_15:8" },
    { 0x004e, "DPLL1_FILTER_23:16" },
    { 0x004f, "DPLL1_FILTER_31:24" },
    { 0x0050, "DPLL1_FILTER_39:32" },
    { 0x0051, "DPLL1_FILTER_47:40" },

    { 0x0054, "DPLL2_FILTER_7:0" },
    { 0x0055, "DPLL2_FILTER_15:8" },
    { 0x0056, "DPLL2_FILTER_23:16" },
    { 0x0057, "DPLL2_FILTER_31:24" },
    { 0x0058, "DPLL2_FILTER_39:32" },
    { 0x0059, "DPLL2_FILTER_47:40" },

    { 0x005c, "DPLL3_FILTER_7:0" },
    { 0x005d, "DPLL3_FILTER_15:8" },
    { 0x005e, "DPLL3_FILTER_23:16" },
    { 0x005f, "DPLL3_FILTER_31:24" },
    { 0x0060, "DPLL3_FILTER_39:32" },
    { 0x0061, "DPLL3_FILTER_47:40" },

    { 0x0064, "DPLL4_FILTER_7:0" },
    { 0x0065, "DPLL4_FILTER_15:8" },
    { 0x0066, "DPLL4_FILTER_23:16" },
    { 0x0067, "DPLL4_FILTER_31:24" },
    { 0x0068, "DPLL4_FILTER_39:32" },
    { 0x0069, "DPLL4_FILTER_47:40" },

    { 0x006c, "DPLL5_FILTER_7:0" },
    { 0x006d, "DPLL5_FILTER_15:8" },
    { 0x006e, "DPLL5_FILTER_23:16" },
    { 0x006f, "DPLL5_FILTER_31:24" },
    { 0x0070, "DPLL5_FILTER_39:32" },
    { 0x0071, "DPLL5_FILTER_47:40" },

    { 0x0074, "DPLL6_FILTER_7:0" },
    { 0x0075, "DPLL6_FILTER_15:8" },
    { 0x0076, "DPLL6_FILTER_23:16" },
    { 0x0077, "DPLL6_FILTER_31:24" },
    { 0x0078, "DPLL6_FILTER_39:32" },
    { 0x0079, "DPLL6_FILTER_47:40" },

    { 0x007c, "DPLL7_FILTER_7:0" },
    { 0x007d, "DPLL7_FILTER_15:8" },
    { 0x007e, "DPLL7_FILTER_23:16" },
    { 0x007f, "DPLL7_FILTER_31:24" },
    { 0x0080, "DPLL7_FILTER_39:32" },
    { 0x0081, "DPLL7_FILTER_47:40" },

    { 0x0084, "DPLL_SYS_FILTER_7:0" },
    { 0x0085, "DPLL_SYS_FILTER_15:8" },
    { 0x0086, "DPLL_SYS_FILTER_23:16" },
    { 0x0087, "DPLL_SYS_FILTER_31:24" },
    { 0x0088, "DPLL_SYS_FILTER_39:32" },
    { 0x0089, "DPLL_SYS_FILTER_47:40" },

    { 0x008a, "USER_GPIO0_TO_7" },
    { 0x008b, "USER_GPIO8_TO_15" },

    { 0x008c, "IN0_MON_FREQ_7:0" },
    { 0x008d, "IN0_MON_FREQ_15:8" },
    { 0x008e, "IN1_MON_FREQ_7:0" },
    { 0x008f, "IN1_MON_FREQ_15:8" },
    { 0x0090, "IN2_MON_FREQ_7:0" },
    { 0x0091, "IN2_MON_FREQ_15:8" },
    { 0x0092, "IN3_MON_FREQ_7:0" },
    { 0x0093, "IN3_MON_FREQ_15:8" },
    { 0x0094, "IN4_MON_FREQ_7:0" },
    { 0x0095, "IN4_MON_FREQ_15:8" },
    { 0x0096, "IN5_MON_FREQ_7:0" },
    { 0x0097, "IN5_MON_FREQ_15:8" },
    { 0x0098, "IN6_MON_FREQ_7:0" },
    { 0x0099, "IN6_MON_FREQ_15:8" },
    { 0x009a, "IN7_MON_FREQ_7:0" },
    { 0x009b, "IN7_MON_FREQ_15:8" },
    { 0x009c, "IN8_MON_FREQ_7:0" },
    { 0x009d, "IN8_MON_FREQ_15:8" },
    { 0x009e, "IN9_MON_FREQ_7:0" },
    { 0x009f, "IN9_MON_FREQ_15:8" },
    { 0x00a0, "IN10_MON_FREQ_7:0" },
    { 0x00a1, "IN10_MON_FREQ_15:8" },
    { 0x00a2, "IN11_MON_FREQ_7:0" },
    { 0x00a3, "IN11_MON_FREQ_15:8" },
    { 0x00a4, "IN12_MON_FREQ_7:0" },
    { 0x00a5, "IN12_MON_FREQ_15:8" },
    { 0x00a6, "IN13_MON_FREQ_7:0" },
    { 0x00a7, "IN13_MON_FREQ_15:8" },
    { 0x00a8, "IN14_MON_FREQ_7:0" },
    { 0x00a9, "IN14_MON_FREQ_15:8" },
    { 0x00aa, "IN15_MON_FREQ_7:0" },
    { 0x00ab, "IN15_MON_FREQ_15:8" },

    { 0x00ac, "OUTPUT_TDC_CFG" },
    { 0x00ad, "OUTPUT_TDC0" },
    { 0x00ae, "OUTPUT_TDC1" },
    { 0x00af, "OUTPUT_TDC2" },
    { 0x00b0, "OUTPUT_TDC3" },

    { 0x00b4, "OUTPUT_TDC0_MEASUREMENT_7:0" },
    { 0x00b5, "OUTPUT_TDC0_MEASUREMENT_15:8" },
    { 0x00b6, "OUTPUT_TDC0_MEASUREMENT_23:16" },
    { 0x00b7, "OUTPUT_TDC0_MEASUREMENT_31:24" },
    { 0x00b8, "OUTPUT_TDC0_MEASUREMENT_39:32" },
    { 0x00b9, "OUTPUT_TDC0_MEASUREMENT_47:40" },

    { 0x00c4, "OUTPUT_TDC1_MEASUREMENT_7:0" },
    { 0x00c5, "OUTPUT_TDC1_MEASUREMENT_15:8" },
    { 0x00c6, "OUTPUT_TDC1_MEASUREMENT_23:16" },
    { 0x00c7, "OUTPUT_TDC1_MEASUREMENT_31:24" },
    { 0x00c8, "OUTPUT_TDC1_MEASUREMENT_39:32" },
    { 0x00c9, "OUTPUT_TDC1_MEASUREMENT_47:40" },

    { 0x00cc, "OUTPUT_TDC2_MEASUREMENT_7:0" },
    { 0x00cd, "OUTPUT_TDC2_MEASUREMENT_15:8" },
    { 0x00ce, "OUTPUT_TDC2_MEASUREMENT_23:16" },
    { 0x00cf, "OUTPUT_TDC2_MEASUREMENT_31:24" },
    { 0x00d0, "OUTPUT_TDC2_MEASUREMENT_39:32" },
    { 0x00d1, "OUTPUT_TDC2_MEASUREMENT_47:40" },

    { 0x00d4, "OUTPUT_TDC3_MEASUREMENT_7:0" },
    { 0x00d5, "OUTPUT_TDC3_MEASUREMENT_15:8" },
    { 0x00d6, "OUTPUT_TDC3_MEASUREMENT_23:16" },
    { 0x00d7, "OUTPUT_TDC3_MEASUREMENT_31:24" },
    { 0x00d8, "OUTPUT_TDC3_MEASUREMENT_39:32" },
    { 0x00d9, "OUTPUT_TDC3_MEASUREMENT_47:40" },

    { 0x00dc, "DPLL0_PHASE_7:0" },
    { 0x00dd, "DPLL0_PHASE_15:8" },
    { 0x00de, "DPLL0_PHASE_23:16" },
    { 0x00df, "DPLL0_PHASE_31:24" },
    { 0x00e0, "DPLL0_PHASE_35:32" },

    { 0x00e4, "DPLL1_PHASE_7:0" },
    { 0x00e5, "DPLL1_PHASE_15:8" },
    { 0x00e6, "DPLL1_PHASE_23:16" },
    { 0x00e7, "DPLL1_PHASE_31:24" },
    { 0x00e8, "DPLL1_PHASE_35:32" },

    { 0x00ec, "DPLL2_PHASE_7:0" },
    { 0x00ed, "DPLL2_PHASE_15:8" },
    { 0x00ee, "DPLL2_PHASE_23:16" },
    { 0x00ef, "DPLL2_PHASE_31:24" },
    { 0x00f0, "DPLL2_PHASE_35:32" },

    { 0x00f4, "DPLL3_PHASE_7:0" },
    { 0x00f5, "DPLL3_PHASE_15:8" },
    { 0x00f6, "DPLL3_PHASE_23:16" },
    { 0x00f7, "DPLL3_PHASE_31:24" },
    { 0x00f8, "DPLL3_PHASE_35:32" },

    { 0x00fc, "DPLL4_PHASE_7:0" },
    { 0x00fd, "DPLL4_PHASE_15:8" },
    { 0x00fe, "DPLL4_PHASE_23:16" },
    { 0x00ff, "DPLL4_PHASE_31:24" },
    { 0x0100, "DPLL4_PHASE_35:32" },

    { 0x0104, "DPLL5_PHASE_7:0" },
    { 0x0105, "DPLL5_PHASE_15:8" },
    { 0x0106, "DPLL5_PHASE_23:16" },
    { 0x0107, "DPLL5_PHASE_31:24" },
    { 0x0108, "DPLL5_PHASE_35:32" },

    { 0x010c, "DPLL6_PHASE_7:0" },
    { 0x010d, "DPLL6_PHASE_15:8" },
    { 0x010e, "DPLL6_PHASE_23:16" },
    { 0x010f, "DPLL6_PHASE_31:24" },
    { 0x0110, "DPLL6_PHASE_35:32" },

    { 0x0114, "DPLL7_PHASE_7:0" },
    { 0x0115, "DPLL7_PHASE_15:8" },
    { 0x0116, "DPLL7_PHASE_23:16" },
    { 0x0117, "DPLL7_PHASE_31:24" },
    { 0x0118, "DPLL7_PHASE_35:32" },

    { 0x011c, "DPLL0_PHASE_PULL_IN" },
    { 0x011d, "DPLL1_PHASE_PULL_IN" },
    { 0x011e, "DPLL2_PHASE_PULL_IN" },
    { 0x011f, "DPLL3_PHASE_PULL_IN" },
    { 0x0120, "DPLL4_PHASE_PULL_IN" },
    { 0x0121, "DPLL5_PHASE_PULL_IN" },
    { 0x0122, "DPLL6_PHASE_PULL_IN" },
    { 0x0123, "DPLL7_PHASE_PULL_IN" },

    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_GPIO_USER_CONTROL_reg_info[] = {
    { 0x0000, "GPIO0_TO_7_OUT" },
    { 0x0001, "GPIO8_TO_15_OUT" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_STICKY_STATUS_CLEAR_reg_info[] = {
    { 0x0000, "IN0_TO_7_MON_STICKY_STATUS_CLEAR" },
    { 0x0001, "IN8_TO_15_MON_STICKY_STATUS_CLEAR" },
    { 0x0002, "DPLL_STICKY_STATUS_CLEAR" },
    { 0x0003, "DPLL_SYS_STICKY_STATUS_CLEAR" },
    { 0x0004, "SYS_APLL_STICKY_STATUS_CLEAR" },
    { 0x0005, "ALL_STICKY_STATUS_CLEAR" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_GPIO_TOD_NOTIFICATION_CLEAR_reg_info[] = {
    { 0x0000, "GPIO0_TO_7_CLEAR" },
    { 0x0001, "GPIO8_TO_15_CLEAR" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_ALERT_CFG_reg_info[] = {
    { 0x0000, "IN1_0_MON_ALERT_MASK" },
    { 0x0001, "IN3_2_MON_ALERT_MASK" },
    { 0x0002, "IN5_4_MON_ALERT_MASK" },
    { 0x0003, "IN7_6_MON_ALERT_MASK" },
    { 0x0004, "IN9_8_MON_ALERT_MASK" },
    { 0x0005, "IN11_10_MON_ALERT_MASK" },
    { 0x0006, "IN13_12_MON_ALERT_MASK" },
    { 0x0007, "IN15_14_MON_ALERT_MASK" },
    { 0x0008, "DPLL3_2_1_0_ALERT_MASK" },
    { 0x0009, "DPLL7_6_5_4_ALERT_MASK" },
    { 0x000A, "SYS_ALERT_MASK " },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_SYS_DPLL_XO_reg_info[] = {
    { 0x0000, "FREQ_M_7:0" },
    { 0x0001, "FREQ_M_15:8" },
    { 0x0002, "FREQ_M_23:16" },
    { 0x0003, "FREQ_M_31:24" },
    { 0x0004, "FREQ_M_39:32" },
    { 0x0005, "FREQ_M_47:40" },
    { 0x0006, "FREQ_N_7:0" },
    { 0x0007, "FREQ_N_15:8" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_SYS_APLL_reg_info[] = {
    { 0x0000, "CP_SS_CURRENT_1" },
    { 0x0001, "CP_SS_CURRENT_2" },
    { 0x0002, "CFG_1" },
    { 0x0003, "CFG_2" },
    { 0x0004, "VREG_CTRL" },
    { 0x0005, "CP_CTRL_0" },
    { 0x0006, "CP_CTRL_1" },
    { 0x0007, "CP_CTRL_2" },
    { 0x0008, "FREQ_M_7:0" },
    { 0x0009, "FREQ_M_15:8" },
    { 0x000a, "FREQ_M_23:16" },
    { 0x000b, "FREQ_M_31:24" },
    { 0x000c, "FREQ_M_39:32" },
    { 0x000d, "FREQ_M_47:40" },
    { 0x000e, "FREQ_N_7:0" },
    { 0x000f, "FREQ_N_15:8" },
    { 0x0010, "CTRL_0" },
    { 0x0011, "CTRL_1" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_INPUT_0_reg_info[] = {
    { 0x0000, "FREQ_M_7:0" },
    { 0x0001, "FREQ_M_15:8" },
    { 0x0002, "FREQ_M_23:16" },
    { 0x0003, "FREQ_M_31:24" },
    { 0x0004, "FREQ_M_39:32" },
    { 0x0005, "FREQ_M_47:40" },
    { 0x0006, "FREQ_N_7:0" },
    { 0x0007, "FREQ_N_15:8" },
    { 0x0008, "DIV_7:0" },
    { 0x0009, "DIV_15:8" },
    { 0x000A, "PHASE_7:0" },
    { 0x000b, "PHASE_15:8" },
    { 0x000C, "SYNC" },
    { 0x000D, "MODE" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_REF_MON_0_reg_info[] = {
    { 0x0000, "FREQ_CFG" },
    { 0x0001, "DSQF_INTV" },
    { 0x0002, "TRANS_THRESHOLD_7:0" },
    { 0x0003, "TRANS_THRESHOLD_15:8" },
    { 0x0004, "TRANS_PERIOD_7:0" },
    { 0x0005, "TRANS_PERIOD_15:8" },
    { 0x0006, "ACT_CFG" },
    { 0x0008, "LOS_TOLERANCE_7:0" },
    { 0x0009, "LOS_TOLERANCE_15:8" },
    { 0x000A, "LOS_CFG" },
    { 0x000B, "CFG" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_DPLL_0_reg_info[] = {
    { 0x0000, "DC0_INC_DEC_SIZE_7:0" },
    { 0x0001, "DC0_INC_DEC_SIZE_15:8" },
    { 0x0002, "CTRL_0" },
    { 0x0003, "CTRL_1" },
    { 0x0004, "CTRL_2" },
    { 0x0005, "UPDATE_RATE_CFG" },
    { 0x0006, "FILTER_STATUS_UPDATE_CFG" },
    { 0x0007, "HO_ADVCD_HISTORY" },
    { 0x0008, "HO_ADVCD_BW DPLL_0" },
    { 0x0009, "HO_ADVCD_BW DPLL_1" },
    { 0x000A, "HO_CFG" },
    { 0x000B, "LOCK_0" },
    { 0x000C, "LOCK_1" },
    { 0x000D, "LOCK_2" },
    { 0x000E, "LOCK_3" },
    { 0x000F, "REF_PRIORITY_0" },
    { 0x0010, "REF_PRIORITY_1" },
    { 0x0011, "REF_PRIORITY_2" },
    { 0x0012, "REF_PRIORITY_3" },
    { 0x0013, "REF_PRIORITY_4" },
    { 0x0014, "REF_PRIORITY_5" },
    { 0x0015, "REF_PRIORITY_6" },
    { 0x0016, "REF_PRIORITY_7" },
    { 0x0017, "REF_PRIORITY_8" },
    { 0x0018, "REF_PRIORITY_9" },
    { 0x0019, "REF_PRIORITY_10" },
    { 0x001A, "REF_PRIORITY_11" },
    { 0x001B, "REF_PRIORITY_12" },
    { 0x001C, "REF_PRIORITY_13" },
    { 0x001D, "REF_PRIORITY_14" },
    { 0x001E, "REF_PRIORITY_15" },
    { 0x001F, "REF_PRIORITY_16" },
    { 0x0020, "REF_PRIORITY_17" },
    { 0x0021, "REF_PRIORITY_18" },
    { 0x0023, "FASTLOCK_CFG_0" },
    { 0x0024, "FASTLOCK_CFG_1" },
    { 0x0025, "MAX_FREQ_OFFSET" },
    { 0x0026, "FASTLOCK_PSL_7:0" },
    { 0x0027, "FASTLOCK_PSL_15:8" },
    { 0x0028, "FASTLOCK_FSL_7:0" },
    { 0x0029, "FASTLOCK_FSL_15:8" },
    { 0x002A, "FASTLOCK_BW_0" },
    { 0x002b, "FASTLOCK_BW_1" },
    { 0x002C, "WRITE_FREQ_TIMER_7:0" },
    { 0x002d, "WRITE_FREQ_TIMER_15:8" },
    { 0x002E, "WRITE_PHASE_TIMER_7:0" },
    { 0x002f, "WRITE_PHASE_TIMER_15:8" },
    { 0x0030, "PRED_CFG" },
    { 0x0031, "TOD_SYNC_CFG" },
    { 0x0032, "COMBO_SLAVE_CFG_0" },
    { 0x0033, "COMBO_SLAVE_CFG_1" },
    { 0x0034, "SLAVE_REF_CFG" },
    { 0x0035, "REF_MODE" },
    { 0x0036, "PHASE_MEASUREMENT_CFG" },
    { 0x0037, "MODE" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_SYS_DPLL_reg_info[] = {
    { 0x0000, "CTRL_0" },
    { 0x0001, "UPDATE_RATE_CFG" },
    { 0x0002, "FILTER_STATUS_UPDATE_CFG" },
    { 0x0003, "LOCK_0" },
    { 0x0004, "LOCK_1" },
    { 0x0005, "LOCK_2" },
    { 0x0006, "LOCK_3" },
    { 0x0007, "REF_PRIORITY_0" },
    { 0x0008, "REF_PRIORITY_1" },
    { 0x0009, "REF_PRIORITY_2" },
    { 0x000a, "REF_PRIORITY_3" },
    { 0x000b, "REF_PRIORITY_4" },
    { 0x000c, "REF_PRIORITY_5" },
    { 0x000d, "REF_PRIORITY_6" },
    { 0x000e, "REF_PRIORITY_7" },
    { 0x000f, "REF_PRIORITY_8" },
    { 0x0010, "REF_PRIORITY_9" },
    { 0x0011, "REF_PRIORITY_10" },
    { 0x0012, "REF_PRIORITY_11" },
    { 0x0013, "REF_PRIORITY_12" },
    { 0x0014, "REF_PRIORITY_13" },
    { 0x0015, "REF_PRIORITY_14" },
    { 0x0016, "REF_PRIORITY_15" },
    { 0x0017, "REF_PRIORITY_16" },
    { 0x0018, "REF_PRIORITY_17" },
    { 0x0019, "REF_PRIORITY_18" },
    { 0x001B, "REF_MODE" },
    { 0x001C, "SYS_DPLL_MODE" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_DPLL_CTRL_0_reg_info[] = {
    { 0x0000, "HS_TIE_RESET" },
    { 0x0001, "MANU_REF_CFG" },
    { 0x0002, "DAMPING" },
    { 0x0003, "DECIMATOR_BW_MULT" },
    { 0x0004, "BW_0" },
    { 0x0005, "BW_1" },
    { 0x0006, "PSL_0" },
    { 0x0007, "PSL_1" },
    { 0x0008, "PRED0_DAMPING" },
    { 0x0009, "PRED0_DECIMATOR_BW_MULT" },
    { 0x000A, "PRED0_BW_0" },
    { 0x000b, "PRED0_BW_1" },
    { 0x000C, "PRED0_PSL_7:0" },
    { 0x000d, "PRED0_PSL_15:8" },
    { 0x000E, "PRED1_DAMPING" },
    { 0x000F, "PRED1_DECIMATOR_BW_MULT" },
    { 0x0010, "PRED1_BW_0" },
    { 0x0011, "PRED1_BW_1" },
    { 0x0012, "PRED1_PSL_7:0" },
    { 0x0013, "PRED1_PSL_15:8" },
    { 0x0014, "PHASE_OFFSET_CFG_7:0" },
    { 0x0015, "PHASE_OFFSET_CFG_15:8" },
    { 0x0016, "PHASE_OFFSET_CFG_23:16" },
    { 0x0017, "PHASE_OFFSET_CFG_31:24" },
    { 0x0018, "PHASE_OFFSET_CFG_35:32" },
    { 0x0019, "HO_HISTORY_RESET" },
    { 0x001a, "FINE_PHASE_ADV_CFG_0" },
    { 0x001b, "FINE_PHASE_ADV_CFG_1" },
    { 0x001c, "FREQ_M_7:0" },
    { 0x001d, "FREQ_M_15:8" },
    { 0x001e, "FREQ_M_23:16" },
    { 0x001f, "FREQ_M_31:24" },
    { 0x0020, "FREQ_M_39:32" },
    { 0x0021, "FREQ_M_47:40" },
    { 0x0022, "FREQ_N_7:0" },
    { 0x0023, "FREQ_N_15:8" },
    { 0x0024, "MASTER_DIV_7:0" },
    { 0x0025, "MASTER_DIV_15:8" },
    { 0x0026, "MASTER_DIV_22:16" },
    { 0x0027, "MASTER_DIV_31:24" },
    { 0x0028, "COMBO_SW_VALUE_CNFG_7:0" },
    { 0x0029, "COMBO_SW_VALUE_CNFG_15:8" },
    { 0x002a, "COMBO_SW_VALUE_CNFG_23:16" },
    { 0x002b, "COMBO_SW_VALUE_CNFG_31:24" },
    { 0x002c, "COMBO_SW_VALUE_CNFG_39:32" },
    { 0x002d, "COMBO_SW_VALUE_CNFG_47:40" },
    { 0x0030, "MANUAL_HOLDOVER_VALUE_7:0" },
    { 0x0031, "MANUAL_HOLDOVER_VALUE_15:8" },
    { 0x0032, "MANUAL_HOLDOVER_VALUE_23:16" },
    { 0x0033, "MANUAL_HOLDOVER_VALUE_31:24" },
    { 0x0034, "MANUAL_HOLDOVER_VALUE_39:24" },
    { 0x0035, "MANUAL_HOLDOVER_VALUE_41:40" },
    { 0x0036, "DCD_FILTER_CNFG_0" },
    { 0x0037, "DCD_FILTER_CNFG_1" },
    { 0x0038, "COMBO_MASTER_BW_0" },
    { 0x0039, "COMBO_MASTER_BW_1" },
    { 0x003A, "COMBO_MASTER_CFG" },
    { 0x003B, "FINE_TUNING" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_SYS_DPLL_CTRL_reg_info[] = {
    { 0x0000, "MANU_REF_CFG" },
    { 0x0001, "DAMPING" },
    { 0x0002, "DECIMATOR_BW_MULT" },
    { 0x0004, "BW_0" },
    { 0x0005, "BW_1" },
    { 0x0006, "PSL_7:0" },
    { 0x0007, "PSL_15:8" },
    { 0x0008, "PRED0_DAMPING" },
    { 0x0009, "PRED0_DECIMATOR_BW_MULT" },
    { 0x000A, "PRED0_BW_0" },
    { 0x000b, "PRED0_BW_1" },
    { 0x000C, "PRED0_PSL_7:0" },
    { 0x000d, "PRED0_PSL_15:8" },
    { 0x000E, "PRED1_DAMPING" },
    { 0x000F, "PRED1_DECIMATOR_BW_MULT" },
    { 0x0010, "PRED1_BW_0" },
    { 0x0011, "PRED1_BW_1" },
    { 0x0012, "PRED1_PSL_7:0" },
    { 0x0013, "PRED1_PSL_15:8" },
    { 0x0014, "COMBO_MASTER_BW_0" },
    { 0x0015, "COMBO_MASTER_BW_1" },
    { 0x003A, "COMBO_MASTER_CFG" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_DPLL_PHASE_0_reg_info[] = {
    { 0x0000, "WRITE_PH_7:0" },
    { 0x0001, "WRITE_PH_15:8" },
    { 0x0002, "WRITE_PH_23:16" },
    { 0x0003, "WRITE_PH_31:24" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_DPLL_FREQ_0_reg_info[] = {
    { 0x0000, "WR_FREQ_7:0" },
    { 0x0001, "WR_FREQ_15:8" },
    { 0x0002, "WR_FREQ_23:16" },
    { 0x0003, "WR_FREQ_31:24" },
    { 0x0004, "WR_FREQ_39:32" },
    { 0x0005, "WR_FREQ_41:40" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_DPLL_PHASR_PULL_IN_0_reg_info[] = {
    { 0x0000, "PHASE_OFFSET_7:0" },
    { 0x0001, "PHASE_OFFSET_15:8" },
    { 0x0002, "PHASE_OFFSET_23:16" },
    { 0x0003, "PHASE_OFFSET_31:24" },
    { 0x0004, "PHASE_SLOPE_LIMIT_7:0" },
    { 0x0005, "PHASE_SLOPE_LIMIT_15:8" },
    { 0x0006, "PHASE_SLOPE_LIMIT_23:16" },
    { 0x0007, "CTRL" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_GPIO_CFG_reg_info[] = {
    { 0x0000, "CFG_GBL" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_GPIO_0_reg_info[] = {
    { 0x0000, "SM" },
    { 0x0001, "OUT_CTRL_0" },
    { 0x0002, "OUT_CTRL_1" },
    { 0x0003, "TOD_TRIG" },
    { 0x0004, "DPLL_INDICATOR" },
    { 0x0005, "LOS_INDICATOR" },
    { 0x0006, "REF_INPUT_DSQ_0" },
    { 0x0007, "REF_INPUT_DSQ_1" },
    { 0x0008, "REF_INPUT_DSQ_2" },
    { 0x0009, "REF_INPUT_DSQ_3" },
    { 0x000A, "MAN_CLK_SEL_0" },
    { 0x000B, "MAN_CLK_SEL_1" },
    { 0x000C, "MAN_CLK_SEL_2" },
    { 0x000D, "SLAVE" },
    { 0x000E, "ALERT_OUT_CFG" },
    { 0x000F, "TOD_NOTIFICATION_CFG" },
    { 0x0010, "CTRL" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_OUT_DIV_MUX_reg_info[] = {
    { 0x0000, "DIV8_MUX" },
    { 0x0001, "DIV11_MUX" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_OUTPUT_0_reg_info[] = {
    { 0x0000, "DIV_7:0" },
    { 0x0001, "DIV_15:8" },
    { 0x0002, "DIV_23:16" },
    { 0x0003, "DIV_31:24" },
    { 0x0004, "DUTY_CYCLE_HIGH_7:0" },
    { 0x0005, "DUTY_CYCLE_HIGH_15:8" },
    { 0x0006, "DUTY_CYCLE_HIGH_23:16" },
    { 0x0007, "DUTY_CYCLE_HIGH_31:24" },
    { 0x0008, "CTRL_0" },
    { 0x0009, "CTRL_1" },
    { 0x000c, "PHASE_ADJ_7:0" },
    { 0x000d, "PHASE_ADJ_15:8" },
    { 0x000e, "PHASE_ADJ_23:16" },
    { 0x000f, "PHASE_ADJ_31:24" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_SERIAL_reg_info[] = {
    { 0x0000, "I2CM" },
    { 0x0002, "SER0" },
    { 0x0003, "SER0_SPI" },
    { 0x0004, "SER0_I2C" },
    { 0x0005, "SER1" },
    { 0x0006, "SER1_SPI" },
    { 0x0007, "SER1_I2C" },
    { 0x0008, "SER_APPLY_CONFIG" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_PWM_ENCODER_0_reg_info[] = {
    { 0x0000, "ID" },
    { 0x0001, "CNFG" },
    { 0x0002, "SIGNATURE_0" },
    { 0x0003, "SIGNATURE_1" },
    { 0x0004, "CMD" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_PWM_DECODER_0_reg_info[] = {
    { 0x0000, "CNFG_0" },
    { 0x0001, "CNFG_1" },
    { 0x0002, "ID" },
    { 0x0003, "SIGNATURE_0" },
    { 0x0004, "SIGNATURE_1" },
    { 0x0005, "CMD" },
    { 0x0000, NULL },
};


idt8a3_reg_t IDT_BLOCK_PWM_USER_DATA_reg_info[] = {
    { 0x0000, "SRC_ENCODER_ID" },
    { 0x0001, "DST_DECODER_ID" },
    { 0x0002, "USER_DATA_SIZE" },
    { 0x0003, "USER_DATA_CMD_STS" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_TOD_0_reg_info[] = {
    { 0x0000, "CFG" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_TOD_WRITE_0_reg_info[] = {
    { 0x0000, "SUBNS_7:0" },
    { 0x0001, "NS_15:8" },
    { 0x0002, "NS_23:16" },
    { 0x0003, "NS_31:24" },
    { 0x0004, "NS_39:32" },
    { 0x0005, "SECOND_47:40" },
    { 0x0006, "SECOND_55:48" },
    { 0x0007, "SECOND_63:56" },
    { 0x0008, "SECOND_71:64" },
    { 0x0009, "SECOND_79:72" },
    { 0x000a, "SECOND_87:80" },
    { 0x000C, "COUNTER" },
    { 0x000D, "SELECT_CFG_0" },
    { 0x000F, "CMD" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_TOD_READ_xxxx_0_reg_info[] = {
    { 0x0000, "SUBNS_7:0" },
    { 0x0001, "NS_15:8" },
    { 0x0002, "NS_23:16" },
    { 0x0003, "NS_31:24" },
    { 0x0004, "NS_39:32" },
    { 0x0005, "SECOND_47:40" },
    { 0x0006, "SECOND_55:48" },
    { 0x0007, "SECOND_63:56" },
    { 0x0008, "SECOND_71:64" },
    { 0x0009, "SECOND_79:72" },
    { 0x000a, "SECOND_87:80" },
    { 0x000B, "COUNTER" },
    { 0x000C, "SEL_CFG_0" },
    { 0x000E, "CMD" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_OUTPUT_TDC_CFG_reg_info[] = {
    { 0x0000, "OUTPUT_TDC_CFG_GBL_0_7:0" },
    { 0x0001, "OUTPUT_TDC_CFG_GBL_0_15:8" },
    { 0x0002, "OUTPUT_TDC_CFG_GBL_1_7:0" },
    { 0x0003, "OUTPUT_TDC_CFG_GBL_1_15:8" },
    { 0x0004, "OUTPUT_TDC_CFG_GBL_2" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_OUTPUT_TDC_0_reg_info[] = {
    { 0x0000, "SAMPLE_7:0" },
    { 0x0001, "SAMPLE_15:8" },
    { 0x0002, "TARGET_PHASE_OFFSET_7:0" },
    { 0x0003, "TARGET_PHASE_OFFSET_15:8" },
    { 0x0004, "OUTPUT_TDC_CTRL_2" },
    { 0x0005, "OUTPUT_TDC_CTRL_3" },
    { 0x0006, "OUTPUT_TDC_CTRL_4" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_INPUT_TDC_reg_info[] = {
    { 0x0004, "INPUT_TDC_FBD_CTRL" },
    { 0x0005, "INPUT_TDC_CTRL" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_SCRATCH_reg_info[] = {
    { 0x0000, "SCRATCH0_7:0" },
    { 0x0001, "SCRATCH0_15:8" },
    { 0x0002, "SCRATCH0_23:16" },
    { 0x0003, "SCRATCH0_31:24" },
    { 0x0004, "SCRATCH1_7:0" },
    { 0x0005, "SCRATCH1_15:8" },
    { 0x0006, "SCRATCH1_23:16" },
    { 0x0007, "SCRATCH1_31:24" },
    { 0x0008, "SCRATCH2_7:0" },
    { 0x0009, "SCRATCH2_15:8" },
    { 0x000a, "SCRATCH2_23:16" },
    { 0x000b, "SCRATCH2_31:24" },
    { 0x000c, "SCRATCH3_7:0" },
    { 0x000d, "SCRATCH3_15:8" },
    { 0x000e, "SCRATCH3_23:16" },
    { 0x000f, "SCRATCH3_31:24" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_EEPROM_reg_info[] = {
    { 0x0000, "I2C_ADDR" },
    { 0x0001, "SIZE" },
    { 0x0002, "OFFSET" },
    { 0x0004, "CMD" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_OTP_reg_info[] = {
    { 0x0000, "CMD_7:0" },
    { 0x0001, "CMD_15:8" },
    { 0x0002, "CMD_23:16" },
    { 0x0003, "CMD_31:24" },
    { 0x0004, "CM_CTR_7:0" },
    { 0x0005, "CM_CTR_15:8" },
    { 0x0006, "HOST_CTR_7:0" },
    { 0x0007, "HOST_CTR_15:8" },
    { 0x0000, NULL },
};

idt8a3_reg_t IDT_BLOCK_BYTE_reg_info[] = {
    { 0x0000, "BUFF_0" },
    { 0x0001, "BUFF_1" },
    { 0x0002, "BUFF_2" },
    { 0x0003, "BUFF_3" },
    { 0x0004, "BUFF_4" },
    { 0x0005, "BUFF_5" },
    { 0x0006, "BUFF_6" },
    { 0x0007, "BUFF_7" },
    { 0x0000, NULL },
};


/* for SyncE recovered clock test */
idt8a3_recovered_clk_t IDT_INPUT_info[FUGAZI_MAX_PHY_PORT_CLK_TEST] = {
 /* input_no, in_freq,          in_div,in_sync,in_mode,clk_status_mask */
    { 0, SYNCE_FREQUCY_25MHZ,     0x01, 0x00, 0x01, 0x0001 }, /* BCM54194 1G PHY0 (REC_CLK1) */
    { 8, SYNCE_FREQUCY_25MHZ,     0x01, 0x00, 0x01, 0x0100 }, /* BCM54194 1G PHY1 (REC_CLK1) */
    { 1, SYNCE_FREQUCY_25MHZ,     0x01, 0x00, 0x01, 0x0002 }, /* BCM54194 1G PHY2 (REC_CLK1) */
    { 9, SYNCE_FREQUCY_25MHZ,     0x01, 0x00, 0x01, 0x0200 }, /* BCM54194 1G PHY3 (REC_CLK1) */
    { 3, SYNCE_FREQUCY_156p25MHZ, 0x01, 0x00, 0x21, 0x0008 }, /* BCM82757 10G PHY0, Port 0 (CLK_RCVCLK0) */
    { 4, SYNCE_FREQUCY_156p25MHZ, 0x01, 0x00, 0x21, 0x0010 }, /* BCM82757 10G PHY0, Port 1 (CLK_RCVCLK1) */
    { 5, SYNCE_FREQUCY_156p25MHZ, 0x01, 0x00, 0x21, 0x0020 }, /* BCM82757 10G PHY1, Port 0 (CLK_RCVCLK0) */
    { 6, SYNCE_FREQUCY_156p25MHZ, 0x01, 0x00, 0x21, 0x0040 }, /* BCM82757 10G PHY1, Port 1 (CLK_RCVCLK1) */
};


/*
 * List of SyncE PLL Utilities Menu.
 */
static submenu_xtable_t SYNCE_util_items[] = {
        {"Set Debug flag",      (PFT)idt8a3_debug_flag_f,       FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Register read",       (PFT)idt8a3_reg_read_f,         FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Register write",      (PFT)idt8a3_reg_write_f,        FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Register dump",       (PFT)idt8a3_reg_dump_f,         FALSE,
            0,                  (PFT)0,                        0,
           (PFT)0,              0},
        {"DPLL Status dump",    (PFT)idt8a3_status_dump_f,      FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Input clock check",   (PFT)idt8a3_input_clock_check_f, FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Show SyncE FW version", (PFT)idt8a3_show_fw_version_f, FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"SW reset",            (PFT)idt8a3_sw_reset_f,          FALSE,
            0,                  (PFT)0,                        0,
           (PFT)0,              0},
        {"EEPROM read",         (PFT)idt8a3_eeprom_read_f,       FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"EEPROM write",        (PFT)idt8a3_eeprom_write_f,      FALSE,
            0,                  (PFT)0,                        0,
           (PFT)0,              0},
        {"Read EEPROM file",    (PFT)idt8a3_eeprom_read_file_f,  FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"EEPROM program",      (PFT)idt8a3_eeprom_prog_f,       FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Set PHY recovered clock output",  (PFT)idt8a3_phy_clk_f, FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
        {"Clear all sticky status", (PFT)idt8a3_clear_all_sticky_f, FALSE,
            0,                  (PFT)0,                        0,
            (PFT)0,             0},
};


/******************************************************************************
 *  List of Menu used for SyncE PLL
 *****************************************************************************/
static submenu_xtable_t SYNCE_PLL_tests_submenu_table[] = {
   {"SyncE PLL Register Test", (type_t(*)())idt8a3_reg_test_f,   TRUE,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SyncE PLL Interrupt Test", (type_t(*)())idt8a3_pll_intr_test_f,   TRUE,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SyncE Recovered Clock Test", (type_t(*)())idt8a3_recovered_clock_test_f,   TRUE,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"SyncE PLL Utility", (type_t(*)())build_synce_pll_utility_menu,   FALSE,
    0, NULL, 0, (type_t(*)())build_synce_pll_utility_menu,   TRUE},
};


/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define SYNCE_TESTS_SUBMENU_TABLE_SIZE (sizeof(SYNCE_PLL_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))
#define SYNCEPLL_TEST_UTIL_SIZE (sizeof(SYNCE_util_items) / \
                                 sizeof(submenu_xtable_t))

/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t SYNCE_PLL_tests_primary_items[SYNCE_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t SYNCE_PLL_tests_secondary_items[SYNCE_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t SYNCE_PLL_tests_primary_util_items[SYNCEPLL_TEST_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t SYNCE_PLL_secondary_util_items[SYNCEPLL_TEST_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];





/******************************************************************************
 * SyncE Test/Utils submenu
 *****************************************************************************/
static struct menuinfo synce_pll_util_menu = {
  "SyncE PLL Utility Menu",	    /* title */
  0,                            /* title string added by init_empty_menu */
  0,                            /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size - bumped by add_menu_item */
  SYNCE_PLL_tests_primary_util_items,
};
static struct menuinfo *syncE_pll_util_menup = &synce_pll_util_menu;


menuinfo_t SYNCE_PLL_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    SYNCE_PLL_tests_primary_items,
};
menuinfo_t *SYNCE_PLL_submenup = &SYNCE_PLL_subtest_menu;


/**********************************************************************
 *
 * function:    build_synce_pll_utility_menu
 *
 * Description: Build menu for SyncE PLL utility.
 *
 * Input:       dummp - Not used.
 *
 * Output: None.
 *
 **********************************************************************
 */
void build_synce_pll_utility_menu(int dummy)
{

    build_primary_submenu(SYNCE_util_items, SYNCEPLL_TEST_UTIL_SIZE,
                          "SyncE PLL Utility Menu", &syncE_pll_util_menup);
    build_secondary_submenu(SYNCE_util_items,  SYNCEPLL_TEST_UTIL_SIZE,
                            SYNCE_PLL_secondary_util_items);

    menu(syncE_pll_util_menup, SYNCE_PLL_secondary_util_items, 0);

}


/*******************************************************************************
 *
 * Function   : synce_pll_test_menu
 * Description: build the menu for SyncE PLL test and utility
 * Inputs     : Test/Menu
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int synce_pll_test_menu (int show_menu)
{
    build_primary_submenu(SYNCE_PLL_tests_submenu_table,
                          SYNCE_TESTS_SUBMENU_TABLE_SIZE,
                          "SyncE PLL", &SYNCE_PLL_submenup);
    build_secondary_submenu(SYNCE_PLL_tests_submenu_table,
                            SYNCE_TESTS_SUBMENU_TABLE_SIZE,
                            SYNCE_PLL_tests_secondary_items);

    if (show_menu) {
        exec_doall_menu_items(SYNCE_PLL_submenup);
    }
    else {
        menu(SYNCE_PLL_submenup, SYNCE_PLL_tests_secondary_items, '\0' );
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : idt8a3_block_ptr
 * Description: point to register block info according to "block_id".
 * Inputs     : block_id - idt8a3 register group by block number (0 ~ 37)
 * Outputs    : pointer to register block info according to "block_id"
 *
 *******************************************************************************
 */
idt8a3_block_t * idt8a3_block_ptr( uint8_t block_id )
{

    uint8_t ix;

    for ( ix=0; ix<IDT_BLOCK_MAX; ix++ ) {
        if ( block_id == idt_block_info[ix].block_id ) {
           return (idt8a3_block_t *)&idt_block_info[ix];
    	}
    }

    printf("ERROR:  %s: Invalid block %d!  returning block 0!\n",
          __FUNCTION__, block_id);

    return (idt8a3_block_t *)&idt_block_info[0];
}

/*******************************************************************************
 *
 * Function   : idt8a3_reg_ptr
 * Description: point to registers group info according to "block_id".
 * Inputs     : block_id - idt8a3 register group by block number (0 ~ 37)
 * Outputs    : pointer to registers group info according to "block_id"
 *
 *******************************************************************************
 */
idt8a3_reg_t * idt8a3_reg_ptr( uint8_t block_id )
{
    switch( block_id ) {
    case IDT_BLOCK_HW_REVISION                : return (idt8a3_reg_t *)&IDT_BLOCK_HW_REVISION_reg_info[0];
    case IDT_BLOCK_RESET_CTRL                 : return (idt8a3_reg_t *)&IDT_BLOCK_RESET_CTRL_reg_info[0];
    case IDT_BLOCK_GENERAL_STATUS             : return (idt8a3_reg_t *)&IDT_BLOCK_GENERAL_STATUS_reg_info[0];
    case IDT_BLOCK_STATUS                     : return (idt8a3_reg_t *)&IDT_BLOCK_STATUS_reg_info[0];
    case IDT_BLOCK_GPIO_USER_CONTROL          : return (idt8a3_reg_t *)&IDT_BLOCK_GPIO_USER_CONTROL_reg_info[0];
    case IDT_BLOCK_STICKY_STATUS_CLEAR        : return (idt8a3_reg_t *)&IDT_BLOCK_STICKY_STATUS_CLEAR_reg_info[0];
    case IDT_BLOCK_GPIO_TOD_NOTIFICATION_CLEAR: return (idt8a3_reg_t *)&IDT_BLOCK_GPIO_TOD_NOTIFICATION_CLEAR_reg_info[0];
    case IDT_BLOCK_ALERT_CFG                  : return (idt8a3_reg_t *)&IDT_BLOCK_ALERT_CFG_reg_info[0];
    case IDT_BLOCK_SYS_DPLL_XO                : return (idt8a3_reg_t *)&IDT_BLOCK_SYS_DPLL_XO_reg_info[0];
    case IDT_BLOCK_SYS_APLL                   : return (idt8a3_reg_t *)&IDT_BLOCK_SYS_APLL_reg_info[0];
    case IDT_BLOCK_INPUT_0                    : return (idt8a3_reg_t *)&IDT_BLOCK_INPUT_0_reg_info[0];
    case IDT_BLOCK_REF_MON_0                  : return (idt8a3_reg_t *)&IDT_BLOCK_REF_MON_0_reg_info[0];
    case IDT_BLOCK_DPLL_0                     : return (idt8a3_reg_t *)&IDT_BLOCK_DPLL_0_reg_info[0];
    case IDT_BLOCK_SYS_DPLL                   : return (idt8a3_reg_t *)&IDT_BLOCK_SYS_DPLL_reg_info[0];
    case IDT_BLOCK_DPLL_CTRL_0                : return (idt8a3_reg_t *)&IDT_BLOCK_DPLL_CTRL_0_reg_info[0];
    case IDT_BLOCK_SYS_DPLL_CTRL              : return (idt8a3_reg_t *)&IDT_BLOCK_SYS_DPLL_CTRL_reg_info[0];
    case IDT_BLOCK_DPLL_PHASE_0               : return (idt8a3_reg_t *)&IDT_BLOCK_DPLL_PHASE_0_reg_info[0];
    case IDT_BLOCK_DPLL_FREQ_0                : return (idt8a3_reg_t *)&IDT_BLOCK_DPLL_FREQ_0_reg_info[0];
    case IDT_BLOCK_DPLL_PHASR_PULL_IN_0       : return (idt8a3_reg_t *)&IDT_BLOCK_DPLL_PHASR_PULL_IN_0_reg_info[0];
    case IDT_BLOCK_GPIO_CFG                   : return (idt8a3_reg_t *)&IDT_BLOCK_GPIO_CFG_reg_info[0];
    case IDT_BLOCK_GPIO_0                     : return (idt8a3_reg_t *)&IDT_BLOCK_GPIO_0_reg_info[0];
    case IDT_BLOCK_OUT_DIV_MUX                : return (idt8a3_reg_t *)&IDT_BLOCK_OUT_DIV_MUX_reg_info[0];
    case IDT_BLOCK_OUTPUT_0                   : return (idt8a3_reg_t *)&IDT_BLOCK_OUTPUT_0_reg_info[0];
    case IDT_BLOCK_SERIAL                     : return (idt8a3_reg_t *)&IDT_BLOCK_SERIAL_reg_info[0];
    case IDT_BLOCK_PWM_ENCODER_0              : return (idt8a3_reg_t *)&IDT_BLOCK_PWM_ENCODER_0_reg_info[0];
    case IDT_BLOCK_PWM_DECODER_0              : return (idt8a3_reg_t *)&IDT_BLOCK_PWM_DECODER_0_reg_info[0];
    case IDT_BLOCK_PWM_USER_DATA              : return (idt8a3_reg_t *)&IDT_BLOCK_PWM_USER_DATA_reg_info[0];
    case IDT_BLOCK_TOD_0                      : return (idt8a3_reg_t *)&IDT_BLOCK_TOD_0_reg_info[0];
    case IDT_BLOCK_TOD_WRITE_0                : return (idt8a3_reg_t *)&IDT_BLOCK_TOD_WRITE_0_reg_info[0];
    case IDT_BLOCK_TOD_READ_PRIMARY_0         :
    case IDT_BLOCK_TOD_READ_SECONDARY_0       : return (idt8a3_reg_t *)&IDT_BLOCK_TOD_READ_xxxx_0_reg_info[0];
    case IDT_BLOCK_OUTPUT_TDC_CFG             : return (idt8a3_reg_t *)&IDT_BLOCK_OUTPUT_TDC_CFG_reg_info[0];
    case IDT_BLOCK_OUTPUT_TDC_0               : return (idt8a3_reg_t *)&IDT_BLOCK_OUTPUT_TDC_0_reg_info[0];
    case IDT_BLOCK_INPUT_TDC                  : return (idt8a3_reg_t *)&IDT_BLOCK_INPUT_TDC_reg_info[0];
    case IDT_BLOCK_SCRATCH                    : return (idt8a3_reg_t *)&IDT_BLOCK_SCRATCH_reg_info[0];
    case IDT_BLOCK_EEPROM                     : return (idt8a3_reg_t *)&IDT_BLOCK_EEPROM_reg_info[0];
    case IDT_BLOCK_OTP                        : return (idt8a3_reg_t *)&IDT_BLOCK_OTP_reg_info[0];
    case IDT_BLOCK_BYTE                       : return (idt8a3_reg_t *)&IDT_BLOCK_BYTE_reg_info[0];

    }

    printf("ERROR:  %s: Invalid block %d!  returning block 0!\n",
          __FUNCTION__, block_id);

    return (idt8a3_reg_t *)&IDT_BLOCK_RESET_CTRL_reg_info[0];
}

/*******************************************************************************
 *
 * Function   : idt8a3_instance_max
 * Description: Get number of register set (instance) in each register block
 *              'block_id' group.
 *              eg. for IDT_BLOCK_INPUT_0, Module INPUT registers, there are
 *              total 16 set of of INPUT, will return 16 for IDT_BLOCK_INPUT_0.
 *                  INPUT_0,
 *                  INPUT_1,
 *                  ...
 *                  INPUT_15
 * Inputs     : block_id - idt8a3 register group by block number (0 ~ 37)
 * Outputs    : number of register set in each register block 'block_id' group.
 *
 *******************************************************************************
 */
uint8_t idt8a3_instance_max( uint8_t block_id )
{
    switch ( block_id ) {
    case IDT_BLOCK_INPUT_0:              return 16;
    case IDT_BLOCK_REF_MON_0:            return 16;
    case IDT_BLOCK_DPLL_0:               return  8;
    case IDT_BLOCK_DPLL_CTRL_0:          return  8;
    case IDT_BLOCK_DPLL_PHASE_0:         return  8;
    case IDT_BLOCK_DPLL_FREQ_0:          return  8;
    case IDT_BLOCK_DPLL_PHASR_PULL_IN_0: return  8;
    case IDT_BLOCK_GPIO_0:               return 16;
    case IDT_BLOCK_OUTPUT_0:             return 12;
    case IDT_BLOCK_PWM_ENCODER_0:        return  8;
    case IDT_BLOCK_PWM_DECODER_0:        return 16;
    case IDT_BLOCK_TOD_0:                return  4;
    case IDT_BLOCK_TOD_WRITE_0:          return  4;
    case IDT_BLOCK_TOD_READ_PRIMARY_0:   return  4;
    case IDT_BLOCK_TOD_READ_SECONDARY_0: return  4;
    case IDT_BLOCK_OUTPUT_TDC_0:         return  4;
    default:                             return  1;
    }
}

/*******************************************************************************
 *
 * Function   : idt8a3_instance_base
 * Description: Get the base address of register set from instance 'inst_id'
 *              in register block group 'block_id'.
 *              eg. if 'block_id' is 10, 'inst_id' is 2, is to get base address of
 *              Module INPUT_2, which is 0xc1d0.
 *
 * Inputs     : block_id - idt8a3 register group by block number (0 ~ 37)
 *              inst_id -  instance of register set
 * Outputs    : base address of register set from instance 'inst_id'
 *              in register block group 'block_id'.
 *
 *******************************************************************************
 */
uint32_t idt8a3_instance_base( uint8_t block_id, uint8_t inst_id )
{
    uint32_t INPUT_0[] = { 0xc1b0, 0xc1c0, 0xc1d0,
                           0xc200, 0xc210, 0xc220, 0xc230, 0xc240, 0xc250, 0xc260,
                           0xc280, 0xc290, 0xc2a0, 0xc2b0, 0xc2c0, 0xc2d0 };
    uint32_t REF_MON_0[] = { 0xc2e0, 0xc2ec,
                             0xc300, 0xc30c,
                             0xc318, 0xc324, 0xc330, 0xc33c,
                             0xc348, 0xc354, 0xc360, 0xc36c,
                             0xc380, 0xc38c, 0xc398, 0xc3a4 };
    uint32_t DPLL_0[] = { 0xc3b0, 0xc400, 0xc438, 0xc480,
                          0xc4b8, 0xc500, 0xc538, 0xc580 };
    uint32_t DPLL_CTRL_0[] = { 0xc600, 0xc63c, 0xc680, 0xc6bc,
                               0xc700, 0xc73c, 0xc780, 0xc7bc };
    uint32_t DPLL_PHASE_0[] = { 0xc818, 0xc81c, 0xc820, 0xc824,
                                0xc828, 0xc82c, 0xc830, 0xc834 };
    uint32_t DPLL_FREQ_0[] = { 0xc838, 0xc840, 0xc848, 0xc850,
                               0xc858, 0xc860, 0xc868, 0xc870 };
    uint32_t DPLL_PHASR_PULL_IN_0[] = { 0xc880, 0xc888, 0xc890, 0xc898,
                                        0xc8a0, 0xc8a8, 0xc8b0, 0xc8b8 };
    uint32_t GPIO_0[] = { 0xc8c2, 0xc8d4, 0xc8e6,
                          0xc900, 0xc912, 0xc924, 0xc936, 0xc948, 0xc95a,
                          0xc980, 0xc992, 0xc9a4, 0xc9b6, 0xc9c8, 0xc9da, 0xca00 };
    uint32_t OUTPUT_0[] = { 0xca14, 0xca24, 0xca34, 0xca44, 0xca54, 0xca64,
                            0xca80, 0xca90, 0xcaa0, 0xcbb0, 0xcac0, 0xcad0 };
    uint32_t PWM_ENCODER_0[] = { 0xcb00, 0xcb08, 0xcb10, 0xcb18,
                                 0xcb20, 0xcb28, 0xcb30, 0xcb38 };
    uint32_t PWM_DECODER_0[] = { 0xcb40, 0xcb48, 0xcb50, 0xcb58, 0xcb60, 0xcb68, 0xcb70,
                                 0xcb80, 0xcb88, 0xcb90, 0xcb98, 0xcba0, 0xcba8, 0xcbb0, 0xcbb8, 0xcbc0};
    uint32_t TOD_0[] = { 0xcbcc, 0xcbce, 0xcbd0, 0xcbd2 };
    uint32_t TOD_WRITE_0[] = { 0xcc00, 0xcc10, 0xcc20, 0xcc30 };
    uint32_t TOD_READ_PRIMARY_0[] = { 0xcc40, 0xcc50, 0xcc60, 0xcc80 };
    uint32_t TOD_READ_SECONDARY_0[] = { 0xcc90, 0xcca0, 0xccb0, 0xccc0 };
    uint32_t OUTPUT_TDC_0[] = { 0xcd00, 0xcd08, 0xcd10, 0xcd18 };

    uint32_t base = idt_block_info[0].base;
    uint8_t max = idt8a3_instance_max( block_id );

    if ( max <= inst_id ) {
         printf("ERROR:  %s: Invalid block/inst %d/%d!\n",
         __FUNCTION__, block_id, inst_id);
    }
    else if ( max == 1 ) {
        /* Base addr for blocks with 1 instance. */
        base = idt_block_info[block_id].base;
    }
    else {
        /* Base addr for block with multi instances. */
        switch ( block_id ) {
        case IDT_BLOCK_INPUT_0:              base = INPUT_0[inst_id];              break;
        case IDT_BLOCK_REF_MON_0:            base = REF_MON_0[inst_id];            break;
        case IDT_BLOCK_DPLL_0:               base = DPLL_0[inst_id];               break;
        case IDT_BLOCK_DPLL_CTRL_0:          base = DPLL_CTRL_0[inst_id];          break;
        case IDT_BLOCK_DPLL_PHASE_0:         base = DPLL_PHASE_0[inst_id];         break;
        case IDT_BLOCK_DPLL_FREQ_0:          base = DPLL_FREQ_0[inst_id];          break;
        case IDT_BLOCK_DPLL_PHASR_PULL_IN_0: base = DPLL_PHASR_PULL_IN_0[inst_id]; break;
        case IDT_BLOCK_GPIO_0:               base = GPIO_0[inst_id];               break;
        case IDT_BLOCK_OUTPUT_0:             base = OUTPUT_0[inst_id];             break;
        case IDT_BLOCK_PWM_ENCODER_0:        base = PWM_ENCODER_0[inst_id];        break;
        case IDT_BLOCK_PWM_DECODER_0:        base = PWM_DECODER_0[inst_id];        break;
        case IDT_BLOCK_TOD_0:                base = TOD_0[inst_id];                break;
        case IDT_BLOCK_TOD_WRITE_0:          base = TOD_WRITE_0[inst_id];          break;
        case IDT_BLOCK_TOD_READ_PRIMARY_0:   base = TOD_READ_PRIMARY_0[inst_id];   break;
        case IDT_BLOCK_TOD_READ_SECONDARY_0: base = TOD_READ_SECONDARY_0[inst_id]; break;
        case IDT_BLOCK_OUTPUT_TDC_0:         base = OUTPUT_TDC_0[inst_id];         break;
        default:
            printf("ERROR:  %s: Invalid block/inst %d/%d !\n",
                   __FUNCTION__, block_id, inst_id);
        }
    }

    return base;
}

/*******************************************************************************
 *
 * Function   : fugazi_idt8a3_set_page
 * Description: In Fugazi, Write register page to idt8a3xxxx
 *              at i2c 1B mode by FPGA I2C bus_5 interface.
 * Inputs     : page - register page (1-byte)
 * Outputs    : PASSED or FAILED
 *
 * 58 FC 00 C0 10 20  #Set page register, 1st byte is dev address, 4th byte is page # (C0)
 *******************************************************************************
 */
int fugazi_idt8a3_set_page( uint8_t *page )
{
    unsigned int  rc = PASSED;
    n2g_i2c_if_t i2c_if;
    uint8_t mode_1B_buf[4];

    /* Set idt8a3 register page from MSB of 2-byte register offset */
    memset(mode_1B_buf, 0, sizeof(mode_1B_buf));
    mode_1B_buf[1] = *page;
    mode_1B_buf[2] = (IDT_1B_MODE & 0xff00) >> 8;
    mode_1B_buf[3] = (IDT_1B_MODE & 0xff);
    if ( idt8a3_debug_flag ) {
         printf("%s(): mode_1B_buf: %02X %02X %02X %02X \n", __FUNCTION__,
             mode_1B_buf[0], mode_1B_buf[1], mode_1B_buf[2], mode_1B_buf[3]);
    }

    memset(&i2c_if, 0, sizeof(i2c_if));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.i2c_dev = MB_I2C_ADDR_SYNCE_REG;
    i2c_if.offset = IDT_PAGE_OFFSET;
    i2c_if.sub_addr_len = 0;
    i2c_if.size = 4;
    i2c_if.buf = (char *)&mode_1B_buf[0];

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("ERROR:  %s:%d Failed to write data to page register offfset 0x%#x with data 0x0x%#x "
                        "at device address 0x%#X (rc = %#x)\n",
                      __FUNCTION__, __LINE__, i2c_if.offset, *mode_1B_buf, i2c_if.i2c_dev, rc);
        return FAILED;
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : fugazi_idt8a3_read
 * Description: In Fugazi, read 1-byte of data from idt8a3xxxx register
 *              at i2c 1B mode by FPGA I2C bus_5 interface.
 * Inputs     : offset - register offset (2-byte)
 *              data   - pointer to data buffer to store read data.
 * Outputs    : PASSED or FAILED
 *
 * example read from register 0xC024:
 * 58 FC 00 C0 10 20  #Set page register, 1st byte is dev address
 * 59 24              #Set page register, 1st byte is dev address
 * B1 <read back data>#Send address with read bit set
 *
 *******************************************************************************
 */
int fugazi_idt8a3_read( uint32_t offset, uint8_t *data, uint32_t *re_addr )
{
    unsigned int  rc = PASSED;
    n2g_i2c_if_t i2c_if;
    uint8_t page;

    /* Set idt8a3 register page from MSB of 2-byte register offset */
    page = (offset & 0xff00) >> 8;
    if (fugazi_idt8a3_set_page (&page) != PASSED) {
        printf("ERROR:  %s:%d Failed to set_page on page register 0x%#x "
                       "at device address 0x%#X \n",
                      __FUNCTION__, __LINE__, page, MB_I2C_ADDR_SYNCE_REG);
        return FAILED;
    }

    memset(&i2c_if, 0, sizeof(i2c_if));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.i2c_dev = MB_I2C_ADDR_SYNCE_REG;
    i2c_if.offset = (offset & 0xff);   /* LSB of 2-byte register offset */
    i2c_if.sub_addr_len = 1;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.buf = (char *)data;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("ERROR:  %s:%d Failed to read data from register on offset 0x%#x "
                       "at device address 0x%#X (rc = %#x)\n",
                      __FUNCTION__, __LINE__, i2c_if.offset, i2c_if.i2c_dev, rc);
        return FAILED;
    }

    if ( idt8a3_debug_flag ) {
        printf("%s:%d read data from register on offset 0x%#x "
               "at device address 0x%#X (rc = %#x)\n",
               __FUNCTION__, __LINE__, i2c_if.offset, i2c_if.i2c_dev, rc);
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   : fugazi_idt8a3_write
 * Description: In Fugazi, write 1-byte of data to idt8a3xxxx register
 *              at i2c 1B mode by FPGA I2C bus_5 interface.
 * Inputs     : offset - register offset (2-byte)
 *              data   - pointer to data buffer stored write data.
 * Outputs    : PASSED or FAILED
 *
 * example write "0x50" to register 0xCBE4:
 * 58 FC 00 CB 10 20  #Set page register, 1st byte is dev address
 * 59 EC 50           #Set page register, 1st byte is dev address
 *
 *******************************************************************************
 */
int fugazi_idt8a3_write( uint32_t offset, uint8_t *data, uint32_t *re_addr )
{
    unsigned int  rc = PASSED;
    n2g_i2c_if_t i2c_if;
    uint8_t page;

    /* Set idt8a3 register page from MSB of 2-byte register offset */
    page = (offset & 0xff00) >> 8;
    if (fugazi_idt8a3_set_page (&page) != PASSED) {
        printf("ERROR:  %s:%d Failed to set_page on page register 0x%#x "
               "at device address 0x%#X \n",
               __FUNCTION__, __LINE__, page, MB_I2C_ADDR_SYNCE_REG);
        return FAILED;
    }

    memset(&i2c_if, 0, sizeof(i2c_if));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.i2c_dev = MB_I2C_ADDR_SYNCE_REG;
    i2c_if.offset = (offset & 0xff);   /* LSB of 2-byte register offset */
    i2c_if.sub_addr_len = 0;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.buf = (char *)data;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("ERROR:  %s:%d Failed to write data to register on offfset 0x%#x with data 0x0x%#x "
                "at device address 0x%#X (rc = %#x)\n",
                __FUNCTION__, __LINE__, i2c_if.offset, *data, i2c_if.i2c_dev, rc);
        return FAILED;
    }

    if ( idt8a3_debug_flag ) {
        printf("%s:%d write data 0x%x to register on offset 0x%#x "
              "at device address 0x%#X (rc = %#x)\n",
               __FUNCTION__, __LINE__, *data, i2c_if.offset, i2c_if.i2c_dev, rc);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : fugazi_idt8a3_eeprom_read
 * Description: In Fugazi, read 1-byte of data from idt8a3xxxx
 *              external EEPROM (Microchip AT24CM01)
 *              by FPGA I2C bus_16 interface to idt8a3xxxx.
 * Inputs     : dev    - I2C device base address
 *            : offset - register offset
 *              data   - pointer to data buffer to store read data.
 *              data_len - data length to read
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int fugazi_idt8a3_eeprom_read( uint8_t dev, uint32_t offset, uint8_t *data, uint32_t data_len )
{
    unsigned int  rc = PASSED;
    n2g_i2c_if_t i2c_if;


    if ( idt8a3_debug_flag ) {
        printf("%s(): dev=0x%X, offset=0x%x \n", __FUNCTION__, dev, offset);
    }

    memset(&i2c_if, 0, sizeof(i2c_if));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_SIXTEEN;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.i2c_dev = dev;
    i2c_if.offset = offset;
    i2c_if.sub_addr_len = 2;
    i2c_if.size = data_len;
    i2c_if.buf = (char *)data;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("ERROR:  %s:%d Failed to read data from EEPROM on offset 0x%#x "
              "at device address 0x%#X (rc = %#x)\n",
              __FUNCTION__, __LINE__, i2c_if.offset, i2c_if.i2c_dev, rc);
        return FAILED;
    }

    if ( idt8a3_debug_flag ) {
        printf(" [0x%02X:0x%04X] -> 0x%02X \n\r", dev, offset, *data );
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   : fugazi_idt8a3_eeprom_write
 * Description: In Fugazi, write 1-byte of data to idt8a3xxxx
 *              external EEPROM (Microchip AT24CM01)
 *              by FPGA I2C bus_16 interface to idt8a3xxxx.
 * Inputs     : dev    - I2C device base address
 *            : offset - register offset
 *              data   - pointer to data buffer to store write data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int fugazi_idt8a3_eeprom_write( uint8_t dev, uint32_t offset, uint8_t *data, uint32_t data_len )
{
    unsigned int  rc = PASSED;
    n2g_i2c_if_t i2c_if;


    if ( idt8a3_debug_flag ) {
         printf("%s(): dev=0x%X, offset=0x%x, data=0x%x \n", __FUNCTION__, dev, offset, *data);
    }

    memset(&i2c_if, 0, sizeof(i2c_if));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_SIXTEEN;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.i2c_dev = dev;
    i2c_if.offset = offset;
    i2c_if.sub_addr_len = 2;
    i2c_if.size = data_len;
    i2c_if.buf = (char *)data;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("ERROR:  %s:%d Failed to write data to EEPROM on offfset 0x%#x with data 0x0x%#x "
               "at device address 0x%#X (rc = %#x)\n",
                __FUNCTION__, __LINE__, i2c_if.offset, *data, i2c_if.i2c_dev, rc);
        return FAILED;
    }

    return (rc);

}


/*******************************************************************************
 *
 * Function   : fugazi_get_idt8a3_eeprom_data
 * Description: In Fugazi, open/read eeprom data from
 *              "/fugazi-diag/idt8a3_eeprom_data" file.
 * Inputs     : eeprom_buf   - pointer to data buffer stored write data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int fugazi_get_idt8a3_eeprom_data( uint8_t *eeprom_buf )
{
    int fd;
    ssize_t size = -1;
    unsigned int  count = IDT_EEPROM_MAX_SIZE;
    off_t offset = 0;
    char idt8a3_eeprom_data_file[64] = {0};



    /* Query FPGA FW filename */
    printf("Please enter EEPROM data file name [ex:idt8a3_eeprom_data.bin], "
           "and make sure it have been copied to fugazi-diag directory\n"
           "(Enter q to quit): ");
    fflush(stdout);

    get_line(idt8a3_eeprom_data_file, sizeof(idt8a3_eeprom_data_file));
    if ( (strcmp(idt8a3_eeprom_data_file, "q") == 0) ||  /* quit */
         (strcmp(idt8a3_eeprom_data_file, "") == 0) ) {   /* quit */
        printf("ERROR:  Failed to get EEPROM data file name!\n");
        return (FAILED);
    }

    /* Got the EEPROM data file name, open and read the file */
    printf("\nGoing to open and read eeprom data from %s \n", idt8a3_eeprom_data_file);
    fd = open(idt8a3_eeprom_data_file, O_RDONLY | O_SYNC);
    if ( fd < 0 )  {
        printf("ERROR:  Failed to open EEPROM data from file path %s!\n",
                idt8a3_eeprom_data_file);
        return FAILED;
    }

    if ( lseek(fd, offset, SEEK_SET) < 0 ) {
        close(fd);
        printf("ERROR:  Failed to lseek beginning of EEPROM data file %s!\n",
                idt8a3_eeprom_data_file);
        return FAILED;
	}

    size = read(fd, eeprom_buf, count);
    if ( size == -1 ) {
        close(fd);
        printf("ERROR:  Failed to read EEPROM data from file path %s!\n",
               idt8a3_eeprom_data_file);
        return FAILED;
	}

    printf("\nRead eeprom data from %s successful\n", idt8a3_eeprom_data_file);

    if (getc_answer("\nDump EEPROM data? (y/n)", "yn", 'n')	== 'y') {

        uint32_t ix, align, start_offset=0, row=0;

        /* display 32 byte aligement */
        printf("eeprom total size is %d (0x%x) \n", (int)size, (int)size);
        start_offset = gethex_answer("display EEPROM data start from offset", 0, 0, IDT_EEPROM_MAX_SIZE-1);
        align = 16;
        for ( ix=start_offset; ix<size; ix++ ) {
            if (( ix % align ) == 0 ) {
                if (row == 24 ) {
                    row = 0;
                    if (getc_answer("\nContinue? (y/n)", "yn", 'n')	== 'n') {
                        break;
                    }
                }
                printf("\n\r [0x%08X]: %02X", ix, eeprom_buf[ix] );
                row ++;
            }
            else {
                printf(" %02X", eeprom_buf[ix] );
            }
        }
        printf("\n");
    }

    return PASSED;
}

/*******************************************************************************
 *
 * Function   : platform_idt8a3_read
 * Description: platform dependent on read from idt8a3xxxx register.
 * Inputs     : offset - register offset
 *              data   - pointer to data buffer to store read data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int platform_idt8a3_read( uint32_t offset, uint8_t *data, uint32_t *re_addr )
{
     return ( fugazi_idt8a3_read( offset, data, re_addr ) );
}

/*******************************************************************************
 *
 * Function   : platform_idt8a3_write
 * Description: platform dependent on write to idt8a3xxxx.
 * Inputs     : offset - register offset
 *              data   - pointer to data buffer stored write data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int platform_idt8a3_write( uint32_t offset, uint8_t *data, uint32_t *re_addr )
{
     return ( fugazi_idt8a3_write( offset, data, re_addr ) );
}


/*******************************************************************************
 *
 * Function   : platform_idt8a3_eeprom_read
 * Description: platform dependent on read from idt8a3xxxx EEPROM.
 * Inputs     : dev    - I2C base address
 *            : offset - register offset
 *              data   - pointer to data buffer to store read data.
 *              data_len - data length to read
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int platform_idt8a3_eeprom_read( uint8_t dev, uint32_t offset, uint8_t *data, uint32_t data_len )
{
    return ( fugazi_idt8a3_eeprom_read( dev, offset, data, data_len ) );
}


/*******************************************************************************
 *
 * Function   : platform_idt8a3_eeprom_write
 * Description: platform dependent on write to idt8a3xxxx EEPROM.
 * Inputs     : dev    - I2C base address
 *            : offset - register offset
 *              data   - pointer to data buffer stored write data.
 *              data_len - data length to write
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int platform_idt8a3_eeprom_write( uint8_t dev, uint32_t offset, uint8_t *data, uint32_t data_len )
{
    return ( fugazi_idt8a3_eeprom_write( dev, offset, data, data_len ) );
}

/*******************************************************************************
 *
 * Function   : platform_get_idt8a3_eeprom_data
 * Description: platform dependent on to read eeprom data from a file.
 * Inputs     : eeprom_buf   - pointer to data buffer stored write data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int platform_get_idt8a3_eeprom_data( uint8_t *eeprom_buf )
{
    return ( fugazi_get_idt8a3_eeprom_data( eeprom_buf ) );
}


/**********************************************************************
 *
 * Function: idt8a3_debug_flag_f
 *
 * Description: Set debug flag on/off for print debug messages or not.
 *
 * Input : none
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int idt8a3_debug_flag_f (void )
{
    uint32_t idt8a3_debug_param = 1;

    idt8a3_debug_param = gethex_answer("Debug flag", 1, 0, 0xFF);
    idt8a3_debug_flag = idt8a3_debug_param;

    return PASSED;
}


/*******************************************************************************
 *
 * Function   : idt8a3_read
 * Description: Read 1-byte of data from idt8a3xxx SyncE PLL device
 * Inputs     : offset - register offset
 *              data   - pointer to data buffer to store read data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_read( uint32_t offset, uint8_t *data, uint32_t *re_addr )
{

    int rv = platform_idt8a3_read( offset, data, re_addr );

    if ( idt8a3_debug_flag ) {
        printf(" [0x%04X] -> 0x%02X \n\r", offset, *data );
    }

    return rv;
}

/*******************************************************************************
 *
 * Function   : idt8a3_write
 * Description: Write 1-byte of data to idt8a3xxx SyncE PLL device
 * Inputs     : offset - register offset
 * 	            data   - pointer to data buffer stored write data.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_write( uint32_t offset, uint8_t *data, uint32_t *re_addr )
{

    if ( idt8a3_debug_flag ) {
        printf(" [0x%04X] <- 0x%02X \n\r", offset, *data );
    }

    return platform_idt8a3_write( offset, data, re_addr );
}

/*******************************************************************************
 *
 * Function   : idt8a3_reg_info
 * Description: display registers information of idt8a3xxx.
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_reg_info( void )
{

    idt8a3_block_t *block_ptr;
    uint8_t  inst_id, inst_max;
    uint8_t  block_id;
    uint32_t base;


    printf("\ndisplay description of all the block ID:\n");

    printf(" ID Unit Base       Name\n\r");

    for ( block_id=0; block_id<IDT_BLOCK_MAX; block_id++ ) {

        block_ptr = idt8a3_block_ptr( block_id );
        inst_max  = idt8a3_instance_max( block_id );

        printf(" %2d  %2d ", block_id, inst_max );

        for ( inst_id=0; inst_id<inst_max; inst_id++ ) {
            base = idt8a3_instance_base( block_id, inst_id );
            printf(" %04X", base );
        }
        printf(" - %s \n\r", block_ptr->name );
    }

    printf(" %2d --- All blocks \n\r", block_id );

    return PASSED;
}

/*******************************************************************************
 *
 * Function   : idt8a3_reg_dump
 * Description: Dump all the registers of idt8a3xxx by block ID.
 * Inputs     : id_param - block ID
 *              non_zero_param   - 1: dump only non_zero contents, 0: dump all contents
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************/
int idt8a3_reg_dump( uint8_t id_param, int non_zero_param )
{
    uint8_t  block_id;
    uint8_t  inst_id, inst_max;
    uint8_t  data[20];
    uint32_t base, addr, re_addr, first_base, non_zero;
    idt8a3_block_t *block_ptr;
    idt8a3_reg_t *reg_ptr;
    uint8_t print_block;

    if (id_param == IDT_BLOCK_MAX) {
        printf("\ndisplay from block ID from %d to %d: \n", IDT_BLOCK_HW_REVISION, (IDT_BLOCK_MAX-1));
    }
    else {
        printf("\ndisplay from block ID %d: \n", id_param);
    }

    for ( block_id=IDT_BLOCK_HW_REVISION; block_id<IDT_BLOCK_MAX; block_id++ ) {

        if (( id_param == IDT_BLOCK_MAX ) || ( id_param == block_id )) {
        }
        else {
            continue;
        }

        block_ptr = idt8a3_block_ptr( block_id );
        reg_ptr   = idt8a3_reg_ptr( block_id );
        inst_max  = idt8a3_instance_max( block_id );

        print_block = 0;

        while ( reg_ptr->name != NULL ) {

            non_zero = 0;

            /* Read for all instances. */
            for ( inst_id=0; inst_id<inst_max; inst_id++ ) {
                base = idt8a3_instance_base( block_id, inst_id );
                addr = base + reg_ptr->offset;
                idt8a3_read( addr, &data[inst_id], &re_addr );

                non_zero |= data[inst_id];
                if ( inst_id == 0 ) {
                    first_base = base;
                }
            }

            /* If all data 0 for this instance and non_zero print, not to print */
            if ( ((non_zero_param ) && ( non_zero != 0)) || (!non_zero_param) ) {
                /* If printing data and not header not yet printed */
                if ( print_block == 0 ) {
                    printf("    base " );
                    for ( inst_id=0; inst_id<inst_max; inst_id++ ) {
                        base = idt8a3_instance_base( block_id, inst_id );
                        printf(" %04X", base );
                    }
                    printf(" - %s \n\r", block_ptr->name );
                    print_block = 1;
                }

                /* print data */
                printf(" [0x%04X]",  reg_ptr->offset );
                for ( inst_id=0; inst_id<inst_max; inst_id++ ) {
                    printf(" 0x%02X", data[inst_id] );
                }
                printf(" - %s \n\r", reg_ptr->name );
            }

            reg_ptr ++;
        } /* while ( reg_ptr->name != NULL ) { */
    } /* for ( block_id=IDT_BLOCK_HW_REVISION; block_id<IDT_BLOCK_MAX; block_id++ ) { */

    return PASSED;
}

/**********************************************************************
 *
 * Function: dt8a3_reg_dump_f
 *
 * Description: Dump idt8a3xxxx (i2c device) registers.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_reg_dump_f( void)
{
    uint32_t idt8a3_block_id_param = IDT_BLOCK_MAX;
    uint32_t idt8a3_nonzero_param = 0;

    /* calling for menu driven */
    idt8a3_block_id_param = getdec_answer("Enter Block ID (0 ~ 37; 38-all; 39-info)",
                      (uint)IDT_BLOCK_HW_REVISION, (uint)0, (uint)(IDT_BLOCK_MAX+1));

    if ( idt8a3_block_id_param == ( IDT_BLOCK_MAX + 1 )) {
        idt8a3_reg_info();
        return PASSED;
    }

    idt8a3_nonzero_param = getdec_answer("Non-zero only (0/1 no/yes)", 0, 0, 1);


    return idt8a3_reg_dump( idt8a3_block_id_param, idt8a3_nonzero_param );
}

/**********************************************************************
 *
 * Function: idt8a3_reg_read_f
 *
 * Description: utility entry point to read idt8a3 (i2c device) register.
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int idt8a3_reg_read_f (void)
{
    uint32_t rc = PASSED;
    uint32_t idt8a3_offset_param = 0x0;
    uint32_t offset;
    uint32_t addr;
    uint8_t  read_data;

    /* calling for menu driven */
   	idt8a3_offset_param = gethex_answer("Enter reg offset", 0x81FA, 0, 0xFFFF);

    offset = idt8a3_offset_param;
    addr = offset;
    rc = idt8a3_read( offset, &read_data, &addr );

    printf(" [0x%04X] = 0x%02X \n\r", addr, read_data );

    return (rc);
}

/**********************************************************************
 *
 * Function: idt8a3_reg_write_f
 *
 * Description: utility entry point to write idt8a3 (i2c device) register
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int idt8a3_reg_write_f(void)
{
    uint32_t idt8a3_offset_param = 0x0;
    uint32_t idt8a3_data_param = 0x0;
    uint32_t offset;
    uint32_t addr;
    uint8_t  write_data;

    /* calling for menu driven */
    idt8a3_offset_param = gethex_answer("Enter reg offset", 0xCF50, 0, 0xFFFF);

    idt8a3_data_param = gethex_answer("Enter data", 0, 0, 0xFF);

    offset = idt8a3_offset_param;
    addr = offset;
    write_data = idt8a3_data_param;
    return idt8a3_write( offset, &write_data, &addr );
}


/*******************************************************************************
 *
 * Function   : idt8a3_status_dump
 * Description: display ida8a3xxxx System DPLLb status from
 *              reg STATUS.DPLL_SYS_STATUS.
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_status_dump( void )
{
    int rv = PASSED;
    uint16_t ix;
    uint32_t base, offset, offset_dpll;
    uint32_t addr;
    uint8_t  data, data2, mask;
    uint8_t  datamon[2];
    uint16_t *p_datamon, ffo_reg;
    int32_t  ffo_value[IDT8A3_INPUT_CLK_FFO_MAX];
    uint8_t  mask_dll[IDT8A3_INPUT_CLK_FFO_MAX];
    char * msg[] = { "freerun",
             "lockacq",
             "lockrec",
             "locked",
             "holdover",
             "open loop" };


    base   =  idt_block_info[IDT_BLOCK_STATUS].base; /* 0xC03C; */
    offset = 0x20;
    rv |= idt8a3_read( base + offset, &data, &addr );
    mask = data & 0xF;
    printf("SYS_DPLL (0x%04X=0x%02x) - ", (base + offset),  data);
    printf("%s",   ( mask < 6 ) ? msg[mask] : "unknown" );
    if ( data & 0x20 ) printf(", holdover transition");
    if ( data & 0x10 ) printf(", locked transition");
    printf("\n\r");

    offset = 0x21;  /* base is 0xC03C; */
    rv |= idt8a3_read( base + offset, &data, &addr );
    printf("SYS_APLL (0x%04X=0x%02x) - ", (base + offset),  data );
    printf("%s", ( data & 0x1 ) ? "unlocked" : "locked" );
    if ( data & 0x10 ) printf(", status changed");
    printf("\n\r");

    /* dump DPLL status */
    for ( ix=0; ix<8; ix++ ) {
        /* base is 0xC03C */
        offset = 0x18 + ix;
        offset_dpll = offset;
        rv |= idt8a3_read( base + offset, &data, &addr );
        offset = 0x22 + ix;
        rv |= idt8a3_read( base + offset, &data2, &addr );

        mask = data & 0xF;
        printf("   DPLL%d (0x%04X=0x%02x, 0x%04X=0x%02x) - ", ix,
            (base + offset_dpll),  data, (base + offset),  data2 );
        printf("%s", ( mask < 6 ) ? msg[mask] : "unknown" );
        if ( data & 0x20 ) printf(", holdover transition");
        if ( data & 0x10 ) printf(", locked transition");

        printf(", ref=" );
        mask = data2 & 0x1F;
        if ( mask < 0x10 ) {
            printf("CLK%d", mask );
        }
        else {
            printf("%s",
                    ( mask == 0x10 ) ? "phase input" :
                    ( mask == 0x11 ) ? "frequency inout" :
                    ( mask == 0x12 ) ? "X0_PLL" :
                    ( mask == 0x1F ) ? "no reference" : "unknown" );
        }
        printf("\n\r");
    }
    printf("\n\r");

    /* dump Input Monitor Frequency status */
    memset(ffo_value, 0, sizeof(ffo_value));
    memset(mask_dll, 0, sizeof(mask_dll));
    for ( ix=0; ix<IDT8A3_INPUT_CLK_FFO_MAX; ix++ ) {
        memset(datamon, 0, sizeof(datamon));
        /* base is 0xC03C */
        offset = 0x8C + (ix * 2);
        offset_dpll = offset;
        rv |= idt8a3_read( (base + offset), &datamon[0], &addr );
        offset = 0x8D + (ix * 2);
        rv |= idt8a3_read( (base + offset), &datamon[1], &addr );

        mask_dll[ix] = datamon[1] & 0xC0;
        mask_dll[ix] = (mask_dll[ix] >> 6);

        p_datamon = (uint16_t *)&datamon[0];
        ffo_reg = *p_datamon;

        /* convert from 14-bit singed value */
        ffo_reg &= 0x3fff;
        ffo_value[ix] = ((int16_t)( (ffo_reg * 4)) / 4 );

        switch (mask_dll[ix]) {
        case 1: /* in 10 ppb unit */
            ffo_value[ix] *= 10;
            break;
        case 2: /* in 100 ppb unit */
            ffo_value[ix] *= 100;
            break;
        }

        printf("   IN%d_MON_FREQ (0x%04X=0x%02x, 0x%04X=0x%02x) - ", ix,
                (base + offset_dpll),  datamon[0], (base + offset),  datamon[1]);
        printf("input clock FFO: % -5d %s (0x%04x)", ffo_value[ix],
                ( mask_dll[ix] < 3 ) ? "ppb" : "ppm", ffo_reg);
        printf("\n\r");
    }

    printf("    FREQ_OFFS_LIM  NO_ACTIVITY  LOS  Input CLK FFO \n\r");
    for ( ix=0; ix<IDT8A3_INPUT_CLK_FFO_MAX; ix++ ) {
        /* base is 0xC03C */
        offset = 0x8 + ix;
        rv |= idt8a3_read( base + offset, &data, &addr );

        printf("   IN%2d - ", ix );
        printf("    %s         %s        %s",
                ( data & 4 ) ? "x" : ".",
                ( data & 2 ) ? "x" : ".",
                ( data & 1 ) ? "x" : "." );
        printf("    % -5d %s  ", ffo_value[ix], ( mask_dll[ix] < 3 ) ? "ppb" : "ppm");
        printf("\n\r");
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: idt8a3_status_dump_f
 *
 * Description: utility entry point to dump status of idt8a3 (i2c device) register
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_status_dump_f( void)
{
    return idt8a3_status_dump();
}


/*******************************************************************************
 *
 * Function   : idt8a3_set_freq_margin
 * Description: Configure SyncE PLL frequency margin from register
 *              DPLL_CTRL_[3:0].DPLL_FOD_FREQ.
 *              Fugazi Each DPLL provides a clock for two outputs as follows:
 *              DPLL[0] margins Q[0] & Q[1],  Output Freq = 500MHz.
 *              DPLL[1] margins Q[2] & Q[3],  Output Freq = 500MHz.
 *              DPLL[2] margins Q[4] & Q[4],  Output Freq = 625MHz.
 *              DPLL[3] margins Q[6] & Q[7],  Output Freq = 500MHz.
 *              Fractional Output Divider (FOD) frequency (in Hz) is determined
 *              by M/N in this register.
 *              Output Frequency = M / N (Hz). N is aways 0x0001
 *              example:
 *              DPLL[0], DPLL[1], or DPLL[3]:  M = 500,000,000 (0x1DCD6500) & N = 1.
 *              DPLL[2]:  M = 625,000,000 (0x2540BE40) & N = 1.
 *              To margin DPLL[0] + 1%, set M = 500,000,000 * 1.01 = 505000000 (0x1E19B040).
 *
 * Inputs     : freq_margin_ppm - margin in ppm uint, 1ppm=1%1000000
 *              freq_margin_high - 1: margin high; 0: margin low;
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_set_freq_margin( uint32_t freq_margin_N, uint32_t freq_margin_ppm,
							uint32_t freq_margin_high)
{
    int rv = PASSED;
    uint32_t base, offset_m, offset_n;
    uint32_t re_addr;
    uint8_t  data_m[6];
    uint8_t  data_n[2];
    uint8_t  dpll_index, ix;
    long long  freq_500mhz_margin_value, freq_625mhz_margin_value;
    float    freq_margin;
    float    freq_current_value;


    if ( idt8a3_debug_flag )
    {
        printf("%s(): freq_margin_N=%d, freq_margin_ppm=%d, freq_margin_high=%d\n", __FUNCTION__,
                      freq_margin_N, freq_margin_ppm, freq_margin_high);
    }

    freq_margin = (float)freq_margin_ppm / SYNCE_FREQUCY_PPM;
    freq_500mhz_margin_value = SYNCE_FREQUCY_500MHZ;
    freq_625mhz_margin_value = SYNCE_FREQUCY_625MHZ;

    if ( idt8a3_debug_flag )
    {
        printf("%s(): freq_margin=%f\n", __FUNCTION__, freq_margin);
        printf("%s(): freq_500mhz_margin_value=%f (0x%X)\n", __FUNCTION__,
                (float)freq_500mhz_margin_value, (int)freq_500mhz_margin_value);
        printf("%s(): freq_625mhz_margin_value=%f (0x%X)\n", __FUNCTION__,
               (float)freq_625mhz_margin_value, (int)freq_625mhz_margin_value);
    }

    /* prepare clock frequency output value */
    if (freq_margin_ppm != 0) {
        if (freq_margin_high) {
            /* margin high */
            freq_500mhz_margin_value += (SYNCE_FREQUCY_500MHZ * freq_margin);
            freq_625mhz_margin_value += (SYNCE_FREQUCY_625MHZ * freq_margin);
        }
        else {
            /* margin low */
            freq_500mhz_margin_value -= (SYNCE_FREQUCY_500MHZ * freq_margin);
            freq_625mhz_margin_value -= (SYNCE_FREQUCY_625MHZ * freq_margin);
        }
        if ( idt8a3_debug_flag )
        {
            printf("%s(): freq_500mhz_margin_value=%f (0x%X)\n", __FUNCTION__,
                    (float)freq_500mhz_margin_value, (int)freq_500mhz_margin_value);
            printf("%s(): freq_625mhz_margin_value=%f (0x%X)\n", __FUNCTION__,
                    (float)freq_625mhz_margin_value, (int)freq_625mhz_margin_value);
        }
    }

    /* Write to DPLL_0, DPLL_1, DPLL_2, DPLL_3 */
    for (dpll_index=FUGAZI_SYNCE_DPLL_CTRL_0;
         dpll_index<MAX_FUGAZI_SYNCE_DPLL_CTRL; dpll_index++) {
        /* get the base offset_m of DPLL_CTRL_0 - 0xC600 */
        base = idt8a3_instance_base( IDT_BLOCK_DPLL_CTRL_0, dpll_index );
        offset_m = base + 0x1c;  /* DPLL_CTRL_[3:0].DPLL_FOD_FREQ - M[7:0] */
        offset_n = base + 0x22;  /* DPLL_CTRL_[3:0].DPLL_FOD_FREQ - N[7:0] */
        if ( idt8a3_debug_flag )
        {
             printf("%s(): DPLL_%d: base=0x%X, offset_m=0x%X, offset_n=0x%X\n",
                    __FUNCTION__, dpll_index, base, offset_m, offset_n);
        }

        /* write to DPLL_CTRL_[3:0].DPLL_FOD_FREQ, M registers */
        memset(data_m, 0, sizeof(data_m));
        /* total 6 M registers */
        for (ix=0; ix<6; ix++) {
            if (dpll_index == FUGAZI_SYNCE_DPLL_CTRL_2) {
                /* 625MHz */
                data_m[ix] = (freq_625mhz_margin_value >> (ix * 8) & 0xff);
                freq_current_value = (float)freq_625mhz_margin_value;
            }
            else {
                /* 500MHz */
                data_m[ix] = (freq_500mhz_margin_value >> (ix * 8)  & 0xff);
                freq_current_value = (float)freq_500mhz_margin_value;
            }
            if ( idt8a3_debug_flag )
            {
                printf("%s(): Write: offset_m=0x%X, data_m[%d]=0x%X\n",
                        __FUNCTION__, (offset_m+ix), ix, data_m[ix]);
            }

            rv |= idt8a3_write( (offset_m+ix), &data_m[ix], &re_addr );
        }

        /* write to DPLL_CTRL_[3:0].DPLL_FOD_FREQ, N registers */
        memset(data_n, 0, sizeof(data_n));
        /* total 2 N registers */
        for (ix=0; ix<2; ix++) {
            data_n[ix] = (freq_margin_N >> (ix * 8) & 0xff);
            if ( idt8a3_debug_flag )
            {
                printf("%s(): Write: offset_n=0x%X, data_n[%d]=0x%X\n",
                        __FUNCTION__, (offset_n+ix), ix, data_n[ix]);
            }

            rv |= idt8a3_write( (offset_n+ix), &data_n[ix], &re_addr );
        }

        freq_current_value /= 1000000;
        printf("DPLL %d: PLL Frequency margin %dppm %s at %fMHz done.\n",
                dpll_index, freq_margin_ppm,
                (freq_margin_ppm==0)? "Normal" : (freq_margin_high)? "High" : "Low",
                freq_current_value);
    }

    return rv;
}

/*******************************************************************************
 *
 * Function   : idt8a3_get_freq_margin
 * Description: Get current SyncE PLL frequency from register
 *              DPLL_CTRL_[3:0].DPLL_FOD_FREQ.
 *
 * Inputs     : dpll_index - to DPLL_CTRL[3:0]
 *              freq_current_value_hz - pointer to store current frequency value
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
static int idt8a3_get_freq_margin( uint8_t dpll_index, unsigned long long *freq_current_value_hz)
{
    int rv = PASSED;
    uint32_t base, offset_m, offset_n;
    uint32_t re_addr;
    uint8_t  data_m[6];
    uint8_t  data_n[2];
    uint8_t  ix;
    unsigned long long *p_data_m, freq_value_hz;
    uint16_t *p_data_n;

    /* Read from DPLL_0, DPLL_1, DPLL_2, DPLL_3 */
    /* get the base offset_m of DPLL_CTRL_0 */
    base = idt8a3_instance_base( IDT_BLOCK_DPLL_CTRL_0, dpll_index );
    offset_m = base + 0x1c;
    offset_n = base + 0x22;
    if ( idt8a3_debug_flag )
    {
        printf("%s(): DPLL_%d: base=0x%X, offset_m=0x%X, offset_n=0x%X\n",
                __FUNCTION__, dpll_index, base, offset_m, offset_n);
    }

    /* read from DPLL_CTRL_[3:0].DPLL_FOD_FREQ, M registers */
    memset(data_m, 0, sizeof(data_m));
    for (ix=0; ix<6; ix++) {
        rv |= idt8a3_read( (offset_m+ix), &data_m[ix], &re_addr );
        if ( idt8a3_debug_flag ) {
            printf("%s(): Read: offset_m=0x%X, data_m[%d]=0x%X\n",
                    __FUNCTION__, (offset_m+ix), ix, data_m[ix]);
        }
    }

    /* read from DPLL_CTRL_[3:0].DPLL_FOD_FREQ, N registers */
    memset(data_n, 0, sizeof(data_n));
    for (ix=0; ix<2; ix++) {
        rv |= idt8a3_read( (offset_n+ix), &data_n[ix], &re_addr );
        if ( idt8a3_debug_flag ) {
            printf("%s(): Read: offset_n=0x%X, data_n[%d]=0x%X\n",
                    __FUNCTION__, (offset_n+ix), ix, data_n[ix]);
        }
    }

    p_data_m = (unsigned long long *)&data_m[0];
    p_data_n = (uint16_t *)&data_n[0];

    freq_value_hz = 0;
    freq_value_hz = *p_data_m / *p_data_n;
    *freq_current_value_hz = freq_value_hz;

    return rv;
}

/*******************************************************************************
 *
 * Function   : idt8a3_show_freq
 * Description: Display current SyncE PLL frequency from register
 *              DPLL_CTRL_[3:0].DPLL_FOD_FREQ.
 *              Fugazi Each DPLL provides a clock for two outputs as follows:
 *              DPLL[0] margins Q[0] & Q[1],  Output Freq = 500MHz.
 *              DPLL[1] margins Q[2] & Q[3],  Output Freq = 500MHz.
 *              DPLL[2] margins Q[4] & Q[4],  Output Freq = 625MHz.
 *              DPLL[3] margins Q[6] & Q[7],  Output Freq = 500MHz.
 *              Fractional Output Divider (FOD) frequency (in Hz) is determined
 *              by M/N in this register.
 *              Output Frequency = M / N (Hz). N is aways 0x0001
 *
 * Inputs     : display (1: display status; 0: not display)
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_show_freq( void )
{
    int rv = PASSED;
    uint8_t  dpll_index;
    unsigned long long freq_normal_value;
    unsigned long long freq_current_value_hz;
    long double freq_current_value_mhz = 0.0;

    /* Read from DPLL_0, DPLL_1, DPLL_2, DPLL_3 */
    for (dpll_index=FUGAZI_SYNCE_DPLL_CTRL_0;
         dpll_index<MAX_FUGAZI_SYNCE_DPLL_CTRL; dpll_index++) {
        freq_current_value_hz = 0;
        /* Get current SyncE PLL frequency from register */
        rv |= idt8a3_get_freq_margin( dpll_index, &freq_current_value_hz );
        if ( rv != PASSED ) {
            printf("%s(): idt8a3_get_freq_margin() failed at DLL %d\n",
                    __FUNCTION__, dpll_index);
            return (rv);
        }

        freq_current_value_mhz = (long double)(freq_current_value_hz / 1000000.0);

        if (dpll_index == FUGAZI_SYNCE_DPLL_CTRL_2) {
            freq_normal_value = SYNCE_FREQUCY_625MHZ;
        }
        else {
            freq_normal_value = SYNCE_FREQUCY_500MHZ;
        }

        /* display current Frequency when call to display */
        printf("DPLL %d: PLL current Frequency %LfMHz - %s \n",
                dpll_index, freq_current_value_mhz,
                (freq_current_value_hz > freq_normal_value)?
                "High" : (freq_current_value_hz < freq_normal_value)? "Low" : "Normal");

    } /* for (dpll_index=0; dpll_index<4; dpll_index++) { */

    return rv;
}


/*******************************************************************************
 *
 * Function   : idt8a3_input_clock_check_get_data
 * Description: Read values of REG. STATUS.IN#_MON_STATUS.
 *
 * Inputs     : print_data - to display value in this function? (1: yes; 0: no)
 *              valid - storage to store bit_2.
 *              no_act - storage to store bit_1.
 *              los - storage to store bit_0.
 *              s_valid - storage to store bit_6.
 *              s_no_act - storage to store bit_5.
 *              s_los - storage to store bit_4.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_input_clock_check_get_data( int print_data, uint32_t *valid, uint32_t *no_act, uint32_t *los,
                                       uint32_t *s_valid, uint32_t *s_no_act, uint32_t *s_los)
{

    int rv = PASSED;
    uint32_t ix;
    uint32_t base, offset;
    uint32_t addr;
    uint8_t  data;

    *valid  = 0;
    *no_act = 0;
    *los    = 0;
    *s_valid  = 0;
    *s_no_act = 0;
    *s_los    = 0;

    for ( ix=0; ix<16; ix++ ) {
        base   = idt_block_info[IDT_BLOCK_STATUS].base; /* 0xC03C */
        offset = 0x8 + ix; /* reg. STATUS.IN#_MON_STATUS offset */
        rv |= idt8a3_read( base + offset, &data, &addr );
        if ( print_data )
        {
            printf(" [0x%04X] = 0x%02X IN%d_MON \n\r", (base + offset), data, ix );
        }

        *valid  |= ( data & 4 ) ? ( 1 << ix ) : 0;
        *no_act |= ( data & 2 ) ? ( 1 << ix ) : 0;
        *los    |= ( data & 1 ) ? ( 1 << ix ) : 0;

        *s_valid  |= ( data & 0x40 ) ? ( 1 << ix ) : 0;
        *s_no_act |= ( data & 0x20 ) ? ( 1 << ix ) : 0;
        *s_los    |= ( data & 0x10 ) ? ( 1 << ix ) : 0;
        if ( print_data )
        {
            printf("%s() ix=%d, los=0x%x, no_act=0x%x, valid=0x%x\n",
                    __FUNCTION__, ix, *los, *no_act, *valid);
        }

    }

    return rv;
}

/*******************************************************************************
 *
 * Function   : idt8a3_input_clock_check_print_data
 * Description: Display 16-bit of input 'data'. display 'x' if bit value is 1,
 *              '.' if bit value is 0.
 *
 * Inputs     : data - data bit value be display
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_input_clock_check_print_data( uint32_t data ) {
    int ix;

    for ( ix=15; 0<=ix; ix-- ) {
    	printf( " %s ", (data&(1<<ix)) ? "x": "." );
    }

    return PASSED;
}


/*******************************************************************************
 *
 * Function   : idt8a3_input_clock_print
 * Description: Display bit of los, no_act, valid in 'x' or '.' format.
 *
 * Inputs     : valid - STATUS.IN#_MON_STATUS bit_2.
 *              no_act - STATUS.IN#_MON_STATUS bit_1.
 *              los - STATUS.IN#_MON_STATUS bit_0.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
void idt8a3_input_clock_print( uint32_t valid, uint32_t no_act, uint32_t los ) {
    printf("\n\r            IN 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0");
    printf("\n\r           LOS " );  idt8a3_input_clock_check_print_data(los);
    printf("\n\r   NO_ACTIVITY " );  idt8a3_input_clock_check_print_data(no_act);
    printf("\n\r FREQ_OFFS_LIM " );  idt8a3_input_clock_check_print_data(valid);
    printf("\n\r");
}

/*******************************************************************************
 *
 * Function   : idt8a3_input_clock_monitor_print
 * Description: Display bit of los, no_act, valid and 'sticky' bits in 'x'
 *              or '.' format.
 *
 * Inputs     : valid - STATUS.IN#_MON_STATUS bit_2.
 *              no_act - STATUS.IN#_MON_STATUS bit_1.
 *              los - STATUS.IN#_MON_STATUS bit_0.
 *              s_valid - STATUS.IN#_MON_STATUS bit_6.
 *              s_no_act - STATUS.IN#_MON_STATUS bit_5.
 *              s_los - STATUS.IN#_MON_STATUS bit_4.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
void idt8a3_input_clock_monitor_print( uint32_t valid, uint32_t no_act, uint32_t los,
                            uint32_t s_valid, uint32_t s_no_act, uint32_t s_los)
{
    printf("\n\r                   IN 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0");
    printf("\n\r                  LOS " );  idt8a3_input_clock_check_print_data(los);
    printf("\n\r          NO_ACTIVITY " );  idt8a3_input_clock_check_print_data(no_act);
    printf("\n\r        FREQ_OFFS_LIM " );  idt8a3_input_clock_check_print_data(valid);
    printf("\n\r           LOS_STICKY " );  idt8a3_input_clock_check_print_data(s_los);
    printf("\n\r   NO_ACTIVITY_STICKY " );  idt8a3_input_clock_check_print_data(s_no_act);
    printf("\n\r FREQ_OFFS_LIM_STICKY " );  idt8a3_input_clock_check_print_data(s_valid);
    printf("\n\r");
}

/*******************************************************************************
 *
 * Function   : idt8a3_clear_all_sticky
 * Description: Clear all stickey status bits in the STATUS module by write
 *              0x01 to reg. STICKY_STATUS_CLEAR.ALL_STICKEY_CLEAR.
 *
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_clear_all_sticky( void )
{
    uint32_t base;
    uint32_t addr;
    uint8_t  data = 1;
    uint32_t re_addr;

    base = idt_block_info[IDT_BLOCK_STICKY_STATUS_CLEAR].base; /* 0xC164 */
    addr = base + 0x05; /* 0xc169 STICKY_STATUS_CLEAR.ALL_STICKEY_CLEAR */

    /* Clear all sticky status bits */
    return idt8a3_write( addr, &data, &re_addr );
}

/*******************************************************************************
 *
 * Function   : idt8a3_input_clock_rate
 * Description: Configure idt8a3xxxx register INPUT_#.IN_FREQ/IN_DIV/IN_MODE,
 *              such as input frequency/reference clock, and enable input clock.
 * Inputs     : phy_clk_index - which PHY port to test recovered clock
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_input_clock_rate( int phy_clk_index )
{
    int    rv = PASSED;
    uint8_t  input_index, ix;
    uint32_t offset_m, offset_n, offset_div, offset_sync, offset_mode;
    uint8_t  data_m[6], data_n[2], data_div[2], data_sync, data_mode, data_r;
    uint16_t data_div_value, freq_n_value;
    uint32_t base, re_addr;
    long long  freq_m_value;

    printf("%s(): phy_clk_index=%d\n", __FUNCTION__, phy_clk_index);

    if (phy_clk_index >= FUGAZI_MAX_PHY_PORT_CLK_TEST) {
        cterr('f',0,"%s(): Incorrect PHY port %d to be tested! \n", __FUNCTION__, phy_clk_index);
    }

    input_index = IDT_INPUT_info[phy_clk_index].input_no;
    base = idt8a3_instance_base( IDT_BLOCK_INPUT_0, input_index ); /* 0xC1B0 */
    if ( idt8a3_debug_flag )
    {
        printf("%s(): phy_clk_index=%d, input_index=%d, base=0x%X\n",
                __FUNCTION__, phy_clk_index, input_index, base);
    }


    /* write to INPUT_#.IN.FREQ, M registers */
    memset(data_m, 0, sizeof(data_m));
    freq_m_value = IDT_INPUT_info[phy_clk_index].in_freq;
    offset_m = base + 0x00;
    for (ix=0; ix<6; ix++) {
        data_m[ix] = (freq_m_value >> (ix * 8) & 0xff);
        if ( idt8a3_debug_flag )
        {
             printf("%s(): Write: offset_m=0x%X, data_m[%d]=0x%X\n",
                       __FUNCTION__, (offset_m+ix), ix, data_m[ix]);
        }

        rv |= idt8a3_write( (offset_m+ix), &data_m[ix], &re_addr );
    }

    /* write to INPUT_#.IN.FREQ, N registers */
    memset(data_n, 0, sizeof(data_n));
    freq_n_value = 0x0001;
    offset_n = base + 0x06;
    for (ix=0; ix<2; ix++) {
        data_n[ix] = (freq_n_value >> (ix * 8) & 0xff);
        if ( idt8a3_debug_flag )
        {
            printf("%s(): Write: offset_n=0x%X, data_n[%d]=0x%X\n",
                    __FUNCTION__, (offset_n+ix), ix, data_n[ix]);
        }

        rv |= idt8a3_write( (offset_n+ix), &data_n[ix], &re_addr );
    }

    /* write to INPUT_#.IN.DIV registers, divided by 1 */
    data_div_value = IDT_INPUT_info[phy_clk_index].in_div;
    offset_div = base + 0x08;
    for (ix=0; ix<2; ix++) {
        data_div[ix] = (data_div_value >> (ix * 8) & 0xff);
        if ( idt8a3_debug_flag )
        {
            printf("%s(): Write: offset_div=0x%X, data_div[%d]=0x%X\n",
                    __FUNCTION__, (offset_div+ix), ix, data_div[ix]);
        }

        rv |= idt8a3_write( (offset_div+ix), &data_div[ix], &re_addr );
    }

    /* write to INPUT_#.IN.SYNC registers, Select CLK1 as reference input */
    data_sync = IDT_INPUT_info[phy_clk_index].in_sync;
    offset_sync = base + 0x0C;
    if ( idt8a3_debug_flag )
    {
        printf("%s(): Write: offset_sync=0x%X, data_sync=0x%X\n",
                        __FUNCTION__, offset_sync, data_sync);
    }
    rv |= idt8a3_write( offset_sync, &data_sync, &re_addr );

    /* write to INPUT_#.IN.MODE registers, enable Input clock  */
    data_mode = IDT_INPUT_info[phy_clk_index].in_mode;
    offset_mode = base + 0x0D;
    rv |= idt8a3_read( offset_mode, &data_r, &re_addr );
    data_mode |= data_r;

    if ( idt8a3_debug_flag )
    {
        printf("%s(): Write: offset_mode=0x%X, data_mode=0x%X\n",
                    __FUNCTION__, offset_mode, data_mode);
    }
    rv |= idt8a3_write( offset_mode, &data_mode, &re_addr );

    return ( rv );
}


/*******************************************************************************
 *
 * Function   : idt8a3_input_ref_monitor_cfg
 * Description: Configure idt8a3xxxx register REF_MON_#.IN_MON_CFG.
 * Inputs     : phy_clk_index - which PHY port to test recovered clock
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_input_ref_monitor_cfg( int phy_clk_index )
{
    int    rv = PASSED;
    uint8_t  mon_index;
    uint32_t base, re_addr;
    uint32_t offset;
    uint8_t  data;


    printf("%s(): phy_clk_index=%d\n", __FUNCTION__, phy_clk_index);

    if (phy_clk_index >= FUGAZI_MAX_PHY_PORT_CLK_TEST) {
        cterr('f',0,"%s(): Incorrect PHY port %d to be tested! \n", __FUNCTION__, phy_clk_index);
    }

    mon_index = IDT_INPUT_info[phy_clk_index].input_no;
    base = idt8a3_instance_base( IDT_BLOCK_REF_MON_0, mon_index );
    if ( idt8a3_debug_flag )
    {
        printf("%s(): phy_clk_index=%d, mon_index=%d, base=0x%X\n",
                __FUNCTION__, phy_clk_index, mon_index, base);
    }

    /* write to REF_MON_#.IN_MON_CFG registers, enable reference monitor  */
    offset = base + 0x0B;
    rv |= idt8a3_read( offset, &data, &re_addr );
    if ( idt8a3_debug_flag )
    {
        printf("%s(): Read: offset=0x%X, data=0x%X\n", __FUNCTION__, offset, data);
    }
    data &= ~0x30;
    data |= 0x0f;
   	rv |= idt8a3_write( offset, &data, &re_addr );
    if ( idt8a3_debug_flag )
    {
        printf("%s(): Write: offset=0x%X, data=0x%X\n",	__FUNCTION__, offset, data);
    }

    return ( rv );
}

/*******************************************************************************
 *
 * Function   : idt8a3_input_clock_check
 * Description: Display Input reference monitor status from
 *              reg STATUS.INx_MON_STATUS.
 * Inputs     : clock_mask - mask for all 16 input clocks
 *              mode      - IDT8A3_CHECK_CLOCK_VALID, check for clock valid,
 *                              no LOS, no No Acticity and no Reeq off limit
 *                          IDT8A3_CHECK_CLOCK_INVALID, check for clock invalid,
 *                              LOS, No Acticity and Reeq off limit
 *   			time      - poll time for checking valid/invalid in seconds.
 *              duration  - test if status changed for this duration.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_input_clock_check( uint32_t clock_mask, int mode, int time, int duration )
{
    int rv = PASSED;
    int rv_2 = PASSED;
    int ix, jx;
    uint32_t los, no_act, valid;
    uint32_t s_los, s_no_act, s_valid;

    /* Get live status of alarms and events from reg. STATUS.IN#_MON_STATUS */
    rv = idt8a3_input_clock_check_get_data( 0, &valid, &no_act, &los, &s_valid, &s_no_act, &s_los);
    if (rv != PASSED) {
        printf("ERROR:  %s:%d Failed on idt8a3_input_clock_check_get_data() (rc = %#x)\n",
                      __FUNCTION__, __LINE__, rv);
        return FAILED;
    }

    printf("\n\rTesting %s clock IN - ",( mode == IDT8A3_CHECK_CLOCK_INVALID )?"invalid":"valid");
    for ( ix=15; 0<=ix; ix-- ) {
        if ( clock_mask & ( 1 << ix )) {
            printf("%d ", ix );
        }
    }

    /* Polling by number of 'time' seconds */
    for ( jx=0; jx<time; jx++ ) {

        rv = PASSED;
        printf("." );
        fflush(stdout);

        /* Verify if read Status match as expect? */
        for ( ix=0; ix<16; ix++ ) {

            /* If testing this clock */
            if ( clock_mask & ( 1 << ix )) {

                if ( mode == IDT8A3_CHECK_CLOCK_VALID ) {
                    if ((( los    & ( 1 << ix )) == 0 ) &&
                        (( no_act & ( 1 << ix )) == 0 ) &&
                        (( valid  & ( 1 << ix )) == 0 )) {
                    }
                    else {
                        rv = FAILED;
                    }
                }
                else if ( mode == IDT8A3_CHECK_CLOCK_INVALID ) {
                    if ((( los    & ( 1 << ix )) == ( 1 << ix )) &&
                        (( no_act & ( 1 << ix )) == ( 1 << ix )) &&
                        (( valid  & ( 1 << ix )) == ( 1 << ix ))) {
                    }
                    else {
                        rv = FAILED;
                    }
                }
            } /* if ( clock_mask & ( 1 << ix )) { */
        } /* for ( ix=0; ix<16; ix++ ) { */

        if ( rv == FAILED ) {
            /* Status not match as expect, wait 1 sec,
             * read again from reg. STATUS.IN#_MON_STATUS */
            msleep( 1000 );
            if ( idt8a3_input_clock_check_get_data( 0, &valid, &no_act, &los,
                    &s_valid, &s_no_act, &s_los ) != PASSED) {
                printf("ERROR:  %s:%d Failed on idt8a3_input_clock_check_get_data()\n",
                              __FUNCTION__, __LINE__);
                return FAILED;
            }
        }
        else {
            /* Status match as expect, break for polling loop */
            break;
        }
    } /* for ( jx=0; jx<time; jx++ ) { */
    printf("\n\r");

    printf(" IDT %s %s clock for input mask 0x%04X after %d seconds \n",
            ( rv == PASSED )?"detected":"not detected",
            ( mode == IDT8A3_CHECK_CLOCK_INVALID )?"invalid":"valid",
            clock_mask, jx+1 );


    /* If status is no good, not to test with duration. */
    if ( rv != PASSED ) {
        idt8a3_input_clock_print( valid, no_act, los );
        printf("\n\r");
        return rv;
    }

    if ( duration != 0 ) {

        rv = idt8a3_clear_all_sticky( );
        if (rv != PASSED) {
            printf("ERROR:  %s:%d Failed on idt8a3_clear_all_sticky() (rc = %#x)\n",
                          __FUNCTION__, __LINE__, rv);
            return FAILED;
        }

        printf(" Monitoring %s clock IN - ",( mode == IDT8A3_CHECK_CLOCK_INVALID )?"invalid":"valid");
        for ( ix=15; 0<=ix; ix-- ) {
            if ( clock_mask & ( 1 << ix )) {
                printf("%d ", ix );
            }
        }

        for ( jx=0; jx<duration; jx++ ) {

            printf("#" );
            fflush(stdout);

            rv = idt8a3_input_clock_check_get_data( 0, &valid, &no_act, &los, &s_valid, &s_no_act, &s_los);
            if (rv != PASSED) {
                printf("ERROR:  %s:%d Failed on idt8a3_input_clock_check_get_data() (rc = %#x)\n",
                          __FUNCTION__, __LINE__, rv);
                return FAILED;
            }

            for ( ix=0; ix<16; ix++ ) {
                if ( clock_mask & ( 1 << ix )) {
                    if (( s_valid ) || ( s_no_act ) || ( s_los )) {
                        rv_2 |= FAILED;
                        idt8a3_input_clock_monitor_print( valid, no_act, los, s_valid, s_no_act, s_los );
                        printf("\n\r");
                        cterr('f', 0, "IN#%d clock sticky status has changed!", ix );
                    }
                }
            }

            msleep( 1000 );
        }

        if ( rv_2 == PASSED) {
            printf("\n\r IDT no clock status changed for input mask 0x%04X for %d seconds\n\r",
                        clock_mask, duration );
            idt8a3_input_clock_print( valid, no_act, los );
        }

    } /* if ( duration != 0 ) { */

    return (rv | rv_2);
}

/**********************************************************************
 *
 * Function: idt8a3_input_clock_check_f
 *
 * Description:	utility entry point to check input clock of idt8a3xxxx.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_input_clock_check_f( void)
{
    int rv = PASSED;
    uint32_t idt8a3_clock_mask_param = 0x1;
    uint32_t idt8a3_mode_param = 0x0;
    uint32_t idt8a3_time_param = 5;
    uint32_t idt8a3_duration_param = 5;


    /* calling for menu driven */

    idt8a3_clock_mask_param = gethex_answer("Input clock mask (0x1-0xFFFF)",
                                             1, 1, 0xFFFF);
    idt8a3_mode_param = getdec_answer("Check for (valid/invalid, 0/1)", 0, 0, 1);
    idt8a3_time_param = getdec_answer("Status poll timeout", 5, 1, 300);  /* max time out 300s */
    idt8a3_duration_param = getdec_answer("Status check duration", 5, 1, 300);  /* max time out 300s */

    rv = idt8a3_input_clock_check(idt8a3_clock_mask_param, idt8a3_mode_param,
                                  idt8a3_time_param, idt8a3_duration_param);

    return rv;
}


/**********************************************************************
 *
 * Function: idt8a3_read_fw_version
 *
 * Description: utility to show syncE firmware version from register
 *              GENERAL_STATUS.MAJ_REL/MIN_REL/HOTFIX_REL. And
 *              configuration data version from register SCRATCH.SCRATCH0.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_read_fw_version( uint8_t *part_id, uint8_t *hw_rev_id,
                            uint8_t *fm_rel_ver, uint8_t *config_data_ver)
{
    int    rv = PASSED;
    uint32_t base, offset;
    uint32_t re_addr;
    uint8_t  ix;


    /*
     * Read syncE product id
     */

	/* read part number low byte */
    base   = idt_block_info[IDT_BLOCK_GENERAL_STATUS].base; /* 0xC014 */
    offset = base + 0x1E;
    for (ix=0; ix<2; ix++) {
        rv |= idt8a3_read( (offset+ix), part_id, &re_addr );
        part_id++;
    }

    /*
     * Read syncE HW revision id
     */
    base   = idt_block_info[IDT_BLOCK_HW_REVISION].base; /* 0x8180 */
    offset = base + 0x7A;
    rv |= idt8a3_read( offset, hw_rev_id, &re_addr );

    /*
     * Read syncE firmware release version
     */
    base   = idt_block_info[IDT_BLOCK_GENERAL_STATUS].base; /* 0xC014 */
    offset = base + 0x10;
    for (ix=0; ix<3; ix++) {
        rv |= idt8a3_read( (offset+ix), fm_rel_ver, &re_addr );
        fm_rel_ver++;
    }

    /*
     * Display syncE configuration data version
     */
    base   = idt_block_info[IDT_BLOCK_SCRATCH].base; /* 0xCF50 */
    offset = base + 0x0;
    for (ix=0; ix<4; ix++) {
        rv |= idt8a3_read( (offset+ix), config_data_ver, &re_addr );
        config_data_ver++;
    }

    return (rv);
}

int idt8a3_show_fw_version( void)
{
    int    rv = PASSED;
    uint8_t  hw_rev_id, part_id[2], fm_rel_ver[3];
    uint8_t  config_data_ver[4];
    uint8_t  data_maj, data_min, data_hotfix, product_rel;
    uint16_t *p_data_n;
    uint32_t *p_config_data_ver;
    char     hw_rev_id_in_char;


    hw_rev_id = 0;
    memset(part_id, 0, sizeof(part_id));
    memset(fm_rel_ver, 0, sizeof(fm_rel_ver));
    memset(config_data_ver, 0, sizeof(config_data_ver));

    /*
     * Read SyncE revision info
     */

    rv = idt8a3_read_fw_version(part_id, &hw_rev_id, fm_rel_ver, config_data_ver);
    if (rv != PASSED) {
        printf("\n\r %s(): SyncE read revision info fail!\n", __FUNCTION__);
        return (rv);
    }

    /*
     * Display syncE product id
     */
    p_data_n = (uint16_t *)&part_id[0];
    printf("\n\r SyncE Product ID: 0x%04X\n", *p_data_n);

    /*
     * Display syncE HW revision id
     */
    hw_rev_id_in_char = 0x40 + hw_rev_id;
    printf(" SyncE HW revision ID: %d (Rev%c)\n", hw_rev_id, hw_rev_id_in_char);

    /*
     * Display syncE firmware release version
     */
    /* read major release number */
    data_maj = fm_rel_ver[0];
    product_rel = data_maj & 0x01;
    data_min = fm_rel_ver[1];
    data_hotfix = fm_rel_ver[2];
    printf(" SyncE PLL firmware version: %d.%d.%d (%s release)\n",
            (data_maj >> 1), data_min, data_hotfix,
            (product_rel)? "product" : "development" );

    /*
     * Display syncE configuration data version
     */
    p_config_data_ver = (uint32_t *)&config_data_ver[0];
    printf(" SyncE PLL configuration version: %d\n", *p_config_data_ver);

    printf("\n");

    return (rv);
}


/**********************************************************************
 *
 * Function: idt8a3_show_fw_version_f
 *
 * Description: utility entry point to show syncE firmware version.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_show_fw_version_f( void)
{
    /* calling for menu driven */
    return ( idt8a3_show_fw_version() );
}

/*******************************************************************************
 *
 * Function   : idt8a3_sw_reset
 * Description: write 0x5a "initiate soft reset" to RESET_CTRL block.
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int idt8a3_sw_reset( void )
{
    uint32_t base,offset;
    uint8_t  data;
    uint32_t re_addr;

    base = idt8a3_instance_base( IDT_BLOCK_RESET_CTRL, 0 );
    offset = base + 0x12;
    data = IDT_BLOCK_RESET_CTRL_SOFT_RESET;
    if ( idt8a3_debug_flag )
    {
        printf("%s(): Write: offset=0x%X, data=0x%X\n",	__FUNCTION__, offset, data);
    }

    return idt8a3_write( offset, &data, &re_addr );
}

/**********************************************************************
 *
 * Function: idt8a3_sw_reset_f
 *
 * Description: test entry point to do idt8a3xxxx Soft Reset.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_sw_reset_f( void)
{
    /* calling for menu driven */
    return idt8a3_sw_reset();
}

/**********************************************************************
 *
 * Function: idt8a3_reg_test
 *
 * Description: Using idt8a3xxxx register SCRATCH.SCRATCH3 (0xCF5C/0xCF5D)
 *              to do register write/read/verify test.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_reg_test( void)
{
#define SAVE_DATA_BYTE 2
    int    rv = PASSED;
    uint32_t offset, offset_tmp, re_addr;
    uint8_t  w_data, r_data, ix, jx;
    uint8_t save_data[SAVE_DATA_BYTE];
    idt8a3_block_t list[]= {{ IDT_BLOCK_SCRATCH, 0x0C, "0" },
                            { IDT_BLOCK_SCRATCH, 0x0D, "1" },
                            { 0x00,          0x00, NULL }};

    printf("\n");

    /* Save original data from original test registers */
    ix = 0;
    while ( list[ix].name ) {
        offset = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
        if (ix >= SAVE_DATA_BYTE) {
            cterr('f', 0, "%s:%d save_data[] is not engough, ix=%d \n",
                     __FUNCTION__, __LINE__, ix);
            return ( FAILED );
        }
        rv |= idt8a3_read( offset, &save_data[ix], &re_addr );
        ix ++;
    }
    if (rv != PASSED) {
        cterr('f', 0, "%s:%d save original data from test register failed! \n",
                 __FUNCTION__, __LINE__);
        return ( FAILED );
    }

    /* Unique access */
    ix = 0;
    while ( list[ix].name ) {
        offset = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
        w_data = ( ix + 1 );
        rv |= idt8a3_write( offset, &w_data, &re_addr );
        ix ++;
    }

    ix = 0;

	offset_tmp = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
    while ( list[ix].name ) {
        offset = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
        w_data = ( ix + 1 );
        rv |= idt8a3_read( offset, &r_data, &re_addr );
        if ( r_data != w_data ) {
            cterr('f', 0, "%s:%d Unique register access test failed, "
                   "[0x%08X]=0x%02X exp=0x%02X\n",
                     __FUNCTION__, __LINE__, offset, r_data, w_data);
            return ( FAILED );
        }
        ix ++;
    }
    printf("Unique register 0x%08X (0x%08X) access test passed\n", offset_tmp, offset);

    /*  Walking 1 */
    ix = 0;
    offset_tmp = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
    while ( list[ix].name ) {
        for ( jx=0; jx<8; jx++ ) {
            offset = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
            w_data  = 1 << jx;
            rv |= idt8a3_write( offset, &w_data, &re_addr );
            rv |= idt8a3_read(  offset, &r_data, &re_addr );
            if ( r_data != w_data ) {
                cterr('f', 0, "%s:%d Walking 1 register test failed, "
                      "[0x%08X]=0x%02X exp=0x%02X\n",
                         __FUNCTION__, __LINE__, offset, r_data, w_data);
                return ( FAILED );
            }
        }
        ix ++;
    }
    printf("Walking 1 to register 0x%08X (0x%08X) test passed\n", offset_tmp, offset);

    /* Walking 0 */
    ix = 0;
	offset_tmp = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
    while ( list[ix].name ) {
        for ( jx=0; jx<8; jx++ ) {
            offset = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
            w_data  = ~(1 << jx);
            rv |= idt8a3_write( offset, &w_data, &re_addr );
            rv |= idt8a3_read(  offset, &r_data, &re_addr );
            if ( r_data != w_data ) {
                cterr('f', 0, "%s:%d Walking 0 register test failed, "
                        "[0x%08X]=0x%02X exp=0x%02X\n",
                         __FUNCTION__, __LINE__, offset, r_data, w_data);
                return ( FAILED );
            }
        }
        ix ++;
    }
    printf("Walking 0 to register 0x%08X (0x%08X) test passed\n", offset_tmp, offset);

    /* restore original data to test registers */
    ix = 0;
    while ( list[ix].name ) {
        offset = idt8a3_instance_base( list[ix].block_id, 0 ) + list[ix].base;
        if (ix >= SAVE_DATA_BYTE) {
            cterr('f', 0, "%s:%d save_data[] is not engough, ix=%d \n",
                     __FUNCTION__, __LINE__, ix);
            return ( FAILED );
        }
        rv |= idt8a3_write( offset, &save_data[ix], &re_addr );
        ix ++;
    }
    if (rv != PASSED) {
        cterr('f', 0, "%s:%d restore original data to test registers failed! \n",
                 __FUNCTION__, __LINE__);
        return ( FAILED );
    }

    return rv;
}

/**********************************************************************
 *
 * Function: idt8a3_reg_test_f
 *
 * Description: test entry point to do idt8a3xxxx register test.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_reg_test_f( void)
{
    int    rv = PASSED;
    char *tname = "SyncE Register";

    /* calling for menu driven */
    testname("%s", tname);
    prpass(testpass, "\n\r%s, ", tname);

    rv = idt8a3_reg_test();
    if (rv == PASSED) {
    	prpass(testpass, "\n%s test passed, ", tname);
    }

    return rv;
}


/**********************************************************************
 *
 * Function:	idt8a3_eeprom_dev_offset
 * Description:	convert EEPROM offset 'addr' to I2C address, store into 'offset'.
 * Inputs     : addr - eeprom offset to be converted,
 *              dev  -
 *              offset  -
 * Output     :	PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_eeprom_dev_offset( uint32_t addr, uint8_t *dev, uint32_t *offset )
{

    if ( IDT_EEPROM_MAX_SIZE <= addr ) {
        printf("ERROR:  %s:%d Invalid addr!  max=0x%x input=0x%x\n",
                  __FUNCTION__, __LINE__, IDT_EEPROM_MAX_SIZE, addr);
        return FAILED;
    }

    *dev    = (( addr >> 16 ) & 0x1 ) + MB_I2C_ADDR_SYNCE_EEPROM;
    *offset = ( addr & 0x0FFFF );

    if ( idt8a3_debug_flag ) {
    printf("%s(): addr=0x%x, dev=0x%X, offset=0x%x\n", __FUNCTION__,
                 addr, *dev, *offset);
    }

    return PASSED;
}


/**********************************************************************
 *
 * Function: idt8a3_eeprom_read
 * Description: Read 1-byte of data from dt8a3xxxx EEPROM by 'count'
 * Inputs     : addr - EEPROM offset (2-BYTE)
 *              count  - read number of count in unit of 1-byte.
 * Output     :	PASSED/FAILED
 *
 * Device Addressing:
 * Dev_Type_Id  HW_Slave_Addr_bits  MSB OF WORD Addr  R/W
 * B7 B6 B5 B4    B3  B2            B1                B0
 * 1  0  1  0     A2  A1            A16               R/W
 *
 * For Random Read/Write, two 8-bit word address bytes must be transmitted to
 * the device immediately following the device address byte. the word address
 * bytes consist of the remaining 16 bits of 17-bit memory array word address,
 * and are used to specify which byte location in the EEPROM to start reading or
 * writing. the first word address byte contains the next eight bits of the
 * word address (A15 through A8) in bit positions seven through zero.
 **********************************************************************
 */
int idt8a3_eeprom_read( uint32_t addr, uint32_t count)
{
    int rv = PASSED;
    uint32_t ix, offset;
    uint8_t data;
    uint8_t dev;
    uint32_t temp, align;

    /*  Make display 16 byte aligement */
    align = 16;

    if ( idt8a3_debug_flag ) {
        printf("%s(): addr=0x%x, count=%d\n", __FUNCTION__, addr, count);
    }

    temp = ( addr % align );
    addr = addr - temp;

    if ( idt8a3_debug_flag ) {
        printf("%s(): addr=0x%x\n", __FUNCTION__, addr);
    }

    for ( ix=addr; ix<(addr + count); ix++ ) {

        if ( idt8a3_eeprom_dev_offset( ix, &dev, &offset ) != PASSED ) {
            return FAILED;
        }

        rv = platform_idt8a3_eeprom_read( dev, offset, &data, 1 );
        if (rv != PASSED) {
            printf("ERROR:  %s:%d Failed to write to idt8a3xxxx eeprom (rc = %#x)\n",
                          __FUNCTION__, __LINE__, rv);
            return FAILED;
        }

        if (( ix % align ) == 0 ) {
            printf("\n\r 0x%02X [0x%08X]: %02X", dev, ix, data );
        }
        else {
            printf(" %02X", data );
        }
    }

    printf("\n\r");

    return rv;
}

/**********************************************************************
 *
 * Function: idt8a3_eeprom_read_f
 *
 * Description: utility entry point to read from dt8a3xxxx EEPROM.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_eeprom_read_f( void)
{
    uint32_t idt8a3_eeprom_offset_param = 0;
    uint32_t idt8a3_eeprom_count_param = 1;

    /* calling for menu driven */
    idt8a3_eeprom_offset_param = gethex_answer("EEPROM offset (0x00 - 0x0x1FFFF)",
                                               0, 0, IDT_EEPROM_MAX_SIZE-1);
    idt8a3_eeprom_count_param = getdec_answer("EEPROM read count",
                                               0, 0, IDT_EEPROM_MAX_SIZE);

    return idt8a3_eeprom_read(idt8a3_eeprom_offset_param, idt8a3_eeprom_count_param);
}

/**********************************************************************
 *
 * Function: idt8a3_eeprom_write
 * Description:	Write 1-byte of data to dt8a3xxxx EEPROM.
 * Inputs     : addr - EEPROM offset (2-BYTE)
 *              data - data to write
 * Output     :	PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_eeprom_write( uint32_t addr, uint8_t data )
{
    uint32_t offset;
    uint8_t dev;

    if ( idt8a3_eeprom_dev_offset( addr, &dev, &offset ) != PASSED ) {
        return FAILED;
    }

    return platform_idt8a3_eeprom_write( dev, offset, &data, 1 );
}

/**********************************************************************
 *
 * Function: idt8a3_eeprom_write_f
 *
 * Description: utility entry point to write to dt8a3xxxx EEPROM.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_eeprom_write_f( void)
{
    uint32_t idt8a3_eeprom_offset_param = 0;
    uint32_t idt8a3_eeprom_data_param = 1;

    /* calling for menu driven */
    idt8a3_eeprom_offset_param = gethex_answer("EEPROM offset (0x00 - 0x0x1FFFF)",
                                               0, 0, IDT_EEPROM_MAX_SIZE-1);
    idt8a3_eeprom_data_param = gethex_answer("EEPROM data", 0, 0, 0xFF);

    return idt8a3_eeprom_write(idt8a3_eeprom_offset_param, idt8a3_eeprom_data_param);
}

/**********************************************************************
 *
 * Function: idt8a3_eeprom_read_file_f
 *
 * Description: utility entry point to program EEPROM data from file.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_eeprom_read_file_f( void)
{
    uint8_t eeprom_buf[IDT_EEPROM_MAX_SIZE];

    /* Get the EERPOM data from file */
    if ( platform_get_idt8a3_eeprom_data (eeprom_buf)  != PASSED ) {
        printf("ERROR:  %s:%d Failed to get EEPROM data !\n)",
                      __FUNCTION__, __LINE__);
        return FAILED;
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: idt8a3_eeprom_prog
 * Description: Program EEPROM data to dt8a3xxxx EEPROM.
 * Inputs     : mode-  1: program only, 2: Verify only, 3: program/verify
 * Output     : PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_eeprom_prog( uint32_t mode )
{
#if defined IDT_EEPROM_PROG_SUPPORT
    int rv = PASSED;
    uint32_t ix, offset;
    uint32_t read_len=256, write_len=256;
    uint8_t dev;
    uint8_t eeprom_data_buf[IDT_EEPROM_MAX_SIZE];
    uint8_t eeprom_read_buf[IDT_EEPROM_MAX_SIZE];



    if ( idt8a3_debug_flag ) {
        printf("%s(): mode = 0x%x\n", __FUNCTION__, mode);
    }

    memset(eeprom_data_buf, 0, sizeof(eeprom_data_buf));
    memset(eeprom_data_buf, 0, sizeof(eeprom_data_buf));

    /* Get the EERPOM data from file */
    if ( platform_get_idt8a3_eeprom_data (eeprom_data_buf)  != PASSED ) {
        printf("%s:%d: ERROR:  Failed to get EEPROM data !\n)",
                      __FUNCTION__, __LINE__);
        return FAILED;
    }

    /*
     * Program EEPROM
     */
    if ( (mode & IDT_EEPROM_PROGRAM) == IDT_EEPROM_PROGRAM ) {

        printf("Programming (~20 seconds) ");
        fflush(stdout);
        ix = 0;
        while (ix < IDT_EEPROM_MAX_SIZE) {
            if ( idt8a3_eeprom_dev_offset( ix, &dev, &offset )  != PASSED ) {
                return FAILED;
            }

            if (( ix % 0x400 ) == 0 ) {
                printf(".");
                fflush(stdout);
            }

            rv = platform_idt8a3_eeprom_write( dev, offset, &eeprom_data_buf[ix], write_len );
            msleep( 4 );
            if ( rv != PASSED ) {
                printf("ERROR:  %s:%d Failed to write to idt8a3xxxx eeprom (offset = 0x%#x)\n",
                              __FUNCTION__, __LINE__, offset);
                return FAILED;
            }
            ix += write_len;

            if ( (IDT_EEPROM_MAX_SIZE - ix) < write_len ) {
                write_len = (IDT_EEPROM_MAX_SIZE - ix);
            }

        } /* while (ix < IDT_EEPROM_MAX_SIZE) { */

        printf("\n\r Programming EEPROM passed\n\r");
    }

    /*
     * Verity EEPROM
     */
    if ( (mode & IDT_EEPROM_VERIFY) == IDT_EEPROM_VERIFY ) {

        printf("\n\rVerifying ( ~15 seconds) \n");

        /* Read data from EEPROM to eeprom_read_buf[] */
        printf("  Read data from EEPROM ");
        fflush(stdout);
        ix = 0;
        while (ix < IDT_EEPROM_MAX_SIZE) {
            if ( idt8a3_eeprom_dev_offset( ix, &dev, &offset )  != PASSED ) {
                return FAILED;
            }

            if (( ix % 0x400 ) == 0 ) {
                printf(".");
                fflush(stdout);
            }

            rv = platform_idt8a3_eeprom_read( dev, offset, &eeprom_read_buf[ix], read_len);
            if (rv != PASSED) {
                printf("ERROR:  %s:%d Failed to read from idt8a3xxxx eeprom (offset = 0x%#x)\n",
                              __FUNCTION__, __LINE__, offset);
                return FAILED;
            }

            ix += read_len;

            if ( (IDT_EEPROM_MAX_SIZE - ix) < read_len ) {
                read_len = (IDT_EEPROM_MAX_SIZE - ix);
            }

        } /* while (ix < IDT_EEPROM_MAX_SIZE) { */

        /* Compare data read from EEPROM eeprom_read_buf[] with EEPROM data eeprom_data_buf[] */
        printf("  \n  Compare EEPROM data with EEPROM file \n");
        for ( ix=0; ix<IDT_EEPROM_MAX_SIZE; ix++ ) {
            if ( eeprom_read_buf[ix] != eeprom_data_buf[ix] ) {
                printf("ERROR: Data read=0x%02X expect=0x%02X at offset 0x%x\n",
                        eeprom_read_buf[ix], eeprom_data_buf[ix], ix );
                rv |= FAILED;
            }
        }

        if (rv != FAILED) {
            printf("\n\r Verifying EEPROM passed\n\r");
        }
        else {
            printf("\n\r Verifying EEPROM failed\n\r");
        }
    }


    /* show current RTC time */

    return rv;
#else
    printf("ERROR:  IDT EEPROM program not supported in this Diag image!!\n" );
    return FAILED;
#endif
}


/**********************************************************************
 *
 * Function: idt8a3_eeprom_prog_f
 *
 * Description: utility entry point to program EEPROM data to dt8a3xxxx EEPROM.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_eeprom_prog_f( void)
{
    uint32_t idt8a3_op_mask_param = 3;

    /* calling for menu driven */
    idt8a3_op_mask_param = gethex_answer("Op mask (program/verify, 1/2))",
                                          0x3, 0, 0x3);

    /* Program EEPROM */
    if ( idt8a3_op_mask_param & IDT_EEPROM_PROGRAM ) {
        if (getc_answer("\nAre you sure you want to program? (y/n)", "yn", 'n')
            == 'n') {
            printf("\nNo action taken.\n");
            return (PASSED);
        }
    }

    return idt8a3_eeprom_prog(idt8a3_op_mask_param);
}


/**********************************************************************
 *
 * Function: idt8a3_gen_int
 *
 * Description: idt8a2 GPIO_8 output is connected to FPGA interrupt (low active).
 *              Configure idt8a2 GPIO_8 as general GPIO output from register
 *              GPIO_0.GPIO_CTRL(0xC96A). Drive GPIO_8 low or high from register
 *              GPIO_USER_CONTROL.GPIO8_TO_15_OUT(0xC161).
 *              if 'enable' is 1, drive GPIO pin 8 low, otherwise, drive high.
 *
 * Input: enable - 1: enable interrupt, 0: disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_gen_int( int enable)
{
    int    rv = PASSED;
    uint32_t base, offset;
    uint32_t re_addr;
    uint8_t  data;


	/* drive GPIO pin 8 (bit 0) */
    /* get the base offset of GPIO_USER_CONTROL */
    base = idt8a3_instance_base( IDT_BLOCK_GPIO_USER_CONTROL, 0 );
    offset = base + 0x01; /* 0xC161 */
    data = 0;
    rv |= idt8a3_read( offset, &data, &re_addr );
    if (enable) {
        data &= ~0x01;  /* drive low */
    }
    else {
        data |= 0x01;	/* drive high */
    }
    if ( idt8a3_debug_flag )
    {
        printf("%s(): Write: offset=0x%X, data=0x%X\n",
                __FUNCTION__, (offset), data);
    }

    rv |= idt8a3_write( offset, &data, &re_addr );

    /* Configure idt8a2 GPIO_8 as general GPIO output */
    /* get the base offset of GPIO_8 */
    base = idt8a3_instance_base( IDT_BLOCK_GPIO_0, 8 );
    offset = base + 0x10;  /* 0xC96A */
    data = 0;
    rv |= idt8a3_read( offset, &data, &re_addr );
    data &= ~0x03;    /* Disable GPIO function mode, select CMOS. */
    data |= 0x04;     /* config GPIO as output */
    rv |= idt8a3_write( offset, &data, &re_addr );
    if ( idt8a3_debug_flag )
    {
        printf("%s(): Write: offset=0x%X, data=0x%X\n",
                __FUNCTION__, (offset), data);
    }

	/* GPIO output stay at this level for 5 us */
    usleep(5);

    return (rv);
}

/**********************************************************************
 *
 * Function: idt8a3_pll_intr_test
 *
 * Description:	Verify dt8a3xxxx generate interrupt to Fugazi FPGA by below sequence:
 *              - Clear SyncE interrupt
 *              - Check if FPGA interrupt bit is clear
 *              - SyncE generate interrupt
 *              - Check if FPGA interrupt bit is pending
 *              - Clear SyncE interrupt
 *              - Check if FPGA interrupt bit is clear
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_pll_intr_test (void)
{
    int    rv = PASSED;
    uint32_t rdval;

    /* Clear Interrupt first */
    printf("\nSyncE clear interrupt\n");
    rv = idt8a3_gen_int( DISABLE );
    if (rv != PASSED) {
        cterr('f',0,"SyncE clear interrupt failed!");
        return(FAILED);
    }

    /* check FPGA Interrupt Status Reg  */
    dash_fpga_reg_read(FPGA_CP_INTR_CTRL_REG_OFFSET, &rdval);
    if (rdval & 0x1 << 13) {
        cterr('f',0,"FPGA Unable to clear iterrupt!");
        return(FAILED);
    }
    else {
        printf("FPGA Interrupt Cleared!\n");
    }

    /* General PLL interrupt
     * C96a -> 0x35
     * C161 -> 0x0(Active LOW)*/
    printf("\nSyncE general interrupt\n");
    rv = idt8a3_gen_int( ENABLE );
    if (rv != PASSED) {
        cterr('f',0,"SyncE general interrupt failed!");
        return(FAILED);
    }

    /* SyncE PLL inerrupt Enable */
    dash_fpga_reg_write(0x104, 2000);

    /* check FPGA Interrupt Status Reg  */
    dash_fpga_reg_read(FPGA_CP_INTR_CTRL_REG_OFFSET, &rdval);
    if (rdval & 0x1 << 13) {
        printf("FPGA SyncE PLL Interrupt pending!!\n");
    }
    else {
        cterr('f',0,"FPGA No interrupt detected!(0x%X=0x%x)",
                FPGA_CP_INTR_CTRL_REG_OFFSET, rdval);
        return(FAILED);
    }

    /* Clear Interrupt */
    printf("\nSyncE clear interrupt\n");
    rv = idt8a3_gen_int( DISABLE );
    if (rv != PASSED) {
        cterr('f',0,"SyncE clear interrupt failed!");
        return(FAILED);
    }

    /* check FPGA Interrupt Status Reg  */
    dash_fpga_reg_read(FPGA_CP_INTR_CTRL_REG_OFFSET, &rdval);
    if (rdval & 0x1 << 13) {
        cterr('f',0,"FPGA Unable to clear iterrupt!");
        return(FAILED);
    }
    else {
        printf("FPGA Interrupt Cleared!\n");
    }

    return (rv);
}

/*-------------------------------------------------------------------
 *
 * Function: idt8a3_pll_intr_test_f
 *
 * invoke interrupt handler for SyncE PLL  interrupt.
 *
 * INPUT : NONE
 * OUTPUT: PASSED/FAILED
 *------------------------------------------------------------------
 */
static int idt8a3_pll_intr_test_f (void)
{
   int    rv = PASSED;
   char *tname = "SyncE PLL Interrupt";

   testname("%s", tname);
   prpass(testpass, "\n\r%s, ", tname);

   rv = idt8a3_pll_intr_test();
   if ( rv == PASSED ) {
       prpass(testpass, "\n%s test passed, ", tname);
   }

   return( rv );
}

/*-------------------------------------------------------------------
 *
 * Function: idt9a3_update_in_freq
 *
 * Update in_freq in IDT_INPUT_info[] table according to current frequency margin
 * normal, high, or low for SyncE recovered clock can work at frequency margin
 * (CSCvq86209).
 * Margin High: normal frequency + 100ppm of normal frequency
 * Margin Low:  normal frequency - 100ppm of normal frequency
 *
 * INPUT : in_freq_index - index to the IDT_INPUT_info[] table to be updated.
 *         freq_margin_status - 0: normal, 1: margin high, 2: margin low
 * OUTPUT: PASSED/FAILED
 *------------------------------------------------------------------
 */
static int idt9a3_update_in_freq(uint8_t in_freq_index, int freq_margin_status)
{
    int rv = PASSED;
    float    freq_margin;
    long long  freq_current_value, freq_update_value;

    if (in_freq_index >= FUGAZI_MAX_PHY_PORT_CLK_TEST ) {
        cterr('f',0,"in_freq_index is incorrect &d !\n", in_freq_index);
        return FAILED;
    }

    freq_margin = (float)SYNCE_FREQUCY_100PPM / SYNCE_FREQUCY_PPM;

    if (in_freq_index < 4 ) {
        /* update IDT_INPUT_info[] for BCM54194 1G input frequency to monitor */
        freq_current_value = SYNCE_FREQUCY_25MHZ;
        freq_update_value = freq_current_value;   /* for FREQ_MARGIN_NORMAL */
        if (freq_margin_status == FREQ_MARGIN_HIGH) {
            freq_update_value = freq_current_value + (SYNCE_FREQUCY_25MHZ * freq_margin);
        }
        else if (freq_margin_status == FREQ_MARGIN_LOW) {
            freq_update_value = freq_current_value - (SYNCE_FREQUCY_25MHZ * freq_margin);
        }
    }
    else {
        /* update IDT_INPUT_info[] for BCM82757 10G input frequency to monitor */
        freq_current_value = SYNCE_FREQUCY_156p25MHZ;
        freq_update_value = freq_current_value;   /* for FREQ_MARGIN_NORMAL */
        if (freq_margin_status == FREQ_MARGIN_HIGH) {
            freq_update_value = freq_current_value + (SYNCE_FREQUCY_156p25MHZ * freq_margin);
        }
        else if (freq_margin_status == FREQ_MARGIN_LOW) {
            freq_update_value = freq_current_value - (SYNCE_FREQUCY_156p25MHZ * freq_margin);
        }
    }

    IDT_INPUT_info[in_freq_index].in_freq = freq_update_value;

    if ( idt8a3_debug_flag )
    {
        printf("%s(): freq_margin=%f\n", __FUNCTION__, freq_margin);
        printf("%s(): freq_current_value=%f (0x%X)\n", __FUNCTION__,
                (float)freq_current_value, (int)freq_current_value);
        printf("%s(): freq_update_value=%f (0x%X)\n", __FUNCTION__,
                (float)freq_update_value, (int)freq_update_value);
        printf("%s(): IDT_INPUT_info[%d].in_freq=%d (0x%X)\n", __FUNCTION__,
            in_freq_index, (int)IDT_INPUT_info[in_freq_index].in_freq, (int)IDT_INPUT_info[in_freq_index].in_freq);
    }

    return (rv);
}

/*-------------------------------------------------------------------
 *
 * Function: idt9a3_in_freq_init
 *
 * Get current frequency margine status, high, low, or normal, then call
 * idt9a3_update_in_freq() to Update in_freq in IDT_INPUT_info[] table according
 * to current frequency margin normal, high, or low for SyncE recovered clock
 * can work at frequency margin. (CSCvq86209).
 *
 * INPUT : NONE
 * OUTPUT: PASSED/FAILED
 *------------------------------------------------------------------
 */
static int idt9a3_in_freq_init( void )
{
    int rv = PASSED;
    uint8_t  dpll_index, in_freq_index;
    unsigned long long freq_normal_value;
    unsigned long long freq_current_value_hz;

    /* Read from DPLL_0, DPLL_1, DPLL_2, DPLL_3 */
    for (dpll_index=FUGAZI_SYNCE_DPLL_CTRL_0;
         dpll_index<MAX_FUGAZI_SYNCE_DPLL_CTRL; dpll_index++) {
        freq_current_value_hz = 0;
        /* Get current SyncE PLL frequency from register */
        rv |= idt8a3_get_freq_margin( dpll_index, &freq_current_value_hz );
        if ( rv != PASSED ) {
            cterr('f',0,"idt8a3_get_freq_margin() failed at DLL %d !\n", dpll_index);
            return (rv);
        }

        if (dpll_index == FUGAZI_SYNCE_DPLL_CTRL_2) {
            /* 625MHz output */
            freq_normal_value = SYNCE_FREQUCY_625MHZ;
        }
        else {
            /* 500MHz output */
            freq_normal_value = SYNCE_FREQUCY_500MHZ;
        }

        if (freq_current_value_hz > freq_normal_value) {
            idt8a3_freq_status = FREQ_MARGIN_HIGH;
        }
        else if (freq_current_value_hz < freq_normal_value) {
            idt8a3_freq_status = FREQ_MARGIN_LOW;
        }
        else {
            idt8a3_freq_status = FREQ_MARGIN_NORMAL;
        }

        /* update IDT_INPUT_info[] table for current frequency */
        in_freq_index = dpll_index * 2;
        rv |= idt9a3_update_in_freq( (in_freq_index), idt8a3_freq_status );
        rv |= idt9a3_update_in_freq( (in_freq_index+1), idt8a3_freq_status );
        if ( rv != PASSED ) {
            cterr('f',0,"idt9a3_update_in_freq() failed at DLL %d, in_freq_index %d !\n",
                    dpll_index, in_freq_index);
            return (rv);
        }

    } /* for (dpll_index=0; dpll_index<4; dpll_index++) { */

    return rv;
}

/**********************************************************************
 *
 * Function:	idt8a3_recovered_clock_test
 *
 * Description:
 *  To test if SyncE received recover clock from BCM54194 1G PHY and BCM82757
 *  10G PHY output by three phases:
 *  1. negative test by disable PHY recovered clock output, verify SyncE LOS,
 *     NO_ACTIVITY, FREQ_OFFS_LIM bits are set, and sticky bit not change.
 *  2. positive test by enable PHY recovered clock output, verify SyncE LOS,
 *     NO_ACTIVITY, FREQ_OFFS_LIM bits are NOT set, and sticky bit not change.
 *  3. negative test by disable PHY recovered clock output, verify SyncE LOS,
 *     NO_ACTIVITY, FREQ_OFFS_LIM bits are set, and sticky bit not change.
 *  Above those bits is get from reg. STATUS.IN#_MON_STATUS.
 *
 *  The expect input clcok frequency is already pre-program in EEPROM,
 *  when power on, these default value, same as table IDT_INPUT_info[] will be
 *  load to register INPUT_#.IN_FREQ/IN_DIV/IN_MODE.
 *  Clock input is per PHY from 1G PHY, per Port from 10G PHY.
 *
 *  If frequency is been margined, then diag will re-configure register
 *  INPUT_#.IN_FREQ/IN_DIV/IN_MODE from IDT_INPUT_info[] with +/- margin value.
 *
 *  Since recover clock source is from PHY's Rx clock, this test requires
 *  SFP and loopback (network side link up) for PHY to have Rx clock.
 *
 * Input:	n/a.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_recovered_clock_test( void)
{
    int    rv = PASSED;
    int    phy_clk_index, ix;
    int    port_10G;
    int    clk_enable, clock_locked;
    int    clk_poll_time, clk_duration;
    uint32_t clock_mask=0;
    rcvd_clock_test_t phy_rcvd_clk_test[] = {
                { FALSE, IDT8A3_CHECK_CLOCK_INVALID },
                { TRUE,  IDT8A3_CHECK_CLOCK_VALID },
                { FALSE, IDT8A3_CHECK_CLOCK_INVALID }
    };

    uint32_t rcvd_clock_test_counts;
    uint8_t  hw_rev_id, part_id[2], fm_rel_ver[3];
    uint8_t  config_data_ver[4];
    uint32_t *p_config_data_ver;
    int phy_port, eth_num;
    unsigned int sys_link;
    unsigned int line_link;


    printf("\nPlease plug-in SFP and loopback for this test\n");
    fflush(stdout);
    msleep( 1000 );
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip SyncE Network Recovered Clock test\n");
        return (FAILED);
    }

    rcvd_clock_test_counts = sizeof(phy_rcvd_clk_test) / sizeof(rcvd_clock_test_t);

    /* update IDT_INPUT_info[] table for current frequency */
    rv = idt9a3_in_freq_init();
    if ( rv != PASSED ) {
         cterr('f', 0, "%s(): Get SyncE frequency margin status failed\n", __FUNCTION__);
         return( rv );
    }

    /* Read SyncE revision info */
    hw_rev_id = 0;
    memset(part_id, 0, sizeof(part_id));
    memset(fm_rel_ver, 0, sizeof(fm_rel_ver));
    memset(config_data_ver, 0, sizeof(config_data_ver));
    rv = idt8a3_read_fw_version(part_id, &hw_rev_id, fm_rel_ver, config_data_ver);
    p_config_data_ver = (uint32_t *)&config_data_ver[0];
    if (rv != PASSED) {
        cterr('f', 0, "%s(): SyncE read revision info fail!\n", __FUNCTION__);
        return (rv);
    }


    /*
     * Set SyncE clock input frequency, and initialize PHYs, etc.
     */
    for (phy_clk_index=0; phy_clk_index<FUGAZI_MAX_PHY_PORT_CLK_TEST; phy_clk_index++) {

        if (
            (idt8a3_freq_status == FREQ_MARGIN_HIGH) ||
            (idt8a3_freq_status == FREQ_MARGIN_LOW) ||
            ((idt8a3_freq_status == FREQ_MARGIN_NORMAL) &&
            (*p_config_data_ver < CONFIG_DATA_VER_4))
           )
        {
            /* config reg INPUT/REF_MON if config data ver is less than 4 at normal frequency */

            /* Configure IDT8A3 register INPUT_# */
            rv |= idt8a3_input_clock_rate( phy_clk_index );
            if ( rv != PASSED ) {
                cterr('f',0,"%s(): config idt8a3_input_clock_rate failed at "
                        "phy_clk_index (%d) \n",
                        __FUNCTION__, phy_clk_index);
            }

            /* Configure IDT8A3 register REF_MON_# */
            rv |= idt8a3_input_ref_monitor_cfg( phy_clk_index );
            if ( rv != PASSED ) {
                cterr('f',0,"%s(): config idt8a3_input_ref_monitor_cfg failed at "
                        "phy_clk_index (%d) \n",
                        __FUNCTION__, phy_clk_index);
            }
        }

        if (phy_clk_index == FUGAZI_LAST_1G_PHY_PORT) {
            /* Initialize bcm54194 1G PHY */
            bcm54194_init_script();
        }
        else if (phy_clk_index >= FUGAZI_1ST_10G_PHY_PORT) {

            /* configure BCM82757 10G PHY */
            port_10G = (phy_clk_index-FUGAZI_1ST_10G_PHY_PORT);

            /* Initialize bcm827575 10G PHY */
            rv |= bcm82757_PHY_init(port_10G, FUGAZI_PORT_SPEED_10G);
            if ( rv != PASSED ) {
                cterr('f',0,"%s(): bcm82757_PHY_init() failed at "
                        "PHY port (%d), speed (%d) \n",
                    __FUNCTION__, port_10G, FUGAZI_PORT_SPEED_10G);
            }

        } /* if (phy_clk_index > FUGAZI_LAST_1G_PHY_PORT) { */

    }  /* for (phy_clk_index=0; phy_clk_index<FUGAZI_MAX_PHY_PORT_CLK_TEST; phy_clk_index++) { */

    /*
     * Check if PHYs Link is up
     */
    for (phy_clk_index=0; phy_clk_index<FUGAZI_MAX_PHY_PORT_CLK_TEST; phy_clk_index++) {
        if (phy_clk_index <= FUGAZI_LAST_1G_PHY_PORT) {
            /* Checking bcm54194 1G PHY link status */
            for (phy_port = (phy_clk_index*2); phy_port < ((phy_clk_index*2)+2); phy_port++) {
                /* two SFP port per PHY (phy_clk_index) */
                eth_num = eth_qlm5_sfp_list[phy_port];
                printf("\nChecking bcm54194 1G PHY link status at port %d ", eth_num);
                if (!fugazi_is_1g_phy_linkup(eth_num)) {
                    cterr('f',0,"bcm54194 1G PHY link down at port %d\n", eth_num);
                }
            }
        }
        else {
            /* Checking bcm82757 10G PHY link status, port_10G is per SFP port */
            port_10G = (phy_clk_index-FUGAZI_1ST_10G_PHY_PORT);
            sys_link = 0;  /* link down */
            line_link = 0; /* link down */
            printf("\nChecking bcm82757 10G PHY link status at port %d ", port_10G);
            if (fugazi_bcm82757_check_link_stable(port_10G, FUGAZI_IF_SIDE_LINE,
                                                   &sys_link, &line_link) ) {
                cterr('f',0,"%s(): Checking bcm82757 10G PHY link status fail at port %d\n",
                        __FUNCTION__, port_10G);
            }
            if ( line_link ) {
                printf("!!! Network link up !!!\n");
            }
            else {
                cterr('f',0,"%s(): bcm82757 10G PHY Network link down at port %d\n",
                        __FUNCTION__, port_10G);
            }
        }
    }  /* for (phy_clk_index=0; phy_clk_index<FUGAZI_MAX_PHY_PORT_CLK_TEST; phy_clk_index++) { */

    /*
     * Recovered Clock lock/unlock test
     */

    for (ix=0; ix < rcvd_clock_test_counts; ix++) {
        rv = PASSED;
        clk_enable = phy_rcvd_clk_test[ix].clock_en;
        clock_locked = phy_rcvd_clk_test[ix].clock_locked;

        /* Clear all sticky status bits */
        rv |= idt8a3_clear_all_sticky( );
        if ( rv != PASSED ) {
            cterr('f',0,"%s(): idt8a3_clear_all_sticky() failed at "
                    "phy_clk_index (%d), clk_enable (%d) \n",
                    __FUNCTION__, phy_clk_index, clk_enable);
        }

        /* Set PHY's output clock */
        clock_mask = 0;
        for (phy_clk_index=0; phy_clk_index<FUGAZI_MAX_PHY_PORT_CLK_TEST; phy_clk_index++) {
            if (phy_clk_index <= FUGAZI_LAST_1G_PHY_PORT) {
                /* configure bcm54194 1G PHY's recovered clock output */
                rv |= bcm54194_recover_clock(phy_clk_index, clk_enable);
                if ( rv != PASSED ) {
                    cterr('f',0,"%s(): config bcm54194_recover_clock() failed at "
                            "phy_clk_index (%d), clk_enable (%d) \n",
                            __FUNCTION__, phy_clk_index, clk_enable);
                }
            }
            else {
                port_10G = (phy_clk_index-FUGAZI_1ST_10G_PHY_PORT);

                /* configure bcm82757 10G PHY's recovered clock output */
                rv |= bcm82757_recover_clock(port_10G, clk_enable);
                if ( rv != PASSED ) {
                    cterr('f',0,"%s(): config bcm82757_recover_clock() failed at "
                            "phy_clk_index (%d), clk_enable (%d) \n",
                            __FUNCTION__, port_10G, clk_enable);
                }
            }

            /* Check Input Clock to IDT8a3 */
            clock_mask |= IDT_INPUT_info[phy_clk_index].clk_status_mask;

        }  /* for (phy_clk_index=0; phy_clk_index<FUGAZI_MAX_PHY_PORT_CLK_TEST; phy_clk_index++) { */

        /* Checking SyncE input clock valitation */
        if (clock_locked == IDT8A3_CHECK_CLOCK_VALID) {
            /* check valid clock */
            clk_poll_time = IDT8A3_CHECK_VALID_POLL_TIME;  /* polling max 60sec */
            clk_duration  = IDT8A3_CHECK_VALID_DURATION_TIME;  /* for 10sec */
        }
        else {
            /* check invalid clock */
            clk_poll_time = IDT8A3_CHECK_INVALID_POLL_TIME;  /* polling max 20sec */
            clk_duration  = IDT8A3_CHECK_INVALID_DURATION_TIME;  /* for 2sec */
        }
        rv |= idt8a3_input_clock_check(clock_mask, clock_locked,
                                clk_poll_time, clk_duration);
        if ( rv != PASSED ) {
            /* Get live status of alarms and events from reg. STATUS.IN#_MON_STATUS */
            printf("\n\rDump DPLL Status:\n\r");
            rv |= idt8a3_status_dump();
            cterr('f',0,"Input clock check failed at idt8a3 reg. "
                        "STATUS.IN#_MON_STATUS for %s! \n",
                        (clock_locked)? "unlocked" : "locked");
            return ( rv );
        }

    } /* for (ix=0; ix < rcvd_clock_test_counts; ix++) { */

    return (rv);
}


/*-------------------------------------------------------------------
 *
 * Function: idt8a3_recovered_clock_test_f
 *
 * Description:	test entry point to do PHY/idt8a3xxxx recovered clock test.
 *
 * INPUT : NONE
 * OUTPUT: PASSED/FAILED
 *------------------------------------------------------------------
 */
static int idt8a3_recovered_clock_test_f (void)
{
    int    rv = PASSED;
    char *tname = "SyncE Network Recovered Clock";

    testname("%s", tname);
    prpass(testpass, "\n\r%s, ", tname);
    /* load 10G firmware if need it */
    if ((fugazi_bcm82757_init(fugazi_struct))) {
        cterr('f', 0, "fugazi_bcm82757_init failed");
        return (FAILED);
    }
    rv = idt8a3_recovered_clock_test();
    if (rv == PASSED) {
        prpass(testpass, "\n%s test passed, ", tname);
    }

    /* Emphasis compliance setting */
    bcm82752_emphasis_setting();

    return( rv );
}



/*-------------------------------------------------------------------
 *
 * Function: idt8a3_phy_clk_f
 *
 * Description: utility entry point to configure PHY's recovered clock output
 *
 * INPUT : NONE
 * OUTPUT: PASSED/FAILED
 *------------------------------------------------------------------
 */
static int idt8a3_phy_clk_f (void)
{
    int  rv = PASSED;
    uint32_t phy_speed_param = 0;
    int phy_param = 0;
    int phy_recovered_clk_param = 0;
    int phy_start, phy_end, phy_index;


    phy_speed_param = getdec_answer("1G/10G PHY (0/1)", 0, 0, 1);
    phy_param = gethex_answer("Enter phy num (1G PHY:0 ~ 3; 10G PHY Port: 0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    phy_recovered_clk_param = getdec_answer("Enable recovered clock output?(1:enable; 0:disable)", 1, 0, 1);


    /* BCM54195 1G PHY, total 4 PHYs, config 1G PHY by per PHY */
    if ( phy_speed_param == 0 ) {
        if ( phy_param == 0xff) {
            phy_start = 0;
            phy_end = MAX_FUGAZI_1G_PHY;
        }
        else {
            phy_start = phy_param;
            phy_end = phy_param + 1;
        }

        /* Initialize bcm54194 1G PHY */
        bcm54194_init_script();

        /* configure bcm54194 1G PHY's recovered clock output */
        for (phy_index=phy_start; phy_index<phy_end; phy_index++) {
            rv |= bcm54194_recover_clock(phy_index, phy_recovered_clk_param);
        }
    }
    /* BCM82757 10G PHY, total 2 PHYs, 2 PHY ports per PHY, config 10G PHY by per PHY port */
    else {
        /* load 10G firmware if need it */
        if ((fugazi_bcm82757_init(fugazi_struct))) {
            cterr('f', 0, "fugazi_bcm82757_init failed");
            return (FAILED);
        }
        if ( phy_param == 0xff) {
            phy_start = 0;
            phy_end = MAX_NR_FUGAZI_LANE;
        }
        else {
            phy_start = phy_param;
            phy_end = phy_param + 1;
        }

        for (phy_index=phy_start; phy_index<phy_end; phy_index++) {
            /* Initialize bcm827575 10G PHY */
            rv |= bcm82757_PHY_init(phy_index, FUGAZI_PORT_SPEED_10G);

            /* configure bcm82757 10G PHY's recovered clock output */
            rv |= bcm82757_recover_clock(phy_index, phy_recovered_clk_param);
        }
   }

   return(rv);
}

static int idt8a3_clear_all_sticky_f( void)
{
    int rv = PASSED;

    /* calling for menu driven */
    rv |= idt8a3_clear_all_sticky();

    return rv;
}

/**********************************************************************
 *
 * Function: idt8a3_check_eeprom_config_status
 *
 * Description:	Checking syncE status of configuration loaded from EEPROM
 *              from register GENERAL_STATUS.EEPROM_CONFIG_STATUS.
 *              Success if register value is 0, Fail if is not 0.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_check_eeprom_config_status( void )
{
    int rv = PASSED;
    uint32_t base, offset, addr;
    uint8_t  data;
    char *status_name[12] = {
    /* 0x0 */ "sucess",
    /* 0x1 */ "not found",
    /* 0x2 */ "in complete",
    /* 0x3 */ "wroong offset",
    /* 0x4 */ "wroong length",
    /* 0x5 */ "SCSR out of range",
    /* 0x6 */ "CRC error",
    /* 0x7 */ "Unknow error",
    /* 0x8 */ "Unknow error",
    /* 0x9 */ "Unknow error",
    /* 0xA */ "corrupt header",
    /* 0xB */ "EEPROM out of range",
    };


    base   =  idt_block_info[IDT_BLOCK_GENERAL_STATUS].base; /* 0xC014 */
    offset = base + 0x26;
    rv |= idt8a3_read( offset, &data, &addr );

    if ( data != 0 ) {
        /* loaded configuration from EEPROM not success*/
        printf("\nSyncE configuration loaded from EEPROM - %s ! (0x%X = 0x%X)\n",
                (data < 12)? status_name[data] : "unknown", offset, data);
        rv |= FAILED;
    }
    return (rv);
}

/**********************************************************************
 *
 * Function: idt8a3_check_dpll_lock_status
 *
 * Description: Checking syncE system DPLL lock status
 *              from register STATUS.DPLL_SYS_STATUS (0xC05C), bit[3:0].
 *              Locked if value is 3, otherwise not locked.
 *
 * Input: n/a.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int idt8a3_check_dpll_lock_status( void )
{
    int rv = PASSED;
    uint32_t base, offset, addr;
    uint8_t  data, mask;
    char * msg[] = { "freerun",
         "lockacq",
         "lockrec",
         "locked",
         "holdover",
         "open loop" };


    base   =  idt_block_info[IDT_BLOCK_STATUS].base; /* 0xC03C */
    offset = 0x20;
    rv |= idt8a3_read( base + offset, &data, &addr );
    mask = data & 0xF;

    if ( mask != 3 ) {
        /* SyncE system PLL is not locked */
        printf("\nsystem PLL is not locked - %s ! (0x%X = 0x%X)\n",
                (mask < 6)? msg[mask] : "unknown", (base +  offset), data);
        rv |= FAILED;
    }
    return (rv);
}


/*-------------------------------------------------
 * $Log: platform_synce_pll_utils.c,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2021/05/17 23:54:25  pdoong
 * correct typo from bcm4194 to bcm54194
 *
 * Revision 1.1.8.3  2021/04/29 01:44:36  pdoong
 * Add checking if PHY Network side link is up in 'SyncE Recovered Clock Test'
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.31  2020/08/24 00:04:24  pdoong
 * Clean code for ER.
 *
 * Revision 1.1.6.30  2020/07/25 00:45:57  pdoong
 * in DPLL Status dump, correct math error when display ppm offset.
 *
 * Revision 1.1.6.29  2020/07/17 04:49:46  pdoong
 * Code clean.
 *
 * Revision 1.1.6.28  2020/06/06 01:31:42  pdoong
 * enhance SyncE Recovered clock test to print out the state of the sticky and live bits when read reg data is unexpected.
 *
 * Revision 1.1.6.27  2020/02/13 00:46:32  pdoong
 * fix display incorrect FFO unit (ppb/ppm) in DPLL Status dump utility
 *
 * Revision 1.1.6.26  2020/02/05 01:37:03  pdoong
 * Clean the code.
 *
 * Revision 1.1.6.25  2020/02/04 23:11:08  pdoong
 * Add display ~'input clock FFO' for 'i: DPLL Status dump' in SyncE PLL Utility Menu
 *
 * Revision 1.1.6.24  2020/01/17 06:30:26  iachang
 * Skip BCM82757 initial with SyncE and BCM57412 submenu and add in SyncE Recovered Clock Test.
 *
 * Revision 1.1.6.23  2020/01/15 07:30:09  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.22  2020/01/07 00:04:48  pdoong
 * Add Checking syncE system DPLL lock status at begin of PHY initialization.
 *
 * Revision 1.1.6.21  2019/11/22 02:44:11  pdoong
 * removed debug code.
 *
 * Revision 1.1.6.20  2019/11/22 01:22:57  pdoong
 * Add Checking syncE status of configuration loaded from EEPROM.
 *
 * Revision 1.1.6.19  2019/10/29 18:58:43  pdoong
 * check SyncE configuration data version, configure SyncE register Input Clock and Reference Monitor only if lower than version 4.
 *
 * Revision 1.1.6.18  2019/10/28 22:30:36  pdoong
 * Reducecd program syncE fm from 1.5mins to 35 sec by increase read/write EEPROM from 16 to 256-byte
 *
 * Revision 1.1.6.17  2019/10/16 06:12:31  letsai
 * Modify file name
 *
 * Revision 1.1.6.16  2019/08/20 22:01:58  pdoong
 * fixed CSCvq86209 for SyncE Recovered Clock test works under frequency margin high/low.
 *
 * Revision 1.1.6.15  2019/08/09 19:46:35  pdoong
 * Workaround for CSCvq86209: SyncE Recovered clock test failed on frequency margin high&low
 *
 * Revision 1.1.6.14  2019/08/03 00:24:51  pdoong
 * Fixed CSCvq77121 - configure IDT8A35004 GPIO_8 to incorrect register offset
 *
 * Revision 1.1.6.13  2019/07/18 22:48:02  pdoong
 * (1). Change test SyncE recovered clock only if ext SFP loopback is plug-in. (2). Change Test SyncE register from SCRTCH0 to SCRACG3. (3). Add display SyncE config version.
 *
 * Revision 1.1.6.12  2019/06/15 00:03:40  pdoong
 * Add SyncE PLL recovered clock test for BCM82757 10G PHY
 *
 * Revision 1.1.6.11  2019/06/12 01:43:24  pdoong
 * skip SyncE recovered clock test for now. HW sill working on it
 *
 * Revision 1.1.6.10  2019/05/21 23:22:10  pdoong
 * Added SyncE recovered clock test from bcm54194 1G PHY output clock
 *
 * Revision 1.1.6.9  2019/05/10 23:31:30  pdoong
 * added display current PLL clock frequency utility
 *
 * Revision 1.1.6.8  2019/05/03 23:27:30  pdoong
 * added pll frequency margin utility
 *
 * Revision 1.1.6.7  2019/04/25 01:19:40  letsai
 * 1. Remove UART utility
 * 2. Modify SyncE PLL Interrupt test
 * 3. Modify Margin utility
 *
 * Revision 1.1.6.6  2019/04/24 21:50:26  pdoong
 * modify syncE interrupt test step by clear, enable, clear interrupt
 *
 * Revision 1.1.6.5  2019/04/24 02:16:50  pdoong
 * added syncE generate interrupt & show firmware version
 *
 * Revision 1.1.6.4  2019/04/23 22:19:18  letsai
 * Add SyncE PLL Interrupt Test
 *
 * Revision 1.1.6.3  2019/04/22 22:47:34  pdoong
 * Add syncE register test and EEPROM program utility
 *
 * Revision 1.1.4.4  2019/04/04 22:54:06  pdoong
 * SyncE utility access to external AT24CM01 EEPROM work
 *
 * Revision 1.1.4.3  2019/03/28 01:59:26  pdoong
 * SyncE idt8a3 register test bring-up passed
 *
 * Revision 1.1.4.2  2019/03/14 01:31:49  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 */
