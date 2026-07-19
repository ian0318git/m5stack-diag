/* $Id: duart16552.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/duart16552.h,v $
 *------------------------------------------------------------------
 * duart16552.h - 16552 DUART defines
 *
 * March 1996, Paul Tong
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __DUART16552_H__
#define __DUART16552_H__

#define DUART_FIFO_SIZE   16 /* 16 char deep */

/*
 * Defines for Interrupt Enable Register (IER), Write/Read
 */
#define RX_DATA_INTR_EN      0x01
#define TX_REG_EMP_INTR_EN   0x02
#define RX_LNSTAT_INTR_EN    0x04
#define MDM_STAT_INTR_EN     0x08

/*
 * Defines for Interrupt Identification Register (IIR), Read Only
 */
#define INTR_PENDING         0x01
#define INTR_SRC_ID          0x0e
#define FIFO_ENABLED         0xc0
#define MDM_INTR             0x00
#define TX_EMP_INTR          0x02
#define RX_DATA_INTR         0x04
#define RX_ERR_INTR          0x06
#define RX_FIFO_TMO_INTR     0x0c

/*
 * Defines for FIFO Control Register (FCR), Write Only
 */
#define FIFO_MODE_EN         0x01
#define RESET_RX_FIFO        0x02
#define RESET_TX_FIFO        0x04
#define SELECT_DMA_MODE      0x10
#define RX_TRIG_MASK         0xc0
#define RX_TRIG_1            0x00
#define RX_TRIG_4            0x40
#define RX_TRIG_8            0x80
#define RX_TRIG_14           0xc0

/*
 * Defines for Line Control Register (LCR), Write/Read
 */
#define CHAR_LEN_SEL         0x03
#define CHAR_LEN5            0x00
#define CHAR_LEN6            0x01
#define CHAR_LEN7            0x02
#define CHAR_LEN8            0x03
#define STOP_BIT1            0x00
#define STOP_BIT2            0x04
#define PARITY_OFF           0x00
#define PARITY_ON            0x08
#define EVEN_PARITY          0x10
#define STICK_PARITY         0x20
#define SEND_BREAK           0x40
#define DIVISOR_ACC          0x80

/*
 * Defines for Modem Control Register (MCR), Write/Read
 */
#define ASSERT_DTR           0x01
#define ASSERT_RTS           0x02
#define ASSERT_OUT1          0x04
#define ASSERT_OUT2          0x08
#define DIAG_LOOPBACK        0x10

/*
 * Defines for Line Status Register (LSR), Write/Read
 */
#define RX_DATA_READY        0x01
#define RX_OVERRUN_ERR       0x02
#define RX_PARITY_ERR        0x04
#define RX_FRAM_ERR          0x08
#define RX_BRK_INTR          0x10
#define TX_READY             0x20
#define TRANSMITTER_EMP      0x40
#define ERR_IN_RX_FIFO       0x80

/*
 * Defines for Modem Status Register (MSR), Write/Read
 */
#define CTS_DELTA            0x01
#define DSR_DELTA            0x02
#define RI_EDGE_DELTA        0x04
#define DCD_DELTA            0x08
#define CTS_ASSERTED         0x10
#define DSR_ASSERTED         0x20
#define RI_ASSERTED          0x40
#define DCD_ASSERTED         0x80

/*
 * Defines for Alternate Function Register (AFR), Write/Read
 */
#define CONCURRENT_WR        0x01
#define SEL_OUT2             0x00
#define SEL_BAUDOUT          0x02
#define SEL_RXRDY            0x04

#endif /* __DUART16552_H__ */
/* end of file */

/******** History ******** 
$Log: duart16552.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
