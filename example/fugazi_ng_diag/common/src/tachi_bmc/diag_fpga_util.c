/* $Id: diag_fpga_util.c,v 1.3 2016/10/27 03:24:46 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_util.c - FPGA Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2015 - 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "slot.h"
#include "menu.h"
#include "ngio.h"
#include "defs.h"
#include "proto.h"
#include "common_utils.h"
#include "diag_fpga_lib.h"
#include "diag_fpga_util.h"
#include "diag_fpga_prog.h"
#include <assert.h>
#include "nvmonvars.h"
#include "linux_api.h"
#include "uio_utils.h"
#include "i2c_api.h"
#include "diag_i2c_api.h"
#include "diag_i2c_test.h"
#include "platform_i2c.h"

int diag_fpga_util(void);
int diag_fpga_ver_display(void);
int lewis_reset_mask_flag;
static int diag_fpga_reg_alter(void);
static int diag_fpga_reg_display(void);
void unreset_platform_ext_dev(int);
void disable_platform_mcu_intr(int);
void disable_platform_vm_mcu_intr(int);
unsigned long get_platform_intr_ctrl_addr(int);
static void enable_top_cp_intr(int);
static void diag_power_ngio(void); 
static void diag_sfp_prsnt_output(void); 
int diag_fpga_i2c_scan(void);

static i2c_table_t fpga_i2c_table[] = {
    /* Dev on Controller 0 */
    {"ACT2", 8, MB_I2C_ADDR_ACT2, "Motherboard ACT2", ONBOARD_DEV, 0, 0, I2C_CTRL_ZERO},
    {"ENV MCU", 8, MB_I2C_ADDR_ENV_MCU, "Power Sequencer", ONBOARD_DEV, 0, 0, I2C_CTRL_TWO},
    {"Barometer", 8, MB_I2C_ADDR_SENSOR, "Altitude Sensor", ONBOARD_DEV, 0, 0, I2C_CTRL_TWO},
    {"Sensor 1", 8, MB_I2C_ADDR_TEMP_INLET_U27, "Temperature Sensor Inlet", ONBOARD_DEV, 0, 0, I2C_CTRL_TWO},
    {"Sensor 2", 8, MB_I2C_ADDR_TEMP_INLET_U29, "Temperature Sensor Inlet NIM", ONBOARD_DEV, 0, 0, I2C_CTRL_TWO},
    {"Sensor 3", 8, MB_I2C_ADDR_TEMP_OUTLET_U39, "Temperature Sensor Outlet U39", ONBOARD_DEV, 0, 0, I2C_CTRL_TWO},
    {"PEM0 EEPROM", 8, MB_I2C_ADDR_PEM0_EEPROM, "PEM0 EEPROM", ONBOARD_DEV, 0, 0, I2C_CTRL_FOUR},
    {"PEM0 MCNTRL", 8, MB_I2C_ADDR_PEM0_MCNTRL, "PEM0 Microcontroller", ONBOARD_DEV, 0, 0, I2C_CTRL_FOUR},
    //    {"ACT2-NIM1", 8, NIM_I2C_ADDR_ACT2, "ACT2-NIM1", ONBOARD_DEV, 0, 0, I2C_CTRL_TWELVE},
    //    {"ACT2-POE", 8, POE_I2C_ADDR_ACT2, "ACT2-POE", ONBOARD_DEV, 0, 0, I2C_CTRL_SEVEN},
    /* end of lists */
    {NULL, 0, 0xFF, NULL, 0, 0, 0},
};

/* Sub Menu used for FPGA utility.
 */
static submenu_xtable_t fpga_util_submenu_table[] = {
    {"Program SPI PROM image without header", (type_t(*)())diag_fpga_spi_prog, 0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Program SPI PROM image with header", (type_t(*)())diag_fpga_spi_prog, 1,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Erase/Program Image Upgrade Header", (type_t(*)())diag_fpga_erase_header, 1,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Set FPGA revision and date", (type_t(*)())diag_fpga_set_date_revision, 1,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Set FPGA update flag", (type_t(*)())diag_fpga_set_update_flag, 1,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Display PROM sector", (type_t(*)())diag_fpga_display_sector, 1,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Register Alter Utility", (type_t(*)())diag_fpga_reg_alter,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Register Display Utility", (type_t(*)())diag_fpga_reg_display,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show FPGA Version", (type_t(*)())diag_fpga_ver_display,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Enable/Disable NIM", (type_t(*)())diag_power_ngio,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"FPGA I2C scan", (type_t(*)())diag_fpga_i2c_scan,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Enable SFP Prsnt Output", (type_t(*)())diag_sfp_prsnt_output,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define FPGA_UTIL_SUBMENU_TABLE_SIZE (sizeof(fpga_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_util_primary_items[FPGA_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t fpga_util_secondary_items[FPGA_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t fpga_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fpga_util_primary_items,
};
menuinfo_t *fpga_util_submenup = &fpga_util_subtest_menu;

int diag_fpga_util (void)
{
    build_primary_submenu(fpga_util_submenu_table,
			              FPGA_UTIL_SUBMENU_TABLE_SIZE,
                          "FPGA Utility", &fpga_util_submenup);
    build_secondary_submenu(fpga_util_submenu_table,
                            FPGA_UTIL_SUBMENU_TABLE_SIZE,
                            fpga_util_secondary_items);    
                            
    menu(fpga_util_submenup, fpga_util_secondary_items, '\0');
    return (PASSED);
}

static int diag_fpga_reg_alter (void)
{
    int reg_addr;
    int value;

    printf("\n\nFPGA Register Alter\n\n");
    reg_addr = gethex_answer("Reg offset to write", 0, 0, 0xFFFFFFFF);
    value    = gethex_answer("Enter new value:", 0, 0, 0xFFFFFFFF);

    /* If writing offset 0x4 the flag will be up to let user modify register's value */
    if (reg_addr == FPGA_EXT_RESET_REG) {
        lewis_reset_mask_flag = 1;
    }

    return (diag_fpga_reg_write(reg_addr, value));
}

static int diag_fpga_reg_display (void)
{
    int reg_addr;
    int value;

    printf("\n\nFPGA Register Read\n\n");
    reg_addr = gethex_answer("Reg offset to read", 0, 0, 0xFFFFFFFF);
    if (diag_fpga_reg_read(reg_addr, &value) == FAILED) {
        return (FAILED);
    }
    
    printf("\nRegister @ %#x = %#x\n", reg_addr, value);

    return (PASSED);
}

int diag_fpga_ver_display (void)
{
    unsigned int fpga_ver;
    unsigned int fpga_rec_sts;

    diag_fpga_reg_read(FPGA_HW_TYPE_REV_REG, &fpga_ver);
    diag_fpga_reg_read(FPGA_RECONF_STS_REG, &fpga_rec_sts);

    printf("FPGA Revision: v%d.%d.%d (%s)\n", 
          (fpga_ver & 0x007F0000) >> 16, (fpga_ver & 0x0000FF00) >> 8,
           fpga_ver & 0x000000FF, (fpga_rec_sts & 0x1) ? "Upgrade":"Golden" );

    return (PASSED);
   
}

unsigned int get_diag_fpga_ver (void) 
{
    unsigned int reg_addr = FPGA_MAS_REV_REG;
    unsigned int value;

    if (diag_fpga_reg_read(reg_addr, &value) == FAILED) {
        return (FAILED);
    } else {
        return value; 
    }
}

static int
byteswap32 (int num)
{
    return num;
}

void
unreset_platform_ext_dev (int bit)
{
    assert(dash_fpga);
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    bit = byteswap32(bit);
    sys->ext_rst &= ~bit;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_vm_base
 * Description: get platform vm addr
 *
 * Input: plane, not used
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_vm_base (int plane)
{
    unsigned long addr = 0;

    addr = FPGA_VOL_MON_OFFSET;

    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_intr_ctrl_addr
 * Description: get intr ctrl address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_intr_ctrl_addr(int plane)
{
    unsigned long addr = 0;
    unsigned long offset;

    switch (plane) {
    case FP:
        offset = FPGA_FP_INTR_CTRL_REG_OFFSET;
        break;
    case CP:
        offset = FPGA_CP_INTR_CTRL_REG_OFFSET;
        break;
    case NIOS:
        offset = FPGA_NIOS_INTR_CTRL_REG_OFFSET;
        break;
    default:
        assert(!"invalid backplane type");
    }
    addr = offset;

    return (addr);
}

/*-------------------------------------------------------------------
 *
 * Function : disable_top_cp_intr
 * disable interrupt to intel at top level
 * see fuction get_platform_intr_sts(int bit) for valid parameter.
 * input: bit.
 * INPUT: bit representing interrupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
static void
disable_top_cp_intr (int bit)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);

    bit = byteswap32(bit);
    diag_fpga_reg_nand(addr + FPGA_INT_EN_REG, bit);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_env_mcu_base
 * Description: get platform env mcu addr
 * 0x33000 Environmental MCU download control register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_env_mcu_base (int plane)
{
    unsigned long addr = 0;

    addr = FPGA_ENV_MCU_OFFSET;
    /*
    printf("get_platform_env_mcu_base: base %p + %#x\n",
           (void *)addr, FPGA_ENV_MCU_OFFSET);
    */
    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_mcu_override_intr
 * Description:  disable Environmental MCU interrupt
 * INPUT: dev : mcu interupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_mcu_intr (int dev)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_env_mcu_base(plane);

    dev = byteswap32(dev);

    diag_fpga_reg_nand(mcu_addr + BMC_MCU_INT_EN_REG, 
                       ENV_MCU_RX_DATA | ENV_MCU_TX_DONE);

    diag_fpga_reg_nand(addr + FPGA_MISC_INT_TEST_REG, FPGA_MISC_ENV_MCU);

    disable_top_cp_intr(FPGA_MISC_INTR);
}


/*-------------------------------------------------------------------
 *
 * Function : disable_platform_vm_mcu_override_intr
 * Description:   disable VOLTAGE MCU override interrupt
 * INPUT: dev : vm mcu type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_vm_mcu_intr (int dev)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_vm_base(plane);

    dev = byteswap32(dev);

    diag_fpga_reg_nand(mcu_addr + BMC_MCU_INT_EN_REG, 
                       ENV_MCU_RX_DATA | ENV_MCU_TX_DONE);

    diag_fpga_reg_nand(addr + FPGA_MISC_INT_TEST_REG, FPGA_MISC_VM_MCU);

    disable_top_cp_intr(FPGA_MISC_INTR);
}

void
enable_platform_vm_mcu_intr (int dev)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_vm_base(plane);

    dev = byteswap32(dev);
    diag_fpga_reg_or(addr + FPGA_MISC_INT_TEST_REG, FPGA_MISC_VM_MCU);
    diag_fpga_reg_or(mcu_addr + BMC_MCU_INT_EN_REG, 
                     ENV_MCU_RX_DATA | ENV_MCU_TX_DONE);

    enable_top_cp_intr(FPGA_MISC_INTR);

}
/*-------------------------------------------------------------------
 *
 * Function : enable_top_cp_intr
 * enable interrupt to intel at top level
 * see fuction get_platform_intr_sts(int bit) for valid parameter.
 * input: bit.
 * INPUT: bit representing interrupt type
 * OUTPUT: intr status
 * -------------------------------------------------------------------
*/
static void
enable_top_cp_intr (int bit)
{
    unsigned int plane = get_platform_plane();
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    bit = byteswap32(bit);

    intr_sts_cntl->top_en |= bit;

    uio_enable_intr();
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val("OIR EN", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->oir_en,
                         __LINE__,  __FILE__);
        print_offset_val("OIR STATUS", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->oir_sts,
                     __LINE__,  __FILE__);
        print_offset_val("TOP EN", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->top_en,
                         __LINE__,  __FILE__);
        print_offset_val("TOP STATUS", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->top_sts,
                         __LINE__,  __FILE__);
    }

}
/*-------------------------------------------------------------------
 *
 * Function : enable_platform_mcu_intr
 * Description: enable Environmental MCU interrupt
 * INPUT: bit : mcu interupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_mcu_intr (int dev)
{

    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_env_mcu_base(plane);

    dev = byteswap32(dev);

    diag_fpga_reg_or(mcu_addr + BMC_MCU_INT_EN_REG, 
                       ENV_MCU_RX_DATA | ENV_MCU_TX_DONE);

    diag_fpga_reg_or(addr + FPGA_MISC_INT_TEST_REG, FPGA_MISC_ENV_MCU);

    /*
    print_offset_val("enable_platform_mcu_intr:", (unsigned long)dash_fpga,
                     (unsigned long)&intr_sts_cntl->misc_intr,
                     __LINE__, __FILE__);
    */

    enable_top_cp_intr(FPGA_MISC_INTR);

}

void diag_power_ngio (void)
{
    unsigned int pwr, slot; 
    struct ngio_intf_t *ngio;

    slot = getdec_answer("Enter NIM number [1/2/3]", 1, 1, 3);
    pwr = getdec_answer("Power on or off NIM [1/0]", 0, 0, 1);
    
    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);

    if (pwr) {
        ngio->on(ngio);
        if (ngio->i2c_unreset(ngio)<0) {
            cterr('f', 0, "slot%d power_ok bit not set", ngio->slot);
        }
        ngio->uart_on(ngio);
        ngio->unreset(ngio);
        ngio->pci_rdy(ngio, 1);
    } else { 
        ngio->pci_rdy(ngio, 0);
        ngio->off(ngio);
    }

    return; 
}

void diag_sfp_prsnt_output (void)
{
    unsigned int prsnt_output, slot; 
    int reg_addr, value;

    slot = getdec_answer("Enter SFP number [0/1]", 0, 0, 1);
    prsnt_output = getdec_answer("Enable or Disable SFP [1/0]", 0, 0, 1);
  
    if (slot == 0) {
        reg_addr = FPGA_SFP0_CONF_REG;
    } else {
        reg_addr = FPGA_SFP1_CONF_REG;
    }

    diag_fpga_reg_read(reg_addr, &value);

    if (prsnt_output == 1) {
        value |= SFP_PRSNT_OUTPUT_EN;
        diag_fpga_reg_write(reg_addr, value);
    } else {
        value &= (~SFP_PRSNT_OUTPUT_EN);
        diag_fpga_reg_write(reg_addr, value);
    }
    return; 
}

int diag_fpga_i2c_scan(void) {

    n2g_i2c_if_t i2c_if;
    unsigned int rc, size = sizeof(uint16_t), i;
    uchar d32[80];
    i2c_table_t *dev = NULL;
    int index = 0;

    set_nios_mode(NIOS_DISABLE_MODE);

    testname("FPGA I2C Scan");
    prpass(testpass, "FPGA I2C Scan Test");
    
    printf("\n");
    printf("DEV_NAME         BUS  ADDRESS   DESCRIPTION                           CTRL   STATUS\n");
    

    do {
        dev = &fpga_i2c_table[index];
        
	i2c_if.i2c_bus_type = IOFPGA_I2C;
	i2c_if.i2c_dev = dev->addr;
	i2c_if.i2c_ctrl = dev->ctrl;
	i2c_if.mux = I2C_MUX_ZERO;
	i2c_if.size = sizeof(uint16_t);
	i2c_if.offset = -1;

	printf("%-15s   %x    0x%02x     %-30s         %02d      ",
	       dev->dev_name, dev->bus, dev->addr, dev->desc, dev->ctrl);

	memset(d32, 0, sizeof(d32));
	i2c_if.buf = (char *)d32;

	rc = n2g_i2c_read(&i2c_if);
	if (rc != RC_I2C_OP_OK) {
	    printf("ERR: unable to read i2c\n");
	    index++;
	    continue;
	} else {
	    printf("OK\n");
	}

	if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("\n");
	    for (i = 0; i < size ; i++) {
		printf("0x%02x ", d32[i]);
	    }
	    printf("\n");
	}
	index++;
    } while (fpga_i2c_table[index].dev_name);
    set_nios_mode(NIOS_DIAG_MODE);
    return PASSED;
}

/*---------------------------------------------------------------
$Log: diag_fpga_util.c,v $
Revision 1.3  2016/10/27 03:24:46  iachang
Fixed Lewis issue with FPGA ver1.4

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.17  2016/02/26 02:04:21  benchen2
add sfp present output enable

Revision 1.1.2.16  2016/01/20 22:55:45  huanngo
Fix FPGA I2C scan by removing unexisting devices in the list

Revision 1.1.2.15  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.14  2015/12/16 01:55:53  huanngo
Add support for FPGA I2C device scan utility

Revision 1.1.2.13  2015/10/05 01:17:59  alpeng
ngio en/disable fix utils

Revision 1.1.2.12  2015/10/01 09:19:51  alpeng
set pci ready when USE_PCIE flag turn on

Revision 1.1.2.11  2015/09/26 08:00:41  alpeng
add pci rdy during init ngio

Revision 1.1.2.10  2015/09/25 02:17:57  tirawan
Add Erase/Program FPGA header and display prom sector utility

Revision 1.1.2.9  2015/09/24 05:31:58  tirawan
Add MCU Register Dump Utility

Revision 1.1.2.8  2015/09/21 13:09:16  tirawan
Display temperature sensor and FPGA version during boot up

Revision 1.1.2.7  2015/09/18 02:40:41  tirawan
No support on MCU firmware upgrade for now

Revision 1.1.2.6  2015/09/17 13:04:34  tirawan
Support Cisco FPGA firmware upgrade

Revision 1.1.2.5  2015/09/04 07:56:31  alpeng
add ngio enable/disable utils

Revision 1.1.2.4  2015/07/31 10:39:59  alpeng
first check in for testcard

Revision 1.1.2.3  2015/07/31 07:49:31  hondwang
add mcu firmware upgrade function

Revision 1.1.2.2  2015/07/12 06:52:45  tirawan
Add Console Switch Utility, SPI driver and FPGA programming

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/

