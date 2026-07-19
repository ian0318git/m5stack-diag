/* $Id: diag_uart_lib.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_uart_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_uart_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define PLAT_UART_READ_TIMEOUT          (1) /* secs */
#define WAIT_SCREEN_PRINT               (1000)
#define PLAT_AUX_UART_DEV_STR           "/dev/ttyS1"
#define PLAT_WIFI_UART_DEV_STR          "/dev/ttyS2"
#define PLAT_DSL_UART_DEV_STR           "/dev/ttyS1"

#define PLAT_UART_BUF_SIZE            1024
#define DIAG_PLAT_NC_TMP_PARMS_FILE         "/tmp/plat_nc.parms"
#define DIAG_PLAT_NC_RTN_PARMS_PORT_BASE                    (2288)
#define DIAG_PLAT_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE     (2291)
#define DIAG_PLAT_NC_EXECUTE_COMMAND_PORT_BASE              (2292)
#define DIAG_PLAT_NC_RET_EXEC_DONE_PORT                     (2293)

typedef struct uart_parm {
    char  *tty_dev; /* i.e. /dev/ttyS0, /dev/ttyS1,ﾡK */
    int    baudrate;
    int    databit;
    char  *parity; /* o=odd, e=even, n=none */
    char  *flow; /* s=soft, h=hard, n=none */
} uart_parm_t;

typedef struct plat_uart_ {
    char *dev;
    char buf[PLAT_UART_BUF_SIZE];
} plat_uart;

extern int plat_console_switch(struct uart_parm *);
extern int plat_uart_rx_polling (int, char *, int);
extern int plat_uart_tx(int, char *);
extern int plat_uart_setup(char *);
extern int  plat_tx_uart(char *, char *);
extern int  plat_rx_uart(char *, int, char *, int);
extern int  plat_rx_polling_uart(char *, char *, int);
extern void plat_nc_init_parms_file(void);
extern int plat_nc_get_parms(int, char *);

/*-------------------------------------------------
 * $Log: diag_uart_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
