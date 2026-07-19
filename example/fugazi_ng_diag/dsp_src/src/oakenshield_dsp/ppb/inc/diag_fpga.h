/* $Id: diag_fpga.h,v 1.4 2021/04/15 00:52:44 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/diag_fpga.h,v $
 *------------------------------------------------------------------
 *
 * diag_fpga.h - This file contains definitions for Oakenshield FPGA.
 *
 * Owen Lin - 2016 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define TDMSW16_CONN_MEM_BASE         0
#define TDMSW16_CONN_MEM_SIZE         0x800

#define TDMSW16_NUM_TDM_STREAM        16

#define NUM_2M_TIMESLOTS              32
#define NUM_8M_TIMESLOTS              128
#define NUM_16M_TIMESLOTS             256
#define NUM_32M_TIMESLOTS             512

#define TDM_RATE_2M                   2
#define TDM_RATE_8M                   8
#define TDM_RATE_16M                  16
#define TDM_RATE_32M                  32


/*
 * Bit definitions for connection memory
 */
#define TDMSW16_CM_PASSWORD		0xCAC00000
#define TDMSW16_CM_FORCELSB		0x00004000
#define TDMSW16_CM_ODRV			0x00008000
#define TDMSW16_CM_FORCEBYTE		0x00010000

#define TDMSW16_CONN_MEM_PW_MASK	0x0001ffff
#define TDMSW16_CONN_MEM_DATA_MASK      0xfff1ffff
#define TDMSW16_INPUT_CID_MASK	        0x00003fff
#define NUM_T1_TSLOT			24
#define NUM_E1_TSLOT			32

#define TDM16_DS0_DUMP_STREAM           13

#define MEM_INCREMENT_1                 0x00000001
#define MEM_DECREMENT_1                 0xffffffff

#define TDMSW16_REG_BASE              0x8000
#define STREAM0                       0X0
#define STREAM1                       0X1


#define TDMSW16_REG_BASE_ENBL_31_00              0x8004
#define TDMSW16_REG_BASE_RATE_15_00              0x801C
#define TDMSW16_REG_BASE_LPBK_31_00              0x8024
#define TDMSW16_REG_BASE_TDMSW16_CTL             0x802C
#if 0
typedef struct tdmsw16_reg_t_ {
    uint32_t reserve0;          /* 0x8000 - 0x8003 */
    uint32_t enbl_31_00;        /* 0x8004, enable TDM streams in 31-00 range */
    uint32_t reserve1[5];       /* 0x8008 - 0x801b */
    uint32_t rate_15_00;        /* 0x801c, set rate for TDM streams in 15-00 */
    uint32_t reserve2;          /* 0x8020 - 0x8023 */
    uint32_t lpbk_31_00;        /* 0x8024, set lpbk for TDM streams in 31-00 */
    uint32_t reserve3;          /* 0x8028 */
    uint32_t tdmsw16_ctl;       /* 0x802c, TDMSW16 control register */
} tdmsw16_reg_t;
#endif

#define FPGA_MB_REG_BASE              0x8100
#define FPGA_MB_CTRL_REG              0x8100
#define FPGA_MB_STAT_REG              0x8104
#define FPGA_MB_HDRLD_REG             0x8108
#define FPGA_MB_HDRDATA_REG           0x810C
#define FPGA_MB_HDRFLAG_REG           0x8110
#define FPGA_MB_HDRMAGIC_REG          0x8114
#define FPGA_MB_HISTORY_REG           0x8118

#if 0
typedef struct fpga_mb_reg_t_ {
    uint32_t mb_ctrl;           /* 0x8100, multiboot control register */
    uint32_t mb_stat;           /* 0x8104, multiboot status register */
    uint32_t mb_hdr_id;         /* 0x8108, multiboot header ID register */
    uint32_t mb_hdr_date;       /* 0x810c, multiboot header date register */
    uint32_t mb_hdr_flag;       /* 0x8110, multiboot header flag register */
    uint32_t mb_hdr_magic_num;  /* 0x8114, multiboot header magic num register */
    uint32_t mb_history;        /* 0x8118, multiboot state history register */
} fpga_mb_reg_t;
#endif

#define FPGA_SPI_REG_BASE              0x9040
#define FPGA_SPI_REG_BASE_CTRL         0x9040
#define FPGA_SPI_REG_BASE_STAT_FIFO    0x9044
#define FPGA_SPI_REG_BASE_STAT_DONE    0x9045
#define FPGA_SPI_REG_BASE_RD_SIZE      0x9048
#define FPGA_SPI_REG_BASE_DATA         0x904C

#define FPGA_SPI_REG_BASE_ADDR         0x9050
#define FPGA_SPI_REG_BASE_ADDR_LO      0x9050
#define FPGA_SPI_REG_BASE_ADDR_ME      0x9051
#define FPGA_SPI_REG_BASE_ADDR_HI      0x9052
#define FPGA_SPI_REG_BASE_ADDR_OPC     0x9053


typedef struct fpga_spi_reg_t_ {
    uint8_t fpga_spi_ctrl[4];    /* 0x9040, FPGA spi control register */
    uint8_t fpga_spi_stat[4];    /* 0x9044, FPGA spi status register */
    uint8_t fpga_spi_rd_size[4]; /* 0x9048, FPGA spi read size register */
    uint8_t fpga_spi_data[4];    /* 0x904c, FPGA spi data register */
    uint8_t fpga_spi_addr[4];    /* 0x9050, FPGA spi address & opcode register */
} fpga_spi_reg_t;

#define FPGA_GENERAL_REG_BASE              0x9000
#define FPGA_GENERAL_FPGA_DATA             0x9000
#define FPGA_GENERAL_MISC_CONTROL          0x9004
#define FPGA_GENERAL_BOARD_ID              0x9005
#define FPGA_GENERAL_FPGA_IMAGE_SB         0x9006
#define FPGA_GENERAL_FPGA_INT_EVENT        0x9008
#define FPGA_GENERAL_FPGA_INT_DIAG_TEST    0x9009
#define FPGA_GENERAL_FPGA_INT_EVENT_ENA    0x900C
#define FPGA_GENERAL_TDMSW_CMD_STATUS      0x9010
#define FPGA_GENERAL_TDMSW_ADR_LO          0x9014
#define FPGA_GENERAL_TDMSW_ADR_HI          0x9015
#define FPGA_GENERAL_TDMSW_DATA_0          0x9018
#define FPGA_GENERAL_TDMSW_DATA_1          0x9019
#define FPGA_GENERAL_TDMSW_DATA_2          0x901A
#define FPGA_GENERAL_TDMSW_DATA_3          0x901B
#define FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C  0x901C
#define FPGA_GENERAL_TDM_PLL_CTRL_STAT_1D  0x901D
#define FPGA_GENERAL_FPGA_REV              0x9020
#define FPGA_GENERAL_DS0_DUMP_CNTL         0x902C
#define FPGA_GENERAL_FXS_FXO_LED           0x90E0

/* Phoenix only MB or DB test */
#define PHOENIX_ONLY_TEST_MB_MASK           0x8
#define PHOENIX_ONLY_TEST_DB1_MASK          0x4
#define PHOENIX_ONLY_TEST_DB2_MASK          0x2
#define PHOENIX_ONLY_TEST_DB3_MASK          0x1

/* Phoenix Host HW board type */
#define PHOENIX_HW_BRD_TYPE(x)             (x & (0x7))
#define PHOENIX_144FXS_HW_BRD_TYPE          7
#define PHOENIX_132FXS_6FXO_HW_BRD_TYPE     6
#define PHOENIX_84FXS_6FXO_HW_BRD_TYPE      5

/* Phoenix only one LED */
#define PHOENIX_FPGA_GENERAL_LED           0x90E0

#define PHOENIX_ALL_LED_EN                 0xFFFFFFFE 
#define PHOENIX_ALL_LED_DIS                0x00000001


/* Phoenix DSP SPI Control Register */
#define PHOENIX_FPGA_DSP_SPI_CTRL           0x937C

#define PHOENIX_FPGA_DSP_SPI_CTRL_ID_MASK   0xFE000000
#define PHOENIX_FPGA_DSP_SPI_ID_SHIFT       25
#define PHOENIX_FPGA_DSP0_SPI_CTRL_ID       0x0
#define PHOENIX_FPGA_DSP1_SPI_CTRL_ID       0x66

#define PHOENIX_FPGA_DSP_SPI_CTRL_INDICATOR_MASK   0x01000000

#define PHOENIX_FPGA_DSP_SPI_CTRL_KEY        0xABCDEF
#define PHOENIX_FPGA_DSP_SPI_CTRL_KEY_MASK   0xFF000000

/* FXS/FXO Port Led for VG400 */ 
#define ALL_FXSP_LEDGRN                0xAAAAFFFF
#define FXSP0_LEDGRN                   0xFFFEFFFF
#define FXSP1_LEDGRN                   0xFFFBFFFF
#define FXSP2_LEDGRN                   0xFFEFFFFF
#define FXSP3_LEDGRN                   0xFFBFFFFF
#define FXSP4_LEDGRN                   0xFEFFFFFF
#define FXSP5_LEDGRN                   0xFBFFFFFF
#define FXSP6_LEDGRN                   0xEFFFFFFF
#define FXSP7_LEDGRN                   0xBFFFFFFF
#define ALL_FXSP_LEDRED                0x5555FFFF
#define FXSP0_LEDRED                   0xFFFDFFFF
#define FXSP1_LEDRED                   0xFFF7FFFF
#define FXSP2_LEDRED                   0xFFDFFFFF
#define FXSP3_LEDRED                   0xFF7FFFFF
#define FXSP4_LEDRED                   0xFDFFFFFF
#define FXSP5_LEDRED                   0xF7FFFFFF
#define FXSP6_LEDRED                   0xDFFFFFFF
#define FXSP7_LEDRED                   0x7FFFFFFF
#define ALL_FXOP_LEDGRN                0xFFFFAAAF 
#define FXOP0_LEDGRN                   0xFFFFFFEF
#define FXOP1_LEDGRN                   0xFFFFFFBF
#define FXOP2_LEDGRN                   0xFFFFFEFF
#define FXOP3_LEDGRN                   0xFFFFFBFF
#define FXOP4_LEDGRN                   0xFFFFEFFF
#define FXOP5_LEDGRN                   0xFFFFBFFF
#define ALL_FXOP_LEDRED                0xFFFF555F
#define FXOP0_LEDRED                   0xFFFFFFDF
#define FXOP1_LEDRED                   0xFFFFFF7F
#define FXOP2_LEDRED                   0xFFFFFDFF
#define FXOP3_LEDRED                   0xFFFFF7FF
#define FXOP4_LEDRED                   0xFFFFDFFF
#define FXOP5_LEDRED                   0xFFFF7FFF

#define ALL_LEDGRN_EN                  0xAAAAAAAF     
#define ALL_REDLED_EN                  0x5555555F
#define ALL_LED_EN                     0x0000000F
#define ALL_LED_DIS                    0xFFFFFFFF

#if 0
typedef struct fpga_reg_t_ {
    uint8_t fpga_rev[4];        /* 0x9000, FPGA timestamp */
    uint8_t misc_ctl;           /* 0x9004, misc control */
    uint8_t board_id;           /* 0x9005, board ID */
    uint8_t fpga_image_sb;      /* 0x9006, SB image or not */
    uint8_t reserve1;           /* 0x9007 */
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
    uint8_t pll_ctrl_status[4]; /* 0x901c - 0x901f, TDM PLL control and status */
    uint8_t fpga_ver[4];        /* 0x9020, FPGA revision */
    uint8_t reserve6[8];        /* 0x9024 - 0x902b */
    uint8_t ds0_dump_ctrl;      /* 0x902c, DS0_DUMP control */
    uint8_t reserve8[3];        /* 0x902d - 0x902f */
} fpga_reg_t;
#endif

#if 0
#define FPGA_SB_REG_BASE              0x9060

typedef struct fpga_sb_reg_t_ {
    uint8_t sb_stat_gd[4];      /* 0x9060, secure boot core status
                                 *         golden register  */
    uint8_t sb_chk_gd[4];       /* 0x9064, secure boot check status
                                 *	   golden register  */
    uint8_t sb_sign_addr_gd[4]; /* 0x9068, secure boot signature address
                                 *	   golden register  */
    uint8_t sb_sign_size_gd[4]; /* 0x906C, secure boot signature size
                                 *	   golden register  */
    uint8_t sb_stat_up[4];      /* 0x9070, secure boot core status
                                 *         upgrade register */
    uint8_t sb_chk_up[4];       /* 0x9074, secure boot check status 
                                 *         upgrade register */
    uint8_t sb_sign_addr_up[4]; /* 0x9078, secure boot signature address
                                 *         upgrade register */
    uint8_t sb_sign_size_up[4]; /* 0x907C, secure boot signature size
                                 *         upgrade register */

} fpga_sb_reg_t;
#endif

#define FPGA_DSP_REG_BASE       0x9080
#define FPGA_TDM_0_RESET       0x9080
#define FPGA_TDM_1_RESET       0x9090
#define FPGA_TDM_2_RESET       0x90A0
#define FPGA_TDM_3_RESET       0x90B0
#define FPGA_TDM_4_RESET       0x90C0
#define FPGA_TDM_5_RESET       0x90D0

#define FPGA_PHOENIX_TDM_0_RESET    0x9080
#define FPGA_PHOENIX_TDM_1_RESET    0x9084
#define FPGA_PHOENIX_TDM_2_RESET    0x9088
#define FPGA_PHOENIX_TDM_3_RESET    0x908C
#define FPGA_PHOENIX_TDM_4_RESET    0x9090
#define FPGA_PHOENIX_TDM_5_RESET    0x9094
#define FPGA_PHOENIX_TDM_6_RESET    0x9098
#define FPGA_PHOENIX_TDM_7_RESET    0x909C
#define FPGA_PHOENIX_TDM_8_RESET    0x90A0
#define FPGA_PHOENIX_TDM_9_RESET    0x90A4

#if 0
typedef struct fpga_dsp_reg_t_ {
    uint8_t mb_fxs_rst;         /* 0x9080, MB FXS codec reset */
    uint8_t mb_fxs_led;         /* 0x9081, MB FXS led */
    uint8_t reserve1[14];       /* 0x9082 - 0x908f */
    uint8_t mb_fxo_rst;         /* 0x9090, MB FXO codec reset */
    uint8_t mb_fxo_led;         /* 0x9091, MB FXO led */
    uint8_t reserve2[14];       /* 0x9092 - 0x909f */
    uint8_t mb_em_led;          /* 0x90a0, MB E/M led and reset */
    uint8_t mb_em_lcr;          /* 0x90a1, MB E/M LMR control */
    uint8_t reserve3[14];       /* 0x90a2 - 0x90af */
    uint8_t mb_bri_rst;         /* 0x90b0, MB BRI reset */
    uint8_t mb_bri_led;         /* 0x90b1, MB BRI led */
    uint8_t mb_bri_nt_te;       /* 0x90b2, MB BRI mode select */
    uint8_t reserve4[13];       /* 0x90b3 - 0x90bf */
    uint8_t dc_fxo_rst;         /* 0x90c0, DC FXO codec reset */
    uint8_t dc_fxo_led;         /* 0x90c1, DC FXO led */
    uint8_t reserve5[14];       /* 0x90c2 - 0x90cf */
    uint8_t dc_em_led;          /* 0x90d0, DC E/M led and reset */
    uint8_t dc_em_lcr;          /* 0x90d1, DC E/M LMR control */
    uint8_t reserve6[14];       /* 0x90d2 - 0x90df */
    uint8_t dc_bri_rst;         /* 0x90e0, DC BRI reset */
    uint8_t dc_bri_led;         /* 0x90e1, DC BRI led */
    uint8_t dc_bri_nt_te;       /* 0x90e2, DC BRI mode select */
    uint8_t reserve7[13];       /* 0x90e3 - 0x90ef */
} fpga_dsp_reg_t;
#endif

#define DS0_DUMP_BUFFER_BASE          0xa000
#define DS0_DUMP_BUFFER_SIZE          0x2000


/* define for TDMSW16_STREAM_RATE registers */
typedef enum {
    TDM_STREAM_2M = 0,
    TDM_STREAM_8M,
    TDM_STREAM_16M,         
    TDM_STREAM_32M,
} stream_rate;
#define TDM_STREAM_MESK 0x0003

typedef enum {
    CTCLK_SRC_8K = 0,
    CTCLK_SRC_2M,
} ctclk_src;

#define TDMSW16_STREAM_ENBL31_00_OFFSET         0x4
#define TDMSW16_STREAM_RATE15_00_OFFSET         0x1c
#define TDMSW16_STREAM_LPBK31_00_OFFSET         0x24
#define TDMSW16_CTL_OFFSET                      0x2c
#define SET_TDM_LPBK_TRUE                       0x00ff

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

#define FPGA_REV_OFFSET   0x000FFFC0   /* The revision position in FPGA image file */
#define PHOENIX_UP_FILE_FPGA_REV_OFFSET   0x001FF000   /* The revision position in FPGA image file */
#define PHOENIX_FPGA_REV_OFFSET   0x007FF000   /* The revision position in FPGA image file */
#define PHOENIX_SPI_FPGA_UPGRADE_SECTOR_SIZE  64    /* there are 64 of 64KB in the 4MB SPI flash upgrade section */
#define FPGA_DATE_LEN                 4     /* four bytes of fpga rev based on yymmddhh*/
#define FPGA_SPI_PAGE_SIZE            256
#define SPI_ERASE_SECTOR_SIZE         0x10000  /* 64KB */
#define SPI_FPGA_UPGRADE_SECTOR_SIZE  16    /* there are 16 of 64KB in the 1MB SPI flash upgrade section */
#define UNPROTECT_ALL_SECTORS 0x00
#define SECONDARY_FPGA_IMAGE_START_ADDR 0x100000
#define PHOENIX_SECONDARY_FPGA_IMAGE_START_ADDR 0x400000 
#define SPI_FLASH_UPGRADE_SIZE        0x100000   /* SPI flash upgrade image size */
#define PHOENIX_SPI_FLASH_UPGRADE_SIZE        0x200000   /* PHOENIX SPI image size 2M */
#define GOLDEN_FPGA_IMAGE_START_ADDR    0x0
#define PHOENIX_CPLD_UPGRADE_SIZE       0x10000

/* define for SPI flash opcode */
#define RD_IDENTIFICATION             0x9f
#define RD_STATUS                     0x05
#define RD_DATA_BYTES                 0x03
#define RD_DATA_BYTES_HIGH_SPEED      0x0b
#define PAGE_PROGRAM                  0x02
#define WRITE_ENABLE                  0x06
#define SECTOR_ERASE                  0xd8
#define WRITE_STATUS                  0x01

/* define for SPI flash status register */
#define RDSR_WIP                      0x01
#define RDSR_WEL                      0x02
 
/* define for TDMSW16_CTL register */
#define TDMSW_RST                     0x10

/* define for MISC_CONTROL register */
#define TDM_PLL_RST                   0x02
#define SGMII_RST                     0x01

/* define for board ID register */
#define FPGA_MB_BOARD_ID_MASK         0xFF
#define BOARD_16FXS_2FXO              0x00
#define BOARD_24FXS_4FXO              0x01
#define BOARD_8FXS_12FXO              0x02
#define BOARD_72FXS                   0x03
#define VG400_2FXS_2FXO               0x04
#define VG400_4FXS_4FXO               0x05
#define VG400_6FXS_6FXO               0x06
#define VG400_8FXS                    0x07
#define PHOENIX_144FXS                0x08
#define PHOENIX_132FXS_6FXO           0x09
#define PHOENIX_84FXS_6FXO            0x0A
#define BOARD_RESERVED                0xFF

#define BOARD_DC_PRESENT              0x10
#define BOARD_ID_MASK                 0x0F
#define BOARD_DB1_MASK                0x10
#define BOARD_DB2_MASK                0x20
#define BOARD_DB3_MASK                0x40

/* define for FPGA interrupt event */
#define TDM_PLL_INT                   0x80
#define DS0_DUMP_RDY                  0x40
#define DS0_DUMP_ERR                  0x20
#define FPGA_LS_ERR                   0x10
#define TDMSW_PWD_ERR                 0x08
#define TDMSW_PAR_ERR                 0x04
#define TDMSW_FSYNC_UNEXP_ERR         0x02
#define TDMSW_FSYNC_MISS_ERR          0x01

/* define for TDMSW_CMD_STATUS register */
#define TDMSW_BUSY                    0x04
#define TDMSW_CMD_WRITE               0x02
#define TDMSW_CMD_READ                0x00
#define TDMSW_CMD_GO                  0x01
#define TDMSW_CMD_WAIT                5000

/* define for TDM_PLL_CNTL_STATUS register */
#define NIM_SYNC_OUT_SM1_EN           0x20
#define NIM_SYNC_EN                   0x10
#define NIM_SYNC_FREQ_25M             0x08
#define TDMPLL_PRI_ENA                0x04
#define TDMPLL_PRI_SEL                0x01
#define TDMPLL_LOCK                   0x01
#define TDMPLL_REF_FAIL               0x02

#define DS0_DUMP_GO                   0x1
#define DS0_DUMP_CIRCULAR_ON          0x2
#define DS0_DUMP_TOP_RDY              0x4
#define DS0_DUMP_BOT_RDY              0x8

#define MB_FXS_RST                    0x1
#define MB_FXO_P0_RST		      0x1
#define MB_FXO_P1_RST		      0x2
#define MB_FXO_RST_ALL (MB_FXO_P0_RST | MB_FXO_P1_RST)
#define DC_FXO_P0_RST		      0x1
#define DC_FXO_P1_RST		      0x2
#define DC_FXO_P2_RST		      0x4
#define DC_FXO_P3_RST		      0x8
#define DC_FXO_RST_ALL (DC_FXO_P0_RST|DC_FXO_P1_RST|DC_FXO_P2_RST|DC_FXO_P3_RST)


#define READ_WRITE    0x0
#define READ_ONLY     0x1
#define WRITE_ONLY    0x2
#define SAVE_RESTORE  0x4	/* For READ_WRITE only. Will not use resetval */
#define REG_ACCESS    0x8	/* Caller provided read/write access */
#define REG_DEV	   0x10	/* Device specifics */

/* control flag */
#define TDM_CONNECT  0x1
#define TDM_DISCONNECT 0x0
#define TDM_ONE_WAY  0x0
#define TDM_BOTH_WAY 0x2

/* CPLD upgrade through JTAG */
#define MB_JTAG_CONTROL     0x9358
#define DB2_JTAG_CONTROL    0x9350
#define DB3_JTAG_CONTROL    0x9354
#define JTAG_DISABLE        0x00
#define JTAG_ENABLE         0x10
#define JTAG_TCK_MASK       0x8
#define JTAG_TMS_MASK       0x4
#define JTAG_TDO_MASK       0x2
#define JTAG_TDI_MASK       0x1
#define MB_PLD_REV          0x9370
#define DB2_PLD_REV         0x9374
#define DB3_PLD_REV         0x9378
#define JTAG_CON_SIZE       0x1
#define PLD_REV_SIZE        0x4


/* Phoenix FXS and FXO port table for CODEC reset test. */
#define PHOENIX_DB_NUM_MAX 3  /* DB1, DB2, DB3 */
#define PHOENIX_FXS_PORT_NUM_PER_CODEC 2
#define PHOENIX_FXO_PORT_NUM_PER_CODEC 1

#define DB_NUM_MAX (PHOENIX_DB_NUM_MAX)
#define FXS_PORT_NUM_PER_CODEC (PHOENIX_FXS_PORT_NUM_PER_CODEC)
#define FXO_PORT_NUM_PER_CODEC (PHOENIX_FXO_PORT_NUM_PER_CODEC)
#define BRD_NUM_MAX (DB_NUM_MAX+1)
#define PORT_IDX_NUM_MAX (2+2*DB_NUM_MAX)

#define BOARD_MB_TEST 0
#define BOARD_DB1_TEST 1
#define BOARD_DB2_TEST 2
#define BOARD_DB3_TEST 3

#define MB_INDEX 0
#define DB1_INDEX 2
#define DB2_INDEX 4
#define DB3_INDEX 6

#define DIAG_CODEC_NORMAL_MODE 0
#define DIAG_CODEC_RESET_MODE 1

typedef struct {
    int board;                            /* Board ID */
    int fxs_test_port[PORT_IDX_NUM_MAX];  /* FXS ports for testing */
    int fxo_test_port[PORT_IDX_NUM_MAX];  /* FXO ports for testing */
} codec_rst_port_tbl_t;

extern int  fpga_reg_test(void);
extern int  fpga_mem_test(void);
extern void fpga_unreset_tdm_pll(void);
extern void fpga_reset_tdm_pll(void);
extern int fpga_reset_tdmsw(void);
extern int show_tdmsw_regs(void);
extern int tdmsw_peek_reg(void);
extern int tdmsw_poke_reg(void);
extern int tdmsw_peek_conn_mem(void);
extern int tdmsw_poke_conn_mem(void);
extern int show_gen_regs(void);
extern int fpga_peek_reg(void);
extern int fpga_poke_reg(void);
extern int fpga_peek_dump_mem(void);
extern int oak_fpga_upgrade_golden(void);
extern int oak_fpga_upgrade_secondary(void);
extern int show_mb_regs(void);
extern int mb_peek_reg(void);
extern int mb_poke_reg(void);
extern int show_spi_regs(void);
extern int spi_peek_reg(void);
extern int spi_poke_reg(void);
extern int peek_spi_flash(void);
extern int poke_spi_flash(void);
extern int fpga_intr_test(void);
extern int is_vg400(void);
extern int is_phoenix(void);
extern int phoenix_has_dbx(int);
extern int phoenix_db1_only_fxs(void);
extern void toggle_sep_test_dbx_flag(uint32_t);
extern void set_host_hw_brd_type_flag(uint32_t);
extern int phoenix_only_test_dbx_flag;
extern int tdm_codec_reset_test(void);
extern int fpga_upgrade_cpld(void);

extern void msleep(uint32);
extern void usleep(uint32);


typedef struct tdmsw_xconnect_cmd_ {
    int connect; /* 0 = disconnect, 1 = connect */
    int src_str; /* source stream number */
    int src_ts;  /* source timeslot */
    int dst_str; /* destination stream number */
    int dst_ts;  /* destination timeslot */
    int num_ts; /* number of timeslots */
} tdmsw_xconnect_cmd_t;


extern int fpga_spi_indirect_read (uint16_t , int , uint32_t *);
extern int fpga_spi_indirect_write (uint16_t , int , uint32_t);
extern int fpga_spi_direct_read (uint16_t , int , uint32_t *);
extern int fpga_spi_direct_write (uint16_t , int , uint32_t);
extern int fpga_unreset_tdmsw (void);
extern void fpga_unreset_tdm_pll(void);
extern void fpga_config_tdm_pll (void);
extern int fpga_check_tdm_pll (void);
extern void fpga_setup (void);
extern void oak_module_tdm_init(void);
extern uchar get_oak_id(void);
extern int set_fail_over_port(void);
extern void oak_tdm_xc_setup(uchar , int);
extern int set_tdmsw_lpbk_test(int , int);
extern int tdmsw_force_byte_test(void);
extern int fpga_image_download(uint32_t);
extern void phoenix_fpga_dsp_spi_controlsw(void);

/******* History ********
$Log: diag_fpga.h,v $
Revision 1.4  2021/04/15 00:52:44  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.3  2018/08/30 06:40:20  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.2.28.3  2018/05/08 23:06:43  haohsu
Add FXS/FXO indivisual LED test

Revision 1.2.28.2  2018/02/06 09:34:05  haohsu
Code change for VG400

Revision 1.2.28.1  2018/01/26 09:42:00  haohsu
*** empty log message ***

Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.3  2017/03/30 10:25:49  harrchan
Add fpga upgrade utility

Revision 1.1.2.2  2017/01/17 05:07:05  olin2
Clean up debug code

Revision 1.1.2.1  2016/12/14 05:03:49  olin2
Initial commit code for Oakenshield



$Endlog$
*/
