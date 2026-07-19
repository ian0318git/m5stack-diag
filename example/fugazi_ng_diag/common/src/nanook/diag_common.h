 /* $Id: diag_common.h,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_common.h,v $
 *------------------------------------------------------------------
 * Filename:   diag_common.h
 *
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_COMMON_H__
#define  __DIAG_COMMON_H__


#define SLEEP_10         10
#define SLEEP_100        100
#define SLEEP_1000       1000
#define SLEEP_5S         5*1000


#define GEPHY_INT_TIMEOUT       10
#define TIMEOUT_600      600

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)


#endif 

/*------------------------------------------------------------------
$Log: diag_common.h,v $
Revision 1.2  2019/12/11 10:10:28  lucywang
Merged Nanook to main trunk


*/
