/* $Id: diag_fpga_lib.h,v 1.5 2020/02/04 08:49:42 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_fpga_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_fpga_lib.h
 * Description: Header file of FPGA Library
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_FPGA_LIB_H__
#define __DIAG_FPGA_LIB_H__

#define FPGA_SIZE     0x10000
#define NUTELLA_FPGA_DEV "fpga"
#define FPGA_MMAP_OFFSET 0xFED40000
#define FPGA_REGISTER_OPERATION_MODE  1
#define IRQ_ENABLE   1
#define IRQ_DISABLE  0
#define IRQ_MAX_POLLING   10
#define WAIT_FOR_IRQ      50
#define FPGA_DEVICE       "/dev/fpga"
#define TEST_PATTERN_SIZE   5

extern int fpga_read_reg(uint, uint *);
extern int fpga_write_reg(uint, uint);
extern int fpga_reset_api(uint, uint, uint, uint);
extern unsigned long get_platform_aikido_addr(void);
extern int fpga_register_operation(uint, uint, int);
extern int fpga_version_is_more_than_2p0(void);
extern int fpga_version_is_more_than_3p0(void);
extern int eobc_or_packet_interrupt_test(int);
extern int enable_irq_mask(int, int, uint);
extern int setup_reset_ctl_reg(int, int);
extern int check_int_irq_stat(int);
extern int fpga_check_serirq(int, int);
extern int toggle_driver_irq_flag(int);
extern int fpga_clear_cpu_serirq_status(void);
extern int fpga_test_pattern_test(uint, uint*, int, uint);
extern int fpga_bit_to_check(uint, int, int);

enum
{
    MASK_ENABLE = 0,
    MASK_DISABLE
};

enum
{
    READY = 0,
    NOT_READY
};

enum
{
    LAST_BYTE = 0,
    LAST_FOUR_BIT,
    SAME,
    EXPECTED
};

typedef enum {
    IOCTL_RD_CMD,
    IOCTL_WR_CMD,
} ioctl_reg_t;

#endif  /*  __DIAG_FPGA_LIB_H__ */

/*-------------------------------------------------
$Log: diag_fpga_lib.h,v $
Revision 1.5  2020/02/04 08:49:42  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
