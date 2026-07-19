/* $Id: quadra28_pkg.h,v 1.2 2018/05/18 09:24:58 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/quadra28_pkg.h,v $
 *-----------------------------------------------------------------------------
 * quadra28_pkg.h - Leverage from BCM API
 * Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/bcm_quadra28_app/quadra28_pkg.h
 *
 * August 2016, meho
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "miura_common.h"


#define SR4_LINE_SIDE       2
#define CR4_LINE_SIDE       13
#define XLAUI_LINE_SIDE     24
#define LR4_LINE_SIDE       28
#define KR4_LINE_SIDE       7   

#define KR4_SYSTEM_SIDE         7
#define CR4_SYSTEM_SIDE         13
#define XLAUI_SYSTEM_SIDE       24
#define SR4_SYSTEM_SIDE         2
#define LR4_SYSTEM_SIDE         28
#define SYSTEM_SIDE_INTERFACE   1
#define LINE_SIDE_INTERFACE     0
#define PHY_LANE                0xF
#define QUADRA_PHY_ID           0x0
#define MAX_QUADRA_PHYS         8

unsigned int FIRMWARE_VERSION = 0x23;
unsigned int REVISION_ID = 0xa0;
unsigned int CRC = 0x600d; 

unsigned int reg_array[] = {0xc804,0xd118,0xc161,0xC8D8,0xc8d9,0xC843,0xD0DC};  
unsigned int val_array[10]={0};
#ifdef DUEL_DIE_DUPLEX_FP_40G

int no_phy_ids = 7;

unsigned int if_side = 0x0; /*0 - Line side, 1 -system side */
unsigned int lane = 0xF;
//unsigned int  tx_rx = 0, rx = 1, tx=2, inv = 0, ena_dis = 1, lb = 0, time_val = 0;
int poly_array[]={0,   /* polynomial 7*/
    1,   /* polynomial 9*/
    2,   /* polynomial 11*/ 
    3,   /* polynomial 15*/
    4,   /* polynomial 23*/
    5,   /* polynomial 31*/
    6};  /* polynomial 58*/

#endif /*DUEL_DIE_DUPLEX*/


