/* $Id: diag_sirius_fpga_util.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_sirius_fpga_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_sirius_fpga_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_FPGA_PROG__
#define __PLUG_FPGA_PROG__

#define FPGA_UPGRADE_IMAGE                      (2)
#define BIOS_UPGRADE_IMAGE                      (2)

#define FPGA_GOLDEN_IMAGE                       (1)
#define BIOS_GOLDEN                             (1)

#define VERIFY_RETRY                            (1)

typedef enum {
    BIT_CLEAR,
    BIT_SET
} bit_check_value;

/* Bit definitions for SPI PROM Control Register */
#define PROM_USE_ADDR                            (0x00000001)
#define PROM_DATA_WRITE                          (0x00000002)
#define PROM_USE_DUMMY                           (0x00000004)
#define PROM_SWAP_DATA_BITS                      (0x00000008)
#define PROM_DFLT_BAUD                           (0x00000030)
#define PROM_BAUD_RATE_MASK                      (0x000007f0)
#define PROM_DONE_INTR_ENAB                      (0x00008000)

/* Bit definitions for SPI PROM Opcode */
#define PROM_WREN_OP                             (0x06000000)   /* Write Enable  */
#define PROM_WRDI_OP                             (0x04000000)   /* Write Disable */
#define PROM_RDID_OP                             (0x9F000000)   /* Read Identification */
#define PROM_RDSR_OP                             (0x05000000)   /* Read Status Register */
#define PROM_WRSR_OP                             (0x01000000)   /* Write Status Register */
#define PROM_READ_OP                             (0x03000000)   /* Read Data Bytes */
#define PROM_FAST_READ_OP                        (0x0B000000)   /* Read Data Bytes at Higher Speed */
#define PROM_PAGE_PROG_OP                        (0x02000000)   /* Page Program */
#define PROM_SECT_ERASE_OP                       (0xD8000000)   /* Sector Erase */
#define PROM_BULK_ERASE_OP                       (0xC7000000)   /* Bulk/Chip Erase */

#define PROM_RD_ONE_BYTE                         (0x00000000)
#define PROM_RD_THREE_BYTES                      (0x00000002)
#define PROM_READ_ID                             (0x9F000000)
#define PROM_READ_STATUS_REG                     (0x05000000)
#define PROM_DATA_READ                           (0x00000002)
#define PROM_BLOCK_ERASE_TWKB                    (0x52000000)
#define PROM_DATA_WRITE                          (0x00000002)
#define PROM_INSERT_FIELD_ADDR                   (0x00000001)
#define PROM_READ_ARRAY_LF                       (0x03000000)
#define PROM_RD_FIFO_EMPTY                       (0x00000002)
#define PROM_WR_FIFO_EMPTY                       (0x00000008)
#define PROM_STAT_DONE                           (0x00800000)
#define SPI_PROM_MAX_WAIT                        (2500)
#define SPI_PROM_ERASE_WAIT                      (20000)
#define PROM_OPER_DONE                           (0x00008000)

/* Bit definition for Read Status Register (RDSR) */
#define PROM_UNPROTECT_ALL                       (0x00)
//#define PROM_RDSR_WIP                            (0x01)  /* Write in Progress */
//#define PROM_RDSR_WEL                            (0x02)  /* Write Enable Latch */
#define PROM_RDSR_BP0                            (0x04)  /* Block 0 Protect */
#define PROM_RDSR_BP1                            (0x08)  /* Block 1 Protect */
#define PROM_RDSR_BP2                            (0x10)  /* Block 2 Protect */
#define PROM_RDSR_SRWD                           (0x80)  /* Status Register Write Protect */
#define PROM_RDSR_WIP                            (0)
#define PROM_RDSR_WEL                            (1)

#define SPI_PROM_SECTOR_SIZE                     (0x10000)

/* Bit definitions for SPI PROM Opcode and Address Register */
#define PROM_SPI_ADDR_MASK                       (0x00FFFFFF)
#define PROM_SPI_OPCODE_MASK                     (0xFF000000)
#define PROM_OPCODE_SHIFT                        (24)

#define UPGRADE_HASH_SECTOR                      (125)
#define UPGRADE_MULTI_BOOT_SECTOR                (127)

/* Bit definition for SPI PROM Read Size Register */
#define PROM_RD_MAX_BYTE                         (0x100)
#define PROM_RD_SIZE_MASK                        (0x000000FF)
#define PROM_RD_1_BYTE                           (0x00000000)
#define PROM_RD_3_BYTE                           (0x00000002)
#define PROM_RD_256_BYTE                         (PROM_RD_MAX_BYTE - 1)

#define PROM_BLANK_DATA                          (0xff)
#define MAX_SECTOR_NUM                           (127)
#define PROM_MAX_ERR_CNT                         (10)

#define FPGA_CONFIG_STS_HDR_SECT                 (126)
#define FPGA_UPGRADE_IMG_HDR_SECT                (127)

#define REGGIO_FPGA_HDR_SIZE                     (64)

#define FPGA_DEFAULT_FILE_PATH                   "/firmware/"
#define MAX_STR_SIZE                             (80)

#define FPGA_SPI_PROM_START_HEADER               (0x7F0000)
#define FPGA_SPI_PROM_GOLDEN_START_ADDR          (0)
#define FPGA_SPI_PROM_UPGRADE_START_ADDR         (0x400000)
#define FPGA_SPI_PROM_GOLDEN_START_SECT          (0)
#define FPGA_SPI_PROM_GOLDEN_END_SECT            (47)
#define FPGA_SPI_PROM_UPGRADE_START_SECT         (64)
#define FPGA_SPI_PROM_UPGRADE_END_SECT           (111)

/* Structure contains FPGA info */
typedef struct fpga_info_s {
    unsigned char *fpga_fw;
    int   fpga_fw_size;
    unsigned char debug_bit;
    unsigned char major_rev;
    unsigned char minor_rev;
    unsigned char debug_rev;
    unsigned char *datecode_year;
    unsigned char *datecode_month;
    unsigned char *datecode_day;
    unsigned char *datecode_hour;
    unsigned char *lh_upgrade_bitfile_hmac;
    unsigned char *lh_upgrade_bitfile_header;
    unsigned char *lh_golden_bitfile_hmac;
    unsigned char *lh_golden_bitfile_header;
    ulong start_addr;
    ulong end_addr;
    //    ulong max_addr;
    ulong hdr_addr;
    ushort start_sector;
    ushort end_sector;
    char image_str[MAX_STR_SIZE];
} reggio_fpga_prog_info_t;

typedef struct _reggio_spi_prom_image_header {
    volatile unsigned char revision_id[4];
    volatile unsigned char revision_date[4];
    volatile unsigned char flags[4];
    volatile unsigned char magic_number[4];
    volatile unsigned int paddings[12]; 
} reggio_spi_prom_image_header_t;

extern int plug_fpga_spi_prog(int);
extern int plug_fpga_set_update_flag(int);
extern int plug_fpga_set_date_revision(int);
extern int plug_fpga_display_sector(int);
extern int plug_fpga_erase_header(int);

#endif /* __PLUG_FPGA_PROG__ */

/*-------------------------------------------------
 * $Log: diag_sirius_fpga_util.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
