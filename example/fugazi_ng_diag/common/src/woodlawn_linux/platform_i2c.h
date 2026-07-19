/* $Id: platform_i2c.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2006-2013 by cisco Systems, Inc.
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
#define CAVIUM_TMP421              (0x98 >> 1)

/* Cavium FPGA  */
#define CAVIUM_FPGA              (0x40 >> 1)

/* Cavium PCA9557  */
#define CAVIUM_PCA9557              (0x30 >> 1)

/* Cavium PLL  */
#define CAVIUM_PLL              (0xAE >> 1)

/* PCA9557 Registers */
#define PCA9557_OUTPUT_REG                  (0x1)
#define PCA9557_CONFIGURATION_REG    (0x3)

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

/* FPGA burst mode size */
#define BURST_MODE_REG_TEST_SIZE    (6)
#define BURST_MODE_TIMES        (100)
#define BURST_MODE_SIZE         (8)

/* Cavium SFP*/
#define SFP_0                   (0xA0 >> 1 )
#define SFP_1                   (0xA2 >> 1 )
#define SFP_2                   (0xA4 >> 1 )
#define SFP_3                   (0xA6 >> 1 )
#define SFP_4                   (0xA8 >> 1 )
#define SFP_5                   (0xAA >> 1 )
#define SFP_PLUS                (0xAC >> 1 )


/* Function prototypes */
extern int read_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
extern int write_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
extern uint32_t open_i2c(n2g_i2c_dev_t *, uint, uint8_t);

#endif /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
 * $Log: platform_i2c.h,v $
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:24  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.6  2013/04/08 08:55:54  leslie
 * Add PCA9557 macros
 *
 * Revision 1.5  2013/03/27 08:45:05  kuangik
 * Code cleanup
 *
 * Revision 1.4  2013/03/20 06:41:51  leslie
 * Add macros for fpga burst mode register test
 *
 * Revision 1.3  2013/03/20 06:34:16  kuangik
 * Correct SFP 5 Eeprom i2c address
 *
 * Revision 1.2  2013/03/13 10:10:05  leslie
 * Define macro for FPGA r/w burst mode size.
 *
 * Revision 1.12  2013/03/01 13:51:56  kuangik
 * Update Loopback Test, SFP Present, and SFP EEPROM display
 *
 * Revision 1.11  2013/02/26 01:46:58  leslie
 * Alter FPGA addr to 0x40.
 *
 * Revision 1.10  2012/10/24 10:43:39  leslie
 * Fix and clean up code.
 *
 * Revision 1.9  2012/08/30 06:38:20  leslie
 * Fix the addr of tmp421 and add pll addr.
 *
 * Revision 1.7  2012/08/18 02:48:43  leslie
 * Add argument to open_i2c function prototype.
 *
 * Revision 1.6  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/07/19 06:46:03  leslie
 * Add function prototype.
 *
 * Revision 1.3  2012/07/03 02:36:55  leslie
 * Add CAVIUM_FPGA Address
 *
 * Revision 1.2  2012/03/26 07:22:40  kody
 * Modify and add for TMP421 temperature sensor test code.
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.3  2011/11/03 14:58:44  palin2
 * Updated Cavium I2C driver related.
 *
 * Revision 1.1.2.2  2011/08/26 08:34:54  palin2
 * Added SFP tests support to Overlord Cavium.
 *
 * Revision 1.1.2.1  2011/07/18 17:32:49  palin2
 * To add DIMM utilities support in Cavium.
 *
 * Revision 1.2  2010/05/05 11:31:27  rjulian
 * Xformers/Three Gorges code merge
 *
 * Revision 1.1.1.1  2009/10/17 02:06:11  huyhoang
 * Initial archive of diaglinux module
 *
 * Revision 1.25.6.1  2009/06/04 09:41:54  sctsai
 * Sync with informers2-tag-060209 repository.
 *
 * Revision 1.25  2009/05/20 21:06:31  aosulliv
 * Sync of xformers-branch to ngd-diags-rep
 *
 * Revision 1.16.2.10  2009/04/22 00:01:57  siyen
 * Added MB slave I2C address per Steelers RTC/I2C changes (CSCsz22629).
 *
 * Revision 1.16.2.9  2009/01/07 01:30:38  amannr
 * Change ISM_I2C_ADDR_WLAN from 0x29 to 0x52
 *
 * Revision 1.24.2.1  2009/02/18 02:59:39  sctsai
 * Sync informers-tag-021609 to informers2-branch.
 *
 * Revision 1.19.2.4  2009/01/23 02:25:46  shhuang
 *  Sync with ngd-informers-012109 repository.
 *
 * Revision 1.24  2009/01/19 18:51:51  aosulliv
 * sync of xformers-tag-011609 to main ngd-diags-rep
 *
 * Revision 1.23  2008/11/13 21:08:12  aosulliv
 * Sync xformers-tag-121208 to the ngd diag repository
 *
 * Revision 1.22  2008/08/15 02:48:55  ncheng
 * Sync xformers-tag-08142008 to the main ngd diag rep
 *
 * Revision 1.16.2.7  2008/09/28 04:39:11  aarwang
 * - Added Apex-Zeta daughter card Quack support.
 *
 * Revision 1.16.2.6  2008/08/14 00:42:23  siyen
 * Added Brawn without Goofy supports.
 *
 * Revision 1.16.2.5  2008/06/13 19:48:16  siyen
 * Fixed TDM/PLL Menu in Ironhide.
 *
 * Revision 1.16.2.4  2008/06/03 01:58:34  siyen
 * Added ISM OIR supports.
 *
 * Revision 1.16.2.3  2008/04/26 01:54:34  siyen
 * Removed Cavium specifics defines to mips/include/ header file.
 *
 * Revision 1.16.2.2  2008/04/15 02:02:19  siyen
 * Added GLC-GE-100FX SFP supports.
 *
 * Revision 1.16.2.1  2008/04/11 02:50:23  siyen
 * Updated Nitrox I2C slave address to the newly assigned value.
 *
 * Revision 1.16  2008/03/06 03:23:46  siyen
 * USB Console revision supports added.
 *
 * Revision 1.15  2008/02/18 23:01:24  siyen
 * CSCsm76972 - Initialize I2C device enum for Oppo.
 *
 * Revision 1.14  2008/02/14 01:38:32  siyen
 * Added PSU cookie supports.
 *
 * Revision 1.13  2008/02/12 01:15:57  siyen
 * Updated with new USB Console I2C address.
 *
 * Revision 1.12  2008/02/09 01:29:17  siyen
 * Added USB Console I2C supports.
 *
 * Revision 1.11  2008/02/05 02:01:05  siyen
 * Added SFP I2C supports for 4K EEPROM SFPs.
 *
 * Revision 1.10  2008/01/10 01:16:10  siyen
 * Added Diode Sensor (Max1617A) supports.
 *
 * Revision 1.9  2008/01/06 03:37:32  siyen
 * Fixed compile warning.
 *
 * Revision 1.8  2008/01/05 04:01:05  siyen
 * Updated Cavium platforms CPU clock speeds.
 *
 * Revision 1.7  2008/01/05 01:08:53  siyen
 * Fixed SM/NM PCI Clock Generator init.
 *
 * Revision 1.6  2007/12/31 23:50:43  siyen
 * Added SM/NM adaptor IRQDiag Device I2C supports.
 *
 * Revision 1.5  2007/12/10 18:53:52  siyen
 * Added TDM/PLL supports.
 *
 * Revision 1.4  2007/11/15 01:24:34  siyen
 * Adjusted to the new Hardware changes.
 *
 * Revision 1.3  2007/11/15 00:19:38  siyen
 * Adjust to Hardware changes.
 *
 * Revision 1.2  2007/11/14 19:26:37  siyen
 * Added generic midplane, sm/ism/wlan defines.
 *
 * Revision 1.1  2007/11/05 17:24:49  siyen
 * Moved from mips directory to xformers directory.
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */
