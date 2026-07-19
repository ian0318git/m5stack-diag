/* $Id: dash_fpga.h,v 1.6 2020/01/09 01:02:37 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/dash_fpga.h,v $
 *------------------------------------------------------------------
 * Filename:    dash_fpga.h
 *
 * Copyright (c) 2015-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 * CSX-Tachi does not have Dash FPGA, create this dummy file for
 * common source file cookie_4_core.c needs to include it.
 *
 *------------------------------------------------------------------
 */
#ifndef __DASH_FPGA__
#define  __DASH_FPGA__

#define FPGA_RST_ACT2   0
#define ISP_FPGA_RST_ACT2   0x32090

extern int read_eeprom_block(unsigned int, unsigned int size, unsigned char *);
extern void reset_plat_dev(unsigned int);
extern void unreset_plat_dev(unsigned int);
extern void reset_isp_dev(unsigned int);
extern void unreset_isp_dev(unsigned int);
#ifdef TACHI
extern int tachi_get_ge_sw_port_num(int slot, int tgt_device, int local_port);
#else
extern int ovld_get_ge_sw_port_num(int slot, int tgt_device, int local_port);
#endif
extern int set_gesw_line_loopback(int port_num, int onoff);
extern int is_goldbeach(void); /* extern the dummy function for cross platform Goldbeach */
extern int is_curie_1ru(void); 
extern int is_curie_2ru(void);

#endif  /* #if __DASH_FPGA */
/*------------------------------------------------------------------
$Log: dash_fpga.h,v $
Revision 1.6  2020/01/09 01:02:37  jiajliu
Merge Curie 2RU to main trunk

Revision 1.5  2019/08/06 06:56:17  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.4.38.1  2019/07/25 00:43:02  alpeng
add weak function is_curie_1ru() on tachi

Revision 1.4  2017/08/10 10:12:48  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.2  2017/02/21 03:50:36  haohsu
Add NIM Dynamo to TACHI

Revision 1.2.14.1  2016/12/22 08:45:48  haohsu
Add NIM on Tachi-l

Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.1.2.2  2016/01/26 06:27:55  benchen2
add daughter card ACT2 programming

Revision 1.1.2.1  2015/07/24 03:39:35  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog $
*/
