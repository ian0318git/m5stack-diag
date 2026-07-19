/* $Id: diag_ge_phy_util.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_ge_phy_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_ge_phy_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "nvmonvars.h"
#include "ethernet.h"
#include "common_utils.h"
#include "diag_cpu_lib.h"
#include "diag_ge_phy_test.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_moka_fpga_lib.h"
#include "diag_smi_lib.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_lib.h"
#include "diag_eth_pkt_txrx.h"

/*************************************************
 * Function   : diag_util_ge_rd_reg
 *
 * Description: an utility to read GE PHY register
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 *************************************************
 */
int diag_util_ge_rd_reg(int ge_num)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc = FAILED;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->util_ge_rd_reg(dev); 

    if (rc != PASSED) {
        printf("%s:%d:Failed in GE PHY register read utility\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        printf("%s:%d:Failed to detach n88e111x object\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }
    
    return (PASSED);
}

/**************************************************
 * Function   : diag_util_ge_wr_reg
 *
 * Description: an utility to write GE PHY register
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 **************************************************
 */
int diag_util_ge_wr_reg(int ge_num)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc = FAILED;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->util_ge_wr_reg(dev); 

    if (rc != PASSED) {
        printf("%s:%d:Failed in GE PHY register write utility\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
            printf("%s:%d:Failed to detach n88e111x object\n", 
                   __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/**********************************************************
 * Function   : diag_util_ge_set_test_mode
 *
 * Description: an utility to set test mode in
 *              10000BASE-T Control Register(Page:0, Reg:9)
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 **********************************************************
 */
int diag_util_ge_set_test_mode(int ge_num)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc = FAILED;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->util_ge_set_test_mode(dev); 

    if (rc != PASSED) {
        printf("%s:%d:Failed in GE PHY set test mode utility\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
            printf("%s:%d:Failed to detach n88e111x object\n", 
                   __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/*****************************************************************
 * Function   : diag_util_ge_set_tx_type
 *
 * Description: an utility to set Tx type in
 *              Copper Specific Control Register 2 (Page:0 Reg:26)
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 *****************************************************************
 */
int diag_util_ge_set_tx_type(int ge_num)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc = FAILED;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->util_ge_set_tx_type(dev); 

    if (rc != PASSED) {
        printf("%s:%d:Failed in GE PHY set Tx type utility\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* detach device */
    if (rc != PASSED) {
            printf("%s:%d:Failed to detach n88e111x object\n", 
                   __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/*****************************************************************
 * Function   : diag_util_ge_set_vod
 *
 * Description: an utility to set VOD in Reg:29 and Reg:30
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 *****************************************************************
 */
int diag_util_ge_set_vod(int ge_num)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc = FAILED;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->util_ge_set_vod(dev); 

    if (rc != PASSED) {
        printf("%s:%d:Failed in GE PHY set VOD utility\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
            printf("%s:%d:Failed to detach n88e111x object\n", 
                   __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_util_ge_send_packet_util
 * Description: Utility to send and check specific ethernet port.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_util_ge_send_packet_util (int eth_num)
{

    if (eth_num == GE0) {
        return (eth_pkt_txrx(GE0_ETH, LPBK_PKG, FALSE));
    } else if (eth_num == GE1) {
        return (eth_pkt_txrx(GE1_ETH, LPBK_PKG, FALSE));
    } else if (eth_num == GEESW) {
        return (eth_pkt_txrx(ESW_ETH, LPBK_PKG, FALSE));
    } else {
        printf("upsupport eth num\n");
        return (FAILED);
    }

}

/**************************************************
 * Function   : diag_util_ge_led
 *
 * Description: an utility to control GE LED ON/OFF
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 **************************************************
 */
int diag_util_ge_led(int ge_num)
{
    int rc = FAILED;
    smi_t page = MRV88E111N_REG_PAGE_3;
    uchar offset = MRV88E111L_P3_R17_POL_CONTROL_REG; 
    smi_t wr_buf = GE_LED_OFF;
    int led_op = 0;

    /* let user to enter the LED operation(ON/OFF) */
    printf("\n"); 
    printf("GE%d Status LED utils: \n", ge_num); 
    printf("0. OFF\n");
    printf("1. Green\n");
    led_op = getdec_answer("Select Toogle (0 ~ 1): ", 0, 0, 1);
    wr_buf = (led_op == 1)? (smi_t) GE_LED_ON : GE_LED_OFF;

    /* write out data into GE PHY to turn ON/OFF LED */
    rc = diag_ge_phy_reg_wr_if(ge_num, page, offset, &wr_buf);

    if (rc != PASSED) {
        printf("%s:%d:Failed control LED ON/OFF \n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : gephy_set_txtype_util
 * Description: Utility to set GE WAN PHY Transmitter Type.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int gephy_set_txtype_util (int ge_num)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc = FAILED;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->util_ge_set_tx_type(dev); 

    if (rc != PASSED) {
        printf("%s:%d:Failed in GE PHY set VOD utility\n", 
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
            printf("%s:%d:Failed to detach n88e111x object\n", 
                   __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_ge_phy_util.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.3  2021/05/31 10:49:47  illiu
 * Fix function diag_util_ge_send_packet_util for GE PHY and ESW PHY
 *
 * Revision 1.1.2.2  2020/11/05 06:40:37  harrchan
 * 1.According elixir fpga spec to change the definition of SFP present and SFP transmitter disable bit.
 * 2.Add GE PHY1 SFP external loopback test
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
