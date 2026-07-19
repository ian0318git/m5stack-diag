/* $Id: diag_fru_util.h,v 1.2 2016/04/20 11:25:32 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fru_util.h,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */

#ifndef DIAG_FRU_UTIL_H_
#define DIAG_FRU_UTIL_H_

extern int diag_fru_util(void);

#define FRU_SET       0
#define FRU_SHOW      1
#define FRU_INVALIDTE 9
#define FRU_ZERO      10

#define FRU_PLATFROM  1
#define FRU_TYPE      10
#define FRU_SLOT      0



#endif /* DIAG_FRU_UTIL_H_ */
