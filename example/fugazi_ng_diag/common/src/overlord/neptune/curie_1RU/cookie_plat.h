/* $Id: cookie_plat.h,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/cookie_plat.h,v $
 *------------------------------------------------------------------
 * cookie_plat.h - From Xfromers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.3  2018/12/27 08:46:18  alpeng
cookie_plat.h is used for cookie_4_core.c; we need it

Revision 1.1.2.1  2018/06/22 08:05:17  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:35  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:24:58  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2016/06/02 22:04:01  jskow
Move Overlord/x86 specific files to Neptune/x86.

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
