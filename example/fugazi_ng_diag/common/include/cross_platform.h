/* $Id: cross_platform.h,v 1.16 2021/02/24 03:45:59 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/cross_platform.h,v $
 *------------------------------------------------------------------
 * cross_platform.h - defines shared by different platforms.
 *
 * Feburary 1999, Ling Lee
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *------------------------------------------------------------------
 */

/*
 * Revision 1.1  1999/04/19 23:05:20  lingl
 * VIC defines shared by different platforms.
 */

#ifndef PLATFORM_H
#define PLATFORM_H
 
/*
 * defines for mother board type device or network module
 */
enum {
    MOTHER_BOARD =  0, /* used for mother board's cookie   */
    NETWK_MODULE =  1,  /* used for network module's cookie */
    WIC_MODULE   =  2,  /* used for WIC module's cookie     */
    VIC_MODULE   =  3,  /* used for VIC module's cookie     */
    SPMM_MODULE  =  4,  /* used for SPMM module's cookie    */
    DAUGHTER_CARD = 5,  /* used for daughter card cookie    */
    VM_MODULE   = 6,  /* used for PVDM module cookie on MB */
    AIM_MODULE    = 7,  /* used for AIM module cookie */
    BACK_PLANE    = 8,  /* for Mid Rise module cookie */
    ISM_MODULE    = 9,  /* for Integrated Service Module cookie */
    NM_ADAPTER    = 10,  /* for Integrated Service Module cookie */
    SM_MODULE     = 11,  /* for Service Module cookie */
    PSU_MODULE    = 12,  /* for Power Supply Uint Module cookie */
    SM_DAUGHTER_CARD  = 13, /* for SM's daughter card cookie */
    WIC_DAUGHTER_CARD,   /* for SM's daughter card cookie */
    HW_CRYPTO_ACC, /* Hardware based crypto accelerator */
    PSU_AC_MODULE, /* for Power Supply AC Module cookie */
    PSU_DC_MODULE,  /* for Power Supply DC Module cookie */
    RISER_CARD,  /* for Riser Card Module cookie */
    ISP_CARD,
    SM_VM_DAUGHTER_CARD, /* for SM's VM daughter card cookie */
    PLUGGABLE_CARD,      /* for Pluggable daughter card cookie */
    SM_DC_WIC_CARD,      /* for SM Daughter WIC card cookie, the SM(like Switzer-Carrier) will be separated to two NIM */
    SM_DC_WIC_DC_VM_CARD,/* for SM Daughter WIC Daughter VM card cookie, the SM(like Switzer-Carrier) will be separated to two NIM */
    MAX_DEVICE_TYPE,
};

#define BDTYPE_UNKNOWN               0xFF
#define BDTYPE_DEVELOPMENT           0xFE   /* Eval/development board */   
#define BDTYPE_OVERLORD              0x00
#define BDTYPE_JUNO                  0x01
#define BDTYPE_UTAH                  0x02
#define BDTYPE_SWORD                 0x03
#define BDTYPE_DAGGER                0x04
#define BDTYPE_TACHI_ENTRY           0x05
#define BDTYPE_TACHI_HIGH            0x06
#define BDTYPE_GOLDBEACH             0x07
#define BDTYPE_VG400                 0x08
#define BDTYPE_NEPTUNE               0x10
#define BDTYPE_TRITON                0x11
#define BDTYPE_PROTEUS               0x12
#define BDTYPE_NESO                  0x13
#define BDTYPE_VG450                 0x14
#define BDTYPE_NEPTUNIUM             0x15
#define BDTYPE_URANIUM               0x16
#define BDTYPE_THORIUM               0x17
#define BDTYPE_RADIUM                0x18
#define BDTYPE_POLONIUM              0x19
#define BDTYPE_THALLIUM              0x1A
#define BDTYPE_TABEI_L               0x1B
#define BDTYPE_NANOOK                0x56
#define BDTYPE_NANOOK_PLUS_4G        0x57
#define BDTYPE_NANOOK_PLUS_8G        0x58

#define BOARD_REVISION_00            0x00
#define BOARD_REVISION_01            0x01
#define BOARD_REVISION_02            0x02
#define BOARD_REVISION_03            0x03

#endif 


/******** History ******** 
$Log: cross_platform.h,v $
Revision 1.16  2021/02/24 03:45:59  xiaolaya
Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie

Revision 1.15  2019/12/11 10:10:22  lucywang
Merged Nanook to main trunk

Revision 1.14  2019/10/17 02:16:14  kehuang2
Collapse Tabei-L into main trunk

Revision 1.13  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.12  2018/08/30 07:03:45  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.11  2018/05/18 09:24:47  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.10  2018/02/09 09:10:26  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.9.22.1  2018/01/20 06:33:45  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.9.6.1  2017/08/15 14:03:14  hondwang
star branch c9xx initial check in

Revision 1.9  2017/07/28 07:49:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.8  2016/10/16 12:28:09  iachang
Supported Goldbeach Platform.

Revision 1.7.16.1  2017/06/13 09:34:52  tirawan
Add Pluggable Card type

Revision 1.7.2.6  2018/05/17 10:50:19  alpeng
 sync with trunk <trunk-051618>

Revision 1.7.2.5  2017/11/22 08:09:17  leschen
Support Barsoom VG450.

Revision 1.7.2.4  2017/09/19 10:18:51  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.7.2.3  2017/04/05 06:30:28  leschen
Sync with <ng_diag-tag-032917>

Revision 1.7.2.2  2016/06/10 18:11:09  ptong
Add is_neso

Revision 1.7.2.1  2016/06/02 10:33:13  leschen
Support to check Neptune/Triton/Proteus board type.

Revision 1.10  2018/02/09 09:10:26  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.9.22.1  2018/01/20 06:33:45  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.9.6.1  2017/08/15 14:03:14  hondwang
star branch c9xx initial check in

Revision 1.9  2017/07/28 07:49:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.8  2016/10/16 12:28:09  iachang
Supported Goldbeach Platform.

Revision 1.7.16.1  2017/06/13 09:34:52  tirawan
Add Pluggable Card type

Revision 1.7  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.6.12.2  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.6.12.1  2015/07/31 07:57:03  alpeng
update board type for tachi entry

Revision 1.6  2014/11/26 03:45:16  alpeng
reverting to version 1.4

Revision 1.4  2013/07/03 23:44:14  ptong
added board type for utah platform

Revision 1.3  2013/05/31 12:51:34  danchung
Add checking board type for Juno.

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
