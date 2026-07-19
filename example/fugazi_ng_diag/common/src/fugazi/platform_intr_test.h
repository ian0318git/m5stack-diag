/* $Id: platform_intr_test.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_intr_test.h,v $
 *------------------------------------------------------------------
 * Filename:  header file for platform_intr_test.c
 *
 *
 * Copyright (c) 2013-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_INTR_TEST_H__
#define __PLATFORM_INTR_TEST_H__

struct intr_t {
    
    void (*enable)(int);
    void (*raise)(int);
    void (*clr)(int);
    void (*disable)(int);
    void (*hndlr)(int, void*);
    void *dev;
    unsigned int irq;
    unsigned int flag;
    char name[20];
};

enum {
    INTR_SFP0 = 0,
    INTR_SFP1,
    INTR_SFP2,
    INTR_SFP3,
    INTR_I2C0,
    INTR_I2C1,
    INTR_I2C2,
    INTR_I2C3,
    INTR_I2C4,
    INTR_I2C5,
    INTR_I2C6,
    INTR_I2C7,
    INTR_I2C8,
    INTR_I2C9,
    INTR_I2C10,
    INTR_I2C11,
    INTR_I2C12,
    INTR_I2C13,
    INTR_I2C14,
    INTR_I2C15,
    INTR_I2C16,
    INTR_UART0,
    INTR_UART1,
    INTR_UART2,
    INTR_UART3,
    INTR_UART4,
    INTR_UART5,
    INTR_UART6,
    INTR_UART7,
    INTR_UART8,
    INTR_OIR_SM1,  /* 30 */
    INTR_OIR_SM2,
    INTR_OIR_SM3,
    INTR_OIR_SM4,
    INTR_OIR_WIC1,
    INTR_OIR_WIC2,
    INTR_OIR_WIC3,
    INTR_OIR_SATA,
    INTR_UART_CONSOLE,
    INTR_ENV_MCU,
    INTR_VM_MCU,
    INTR_PWR,
    INTR_NIOS_CP,
    INTR_EXT_ENV,  /*max1617*/
    INTR_POE_DC,   /* poe daughter card */
    INTR_POE2_OUTPUT, /* poe psu 2 output ok */
    INTR_POE2_PRES,   /* poe psu 2 present */
    INTR_POE1_OUTPUT, /* poe psu 1 output ok */
    INTR_POE1_PRES, /* poe psu 1 present */
    INTR_DMA, /* DMA */
    INTR_MAX
};

#define POE_PSU_INTR_NUM 5

/* Externs */
extern void platform_init_intr(void);
extern int  free_irq(int, void *);
extern int  request_irq(int, void (*handler)(int, void *), int, void *);
extern void hts_intr(void);
extern int fpga_intr_test(int);
#endif   /* __PLATFORM_INTR_TEST_H__ */


/*-------------------------------------------------
 * $Log: platform_intr_test.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.3  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
