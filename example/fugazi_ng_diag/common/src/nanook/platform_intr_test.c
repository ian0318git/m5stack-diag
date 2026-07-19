/* $Id: platform_intr_test.c,v 1.2 2019/12/11 10:10:34 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_intr_test.c,v $
 *------------------------------------------------------------------
 * Filename:  platform_intr_test.c
 *            interrupt test for different types of interrrupt
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
//#include "platform_sensor.h"
#include "error.h"
#include "ngio.h"

#define MAX_ENTRY 14

//static  pthread_t threads;
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
#if 0
    printf("1) i2c interrupt\n");
    printf("2) uart interrupt\n");
    if ( !is_usd_machines() ) {
        /* Utah has no Env MCU */
        printf("3) env MCU interrupt\n");
        printf("4) voltage monitor interrupt\n");
    }
    printf("5) Module OIR SM1 interrupt\n"); 
    /* not support so far */
    /*
        printf("5) uart console multiplex interrupt\n");
        printf("6) quad phy interrupt\n");
        printf("7) sp prom interrupt\n");
        printf("8) NIOS spi prom interrupt\n");
    */
#endif
    printf("8) SFP interrupt\n");
//    printf("9) Max1617 interrupt\n");
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
#ifndef UTAH
    case 3:
        irq = INTR_ENV_MCU;
        break;
    case 4:
        irq = INTR_VM_MCU;
        break;
#endif
    case 5:
        irq = INTR_OIR_SM1;  //pfix  
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

//    printf("platform_intr_test.c:  irq %d; line=%d\n", irq, __LINE__);
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
            } else
                if ((irq >= INTR_OIR_SM1) && (irq<MAX_SM+INTR_OIR_SM1)) {
                    dev_no = irq - INTR_OIR_SM1;
                    if (dev_no == 0)
                        dev_no = FPGA_OIR_NGSM1;
                    else if (dev_no == 1) 
                        dev_no = FPGA_OIR_NGSM2;
                    else if (dev_no == 2)
                        dev_no = FPGA_OIR_NGSM3;
                    else 
                        dev_no = FPGA_OIR_NGSM4;
                } else
                    if ((irq >= INTR_OIR_WIC1) && (irq<MAX_SM+INTR_OIR_WIC1)) {
                        dev_no = irq - INTR_OIR_WIC1;
                        if (dev_no == 0)
                            dev_no = FPGA_OIR_NGWIC1;
                        else if (dev_no == 1)
                            dev_no = FPGA_OIR_NGWIC2;
                        else 
                            dev_no = FPGA_OIR_NGWIC3;
                    } else
                        if (irq == INTR_VM_MCU) {
                            dev_no = FPGA_MISC_VM_MCU;
                        } else
                            if (irq == INTR_ENV_MCU) {
                                dev_no = FPGA_MISC_ENV_MCU;
                            } else 
                            	if (irq == INTR_EXT_ENV) {
                                    dev_no = FPGA_ENV_INTR;  /* max1617*/
                } else
                    if ((irq >= INTR_POE_DC) && (irq<POE_PSU_INTR_NUM+INTR_POE_DC)) {
                       dev_no = irq - INTR_POE_DC;
                       if (dev_no == 0)
                           dev_no = POE_DC;
                       else if (dev_no == 1)
                           dev_no = POE_PSU2_OUTPUT_OK;
                       else if (dev_no == 2)
                           dev_no = POE_PSU2_PRESENT;
                       else if (dev_no == 3)
                           dev_no = POE_PSU1_OUTPUT_OK;
                       else 
                           dev_no = POE_PSU1_PRESENT;
                            
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
int
request_irq (int irq,  void (*handler)(int, void *), int flags, void *p)
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
int
free_irq (int irq, void *p)
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

#if 0
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
static
void sfp_intr (void)
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
static
void *rx_intr (void *argument)
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
        
        if (sts & FPGA_SFP_INTR) {
            sfp_intr();
        }
    }    
    return (void*)NULL;
}
#endif

/*-------------------------------------------------------------------
 * Function: platform_init_intr
 *
 * initialize all functino pointers. call once.
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
void
platform_init_intr (void)
{
    int irq, ii;
    volatile unsigned int *dp;

    memset(intr, 0, sizeof(intr));
    
#if 0
    /* create thread to block waiting for interrupt */
    if(pthread_create(&threads, NULL, rx_intr, (void *)NULL)) {
        printf("pthread_create failed \n");
        exit(-1);
        return;
    }
#endif
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
#if 0
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

static void
force_intr_hndlr (int irq, void *arg)
{
    printf("\n*******force_intr_hndlr irq=%d, count %d; addr arg %p****\n", irq,
           *(unsigned int *)arg, arg);
}
#endif
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
int
platform_intr_test (int dummy)
{
    unsigned int icount, dev_no;
    int          irq, rc = FAILED;
    icount = 0;

    ovrd = 1;
    while (1) {

        irq = show_menu();
        if ((irq < 0)) {
            printf("exit from force interrupt test menu\n");
            goto done;
        }
    
        /* SFP interrupt must select SFP ports number */
        printf("0) SPF0 interrupt\n");
        printf("1) SPF1 interrupt\n");
        irq = getdec_answer("Enter bit value: ", 0, 0, 1);
 
        /*genearate interrupt here. driver will disable intr in fpga
          config space. uio_read() will re-enable interrupt. */
        icount = 0;
        force_irq = irq;
        request_irq(irq, sfp_intr_hndlr, 0, (void *)&icount);
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
		
		if (check_sfp_int_sts(dev_no)) {
			rc = PASSED;
		}
		
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
    return rc;
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
int
platform_ser_irq_intr_test (int dummy)
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

/*------------------------------------------------------------------
$Log: platform_intr_test.c,v $
Revision 1.2  2019/12/11 10:10:34  lucywang
Merged Nanook to main trunk


$Endlog$
*/
