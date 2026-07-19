 /* $Id: diag_gephy_util.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_gephy_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_util.c - GE PHY Utility function
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
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "common.h"
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

extern int diag_ge_phy_no;


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
 * Description: Utility to change gephy EEE test mode
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
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.4  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.3  2018/04/26 08:14:16  lucywang
 * Added utility to set 88E1514 EEE
 *
 * Revision 1.1.2.2  2018/03/16 01:59:55  olin2
 * Support GE PHY testmode util
 *
 * Revision 1.1.2.1  2018/02/27 08:06:43  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
