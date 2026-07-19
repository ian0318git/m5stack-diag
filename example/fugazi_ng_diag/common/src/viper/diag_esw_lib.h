 /* $Id: diag_esw_lib.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_esw_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_esw_lib.h
 * Description: Header file of Ethernet Switch Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_ESW_LIB_H__
#define __DIAG_ESW_LIB_H__

#include "dev_88e6176.h"

extern int diag_esw_init(void);
extern int diag_reset_esw_to_default(int);
extern int diag_esw_dev_create(dev_88e6176_object_t *);

#endif  /* __DIAG_ESW_LIB_H__*/

/*-------------------------------------------------
 * $Log: diag_esw_lib.h,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.1  2018/02/27 08:06:34  harrchan
 * Initial viper application code base
 *
 * $Endlog$
 *-------------------------------------------------
 */
