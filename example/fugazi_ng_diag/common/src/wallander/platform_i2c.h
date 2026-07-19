/* $Id: platform_i2c.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__

#define I2C_ERR_BUF_SIZE           80

/* Transformers (and N2G) I2C Bus enumeration */
typedef enum {
    CPU_I2C0 = 0,	/* Cavium TWSI 0 */
    CPU_I2C1,		/* Cavium TWSI 1 */
    I2C_BUS_INVALID,	/* Invalid I2C bus */
} I2C_BUS;

/* I2C Device address defines */
/* Cavium Temperature Sensor */
#define CAVIUM_TMP421               (0x98 >> 1)

/* Cavium DS4424 */
#define CAVIUM_I2C_DS4424           (0x60 >> 1)

/* Cavium FPGA  */
#define CAVIUM_I2C_FPGA             (0x40 >> 1)

/* Cavium PCA9557  */
#define CAVIUM_PCA9557              (0x30 >> 1)

/* Cavium PLL  */
#define CAVIUM_PLL                  (0xAE >> 1)

/* PCA9557 Registers */
#define PCA9557_OUTPUT_REG          (0x1)
#define PCA9557_CONFIGURATION_REG   (0x3)

#define PCA9557_CONFIGURATION_VAL                      (0x0)
#define PCA9557_VAL_0_1                                (0x1)
#define PCA9557_VAL_0_2                                (0x2)
#define PCA9557_VAL_1_1                                (0x4)
#define PCA9557_VAL_1_2                                (0x8)
#define PCA9557_VAL_2_1                                (0x10)
#define PCA9557_VAL_2_2                                (0x20)
#define PCA9557_VAL_3_1                                (0x40)
#define PCA9557_VAL_3_2                                (0x80)

typedef uint8_t pca9557;                     /* PCA9557 One Byte Size Register */

/* DS4424 Parameters */
#define OUT0_33    0xF8
#define OUT1_15    0xF9
#define OUT2_092   0xFA
#define OUT2_085   0xFA
#define OUT3_10    0xFB

#define VOLTAGE_33_NO        0
#define VOLTAGE_33_NOM       128
#define VOLTAGE_33_HIGH      44
#define VOLTAGE_33_LOW       172

#define VOLTAGE_15_NO        0
#define VOLTAGE_15_NOM       128
#define VOLTAGE_15_HIGH      44
#define VOLTAGE_15_LOW       172

#define VOLTAGE_092_NO       0
#define VOLTAGE_092_NOM      128
#define VOLTAGE_092_HIGH     128    // 29
#define VOLTAGE_092_LOW      128    // 174

#define VOLTAGE_085_NO       0
#define VOLTAGE_085_NOM      129
#define VOLTAGE_085_HIGH     129    // 38
#define VOLTAGE_085_LOW      129    // 183

#define VOLTAGE_10_NO        0
#define VOLTAGE_10_NOM       1      /* ?? */
#define VOLTAGE_10_HIGH      43
#define VOLTAGE_10_LOW       171

typedef uint8_t ds4424;                     /* DS4424 One Byte Size Register */


/* FPGA burst mode size */
#define BURST_MODE_REG_TEST_SIZE    (6)
#define BURST_MODE_TIMES            (100)
#define BURST_MODE_SIZE             (8)

/* Cavium SFP*/
#define CAVIUM_I2C_FPGA_MUX0        (0xE2 >> 1 )
#define CAVIUM_I2C_FPGA_MUX1        (0xE4 >> 1 )
#define CAVIUM_I2C_SFP              (0xA0 >> 1 )

/* Function prototypes */
extern int read_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
extern int write_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
extern uint32_t open_i2c(n2g_i2c_dev_t *, uint, uint8_t);

extern int i2c_rd_util(void);
extern int i2c_wr_util(void);
extern int show_mux(void);
extern int alter_mux(void);

extern int voltage_margin_specific(void);
extern void voltage_margin_display(void);
extern int voltage_margin_low(void);
extern int voltage_margin_nom(void);
extern int voltage_margin_high(void);
extern int voltage_no_margin(void);
extern int voltage_margin_low(void);
extern int voltage_margin_nom(void);
extern int voltage_margin_high(void);
extern int voltage_no_margin(void);

#endif /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
 * $Log: platform_i2c.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */
