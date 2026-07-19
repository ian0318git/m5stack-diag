/* $Id: diag_gephy_lib.c,v 1.8 2020/09/30 09:46:09 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_gephy_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_lib.c - GE PHY functions library
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
#include "proto.h"
#include "common.h"
#include "types.h"
#include "diag_gephy_lib.h"
#include "dnv_eth_lib.h"
#include "diag_fpga.h"
#include "diag_common.h"
#include "diag_fpga.h"
#include "nvmonvars.h"
#include "diag_gephy_test.h"


/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/



/*===================================================================*
 *                    Local variables                                *
 *===================================================================*/
static char mrvl_err_msg[MRV88E1543_ERR_MSG_LEN];
extern uint32 err_report(dev_object_t *, char *, uint32);

int diag_gephy_dev_create(dev_88e1543_object_t *);
int diag_gephy_init(void);
int diag_gephy_smi_rd(uint, uint, uint, uint *); 
int diag_gephy_read_page_reg(void);
int diag_gephy_smi_wr(uint, uint, uint, uint);
int phy_88e1543_dev_init(dev_88e1543_object_t *, int, int);
void phy_88e1543_err_report(dev_object_t *, char *, uint32);
int diag_gephy_get_linkup_status(uint, uint *);

/*******************************************************************************
 *
 * Function    : diag_gephy_dev_create
 * Description : Function to create 88E1543 Device Object
 * Inputs      : gephy_obj - Pointer of 88E1543 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_gephy_dev_create (dev_88e1543_object_t *gephy_obj)
{
    dev_object_t *dev = (dev_object_t *)gephy_obj;

    /* Create common device object */
    dev_88e1543_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }   

    /* Attach the device */
    gephy_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    gephy_obj->callout_fvt->smi_read = diag_gephy_smi_rd;
    gephy_obj->callout_fvt->smi_write = diag_gephy_smi_wr;
    gephy_obj->callout_fvt->get_linkup_status = diag_gephy_get_linkup_status;

    return (PASSED);
}

int diag_gephy_read_page_reg (void)
{
    int rc = PASSED, phy_addr;
    uint buf;

    phy_addr = gethex_answer("Enter the phy address: ", 0x0, 0x0, 0x3);

    rc = dnv_read_phy_reg(phy_addr, phy_addr, PHY_1543_PAGE_REG_ADDR, &buf);

    printf("\nPage number is %d\n", buf);

    return (rc);
}

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
    uint phy_addr = 0;
    
    switch (portnum) {
        case DNV_LAN0_PORT0:
             phy_addr = NUTELLA_1543_P0_PHY_ADDR;
            break;
        case DNV_LAN0_PORT1:
             phy_addr = NUTELLA_1543_P1_PHY_ADDR;
            break;
        case DNV_LAN1_PORT0:
             phy_addr = NUTELLA_1543_P2_PHY_ADDR;
            break;
        case DNV_LAN1_PORT1:
             phy_addr = NUTELLA_1543_P3_PHY_ADDR;
            break;
        default:
            rc = FAILED;
            rc2 = FAILED;
            cterr('f', 0, "%s: Wrong port mapping.\n", __FUNCTION__);
            goto _exit;
    }
    
    /* Change PHY page first */
    rc = dnv_write_phy_reg(portnum, phy_addr, MRV88E1543_PAGE_ADDRESS_REG, page);
    rc2 = dnv_read_phy_reg(portnum, phy_addr, addr, buf);

    if ((rc == PASSED) && (rc2 == PASSED)) {
        return (PASSED);
    } else {
_exit:
        return (FAILED);
    }
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
    uint phy_addr = 0;
    
    switch (portnum) {
        case DNV_LAN0_PORT0:
            phy_addr = NUTELLA_1543_P0_PHY_ADDR;
            break;
        case DNV_LAN0_PORT1:
            phy_addr = NUTELLA_1543_P1_PHY_ADDR;
            break;
        case DNV_LAN1_PORT0:
            phy_addr = NUTELLA_1543_P2_PHY_ADDR;
            break;
        case DNV_LAN1_PORT1:
            phy_addr = NUTELLA_1543_P3_PHY_ADDR;
            break;
        default:
            rc = FAILED;
            rc2 = FAILED;
            cterr('f', 0, "%s: Wrong port mapping.\n", __FUNCTION__);
            goto _exit;
    }
    
    rc = dnv_write_phy_reg(portnum, phy_addr, MRV88E1543_PAGE_ADDRESS_REG, page);
    rc2 = dnv_write_phy_reg(portnum, phy_addr, addr, data);
    if ((rc == PASSED) && (rc2 == PASSED)) {
        return (PASSED);
    } else {
_exit:
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function    : diag_gephy_get_linkup_status
 * Description : Function to get PHY link status
 * Inputs      : portnum - port number
 *               port_link_status - link status of port
 * Outputs     : TRUE / FALSE
 *
 *******************************************************************************
 */
int diag_gephy_get_linkup_status (uint portnum, uint *port_link_status) 
{
    uint phy_addr = 0;
    uint link_status;
    
    switch (portnum) {
        case DNV_LAN0_PORT0:
            phy_addr = NUTELLA_1543_P0_PHY_ADDR;
            break;
        case DNV_LAN0_PORT1:
            phy_addr = NUTELLA_1543_P1_PHY_ADDR;
            break;
        case DNV_LAN1_PORT0:
            phy_addr = NUTELLA_1543_P2_PHY_ADDR;
            break;
        case DNV_LAN1_PORT1:
            phy_addr = NUTELLA_1543_P3_PHY_ADDR;
            break;
        default:
            cterr('f', 0, "%s: Wrong port mapping.\n", __FUNCTION__);
            return (FAILED);
    }
    
    link_status = dnv_eth_link_is_up(phy_addr);
    *port_link_status = link_status;
    
    if (link_status != FALSE) {
        return (TRUE);
    } 

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : diag_gephy_init
 * Description: Function to init Nutella GE PHY
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_gephy_init (void)
{
    uint wr_data = 0, rd_data = 0, ix = 0;

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


    system(ETH_RM_IXGBE_MODULE);
    msleep(SLEEP_1000);
    system(ETH_INS_IXGBE_MODULE);
    msleep(SLEEP_1000);

    /* Change the initialization sequence move "adjust amplitude"
     * before "ifconfig up" to avoid the race condition.(CDETS number:CSCvq58855)*/
    /* Nutella HW request */
    /* Set 26_1 [2:0] SGMII differential voltage peak to 308mV */
    for (ix = NUTELLA_PHY_START_ADDR; ix <= NUTELLA_PHY_MAX_PORT_NUM; ix++) {
        if (diag_gephy_smi_rd(ix, MRV88E1543_REG_PAGE_1,
                              MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG, &rd_data)
                              != PASSED) {

            cterr('f', 0, "%s: Fail to read SGMII output amplitude\n", __FUNCTION__);
            return (FAILED);
        } 

        wr_data = ((rd_data & (~MRV88E1543_P1_R26_SGMII_AMP_CLR_MUSK)) 
                   |  MRV88E1543_P1_R26_SGMII_AMP_308MV);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s() line %d, port %d, page %d, register %d, write %#x\n",
                   __FUNCTION__, __LINE__, ix, MRV88E1543_REG_PAGE_1,
                   MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG, wr_data);
        }

        if (diag_gephy_smi_wr(ix, MRV88E1543_REG_PAGE_1,
                              MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG, wr_data)
                              != PASSED) {
            cterr('f', 0, "%s: Fail to set up SGMII output amplitude\n", __FUNCTION__);
            return (FAILED);
        } 
        
        if (diag_gephy_smi_rd(ix, MRV88E1543_REG_PAGE_1,
                              MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG, &rd_data)
                              != PASSED) {
            cterr('f', 0, "%s: Fail to read SGMII output amplitude\n", __FUNCTION__);
            return (FAILED);
        }
        
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s() line %d, port %d, page %d, register %d, read %#x\n",
                   __FUNCTION__, __LINE__, ix, MRV88E1543_REG_PAGE_1,
                   MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG, rd_data);
        }

        if(rd_data != wr_data){
            cterr('f', 0, "%s: Fail to adjust SGMII output amplitude, port %d, "
                  "page %d, register %d, expect read %#x, but read %#x\n", 
                  __FUNCTION__, ix, MRV88E1543_REG_PAGE_1, 
                  MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG, wr_data, rd_data);
            return (FAILED); 
        }
    }

    if (has_sfp_sku() == TRUE) {
        system(ETH_PHY_1543_P0_UP);
        system(ETH_PHY_1543_P1_UP);
        system(ETH_PHY_1543_P2_UP);
        system(ETH_PHY_1543_P3_UP);
    } else {
        system(ETH_100M_PHY_1543_P0_UP);
        system(ETH_100M_PHY_1543_P1_UP);
        system(ETH_100M_PHY_1543_P2_UP);
        system(ETH_100M_PHY_1543_P3_UP);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function:    phy_88e1543_dev_init
 *
 * Description: Initialize Marvell 88E1543 GE PHY.
 *
 * Inputs:  dev_object - 
 *          start_phy_addr - NUTELLA PHY address starts from zero
 *          addr_seq - MRV88E1543 PHY address is incremental.
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
$Log: diag_gephy_lib.c,v $
Revision 1.8  2020/09/30 09:46:09  alicehua
CSCvv85097: Marvell 88e1543 Register test failed when port is plugged with cable

Revision 1.7  2019/10/16 23:59:58  alicehua
CSCvr66530: Add utility to read PHY page directly.

Revision 1.6  2019/10/16 23:47:51  alicehua
CSCvr66530: Add utility to read PHY page directly.

Revision 1.5  2019/08/16 10:55:32  alicehua
Change the GE PHY initialization sequence,
move "adjust amplitude" before "ifconfig up" to avoid the race condition.
(CDETS number:CSCvq58855)

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
