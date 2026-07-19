/* $Id: tam_aikido_upgrade.h,v 1.2 2019/06/14 05:24:52 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/tam_aikido_upgrade.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __KATAR_TAM_AIKIDO_UPGRADE_H__
#define __KATAR_TAM_AIKIDO_UPGRADE_H__

//TSN project use old api, katar use different api to update aikido
//#define USE_OLD_API

#define BUF_SIZE                (4096)
#define USER_SPI_ADDR_START     (0x0)
#define USER_SPI_ADDR_END       (0x200000)
#define SIZE_64K                (0x10000)
#define SIZE_4K                 (0x1000)
/* #define MASK_4K              (0xFFFFF000) */
#define FPGA_FIRMWARE_NAME              "fpga_bitstream.bin"
#define AIKIDO_FIRMWARE_NAME    "fw_bitstream.bin"
#define SPI_FLASH_TABLE_NAME    "spidir_bitstream.bin"

#define SPI_DIR_TABLE_START     (0x0)
#define SPI_DIR_TABLE_SIZE      (12)
#define SPI_DIR_FPGA_VER_OFFSET (10)
#define SPI_DIR_FPGA_VER_SIZE   (2)
#define BITSTREAM_BUF_SIZE      (0x3F0000) //32-bit machine max buf size is 4MB

#ifdef USE_OLD_API
#define GOLDEN_FW_START         (0x00001000)   
#define UPDATE_FW_START         (0x00080000)
#define GOLDEN_FPGA_START       (0x00100000)
#define UPDATE_FPGA_START       (0x00180000)
#else
#define UPDATE_FW_START         (0x00280000)
#define UPDATE_FPGA_START       (0x003CF000)
#endif

#define WR_DATA_LEN             (0x400)                 //LPC using old setting
#define SMB_WR_DATA_LEN         (20)                    //SMB limit only 20 byte
#define GOLDEN_IMAGE            (1)
#define UPGRADE_IMAGE           (2)

extern int program_reggio_spi_prom(void);
extern int program_spi_update_version(void);
extern boolean aikido_mailbox_flag;
extern boolean aikido_act2_flag;
extern int act2_i2c_debug;
extern int tftp_get(char *, char *, char *, char *, int);

#endif   /* __KATAR_TAM_AIKIDO_UPGRADE_H__ */

/*
 *------------------------------------------------------------------
 * $Log: tam_aikido_upgrade.h,v $
 * Revision 1.2  2019/06/14 05:24:52  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.1  2019/02/12 08:06:31  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.3  2018/12/12 02:03:39  peteteng
 * Add Aikido FW upgrade through LPC
 *
 * Revision 1.1.2.2  2018/12/06 08:32:25  mikech2
 * Fine-tune Aikido I2C r/w and fix Aikido update FW utility
 *
 * Revision 1.1.2.1  2018/11/30 06:19:19  mikech2
 * Modify Aikido eSPI memory map
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

