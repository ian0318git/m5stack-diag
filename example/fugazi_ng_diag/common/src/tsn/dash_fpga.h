/* $Id: dash_fpga.h,v 1.2 2017/08/02 14:21:44 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/dash_fpga.h,v $
 *------------------------------------------------------------------
 * Filename:    dash_fpga.h
 *
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 * TSN does not have Dash FPGA, create this dummy file for
 * common source file cookie_4_core.c needs to include it.
 *
 *------------------------------------------------------------------
 */
#ifndef __DASH_FPGA__
#define  __DASH_FPGA__

#define FPGA_RST_ACT2   0

extern int read_eeprom_block(unsigned int, unsigned int size, unsigned char *);
extern void reset_plat_dev(unsigned int);
extern void unreset_plat_dev(unsigned int);
extern void reset_tam_dev(void);

#endif  /* #if __DASH_FPGA */
/*------------------------------------------------------------------
$Log: dash_fpga.h,v $
Revision 1.2  2017/08/02 14:21:44  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:01  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:03  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3  2016/08/09 09:47:54  iachang
Supported FPGA/Aikido firmware upgrade.

Revision 1.1.4.2  2016/06/30 06:22:47  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/22 09:26:12  leschen
Initial check in.

*/
