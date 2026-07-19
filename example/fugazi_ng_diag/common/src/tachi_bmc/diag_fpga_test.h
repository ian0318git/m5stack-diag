/* $Id: diag_fpga_test.h,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_test.h - Header file for FPGA Tests
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_TEST__
#define __DIAG_FPGA_TEST__

#define FPGA_INTR_TEST_TOUT                         (200)

#define FPGA_RW         (READ_WRITE | REG_ACCESS)

extern int diag_fpga_test(int);
extern int diag_sgpio_test(void);
#endif /* __DIAG_FPGA_TEST__ */

/*---------------------------------------------------------------
$Log: diag_fpga_test.h,v $
Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/03/07 07:15:50  benchen2
add sgpio test

Revision 1.1.2.2  2015/11/13 00:50:32  tirawan
Remove FPGA SGPIO and add FPGA Interrupt

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/
