/* $Id: diag_console_util.h,v 1.2 2016/04/20 11:25:28 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_console_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_console_util.h - Header file for Console Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_CONSOLE_UTIL__
#define __DIAG_CONSOLE_UTIL__

#define PICOCOM_CMD_LENGTH          (64)
/* ttyS2 connect to BMC UART1 */
/* ttyS3 connects between BMC virtual UART and Intel LPC UART */
#define UART_TTYS2_DEV                "/dev/ttyS2"
#define UART_TTYS3_DEV                "/dev/ttyS3"
#define BAUD9600                    "-b9600"
#define BAUD115200                  "-b115200"

extern int diag_console_util(void);
extern int diag_uart_to_nim_cnnt(int);
extern int diag_uart_to_isp_cnnt(void);

#endif /* __DIAG_CONSOLE_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_console_util.h,v $
Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.4  2016/03/08 03:07:07  jimmyya
Add ISP testcard uart test

Revision 1.1.2.3  2015/09/26 05:20:42  alpeng
update console switch utils for intel

Revision 1.1.2.2  2015/09/04 01:45:44  alpeng
update console swtich to use ttyS2(BMC UART1)

Revision 1.1.2.1  2015/07/12 06:52:45  tirawan
Add Console Switch Utility, SPI driver and FPGA programming


$Endlog$
*/
