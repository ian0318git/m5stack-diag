/* $Id: diag_ge_phy_88E1548L_lib.c,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1548L_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1548L_lib.c - Utility Menu and Functions for Woodlawn PHY 88E1548L
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "diag_fpga_lib.h"
#include "platform_eth.h"
#include "diag_common_drv.h"
#include "diag_ge_phy_88E1548L_lib.h"

int diag_88e1548_init(void);

/*******************************************************************************
 *  
 * Function    : get_88e1548_phy_addr
 *
 * Description : get 88e1548 phy addr
 *
 * Inputs      : port_num - port num
 *               port_map - port mapping
 *
 * Outputs     : phy_addr
 *       
 ********************************************************************************/
int get_88e1548_phy_addr (int port_num, int port_map)
{
    int phy_addr;

    if ((port_num == MRVL_1548_GE0) || (port_num == MRVL_1548_GE1)) {
        phy_addr = MRVL_88E1548_PHY1_SMI_ADDR + port_map;
    } else {
        phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
    }
    
    return (phy_addr);
}

int get_88e1548_4ge_phy_addr (int port_num, int port_map)
{
    int phy_addr;

    phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
    
    return (phy_addr);
}

/*******************************************************************************
 *  
 * Function    : get_88e1548_bus_id
 *    
 * Description : get 88e1548 bus id
 *      
 * Inputs      : phy_adr - phy addr
 *         
 * Outputs     : bus_id
 *          
 *********************************************************************************/
int get_88e1548_bus_id (int phy_addr)
{
    int bus_id;
    
    if (phy_addr & 0x4) {
        bus_id = SMI_BUS_1;
    } else {
        bus_id = SMI_BUS_0;
    }

    return (bus_id);
}

/*******************************************************************************
 *    
 * Function    : get_phy_port
 *    
 * Description : get 88e1548 phy port number
 *      
 * Inputs      : sku_id - sku id
 *               port_num - port number
 *         
 * Outputs     : port_map
 *           
 ********************************************************************************/
int get_phy_port (int sku_id, int port_num) 
{
    int port_map;
    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        port_map = old_ge_mapping_phy_port[port_num];
    } else {
        /* Official SKU */
        if (sku_id == WOODLAWN_6GE) {
            port_map = two_phy_ge_mapping_phy_port[port_num];
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            port_map = one_phy_ge_mapping_phy_port[port_num];
        }
    }
    
    return (port_map);
}

/*********************************************************************
 * 
 * Function: diag_88e1548_init
 *    
 * Description: Init 88e1548
 *      
 * Inputs: None
 *        
 * Outputs: PASSED
 *       
 ***********************************************************************/
int diag_88e1548_init (void)
{
    int start_phy_addr[] = {0x0, 0x4};
    int phy_addr, reg_data, bus_id;
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
            bus_id   = get_smi_bus_id(phy_addr);

            /* Init sequence
             */
            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x00fa);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 8, 0x0010);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x00fb);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x4099);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x1120);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 11, 0x113c);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 14, 0x8100);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 15, 0x112a);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x00fc);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x20b0);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x00ff);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x0000);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2000);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x4444);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2140);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x8064);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2141);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x0108);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2144);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x0f16);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2146);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x8c44);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x214b);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x0f90);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x214c);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0xba33);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x214d);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x39aa);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x214f);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x8433);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2151);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x2010);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2152);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x99eb);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2153);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x2f3b);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2154);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x584e);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2156);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x1223);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x2158);

            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0000);
            /* init sequence done */

            /* Turn on copper power
             * Read reg 0 after power up
             */
            woodlawn_phy_reg_rd(bus_id, phy_addr, 0x0, &reg_data);
            reg_data &= ~0x0800;
            woodlawn_phy_reg_wr(bus_id, phy_addr, 0x0, reg_data);

            woodlawn_phy_reg_rd(bus_id, phy_addr, 0x0, &reg_data);

            /* Fix LED mode into copper mode */
            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x3);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 16, 0x8211);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 17, 0x8844);
            woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0);

            /* set register page 4 */
            woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
			        MRV88E1548L_REG_PAGE_4);

            /* turn on qsgmii auto-negotiation and do soft-reset */
            woodlawn_phy_reg_wr(bus_id, phy_addr, 0x0, 0x9140);

            /* set register page 1 */
            woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
			        MRV88E1548L_REG_PAGE_1);

            /* turn on qsgmii auto-negotiation and do soft-reset */
            woodlawn_phy_reg_wr(bus_id, phy_addr, 0x0, 0x9140);
        }
    }

    return (PASSED);
}

void disable_ptp_engine (int eth_port)
{
    int start_phy_addr[] = {0x0, 0x4};
    int four_ge_eth_port_map[] = {0x0, 0x0, 0x0, 0x0, 0x3, 0x2, 0x1, 0x0};
    int six_ge_eth_port_map[] = {0x1, 0x0, 0x0,0x0, 0x3, 0x2, 0x1, 0x0};
    int phy_addr, bus_id;
    int sku_id;
    int port_num;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        port_num = four_ge_eth_port_map[eth_port];
        phy_addr = start_phy_addr[0];
    } else {
        port_num = six_ge_eth_port_map[eth_port];
        if (eth_port < 0x4) {
            phy_addr = start_phy_addr[1];
        } else {
            phy_addr = start_phy_addr[0];
        }
    }

     bus_id   = get_smi_bus_id(phy_addr);
     
    /* Disable PTP Core */
    /* RW U1 p0 R1 h3480 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3480 + (0x800 * port_num));
    /* RW U1 p0 R2 h0008 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0008);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    
    /* RW U1 p0 R1 h3080 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3080 + (0x800 * port_num));
    /* RW U1 p0 R2 h0008 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0008);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
}

/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1548L_lib.c,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/08/06 09:31:02  leschen
 * Add disable PTP engine function.
 *
 * Revision 1.1.2.2  2013/05/17 04:15:09  leschen
 * Turn on sgmii and qsgmii auto-nogotiation when init phy 1548
 *
 * Revision 1.1.2.1  2013/04/24 10:37:17  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/29 09:25:46  leslie
 * Add comment to each function
 *
 * Revision 1.2  2013/03/19 09:51:23  kuangik
 * Add retry mechanism (ported from O2) and reset quad phy if the test fails
 *
 * Revision 1.6  2013/02/19 08:39:40  leslie
 * Create lib for replacing redundant code
 *
 * Revision 1.5  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/04/06 06:03:54  kuangik
 * Update for GE PHY 88E1548 Test Item
 *
 * Revision 1.2  2012/03/26 07:18:27  kody
 * Add stdio.h
 *
 * Revision 1.1  2012/02/10 06:57:47  leslie
 * Add Woodlawn phy 88E1548L lib file.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
