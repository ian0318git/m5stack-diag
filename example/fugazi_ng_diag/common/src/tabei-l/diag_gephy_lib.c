 /* $Id: diag_gephy_lib.c,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_gephy_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_lib.c - GE PHY functions library
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "diag_gephy_util.h"

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
    if (phy_no == TABEI_GE0_88E1514_PHY) {
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
    return (dnv_read_phy_reg(DNV_LAN0_PORT0, TABEI_1514_GE0_PHY_ADDR, 
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
    return (dnv_write_phy_reg(DNV_LAN0_PORT0, TABEI_1514_GE0_PHY_ADDR,
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
    return (dnv_read_phy_reg(DNV_LAN0_PORT1, TABEI_1514_GE1_PHY_ADDR, 
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
    return (dnv_write_phy_reg(DNV_LAN0_PORT1, TABEI_1514_GE1_PHY_ADDR, 
                              addr, data));
}



/*******************************************************************************
 *
 * Function   : diag_gephy_init
 * Description: Function to init GE PHY
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_init (void)
{
    int ix = 0, rc = 0;
    ushort read_data;
    /* REG 0x4, bit 0 */
    if (fpga_reset_api(FPGA_EXT_DEVICE_RESET_REG, FPGA_EXT_DEVICE_RESET_GE, TRUE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to put GE PHY in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    if (fpga_reset_api(FPGA_EXT_DEVICE_RESET_REG, FPGA_EXT_DEVICE_RESET_GE, FALSE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to release GE PHY from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    msleep(SLEEP_1000);

    /* Init GE PHY */
    diag_gephy_1514_init(TABEI_GE0_88E1514_PHY);
    diag_gephy_1514_init(TABEI_GE1_88E1514_PHY);

    msleep(SLEEP_100);


    /* Set Interrupt pin is always in interrupt mode */
    for (ix = DNV_LAN0_PORT0; ix <= DNV_LAN0_PORT1; ix++) {
        rc = dnv_write_phy_reg(DNV_LAN0_PORT0 + ix, TABEI_1514_GE0_PHY_ADDR + ix, 
                               MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_3);
        if (rc != PASSED) {
            cterr('f', 0, "%s: Failed to init GE PHY.\n", __FUNCTION__);
            return (FAILED);
        }
        rc = dnv_read_phy_reg(DNV_LAN0_PORT0 + ix, TABEI_1514_GE0_PHY_ADDR + ix, 
                              MRV88E151XL_TMR_CONTROL_REG, &read_data);
        if (rc != PASSED) {
            cterr('f', 0, "%s: Failed to init GE PHY.\n", __FUNCTION__);
            return (FAILED);
        }
    
        read_data |= PHY_TIMER_CNTRL_INTR_EN;
    
        rc = dnv_write_phy_reg(DNV_LAN0_PORT0 + ix, TABEI_1514_GE0_PHY_ADDR + ix, 
                               MRV88E151XL_TMR_CONTROL_REG, read_data);
        if (rc != PASSED) {
            cterr('f', 0, "%s: Failed to init GE PHY.\n", __FUNCTION__);
            return (FAILED);
        }
        rc = dnv_write_phy_reg(DNV_LAN0_PORT0 + ix, TABEI_1514_GE0_PHY_ADDR + ix, 
                               MRV88E151X_PAGE_ADDRESS_REG, MRV88E151X_REG_PAGE_0);
        if (rc != PASSED) {
            cterr('f', 0, "%s: Failed to init GE PHY.\n", __FUNCTION__);
            return (FAILED);
        }
    }
    /* HW request: Set SGMII output Amplitude */
    diag_gephy_set_sgmii_amp(TABEI_GE0_88E1514_PHY);
    msleep(SLEEP_100); 
    diag_gephy_set_sgmii_amp(TABEI_GE1_88E1514_PHY);
    msleep(SLEEP_100); 


    system(ETH_PHY_1514_GE0_UP);
    system(ETH_PHY_1514_GE1_UP);

    return (PASSED);

}

/*-------------------------------------------------
 * $Log: diag_gephy_lib.c,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.9  2019/08/29 03:49:26  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.8  2019/07/26 03:41:56  kehuang2
 * Add setting interrupt pin configuration into initial sequence
 *
 * Revision 1.1.2.7  2019/03/12 07:24:09  olin2
 * Adjust SGMII output amp and revise GE PHY init sequence
 *
 * Revision 1.1.2.6  2019/02/11 11:24:56  harrchan
 * Support Init gephy
 *
 * Revision 1.1.2.5  2019/01/25 03:54:27  wilbhuan
 * 1. Removed re-load ixgbe.ko procedure in below files:
 *    (1)diag_gephy_1543_lib.c
 *    (2)diag_gephy_lib.c
 * 2. Updated "dnv_get_correct_iface_name" function to support ESW configuration.
 *
 * Revision 1.1.2.4  2018/12/25 06:41:49  olin2
 * Update GE PHY init sequence
 *
 * Revision 1.1.2.3  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.2.2  2018/10/24 02:47:27  harrchan
 * 88E1514 GEPHY test
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
