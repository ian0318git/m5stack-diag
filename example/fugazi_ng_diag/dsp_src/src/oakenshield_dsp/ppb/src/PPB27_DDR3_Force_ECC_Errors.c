/* $Id: PPB27_DDR3_Force_ECC_Errors.c,v 1.2 2017/07/28 07:58:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/PPB27_DDR3_Force_ECC_Errors.c,v $
 *------------------------------------------------------------------
 * PPB27_DDR3_Force_ECC_Errors.c 
 * Description: SP27XX DDR3 Program That Forces ECC Errors
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2012 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * $RCSfile: PPB27_DDR3_Force_ECC_Errors.c,v $
 *
 * Description:	SP27XX DDR3 Program That Forces ECC Errors
 *
 * - Sets up DDR3 controller for LSI Evaluation Boards
 * - Initializes entire DDR3 memory space with DMA controller to initialize ECC
 * - Forces single and multibit ECC Errors, and displays notifications of the
 *   ECC errors out the UART
 *
 * Authors    :   JWB / VL
 *
 *****************************************************************************/

/* standard header files */
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libddr3.h"
#include "libdma.h"
#include "libppbint.h"
#include "DDR3_forceECC.h"
#include <stdio.h>
#include "libuart.h"
#include "common.h"
#include "diag_ppb.h"
#include "uart.h"

#define PLL_MULTIPLIER 30
#define PLL_F_CKI 50
#define PLL_OUTPUT_DIVIDER 2
#define LSI_MG_DDR3MEM_BASE 0xE0000000

uint32_t ctrl_raw_temp;
uint32_t ctrl_raw_temp1;
uint32_t single_bit_correct;
uint32_t single_bit_uncorrect;
uint32_t multi_bit_uncorrect;
uint32_t multi_bit_correct;

extern volatile dspif_info_t *hd_if;

/* Force a single-bit error into a specific bit position in a
 * 32-bit word, or force a multibit error into a specific
 * 32-bit word.  This 32-bit word is part of the 128-bit user word.
 * Make sure that userword_base is 16-byte aligned.
 */

static void force_ECC_error(user_word_num_t word, bit_pos_t bitposition, 
                            uint32_t userword_base);

/* common routine to report DDR3 fatal Errors */
/* in: indicates the function in which error occurred */
void ddr3_init_fail (ddr3Stat_t val)		
{
    /* library routine lsi_mg_report() should be overridden by user error 
       handler */
    lsi_mg_report(0xDD300000 | (uint32_t) val);
}

/* Data abort handler */
void data_abort_handler (void)
{
    uint32_t addr, data, src_id, synd;

    PRINT_STR("\r\n ++ data_abort_handler +++n");

    /* Check if any of the DDR3 multibit ECC error interrupt raw status bits 
       are set and clear them */

    if ((DDR3_REG->denali_ctl_71.fields.single_uncorr_ecc == 0) && 
        (DDR3_REG->denali_ctl_71.fields.mult_uncorr_ecc == 0)) {
        PRINT_STR("\r\nERROR:  Data abort encountered which is unrelated to ");
        PRINT_STR("the DDR3 ECC example\r");
        return;
    } else {
        if (DDR3_REG->denali_ctl_71.fields.single_uncorr_ecc == 1) {
            single_bit_uncorrect = 1;
            PRINT_STR("\r\nDetected Single uncorrectable (multibit) ECC error event ");
            REG32_SET_BITS(LSI_SP27XX_DDR3_DENALI_CTL_69_RA, 
                           LSI_SP27XX_DENALI_CTL_71_SINGLE_UNCORR_ECC_BM);
        }
        if (DDR3_REG->denali_ctl_71.fields.mult_uncorr_ecc == 1) {
            multi_bit_uncorrect = 1;
            PRINT_STR("\r\nDetected Multiple uncorrectable (multibit) ECC error ");
            REG32_SET_BITS(LSI_SP27XX_DDR3_DENALI_CTL_69_RA, 
                           LSI_SP27XX_DENALI_CTL_71_MULT_UNCORR_ECC_BM);
        }
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_109_RA, addr);
        PRINT_STR("\r\ndenali_ctl_109 (address field) = ");
        PRINT_HEX((uint32_t)(addr));
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_84_RA, data);
        PRINT_STR("\r\ndenali_ctl_84 (data field) = ");
        PRINT_HEX((uint32_t)(data));
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_47_RA, src_id);
        PRINT_STR("\r\ndenali_ctl_47 (DBM master port [bit 16-31]) = ");
        PRINT_HEX((uint32_t)(src_id));
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_33_RA, synd);
        PRINT_STR("\r\ndenali_ctl_33 (Syndrome [bit 16-22]) = ");
        PRINT_HEX((uint32_t)(synd));
    }
    /* Clean up */
    DDR3_REG->denali_ctl_04.fields.fwc = 0;
    /* Write the original CTRL_RAW bit back to the CTL_15 register */
    DDR3_REG->denali_ctl_15.fields.ctrl_raw = ctrl_raw_temp;
}

/* DDR3 interrupt handler */
void DDR3_interrupt_handler (void)
{
    uint32_t addr, data, src_id, synd;

    PRINT_STR("\r\n ++++ DDR3_interrupt_handler +++ \n");
    /* Check if any of the DDR3 single-bit ECC error interrupt raw status 
       bits are set and clear them */
    if ((DDR3_REG->denali_ctl_71.fields.single_corr_ecc == 0) && 
        (DDR3_REG->denali_ctl_71.fields.mult_corr_ecc == 0)) {
        PRINT_STR("\r\nERROR:  Unexpected DDR3 interrupt\r");
        return;
    } else {
        if (DDR3_REG->denali_ctl_71.fields.single_corr_ecc == 1) {
            single_bit_correct = 1;
            PRINT_STR("\r\nDetected Single correctable (single-bit) ECC error event");
            REG32_SET_BITS(LSI_SP27XX_DDR3_DENALI_CTL_69_RA, 
                           LSI_SP27XX_DENALI_CTL_71_SINGLE_CORR_ECC_BM);
        }
        if (DDR3_REG->denali_ctl_71.fields.mult_corr_ecc == 1) {
            multi_bit_correct = 1;
            PRINT_STR("\r\nDetected Multiple correctable (single-bit) ECC error ");
            REG32_SET_BITS(LSI_SP27XX_DDR3_DENALI_CTL_69_RA, 
                           LSI_SP27XX_DENALI_CTL_71_MULT_CORR_ECC_BM);
        }
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_107_RA, addr);
        PRINT_STR("\r\ndenali_ctl_107 (address field) = ");
        PRINT_HEX((uint32_t)(addr));
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_83_RA, data);
        PRINT_STR("\r\ndenali_ctl_83 (data field) = ");
        PRINT_HEX((uint32_t)(data));
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_47_RA, src_id);
        PRINT_STR("\r\ndenali_ctl_47 (DBM master port [bit 0-15]) = ");
        PRINT_HEX((uint32_t)(src_id));
        REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_33_RA, synd);
        PRINT_STR("\r\ndenali_ctl_33 (Syndrome [bit 8-14]) = ");
        PRINT_HEX((uint32_t)(synd));
        
    }
    /* Clean up */
    DDR3_REG->denali_ctl_04.fields.fwc = 0;
    /* Write the original CTRL_RAW bit back to the CTL_15 register */
    DDR3_REG->denali_ctl_15.fields.ctrl_raw = ctrl_raw_temp;

}

int ecc_mem_test (void)
{
    uint32_t ddrMemSize;
    uint32_t temp;
    int result = PASSED;

    multi_bit_correct = 0;
    multi_bit_uncorrect = 0;
    single_bit_correct = 0;
    single_bit_uncorrect = 0;

    /* test all 512M */
    ddrMemSize = LSI_MG_DDR3MEM_BSIZE;				
   
    if (sp_dma_init(DBM_DMAC0, (uint32_t)&g_dma_desc 
        /* all descriptors will be in PPB local memory */, 0) != 0) {
        /* dma0 initialization failed */
        PRINT_STR("\r\n*** ERROR: sp_dma_init() failed during ECC Memory test :ERROR_BAD_PARAM\r");
        PRINT_STR("\r\n*** ECC test failed");
        sprintf((char *)&(hd_if->errmsg[0]),
                "sp_dma_init() failed during ECC Memory test : ERROR_BAD_PARAM\n");
        uart_puts((char *)hd_if->errmsg);
        cterr('f', 0, (char *)hd_if->errmsg);
        return (FAILED);
    }

    /* set all locations in DDR3 to initialize ECC */

    sp_dbmdmac_MemSet128((void *)LSI_MG_DDR3MEM_BASE, 0xCAFE4DAD, ddrMemSize/16);
    PRINT_STR("\r\nInitialized ECC on DDR3 memory\r");

    /* make sure ECC status is not set */

    if (ddr3_hasECCerrors()) {

        PRINT_STR("\r\n*** ERROR:  ECC status is set before running ECC example\r");
        sprintf((char *)&(hd_if->errmsg[0]),
                "ddr3_hasECCerrors() failed during ECC Memory test : ECC status is set before running ECC example\n");
        cterr('f', 0, (char *)hd_if->errmsg);
        return (FAILED);


    }

    /* Enable only DDR3 single-bit ECC error interrupts to propagate toward 
       the ARM */

    REG32_SET_BITS(LSI_SP27XX_DDR3_DENALI_CTL_70_RA, 
                   LSI_SP27XX_DENALI_CTL_70_RM);

    REG32_RESET_BITS(LSI_SP27XX_DDR3_DENALI_CTL_70_RA,	
                     LSI_SP27XX_DENALI_CTL_71_OR_ALL_BITS_BM |
                     LSI_SP27XX_DENALI_CTL_71_SINGLE_CORR_ECC_BM |
                     LSI_SP27XX_DENALI_CTL_71_MULT_CORR_ECC_BM);

    /* enable interrupt controller */

    sp_InitInterruptController();

    /* Connect DDR3_interrupt_handler to DDR3 interrupt */

    sp_SetInterrupt(HW_INT_NUM_TO_ID(INT_DDR3), TYPE_INT_DDR3, PRIORITY0, 
                    &DDR3_interrupt_handler);

    /* Force a single-bit DDR3 ECC error */

    PRINT_STR("\r\nForcing single-bit ECC error on data bit 7 of word 3\r");

    force_ECC_error(WORD3, DATA7, 0xE0000000);

    if (single_bit_correct == 1) {
        PRINT_STR("\r\n Received single_bit_correctable event\n");
        result |= PASSED;
    } else {
        PRINT_STR("\r\n Did not receive single_bit_correctable event \n");
        result |= FAILED;
    }

    /* Force a multibit DDR3 ECC error */

    PRINT_STR("\r\nForcing multibit ECC error on word 1\r");

    force_ECC_error(WORD1, MULTIBIT_ERROR, 0xE0000020);

    if (single_bit_uncorrect == 1) {
        if (result != PASSED) {
            sprintf((char *)&(hd_if->errmsg[0]),
                    "Did not receive single_bit_correctable event\n");
        }
        PRINT_STR("\r\n Received single_bit_uncorrectable event \n");
        result |= PASSED;
    } else {
        PRINT_STR("\r\n Did not receive single_bit_uncorrectable event \n");
        if (result != PASSED) {
            sprintf((char *)&(hd_if->errmsg[0]),
                    "Did not receive single_bit_correctable and single_bit_uncorrectable event\n");
        }
        result |= FAILED;
    }

    PRINT_STR("\r\nECC example completed...\r\r");
    REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_71_RA, temp);
    PRINT_STR("\r\ndenali_ctl_71 = ");
    PRINT_HEX((uint32_t)(temp));
    REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_174_RA, temp);
    PRINT_STR("\r\ndenali_ctl_174 = ");
    PRINT_HEX((uint32_t)(temp));

    if (result == PASSED) {
        PRINT_STR("\r\nECC test passed");
    } else {
        PRINT_STR("\r\n*** ECC test failed");
    }
    return (result);

}

void force_ECC_error (user_word_num_t word, bit_pos_t bitposition, 
                      uint32_t userword_base)
{
    uint32_t read_val;

    /* Tell the controller to report ECC events on reads,
     * but to not correct them. */
    ctrl_raw_temp = DDR3_REG->denali_ctl_15.fields.ctrl_raw;

    DDR3_REG->denali_ctl_15.fields.ctrl_raw = 1;
    REG32_READ(LSI_SP27XX_DDR3_DENALI_CTL_15_RA, ctrl_raw_temp1);

    PRINT_STR("\r\ndenali_ctl_15 = ");
    PRINT_HEX((uint32_t)(ctrl_raw_temp1));

    /* Prepare the ECC error to get generated  */
    DDR3_REG->denali_ctl_74.fields.xor_check_bits = (bitposition << (word * 7));
    /* Set the FWC bit to force a write check */
    DDR3_REG->denali_ctl_04.fields.fwc = 1;

    /* Write one 128-bit burst of data to DDR3 to create the ECC error */

    sp_dbmdmac_MemSet128((void *)userword_base, 0x55555555, 1);
    PRINT_STR("\r\ntriggering ECC event");

    /* Read a 32-bit location in the user word to trigger the ECC error event */
    /* Debug dump the DRAM register here */
   
    read_val = *(uint32_t *)userword_base;

    /* Write a 64-byte burst of data to DDR3 to re-initialize the ECC
     * for memory that includes the 128-bit user word */

    sp_dbmdmac_MemSet128((void *)userword_base, 0xFFFFFFFF, 4);

}

/******** History ********
$Log: PPB27_DDR3_Force_ECC_Errors.c,v $
Revision 1.2  2017/07/28 07:58:50  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:37  harrchan
Initial commit code for Oakenshield

Revision 1.3  2012/09/10 06:36:09  srane
Add details to errmsg.

Revision 1.2  2012/08/15 14:52:23  srane
cleanup code.

Revision 1.1  2012/06/07 22:34:29  srane
Initial checkin for ECC memory test.


$Endlog$
*/

