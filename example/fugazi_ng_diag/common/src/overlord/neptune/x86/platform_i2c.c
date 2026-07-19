/* $Id: platform_i2c.c,v 1.3 2018/05/22 02:31:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Overlord I2C utility menu. P40t is header for SM1 and P38t is header
 * for SM2
 *
 * Sept. 2007, Simon Yen
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca9545a.h"
#include "n2g_api_rc.h"
#include "queryflags.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "platform_env.h"
#include "platform_i2c_usb.h"
#include "platform_psu.h"
#include "platform_poe_psu.h"
#include "platform_30w_poe.h"
#include "platform_pwr_seq.h"
#include "proto.h"
#include "platform_pcie_clk.h"
#include "platform_sensor.h"
#include "platform_vtg_mntr.h"
#include "platform_psu.h"
#include "byteswap.h"
#include "goofy_i2c.h"
#include "dash_fpga.h"
#include "i2c_address.h"
#include "platform_i2c.h"
#include "platform_slot.h"
#include "mb_tests.h"
#include "platform_pci.h"
#include "platform_temp_sensor.h"

/*
 *  Externs
 */
extern jmp_buf *monjmpptr;
extern int get_i2c_fd(int);
extern boolean is_overlord(void);

/* for utilities submenu. */
extern void build_pcie_clk_test_menu(void);
extern int show_dimm(int dimm_no);
extern int alter_dimm(int dimm_no);
extern int show_ich_i2c(void);
extern uint32_t get_platform_memsize(void);
extern int build_sm_pclk_menu(void);
extern void build_pwr_seq_menu(int);
extern int build_ts_menu(int);
extern int rtc_init(int);
extern int build_sfp_cookie_menu(int);
extern int build_eeprom_menu(int);
extern int build_i2c_usb_menu(int);
extern int build_mux_menu(uint32_t);
extern void build_poe_psu_menu(uint32_t);
extern void build_psu_menu(uint32_t);
extern void build_vtg_mntr_menu(int);
extern int build_30w_poe_menu(int);
extern void unreset_platform_in_dev(int);
extern int show_barometer_info(void);
extern int display_barometer_reg(void);
extern boolean has_poe_psu(uint32_t);
extern int ovld_pcie_clk_i2c_scan_test(char *);
extern int ovld_sys_clk_i2c_scan_test(char *);
extern int force_skip_30wpoe(void);
extern uint32_t ovld_check_poe_psu_wrap(void);
extern void build_sys_clk_menu(void); 
extern int build_snsr_menu(void);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t, uint32_t);
extern void build_vtg_rgltr_menu(void);

/*
 * Functional prototype
 */
boolean has_dimm1(void);

int show_temp(int, int);
extern int show_margins(int);

boolean has_midplane(void);
boolean has_ps1(void);
boolean has_ps2(void);
boolean has_sfp1(void);
boolean has_sfp2(void);

static int fpga_i2c_scan_addr(int);
static int write_i2c(int);
static int read_i2c(int);
static int platform_i2c_debug(int);

int ovld_dimm_i2c_scan_test(char *, uint32_t);
int reset_i2c_controller(int);
int display_mem_info(void);
extern uint32_t get_ngio_pcie_bus_num(void);


/*
 *  Globals  
 */
unsigned char i2c_debug = 0;

static int i2c_fd = -1;

static n2g_i2c_if_t fpga_i2c_dev[] = {
    {
        .dev_name = "ACT2",
        .offset = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_ACT2,
        .i2c_ctrl = I2C_CTRL_ZERO,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
    {
        .dev_name = "PCIe CLK Buf",
        .offset = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_CLK_BUFFER,
        .i2c_ctrl = I2C_CTRL_FIVE,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
    {
        .dev_name = "Clock Generator",
        .offset = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_CLK_GENERATOR,
        .i2c_ctrl = I2C_CTRL_FIVE,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
    {
        .dev_name = "USB Console",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB_CONSOLE,
        .i2c_ctrl = I2C_CTRL_ONE,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "USB Console Fw",
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB_CONSOLE_FW_DL,
        .i2c_ctrl = I2C_CTRL_ONE,
        .offset = -1,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Power Sequencer",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PWR_SEQ,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "ST Barometer",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SENSOR,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor(MAX1617A)",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_MB_TEMP,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp1 INLET1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP1_IN_1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp2 INLET2",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP2_IN_2,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp3 OUTLET1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP3_out_1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp4 OUTLET2",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP4_out_2,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU1 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU1_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU1 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU1_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_WORD_DATA,
        .wr_hd_size = I2C_SMBUS_WORD_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU2 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU2_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU2 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU2_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_WORD_DATA,
        .wr_hd_size = I2C_SMBUS_WORD_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PoE PSU1 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_PSU1_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PoE PSU1 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_PSU1_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .rd_hd_size = I2C_SMBUS_BYTE_DATA,
        .wr_hd_size = I2C_SMBUS_BYTE_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PoE PSU2 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_PSU2_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PoE PSU2 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_PSU2_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .rd_hd_size = I2C_SMBUS_BYTE_DATA,
        .wr_hd_size = I2C_SMBUS_BYTE_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "30W Powerball(PoE) Controller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
        .i2c_ctrl = I2C_CTRL_EIGHT,
        .sub_addr_len = 1, /* no stop bit if fpga slave sub adr is used */
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "30W Powerball(PoE) Quack",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_30W_QUACK,
        .i2c_ctrl = I2C_CTRL_EIGHT,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ONE,
        .buf = NULL,
    },
    {
        .dev_name = "PCIe Switch",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PCIE_SWITCH,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "USB0 Redriver",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB0_REDRIVER,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "USB1 Redriver",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB1_REDRIVER,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "IR35700 1.2V DP",
        .offset = -1,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_3570_1P2V_DP,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_THREE,
        .buf = NULL,
    },
    {
        .dev_name = "IR35700 1.2V CP",
        .offset = -1,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_3570_1P2V_CP,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_THREE,
        .buf = NULL,
    },
    {
        .dev_name = "IR35700 0.85V",
        .offset = -1,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_3570_0P85V,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_THREE,
        .buf = NULL,
    },
};

    /*
    NGIO_I2C_ADDR_ACT2;
   */
static uint8_t sm_i2c_ctrl[] = {0,  SM1_I2C_CTRL, SM2_I2C_CTRL, SM3_I2C_CTRL, SM4_I2C_CTRL};
static uint8_t wic_i2c_ctrl[] = {0,  WIC1_I2C_CTRL, WIC2_I2C_CTRL, WIC3_I2C_CTRL};
static uint8_t vm_i2c_ctrl[] = {0,  VM_I2C_CTRL};

static n2g_i2c_if_t wic_oir[MAX_WIC+FIRST_SLOT];
static n2g_i2c_if_t wic_act2[MAX_WIC+FIRST_SLOT];
static char wic_oir_buf[MAX_WIC+FIRST_SLOT][256];

static n2g_i2c_if_t sm_oir[MAX_SM+FIRST_SLOT];
static n2g_i2c_if_t sm_act2[MAX_SM+FIRST_SLOT];
static char sm_oir_buf[MAX_SM+FIRST_SLOT][256];

static n2g_i2c_if_t vm_oir[MAX_VM+FIRST_SLOT];
static char vm_oir_buf[MAX_VM+FIRST_SLOT][256];

static n2g_i2c_if_t carrier_oir[MAX_SM+FIRST_SLOT];
static char carrier_oir_buf[MAX_SM+FIRST_SLOT][256];

static n2g_i2c_if_t ngio_oir[] = {
    {
        .dev_name = "OIR",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .size    = sizeof(uint16_t),
        .sub_addr_len = 1,
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
};
        
static n2g_i2c_if_t ngio_act2[] = {

    {
        .dev_name = "ACT2",
        .offset = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
};


/*
 * I2C Utility Menu.
 */
static submenu_xtable_t i2c_menu_table[] = {
    {"Display Memory Info",            (PFT)display_mem_info,  TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Show Barometer Info",            (PFT)show_barometer_info,        TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Display Barometer Registers",    (PFT)display_barometer_reg,        TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Temperature Sensor",             (PFT)build_ts_menu,            TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"MAX1617A Temp. Sensor",          (PFT)build_snsr_menu,            TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PCIe Clock",                     (PFT)build_pcie_clk_test_menu,   TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Sys Clock",                      (PFT)build_sys_clk_menu,         0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"30W POE",                        (PFT)build_30w_poe_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Power Sequencer",                (PFT)build_pwr_seq_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU MUX Utility",	               (PFT)build_mux_menu,             1,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    /*neptune using the same PSU as O2 */
    {"PSU1",                           (PFT)build_psu_menu,             OVLD_PSU1_TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU2",                           (PFT)build_psu_menu,             OVLD_PSU2_TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Check PoE PSU available",        (PFT)ovld_check_poe_psu_wrap,    0, 
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"12V PoE PSU1",                   (PFT)build_poe_psu_menu,         OVLD_POE_PSU1_TRUE,
        0,                             (PFT)has_poe_psu,                POE_PSU_ONE,
      (PFT)0,                          0},
    {"12V PoE PSU2",                   (PFT)build_poe_psu_menu,         OVLD_POE_PSU2_TRUE,
        0,                             (PFT)has_poe_psu,                POE_PSU_TWO,
      (PFT)0,                          0},
    {"USB Console",                    (PFT)build_i2c_usb_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Display ICH I2C Registers",      (PFT)show_ich_i2c,               FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"FPGA I2C scan addr.",            (PFT)fpga_i2c_scan_addr,         FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Voltage Regulator",              (PFT)build_vtg_rgltr_menu,       0,
         0,                             (PFT)0,                          0,
       (PFT)0,                          0},
    {"I2C read (no offset)",           (PFT)read_i2c,                   FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C write (no offset)",          (PFT)write_i2c,                  FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C read",                       (PFT)read_i2c,                   TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C write",                      (PFT)write_i2c,                  TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Toggle i2c debug flag",          (PFT)platform_i2c_debug,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Reset I2C controller",           (PFT)reset_i2c_controller,       TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
};

#define I2C_MENU_TABLE_SIZE \
        (sizeof(i2c_menu_table) / sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_menu_primary_items[I2C_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t i2c_menu_secondary_items[I2C_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo i2cdiag = {
  "I2C Utility Menu",		    /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
  i2c_menu_primary_items,
};
static struct menuinfo *i2cdiagp = &i2cdiag;


/**********************************************************************
 *
 * Function: build_i2c_menu
 *
 * Description: Build I2C menu.
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
void
build_i2c_menu (void)
{

    build_primary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                          "I2C Utility Menu", &i2cdiagp);
    build_secondary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                            i2c_menu_secondary_items);
    menu(&i2cdiag, i2c_menu_secondary_items, 0);
}

/**********************************************************************
 *
 * Function:	platform_cpu_i2c_init
 *
 * Description:	Initialize CPU I2C bus controllers.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int
platform_cpu_i2c_init(void)
{
    uint32_t rc = PASSED;
    int i;
    n2g_i2c_if_t i2c_if;

    /*currently not used??? */
    for (i = CPU_I2C1; i <= CPU_I2C1; i++) {
	i2c_if.i2c_bus_type = i;	/* Setup the bus controller number */
	/* CPU I2C controller uses 100 KHz */
	i2c_if.i2c_speed = N2G_I2C_100KHZ;
	if (n2g_i2c_init(&i2c_if) != PASSED) {
	    rc = FAILED;
	}
    }
    return (rc);
}


/**********************************************************************
 *
 * Function: has_dimm1
 *
 * Description: Check if the unit has DIMM1. Brawn, Silverbolt, Ironhide,
 *		and Ramjet have onboard DRAM.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
boolean
has_dimm1(void)
{
    return(TRUE);
}


/**********************************************************************
 *
 * Function: has_midplane
 *
 * Description: Check if the unit has Midplane.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
boolean
has_midplane(void)
{
    return(TRUE);
}

/**********************************************************************
 *
 * Function: has_ps1
 *
 * Description: Check if the unit has PSU1.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
boolean
has_ps1(void)
{
    return(TRUE);
}


/**********************************************************************
 *
 * Function: has_ps2
 *
 * Description: Check if the unit has PSU2.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
boolean
has_ps2(void)
{
    return(TRUE);
}

/**********************************************************************
 *
 * Function: has_sfp1
 *
 * Description: Check if the unit has SFP1.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
boolean
has_sfp1(void)
{
    return(TRUE);
}

/**********************************************************************
 *
 * Function: has_sfp2
 *
 * Description: Check if the unit has SFP2.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
boolean
has_sfp2(void)
{
    return(TRUE);
}


/**********************************************************************
 *
 * Function:	show_temp
 *
 * Description:	Display temperatures.
 *
 * Inputs:	err_log - TRUE to cterr. FALSE to printf.
 *		format - Display format of display_format_t in common.h
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int
show_temp(int err_log, int format)
{
    int rc = FAILED;

    rc = show_temperature_all(); 
    return (rc);
}

/******************************************************************************
 *
 * Function   : init_dimm_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_dimm_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t dimm_no) {
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;

    switch(dimm_no) {
    case MB_I2C_DIMM0:
        i2c_dev->dev_addr = MB_I2C_ADDR_DIMM0;
        break;
    case MB_I2C_DIMM1:
        i2c_dev->dev_addr = MB_I2C_ADDR_DIMM1;
        break;
    default:
        printf("%s: Unknown DIMM no. = %d.\n", __FUNCTION__, dimm_no);
        return (FAILED);
        break;
    }

    i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = i2c_fd;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : reset_i2c_controller (int option)
 * Description: flush i2c device by writing to bit bang i2c register
 *              i2c register.
 * Inputs     : option , not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int reset_i2c_controller (int option)
{
    n2g_i2c_if_t i2c_if;
    unsigned long i2c_ctrl_addr = 0;

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 16);

    /* reset I2C controller */
    i2c_ctrl_addr = get_platform_i2c_addr(i2c_if.i2c_ctrl); 
    gfy_i2c_reset((goofy_i2c_t *)i2c_ctrl_addr);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : read_i2c_reg (int yes_offset)
 * Description: generic i2c read funtcion, allowing user to manually read from
 *              i2c register.
 * Inputs     : yes_offset. not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int
read_i2c_reg (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, offset, rc, i;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", I2C_CTRL_MAX, 0, I2C_CTRL_MAX );
    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, I2C_MUX_MAX);
    addr = gethex_answer("Enter 7 bit slave address ", 0x7F, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
    }
    else {
        i2c_if.offset = -1;
    }

    size = gethex_answer("Enter length (in bytes)", 2, 1, 10);
    i2c_if.size = size;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to read i2c\n");
        return FAILED;
    }

    printf("\n");
    for (i = 0; i < size ; i++) {
        printf("0x%02x ", d32[i]);
    }
    
    return PASSED;
}

/*******************************************************************************
 *
 * Function   : write_i2c_reg (int yes_offset)
 * Description: generic i2c write funtcion, allowing user to manually write to
 *              i2c register.
 *                example:
 *                    byte3 -  byte0 = 00 12 34 56
 *                    *buf is 0x56;  (little endian)
 *                    *(buf+1) is 0x34
 *                    *(buf+2) is 0x12
 *                    *(buf+3) is 0x0
 *    if we want to send data out, we need to shift to the left by 8 bits.
 *    we copy this to the data fifo. data fifo.
 *    format of fifo is byte0 to byte3.
 *    we will copy *(buf+3) to byte0 of fifo, *(buf+2) to byte1 of fifo. etc...
 *
 *
 * Inputs     : yes_offset, flag set if user is expected to enter register offset
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int
write_i2c_reg (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, rc, i;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    
    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", I2C_CTRL_MAX, 0, I2C_CTRL_MAX);

    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, I2C_MUX_MAX);
    
    addr = gethex_answer("Enter 7 bit slave address (on mux 0)", 0x7F, 1, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0x0, 0, 0xFF);
        i2c_if.offset = offset;
    } else {
        i2c_if.offset = -1;
    }
            
    size = gethex_answer("Enter length ", 2, 1, 20);
    i2c_if.size = size;
    
    for (i = 0; i < size; i++) {
        sprintf(msg, "Enter bytes %d", i);
        d8[i] = gethex_answer(msg, 0x0, 0, 0xFF);
    }

    i2c_if.buf = (char *)&d8[0];
 
    rc = n2g_i2c_write(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to write i2c.\n");
        return FAILED;
    }

    return PASSED;
}

/*****************************************************************************
 *
 * Function   : ovld_x86_i2c_scan_test (int option)
 * Description: scan all i2c devices on overlord
 *
 * Inputs     : optoin , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int ovld_x86_i2c_scan_test (int option)
{
    n2g_i2c_if_t  i2c_if;
    goofy_i2c_t *i2c;
    uint32_t      reg_val = 0, ret_val = FAILED, fail_ctr = 0;
    uint32_t      ix, max_retry, status;
    uint8_t       now_test = 0, test_end = 0, psu_skip = 0, psu_pre = 0, test_num = 1;
    uchar         *tname = (uchar *)"I2C scan";
    char          errbuf[OVLD_BUF_SIZE];
    int err_code = 0;
    unsigned int board_rev;

    testname("%s", tname);

    /* Detect board type is p1b or later */
    get_platform_bd_rev(&board_rev);

    /* Setup end of test by calculate all FPGA I2C device number */
    test_end = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));

    for (now_test = 0, test_num = 1; now_test < test_end; now_test++) {
        /* Get I2C device structure */
        memcpy(&i2c_if, &fpga_i2c_dev[now_test], sizeof(n2g_i2c_if_t));

        if (board_rev <= 1) { 
            /* Neptune P1b or older build doesn't has USB re-driver */
            if ((fpga_i2c_dev[now_test].i2c_dev == MB_I2C_ADDR_USB0_REDRIVER) || 
                (fpga_i2c_dev[now_test].i2c_dev == MB_I2C_ADDR_USB1_REDRIVER)) {
                if ((fpga_i2c_dev[now_test].mux == I2C_MUX_ZERO) && 
                    (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_SIXTEEN)) {
                    continue;
                }
            } 
        }

        /* skipped Max1617 Alert, since it can only be read when 
         * interrupt is available.  
         */
        if (fpga_i2c_dev[now_test].i2c_dev == MB_I2C_ADDR_MB_TEMP_ALRT &&
            fpga_i2c_dev[now_test].mux == I2C_MUX_ZERO &&
            fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_TWO) 
            continue;

        /* check PSU and POE PSU is available or not. 
         */
        /* we check both present and stat because: 
         * 1. the psu_stat might pass even though there is no psu, 
         * 2. the psu is installed, but not connected with power line(stat)
         */
        if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR) {
           switch (fpga_i2c_dev[now_test].mux){
           case I2C_MUX_ZERO:
            if ((strcmp(fpga_i2c_dev[now_test].dev_name,"PSU2 EEPROM") == 0) ||
                (strcmp(fpga_i2c_dev[now_test].dev_name,"PSU2 Microcontroller") == 0)) {
                psu_skip = check_psu_stat(OVLD_PSU2);
                psu_pre = check_psu_present(OVLD_PSU2);
                set_mux_channel(NULL, I2C1_MUX_PORT1_MASK, OVLD_PSU_I2C_MUX);
            } else if ((strcmp(fpga_i2c_dev[now_test].dev_name,"PoE PSU1 EEPROM") == 0) ||
                (strcmp(fpga_i2c_dev[now_test].dev_name,"PoE PSU1 Microcontroller") == 0)) { 
                psu_skip = has_poe_psu(POE_PSU_ONE);
                if (psu_skip) {
                    set_mux_channel(NULL, I2C1_MUX_PORT2_MASK, OVLD_PSU_I2C_MUX);
                }
            } else if ((strcmp(fpga_i2c_dev[now_test].dev_name,"PoE PSU2 EEPROM") == 0) ||
                (strcmp(fpga_i2c_dev[now_test].dev_name,"PoE PSU2 Microcontroller") == 0)) {
                psu_skip = has_poe_psu(POE_PSU_TWO);
                if (psu_skip) {
                    set_mux_channel(NULL, I2C1_MUX_PORT3_MASK, OVLD_PSU_I2C_MUX);
                }
            } else {
                psu_skip = check_psu_stat(OVLD_PSU1);
                psu_pre = check_psu_present(OVLD_PSU1);
                set_mux_channel(NULL, I2C1_MUX_PORT0_MASK, OVLD_PSU_I2C_MUX);
            } 
               break;
           case I2C_MUX_ONE:
               psu_skip = check_psu_stat(OVLD_PSU2);
               psu_pre = check_psu_present(OVLD_PSU2);
               break;
           case I2C_MUX_TWO:
               psu_skip = has_poe_psu(POE_PSU_ONE);
               break;
           case I2C_MUX_THREE:
               psu_skip = has_poe_psu(POE_PSU_TWO);
               break;
           default: 
               printf("\n%s failure report: no such case", __FUNCTION__);
               break;
           } 

           /* device is not present or without power */
           if (psu_skip == TRUE) {
               if ((fpga_i2c_dev[now_test].mux == I2C_MUX_ZERO) ||
                   (fpga_i2c_dev[now_test].mux == I2C_MUX_ONE)) {
                   /* mux are for PSU  */ 
                   if (psu_pre != TRUE) {
                       /* psu is not present, skipped */
                       continue;
                   } else {
                       /* use fix number 0x5D(IIN_OC_WARN_LIMIT) for ucontroller */
                       i2c_if.offset = 0x5D;
                       i2c_if.size = 2;
                   }
               }
           } else {
               /* psu or poe stat is not good, skipped */
               continue;
           }
        }

        if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR ||
            fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FIVE) {
            i2c = (goofy_i2c_t *)get_platform_i2c_addr(fpga_i2c_dev[now_test].i2c_ctrl); 
            gfy_i2c_reset((goofy_i2c_t *)i2c);
            max_retry = 3;  /* for PSU and PSU2, retry may be necessary */
        } else {
            max_retry = 1;
        }

        /* check POE DC Controller and quack is present or not.
         */

        if ((i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_CTRLER) ||
            (i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_QUACK)) {
	    if (check_skip_test(mb_skip_item_name[POECARD_SK]) == TRUE) {
                continue;
            }

            if (force_skip_30wpoe()) {
                continue;
            }
            if(is_poe_present(errbuf) == FALSE) {
                cterr('f', 0, "failed to access %s. "
                      "%s is device installed?",
                      i2c_if.dev_name, errbuf);
                return(FAILED);
            }
        }

        i2c_if.buf = (char *)&reg_val;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Now testing %2d: I2C ctrl %2d, Mux %d, %-29s(0x%.2X)... ",
                   now_test, i2c_if.i2c_ctrl, i2c_if.mux, i2c_if.dev_name,
                   (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "[%2d] %s, ", test_num, i2c_if.dev_name);  
        } 
   
        /* Read I2C device Register 0 */
        for (fail_ctr = ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                fail_ctr++;
                err_code = i2c_err_no(&status);
                if (max_retry > 1) 
                      printf("warning: %s failed %s [i2c_status=%#x] during pass %d",
                      i2c_if.dev_name,
                      i2c_err_str(err_code), status, ix);
            } else {
                break;
            }
            msleep(30);
        }
        
        if (ret_val != PASSED) {
            err_code = i2c_err_no(&status);
            cterr('f', 0, "%s %s failed %s [i2c_status=%#x]",
                  tname, i2c_if.dev_name,
                  i2c_err_str(err_code), status );
            return(FAILED);
        } 

        if ((NVRAM)->diagflag & D_VERBOSE)
            printf("Done\n");

        test_num++;
        if (i2c_if.i2c_dev == MB_I2C_ADDR_PSU2_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU2_MCNTRL ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_MCNTRL ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM0_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM0_MCNTRL ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM1_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM1_MCNTRL ) { 

            msleep(30);
        }
    }

    prpass(testpass, NULL);
    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
 * Description: give address and controller number, return i2c structure
 *
 * Inputs     : addr: i2c addres; ctrl_no: i2c controller number
 *
 * Outputs    : pointer to i2c structure, or NULL if i2c struct is not found.
 *
 *******************************************************************************
 */
void *
platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
{
    int i, max;
    n2g_i2c_if_t *i2c;
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
   
    for (i = 0; i < size; i++) {
        if (fpga_i2c_dev[i].i2c_dev == addr) {
            /* to support different type of module/motherboard, etc...*/
            fpga_i2c_dev[i].i2c_ctrl = ctrl_no;
            return ((void *)(&fpga_i2c_dev[i]));
        }
    }

    if (addr == NGIOSM_I2C_ADDR_ACT2) {
        max = get_max_sm_slots();
        for (i = FIRST_SLOT; i <= max; i++) {
            i2c = (n2g_i2c_if_t *)platform_get_sm_act2(i);
            if (i2c->i2c_ctrl == ctrl_no)
                return((void*)i2c);
        }
    }

    if (addr == NGIOWIC_I2C_ADDR_ACT2) {
        max = get_max_wic_slots();
        for (i = FIRST_SLOT; i <= max; i++) {
            i2c = (n2g_i2c_if_t *)platform_get_wic_act2(i);
            if (i2c->i2c_ctrl == ctrl_no){
                return((void*)i2c);
            }
        }

        /* it's possible that wic is installed in carrier card.
         then, act2 addr is WIC act2 addr but other properties must
         be derived from sm slot.
         */
        max = get_max_sm_slots();
        for (i = FIRST_SLOT; i <= max; i++) {
            i2c = (n2g_i2c_if_t *)platform_get_sm_act2(i);
            if (i2c->i2c_ctrl == ctrl_no){
                i2c->i2c_dev = addr;
                return((void*)i2c);
            }
        }
    }
    /* check mdoules now */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
           addr, ctrl_no);

    return (void*)NULL;
}

/*******************************************************************************
 *
 * Function   : get_n2g_i2c_if 
 * Description: ret
 *
 * Inputs     : optin ...not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
void *
get_n2g_i2c_if (uint8_t i2c, uint8_t mux, uint8_t addr)
{
    int i;
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    for (i = 0; i < size; i++) {
        if (fpga_i2c_dev[i].i2c_dev == addr &&
            fpga_i2c_dev[i].mux == mux &&
            fpga_i2c_dev[i].i2c_ctrl == i2c) {
            return ((void *)(&fpga_i2c_dev[i]));
        }
    }
    printf("problem trying to get n2g_i2c_if; i2c_ctrl=%d, mux=%d, addr=%#x\n",
           i2c, mux, addr);
    fflush(stdout);
    return (void*)NULL;
}

/*******************************************************************************
 *
 * Function   : fpga_i2c_scan_addr (int option)
 * Description: scan i2c devices on fpga
 *
 * Inputs     : optin ...not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int
fpga_i2c_scan_addr (int option)
{
    n2g_i2c_if_t  i2c_if;
    uint32_t      ret_val = FAILED, now_addr = 0, mask = 0, ctr = 0;
    uchar         d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;

    /* Out-of-rest all I2C controllers */
    mask = (FPGA_IN_I2C_0_RST | FPGA_IN_I2C_2_RST | FPGA_IN_I2C_4_RST |
            FPGA_IN_I2C_8_RST | FPGA_IN_I2C_10_RST | FPGA_IN_I2C_11_RST |
            FPGA_IN_I2C_12_RST | FPGA_IN_I2C_13_RST | FPGA_IN_I2C_14_RST |
            FPGA_IN_I2C_15_RST);
    unreset_platform_in_dev(mask);

    /* Get I2C controller & MUX number that you want to scan */
    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 16);
    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, 4);

    i2c_if.offset = 0;
    i2c_if.size = 1;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    printf("\nI2C Ctrl %d, Mux %d, available addr. =", i2c_if.i2c_ctrl, i2c_if.mux);

    for (now_addr = 0x00; now_addr <= 0x7F; now_addr++) {
        i2c_if.i2c_dev = now_addr;
        ret_val = FAILED;

        /* Read I2C device Register 0 */
        ret_val = n2g_i2c_read(&i2c_if);
        if (ret_val == PASSED) {
            printf(" %#x", now_addr);
            ctr++;
        }
    }

    if (ctr == 0) {
        printf(" None");
    }

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : get_wic_i2c_ctrl (int slot)
 * Description: returns i2c address of wic i2c controller
 *
 * Inputs     : slot number
 *
 * Outputs    : i2c address
 *
 *******************************************************************************
 */
uint8_t
get_wic_i2c_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_wic_i2c_ctrl");
    }
    return (wic_i2c_ctrl[slot]);

}

/*******************************************************************************
 *
 * Function   : get_sm_i2c_ctrl (int slot)
 * Description: returns i2c address of sm i2c controller
 *
 * Inputs     : slot number
 *
 * Outputs    : i2c address
 *
 *******************************************************************************
 */
uint8_t
get_sm_i2c_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_sm_i2c_ctrl: wrong slot 0");
    }
    return (sm_i2c_ctrl[slot]);
}

/*******************************************************************************
 *
 * Function   : platform_get_wic_oir
 * Description: returns WIC OIR 
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to OIR
 *
 *******************************************************************************
 */
void *
platform_get_wic_oir (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&wic_oir[slot], ngio_oir, sizeof(n2g_i2c_if_t));
    wic_oir[slot].buf = wic_oir_buf[slot];
    wic_oir[slot].i2c_ctrl = wic_i2c_ctrl[slot];
    wic_oir[slot].i2c_dev = NGIOWIC_I2C_ADDR_OIR;
    return (void *)&wic_oir[slot];
}

/*******************************************************************************
 *
 * Function   : platform_get_wic_act2
 * Description: returns act2 WIC struture
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to act2 struct for wic
 *
 *******************************************************************************
 */
void *
platform_get_wic_act2 (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0"); 
    }
    memcpy(&wic_act2[slot], ngio_act2, sizeof(n2g_i2c_if_t));
    wic_act2[slot].i2c_ctrl = wic_i2c_ctrl[slot];
    wic_act2[slot].i2c_dev = NGIOWIC_I2C_ADDR_ACT2;
    return (void *)&wic_act2[slot]; 
}

/*******************************************************************************
 *
 * Function   : platform_get_sm_oir
 * Description: returns SM OIR 
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to OIR
 *
 *******************************************************************************
 */
void *
platform_get_sm_oir (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&sm_oir[slot], ngio_oir, sizeof(n2g_i2c_if_t));
    sm_oir[slot].buf = sm_oir_buf[slot];
    sm_oir[slot].i2c_ctrl = sm_i2c_ctrl[slot];
    sm_oir[slot].i2c_dev = NGIOSM_I2C_ADDR_OIR;
    return (void *)&sm_oir[slot];
}

/*******************************************************************************
 *
 * Function   : platform_get_sm_act2
 * Description: returns act2 SM struture
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to act2 struct for SM
 *
 *******************************************************************************
 */
void *
platform_get_sm_act2 (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&sm_act2[slot], ngio_act2, sizeof(n2g_i2c_if_t));
    sm_act2[slot].i2c_ctrl = sm_i2c_ctrl[slot];
    sm_act2[slot].i2c_dev = NGIOSM_I2C_ADDR_ACT2;
    return (void *)&sm_act2[slot];
}

/*******************************************************************************
 *
 * Function   : platform_get_vm_oir
 * Description: returns VM OIR 
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to OIR
 *
 *******************************************************************************
 */
void *
platform_get_vm_oir (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&vm_oir[slot], ngio_oir, sizeof(n2g_i2c_if_t));
    vm_oir[slot].buf = vm_oir_buf[slot];
    vm_oir[slot].i2c_ctrl = vm_i2c_ctrl[slot];
    vm_oir[slot].i2c_dev = NGIOVM_I2C_ADDR_OIR;
    return (void *)&vm_oir[slot];
}


/*******************************************************************************
 *
 * Function   : platform_get_dc_oir
 * Description: returns daughter card OIR 
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to OIR
 *
 *******************************************************************************
 */
void *
platform_get_dc_oir (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    printf("**FIXE ME: %s %d\n", __FILE__, __LINE__);
    return (void *)NULL;
}

/*******************************************************************************
 *
 * Function   : ovld_dimm_i2c_scan_test
 * Description: This function to check Overlord DIMM
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              dimm_no - number of DIMM
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_dimm_i2c_scan_test (char *errbuf, uint32_t dimm_no)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0, rc = FAILED;

    /* Init device structure */
    if (init_dimm_i2c_struct(&i2c_dev, dimm_no) != PASSED) {
        sprintf(errbuf, "Init DIMM%d i2c_dev struct failed.", dimm_no);
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Read the bytes from SPD DIMM */
    i2c_if.size = sizeof(uint32_t);	  /* Read 4 bytes at a time */
    i2c_if.offset = 0;
    i2c_if.buf = (char *)&reg_val;

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: Dimm%d is not installed.\n",
                            __FUNCTION__, dimm_no);
        } else {
            sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",
                            __FUNCTION__, rc);
        }
        return(FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: read_i2c
 *
 * Description: entry point to read i2c device
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static
int read_i2c (int has_offset)
{
    read_i2c_reg(has_offset);
    return PASSED;
}

/**********************************************************************
 *
 * Function: write_i2c
 *
 * Description: entry point to write i2c device
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static
int write_i2c(int has_offset)
{
    write_i2c_reg(has_offset);
    return PASSED;
}

/**********************************************************************
 *
 * Function: platform_i2c_debug
 *
 * Description: set global i2c debug flag. this will cause i2c driver
 *              to spit out a lot of debugging messages
 *
 * Input : d -- not used
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static
int platform_i2c_debug (int d)
{
    i2c_debug ^= 1;
    printf("i2c_debug flag is %d\n", i2c_debug);
    return PASSED;
}

void *
platform_get_carrier_wic_oir (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&carrier_oir[slot], ngio_oir, sizeof(n2g_i2c_if_t));
    carrier_oir[slot].buf = carrier_oir_buf[slot];
    carrier_oir[slot].i2c_ctrl = sm_i2c_ctrl[slot];
    carrier_oir[slot].i2c_dev = NGIOWIC_I2C_ADDR_OIR;
    return (void *)&carrier_oir[slot];
}

/**********************************************************************
 *
 * Function: display_mem_info 
 *
 * Description: Display memory/ram information 
 *
 * Input : NONE 
 *
 * Output: PASSED 
 *
 **********************************************************************
 */
int display_mem_info (void)
{
    char cmd[128];
    sprintf(cmd, "dmidecode -t memory");
    system(cmd);

    return (PASSED);
}
/* end of file */


/******** History ******** 
*---------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.3  2018/05/22 02:31:12  alpeng
fixed compiler warning, CSCvj57934

Revision 1.2  2018/05/18 09:24:59  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.26  2018/02/02 06:47:43  leschen
Support IR3570 voltage regulator utility.

Revision 1.1.2.25  2018/01/25 08:12:28  leschen
Adding PSU, POE_PSU and IR3570 into I2C lane scan test.

Revision 1.1.2.24  2018/01/25 06:54:23  alpeng
max addr value should be 0x7F for 7 bit addressing

Revision 1.1.2.23  2017/10/26 07:25:02  alpeng
remove set pericom power state to d0, the lastest fw image configures to d0 already

Revision 1.1.2.22  2017/10/03 06:53:56  leschen
Skip USB re-driver for P1B or older build for I2c scan test.

Revision 1.1.2.21  2017/09/27 06:01:46  leschen
Support usb re-driver for I2c scan test.

Revision 1.1.2.20  2017/07/05 06:31:15  alpeng
update fan info, update PSU and remove pem files

Revision 1.1.2.19  2017/07/05 03:13:22  alpeng
redirect show temp func, add d0 setup on i2c scan

Revision 1.1.2.18  2017/06/19 06:33:28  alpeng
add PCIe switch i2c test for v4.9 kernel is fixed D3 state issue

Revision 1.1.2.17  2017/06/13 09:26:42  alpeng
skip PCIe switch i2c test temporarily for v4.9 kernel

Revision 1.1.2.16  2017/05/03 03:46:44  alpeng
add max1617 into i2c util

Revision 1.1.2.15  2017/04/05 08:27:54  leschen
Sync with <ng_diag-tag-032917>

Revision 1.1.2.14  2017/03/23 06:35:06  leschen
Support Barometer LPS25H

Revision 1.1.2.13  2017/01/10 23:42:34  ptong
Print item skipped msg in the mb submenu

Revision 1.1.2.12  2017/01/04 08:47:04  leschen
Remove unnecessary utility.

Revision 1.1.2.11  2016/12/27 09:49:22  leschen
Clean up codes and add display mem info utility.

Revision 1.1.2.10  2016/12/26 12:50:47  alpeng
add sys clk into i2c utility

Revision 1.1.2.9  2016/12/26 07:00:49  leschen
Support to skip pluggable modules tests and clean up codes.

Revision 1.1.2.8  2016/10/26 07:06:14  leschen
Modify I2C scan test and POE daughter card support.

Revision 1.1.2.7  2016/10/20 17:51:07  alpeng
update i2c num for sm3 and sm4

Revision 1.1.2.6  2016/10/12 23:38:19  leschen
Modify for PSU utility.

Revision 1.1.2.5  2016/10/06 20:29:58  leschen
Modify PCIe clk buf for Neptune.

Revision 1.1.2.4  2016/08/30 08:28:27  leschen
Modify i2c_sacn test for Neptune.

Revision 1.1.2.3  2016/06/06 09:39:38  leschen
Remove ENV MCU related item for Neptune.

Revision 1.1.2.2  2016/06/03 09:49:35  leschen
Support Neptune temperature sensor utilites.

Revision 1.1.2.1  2016/06/02 22:04:01  jskow
Move Overlord/x86 specific files to Neptune/x86.

Revision 1.50  2015/04/22 01:06:49  alpeng
adding a new item to display poe psu

Revision 1.49  2014/07/01 08:58:51  bowang3
Add functions to support NGSM carrier card Thule

Revision 1.48  2014/06/27 08:04:39  danchung
Don't try to access PoE PSU module when the AC+IP supply is plugged in Juno

Revision 1.47  2014/05/14 19:22:27  mcharon
add support for wic on thule..i2c_get_quack returns correct i2c structure

Revision 1.46  2013/11/26 08:40:38  hroni
fix compiler warning

Revision 1.45  2013/11/01 07:04:45  alpeng
support i2c scan test on juno-plx

Revision 1.44  2013/10/09 08:10:49  alpeng
fixed a typo for i2c scan on juno psu

Revision 1.43  2013/09/14 02:36:25  alpeng
support ACT2 on modules

Revision 1.42  2013/09/11 02:25:08  alpeng
1. support Juno fan info and display on initialize stage.
2. support fedora rootfs

Revision 1.41  2013/08/13 07:19:29  alpeng
support i2c scan on PEM ucontroller, update the code for new PSU eeprom w/r and ucontroller read

Revision 1.40  2013/08/12 11:06:38  alpeng
clean up msg, skip item before check status

Revision 1.39  2013/08/12 08:32:47  alpeng
add psu present check before i2c scan test, update sub_addr_sz to 1 for accessing ucontroller of psu

Revision 1.38  2013/07/04 08:02:19  alpeng
fixed is_overlord() for latest FPGA rev.

Revision 1.37  2013/06/04 07:09:15  hroni
move platform specific files to the corresponding directory (i.e. x86 and utah)

Revision 1.3  2013/05/31 12:51:28  danchung
Add checking board type for Juno.

Revision 1.2  2013/05/09 23:12:42  mcharon
remove #if 0

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.35  2013/04/25 16:39:04  mcharon
remove psu_scan_retry variable

Revision 1.34  2013/04/23 17:09:17  mcharon
fix menu item for i2c read (no offset)

Revision 1.33  2013/04/15 21:26:47  mcharon
cast char to uchar so can compile on n2g-diag2 without warning

Revision 1.32  2013/04/15 21:15:48  mcharon
remove i2c retry

Revision 1.31  2013/03/29 05:48:35  mcharon
dont' use cterr to print warning or test will stop if stop on error flag is on

Revision 1.30  2013/03/27 20:23:22  mcharon
add err message

Revision 1.29  2013/03/25 22:15:20  mcharon
add retry for i2c for psu1

Revision 1.28  2013/03/17 02:04:14  mcharon
support command line for testing 30w poe

Revision 1.27  2013/03/14 18:18:14  mcharon
add argument to i2c_err_no

Revision 1.26  2013/02/27 00:17:59  mcharon
reset controller very first time

Revision 1.25  2013/02/13 18:19:42  mcharon
when i2c scan test fails, do cterr and quit for loop

Revision 1.24  2013/02/08 22:26:59  mcharon
write i2c erro messages to log file

Revision 1.23  2013/02/06 06:09:35  mcharon
add i2c err_no . when fail use this to get failure reason

Revision 1.22  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.21  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.20  2012/11/06 20:39:51  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.19  2012/09/26 18:00:23  palin2
Add "30W PoE", "EEPROM", "PCIe Switch", "System Clock", "PoE PSU",
"Power Sequencer", and "PSU" to support do group tests of I2C utilities.

Revision 1.18  2012/09/19 22:42:52  palin2
Rename "FPGA I2C scan test" to "I2C scan test".

Revision 1.17  2012/09/19 22:30:50  palin2
Add I2C scan test support those I2C devices that are connected to Cavecreek.

Revision 1.16  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

Revision 1.15  2012/09/18 07:47:32  palin2
Add function to check system pressure in Diag boot-up process.

Revision 1.14  2012/08/22 19:44:27  palin2
Remove the process that reset I2C controller when I2C scan test failed to
avoid system reboot that is caused by it (CSCub38701).

Revision 1.13  2012/08/07 10:03:05  palin2
1)Replace to flag "fatal error" by "warning"
  when 30W PoE DC is not present.
2)Not try to reset I2C controller 2 to avoid
  system reboot based on HW's comment.

Revision 1.12  2012/07/17 23:59:11  mcharon
fix i2c utility...missing first byte && add i2c debug flag

Revision 1.11  2012/06/28 23:38:16  mcharon
fixed warning

Revision 1.10  2012/06/28 23:36:52  mcharon
fixed wording so it's more clear how to use i2c read/write utility

Revision 1.9  2012/06/14 09:54:34  alpeng
support skipped POE DC on i2c scan test, if it is not be installed

Revision 1.8  2012/06/05 11:44:37  palin2
Clean up compiler warnings.

Revision 1.7  2012/05/09 08:28:14  alpeng
moving FPGA I2C scan test to MB test menu

Revision 1.6  2012/05/07 17:05:04  palin2
Update 30W PoE DC Quack I2C info.

Revision 1.5  2012/05/04 08:03:32  alpeng
skip Max1617, check PoE and PoE PSU is present before I2C scan test

Revision 1.4  2012/04/17 14:14:06  palin2
Add 12V PoE PSU cookie utility support.

Revision 1.3  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
