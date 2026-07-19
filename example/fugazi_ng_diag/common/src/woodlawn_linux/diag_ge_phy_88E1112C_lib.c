/* $Id: diag_ge_phy_88E1112C_lib.c,v 1.3 2014/11/12 06:19:41 leschen Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1112C_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1112C_lib.c - Utility Menu and Functions for Woodlawn PHY 88E1112C
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "types.h"
#include "platform_eth.h"
#include "diag_common_drv.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_88E1112C_test.h"
#include "diag_ge_phy_88E1112C_lib.h"
#include "diag_fpga_lib.h"

uint32_t woodlawn_88e1112c_smi_read(smi_if_t *);
uint32_t woodlawn_88e1112c_smi_write(smi_if_t *);
uint32_t woodlawn_88e1112c_smi_open(smi_if_t *);
uint32_t woodlawn_88e1112c_smi_close(smi_if_t *);
void woodlawn_88e1112c_sfp_operation(int, int);
dev_object_t *diag_get_88e11112c_obj(int);
int ge_phy_reset(boolean);
int setting_1112_lpbk_bit(int);

/*
 *  88E1112C Device Driver Object
 */
static int mrv_88e1112c_dev_init = FALSE;
static dev_mrvl_ge_object_t mrvl_88e1112c_obj;
static smi_if_t mrvl_88e1112c_smi;
static smi_t mrvl_88e1112c_smi_buf;

void woodlawn_88e1112c_sfp_operation (int sfp, int sfp_ctrl)
{
}

uint32_t woodlawn_88e1112c_smi_open (smi_if_t *smi_p)
{
    return (PASSED);
}

uint32_t woodlawn_88e1112c_smi_close (smi_if_t *smi_p)
{
    return (PASSED);
}

uint32_t woodlawn_88e1112c_smi_read (smi_if_t *smi_p)
{
    int phy_id, bus_id;
    int rc, val;
    phy_id = WOODLAWN_88E1112C_PHY_ID;
    bus_id = SMI_BUS_3;
    
    rc = woodlawn_phy_reg_rd(bus_id, phy_id,  smi_p->offset, &val);
    *(smi_p->buf) = (smi_t)val;

    return (rc);
}

uint32_t woodlawn_88e1112c_smi_write (smi_if_t *smi_p)
{
    int phy_id, bus_id;
    int rc, val;

    phy_id = WOODLAWN_88E1112C_PHY_ID; 
    bus_id = SMI_BUS_3;
    
    val = (int)*(smi_p->buf);

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, smi_p->offset, val);

    return (rc);
}

/***********************************************************************
 *
 * Function: ge_phy_reset
 *
 * Description: Reset GE PHY
 *
 * Inputs: reset - ENABLE for reset or DISABLE for clear reset.
 *
 * Outputs: PASSED/FAILED
 *
 ***********************************************************************
 */
int ge_phy_reset (boolean reset) 
{
    char reg_addr, reg_val;
    reg_addr = FPGA_RST_SIG_REG;
    
    /* Read reg 0x5(SM_RESET_L Reset Devices Enable Register) value */
    if (fpga_reg_read((int)reg_addr, &reg_val) == FAILED) {
        printf("Read FPGA register %#.8x failed\n", reg_addr);
        return (FAILED);
    }

    if (reset == ENABLE) {
        reg_val &= ~(FPGA_GE_PHY_3P3_RST_L);
    } else {
        /* Write bit 0 FPGA_GE_PHY_3P3_RST_L with val 0 - not related with SM_RESET_L */
        reg_val |= FPGA_GE_PHY_3P3_RST_L;
    }
    
    if (fpga_reg_write((int)reg_addr, reg_val) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", reg_val, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: im_get_88e1112c_obj
 *
 * Description: Get 88E1112c device driver object for IM
 *
 **********************************************************************
 */
dev_object_t *diag_get_88e11112c_obj (int phy_id)
{
    dev_mrvl_ge_object_t *mrvl_obj;
    int rc;

    if (phy_id == WOODLAWN_88E1112C_PHY_ID) {
        mrvl_obj = &mrvl_88e1112c_obj;
    } else {
        return (NULL);
    }

    if (phy_id == WOODLAWN_88E1112C_PHY_ID && mrv_88e1112c_dev_init == FALSE) {
        /* Create the device object */
        dev_88e1112c_create(mrvl_obj, &mrvl_88e1112c_smi);

        /* Attach the device object */
        rc = mrvl_obj->base.dev_object_fvt->dev_attach((dev_object_t *)mrvl_obj);

        mrvl_obj->smi_p->buf = &mrvl_88e1112c_smi_buf;

        mrv_88e1112c_dev_init = TRUE;
    }

    return ((dev_object_t *)mrvl_obj);
}

int setting_1112_lpbk_bit (int lpbk_bit) 
{
    int phy_id_1112c, bus_id;

    phy_id_1112c = WOODLAWN_88E1112C_PHY_ID;
    bus_id = SMI_BUS_3;

    /* set page */
    woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111N_PAGE_ADDRESS_REG,
                                MRV88E111N_REG_PAGE_2);

    if (lpbk_bit == SET_1112C_LPBK_BIT) {
        /* Set up LPBK bit */
        woodlawn_phy_reg_wr(bus_id, phy_id_1112c, 0x0, SET_1112C_REMOTE_LPBK_VAL);
    } else {
        /* Clear LPBK bit */
        woodlawn_phy_reg_wr(bus_id, phy_id_1112c, 0x0, CLEAR_1112C_REMOTE_LPBK_VAL);
    } 

    return (PASSED);
}
 /*-------------------------------------------------
 * $Log: diag_ge_phy_88E1112C_lib.c,v $
 * Revision 1.3  2014/11/12 06:19:41  leschen
 * Support turn on/off 1112 lpbk bit
 *
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/28 10:19:23  leslie
 * Fix 88e1112c SMI r/w bus to SMI_BUS_3
 *
 * Revision 1.2  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.2  2013/02/19 00:51:04  leslie
 * Add the contents of 88e1112c lib file
 *
 * Revision 1.1  2013/01/16 02:34:50  leslie
 * Add Woodlawn PHY 88E1112C lib file.
 *
 * $Endlog$
 *-------------------------------------------------
 */
