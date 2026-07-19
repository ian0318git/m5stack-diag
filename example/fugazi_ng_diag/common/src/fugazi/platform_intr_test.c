/* $Id: platform_intr_test.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_intr_test.c,v $
 *------------------------------------------------------------------
 * Filename:  platform_intr_test.c
 *            interrupt test for different types of interrrupt
 *
 *
 * Copyright (c) 2014-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "common.h"
#include "proto.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "platform_intr_test.h"
#include "uio_utils.h"
#include "error.h"
#include "ngio.h"

#define MAX_ENTRY 14
#define MAX_UART                  8


static  pthread_t threads;
static int ovrd = 0;
static int force_irq = 0;

#if defined (DUMP_FPGA)


struct fpga_rev {
    unsigned int debug_rev:8;
    unsigned int minor_rev:8;
    unsigned int major_rev:7;
    unsigned int debug:1;
    unsigned int pad:8;
    dp = regs + 0x0;
    lexdump((unsigned char *)dp, 0xff, "top");
    dp = regs + 0x32000;
    lexdump((unsigned char *)dp, 0xff, "bottom");
};

#endif

#if defined (DISABLE_NIOS)
/*
 * turn off NIOS.
 */
dp = regs + 0x08;
*dp |= (1 << 24);
#endif



static unsigned char const hexchars[] = "0123456789abcdef";
static struct intr_t intr[INTR_MAX+2];


#define tohexchar(c) (hexchars[(c)&0x0f])


/*-------------------------------------------------------------------
 *
 * Function: lexdump
 *
 * hex dump in little endia.
 *
 * INPUT : char *s: string to dump
 *         int l: how many bytes to dump
 *         char *t: message to display
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
void
lexdump (unsigned char *s, int l, char *t)
{
    register unsigned char *q;
    char line[4];
    int cnt;

    cnt = 0;
    q = s + l;
    while (s < q) {
        if ((cnt & 0xf) == 0) {
            if (cnt > 0)
                if (cnt > 0)
                    printf("\n");
            printf("%s: ", t);
        }
        line[0] = toupper(tohexchar(*s >> 4));
        line[1] = toupper(tohexchar(*s));
        line[2] = ' ';
        line[3] = 0;
        printf(line);
        cnt++;
        s++;
    }
    printf("\n");
}

/*-------------------------------------------------------------------
 *
 * Function: show_menu
 *
 * display option. 
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static int
show_menu (void)
{
    int irq;
    printf("0) quit\n");
    printf("1) i2c interrupt\n");
    printf("2) uart interrupt\n");
    printf("3) env MCU interrupt\n");
    printf("4) voltage monitor interrupt\n");
    /* not support so far */
    /*
        printf("5) uart console multiplex interrupt\n");
        printf("6) quad phy interrupt\n");
        printf("7) sp prom interrupt\n");
        printf("8) NIOS spi prom interrupt\n");
    */
    printf("8) SFP interrupt\n");
    printf("9) Max1617 interrupt\n");
    irq = getdec_answer("Enter bit value: ", 0, 0, 9);
    if (!irq) {
        return -1;
    }
    switch (irq) {
    case 1:
        irq = INTR_I2C0;
        break;
    case 2:
        irq = INTR_UART0;
        break;
    case 3:
        irq = INTR_ENV_MCU;
        break;
    case 4:
        irq = INTR_VM_MCU;
        break;
    case 5:
        break;
    case 6:
    case 7:
    case 8:
        irq = INTR_SFP0;
        break;
    case 9:
        irq = INTR_EXT_ENV;
        break;
    default:
        printf("\n\n*** not suported***\n\n");
        irq = -1;    	
        break; 
    }

    printf("platform_intr_test.c:  irq %d; line=%d\n", irq, __LINE__);
    return irq;
}

/*-------------------------------------------------------------------
 *
 * Function: get_device_no
 *
 * given irq number, return corresponding register bit.
 *
 * INPUT : irq number from enum list in platform_intr_test.h
 *         used by request_irq and free_irq
 * OUTPUT: retrun register bit
 *------------------------------------------------------------------
 */
static int
get_device_no (int irq)
{
    int dev_no = 0;
    if ((irq >= INTR_SFP0) && (irq<MAX_SFP+INTR_SFP0)) {
        dev_no = irq - INTR_SFP0;
    } else
        if ((irq >= INTR_I2C0) && (irq<MAX_SFP+INTR_I2C0)) {
            dev_no = irq - INTR_I2C0;
        } else
            if ((irq >= INTR_UART0) && (irq<MAX_UART+INTR_UART0)) {
                dev_no = irq - INTR_UART0;
                        
            } else {
                printf("irq = %d\n", irq);
                assert(!"get_device_no: unable to find dev_no ");
            }
    
    if (dev_no < 0) {
        printf("irq = %d\n", irq);
        assert(!"get_device_no incorreect irq");
    }

    return (dev_no+1);
}

/*-------------------------------------------------------------------
 *
 * Function: request_irq
 *
 * install user specified handler handler
 *
 * INPUT : irq, irq number from enum in platform_intr_test.h,
 *         handler:  pointer to isr
 *         flags: not used
 *         p: pointer to argument to be passed to isr handler
 * OUTPUT: passed
 *------------------------------------------------------------------
 */
int request_irq (int irq,  void (*handler)(int, void *), int flags, void *p)
{
    if (irq >= INTR_MAX || (irq < 0)) {
        printf("irq is %d\n", irq);
        assert(!"request_irq: invalide IRQ number!!!!!!\n");
    }

    intr[irq].hndlr = handler;
    intr[irq].dev = (void *)p;
    intr[irq].irq = irq;

    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: free_irq
 *
 * remove interupt handler
 *
 * INPUT : irq, irq number from enum in platform_intr_test.h,
 *         p: pointer to argument to be passed to isr handler
 * OUTPUT: passed
 *------------------------------------------------------------------
 */
int free_irq (int irq, void *p)
{
    int dev_no;

    dev_no = get_device_no(irq);
    if (intr[irq].disable)
        intr[irq].disable(dev_no);
    else {
        assert(!"free_irq: disable method not defined");
    }
    intr[irq].hndlr = NULL;
    intr[irq].dev = NULL;
    intr[irq].irq = 0;

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: psu_intr
 *
 * invoke intrrupt handler for psu interrupt.
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
void psu_intr (void)
{
    printf("PSU interrupt handler is not implemented ..\n");
}

/*-------------------------------------------------------------------
 *
 * Function: env_intr
 *
 * invoke intrrupt handler for enviornment interrupt.
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
void env_intr (void)
{
    unsigned int sts = get_platform_env_intr_stat();

    printf("function: %s sts = %#x \n", __FUNCTION__, sts); 
     
    if ((sts & EXT_ENV_INTR_EN) && (intr[INTR_EXT_ENV].hndlr) ) {
        intr[INTR_EXT_ENV].hndlr(INTR_EXT_ENV, (void *)intr[INTR_EXT_ENV].dev);
    }

}

/*-------------------------------------------------------------------
 *
 * Function: misc_intr
 *
 * invoke interrupt handler for misc interrupt.
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static void misc_intr (void)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *dev_intr = (fpga_intr_t *)addr;
    unsigned int sts = dev_intr->misc_sts;
    
    if ((sts & FPGA_MISC_ENV_MCU) && (intr[INTR_ENV_MCU].hndlr) ) {
        intr[INTR_ENV_MCU].hndlr(INTR_ENV_MCU, (void *)intr[INTR_ENV_MCU].dev);
    }

    if ((sts & FPGA_MISC_VM_MCU) && (intr[INTR_VM_MCU].hndlr) ) {
        intr[INTR_VM_MCU].hndlr(INTR_VM_MCU, (void *)intr[INTR_VM_MCU].dev);
    }

    if ((sts & FPGA_MISC_UART_CONSOLE) && (intr[INTR_UART_CONSOLE].hndlr) ) {
        
        intr[INTR_UART_CONSOLE].hndlr(INTR_UART_CONSOLE, (void *)intr[INTR_UART_CONSOLE].dev);
    }
    /* for uart, it's handled by kernel driver */

}

/*-------------------------------------------------------------------
 *
 * Function: oir_intr
 *
 * invoke interrupt handler for oir_intr
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static void oir_intr (void)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *top_intr = (fpga_intr_t *)addr;
    unsigned int sts = top_intr->oir_sts;

    if ((sts & FPGA_OIR_NGSM1) && (intr[INTR_OIR_SM1].hndlr) ) {
        intr[INTR_OIR_SM1].hndlr(INTR_OIR_SM1,
        (void *)intr[INTR_OIR_SM1].dev);
    }
    if ((sts & FPGA_OIR_NGSM2) && (intr[INTR_OIR_SM2].hndlr) ) {
        intr[INTR_OIR_SM2].hndlr(INTR_OIR_SM2,
        (void *)intr[INTR_OIR_SM2].dev);
    }
    if ((sts & FPGA_OIR_NGWIC1) && (intr[INTR_OIR_WIC1].hndlr) ) {
        intr[INTR_OIR_WIC1].hndlr(INTR_OIR_WIC1, (void *)intr[INTR_OIR_WIC1].dev);
    }
    if ((sts & FPGA_OIR_NGWIC2) && (intr[INTR_OIR_WIC2].hndlr) ) {
        intr[INTR_OIR_WIC2].hndlr(INTR_OIR_WIC2, (void *)intr[INTR_OIR_WIC2].dev);
    }
    if ((sts & FPGA_OIR_NGWIC3) && (intr[INTR_OIR_WIC3].hndlr) ) {
        intr[INTR_OIR_WIC3].hndlr(INTR_OIR_WIC3, (void *)intr[INTR_OIR_WIC3].dev);
    }
    
    if ((sts & FPGA_OIR_SATA) && (intr[INTR_OIR_SATA].hndlr) ) {
        intr[INTR_OIR_SATA].hndlr(INTR_OIR_SATA, (void *)intr[INTR_OIR_SATA].dev);
    }


}

/*-------------------------------------------------------------------
 *
 * Function: sfp_intr
 *
 * sfp interrupt handler. 
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static void sfp_intr (void)
{
    unsigned long sts;
    int irq, i;
    sts = get_platform_sfp_intr_sts();
    for (i = 0, irq = INTR_SFP0; irq < INTR_SFP0+MAX_SFP; irq++, i++) {
        if (sts & (1<<i)) {
            if (intr[irq].hndlr) {
                intr[irq].hndlr(irq, (void *)intr[irq].dev);
            } else {

            }
        }
    }
}

/*-------------------------------------------------------------------
 *
 * Function: i2c_intr
 *
 * i2c interrupt hndler
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static void i2c_intr (void)
{
    unsigned long sts;
    int i, irq;
    sts = get_platform_i2c_sts();
    for (i = 0, irq = INTR_I2C0 ; irq < INTR_I2C0 + MAX_I2C; i++, irq++) {
        if (sts & (1<<i)) {
            if (intr[irq].hndlr) {
                intr[irq].hndlr(irq, (void *)intr[irq].dev);
            } else {
                printf("\n***i2c%d interrupt..no handler installed****\n", i);
                fflush(stdout);
                exit(0);
            }
        }
    } /* for */
}

/*-------------------------------------------------------------------
 *
 * Function: rx_intr
 *
 * thread blocks until kernel receivs interrrupt. when kernel received interrupt,
 * this thread becomes unblocked. then the thread will invoke the handler that
 * has been installed previously, corresponding to the interrupt.
 *
 * INPUT : argument, not used
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static void *rx_intr (void *argument)
{
    unsigned int sts, icount = 0;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while (1) {
        uio_read(&icount);

        /* handle override interrupt differently (ovrd = 1) */
        if (ovrd) {
            //printf("force interrupt force_irq %d icount=%d; addr %p\n", force_irq, icount, &icount);
            if (intr[force_irq].hndlr) {
                *((unsigned int *)intr[force_irq].dev) = icount;
                //   intr[force_irq].hndlr(force_irq, (void *)&icount); //(void *)intr[force_irq].dev);
                intr[force_irq].hndlr(force_irq, ((void *)intr[force_irq].dev));
            }
            continue;
        }
        sts = get_platform_top_intr();
        
        if (sts & FPGA_PWR_SUPPLY_INTR) {
            psu_intr();
        }

        if (sts & FPGA_ENV_INTR) {
            env_intr();
        }
        
        if (sts & FPGA_SFP_INTR) {
            sfp_intr();
        }
         
        if (sts & FPGA_I2C_INTR) {
            i2c_intr();
        }

        if (sts & FPGA_MISC_INTR) {
            misc_intr();
        }
        
        if (sts & FPGA_OIR_INTR) {
            oir_intr();
        }

        //handle DMA related interrupts if any. 
        //        hts_intr();

    }    
    return (void*)NULL;
}


/*-------------------------------------------------------------------
 * Function: platform_init_intr
 *
 * initialize all functino pointers. call once.
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
void platform_init_intr (void)
{
    int irq, ii;
    volatile unsigned int *dp;

    memset(intr, 0, sizeof(intr));
    
    /* create thread to block waiting for interrupt */
    if(pthread_create(&threads, NULL, rx_intr, (void *)NULL)) {
        printf("pthread_create failed \n");
        exit(-1);
        return;
    }


    /* external env interrupts ..max1617 */
    intr[INTR_EXT_ENV].irq = INTR_EXT_ENV;
    intr[INTR_EXT_ENV].enable = enable_platform_env_intr;
    intr[INTR_EXT_ENV].disable = disable_platform_env_intr;
    //intr[INTR_EXT_ENV].raise = (void *)gen_snsr_alert;
    sprintf(intr[INTR_EXT_ENV].name, "ext env");

    /* misc interrupts ..env mcu */
    intr[INTR_ENV_MCU].irq = INTR_ENV_MCU;
    intr[INTR_ENV_MCU].enable = enable_platform_mcu_override_intr;
    intr[INTR_ENV_MCU].disable = disable_platform_mcu_intr;
    intr[INTR_ENV_MCU].raise = enable_platform_mcu_override_intr;
    sprintf(intr[INTR_ENV_MCU].name, "env");
    
    /* misc interrupts ..volatage monitor */
    intr[INTR_VM_MCU].irq = INTR_VM_MCU;
    intr[INTR_VM_MCU].enable = enable_platform_vm_mcu_override_intr;
    intr[INTR_VM_MCU].disable = disable_platform_vm_mcu_intr;
    intr[INTR_VM_MCU].raise = enable_platform_vm_mcu_override_intr;
    sprintf(intr[INTR_VM_MCU].name, "vm");

    /* misc interrupts ..uart console */
    intr[INTR_UART_CONSOLE].irq = INTR_UART_CONSOLE;
    //intr[INTR_UART_CONSOLE].enable = enable_platform_uart_console_intr;
    //intr[INTR_UART_CONSOLE].disable = disable_platform_uart_console_intr;
    //intr[INTR_UART_CONSOLE].raise = enable_platform_uart_console_override_intr;
    sprintf(intr[INTR_UART_CONSOLE].name, "uart_console");

    /* i2c */
    for (ii = 0, irq = INTR_I2C0; irq < INTR_I2C0+MAX_I2C; irq++, ii++) {
        intr[irq].enable = enable_platform_c2w_intr;
        intr[irq].disable = disable_platform_c2w_intr;
        intr[irq].raise = enable_platform_c2w_override_intr;
        intr[irq].irq = irq;
        sprintf(intr[irq].name, "i2c%d", ii);
    }

    /* uart */
    for (ii = 0, irq = INTR_UART0; irq < INTR_UART0 + MAX_UART; irq++, ii++) {
        intr[irq].enable = enable_platform_uart_intr;
        intr[irq].disable = disable_platform_uart_intr;
        intr[irq].raise = enable_platform_uart_override_intr;
        intr[irq].irq = irq;
        sprintf(intr[irq].name, "uart%d", ii);
    }

    /* sfp */
    for (ii = 0, irq = INTR_SFP0; irq < INTR_SFP0+MAX_SFP; irq++, ii++) {
        request_irq(irq,  sfp_intr_hndlr, 0, &ii);
        intr[irq].enable = enable_platform_sfp_intr;
        intr[irq].disable = disable_platform_sfp_intr;
        intr[irq].raise = enable_platform_sfp_override_intr;
        intr[irq].irq = irq;
        sprintf(intr[irq].name, "sfp%d", ii);
    }

    /* now enable main interrupt at pci --this is alter specific */
    dp = (volatile unsigned int *)(dash_fpga + 0xFF0050);
    *dp = 0xFFFFFFFF;

    

    return;
}

/*-------------------------------------------------------------------
 *
 * Function:  force_intr_hndlr
 *
 * handler for force interrupt
 *
 * INPUT : irq number from enum in platform_intr_test.h
 * INPUT : arg not used
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */

static void force_intr_hndlr (int irq, void *arg)
{
    printf("\n- force_intr_hndlr irq=%d, count %d; addr arg %p -\n", irq,
           *(unsigned int *)arg, arg);
}

/*-------------------------------------------------------------------
 *
 * Function:  platform_intr_test
 *
 * test interrupt
 *
 * INPUT : dummy not used
 * OUTPUT: always return PASSED. this is utility.
 *------------------------------------------------------------------
 */
int platform_intr_test (int dummy)
{
    unsigned int icount, dev_no;
    int          irq;
    icount = 0;

    ovrd = 1;
    while (1) {
         
        irq = show_menu();
        if ((irq < 0)) {
            printf("exit from force interrupt test menu\n");
            goto done;
        }
    
        /* SFP interrupt must select SFP ports number */
        if (irq == INTR_SFP0) {
            printf("0) SPF0 interrupt\n");
            printf("1) SPF1 interrupt\n");
            printf("2) SPF2 interrupt\n");
            printf("3) SPF3 interrupt\n");
            irq = getdec_answer("Enter bit value: ", 0, 0, 3);
        }
 
        /*genearate interrupt here. driver will disable intr in fpga
          config space. uio_read() will re-enable interrupt. */
        icount = 0;
        force_irq = irq;
        request_irq(irq, force_intr_hndlr, 0, (void *)&icount);
        dev_no = get_device_no(irq);
        if (intr[irq].enable)
            intr[irq].enable(dev_no);
        else {
            assert(!"enable function pointer not initialized\n");
        }

        if (intr[irq].raise) {
            intr[irq].raise(dev_no);
        } else {
            assert(!"raise function pointer not initialized\n");
        }

        msleep(10);
        if (intr[irq].disable) {
            intr[irq].disable(dev_no);
        } else {
            assert(!"disable function pointer not initialized\n");
        }
        free_irq(irq, NULL);
        printf("icount is %d\n", icount);
                
        /* we should keep getting interrupt. if not fpga has deaserted interrupt
           erroneously 
           for (i = 0; i< 4; i++) {
           uio_enable_intr();
           msleep(1);
           uio_select(&icount2, 1, 0);
           }
        */

    }
 done:
    ovrd = 0;
    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function:  fpga_intr_test
 *
 * test interrupt
 *
 * INPUT : dummy not used
 * OUTPUT: always return PASSED. this is utility.
 *------------------------------------------------------------------
 */
int fpga_intr_test (int dummy)
{
    unsigned int icount, dev_no;
    int          irq;
    icount = 0;

    ovrd = 1;
    irq = INTR_I2C0;

    /*genearate interrupt here. driver will disable intr in fpga
      config space. uio_read() will re-enable interrupt. */
    icount = 0;
    force_irq = irq;
    request_irq(irq, force_intr_hndlr, 0, (void *)&icount);
    dev_no = get_device_no(irq);
    if (intr[irq].enable)
        intr[irq].enable(dev_no);
    else {
        assert(!"enable function pointer not initialized\n");
    }

    if (intr[irq].raise) {
        intr[irq].raise(dev_no);
    } else {
        assert(!"raise function pointer not initialized\n");
    }

    msleep(10);
    if (intr[irq].disable) {
        intr[irq].disable(dev_no);
    } else {
        assert(!"disable function pointer not initialized\n");
    }
    free_irq(irq, NULL);
    printf("icount is %d\n", icount);
                
    ovrd = 0;
    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function: platform_ser_irq_intr_test 
 *
 * Test serial IRQ interrupt by writing to force interrupt register.
 * cpld.ko handles the interrupt and will keep track of number of interrupt
 * generated.
 * writing 1 to the /proc/dashcpld clears the interrupt count in the driver.
 * reading from /proc/dashcpld gets the interrupt count from driver.
 *
 * INPUT : dummy , not used
 * OUTPUT: passed or failed.
 *------------------------------------------------------------------
 */
int platform_ser_irq_intr_test (int dummy)
{
    int fp;
    char buf[4];
    unsigned int cnt;

    testname("serial irq interrupt");
    
    fp = open("/proc/dashcpld/irq", O_RDWR);
    if (fp < 0) {
        perror("can't open /proc/dashcpld/irq");
        cterr('f', 0, "can't open /proc/dashcpld/irq");
        return FAILED;
    }
    
    if (write(fp, "0", 1) < 0) {
        perror("can't write /proc/dashcpld/irq");
        cterr('f', 0, "can't write /proc/dashcpld/irq");
        return FAILED;
    }
    
    platform_irq0_test();
    
    /* wait for interupt handler to process */
    msleep(250);
    if (read(fp, &buf, sizeof(buf))<0) {
        perror("can't read /proc/dashcpld/irq");
        cterr('f', 0, "can't read /proc/dashcpld/irq");
        return FAILED;
    }

    cnt = atoi(buf);
    if (cnt < 2) {
        cterr('f', 0, "Did not receive interrupt");
        return FAILED;
    }

    prcomplete(testpass, errcount, 0);
    return PASSED;
}


/*-------------------------------------------------
 * $Log: platform_intr_test.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.7  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.6  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.5  2019/04/11 22:32:29  letsai
 * 1. Replace the sign "*" to "-" when doing FPGA interrupt test
 * 2. Fix M.2 combo test when slot is empty.
 * 3. Make "check link utility" easy to use.
 * 4. When USB console detected, check the corresponding FPGA register bit.
 *
 * Revision 1.1.6.4  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.3  2019/03/18 09:22:23  letsai
 * Fixed 1.Boot flash test 2.I2C scan test 3. FPGA interrupt test
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

