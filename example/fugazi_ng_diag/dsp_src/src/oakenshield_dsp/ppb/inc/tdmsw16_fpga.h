/* $Id: tdmsw16_fpga.h,v 1.3 2021/04/15 00:52:45 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/tdmsw16_fpga.h,v $
 *----------------------------------------------------------------
 * tdmsw64_fpga.h
 * 
 * This file contains data structures related to FPGA TDM driver.
 *
 * Copyright (c) 2010-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Jim Muir May 2013 from t1e1_tdmsw64_fpga.h by Da Lin - Jan 2012
 *----------------------------------------------------------------
 */

#ifndef __TDMSW16_FPGA_H__
#define __TDMSW16_FPGA_H__

#define OAKENSHIELD_TDMSW16_MAX_STREAMS    16
#define OAKENSHIELD_INVALID_STREAM         0xFFFF

#define OAKENSHIELD_TDMSW16_MAX_CHANNELS_2M   32
#define OAKENSHIELD_TDMSW16_MAX_CHANNELS_8M   128
#define OAKENSHIELD_TDMSW16_MAX_CHANNELS_16M  256

#define OAKENSHIELD_TDMSW16_CONN_MEM_WORD_SIZE  \
       (OAKENSHIELD_TDMSW16_MAX_STREAMS * OAKENSHIELD_TDMSW16_MAX_CHANNELS_8M)

#define TDMSW16_MB_CODEC_STREAM 0
#define TDMSW16_DC_CODEC_STREAM 2
#define TDMSW16_DSP0_STREAM     4
#define TDMSW16_DSP1_STREAM     8
#define TDMSW16_DS0_DUMP_STREAM 13

#define TDMSW_ACCESS_TIMEOUT 30000

/* stream rate register */
#define TDMSW16_STREAMS_PER_STREAM_RATE_REG  16
#define TDMSW16_STREAM_RATE_BITS_PER_STREAM  2
#define TDMSW16_STREAM_RATE_2MBPS            0x0
#define TDMSW16_STREAM_RATE_8MBPS            0x1
#define TDMSW16_STREAM_RATE_16MBPS           0x2
#define TDMSW16_STREAM_RATE_32MBPS           0x3
#define TDMSW16_STREAM_RATE_MASK             0x3

/* enable register */
#define TDMSW16_STREAMS_PER_ENABLE_REG  32

/* loopback register */
#define TDMSW16_STREAMS_PER_LPBK_REG  32

/* Connection Memory */
#define CM_PASSWORD         0xCAC00000
#define CM_FORCEBYTE        0x00010000
#define CM_ODRV             0x00008000
#define CM_FORCELSB         0x00004000
#define CM_CID_MASK         0x00003FFF
#define CM_CID_STR_MASK     0x00003F80
#define CM_CID_STR_SHFT     7
#define CM_CID_TS_MASK      0x0000007F
#define MAX_COMBINE_STRS    4  /* 4 streams combine to yield 32M stream */

/* TDMSW16_CTL register */
#define TDMSW_RST  0x10    /* 1 - held in reset, 0 - out of reset */

/*
 * TDM Stream Data Rate Enumerations
 */
typedef enum {
    TDMSW16_0MBPS,  
    TDMSW16_2MBPS,
    TDMSW16_8MBPS,
    TDMSW16_16MBPS,
    TDMSW16_32MBPS
} oak_tdmsw16_str_rate_e;

/*
 * TDM stream Enable/Disable bit field
 */
typedef enum {
    TDMSW16_DISABLE,  /* 0 - Disable */
    TDMSW16_ENABLE    /* 1 - Enable */
} oak_tdmsw16_on_off_e;

/*
 * Loopback Enable/Disable 
 */
typedef enum {
    TDMSW16_LOOP_DISABLE,  /* 0 - Disable */
    TDMSW16_LOOP_ENABLE    /* 1 - Enable */
} oak_tdmsw16_loopback_e;

/*
 * TDM Timeslot Data Select Mode Enumerations
 */
typedef enum {
    TDMSW16_HI_Z,     /* 0 - high-impedance mode */
    TDMSW16_NORMAL,   /* 1 - Normal Time Slot Data Mode */
    TDMSW16_RESERVE,  /* 2 - Reserved */
    TDMSW16_HDSM      /* 3 - host data substitution mode */ 
} tdmsw16_data_mode_e;

/*
 * Oakenshield specific TDM Port Type Enumerations
 */
typedef enum {
    TDMSW16_PORT_NC,          /* not connected */
    TDMSW16_PORT_CODEC,       /* To Si3241 codec */
    TDMSW16_PORT_CPU,         /* To CPU mcBSP */
    TDMSW16_PORT_DS0_DUMP,    /* used for ds0-dump */  
    TDMSW16_PORT_COMBINED     /* streams that are combined to form 
                               * 16M or 32M streams */
} oak_tdmsw16_porttype_e;

/*
 * TDM stream information.
 */
typedef struct tdmsw_stream_info_ {
    oak_tdmsw16_porttype_e porttype;
    oak_tdmsw16_str_rate_e rate;
    uint16_t max_timeslots;
    uint16_t max_num_ports;
} tdmsw_stream_info_t;

/* TDM System data structure */
typedef struct oak_tdm_info_ {
    void *tdmsw_base_addr;      /* base address of the TDMSW16 registers */
    uint16_t conn_msg_seqno;    /* keep track of connection request msg */
    uint32_t out_of_seq_cnt;
    uint32_t conn_conn_cnt;
    uint32_t dis_dis_conn_cnt;
    uint16_t max_streams;
    uint16_t ds0_dump_stream_num; /* 5 */
    tdmsw_stream_info_t streams[OAKENSHIELD_TDMSW16_MAX_STREAMS];
} oak_tdm_info_t;

/* ds0-dump information */
#define MAX_DEST_LENGTH                 335
#define TDM_SAMPLES_PER_SECOND          8000 /* 8kHz */
#define DS0_DUMP_BASE_FN_MAX            MAX_DEST_LENGTH
#define DS0_DUMP_FN_POSTFIX_MAX         13  /* timestamp + rx or tx */
#define DS0_DUMP_BUFF_SIZE              (4096 * 4)
#define DS0_DUMP_MAX_CMD_LEN            1000


/* TDMSW register offsets.
 * NOTE these offset must match the fpga_regs_t struct defines  
 */
#define TDMSW64_ENBL_63_32   0x8000
#define TDMSW64_ENBL_31_00   0x8004
#define TDMSW64_RATE_63_48   0x8010
#define TDMSW64_RATE_47_32   0x8014
#define TDMSW64_RATE_31_16   0x8018
#define TDMSW64_RATE_15_00   0x801C
#define TDMSW64_LPBK_63_32   0x8020
#define TDMSW64_LPBK_31_00   0x8024
#define TDMSW64_CTL          0x802C
#define NGVMTDM_CTL          0x8030

#define TDMSW64_MEM_END      0x8FFC
#define MB_CTRL              0x8100

#define MB_CTRL_IMG_TYPE     0x00000004   /* active one is upgrade FPGA when bit set */

typedef struct tdm_ds0_dump_info_ {
    boolean active;
    uint32_t total_samples; /* total samples to capture per direction */
    uint16_t rx_stream;    /* rx, from PSTN */
    uint16_t rx_timeslot;
    uint16_t tx_stream;    /* tx, to PSTN */
    uint16_t tx_timeslot;
    uint32_t samples_tx;  /* number of capture tx samples */
    uint32_t samples_rx;
    char rx_fn[DS0_DUMP_BASE_FN_MAX + DS0_DUMP_FN_POSTFIX_MAX];
    char tx_fn[DS0_DUMP_BASE_FN_MAX + DS0_DUMP_FN_POSTFIX_MAX];
    FILE *local_file_rx;
    FILE *local_file_tx;
    uint8_t *rx_buff;
    uint8_t *tx_buff;
    boolean response_pending;
} tdm_ds0_dump_info_t;

typedef enum {
    TDMSW64_SUCCESS = 0, 
    TDMSW64_WR_TIMEOUT = 1,
    TDMSW64_RD_TIMEOUT = 2,
    TDMSW64_CONNECT_CONNECT = 3,
    TDMSW64_DISCONNECT_DISCONNECT = 4,
    TDMSW64_INVALID_STREAM_NUM = 5,
    TDMSW64_INVALID_TIMESLOT = 6,
    TDMSW64_INVALID_RATE = 7,
    TDMSW64_INVALID_MODE = 8,
    TDMSW64_VERIFICATION_FAILED = 9
} tdm_status_e;



void test_tdmsw_xconnect (tdmsw_xconnect_cmd_t *);
//extern int tdm_rd(uint16_t addr, int size, uint32_t*);
extern int fpga_spi_indirect_read (uint16_t , int , uint32_t *);
//extern int tdm_wr(uint16_t addr, int size, uint32_t);
extern int fpga_spi_indirect_write (uint16_t , int , uint32_t);
extern void oak_diag_codec_reset(int reset);
extern uchar sku_id;


#endif // __TDMSW16_FPGA_H__

