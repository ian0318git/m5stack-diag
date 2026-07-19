/* $Id: platform_i2c_api.h,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_i2c_api.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


/* Katar Intel side I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,          /* Intel North Bridge I2C */
    CPU_I2C1,              /* Intel South Bridge I2C */
    IOFPGA_I2C,            /* I2C controller on the IO FPGA */
    I2C_BUS_INVALID,       /* Invalid I2C bus */
} I2C_BUS;

/* CPU I2C Master (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_DIMM1 = 0,      /* DIMM1 */
    MB_I2C_DIMM2,          /* DIMM2 */
    MB_I2C_SFP_SWITCH,     /* SFP Switch */
    MB_I2C_SFP_SPD,        /* SFP SPD */
    MB_I2C_TEMP1,          /* Temperature Sensor #1 */
    MB_I2C_TEMP2,          /* Temperature Sensor #2 */
    MB_I2C_POE,            /* POE Sensor */
    MB_I2C_0_INVALID,      /* Invalid I2C */
} MB_I2C0_DEVICE;

/* CPU I2C Master (CPU_I2C1 bus) */
typedef enum {
//    MB_I2C_SFP_SWITCH = 0, /* SFP Switch */
//    MB_I2C_SFP_SPD,        /* SFP SPD */
//    MB_I2C_TEMP1,          /* Temperature Sensor #1 */
//    MB_I2C_TEMP2,          /* Temperature Sensor #2 */
//    MB_I2C_POE,            /* POE Sensor */
    MB_I2C_1_INVALID,      /* Invalid I2C */
} MB_I2C1_DEVICE;

#define I2CBUS0      "/dev/i2c-0"
#define I2CBUS1      "/dev/i2c-1"

#define MB_I2C_ADDR_DIMM1    (0xA0 >> 1)    /* DIMM1 */
#define MB_I2C_ADDR_DIMM2    (0xA4 >> 1)    /* DIMM2 */
#define MB_I2C_ADDR_SFP_SWITCH    (0x70)    /* SFP Switch*/
#define MB_I2C_ADDR_SFP_SPD       (0x50)    /* SFP SPD */
#define MB_I2C_ADDR_TEMP1         (0x48)    /* Temperature Sensor #1 */
#define MB_I2C_ADDR_TEMP2         (0x4F)    /* Temperature Sensor #2 */
#define MB_I2C_ADDR_POE           (0x21)    /* POE Sensor */


/*
 *------------------------------------------------------------------
 * $Log: platform_i2c_api.h,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.1  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.2  2018/09/28 03:09:18  peteteng
 * Fix SFP SPD issue
 *
 * Revision 1.1.2.1  2018/07/24 09:54:12  peteteng
 * Add SFP cookie - read
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

