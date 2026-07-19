/* $Id: TDM_tests.c,v 1.3 2021/04/15 00:53:07 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/TDM_tests.c,v $
 *------------------------------------------------------------------
 * TDM_tests.c
 *     TDM functions 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*-----------------------------------------------------------------------*
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 *  * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * TDM_self_loopback.c - Test all 6 TDM ports using SIU self loop-back capability
 *
 * From the "StarPro SP2700 DSP Family Register Programming Guide":
 *
 *    Each SIU includes an internal diagnostic mode to verify the functionality of the SIU
 *    without requiring system intervention. If the SIOLB field (SCON0[24]) is set, the SIU
 *    output data pin (SOD) is internally looped back to the SIU input data pin (SID), the
 *    output bit clock is internally connected to the input bit clock, and the output frame
 *    sync is internally connected to the input frame sync. Any input at the SID pin is ignored
 *    while loopback is enabled.
 *
 *-----------------------------------------------------------------------*/
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "types.h"
#include "libtdm.h"
#include "libuart.h"

#include "libppbint.h"
#include "arm_common.h"
#include "diag_ppb.h"
#include "debug_console.h"
#include "uart.h"
#include "tdm_utils.h"
#include "common.h"
#include "diag_fpga.h"
#include "tdmsw16_fpga.h"


int tdm_lpbk (int lpbk);

/* TDM configuration option */
uint32_t swtu_conf = TWO_DIMENSIONAL;

uint32_t tdm_intr_received[MAX_NUM_PORT] = {0};

extern dspif_info_t *hd_if;

extern void _start(void);

extern void odo_exception_handler(void);
extern uint32_t sp_UartIntrInit(int intr_type);
extern unsigned char pid[128];
extern int dc_slot;

void TDM_isr(void)
{
    int i;
    uint32_t intr_status;

    
    /* check which port received interrupt */
    for (i=0; i<MAX_NUM_PORT; i++) {
        if ((intr_status = sp_TdmIdentifyIntr(TDM(i)))!=0) {
            /* clear it */
            sp_TdmIntrClr(TDM(i), intr_status);


            /* set the flag */
            tdm_intr_received[TDM(i)] = 1;
        }
    }

    /* clear interrupt in the core */
}

/* indentify the interrupt of certain port */
uint32_t sp_UartIdentifyIntr(void)
{
    uint32_t intr_stat = 0;
    uint32_t raw_intr_stat = 0;

    REG32_READ(LSI_SP27XX_UARTRIS_RA, raw_intr_stat);
    REG32_READ(LSI_SP27XX_UARTMIS_RA, intr_stat);

    PRINT_STR("\r\nLSI_SP27XX_UARTRIS_RA = ");
    PRINT_HEX(LSI_SP27XX_UARTRIS_RA);
    PRINT_STR(" content = ");
    PRINT_HEX(raw_intr_stat);
    PRINT_STR("\r\nLSI_SP27XX_UARTMIS_RA = ");
    PRINT_HEX(LSI_SP27XX_UARTMIS_RA);
    PRINT_STR(" content = ");
    PRINT_HEX(intr_stat);

    return intr_stat;
}

void UART_isr(void)
{
    uint32_t intr_status;

    sp_SerialPutS("\r\n -------------------------- in UART_isr() ");
    /* check which port received interrupt */
    if ((intr_status = sp_UartIdentifyIntr())!=0) {
        /* clear it */
        //sp_UartIntrClr(intr_status);
        ;
    }
}

int tdm_int_lpbk (int lpbk)
{
    return (tdm_lpbk(INTERNAL));
}

int tdm_ext_lpbk (int lpbk)
{
    int result = FAILED, stream1, stream2;
    uchar board_id = get_oak_id();
    char errstr[128]; 

#if 0
    /* Disconnect from the Codecs */
    oak_tdm_xc_setup(board_id, FALSE);
    msleep(1000);
#endif

    if (is_phoenix()) {
        /* Set the TDM loopback */
        stream1 = TDMSW16_DSP0_STREAM; /* DSP SIU0 only */
        stream2 = TDMSW16_DSP1_STREAM; /* DSP SIU1 only */

        if ((set_tdmsw_lpbk_test(stream1, TRUE) == FAILED) ||
            (set_tdmsw_lpbk_test(stream2, TRUE) == FAILED)) {
            cterr('f', 0, "%s Unable to set stream1 %d or stream2 %d in loopback mode",
                  __FUNCTION__, stream1, stream2);
            snprintf(errstr, 128,
                     "\n%s Unable to set stream1 %d or stream2 %d in loopback mode\n",
                     __FUNCTION__, stream1, stream2);
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }

        msleep(1000);
    } else {
        /* Set the TDM loopback */
        stream1 = TDMSW16_DSP0_STREAM; /* DSP SIU0 only */

        if (set_tdmsw_lpbk_test(stream1, TRUE) == FAILED) {
            cterr('f', 0, "%s Unable to set stream %d in loopback mode",
                  __FUNCTION__, stream1);
            snprintf(errstr, 128,
                    "\n%s Unable to set stream %d in loopback mode\n",
                    __FUNCTION__, stream1);
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }

        msleep(1000);
    }

    result = tdm_lpbk(EXTERNAL);

    PRINT_STR("Clean Loopback setup. \r");

    if (is_phoenix()) {
        /* Take the TDM out of loopback */
        if ((set_tdmsw_lpbk_test(stream1, FALSE) == FAILED) ||
            (set_tdmsw_lpbk_test(stream2, FALSE) == FAILED)){
            cterr('f', 0, "%s Unable to take stream1 %d or stream2 %d out of loopback mode",
                           __FUNCTION__, stream1, stream2);
            snprintf(errstr, 128,
                     "%s\nUnable to take stream1 %d and stream2 %d out of loopback mode\n",
                     __FUNCTION__, stream1, stream2);
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    } else {
        /* Take the TDM out of loopback */
        if (set_tdmsw_lpbk_test(stream1, FALSE) == FAILED) {
            cterr('f', 0, "%s Unable to take stream %d out of loopback mode",
                           __FUNCTION__, stream1);
            snprintf(errstr, 128,
                     "%s\nUnable to take stream %d out of loopback mode\n",
                     __FUNCTION__, stream1);
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    }

    /* connect TDM switch */
    oak_tdm_xc_setup(board_id, TRUE);


    return (result);
}

int tdm_lpbk (int lpbk)
{
    uint32_t fail, no_ports;
    uint32_t *src_buf_base, *dst_buf_base;
    int32_t retval;
    int32_t i, j, k, wastetime;

    /* enabling TDM interrupt */
    sp_InitInterruptController(); /* enable interrupt controller */

    /* Connect TDM_isr to TDM interrupt */
    sp_SetInterrupt(HW_INT_NUM_TO_ID(INT_TDM), TYPE_INT_TDM, PRIORITY0, 
                    &TDM_isr);

    /* initialize UART */
    SP_INIT_SERIAL(375, 9600); /* SYSCLK = 375MHz and baud rate = 9600 */

    PRINT_STR("\f");

    /* Phoenix need TDM SIU port 1 */
    if (is_phoenix()) {
        if (lpbk == INTERNAL) {
            PRINT_STR("** SIU port 0/1 Internal loopback Test\r\r");
        } else {
            PRINT_STR("** SIU port 0/1 External loopback Test\r\r");
        }
        no_ports = 2;
    } else {
        if (lpbk == INTERNAL) {
            PRINT_STR("** SIU port 0 Internal loopback Test\r\r");
        } else {
            PRINT_STR("** SIU port 0 External loopback Test\r\r");
        }
        no_ports = 1;
    }
    PRINT_STR("** Test environment:\r");
    PRINT_STR("** Number of ports tested: ");
    PRINT_DEC(no_ports);
    PRINT_STR("\r");
    PRINT_STR("** Number of channels/port: ");
    PRINT_DEC(NUMBER_OF_CHANNEL);
    PRINT_STR("\r");
    PRINT_STR("** Number of samples/channel: ");
    PRINT_DEC(NUMBER_OF_SAMPLE_BUFFER);
    PRINT_STR("\r");
    PRINT_STR("** Number of bits/sample: ");
    PRINT_DEC(N_BITS_PER_SAMPLE_PER_CH);
    PRINT_STR("\r\r");

    retval = 0;
    for (i = 0; i < no_ports; i++) {
        PRINT_STR("  PORT");
        PRINT_DEC(i);
        PRINT_STR(" channel initialization starts..\r");

        PRINT_STR("  - Initialize source memory space at ");
        PRINT_HEX((uint32_t)&SWTU0_SOURCEBUFFER[i][0][0]);
        PRINT_STR(" .");

        /* fill out src space with SAMPLE number */
        for (j = 0; j < NUMBER_OF_CHANNEL; j++) {

            if (j%10 == 0) {
                PRINT_STR(".");
            }

            for(k = 0; k < NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1); k++) {
                SWTU0_SOURCEBUFFER[i][j][k] = j;
                SWTU0_DESTINATIONBUFFER[i][j][k] = 0;
            }
        }
        PRINT_STR("Done\r");

        PRINT_STR("  - Disabling channels .");
        /* all channels unmasked by default */
        sp_TdmDisableInChan(TDM(i), 0, NUMBER_OF_CHANNEL-1);
        /* all channels unmasked by default */
        sp_TdmDisableOutChan(TDM(i), 0, NUMBER_OF_CHANNEL-1);
        PRINT_STR("... Done\r");

        PRINT_STR("  - SIU initialize .");
        if (lpbk == INTERNAL) {
            sp_TdmSIUConfInternalLB(lpbk, TDM(i), NUMBER_OF_CHANNEL, BIT_SIZE_8, 
                                CH_MODE, 23 /* for 16.3MHz of SIU clk */);
        } else {
            sp_TdmSIUConfExtLB_usingBB(TDM(i), NUMBER_OF_CHANNEL, BIT_SIZE_8, 
                                   CH_MODE);
        }
        PRINT_STR("... Done\r");

        PRINT_STR("  - Enabling channels .");
        /* enable channels */
        sp_TdmEnableInChan(TDM(i), 0, NUMBER_OF_CHANNEL-1);
        sp_TdmEnableOutChan(TDM(i), 0, NUMBER_OF_CHANNEL-1);
        PRINT_STR("... Done\r");

        PRINT_STR("  - SWTU initialize .");

        if (swtu_conf==ONE_DIMENSIONAL) {
            /* configuration for one-dimensional mode */
            sp_TdmSWTU1Dconfig(TDM(i), AUTOLOAD_ON, SIGCON_LCOL2_LROW_INTC, 
                       (BUFFERING_OPTION+1)*NUMBER_OF_CHANNEL);
            PRINT_STR("    SWTU port");
            PRINT_DEC(TDM(i));
            PRINT_STR(" initialized for 1D mode\r");
        } else if (swtu_conf==TWO_DIMENSIONAL_BACKWARD_COMP) {
            /* configuration for old two-dimensional mode */
            /* configure SWTU for 2D backward compatible mode */
            sp_TdmSWTU2D_backward_config(TDM(i), AUTOLOAD_ON, 
                (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER,
                (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER*(NUMBER_OF_CHANNEL-1)-1,             
                SIGCON_LCOL2_LROW_INTC, (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);
            PRINT_STR("    SWTU port");
            PRINT_DEC(TDM(i));
            PRINT_STR(" initialized for 2D Backward Compatible mode\r");
        } else {
            /* swtu_conf==TWO_DIMENSIONAL */ 
            /* configuration for new two-dimensional mode */
            sp_TdmSWTU2Dconfig(TDM(i), AUTOLOAD_ON, SIGCON_LCOL2_LROW_INTC, 
            (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);
            PRINT_STR("    SWTU port");
            PRINT_DEC(TDM(i));
            PRINT_STR(" initialized for 2D Normal mode\r");
        }

        /* address translation for DSS core */
        src_buf_base = (uint32_t*)&SWTU0_SOURCEBUFFER[i][0][0];
        dst_buf_base = (uint32_t*)&SWTU0_DESTINATIONBUFFER[i][0][0];

        if (swtu_conf == ONE_DIMENSIONAL) {
            sp_Tdm1D_InitChan(TDM(i), NUMBER_OF_CHANNEL, 
                      NUMBER_OF_SAMPLE_BUFFER, src_buf_base, dst_buf_base, 
                      BUFFERING_OPTION);
        } else if (swtu_conf == TWO_DIMENSIONAL_BACKWARD_COMP) {
            sp_Tdm2D_Backward_InitChan(TDM(i), NUMBER_OF_CHANNEL, 
            NUMBER_OF_SAMPLE_BUFFER, src_buf_base, dst_buf_base, 
            BUFFERING_OPTION);
        } else {
            /* swtu_conf == TWO_DIMENSIONAL */
            sp_Tdm2DInitChan(TDM(i), 0, NUMBER_OF_CHANNEL-1, 
            N_BITS_PER_SAMPLE_PER_CH, NUMBER_OF_SAMPLE_BUFFER, 
            src_buf_base, dst_buf_base, NOT_USING_UNIVERSAL_COUNTER, 
            BUFFERING_OPTION);
        }
        PRINT_STR("... Done\r\r");
    }

    /* Enable interrupt before we start TDM */
    for(i = 0; i < no_ports; i++) {
        sp_TdmIntrInit(TDM(i), TDM_INTR_SWTU_DST);
    }
    
    for(i = 0; i < no_ports; i++) {
        PRINT_STR("\r** Now we start TDM port...");
        PRINT_DEC(i);
        sp_TdmInternalClkRun(TDM(i));
        sp_TdmRun(TDM(i));

        PRINT_STR("  - PORT ");
        PRINT_DEC(i);
        PRINT_STR(" is free running..\r");
    }

    fail = 0;

    /* wait until core receives interrupt signals from all 6 ports */
    /* SR Put timing detail here so the test does not hang */
    wastetime = 100;
    if (is_phoenix()) {
        while ((!(tdm_intr_received[TDM(0)])) || (!(tdm_intr_received[TDM(1)]))) { 
            if (wastetime == 0) {
                break;
           }
           wastetime--;
        }
    } else {
        while (!(tdm_intr_received[TDM(0)])) { 
            if (wastetime == 0) {
                break;
           }
           wastetime--;
        }
    }

    if (wastetime == 0) {
        PRINT_STR(" No TDM interrupt received \r");
        sprintf((char *)&(hd_if->errmsg), "TDM_lpbk() TDM interrupt Error.\
                        TDM interrupt for Port 0 = %d, and for port 1 = %d \n",
                        (int)tdm_intr_received[TDM(0)], (int)tdm_intr_received[TDM(1)]);
        retval = 1;
        PRINT_STR(" Test FAILED");
        cterr('f', 0, "TDM Loopback Failed");
        for (i = 0; i < no_ports; i++) {
            sp_TdmSIUResetInternalLB(i);
        }
        return (retval);
    }
        
    PRINT_STR("\r\r** Check data @ destination \r");

    for (i = 0; i < no_ports; i++) {
        PRINT_STR("  PORT ");
        PRINT_DEC(i);
        PRINT_STR(": ");

        fail = 0;

        /* fill out src space with SAMPLE number */
        for (j = 0; j < NUMBER_OF_CHANNEL; j++) {
            for (k = 0; k < NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1); k++) {
                if (SWTU0_DESTINATIONBUFFER[i][j][k] != j) {
                    fail++;
                } else {
                    ;
                }
            }
        }

        if (fail > 0) {
            PRINT_DEC(fail);
            PRINT_STR(" Errors found\r");
            sprintf((char *)&(hd_if->errmsg), "TDM_lpbk() data compare failed for port%d,\
                            channel %d, sample %d, total failures = %d\n", (int)i, (int)j, (int)k, (int)fail);
            retval += fail;
        } else {
            PRINT_STR("No error found\r");
        }
    } 

    PRINT_STR("\r");

    if (retval) {
        PRINT_STR(" Test FAILED\r");
        cterr('f', 0, "TDM Loopback Failed");
    } else {
        PRINT_STR(" Test PASSED\r");
    }

    for (i = 0; i < no_ports; i++) {
        sp_TdmSIUResetInternalLB(i);
    }

    return (retval);
}

/******** History ********
$Log: TDM_tests.c,v $
Revision 1.3  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.2  2017/07/28 07:58:50  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.9.78.5  2017/04/17 06:08:46  olin2
Remove Enable PLL function

Revision 1.9.78.4  2017/03/09 07:23:33  harrchan
Support oakenshield double wide case

Revision 1.9.78.3  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.9.78.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.9.78.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield

Revision 1.9  2013/03/13 17:30:09  srane
Add a timeout for the while loop.

Revision 1.8  2012/09/10 06:37:10  srane
Remove while forever loop, add more detailed errmsg.

Revision 1.7  2012/08/28 18:23:46  srane
Add check for the dest_id in received messages from host and DC for TDM
test.

Revision 1.6  2012/08/15 14:52:23  srane
cleanup code.

Revision 1.5  2012/07/17 20:34:38  srane
cleanup

Revision 1.4  2012/06/28 21:25:56  srane
fix TDM isr, add delay for ethernet loopback etc

Revision 1.3  2012/06/07 22:50:59  srane
TDM external loopback, ECC memory test

Revision 1.2  2012/05/24 23:25:38  srane
Add GPIO code to set ready bit, uart test, support both
uart mode and ethernet mode, other cleanup

Revision 1.1  2012/05/10 22:57:58  srane
Add TDM support. Adjust the linker sections.



$Endlog$
*/

