 /* $Id: diag_gephy_lib.h,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_gephy_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_lib.h
 * Description: Header file of GE PHY Library
 * 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_GE_PHY_LIB_H__
#define __DIAG_GE_PHY_LIB_H__

#include "dev_88e151x.h"

extern int diag_gephy_dev_create(int, dev_88e151x_object_t *);
extern int diag_gephy_init (void);

#endif

/*-------------------------------------------------
 * $Log: diag_gephy_lib.h,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
