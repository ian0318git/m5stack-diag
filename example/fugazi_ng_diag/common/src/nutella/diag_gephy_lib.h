/* $Id: diag_gephy_lib.h,v 1.6 2020/09/30 09:46:09 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_gephy_lib.h,v $
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
  
#ifndef __DIAG_GE_PHY_LIB_H__
#define __DIAG_GE_PHY_LIB_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"
#include "dev_88e1543.h"
/* For platform side use*/
#define SPD_10MBPS   10
#define SPD_100MBPS  100
#define SPD_1000MBPS 1000
#define SPD_COUNT    3
#define NUTELLA_PHY_START_ADDR       (0x0)
#define NUTELLA_PHY_MAX_PORT_NUM     3
#define PHY_1543_PAGE_REG_ADDR       (0x16)

extern int diag_gephy_dev_create(dev_88e1543_object_t *);
extern int diag_gephy_init (void);
extern int diag_gephy_smi_rd(uint, uint, uint, uint *); 
extern int diag_gephy_smi_wr(uint, uint, uint, uint);
extern int diag_gephy_get_linkup_status(uint, uint *);
extern int diag_gephy_read_page_reg(void);
extern int phy_88e1543_dev_init(dev_88e1543_object_t *, int, int);
void phy_88e1543_err_report(dev_object_t *, char *, uint32);



#endif

/*-------------------------------------------------
$Log: diag_gephy_lib.h,v $
Revision 1.6  2020/09/30 09:46:09  alicehua
CSCvv85097: Marvell 88e1543 Register test failed when port is plugged with cable

Revision 1.5  2019/10/16 23:47:51  alicehua
CSCvr66530: Add utility to read PHY page directly.

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
