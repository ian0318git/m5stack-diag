/* $Id: platform_smi.h,v 1.2 2017/08/02 14:21:49 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_smi.h,v $
 *------------------------------------------------------------------
 * Filename: platform_smi.h
 *
 * Description: Platform specific SMI header file.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SMI_H__
#define __PLATFORM_SMI_H__

/* Common */
#define TSN_SMI_RETRY_MAX   100

/* Externs */
extern int tsn_smi_read(int, int, ushort *);
extern int tsn_smi_write(int, int, ushort);
extern int tsn_smi_read_util(int);
extern int tsn_smi_write_util(int);

#endif   /* __PLATFORM_SMI_H__ */


/*------------------------------------------------------------------
$Log: platform_smi.h,v $
Revision 1.2  2017/08/02 14:21:49  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/04/29 10:14:57  palin2
Updated code and added support ext. loopback test after bring up Switch.

$Endlog$
*/

