/* $Id: platform_i2c.c,v 1.66 2019/09/11 07:18:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Overlord I2C utility menu. P40t is header for SM1 and P38t is header
 * for SM2
 *
 * Sept. 2007, Simon Yen
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
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
#include "platform_psu.h"
#include "byteswap.h"
#include "goofy_i2c.h"
#include "dash_fpga.h"
#include "i2c_address.h"
#include "platform_sfp_cookie.h"
#include "platform_slot.h"
#include "platform_fru.h"
#include "cli_cmd.h" /* show margining */
#include "bcm_gesw_defs.h" /* is gesw greyhound? */
#include "platform_eth.h"
#include "slot.h"

/*
 *  Externs
 */
extern int wic_unreset (void *p);
extern uchar mb_i2c_loc[];
extern fru_table_t platform_fru_table[];
extern jmp_buf *monjmpptr;
extern int get_i2c_fd(int);

/* for utilities submenu. */
extern void build_sys_clk_menu(uint32_t);
extern void build_gh_gesw_clk_menu(uint32_t);
extern void build_pcie_clk_test_menu(void);
extern int show_dimm(int dimm_no);
extern int alter_dimm(int dimm_no);
extern int show_ich_i2c(void);
extern uint32_t get_platform_memsize(void);
extern int build_sm_pclk_menu(void);
extern int build_env_menu(void);
extern void build_pwr_seq_menu(int);
extern int build_snsr_menu(void);
extern int rtc_init(int);
extern int build_sfp_cookie_menu(int);
extern int build_eeprom_menu(int);
extern int build_i2c_usb_menu(int);
extern int build_mux_menu(uint32_t);
extern void build_pem_psu_menu(uint32_t);
extern void build_poe_psu_menu(uint32_t);
extern int build_30w_poe_menu(int);
extern void build_pcie_sw_menu(int);
extern int build_plat_dimm_util_menu(int);
extern int build_ts_menu (int);
extern void unreset_platform_in_dev(int);
extern int  show_barometer_info(void);
extern boolean has_poe_psu(uint32_t);
extern int ovld_pcie_clk_i2c_scan_test(char *);
extern int utah_sys_clk_i2c_scan_test(char *);
extern int force_skip_30wpoe(void);
extern int force_skip_dimm1(void);
extern int pwr_read(n2g_i2c_if_t *, char *); 
extern uint32_t ovld_check_poe_psu_wrap(void);
extern int brd_ver;


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
boolean has_sfp(int);

static int fpga_i2c_scan_addr(int);
static int write_i2c(int);
static int read_i2c(int);
static int platform_i2c_debug(int);
static int stopbit_toggle(int d);
int ovld_dimm_i2c_scan_test(char *, uint32_t);
int reset_i2c_controller(int);
int gb_dcp_i2c_scan_test(char *errbuf);


/*
 *  Globals  
 */
unsigned char i2c_debug = 0;
static unsigned char stop_bit = 0; 

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
        .dev_name = "Altitude Sensor",
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
        .dev_name = "Rangeley SMLink",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SMLINK,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ONE,
        .buf = NULL,
    },
    {   /* for Sword and Dagger */
        .dev_name = "PSU1 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU1_EEPROM_SD,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {   /* PSU1 microcontroller for Utah */
        .dev_name = "PSU1 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU1_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
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
        .dev_name = "30W Powerball(PoE) Controller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
        .i2c_ctrl = I2C_CTRL_EIGHT,
        .sub_addr_len = 1, /* 1 means no stop bit -- fpga slave sub addr reg is used */
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
        .dev_name = "PLX PCIe Switch",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PLX_PCIE_SWITCH,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
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
    /* pfix-tmpout new sdk turned of i2c bus
    {
        .dev_name = "GE Switch",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_GE_SWITCH,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    */ //pfix-
    {
        .dev_name = "SFP MUX",
        .offset = 0xFFFFFFFF,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SFP_I2C_MUX,
        .i2c_ctrl = I2C_CTRL_SEVENTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t), /* sizeof(pca_t) */
        .mux = I2C_MUX_ZERO, 
        .buf = NULL,
    },
    {
        .dev_name = "PSU MUX",
        .offset = 0xFFFFFFFF,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU_I2C_MUX,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t), /* sizeof(pca_t) */
        .mux = I2C_MUX_ZERO, 
        .buf = NULL,
    },
    {
        .dev_name = "Bezel side temp sensor 0",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_BEZEL_TEMP0,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t), /* sizeof(ts_t) */
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    { 
        .dev_name = "Bezel side temp sensor 1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_BEZEL_TEMP1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t), /* sizeof(ts_t) */
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },  
    { 
        .dev_name = "I/O side temp sensor 0",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_IO_TEMP0,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t), /* sizeof(ts_t) */
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },  
    { 
        .dev_name = "I/O side temp sensor 1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_IO_TEMP1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t), /* sizeof(ts_t) */
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },  
    {
        .dev_name = "GE Clock Generator", /* 8T49N4811, for utah GH only */
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_GH_GESW_CLK,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "SFP",
        .offset = 0,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = SFP_EEPROM_BASE,
        .i2c_ctrl = I2C_CTRL_SEVENTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t), /* sizeof(pca_t) */
        .mux = I2C_MUX_ZERO, 
        .buf = NULL,
    },
    {
        .dev_name = "Virtual NIM GPIO Expander",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_IO_VIRTUAL_NIM_GPIO_EXPANDER ,
        .i2c_ctrl = I2C_CTRL_TWELVE,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t), /* sizeof(pca_t) */
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
};

    /*
    NGIO_I2C_ADDR_ACT2;
   */
static uint8_t sm_i2c_ctrl[] = {0,  SM1_I2C_CTRL, SM2_I2C_CTRL};
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
    {"DIMM0",                          (PFT)build_plat_dimm_util_menu,  MB_I2C_DIMM0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"DIMM1",                          (PFT)build_plat_dimm_util_menu,  MB_I2C_DIMM1,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Show Barometer Info",            (PFT)show_barometer_info,        TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Temperature Sensor",             (PFT)build_ts_menu,            TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PCIe Clock",                     (PFT)build_pcie_clk_test_menu,   TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PCIe Switch",                    (PFT)build_pcie_sw_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"30W POE",                        (PFT)build_30w_poe_menu,         TRUE,
        0,                             (PFT)is_utah,                    0,
      (PFT)0,                          0},
    {"SYS CLK",                        (PFT)build_sys_clk_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Greyhound GESW CLK",             (PFT)build_gh_gesw_clk_menu,     TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Power Sequencer",                (PFT)build_pwr_seq_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU MUX Utility",                (PFT)build_mux_menu,             1,
        0,                             (PFT)is_utah,                    0,
      (PFT)0,                          0},
    {"PSU1",                           (PFT)build_pem_psu_menu,      OVLD_PSU1_TRUE,
        0,                             (PFT)is_us_machines,             0,
      (PFT)0,                          0},
    {"Check PoE PSU available",        (PFT)ovld_check_poe_psu_wrap,    0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"12V PoE PSU1",                   (PFT)build_poe_psu_menu,  OVLD_POE_PSU1_TRUE,
        0,                             (PFT)has_poe_psu,                POE_PSU_ONE,
      (PFT)0,                          0},
    {"SFP MUX Utility",                (PFT)build_mux_menu,             0,
        0,                             0,                               0,
      (PFT)0,                          0},
    {"SFP0 cookie utility",            (PFT)build_sfp_cookie_menu,      1,
        0,                             (type_t(*)())has_sfp,     SFP_ZERO,
        (PFT)0,                        0},
    {"SFP1 cookie utility",            (PFT)build_sfp_cookie_menu,      2,
        0,                             (type_t(*)())has_sfp,      SFP_ONE,
        (PFT)0,                        0},
    {"SFP2 cookie utility",            (PFT)build_sfp_cookie_menu,      3,
        0,                             (type_t(*)())has_sfp,      SFP_TWO,
        (PFT)0,                        0},
    {"SFP3 cookie utility",            (PFT)build_sfp_cookie_menu,      4,
        0,                             (type_t(*)())has_sfp,    SFP_THREE,
        (PFT)0,                        0},
    {"USB Console",                    (PFT)build_i2c_usb_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Display ICH I2C Registers",      (PFT)show_ich_i2c,               FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"FPGA I2C scan addr.",            (PFT)fpga_i2c_scan_addr,         FALSE,
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
    {"Toggle i2c stop bit flag ", (PFT)stopbit_toggle,       TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Reset I2C controller",           (PFT)reset_i2c_controller,       TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Toggle i2c debug flag",          (PFT)platform_i2c_debug,         TRUE,
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

static void
display_env (void)
{
    show_margins_x(0, CLI_MODE);
}

static void
display_i2c_reg (void)
{
    /* cterr_db_print("Display ICH I2C regs:\n"); */
    show_ich_i2c();
}

static void
add_i2c_scan_err_report (void)
{
    fru_table_offset = MB_I2C;
    platform_fru_table[MB_I2C].pid_string = mb_pid;
    platform_fru_table[MB_I2C].location_string = mb_i2c_loc;
    cterr_add_component("MB", "FPGA/Rangeley", "I2C");
    cterr_add_reg_dump((PFV)display_i2c_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please use i2c utility to make sure i2c is working");
}

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
 * Function: has_sfp
 *
 * Description: Check whether the unit support the given sfp
 *              Supported platforms are Utah, Sword, and Dagger
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */
int 
has_sfp(int id)
{
    switch(id) 
    {
        case SFP_ZERO: /* avalable in Utah, Sword, and Dagger */
            return TRUE;
        case SFP_ONE: /* available in Utah */
            if (is_utah()) {
                return TRUE;
            }
            break;
        case SFP_TWO: /* available in Utah, Sword */
            if (is_utah() || is_sword()) {
                return TRUE;
            }
            break;
        case 3: /* not available in Utah, Sword, and Dagger */
        default:
            break;
    }
    return FALSE;
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

    printf("fixed me : %s \n", __FUNCTION__);
    /*
    rc = show_env_temp(err_log, format);
    rc |= show_mb_temp(format);
    */
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
    char d32[80];
    uint8_t d8;

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", I2C_CTRL_MAX, 0, I2C_CTRL_MAX);
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
    i2c_if.buf = d32;
    i2c_if.sub_addr_len = stop_bit;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to read i2c\n");
        return FAILED;
    }

    printf("\n");
    for (i = 0; i < size ; i++) {
        d8 = d32[i] & 0xFF;
        printf("0x%02x ", d8);
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

/****************************************************************************
 *
 * Function : ngiowic_unreset
 *
 ****************************************************************************
 */
int wic_unreset (void *p) {
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);

    ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_RESET);
    return (OK);
}


/*****************************************************************************
 *
 * Function   : ovld_x86_i2c_scan_test (int option)
 * Description: scan all i2c devices on overlord
 *
 * Inputs     : option , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int ovld_x86_i2c_scan_test (int option)
{
    struct ngio_intf_t *ngio;
    int slot;
    n2g_i2c_if_t  i2c_if;
    //goofy_i2c_t *i2c;
    char reg_tmp[32]; 
    uint32_t      ret_val = FAILED, fail_ctr = 0;
    char *reg_val;
    uint32_t      ix, status, max_retry = 1, x86_dimm_num = OVLD_X86_DIMM_NUM ;
    uint8_t       now_test = 0, test_end = 0, test_num = 1, test_ctr = 0;
    char          *tname = "I2C scan";
    char          errbuf[OVLD_BUF_SIZE], errbuf_pwr[OVLD_BUF_SIZE *2];
    int err_code = 0;

    if (get_enhance_err_flag()) {
        add_i2c_scan_err_report();
    }

    testname("%s", tname);

    /* Vg400 GPIO pin needs to unrest for I2C sacn */
    if (is_vg400()) {
        for (slot = FIRST_SLOT; slot < MAX_WIC_GOLDBEACH; slot++) {
            ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
            ngiowic_enable(ngio);
            ngiowic_i2c_unreset(ngio);
            wic_unreset(ngio); 
        }
    }

    /* Setup end of test by calculate all FPGA I2C device number */
    test_end = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));

    for (now_test = 0, test_num = 1; now_test < test_end; now_test++) {
        /* Get I2C device structure */
        memcpy(&i2c_if, &fpga_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        reg_val = &reg_tmp[0];
        i2c_if.buf = reg_val;
        if (is_goldbeach() || is_vg400()) { 
            /* Goldbeach only has one DIMM */
            x86_dimm_num = GB_X86_DIMM_NUM;
            /* Goldbeach didn't have below I2C device, skip test */
            if ((i2c_if.i2c_ctrl == I2C_CTRL_ONE) && (
                (i2c_if.i2c_dev == MB_I2C_ADDR_USB_CONSOLE)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_USB_CONSOLE_FW_DL)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_USB_CONSOLE_FW_DL))) {
               continue;
            }
            if ((i2c_if.i2c_ctrl == I2C_CTRL_FOUR) && (
                (i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_EEPROM_SD)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_MCNTRL)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_EEPROM)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_POE_PSU1_EEPROM)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_PSU_I2C_MUX)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_POE_PSU1_MCNTRL))) {
               continue;
            }
            if ((i2c_if.i2c_ctrl == I2C_CTRL_EIGHT) &&  (
                (i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_CTRLER)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_QUACK))) {
               continue;
            }
            if ((i2c_if.i2c_ctrl == I2C_CTRL_SIXTEEN) && (
                (i2c_if.i2c_dev == MB_I2C_ADDR_PLX_PCIE_SWITCH)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_PCIE_SWITCH)
                || (i2c_if.i2c_dev == MB_I2C_ADDR_GE_SWITCH))) {
               continue;
            }
            if ((i2c_if.i2c_ctrl == I2C_CTRL_SEVENTEEN) && 
               (i2c_if.i2c_dev == MB_I2C_ADDR_SFP_I2C_MUX)) {
               continue;
            }
            if ((i2c_if.i2c_dev == SFP_EEPROM_BASE) && 
                (i2c_if.i2c_ctrl == I2C_CTRL_SEVENTEEN)) {
                /* check SFP is available. if not, skipped */
                if((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            	    printf("\nSkip SFP I2C Scan\n");
                    fflush(stdout);
                    continue;
                } 
            } 
        }

        /* Greyhound GESW has additional clock chip 
         * skipped non-greyhound platform 
         */
        if ((i2c_if.i2c_ctrl == I2C_CTRL_SIXTEEN) &&
               (i2c_if.i2c_dev == MB_I2C_ADDR_GH_GESW_CLK)) {
            if (is_bcm_greyhound()) { 
                prpass(testpass,"Platform is Greyhound GESW, keep testing.");
            } else {
                continue; /* skipped on non-greyhound plat. */
            }
        }

        /* special test for power sequencer */
        if (i2c_if.i2c_ctrl == I2C_CTRL_TWO) {
            if (i2c_if.i2c_dev == MB_I2C_ADDR_PWR_SEQ) {
                ret_val = pwr_read(&i2c_if, &errbuf_pwr[0]);
                if (ret_val != PASSED) {
                    err_code = i2c_err_no(&status);
                    cterr('f', 0, "%s %s failed %s [i2c_status=%#x]",
                        tname, i2c_if.dev_name, i2c_err_str(err_code), status );
                    ret_val = FAILED;
                    goto end_test;
                }  else {
                   continue;
                }
            }
        }

        /* SMLINK/PECI for NIOS debugging */
        if (i2c_if.i2c_dev == MB_I2C_ADDR_SMLINK && 
            strstr(i2c_if.dev_name, "Rangeley SMLink")) {
            /*
              printf("\nbypass:  %2d: I2C ctrl %2d, Mux %d, %-29s(0x%.2X)\n ",
                   now_test, i2c_if.i2c_ctrl, i2c_if.mux, i2c_if.dev_name,
                   (i2c_if.i2c_dev << 1));
             */      
            continue;
        }

        /* both Sword and Dagger are using PLX PCIe switch */
        if (is_utah_plx() || is_sword() || is_dagger()) {
           if (i2c_if.i2c_dev == MB_I2C_ADDR_PCIE_SWITCH)  {
            continue;
           }
        } else { 
           if (i2c_if.i2c_dev == MB_I2C_ADDR_PLX_PCIE_SWITCH) {
            continue;
           }
        }

        if (is_utah()) {
            /* 
             * PSU and POE PSU are behind mux. 
             * They will be scanned during PSU mux scan.
             */
            if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR &&
                fpga_i2c_dev[now_test].i2c_dev != MB_I2C_ADDR_PSU_I2C_MUX) {
                continue;
            }
        } else if (is_dg_machines() || is_vg400()) {
            /* There is no I2C access to the PSU on Dagger/Goldbeach and VG400*/
            if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR) {
                continue;
            }
        } else { /* sword */
            /* 
             * Sword directly access the PSU and POE PSU 
             * Unlike Utah, Sword only has access to PSU1 EEPROM
             * and has no PSU1 microcontroller
             */
            if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR &&
                fpga_i2c_dev[now_test].i2c_dev == MB_I2C_ADDR_PSU1_EEPROM_SD) {
                /* Sword PSU 1 EEPROM */
            } else if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR) {
                /* Sword has no other devices in I2C_CTRL_FOUR */
                continue;
            }
        }
        /* check Virtual NIM GPIO Expander, only VG400 has it */
        if (i2c_if.i2c_ctrl == I2C_CTRL_TWELVE) {
            if (i2c_if.i2c_dev == MB_I2C_ADDR_IO_VIRTUAL_NIM_GPIO_EXPANDER) {
                if (!is_vg400()) {
                    continue;
                }
            }
        }
        /* check POE DC Controller and quack is present or not. */
        /* 30W POE does not exist on sword and dagger */
        if (i2c_if.i2c_ctrl == I2C_CTRL_EIGHT) {
            if (i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_CTRLER ||
                i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_QUACK) {
                if (force_skip_30wpoe() || is_dagger() || is_sword() || is_goldbeach() || is_vg400()) {
                    continue;
                }
                if(is_poe_present(errbuf) == FALSE) {
                    cterr('f', 0, "failed to access %s. "
                          "%s is device installed?",
                          i2c_if.dev_name, errbuf);
                    ret_val = FAILED;
                    goto end_test;
                }
            }
        }

        /* for sword, Bezel side temp sensor 0 doesn't exist 
         * for dagger, Bezel side0 and I/O side1 temp sensors don't exist 
         */
        if ((is_sword() && (i2c_if.i2c_dev == MB_I2C_ADDR_BEZEL_TEMP0)) ||
            (is_dagger() && ((i2c_if.i2c_dev == MB_I2C_ADDR_BEZEL_TEMP0 ) ||
                             (i2c_if.i2c_dev == MB_I2C_ADDR_IO_TEMP1)))) {
            if (i2c_if.i2c_ctrl == I2C_CTRL_TWO) {
                continue;
            }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr_db_print("Now testing %2d: I2C ctrl %2d, Mux %d, %-29s(0x%.2X)... \n",
                   now_test, i2c_if.i2c_ctrl, i2c_if.mux, i2c_if.dev_name,
                   (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "[%2d] %s, ", test_num, i2c_if.dev_name);  
        } 
        if (!(is_goldbeach()) || !(is_vg400())) { 
            if ((i2c_if.i2c_ctrl == I2C_CTRL_SEVENTEEN) &&
                (i2c_if.i2c_dev == SFP_EEPROM_BASE)) {
                continue;
            }
        }
        /* Read I2C device Register 0 */
        for (fail_ctr = ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                fail_ctr++;
                err_code = i2c_err_no(&status);
                if (max_retry > 1) {
                      cterr_db_print("\nwarning: %s failed %s [i2c_status=%#x] during pass %d",
                      i2c_if.dev_name,
                      i2c_err_str(err_code), status, ix);
                }
            } else {
                /* SFP scan test */
                if ((i2c_if.i2c_dev == MB_I2C_ADDR_SFP_I2C_MUX && 
                    i2c_if.i2c_ctrl == I2C_CTRL_SEVENTEEN) || 
                    (i2c_if.i2c_dev == SFP_EEPROM_BASE && 
                    i2c_if.i2c_ctrl == I2C_CTRL_SEVENTEEN)) {
                    ret_val = sfp_i2c_test_warp();
                }
                /* PSU scan test */
               else if((i2c_if.i2c_dev == MB_I2C_ADDR_PSU_I2C_MUX &&
                        i2c_if.i2c_ctrl == I2C_CTRL_FOUR) && is_utah()) {
                    ret_val = psu_i2c_test_warp();
                } else {
                    break;
                }
            }
            msleep(30);
        }
        
        if (ret_val != PASSED) {
            err_code = i2c_err_no(&status);
            cterr('f', 0, "%s %s failed %s [i2c_status=%#x]",
                  tname, i2c_if.dev_name,
                  i2c_err_str(err_code), status );
            ret_val = FAILED;
            goto end_test;
        } 

        test_num++;
        if (i2c_if.i2c_dev == MB_I2C_ADDR_PSU2_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_MCNTRL) {
            msleep(30);
        }
    }
   
    /* PCIe Clock */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        cterr_db_print("Rangeley I2C device: PCIe Clock I2C scan test.\n");
    } else {
        prpass(testpass, "[%2d] Rangeley I2C device: PCIe Clock, ", test_num);
    }
    test_num++;
 
    if (is_vg400()) {
        /* VG400 don't have PCIe Clock */
        ret_val = PASSED;  
    } else {
        if (ovld_pcie_clk_i2c_scan_test(errbuf) != PASSED) {        
            fail_ctr++;
            err_code = i2c_err_no(&status);
            cterr('f', 0, "PCIe Clock I2C scan failed %s %s [i2c_status=%#x]",
                  errbuf, i2c_err_str(err_code), status);
            ret_val = FAILED;
            goto end_test;
        }
    }

    /* System Clock */
    if ((NVRAM)->diagflag & D_VERBOSE) {
	    cterr_db_print("Rangeley I2C device: System Clock I2C scan test.\n");
    } else {
        prpass(testpass, "[%2d] Rangeley I2C device: System Clock, ",
                         test_num);
    }
    test_num++;
 
    if (utah_sys_clk_i2c_scan_test(errbuf) != PASSED) {        
        fail_ctr++;
        err_code = i2c_err_no(&status);
        cterr('f', 0, "System Clock I2C scan failed %s %s [i2c_status = %#x] ",
             errbuf,  i2c_err_str(err_code), status);
        ret_val = FAILED;
        goto end_test;
    }

    /* DIMM */
    for (test_ctr = 0; test_ctr < x86_dimm_num; test_ctr++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr_db_print("Rangeley I2C device: DIMM%d I2C scan test.\n", test_ctr);
        } else {
            prpass(testpass, "[%2d] Rangeley I2C device: DIMM%d, ",
                             test_num, test_ctr);
        }
        test_num++;
 
        if (ovld_dimm_i2c_scan_test(errbuf, test_ctr) != PASSED) {        
            fail_ctr++;
            err_code = i2c_err_no(&status);
            cterr('f', 0, "DIMM%d I2C scan failed %s %s [i2c_status=%#x]",
                  test_ctr, errbuf,
                  i2c_err_str(err_code), status);
            ret_val = FAILED;
            goto end_test;
        }
    }
    if (is_goldbeach() || is_vg400()) {
        /*DCP ISL90727 */
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr_db_print("\nRangeley I2C device: DCP I2C scan test.\n");
        } else {
            prpass(testpass, "[%2d] DCP device, ", test_num);
        }
        if (gb_dcp_i2c_scan_test(errbuf) != PASSED) {        
            fail_ctr++;
            err_code = i2c_err_no(&status);
            cterr('f', 0, "System Clock I2C scan failed %s %s [i2c_status = %#x] ",
                 errbuf,  i2c_err_str(err_code), status);
            ret_val = FAILED;
            goto end_test;
        }
    }
    prpass(testpass, NULL);
    
end_test:
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

            /*check special case for 30wpoe on sword */
            if (is_sword() && addr == MB_I2C_ADDR_ACT2 &&
                ctrl_no == I2C_CTRL_EIGHT) {
                if (fpga_i2c_dev[i].i2c_ctrl != I2C_CTRL_EIGHT) {
                    /* this must be mb quack, keep searching */
                    continue;
                }
            }
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
        /* it's possible that wic is installed on carrier card.
         then, act2 addr is based on the WIC act2 addr but other properties must
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
 * Inputs     : option ...not used
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
    char          d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;

    /* Out-of-rest all I2C controllers */
    mask = (FPGA_IN_I2C_0_RST | FPGA_IN_I2C_2_RST | FPGA_IN_I2C_4_RST |
            FPGA_IN_I2C_8_RST | FPGA_IN_I2C_10_RST | FPGA_IN_I2C_11_RST |
            FPGA_IN_I2C_12_RST | FPGA_IN_I2C_13_RST | FPGA_IN_I2C_14_RST |
            FPGA_IN_I2C_15_RST);
    unreset_platform_in_dev(mask);

    /* Get I2C controller & MUX number that you want to scan */
    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", I2C_CTRL_MAX, 0, I2C_CTRL_MAX);
    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, I2C_MUX_MAX);

    i2c_if.offset = 0;
    i2c_if.size = 1;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = d32;

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
    int res = 0; 

    if ((dimm_no == MB_I2C_DIMM1) && (force_skip_dimm1())) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDIMM1 I2C scan is skipped by user \n");
        }
        return (PASSED);
    }

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
    i2c_if.size = sizeof(uint8_t);	  /* Read 1 byte at a time */
    i2c_if.offset = 0;
    i2c_if.buf = (char *)&reg_val;

    /* DIMM1 is optional for dagger, when DIMM1 i2c scan fails
     * not to fail the test but print the warning message
     */
    if (is_dg_machines() && (dimm_no == 1)) {
        res = i2c_smbus_read_byte(i2c_dev.fp, (__u8 *)i2c_if.buf);
        if (res < 0) {
            printf("\n***warning: Dagger DIMM%d is optional and it is not detected. Test skipped.\n",dimm_no);            
            return (PASSED);
        }
    }    

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
/******************************************************************************
 *
 * Function   : init_dcp_i2c_struct
 * Description: To init DCP i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_dcp_i2c_struct (n2g_i2c_dev_t *i2c_dev) {
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;
    i2c_dev->dev_addr = MB_I2C_ADDR_DCP;

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
 * Function   : gb_dcp_i2c_scan_test
 * Description: This function to check DCP
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int gb_dcp_i2c_scan_test (char *errbuf)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0, rc = FAILED;
    int res = 0; 

    /* Init device structure */
    if (init_dcp_i2c_struct(&i2c_dev) != PASSED) {
        sprintf(errbuf, "Init DCP i2c_dev struct failed.");
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Read the bytes from DCP */
    i2c_if.size = sizeof(uint8_t);	  /* Read 1 byte at a time */
    i2c_if.offset = 0;
    i2c_if.buf = (char *)&reg_val;

    res = i2c_smbus_read_byte(i2c_dev.fp, (__u8 *)i2c_if.buf);
    if (res < 0) {
        return(FAILED);
    }

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: DCP is not installed.\n",
                            __FUNCTION__);
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
 * Function: stopbit_toggle
 *
 * Description: set global stop bit debug flag. this will tell software
 *              to either send or not send stop bit when
 * read_i2c_reg and write_i2c_reg are called
 *
 * Input : d -- not used
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static
int stopbit_toggle (int d)
{
    stop_bit ^= 1;
    if (stop_bit)
        printf("No stop bit will be sent. (flag=%d)\n", stop_bit);
    else
        printf("Stop bit will be sent. (flag=%d)\n", stop_bit);
    return PASSED;
}

void
fixup_30wpoe_addr (void)
{
    int i;
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    for (i = 0; i < size; i++) {
        if (fpga_i2c_dev[i].i2c_dev == MB_I2C_ADDR_POE_30W_QUACK
            && fpga_i2c_dev[i].i2c_ctrl == I2C_CTRL_EIGHT) {
            printf("new 30w poe found!!!!\n");
            fpga_i2c_dev[i].i2c_dev = MB_I2C_ADDR_ACT2;
            break;
        }
    }

}
/* end of file */

/******** History ******** 
*---------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.66  2019/09/11 07:18:15  alpeng
CSCvr18160 - adjust NIOS mode setup on Utah

Revision 1.65  2019/08/26 03:36:11  alpeng
CSCvq64781 - dimm1 is optional, provide dimm1 option arg for MFG

Revision 1.64  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.63  2018/05/18 09:25:01  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.62  2017/08/10 10:12:43  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.61  2017/07/10 03:21:55  leschen
Remove unused variable

Revision 1.60  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.59.20.3  2018/05/17 10:50:23  alpeng
 sync with trunk <trunk-051618>

Revision 1.59.20.2  2018/01/25 06:54:23  alpeng
max addr value should be 0x7F for 7 bit addressing

Revision 1.59.20.1  2017/04/05 09:27:13  leschen
Sync with <ng_diag-tag-032917>

Revision 1.62  2017/08/10 10:12:43  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.61  2017/07/10 03:21:55  leschen
Remove unused variable

Revision 1.60  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.59  2015/04/22 01:06:49  alpeng
adding a new item to display poe psu

Revision 1.58  2014/10/21 01:33:06  ptong
Temporary: Not checking GESW I2C

Revision 1.57  2014/09/10 01:24:20  ptong
Minor change in a printf statement

Revision 1.56  2014/09/09 11:38:23  danchung
correct the code to print warning instead of error when DIMM1 i2c scan
fails for Dagger

Revision 1.55  2014/09/05 10:45:35  danchung
DIMM1 is optional for dagger, when DIMM1 i2c scan fails not to fail the
test but print the warning message

Revision 1.54  2014/08/20 06:21:18  alpeng
support new testcard on non-GH platforms

Revision 1.53  2014/08/14 10:27:00  alpeng
support greyhound gesw clk gen on i2c scan test and its util

Revision 1.52  2014/07/22 11:59:51  danchung
1. For sword, remove bezel0(UT8) temp sensor related code
2. For dagger, remove bezel0(UT8) and IO1(UT7) temp sensors related code

Revision 1.51  2014/07/01 08:58:51  bowang3
Add functions to support NGSM carrier card Thule

Revision 1.50  2014/05/18 06:36:07  mcharon
disable NIOS before running scanning test.

Revision 1.49  2014/05/14 19:22:27  mcharon
add support for wic on thule..i2c_get_quack returns correct i2c structure

Revision 1.48  2014/03/11 08:08:55  alpeng
supprot 30w poe for utah only

Revision 1.47  2014/03/07 01:38:12  hroni
recover the i2c scan for usb console

Revision 1.46  2014/03/06 01:29:54  hroni
modify POE PSU mux for utah to I2C_MUX_ZERO

Revision 1.45  2014/03/05 06:22:07  hroni
1. rename macro check for bypassing usb console. currently USD machines don't needto do i2c scan for usb console. 2. do some cleanup

Revision 1.44  2014/02/26 10:25:33  alpeng
USD doesn't support 30w poe anymore; still keep the code for platform_cookie.c

Revision 1.43  2014/02/21 06:53:44  hroni
add enhance error messages for i2c scan test, aux loopback test, and dash fpga register test

Revision 1.42  2014/02/20 00:06:09  mcharon
in fixup function, add offset=-1 to support act2 mode for 30wpoe

Revision 1.41  2014/02/13 19:03:12  mcharon
support act2 authentication on sword

Revision 1.40  2014/02/08 03:13:15  alpeng
enable SMbus i2c scan test

Revision 1.39  2014/01/29 20:37:34  mcharon
support option not to send stop bit

Revision 1.38  2014/01/29 08:10:14  alpeng
update pwr seq i2c scan test

Revision 1.37  2014/01/28 02:02:50  mcharon
support 30w poe i2c scan test on sword

Revision 1.36  2014/01/28 01:23:02  mcharon
bring back 30w poe menu for utah and sword

Revision 1.35  2014/01/22 10:32:48  hroni
fix Utah POE PSU access

Revision 1.34  2014/01/22 02:43:21  hroni
fix i2c read util display

Revision 1.33  2014/01/21 10:44:46  hroni
Sword I2C scan for PSU only need to scan for PSU EEPROM

Revision 1.32  2014/01/17 09:24:15  hroni
separate PSU definition into for utah and for sword/dagger

Revision 1.31  2014/01/14 02:44:20  hroni
support NIOS_DIAG_MODE. use NIOS_DIAG_MODE instead of NIOS_NORMAL_MODE

Revision 1.30  2014/01/13 09:35:43  hroni
fix psu cookie write for sword

Revision 1.29  2014/01/09 06:21:13  hroni
add POE PSU1's microcontroller to I2C struct

Revision 1.28  2014/01/08 07:56:09  hroni
use enable_nios() instead of reseting NIOS

Revision 1.27  2014/01/08 07:26:14  danchung
Remove 30W POE of i2c utility for Sword and Dagger

Revision 1.26  2014/01/07 05:55:59  hroni
support psu diag for sword

Revision 1.25  2014/01/06 09:03:25  danchung
Remove 256 byte EEPROM from I2C utility for USD platforms

Revision 1.24  2013/12/26 02:42:38  hroni
1. remove NIOS reset that were done during diag init.
2. put NIOS to reset during i2c scan test and i2c utility. unreset after scan test or utility is finished

Revision 1.23  2013/12/18 07:53:35  hroni
sfp cookie utility only show the supported sfp port for each platform

Revision 1.22  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.21  2013/12/18 02:39:26  hroni
Fix psu mux util and do some clean up

Revision 1.20  2013/12/11 10:12:40  alpeng
remove usb console i2c test due to rommon issue; 30w poe is not supported on sword and dagger; adding temp sensor i2c test

Revision 1.19  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.18  2013/11/18 10:37:09  alpeng
support i2c scan test for PLX on sword/dagger

Revision 1.17  2013/10/02 07:55:19  hroni
due to rommon issue, temporarily bypass Rangeley I2C scan

Revision 1.16  2013/09/12 19:17:12  mcharon
support act2 structure for wic/sm

Revision 1.15  2013/08/22 06:40:49  alpeng
support fan utility on Utah

Revision 1.14  2013/07/25 17:17:05  hroni
turn off i2c_debug

Revision 1.13  2013/07/25 17:01:22  hroni
1. check psu presence during psu scan in psu_i2c_test_warp()
2. psu tests are done inside psu_i2c_test_warp()
3. sync sys clock scan test function name. now it is called utah_sys_clk_scan_test
4. modify DIMM i2c read size to 1 byte

Revision 1.12  2013/07/23 23:11:23  hroni
I2C scan will scan core #2 and core #8.

Revision 1.11  2013/07/18 17:17:03  mcharon
add -Wal and clean up compile warnings

Revision 1.10  2013/07/11 16:26:24  hroni
add menu for PSU mux

Revision 1.9  2013/07/03 03:22:31  hroni
remove ENV MCU from I2C utility menu because Utah has no ENV MCU

Revision 1.8  2013/07/01 07:50:29  hroni
use macro to define max range of i2c ctrl number and i2c mux number

Revision 1.7  2013/06/28 06:53:25  hroni
fix sfp mux utility

Revision 1.6  2013/06/19 09:45:40  hroni
add utilities for I/O side and Bezel side temperature sensors

Revision 1.5  2013/06/14 10:25:48  alpeng
support voltage margin

Revision 1.4  2013/06/14 09:51:05  hroni
1. support to PSU mux
2. temporarily uses #ifdef MUX124 to turn off/on PCA9545 mux support

Revision 1.3  2013/06/13 10:53:03  hroni
add mux_id in set_mux_channel()
uses set_i2c_if_struct() to set i2c_if structure

Revision 1.2  2013/06/13 08:34:56  hroni
add support for SFP mux

Revision 1.1  2013/06/04 07:09:14  hroni
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
