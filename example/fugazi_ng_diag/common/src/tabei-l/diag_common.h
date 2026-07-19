 /* $Id: diag_common.h,v 1.2 2019/10/17 02:16:19 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_common.h,v $
 *------------------------------------------------------------------
 * Filename:   diag_common.h
 *
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_COMMON_H__
#define  __DIAG_COMMON_H__


#define SLEEP_1          1
#define SLEEP_10         10
#define SLEEP_100        100
#define SLEEP_250        250
#define SLEEP_1000       1000
#define SLEEP_2000       2000
#define SLEEP_5S         5*1000


#define GEPHY_INT_TIMEOUT       10
#define TIMEOUT_600      600


/*
 *  * Main menu test flag defines
 *   */
#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)


#endif 

/*------------------------------------------------------------------
$Log: diag_common.h,v $
Revision 1.2  2019/10/17 02:16:19  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.6  2019/08/29 03:49:26  kehuang2
Clean up code by the comment of code review

Revision 1.1.4.5  2019/08/26 07:54:59  kehuang2
Clean up code by the comment of code review

Revision 1.1.4.4  2019/07/31 08:00:48  olin2
Clean up code

Revision 1.1.4.3  2019/03/07 05:53:15  olin2
Clean up code

Revision 1.1.4.2  2018/10/02 01:49:58  harrchan
Initial commit for Tabei-L P1A bring up.

*/
