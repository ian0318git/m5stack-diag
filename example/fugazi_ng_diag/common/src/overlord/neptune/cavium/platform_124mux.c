/* $Id: platform_124mux.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_124mux.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_124mux.c
 *
 * Description: Neptune Cavium TI PCA9545A 1:4 Mux I2C device.
 *              This file is ported from Informers
 *
 * Copyright (c) 2011-2018 by cisco Systems, Inc.
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


/*****************************************************************************
 *                         Function prototypes 
 *****************************************************************************/

static int show_mux(int);
static int alter_mux(int);
int init_mux(int);
int test_mux(int);


/*****************************************************************************
 *                               Externs
 *****************************************************************************/
extern void set_mux_shadow (char pattern, int mux);
extern uint32_t api_mb_i2c_read(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);
extern uint32_t api_mb_i2c_write(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);

extern int32_t cavium_i2c_fd1;


/*****************************************************************************
 *                           Global variables 
 *****************************************************************************/
char mux_err_buf[ERR_BUF_SIZE];

/*
 * Control Register Bit Table
 */
static dev_pca_desc_t mux_bit_desc[] = {
    {"SFP0", "Gated", "Unconnected", MUX9545_PORT0_MASK},
    {"SFP1", "Gated", "Unconnected", MUX9545_PORT1_MASK},
    {"SFP2", "Gated", "Unconnected", MUX9545_PORT2_MASK},
    {"SFP3", "Gated", "Unconnected", MUX9545_PORT3_MASK},
    {0, 0, 0, 0},
};


/*****************************************************************************
 *                                 Menus
 *****************************************************************************/
/*
 * 1:4 Mux Menu
 */
static submenu_xtable_t mux_menu_table[] = {
    {"Show Mux Control Register", (PFT)show_mux, 0, 0,
	(type_t(*)())0, 0, (PFT)show_mux, 0},
    {"Alter Mux Control Register", (PFT)alter_mux, 0, 0,
	(type_t(*)())0, 0, (PFT)alter_mux, 0},
    {"Clear Control Register", (PFT)init_mux, 0, 0,
	(type_t(*)())0, 0, (PFT)init_mux, 0},
};

#define PCA_MENU_TABLE_SIZE (sizeof(mux_menu_table) / \
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
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int build_mux_menu (void)
{
    /* Init Mux */
    if (init_mux(0) != PASSED) {
        /* Unable to init mux */
        cterr('f', 0, "%s:%d Mux init failed.", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    testname("PCA9545a 1:4 Mux");

    build_primary_submenu(mux_menu_table, PCA_MENU_TABLE_SIZE,
                          "1:4 Mux Utility Menu", &muxdiagp);
    build_secondary_submenu(mux_menu_table, PCA_MENU_TABLE_SIZE,
                            mux_menu_secondary_items);
    menu(&muxdiag, mux_menu_secondary_items, 0);

    return (PASSED);
}


/*****************************************************************************
 * 
 * Function   : init_pca9545a
 * Description:	Initilialize PCA9545A control register.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 *              Control registeControl register to be initialized to is 
 *              also passed in
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint32 init_pca9545a (n2g_i2c_dev_t *dev, pca_t init)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;
    pca_t ctrl;

    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = sizeof(ctrl);
    i2c_if.buf = (char *)&ctrl;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    ctrl = init;
	
    rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
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
 * Inputs     : pattern - Data pattern to be initialized.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int init_mux (int pattern)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = OVLD_CAVIUM_MUX_I2C_ADDR;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd1 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd1, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd1;
        }
    }

    /* Init MUX (TI PCA9545a) */
    rc = init_pca9545a(&i2c_dev, (pca_t)pattern);
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
static uint32 show_pca9545a_reg (n2g_i2c_dev_t *dev, dev_pca_desc_t *pdesc)
{
    uint32 rc = FAILED;
    pca_t ctrl;	/* Control register from PCA9545A */
    n2g_i2c_if_t i2c_if;
    
    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;
    i2c_if.buf = (char *)&ctrl;
    i2c_if.size = sizeof(ctrl);

    /* Read the control register of PCA9545A */
    rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);

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
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int show_mux (int opt)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    dev_pca_desc_t *pca_desc_p;
    int dummy = 0;

    dummy = opt;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = OVLD_CAVIUM_MUX_I2C_ADDR;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd1 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd1, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd1;
        }
    }

    /* Setup descriptor table */
    pca_desc_p = (dev_pca_desc_t *)&mux_bit_desc;

    /* Display register contents */
    rc = show_pca9545a_reg(&i2c_dev, pca_desc_p);
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
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int alter_pca9545a_ctrl_reg (n2g_i2c_dev_t *dev)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    pca_t old_ctrl, new_ctrl;

    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = sizeof(pca_t);
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    /* Read the register first. */
    i2c_if.buf = (char *)&old_ctrl;

    rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    if (rc != PASSED) {
        sprintf(mux_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    new_ctrl = gethex_answer("Enter the new control register", old_ctrl,
                             PCA9545_CTRL_MIN, PCA9545_CTRL_MAX);

    /* Write the new data */
    i2c_if.buf = (char *)&new_ctrl;

    rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
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
 * Inputs     : None
 * Outputs    : PASSED/FAILED.
 *
 *****************************************************************************/
static int alter_mux (int opt)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    int dummy = 0;

    dummy = opt;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = OVLD_CAVIUM_MUX_I2C_ADDR;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd1 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd1, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd1;
        }
    }

    /* Alter PCA9545a Ctrol Reg */
    rc = alter_pca9545a_ctrl_reg(&i2c_dev);
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
static int test_pca9545a (n2g_i2c_dev_t *dev)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;
    pca_t ctrl;
    uint8_t chan;

    /* Clear MUX error buffer */
    memset(&mux_err_buf[0], 0, sizeof(mux_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = sizeof(ctrl);
    i2c_if.buf = (char *)&ctrl;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    for (chan = PCA9545_B0; chan <= PCA9545_B3; chan = (chan << 1)) {
        ctrl = chan;
        rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
        if (rc != PASSED) {
            sprintf(mux_err_buf, "%s:%d I2C write failed(rc = %#x).",
                                 __FUNCTION__, __LINE__, rc);
            return (FAILED);
        } /* endof if rc */

        ctrl = 0; // clear the buffer before read
        rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
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
int test_mux(int opt)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    int dummy = 0;

    dummy = opt;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = OVLD_CAVIUM_MUX_I2C_ADDR;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd1 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd1, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd1;
        }
    }

    /* Test the Mux Control reg */
    rc = test_pca9545a(&i2c_dev);
    if (rc != PASSED) {
        cterr('f', 0, "%s", mux_err_buf);
	}
    return (rc);
}


/******************************************************************************
 *
 * Function   : set_mux_channel
 * Description: This function set the mux channel for the slave device.
 * Inputs     : i2c_p - Pointer to the N2G I2C API interface struct
 *                      Fields needed in the struct are:
 *                      i2c_bus_type, i2c_dev
 *              mux_mask - data for mux setup
 * Outputs    : PASSED or I2C error code
 *
 ******************************************************************************/
int set_mux_channel (n2g_i2c_dev_t *i2c_p, uint8_t mux_mask)
{
    uint rc = FAILED;

    /* Init Mux */
    rc = init_mux(0);
    if (rc != PASSED) {
        /* Unable to init Mux */
        cterr('f', 0, "%s:%d Mux init failed (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    /* set Mux channel */
    rc = api_mb_i2c_write(i2c_p, 0, 1, (char *)&mux_mask);
    if (rc != PASSED) {
        /* Unable to set Mux */
        cterr('f', 0, "%s:%d set Mux failed (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
    }
    return (rc);
}


/*------------------------------------------------------------------
$Log: platform_124mux.c,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

Revision 1.4  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.3  2012/06/06 09:57:28  iachang
Clean up complier warnings.

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module

$Endlog$
*/
