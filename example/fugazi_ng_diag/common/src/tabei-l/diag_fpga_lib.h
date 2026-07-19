 /* $Id: diag_fpga_lib.h,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_fpga_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_fpga_lib.h
 * Description: Header file of FPGA Library
 * 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_FPGA_LIB_H__
#define __DIAG_FPGA_LIB_H__

#define FPGA_SIZE             0x10000
#define FPGA_MMAP_OFFSET      0xFED40000
#define DRV_NAME              "uio_fpga_dash"

#define SMART_FAN_ENABLE      0x300
#define UIO_SIZE_16MB         1024 * 1024 * 16

extern int        fpga_read_reg(uint, uint *);
extern int        fpga_write_reg(uint, uint);
extern int fpga_register_operation(uint , uint , uint);
extern int        fpga_reset_api(uint, uint, uint, uint);
extern void       tabei_fpga_base_addr_init(void);
extern int        uio_open(void);
extern unsigned long get_platform_aikido_addr (void);
extern void dash_uart_reset(int port);
extern int dash_uart_tx (int port, int test_speed, char* tx_str, int test_sz, int);
extern int dash_uart_rx(int port, int *, char* rx_str);

#endif  /*  __DIAG_FPGA_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_fpga_lib.h,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.6  2019/09/27 07:57:23  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.5  2019/02/22 06:43:03  olin2
 * Add Reva on Tabei-L
 *
 * Revision 1.1.2.4  2018/12/05 06:50:34  olin2
 * initial commit for Aikido
 *
 * Revision 1.1.2.3  2018/11/16 05:42:09  olin2
 * Clean up code
 *
 * Revision 1.1.2.2  2018/10/15 09:31:32  kodko
 * Porting FPGA UIO driver read/write function.
 *
 * Revision 1.1.2.1  2018/10/02 01:49:58  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
