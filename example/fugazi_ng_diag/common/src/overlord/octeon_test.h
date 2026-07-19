/* $Id: octeon_test.h,v 1.1 2013/05/09 05:42:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/octeon_test.h,v $
 *------------------------------------------------------------------
 * octeon_test.h - Octeon test header file
 *
 * Paul Tong, July 2011
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __OCTEON_TEST_H__
#define __OCTEON_TEST_H__

extern int octeon_tests(boolean);
extern int oct_remote_reset(void);
extern int oct_remote_memcheck(void);
extern int oct_remote_core(void);
extern int oct_remote_bootuboot(void);
extern int oct_remote_bootlinux(int);
extern int switch_to_dp_console(void);

#endif /* __OCTEON_TEST_H__ */

/******** History ******** 
$Log: octeon_test.h,v $
Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.6  2012/11/07 10:58:15  alpeng
remove useless file and clean up code

Revision 1.5  2012/09/12 23:44:59  ptong
Code cleanup and add comments

Revision 1.4  2012/04/17 22:01:27  ptong
Added more utility to run DP test from host.

Revision 1.3  2012/04/11 21:27:17  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
