/* $Id: platform_gh_gesw_clk.c,v 1.1 2014/08/14 10:26:59 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_gh_gesw_clk.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_gh_gesw_clk.c 
 * Description: Utah Greyhound GE switch clock generator, IDT8T49N4811, 
 *              related diag tests and utilities.
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "proto.h"
#include "i2c_api.h"
#include "menu.h"
#include "queryflags.h"
#include "i2c_address.h"
#include "platform_i2c.h"
#include "platform_gh_gesw_clk.h"


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */


/*******************************************************************************
 *                              Function Prototype                             *
 *******************************************************************************
 */
void build_gh_gesw_clk_menu(uint32_t);
static int get_gh_gesw_clk_i2c_struct(n2g_i2c_if_t *);
static uint32_t gh_gesw_clk_i2c_read(n2g_i2c_if_t *);
static uint32_t gh_gesw_clk_i2c_write(n2g_i2c_if_t *);
static int display_reg(void);
static void alter_reg(void);

/*******************************************************************************
 *                              Menu Related.                                  *
 *******************************************************************************
 */
/*
 * Greyhound GESW CLK Main menu
 */
static submenu_xtable_t gh_gesw_clk_menu_table[] = {
    {"Show Greyhound GESW CLK registers",  (PFT)display_reg,             0,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
    {"Alter Greyhound GESW CLK registers", (PFT)alter_reg,               0,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
};

#define GH_GESW_CLK_MENU_TABLE_SIZE \
        (sizeof(gh_gesw_clk_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gh_gesw_clk_menu_primary_items[GH_GESW_CLK_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t gh_gesw_clk_menu_secondary_items[GH_GESW_CLK_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo gh_gesw_clkdiag = {
  "Greyhound GESW CLK Utilities Menu",    /* title */
  0,                            /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,        /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  gh_gesw_clk_menu_primary_items,
};
static struct menuinfo *gh_gesw_clkdiagp = &gh_gesw_clkdiag;


/*******************************************************************************
 *
 * Function   : build_gh_gesw_clk_menu
 * Description: Build Greyhound GESW CLK, IDT8T49N4811, menu.
 * Inputs     : None.
 * Outputs    : None.
 *
 *******************************************************************************
 */
void build_gh_gesw_clk_menu (uint32_t menu_opt)
{
    build_primary_submenu(gh_gesw_clk_menu_table, GH_GESW_CLK_MENU_TABLE_SIZE,
                          "Greyhound GESW CLK Utilities Menu", &gh_gesw_clkdiagp);
    build_secondary_submenu(gh_gesw_clk_menu_table, GH_GESW_CLK_MENU_TABLE_SIZE,
                            gh_gesw_clk_menu_secondary_items);

    /* Entered with submenu */
     menu(&gh_gesw_clkdiag, gh_gesw_clk_menu_secondary_items, 0);

    return;
}


/*******************************************************************************
 *
 * Function   : get_gh_gesw_clk_i2c_struct
 * Description: To get gh_gesw clk I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_gh_gesw_clk_i2c_struct (n2g_i2c_if_t *gh_gesw_clk_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_SIXTEEN, I2C_MUX_ZERO,
                                         MB_I2C_ADDR_GH_GESW_CLK);

    if (tmp == NULL) {
        printf("%s: Failed to get Greyhound GESW CLK Gen I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(gh_gesw_clk_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : gh_gesw_clk_i2c_read
 * Description: To read the expected offset register data out.
 * Inputs     : *i2c_if - Pointer to the expected I2C interface structure
 *              offset - the offset of the expected register to read
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t gh_gesw_clk_i2c_read (n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    i2c_if->offset = -1;
	
    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        /* Unable to read data */
        printf("*** %s: Unable to read %s Register 0x%02x(rc = %#x).\n",
               __FUNCTION__, i2c_if->dev_name, i2c_if->offset, rc);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : gh_gesw_clk_i2c_write
 * Description: To write the data into expected register.
 * Inputs     : *i2c_if - Pointer to the expected I2C interface structure
 *              offset - the offset of the expected register to write
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t gh_gesw_clk_i2c_write (n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    i2c_if->offset = -1; 

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        /* Unable to read data */
        printf("*** %s: Unable to write %s Register 0x%02x(rc = %#x).\n",
               __FUNCTION__, i2c_if->dev_name, i2c_if->offset, rc);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : display_reg
 * Description: To read the register on gh_gesw clock 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int display_reg (void)
{
    char i2c_data_val[32];
    int  rc;
    n2g_i2c_if_t i2c_if;


    /* Get Greyhound GESW clk I2C interface structure */
    if (get_gh_gesw_clk_i2c_struct(&i2c_if) != PASSED) {
        printf("%s:%d Failed to get sensor I2C structure.",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    memset(i2c_data_val, 0, sizeof(i2c_data_val));

    i2c_if.buf = i2c_data_val;
    i2c_if.size = IDT8T49N4811_GH_GESW_CLK_REG_SIZE; /* 0x6 */

    rc = gh_gesw_clk_i2c_read(&i2c_if); 
    if (rc == FAILED) {
        printf("Failed for read from gh_gesw clk, IDT8T49N4811 \n");
        return (FAILED);
    } else { 

        printf("Frequency Selection Register, Output     = 0x%x \n", i2c_data_val[0]);
        printf("Frequency Selection Register, Misc.      = 0x%x \n", i2c_data_val[1]);
        printf("Output Enable Bank A and B Register      = 0x%x \n", i2c_data_val[2]);
        printf("Output Enable Bank C and D. And Type Reg.= 0x%x \n", i2c_data_val[3]);
        printf("Output Type Select Bank A and B          = 0x%x \n", i2c_data_val[4]);
        printf("Misc. Control                            = 0x%x \n", i2c_data_val[5]);

        return (PASSED);
    }
   
}

/*******************************************************************************
 *
 * Function   : alter_reg
 * Description: To alter the register on gh_gesw clock
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static void alter_reg (void)
{
    char i2c_data_val[32], c;
    n2g_i2c_if_t i2c_if;
    ushort wrval, rdval, byte;


    /* Get Greyhound GESW clk I2C interface structure */
    if (get_gh_gesw_clk_i2c_struct(&i2c_if) != PASSED) {
        printf("%s:%d Failed to get sensor I2C structure.",
               __FUNCTION__, __LINE__);
        return;
    }

    memset(i2c_data_val, 0, sizeof(i2c_data_val));
    i2c_if.buf = i2c_data_val;
    i2c_if.size = IDT8T49N4811_GH_GESW_CLK_REG_SIZE; /* 0x6 */

    do {
        byte = gethex_answer("\nEnter reg byte", 0, 0, 0x5);
        gh_gesw_clk_i2c_read(&i2c_if); 
        rdval = i2c_data_val[byte] & 0xFF; 
        printf("Current value of byte 0x%x = 0x%x\n", byte, rdval);

        c = getc_answer("Do you want to change value?", "yn",'n');

        if (c == 'y') {
            wrval = gethex_answer("Enter value:", 0, 0, 0xff);
            i2c_data_val[byte] = wrval; 
            gh_gesw_clk_i2c_write(&i2c_if); 

            gh_gesw_clk_i2c_read(&i2c_if); 

            rdval = i2c_data_val[byte] & 0xFF; 
            printf("Read back byte 0x%x = 0x%x\n", byte, rdval); 
        }
    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return;

}

/*------------------------------------------------------------------
$Log: platform_gh_gesw_clk.c,v $
Revision 1.1  2014/08/14 10:26:59  alpeng
support greyhound gesw clk gen on i2c scan test and its util



$Endlog$
*/
