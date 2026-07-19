 /* $Id: diag_gephy_lib.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_gephy_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_lib.c - GE PHY functions library
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proto.h"
#include "common.h"
#include "types.h"
#include "diag_gephy_lib.h"
#include "dev_88e151x.h"
#include "dnv_eth_lib.h"
#include "diag_fpga.h"
#include "diag_common.h"
#include "diag_fpga.h"

static uint32 diag_gephy0_smi_rd(uint32, ushort *); 
static uint32 diag_gephy0_smi_wr(uint32, ushort); 
static uint32 diag_gephy1_smi_rd(uint32, ushort *); 
static uint32 diag_gephy1_smi_wr(uint32, ushort); 

extern uint32 err_report(dev_object_t *, char *, uint32);

int diag_gephy_dev_create(int, dev_88e151x_object_t *);
int diag_gephy_init (void);

/*******************************************************************************
 *
 * Function    : diag_gephy_dev_create
 * Description : Function to create 88E151X Device Object
 * Inputs      : phy_no    - GE PHY 0 or PHY 1 
 *               gephy_obj - Pointer of 88E151X device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_gephy_dev_create (int phy_no, dev_88e151x_object_t *gephy_obj)
{
    dev_object_t *dev = (dev_object_t *)gephy_obj;

    /* Create common device object */
    mrv88e151x_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }   

    /* Attach the device */
    gephy_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    if (phy_no == VIPER_88E1514_PHY) {
        gephy_obj->callout_fvt->rd = diag_gephy0_smi_rd;
        gephy_obj->callout_fvt->wr = diag_gephy0_smi_wr;
    } else {
        gephy_obj->callout_fvt->rd = diag_gephy1_smi_rd;
        gephy_obj->callout_fvt->wr = diag_gephy1_smi_wr;
    }

    return (PASSED);
}


/*
 *********
 * PHY 0 *
 *********
*/

/*******************************************************************************
 *
 * Function    : diag_gephy0_smi_rd 
 * Description : Function to read PHY 0 register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32 diag_gephy0_smi_rd (uint32 addr, ushort *buf) 
{
    return (dnv_read_phy_reg(DNV_LAN0_PORT0, VIPER_1514_GE0_PHY_ADDR, 
                             addr, buf));
}


/*******************************************************************************
 *
 * Function    : diag_gephy0_smi_wr
 * Description : Function to read PHY 0 register through SMI
 * Inputs      : addr - Register Address
 *               data - Data to be written to PHY register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32 diag_gephy0_smi_wr (uint32 addr, ushort data) 
{
    return (dnv_write_phy_reg(DNV_LAN0_PORT0, VIPER_1514_GE0_PHY_ADDR,
                              addr, data));
}


/*
 *********
 * PHY 1 *
 *********
*/

/*******************************************************************************
 *
 * Function    : diag_gephy1_smi_rd 
 * Description : Function to read PHY 1 register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32 diag_gephy1_smi_rd (uint32 addr, ushort *buf) 
{
    return (dnv_read_phy_reg(DNV_LAN0_PORT1, VIPER_1514_GE1_PHY_ADDR, 
                             addr, buf));
}


/*******************************************************************************
 *
 * Function    : diag_gephy1_smi_wr
 * Description : Function to read PHY 1 register through SMI
 * Inputs      : addr - Register Address
 *               data - Data to be written to PHY register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32 diag_gephy1_smi_wr (uint32 addr, ushort data) 
{
    return (dnv_write_phy_reg(DNV_LAN0_PORT1, VIPER_1514_GE1_PHY_ADDR, 
                              addr, data));
}



/*******************************************************************************
 *
 * Function   : diag_gephy_init
 * Description: Function to init Viper GE PHY
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_init (void)
{

    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_GEWAN0_RESET, TRUE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to put GE0 PHY in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_GEWAN0_RESET, FALSE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to release GE0 PHY from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    if (has_ge1_sku() == TRUE) {
        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_GEWAN1_RESET, TRUE,
                           WAITTIME_20_MS) != PASSED) {
            cterr('f', 0, "%s: Failed to put GE1 PHY in Reset.\n", __FUNCTION__);
            return (FAILED);
        }

        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_GEWAN1_RESET, FALSE,
                           WAITTIME_20_MS) != PASSED) {
            cterr('f', 0, "%s: Failed to release GE1 PHY from Reset.\n", __FUNCTION__);
            return (FAILED);
        }
    }

    system(ETH_RM_IXGBE_MODULE);
    msleep(SLEEP_1000);
    system(ETH_INS_IXGBE_MODULE);
    msleep(SLEEP_1000);

    if(this_is_viper_j()) {
        system(VIPERJ_ETH_PHY_1514_GE0_UP);
    } else {
        system(ETH_PHY_1514_GE0_UP);
    }

    if (has_ge1_sku() == TRUE) {
        if(this_is_viper_j()) {
            system(VIPERJ_ETH_PHY_1514_GE1_UP);
        } else {
            system(ETH_PHY_1514_GE1_UP);
        }
    }

    return (PASSED);

}

/*-------------------------------------------------
 * $Log: diag_gephy_lib.c,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.4  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.3  2018/03/28 07:03:51  lucywang
 * Added API to check SKU ViperJ and changed interface name for ViperJ
 *
 * Revision 1.1.2.2  2018/03/14 06:59:34  olin2
 * Modify 1514 init sequence
 *
 * Revision 1.1.2.1  2018/02/27 08:06:42  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
