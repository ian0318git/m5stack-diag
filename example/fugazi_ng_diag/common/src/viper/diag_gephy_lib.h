 /* $Id: diag_gephy_lib.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_gephy_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_lib.h
 * Description: Header file of GE PHY Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.2  2018/03/14 06:59:34  olin2
 * Modify 1514 init sequence
 *
 * Revision 1.1.2.1  2018/02/27 08:06:43  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
