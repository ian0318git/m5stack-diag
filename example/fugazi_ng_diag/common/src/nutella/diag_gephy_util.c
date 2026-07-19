/* $Id: diag_gephy_util.c,v 1.4 2019/07/11 12:31:28 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_gephy_util.c,v $
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
#include "types.h"
#include "common.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_gephy_lib.h"
#include "diag_gephy_test.h"
#include "diag_gephy_util.h"
#include "dnv_eth_lib.h"

/*
 * Global variables
 */

int diag_gephy_read_reg_util(void);
int diag_gephy_alter_reg_util(void);
int diag_gephy_send_pkt_util(void);
int diag_gephy_testmode_util(void);



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
    int rc = FAILED, phy_port;
    dev_88e1543_object_t *gephy_obj_p;
    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    phy_port = gethex_answer("Enter the test port number: ", 0, 0, MRV88E1543_PORTS); 

    rc = gephy_obj_p->callin_fvt->set_test_mode((dev_object_t *)gephy_obj_p, phy_port);


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
    int rc = FAILED;
    dev_88e1543_object_t *gephy_obj_p;
    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->display_reg((dev_object_t *)gephy_obj_p);


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
    int rc = FAILED;
    dev_88e1543_object_t *gephy_obj_p;
    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
     }

    rc = gephy_obj_p->callin_fvt->alter_reg((dev_object_t *)gephy_obj_p);


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
    int phy_addr_num;

    printf("Please enter PHY addr:");
    scanf("%x", &phy_addr_num);
    if (dnv_eth_get_iface_name(phy_addr_num, iface_name) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, phy_addr_num);
        return (FAILED);
    }

    printf("Sending packet through interface '%s' ...\n", iface_name);
    fflush(stdout);

    rc = eth_pkt_txrx(iface_name, SENDUTIL_PKT_CNT, FALSE);

    return (rc);
}

/*-------------------------------------------------
$Log: diag_gephy_util.c,v $
Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
