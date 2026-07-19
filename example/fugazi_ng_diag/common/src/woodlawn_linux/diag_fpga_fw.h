/* $Id: diag_fpga_fw.h,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_fpga_fw.h,v $ 
 *-----------------------------------------------------------------------------
 * diag_fpga_fw.h - Woodlawn FPGA Firmware header file
 *
 * April 2012, Times Huang
 * Copyright (c) 2013 by Cisco Systems, Inc.
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
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:15  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:50  kuangik
 * Add for the first time
 *
 * Revision 1.5  2013/01/07 23:06:17  leslie
 * Fix fpga program firmware array declare issue.
 *
 * Revision 1.4  2012/10/04 03:23:22  leslie
 * Update for FPGA program utility.
 *
 * Revision 1.3  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/04/16 12:33:23  kuangik
 * Add FPGA Firmware for the first time
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
