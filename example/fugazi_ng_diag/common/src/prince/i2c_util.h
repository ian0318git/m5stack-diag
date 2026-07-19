/* $Id: i2c_util.h,v 1.3 2013/07/16 02:22:18 liwwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/i2c_util.h,v $
 *
 * i2c_util.h - definitions for voltage margin utilities
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#define OUT0_33    0xF8
#define OUT1_18    0xF9
#define OUT2_15    0xFA
#define OUT3_10    0xFB

#define VOLTAGE_33_NO        0
#define VOLTAGE_33_NORMAL    144
#define VOLTAGE_33_HIGH      90
#define VOLTAGE_33_LOW       248

#define VOLTAGE_18_NO        0
/* P1B 135 */
#define VOLTAGE_18_NORMAL    133
/* P1B 75 */
#define VOLTAGE_18_HIGH      69
/* P1B 230 */
#define VOLTAGE_18_LOW       207

#define VOLTAGE_15_NO        0
#define VOLTAGE_15_NORMAL    11
#define VOLTAGE_15_HIGH      121
#define VOLTAGE_15_LOW       218

#define VOLTAGE_10_NO        0
#define VOLTAGE_10_NORMAL    131
#define VOLTAGE_10_HIGH      40
#define VOLTAGE_10_LOW       171

#define EXIT_CHAR '\033'

#define  VTG_3_3    2
#define  VTG_1_8    3
#define  VTG_1_5    4
#define  VTG_1_0    5

extern int zynq_i2c_reset(void);
extern int zynq_i2c_regtest(void);
extern int zynq_i2c_read(uchar dev_addr, volatile uchar *buf, uint length);
extern int zynq_i2c_write(uchar dev_addr, uchar *buf, uint length);
extern int zynq_i2c_read_byte(uint32_t offset, volatile uchar *buf);
extern int zynq_i2c_write_byte(uint32_t offset, uchar value);


int voltage_margin_specific(void);
void voltage_margin_diaplay(void);
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
Revision 1.3  2013/07/16 02:22:18  liwwang
change margin value for P2

Revision 1.2  2013/06/25 07:45:48  xiaoyizh
Update Margin test per EDVT's requirement.

$Endlog$
*/
