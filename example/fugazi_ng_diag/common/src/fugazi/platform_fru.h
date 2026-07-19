/* $Id: platform_fru.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_fru.h
 *
 * Description: Enhanced error message for overlord FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2016, Eric Wu
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_FRU_H_
#define _PLATFORM_FRU_H_

/* define fru offset for fugazi  platform */
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

/*-------------------------------------------------
 * $Log: platform_fru.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.3  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

