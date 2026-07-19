/* $Id: i2c_util.h,v 1.3 2017/03/20 09:34:14 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/sm/i2c_util.h,v $
 *
 * i2c_util.h - definitions for voltage margin utilities
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/* I2C0 slave device address */
#define ZYNQ_I2C_ADDR_DS4424        0x30    /* Voltage Margin address >> 1 */
/* I2C0 slave device address */
#define ZYNQ2_I2C_ADDR_DS4424       0x70    /* Voltage Margin address >> 1 */

/* Voltage 1: 3.3V, 1.8V, 1.5V 1.0V */
#define OUT0_33    0xF8
#define OUT1_18    0xF9
#define OUT2_15    0xFA
#define OUT3_10    0xFB

/* Voltage 2: 1.2V, 1.0V(GTP) */
#define OUT0_12    0xF8
#define OUT1_10    0xF9

/* Voltage 1: 3.3V, 1.8V, 1.5V 1.0V */
#define VOLTAGE_33_NO        0
#define VOLTAGE_33_NORMAL    142
#define VOLTAGE_33_HIGH      52
#define VOLTAGE_33_LOW       207

#define VOLTAGE_18_NO        0
#define VOLTAGE_18_NORMAL    128
#define VOLTAGE_18_HIGH      3
#define VOLTAGE_18_LOW       131

#define VOLTAGE_15_NO        0
#define VOLTAGE_15_NORMAL    3
#define VOLTAGE_15_HIGH      13
#define VOLTAGE_15_LOW       135

#define VOLTAGE_10_NO        0
#define VOLTAGE_10_NORMAL    128
#define VOLTAGE_10_HIGH      27
#define VOLTAGE_10_LOW       154

/* Voltage 2: 1.2V, 1.0V(GTP) */
#define VOLTAGE2_12_NO        0
#define VOLTAGE2_12_NORMAL    129
#define VOLTAGE2_12_HIGH      15
#define VOLTAGE2_12_LOW       145

#define VOLTAGE2_10_NO        0
#define VOLTAGE2_10_NORMAL    129
#define VOLTAGE2_10_HIGH      6
#define VOLTAGE2_10_LOW       136

extern int zynq_i2c_reset(void);
extern int zynq_i2c_regtest(void);
extern int zynq_i2c_read(uchar dev_addr, volatile uchar * buf, uint length);
extern int zynq_i2c_write(uchar dev_addr, uchar * buf, uint length);

int zynq2_i2c_read_byte(uchar offset, volatile uchar * buf);
int zynq2_i2c_write_byte(uchar offset, uchar value);
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
Revision 1.3  2017/03/20 09:34:14  umlin
Reva-SM: Change definitions for voltage margin 1.0V(GTP)

Revision 1.2  2017/03/16 05:20:22  umlin
Reva-SM: Commit Reva-SM module side diag codes to main trunk

Revision 1.1.2.1  2016/11/02 18:00:16  umlin
Reva-SM: Add 1.2V and 1.0V voltage margin. Fine-tune voltage margin value for Reva-SM


$Endlog$
*/
