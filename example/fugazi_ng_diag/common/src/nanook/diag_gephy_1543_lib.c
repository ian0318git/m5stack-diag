/* $Id: diag_gephy_1543_lib.c,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_gephy_1543_lib.c,v $
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
#include "nvmonvars.h"
#include "diag_gephy_1543_lib.h"
#include "dnv_eth_lib.h"
#include "dash_fpga.h"
#include "diag_common.h"
#include "dev_88e1543.h"


/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/


/*===================================================================*
 *                    Local variables                                *
 *===================================================================*/
static char mrvl_err_msg[MRV88E1543_ERR_MSG_LEN];
extern uint32 err_report(dev_object_t *, char *, uint32);

int diag_gephy_init(void);
int diag_gephy_smi_rd(uint, uint, uint, uint *); 
int diag_gephy_smi_wr(uint, uint, uint, uint);
int phy_88e1543_dev_init(dev_88e1543_object_t *, int, int);
void phy_88e1543_err_report(dev_object_t *, char *, uint32);

extern int fpga_reset_api(uint, uint, uint, uint);
/*
 *********
 * PHY 0 *
 *********
*/

/*******************************************************************************
 *
 * Function    : diag_gephy_smi_rd 
 * Description : Function to read PHY register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_gephy_smi_rd (uint portnum, uint page, uint addr, uint *buf) 
{
    int rc = FAILED, rc2 = FAILED;  
    switch (portnum) {
        case NANOOK_88E1543_P0_QSGMII_PHY:
             /* Change PHY page first , then read back the data */
             rc = dnv_write_phy_reg(DNV_LAN1_PORT0, NANOOK_1543_P0_QSGMII_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
             rc2 = dnv_read_phy_reg(DNV_LAN1_PORT0, NANOOK_1543_P0_QSGMII_PHY_ADDR, 
                                      (ushort) addr,(ushort *) buf);
            break;
        case NANOOK_88E1543_P0_AUTO_DETECT_PHY:
             /* Change PHY page first , then read back the data */
             rc = dnv_write_phy_reg(DNV_LAN1_PORT0, NANOOK_1543_P0_AUTO_DET_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
             rc2 = dnv_read_phy_reg(DNV_LAN1_PORT0, NANOOK_1543_P0_AUTO_DET_PHY_ADDR, 
                                      (ushort) addr,(ushort *) buf);
            break;
        case NANOOK_88E1543_P1_QSGMII_PHY:
             /* Change PHY page first , then read back the data */
             rc = dnv_write_phy_reg(DNV_LAN1_PORT1, NANOOK_1543_P1_QSGMII_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
             rc2 = dnv_read_phy_reg(DNV_LAN1_PORT1, NANOOK_1543_P1_QSGMII_PHY_ADDR, 
                                      (ushort) addr,(ushort *) buf);
            break;
        case NANOOK_88E1543_P1_AUTO_DETECT_PHY:
             /* Change PHY page first , then read back the data */
             rc = dnv_write_phy_reg(DNV_LAN1_PORT1, NANOOK_1543_P1_AUTO_DET_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
             rc2 = dnv_read_phy_reg(DNV_LAN1_PORT1, NANOOK_1543_P1_AUTO_DET_PHY_ADDR, 
                                      (ushort) addr,(ushort *) buf);
            break;

        default:
            rc = FAILED;
            cterr('f', 0, "%s: Wrong port mapping.\n", __FUNCTION__);
            break;
    }
    if (rc == FAILED){
        cterr('f', 0, "%s: Change Phy 1543 Page FAILED.\n", __FUNCTION__);
        return (FAILED);
    }
    if (rc2 == FAILED){
        cterr('f', 0, "%s: Read Phy 1543 Reg FAILED.\n", __FUNCTION__);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_gephy_smi_wr
 * Description : Function to read PHY  register through SMI
 * Inputs      : addr - Register Address
 *               data - Data to be written to PHY register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_gephy_smi_wr (uint portnum, uint page, uint addr, uint data) 
{
    int rc = FAILED, rc2 = FAILED;
    switch (portnum) {
        case NANOOK_88E1543_P0_QSGMII_PHY:
             /* Change PHY page first , then read back the data */
            rc = dnv_write_phy_reg(DNV_LAN1_PORT0,NANOOK_1543_P0_QSGMII_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
            rc2 = dnv_write_phy_reg(DNV_LAN1_PORT0, NANOOK_1543_P0_QSGMII_PHY_ADDR,
                                   addr, data);
            break;
        case NANOOK_88E1543_P0_AUTO_DETECT_PHY:
             /* Change PHY page first , then read back the data */
            rc = dnv_write_phy_reg(DNV_LAN1_PORT0,NANOOK_1543_P0_AUTO_DET_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
            rc2 = dnv_write_phy_reg(DNV_LAN1_PORT0, NANOOK_1543_P0_AUTO_DET_PHY_ADDR,
                                   addr, data);
            break;
        case NANOOK_88E1543_P1_QSGMII_PHY:
             /* Change PHY page first , then read back the data */
            rc = dnv_write_phy_reg(DNV_LAN1_PORT1,NANOOK_1543_P1_QSGMII_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
            rc2 = dnv_write_phy_reg(DNV_LAN1_PORT1, NANOOK_1543_P1_QSGMII_PHY_ADDR,
                                   addr, data);
            break;
        case NANOOK_88E1543_P1_AUTO_DETECT_PHY:
             /* Change PHY page first , then read back the data */
            rc = dnv_write_phy_reg(DNV_LAN1_PORT1,NANOOK_1543_P1_AUTO_DET_PHY_ADDR, 
                                    MRV88E1543_PAGE_ADDRESS_REG, page);
            rc2 = dnv_write_phy_reg(DNV_LAN1_PORT1, NANOOK_1543_P1_AUTO_DET_PHY_ADDR,
                                   addr, data);
            break;


        default:
            rc = FAILED;
            cterr('f', 0, "%s: Wrong port mapping.\n", __FUNCTION__);
            break;
    }
    if (rc == FAILED){
        cterr('f', 0, "%s: Change Phy 1543 Page FAILED.\n", __FUNCTION__);
        return (FAILED);
    }
    if (rc2 == FAILED){
        cterr('f', 0, "%s: Write Phy 1543 Reg FAILED.\n", __FUNCTION__);
        return (FAILED);
    }
    return (PASSED);

}

/*******************************************************************************
 *
 * Function   : diag_gephy_init
 * Description: Function to init 1543 GE PHY
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_1543_init (void)
{
    char cmd[256];
    
    if (fpga_reset_api(FPGA_EXT_DEVICE_RST_REG, FPGA_DEV_RST_88E1543_RST, TRUE,
                       GE_RESET_TIMER) != PASSED) {
        printf("%s: Failed to put GE in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Release Phy from Reset. */
    if (fpga_reset_api(FPGA_EXT_DEVICE_RST_REG, FPGA_DEV_RST_88E1543_RST, FALSE,
                       GE_RESET_TIMER) != PASSED) {
        printf("%s: Failed to release GE from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    sprintf(cmd, "ifconfig %s up > /dev/null; ifconfig %s down > /dev/null",
            inface_lan1p0, inface_lan1p0);
    system(cmd);
    sprintf(cmd, "ifconfig %s up > /dev/null; ifconfig %s down > /dev/null",
            inface_lan1p1, inface_lan1p1);
    system(cmd);

    sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p0);
    system(cmd);
    sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p1);
    system(cmd);

    return (PASSED);

}

/**********************************************************************
 *
 * Function:    phy_88e1543_dev_init
 *
 * Description: Initialize 88E1543 GE PHY.
 *
 * Inputs:  phy, start_phy_addr, addr_seq
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int phy_88e1543_dev_init (dev_88e1543_object_t *phy,
                          int start_phy_addr, int addr_seq)
{
    phy->start_phy_addr = start_phy_addr;
    phy->addr_seq = addr_seq;
    if (DEV_INIT((dev_object_t *)phy) == FAILED) {
        cterr('f', 0, "Failed to init PHY, phy addr is 0x%x", start_phy_addr);
        return (FAILED);
    }
    
    return (PASSED);
}
/*********************************************************************
 *
 * Function: phy_88e1543_err_report()
 *
 * Description: This function reports error or warning
 *              depends on the error ID flag
 *
 * Inputs:  dev      - Pointer to the PHY 88E1111 common device object
 *          err_msg  - Error message to be reported
 *          err_id   - Error reporting type identifier
 *
 * Outputs: void
 *
 *********************************************************************
 */
void phy_88e1543_err_report (dev_object_t *dev, char *err_msg, 
                                    uint32 err_id)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    char *mrvl_msg;
    int msg_count;

    mrvl_msg = mrvl_err_msg;

    msg_count = sprintf(mrvl_msg,"%s %s", err_msg, 
                                  (char *)phy->base.dev_object_fvt->dev_name);

    switch (err_id & ~FATAL) {
    case MRVL_88E1543_DEV_STATE:
        msg_count += sprintf(mrvl_msg+msg_count,"88E1543 dev_state = %x",
                                                 phy->base.dev_state);
        break;
    case MRVL_88E1543_REG_TEST:
        /*FALLTHRU*/ 
    case MRVL_88E1543_SET_LPBK:
        /*FALLTHRU*/ 
    case MRVL_88E1543_CLN_LPBK:
        /*FALLTHRU*/ 
    case MRVL_88E1543_POWER_UP:
        /*FALLTHRU*/ 
    case MRVL_88E1543_PHONE_DETECT:
        /*FALLTHRU*/ 
    default:
//        DEV_SHOW(dev, (print_fn_t)db_print, DEV_SHOW_REGISTERS);
        break;
    }

    switch (err_id & FATAL) {
    case WARNING:
        cterr('w', 0, "%s", mrvl_msg);
        break;
    case RETRY:
        printf("\nRetry: %s\n", mrvl_msg);
        break;
    default:
        cterr('f', 0, "%s", mrvl_msg);
        break;
    }

    msg_count = 0;

}

/*-------------------------------------------------
$Log: diag_gephy_1543_lib.c,v $
Revision 1.2  2019/12/11 10:10:29  lucywang
Merged Nanook to main trunk


$Endlog$
*/
