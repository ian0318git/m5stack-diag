 /*------------------------------------------------------------------
 * Filename:	platform_124mux.c
 *
 * Description: TI PCA9545A 1:4 Mux I2C device.
 *              This file is ported from Cavium
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "string.h"
#include "endians.h"
#include "common.h"
#include "dev_pca9545.h"
#include "pca9545a.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "goofy_i2c.h"
#include "dash_fpga.h"
#include "i2c_address.h"

/*****************************************************************************
 *                         Function prototypes 
 *****************************************************************************/

static int show_mux(uint32_t mux_id);
static int alter_mux(uint32_t mux_id);
int init_mux(uint32_t mux_id);
int test_mux(uint32_t mux_id);


/*****************************************************************************
 *                               Externs
 *****************************************************************************/
extern unsigned char i2c_debug;
extern void set_mux_shadow (char pattern, int mux);


/*****************************************************************************
 *                           Global variables 
 *****************************************************************************/
char mux_err_buf[ERR_BUF_SIZE];

/*
 * Control Register Bit Table
 */
static dev_pca_desc_t mux_bit_desc[] = {
    {"MUX0", "Gated", "Unconnected", MUX9545_PORT0_MASK},
    {"MUX1", "Gated", "Unconnected", MUX9545_PORT1_MASK},
    {"MUX2", "Gated", "Unconnected", MUX9545_PORT2_MASK},
    {"MUX3", "Gated", "Unconnected", MUX9545_PORT3_MASK},
    {0, 0, 0, 0},
};


/*****************************************************************************
 *                                 Menus
 *****************************************************************************/
/*
 * 1:4 Mux Menu
 */
static submenu_xtable_t sfp_mux_menu_table[] = {
    {"Show Mux Control Register", (PFT)show_mux, 0, 0,
	(type_t(*)())0, 0, (PFT)show_mux, 0},
    {"Alter Mux Control Register", (PFT)alter_mux, 0, 0,
	(type_t(*)())0, 0, (PFT)alter_mux, 0},
    {"Clear Control Register", (PFT)init_mux, 0, 0,
	(type_t(*)())0, 0, (PFT)init_mux, 0},
};

static submenu_xtable_t psu_mux_menu_table[] = {
    {"Show Mux Control Register", (PFT)show_mux, 1, 0,
	(type_t(*)())0, 0, (PFT)show_mux, 1},
    {"Alter Mux Control Register", (PFT)alter_mux, 1, 0,
	(type_t(*)())0, 0, (PFT)alter_mux, 1},
    {"Clear Control Register", (PFT)init_mux, 1, 0,
	(type_t(*)())0, 0, (PFT)init_mux, 1},
};

/* size of sfp_mux_menu_table is equal to psu_mux_menu_table */
#define PCA_MENU_TABLE_SIZE (sizeof(sfp_mux_menu_table) / \
		sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mux_menu_primary_items[PCA_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];
static mitem_t mux_menu_secondary_items[PCA_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];

static struct menuinfo muxdiag = {
    "1:4 Mux Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    mux_menu_primary_items,
};

static struct menuinfo *muxdiagp = &muxdiag;

/*****************************************************************************
 *
 * Function   : build_mux_menu
 * Description:	Build PCA9545a 1:4 Mux sub-menu.
 * Inputs     : Mux ID (0: SFP MUX, 1: PSU MUX)
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int build_mux_menu (uint32_t mux_id)
{
    submenu_xtable_t *mux_menu_table;

    testname("PCA9545a 1:4 Mux");
    
    /* get mux_menu_table */
    mux_menu_table = (mux_id == OVLD_SFP_I2C_MUX ? sfp_mux_menu_table : psu_mux_menu_table); 

    build_primary_submenu(mux_menu_table, PCA_MENU_TABLE_SIZE,
                          "1:4 Mux Utility Menu", &muxdiagp);
    build_secondary_submenu(mux_menu_table, PCA_MENU_TABLE_SIZE,
                            mux_menu_secondary_items);
    menu(&muxdiag, mux_menu_secondary_items, 0);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : set_i2c_if_struct
 * Description:	fill n2g_i2c_if_t struct based on different mux_id.
 * Inputs     : Mux ID: 0. SFP MUX, 1. PSU MUX
 *              i2c_if_p: pointer to n2g_i2c_if_t struct
 *              buf_p: pointer to i2c tx/rx buffer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int set_i2c_if_struct(uint32_t mux_id, n2g_i2c_if_t* i2c_if_p, char* buf_p)
{
    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
    i2c_if_p->offset = 0xFFFFFFFF;
    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
    i2c_if_p->size = sizeof(pca_t);
    i2c_if_p->buf = buf_p;
    switch (mux_id) {
        case OVLD_SFP_I2C_MUX:
            i2c_if_p->mux = I2C_MUX_ZERO;
            i2c_if_p->i2c_ctrl = I2C_CTRL_SEVENTEEN;
            i2c_if_p->i2c_dev = MB_I2C_ADDR_SFP_I2C_MUX;
            break;
        case OVLD_PSU_I2C_MUX:
            i2c_if_p->mux = I2C_MUX_ZERO;
            i2c_if_p->i2c_ctrl = I2C_CTRL_FOUR;
            i2c_if_p->i2c_dev = MB_I2C_ADDR_PSU_I2C_MUX;
            break;
        default:
            return FAILED;
    }
    return PASSED;
}


/*****************************************************************************
 * 
 * Function   : init_pca9545a
 * Description:	Initilialize PCA9545A control register.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 *              Control registeControl register to be initialized to is 
 *              also passed in
 *              mux_id: 0: SFP I2C mux, 1: PSU I2C mux
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint32 init_pca9545a (n2g_i2c_dev_t *dev, pca_t init, uint32_t mux_id)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;
    pca_t ctrl;

    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));
    
    ctrl = init;
    /* Setup I2C API interface struct */
	set_i2c_if_struct(mux_id, &i2c_if, (char *)&ctrl);
    
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        sprintf(mux_err_buf, "%s:%d I2C write failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */
    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : init_mux
 * Description:	Wrap function to init 1:4 Mux Control Register.
 * Inputs     : mux_id - 0: SFP mux, 1: PSU mux.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int init_mux (uint32_t mux_id)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    
    /* Init MUX (TI PCA9545a) */
    rc = init_pca9545a(&i2c_dev, (pca_t)0, mux_id);
    if (rc != PASSED) {
        cterr('f', 0, "%s", mux_err_buf);
    }

    return (rc);
}


/*****************************************************************************
 *
 * Function   : show_pca9545a_reg
 * Description:	Provide platforms with a mechanism to display some common
 *		        device information via the device print function argument.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 *              dev_pca_desc_t pointer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint32 show_pca9545a_reg (n2g_i2c_dev_t *dev, dev_pca_desc_t *pdesc, uint32_t mux_id)
{
    uint32 rc = FAILED;
    pca_t ctrl;	/* Control register from PCA9545A */
    n2g_i2c_if_t i2c_if;
    
    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));

    /* Setup I2C API interface struct */
	set_i2c_if_struct(mux_id, &i2c_if, (char *)&ctrl);
    
    /* Read the control register of PCA9545A */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        sprintf(mux_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    }

    printf("\nPCA9545a 1:4 Mux Control register = %02x:\n", ctrl);

    while(pdesc && pdesc->name) {
        printf(" %s - ", pdesc->name);
        if (ctrl & pdesc->mask) {
            printf("%s\n", pdesc->true);
        } else {
            printf("%s\n", pdesc->false);
        }
        pdesc++;   /* points to the next field */
    } /* endof while */

    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : show_mux
 * Description:	Display 1:4 Mux Control Register.
 * Inputs     : mux_id
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int show_mux (uint32_t mux_id)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    dev_pca_desc_t *pca_desc_p;

    /* Setup descriptor table */
    pca_desc_p = (dev_pca_desc_t *)&mux_bit_desc;

    /* Display register contents */
    rc = show_pca9545a_reg(&i2c_dev, pca_desc_p, mux_id);
    if (rc != PASSED) {
        cterr('f', 0, "%s", mux_err_buf);
	}
    return (rc);
}


/*****************************************************************************
 *
 * Function   : alter_pca9545a_ctrl_reg
 * Description:	Peek-n-poke PCA9545A control register.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 *              mux_id
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int alter_pca9545a_ctrl_reg (n2g_i2c_dev_t *dev, uint32_t mux_id)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    pca_t old_ctrl, new_ctrl;

    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));
    
    /* Setup I2C API interface struct - read the register first*/
	set_i2c_if_struct(mux_id, &i2c_if, (char *)&old_ctrl);

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        sprintf(mux_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    new_ctrl = gethex_answer("Enter the new control register", old_ctrl,
                             PCA9545_CTRL_MIN, PCA9545_CTRL_MAX);

    /* Write the new data */
    i2c_if.buf = (char *)&new_ctrl;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        sprintf(mux_err_buf, "%s:%d I2C write failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */
    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : alter_mux
 * Description: Alter 1:4 Mux Control Register.
 * Inputs     : mux_id 
 * Outputs    : PASSED/FAILED.
 *
 *****************************************************************************/
static int alter_mux (uint32_t mux_id)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    
    /* Alter PCA9545a Ctrol Reg */
    rc = alter_pca9545a_ctrl_reg(&i2c_dev, mux_id);
    if (rc != PASSED) {
        cterr('f', 0, "%s", mux_err_buf);
	}
    return (rc);
}


/*****************************************************************************
 *
 * Function   : test_pca9545a
 * Description:	Test PCA9545A control register.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int test_pca9545a (n2g_i2c_dev_t *dev, uint32_t mux_id)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;
    pca_t ctrl;
    uint8_t chan;

    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));

    /* Setup I2C API interface struct */
	set_i2c_if_struct(mux_id, &i2c_if, (char *)&ctrl);
    
    for (chan = PCA9545_B0; chan <= PCA9545_B3; chan = (chan << 1)) {
        ctrl = chan;
        rc = n2g_i2c_write(&i2c_if);
        if (rc != PASSED) {
            sprintf(mux_err_buf, "%s:%d I2C write failed(rc = %#x).",
                                 __FUNCTION__, __LINE__, rc);
            return (FAILED);
        } /* endof if rc */

        ctrl = 0; // clear the buffer before read
        rc = n2g_i2c_read(&i2c_if);
        if (rc == PASSED) {
	        if (ctrl != chan)  {
	            sprintf(mux_err_buf, "%s:%d I2C read back data mismatch."
                                     " Exp %#x Act %#x", __FUNCTION__,
                                     __LINE__, chan, ctrl);
                return (FAILED);
            }
        } else {
            sprintf(mux_err_buf, "%s:%d I2C read failed(rc = %#x).",
                                 __FUNCTION__, __LINE__, rc);
            return (FAILED);
        }
    }
    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : test_mux
 * Description:	Wrap function to test 1:4 Mux Control Register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED.
 *
 *****************************************************************************/
int test_mux(uint32_t mux_id)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;

    /* Test the Mux Control reg */
    rc = test_pca9545a(&i2c_dev, mux_id);
    if (rc != PASSED) {
        cterr('f', 0, "%s", mux_err_buf);
	}
    return (rc);
}


/******************************************************************************
 *
 * Function   : get_mux_mask
 * Description: Get the Mux Mask of the selected device.
 * Inputs     : dev_no - Number of the selected device
 * Outputs    : SFP/PSU MASK, return 0 if invalid
 *  
 ******************************************************************************/
int get_mux_mask (uint dev_no)
{   
    switch (dev_no) {
    case SFP_PSU_ZERO:
        return I2C1_MUX_PORT0_MASK;
    case SFP_PSU_ONE:
        return I2C1_MUX_PORT1_MASK;
    case SFP_PSU_TWO:
        return I2C1_MUX_PORT2_MASK;
    case SFP_PSU_THREE:
        return I2C1_MUX_PORT3_MASK;
    default:
        printf("%s:%d Unknown SFP/PSU No.\n", __FUNCTION__, __LINE__);
        return 0;
    }
}


/******************************************************************************
 *
 * Function   : set_mux_channel
 * Description: This function set the mux channel for the slave device.
 * Inputs     : i2c_p - Pointer to the N2G I2C API interface struct
 *                      Fields needed in the struct are:
 *                      i2c_bus_type, i2c_dev
 *              mux_mask - data for mux setup
 *              mux_id - 0: SFP mux, 1: PSU mux
 * Outputs    : PASSED or I2C error code
 *
 ******************************************************************************/
int set_mux_channel (n2g_i2c_dev_t *i2c_p, uint8_t mux_mask, uint32_t mux_id)
{
    uint rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Init Mux */
    rc = init_mux(mux_id);
    if (rc != PASSED) {
        /* Unable to init Mux */
        cterr('f', 0, "%s:%d Mux %d init failed (rc = %#x).",
                      __FUNCTION__, __LINE__, mux_id, rc);
        return (rc);
    }

    /* Setup I2C API interface struct */
    set_i2c_if_struct(mux_id, &i2c_if, (char *)&mux_mask);

    /* set Mux channel */
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        /* Unable to set Mux */
        cterr('f', 0, "%s:%d set Mux failed (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
    }
    return (rc);
}

/*
 *-----------------------------------------------------------------------------
$Log: platform_124mux.c,v $
Revision 1.1  2020/01/09 01:02:01  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
