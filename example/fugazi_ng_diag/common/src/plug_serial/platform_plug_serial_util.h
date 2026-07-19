/* $Id: platform_plug_serial_util.h,v 1.4 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/platform_plug_serial_util.h,v $
 *------------------------------------------------------------------
 *
 * plug_serial_util.h - Header file for Pluggable Serial Utilities
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLATFORM_PLUG_SERIAL_UTIL__
#define __PLATFORM_PLUG_SERIAL_UTIL__

#define FOXCONN_PLUG_UART_TEST_TOUT         (200)

#define UART_TEST_DELAY                     (100)
#define UART_FLUSH_FIFO_TIMES               (50)
#define BAUD9600                            (9600)
#define WAIT_SCREEN_PRINT                   (1000)

typedef enum {
    OPT_READ,
    OPT_WRITE
} reg_util_opt_t;

extern int pluggable_serial_utils(void);
extern int pluggable_serial_uart_test (void);
extern int system(const char *); 
extern int ExecuteCmdbyPopen(char *, char *, int);


#endif

/******** History ********
$Log: platform_plug_serial_util.h,v $
Revision 1.4  2018/11/23 09:28:46  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.3.54.1  2018/10/15 06:51:13  hondwang
pluggable common code re-instruct modify code

Revision 1.3  2018/02/09 09:17:34  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:54:53  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:58:56  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.3  2017/08/22 03:29:59  lucywang
set 1000Base-X for pluggable serial and set sgmii for pluggable test card


$Endlog$
*/
