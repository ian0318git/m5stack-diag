/* $Id: uart_api.h,v 1.2 2017/08/02 14:21:50 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/uart_api.h,v $
 *------------------------------------------------------------------
 * uart_api.h
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __UART_API__
#define __UART_API__

#define TSN_UART_READ_TIMEOUT          (1) /* secs */
#define WAIT_SCREEN_PRINT               (1000)
#define TSN_AUX_UART_DEV_STR           "/dev/ttyS1"
#define TSN_WIFI_UART_DEV_STR          "/dev/ttyS2"
#define TSN_DSL_UART_DEV_STR           "/dev/ttyS1"

typedef struct uart_parm {
    char  *tty_dev; /* i.e. /dev/ttyS0, /dev/ttyS1,ﾡK */
    int    baudrate;
    int    databit;
    char  *parity; /* o=odd, e=even, n=none */
    char  *flow; /* s=soft, h=hard, n=none */
} uart_parm_t;

extern int tsn_console_switch(struct uart_parm *);
extern int tsn_uart_rx_polling (int, char *, int);
extern int tsn_uart_tx(int, char *);
extern int tsn_uart_setup(char *);

#endif /* __UART_API__ */

/*-------------------------------------------------
$Log: uart_api.h,v $
Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:52  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.2  2016/04/22 09:48:43  leschen
Check in codes for xDSL bring up done.

Revision 1.1.2.1  2016/03/21 02:56:06  steja
Add debug card test items



*/
