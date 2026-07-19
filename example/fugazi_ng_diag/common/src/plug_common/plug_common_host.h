/* $Id: plug_common_host.h,v 1.4 2019/08/06 06:56:16 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_common_host.h,v $
 *------------------------------------------------------------------
 *
 * plug_common_host.h - Header file for Pluggable Common Host 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_COMMON_HOST__
#define __PLUG_COMMON_HOST__

extern int plug_common_host_i2c_rd(uint8_t, uint8_t, uint32, char *);
extern int plug_common_host_i2c_wr(uint8_t, uint8_t, uint32, char);
extern int plug_common_host_i2c_rd_2bytes(uint8_t, uint8_t, uint32, ushort *);
extern int plug_common_host_i2c_wr_2bytes(uint8_t, uint8_t, uint32, ushort);
extern int plug_common_host_usb_3p0_mode_set(int);
extern int plug_common_host_usb_2p0_mode_set(int);
extern int plug_common_host_diag_fpga_reg_bitops(uint, uint, uint);
extern int plug_common_host_plug_fpga_reg_read(uint, uint *);
extern int plug_common_host_plug_fpga_reg_write(uint, uint);
extern ushort plug_common_host_get_cookie_id(int, int, uchar *,uint16_t *, char *);
extern void plug_common_host_usb_hub_reset(int);
extern void plug_common_host_pcie_dev_disable(int);

#endif

/*-------------------------------------------------
$Log: plug_common_host.h,v $
Revision 1.4  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.1  2018/10/15 06:50:18  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 04:53:28  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:39:41  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.1  2017/07/20 17:23:10  tirawan
Add Pluggable host implementation codes



*/

