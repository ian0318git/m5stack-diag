/* $Id: diag_geswitch_test.h,v 1.4 2019/09/10 01:03:39 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_geswitch_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_geswitch_test.h - Header file for GE PHY 88E1512 Test
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_GESWITCH_TEST__
#define __DIAG_GESWITCH_TEST__

extern int diag_geswitch_build_test(int);
extern int geswitch_loopback_enable(int, int);
extern int diag_geswitch_i350_eth1_lpbk_test(void);
extern int geswitch_apply_errata(void);

/*define all related mrvl6320 macro */
#define MRVL6320_ADDRESS              (0x8)

#define PACKET_COUNT                    (3)
#define MRVL6320_PORT_0                 (0)
#define MRVL6320_PORT_2                 (2)
#define MRVL6320_PORT_4                 (4)
#define MRVL6320_PORT_5                 (5)
#define MRVL6320_PORT_6                 (6)
#define MRVL6320_GLOBAL_1                (11)
#define MRVL6320_GLOBAL_2                (28)
#define MRVL6320_PORT_BASED_VLAN_MAP    (6)
#define MRVL6320_PORT_0_LOOPBACK        (0x1)
#define MRVL6320_PORT_5_LOOPBACK        (0x20)
#define MRVL6320_P5_VLAN_ALL_ONE        (0x5F)
#define MRVL6320_PORT_CTRL_PORT_STATE   (0x03)
#define MRVL6320_PORT_CTRL_REG          (4)
#define MRVL6320_VLAN_PORT5_PORT6       (0x40)
#define MRVL6320_VLAN_PORT6_PORT5       (0x20)
#define MRVL6320_VLAN_PORT5_PORT2       (0x4)
#define MRVL6320_VLAN_PORT2_PORT5       (0x20)
#define MRVL6320_VLAN_PORT0_PORT2       (0x4)
#define MRVL6320_VLAN_PORT2_PORT0       (0x1)
#define MRVL6320_VLAN_PORT0_PORT5       (0x20)
#define MRVL6320_VLAN_PORT5_PORT0       (0x1)
#define MRVL6320_VLAN_PORT6             (0x3f)
#define MRVL6320_VLAN_PORT5             (0x5f)
#define MRVL6320_VLAN_PORT2             (0x7b)
#define MRVL6320_VLAN_PORT0             (0x7f)
#define MRVL6320_REG_26                 (26)
#define MRVL6320_ERRATA_1               (0xe000)
#define MRVL6320_ERRATA_2               (0xc1e7)
#define MRVL6320_ERRATA_3               (0x81e7)
#define MRVL6320_ERRATA_4               (0xfdb7)
/* 6320  */
#define MRVL6320_PHY_CTRL_REG            (1)
#define MRVL6320_JAMMING_CTRL_REG        (2)
#define MRVL6320_PORT_CTRL_REG           (4)
#define MRVL6320_PORT_CTRL_1_REG         (5)
#define MRVL6320_PORT_BASED_VLAN_MAP_REG (6)
#define MRVL6320_PORT_CTRL_2_REG         (8)
#define MRVL6320_EGRESS_RATE_CTRL_REG    (9)
#define MRVL6320_SWITCH_CTRL_REG         (4)

#define MRVL6320_EE_INT_EN               (1)
#define MRVL6320_INIT_DELAY              (2000)

#define GESWITCH_RW     (READ_WRITE | SAVE_RESTORE | REG_ACCESS)
#define GESWITCH_RO     (READ_ONLY | SAVE_RESTORE | REG_ACCESS)

#define MARVL_ERRATA_SET  "echo 1 > /var/log/marvell_errata_applied.txt"
#define I350_VID 0x157
#define I350_VID_PROC_PATH "/proc/i350_vid" 
#endif /* __DIAG_GESWITCH_TEST__ */

/*---------------------------------------------------------------
$Log: diag_geswitch_test.h,v $
Revision 1.4  2019/09/10 01:03:39  haohsu
[CSCvr07313]-Marvell 6320 to BMC eth1 frame error issue

Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.11  2016/04/08 07:33:29  benchen2
move i350 ncsi lpbk to intel

Revision 1.1.2.10  2016/03/10 00:42:43  huanngo
Adding I350 datapath test

Revision 1.1.2.9  2016/03/09 07:36:47  benchen2
add 6320 interrupt test

Revision 1.1.2.8  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.7  2015/10/14 07:21:06  alpeng
update get host mac addr for f35

Revision 1.1.2.6  2015/10/02 04:35:53  benchen2
fix 710 lpbtest phase 1

Revision 1.1.2.5  2015/09/15 06:46:53  benchen2
add i350 lpbk func

Revision 1.1.2.4  2015/08/14 05:54:02  benchen2
add verbose flag

Revision 1.1.2.3  2015/08/04 02:26:45  hondwang
add packet count define

Revision 1.1.2.2  2015/07/31 07:33:27  hondwang
geswitch test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/
