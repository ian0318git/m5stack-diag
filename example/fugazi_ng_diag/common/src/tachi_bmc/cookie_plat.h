/* $Id: cookie_plat.h,v 1.2 2016/04/20 11:25:25 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/cookie_plat.h,v $
 *------------------------------------------------------------------
 *
 * cookie_plat.h - Header file for platform cookie
 *
 * June 2015, Times Huang ported from Overlord
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __COOKIE_PLAT__
#define __COOKIE_PLAT__

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

#endif /* __COOKIE_PLAT__ */

/*---------------------------------------------------------------
$Log: cookie_plat.h,v $
Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/
