/* $Id: skye_i2c.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: skye_i2c.h
 *
 * Description: Header file of Skye I2C interface.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SHRINKRAY_I2C_H__
#define __SHRINKRAY_I2C_H__

/* Common definition */
#define I2C_DEV_ADDR_SHIFT   1  
#define SKYE_I2C_RETRY_MAX   3

/* Definition of Register Size (Byte) */
#define FOUR_B_REG              4
#define TWO_B_REG               2
#define ONE_B_REG               1

/* Definition of Skye CPU I2C Bus Type */
#define SR_CPU_I2C          0

/* Definition of Skye CPU I2C Bus number */
#define SR_CPU_I2CM0        0
#define SR_CPU_I2CM1        1
#define SR_CPU_I2CM2        2

/* Definition of Skye I2C device Address size */
#define SR_BIB_I2C_ADDR_SZ  2
#define SR_DIMM_ADDR_SZ     1
#define SR_FPGA_ADDR_SZ     1
#define SR_PWR_SEQ_ADDR_SZ  1
#define SR_I2C_MUX_ADDR_SZ  1
#define SR_THERMAL_ADDR_SZ  1
#define SR_CURRENT_ADDR_SZ  1
#define SR_CLK_BUF_ADDR_SZ  2

/* Definition of Skye I2C device Register size */
#define SR_BIB_REG_SZ       ONE_B_REG
#define SR_DIMM_REG_SZ      ONE_B_REG
#define SR_I2C_MUX_REG_SZ   ONE_B_REG
#define SR_PWR_SEQ_REG_SZ   ONE_B_REG
#define SR_CLK_BUF_REG_SZ   ONE_B_REG
#define SR_THERMAL_REG_SZ   ONE_B_REG
#define SR_CURRENT_REG_SZ   ONE_B_REG
#define SR_FPGA_REG_SZ      ONE_B_REG
#define SR_DIMM_TS_REG_SZ   TWO_B_REG

/* Definition of Skye I2C devices */
/* CPU I2CM0 */
#define SR_BIB_I2C_ADDR         (0xA8 >> 1)

/* CPU I2CM1 */
#define SR_DIMM0_SPD_I2C_ADDR   (0xA0 >> 1)
#define SR_DIMM1_SPD_I2C_ADDR   (0xA4 >> 1)
#define SR_DIMM0_TS_I2C_ADDR    (0x30 >> 1)
#define SR_DIMM1_TS_I2C_ADDR    (0x34 >> 1)

/* CPU I2CM2 */
#define SR_I2C_MUX_ADDR         (0xE0 >> 1)
#define SR_PWR_SEQ_I2C_ADDR     (0x82 >> 1)   /* I2C Mux Channel 0 */
#define SR_CLK_BUF_I2C_ADDR     (0xD8 >> 1)   /* I2C Mux Channel 0 */
#define SR_THERMAL_I2C_ADDR     (0x98 >> 1)   /* I2C Mux Channel 1 */
#define SR_CPU0_FPGA_I2C_ADDR   (0x54 >> 1)   /* I2C Mux Channel 2 with CPU0 */
#define SR_CPU1_FPGA_I2C_ADDR   (0x58 >> 1)   /* I2C Mux Channel 2 with CPU1 */
#define SR_CUR_SENSOR_I2C_ADDR  (0x80 >> 1)   /* I2C Mux Channel 3 */

/* Offset to subtract from returned Kelvin temperature to get degrees Celsius. */
#define TEMP_C_TO_K             273.15
#define IDEA_FACTOR             1.008 / 1.021
#define ACTURE_TEMP(x)          (((x) + TEMP_C_TO_K) * IDEA_FACTOR ) - TEMP_C_TO_K

/* Convert DIMM temperature. */
#define DIMM_MASK(x)      ((x & 0x0FF0 ) >> 4)  
#define DECIMAL_ONE(x)    (((x & 0x0008 ) == 0) ? 0 : 0.5)
#define DECIMAL_TWO(x)    (((x & 0x0004 ) == 0) ? 0 : 0.25)
#define DIMM_TEMP(x)      (DIMM_MASK(x) + DECIMAL_ONE(x) + DECIMAL_TWO(x))

/* Skye I2C Mux channel */
#define I2C_MUX_CH0   0x00
#define I2C_MUX_CH1   0x01
#define I2C_MUX_CH2   0x02
#define I2C_MUX_CH3   0x03

/* Skye I2C Mux(PCA9546a) register offset */
#define PCA9546A_CTRL_REG     0x00

/* Skye I2C Mux(PCA9546a) channel definition */
#define PCA9546A_I2C_CH0      0x01
#define PCA9546A_I2C_CH1      0x02
#define PCA9546A_I2C_CH2      0x04
#define PCA9546A_I2C_CH3      0x08
#define PCA9546A_I2C_ALL_CH   0x0F

/* Skye I2CM2 connection definition */
#define SR_PWR_SEQ_CH         I2C_MUX_CH0
#define SR_CLK_BUF_CH         I2C_MUX_CH0
#define SR_THERMAL_CH         I2C_MUX_CH1
#define SR_FPGA_CH            I2C_MUX_CH2
#define SR_CUR_SENSOR_CH      I2C_MUX_CH3

/* Definition of Thermal Sensor registers' offset */
#define TS_TEMP_REG        0x00   /* Read ambient temperature. */
#define TS_CPU0_REG        0x01   /* Read CPU0 Tj. */
#define TS_STATUS_REG      0x02   /* Read status byte */
#define TS_R_CONFIG_REG    0x03   /* Read configuration byte */
#define TS_W_CONFIG_REG    0x09   /* Write configuration byte */
#define TS_TEMP_EXT_REG    0x10   /* Read ambient temperature (extended temperature). */
#define TS_CPU0_EXT_REG    0x11   /* Read CPU0 Tj (extended temperature) */
#define TS_R_TEMP_HIGH_REG 0x05   /* Read Ambient temperature high limit */
#define TS_R_CPU0_HIGH_REG 0x07   /* Read CPU0 Tj high limit. */
#define TS_W_TEMP_HIGH_REG 0x0B   /* Write Ambient temperature high limit */
#define TS_W_CPU0_HIGH_REG 0x0D   /* Write CPU0 Tj high limit. */
#define TS_CPU0_OVER_REG   0x19   /* CPU0 Tj Over-temperature limit. */
#define TS_OVER_HYS_REG    0x21   /* Over-temperature hysteresis. */
#define TS_ID_REG_OFF      0xFE   /* Manufacture ID. */
#define TS_REV_REG_OFF     0xFF   /* Revision ID. */


#define TS_TEMP_REG        0x00   /* Read ambient temperature. */
#define TS_CPU1_REG        0x01   /* Read CPU1 Tj. */
#define TS_TEMP_EXT_REG    0x10   /* Read ambient temperature (extended temperature). */
#define TS_CPU1_EXT_REG    0x11   /* Read CPU1 Tj (extended temperature) */
#define TS_R_HOT_SPOT_REG  0x05   /* Read PCB hot-spot temperature high limit */
#define TS_R_CPU1_HIGH_REG 0x07   /* Read CPU1 Tj high limit. */
#define TS_W_HOT_SPOT_REG  0x0B   /* Write PCB hot-spot temperature high limit */
#define TS_W_CPU1_HIGH_REG 0x0D   /* Write CPU1 Tj high limit. */
#define TS_CPU1_OVER_REG   0x19   /* CPU1 Tj Over-temperature limit. */

/* Definition of Current Sensor registers' offset */
#define CURRENT_CONF_REG        0x00   /* Configuration. */
#define CURRENT_SHUNT_VOL_REG   0x01   /* Shunt Voltage. */
#define CURRENT_BUS_VOL_REG     0x02   /* Bus Voltage */
#define CURRENT_POWER_REG       0x03   /* Power. */
#define CURRENT_CURRENT_REG     0x04   /* Current */
#define CURRENT_CALIBRATION_REG 0x05   /* Calibration */
#define CURRENT_MASK_REG        0x06   /* Mask/Enable. */
#define CURRENT_ALERT_REG       0x07   /* Alert Limit */
#define CURRENT_ID_REG          0xFF   /* Die ID. */

/* Definition of BIB ROM */
#define BIB_START_OFFSET    0xFDC8
#define BIB_END_OFFSET      0x10000
#define BIB_DUMP_START_OFF  0xFDC0

/* Definition of Skye I2C device characteristics */
typedef struct skye_i2c_device {
    uint32_t   offset;      /* I2C device register or memory offset...
                             * if < 0, no offset will be sent on the bus.
                             */
    uint8_t    dev_addr;    /* MB_I2C_DEVICE device ID */
    uint8_t    addr_sz;     /* I2C device address size */
    uint8_t    i2c_ctrl;    /* MB_I2C_DEVICE device ID */
    uint16_t   size;        /* Default data size for read/write */
    uint8_t    mux_ch;      /* Mux number that I2C device connected to */
    uint8_t    mux;         /* Data to setup Mux to enable related channel */
    uint32_t   err_no;
    uchar      *buf;        /* Read/write buffer pointer */
    char       *dev_name;   /* I2C device name */
} skye_i2c_dev_t;

/* Externs */
extern int skye_i2c_read(int, uint16_t, int, uint16_t, uint16_t, uchar*);
extern int skye_i2c_write(int, uint16_t, int, uint16_t, uint16_t, uchar*);
extern int skye_dimm_spd_read(int, uint16_t, uint16_t, uchar*);
extern int skye_dimm_spd_write(int, uint16_t, uint16_t, uchar*);
extern int skye_dimm_thermal_rd(int, uint16_t, uint16_t *);
extern int skye_dimm_thermal_wr(int, uint16_t, uint16_t *);
extern int skye_fpga_i2c_read(uint16_t, uint16_t, uchar*);
extern int skye_fpga_i2c_write(uint16_t, uint16_t, uchar*);
extern int plat_ps_i2c_rd(uint16_t, uint16_t, uchar*);
extern int plat_ps_i2c_wr(uint16_t, uint16_t, uchar*);
extern int skye_i2c_rd_util(void);
extern int skye_i2c_wr_util(void);
extern int skye_i2c_mux_ctrl_reg_wr(uchar *);


#endif /* __SHRINKRAY_I2C_H__ */

/*------------------------------------------------------------------
$Log: skye_i2c.h,v $
Revision 1.2  2015/05/25 03:59:11  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:28  steja
Code check-in to skye-branch2 for ER code review


-------------------------------------------------------------------
Revision 1.1.2.3  2015/01/20 00:51:22  palin2
Updated thermal sensor ideality factor of the remote "diode".

Revision 1.1.2.2  2014/11/27 09:19:14  palin2
Added BIB starting address.

Revision 1.1.2.1  2014/07/21 01:56:39  palin2
Initial check-in Skye module side Diag code.

--------------------------------------------------------------------
shrinkray_i2c.h:
Revision 1.2.8.3  2014/07/09 02:20:58  palin2
Support I2C scan test for Shrinkray.

Revision 1.2.8.2  2014/06/27 09:40:40  palin2
Add definition of Over-temperature hysteresis register offset(0x21)
for Thermal sensor chip.

Revision 1.2.8.1  2014/05/20 17:55:21  palin2
1. Move power sequencer register(s) and voltage margin related definition
   to "pwr_seq_diag.h".
2. Update extern function list.

Revision 1.2  2014/02/27 15:01:10  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.11  2014/01/16 03:44:42  steja
Update BIB R/W MAC utility

Revision 1.1.4.10  2014/01/13 03:25:38  iachang
CSCum50313 : CPU0 thermal interrupt test

Revision 1.1.4.9  2013/12/18 05:03:10  steja
1. support PSE2 backplane loopback test
2. support BIB change MAC address utility

Revision 1.1.4.8  2013/12/16 08:34:31  iachang
Support current sensor
Modify on-board thermal sensor

Revision 1.1.4.7  2013/12/06 09:39:35  iachang
Move DIMM Thermal sensor to skye_thermal.c
Support on-board Thermal sensor
Convert the measure to actual temperature

Revision 1.1.4.6  2013/11/18 11:00:15  iachang
Support CPU1 Szalinski FPGA I2C access.

Revision 1.1.4.5  2013/11/13 08:18:36  palin2
Update DIMM Thermal sensor I2C write function proto definition.

Revision 1.1.4.4  2013/11/13 01:34:27  palin2
Add definitions for ShrinkRay DIMM thermal sensor.

Revision 1.1.4.3  2013/10/07 21:34:33  palin2
Add ShrinkRay I2C Mux related definition.(channel mapping, I2C Mux address, ...)

Revision 1.1.4.2  2013/09/13 07:00:00  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.5  2013/08/27 02:57:15  palin2
Add ShinkRay I2C access uilities wrap.

Revision 1.1.2.4  2013/08/19 07:11:47  palin2
Add Voltage Margin utility.

Revision 1.1.2.3  2013/07/15 21:54:56  palin2
Initial check-in for ShrinkRay FPGA(Szalinski) Diag test and utility.

Revision 1.1.2.2  2013/07/14 22:03:07  palin2
Added ShrinkRay I2C write support and DDR DIMM SPD write utility.

Revision 1.1.2.1  2013/07/09 07:23:51  palin2
Create for ShrinkRay I2C definitions.

--------------------------------------------------------------------
$Endlog$
*/

