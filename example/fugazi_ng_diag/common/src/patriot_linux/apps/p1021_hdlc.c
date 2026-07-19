/* $Id: p1021_hdlc.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: p1021_hdlc.c
 *
 * Description: This file is for HDLC over TDM interface with QE
 *
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "patriot_main.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "p1021_etsec.h"
#include "p1021_immap.h"
#include "ds3170.h"
#include "common_utils.h"

//#define DEBUG 1

#define MAX_HDLC_RX_BUF 2000
extern uchar err_msg[];
extern uchar dismem_msg[];
extern int patriot_framer_debug;
extern int hdlc_rx_frames;
volatile  char *ucc_param_ptr;
volatile qe_bd_t    *bd;
volatile qe_bd_t    *bd_phy;
static volatile unsigned long vir_ptr, phy_ptr;
static uchar riptr[32];
static uchar tiptr[32];

static volatile unsigned long g_txbuf;
static volatile unsigned long g_txbuf1;

static volatile unsigned long g_rxbuf;
static volatile unsigned long g_rxbuf1;


static volatile unsigned long g_txbuf_phy;
static volatile unsigned long g_txbuf1_phy;

static volatile unsigned long g_rxbuf_phy;
static volatile unsigned long g_rxbuf1_phy;

static volatile unsigned long g_rxbuf_dummy;
static volatile unsigned long g_txbuf_dummy;

static volatile unsigned long g_rxbuf_dummy_phy;
static volatile unsigned long g_txbuf_dummy_phy;

#define MURAM_BASE        (unsigned int *)&(REGB->qe.muram)

/**************************************************************************
 * Name: reset_qe
 *
 * Description: Reset QE
 *
 * Inputs: None
 *
 * Output: None
 *
 *************************************************************************/
void
reset_qe (void)
{
#ifdef DEBUG    
    printf("\nreset_qe\n");
#endif    
    /* reset QE */
    REGB->qe.cp.cecr = QE_CECR_RESET;
    asm volatile ("msync");

    /* global mode 1 for coherent */
    REGB->qe.sdma.sdmr |= QE_SDMR_GLB_1_MSK;
    asm volatile ("msync");
}
/**************************************************************************
 *
 * Function: qe_hdlc_show_regs
 * 
 * Description: Display the HDLC Registers
 *
 * Input : none
 *
 * Output: none
 *
 *************************************************************************/
void
qe_hdlc_show_regs (void)
{
     
    printf("\nDisplay HDLC Registers\n");
    printf("UPSMR     @address 0x%08x contains 0x%08x\n", 
           (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.upsmr,
	   REGB->qe.ucc3.mode.fast.reg.upsmr);
    
    printf("UCCE      @address 0x%08x contains 0x%08x\n", 
           (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.ucce,
	   REGB->qe.ucc3.mode.fast.reg.ucce);

    printf("UCCM      @address 0x%08x contains 0x%08x\n", 
           (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.uccm,
	   REGB->qe.ucc3.mode.fast.reg.uccm);

    printf("UCCS      @address 0x%08x contains 0x%08x\n", 
           (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.uccs,
	   REGB->qe.ucc3.mode.fast.reg.uccs);

    return;
}
    
/**************************************************************************
 *
 * Function: qe_hdlc_show_bd
 * 
 * Description: Display the HDLC Buffer Discriptors
 *
 * Input : none
 *
 * Output: none
 *
 *************************************************************************/
void
qe_hdlc_show_bd (void)
{
    qe_bd_t *tmd, *rmd, *tmd1, *rmd1;		 /* tx, rx buffer descr. */
    qe_fast_hdlc_param_t *hdlc_param_p;

    hdlc_param_p = (qe_fast_hdlc_param_t *)&REGB->qe.muram;
    
    rmd = (qe_bd_t *)bd;
    rmd1 = (qe_bd_t *)(bd + 1);
    
    tmd = (qe_bd_t *)((int)rmd + 2 * sizeof(qe_bd_t));
    tmd1 = (qe_bd_t *)(tmd + 1);
    
    printf("\n\nUCC HDLC Buffer Descriptor RAM:");
    printf("\n\nRx BD's :-");
    dismem((unsigned char *)(rmd), (sizeof(qe_bd_t)) * 2,
            (unsigned)(rmd), 4);
    printf("\nRx data :-");
    dismem((unsigned char *)vir_addr((unsigned long)rmd->buf_ptr), 0x40,
            (unsigned)vir_addr((unsigned long)rmd->buf_ptr), 4);

    printf("\n\nRx BD's :-");
    dismem((unsigned char *)(rmd1), (sizeof(qe_bd_t)) * 2,
            (unsigned)(rmd1), 4);
    printf("\nRx data :-");
    dismem((unsigned char *)vir_addr((unsigned long)rmd1->buf_ptr), 0x40,
            (unsigned)vir_addr((unsigned long)rmd1->buf_ptr), 4);
    
    
    printf("\n\nTx BD's :-");
    dismem((unsigned char *)(tmd), (sizeof(qe_bd_t)) * 2,
            (unsigned)(tmd), 4);
    printf("\nTx data :-");
    dismem((unsigned char *)vir_addr((unsigned long)tmd->buf_ptr), 0x40,
            (unsigned)vir_addr((unsigned long)tmd->buf_ptr), 4);

    printf("\n\nTx BD's :-");
    dismem((unsigned char *)(tmd1), (sizeof(qe_bd_t)) * 2,
            (unsigned)(tmd1), 4);
    printf("\nTx data :-");
    dismem((unsigned char *)vir_addr((unsigned long)tmd1->buf_ptr), 0x40,
            (unsigned)vir_addr((unsigned long)tmd1->buf_ptr), 4);
    printf("\n\n");
}


/**************************************************************************
 *
 * Name: qe_hdlc_param_ram_dump
 *
 * Description: Debug function used to dump all HDLC PRAM memory registers
 *
 * Input: None
 *
 * Output: None
 *
 *************************************************************************/
int 
qe_hdlc_param_ram_dump (void)
{
    qe_fast_hdlc_param_t *hdlc_param_p;

    hdlc_param_p = (qe_fast_hdlc_param_t *)&REGB->qe.muram;

    printf("\nStarting Parameter Memory Address = 0x%08x\n", 
           (uint32_t)hdlc_param_p);
    printf("RIPTR      @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->riptr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->riptr);
    printf("TIPTR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tiptr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tiptr);
    printf("MRBLR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->mrblr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->mrblr);
    printf("RSTATE    @offset 0x%04x contains 0x%08x\n", 
           (uint32_t)&(hdlc_param_p->rstate) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rstate);
    printf("RBASE     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rbase) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rbase);
    printf("RBDSTAT     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rbdstat) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rbdstat);
    printf("RBDLEN     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rbdlen) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rbdlen);
    printf("RDPTR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rdptr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rdptr); 
    printf("TSTATE    @offset 0x%04x contains 0x%08x\n", 
           (uint32_t)&(hdlc_param_p->tstate) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tstate);
    printf("TBASE     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tbase) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tbase);
    printf("TBDSTAT     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tbdstat) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tbdstat);
    printf("TBDLEN     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tbdlen) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tbdlen);
    printf("TDPTR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tdptr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tdptr); 
    printf("RBPTR      @offset 0x%04x contains 0x%08x\n", 
           (uint32_t)&(hdlc_param_p->rbptr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rbptr);
    printf("TBPTR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tbptr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tbptr);
    printf("RCRC     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rcrc) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rcrc);
    printf("TCRC     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tcrc) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tcrc);
    printf("C_MASK     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->c_mask) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->c_mask);
    printf("C_PRES     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->c_pres) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->c_pres);
    printf("DISFC     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->disfc) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->disfc);
    printf("CRCEC     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->crcec) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->crcec);
    printf("ABTSC     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->abtsc) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->abtsc);
    printf("NMARC     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->nmarc) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->nmarc);
    printf("MAX_CNT     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->max_cnt) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->max_cnt);
    printf("MFLR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->mflr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->mflr);
    printf("RFTHR     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rfthr) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rfthr);
    printf("RFCNT     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->rfcnt) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->rfcnt);
    printf("HMASK     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->hmask) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->hmask);
    printf("HADDR1     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->haddr1) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->haddr1);
    printf("HADDR2     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->haddr2) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->haddr2);
    printf("HADDR3     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->haddr3) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->haddr3);
    printf("HADDR4     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->haddr4) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->haddr4);
    printf("TS_TMP     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->ts_tmp) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->ts_tmp);
    printf("TMP_MB     @offset 0x%04x contains 0x%04x\n", 
           (uint32_t)&(hdlc_param_p->tmp_mb) -
           (uint32_t)&(hdlc_param_p->riptr), hdlc_param_p->tmp_mb);
    return (PASSED);
    
}


/**********************************************************************
 *
 * Function: enable_ucc
 *
 * Description: Enable RX (ENR) and Enable TX (ENT) are set in the GUMR for UCC,
 * thus allowing UCC to receive and transmit.
 *
 * Inputs: ucc_num - UCC number
 *
 * Outputs: None
 *
 **********************************************************************
 */
void enable_ucc(int ucc_num)
{
#ifdef DEBUG    
    printf("\nenable_ucc\n");
#endif    
    switch (ucc_num) {
    case QE_UCC1:
        REGB->qe.ucc1.mode.fast.reg.gumr |= (GUMR_ENR | GUMR_ENT);
        break;
    case QE_UCC3:
	REGB->qe.ucc3.mode.fast.reg.gumr |= (GUMR_ENR | GUMR_ENT);
        break;
    case QE_UCC5:
	REGB->qe.ucc5.mode.fast.reg.gumr |= (GUMR_ENR | GUMR_ENT);
        break;
    case QE_UCC7:
	REGB->qe.ucc7.mode.fast.reg.gumr |= (GUMR_ENR | GUMR_ENT);
        break;
    default:
        printf("\nSpecified UCC invalid\n");
        break;
    }
}



/**********************************************************************
 *
 * Function: disable_ucc
 *
 * Description: Disable RX (ENR) and Enable TX (ENT) in the GUMR for UCC,
 * thus stopping UCC reception and transmission.
 *
 * Inputs: ucc_num - UCC number
 *
 * Outputs: None
 *
 **********************************************************************
 */
void disable_ucc(int ucc_num)
{
#ifdef DEBUG    
    printf("\ndisable_ucc\n");
#endif    
    switch (ucc_num) {
    case QE_UCC1:
        REGB->qe.ucc1.mode.fast.reg.gumr &= ~(GUMR_ENR | GUMR_ENT);
        break;
    case QE_UCC3:
	REGB->qe.ucc3.mode.fast.reg.gumr &= ~(GUMR_ENR | GUMR_ENT);
        break;
    case QE_UCC5:
	REGB->qe.ucc5.mode.fast.reg.gumr &= ~(GUMR_ENR | GUMR_ENT);
        break;
    case QE_UCC7:
	REGB->qe.ucc7.mode.fast.reg.gumr &= ~(GUMR_ENR | GUMR_ENT);
        break;
    default:
        printf("\nSpecified UCC invalid\n");
        break;
    }
}


/**********************************************************************
 *
 * Function: clk_nmsi_init
 *
 * Description: Init clock for NMSI mode
 * 
 * Inputs: None
 *
 * Outputs: None.
 *
 **********************************************************************
 */
static void clk_nmsi_init (int lpbk_opt)
{
#ifdef DEBUG
    printf("\nclk_nmsi_init\n");
#endif
    /* We are using UCC3
     * Transmit Clock source is CLK11 pin
     * Receive Clock source is CLK10 pin
     */

    if (lpbk_opt == PATRIOT_UCC_LPBK) {
	REGB->qe.brg.brgc1 = QE_BRGCX_EN | QE_BRGCX_CD(176);
	REGB->qe.mux.cmxucr1 = QE_CMXUCR1_TU3CS_BRG1 |
	    QE_CMXUCR1_RU3CS_BRG1;
	REGB->im_gur.pmuxcr &= ~0x00002000;
    } else {
	REGB->qe.mux.cmxucr1 = QE_CMXUCR1_TU3CS_CLK11 |
	    QE_CMXUCR1_RU3CS_CLK10;
	REGB->im_gur.pmuxcr |= 0x00002000;
    }
#ifdef DEBUG    
    printf("\nREGB->im_gur.pmuxcr = 0x%08x", REGB->im_gur.pmuxcr);
    printf("\nREGB->qe.mux.cmxucr1 = 0x%08x", REGB->qe.mux.cmxucr1);
#endif    

    /* 
     * UCC3 is not connected to TSA, UC3 = 0
     * HDLC set to default for both TSA and NMSI, HBM3 = 0
     * 
     * 
     */
    REGB->qe.mux.cmxucr1 &= ~(QE_CMXUCR1_UC3 | QE_CMXUCR1_HBM3);

    /* 
     * Assign parallet port pins for UCC3 function 
     */
    /* Configure pin direction and  function */
    /* PA27 as input and Clock 11 */
    /* clear direction bits for PA27 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(27));
    /* PA27 as intput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_IN(27);
    /* clear function bits for PA27 and set it as clock 11 */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(27, 0x3));
    REGB->im_gur.cppar2a |= MPC8500_CPPAR2(27, 0x2);
#ifdef DEBUG    
    printf("\nPA27 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA27 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);
#endif    
    /* PA28 as input and Clear to Send */
    /* clear direction bits for PA28 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(28));
    /* PA28 as intput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_IN(28);
    /* clear function bits for PA28 and set it as Clear to Send */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(28, 0x3));
    REGB->im_gur.cppar2a |= MPC8500_CPPAR2(28, 0x2);
#ifdef DEBUG    
    printf("\nPA28 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA28 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);    
#endif    

    /* PA30 as input and SER3_RXD[0] */
    /* clear direction bits for PA30 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(30));
    /* PA30 as intput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_IN(30);
    /* clear function bits for PA30 and set it as SER3_RXD[0] */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(30, 0x3));
    REGB->im_gur.cppar2a |= MPC8500_CPPAR2(30, 0x2);
#ifdef DEBUG    
    printf("\nPA30 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA30 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);
#endif    

    /* PB00 as input and Carrier Detect */
    /* clear direction bits for PB00 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(0));
    /* PB00 as intput */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_IN(0);
    /* clear function bits for PB00 and set it as Carrier Detect */
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(0, 0x3));
    REGB->im_gur.cppar1b |= MPC8500_CPPAR1(0, 0x2);
#ifdef DEBUG    
    printf("\nPB00 REGB->im_gur.cpdir1b = 0x%08x", REGB->im_gur.cpdir1b);
    printf("\nPB00 REGB->im_gur.cppar1b = 0x%08x", REGB->im_gur.cppar1b);
#endif
    
    /* PA14 as output and SER3_TXD[1] */
    /* clear direction bits for PA14 */
    REGB->im_gur.cpdir1a &= ~(MPC8500_CPDIR1_INOUT(14));
    /* PA14 as outtput */
    REGB->im_gur.cpdir1a |=  MPC8500_CPDIR1_OUT(14);
    /* clear function bits for PA14 and set it as SER3_TXD[1] */
    REGB->im_gur.cppar1a &= ~(MPC8500_CPPAR1(14, 0x3));
    REGB->im_gur.cppar1a |= MPC8500_CPPAR1(14, 0x2);
#ifdef DEBUG    
    printf("\nPA14 REGB->im_gur.cpdir1a = 0x%08x", REGB->im_gur.cpdir1a);
    printf("\nPA14 REGB->im_gur.cppar1a = 0x%08x", REGB->im_gur.cppar1a);   
#endif    

    /* PA15 as output and SER3_TXD[2] */
    /* clear direction bits for PA15 */
    REGB->im_gur.cpdir1a &= ~(MPC8500_CPDIR1_INOUT(15));
    /* PA15 as outtput */
    REGB->im_gur.cpdir1a |=  MPC8500_CPDIR1_OUT(15);
    /* clear function bits for PA15 and set it as SER3_TXD[2] */
    REGB->im_gur.cppar1a &= ~(MPC8500_CPPAR1(15, 0x3));
    REGB->im_gur.cppar1a |= MPC8500_CPPAR1(15, 0x2);
#ifdef DEBUG    
    printf("\nPA15 REGB->im_gur.cpdir1a = 0x%08x", REGB->im_gur.cpdir1a);
    printf("\nPA15 REGB->im_gur.cppar1a = 0x%08x", REGB->im_gur.cppar1a);       
#endif

    /* PA16 as output and SER3_TXD[3] */
    /* clear direction bits for PA16 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(16));
    /* PA16 as outtput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_OUT(16);
    /* clear function bits for PA16 and set it as SER3_TXD[3] */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(16, 0x3));
    REGB->im_gur.cppar2a |= MPC8500_CPPAR2(16, 0x2);
#ifdef DEBUG    
    printf("\nPA16 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA16 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);     
#endif    
    
    /* PB01 as input and receive Clock 10 */
    /* clear direction bits for PB01 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(1));
    /* PB01 as intput */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_IN(1);
    /* clear function bits for PB01 and set it RX Clock 10 */
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(1, 0x3));
    REGB->im_gur.cppar1b |= MPC8500_CPPAR1(1, 0x2);
#ifdef DEBUG    
    printf("\nPB01 REGB->im_gur.cpdir1b = 0x%08x", REGB->im_gur.cpdir1b);
    printf("\nPB01 REGB->im_gur.cppar1b = 0x%08x", REGB->im_gur.cppar1b);
#endif

    /* PA31 as output and SER3_TXD[0] */
    /* clear direction bits for PA31 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(31));
    /* PA31 as outtput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_OUT(31);
    /* clear function bits for PA31 and set it as SER3_TXD[0] */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(31, 0x3));
    REGB->im_gur.cppar2a |= MPC8500_CPPAR2(31, 0x2);
#ifdef DEBUG    
    printf("\nPA31 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA31 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);
#endif

    /* PA29 as output and Ready to Send */
    /* clear direction bits for PA29 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(29));
    /* PA29 as outtput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_OUT(29);
    /* clear function bits for PA29 and set it as Ready to Send */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(29, 0x3));
    REGB->im_gur.cppar2a |= MPC8500_CPPAR2(29, 0x2);
#ifdef DEBUG    
    printf("\nPA29 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA29 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);     
#endif

    /* PA11 as input and SER3_RXD[1] */
    /* clear direction bits for PA11 */
    REGB->im_gur.cpdir1a &= ~(MPC8500_CPDIR1_INOUT(11));
    /* PA11 as intput */
    REGB->im_gur.cpdir1a |=  MPC8500_CPDIR1_IN(11);
    /* clear function bits for PA11 and set it as SER3_RXD[1] */
    REGB->im_gur.cppar1a &= ~(MPC8500_CPPAR1(11, 0x3));
    REGB->im_gur.cppar1a |= MPC8500_CPPAR1(11, 0x2);
#ifdef DEBUG    
    printf("\nPA11 REGB->im_gur.cpdir1a = 0x%08x", REGB->im_gur.cpdir1a);
    printf("\nPA11 REGB->im_gur.cppar1a = 0x%08x", REGB->im_gur.cppar1a);  
#endif
    
    /* PA12 as input and SER3_RXD[2] */
    /* clear direction bits for PA12 */
    REGB->im_gur.cpdir1a &= ~(MPC8500_CPDIR1_INOUT(12));
    /* PA12 as intput */
    REGB->im_gur.cpdir1a |=  MPC8500_CPDIR1_IN(12);
    /* clear function bits for PA12 and set it as SER3_RXD[2] */
    REGB->im_gur.cppar1a &= ~(MPC8500_CPPAR1(12, 0x3));
    REGB->im_gur.cppar1a |= MPC8500_CPPAR1(12, 0x2);
#ifdef DEBUG    
    printf("\nPA12 REGB->im_gur.cpdir1a = 0x%08x", REGB->im_gur.cpdir1a);
    printf("\nPA12 REGB->im_gur.cppar1a = 0x%08x", REGB->im_gur.cppar1a);  
#endif
    
    /* PA13 as input and SER3_RXD[3] */
    /* clear direction bits for PA13 */
    REGB->im_gur.cpdir1a &= ~(MPC8500_CPDIR1_INOUT(13));
    /* PA13 as intput */
    REGB->im_gur.cpdir1a |=  MPC8500_CPDIR1_IN(13);
    /* clear function bits for PA13 and set it as SER3_RXD[3] */
    REGB->im_gur.cppar1a &= ~(MPC8500_CPPAR1(13, 0x3));
    REGB->im_gur.cppar1a |= MPC8500_CPPAR1(13, 0x2);
#ifdef DEBUG    
    printf("\nPA13 REGB->im_gur.cpdir1a = 0x%08x", REGB->im_gur.cpdir1a);
    printf("\nPA13 REGB->im_gur.cppar1a = 0x%08x", REGB->im_gur.cppar1a);
#endif    

    return;

}

/**********************************************************************
 *
 * Function: init_ucc_hdlc_param
 *
 * Description: This routine initializes the parameter RAM (PRAM)
 * for UCC. It is configured with the values for HDLC mode.
 * 
 * Inputs: buf_size - buffer size
 *
 * Outputs: None.
 *
 **********************************************************************
 */
int init_ucc_hdlc_param(int buf_size)
{

    int i;
    qe_fast_hdlc_param_t *hdlc_param_p;
#ifdef DEBUG    
    printf("\ninit_ucc_hdlc_param\n");
#endif

    hdlc_param_p = (qe_fast_hdlc_param_t *)ucc_param_ptr;
#ifdef DEBUG   
    printf("\n%s:hdlc_param_p = 0x%08x", __FUNCTION__,hdlc_param_p);
#endif    

    vir_ptr = (unsigned long)malloc_nm(4096 * 20);
    bd = (qe_bd_t *)vir_ptr;
    bd_phy = (qe_bd_t *)phy_addr((unsigned long)bd);
#ifdef DEBUG    
    printf("\n%s:bd = 0x%08x", __FUNCTION__,bd);
#endif    
    /*Set RIPTR & TIPTR to point to MURAM as defined by the programmer 14.2.2.1 */

    hdlc_param_p->riptr = RIPTR;
    hdlc_param_p->tiptr = TIPTR;
#ifdef DEBUG    
    printf("\nhdlc_param_p->riptr = 0x%08x", hdlc_param_p->riptr);
    printf("\nhdlc_param_p->tiptr = 0x%08x", hdlc_param_p->tiptr);
#endif    
    /* Set MRBLR to the size of the RX and TX buffers. */
    hdlc_param_p->mrblr = MAX_HDLC_RX_BUF * 2;
    
    /* Set RBASE and TBASE to point to RX and TX BDs in MURAM */
    hdlc_param_p->rbase = (int)bd_phy;
    hdlc_param_p->tbase = (int)(hdlc_param_p->rbase + 2 * sizeof(qe_bd_t));
#ifdef DEBUG    
    printf("\n%s:hdlc_param_p->rbase = 0x%08x\n", __FUNCTION__,
	   hdlc_param_p->rbase);
    printf("\n%s:hdlc_param_p->tbase = 0x%08x\n", __FUNCTION__,
	   hdlc_param_p->tbase);
#endif

    /* Update RBPTR and TBPTR */
    if (qe_send_risc_cmd(QE_CECR_SBC(QE_CECR_SBC_UCC3 |
				     ((QE_UCC3 - 1) << 4)) |
			 QE_CECR_CHANNEL(QE_CECR_CHAN_HDLC) |
			 QE_CECR_INIT_RXTX) == FAILED) {
        sprintf(err_msg, "%s, [#%d]: risc command failed\n",__FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_3);
        return (FAILED);
    }

    usleep(100);
    
    /* Set RSTATE & TSTATE. The high 8 bits of these registers are RBMR
       and TBMR. All other bits must be cleared
       RBMR and TBMR are set as follows:
       GBL = 0, snooping disabled (should be set when cache is enabled).
       BO = b10, big endian byte ordering
       CETM = 0, not used
       DTB = BDB = 0; buffers and buffer descriptors on CSB */

    hdlc_param_p->rstate = 0x30000000;
    hdlc_param_p->tstate = 0x30000000;

    /* Set C_MASK & C_PRES for 16 bit CRC. Defined in RM section 14.2.2.1 */
    hdlc_param_p->c_mask = 0x0000F0B8;
    hdlc_param_p->c_pres = 0x0000FFFF;

    /* Clear DISFC, CRCEC, ABTSC & NMARC
       Note that these steps are shown for completeness. They could be
       skipped due to the clear at the start of this routine. */
    hdlc_param_p->disfc = 0x0000;
    hdlc_param_p->crcec = 0x0000;
    hdlc_param_p->abtsc = 0x0000;
    hdlc_param_p->nmarc = 0x0000;

    /* Set RFTHR & RFCNT */
    hdlc_param_p->rfthr = RFTHR;
    hdlc_param_p->rfcnt = RFCNT;

    /* Set MFLR */
    hdlc_param_p->mflr = MAX_HDLC_RX_BUF;
#ifdef DEBUG    
    printf("\nhdlc_param_p->mflr @ 0x%08x = 0x%08x", &hdlc_param_p->mflr,
	   hdlc_param_p->mflr);
#endif    
    return (PASSED);

}



/**********************************************************************
 *
 * Function: init_ucc_hdlc
 *
 * Description: This routine initializes the UCC.
 * It is configured with the values for HDLC mode.
 * 
 * Inputs: buf_size - size of buffer
 *         tx_buf - pointer to transmit buffer
 *         rx_buf - pointer to receive buffer
 *         lpbk_op - PATRIOT_UCC_LPBK or PATRIOT_UCC_PASS
 *
 * Outputs: PASSED/FAILED
 *
 **********************************************************************
 */
int init_ucc_hdlc(int buf_size, uchar *tx_buf_phy, uchar *rx_buf_phy,
		  int lpbk_op)
{
    
    qe_bd_t *rx_bd_ptr, *tx_bd_ptr;
    uint32_t *cecr_ptr, *muram_base;
    unsigned int *ucce_p, *uccm_p;
    int i;
#ifdef DEBUG
    printf("\ninit_ucc_hdlc\n");
#endif    
    /* Setup GUEMR to configure UCC3 for FAST mode on both TX and RX
       note that bit 3 is reserved and must be set. RM section 6.3.2 */
    
    REGB->qe.ucc3.mode.fast.reg.guemr = 0x10 | GUEMR_URMODE_FAST |
	GUEMR_UTMODE_FAST;
#ifdef DEBUG    
    printf("\nREGB->qe.ucc3.mode.fast.reg.guemr = 0x%08x",
	   REGB->qe.ucc3.mode.fast.reg.guemr);
#endif
    /* Masked all the interrupts */
    uccm_p = (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.uccm;
    /* Masked all the interrupts */
    *uccm_p = ALL_QE_INTERRUPTS;
    
    ucce_p = (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.ucce;
    *ucce_p = ALL_QE_INTERRUPTS;
#ifdef DEBUG    
    printf("\nucce_p = 0x%08x, *ucce_p = 0x%08x", ucce_p, *ucce_p);
#endif    
    *uccm_p = ALL_QE_INTERRUPTS;

    /* Enable the interrupts in QE */
    REGB->qe.irq.cimr = QE_INTERRUPT_MASK; // 0xf8f0e07e
#ifdef DEBUG    
    printf("\nREGB->qe.irq.cimr = 0x%08x", REGB->qe.irq.cimr);
#endif    
    /* Setup virtual FIFOs. RM section 7.5 */
    REGB->qe.ucc3.mode.fast.reg.urfb = RX_VFIFO; /* RX VFIFO base */
    REGB->qe.ucc3.mode.fast.reg.urfs = VFIFO_SIZE; /* RX VFIFO size */
    REGB->qe.ucc3.mode.fast.reg.urfet = VFIFO_SIZE/2; /* URFET = 1/2 size */
    REGB->qe.ucc3.mode.fast.reg.urfset = (VFIFO_SIZE*3)/4;/* URFET = 3/4 size */
    REGB->qe.ucc3.mode.fast.reg.utfb = TX_VFIFO; /* TX VFIFO base */
    REGB->qe.ucc3.mode.fast.reg.utfs = VFIFO_SIZE; /* RX VFIFO size */
    REGB->qe.ucc3.mode.fast.reg.utfet = VFIFO_SIZE/2; /* URFET = 1/2 size */
    REGB->qe.ucc3.mode.fast.reg.utftt = VFIFO_SIZE/4; /* URFET = 1/4 size */    
    
    /* Set UCC3's GUMR. RM section 7.4.2.1
       DIAG = b00, normal mode/b01, loopback mode
       TCI = 1, data is clocked out on rising edge, 0 for TDM, 1 for NMSI
       TRX = 0, normal, no transparent receiver.
       TTX = 0, normal, no transparent transmitter.
       CDP = 0, normal/envelope; must be used with HDLC in TSA mode.
       CTSP = 0, normal mode, CTS envelops the frames.
       CDS = 1, synchronous mode, CD is synchronized with the data,
       internal loopback
       CTSS = 1, synchronous mode CTS is synchronized with the data.
       FFTH = 0, default for all protocols, except POS
       TXSY = 0, no synchronization between RX & TX.
       RSYN = 0, normal; RX sync timing adjust only used with transparent RX
       SYNL = b00, not applicable to HDLC.
       RTSM = 1, Send flags/syncs between frames, must be set in HDLC nibble
       mode for loopback
       RENC = b00, NRZ decode for RX.
       REVD = 0, normal bit order.
       TENC = b00, NRZ decode for TX.
       TCRC = b00, CRC selection for transparent TX mode only.
       ENR = 0, disabled for now.
       ENT = 0, disabled for now.
       MODE = b0000, HDLC mode */
    REGB->qe.ucc3.mode.fast.reg.gumr = 0x00000000;
    REGB->qe.ucc3.mode.fast.reg.gumr = GUMR_FAST_DIAG(0) |
	                          GUMR_FAST_TCI |
                                  GUMR_FAST_CDS |
	                          GUMR_FAST_CTSS |
	                          GUMR_FAST_FFTH(0) |
	                          GUMR_FAST_SYNL(0) |
	                          GUMR_FAST_RTSM |
	                          GUMR_FAST_RENC(0) |
	                          GUMR_FAST_TENC(0) |
	                          GUMR_FAST_TCRC(0) |
	                          GUMR_FAST_MODE(0);
    
    if (lpbk_op == PATRIOT_UCC_LPBK) {
	REGB->qe.ucc3.mode.fast.reg.gumr |= GUMR_FAST_DIAG(1);
    } else {
	REGB->qe.ucc3.mode.fast.reg.gumr &= ~GUMR_FAST_DIAG(3);
    }
#ifdef DEBUG    
    printf("\n%s:REGB->qe.ucc3.mode.fast.reg.gumr @offset 0x%08x = 0x%08x",
	   __FUNCTION__, (unsigned int)&REGB->qe.ucc3.mode.fast.reg.gumr,
	   REGB->qe.ucc3.mode.fast.reg.gumr);
#endif    
    /* Setup HDLC Mode Register (UPSMR). RM section 14.2.2.2
       NOF = b0000, no flags.
       FSE = 0, normal operation.
       MFF = 1, multiple frames allowed in transmit FIFO.
       RTE = 0, no retransmission.
       TS = 0, normal, no timestamp in RX buffer.
       BUS = 0, normal (not HDLC bus mode).
       BRM = 0, ignored in non bus mode.
       DRT = 0, normal RX operation.
       NBO = b01, nibble mode (4 bits of data per clock).
       CW = b000, ignored in non bus mode.
       CRC = b00, 16 bit HDCL CRC. */
    REGB->qe.ucc3.mode.fast.reg.upsmr = 0x00000000; /* Clear all bits */

#ifdef DEBUG    
    printf("\nREGB->qe.ucc3.mode.fast.reg.upsmr = 0x%08x",
	   REGB->qe.ucc3.mode.fast.reg.upsmr);
#endif    
    /* Set UDSR (data sync) to 0x7E7E. RM section 7.4.5 */
    REGB->qe.ucc3.mode.fast.reg.udsr = UDSR_DEFAULT; /* two HDLC flags */

    /* Init PARAM for an HDLC channel. */
    if (init_ucc_hdlc_param (buf_size)) {
    	sprintf(err_msg, "\n%s, [#%d]:Init ucc hdlc param fail\n",__FUNCTION__, __LINE__);
    	print_err(FALSE, err_msg, LVL_2);
	return (FAILED);
    }
#ifdef DEBUG    
    printf("\nAfter init_ucc_hdlc_param\n");
#endif
#if TEST
    g_rxbuf = (unsigned long)malloc_nm(4096);
    g_rxbuf_phy = (unsigned long)phy_addr(g_rxbuf);
    g_rxbuf1 = (unsigned long)malloc_nm(4096);
    g_rxbuf1_phy = (unsigned long)phy_addr(g_rxbuf1);
    
    rx_bd_ptr = (qe_bd_t *)bd;
    printf("\n%s:rx_bd_ptr = 0x%08x", __FUNCTION__, rx_bd_ptr);

    rx_bd_ptr->status = (QE_HDLC_RX_BDSTAT_E |
			 QE_HDLC_RX_BDSTAT_I); /*Set Empty & Int & Wrap bits.*/
    rx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    rx_bd_ptr->buf_ptr = (uint8_t *)g_rxbuf_phy; 
    
    rx_bd_ptr++;

    rx_bd_ptr->status = (QE_HDLC_RX_BDSTAT_E | QE_HDLC_RX_BDSTAT_W |
			 QE_HDLC_RX_BDSTAT_I); /*Set Empty & Int & Wrap bits.*/
    rx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    rx_bd_ptr->buf_ptr = (uint8_t *)g_rxbuf1_phy;     
    
    /* Setup a single TX BD in MURAM. RM section 14.2.2.4
       First, set local pointer to TX BD in MURAM */

    g_txbuf = (unsigned long)malloc_nm(4096);
    g_txbuf_phy = (unsigned long)phy_addr(g_txbuf);
    g_txbuf1 = (unsigned long)malloc_nm(4096);
    g_txbuf1_phy = (unsigned long)phy_addr(g_txbuf1);
    
    tx_bd_ptr = (qe_bd_t *)((int)bd + 2 * sizeof(qe_bd_t)); 
    printf("\n%s:tx_bd_ptr = 0x%08x", __FUNCTION__, tx_bd_ptr);
    /* Set Last, Wrap, Transmit CRC*/
    tx_bd_ptr->status = (QE_HDLC_TX_BDSTAT_L |
			 QE_HDLC_TX_BDSTAT_I | QE_HDLC_TX_BDSTAT_TC); 
    tx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    tx_bd_ptr->buf_ptr = (uint8_t *)g_txbuf_phy;
    
    tx_bd_ptr++;

    printf("\n%s:tx_bd_ptr = 0x%08x", __FUNCTION__, tx_bd_ptr);
    /* Set Last, Wrap, Transmit CRC*/
    tx_bd_ptr->status = (QE_HDLC_TX_BDSTAT_W | QE_HDLC_TX_BDSTAT_L |
			 QE_HDLC_TX_BDSTAT_I | QE_HDLC_TX_BDSTAT_TC); 
    tx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    tx_bd_ptr->buf_ptr = (uint8_t *)g_txbuf1_phy;
#endif   /* #if TEST */

    rx_bd_ptr = (qe_bd_t *)bd;
#ifdef DEBUG    
    printf("\n%s:rx_bd_ptr = 0x%08x", __FUNCTION__, rx_bd_ptr);
#endif
    /*Set Empty & Int & Wrap bits.*/
    rx_bd_ptr->status = (QE_HDLC_RX_BDSTAT_E |
			 QE_HDLC_RX_BDSTAT_I); 
    rx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    rx_bd_ptr->buf_ptr = (uint8_t *)rx_buf_phy; 

    rx_bd_ptr++;
    /*Set Empty & Int & Wrap bits.*/
    rx_bd_ptr->status = (QE_HDLC_RX_BDSTAT_E | QE_HDLC_RX_BDSTAT_W |
			 QE_HDLC_RX_BDSTAT_I); 
    rx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    /*Create a dummy buffer for the 2nd BD */
    g_rxbuf_dummy = (unsigned long)malloc_nm(4096*20);
    g_rxbuf_dummy_phy = (unsigned long)phy_addr(g_rxbuf_dummy);
    rx_bd_ptr->buf_ptr = (uint8_t *)g_rxbuf_dummy_phy;     
    
    /* Setup a single TX BD in MURAM. RM section 14.2.2.4
       First, set local pointer to TX BD in MURAM */
    
    tx_bd_ptr = (qe_bd_t *)((int)bd + 2 * sizeof(qe_bd_t));
#ifdef DEBUG    
    printf("\n%s:tx_bd_ptr = 0x%08x", __FUNCTION__, tx_bd_ptr);
#endif
    /* Set Last, Wrap, Transmit CRC*/
    tx_bd_ptr->status = (QE_HDLC_TX_BDSTAT_L |
			 QE_HDLC_TX_BDSTAT_I | QE_HDLC_TX_BDSTAT_TC);
    tx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    tx_bd_ptr->buf_ptr = (uint8_t *)tx_buf_phy;

    tx_bd_ptr++;
#ifdef DEBUG
    printf("\n%s:tx_bd_ptr = 0x%08x", __FUNCTION__, tx_bd_ptr);
#endif
    /* Set Last, Wrap, Transmit CRC*/
    tx_bd_ptr->status = (QE_HDLC_TX_BDSTAT_W | QE_HDLC_TX_BDSTAT_L |
			 QE_HDLC_TX_BDSTAT_I | QE_HDLC_TX_BDSTAT_TC); 
    tx_bd_ptr->length = 0; /* Clear the length. */
    /*Set the data pointer to the RX buffer. */
    /*Create a dummy buffer for the 2nd BD */
    g_txbuf_dummy = (unsigned long)malloc_nm(4096*20);
    g_txbuf_dummy_phy = (unsigned long)phy_addr(g_txbuf_dummy);    
    tx_bd_ptr->buf_ptr = (uint8_t *)g_txbuf_dummy_phy;

    
    /* Send an INIT RX & TX Command to the QUICC Engine. This causes
       the QE to initialize all internal data structures as per the
       programmed registers and PRAM. */
#ifdef DEBUG    
    printf("\nBefore setting, REGB->qe.cp.cecr= 0x%08x\n",
	   REGB->qe.cp.cecr);
#endif    
    /* UCC3 HDLC Init RX&TX. RM Section 4.3.1 */
    REGB->qe.cp.cecr = QE_CPCR_SUBBLOCK(QE_CECR_SBC_UCC3) | QE_CECR_FLG |
	               QE_CECR_OPCODE(QE_CECR_INIT_RXTX);

    /* Wait for the QE to indicate it is ready for a new command. */
    for (i = 0; i < QE_CECR_SPIN_MAX; i++) {
        if (((REGB->qe.cp.cecr) & QE_CECR_FLG) == 0) {
            break;;
        }
	msleep(10);
    }
#ifdef DEBUG    
    printf("\nREGB->qe.cp.cecr= 0x%08x\n",
	   REGB->qe.cp.cecr);
#endif    
    if (i == QE_CECR_SPIN_MAX) {
	sprintf(err_msg, "\n%s, [#%d]:Time out waiting for QE ready for command, cecr = 0x%08x\n",__FUNCTION__, __LINE__,
	       REGB->qe.cp.cecr);
	print_err(FALSE, err_msg, LVL_2);
	free_nm((void *)g_txbuf_dummy);
	free_nm((void *)g_rxbuf_dummy);
	return (FAILED);
    }
    
    return (PASSED);
    
}


/**************************************************************************
 *
 * Name: qe_muram_init
 *
 * Description: This routine writes zeroes to all locations of the
 *              QUICC engine MURAM
 *
 * Inputs: None
 *
 * Output: None
 *
 *************************************************************************/
void
qe_muram_init (void)
{
    unsigned int *muram_addr, *muram_start, *muram_end;

#ifdef DEBUG    
    printf("\nqe_muram_init\n");
#endif    
    muram_start = (unsigned int *)&REGB->qe.muram;
    muram_end = (unsigned int *)((unsigned int)(&REGB->qe.muram) + 0x6000);

    for (muram_addr = muram_start; muram_addr < muram_end; muram_addr++) {

        /*
         * This 'if' is to allow me to generate ECC error
         * Modify the first idata value to a non-zero value
         * and then access the first location to generate the error
         */
        if (muram_addr == muram_start) {
            *muram_addr = 0;
        } else {
            *muram_addr = 0;
        }
    }
}



/**************************************************************************
 *
 * Name: qe_ram_ecc_enable
 *
 * Description: This routine zeroes out the specified RAM and enables 
 *              its ECC.
 *
 * Inputs: ram_ecc -> 0 for MURAM, 1 for IRAM or 2 for both
 *
 * Output: None
 *
 *************************************************************************/
void
qe_ram_ecc_enable (uint ram_ecc)
{

#ifdef DEBUG    
    printf("\nqe_ram_ecc_enable\n");
#endif    
    if (ram_ecc == MURAM_ECC_EN || ram_ecc == MURAM_IRAM_ECC_EN) {
        qe_muram_init();
        if (!(REGB->qe.cp.cercr & QE_CERCR_MEE)) {
            REGB->qe.cp.cercr |= QE_CERCR_MEE;
#ifdef QE_DEBUG
            printf("\nEnabling MURAM ECC: 0x%04x\n", REGB->qe.cp.cercr);
#endif
        }
    }

    if (ram_ecc == IRAM_ECC_EN || ram_ecc == MURAM_IRAM_ECC_EN) {
        if (!(REGB->qe.cp.cercr & QE_CERCR_IEE)) {
            REGB->qe.cp.cercr |= QE_CERCR_IEE;
#ifdef QE_DEBUG
            printf("\nEnabling IRAM ECC: 0x%04x\n", REGB->qe.cp.cercr);
#endif
        }
    }

}

/**************************************************************************
 *
 * Name: qe_send_risc_cmd
 *
 * Description: This routine is used to send a command to the risc processor.
 *
 * Inputs: ushort - command to issue.
 *
 * Outputs: PASSED if the command was sent.
 *          FAILED if the risc was unavailable.
 *
 *************************************************************************/
int
qe_send_risc_cmd (uint command)
{
    ushort count;

#ifdef DEBUG    
    printf("\nqe_send_risc_cmd\n");
    printf("command = %08x\n", command);
#endif
    /*
     * Check the command semaphore flag to see if the risc
     * is busy.  If so try again for a little while and then
     * give up.
     */
    for (count = 0; count < QE_CECR_SPIN_MAX; count++) {
        if (((REGB->qe.risc.cecr) & QE_CECR_FLG) == 0) {
            REGB->qe.risc.cecr = (command | QE_CECR_FLG);
#ifdef DEBUG	    
	    printf("\n%s:REGB->qe.risc.cecr = 0x%08x",__FUNCTION__,
		   REGB->qe.risc.cecr);
#endif	    
            return (PASSED);
        }
	msleep(10);
    }
    return (FAILED);
}

/**************************************************************************
 *
 * Name: restore_gpio_pin
 *
 * Description: This routine restores GPIO pin 31 to its GPIO function.
 *              Otherwise the characters for console redirect will be
 *              mesy
 * Inputs: None
 *
 * Outputs: None
 *
 *************************************************************************/
void
restore_gpio_pin (void)
{

    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(31));
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(31, 0x3));

}


/**********************************************************************
 *
 * Function: hdlc_send_and_receive_data
 *
 * Description: Send and receive data in NMSI mode
 * 
 * Inputs: frame_no - number of frame
 *         buf_size - size of buffer
 *         tx_buf - pointer to transmit buffer
 *         rx_buf - pointer to receive buffer
 *         lpbk_op - TDM loopback or pass through
 *
 * Outputs: PASSED/FAILED
 *
 **********************************************************************
 */
int hdlc_send_and_receive_data (int frame_no, int buf_size, uchar *tx_buf, uchar *rx_buf,
				uchar *tx_buf_phy, uchar *rx_buf_phy,
				int lpbk_op)
{
    int i;
    unsigned int *ucce_p, *uccm_p, *muram_base;
    qe_bd_t *rx_bd_ptr, *tx_bd_ptr;
    qe_bd_t *rx_bd_ptr1, *tx_bd_ptr1;
    uchar *tx_temp, *rx_temp, temp;
    int protocol_spd = FAST_MODE;
    char msg[25];

#ifdef DEBUG    
    printf("\nhdlc_send_and_receive_data\n");
    printf("\nbuf_size = 0x%04x, tx_buf = 0x%08x, rx_buf = 0x%08x, lpbk_op = %d, tx_buf_phy = 0x%08x, rx_buf_phy = 0x%08x", buf_size, tx_buf, rx_buf, lpbk_op,tx_buf_phy,rx_buf_phy );
#endif    
    hdlc_rx_frames = 0;

    /* First, initialize and enable the clock for NMSI mode */
    clk_nmsi_init(lpbk_op);

    /*
     * Download microcode into QE
     * Note: Newer QE are RAM based, microcode must be downloaded
     * to QE IRAM and then set the IREADY bit so that RISC processor
     * in QE can start running from the downloaded microcode
     */    
    platform_microcode_download();

    /* Enable IREADY bit in IRAM READY register */
    REGB->qe.iram.iready = QE_IRAM_IREADY;
    
    /*
     * Reset QE
     */
    reset_qe();

    /*
     * Initialize and enable ECC for MURAM and IRAM
     */
    qe_ram_ecc_enable(MURAM_IRAM_ECC_EN);

    ucc_param_ptr = (uchar *)&REGB->qe.muram;
#ifdef DEBUG    
    printf("\n%s: ADRSPC_PQUICC_REGB = 0x%08x", __FUNCTION__, ADRSPC_PQUICC_REGB);
    printf("\n%s:ucc_param_ptr = 0x%08x", __FUNCTION__,ucc_param_ptr);
    printf("\nREGB->qe.irq.cipxcc @0x%08x = 0x%08x", &REGB->qe.irq.cipxcc, REGB->qe.irq.cipxcc);
#endif    
    /* Issue ASSIGN PAGE cmd to relocate Parameter RAM */
    REGB->qe.risc.cecdr = 0x00000000;
#ifdef DEBUG
    printf("\nPrint the cecdr contents = 0x%08x\n", REGB->qe.risc.cecdr);
#endif

    /* Re-allocate Rx Param */
    if (qe_send_risc_cmd(QE_CECR_SNUM(QE_CECR_SNUM_UCC3_RX) |
                                 QE_CECR_ASSIGN_PAGE) == FAILED) {
        sprintf(err_msg, "%s, [#%d]:Re-allocate Rx Param risc command "
        		"failed (CECR=%08x)",__FUNCTION__, __LINE__
		        , REGB->qe.risc.cecr);
        print_err(FALSE, err_msg, LVL_1);
        restore_gpio_pin();
        return (FAILED);
    }

    /* Re-allocate Tx Param */
    if (qe_send_risc_cmd(QE_CECR_SNUM(QE_CECR_SNUM_UCC3_TX) |
                                 QE_CECR_ASSIGN_PAGE) == FAILED) {
        sprintf(err_msg, "%s, [#%d]:Re-allocate Tx Param risc command "
        		"failed (CECR=%08x)",__FUNCTION__, __LINE__
		        , REGB->qe.risc.cecr);
        print_err(FALSE, err_msg, LVL_1);
	restore_gpio_pin();
        return (FAILED);
    }
    
    /* Setup UCC3 and its parameter RAM for HDLC operation. UCC3 is not
       enabled yet. */
    if (init_ucc_hdlc(buf_size, tx_buf_phy, rx_buf_phy, lpbk_op)) {
        sprintf(err_msg, "%s, [#%d]:init ucc hdlc fail",
        		__FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }

    /* Set the RX/TX BD pointer to the area of MURAM used to hold RX/TX BDs. Note
       that this matches the values loaded into UCC3's PRAM RX BD pointer. */
    ucce_p = (unsigned int *)&REGB->qe.ucc3.mode.fast.reg.ucce;
#if TEST
    rx_bd_ptr = (qe_bd_t *)bd;
    rx_bd_ptr1 = rx_bd_ptr + 1;
    tx_bd_ptr = (qe_bd_t *)((int)bd + 2 * sizeof(qe_bd_t));
    tx_bd_ptr1 = tx_bd_ptr + 1;

    strcpy ((uchar *)g_txbuf, "abcdefghijklmnopqrstuvwxyz!");
    tx_bd_ptr->length = 27;
    /* The TX BD and data are ready to transmit, so set R bit in TX BD */
    tx_bd_ptr->status |= QE_HDLC_TX_BDSTAT_R;

    strcpy ((uchar *)g_txbuf1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ!");
    tx_bd_ptr1->length = 27;
    /* The TX BD and data are ready to transmit, so set R bit in TX BD */
    tx_bd_ptr1->status |= QE_HDLC_TX_BDSTAT_R;    
        
    printf("\nrx_bd_ptr = 0x%08x, tx_bd_ptr = 0x%08x", rx_bd_ptr, tx_bd_ptr);
#endif /* #if TEST */

    rx_bd_ptr = (qe_bd_t *)bd;
    rx_bd_ptr1 = rx_bd_ptr + 1;
    tx_bd_ptr = (qe_bd_t *)((int)bd + 2 * sizeof(qe_bd_t));
    tx_bd_ptr1 = tx_bd_ptr + 1;

    tx_bd_ptr->length = buf_size;
    /* The TX BD and data are ready to transmit, so set R bit in TX BD */
    tx_bd_ptr->status |= QE_HDLC_TX_BDSTAT_R;

    /* Fill up the 2nd buffer (dummy buffer) with some data pattern */
    strcpy ((uchar *)g_txbuf_dummy, "ABCDEFGHIJKLMNOPQRSTUVWXYZ!");
    tx_bd_ptr1->length = 27;
    /* The TX BD and data are ready to transmit, so set R bit in TX BD */
    tx_bd_ptr1->status |= QE_HDLC_TX_BDSTAT_R;

    /* Now that the TX and RX buffers and BDs are ready and all hardware is
       configured, enable the UCC */
#ifdef DEBUG    
    dismem((unsigned char *)(ucc_param_ptr), 128,
	   (unsigned)(ucc_param_ptr), 4);
    dismem((unsigned char *)(rx_bd_ptr), 128,
            (unsigned)(rx_bd_ptr), 4);
    dismem((unsigned char *)(rx_buf), 128,
	   (unsigned)(rx_buf), 4);
    dismem((unsigned char *)(tx_buf), 128,
	   (unsigned)(tx_buf), 4);

    display_qe_ucc3_registers();
    qe_hdlc_show_regs(); 
    qe_hdlc_show_bd();
    qe_hdlc_param_ram_dump();
    display_qe_irq_registers();
    display_qe_mux_registers();
    display_qe_brg_registers();
    display_gpio_registers();
    patriot_dump_fpga_reg();
    patriot_ds3170_dump_reg(DS3170_GLOBAL);
    patriot_ds3170_dump_reg(DS3170_PORT);
    patriot_ds3170_dump_reg(DS3170_BERT);
    patriot_ds3170_dump_reg(DS3170_LINE);
    patriot_ds3170_dump_reg(DS3170_HDLC);
    patriot_ds3170_dump_reg(DS3170_FEAC);
    patriot_ds3170_dump_reg(DS3170_TT);
    patriot_ds3170_dump_reg(DS3170_T3);
    patriot_ds3170_dump_reg(DS3170_E3G751);
    patriot_ds3170_dump_reg(DS3170_E3G832);
#endif

    enable_ucc(QE_UCC3);

    for (i = 0; i < 1000; i++) {
	if ((tx_bd_ptr->status & QE_HDLC_TX_BDSTAT_R) == 0) {
	    break;
	}
	msleep(10);
    }
#ifdef DEBUG    
    printf("\nucce_p = 0x%08x, *ucce_p = 0x%08x\n", ucce_p, *ucce_p);
#endif    
    if (i == 1000) {
	sprintf(err_msg, "\n%s, [#%d]:Failure to transmit data, "
			"tx_bd_ptr->status = 0x%04x\n"
           "\ntx_bd_ptr->length = 0x%08x, tx_bd_ptr->buf_ptr = 0x%08x"
			,__FUNCTION__, __LINE__, tx_bd_ptr->status, tx_bd_ptr->length,
			tx_bd_ptr->buf_ptr);
	print_err(FALSE, err_msg, LVL_1);
	qe_hdlc_show_regs(); 
	qe_hdlc_show_bd();
	qe_hdlc_param_ram_dump();
	free_nm((void *)vir_ptr);
	free_nm((void *)g_txbuf_dummy);
	free_nm((void *)g_rxbuf_dummy);
	restore_gpio_pin();
	return (FAILED);
    }

    if (tx_bd_ptr->status & QE_HDLC_TX_BDSTAT_CT) {
	sprintf(err_msg, "\n%s, [#%d]:CTS Lost error in TX descriptor"
			,__FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
	restore_gpio_pin();
	return (FAILED);
    }

    for (i = 0; i < 1000; i++) {
	if ((rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_E) == 0) {
	    break;
	}
	msleep(10);
    }
#ifdef DEBUG
    dismem((unsigned char *)(ucc_param_ptr), 128,
	   (unsigned)(ucc_param_ptr), 4);
    dismem((unsigned char *)(rx_bd_ptr), 128,
            (unsigned)(rx_bd_ptr), 4);
    dismem((unsigned char *)(rx_buf), 1520,
	   (unsigned)(rx_buf), 4);
    dismem((unsigned char *)(tx_buf), 1520,
	   (unsigned)(tx_buf), 4);
#endif    
    if (patriot_framer_debug) {
        sprintf(msg, "\nFrame [%d], Tx_buf\n", frame_no);
        dismem_x(TRUE, msg, (unsigned char *)(tx_buf), 112,
                        (unsigned)(tx_buf), 4);
        sprintf(msg, "\nFrame [%d], Rx_buf\n", frame_no);
        dismem_x(TRUE, msg, (unsigned char *)(rx_buf), 112,
                        (unsigned)(rx_buf), 4);
    }

    if (i == 1000) {
	sprintf(err_msg, "\n%s, [#%d]:Failure to receive data, "
			"rx_bd_ptr->status = 0x%08x",__FUNCTION__, __LINE__,
	       rx_bd_ptr->status);
	print_err(FALSE, err_msg, LVL_1);
	dismem((unsigned char *)(ucc_param_ptr), 128,
            (unsigned)(ucc_param_ptr), 4);
	dismem((unsigned char *)(rx_bd_ptr), 128,
            (unsigned)(rx_bd_ptr), 4);
	dismem((unsigned char *)(rx_buf), 128,
            (unsigned)(rx_buf), 4);
	dismem((unsigned char *)(tx_buf), 128,
            (unsigned)(tx_buf), 4);
	/* Print dismem to host */
    sprintf(msg, "\nFrame [%d], Tx_buf-", frame_no);
    dismem_x(TRUE, msg, (unsigned char *)(tx_buf), 0x40,
                    (unsigned)(tx_buf), 4);
    sprintf(msg, "\nFrame [%d], Rx_buf-", frame_no);
    dismem_x(TRUE, msg, (unsigned char *)(rx_buf), 0x40,
                    (unsigned)(rx_buf), 4);
	qe_hdlc_param_ram_dump();
	qe_hdlc_show_bd();
	free_nm((void *)vir_ptr);
	free_nm((void *)g_txbuf_dummy);
	free_nm((void *)g_rxbuf_dummy);
	restore_gpio_pin();
	
	return (FAILED);
    }

    if (rx_bd_ptr->status & (QE_HDLC_RX_BDSTAT_LG |
			     QE_HDLC_RX_BDSTAT_NO |
			     QE_HDLC_RX_BDSTAT_AB |
			     QE_HDLC_RX_BDSTAT_CR |
			     QE_HDLC_RX_BDSTAT_OV |
			     QE_HDLC_RX_BDSTAT_CD)) {
	sprintf(err_msg, "\n%s, [#%d]:Rx Frame Error, status = 0x%04x"
			,__FUNCTION__, __LINE__, rx_bd_ptr->status);
	print_err(FALSE, err_msg, LVL_1);
	if (rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_LG) {
	    sprintf(err_msg, "\n%s, [#%d]:Rx Frame length violation\n"
	    		,__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	}
	if (rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_NO) {
	    sprintf(err_msg, "\n%s, [#%d]:Rx No-octet aligned frame\n"
	    		,__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	}
	if (rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_AB) {
	    sprintf(err_msg, "\n%s, [#%d]:Rx Abprt sequence \n"
	    		,__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	}
	if (rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_CR) {
	    sprintf(err_msg, "\n%s, [#%d]:Rx CRC error\n",__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	}
	if (rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_OV) {
	    sprintf(err_msg, "\n%s, [#%d]:Rx Overrun\n",__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	}
	if (rx_bd_ptr->status & QE_HDLC_RX_BDSTAT_CD) {
	    sprintf(err_msg, "\n%s, [#%d]:Rx Carrier detect lost\n"
	    		,__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	}

	printf("\nRx data :-");
	dismem((unsigned char *)(rx_buf), buf_size,
	       (unsigned)(rx_buf), 4);
	/* Print dismem to host */
    sprintf(msg, "\nFrame [%d], Rx data :-\n", frame_no);
    dismem_x(TRUE, msg, (unsigned char *)(rx_buf), 0x40,
                    (unsigned)(rx_buf), 4);

	disable_ucc(QE_UCC3);
	free_nm((void *)vir_ptr);
	free_nm((void *)g_txbuf_dummy);
	free_nm((void *)g_rxbuf_dummy);
	restore_gpio_pin();
	return (FAILED);
    }

#if SECOND_RXBD
    /************************ Check the 2nd rx descriptor *******************/
    for (i = 0; i < 1000; i++) {
	if ((rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_E) == 0) {
	    break;
	}
	msleep(10);
    }

    dismem((unsigned char *)(ucc_param_ptr), 128,
	   (unsigned)(ucc_param_ptr), 4);
    dismem((unsigned char *)(rx_bd_ptr1), 128,
            (unsigned)(rx_bd_ptr), 4);
    dismem((unsigned char *)(g_rxbuf_dummy), 128,
	   (unsigned)(rx_buf), 4);
    dismem((unsigned char *)(g_txbuf_dummy), 128,
	   (unsigned)(tx_buf), 4);
    printf("\nAfter dismem");

    if (i == 1000) {
	printf("\nFailure to receive data, rx_bd_ptr1->status = 0x%08x\n",
	       rx_bd_ptr1->status);
	dismem((unsigned char *)(ucc_param_ptr), 128,
            (unsigned)(ucc_param_ptr), 4);
	dismem((unsigned char *)(rx_bd_ptr1), 128,
            (unsigned)(rx_bd_ptr), 4);
	dismem((unsigned char *)(g_rxbuf_dummy), 128,
            (unsigned)(rx_buf), 4);
	dismem((unsigned char *)(g_txbuf_dummy), 128,
            (unsigned)(tx_buf), 4);
	qe_hdlc_param_ram_dump();
	qe_hdlc_show_bd();
	free_nm((void *)vir_ptr);
	free_nm(g_txbuf_dummy);
	free_nm(g_rxbuf_dummy);
	restore_gpio_pin();
	return (FAILED);
    }
    printf("\n%s:%d\n", __FUNCTION__, __LINE__);

    if (rx_bd_ptr1->status & (QE_HDLC_RX_BDSTAT_LG |
			     QE_HDLC_RX_BDSTAT_NO |
			     QE_HDLC_RX_BDSTAT_AB |
			     QE_HDLC_RX_BDSTAT_CR |
			     QE_HDLC_RX_BDSTAT_OV |
			     QE_HDLC_RX_BDSTAT_CD)) {
	
	if (rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_LG) {
	    printf("\nRx Frame length violation\n");
	}
	if (rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_NO) {
	    printf("\nRx No-octet aligned frame\n");
	}
	if (rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_AB) {
	    printf("\nRx Abprt sequence \n");
	}
	if (rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_CR) {
	    printf("\nRx CRC error\n");
	}
	if (rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_OV) {
	    printf("\nRx Overrun\n");
	}
	if (rx_bd_ptr1->status & QE_HDLC_RX_BDSTAT_CD) {
	    printf("\nRx Carrier detect lost\n");
	}
	free_nm(g_txbuf_dummy);
	free_nm(g_rxbuf_dummy);

    }
    /********************* End of checking 2nd descriptor **********************/
#endif
    

    for (i = 0; i < 100; i++) {
	if ((*ucce_p & RXF) == 0) {
	    msleep(10);
	} else {
	    break;
	}
    }

    if (i == 100) {
	sprintf(err_msg, "\n%s, [#%d]:Time out, do not receive frame, "
			"ucce = 0x%08x\n",__FUNCTION__, __LINE__, *ucce_p);
	print_err(FALSE, err_msg, LVL_1);
	qe_hdlc_show_bd();
	free_nm((void *)g_txbuf_dummy);
	free_nm((void *)g_rxbuf_dummy);
	restore_gpio_pin();
	return (FAILED);
    } else {
	hdlc_rx_frames++;
	/* Clear RXF flag to allow a new event to be detected */
	*ucce_p = RXF;
    }
    
    /*
     * For Poll Mode test, clear any pending UCC Events,
     * pending interrupt condition and status.
     */
    *ucce_p = ALL_QE_INTERRUPTS;

    disable_ucc(QE_UCC3);

    if (tx_bd_ptr->status & QE_HDLC_TX_BDSTAT_R) {
	free_nm((void *)g_txbuf_dummy);
	free_nm((void *)g_rxbuf_dummy);
	restore_gpio_pin();
	sprintf(err_msg, "\n%s, [#%d]:No data transmitted\n"
			,__FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }
    
    if (hdlc_rx_frames == 0) {
	sprintf(err_msg, "\n%s, [#%d]:Do not receive frame\n"
			,__FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }

    tx_temp = (uchar *)tx_buf;
    rx_temp = (uchar *)rx_buf;
    for (i = 0; i < tx_bd_ptr->length; i++) {
	if (rx_buf[i] != tx_buf[i]) {
	    sprintf(err_msg, "\n%s, [#%d]:Mismatch at rx_buf[%d] "
	    		"expect 0x%02x, receive 0x%02x\n",__FUNCTION__, __LINE__,
		   i, tx_buf[i], rx_buf[i]);
	    print_err(FALSE, err_msg, LVL_1);
	    qe_hdlc_show_bd();
	    free_nm((void *)g_txbuf_dummy);
	    free_nm((void *)g_rxbuf_dummy);
	    restore_gpio_pin();
	    return (FAILED);
	}
    }

#ifdef DEBUG    
    tx_temp = (uchar *)g_txbuf_dummy;
    rx_temp = (uchar *)g_rxbuf_dummy;
    for (i = 0; i < tx_bd_ptr1->length; i++) {
	if (rx_temp[i] != tx_temp[i]) {
	    printf("\nMismatch at rx_temp[%d] expect 0x%02x, receive 0x%02x\n",
		   i, tx_temp[i], rx_temp[i]);
	    qe_hdlc_show_bd();
	    free_nm(g_txbuf_dummy);
	    free_nm(g_rxbuf_dummy);
	    restore_gpio_pin();
	    return (FAILED);
	}
    }

    qe_hdlc_show_bd();
#endif    
    /* Set the empty bit in the RX BD to allow another frame to be
       received */

    free_nm((void *)vir_ptr);
#if TEST
    free_nm((void *)g_rxbuf);
    free_nm((void *)g_rxbuf1);
    free_nm((void *)g_txbuf);
    free_nm((void *)g_txbuf1);
#endif

    free_nm((void *)g_txbuf_dummy);
    free_nm((void *)g_rxbuf_dummy);
    restore_gpio_pin();
	
#ifdef DEBUG    
    printf("\nEnd of %s", __FUNCTION__);
#endif    
    return (PASSED);
    
}




/*------------------------------------------------------------------------------
 * $Log: p1021_hdlc.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.14  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.13  2012/03/27 07:49:59  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.12  2012/03/21 08:38:20  steja
 * Include  the verilink subrate on the loopback test
 *
 * Revision 1.1.4.11  2012/03/12 23:00:55  huanngo
 * Fix the bug when transmitting more than 1717 bytes in HDLC
 *
 * Revision 1.1.4.10  2012/02/28 02:20:45  huanngo
 * Restore the GPIO pin PA31 otherwise the console will be messed up
 *
 * Revision 1.1.4.9  2012/02/02 20:54:00  huanngo
 * Adding more debug printing
 *
 * Revision 1.1.4.8  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.7  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.6  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.5  2011/10/12 08:49:45  steja
 * Fix the compile error lpbk_op
 *
 * Revision 1.1.4.4  2011/10/11 23:30:55  huanngo
 * Fix bug for UCC internal loopback on Patriot
 *
 * Revision 1.1.4.3  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:24  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.8  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.7  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.6  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.5  2011/07/08 00:08:48  huanngo
 * Clean up code
 *
 * Revision 1.1.2.4  2011/07/01 22:13:02  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.3  2011/06/28 06:27:55  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.2  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.1  2011/05/09 21:10:32  huanngo
 * Initial check in to support HDLC over TDM interface
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
