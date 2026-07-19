/* $Id: diag_gephy_1543_util.c,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_gephy_1543_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_1543_util.c - GE PHY Utility function
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
#include "diag_gephy_1543_lib.h"
#include "diag_gephy_1543_test.h"
#include "diag_gephy_1543_util.h"
#include "dnv_eth_lib.h"

/*
 * Global variables
 */
extern int mrvl88e1543_dev_init;
extern dev_object_t *mrvl88e1543_get_object(void);
extern int check_sfp_speed_100(int);

int diag_gephy_1543_read_reg_util(void);
int diag_gephy_1543_alter_reg_util(void);
int diag_gephy_1543_send_pkt_util(void);
int diag_gephy_1543_testmode_util(void);
int diag_gephy_1543_ping_config_util(int);



/*******************************************************************************
 *
 * Function   : diag_gephy_1543_testmode_util
 * Description: Utility to set ge phy test mode
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_1543_testmode_util (void)
{
    int rc = FAILED, phy_addr;
    dev_88e1543_object_t *gephy_obj_p;
    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    phy_addr = gethex_answer("Enter the test address number: (0x8~0xb)",
                             NANOOK_88E1543_P0_QSGMII_PHY, 0x8, 0xb);
    rc = gephy_obj_p->callin_fvt->set_test_mode((dev_object_t *)gephy_obj_p, phy_addr);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_gephy_1543_read_reg_util
 * Description: Utility to read PHY Register
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_1543_read_reg_util (void)
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
 * Function   : diag_gephy_1543_alter_reg_util
 * Description: Utility to alter PHY Register
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_1543_alter_reg_util (void)
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

/*******************************************************************************
 *
 * Function   : diag_gephy_1543_ping_config_util
 * Description: Utility to set ge phy ping config mode
 * Inputs     : Test Mode - COPPER
 *                          FIBER
 * Outputs    : PASSED/FAILED
 *
 ********************************************************************************
 */
int diag_gephy_1543_ping_config_util (int TestMode)
{
    int  port;
    char iface_name[16], cmd[256];
    dev_88e1543_object_t *gephy_obj_p;

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    memset(cmd, 0, sizeof(cmd));
    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    for (port = NANOOK_PHY_AUTO_PORT0; port <= NANOOK_PHY_AUTO_PORT1; port+=2) {

        if (port == NANOOK_PHY_AUTO_PORT0) {
            sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p0);
            system(cmd);
        }else if (port == NANOOK_PHY_AUTO_PORT1) {
            sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p1);
            system(cmd);
        }else{
            printf("%s: Port Setting failed (%d)\n", __func__, port);
                    return (FAILED);
        }


        /* Give the interface name */
                if (dnv_eth_get_iface_name(port, iface_name) == FAILED) {
                    printf("%s: Get Iface name failed (%d)\n", __func__, port);
                    return (FAILED);
                }
    
        /* Set QSGMII to Auto Detect and Reset Phy (P18_R20) */
        if (diag_gephy_smi_wr(port, 0x12, 0x14, 0x8007) == FAILED ){
            return (FAILED);
        }

        if (port == NANOOK_PHY_AUTO_PORT0) {
            sprintf(cmd, "ifconfig %s down > /dev/null; ifconfig %s up > /dev/null", 
                    inface_lan1p0, inface_lan1p0);
            system(cmd);
        }else if (port == NANOOK_PHY_AUTO_PORT1) {
            sprintf(cmd, "ifconfig %s down > /dev/null; ifconfig %s up > /dev/null", 
                    inface_lan1p1, inface_lan1p1);
            system(cmd);
        }else{
            printf("%s: Port Setting failed (%d)\n", __func__, port);
                    return (FAILED);
        }

        /* Reset QSGMII (P4_R0_B15)*/
        if (diag_gephy_smi_wr(port, 0x4, 0x0, 0x9140) == FAILED ){
            return (FAILED);
        }

        if (TestMode == COPPER){

            /* Reset COPPER (P0_R0_B15)*/
            if (diag_gephy_smi_wr(port, 0x0, 0x0, 0x8140) == FAILED ){
                return (FAILED);
            }

        }else if (TestMode == FIBER){

             /* Reset FIBER (P1_R0_B15)*/
            if (diag_gephy_smi_wr(port, 0x1, 0x0, 0x8140) == FAILED ){
                return (FAILED);
            }
   
        }else{
            printf("%s: Unknown mode\n", __func__);
            return (FAILED);
        }

    } /*End of for*/
    printf("Configuration Done.");
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_gephy_1543_send_pkt_util 
 *
 * Description: Utility to manually send packet
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_1543_send_pkt_util (void)
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

/******************************************************************************
 *
 * Function: diag_gephy_1543_sfp_send_pkt_util 
 *
 * Description: Utility to manually send packet
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_1543_sfp_send_pkt_util (void)
{
    char iface_name1[16], iface_name2[16];
    int rc, phy_speed1 = ETH_MODE_GE, phy_speed2 = ETH_MODE_GE;
    int phy_addr_num1 = NANOOK_1543_P0_AUTO_DET_PHY_ADDR;
    int phy_addr_num2 = NANOOK_1543_P1_AUTO_DET_PHY_ADDR;
    
    if (check_sfp_speed_100(NANOOK_1543_P0_AUTO_DET_PHY_ADDR)) {
        phy_speed1 = ETH_MODE_FE100;
    }
    if (check_sfp_speed_100(NANOOK_1543_P0_AUTO_DET_PHY_ADDR)) {
        phy_speed2 = ETH_MODE_FE100;
    }
    
    if (phy_speed1 != phy_speed2) {
        printf("The two SFPs are not match!\n");
        return (FAILED);
    } else if (phy_speed1 == ETH_MODE_FE100){
        diag_gephy_1543_sfp_force_100();
    }

    if (dnv_eth_get_iface_name(phy_addr_num1, iface_name1) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, phy_addr_num1);
        return (FAILED);
    }
    if (dnv_eth_get_iface_name(phy_addr_num2, iface_name2) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, phy_addr_num2);
        return (FAILED);
    }

    printf("Sending packet through interface '%s' ...\n", iface_name1);
    fflush(stdout);

    rc = eth_pkt_txrx_sfp(iface_name1, iface_name2, SENDUTIL_PKT_CNT, FALSE);
    if (rc != PASSED) {
        printf("%s(): SFP0 send packets to SFP1 Failed\n", __FUNCTION__);
        return (FAILED);
    } else {
        printf("SFP0 send packets to SFP1 Success\n");
    }
    
    printf("Sending packet through interface '%s' ...\n", iface_name2);
    fflush(stdout);
    
    rc = eth_pkt_txrx_sfp(iface_name2, iface_name1, SENDUTIL_PKT_CNT, FALSE);
    if (phy_speed1 == ETH_MODE_FE100){
        diag_gephy_1543_init();
    }
    if (rc != PASSED) {
        printf("%s(): SFP1 send packets to SFP0 Failed\n", __FUNCTION__);
        return (FAILED);
    } else {
        printf("SFP1 send packets to SFP0 Success\n");
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_gephy_1543_force_phy_speed_util
 * Description: Utility to force set phy speed to 10/100/1000M
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_1543_force_phy_speed_util (void)
{
    int rc = FAILED;
    uint port = 0, speed = 0, reg_val;
    
    port = gethex_answer("Enter the phy number: (0x0:WAN0, 0x1:WAN1)",
                             0x0, 0x0, 0x1);
    speed = gethex_answer("set the phy speed: (0x0:10M, 0x1:100M, 0x2:1000M)",
                             0x2, 0x0, 0x2);
    
    if (port == 0) {
        port = NANOOK_PHY_AUTO_PORT0;
    } else if (port == 1) {
        port = NANOOK_PHY_AUTO_PORT1;
    }
    
    if (diag_gephy_smi_wr(port, 0x10, 0x0, 0x0) == FAILED ){
        return (FAILED);
    }
    
    switch (speed) {
        case 0:
            reg_val = 0x8100;
            break;
        case 1:
            reg_val = 0xa100;
            break;
        case 2:
            reg_val = 0x9140;
            break;
        default:
            return (FAILED);
    }

    if (diag_gephy_smi_wr(port, 0x0, 0x0, reg_val) == FAILED ){
        return (FAILED);
    }

    return (rc);
}


/*-------------------------------------------------
$Log: diag_gephy_1543_util.c,v $
Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
