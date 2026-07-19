/* $Id: cookie_plat.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/cookie_plat.h,v $
 *------------------------------------------------------------------
 * cookie_plat.h - From Xfromers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2008-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _COOKIE_PLAT_H_
#define _COOKIE_PLAT_H_

typedef struct cookie_plat cookie_plat_t;
struct cookie_plat {		/* byte #  */
    uchar interfaces;           /* Which interfaces does it have?      0   */
    uchar vendor;               /* Which vendors version of the IGS    1   */
    uchar ether0_hw_addr[6];    /* Increment for ethernet 1           2-7  */
    uchar processor;            /* Which processor is this?           8    */
    uchar hw_rework[4];         /* Mfg. Rework number (deviation)     9-12 */
    uchar UNUSED_1[3];          /* Unused (padding)                  13-15 */
    uchar serial[4];            /* BCD packed 8 digit serial number  16-19 */
    uchar UNUSED_2[7];          /* Unused                            20-26 */
    uchar magic[2];             /* Magic number for old cookies      27-28 */
    uchar capabilities[2];      /* capabilities (future stuff)       29-30 */
    uchar version;              /* cookie version number             31    */
    uchar UNUSED_3[96];         /* Unused                            32-127*/
};

#endif /* _COOKIE_PLAT_H_ */


/*-------------------------------------------------
 * $Log: cookie_plat.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:05  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:24  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */
