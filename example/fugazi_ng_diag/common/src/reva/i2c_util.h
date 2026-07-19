/* $Id: i2c_util.h,v 1.3 2016/05/09 05:51:57 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/i2c_util.h,v $
 *
 * i2c_util.h - definitions for voltage margin utilities
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#define OUT0_33    0xF8
#define OUT1_18    0xF9
#define OUT2_15    0xFA
#define OUT3_10    0xFB

#define VOLTAGE_33_NO        0
#define VOLTAGE_33_NORMAL    144
#define VOLTAGE_33_HIGH      46
#define VOLTAGE_33_LOW       206

#define VOLTAGE_18_NO        0
/* P1B 135 */
#define VOLTAGE_18_NORMAL    133
/* P1B 75 */
#define VOLTAGE_18_HIGH      64
/* P1B 230 */
#define VOLTAGE_18_LOW       182

#define VOLTAGE_15_NO        0
#define VOLTAGE_15_NORMAL    4
#define VOLTAGE_15_HIGH      12
#define VOLTAGE_15_LOW       136

#define VOLTAGE_10_NO        0
#define VOLTAGE_10_NORMAL    131
#define VOLTAGE_10_HIGH      25
#define VOLTAGE_10_LOW       154

#define EXIT_CHAR '\033'

#define  VTG_3_3    2
#define  VTG_1_8    3
#define  VTG_1_5    4
#define  VTG_1_0    5

extern int zynq_i2c_reset(void);
extern int zynq_i2c_regtest(void);
extern int zynq_i2c_read(uchar dev_addr, volatile uchar * buf, uint length);
extern int zynq_i2c_write(uchar dev_addr, uchar * buf, uint length);
extern int zynq_i2c_read_byte(uint32_t offset, volatile uchar * buf);
extern int zynq_i2c_write_byte(uint32_t offset, uchar value);


int voltage_margin_specific(void);
void voltage_margin_display(void);
int voltage_margin_low(void);
int voltage_margin_normal(void);
int voltage_margin_high(void);
int voltage_no_margin(void);
int voltage_margin_low(void);
int voltage_margin_normal(void);
int voltage_margin_high(void);
int voltage_no_margin(void);

/******** History ******** 
$Log: i2c_util.h,v $
Revision 1.3  2016/05/09 05:51:57  umlin
Reva:
common/src/reva/diag.c        => Change wording in comment
common/src/reva/reva_ge_dma.c => Change wording in comment
common/src/reva/i2c_util.c    => Change wording for function name
common/src/reva/i2c_util.h    => Change wording for function name
common/src/reva/reva_ge_phy.c => Polling to check copper link status
utils/banner.sh               => Add banner BOX_TYPE for Reva
common/src/reva/Makefile      => Update FPGA bin file: reva_sb_mboot_rel

Revision 1.2  2016/05/06 03:43:52  umlin
Reva: Commit Reva module side diag codes to main trunk


$Endlog$
*/
