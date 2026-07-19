/* $Id: tam_aikido_upgrade.h,v 1.4 2019/08/06 06:56:06 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/tam_aikido_upgrade.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: tam_aikido_upgrade.h
 *
 * Aug 2016 - TSN TAM Aikido Upgrade Header File
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#ifndef __TAM_AIKIDO_UPGRADE_H__
#define __TAM_AIKIDO_UPGRADE_H__

#define BUF_SIZE                (4096)
#define USER_SPI_ADDR_START     (0x0)
#define USER_SPI_ADDR_END       (0x200000)
#define SIZE_64K                (0x10000)
#define SIZE_4K                 (0x1000)
/* #define MASK_4K              (0xFFFFF000) */
#define FPGA_FIRMWARE_NAME 	"fpga_bitstream.bin"
#define AIKIDO_FIRMWARE_NAME 	"fw_bitstream.bin"
#define SPI_FLASH_TABLE_NAME 	"spidir_bitstream.bin"

#define SPI_DIR_TABLE_START     (0x0)
#define SPI_DIR_TABLE_SIZE      (12)
#define BITSTREAM_BUF_SIZE      (0x3F0000)
#define GOLDEN_FW_START         (0x00001000)   
#define UPDATE_FW_START         (0x00080000)
#define UPDATE_FW_START_V2P3    (0x00000000)
#define GOLDEN_FPGA_START       (0x00100000)
#define UPDATE_FPGA_START       (0x00180000)
#define UPDATE_FPGA_START_V2P3  (0x00000000)
#define DEV_KEY_START           (0x00200000)

#define GOLDEN_IMAGE            (1)
#define UPGRADE_IMAGE           (2)
#define UPGRADE_FPGA            (0)
#define UPGRADE_FW              (1)

#define NEW_GOLDEN_VER          (1)
#define NEW_UPDATE_VER          (0x2711)
#define SPI_DIR_FPGA_VER_OFFSET (10)
#define SPI_DIR_FPGA_VER_SIZE   (2)

#define CHIP_INFO_FW_VER_BYTE0   (0x23)

extern int program_reggio_spi_prom(void);
extern int program_reggio_spi_prom_with_mailbox(void);
extern int program_aikido_dev_key(void);
extern int program_spi_update_version(void);
extern boolean aikido_mailbox_flag;
extern boolean aikido_act2_flag;
extern int act2_i2c_debug;
extern int tftp_get(char *, char *, char *, char *, int);
extern int utility_get_rtc(int);
extern int program_aikido_dev_key(void);

#endif   /* __TAM_AIKIDO_UPGRADE_H__ */

/*
 *------------------------------------------------------------------
 * $Log: tam_aikido_upgrade.h,v $
 * Revision 1.4  2019/08/06 06:56:06  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.3  2019/07/11 12:34:40  alicehua
 * Collapse Nutella codes into main trunk
 *
 * Revision 1.2.112.5  2019/07/08 04:52:14  alicehua
 * Added -DAIKIDO_SUPPORT_AIK flag.
 *
 * Revision 1.2.112.4  2019/04/26 09:00:59  harrchan
 * Base on review comments to clean up code
 *
 * Revision 1.2.112.3  2019/04/10 06:42:47  harrchan
 * Fixed the issue for aikido SPI maximum reading length
 *
 * Revision 1.2.112.2  2019/04/09 06:56:58  harrchan
 * Add program/verify aikido dev key
 *
 * Revision 1.2.112.1  2019/03/08 05:51:24  harrchan
 * 1.Add utility for Aikido FPGA upgrade
 *
 * Revision 1.2  2017/08/02 14:21:28  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.4.2  2017/07/29 03:40:43  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.2.1  2017/07/21 09:17:31  iachang
 * clean up code
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
