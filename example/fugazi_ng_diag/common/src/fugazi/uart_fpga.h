/* $Id: uart_fpga.h,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/uart_fpga.h,v $
 *-----------------------------------------------------------------------------
 * File: uart_fpga.h
 *
 * March. 2011, mcharon
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */


#ifndef __UART_FPGA__
#define  __UART_FPGA__

#define DATA_MASK 0xFF

/* dash register offset */
#define DASH_UART_DLL                                0x00
#define DASH_UART_RX                                 0x00
#define DASH_UART_TX                                 0x00

#define DASH_UART_DLM                                0x04
#define DASH_UART_IER                                0x04

#define DASH_UART_IIR                                0x08
#define DASH_UART_FCR                                0x08

#define DASH_UART_LCR                                0x0C
#define DASH_UART_MCR                                0x10

#define DASH_UART_LSR                                0x14
#define DASH_UART_MSR                                0x18

#define DASH_UART_SCR                                0x1C

#define FPGA_UART_BASE                               0x20000
#define FPGA_UART_OFFSET                             0x100
//#define FPGA_UART8_BASE                              0x31400

#define MAX_UART                  8


/*
#define UART_RX        0
#define UART_IER    0x04
#define UART_IIR    0x08
#define UART_LCR    0x0c
#define UART_MCR    0x10
#define UART_LSR    0x14
#define UART_MSR    0x18
#define UART_SCR    0x1c

#define UART_TX     UART_RX
#define UART_FCR    UART_IIR
*/

typedef struct uart_t_ {
    volatile unsigned int dll;  /* 0 */
    volatile unsigned int dlm;  /* 4 ier*/
    volatile unsigned int fcr; /* 8 */
    volatile unsigned int lcr; /* c */
    volatile unsigned int mcr;/* 10 */
    volatile unsigned int lsr; /* 14 */
    volatile unsigned int msr; /* 18 */
    volatile unsigned int scr;
} uart_t;


#endif /* uart_fpga */



/*-------------------------------------------------
 * $Log: uart_fpga.h,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:29  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
