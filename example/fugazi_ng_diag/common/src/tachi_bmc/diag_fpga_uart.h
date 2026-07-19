/* $Id: diag_fpga_uart.h,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_uart.h,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_uart.h - Header file for FPGA UART functions
 *
 * June 2015, Alan Peng 
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_UART__
#define __DIAG_FPGA_UART__

extern int uart_lpbk_txrx(int, char*, int, char*, int *, int, int);

#endif /* __DIAG_FPGA_UART__ */

/*---------------------------------------------------------------
$Log: diag_fpga_uart.h,v $
Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/31 10:39:59  alpeng
first check in for testcard

$Endlog$
*/
