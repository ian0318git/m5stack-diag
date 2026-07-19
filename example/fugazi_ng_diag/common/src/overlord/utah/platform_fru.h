/* $Id: platform_fru.h,v 1.5 2014/02/24 00:28:06 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_fru.h
 *
 * Description: Enhanced error message for overlord FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2013, Eric Wu
 * Copyright (c) 2013-2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_FRU_H_
#define _PLATFORM_FRU_H_

/* define fru offset for overlord-utah platform */
typedef enum {
    MB = 0,
    MB_XAUI,
    MB_AUX,
    MB_I2C,
    MB_FPGA_REG,
    MB_PHY,
    MB_EMMC,
    MB_EUSB,
    MB_USB0,
    MB_MSATA,
    MEM_DIMM0,
    MEM_DIMM1,
    MB_SFP0,
    MB_SFP1,
    MB_SFP2,
    MB_SFP3,
    PSU0,
    PSU1,
    RPS,
    PVDM0,
    BACKPLANE,
    RISERCARD,
    SM0,
    SM1,
    WIC0,
    WIC1,
    WIC2,
    SM0_WIC,
    SM1_WIC,
    SM0_PVDM0,
    SM0_PVDM1,
    SM0_PVDM2,
    SM1_PVDM0,
    SM1_PVDM1,
    SM1_PVDM2,
    SM0_WIC0_DC,
    SM1_WIC0_DC,
    SM0_WIC1_DC,
    SM1_WIC1_DC,
    SM0_DC,
    SM1_DC,
} fru_offset_t;

extern uchar mb_pid[];
extern uchar dimm_pid[];
extern uchar sfp_pid[];
extern uchar psu_pid[];
extern uchar rps_pid[];
extern uchar pvdm_pid[];
extern uchar backplane_pid[];
extern uchar risercard_pid[];
extern uchar sm_pid[];
extern uchar wic_pid[];
extern uchar dc_pid[];

extern uchar mb_loc[];
extern uchar mb_xaui_loc[];
extern uchar mb_phy_loc[];
extern uchar dimm0_loc[];
extern uchar dimm1_loc[];
extern uchar sfp0_loc[];
extern uchar sfp1_loc[];
extern uchar sfp2_loc[];
extern uchar sfp3_loc[];
extern uchar psu0_loc[];
extern uchar psu1_loc[];
extern uchar rps_loc[];
extern uchar pvdm0_loc[];
extern uchar backplane_loc[];
extern uchar risercard_loc[];
extern uchar sm0_loc[];
extern uchar sm1_loc[];
extern uchar wic0_loc[];
extern uchar wic1_loc[];
extern uchar wic2_loc[];
extern uchar sm0wic_loc[];
extern uchar sm1wic_loc[];
extern uchar sm0pvdm0_loc[];
extern uchar sm0pvdm1_loc[];
extern uchar sm0pvdm2_loc[];
extern uchar sm1pvdm0_loc[];
extern uchar sm1pvdm1_loc[];
extern uchar sm1pvdm2_loc[];
extern uchar sm0wic0dc_loc[];
extern uchar sm1wic0dc_loc[];
extern uchar sm0wic1dc_loc[];
extern uchar sm1wic1dc_loc[];
extern uchar sm0dc_loc[];
extern uchar sm1dc_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif

/******** History ******** 
$Log: platform_fru.h,v $
Revision 1.5  2014/02/24 00:28:06  hroni
add definition for MB_EMMC, MB_EUSB, MB_USB0, and MB_MSATA

Revision 1.4  2014/02/21 06:53:44  hroni
add enhance error messages for i2c scan test, aux loopback test, and dash fpga register test

Revision 1.3  2014/02/19 09:11:34  alpeng
suport enhanced error code on loobpack tests

Revision 1.2  2013/12/04 18:53:55  mcharon
add entry for xaui to fru table

Revision 1.1  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in


$Endlog$
*/

