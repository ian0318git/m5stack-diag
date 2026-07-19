 /* $Id: diag_uart_lib.h,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_uart_lib.h,v $
 *------------------------------------------------------------------
 * diag_uart_lib.h 
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DNV_UART_LIB_H__
#define __DNV_UART_LIB_H__

#define UART_READ_TIMEOUT          (1) /* secs */
#define WAIT_SCREEN_PRINT               (1000)

typedef struct uart_parm {
    char  *tty_dev; /* i.e. /dev/ttyS0, /dev/ttyS1,ﾡK */
    int    baudrate;
    int    databit;
    char  *parity; /* o=odd, e=even, n=none */
    char  *flow; /* s=soft, h=hard, n=none */
} uart_parm_t;

extern int diag_console_switch(struct uart_parm *);
extern int diag_uart_rx_polling (int, char *, int);
extern int diag_uart_tx(int, char *);
extern int diag_uart_setup(char *);

#endif /* __DIAG_UART_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_uart_lib.h,v $
 * Revision 1.2  2019/10/17 02:16:23  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.1  2018/10/02 01:50:01  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
