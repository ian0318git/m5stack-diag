/* $Id: cookie_plat.h,v 1.1 2013/05/09 05:52:59 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/cookie_plat.h,v $
 *------------------------------------------------------------------
 * cookie_plat.h - From Xfromers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
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
    uchar UNUSED_1[3];          /* Unused (padding)		     13-15 */
    uchar serial[4];            /* BCD packed 8 digit serial number  16-19 */
    uchar UNUSED_2[7];          /* Unused                            20-26 */
    uchar magic[2];             /* Magic number for old cookies      27-28 */
    uchar capabilities[2];      /* capabilities (future stuff)       29-30 */
    uchar version;              /* cookie version number              31    */
    uchar UNUSED_3[96];         /* Unused                            32-127 */
};

#endif /* _COOKIE_PLAT_H_ */

/******** History ******** 
$Log: cookie_plat.h,v $
Revision 1.1  2013/05/09 05:52:59  alpeng
add utah tree

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
