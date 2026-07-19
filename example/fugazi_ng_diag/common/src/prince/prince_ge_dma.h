/* $Id: prince_ge_dma.h,v 1.2 2013/06/25 08:01:42 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_ge_dma.h,v $
 *------------------------------------------------------------------
 * prince_ge_dma.h 
 *      Prince GE DMA definitions.
 *
 * Xiaoying Zhang -- Dec. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef _GE_DMA_H_
#define _GE_DMA_H_

/* Defines for bit operations */
#define setbit(_reg, _mask)                 (_reg |= (_mask))
#define clrbit(_reg, _mask)                 (_reg &= ~(_mask))
#define setbits(_reg, _mask, _off, _val)    (_reg = \
                                                (_reg & ~(_mask)) | \
                                                (((_val)<<(_off)) & (_mask)))
#define getbits(_reg, _mask, _off)          ((_reg & (_mask)) >>(_off))

#define GE_RETRY_MAX                    8000
#define GE_POLL_DELAY                   250
#define PRINCE_GE_PACKET_NUM            512

/* Defines for GE DMA Control Reg */
#define GE_DMA_RESET_MASK               0x80000000
#define GE_DMA_E_FIFO_FLUSH_MASK        0x10000000
#define GE_DMA_TX_RING1_PRI_MASK        0x001c0000
#define GE_DMA_AXI_MTU_MASK             0x00004000
#define GE_DMA_FLOW_CTRL_EN_MASK        0x00002000
#define GE_DMA_RX_EN_MASK               0x00000100
#define GE_DMA_TX2_EN_MASK              0x00000004
#define GE_DMA_TX1_EN_MASK              0x00000002
#define GE_DMA_TX0_EN_MASK              0x00000001

#define GE_DMA_E_FIFO_FLUSH_SHIFT       28
#define GE_DMA_TX_RING1_PRI_SHIFT       18
#define GE_DMA_AXI_MTU_SHIFT            14
#define GE_DMA_FLOW_CTRL_EN_SHIFT       13

/* Defines for GE Rx Flow Control Reg */
#define GE_DMA_XON_TRHD_MASK            0x03ff0000
#define GE_DMA_XOFF_TRHD_MASK           0x000003ff

#define GE_DMA_XON_TRHD_SHIFT           16
#define GE_DMA_XOFF_TRHD_SHIFT          0

/* Defines for GE DMA Rx Buffer Size Reg */
#define GE_DMA_RX_BUF_SIZE_MASK         0x00003fff
#define GE_DMA_RX_BUF_SIZE_SHIFT        0

/* Defines for GE Rx Dring Base/Head Reg */
#define GE_DMA_RX_BASE_OFFSET_MASK      0x0000e000
#define GE_DMA_RX_HEAD_MASK             0x00001ff8

#define GE_DMA_RX_BASE_OFFSET_SHIFT     13
#define GE_DMA_RX_HEAD_SHIFT            3

/* Defines for GE Rx Dring Add Reg */
#define GE_DMA_RX_ADD_MASK              0x000003ff
#define GE_DMA_RX_ADD_SHIFT             0

/* Defines for GE Tx Dring Base/Tail Reg */
#define GE_DMA_TX_BASE_OFFSET_MASK      0x0000f100
#define GE_DMA_TX_TAIL_MASK             0x000007f8

#define GE_DMA_TX_BASE_OFFSET_SHIFT     11
#define GE_DMA_TX_TAIL_SHIFT            3

/* Defines for GE Tx Dring Head Reg */
#define GE_DMA_TX_HEAD_MASK             0x000007f8
#define GE_DMA_TX_HEAD_SHIFT            3

/* Defines for GE Tx Dring Add Reg */
#define GE_DMA_TX_ADD_MASK              0x000000ff
#define GE_DMA_TX_ADD_SHIFT             0

/* Defines for RXBD */
#define RXBD_SIZE_MASK                  0x0000ffff
#define RXBD_DONE                       0x00010000
#define RXBD_FIRST                      0x00020000
#define RXBD_LAST                       0x00040000
#define RXBD_OVRN                       0x00080000
#define RXBD_DISCARD                    0x00100000

/* Defines for TXBD */
#define TXBD_SIZE_MASK                  0x00003fff
#define TXBD_DONE                       0x00010000
#define TXBD_FIRST                      0x00020000
#define TXBD_LAST                       0x00040000 
#define TXBD_INT                        0x00080000
#define TXBD_STATUS_MASK                0xfff00000

/* Defines for Interrupt control and status */
#define PRINCE_GE_FORCE_IRQ         0x00008000

#define PRINCE_GE_TX0_INTR_EN       0x00010000
#define PRINCE_GE_TX1_INTR_EN       0x00020000
#define PRINCE_GE_TX2_INTR_EN       0x00040000

#define PRINCE_GE_TX0_INTR_STS      0x00010000
#define PRINCE_GE_TX1_INTR_STS      0x00020000
#define PRINCE_GE_TX2_INTR_STS      0x00040000

#define PRINCE_GE_RX_INTR_EN        0x00000001
#define PRINCE_GE_RX_INTR_STS       0x00000001

#endif //_GE_DMA_H_

/******** History ********
$Log: prince_ge_dma.h,v $
Revision 1.2  2013/06/25 08:01:42  xiaoyizh
Add definitions for interrupt control and status.


$Endlog$
*/
