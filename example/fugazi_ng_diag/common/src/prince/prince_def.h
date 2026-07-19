/* $Id: prince_def.h,v 1.6 2018/01/17 11:48:04 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_def.h,v $
 *------------------------------------------------------------------
 * prince_def.h 
 *      Prince projects - NIM 1T/2T/4T definitions and prototypes.
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PRINCE_DEFS__
#define __PRINCE_DEFS__

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
#define MAX_PRINCE_BUFFERS          0x4      /* can only be 2's power */
#define MAX_PRINCE_BUFFER_SIZE      0x80
#define MAX_PRINCE_SMALL_BUFFER_SIZE   0x10
#define MAX_PRINCE_LARGE_BUFFER_SIZE   0x100
#define PRINCE_CRC_RESERVED       0x10
#define PRINCE_DATA_PATTERN_55    0x55
#define PRINCE_INC_PAT            0x01
#define PRINCE_DEC_PAT            0x02
#define PRINCE_DATA_PAT           0x03
#define PRINCE_STX                0x02
#define PRINCE_ETX                0x03

/* enum for sync baud rate generator */

enum {
    PRINCE_SYNC_BPS_NONE = 0,
    PRINCE_SYNC_BPS_300,
    PRINCE_SYNC_BPS_600,
    PRINCE_SYNC_BPS_1200,
    PRINCE_SYNC_BPS_2400,
    PRINCE_SYNC_BPS_4800,
    PRINCE_SYNC_BPS_9600,
    PRINCE_SYNC_BPS_14400,
    PRINCE_SYNC_BPS_19200,
    PRINCE_SYNC_BPS_28800,
    PRINCE_SYNC_BPS_32K,
    PRINCE_SYNC_BPS_38400,
    PRINCE_SYNC_BPS_48K,
    PRINCE_SYNC_BPS_56K,
    PRINCE_SYNC_BPS_57600,
    PRINCE_SYNC_BPS_64K,
    PRINCE_SYNC_BPS_72K,
    PRINCE_SYNC_BPS_115200,
    PRINCE_SYNC_BPS_128K,
    PRINCE_SYNC_BPS_230400,
    PRINCE_SYNC_BPS_256K,
    PRINCE_SYNC_BPS_512K,
    PRINCE_SYNC_BPS_1024K,
    PRINCE_SYNC_BPS_2016K,
    PRINCE_SYNC_BPS_2048K,
    PRINCE_SYNC_BPS_4032K,
    PRINCE_SYNC_BPS_8064K,
    PRINCE_SYNC_BPS_10M
};

/* enum for async baud rate generator */

enum {
    PRINCE_AS_BPS_NONE = 0,
    PRINCE_AS_BPS_300,
    PRINCE_AS_BPS_600,
    PRINCE_AS_BPS_1200,
    PRINCE_AS_BPS_2400,
    PRINCE_AS_BPS_4800,
    PRINCE_AS_BPS_9600,
    PRINCE_AS_BPS_14400,
    PRINCE_AS_BPS_19200,
    PRINCE_AS_BPS_28800,
    PRINCE_AS_BPS_32K,
    PRINCE_AS_BPS_38400,
    PRINCE_AS_BPS_56K,
    PRINCE_AS_BPS_57600,
    PRINCE_AS_BPS_64K,
    PRINCE_AS_BPS_72K,
    PRINCE_AS_BPS_115200,
    PRINCE_AS_BPS_128K,
    PRINCE_AS_BPS_230400
};

/* Defines for PLL */
#define  PRINCE_PLL_TABLE_START         15 
#define  PRINCE_PLL_TABLE_INC           16 
#define  PRINCE_PLL_BAUD_DIV            24   /* used by 4AS and 8AS */

/* Defines for cable id */
#define CABLE_RS232AS    0x6 
#define CABLE_RS232A     0x7

/* Defines for freq counter register */
#define PRINCE_FREQ_SELECT_MASK         0x0007
#define PRINCE_FREQ_COUNT_WAIT          1100 
#define PRINCE_FREQ_MICRO_WAIT          100
#define PRINCE_FREQ_MAX_PASS            15

/* Defines for DMA waiting */
#define PRINCE_MAX_DMA_PASS             200
#define PRINCE_DMA_RETRY_PASS           66 
#define PRINCE_HIGH_SPEED_WAIT          1
#define PRINCE_LOW_SPEED_WAIT           6 

/* Defines for DMA waiting */
#ifdef REVA
#define PRINCE_SCC_HIGH_PRIORITY        0
#define PRINCE_SCC_LOW_PRIORITY         1
#else
#define PRINCE_SCC_HIGH_PRIORITY_1      0
#define PRINCE_SCC_HIGH_PRIORITY_2      1
#define PRINCE_SCC_LOW_PRIORITY         2
#endif

/*
 * Defines used to support the Prince SCC Loopback Modes.
 */
#define  PRINCE_SCC_INT_LOOPBACK        0x01
#define  PRINCE_SCC_EXT_LOOPBACK        0x02

/*
 * Defines used to support the Prince net interrupt test
 */
#define  PRINCE_RX_BUF_TYPE             0x01
#define  PRINCE_TX_BUF_TYPE             0x02
#define  PRINCE_RX_FRAME_TYPE           0x03
#define  PRINCE_TX_FRAME_TYPE           0x04

/*
 * Defines used to support the Prince Run Modes.
 */
#define  PRINCE_POLL_MODE               0x01
#define  PRINCE_INT_MODE                0x02
#define  PRINCE_FRAME_INT_MODE          0x03
#define  PRINCE_NO_CHECK_MODE           0x04

/*
 * Defines used to support the Prince clock source.
 */
#define  PRINCE_CLK_SRC_SYNC            0x01
#define  PRINCE_CLK_SRC_ASYNC           0x02
#define  PRINCE_CLK_SRC_PLL1_64K        0x03
#define  PRINCE_CLK_SRC_PLL1_56K        0x04
#define  PRINCE_CLK_SRC_PLL2_64K        0x05
#define  PRINCE_CLK_SRC_PLL2_56K        0x06
#define  PRINCE_CLK_SRC_PLL_AS          0x07      /* 8AS and 4AS */

/*
 * Defines used to support the Prince Loopback Protocol.
 */
#define  PRINCE_HDLC                    0x01
#define  PRINCE_BISYNC                  0x02
#define  PRINCE_UART_ASYNC              0x03
#define  PRINCE_UART_PPP                0x04
#define  PRINCE_TRANSPARENT             0x05

/*
 * Defines used to support the channel# mode register low
 */
#define  PRINCE_MODE_DISABLE            0x0000
#define  PRINCE_MODE_HDLC               0x0001
#define  PRINCE_MODE_BISYNC             0x0002
#define  PRINCE_MODE_TRANSPARENT        0x0003
#define  PRINCE_MODE_UART_ASYNC         0x0004
#define  PRINCE_MODE_UART_PPP           0x0005
#define  PRINCE_MODE_MASK               0x000F 
#define  PRINCE_MODE_CRC                0x0030
#define  PRINCE_MODE_CRC_16_MASK        0x18000
#define  PRINCE_MODE_EBCDIC             0x4000 
#define  PRINCE_MODE_IDLE_MARK          0x3000 
#define  PRINCE_MODE_IDLE_FLAG          0x1000 

/*
 * Defines used to support the channel# mode register 
 */
#define  PRINCE_MODE_NRZI               0x00800000
#define  PRINCE_MODE_NRZ_RSV            0x01000000
#define  PRINCE_MODE_NRZ_MASK           0x01800000
#define  PRINCE_MODE_FCS_LSB            0x02000000
#define  PRINCE_MODE_FCS_MASK           0x06000000
#define  PRINCE_MODE_FLAG_CHK_MASK      0x18000000
#define  PRINCE_MODE_FLAG1_CHK          0x08000000      /* HDLC   */
#define  PRINCE_MODE_FLAG_BOTH_CHK      0x18000000      /* Bisync */

/*
 * Defines used to support the channel# command register. .
 */
#ifdef REVA
#define  PRINCE_CMD_START_TX_RX         0x8050
#define  PRINCE_CMD_START_TX            0x8040
#define  PRINCE_CMD_START_RX            0x0010
#define  PRINCE_CMD_STOP_TX_RX          0x0028
#define  PRINCE_CMD_RESET_TX_RX         0x0003

#define  PRINCE_CMD_LP_START_TX_RX      0x6010
#define  PRINCE_CMD_LP_START_TX         0x6000
#define  PRINCE_CMD_LP_STOP_TX_RX       0x1008
#else
#define  PRINCE_CMD_START_RX            0x0010
#define  PRINCE_CMD_RESET_TX_RX         0x0003

#define  PRINCE_CMD_HP1_START_TX_RX     0x8050
#define  PRINCE_CMD_HP1_START_TX        0x8040
#define  PRINCE_CMD_HP1_STOP_TX_RX      0x0028

#define  PRINCE_CMD_HP2_START_TX_RX     0x6010
#define  PRINCE_CMD_HP2_START_TX        0x6000
#define  PRINCE_CMD_HP2_STOP_TX_RX      0x1008

#define  PRINCE_CMD_LP_START_TX_RX      0x0610
#define  PRINCE_CMD_LP_START_TX         0x0600
#define  PRINCE_CMD_LP_STOP_TX_RX       0x0108
#endif
/*
 * Defines used to support the Modem control register (per chan).
 */
#define  PRINCE_MODEM_BRG_MASK          0x00e0
#define  PRINCE_MODEM_BRG_2M            0x0000
#define  PRINCE_MODEM_BRG_252K          0x0020
#define  PRINCE_MODEM_BRG_32M           0x0040
#define  PRINCE_MODEM_BRG_1M            0x0060     /* 1.5625MHZ */
#define  PRINCE_MODEM_BRG_PLL1          0x0080
#define  PRINCE_MODEM_BRG_PLL2          0x00a0
#define  PRINCE_MODEM_BRG_10M           0x00c0
#define  PRINCE_MODEM_LOOPBACK          0x0002 

/*
 * Defines used to support the Flow control register (per chan).
 */
#define PRINCE_DCD_ENABLE               0x8000
#define PRINCE_DSR_ENABLE               0x4000
#define PRINCE_CTS_ENABLE               0x2000
#define PRINCE_CABLEID_ENABLE           0x1000
#define PRINCE_DCD_INPUT_IGNORE         0x0400

/*
 * Defines used to support the modem interrupt status register (per chan).
 */
#define  PRINCE_DCD_FORCE_IRQ           0x0080
#define  PRINCE_DSR_FORCE_IRQ           0x0040 
#define  PRINCE_CTS_FORCE_IRQ           0x0020 
#define  PRINCE_CABLEID_FORCE_IRQ       0x0010  
#define  PRINCE_DCD_STATUS              0x0008  
#define  PRINCE_DSR_STATUS              0x0004  
#define  PRINCE_CTS_STATUS              0x0002  
#define  PRINCE_CABLEID_STATUS          0x0001  

/*
 * Defines used to support the serial interface control register (per chan).
 */
#define  PRINCE_CABLE_ID_MASK           0x001e
#define  PRINCE_CABLE_ID_MODE_BIT       0x0001
#define  PRINCE_RESET_ITF_CKT           0x0200
#define  PRINCE_INVERT_RXC              0x0400
#define  PRINCE_DTE_EXT_EN              0x8000

/*
 * Defines used to support the protocol int enable register (per chan).
 */
#define  PRINCE_DMA_RX_BUF_ENB          0x0001
#define  PRINCE_DMA_RX_FRAME_ENB        0x0002
#define  PRINCE_DMA_TX_BUF_ENB          0x0004
#define  PRINCE_DMA_TX_FRAME_ENB        0x0008

/*
 * Defines used to support the protocol int status register (per chan).
 */
#define  PRINCE_DMA_RX_BUF_STATUS       0x0001
#define  PRINCE_DMA_RX_FRAME_STATUS     0x0002
#define  PRINCE_DMA_TX_BUF_STATUS       0x0004
#define  PRINCE_DMA_TX_FRAME_STATUS     0x0008

/*
 * Defines used to support the int1 enable register 
 */
#define  PRINCE_MODEM_ENB               0x8
#define  PRINCE_FREQ_ENB                0x10
#define  PRINCE_DMA_ENB                 0x20
#define  PRINCE_TIMER_17_20_ENB         0x2
#define  PRINCE_TIMER_1_16_ENB          0x1

/*
 * Defines used to support the int1 status register 
 */
#define  PRINCE_MODEM_STATUS            0x8
#define  PRINCE_FREQ_STATUS             0x10
#define  PRINCE_DMA_STATUS              0x20

/*
 * Defines used to support PLL1 and PLL2 control register.
 */
#define  PRINCE_PLL_SELECT_MASK         0x000f
#define  PRINCE_PLL_SELECT_OSC          0x0008
#define  PRINCE_PLL_ENABLE              0x0020
#define  PRINCE_PLL_8K_REF_ENABLE       0x0010
#define  PRINCE_PLL_8K_REF_STATUS       0x0040

/*
 * Defines used to support TDM control register.
 */
#define  PRINCE_TDMA_SELECT_MASK        0x00f0
#define  PRINCE_TDMB_SELECT_MASK        0x000f
#define  PRINCE_TDMA_SELECT_OSC         0x0080
#define  PRINCE_TDMB_SELECT_OSC         0x0008
#define  PRINCE_TDMA_8KHZ_ENABLE        0x0200
#define  PRINCE_TDMB_8KHZ_ENABLE        0x0100
#define  PRINCE_TDMA_8KHZ_REF_STATUS    0x8000
#define  PRINCE_TDMB_8KHZ_REF_STATUS    0x4000

#define  PRINCE_8KHZ_REF_WAIT           100 
#define  PRINCE_8KHZ_REF_WAIT_ONE       100

/* max counter for IRQ occurrence */
#define  PRINCE_IRQ_COUNT_MAX   10

/* Defines used to test BCC register */
#define  PRINCE_BCC_CHAN_SIZE           0x10
#define  PRINCE_BCC_CHAN_NUM            8 
#define  PRINCE_MAX_CHAN_NUM            16 

/* define options to display register */
#define  PRINCE_DMA_RX          1
#define  PRINCE_DMA_TX          2
#define  PRINCE_PPP_RX          1
#define  PRINCE_PPP_TX          2
#define  PRINCE_REG_ALL         1
#define  PRINCE_REG_ONE         2
#define  PRINCE_REG_COMMON      1
#define  PRINCE_REG_DMA         2
#define  PRINCE_REG_PROTOCOL    3
#define  PRINCE_REG_SERIAL      4

/*
 * Defines used to support Cheerios control register.
 */
#define  PRINCE_LED1_MASK               0x0003
#define  PRINCE_LED1_YELLOW             0x0002
#define  PRINCE_LED1_GREEN              0x0001
#define  PRINCE_LED2_MASK               0x0018
#define  PRINCE_LED2_YELLOW             0x0010
#define  PRINCE_LED2_GREEN              0x0008
#define  PRINCE_FAST_MODE               0x8000

/*
 * Defines used to support low speed index (<14400).
 */
#define  PRINCE_LOW_SPEED_INDEX         7 
#define  PRINCE_SPEED_INDEX_INC         1 

/*
 * Defines used to support ppp CRC rx buffer option.
 */
#define  CRC_RX_BUF_OPT_ON              1 

/*
 * Defines used to support async timer roll over value.
 * Count * one bit clock.  Program to max value.
 */
#define  PRINCE_ASYNC_TIMER_COUNT       0x7f 

/*
 * Defines used to support PLL choice.
 */
#define  PRINCE_MAIN_MENU               1 
#define  PRINCE_SUB_MENU                2 
#define  PRINCE_PLL1                    1 
#define  PRINCE_PLL2                    2 

/*
 * Defines used to support frequency margin.
 */
#define  PRINCE_MARGIN_100              100 
#define  PRINCE_MARGIN_95               95 
#define  PRINCE_MARGIN_90               90 
#define  PRINCE_MARGIN_105              105 
#define  PRINCE_MARGIN_110              110 
#define  PRINCE_RETRY                   2
#define  PRINCE_RETRY_COUNT             2     /* turn PLL retry on */

#define MAX_CH_NUM      4
#define CASE_ONE        1   /* special case for test items */
#define CASE_TWO        2   /* special case for test items */
#define DOWNLOAD_DONE   1

#define PRINCE_INACTIVE_MODE            0x01
#define PRINCE_FPGA_MODE                0x02
#define PRINCE_EEPROM_MODE              0x03

/*
 * External Functions
 */
extern ulong get_scc_dma_rx_phys();
extern ulong get_scc_dma_rx_virt();
extern ulong get_scc_dma_tx_phys();
extern ulong get_scc_dma_tx_virt();
extern ulong get_scc_dma_rxbd_phys();
extern ulong get_scc_dma_rxbd_virt();
extern ulong get_scc_dma_txbd_phys();
extern ulong get_scc_dma_txbd_virt();

extern ulong get_ge_dma_rx_phys();
extern ulong get_ge_dma_rx_virt();
extern ulong get_ge_dma_tx_phys();
extern ulong get_ge_dma_tx_virt();
extern ulong get_ge_rxbd_phys();
extern ulong get_ge_rxbd_virt();
extern ulong get_ge_txbd_phys();
extern ulong get_ge_txbd_virt();

extern int ge_dma_reg_test();
extern int ge_dma_intr_test();
extern int ge_mac_reg_test();
extern int ge_mac_intr_test();
extern int ge_mac_lpbk_test();
extern int ge_mac_lpbk_test_raw_skt();
extern int phy_reg_test();
extern int phy_intr_test();
extern int phy_int_lpbk_test();
extern int phy_int_lpbk_test_raw_skt();
extern int phy_ext_lpbk_test();
extern int phy_ext_lpbk_test_raw_skt();
extern int phy_reg_rd();
extern int phy_reg_wr();
extern int phy_reg_dp();
extern int smi_reg_dp();
extern int ge_dma_reg_rd();
extern int ge_dma_reg_wr();
extern int ge_dma_reg_dp();
extern int ge_mac_reg_rd();
extern int ge_mac_reg_wr();
extern int ge_mac_reg_dp();
extern int scc_reg_rd();
extern int scc_reg_wr();
extern int scc_reg_dp();

extern int serial_channel_test (int);
extern int prince_pkt_lpbk_test();

extern int check_offset (ushort, reg_info_t*);
extern ulong get_reg_size (ushort, reg_info_t*);
extern void reg_dump(ulong, reg_info_t*);
extern ulong fpga_ver;

#endif /* end __PRINCE_DEFS__ */


/******** History ******** 
$Log: prince_def.h,v $
Revision 1.6  2018/01/17 11:48:04  iachang
CSCvh19145 : Fixed Reva firmware compiler issue. Reva didn't support new FPGA.

Revision 1.5  2017/07/18 08:48:41  iachang
Prince FPGA Enhanced Feature, Support HP1, HP2, and LP.

Revision 1.4  2014/01/13 03:14:22  xiaoyizh
Add new routine for PHY external loopback test using raw socket.

Revision 1.3  2013/08/02 09:33:58  xiaoyizh
Add new routines using raw socket.

Revision 1.2  2013/06/25 07:49:21  xiaoyizh
Add new control bit for enable the clocks in DTE mode.

Revision 1.1  2013/04/19 07:17:50  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/
