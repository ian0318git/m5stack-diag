/* $Id: platform_sys_clk.c,v 1.2 2018/05/18 09:25:00 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/platform_sys_clk.c,v $
 *------------------------------------------------------------------------------
 * platform_sys_clk.c - Neptune SYS CLK chip IDT8T49N287I. 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include "cpu.h"
#include "platform_i2c.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "cli_cmd.h"
#include "queryflags.h"
#include "goofy_i2c.h"

#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "types.h"
#include "proto.h"
#include "free.h"
#include "defs.h"
#include "error.h"
#include "dev_print.h"
#include "dev_object.h"
#include "common.h"
#include "common_utils.h"
#include "goofy_i2c.h"
#include "byteswap.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "platform_margin_utils.h"


/******************************************************************************
 *                              Function proto
 ******************************************************************************/
static void init_i2c_struct(void);
static n2g_i2c_if_t sys_clk_i2c_if; 
static void read_sys_clk_reg(void); 
static void write_sys_clk_reg(void); 
static int idt286_byte_write (int, uchar);

/******************************************************************************
 *                                  Globals 
 ******************************************************************************/

/******************************************************************************
 *                                    Menus
 ******************************************************************************/ 
/*
 * System CLK Main menu
 */
static submenu_xtable_t sys_clk_menu_table[] = {
    {"Read SYS CLK registers",   (PFT)read_sys_clk_reg,        0,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
    {"Write SYS CLK registers",  (PFT)write_sys_clk_reg,       0,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
};

#define SYS_CLK_MENU_TABLE_SIZE \
        (sizeof(sys_clk_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t sys_clk_menu_primary_items[SYS_CLK_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t sys_clk_menu_secondary_items[SYS_CLK_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo sys_clkdiag = {
  "SYS CLK Utilities Menu",     /* title */
  0,				/* title string added by init_empty_menu */
  (PFT)menu_show_dflags,	/* shows major flags */
  0,				/* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  sys_clk_menu_primary_items,
};
static struct menuinfo *sys_clkdiagp = &sys_clkdiag;

/**************************************************************************
 *
 * Function   : build_sys_clk_menu
 * Description: Build System CLK menu.
 * Inputs     : None.
 * Outputs    : None.
 *
 **************************************************************************
 */
void build_sys_clk_menu (void)
{
    /* init i2c struceture first */
    init_i2c_struct();

    build_primary_submenu(sys_clk_menu_table, SYS_CLK_MENU_TABLE_SIZE,
			  "SYS CLK Utilities Menu", &sys_clkdiagp);
    build_secondary_submenu(sys_clk_menu_table, SYS_CLK_MENU_TABLE_SIZE,
			    sys_clk_menu_secondary_items);

    menu(&sys_clkdiag, sys_clk_menu_secondary_items, 0);
}

/*******************************************************************************
 * 
 * Function   : init_i2c_struct
 * Description: Init Sys CLK device structure
 * Inputs     : i2c_if - Points to I2C API interface struct
 * Outputs    : None
 *
 *******************************************************************************
 */
static void init_i2c_struct (void) 
{
    n2g_i2c_if_t *i2c_if;
    i2c_if = get_n2g_i2c_if(I2C_CTRL_FIVE, 
                            I2C_MUX_ZERO,
                            MB_I2C_ADDR_CLK_GENERATOR); 
    memcpy(&sys_clk_i2c_if, i2c_if,   sizeof(n2g_i2c_if_t)); 
    printf("sys_clk_i2c_if.i2c_dev = 0x%x\n", sys_clk_i2c_if.i2c_dev);
    printf("sys_clk_i2c_if.i2c_ctl = 0x%x\n", sys_clk_i2c_if.i2c_ctrl);
    printf("sys_clk_i2c_if.bus type= 0x%x\n", sys_clk_i2c_if.i2c_bus_type);
}


/**************************************************************************
 *
 * Function   : read_sys_clk_reg
 * Description: Read SYS CLK registers value
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void read_sys_clk_reg (void) 
{
    unsigned int offset, rc, size, ia; 
    unsigned char d8[2], r8[8];

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFFFF);
    size = gethex_answer("Enter read size", 0, 0, 0x8);


    /* based on the datasheet, we must write MSB and LSB for offset
     * before reading */
    sys_clk_i2c_if.size = 2; 
    d8[0] = (offset & 0xFF00) << 16;  /* offset MSB */
    d8[1] = (offset & 0x00FF);  /* offset LSB */

    sys_clk_i2c_if.buf = (char *)d8; 
    rc = n2g_i2c_write(&sys_clk_i2c_if); 
    if (rc != RC_I2C_OP_OK) {
        printf("unable to write i2c.\n");
        return;
    }
  
    sys_clk_i2c_if.size = size; 
    sys_clk_i2c_if.buf = (char *)r8; 
    rc = n2g_i2c_read(&sys_clk_i2c_if); 
    if (rc != RC_I2C_OP_OK) {
        printf("unable to read i2c\n");
        return;
    }

    printf("\n");
    for (ia = 0; ia < size ; ia++) {
        printf("0x%02x ", r8[ia]);
    }

} 

/**************************************************************************
 *
 * Function   : write_sys_clk_reg
 * Description: Write SYS CLK registers value
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void write_sys_clk_reg (void) 
{
    unsigned int offset, rc, data;
    unsigned char d8[3];

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFFFF);
    data = gethex_answer("Enter write data", 0, 0, 0xFF);

    /* based on the datasheet, we must write MSB and LSB for offset
     * before reading */
    sys_clk_i2c_if.size = 3;
    d8[0] = (offset & 0xFF00) << 16;  /* offset MSB */
    d8[1] = (offset & 0x00FF);  /* offset LSB */
    d8[2] = data; 

    sys_clk_i2c_if.buf = (char *)d8;
    rc = n2g_i2c_write(&sys_clk_i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to write i2c.\n");
    } else {
        printf("Write OP done.      \n");
    }

}

int idt286_byte_read (int offset, uchar *data)
{
    unsigned int rc, size = 1;
    unsigned char d8[2];

    /* based on the datasheet, we must write MSB and LSB for offset
     * before reading */
    sys_clk_i2c_if.size = 2;
    d8[0] = (offset & 0xFF00) << 16;  /* offset MSB */
    d8[1] = (offset & 0x00FF);  /* offset LSB */

    sys_clk_i2c_if.buf = (char *)d8;
    rc = n2g_i2c_write(&sys_clk_i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to write i2c.\n");
        return (rc);
    }

    sys_clk_i2c_if.size = size;
    sys_clk_i2c_if.buf = (char *)data;
    rc = n2g_i2c_read(&sys_clk_i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to read i2c\n");
        return (rc);
    }

    return (rc);
}

static int idt286_byte_write (int offset, uchar data)
{
	int rc;
	unsigned char d8[3];

    /* based on the datasheet, we must write MSB and LSB for offset
     * before reading */
    sys_clk_i2c_if.size = 3;
    d8[0] = (offset & 0xFF00) << 16;  /* offset MSB */
    d8[1] = (offset & 0x00FF);  /* offset LSB */
    d8[2] = data;

    sys_clk_i2c_if.buf = (char *)d8;
    rc = n2g_i2c_write(&sys_clk_i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to write i2c.\n");
    }

    return (rc);
}

int diag_freq_idt_margin (int mrgn)
{
    /* init i2c struceture first */
    init_i2c_struct();

    printf("Margin Frequency to ");
    switch (mrgn) {
    case FREQ_MARG_HIGH:
        printf("+5%% - ");
    	idt286_byte_write(0x3D, 0xD0); /* Enable CLK1 selection for PLL1 */
    	idt286_byte_write(0x64, 0x00); /* Integer Portion of FB for APLL1 */
    	idt286_byte_write(0x65, 0x31);

    	idt286_byte_write(0x66, 0x00); /* Fractional Portion of FB for APLL1 */
    	idt286_byte_write(0x67, 0x00);
    	idt286_byte_write(0x68, 0x00);

    	idt286_byte_write(0x41, 0x00); /* Feedback Divider for CLK1 */
    	idt286_byte_write(0x42, 0x01);
    	idt286_byte_write(0x43, 0x09);
    	idt286_byte_write(0x4D, 0x00);
    	idt286_byte_write(0x4E, 0x79);
    	idt286_byte_write(0x4F, 0xBC);

    	idt286_byte_write(0x8F, 0x02); /* Output Divider for Q6 */
    	idt286_byte_write(0x90, 0x00);
    	idt286_byte_write(0x91, 0x0E);

    	idt286_byte_write(0x92, 0x02); /* Output Divider for Q7 */
    	idt286_byte_write(0x93, 0x00);
    	idt286_byte_write(0x94, 0x0E);

    	idt286_byte_write(0xAA, 0x11); /* Select PLL1 as Q6 and Q7 source */
    	idt286_byte_write(0xB1, 0x00); /* PLL1 as JA mode */
    	idt286_byte_write(0xB6, 0x00);
    	idt286_byte_write(0xB8, 0x00);
        break;
    case FREQ_MARG_LOW:
        printf("-5%% - ");
    	idt286_byte_write(0x64, 0x00); /* Integer Portion of FB for APLL1 */
    	idt286_byte_write(0x65, 0x2F);
    	idt286_byte_write(0x66, 0x10); /* Fractional Portion of FB for APLL1 */
    	idt286_byte_write(0x67, 0x00);
    	idt286_byte_write(0x68, 0x00);

    	idt286_byte_write(0x41, 0x00); /* Feedback Divider for CLK1 */
    	idt286_byte_write(0x42, 0x01);
    	idt286_byte_write(0x43, 0x05);
    	idt286_byte_write(0x4D, 0x00);
    	idt286_byte_write(0x4E, 0x74);
    	idt286_byte_write(0x4F, 0x3A);

    	idt286_byte_write(0x8F, 0x01); /* Output Divider for Q6 */
    	idt286_byte_write(0x90, 0x00);
    	idt286_byte_write(0x91, 0x0A);

    	idt286_byte_write(0x92, 0x01); /* Output Divider for Q7 */
    	idt286_byte_write(0x93, 0x00);
    	idt286_byte_write(0x94, 0x0A);

    	idt286_byte_write(0xAA, 0x11); /* Select PLL1 as Q6 and Q7 source */
    	idt286_byte_write(0xB1, 0x00); /* PLL1 as JA mode */
    	idt286_byte_write(0xB6, 0x00);
    	idt286_byte_write(0xB8, 0x00);
        break;
    case FREQ_MARG_NORM:
        printf("Normal - ");
    	idt286_byte_write(0x3D, 0xD0); /* Enable CLK1 selection for PLL1 */
    	idt286_byte_write(0x64, 0x00); /* Integer Portion of FB for APLL1 */
    	idt286_byte_write(0x65, 0x32);

    	idt286_byte_write(0x66, 0x00); /* Fractional Portion of FB for APLL1 */
    	idt286_byte_write(0x67, 0x00);
    	idt286_byte_write(0x68, 0x00);

    	idt286_byte_write(0x41, 0x00); /* Feedback Divider for CLK1 */
    	idt286_byte_write(0x42, 0x01);
    	idt286_byte_write(0x43, 0x05);
    	idt286_byte_write(0x4D, 0x00);
    	idt286_byte_write(0x4E, 0x7A);
    	idt286_byte_write(0x4F, 0x58);

    	idt286_byte_write(0x8F, 0x01); /* Output Divider for Q6 & Q7 */
    	idt286_byte_write(0x90, 0x00);
    	idt286_byte_write(0x91, 0x0A);
    	idt286_byte_write(0x92, 0x01);
    	idt286_byte_write(0x93, 0x00);
    	idt286_byte_write(0x94, 0x0A);

    	idt286_byte_write(0xAA, 0x11); /* Select PLL1 as Q6 and Q7 source */
    	idt286_byte_write(0xB1, 0x00); /* PLL1 as JA mode */
    	idt286_byte_write(0xB6, 0x00);
    	idt286_byte_write(0xB8, 0x00);
        break;
    default:
        break;
    }
    return (PASSED);
}

int sq420d_get_reg (int reg_offset, char *val_buf)
{

   printf("fix me  %s \n", __FUNCTION__);
   return 0; 

}

int sq420d_set_freq (char desired_freq, boolean spread, boolean mode)
{
   printf("fix me  %s \n", __FUNCTION__);
   return 0; 
}

/******** History ********
*----------------------------------------------------
$Log: platform_sys_clk.c,v $
Revision 1.2  2018/05/18 09:25:00  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.4  2017/05/08 01:21:40  meho
Added freq margining utility.

Revision 1.1.2.3  2017/02/18 03:37:50  meho
Added voltage margin utility.

Revision 1.1.2.2  2017/02/09 07:09:03  meho
Fixed i2c r/w bug.

Revision 1.1.2.1  2016/12/26 12:50:47  alpeng
add sys clk into i2c utility



$Endlog$
*/
