/* $Id: reva_reg.h,v 1.3 2018/07/23 06:45:00 easochen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/reva_reg.h,v $
 *------------------------------------------------------------------
 * reva_reg.h 
 *      Reva projects - NIM 16A/24A
 *                        memory map and register structures.
 *
 * Copyright (c) 2015-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __REVA_REG__
#define __REVA_REG__

#define BOARD_ID_SHIFT      8
#define BOARD_ID_NIM_16A     0
#define BOARD_ID_NIM_24A     1
#define BOARD_ID_NIM_16A_4G     2
#define BOARD_ID_NIM_24A_4G     3

#define ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET      0x2600
#define ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET      0x2800

/* Defines for LED and Status Register */
#define LED_3_AMBER         0x00000800
#define LED_3_GREEN         0x00000400
#define LED_2_AMBER         0x00000008
#define LED_2_GREEN         0x00000004
#define LED_1_AMBER         0x00000002
#define LED_1_GREEN         0x00000001

#endif /* end __REVA_REG__ */

/******** History ********
$Log: reva_reg.h,v $
Revision 1.3  2018/07/23 06:45:00  easochen
Reva: Add new board_id and firmware for 4G DDR

Revision 1.2  2016/05/06 03:43:53  umlin
Reva: Commit Reva module side diag codes to main trunk


$Endlog$
*/
