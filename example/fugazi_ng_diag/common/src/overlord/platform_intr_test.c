/* $Id: platform_intr_test.c,v 1.15 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_intr_test.c,v $
 *------------------------------------------------------------------
 * Filename:  platform_intr_test.c
 *            interrupt test for different types of interrrupt
 *
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
#include "platform_sensor.h"
#include "error.h"
#include "ngio.h"

#define MAX_ENTRY 14

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
    if ( !is_usd_machines() ) {
        /* Utah has no Env MCU */
        printf("3) env MCU interrupt\n");
        printf("4) voltage monitor interrupt\n");
    }
    if ( !is_curie_1ru() || !is_curie_2ru()) {
        printf("8) SFP interrupt\n");
        printf("9) Max1617 interrupt\n");
    }
    printf("10) Module OIR SM1 interrupt\n");
    printf("11) Module OIR SM2 interrupt\n");
    printf("12) Module OIR NIM1 interrupt\n");
    printf("13) Module OIR NIM2 interrupt\n");
    printf("14) Module OIR NIM3 interrupt\n");
    printf("15) Module OIR PIM interrupt\n");
    irq = getdec_answer("Enter bit value: ", 0, 0, 20);
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
    case 10:
        irq = INTR_OIR_SM1;
        break;
    case 11:
        irq = INTR_OIR_SM2; 
        break;
    case 12:
        irq = INTR_OIR_WIC1; 
        break;
    case 13:
        irq = INTR_OIR_WIC2;  
        break;
    case 14:
        irq = INTR_OIR_WIC3;
        break;
    case 15:
        irq = INTR_OIR_PIM;
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
                            
                } else
                    if (irq == INTR_OIR_PIM) { 
                       dev_no = FPGA_OIR_PIM; 
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
void
psu_intr (void)
{
    printf("PSU interrupt handler is not implemented ..\n");
}

/*-------------------------------------------------------------------
 *
 * Function: poe_psu_intr
 *
 * invoke intrrupt handler for poe psu interrupt.
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
void
poe_psu_intr (void)
{
    unsigned long sts = get_platform_poe_psu_intr_stat();

    printf("%s sts = %ld \n", __FUNCTION__, sts); 

    if ((sts & POE_DC) && (intr[INTR_POE_DC].hndlr) ) {
        intr[INTR_POE_DC].hndlr(INTR_POE_DC, (void *)intr[INTR_POE_DC].dev);
    }
    if ((sts & POE_PSU2_OUTPUT_OK) && (intr[INTR_POE2_OUTPUT].hndlr) ) {
        intr[INTR_POE2_OUTPUT].hndlr(INTR_POE2_OUTPUT , (void *)intr[INTR_POE2_OUTPUT].dev);
    }
    if ((sts & POE_PSU2_PRESENT) && (intr[INTR_POE2_PRES].hndlr) ) {
        intr[INTR_POE2_PRES].hndlr(INTR_POE2_PRES, (void *)intr[INTR_POE2_PRES].dev);
    }
    if ((sts & POE_PSU1_OUTPUT_OK) && (intr[INTR_POE1_OUTPUT].hndlr) ) {
        intr[INTR_POE1_OUTPUT].hndlr(INTR_POE1_OUTPUT, (void *)intr[INTR_POE1_OUTPUT].dev);
    }
    if ((sts & POE_PSU1_PRESENT) && (intr[INTR_POE1_PRES].hndlr) ) {
        intr[INTR_POE1_PRES].hndlr(INTR_POE1_PRES, (void *)intr[INTR_POE1_PRES].dev);
    }
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
void
env_intr (void)
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
static
void misc_intr (void)
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
static
void oir_intr (void)
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

    if (is_neptune() || is_vg450()) {
        if ((sts & FPGA_OIR_NGSM3) && (intr[INTR_OIR_SM3].hndlr) ) {
            intr[INTR_OIR_SM3].hndlr(INTR_OIR_SM3,
           (void *)intr[INTR_OIR_SM3].dev);
        }
        if ((sts & FPGA_OIR_NGSM4) && (intr[INTR_OIR_SM4].hndlr) ) {
            intr[INTR_OIR_SM4].hndlr(INTR_OIR_SM4,
           (void *)intr[INTR_OIR_SM4].dev);
        }
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
 * Function: i2c_intr
 *
 * i2c interrupt hndler
 *
 * INPUT : NONE
 * OUTPUT: NONE
 *------------------------------------------------------------------
 */
static
void i2c_intr (void)
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
        
        if (sts & FPGA_PWR_SUPPLY_INTR) {
            poe_psu_intr();
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
void
platform_init_intr (void)
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

    clr_all_oir_intr();
    request_irq(INTR_OIR_SM1,  oir_sm1_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_OIR_SM2,  oir_sm2_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_OIR_WIC1,  oir_wic1_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_OIR_WIC2,  oir_wic2_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_OIR_WIC3,  oir_wic3_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_OIR_SATA,  oir_sata_intr_hndlr, 0, (void *)NULL);
    if (is_neptune() || is_vg450()) {
        request_irq(INTR_OIR_SM3,  oir_sm3_intr_hndlr, 0, (void *)NULL);
        request_irq(INTR_OIR_SM4,  oir_sm4_intr_hndlr, 0, (void *)NULL);
    }

    /* oir interrupt SM/WIC */
    for (ii = 1, irq = INTR_OIR_SM1; irq < INTR_OIR_SM1+MAX_SM; irq++, ii++) {
        intr[irq].enable = enable_platform_sm_oir_intr;
        intr[irq].disable = disable_platform_sm_oir_intr;
        intr[irq].raise = enable_platform_sm_oir_override_intr;
        intr[irq].irq = irq;
        sprintf(intr[irq].name, "ngiosm%d", ii);
        intr[irq].enable(ii);
    }
    for (ii = 1, irq = INTR_OIR_WIC1; irq < INTR_OIR_WIC1+MAX_WIC; irq++, ii++) {
        intr[irq].enable = enable_platform_wic_oir_intr;
        intr[irq].disable = disable_platform_wic_oir_intr;
        intr[irq].raise = enable_platform_wic_oir_override_intr;
        intr[irq].irq = irq;
        sprintf(intr[irq].name, "ngiowic%d", ii);
        intr[irq].enable(ii);
    }

    /* PIM */
    intr[INTR_OIR_PIM].irq = INTR_OIR_PIM;
    intr[INTR_OIR_PIM].enable = enable_platform_pim_oir_intr;
    intr[INTR_OIR_PIM].disable = disable_platform_pim_oir_intr;
    intr[INTR_OIR_PIM].raise = enable_platform_pim_oir_override_intr;
    sprintf(intr[INTR_OIR_PIM].name, "PIM");

    /* sata */
    intr[INTR_OIR_SATA].irq = INTR_OIR_SATA;
    intr[INTR_OIR_SATA].enable = enable_platform_sata_oir_intr;
    intr[INTR_OIR_SATA].disable = disable_platform_sata_oir_intr;
    intr[INTR_OIR_SATA].raise = enable_platform_sata_oir_override_intr;
    sprintf(intr[INTR_OIR_SATA].name, "sata");


    /* psu and poe psu interrupts ..poe psu */
    request_irq(INTR_POE_DC,  poe_dc_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_POE2_OUTPUT,  poe2_output_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_POE2_PRES,  poe2_present_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_POE1_OUTPUT,  poe1_output_intr_hndlr, 0, (void *)NULL);
    request_irq(INTR_POE1_PRES,  poe1_present_intr_hndlr, 0, (void *)NULL);

    intr[INTR_POE_DC].irq = INTR_POE2_OUTPUT;
    intr[INTR_POE_DC].enable = enable_platform_poe_psu_intr;
    intr[INTR_POE_DC].disable = disable_platform_poe_psu_intr;
    intr[INTR_POE_DC].raise = NULL;
    sprintf(intr[INTR_POE_DC].name, "poe dc");

    intr[INTR_POE2_OUTPUT].irq = INTR_POE2_OUTPUT;
    intr[INTR_POE2_OUTPUT].enable = enable_platform_poe_psu_intr;
    intr[INTR_POE2_OUTPUT].disable = disable_platform_poe_psu_intr;
    intr[INTR_POE2_OUTPUT].raise = NULL;
    sprintf(intr[INTR_POE2_OUTPUT].name, "poe psu2 output");
    intr[INTR_POE2_PRES].irq = INTR_POE2_PRES;
    intr[INTR_POE2_PRES].enable = enable_platform_poe_psu_intr;
    intr[INTR_POE2_PRES].disable = disable_platform_poe_psu_intr;
    intr[INTR_POE2_PRES].raise = NULL;
    sprintf(intr[INTR_POE2_PRES].name, "poe psu2 present");

    intr[INTR_POE1_OUTPUT].irq = INTR_POE1_OUTPUT;
    intr[INTR_POE1_OUTPUT].enable = enable_platform_poe_psu_intr;
    intr[INTR_POE1_OUTPUT].disable = disable_platform_poe_psu_intr;
    intr[INTR_POE1_OUTPUT].raise = NULL;
    sprintf(intr[INTR_POE1_OUTPUT].name, "poe psu1 output");
    intr[INTR_POE1_PRES].irq = INTR_POE1_PRES;
    intr[INTR_POE1_PRES].enable = enable_platform_poe_psu_intr;
    intr[INTR_POE1_PRES].disable = disable_platform_poe_psu_intr;
    intr[INTR_POE1_PRES].raise = NULL;
    sprintf(intr[INTR_POE1_PRES].name, "poe psu1 present");
    /* end of psu and poe psu */


    /* external env interrupts ..max1617 */
    intr[INTR_EXT_ENV].irq = INTR_EXT_ENV;
    intr[INTR_EXT_ENV].enable = enable_platform_env_intr;
    intr[INTR_EXT_ENV].disable = disable_platform_env_intr;
    intr[INTR_EXT_ENV].raise = (void *)gen_snsr_alert;
    sprintf(intr[INTR_EXT_ENV].name, "ext env");

#ifndef UTAH
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
#endif

    /* misc interrupts ..uart console */
    intr[INTR_UART_CONSOLE].irq = INTR_UART_CONSOLE;
    intr[INTR_UART_CONSOLE].enable = enable_platform_uart_console_intr;
    intr[INTR_UART_CONSOLE].disable = disable_platform_uart_console_intr;
    intr[INTR_UART_CONSOLE].raise = enable_platform_uart_console_override_intr;
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
        /* 190624 - confirmed with HW, override is only applicable 
         * with top level interrupt status check.
         * Local interrupt status check, e.g. FPGA register 0x10000,
         * is not working anymore */
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

static void
force_intr_hndlr (int irq, void *arg)
{
    printf("\n*******force_intr_hndlr irq=%d, count %d; addr arg %p****\n", irq,
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
int
platform_intr_test (int dummy)
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
Revision 1.15  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.14  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.13  2019/06/26 08:49:42  alpeng
support side band signal test for neptune; remove local intr check for sfp, since fpga is not support anymore.

Revision 1.12  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.11.40.3  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.11.40.2  2016/12/28 07:41:57  alpeng
remove sm3 and sm4 intr check; otherwise it will be complaint on USD, which has not program SM3 and SM4 portions on FPGA

Revision 1.11.40.1  2016/10/18 18:58:55  alpeng
support sm3 and sm4, update intr table

Revision 1.11  2014/06/03 19:11:26  mcharon
change ulong to uint in env_intr function

Revision 1.10  2014/06/03 19:03:33  mcharon
add debug info to env_intr

Revision 1.9  2014/03/05 02:23:09  hroni
USD machines does not have env mcu. Remove platform_mcu.c and platform_mcu.h and cleanup the related code

Revision 1.8  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.7  2013/08/14 08:44:26  alpeng
support sfp interrupt

Revision 1.6  2013/08/08 00:40:44  hroni
fix irq index

Revision 1.5  2013/08/07 10:57:41  alpeng
support irq = 0, which is sfp0 interrupt

Revision 1.4  2013/08/07 09:53:57  alpeng
support SFP interrupt test and handler

Revision 1.3  2013/08/07 09:40:21  alpeng
support SFP interrupt test and handler

Revision 1.2  2013/07/22 19:37:03  mcharon
move hts to utah dir/add platform_stub

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.11  2013/01/25 05:50:19  alpeng
supported poe psu interrupt

Revision 1.10  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.9  2012/11/06 20:39:51  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.8  2012/09/20 00:13:01  mcharon
support oir

Revision 1.7  2012/09/19 09:20:45  alpeng
support OIR SM1 interrupt test

Revision 1.6  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.5  2012/09/13 18:25:08  mcharon
add serial irq test

Revision 1.4  2012/05/31 14:24:40  palin2
Clean up compile warnings.

Revision 1.3  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
