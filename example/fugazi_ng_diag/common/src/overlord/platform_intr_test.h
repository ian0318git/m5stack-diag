/* $Id: platform_intr_test.h,v 1.5 2019/08/06 06:56:10 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_intr_test.h,v $
 *------------------------------------------------------------------
 * Filename:  header file for platform_intr_test.c
 *
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
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
    INTR_OIR_PIM, 
    INTR_MAX
};

#define POE_PSU_INTR_NUM 5

/* Externs */
extern void platform_init_intr(void);
extern int  free_irq(int, void *);
extern int  request_irq(int, void (*handler)(int, void *), int, void *);
extern void hts_intr(void);
#endif   /* __PLATFORM_INTR_TEST_H__ */

/*------------------------------------------------------------------
$Log: platform_intr_test.h,v $
Revision 1.5  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.4.2.1  2018/12/28 03:30:23  alpeng
update intr util

Revision 1.4  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.3.54.1  2016/10/18 18:58:55  alpeng
support sm3 and sm4, update intr table

Revision 1.3  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.2  2013/07/22 19:37:03  mcharon
move hts to utah dir/add platform_stub

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.5  2013/01/25 05:50:19  alpeng
supported poe psu interrupt

Revision 1.4  2012/09/19 09:20:45  alpeng
support OIR SM1 interrupt test

Revision 1.3  2012/06/05 11:44:37  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
