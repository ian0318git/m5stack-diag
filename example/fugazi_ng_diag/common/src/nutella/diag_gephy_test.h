/* $Id: diag_gephy_test.h,v 1.5 2019/12/19 07:27:17 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_gephy_test.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_gephy_test.h
 * Description: Header file of Nutella GE PHY(Marvell 1514) platform.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_TEST_H__
#define __DIAG_GE_PHY_TEST_H__

#include "dev_88e1543.h"


#define LPBKTEST_PKT_CNT                (3)
#define LPBK_LINK_UP_TOUT               (500)
#define CHECK_LINK_UP_DELAY             (10)

/* Common */
#define NUTELLA_PHY_PORT0               0
#define NUTELLA_PHY_PORT1               1
#define NUTELLA_PHY_PORT2               2
#define NUTELLA_PHY_PORT3               3
#define NUTELLA_PHY_PORT_NUM               4
#define REG_BIT(x)  (1 << (x))

extern int build_gephy_test_menu(boolean);
extern int mrvl88e1543_dev_init;
extern dev_object_t *mrvl88e1543_get_object(void);

/* define loopback mode and port */
enum
{
    SGMII_PHY_LPBK_INTERNAL_PORT0,   /* internal loopback at marvell GE PHY port 0 */
    SGMII_PHY_LPBK_INTERNAL_PORT1,   /* internal loopback at marvell GE PHY port 1 */
    SGMII_PHY_LPBK_INTERNAL_PORT2,   /* internal loopback at marvell GE PHY port 2 */
    SGMII_PHY_LPBK_INTERNAL_PORT3,   /* internal loopback at marvell GE PHY port 3 */
    SGMII_PHY_LPBK_EXTERNAL_PORT0,   /* external loopback at marvell GE PHY port 0 */
    SGMII_PHY_LPBK_EXTERNAL_PORT1,   /* external loopback at marvell GE PHY port 1 */
    SGMII_PHY_LPBK_EXTERNAL_PORT2,   /* external loopback at marvell GE PHY port 2 */
    SGMII_PHY_LPBK_EXTERNAL_PORT3,   /* external loopback at marvell GE PHY port 3 */
};

#endif   /* __DIAG_GE_PHY_TEST_H__ */


/*-------------------------------------------------
$Log: diag_gephy_test.h,v $
Revision 1.5  2019/12/19 07:27:17  harrchan
Add single port test in GEPHY menu(CSCvs46809)

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
