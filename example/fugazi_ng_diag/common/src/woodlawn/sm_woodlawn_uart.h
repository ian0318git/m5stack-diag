/* $Id: sm_woodlawn_uart.h,v 1.2 2013/10/08 08:48:26 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn_uart.h,v $
 *------------------------------------------------------------------
 * Filename: sm_woodlawn_uart.h
 *
 * Description: Header file of SM Woodlawn UART Library
 * Author: Times Huang
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __SM_WOODLAWN_UART__
#define __SM_WOODLAWN_UART__

#define WOODLAWN_UART_READ_TIMEOUT          (1) /* secs */

extern int woodlawn_rx_polling_uart(char *, char *, int);
extern int woodlawn_tx_uart(char *, char *);
extern int woodlawn_uart_setup(char *);

#endif /* __SM_WOODLAWN_UART__ */


/*------------------------------------------------------------------
 * $Log: sm_woodlawn_uart.h,v $
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/03 05:46:41  tirawan
 * Add auto boot by UART function, and auto run by nc utility
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
