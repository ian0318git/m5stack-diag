/* $Id: fpga_sfp_intr.c,v 1.5 2012/06/27 01:49:17 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/fpga_sfp_intr.c,v $
 *------------------------------------------------------------------
 * FPGA SFP interrupt to data plane CPU
 * 
 * Dec 2012 ptong
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "error.h"
#include "ethernet.h"
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "cvmx.h"

#undef DEBUG

/* FPGA interrupt pin is wired to Cavium GPIO bit 2
 */
#define FPGA_INTR_GPIO_MASK   0x00000004

/**********************************************************************
 *
 * Function: en_sfp_intr_type
 *    Enable or disable a specific type of interrupt on a specific SFP.
 * 
 * Input: sfp_num - SFP0 to SFP3
 *        intr_type - one of the 3 interrupt type
 *        flag - flag to set or clear the enable
 *
 * Return: none
 */
void
en_sfp_intr_type(int sfp_num, int intr_type, int flag)
{
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;
    unsigned int *cfg_p, itype;

    switch(sfp_num){
    case SFP0:
      cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp0_conf;
	break;
    case SFP1:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp1_conf;
	break;
    case SFP2:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp2_conf;
	break;
    case SFP3:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp3_conf;
	break;
    default:
        printf("SFP port number %d is invalid\n", sfp_num);
	break;
    }

    switch(intr_type) {
    case SFP_TX_FAULT_INTR:
	itype = SFP_TX_FAULT_INTR_EN;
	break;
    case SFP_LOSS_SIG_INTR:
        itype = SFP_LOSS_SIG_INTR_EN;
	break;
    case SFP_PRESENT_INTR :
        itype = SFP_PRESENT_INTR_EN;
	break;
    default:
        printf("SFP intr type %#.8x is invalid\n", intr_type);
	break;
    }

    if (flag == TRUE) {
        *cfg_p |= itype;
    }
    else {
        *cfg_p &= ~itype;
    }
}

/**********************************************************************
 *
 * Function: en_sfp_intr
 *    Enable or disable the interrupt on a specific SFP
 * 
 * Input: sfp_num - SFP0 to SFP3
 *        flag - flag to set or clear the enable big
 *
 * Return: none
 */
void
en_sfp_intr(int sfp_num, int flag)
{
    unsigned long addr = get_platform_intr_ctrl_addr(FP);
    fpga_intr_t *intrp = (fpga_intr_t *)addr;
    unsigned int sfpien;

    sfpien = (1 << sfp_num);
    if (flag == TRUE) {
	intrp->sfp_en |= sfpien;
    }
    else {
	intrp->sfp_en &= ~sfpien;
    }
}

/**********************************************************************
 *
 * Function: en_sfp_top_intr
 *    Enable or disable the SFP interrupt in the top level register
 * 
 * Input: flag - enable or disable the interrupt
 *
 * Return: none
 */
void
en_sfp_top_intr(int flag)
{
    unsigned long addr = get_platform_intr_ctrl_addr(FP);
    fpga_intr_t *intrp = (fpga_intr_t *)addr;

    if (flag == TRUE) {
	intrp->top_en |= FPGA_SFP_INTR;
    }
    else {
	intrp->top_en &= ~FPGA_SFP_INTR;
    }
}

/**********************************************************************
 *
 * Function: ovride_sfp_intr_type
 *    Set or clear the interrupt override bit for a specific SFP
 *    and a specific interrupt type.
 * 
 * Input: sfp_num - SFP0 to SFP3
 *        intr_type - one of the 3 interrupt type
 *        flag - flag to set or clear the bit
 *
 * Return: none
 */
void
ovride_sfp_intr_type(int sfp_num, int intr_type, int flag)
{
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;
    unsigned int *cfg_p, itype;

    switch(sfp_num){
    case SFP0:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp0_conf;
	break;
    case SFP1:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp1_conf;
	break;
    case SFP2:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp2_conf;
	break;
    case SFP3:
        cfg_p = (unsigned int *) &sfp_stat_ctrl->sfp3_conf;
	break;
    default:
        printf("SFP port number %d is invalid\n", sfp_num);
	break;
    }

    switch(intr_type) {
    case SFP_TX_FAULT_INTR:
	itype = SFP_TX_FAULT_INTR_OVERRIDE;
	break;
    case SFP_LOSS_SIG_INTR:
        itype = SFP_LOSS_SIG_INTR_OVERRIDE;
	break;
    case SFP_PRESENT_INTR :
        itype = SFP_PRESENT_INTR_OVERRIDE;
	break;
    default:
        printf("SFP intr type %#.8x is invalid\n", intr_type);
	break;
    }

    if (flag == TRUE) {
        *cfg_p |= itype;
    }
    else {
        *cfg_p &= ~itype;
    }
}

/**********************************************************************
 *
 * Function: show_sfp_stscfg
 *    Display the status and config registers
 * 
 * Input: sfp_num - SFP0 - SFP3
 *
 * Return: none
 */
void
show_sfp_stscfg(int sfp_num, char *msg_p)
{
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;
    fpga_intr_t *intrp;
    unsigned int *sts_p, temp;

    switch(sfp_num){
    case SFP0:
        sts_p = (unsigned int *) &sfp_stat_ctrl->sfp0_intr;
	break;
    case SFP1:
        sts_p = (unsigned int *) &sfp_stat_ctrl->sfp1_intr;
	break;
    case SFP2:
        sts_p = (unsigned int *) &sfp_stat_ctrl->sfp2_intr;
	break;
    case SFP3:
        sts_p = (unsigned int *) &sfp_stat_ctrl->sfp3_intr;
	break;
    default:
        printf("SFP port number %d is invalid\n", sfp_num);
	break;
    }

    addr = get_platform_intr_ctrl_addr(FP);
    intrp = (fpga_intr_t *)addr;
    temp = 0;

#if DEBUG
    printf("--- %s\n", msg_p);
    printf("SFP %d intr sts= %#.8x\n", sfp_num, *sts_p);
    printf("SFP %d intr cfg= %#.8x\n", sfp_num, *(sts_p+1));
    printf("sfp_sts %#.8x\n", intrp->sfp_sts);
    printf("sfp_en %#.8x\n", intrp->sfp_en);
    printf("top_sts %#.8x\n", intrp->top_sts);
    printf("top_en %#.8x\n", intrp->top_en);
#else
    temp = *sts_p;
    temp = *(sts_p+1);
    temp = intrp->sfp_sts;
    temp = intrp->top_sts;
#endif
}

/**********************************************************************
 *
 * Function: clr_sfp_all_intr_en
 *    Clear all the interrupt enable bits in the SFP config reg
 * 
 * Input: sfp_num - SFP0 - SFP3
 *
 * Return: none
 */
void
clr_sfp_all_intr_en(int sfp_num)
{
    en_sfp_intr_type(sfp_num, SFP_TX_FAULT_INTR, FALSE);
    en_sfp_intr_type(sfp_num, SFP_LOSS_SIG_INTR, FALSE);
    en_sfp_intr_type(sfp_num, SFP_PRESENT_INTR, FALSE);
    en_sfp_intr(sfp_num, FALSE);
}

/**********************************************************************
 *
 * Function: clr_all_sfp_intr_en
 *    Clear all the interrupt enable bits for all SFP
 * 
 * Input: none
 *
 * Return: none
 */
void
clr_all_sfp_intr_en(void)
{
    int sfp_num;

    for (sfp_num=SFP0; sfp_num <= SFP3; sfp_num++) {
        clr_sfp_all_intr_en(sfp_num);
    }
    en_sfp_top_intr(FALSE);
}

/**********************************************************************
 *
 * Function: clr_sfp_all_ovride
 *    Clear all the interrupt override bits in the SFP config reg
 * 
 * Input: sfp_num - SFP0 to SFP3
 *
 * Return: none
 */
void
clr_sfp_all_ovride(int sfp_num)
{
    ovride_sfp_intr_type(sfp_num, SFP_TX_FAULT_INTR, FALSE);
    ovride_sfp_intr_type(sfp_num, SFP_LOSS_SIG_INTR, FALSE);
    ovride_sfp_intr_type(sfp_num, SFP_PRESENT_INTR, FALSE);
}

/**********************************************************************
 *
 * Function: clr_all_sfp_ovride
 *    Clear all the interrupt override bits for all SFP
 * 
 * Input: none
 *
 * Return: none
 */
void
clr_all_sfp_ovride(void)
{
    int sfp_num;

    for (sfp_num=SFP0; sfp_num <= SFP3; sfp_num++) {
        clr_sfp_all_ovride(sfp_num);
    }
}

/**********************************************************************
 *
 * Function: get_sfp_intr_sts
 *    Get the top level status register and the sfp level status register
 * 
 * Input: sfp_num - SFP0 to SFP3
 *        top_sfpsts - address of buffer to hold top level intr status reg
 *        sfpsts - ptr to buffer to hold the SFP level interupt status reg
 *
 * Return: none
 */
void
get_sfp_intr_sts(int sfp_num, uint32_t *top_sfpsts, uint32 *sfpsts)
{

    unsigned long addr = get_platform_intr_ctrl_addr(FP);
    fpga_intr_t *intrp = (fpga_intr_t *)addr;

    *sfpsts = intrp->sfp_sts;
    *top_sfpsts = intrp->top_sts;
}

/**********************************************************************
 *
 * Function: get_gpio_rx_dat_bits
 *    Get the Cavium GPIO bit value specified by the bit mask
 * 
 * Input: bitmask - bit mask for the GPIO bits
 *
 * Return: GPIO value
 */
uint32_t
get_gpio_rx_dat_bits (uint32_t bitmask)
{
    cvmx_gpio_rx_dat_t gpio_rx_dat;
    uint32_t bitval;

    /* Make sure gmxno is clear for rx */
    gpio_rx_dat.u64 = cvmx_read_csr(CVMX_GPIO_RX_DAT);
    bitval = (uint32_t)gpio_rx_dat.u64 & bitmask;

#if DEBUG
    printf("%s gpio_rx_dat= 0x%lx bitval= %x\n",
	   __FUNCTION__, gpio_rx_dat.u64, bitval);
#endif

    return(bitval);
}

/**********************************************************************
 *
 * Function: sfp_intr_test_type
 *    Test a specific type of interrupt of a specific SFP
 * 
 * Input: sfp_num - SFP0 - SFP3
 *        intr_type - one of the 3 interrupt type
 *
 * Return: pass/fail
 */
int
sfp_intr_test_type(int sfp_num, int intr_type)
{
    uint32_t fpga_intr_bit;
    uint32_t top_sfpsts, sfpsts;

#if DEBUG
    printf("\n");
    show_sfp_stscfg(sfp_num, "init state of stscfg");
#endif

    /* Set up interrupt enable bit at all levels
     */
    en_sfp_intr_type(sfp_num, intr_type, TRUE);
    en_sfp_intr(sfp_num, TRUE);
    en_sfp_top_intr(TRUE);

#if DEBUG
    show_sfp_stscfg(sfp_num, "enable is set");
#endif

    /* Use the override bit to simulate the interrupt event.
     */
    ovride_sfp_intr_type(sfp_num, intr_type, TRUE);
    msleep(1);
    show_sfp_stscfg(sfp_num, "override is set");

    /* Check if the FPGA SFP interrupt status bit are set accordingly
     */
    get_sfp_intr_sts(sfp_num, &top_sfpsts, &sfpsts);
    if ((top_sfpsts & FPGA_SFP_INTR) == 0) {
        printf("SFP interrupt status bit is not set in the FPGA intr status reg.\n");
	return(FAIL);
    }

    if ((sfpsts != (1 << sfp_num))) {
        printf("SFP %d interrup status bit is not set in SFP intr status reg.\n", sfp_num);
	return(FAIL);
    }
  
    /* Check if the Cavium GPIO bit is asserted.
     * The FPGA implemented the assertion of the interrupt
     * to be low.
     */
    fpga_intr_bit = get_gpio_rx_dat_bits(FPGA_INTR_GPIO_MASK);
    if (fpga_intr_bit != 0) {
        printf("FPGA did not assert interrupt signal\n");
	return(FAIL);
    }

    /* Clear the override bit
     */
    ovride_sfp_intr_type(sfp_num, intr_type, FALSE);
    msleep(1);
    show_sfp_stscfg(sfp_num, "override is cleared");

    /* Check if the FPGA SFP interrupt status bit are cleared accordingly
     */
    get_sfp_intr_sts(sfp_num, &top_sfpsts, &sfpsts);
    if ((top_sfpsts & FPGA_SFP_INTR) != 0) {
        printf("SFP interrupt status bit is not cleared in the FPGA intr status reg.\n");
	return(FAIL);
    }

    if (sfpsts != 0) {
        printf("SFP %d interrup status bit is not cleared in SFP intr status reg.\n", sfp_num);
	return(FAIL);
    }
  
    /* Check if the Cavium GPIO bit is de-asserted.
     */
    fpga_intr_bit = get_gpio_rx_dat_bits(FPGA_INTR_GPIO_MASK);
    if (fpga_intr_bit == 0) {
        printf("FPGA did not de-assert interrupt signal\n");
	return(FAIL);
    }

    return(PASS);
}

/**********************************************************************
 *
 * Function: fpga_sfp_intr_test
 *    Test the FPGA SFP interrupt to the data plane CPU
 * 
 * Input: none
 *
 * Return: none
 */
int
fpga_sfp_intr_test(void)
{
    unsigned int result = PASS;
    int sfp, ii, itype;
    char *tname = "FPGA SFP intr to Cavium";

    testname("%s", tname);
    prpass(testpass, "");
    for (sfp=SFP0; sfp <= SFP3; sfp++) {
        /* Test the 3 SFp interrupt types
	 */
        for (ii=0; ii < 3; ii++) {
	    switch (ii) {
	    case 0:
	        itype = SFP_PRESENT_INTR;
		break;
	    case 1:
	        itype = SFP_LOSS_SIG_INTR;
	        break;
	    case 2:
	        itype = SFP_TX_FAULT_INTR;
	        break;
	    }

	    clr_all_sfp_intr_en();
	    clr_all_sfp_ovride();
	    result = sfp_intr_test_type(sfp, itype);
	    if (result == FAIL) {
	        cterr('f',0,"%s test failed at SFP %d type %#.8x\n",
		      tname, sfp, itype);
		break;
	    }
	}
	if (result == FAIL)  break;
    }

    clr_all_sfp_intr_en();
    clr_all_sfp_ovride();

    return(result);
}

/*-------------------------------------------------
$Log: fpga_sfp_intr.c,v $
Revision 1.5  2012/06/27 01:49:17  ptong
Read status reg to clear a status bit

Revision 1.4  2012/06/21 19:52:42  ptong
Read status reg to clear a status bit

Revision 1.3  2012/06/20 02:25:55  ptong
Fix compile warning

Revision 1.2  2012/06/19 23:12:27  ptong
Fix minor test issue

Revision 1.1  2012/06/14 22:37:43  ptong
Add FPGA SFP interrupt test

$Endlog$
*/
