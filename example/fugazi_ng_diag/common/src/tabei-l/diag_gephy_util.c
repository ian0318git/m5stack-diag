 /* $Id: diag_gephy_util.c,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_gephy_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_util.c - GE PHY Utility function
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "proto.h"
#include "types.h"
#include "common.h"
#include "diag_common.h"
#include "diag_eth_pkt_txrx.h"
#include "dev_88e151x.h"
#include "diag_gephy_lib.h"
#include "diag_gephy_util.h"
#include "dnv_eth_lib.h"


int diag_gephy_read_reg_util(void);
int diag_gephy_alter_reg_util(void);
int diag_gephy_dump_reg_util(void);
int diag_gephy_send_pkt_util(void);
int diag_gephy_testmode_util(void);
int diag_gephy_eee_util(void);
int diag_gephy_init_util(void);
int diag_gephy_1514_init(int);
int diag_gephy_set_sgmii_amp(int);


extern int diag_ge_phy_no;


/*******************************************************************************
 *
 * Function   : diag_gephy_set_sgmii_amp
 * Description: Set SGMII output amplitude
 * Inputs     : GE PHY number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_set_sgmii_amp (int diag_1514_no)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_1514_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    /* Set Page1 Register 26 [2:0] = 110 (602mV) (CSCvo75861) */
    rc = gephy_obj_p->callin_fvt->set_sgmii_output_amp((dev_object_t *)gephy_obj_p, TABEI_SGMII_AMP_VAL);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}
/*******************************************************************************
 *
 * Function   : diag_gephy_1514_init
 * Description: Init PHY
 * Inputs     : GEPHY number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_1514_init (int diag_1514_no)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_1514_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->init_phy((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_gephy_init_util
 * Description: Utility to init PHY
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_init_util (void)
{
    int ix = 0, rc = 0;
    ushort read_data;

    /* Init GE PHY */
    diag_gephy_1514_init(TABEI_GE0_88E1514_PHY);
    diag_gephy_1514_init(TABEI_GE1_88E1514_PHY);

    msleep(WAITING_FOR_1514_INIT);


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

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : diag_gephy_testmode_util
 * Description: Utility to set ge phy test mode
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_testmode_util (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->set_testmode((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_gephy_read_reg_util
 * Description: Utility to read PHY Register
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_read_reg_util (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->display_register((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : diag_gephy_alter_reg_util
 * Description: Utility to alter PHY Register
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_alter_reg_util (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->alter_register((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : diag_gephy_dump_reg_util
 * Description: Utility to dump PHY Register
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_dump_reg_util (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->alter_register((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_gephy_eee_util
 * Description: Utility to change gephy EEE(Energy Efficient Ethernet) test mode
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_eee_util (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->set_eee((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}



/******************************************************************************
 *
 * Function: diag_gephy_send_pkt_util 
 *
 * Description: Utility to manually send packet
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_send_pkt_util (void)
{
    char iface_name[16];
    int rc;

    if (dnv_eth_get_iface_name(diag_ge_phy_no, iface_name) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, diag_ge_phy_no);
        return (FAILED);
    }

    printf("Sending packet through interface '%s' ...\n", iface_name);
    fflush(stdout);

    rc = eth_pkt_txrx(iface_name, SENDUTIL_PKT_CNT, FALSE);

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_gephy_util.c,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.6  2019/08/29 03:49:27  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.5  2019/07/26 03:41:56  kehuang2
 * Add setting interrupt pin configuration into initial sequence
 *
 * Revision 1.1.2.4  2019/05/24 09:56:11  kehuang2
 *
 * 1.Update Temp Interrupt test
 * 2.Clean up code
 *
 * Revision 1.1.2.3  2019/03/12 07:24:09  olin2
 * Adjust SGMII output amp and revise GE PHY init sequence
 *
 * Revision 1.1.2.2  2018/12/25 06:38:40  olin2
 * Support initial GE PHY util
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
