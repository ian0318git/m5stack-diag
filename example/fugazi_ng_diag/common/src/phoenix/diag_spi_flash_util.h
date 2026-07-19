/* $Id: diag_spi_flash_util.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_spi_flash_util.h,v $
 *------------------------------------------------------------------
 * Filename: diag_spi_flash_util.h
 *
 * Description: Diag spi flash util header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_SPI_FLASH_UTIL_H__
#define __DIAG_SPI_FLASH_UTIL_H__

#define NAME_LEN	32

#define PHOENIX_DENVERTON_SPI_CTRL_MM_ADDR  0xFE010000

#define BFPREG  0x00

#define HSFSTS_CTL  0x04
#define HSFSTS_CTL_FSMIE  BIT(31)
#define HSFSTS_CTL_FDBC_SHIFT  24
#define HSFSTS_CTL_FDBC_MASK  (0x3f << HSFSTS_CTL_FDBC_SHIFT)
#define HSFSTS_CTL_WRSDIS  0x800

#define HSFSTS_CTL_FCYCLE_SHIFT  17
#define HSFSTS_CTL_FCYCLE_MASK  (0x0f << HSFSTS_CTL_FCYCLE_SHIFT)
/* HW sequencer opcodes */
#define HSFSTS_CTL_FCYCLE_READ  (0x00 << HSFSTS_CTL_FCYCLE_SHIFT)
#define HSFSTS_CTL_FCYCLE_WRITE  (0x02 << HSFSTS_CTL_FCYCLE_SHIFT)
#define HSFSTS_CTL_FCYCLE_ERASE  (0x03 << HSFSTS_CTL_FCYCLE_SHIFT)
#define HSFSTS_CTL_FCYCLE_ERASE_64K  (0x04 << HSFSTS_CTL_FCYCLE_SHIFT)
#define HSFSTS_CTL_FCYCLE_RDID	  (0x06 << HSFSTS_CTL_FCYCLE_SHIFT)
#define HSFSTS_CTL_FCYCLE_WRSR  (0x07 << HSFSTS_CTL_FCYCLE_SHIFT)
#define HSFSTS_CTL_FCYCLE_RDSR  (0x08 << HSFSTS_CTL_FCYCLE_SHIFT)

#define HSFSTS_CTL_FGO  BIT(16)
#define HSFSTS_CTL_FLOCKDN  BIT(15)
#define HSFSTS_CTL_FDV  BIT(14)
#define HSFSTS_CTL_SCIP  BIT(5)
#define HSFSTS_CTL_AEL  BIT(2)
#define HSFSTS_CTL_FCERR  BIT(1)
#define HSFSTS_CTL_FDONE  BIT(0)

#define FADDR  0x08
#define DLOCK  0x0c
#define FDATA(n)  (0x10 + ((n) * 4))

#define FRACC  0x50

#define FREG(n)  (0x54 + ((n) * 4))
#define FREG_BASE_MASK  0x3fff
#define FREG_LIMIT_SHIFT  16
#define FREG_LIMIT_MASK  (0x03fff << FREG_LIMIT_SHIFT)

/* Flash opcodes. */
#define SPINOR_OP_WREN  0x06  /* Write enable */
#define SPINOR_OP_RDSR  0x05  /* Read status register */
#define SPINOR_OP_WRSR	  0x01  /* Write status register 1 byte */
#define SPINOR_OP_READ  0x03  /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST  0x0b  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2  0x3b  /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2  0xbb  /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4  0x6b  /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4  0xeb  /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_PP  0x02  /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4  0x32  /* Quad page program */
#define SPINOR_OP_PP_1_4_4  0x38  /* Quad page program */
#define SPINOR_OP_BE_4K  0x20  /* Erase 4KiB block */
#define SPINOR_OP_BE_4K_PMC  0xd7  /* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K  0x52  /* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE  0xc7  /* Erase whole flash chip */
#define SPINOR_OP_SE  0xd8  /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID  0x9f  /* Read JEDEC ID */
#define SPINOR_OP_RDCR  0x35  /* Read configuration register */
#define SPINOR_OP_RDFSR  0x70  /* Read flag status register */

#define SPI_WAIT_TIME  200  /* ms */
#define INTEL_SPI_TIMEOUT  5000  /* ms */
#define INTEL_SPI_FIFO_SZ  64

#define BYTE_MASK  0xFF
#define BYTE_SHIFT  0x8
#define BYTES_READ  0x4


#define GOLDEN_ROMMON_SPI_FLASH  (0)
#define UPGRADE_ROMMON_SPI_FLASH  (1)
#define FPGA_SPI_FLASH  (2)

#define GOLDEN_ROMMON_SPI_FLASH_NAME  "Rommon Golden Flash"
#define UPGRADE_ROMMON_SPI_FLASH_NAME  "Rommon Upgrade Flash"
#define FPGA_SPI_FLASH_NAME  "FPGA Flash"

#define SPI_FLASH_STATUS_REG_PROTECT_EN  0x80

/* Main source */
#define SPI_W25Q128JV_PROD_NAME "W25Q128JV"
#define SPI_W25Q128JV_ID 0xEF4018
#define SPI_W25Q128JV_PROTECTED_256KB_BP_CONF 0x04 /* 0000 0100 = 0xFC0000 - 0xFFFFFF */
/* Second source*/
#define SPI_MX25L12833F_PROD_NAME "MX25L12833F"
#define SPI_MX25L12833F_ID 0xC22018
#define SPI_MX25L12833F_PROTECTED_256KB_BP_CONF 0x0C /* 0000 1100 = 0xFC0000 - 0xFFFFFF */

/* Main source */
#define SPI_W25Q16JV_PROD_NAME "W25Q16JV"
#define SPI_W25Q16JV_ID 0xEF4015
/* Second source*/
#define SPI_MX25V1635F_PROD_NAME "MX25V1635F"
#define SPI_MX25V1635F_ID 0xC22315

/* P1 source*/
#define SPI_MX25L6433F_PROD_NAME "MX25L6433F"
#define SPI_MX25L6433F_ID 0xC22017
#define SPI_MX25L6433F_PROTECTED_1M_BP_CONF 0x54 /* 0101 0100 */
/* Main source */
#define SPI_MX25L1606E_PROD_NAME "MX25L1606E"
#define SPI_MX25L1606E_ID 0xC22015
#define SPI_MX25L1606E_PROTECTED_1M_BP_CONF 0x28 /* 0010 1000 */
/* Second source*/
#define SPI_GD25Q16C_PROD_NAME "GD25Q16C"
#define SPI_GD25Q16C_ID 0xC84015
#define SPI_GD25Q16C_PROTECTED_1M_BP_CONF 0x34 /* 0011 0100 */


#define SPI_BP_BIT_MASK 0x7C /* 0111 1100 */
#define SPI_MAX_ID_LEN	3
#define SPI_ID_BYTE_0	0
#define SPI_ID_BYTE_1	1
#define SPI_ID_BYTE_2	2
#define SPI_ID_BIT_0  0
#define SPI_ID_BIT_8  8
#define SPI_ID_BIT_7  7
#define SPI_ID_BIT_16  16


#define BUFFER_LEN						128
#define SPI_FLASH_DEV_OPEN_RETRY_MAX	10
#define SPI_FLASH_SLEEP_SECOND			1

#define WRITE_PROTECT_TEST_LEN       0x40000
#define SECTOR_00  0x0
#define SECTOR_15  0xF
#define SECTOR_252  0xFC

#define SR_WRITE_PROTECT_TEST_PATTERN       0x00

extern int denverton_spi_write_status_reg (int);
extern int denverton_spi_read_status_reg(int);
extern int phoenix_spi_flash_utils (int);
extern int phoenix_spi_flash_get_rdsr (int, uchar *); 
extern int phoenix_spi_flash_get_prod_id (int, uint*);


#endif                          /* __DIAG_SPI_FLASH_UTIL_H__ */


