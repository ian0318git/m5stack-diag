/* $Id: diag_fpga_test.h,v 1.2 2013/10/08 08:48:28 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_fpga_test.h,v $
 *------------------------------------------------------------------
 * diag_fpga_test.h 
 * 
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __DIAG_FPGA_TEST_H__
#define __DIAG_FPGA_TEST_H__

/* FPGA interrupt pin is wired to Cavium GPIO bit 10
 */
#define FPGA_INTR_GPIO10_MASK   0x00000400

extern int fpga_test(int);
extern int fpga_do_all_wrapper(void);

#endif
/*-------------------------------------------------
 * $Log: diag_fpga_test.h,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.6  2012/11/19 02:31:16  leslie
 * Change check gpio bit 3 to bit 10.
 *
 * Revision 1.5  2012/08/28 08:35:23  leslie
 * Update for fpga test item.
 *
 * Revision 1.4  2012/08/03 10:16:55  leslie
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/02/10 06:48:57  leslie
 * Add Woodlwan fpga test header file.
 * 
 * $Endlog$
 *-------------------------------------------------
 */
