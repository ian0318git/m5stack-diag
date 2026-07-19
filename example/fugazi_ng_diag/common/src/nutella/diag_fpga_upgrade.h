/* $Id: diag_fpga_upgrade.h,v 1.5 2020/05/07 01:34:56 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_fpga_upgrade.h,v $
 *--------------------------------------------------------------------
 * diag_fpga_upgrade.h
 *
 * Dec 2010. Alan O'Sullivan
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------*/

#ifndef __DIAG_FPGA_UPGRADE_H__
#define __DIAG_FPGA_UPGRADE_H__

#define MAX_POLLING_FPGA_FLASH_BUSY_TIMES     100
#define EACH_POLLING_LOOP_DELAY_TIME          100 
#define FLASH_BUSY_STATUS                     1

/*
Description	                Opcode	Address Bytes	Dummy Bytes	Data Bytes
---------------------------------------------------------------------------
Wr Enable	                0x06	0	0	0
Wr Disable	                0x04	0	0	0
Rd Identification	        0x9F	0	0	1-4
Rd Status Register	        0x05	0	0	1+
Wr Status Register	        0x01	0	0	1
Rd Data Bytes	                0x03	3	0	1+
Rd Data Bytes High Speed 	0x0B	3	1	1+
Page Program	                0x02	3	0	1-256
Sector Erase	                0xD8	3	0	0
Bulk/Chip Erase	                0xC7	0	0	0
---------------------------------------------------------------------------
*/

#define MAX_STR_SIZE  80

#define FPGA_DEFAULT_FILE_PATH            "/firmware/fpga_fw"
#define FPGA_SPI_PROM_OFFSET   		      0xB00

typedef struct prom_t_ {  //typedef struct _reggio_spi_prom_reg_t {
    volatile unsigned int control; //    volatile uint   control;		 /* 0x00000B00 */
    volatile unsigned int status;  //   volatile uint   status;	         /* 0x00000B04 */
    volatile unsigned int size;    //    volatile uint   size;           /* 0x00000B08 */
    volatile unsigned int data;    //    volatile uint   data;           /* 0x00000B0C */
    volatile unsigned int opcode_addr;  // volatile uint   opcode_addr;  /* 0x00000B10 */
} prom_t ;

typedef struct reconf_ {
    volatile unsigned int sts;
    volatile unsigned int rev_id;
    volatile unsigned int rev_date;
    volatile unsigned int flags;
    volatile unsigned int magic;
    volatile unsigned int hist;
} reconf_t ;

/* Reggio FPGA Multiboot registers */
typedef struct _reggio_multiboot_reg_t {
    volatile unsigned int   recfg_control;	 /* 0x0000_0500 */
    volatile unsigned int   recfg_status;	 /* 0x0000_0504 */
    volatile unsigned int   image_head;         /* 0x0000_0508 */
    volatile unsigned int   rev_id;             /* 0x0000_050C */
    volatile unsigned int   rev_date;           /* 0x0000_0510 */
    volatile unsigned int   flag;               /* 0x0000_0514 */
} reggio_multiboot_reg_t;

#define PROM_SECTOR_SIZE    0x10000   /* 64KB size */
typedef struct _reggio_spi_eprom_info_t {
   unsigned int   start_g;
    //   unsigned int   end_g;
   unsigned int   start_u;
    //   unsigned int   end_u;
   unsigned short  start_sector_g;
   unsigned short  end_sector_g; 
   unsigned short  start_sector_u;
   unsigned short  end_sector_u;
   unsigned int   start_header;
   unsigned int   end_header;
} reggio_spi_eprom_info_t;

typedef struct _reggio_spi_prom_image_header {
   volatile unsigned char revision_id[4];
   volatile unsigned char revision_date[4];
   volatile unsigned char flags[4];
   volatile unsigned char magic_number[4];
   volatile unsigned int paddings[12]; 
} reggio_spi_prom_image_header_t;

/* Reggio Internal Module A SPI Interface registers */
typedef struct _reggio_spi_a_reg_t {
    volatile unsigned int   spi_cfg;	         /* 0x0000_1000 */
    volatile unsigned int   spi_base_tail;	 /* 0x0000_1002 */
    volatile unsigned int   spi_size_head;      /* 0x0000_1004 */
    volatile unsigned int   spi_base_head;      /* 0x0000_1006 */
    volatile unsigned int   spi_size_tail;      /* 0x0000_1008 */
    volatile unsigned int   spi_buf_size;       /* 0x0000_100A */
    volatile unsigned int   spi_drop_cnt;       /* 0x0000_100C */
} reggio_spi_a_reg_t;

/* Defines used for SPI PROM programming */
#define DEVICE_TYPE_ATMEL                       0x1F
#define DEVICE_TYPE_MICRO                       0x20
#define ID1_TYPE_AT25DF321                      0x47
#define ID1_TYPE_AT25DF641                      0x48
#define ID1_TYPE_AT25FS040                      0x66
#define ID1_TYPE_AT25DF041A                     0x44
#define ID1_TYPE_M25P40                         0x20
#define ID1_TYPE_M25PX32                        0x71
#define ID1_TYPE_M25P64  	                    0x20
			 
#define ID2_TYPE_AT25DF321                      0x00
#define ID2_TYPE_AT25FS040                      0x04
#define ID2_TYPE_AT25DF041A                     0x01
#define ID2_TYPE_M25P40                         0x13
#define ID2_TYPE_M25PX32                        0x16
#define ID2_TYPE_M25P64			           	    0x17

#define ID2_GENERIC_TYPE_ATMEL                  0x00
#define ID2_GENERIC_TYPE_MICRO                  0x17

#define  PROM_RD_ONE_BYTE                    0x00000000
#define  PROM_RD_THREE_BYTES                 0x00000002
#define  PROM_READ_ID                        0x9F000000
#define  PROM_READ_STATUS_REG                0x05000000
#define  PROM_DATA_READ                      0x00000002
#define  PROM_BLOCK_ERASE_TWKB               0x52000000
#define  PROM_DATA_WRITE                     0x00000002
#define  PROM_INSERT_FIELD_ADDR              0x00000001
#define  PROM_READ_ARRAY_LF                  0x03000000
#define  PROM_RD_FIFO_EMPTY                  0x00000002
#define  PROM_STAT_DONE                      0x00800000
#define  SPI_PROM_MAX_WAIT                   2500
#define PROM_OPER_DONE       0x00008000

#define START_ADDR_FOR_UPDRADE_REGGIO           0x00400000
#define START_ADDR_FOR_GOLDEN_REGGIO            0x00000000
#define  PROM_UPDATE_IMAGE                   2
#define  PROM_GOLDEN_IMAGE                   1

/* Bit definitions for SPI PROM Opcode and Address Register */
#define PROM_SPI_ADDR_MASK   0x00FFFFFF
#define PROM_SPI_OPCODE_MASK 0xFF000000
#define PROM_OPCODE_SHIFT    24



/* Bit definition for SPI PROM Read Size Register */
#define PROM_RD_MAX_BYTE     0x100
#define PROM_RD_SIZE_MASK    0x000000FF
#define PROM_RD_1_BYTE       0x00000000
#define PROM_RD_3_BYTE       0x00000002
#define PROM_RD_256_BYTE     (PROM_RD_MAX_BYTE - 1)
/* Bit definitions for SPI PROM Opcode */
#define PROM_WREN_OP         0x06000000   /* Write Enable  */
#define PROM_WRDI_OP         0x04000000   /* Write Disable */
#define PROM_RDID_OP         0x9F000000   /* Read Identification */
#define PROM_RDSR_OP         0x05000000   /* Read Status Register */
#define PROM_WRSR_OP         0x01000000   /* Write Status Register */
#define PROM_READ_OP         0x03000000   /* Read Data Bytes */
#define PROM_FAST_READ_OP    0x0B000000   /* Read Data Bytes at Higher Speed */
#define PROM_PAGE_PROG_OP    0x02000000   /* Page Program */
#define PROM_SECT_ERASE_OP   0xD8000000   /* Sector Erase */
#define PROM_BULK_ERASE_OP   0xC7000000   /* Bulk/Chip Erase */

/* Bit definitions for SPI PROM Control Register */
#define PROM_USE_ADDR        0x00000001
#define PROM_DATA_WRITE      0x00000002
#define PROM_USE_DUMMY       0x00000004
#define PROM_SWAP_DATA_BITS  0x00000008
#define PROM_DFLT_BAUD       0x00000030
#define PROM_BAUD_RATE_MASK  0x000007f0
#define PROM_DONE_INTR_ENAB  0x00008000

#define SPI_PROM_SECTOR_SIZE  0x10000


#define SPI_PROM_MAX_WAIT     2500
#define SPI_PROM_ERASE_WAIT   20000
#define PROM_BLANK_DATA       0xff
#define MAX_SECTOR_NUM        127
#define PROM_MAX_ERR_CNT      10

/* Bit definition for Read Status Register (RDSR) */
#define PROM_UNPROTECT_ALL   0x00
#define PROM_RDSR_WIP        0x01  /* Write in Progress */
#define PROM_RDSR_WEL        0x02  /* Write Enable Latch */
#define PROM_RDSR_BP0        0x04  /* Block 0 Protect */
#define PROM_RDSR_BP1        0x08  /* Block 1 Protect */
#define PROM_RDSR_BP2        0x10  /* Block 2 Protect */
#define PROM_RDSR_SRWD       0x80  /* Status Register Write Protect */

/* Defines for Secure boot Signatures */
typedef struct secure_boot_hash_s {
    char *hash_name;
    unsigned char *hash_ptr;
    unsigned int hash_addr;
    unsigned int hash_size;
} secure_boot_hash_t;

#define SBOOT_HASH_NUM            2  /* Number of hashes to program */
#define PROM_SECTOR_SIZE      0x10000   /* 1 sector = 64 kB = 512 kbit */
#define GOLDEN_HASH_SECTOR        61
#define GOLDEN_MULTI_BOOT_SECTOR  63
#define UPGRADE_HASH_SECTOR       125
#define UPGRADE_MULTI_BOOT_SECTOR 31
#define BIOS_VERSION_ADDR         0xFFF90000


#define FPGA_UPGRADE_IMAGE                      2
#define BIOS_UPGRADE_IMAGE                      2

#define FPGA_GOLDEN_IMAGE                1
#define BIOS_GOLDEN                      1
#define REGGIO_FPGA_HDR_SIZE             64

#define NEWER_UPGRADE_A0 0xA0
#define WRONG_UPGRADE_A3 0xA3

#define FPGA_RECONFI_SLEEP    500	/* 500ms */

#define SPI_STATUS_DONE             0x8000
#define SPI_STATUS_WR_ERR           0x0020
#define SPI_STATUS_WR_FIFO_FULL     0x0010
#define SPI_STATUS_WR_FIFO_EMPT     0x0008
#define SPI_STATUS_RD_FIFO_FULL     0x0004
#define SPI_STATUS_RD_FIFO_EMPT     0x0002
#define SPI_STATUS_WR_FIFO_OVER     0x0001

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

extern int display_spi_prom_a(int);
extern int display_prom_sector(int);
extern int save_downloaded_image_a(int);
extern int spi_prom_read_sector(int, unsigned char *, int);
extern int clear_spi_prom_image_header(void);
extern int program_reggio_spi_prom_old(int);
extern int program_image_upgrade_header(int);
extern int program_secure_boot_hash(void);
extern int read_secure_boot_image_header(void);
extern int clear_secure_boot_image_header(void);
extern int read_spi_prom_status_reg (unsigned char *);
extern int debug_reggio_read_spi_prom(void);
extern int debug_reggio_write_spi_prom(void);
extern int debug_reggio_erase_spi_prom(void);
extern int program_image_update_type(int);
extern int set_date_revision(int);
extern int erase_header_spi_prom_image(uchar, boolean);
extern int fpga_spi_read_id(uint *);
extern int fpga_write_spi_prom_status_reg(uchar);
extern int fpga_read_spi_prom_status_reg(uchar*);
extern int fpga_spi_write_protect_test(uint);
extern int is_read_fifo_empty(prom_t *);
extern int is_rd_wr_op_done(prom_t *);
extern void aikido_spi_read_util(void);
extern void aikido_spi_write_util(void);
extern void aikido_flag_mailbox(void);
extern void aikido_flag_act2(void);
extern int program_reggio_spi_prom(void);
extern int fpga_read_spi_prom_reg(int);
extern int fpga_write_spi_prom_reg(int);
extern boolean fpga_spi_flash_busy_status(void); 
extern int fpga_corrupt_header_spi_prom (int);

#endif /* __REGGIO_FPGA_REGS_H__ */

/* ------------------------------- End of file --------------------------- */

/*
$Log: diag_fpga_upgrade.h,v $
Revision 1.5  2020/05/07 01:34:56  alicehua
CSCvu09464: Add utility and modify test case for FPGA test plan.

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
