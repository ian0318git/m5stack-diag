/* $Id: diag_margin_util.c,v 1.4 2016/05/05 01:01:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_margin_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_margin_util.c - Margin Utility
 * 
 * November 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
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
#include "diag_fpga_lib.h"
#include "platform_i2c.h"
#include "diag_i2c_lib.h"
#include "diag_margin_util.h"

int diag_margin_util(void);
static void diag_volt_margin(int);
static void diag_disp_volt_margin(int);
static void diag_freq_margin(int);
static int diag_freq_idt_margin(ulong, uchar *);

extern ulong typ_size;
extern ulong pppm_ppctg_size;
extern ulong nppm_npctg_size;

extern uchar typ[];
extern uchar pppm_ppctg[];
extern uchar nppm_npctg[];

/* Sub Menu used for Margin utility.
 */
static submenu_xtable_t margin_util_submenu_table[] = {
    {"Voltage margin 1.5V High", (type_t(*)())diag_volt_margin, VOLT_MARG_1P5_HIGH,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Voltage margin 1.5V Low", (type_t(*)())diag_volt_margin, VOLT_MARG_1P5_LOW,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Voltage margin 1.5V Normal", (type_t(*)())diag_volt_margin, VOLT_MARG_1P5_NORM,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Voltage margin 3.3V High", (type_t(*)())diag_volt_margin, VOLT_MARG_3P3_HIGH,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Voltage margin 3.3V Low", (type_t(*)())diag_volt_margin, VOLT_MARG_3P3_LOW,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Voltage margin 3.3V Normal", (type_t(*)())diag_volt_margin, VOLT_MARG_3P3_NORM,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Display Voltage Margin", (type_t(*)())diag_disp_volt_margin, 0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Frequency margin +5%%", (type_t(*)())diag_freq_margin, FREQ_MARG_HIGH,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Frequency margin -5%%", (type_t(*)())diag_freq_margin, FREQ_MARG_LOW,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Frequency margin Normal", (type_t(*)())diag_freq_margin, FREQ_MARG_NORM,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define MARGIN_UTIL_SUBMENU_TABLE_SIZE (sizeof(margin_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t margin_util_primary_items[MARGIN_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t margin_util_secondary_items[MARGIN_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t margin_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    margin_util_primary_items,
};
menuinfo_t *margin_util_submenup = &margin_util_subtest_menu;


int diag_margin_util (void)
{
    build_primary_submenu(margin_util_submenu_table,
			              MARGIN_UTIL_SUBMENU_TABLE_SIZE,
                          "Margin Utility", &margin_util_submenup);
    build_secondary_submenu(margin_util_submenu_table,
                            MARGIN_UTIL_SUBMENU_TABLE_SIZE,
                            margin_util_secondary_items);    
                            
    menu(margin_util_submenup, margin_util_secondary_items, '\0');
    return (PASSED);
}


static void diag_volt_margin (int arg)
{
    switch (arg) {
    case VOLT_MARG_3P3_NORM:
    case VOLT_MARG_3P3_HIGH:
    case VOLT_MARG_3P3_LOW:
        diag_margin_fpga_reg_nand(FPGA_VOLT_MARG_REG, VOLT_MARG_MASK);
        printf("Margin 3.3V to ");
        if (arg == VOLT_MARG_3P3_HIGH) {
            printf("High\n");
            diag_fpga_reg_or(FPGA_VOLT_MARG_REG, VOLT_MARG_TUNE | VOLT_MARG_HIGH);
        } else if (arg == VOLT_MARG_3P3_LOW) {
            printf("Low\n");
            diag_fpga_reg_or(FPGA_VOLT_MARG_REG, VOLT_MARG_TUNE | VOLT_MARG_LOW);
        } else {
            printf("Normal\n");
            diag_fpga_reg_or(FPGA_VOLT_MARG_REG, VOLT_MARG_TUNE);
        }
        break;
    case VOLT_MARG_1P5_NORM:
    case VOLT_MARG_1P5_HIGH:
    case VOLT_MARG_1P5_LOW:
        diag_margin_fpga_reg_nand(FPGA_VOLT_MARG_REG, VOLT_MARG_MASK << VOLT_1P5_SHIFT);
        printf("Margin 1.5V to ");
        if (arg == VOLT_MARG_1P5_HIGH) {
            printf("High\n");
            diag_fpga_reg_or(FPGA_VOLT_MARG_REG, VOLT_MARG_TUNE | (VOLT_MARG_HIGH << VOLT_1P5_SHIFT));
        } else if (arg == VOLT_MARG_1P5_LOW) {
            printf("Low\n");
            diag_fpga_reg_or(FPGA_VOLT_MARG_REG, VOLT_MARG_TUNE | (VOLT_MARG_LOW << VOLT_1P5_SHIFT));
        } else {
            printf("Normal\n");
            diag_fpga_reg_or(FPGA_VOLT_MARG_REG, VOLT_MARG_TUNE);
        }
        break;
    default:
        break;
    }
}

static void diag_disp_volt_margin (int arg)
{
    int data;

    diag_fpga_reg_read(FPGA_VOLT_MARG_REG, &data);

    printf("Voltage Margin Status:\n");
    printf("1.5V: ");
    if (((data >> VOLT_1P5_SHIFT) & VOLT_MARG_MASK) == VOLT_MARG_LOW) {
        printf("Low\n");
    } else if (((data >> VOLT_1P5_SHIFT) & VOLT_MARG_MASK) == VOLT_MARG_HIGH) {
        printf("High\n");
    } else {
        printf("Normal\n");
    }

    printf("3.3V: ");
    if ((data & VOLT_MARG_MASK) == VOLT_MARG_LOW) {
        printf("Low\n");
    } else if ((data & VOLT_MARG_MASK) == VOLT_MARG_HIGH) {
        printf("High\n");
    } else {
        printf("Normal\n");
    }
}

static void diag_freq_margin (int arg)
{
    printf("Margin Frequency to ");
    switch (arg) {
    case FREQ_MARG_HIGH:
        printf("+5%% - ");
        if (diag_freq_idt_margin(pppm_ppctg_size, pppm_ppctg) == PASSED) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
        }
        break;
    case FREQ_MARG_LOW:
        printf("-5%% - ");
        if (diag_freq_idt_margin(nppm_npctg_size, nppm_npctg) == PASSED) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
        }
        break;
    case FREQ_MARG_NORM:
        printf("Normal - ");
        if (diag_freq_idt_margin(typ_size, typ) == PASSED) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
        }
        break;
    default:
        break;
    }
}

static int diag_freq_idt_margin (ulong arr_size, uchar *arr)
{
    int ix;
    uchar data;
    int offset = MB_IDT286_PLL0_CTRL_REG;

    for (ix = 0; ix < arr_size ;ix++) {
        diag_i2c_byte_write(I2C_CTRL_THREE, MB_I2C_IDT286, offset + ix, arr[ix]);
    }
    
    for (ix = 0; ix < arr_size ; ix++) {
        diag_i2c_byte_read(I2C_CTRL_THREE, MB_I2C_IDT286, offset + ix, &data);
        if (arr[ix] != data) {
            printf("\n0x%x expected 0x%x  actual 0x%xi\n", ix, arr[ix], data);
            return (FAILED);
        }
    }
    
    return (PASSED);

}


