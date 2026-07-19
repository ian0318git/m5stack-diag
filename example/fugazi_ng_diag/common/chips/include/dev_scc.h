/* $Id: dev_scc.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/dev_scc.h,v $
 *------------------------------------------------------------------
 *
 * dev_scc.h : Common Device Driver header of goofy SCC module.
 *
 * June 2006 - Alan Hsu
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DEV_SCC_H__
#define __DEV_SCC_H__

/* structure for scc protocol regs */
typedef struct scc_proto_regs_t_ {
    volatile ushort mode_reg_hi;              /*0x0000*/
    volatile ushort mode_reg_lo;              /*0x0002*/
    volatile ushort flag_config;              /*0x0004*/
    volatile ushort pad_config;               /*0x0006*/
    volatile ushort flow_cntrl_config;        /*0x0008*/
    volatile ushort intr_status;              /*0x000A*/
    volatile ushort intr_mask;                /*0x000C*/
    volatile ushort command_reg;              /*0x000E*/
} scc_proto_regs_t;

/* structure for scc dma regs */
typedef struct scc_dma_regs_t_ {
    volatile uint ring_start;                 /*0x0000*/
    volatile uint ring_size_mask_index;       /*0x0004*/
    volatile uint reserved[0x6];              /*0x0008-001f*/
} scc_dma_regs_t;

/* structure for scc ppp regs */
typedef struct scc_ppp_regs_t_ {
    volatile uint cntrl_char_map;            /*0x0000*/
    volatile uint tx_special;                /*0x0004*/
    volatile uint unused[1];                 /*0x0008*/
    volatile uint context;                   /*0x000c*/
} scc_ppp_regs_t;

/* structure for scc BCC regs */
typedef struct scc_bcc_regs_t_ {
    volatile uint context;                   /*0x0000*/
    volatile uint unused[3];                 /*0x0004-0x000f*/
} scc_bcc_regs_t;

/* structure for scc serial interface regs */
typedef struct scc_serial_itf_t_ {
    volatile ushort serial_itf_cntrl;        /*0x0000*/
    volatile ushort modem_cntrl;             /*0x0002*/
    volatile ushort flow_cntrl;              /*0x0004*/
    volatile ushort brg_divider;             /*0x0006*/
    volatile ushort modem_intr_status;       /*0x0008*/
    volatile ushort unused1;                 /*0x000A*/
    volatile ushort unused2;                 /*0x000C*/
    volatile ushort unused3;                 /*0x000E*/
} scc_serial_itf_t;

/* structure for scc control_1 regs  0x1000-0x10ff */
typedef struct scc_cntrl_1_regs_t_ {
    volatile ushort creation_mm_dd;               /*0x00*/
    volatile ushort creation_yy;                  /*0x02*/
    volatile ushort goofy_scc_cntrl;              /*0x04*/
    uchar pad1[0xa];                              /*0x06-0x0F*/
    volatile ushort int_1_intr_status;            /*0x10*/
    volatile ushort int_2_intr_status;            /*0x12*/
    volatile ushort timer_1_8_intr_status;        /*0x14*/
    volatile ushort timer_17_20_intr_status;      /*0x16*/
    uchar pad2[8];                                /*0x18-0x1E*/
    volatile ushort int_1_intr_mask;              /*0x20*/
    volatile ushort int_2_intr_mask;              /*0x22*/
    volatile ushort timer_1_8_intr_mask;          /*0x24*/
    volatile ushort timer_17_20_intr_mask;        /*0x26*/
    uchar pad3[8];                                /*0x28-0x2E*/
    volatile ushort timer_1_2_pgm;                /*0x30*/
    volatile ushort timer_3_4_pgm;                /*0x32*/
    volatile ushort timer_5_6_pgm;                /*0x34*/
    volatile ushort timer_7_8_pgm;                /*0x36*/
    uchar pad4[8];                                /*0x38-0x3f*/
    volatile ushort timer_17_pgm;                 /*0x40*/
    volatile ushort timer_18_pgm;                 /*0x42*/
    volatile ushort timer_19_pgm;                 /*0x44*/
    volatile ushort timer_20_pgm;                 /*0x46*/
    uchar pad5[0xb8];                             /*0x48-0xFF*/
} scc_cntrl_1_regs_t;

/* structure for scc control_2 regs, 0x3100-0x31ff  */
typedef struct scc_cntrl_2_regs_t_ {
    volatile ushort tdm_cntrl;                    /*0x00*/
    volatile ushort tdma_8k_divider;              /*0x02*/
    volatile ushort tdmb_8k_divider;              /*0x04*/
    volatile ushort serial_intr_status;           /*0x06*/
    volatile ushort unused7;                      /*0x08*/
    volatile ushort freq_cnt_port_select;         /*0x0A*/
    volatile ushort freq_cnt_hi;                  /*0x0C*/
    volatile ushort freq_cnt_lo;                  /*0x0E*/
    volatile ushort pll1_cntrl;                   /*0x10*/
    volatile ushort pll1_8k_divider;              /*0x12*/
    volatile ushort pll1_pre_divider;             /*0x14*/
    volatile ushort pll1_post_divider;            /*0x16*/
    volatile ushort unused8[4];                   /*0x18-0x1E*/
    volatile ushort pll2_cntrl;                   /*0x20*/
    volatile ushort pll2_8k_divider;              /*0x22*/
    volatile ushort pll2_pre_divider;             /*0x24*/
    volatile ushort pll2_post_divider;            /*0x26*/
    volatile ushort unused9[4];                   /*0x28-0x2E*/
    volatile ushort unused10[0x68];               /*0x30-0xFF*/
} scc_cntrl_2_regs_t;

/* structure for scc memory map */
typedef struct scc_regs_t_ {
    uint32_t pad1;                               /* 0x0000*/
    volatile uint32_t sti_err_int_status_reg;    /* 0x0004 */
    volatile uint32_t sti_err_int_enable_reg;    /* 0x0008 */
    uchar    pad2[0xFF4];                        /* 0x000C-0x0FFF */

    /* hwic scc control_1 regs 0x1000 */
    scc_cntrl_1_regs_t cntrl_1_regs;     /* 0x1000-0x10FF */

    /* hwic scc protocol regs 0x1100*/
    scc_proto_regs_t proto_regs[8];      /* 0x1100-0x117F */
    uchar pad3[0xE7F];                   /* 0x1180-0x1FFF */

    /* hwic scc dma regs  0x2000 */
    scc_dma_regs_t    dma_rx_regs[8];    /* 0x2000-0x20FF */
    uchar pad4[0x100]; 
    scc_dma_regs_t    dma_tx_regs[8];    /* 0x2200-0x22FF */
    uchar pad5[0x100]; 

    /* hwic scc protocol extra  0x2400*/
    scc_bcc_regs_t   bcc_regs[8];        /* 0x2400-0x247F */
    uchar pad6[0x180];                   /* 0x2480-0x25FF */
    scc_ppp_regs_t   ppp_tx_regs[8];     /* 0x2600-0x267F */
    uchar pad7[0x180];                   /* 0x2680-0x27FF */
    scc_ppp_regs_t   ppp_rx_regs[8];     /* 0x2800-0x287F */
    uchar pad8[0x180];                   /* 0x2880-0x29FF */
    scc_bcc_regs_t   bcc_regs_rx[8];     /* 0x2a00-0x2a7F */
    uchar pad9[0x580];                   /* 0x2a80-0x2FFF */

    /* goofy scc serial interface regs 0x3000 */
    scc_serial_itf_t serial_itf[8];      /* 0x3000-0x307F */
    uchar pada[0x80];                    /* 0x3080-0x30ff */
    scc_cntrl_2_regs_t cntrl_2_regs;     /* 0x3100-0x31FF */
} scc_regs_t;

/*
 *  DMA Buffer Descriptors
 */
typedef struct scc_dma_bd_t_ {
    volatile ushort command_status;
    volatile ushort byte_count;
    volatile uint32 buff_ptr;
} scc_dma_bd_t;

/*
 *  Serial data buffer management structure.
 *  User provides buffers.
 */
typedef struct scc_serial_buf_m_t_{
    ulong	tx_buf_addr;	/* User supplied Tx buffer pointer  */
    ulong	rx_buf_addr;	/* User supplied Rx buffer pointer  */
    uint	duration;	/* Wait time after Tx		    */
    ushort	tx_num_buff;	/* Number of user provided tx buffs */
    ushort	rx_num_buff;	/* Number of user provided rx buffs */
    ushort	tx_buf_size;	/* Number of bytes in each tx buf   */
    ushort	rx_buf_size;	/* Number of bytes in each rx buf   */
    ushort	rx_min_buf;	/* Minimum of rx buf expect to rx   */
} scc_serial_buf_m_t;

/*
 *  Serial Data Structure.
 */
typedef struct scc_serial_ds_t_ {
    scc_proto_regs_t   *proto_regs_addr;
    scc_dma_regs_t     *dma_tx_regs_addr;
    scc_dma_regs_t     *dma_rx_regs_addr;
    scc_ppp_regs_t     *ppp_tx_regs_addr;
    scc_ppp_regs_t     *ppp_rx_regs_addr;
    scc_bcc_regs_t     *bcc_tx_regs_addr;
    scc_bcc_regs_t     *bcc_rx_regs_addr;
    scc_serial_itf_t   *serial_itf_addr;
    scc_cntrl_1_regs_t *cntrl_1_regs_addr;
    scc_cntrl_2_regs_t *cntrl_2_regs_addr;
    ushort   port_num;               /* port number                 */
    ushort   org_port_num;           /* original port number        */
    ushort   brg_num;                /* BRG Rate Number             */
    uint     divider;                /* divider                     */
    uchar    clk_src;                /* Brg clock source            */
    ushort   clk_src_bits;
    uchar    speed_idx;              /* speed index                 */
    uchar    lpbk_mode;              /* Loopback Mode               */
    uchar    protocol;               /* Protocol                    */
    ulong    ctrl_id;                /* Device vendor-id            */
    ushort   num_buff;               /* Number of buffers           */
    ushort   buff_size;              /* Size of Buffers             */
    ulong    last_rx_desc_addr;      /* Address of last Rx desc     */
    ulong    tx_desc_addr;           /* Address of Tx Ring          */
    ulong    rx_desc_addr;           /* Address of Rx Ring          */
    ulong    tx_buf_addr;            /* Address of Tx Buffer        */
    ulong    rx_buf_addr;            /* Address of Rx Buffer        */
    uint     baudrate;               /* baud rate                   */
    uint     des_baud;  
    uchar    crc_rx_buf_opt;         /* need 2 crc bytes in rx buf  */
    uchar    crc_enb;                /* crc default enb             */
    uchar    run_mode;               /* run mode                    */
    ushort   scc_type;
    dev_object_t *dev;
    scc_serial_buf_m_t	*scc_mgmt;   /* Pointer to buffer manager   */
} scc_serial_ds_t;

typedef struct scc_baud_t_ {
    uint baudrate_code;         /* one of the sync/async enums */
    uint desired_baud;
    uint baudrate;              /* the actual baudrate */
    uint divider;               /* clock divider */
    ushort clk_src;             /* clock source */
} scc_baud_t;

typedef struct scc_pll_t_ {
    uint baudrate;              /* the actual baudrate */
    uint pre_divider;           /* clock pre-divider */
    uint post_divider;          /* clock post-divider */
} scc_pll_t;

/* Descriptor defines. */
#define DMA_CMD_STAT_OWNER                            0x8000
#define DMA_CMD_STAT_RX_BUF_INT                       0x0800
#define DMA_CMD_STAT_TX_BUF_INT                       0x0800
#define DMA_CMD_STAT_RX_FRAME_INT                     0x0400
#define DMA_CMD_STAT_TX_FRAME_INT                     0x0400
#define DMA_EARLY_TRANSMIT                            0x0100
#define DMA_CMD_STAT_SEQ                              0x00C0
#define DMA_CMD_STAT_SEQ_MIDDLE                       0x0000
#define DMA_CMD_STAT_SEQ_START                        0x0040
#define DMA_CMD_STAT_SEQ_END                          0x0080
#define DMA_CMD_STAT_SEQ_COMPLETE                     0x00C0
#define DMA_CMD_STAT_RX_OVERRUN                       0x0008
#define DMA_CMD_STAT_TX_UNDERRUN                      0x0008
#define DMA_CMD_STAT_RX_RESIDUAL                      0x0004
#define DMA_CMD_STAT_RX_ABORT                         0x0002
#define DMA_CMD_STAT_RX_CRC                           0x0001
#define DMA_CMD_STAT_ERR_MASK                         0x000F
#define DMA_CMD_STAT_ERR_MASK_BISYNC                  0x000E

/* buffer num and size defines */
#define MAX_SCC_BUFFERS 	    2 /* can only be 2's power */
#define MAX_SCC_BUFFER_SIZE  	    0x80
#define MAX_SCC_SMALL_BUFFER_SIZE   0x10
#define MAX_SCC_LARGE_BUFFER_SIZE   0x100
#define SCC_CRC_RESERVED            0x10
#define SCC_DATA_PATTERN_55         0x55
#define SCC_INC_PAT                 0x01
#define SCC_DEC_PAT                 0x02
#define SCC_DATA_PAT                0x03
#define SCC_STX                     0x02
#define SCC_ETX                     0x03

/* enum for sync baud rate generator */

enum {
    SCC_SYNC_BPS_NONE = 0,
    SCC_SYNC_BPS_300,
    SCC_SYNC_BPS_600,
    SCC_SYNC_BPS_1200,
    SCC_SYNC_BPS_2400,
    SCC_SYNC_BPS_4800,
    SCC_SYNC_BPS_9600,
    SCC_SYNC_BPS_14400,
    SCC_SYNC_BPS_19200,
    SCC_SYNC_BPS_28800,
    SCC_SYNC_BPS_32K,
    SCC_SYNC_BPS_38400,
    SCC_SYNC_BPS_56K,
    SCC_SYNC_BPS_57600,
    SCC_SYNC_BPS_64K,
    SCC_SYNC_BPS_72K,
    SCC_SYNC_BPS_115200,
    SCC_SYNC_BPS_128K,
    SCC_SYNC_BPS_230400,
    SCC_SYNC_BPS_256K,
    SCC_SYNC_BPS_512K,
    SCC_SYNC_BPS_1024K,
    SCC_SYNC_BPS_2016K,
    SCC_SYNC_BPS_2048K,
    SCC_SYNC_BPS_4032K,
    SCC_SYNC_BPS_8064K,
    SCC_SYNC_BPS_10M,        /* 26 */
};

/* enum for async baud rate generator */

enum {
    SCC_AS_BPS_NONE = 0,
    SCC_AS_BPS_300,
    SCC_AS_BPS_600,
    SCC_AS_BPS_1200,
    SCC_AS_BPS_2400,
    SCC_AS_BPS_4800,
    SCC_AS_BPS_9600,
    SCC_AS_BPS_14400,
    SCC_AS_BPS_19200,
    SCC_AS_BPS_28800,
    SCC_AS_BPS_32K,
    SCC_AS_BPS_38400,
    SCC_AS_BPS_56K,
    SCC_AS_BPS_57600,
    SCC_AS_BPS_64K,
    SCC_AS_BPS_72K,
    SCC_AS_BPS_115200,
    SCC_AS_BPS_128K,
    SCC_AS_BPS_230400,
    SCC_AS_BPS_256000      /* 19 */
};

/* Defines for PLL */
#define  SCC_PLL_TABLE_MAX       32      /* used by 4T only */
#define  SCC_PLL_AS_TABLE_MAX    2       /* used by 8AS and 4AS */ 
#define  SCC_PLL_SYNC_MAX        8       
                         /* freq counter support 8 sync channels */
#define  SCC_PLL_TABLE_START        15 
#define  SCC_PLL_TABLE_INC          16 
#define  SCC_PLL_BAUD_DIV           24   /* used by 4AS and 8AS */

/* Defines for cable id */
#define CABLE_RS232AS    0x6 
#define CABLE_RS232A     0x7

/* Defines for freq counter register */
#define SCC_FREQ_SELECT_MASK       0x0007
#define SCC_FREQ_COUNT_WAIT        1100 
#define SCC_FREQ_MICRO_WAIT         100
#define SCC_FREQ_MAX_PASS            15

/* Defines for DMA waiting */
#define SCC_MAX_DMA_PASS         400
#define SCC_DMA_RETRY_PASS        66 
#define SCC_HIGH_SPEED_WAIT      1
#define SCC_LOW_SPEED_WAIT       6 

/*
 * Defines used to support the Simpsons HWIC Loopback Modes.
 */
#define  SCC_INTERNAL_LOOPBACK       0x01
#define  SCC_EXTERNAL_LOOPBACK       0x02
#define  SCC_EXTERNAL_LOOPBACK_NRZI  0x03

/*
 * Defines used to support the Simpsons net interrupt test
 */
#define  SCC_RX_BUF_TYPE       0x01
#define  SCC_TX_BUF_TYPE       0x02
#define  SCC_RX_FRAME_TYPE     0x03
#define  SCC_TX_FRAME_TYPE     0x04

/*
 * Defines used to support the Simpsons HWIC Run Modes.
 */
#define  SCC_POLL_MODE      0x01
#define  SCC_INT_MODE       0x02
#define  SCC_FRAME_INT_MODE 0x03
#define  SCC_NO_CHECK_MODE  0x04

/*
 * Defines used to support the Simpsons HWIC clock source.
 */
#define  SCC_CLK_SRC_SYNC     0x01
#define  SCC_CLK_SRC_ASYNC    0x02
#define  SCC_CLK_SRC_PLL1_64K 0x03
#define  SCC_CLK_SRC_PLL1_56K 0x04
#define  SCC_CLK_SRC_PLL2_64K 0x05
#define  SCC_CLK_SRC_PLL2_56K 0x06
#define  SCC_CLK_SRC_PLL_AS   0x07      /* 8AS and 4AS */

/*
 * Defines used to support the Simpsons HWIC Loopback Protocol.
 */
#define  SCC_HDLC             0x01
#define  SCC_BISYNC           0x02
#define  SCC_UART_ASYNC       0x03
#define  SCC_UART_PPP         0x04
#define  SCC_TRANSPARENT      0x05

/*
 * Defines used to support the channel# mode register low
 */
#define  SCC_MODE_DISABLE          0x0000
#define  SCC_MODE_HDLC             0x0001
#define  SCC_MODE_BISYNC           0x0002
#define  SCC_MODE_TRANSPARENT      0x0003
#define  SCC_MODE_UART_ASYNC       0x0004
#define  SCC_MODE_UART_PPP         0x0005
#define  SCC_MODE_MASK             0x000F 
#define  SCC_MODE_CRC              0x0030
#define  SCC_MODE_CRC_16           0x8000
#define  SCC_MODE_EBCDIC           0x4000 
#define  SCC_MODE_IDLE_MARK        0x3000 

/*
 * Defines used to support the channel# mode register high 
 */
#define  SCC_MODE_NRZI             0x0080
#define  SCC_MODE_NRZ_RSV          0x0100
#define  SCC_MODE_NRZ_MASK         0x0180
#define  SCC_MODE_FCS_LSB          0x0200
#define  SCC_MODE_FCS_MASK         0x0600
#define  SCC_MODE_FLAG_CHK_MASK    0x1800
#define  SCC_MODE_FLAG1_CHK        0x0800      /* HDLC   */
#define  SCC_MODE_FLAG_BOTH_CHK    0x1800      /* bisync */

/*
 * Defines used to support the channel# command register. .
 */
#define  SCC_CMD_START_TX_RX       0x8050
#define  SCC_CMD_STOP_TX_RX        0x0028
#define  SCC_CMD_RESET_TX_RX       0x0003

/*
 * Defines used to support the Modem control register (per chan).
 */
#define  SCC_MODEM_BRG_MASK        0x00e0
#define  SCC_MODEM_BRG_2M          0x0000     /* 2.016MHZ */
#define  SCC_MODEM_BRG_252K        0x0020
#define  SCC_MODEM_BRG_10M         0x00c0
#define  SCC_MODEM_BRG_32M         0x0040     /* 32.256MHZ */
#define  SCC_MODEM_BRG_1M          0x0060     /* 1.5625MHZ */
#define  SCC_MODEM_BRG_PLL1        0x0080
#define  SCC_MODEM_BRG_PLL2        0x00a0
#define  SCC_MODEM_LOOPBACK        0x0002 

/*
 * Defines used to support the Flow control register (per chan).
 */
#define SCC_DCD_ENABLE           0x8000
#define SCC_DSR_ENABLE           0x4000
#define SCC_CTS_ENABLE           0x2000
#define SCC_CABLEID_ENABLE       0x1000
#define SCC_DCD_INPUT_IGNORE     0x0400

/*
 * Defines used to support the modem interrupt status register (per chan).
 */
#define  SCC_DCD_FORCE_IRQ       0x0080
#define  SCC_DSR_FORCE_IRQ       0x0040 
#define  SCC_CTS_FORCE_IRQ       0x0020 
#define  SCC_CABLEID_FORCE_IRQ   0x0010  
#define  SCC_DCD_STATUS          0x0008  
#define  SCC_DSR_STATUS          0x0004  
#define  SCC_CTS_STATUS          0x0002  
#define  SCC_CABLEID_STATUS      0x0001  

/*
 * Defines used to support the serial interface control register (per chan).
 */
#define  SCC_CABLE_ID_MASK         0x001e
#define  SCC_CABLE_ID_MODE_BIT     0x0001
#define  SCC_RESET_ITF_CKT         0x0200
#define  SCC_INVERT_RXC            0x0400

/*
 * Defines used to support the protocol int enable register (per chan).
 */
#define  SCC_DMA_RX_BUF_ENB        0x0001
#define  SCC_DMA_RX_FRAME_ENB      0x0002
#define  SCC_DMA_TX_BUF_ENB        0x0004
#define  SCC_DMA_TX_FRAME_ENB      0x0008

/*
 * Defines used to support the protocol int status register (per chan).
 */
#define  SCC_DMA_RX_BUF_STATUS     0x0001
#define  SCC_DMA_RX_FRAME_STATUS   0x0002
#define  SCC_DMA_TX_BUF_STATUS     0x0004
#define  SCC_DMA_TX_FRAME_STATUS   0x0008

/*
 * Defines used to support the int1 enable register 
 */
#define  SCC_MODEM_ENB         0x8
#define  SCC_FREQ_ENB          0x10
#define  SCC_DMA_ENB           0x20
#define  SCC_TIMER_17_20_ENB   0x2
#define  SCC_TIMER_1_16_ENB    0x1

/*
 * Defines used to support the int1 status register 
 */
#define  SCC_MODEM_STATUS      0x8
#define  SCC_FREQ_STATUS       0x10
#define  SCC_DMA_STATUS        0x20

/*
 * Defines used to support PLL1 and PLL2 control register.
 */
#define  SCC_PLL_SELECT_MASK        0x000f
#define  SCC_PLL_SELECT_OSC         0x0008
#define  SCC_PLL_ENABLE             0x0020
#define  SCC_PLL_8K_REF_ENABLE      0x0010
#define  SCC_PLL_8K_REF_STATUS      0x0040

/*
 * Defines used to support TDM control register.
 */
#define  SCC_TDMA_SELECT_MASK        0x00f0
#define  SCC_TDMB_SELECT_MASK        0x000f
#define  SCC_TDMA_SELECT_OSC         0x0080
#define  SCC_TDMB_SELECT_OSC         0x0008
#define  SCC_TDMA_8KHZ_ENABLE        0x0200
#define  SCC_TDMB_8KHZ_ENABLE        0x0100
#define  SCC_TDMA_8KHZ_REF_STATUS    0x8000
#define  SCC_TDMB_8KHZ_REF_STATUS    0x4000

#define  SCC_8KHZ_REF_WAIT           100 
#define  SCC_8KHZ_REF_WAIT_ONE       100

/* max counter for IRQ occurrence */
#define  SCC_IRQ_COUNT_MAX   10

/* Defines used to test BCC register */
#define  SCC_BCC_CHAN_SIZE           0x10
#define  SCC_BCC_CHAN_NUM            8 
#define  SCC_MAX_CHAN_NUM            16 

/* define options to display register */
#define  SCC_DMA_RX       1
#define  SCC_DMA_TX       2
#define  SCC_PPP_RX       1
#define  SCC_PPP_TX       2
#define  SCC_REG_ALL      1
#define  SCC_REG_ONE      2
#define  SCC_REG_COMMON   1
#define  SCC_REG_DMA      2
#define  SCC_REG_PROTOCOL 3
#define  SCC_REG_SERIAL   4

/*
 * Defines used to support Goofy SCC control register.
 */
#define  SCC_LED1_MASK               0x0003
#define  SCC_LED1_YELLOW             0x0002
#define  SCC_LED1_GREEN              0x0001
#define  SCC_LED2_MASK               0x0018
#define  SCC_LED2_YELLOW             0x0010
#define  SCC_LED2_GREEN              0x0008
#define  SCC_FAST_MODE               0x8000

/*
 * Defines used to support low speed index (<14400).
 */
#define  SCC_LOW_SPEED_INDEX              7 
#define  SCC_SPEED_INDEX_INC              1 

/*
 * Defines used to support ppp CRC rx buffer option.
 */
#define  CRC_RX_BUF_OPT_ON              1 

/*
 * Defines used to support async timer roll over value.
 * Count * one bit clock.  Program to max value.
 */
#define  SCC_ASYNC_TIMER_COUNT     0x7f 

/*
 * Defines used to support PLL choice.
 */
#define  SCC_MAIN_MENU              1 
#define  SCC_SUB_MENU               2 
#define  SCC_PLL1                   1 
#define  SCC_PLL2                   2 

/*
 * Defines used to support frequency margin.
 */
#define  SCC_MARGIN_100              100 
#define  SCC_MARGIN_95               95 
#define  SCC_MARGIN_90               90 
#define  SCC_MARGIN_105              105 
#define  SCC_MARGIN_110              110 
#define  SCC_RETRY                   2
#define  SCC_RETRY_COUNT             2     /* turn PLL retry on */

#define NUM_HWIC_CH 16
#define CASE_ONE    1	/* special case for test items */
#define CASE_TWO    2	/* special case for test items */
#define DOWNLOAD_DONE  1

/* hwic_simp_download.c */
#define SCC_INACTIVE_MODE               0x01
#define SCC_FPGA_MODE                   0x02
#define SCC_EEPROM_MODE                 0x03

#ifdef AHSU
/*
 * register test table
 * rd_mask masks off all but valid register read bits.
 * wr_mask holds write data used for register uniqueness test.
 * rw_mask holds r/w mask for r/w registers but for ro registers
 * rw_mask holds mask used when checking reset_val (allows us to
 * mask off changable bits, .e.g., CP_L bit in status register
 * is dependent on whether a hwic is inserted or not).
 * NOTE:
 * register values used for the interrupt enable/interrupt event
 * registers during the register uniqueness test (in wr_mask)
 * are chosen so that no interrupts are generated.
 */
typedef struct goofy_scc_test_regs_t_ {
    char           *name;
    uint           offset;
    uchar          type;
    uint           size;
    ulong          mask;
    ulong          reset_val;
} goofy_scc_test_regs_t;
#endif

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 * device callin function - service provided and defined by the device
 */
typedef struct goofy_scc_callin_fvt_t {
  type_t     (*register_test)(dev_object_t *);
  void    (*init_chan)(dev_object_t *);
  type_t     (*loopback)(dev_object_t *);
  type_t     (*init_tx_dma_ring)(dev_object_t *);
  type_t     (*init_rx_dma_ring)(dev_object_t *);
  type_t     (*start_tx)(dev_object_t *);
  void    (*scc_init_tx_dma_buffers)(dev_object_t *dev, uchar data, uchar pattern);
  void    (*scc_init_rx_dma_buffers)(dev_object_t *dev);
  type_t     (*check_rx_data)(dev_object_t *);
  type_t     (*reset_chan)(dev_object_t *, int);
  void    (*reset_scc_module)(dev_object_t *, int );
  void    (*set_baud_rate_divider)(dev_object_t *);
  void    (*set_protocol_mode)(dev_object_t *);
/*
  int     (*enable_scc_int)(dev_object_t *);
  int     (*disable_scc_int)(dev_object_t *);
  int     (*reset_scc)(dev_object_t *);
  int     (*enable_lpbk)(dev_object_t *);
  int     (*disable_lpbk)(dev_object_t *);
*/

}goofy_scc_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct goofy_scc_callout_fvt_t_ {
  /*
   * Vectors set by the upper level (eg., platform).
   */
   void   (*reset)(dev_object_t *, goofy_dev_t, int, boolean);
   type_t    (*set_intr)(dev_object_t *, goofy_intr_class_t,
                      uint, uint, uint, boolean);
   type_t    (*install_intr_vect)(dev_object_t *, goofy_intr_class_t,
                               uint, uint, PFT);
   void*  (*get_plat_goofy_inst)(uint32_t, uint32_t);
   type_t  (*make_phy_addr)(ulong address);
}goofy_scc_callout_fvt_t;

/*
** Define the dev goofy device object structure.
*/
typedef struct goofy_scc_object_t_ {
    dev_object_t             base;
    goofy_scc_callin_fvt_t   *callin_fvt;
    goofy_scc_callout_fvt_t  *callout_fvt;
    scc_serial_ds_t          *s_ds;
    int                      channel;
    ushort                   scc_type;
    ushort                   run_mode;
    ushort                   protocol;
    ushort                   lpbk_mode;
    ushort                   clk_src;
    ushort                   index;
}goofy_scc_object_t;

extern scc_serial_ds_t scc_serial_ds[];
extern scc_baud_t scc_sync_baud[];
extern int scc_sync_baud_size;
extern scc_baud_t scc_async_baud[];
extern scc_pll_t scc_pll_56k[];
/*
extern goofy_scc_test_regs_t goofy_scc_test_table[];
*/
extern void goofy_scc_create(dev_object_t *dev, 
                             dev_error_report_t error_report_fn);
extern void goofy_scc_dev_destroy(dev_object_t **dev);
extern void goofy_scc_cleanup (dev_object_t *dev);

#endif /* __DEV_SCC_H__ */


/******** History ******** 
$Log: dev_scc.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
