/* $Id: nanook_crocus_def.h,v 1.2 2019/12/11 10:10:33 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/nanook_crocus_def.h,v $
 *------------------------------------------------------------------
 *
 * nanook_crocus_def.h - Crocus definitions and prototypes.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __NANOOK_CROCUS_DEFS__
#define __NANOOK_CROCUS_DEFS__

#include "crocus_reg.h"

#define XDMA

#define NANOOK_ASYNC_MAX_CH_NUM        32

/*
 * Defines used to support the Loopback Protocol.
 */
#define CROCUS_ASYNC_UART              0x4
#define CROCUS_ASYNC_PPP               0x5

/*
 * Defines used to support the SCC Loopback Modes.
 */
#define CROCUS_ASYNC_SCC_INT_LOOPBACK  0x01
#define CROCUS_ASYNC_SCC_EXT_LOOPBACK  0x02

/*
 * Defines used to support the Modem control register (per chan).
 */
#define CROCUS_MODEM_BRG_MASK          0x00e0
#define CROCUS_MODEM_BRG_2M            0x0000
#define CROCUS_MODEM_BRG_252K          0x0020
#define CROCUS_MODEM_BRG_32M           0x0040
#define CROCUS_MODEM_BRG_1M            0x0060     /* 1.5625MHZ */
#define CROCUS_MODEM_BRG_PLL1          0x0080
#define CROCUS_MODEM_BRG_PLL2          0x00a0
#define CROCUS_MODEM_BRG_10M           0x00c0
#define CROCUS_MODEM_LOOPBACK          0x0002

/*
 * Defines used to support the Flow control register (per chan).
 */
#define CROCUS_DCD_ENABLE               0x8000
#define CROCUS_DSR_ENABLE               0x4000
#define CROCUS_CTS_ENABLE               0x2000
#define CROCUS_CABLEID_ENABLE           0x1000
#define CROCUS_DCD_INPUT_IGNORE         0x0400

/*
 * Defines used to support the modem interrupt status register (per chan).
 */
#define CROCUS_DCD_FORCE_IRQ            0x0080
#define CROCUS_DSR_FORCE_IRQ            0x0040
#define CROCUS_CTS_FORCE_IRQ            0x0020
#define CROCUS_CABLEID_FORCE_IRQ        0x0010
#define CROCUS_DCD_STATUS               0x0008
#define CROCUS_DSR_STATUS               0x0004
#define CROCUS_CTS_STATUS               0x0002
#define CROCUS_CABLEID_STATUS           0x0001

/*
 * Defines used to support low speed index (<14400).
 */
#define CROCUS_LOW_SPEED_INDEX         7
#define CROCUS_SPEED_INDEX_INC         1

/*
 * Defines used to support ppp CRC rx buffer option.
 */
#define CRC_RX_BUF_OPT_ON              1

/*
 * Defines used to support async timer roll over value.
 * Count * one bit clock.  Program to max value.
 */
#define CROCUS_ASYNC_TIMER_COUNT       0x7f

/* buffer num and size defines */
#define MAX_CROCUS_BUFFERS             0x4      /* can only be 2's power */
#define MAX_CROCUS_BUFFER_SIZE         0x80
#define MAX_CROCUS_SMALL_BUFFER_SIZE   0x10
#define MAX_CROCUS_LARGE_BUFFER_SIZE   0x100
#define CROCUS_CRC_RESERVED            0x10
#define CROCUS_DATA_PATTERN_55         0x55
#define CROCUS_INC_PAT                 0x01
#define CROCUS_DEC_PAT                 0x02
#define CROCUS_DATA_PAT                0x03
#define CROCUS_STX                     0x02
#define CROCUS_ETX                     0x03

/*
 * Defines used to support the channel# mode register low
 */
#define CROCUS_MODE_DISABLE            0x0000
#define CROCUS_MODE_UART_ASYNC         0x0004
#define CROCUS_MODE_UART_PPP           0x0005
#define CROCUS_MODE_MASK               0x000F
#define CROCUS_MODE_CRC                0x0030
#define CROCUS_MODE_CRC_16_MASK        0x18000
#define CROCUS_MODE_EBCDIC             0x4000
#define CROCUS_MODE_IDLE_MARK          0x3000
#define CROCUS_MODE_IDLE_FLAG          0x1000

/*
 * Defines used to support the channel# mode register
 */
#define CROCUS_MODE_NRZI               0x00800000
#define CROCUS_MODE_NRZ_RSV            0x01000000
#define CROCUS_MODE_NRZ_MASK           0x01800000
#define CROCUS_MODE_FCS_LSB            0x02000000
#define CROCUS_MODE_FCS_MASK           0x06000000
#define CROCUS_MODE_FLAG_CHK_MASK      0x18000000
#define CROCUS_MODE_FLAG1_CHK          0x08000000      /* HDLC   */
#define CROCUS_MODE_FLAG_BOTH_CHK      0x18000000      /* Bisync */

/*
 * Defines used to support the channel# command register. .
 */
#define CROCUS_CMD_START_TX_RX         0x8050
#define CROCUS_CMD_START_TX            0x8040
#define CROCUS_CMD_START_RX            0x0010
#define CROCUS_CMD_STOP_TX_RX          0x0028
#define CROCUS_CMD_RESET_TX_RX         0x0003

/*
 * Defines used to support the serial interface control register (per chan).
 */
#define CROCUS_CABLE_ID_MASK           0x001e
#define CROCUS_CABLE_ID_MODE_BIT       0x0001
#define CROCUS_RESET_ITF_CKT           0x0200
#define CROCUS_INVERT_RXC              0x0400

/*
 * Defines used to support the Crocus Run Modes.
 */
#define CROCUS_POLL_MODE               0x01
#define CROCUS_INT_MODE                0x02
#define CROCUS_FRAME_INT_MODE          0x03
#define CROCUS_NO_CHECK_MODE           0x04

/* define options to display register */
#define  CROCUS_PPP_RX          1
#define  CROCUS_PPP_TX          2
#define  CROCUS_REG_ALL         1
#define  CROCUS_REG_ONE         2
#define  CROCUS_REG_COMMON      1
#define  CROCUS_REG_PROTOCOL    3
#define  CROCUS_REG_SERIAL      4

/*
 * Defines used to support the ring buffer mode register (per chan)
 */
#define CROCUS_RING_BUFFER_MODE_MASK   0x0001
#define CROCUS_RING_BUFFER_PPP         0x0001
#define CROCUS_RING_BUFFER_TTY         0x0000

/*
 * Defines used to support the ring buffer interrupt enable register (per chan)
 */
#define CROCUS_RING_BUFFER_RX_INTR_ENB 0x0001
#define CROCUS_RING_BUFFER_TX_INTR_ENB 0x0002

/*
 * Defines used to support the ring buffer interrupt status register (per chan)
 */
#define CROCUS_RING_BUFFER_RX_INTR_STATUS 0x0001
#define CROCUS_RING_BUFFER_TX_INTR_STATUS 0x0002

/*
 * Defines used to support the interrupt enable register (per chan)
 */
#define CROCUS_TIMER_ENB               0x1
#define CROCUS_MODEM_ENB               0x2
#define CROCUS_FREQ_ENB                0x4
#define CROCUS_SCC_ENB                 0x8
#define CROCUS_RING_ENB                0x10

/*
 * Defines used to support the interrupt status register (per chan)
 */
#define CROCUS_TIMER_STATUS            0x1
#define CROCUS_MODEM_STATUS            0x2
#define CROCUS_FREQ_STATUS             0x4
#define CROCUS_SCC_STATUS              0x8
#define CROCUS_RING_STATUS             0x10

/*
 * Defines used to support Frequency Counter Select Register
 */
#define CROCUS_TRIGGER_LOGIC_ANALYZER  0x8000

/*
 * Defines used for loopback
*/
#define CROCUS_LOOPBACK_XON_XOFF_FLAG  0xffee
#define CROCUS_LOOPBACK_FLOW_CTRL_FLAG 0x5000

/*
 * Define used for PPP */
#define CROCUS_PPP_START_FLAG          0xaa000000
#define CROCUS_PPP_START_FLAG_MASK     0xff000000
#define CROCUS_PPP_STATUS_EOF          (1 << 11)
#define CROCUS_PPP_MAX_LENGTH          512
#define CROCUS_PPP_LENGTH_MASK         0x3ff


#define CROCUS_TIMEOUT_1S              100 /* 100 * 10ms */
#define CROCUS_TIMEOUT_5S              500 /* 500 * 10ms */
#define CROCUS_TIMEOUT_10S             1000 /* 1000 * 10ms */
#define CROCUS_SLEEP_10MS              10
#define CROCUS_SLEEP_500MS             500
#define CROCUS_RING_BUFFER_ADDRESS_ALIGN_MASK   0x0003
#define CROCUS_RING_BUFFER_ADDRESS_ALIGN_OFFSET 0x0004
#define CROCUS_TX_DATA_START           0x1
#define CROCUS_TIMER_PROG_SHIFT        8
#define CROCUS_MODEM_CTRL_DTR          0x100
#define CROCUS_FLOW_CTRL_DCD_STATUS    0x40
#define CROCUS_MODEM_CTRL_RTS          0x8
#define CROCUS_FLOW_CTRL_CTS_STATUS    0x10


#define REVERSE_UINT(n) ((uint) (((n & 0xFF) << 24) | \
                                 ((n & 0xFF00) << 8) | \
                                 ((n & 0xFF0000) >> 8) | \
                                 ((n & 0xFF000000) >> 24)))


/* enum for async baud rate generator */
enum {
    CROCUS_ASYNC_BPS_NONE = 0,
    CROCUS_ASYNC_BPS_300,
    CROCUS_ASYNC_BPS_600,
    CROCUS_ASYNC_BPS_1200,
    CROCUS_ASYNC_BPS_2400,
    CROCUS_ASYNC_BPS_4800,
    CROCUS_ASYNC_BPS_9600,
    CROCUS_ASYNC_BPS_14400,
    CROCUS_ASYNC_BPS_19200,
    CROCUS_ASYNC_BPS_28800,
    CROCUS_ASYNC_BPS_32K,
    CROCUS_ASYNC_BPS_38400,
    CROCUS_ASYNC_BPS_48K,
    CROCUS_ASYNC_BPS_56K,
    CROCUS_ASYNC_BPS_57600,
    CROCUS_ASYNC_BPS_64K,
    CROCUS_ASYNC_BPS_72K,
    CROCUS_ASYNC_BPS_115200,
    CROCUS_ASYNC_BPS_128K,
    CROCUS_ASYNC_BPS_230400,
	CROCUS_ASYNC_BPS_256000,
};

/*
 *  Serial Data Structure.
 */
typedef struct crocus_serial_ds_t_ {
    crocus_global_regs_t      *global_regs_addr;
    crocus_ctrl_intr_regs_t   *ctrl_intr_regs_addr;
    crocus_scc_regs_t         *scc_regs_addr;
    crocus_ppp_tx_regs_t      *ppp_tx_regs_addr;
    crocus_ppp_rx_regs_t      *ppp_rx_regs_addr;
    crocus_serial_itf_t       *serial_itf_addr;
    crocus_ring_buf_regs_t    *ring_buf_regs_addr;
    ushort   port_num;               /* port/channel number         */
    ushort   brg_num;                /* BRG Rate Number             */
    uint     divider;                /* divider                     */
    uchar    clk_src;                /* Brg clock source            */
    uchar    speed_idx;              /* speed index                 */
    uchar    lpbk_mode;              /* Loopback Mode               */
    ushort   protocol;               /* Protocol                    */
    ulong    ctrl_id;                /* Device vendor-id            */
    ushort   num_buff;               /* Number of buffers           */
    ushort   buff_size;              /* Size of Buffers             */
    ulong    last_rx_desc_addr;      /* Address of last Rx desc     */
    uchar    *tx_buf_addr;           /* Tx Buffer Address           */
    uchar    *rx_buf_addr;           /* Rx Buffer Address           */
    uchar    *tx_ring_addr;          /* TX Ring Address             */
    uchar    *rx_ring_addr;          /* RX Ring Address             */
    uint     baudrate;               /* baud rate                   */
    uchar    crc_rx_buf_opt;         /* need 2 crc bytes in rx buf  */
    uchar    crc_enb;                /* crc default enb             */
    uchar    run_mode;               /* run mode                    */
    ulong    base_addr;
	char     *xdma_path;
} crocus_serial_ds_t;

typedef struct crocus_baud_t_ {
    uint baudrate_code;         /* one of the sync/async enums */
    uint desired_baud;
    uint baudrate;              /* the actual baudrate */
    uint divider;               /* clock divider */
    ushort clk_src;             /* clock source */
} crocus_baud_t;

/*
 * External Functions
 */
extern int crocus_async_reg_test(int);
extern int crocus_async_int_test(int);
extern int crocus_async_chan_lpbk_test(int);
extern int crocus_async_chan_lpbk_test2(int, int);
extern int crocus_async_chan_lpbk_test_all(void);

#endif /* end __NANOOK_CROCUS_DEFS__ */


/******** History ********
$Log: nanook_crocus_def.h,v $
Revision 1.2  2019/12/11 10:10:33  lucywang
Merged Nanook to main trunk


$Endlog$
*/
