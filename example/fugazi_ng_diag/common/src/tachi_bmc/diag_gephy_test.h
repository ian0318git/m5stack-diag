/* $Id: diag_gephy_test.h,v 1.2 2016/04/20 11:25:32 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_gephy_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_test.h - Header file for GE PHY 88E1512 Test
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_GEPHY_TEST__
#define __DIAG_GEPHY_TEST__

extern int diag_mgmt_gephy_build_test(int);
extern int diag_ncsi_gephy_build_test(int);
extern int gephy_if_enable(char *, int);
extern int diag_ncsi_gephy_pass_through_start (void);
extern int diag_ncsi_gephy_pass_through_stop (void);

#define PHY_LOOPBACK_REG             (0x4000)
#define PHY_RESET_TIMEOUT            (100)
#define PHY_STATUS_LINK_UP           (0x0004)
#define GEPHY_RW                     (READ_WRITE | SAVE_RESTORE | REG_ACCESS)
#define GEPHY_RO                     (READ_ONLY | SAVE_RESTORE | REG_ACCESS)
#define MRV88E1512_REG_PAGE_250      (250)

/* P0R9*/
#define MRVL1512_1000B_T_CTRL_HALF_DUPLEX              (0x100)
#define MRVL1512_1000B_T_CTRL_M_S_CONF_EN              (0x1000)
#define MRVL1512_1000B_T_CTRL_M_S_CONF_VAL             (0x800)
#define MRVL1512_1000B_T_CTRL_M_S_CONF_PORT_TYPE       (0x400)
#define MRVL1512_1000B_T_CTRL_1G_SETTING               (MRVL1512_1000B_T_CTRL_M_S_CONF_EN | MRVL1512_1000B_T_CTRL_M_S_CONF_VAL | MRVL1512_1000B_T_CTRL_M_S_CONF_PORT_TYPE)

/* P0R16*/
#define MRVL1512_COPPER_SPEC_CTRL_DIS_LINK_PULSES      (0x8000)

/* P250*/
#define MRV88E1512_REG_PAGE_250_REG_1 (1)
#define MRV88E1512_REG_PAGE_250_REG_7 (7)
#define MRV88E1512_REG_P250_R1        (0x0418)
#define MRV88E1512_REG_P250_R1_ORI    (0x0400)
#define MRV88E1512_REG_P250_R7        (0x020c)
#define MRV88E1512_REG_P250_R7_ORI    (0x0200)

/* P6R16*/
#define MRV88E1512_COPPER_PORT_PKT_GEN        (16)
#define MRV88E1512_COPPER_PORT_PKT_GEN_EN_CRC (0x10)

#endif /* __DIAG_GEPHY_TEST__ */

/*---------------------------------------------------------------
$Log: diag_gephy_test.h,v $
Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.6  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.5  2015/09/14 07:25:37  benchen2
fix 1512 define error

Revision 1.1.2.4  2015/09/14 07:09:47  benchen2
phy1512 lpbk test

Revision 1.1.2.3  2015/08/04 02:26:59  hondwang
add packet count define

Revision 1.1.2.2  2015/07/31 07:31:49  hondwang
gephy test

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/
