/* $Id: diag_xaui_88X3120_lib.h,v 1.2 2013/10/08 08:48:29 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_xaui_88X3120_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_xaui_88X3120_lib.h
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __DIAG_XAUI_88X3120_LIB_H__
#define __DIAG_XAUI_88X3120_LIB_H__
/* Slave Responses */
#define VERIFY_OK   (0x100)
#define ERROR_CMD   (0x200)
#define FLASH_BUSY  (0x300)
#define VERIFY_ERR  (0x400)

typedef enum {
    NORMAL_MODE = 0,
    TEST_MODE_1,
    TEST_MODE_2,
    TEST_MODE_3,
    TEST_MODE_4_TONE_1,
    TEST_MODE_4_TONE_2,
    TEST_MODE_4_TONE_3,
    TEST_MODE_4_TONE_4,
    TEST_MODE_4_TONE_5,
    TEST_MODE_5,
    TEST_MODE_6,
} PHY_3120_TEST_MODE;

typedef enum
{
    SFT9001 = 0,
    SFT910X = 1,
    SFINVALID = 10
}DEV_ID;

typedef enum
{
    REVA = 0,
    REVB = 1,
    REVC = 2
}DEV_REV;

typedef enum
{
    SUBREV0 = 0,
    SUBREV1 = 1,
    SUBREV2 = 2,
    SUBREV1S = 10
}DEV_SUB_REV;

typedef struct
{
    DEV_ID devID;
    DEV_REV devRev;
    DEV_SUB_REV devSubRev;
}STRUCT_DEVICE_TYPE;

typedef unsigned long MEM_SIZE_BYTES;

#define IEEE_PMA_DEVID_REG 1,3
#define UPC_VER_REG 3,53249
#define MANUF_MODEL_NUM_MASK 0x3F
#define MANUF_MODEL_NUM_BIT_POS 4
#define CHIP_ID_MASK 0xF

#define ERR_PHY_NOT_IN_DOWNLOAD_MODE    0xFFFF

#define RAM_CHECKSUM_REG 3, 0xD05B

/* RAM checksum register (present only in SFT910X RevB or later) had wrong checksum */
#define ERR_RAM_HW_CHECKSUM_ERR         0xFFE9

#define MRVL_88X3120_PORT_0_ADDR        (0x0)
#define MRVL_88X3120_PORT_1_ADDR        (0x3)
#define MRVL_88X3120_SMI2_ADDR          (0x2)
#define MRVL_88X3120_PORTS              (1)

#define MRVL_88X3120_SMI2_PORT0_ADDR    (MRVL_88X3120_PORT_0_ADDR | \
                                         (MRVL_88X3120_SMI2_ADDR << 4))
#define MRVL_88X3120_SMI2_PORT1_ADDR    (MRVL_88X3120_PORT_1_ADDR | \
                                         (MRVL_88X3120_SMI2_ADDR << 4))


#define MRVL_88X3120_PHY_REG_LEN        (2)

/* 88X3120 Device address and Register */
/* Device Address 1 */
#define MRV88X3120_REG_DEVICE_1         (0x1)
/* Register Offset 0x0 of Device Address 1 */
#define PMA_PMD_CTRL_1_REG              (0x0)
#define EN_PMA_LPBK_MODE                (0x1)
#define SOFTWARE_RESET                  (0x8000)
#define PMA_LPBK_BIT_MASK               (0xFFFE)

/* Register Offset 132 of Device Address 1 */
#define PMA_TEST_MODE_REG               (132)
#define PMA_TEST_MODE_MASK              (0x3F << 10)
#define PMA_NORMAL_MODE                 (0x00 << 10)
#define PMA_TEST_MODE_1                 (0x08 << 10)
#define PMA_TEST_MODE_2                 (0x10 << 10)
#define PMA_TEST_MODE_3                 (0x18 << 10)
#define PMA_TEST_MODE_4_TONE_1          (0x21 << 10)
#define PMA_TEST_MODE_4_TONE_2          (0x22 << 10)
#define PMA_TEST_MODE_4_TONE_3          (0x24 << 10)
#define PMA_TEST_MODE_4_TONE_4          (0x25 << 10)
#define PMA_TEST_MODE_4_TONE_5          (0x26 << 10)
#define PMA_TEST_MODE_5                 (0x28 << 10)
#define PMA_TEST_MODE_6                 (0x30 << 10)

/* Register Offset 0xc002 of Device Address 1 */
#define PHY_CFG_MODE                    (0xc002)

/* Device Address 3 */
#define MRV88X3120_REG_DEVICE_3         (0x3)

/* Device Address 4 */
#define MRV88X3120_REG_DEVICE_4         (0x4)

/* Device Address 7 */
#define MRV88X3120_REG_DEVICE_7         (0x7)

/* Device Address 29 */
#define MRV88X3120_REG_DEVICE_29        (0x1D)

/* Device Address 30 */
#define MRV88X3120_REG_DEVICE_30        (0X1E)

/* Device Address 31 */
#define MRV88X3120_REG_DEVICE_31        (0x1F)

/* Ported from PHY 3120 FW download code */
/* This type is provided as a type for the host to pass a context parameter
   into/through the API for whatever purposes the host may need */
typedef unsigned int *CTX_PTR_TYPE;

/* see specification/boot code spec for data space at end of LM which is
 * reserved for boot code usage. added some pad in case this expands later */
#define RESERVED_SIZE 256

/* max application is 160K bytes - data at end of RAM for*/
/* SFX7101 and SFT9001 Rev A parts*/
#define MAX_APP_SIZE_OLDPARTS (160UL*1024UL - RESERVED_SIZE)

/* API 1.7 update from 14 to 32 */
#define HEADER_SIZE 32
#define MAX_HEADER_SIZE 32

#define MAX_IMAGE_SIZE_OLDPARTS (MAX_APP_SIZE_OLDPARTS + HEADER_SIZE)

#define DATA_OFFSET_OLDPARTS HEADER_SIZE
#define DATA_OFFSET_NEWPARTS MAX_HEADER_SIZE

#define BOOT_RAM_USAGE_BYTES 256

#define VERIFY_FLASH (0x7)      // Reads flash and makes sure header and checksum
                               // and app checksum match what's stored in the
                               // flash header and app area

/*Image is too large for SFX7101*/
#define ERR_IMAGE_TOO_LARGE_TO_DOWNLOAD     0xFFF0
/* RAM download error - slave code - ram download func returned error */
#define ERR_DOWNLOAD_TO_RAM                 0xFFF1
/* Slave code did not start. - Slave code failed to download properly. */
#define ERR_SLAVE_CODE_DID_NOT_START        0xFFF2
/* Flash verifed FAILED! Flash probably corrupted */
#define ERR_VERIFY_ERR                      0xFFF3
/* Unknown error, downloading the flash failed! */
#define ERR_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL  0xFFF4
/* App code did not start. - App code failed to download properly in to the RAM. */
#define ERR_APP_CODE_DID_NOT_START        0xFFF5
/* App code failed. - Number of ports to download is greater than maximum ports */
#define ERR_NUM_PORTS_TOO_LARGE             0xFFF6

/* Error codes */
/*size must be an even number of bytes*/
#define ERR_SIZE_NOT_EVEN               0xFFE0
/* Slave encountered error while erasing flash */
#define ERR_ERASING_FLASH               0xFFE1
/* unexpected value read back from download code */
#define ERR_VALUE_READ_BACK             0xFFE2
/* Did not get OK for writing the data */
#define ERR_START_WRITE_DATA            0xFFE3
/* Slave failed to get all the data correctly*/
#define ERR_SLAVE_FAIL_TO_GET_DATA      0xFFE4
/* Some kind of error occurred on Slave */
#define ERR_ON_SLAVE                    0xFFE5
/* Checksum error */
#define ERR_CHECKSUM                    0xFFE6
/* Slave didn't write enough words to flash. Some kind of error occurred*/
#define ERR_SLAVE_WRITE_FULL            0xFFE7
/* last transfer failed */
#define ERR_LAST_TRANSFER               0xFFE8

/* Master-Slave Protocol Definitions MDIO Register to slave */
#define MAX_BUFF_SIZE_OUT_REG        1,49192 /* MDIO Registers used to communicate with slave code */
#define ACTUAL_BUFF_SIZE_IN_REG      1,49193
#define COMMAND_REG                  1,49194
#define WORDS_WRITTEN_REG            1,49195
#define LOW_ADDRESS_REG              1,49196
#define HIGH_ADDRESS_REG             1,49197
#define DATA_REG                     1,49198
#define CHECKSUM_REG                 1,49199
#define WORDS_RCVD_REG               1,49200

/* Reads flash and makes sure header and checksum and app checksum match
 * what's stored in the flash header and app area */
#define VERIFY_NG4_FLASH (0x7)
/* Same but for SFX7101 or SFT9001 Rev A */
#define VERIFY_10X_FLASH (0x8)

/* Host Commands */
#define ERASE_FLASH_PROGRAM_AREA  (0x1)
#define FILL_BUFFER               (0x2)
/*#define WRITE_BUFFER              (0x3)*/
/*#define READ_BUFFER               (0x4)*/
/*#define ERASE_FLASH               (0x5)*/
#define WRITE_VERIFY_BUFFER       (0x6)
/* Always does a write/verify */
#define WRITE_BUFFER WRITE_VERIFY_BUFFER
/* Reads flash and makes sure header and checksum and app checksum match
 * what's stored in the flash header and app area */
#define VERIFY_NG4_FLASH (0x7)
/* Same but for SFX7101 or SFT9001 Rev A */
#define VERIFY_10X_FLASH (0x8)

/* Slave Responses */
#define DOWNLOAD_OK (0x100)
#define ERROR_CMD   (0x200)
#define FLASH_BUSY  (0x300)
#define VERIFY_ERR  (0x400)

extern unsigned short SFPhyDownLoadFlash(CTX_PTR_TYPE, unsigned long,
                                         unsigned char *appData, unsigned long,
                                         unsigned char *slaveData, unsigned int);
extern void SFPhyRemovePhyDownloadMode(CTX_PTR_TYPE, unsigned long);

#endif
/*-------------------------------------------------
 * $Log: diag_xaui_88X3120_lib.h,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:20  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 04:49:36  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.11  2012/10/03 06:05:19  kody
 * Add X3120 PHY configuration mode definition.
 *
 * Revision 1.10  2012/09/21 11:49:43  kody
 * Fix the 88X3120 FW download issue.
 *
 * Revision 1.9  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.7  2012/07/11 08:10:18  kody
 * Modify the 88X3120 test mode setting.
 *
 * Revision 1.6  2012/07/09 08:51:11  kody
 * Add Phy 3120 test mode in utilities.
 *
 * Revision 1.5  2012/07/05 02:07:27  kody
 * Add Phy 3120 FW download utility.
 *
 * Revision 1.4  2012/05/18 10:23:13  kody
 * Add 88X3210 register Macro
 *
 * Revision 1.3  2012/05/15 01:33:50  leslie
 * Update for 88X3120 test item
 *
 * Revision 1.2  2012/04/16 02:41:39  kody
 * Clean up the 88X3120 test code.
 *
 * Revision 1.1  2012/02/10 07:11:15  leslie
 * Add Woodlawn phy 88X3120 lib header file.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
