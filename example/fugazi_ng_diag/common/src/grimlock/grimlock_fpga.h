/* $Id: grimlock_fpga.h,v 1.2 2020/03/13 12:06:53 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/grimlock_fpga.h,v $
 *------------------------------------------------------------------
 * grimlock_fpga.h
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef _GRIMLOCK_FPGA_H_
#define _GRIMLOCK_FPGA_H_

#define TDMSW64_CONN_MEM_BASE         0
#define TDMSW64_CONN_MEM_SIZE         0x8000
#define TDMSW16_CONN_MEM_SIZE         0x0800

#define TDMSW64_NUM_TDM_STREAM        64
#define TDMSW16_NUM_TDM_STREAM        16

#define NUM_2M_TIMESLOTS              32
#define NUM_8M_TIMESLOTS              128
#define NUM_16M_TIMESLOTS             256
#define NUM_32M_TIMESLOTS             512

/*
 * Bit definitions for connection memory
 */
#define TDMSW64_CM_PASSWORD		0xCAC00000
#define TDMSW64_CM_FORCELSB		0x00004000
#define TDMSW64_CM_ODRV			0x00008000
#define TDMSW64_CM_FORCEBYTE		0x00010000

#define TDMSW64_CONN_MEM_PW_MASK	0x0001ffff
#define TDMSW64_CONN_MEM_DATA_MASK      0xfff1ffff
#define TDMSW64_INPUT_CID_MASK	        0x00003fff
#define NUM_T1_TSLOT			24
#define NUM_E1_TSLOT			32

#define TDM64_DS0_DUMP_STREAM           61
#define TDM16_DS0_DUMP_STREAM           13

#define TDMSW64_NGVM_STREAM_1           52
#define TDMSW64_NGVM_STREAM_2           56
#define TDMSW16_NGVM_STREAM_1           8
#define TDMSW16_NGVM_STREAM_2           10

#define MEM_INCREMENT_1                 0x00000001
#define MEM_DECREMENT_1                 0xffffffff

#define TDMSW64_REG_BASE              0x8000

typedef struct tdmsw64_reg_t_ {
    uint32_t enbl_63_32;        /* 0x8000, enable TDM streams in 63-32 range */
    uint32_t enbl_31_00;        /* 0x8004, enable TDM streams in 31-00 range */
    uint32_t reserve1[2];       /* 0x8008 - 0x800c */
    uint32_t rate_63_48;        /* 0x8010, set rate for TDM streams in 63-48 */
    uint32_t rate_47_32;        /* 0x8014, set rate for TDM streams in 47-32 */
    uint32_t rate_31_16;        /* 0x8018, set rate for TDM streams in 31-16 */
    uint32_t rate_15_00;        /* 0x801c, set rate for TDM streams in 15-00 */
    uint32_t lpbk_63_32;        /* 0x8020, set lpbk for TDM streams in 63-32 */
    uint32_t lpbk_31_00;        /* 0x8024, set lpbk for TDM streams in 31-00 */
    uint32_t reserve2;          /* 0x8028 */
    uint32_t tdmsw64_ctl;       /* 0x802c, TDMSW64 control register */
    uint32_t ngvmtdm_ctl;       /* 0x8030, configure NGVM TDM clk/sync */
} tdmsw64_reg_t;

#define FPGA_MB_REG_BASE              0x8100

typedef struct fpga_mb_reg_t_ {
    uint32_t mb_ctrl;           /* 0x8100, multiboot control register */
    uint32_t mb_stat;           /* 0x8104, multiboot status register */
    uint32_t mb_hdr_id;         /* 0x8108, multiboot header ID register */
    uint32_t mb_hdr_date;       /* 0x810c, multiboot header date register */
    uint32_t mb_hdr_flag;       /* 0x8110, multiboot header flag register */
    uint32_t mb_hdr_magic_num;  /* 0x8114, multiboot header magic num register */
    uint32_t mb_history;        /* 0x8118, multiboot state history register */
} fpga_mb_reg_t;

#define FPGA_SPI_REG_BASE              0x9040

typedef struct fpga_spi_reg_t_ {
    uint8_t fpga_spi_ctrl[4];    /* 0x9040, FPGA spi control register */
    uint8_t fpga_spi_stat[4];    /* 0x9044, FPGA spi status register */
    uint8_t fpga_spi_rd_size[4]; /* 0x9048, FPGA spi read size register */
    uint8_t fpga_spi_data[4];    /* 0x904c, FPGA spi data register */
    uint8_t fpga_spi_addr[4];    /* 0x9050, FPGA spi address & opcode register */
} fpga_spi_reg_t;

#define FPGA_GENERAL_REG_BASE         0x9000

typedef struct fpga_reg_t_ {
    uint8_t fpga_rev[4];        /* 0x9000, FPGA timestamp */
    uint8_t misc_ctl;           /* 0x9004, misc control */
    uint8_t board_id;           /* 0x9005, board ID */
    uint8_t reserve1[2];        /* 0x9006 - 0x9007 */
    uint8_t fpga_int_event;     /* 0x9008, FPGA interrupt sources */ 
    uint8_t fpga_int_diag_test; /* 0x9009, FPGA interrupt diag test */
    uint8_t reserve2[2];        /* 0x900a - 0x900b */ 
    uint8_t fpga_int_event_ena; /* 0x900c, FPGA interrupt event mask */
    uint8_t reserve3[3];        /* 0x900d - 0x900f */
    uint8_t tdmsw_cmd_status;   /* 0x9010, TDMSW indirect access cmd/status */
    uint8_t reserve4[3];        /* 0x9011 - 0x9013 */
    uint8_t tdmsw_adr[2];       /* 0x9014, TDMSW indirect access address */
    uint8_t reserve5[2];        /* 0x9016 - 0x9017 */
    uint8_t tdmsw_data[4];      /* 0x9018, TDMSW indirect access data */
    uint8_t pll_ctrl_status[2]; /* 0x901c, TDM PLL control and status */
    uint8_t reserve6[2];        /* 0x901e - 0x901f */
    uint8_t led_ctrl[4];        /* 0x9020, T1/E1 port LEDs */
    uint8_t pmc_mode_status1[3];/* 0x9024, framer snoop mode status1 */
    uint8_t reserve7;           /* 0x9027 */
    uint8_t pmc_mode_status2[4];/* 0x9028, framer snoop mode status2 */
    uint8_t ds0_dump_ctrl;      /* 0x902c, DS0_DUMP control */
    uint8_t reserve8[3];        /* 0x902d - 0x902f */
    uint8_t fpga_ver[4];        /* 0x9030, FPGA revision */
} fpga_reg_t;

#define DS0_DUMP_BUFFER_BASE          0xa000
#define DS0_DUMP_BUFFER_SIZE          0x2000

/* define for TDMSW64_STREAM_RATE registers */
typedef enum {
    TDM_STREAM_2M = 0,
    TDM_STREAM_8M,
    TDM_STREAM_16M,         
    TDM_STREAM_32M,
} stream_rate;

typedef enum {
    CTCLK_SRC_8K = 0,
    CTCLK_SRC_2M,
} ctclk_src;

#define TDMSW64_STREAM_ENBL63_32_OFFSET         0x0
#define TDMSW64_STREAM_ENBL31_00_OFFSET         0x4
#define TDMSW64_STREAM_RATE63_48_OFFSET         0x10
#define TDMSW64_STREAM_RATE47_32_OFFSET         0x14
#define TDMSW64_STREAM_RATE31_16_OFFSET         0x18
#define TDMSW64_STREAM_RATE15_00_OFFSET         0x1c
#define TDMSW64_STREAM_LPBK63_32_OFFSET         0x20
#define TDMSW64_STREAM_LPBK31_00_OFFSET         0x24
#define TDMSW64_CTL_OFFSET                      0x2c
#define NGVMTDM_CTL_OFFSET                      0x30

#define MB_CTRL_OFFSET                          0x0
#define MB_STAT_OFFSET                          0x4
#define MB_HDR_ID_OFFSET                        0x8
#define MB_HDR_DATE_OFFSET                      0xc
#define MB_HDR_FLAG_OFFSET                      0x10
#define MB_HDR_MAGIC_NUM_OFFSET                 0x14
#define MB_HISTORY_OFFSET                       0x18

#define FPGA_SPI_CTRL_OFFSET                    0x0
#define FPGA_SPI_STAT_OFFSET                    0x4
#define FPGA_SPI_RD_SIZE_OFFSET                 0x8
#define FPGA_SPI_DATA_OFFSET                    0xc
#define FPGA_SPI_ADDR_OFFSET                    0x10

/* define for fpga_spi_ctrl register */
#define FPGA_SPI_ADDR_EN              0x1
#define FPGA_SPI_WRITE                0x2
#define FPGA_SPI_DUMMY_BYTE_EN        0x4

/* define for fpga_spi_stat register */
#define FPGA_SPI_WR_FIFO_ORUN         0x1
#define FPGA_SPI_RD_FIFO_EMPTY        0x2
#define FPGA_SPI_RD_FIFO_FULL         0x4
#define FPGA_SPI_WR_FIFO_EMPTY        0x8
#define FPGA_SPI_WR_FIFO_FULL         0x10
#define FPGA_SPI_DONE                 0x80

#define FPGA_SPI_PAGE_SIZE            256
#define SECONDARY_FPGA_IMAGE_START_ADDR 0x100000
#define GOLDEN_FPGA_IMAGE_START_ADDR    0x0

/* define for SPI flash opcode */
#define RD_IDENTIFICATION             0x9f
#define RD_STATUS                     0x05
#define RD_DATA_BYTES                 0x03
#define RD_DATA_BYTES_HIGH_SPEED      0x0b
#define PAGE_PROGRAM                  0x02
#define WRITE_ENABLE                  0x06
#define SECTOR_ERASE                  0xd8
#define WR_STATUS                     0x01

/* define for SPI flash status register */
#define RDSR_WIP                      0x01
#define RDSR_WEL                      0x02
 
/* define for TDMSW64_CTL register */
#define TDMSW_RST                     0x10

/* define for MISC_CONTROL register */
#define NOR_FLASH_A23_INV             0x20
#define CTC_SRC_2M                    0x08
#define FRAMER_TXHIZ                  0x04
#define TDM_PLL_RST                   0x02
#define FRAMER_RST                    0x01

/* define for board ID register */
#define BOARD_2PORTS                  0x01
#define BOARD_4PORTS                  0x02
#define BOARD_8PORTS                  0x03
#define BOARD_CHANNELIZED             0x08
#define BOARD_ID_MASK                 0x03

/* define for FPGA interrupt event */
#define TDM_PLL_INT                   0x40
#define SPI_FLASH_DONE                0x20
#define DS0_DUMP_DONE                 0x10
#define FPGA_LS_ERR                   0x08
#define TDMSW_LS_ERR                  0x04
#define TDMSW_FSYNC_UNEXP_ERR         0x02
#define TDMSW_FSYNC_MISS_ERR          0x01

/* define for TDMSW_CMD_STATUS register */
#define TDMSW_BUSY                    0x04
#define TDMSW_CMD_WRITE               0x02
#define TDMSW_CMD_READ                0x00
#define TDMSW_CMD_GO                  0x01
#define TDMSW_CMD_WAIT                5000

/* define for TDM_PLL_CNTL_STATUS register */
#define TDMPLL_PRI_ENA                0x04
#define RSYNC_FREQ_8K                 0x02
#define NGWIC_SYNC_OUT                0x01
#define NGWIC_SYNC_EN                 0x10
#define TDMPLL_LOCK                   0x01
#define TDMPLL_REF_FAIL               0x02

/* define for PMC_MODE_STATUS1 register */
#define SNOOP_E1                      0x1
#define PORT0_PMC_CLK_MASTER          0x1

/* define for LED_CTRL register */
#define PORT0_LED_AL_ON               0x01
#define PORT0_LED_CD_ON               0x02
#define PORT0_LED_LP_ON               0x04
#define PORT1_LED_AL_ON               0x10
#define PORT1_LED_CD_ON               0x20
#define PORT1_LED_LP_ON               0x40
#define LED_PORT_SHIFT                4

#define PORT0_PMC_CLK_MSTR            1

#define DS0_DUMP_GO                   0x1
#define DS0_DUMP_CIRCULAR_ON          0x2
#define DS0_DUMP_TOP_RDY              0x4
#define DS0_DUMP_BOT_RDY              0x8

#endif  /* _GRIMLOCK_FPGA_H_ */

/******** History ********
$Log: grimlock_fpga.h,v $
Revision 1.2  2020/03/13 12:06:53  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.2  2020/01/15 03:30:11  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/
