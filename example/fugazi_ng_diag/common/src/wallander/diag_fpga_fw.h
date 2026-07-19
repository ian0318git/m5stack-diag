/* $Id: diag_fpga_fw.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_fpga_fw.h,v $ 
 *-----------------------------------------------------------------------------
 * diag_fpga_fw.h - Wallander FPGA Firmware header file
 *
 * Apr 2014, Xiaoying Zhang
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_FPGA_FW_H__
#define __DIAG_FPGA_FW_H__

extern unsigned short fpga_fw_major_rev;
extern unsigned short fpga_fw_minor_rev;

extern unsigned char fpga_fw_array[];
extern unsigned char fpga_normal_image_fw_array[];

extern unsigned long fgpa_fw_size;
extern unsigned long fpga_normal_image_fw_size;


#endif /* __DIAG_FPGA_FW_H__ */

/*-------------------------------------------------
 * $Log: diag_fpga_fw.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
