/* $Id: p1021_etsec.h,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: p1021_etsec.h
 *
 * Description: eTSEC definition file for QorIQ P1021
 * Ported from Xformers, ppc_tsec.h
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


#ifndef __P1021_ETSEC_H__
#define __P1021_ETSEC_H__

#define NUM_ETSEC               3
#define ETSEC1                  1
#define ETSEC2                  2
#define ETSEC3                  3
#define ETSEC_OFFSET            0x00001000
#define DISP_DFLT_ETSEC_REGS    0x0000000f
#define BIT_ZERO                0x01
#define THREE_WORDS             3        /* addr len = 6 bytes/3 words */
#define ONE_THOUSAND            1000
#define ONE_MILLION             1000000
#define ETSEC_SGMII             0x0200


typedef enum {
    ETSEC_MDIO = 0,
    ETSEC_GROUP0,
    ETSEC_GROUP1
} MB_ETSEC_MODE;

#define ETSEC1_GROUP0_OFFSET    0xB0000
#define ETSEC2_GROUP0_OFFSET    0xB1000
#define ETSEC3_GROUP0_OFFSET    0xB2000
#define ETSEC1_GROUP1_OFFSET    0xB4000
#define ETSEC2_GROUP1_OFFSET    0xB5000
#define ETSEC3_GROUP1_OFFSET    0xB6000

typedef struct tsec_bd {
   volatile ushort status;        /* Status Fields  */
   volatile ushort length;        /* Buffer length  */
   volatile uchar  *buf_ptr;      /* Buffer Pointer */
} tsec_bd_t;

/*
 * flags:
 *  bit    8: 0 RGMII mode, 1 SGMII mode
 *  bit    7: 0 link is down, 1 link is up
 *  bit    6: 0 half duplex, 1 full duplex
 *  bit    5: 0 FE speed, 1 GE speed (1000 Mbps)
 *  bit    4: 0 10 Mbps, 1 100 Mbps
 *  bits 3-0: loopback type
 */
typedef struct tsec_info_struct {
    unsigned char *name;
    unsigned int reg_base_addr;     /* ptr to SMI controller base address. */
    unsigned int phyaddr;
    unsigned int flags;
    unsigned char mac_addr[6];
    unsigned int ip_addr;
    unsigned int tsec_num;
    unsigned int tx_bd;             /* ptr to TxBD ring */
    unsigned int tx_buf;            /* ptr to start of tx_buf */
    unsigned int rx_bd;             /* ptr to RxBD ring */
    unsigned int rx_buf;            /* ptr to start of rx_buf */
} tsec_info_struct_t;


/* etsec, initialization values for tsec_info_struct */
#define CONFIG_TSEC1            1
#define CONFIG_MPC85XX_TSEC1    1
#define CONFIG_TSEC1_NAME       "eTSEC1"
#define CONFIG_TSEC1_FLAG \
        (SGMII_SPEED_1000 | SGMII_FULL_DUPLEX | SGMII_FLOW_CTRL)
#define TSEC1_PHY_ADDR          0
#define TSEC1_MAC_ADD_0         0x00
#define TSEC1_MAC_ADD_1         0xE0
#define TSEC1_MAC_ADD_2         0x0C
#define TSEC1_MAC_ADD_3         0x00
#define TSEC1_MAC_ADD_4         0x40
#define TSEC1_MAC_ADD_5         0xFD
#define TSEC1_IP_ADD            0xC0A80465      /* 192.168.4.101 */
#define TSEC1_BASE_ADDR         (ADRSPC_PQUICC_IMEMB + ETSEC1_GROUP0_OFFSET)

#define CONFIG_TSEC2            1
#define CONFIG_MPC85XX_TSEC2    1
#define CONFIG_TSEC2_NAME       "eTSEC2"
#define CONFIG_TSEC2_FLAG \
        (SGMII_SPEED_1000 | SGMII_FULL_DUPLEX | SGMII_FLOW_CTRL | ETSEC_SGMII)
#define TSEC2_PHY_ADDR          1
#define TSEC2_MAC_ADD_0         0x00
#define TSEC2_MAC_ADD_1         0xE0
#define TSEC2_MAC_ADD_2         0x0C
#define TSEC2_MAC_ADD_3         0x00
#define TSEC2_MAC_ADD_4         0x41
#define TSEC2_MAC_ADD_5         0xFD
#define TSEC2_IP_ADD            0xC0A80466      /* 192.168.4.102 */
#define TSEC2_BASE_ADDR         (ADRSPC_PQUICC_IMEMB + ETSEC2_GROUP0_OFFSET)

#define CONFIG_TSEC3            1
#define CONFIG_MPC85XX_TSEC3    1
#define CONFIG_TSEC3_NAME       "eTSEC3"
#define CONFIG_TSEC3_FLAG \
        (SGMII_SPEED_1000 | SGMII_FULL_DUPLEX | SGMII_FLOW_CTRL | ETSEC_SGMII)
#define TSEC3_PHY_ADDR          2
#define TSEC3_MAC_ADD_0         0x00
#define TSEC3_MAC_ADD_1         0xE0
#define TSEC3_MAC_ADD_2         0x0C
#define TSEC3_MAC_ADD_3         0x00
#define TSEC3_MAC_ADD_4         0x42
#define TSEC3_MAC_ADD_5         0xFD
#define TSEC3_IP_ADD            0xC0A80467      /* 192.168.4.103 */
#define TSEC3_BASE_ADDR         (ADRSPC_PQUICC_IMEMB + ETSEC3_GROUP0_OFFSET)

/* Ethernet destination address */ 
#define ENET_PRAM_PADDR_H       0x0019        /* station address is */
#define ENET_PRAM_PADDR_M       0x2233        /* 00 19 22 33 48 55  */
#define ENET_PRAM_PADDR_L       0x4855        /* 00 is the low order byte */

/* Ethernet-Specific Parameter RAM stations address */
#define ENET_SPRAM_PADDR_H      0x5648        /* station address is    */
#define ENET_SPRAM_PADDR_M      0x3322        /* 56 48 33 22 19 00    */
#define ENET_SPRAM_PADDR_L      0x1900        /* 00 is the high order byte */

#define PROTOCOL_TYPE           0x800         /* DOD IP */

/*
 * Transmit Descriptor Flags.
 */
#define PQUICC_BDSTAT_TX_RDY        0x8000      /* Ready */
#define PQUICC_BDSTAT_TX_PAD        0x4000      /* Pad */
#define PQUICC_BDSTAT_TX_WRAP       0x2000      /* Wrap */
#define PQUICC_BDSTAT_TX_INT        0x1000      /* Interrupt */
#define PQUICC_BDSTAT_TX_LAST       0x0800      /* Last */
#define PQUICC_BDSTAT_TX_TC         0x0400      /* Tx CRC */
#define PQUICC_BDSTAT_TX_PRE        0x0200      /* Preamble/Defer */
#define PQUICC_BDSTAT_TX_LCOL       0x0080      /* Late Collision (Ethernet) */
#define PQUICC_BDSTAT_TX_RLIM       0x0040      /* Retransmission Limit (Eth) */
#define PQUICC_BDSTAT_TX_RC_MASK    0x003C      /* Retry Count */
#define PQUICC_BDSTAT_TX_UNRRUN     0x0002      /* Underrun */
#define PQUICC_BDSTAT_TX_CSLOS      0x0001      /* Carrier Sense Lost (Eth) */
#define PQUICC_BDSTAT_TX_TR         0x0001      /* Truncation */
#define PQUICC_BDSTAT_TX_STATS      0x02ff

/*
 * Buffer Descriptor Flags.  If a flag is specific to a
 * protocol then the mode where that flag is defined is
 * included in parenthesis.
 */
#define PQUICC_BDSTAT_RX_EMPTY  0x8000          /* Empty */
#define PQUICC_BDSTAT_RX_RO1    0x4000          /* Receive software ownership */
#define PQUICC_BDSTAT_RX_WRAP   0x2000          /* Wrap */
#define PQUICC_BDSTAT_RX_INT    0x1000          /* Interrupt */
#define PQUICC_BDSTAT_RX_LAST   0x0800          /* Last */
#define QUICC_BDSTAT_RX_CNTRL   0x0800          /* Control Char UART only */
#define QUICC_BDSTAT_RX_ADDR    0x0400          /* Address UART only */
#define QUICC_BDSTAT_RX_FIRST   0x0400          /* First */
#define QUICC_BDSTAT_RX_CONTIN  0x0200          /* Continuious Mode */
#define QUICC_BDSTAT_RX_MISS    0x0100          /* Miss (Ethernet) */
#define QUICC_BDSTAT_RX_AM      0x0100          /* Address Match UART only */
#define QUICC_BDSTAT_RX_DERR    0x0080          /* DPLL Error (HDLC) */
#define PQUICC_BDSTAT_RX_BC     0x0080          /* Broadcast */
#define PQUICC_BDSTAT_RX_MULTI  0x0040          /* Multicast */
#define PQUICC_BDSTAT_RX_FLV    0x0020          /* Frame Length Violation */
#define QUICC_BDSTAT_RX_BREAK   0x0020          /* Break detect UART only */
#define PQUICC_BDSTAT_RX_NOFA   0x0010          /* Non-octet frame alignment */
#define QUICC_BDSTAT_RX_FR      0x0010          /* Framing error UART only */
#define QUICC_BDSTAT_RX_ABORT   0x0008          /* Abort Sequence (HDLC) */
#define PQUICC_BDSTAT_RX_SHORT  0x0008          /* Short Frame (Ethernet) */
#define QUICC_BDSTAT_RX_PR      0x0008          /* Parity error UART only */
#define PQUICC_BDSTAT_RX_CRCERR 0x0004          /* CRC Error */
#define PQUICC_BDSTAT_RX_OVRRUN 0x0002          /* Overrun */
#define PQUICC_BDSTAT_RX_TRUNC  0x0001          /* Collision (Ethernet) */
#define PQUICC_BDSTAT_RX_STATS  0x001f

/* error flags */
#define POLL_TX_FAIL           0x0001
#define POLL_TX_ERR            0x0002
#define POLL_RX_FAIL           0x0004
#define POLL_RX_ERR            0x0008
#define INTR_RX_FAIL           0x0010
#define INTR_RX_ERR            0x0020
#define INTR_TX_FAIL           0x0040
#define INTR_TX_ERR            0x0080
#define DATA_MISMATCH          0x0100
#define CNT_MISMATCH           0x0200
#define BUILD_PKT_ERR          0x1000
#define XMISSION_ERR           0x2000
#define PKT_TIMEOUT            0x4000
#define NO_RX_FRAME            0x8000

#define RX_BD_ERR_MASK  (PQUICC_BDSTAT_RX_FLV | PQUICC_BDSTAT_RX_NOFA |\
                         PQUICC_BDSTAT_RX_SHORT | PQUICC_BDSTAT_RX_OVRRUN |\
                         PQUICC_BDSTAT_RX_TRUNC | PQUICC_BDSTAT_RX_CRCERR )

#define TX_BD_ERR_MASK  (PQUICC_BDSTAT_TX_LCOL | PQUICC_BDSTAT_TX_RLIM |\
                         PQUICC_BDSTAT_TX_UNRRUN )

/* TSEC Ievent register (IEVENT) */
#define TSEC_IEVENT_BABR    0x80000000    /* Babbling receive error */
#define TSEC_IEVENT_RXC     0x40000000    /* Receive control interrupt */
#define TSEC_IEVENT_BSY     0x20000000    /* Busy condition interrupt */
#define TSEC_IEVENT_EBERR   0x10000000    /* Ethernet bus error */
#define TSEC_IEVENT_MSRO    0x04000000    /* MSTAT register overflow */
#define IEVENT_GTSC_SET     0x02000000    /* Graceful transmit stop complete */
#define TSEC_IEVENT_BABT    0x01000000    /* Babbling transmit error */
#define TSEC_IEVENT_TXC     0x00800000    /* Transmit control interrupt */
#define TSEC_IEVENT_TXE     0x00400000    /* Transmit error */
#define TSEC_IEVENT_TXB     0x00200000    /* Transmit buffer */
#define TSEC_IEVENT_TXF     0x00100000    /* Transmit frame interrupt */
#define TSEC_IEVENT_LC      0x00040000    /* Late collision */
#define TSEC_IEVENT_CRL     0x00020000    /* Collision retry limit */
#define TSEC_IEVENT_XFUN    0x00010000    /* Transmit FIFO underrun */
#define TSEC_IEVENT_RXBO    0x00008000    /* Receive buffer */
#define TSEC_IEVENT_MAG     0x00000800    /* Magic Packet detected */
#define TSEC_IEVENT_MMRD    0x00000400    /* MMI management read completion */
#define TSEC_IEVENT_MMWR    0x00000200    /* MMI management write completion */
#define IEVENT_GRSC_SET     0x00000100    /* Graceful receive stop complete */
#define TSEC_IEVENT_RXFO    0x00000080    /* Receive frame interrupt */
#define TSEC_IEVENT_FIR     0x00000008    /* Receive filer result invalid */
#define TSEC_IEVENT_FIQ     0x00000004    /* Filed frame to invalid Rx queue */
#define TSEC_IEVENT_DPE     0x00000002    /* Internal data parity error */
#define TSEC_IEVENT_PERR    0x00000001    /* Receive frame parse error */
#define TSEC_IEVENTS        (TSEC_IEVENT_BABR | TSEC_IEVENT_RXC   | \
                             TSEC_IEVENT_BSY  | TSEC_IEVENT_EBERR | \
                             TSEC_IEVENT_MSRO | IEVENT_GTSC_SET   | \
                             TSEC_IEVENT_BABT | TSEC_IEVENT_TXC   | \
                             TSEC_IEVENT_TXE  | TSEC_IEVENT_TXB   | \
                             TSEC_IEVENT_TXF  | TSEC_IEVENT_LC    | \
                             TSEC_IEVENT_CRL  | TSEC_IEVENT_XFUN  | \
                             TSEC_IEVENT_RXBO | TSEC_IEVENT_MAG   | \
                             TSEC_IEVENT_MMRD | TSEC_IEVENT_MMWR  | \
                             IEVENT_GRSC_SET  | TSEC_IEVENT_RXFO  | \
                             TSEC_IEVENT_FIR  | TSEC_IEVENT_FIQ   | \
                             TSEC_IEVENT_DPE  | TSEC_IEVENT_PERR)

#define TSEC_ERR_IEVENTS    (TSEC_IEVENT_BABR  | TSEC_IEVENT_RXC  | \
                             TSEC_IEVENT_BSY   | TSEC_IEVENT_EBERR| \
                             IEVENT_GTSC_SET   | TSEC_IEVENT_BABT | \
                             TSEC_IEVENT_TXC   | TSEC_IEVENT_TXE  | \
                             TSEC_IEVENT_LC    | TSEC_IEVENT_CRL  | \
                             TSEC_IEVENT_XFUN  | TSEC_IEVENT_MAG  | \
                             TSEC_IEVENT_FIR   | IEVENT_GRSC_SET  | \
                             TSEC_IEVENT_FIQ   | TSEC_IEVENT_DPE  | \
                             TSEC_IEVENT_PERR)

/* Ethernet Control Register (ECNTRL) */
#define ECNTRL_FIFM             0x00008000
#define ECNTRL_CLRCNT           0x00004000
#define ECNTRL_AUTOZ            0x00002000
#define ECNTRL_STEN             0x00001000
#define ECNTRL_GMIIM            0x00000040
#define ECNTRL_TBIM             0x00000020
#define ECNTRL_RPM              0x00000010
#define ECNTRL_R100M            0x00000008
#define ECNTRL_RMM              0x00000004
#define ECNTRL_SGMIIM           0x00000002

/* DMA Control Register (DMACTRL) */
#define TSEC_DMACTRL_LE        0x00001000    /* Little Endian descriptor mode */
#define TSEC_DMACTRL_TDSEN     0x00000080    /* Tx Data snoop enable */
#define TSEC_DMACTRL_TBDSEN    0x00000040    /* TxBD snoop enable */
#define DMACTRL_SET_GRS        0x00000010    /* Graceful receive stop */
#define DMACTRL_SET_GTS        0x00000008    /* Graceful transmit stop */
#define TSEC_DMACTRL_TOD       0x00000004    /* Transmit on demand */
#define TSEC_DMACTRL_WWR       0x00000002    /* Write with response */
#define TSEC_DMACTRL_WOP       0x00000001    /* Wait or poll */

/* Minimum Frame Length Register (MINFLR) */
#define TSEC_MINFLR_64         0x00000040    /* 64 bytes */

/* Transmit Control Register (TCTRL) */
#define TSEC_TCTRL_THDF        0x00000800
#define TSEC_TCTRL_RFC_PAUSE   0x00000010
#define TSEC_TCTRL_TFC_PAUSE   0x00000008
#define TSEC_TCTRL_TXSCHED_RR  0x00000004
#define TSEC_TCTRL_TXSCHED_PRI 0x00000002
#define TSEC_TCTRL_TXSCHED_ONE 0x00000000
#define TSEC_TCTRL_TXSCHED_MSK 0x00000006

/* Transmit Status Register (TSTAT) */
#define TSEC_TSTAT_THLT        0x80000000    /* Transmit halt */
#define TSEC_TSTAT_TXF         0x00008000    /* Transmit frame event */

/* Receive Control Register (RCTRL) */
#define RCTRL_LFC              0x00004000
#define RCTRL_VLEX             0x00002000
#define RCTRL_BC_REJ           0x00000010
#define RCTRL_PROM_MODE        0x00000008
#define RCTRL_RSF              0x00000004
#define RCTRL_EMEN             0x00000002

/* Receive Status Register (RSTAT) */
#define TSEC_RSTAT_QHLT        0x00800000    /* RxBD queue is halted */
#define TSEC_RSTAT_RXF         0x00000080    /* RxBD frame event */

/* MAC Configuration Register 1 (MACCFG1) */
#define MACCFG1_SOFT_RESET     0x80000000
#define MACCFG1_RESET_RX_MC    0x00080000
#define MACCFG1_RESET_TX_MC    0x00040000
#define MACCFG1_RESET_RX_FUN   0x00020000
#define MACCFG1_RESET_TX_FUN   0x00010000
#define MACCFG1_LOOPBACK       0x00000100
#define MACCFG1_RX_FLOW        0x00000020
#define MACCFG1_TX_FLOW        0x00000010
#define MACCFG1_SYNCD_RX_EN    0x00000008
#define MACCFG1_RX_ENABLE      0x00000004
#define MACCFG1_SYNCD_TX_EN    0x00000002
#define MACCFG1_TX_ENABLE      0x00000001

/* MAC Configuration Register 2 (MACCFG2) */
#define MACCFG2_PREAM_LEN_MASK 0x0000F000
#define MACCFG2_PREAM_DEFAULT  0x00007000
#define MACCFG2_IF_MODE_MASK   0x00000300
#define MACCFG2_BYTE_MODE      0x00000200
#define MACCFG2_NIBBLE_MODE    0x00000100
#define MACCFG2_PREAM_RX_EN    0x00000080
#define MACCFG2_PREAM_TX_EN    0x00000040
#define MACCFG2_HUGE_FRAME     0x00000020
#define MACCFG2_LEN_CK         0x00000010
#define MACCFG2_MPEN           0x00000008
#define MACCFG2_PAD_CRC        0x00000004
#define MACCFG2_CRC_EN         0x00000002
#define MACCFG2_FULL_DUPLEX    0x00000001

/* MII Management Configuration Register (MIIMCFG) */
#define MIICFG_RESET           0x80000000
#define MIICFG_NO_PREAMBLE     0x00000010
#define MIICFG_MGMTCLK_MASK    0x00000007
#define MIICFG_DIVIDE_28       0x00000007    /* default, (esec clock/28)/8 */
#define MIICFG_DIVIDE_20       0x00000006    /* 1/20 of the etsec clock/8 */
#define MIICFG_DIVIDE_14       0x00000005    /* 1/14 of the etsec clock/8 */
#define MIICFG_DIVIDE_10       0x00000004    /* 1/10 of the etsec clock/8 */
#define MIICFG_DIVIDE_8        0x00000003    /* 1/8 of the etsec clock/8  */
#define MIICFG_DIVIDE_6        0x00000002    /* 1/6 of the etsec clock/8  */
#define MIICFG_DIVIDE_4        0x00000001    /* 1/4 of the etsec clock/8  */

/* MII Management Command Register (MIIMCOM) */
#define MIIMCOM_SCAN_CYCLE     0x00000002    /* continuous read cycles */
#define MIIMCOM_READ_CYCLE     0x00000001    /* single read cycles */
#define MIIM_READ_CYCLE        0x00000001

/* MII Management Address Register (MIIMADD) */
#define MIIMADD_PHY_ADDR_SHIFT 8

/* MII Management Indicator Register (MIIMIND) */
#define MIIMIND_NOTVALID       0x00000004
#define MIIMIND_SCAN           0x00000002
#define MIIMIND_BUSY           0x00000001

/* FIFO Transmit Threshold Register (FIFO_TX_THR) */
#define TSEC_FIFO_TX_THR_16    0x00000010    /* 16 entries */

/* FIFO Transmit Starve Register (FIFO_TX_STARVE) */
#define TSEC_FIFO_TX_STARVE_8  0x00000008    /* 8 entries */

/* FIFO Transmit Starve Shutoff Register (FIFO_TX_STARVE_SHUTOFF) */
#define TSEC_FIFO_TX_SRVSHUT_32    0x00000020    /* 32 entries */

/* Attribute Register (ATTR) */
#define TSEC_ATTR_RDSEN        0x00000080    /* Rx data snoop enable */
#define TSEC_ATTR_RBDSEN       0x00000040    /* RxBD snoop enable */

/* Receive Queue Parameter Register (RQPRM) */
#define TSEC_RQPRM_FBTHR_FOUR  0x04000000    /* Free BD threshold */
#define TSEC_RQPRM_FBTHR_MASK  0xFF000000
#define TSEC_RQPRM_LEN_MASK    0x00FFFFFF    /* Num of RxBDs in this ring */
#define TSEC_RQPRM_FBTHR_SHIFT 24

/* Extracted L2 cache write type (ELCWT) */
#define TSEC_ATTR_ELCWT_LOCK   0x00006000    /* Allocate and lock L2 cache */
#define TSEC_ATTR_ELCWT_L2     0x00004000    /* Allocate L2 cache line */

/* Buffer descriptor L2 cache write type (BDLWT) */
#define TSEC_ATTR_BDLWT_LOCK   0x00000C00    /* Allocate and lock L2 cache */
#define TSEC_ATTR_BDLWT_L2     0x00000800    /* Allocate L2 cache line */

/* TBI MII register */
#define TBI_CTRL_REG           0x00
#define TBI_STAT_REG           0x01
#define TBI_AN_ADVERTISEMENT   0x04
#define TBI_ANLPBPA            0x05
#define TBI_AN_EXPANSION       0x06
#define TBI_AN_NEXT_PAGE_TX    0x07
#define TBI_ANLPANP            0x08
#define TBI_EXTENDED_STATUS    0x0F
#define TBI_JITTER_DIAGNOSTICS 0x10
#define TBI_CONTROL            0x11

/* Control register */
#define TBI_PHY_RESET          0x8000
#define TBI_AN_ENABLE          0x1000
#define TBI_RESET_AN           0x0200
#define TBI_FULL_DUPLEX        0x0100

/* TBI Control register */
#define TBI_SOFT_RESET         0x8000
#define TBI_DISABLE_RX_DIS     0x2000
#define TBI_DISABLE_TX_DIS     0x1000
#define TBI_AN_SENSE           0x0100
#define TBI_CLOCK_SELECT       0x0020

#define AUTO_NEG_TIMEOUT       15000   /* Re-auto negotiation 500-1000ms */
#define RX_TIMEOUT             0x300   /* 8 port switch Rx timeout - 700 ms */
#define MII_OP_TIMEOUT         200     /* SMI operation timeout in ms */

#define PORT0                  0
#define PORT1                  1

#define LINKON                 1
#define LINKOFF                2
#define SPEEDON                3
#define SPEEDOFF               4
#define DUPLEXON               5
#define DUPLEXOFF              6
#define NOT_AVAIL             -1

#define MAC_ADDR_SIZE          6
#define CRC_SIZE               4
#define NUM_TX_BD             10    /* Number of Tx BD for loopback */
#define NUM_RX_BD             10    /* Number of Rx BD for loopback */
#define NUM_TX_BD_RINGS        8    /* Number of Tx BD rings */
#define NUM_RX_BD_RINGS        8    /* Number of Rx BD rings */
#define MAX_TX_BUF          1536    /* Maximum Tx Buf size (0x600) */
#define MAX_RX_BUF          1536    /* Maximum Rx Buf size (0x600) */
#define ENET_FRAME_LEN_MIN    64
#define ENET_FRAME_LEN_MAX  1518

/* MIIMCFG[MgmtClk] table converter lookup */
typedef struct mgmt_clk {
    uint min;        /* Low range of SMI MgmtClk */
    uint max;        /* High range of SMI MgmtClk */
    uint mgmt_clk;   /* MIICFG_DIVIDE */
} mgmt_clk_t;

/* init.c */
extern void *malloc_nm (unsigned long nbytes);

/* p1021_etsec.c */
extern int  check_tsec_tx_status(volatile tsec_bd_t *);
extern int  check_tsec_rx_status(volatile tsec_bd_t *);
extern int  check_tsec_rx_frame(volatile tsec_bd_t *,volatile tsec_bd_t *);
extern int  etsec_adjust_link(int, int, int, int);
extern int  etsec_get_info_ptr(int);
extern void etsec_get_mac_addr(int, char *);
extern int  etsec_get_rxbd(tsec_info_struct_t *);
extern int  etsec_get_txbd(tsec_info_struct_t *);
extern int  etsec_init(int, int, int, boolean);
extern int  etsec_lpbk_test(int, int, int, boolean);
extern int  etsec_mac_lpbk_wrapper(int, int);
extern int  etsec_recv(int, tsec_bd_t *, int);
extern int  etsec_recv_frame_ready(int, int);
extern int  etsec_send(int, tsec_bd_t *);
extern void etsec_start(int, boolean);
extern int  etsec_stop(int);
extern int  send_eth_message(int, uchar *, ushort);
extern int  test_set_debug_flag(void);

/* p1021_eth_frames.c */
extern int  build_eth_frame(fe_packet_t *, mac_addr_t,
                            mac_addr_t, uint16);
extern void build_eth_header(fe_packet_t *, mac_addr_t, mac_addr_t, uint16);
extern int  cleanup_tsec(int);
extern int  eth_intr_lpbk_test(int);
extern int  eth_msg_test(int);
extern int  eth_poll_lpbk_test(int);
extern int  init_tx_rings(tsec_info_struct_t *);
extern int  init_rx_rings(tsec_info_struct_t *);
extern int  rx_eth_pkt(int, int, int);
extern int  tx_eth_pkt(int, eth_tx_pkt_t *);
extern int  etsec_ext_lpbk_test(int, int, int, int, int);
extern int  cleanup_tsec (int etsec_num);

#endif /* #ifndef __P1021_ETSEC_H__ */

/*------------------------------------------------------------------------------
 * $Log: p1021_etsec.h,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.3  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.2  2011/08/18 19:43:23  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.3  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.2  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.1  2011/05/02 23:33:22  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */


