/* $Id: reva_sm_reg.h,v 1.3 2017/03/20 09:44:02 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/sm/reva_sm_reg.h,v $
 *------------------------------------------------------------------
 * reva_reg.h 
 *      Reva projects - SM 64A
 *                        memory map and register structures.
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "reva_reg.h"

#define BOARD_ID_SM_64A_2G     0
#define BOARD_ID_SM_64A_4G     1

/* Defines for LED and Status Register */
#define LED_8_AMBER         (0x00200000<<1)
#define LED_8_GREEN         (0x00100000<<1)
#define LED_7_AMBER         (0x00080000<<1)
#define LED_7_GREEN         (0x00040000<<1)
#define LED_6_AMBER         (0x00020000<<1)
#define LED_6_GREEN         (0x00010000<<1)
#define LED_5_AMBER         (0x00008000<<1)
#define LED_5_GREEN         (0x00004000<<1)
#define LED_4_AMBER         (0x00002000<<1)
#define LED_4_GREEN         (0x00001000<<1)

/******** History ********
$Log: reva_sm_reg.h,v $
Revision 1.3  2017/03/20 09:44:02  umlin
Reva-SM:
Start from P1C-2nd:
1. Change board id from 0x2 to 0x0 for Reva-SM-2G and 0x1 for Reva-SM-4G
2. Including secure boot image upgrade

Revision 1.2  2017/03/16 05:20:26  umlin
Reva-SM: Commit Reva-SM module side diag codes to main trunk

Revision 1.1.2.3  2016/12/08 07:31:51  umlin
Reva-SM: Depend on FPGA, this version still need to skip MAC loopback and LED test.

Revision 1.1.2.2  2016/12/05 07:38:27  umlin
Reva-SM: 1. Support new LEDs and MAC loopback. 2.New 7z015 image upgrade.

Revision 1.1.2.1  2016/10/18 22:05:19  umlin
Reva-SM: SM-Module side diag, refer to FPGA 24~63 ports memory mapping to add those async loopback test. Removed PPP loopback function because of FPGA PPP logic is removed due to FPGA resource constraint.



$Endlog$
*/
