/* $Id: plat_defs.h,v 1.4 2016/07/12 01:53:19 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Tachi BMC platform defines.
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2011-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_


#define NGWIC1_SLOT    1
#define NGWIC2_SLOT    2
#define NGWIC3_SLOT    3

/* GESW use these macro
 */
#define TGT_DEV_CPU     0 /* Control Plane CPU */
#define TGT_DEV_NGSM    1
#define TGT_DEV_NGWIC   2
#define TGT_DEV_NGVM    3
#define TGT_DEV_DP      4 /* Data Plane cpu */

/* Cavecreek SGMII 0 is system management port,
 * SGMII 1 to 4 are connected to the GE switch
 */
#define CPU_SGMII_PORT0         0
#define CPU_SGMII_PORT1         1
#define CPU_SGMII_PORT2         2
#define CPU_SGMII_PORT3         3
#define CPU_SGMII_PORT4         4
#define NUMBER_OF_CPU_PHY_PORTS	    1
#define NUMBER_OF_CPU_SW_PORTS	    3

/* Daughter card */
#define POE_CARD      (1)
#define RAID_CARD     (2)

int msleep_delay; 
int skip_init_seq;

#endif  /* _PLAT_DEFS_H_ */

/******** History ******** 
$Log: plat_defs.h,v $
Revision 1.4  2016/07/12 01:53:19  hondwang
Fix F2W bug and add PCAMAP ID

Revision 1.3  2016/06/04 09:22:20  alpeng
initial check in for f2w

Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.4  2016/01/18 07:02:28  alpeng
update cookie info for read mac

Revision 1.1.2.3  2015/08/14 09:50:37  alpeng
create intel dir and update plat_def

Revision 1.1.2.2  2015/08/11 07:44:28  meho
Added f35 nim tests.

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project



$Endlog$
*/
