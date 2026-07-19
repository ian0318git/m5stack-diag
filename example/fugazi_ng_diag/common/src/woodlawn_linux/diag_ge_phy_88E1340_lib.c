/* $Id: diag_ge_phy_88E1340_lib.c,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1340_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1340_lib.c - Utility Menu and Functions for Woodlawn PHY 88E1340
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
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
#include "platform_ext_lpbk.h"
#include "diag_ge_phy_88E1340_lib.h"
#include "diag_common_drv.h"
#include "diag_fpga_lib.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/


/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int diag_88e1340_smi_phy_wr(uint, int, int);
int diag_88e1340_smi_phy_rd(uint, int, int *);
int diag_88e1340_init(void);
/***********************************************************************
 *  Externs
 ************************************************************************/

/***********************************************************************
 *  Global Variable
 ************************************************************************/

/***********************************************************************
 *  Functions
 ************************************************************************/
int diag_88e1340_smi_phy_wr (uint phy_addr, int reg, int val)
{
    int bus_id;

    bus_id = get_smi_bus_id(phy_addr);
    
    return (woodlawn_phy_reg_wr(bus_id, phy_addr, reg, val));
}

int diag_88e1340_smi_phy_rd (uint phy_addr, int reg, int *val)
{
    int bus_id, rc;

    bus_id = get_smi_bus_id(phy_addr);
    rc = woodlawn_phy_reg_rd(bus_id, phy_addr, reg, val);
    if (rc == FAILED) {
        return (rc);
    }

    return (PASSED);
}

/*********************************************************************
 *   
 * Function: diag_88e1340_init
 *    
 * Description: Init 88e1340
 *      
 * Inputs: None
 *        
 * Outputs: PASSED
 *          
 **********************************************************************/
int diag_88e1340_init (void)
{
    int start_phy_addr[] = {0x8, 0xC};
    int phy_addr, rdval;
    int ix, jx, sku_id;
    int phy_num;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        phy_num = 1;
    } else {
        phy_num = 2;
    }

    for (ix = 0; ix < phy_num; ix++) {
        for (jx = 0; jx < 4; jx++) {
            phy_addr = start_phy_addr[ix] + jx;

            /* Init sequence #1 */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x00ff);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(24), 0x2800);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(23), 0x2001);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0000);

            /* Check init sequence #1
             */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x00ff);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(23), 0x1001);
            diag_88e1340_smi_phy_rd(phy_addr, PHY_REG(25), &rdval);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0000);

            /* Init sequence #2 for the BGA package
             */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0000);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(29), 0x0003);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(30), 0x0002);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(29), 0x0000);

            /* Check init sequence #2
             */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(29), 0x0003);
            diag_88e1340_smi_phy_rd(phy_addr, PHY_REG(30), &rdval);
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(29), 0x0000);

            /* Turn on the power
             * Set page 4 reg 0 (QSGMII control reg) power
             * down bit to normal
             */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0004);
            diag_88e1340_smi_phy_rd(phy_addr, PHY_REG(0), &rdval);
            rdval &= ~0x0800;
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(0), rdval);
            diag_88e1340_smi_phy_rd(phy_addr, PHY_REG(0), &rdval);

            /* Workaround for 88E1340 intermittent link status issue
             * under 156.25Mhz */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0001);
            diag_88e1340_smi_phy_rd(phy_addr, PHY_REG(0), &rdval);
            rdval |= 0x0800;
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(0), rdval);
            rdval &= ~0x0800;
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(0), rdval);
            diag_88e1340_smi_phy_rd(phy_addr, PHY_REG(0), &rdval);

            /* set register page 0 */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0000);

            /* set register page 4 */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0004);

            /* turn on qsgmii auto-negotiation and do soft-reset */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(0), 0x9140);

            /* set register page 1 */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(22), 0x0001);

            /* turn on sgmii auto-negotiation and do soft-reset */
            diag_88e1340_smi_phy_wr(phy_addr, PHY_REG(0), 0x9140);
        }
    }

    return (PASSED);
}
/***********************************************************************
 *  Static Functions
 ************************************************************************/


 /*-------------------------------------------------
 * $Log: diag_ge_phy_88E1340_lib.c,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/05/17 04:11:50  leschen
 * Turn on sgmii and qsgmii auto-nogotiation when init phy 1340
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.6  2013/03/29 08:54:11  leslie
 * Add comment to each function
 *
 * Revision 1.5  2013/03/27 08:45:05  kuangik
 * Code cleanup
 *
 * Revision 1.6  2013/02/18 07:57:51  leslie
 * Using woodlawn phy reg r/w lib instead of using ovld phy reg r/w lib
 *
 * Revision 1.5  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.4  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/04/06 06:06:30  kuangik
 * Update for 88E1340 Test item
 *
 * Revision 1.1  2012/02/10 06:51:53  leslie
 * Add Woodlawn phy lib file.
 * 
 *
 * $Endlog$
 *-------------------------------------------------
 */
