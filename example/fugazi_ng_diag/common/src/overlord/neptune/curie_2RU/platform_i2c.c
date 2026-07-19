/* $Id: platform_i2c.c,v 1.1 2020/01/09 01:02:02 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Overlord I2C utility menu. P40t is header for SM1 and P38t is header
 * for SM2
 *
 * Sept. 2007, Simon Yen
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
extern void build_pem_menu(uint32_t);
extern void build_psu_menu(uint32_t);
extern void build_poe_psu_menu(uint32_t);
extern void build_vtg_mntr_menu(int);
extern void unreset_platform_in_dev(int);
extern int show_barometer_info(void);
extern int display_barometer_reg(void);
extern boolean has_poe_psu(uint32_t);
extern int ovld_pcie_clk_i2c_scan_test(char *);
extern int ovld_sys_clk_i2c_scan_test(char *);
extern uint32_t ovld_check_poe_psu_wrap(void);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t, uint32_t);
extern void build_vtg_rgltr_menu(void);
extern void build_vtg_rgltr_menu_2ru(void);

/*
 * Functional prototype
 */
boolean has_dimm1(void);

int show_temp(int, int);
extern int show_margins(int);

static int fpga_i2c_scan_addr(int);
static int write_i2c(int);
static int read_i2c(int);
static int platform_i2c_debug(int);

int curie_ir3570_i2c_scan_test(char *, uint32_t);
int curie_tps536xx_i2c_scan_test (char *, uint32_t);
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
        .i2c_dev = MB_I2C_ADDR_PEM0_EEPROM,
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
        .i2c_dev = MB_I2C_ADDR_PEM0_MCNTRL,
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
        .i2c_dev = MB_I2C_ADDR_PEM1_EEPROM,
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
        .i2c_dev = MB_I2C_ADDR_PEM1_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_WORD_DATA,
        .wr_hd_size = I2C_SMBUS_WORD_DATA,
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
};

static n2g_i2c_if_t fpga_i2c_dev_2ru[] = {
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
};

/*
 *  NGIO_I2C_ADDR_ACT2;
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

/* used for ngio */
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

/* used for general act2 devices, mb, non-ngio modules */
static n2g_i2c_if_t general_act2[] = {
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

static n2g_i2c_if_t plug_fpga_tc_i2c_dev[] = {

    /* I2C Device for Pluggable FPGA */
    {
     .dev_name = "Pluggable Test Card Temperature Sensor(LM75BDP)",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_I2C_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable Test Card ACT2",
     .offset = -1,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable (SGMII) Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_SGMII_TC_I2C_ADDR_GPIO_EXP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable (PCIe) Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_PCIE_TC_I2C_ADDR_GPIO_EXP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
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
    {"PCIe Clock",                     (PFT)build_pcie_clk_test_menu,   TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Power Sequencer",                (PFT)build_pwr_seq_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU MUX Utility",                (PFT)build_mux_menu,             OVLD_PSU_I2C_MUX,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    /*neptune using the same PSU as O2 */
    {"PSU1",                           (PFT)build_pem_menu,             OVLD_PSU1_TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU2",                           (PFT)build_pem_menu,             OVLD_PSU2_TRUE,
        0,                             (PFT)0,                          0,
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
  "I2C Utility Menu",           /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
  i2c_menu_primary_items,
};
static struct menuinfo *i2cdiagp = &i2cdiag;


/*
 * I2C Utility Menu 2RU.
 */
static submenu_xtable_t i2c_menu_table_2ru[] = {
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
    {"PCIe Clock",                     (PFT)build_pcie_clk_test_menu,   TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Power Sequencer",                (PFT)build_pwr_seq_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU MUX Utility",	               (PFT)build_mux_menu,             OVLD_PSU_I2C_MUX,
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
    {"Voltage Regulator",              (PFT)build_vtg_rgltr_menu_2ru,   0,
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

#define I2C_MENU_TABLE_SIZE_2RU \
        (sizeof(i2c_menu_table_2ru) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_menu_primary_items_2ru[I2C_MENU_TABLE_SIZE_2RU + MAX_BASE_ITEMS];
static mitem_t i2c_menu_secondary_items_2ru[I2C_MENU_TABLE_SIZE_2RU + MAX_BASE_ITEMS];

static struct menuinfo i2cdiag_2ru = {
  "I2C Utility Menu",           /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
  i2c_menu_primary_items_2ru,
};
static struct menuinfo *i2cdiagp_2ru = &i2cdiag_2ru;

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
    if (is_curie_1ru()) {
        build_primary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                              "I2C Utility Menu", &i2cdiagp);
        build_secondary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                                i2c_menu_secondary_items);
        menu(&i2cdiag, i2c_menu_secondary_items, 0);
    } else if (is_curie_2ru()) {
        build_primary_submenu(i2c_menu_table_2ru, I2C_MENU_TABLE_SIZE_2RU,
                              "I2C Utility Menu", &i2cdiagp_2ru);
        build_secondary_submenu(i2c_menu_table_2ru, I2C_MENU_TABLE_SIZE_2RU,
                                i2c_menu_secondary_items_2ru);
        menu(&i2cdiag_2ru, i2c_menu_secondary_items_2ru, 0);
    }
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
 *
 * Description: Check if the unit has Midplane.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */

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

/**********************************************************************
 *
 *
 * Description: Check if the unit has SFP1.
 *
 * Inputs: None.
 *
 * Outputs: TRUE/FALSE.
 *
 **********************************************************************
 */

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

/******************************************************************************
 *
 * Function   : init_ir3570_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_ir3570_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t chip_no)
{
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;

    /* default read header is 0, user need to
     * write address to chip then read data */
    i2c_dev->rd_hd_size = 0;
    i2c_dev->wr_hd_size = 1;

    switch(chip_no) {
    case MB_I2C_IR3570_VCORE:
        i2c_dev->dev_addr = MB_I2C_ADDR_3570_VCORE;
        break;
    case MB_I2C_IR3570_VCCSCSUS:
        i2c_dev->dev_addr = MB_I2C_ADDR_3570_1P05V_SCSUS;
        break;
    case MB_I2C_IR3570_VCCGBE:
        i2c_dev->dev_addr = MB_I2C_ADDR_3570_1P05V_GBE;
        break;
    case MB_I2C_IR3570_1_2V:
        i2c_dev->dev_addr = MB_I2C_ADDR_3570_1P2V;
        break;
    default:
        printf("%s: Unknown IR3570 no. = %d.\n", __FUNCTION__, chip_no);
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

/******************************************************************************
 *
 * Function   : init_tps536xx_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_tps536xx_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t chip_no)
{
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;

    /* default read header is 0, user need to
     * write address to chip then read data */
    i2c_dev->rd_hd_size = 0;
    i2c_dev->wr_hd_size = 1;

    switch(chip_no) {
    case MB_I2C_TPS536XX_VCORE_0P85_VCCSA:
        i2c_dev->dev_addr = MB_I2C_ADDR_TPS536XX_VCORE_0P85_VCCSA;
        break;
    case MB_I2C_TPS536XX_1P0V:
        i2c_dev->dev_addr = MB_I2C_ADDR_TPS536XX_1P0V;
        break;
    case MB_I2C_TPS536XX_1P2V_0P9VNN:
        i2c_dev->dev_addr = MB_I2C_ADDR_TPS536XX_1P2V_0P9VNN;
        break;
    case MB_I2C_TPS536XX_1P2V_1P05:
        i2c_dev->dev_addr = MB_I2C_ADDR_TPS536XX_1P2V_1P05;
        break;
    default:
        printf("%s: Unknown TPS536XX no. = %d.\n", __FUNCTION__, chip_no);
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
 * Function   : curie_1ru_x86_i2c_scan_test (int option)
 * Description: scan all i2c devices on overlord
 *
 * Inputs     : optoin , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
static int curie_1ru_x86_i2c_scan_test (int option)
{
    n2g_i2c_if_t  i2c_if;
    goofy_i2c_t *i2c;
    uint32_t      reg_val = 0, ret_val = FAILED, fail_ctr = 0, psu_skip = 0, psu_pre = 0;
    uint32_t      ix, max_retry, status;
    uint8_t       now_test = 0, test_end = 0, test_num = 1;
    uchar         *tname = (uchar *)"I2C scan";
    char          errbuf[OVLD_BUF_SIZE];
    int err_code = 0, test_ctr;
    unsigned int board_rev;

    testname("%s", tname);

    /* Detect board type is p1b or later */
    get_platform_bd_rev(&board_rev);

    /* Setup end of test by calculate all FPGA I2C device number */
    test_end = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));

    for (now_test = 0, test_num = 1; now_test < test_end; now_test++) {
        /* Get I2C device structure */
        memcpy(&i2c_if, &fpga_i2c_dev[now_test], sizeof(n2g_i2c_if_t));

        /* check PSU and POE PSU is available or not.
         */
        /* we check both present and stat because:
         * 1. the psu_stat might pass even though there is no psu,
         * 2. the psu is installed, but not connected with power line(stat)
         */
        if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR) {
            if ((strcmp(fpga_i2c_dev[now_test].dev_name,"PSU2 EEPROM") == 0) ||
                (strcmp(fpga_i2c_dev[now_test].dev_name,"PSU2 Microcontroller") == 0)) {
                psu_skip = check_psu_stat(OVLD_PSU2);
                psu_pre = check_psu_present(OVLD_PSU2);
                set_mux_channel(NULL, I2C1_MUX_PORT1_MASK, OVLD_PSU_I2C_MUX);
            } else {
                psu_skip = check_psu_stat(OVLD_PSU1);
                psu_pre = check_psu_present(OVLD_PSU1);
                set_mux_channel(NULL, I2C1_MUX_PORT0_MASK, OVLD_PSU_I2C_MUX);
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

    /* IR3570, I2C devices on CPU I2C bus */
    for (test_ctr = 0; test_ctr < 4; test_ctr++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr_db_print("Curie I2C device: IR3570-%d I2C scan test.\n",
            test_ctr);
        } else {
            prpass(testpass, "[%2d] CPU I2C devices: IR3570-%d, ",
                             test_num, test_ctr);
        }
        test_num++;

        if (curie_ir3570_i2c_scan_test(errbuf, test_ctr) != PASSED) {
            fail_ctr++;
            err_code = i2c_err_no(&status);
            cterr('f', 0, "IR3570-%d I2C scan failed %s %s [i2c_status=%#x]",
                  test_ctr, errbuf,
                  i2c_err_str(err_code), status);
            ret_val = FAILED;  /* test all the ir3570 */
        }
    }


    prpass(testpass, NULL);
    return (ret_val);
}

/*****************************************************************************
 *
 * Function   : curie_2ru_x86_i2c_scan_test (int option)
 * Description: scan all i2c devices on overlord
 *
 * Inputs     : optoin , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
static int curie_2ru_x86_i2c_scan_test (int option)
{
    n2g_i2c_if_t  i2c_if;
    goofy_i2c_t *i2c;
    uint32_t      reg_val = 0, ret_val = FAILED, fail_ctr = 0, psu_skip = 0, psu_pre = 0;
    uint32_t      ix, max_retry, status;
    uint8_t       now_test = 0, test_end = 0, test_num = 1;
    uchar         *tname = (uchar *)"I2C scan";
    char          errbuf[OVLD_BUF_SIZE];
    int err_code = 0, test_ctr;
    unsigned int board_rev;

    testname("%s", tname);

    /* Detect board type is p1b or later */
    get_platform_bd_rev(&board_rev);

    /* Setup end of test by calculate all FPGA I2C device number */
    test_end = (sizeof(fpga_i2c_dev_2ru)/sizeof(n2g_i2c_if_t));

    for (now_test = 0, test_num = 1; now_test < test_end; now_test++) {
        /* ACT2 is removed from P2 */
        if (strcmp(fpga_i2c_dev_2ru[now_test].dev_name, "ACT2") == 0)
            continue;
        /* Get I2C device structure */
        memcpy(&i2c_if, &fpga_i2c_dev_2ru[now_test], sizeof(n2g_i2c_if_t));

        /* check PSU and POE PSU is available or not.
         */
        /* we check both present and stat because:
         * 1. the psu_stat might pass even though there is no psu,
         * 2. the psu is installed, but not connected with power line(stat)
         */
        if (fpga_i2c_dev_2ru[now_test].i2c_ctrl == I2C_CTRL_FOUR) {
            switch (fpga_i2c_dev_2ru[now_test].mux){
            case I2C_MUX_ZERO:
                if ((strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PSU1 EEPROM") == 0) ||
                    (strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PSU1 Microcontroller") == 0)) {
                    psu_skip = check_psu_stat(OVLD_PSU1);
                    psu_pre = check_psu_present(OVLD_PSU1);
                    set_mux_channel(NULL, I2C1_MUX_PORT0_MASK, OVLD_PSU_I2C_MUX);
                } else if ((strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PSU2 EEPROM") == 0) ||
                           (strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PSU2 Microcontroller") == 0)) {
                    psu_skip = check_psu_stat(OVLD_PSU2);
                    psu_pre = check_psu_present(OVLD_PSU2);
                    set_mux_channel(NULL, I2C1_MUX_PORT1_MASK, OVLD_PSU_I2C_MUX);
                } else if ((strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PoE PSU1 EEPROM") == 0) ||
                           (strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PoE PSU1 Microcontroller") == 0)) {
                    psu_skip = has_poe_psu(POE_PSU_ONE);
                    if (psu_skip) {
                        set_mux_channel(NULL, I2C1_MUX_PORT2_MASK, OVLD_PSU_I2C_MUX);
                    }
                } else if ((strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PoE PSU2 EEPROM") == 0) ||
                           (strcmp(fpga_i2c_dev_2ru[now_test].dev_name,"PoE PSU2 Microcontroller") == 0)) {
                    psu_skip = has_poe_psu(POE_PSU_TWO);
                    if (psu_skip) {
                        set_mux_channel(NULL, I2C1_MUX_PORT3_MASK, OVLD_PSU_I2C_MUX);
                    }
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
                if ((fpga_i2c_dev_2ru[now_test].mux == I2C_MUX_ZERO) ||
                    (fpga_i2c_dev_2ru[now_test].mux == I2C_MUX_ONE)) {
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

        if (fpga_i2c_dev_2ru[now_test].i2c_ctrl == I2C_CTRL_FOUR ||
            fpga_i2c_dev_2ru[now_test].i2c_ctrl == I2C_CTRL_FIVE) {
            i2c = (goofy_i2c_t *)get_platform_i2c_addr(fpga_i2c_dev_2ru[now_test].i2c_ctrl);
            gfy_i2c_reset((goofy_i2c_t *)i2c);
            max_retry = 3;  /* for PSU and PSU2, retry may be necessary */
        } else {
            max_retry = 1;
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

    /* TPS53659/TPS53622, I2C devices on CPU I2C bus */
    for (test_ctr = 0; test_ctr < 4; test_ctr++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr_db_print("Curie I2C device: TPS536XX-%d I2C scan test.\n",
            test_ctr);
        } else {
            prpass(testpass, "[%2d] CPU I2C devices: TPS536XX-%d, ",
                             test_num, test_ctr);
        }
        test_num++;

        if (curie_tps536xx_i2c_scan_test(errbuf, test_ctr) != PASSED) {
            fail_ctr++;
            err_code = i2c_err_no(&status);
            cterr('f', 0, "TPS536XX-%d I2C scan failed %s %s [i2c_status=%#x]",
                  test_ctr, errbuf,
                  i2c_err_str(err_code), status);
            ret_val = FAILED;  /* test all the TPS536XX */
        }
    }


    prpass(testpass, NULL);
    return (ret_val);
}

/*****************************************************************************
 *
 * Function   : curie_x86_i2c_scan_test (int option)
 * Description: scan all i2c devices on overlord
 *
 * Inputs     : optoin , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int curie_x86_i2c_scan_test (int option)
{
    if (is_curie_1ru())
        return curie_1ru_x86_i2c_scan_test(option);
    else if (is_curie_2ru())
        return curie_2ru_x86_i2c_scan_test(option);
    return 0;
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
    n2g_i2c_if_t *i2c = NULL;
    int size = 0;
    int size_tc_plug_fpga = (sizeof(plug_fpga_tc_i2c_dev) / sizeof(n2g_i2c_if_t));

    if (is_curie_1ru()) {
        i2c = fpga_i2c_dev;
        size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    } else if (is_curie_2ru()) {
        i2c = fpga_i2c_dev_2ru;
        size = (sizeof(fpga_i2c_dev_2ru)/sizeof(n2g_i2c_if_t));
    }

    for (i = 0; i < size; i++) {
        if (i2c[i].i2c_dev == addr) {
            /* to support different type of module/motherboard, etc...*/
            memcpy(&general_act2[0], &i2c[i], sizeof(n2g_i2c_if_t));

            general_act2[0].i2c_ctrl = ctrl_no;
            return ((void *)(&general_act2[0]));
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

    for (i = 0; i < size_tc_plug_fpga; i++) {
        if (plug_fpga_tc_i2c_dev[i].i2c_dev == addr) {
            plug_fpga_tc_i2c_dev[i].i2c_ctrl = ctrl_no;
            return ((void *) (&plug_fpga_tc_i2c_dev[i]));
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
    n2g_i2c_if_t *i2c_dev = NULL;
    int size = 0;

    if (is_curie_1ru()) {
        i2c_dev = fpga_i2c_dev;
        size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    } else if (is_curie_2ru()) {
        i2c_dev = fpga_i2c_dev_2ru;
        size = (sizeof(fpga_i2c_dev_2ru)/sizeof(n2g_i2c_if_t));
    }
    for (i = 0; i < size; i++) {
        if (i2c_dev[i].i2c_dev == addr &&
            i2c_dev[i].mux == mux &&
            i2c_dev[i].i2c_ctrl == i2c) {
            return ((void *)(&i2c_dev[i]));
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
    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 20);
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
 * Function   : curie_ir3570_i2c_scan_test
 * Description: This function to check Curie IR3570
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              chip_no - number of IR3570
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int curie_ir3570_i2c_scan_test (char *errbuf, uint32_t chip_no)
{
    n2g_i2c_dev_t i2c_dev;
    uint32_t      reg_val = 0;
    int rc = FAILED;

    /* Init device structure */
    if (init_ir3570_i2c_struct(&i2c_dev, chip_no) != PASSED) {
        sprintf(errbuf, "Init IR3570%d i2c_dev struct failed.", chip_no);
        return (FAILED);
    }

    rc = i2c_smbus_read_byte(i2c_dev.fp, (__u8 *)&reg_val);

    if (rc < 0) {
       return (FAILED);
    } else {
       return (PASSED);
    }
}

/*******************************************************************************
 *
 * Function   : curie_tps536xx_i2c_scan_test
 * Description: This function to check Curie TPS536XX
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              chip_no - number of IR3570
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int curie_tps536xx_i2c_scan_test (char *errbuf, uint32_t chip_no)
{
    n2g_i2c_dev_t i2c_dev;
    uint32_t      reg_val = 0;
    int rc = FAILED;

    /* Init device structure */
    if (init_tps536xx_i2c_struct(&i2c_dev, chip_no) != PASSED) {
        sprintf(errbuf, "Init TPS536XX%d i2c_dev struct failed.", chip_no);
        return (FAILED);
    }

    rc = i2c_smbus_read_byte(i2c_dev.fp, (__u8 *)&reg_val);

    if (rc < 0) {
        return (FAILED);
    } else {
        return (PASSED);
    }
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

/*
 *-----------------------------------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.1  2020/01/09 01:02:02  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
