 /* $Id: diag_fpga_lib.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_fpga_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_fpga_lib.h
 * Description: Header file of FPGA Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_FPGA_LIB_H__
#define __DIAG_FPGA_LIB_H__

#define FPGA_SIZE     0x10000
#define VIPER_FPGA_DEV "fpga"
#define FPGA_MMAP_OFFSET 0xFED40000


extern int        fpga_read_reg(uint, uint *);
extern int        fpga_write_reg(uint, uint);
extern int        fpga_reset_api(uint, uint, uint, uint);

#endif  /*  __DIAG_FPGA_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_fpga_lib.h,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.2  2018/03/15 08:26:16  harrchan
 * Change I/O access to memory map
 *
 * Revision 1.1.2.1  2018/02/27 08:06:42  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
