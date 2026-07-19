/* $Id: plug_testcard_util.h,v 1.2 2018/01/20 05:01:10 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_util.h,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_util.h - Header file for Pluggable Test card Utilities
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_TESTCARD_UTIL__
#define __PLUG_TESTCARD_UTIL__

typedef enum {
    OPT_READ,
    OPT_WRITE
} reg_util_opt_t;

extern int plug_testcard_util(void);

#endif

/*-------------------------------------------------
$Log: plug_testcard_util.h,v $
Revision 1.2  2018/01/20 05:01:10  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
add pluggable testcard for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:22  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.2  2017/06/22 19:27:12  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

