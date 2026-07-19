/* $Id: diag_gephy_1543_lib.h,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_gephy_1543_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_lib.h
 * Description: Header file of GE PHY Library
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_GE_PHY_1543_LIB_H__
#define __DIAG_GE_PHY_1543_LIB_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"
#include "dev_88e1543.h"
/* For platform side use*/
#define SPD_10MBPS   10
#define SPD_100MBPS  100
#define SPD_1000MBPS 1000
#define SPD_COUNT    3
#define GE_RESET_TIMER  100 
#define NANOOK_PHY0_START_ADDR       (0x8)

extern char inface_lan1p0[32];
extern char inface_lan1p1[32];

/* define qsgmii configuration mode */
enum
{
    COPPER = 0,
    FIBER,
};
extern int diag_gephy_1543_init (void);
extern int diag_gephy_smi_rd(uint, uint, uint, uint *); 
extern int diag_gephy_smi_wr(uint, uint, uint, uint);
extern int phy_88e1543_dev_init(dev_88e1543_object_t *, int, int);
void phy_88e1543_err_report(dev_object_t *, char *, uint32);
extern int diag_gephy_get_flag(int);


#endif

/*-------------------------------------------------
$Log: diag_gephy_1543_lib.h,v $
Revision 1.2  2019/12/11 10:10:29  lucywang
Merged Nanook to main trunk


$Endlog$
*/
