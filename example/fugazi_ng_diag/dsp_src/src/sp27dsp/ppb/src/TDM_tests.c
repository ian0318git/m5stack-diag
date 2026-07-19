/* $Id: TDM_tests.c,v 1.9 2013/03/13 17:30:09 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/TDM_tests.c,v $
 *------------------------------------------------------------------
 * TDM_tests.c
 *     TDM functions 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
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

#include "libtdm.h"
#include "libuart.h"

#include "libppbint.h"
#include "arm_common.h"
#include "diag_ppb.h"
#include "debug_console.h"
#include "uart.h"

#define NUMBER_OF_CHANNEL	256
#define N_BITS_PER_SAMPLE_PER_CH	8
#define NUMBER_OF_SAMPLE_BUFFER	8

int tdm_lpbk (int lpbk);

/* TDM configuration option */
uint32_t swtu_conf = TWO_DIMENSIONAL;

#define BUFFERING_OPTION DOUBLE_BUFFERING

volatile uint8_t siu_src[MAX_NUM_PORT][NUMBER_OF_CHANNEL][NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1)];
volatile uint8_t siu_dst[MAX_NUM_PORT][NUMBER_OF_CHANNEL][NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1)];

uint32_t tdm_intr_received[MAX_NUM_PORT] = {0};

extern volatile dspif_info_t *hd_if;

extern void _start(void);

extern void odo_exception_handler(void);
extern uint32_t sp_UartIntrInit(int intr_type);
extern unsigned char pid[128];
extern int dc_slot;

void TDM_isr(void)
{
    int i;
    uint32_t intr_status;

    //sp_SerialPutS("\r\n -------------------------- in TDM_isr() ");
    /* check which port received interrupt */
    for (i=0; i<MAX_NUM_PORT; i++) {
        if ((intr_status = sp_TdmIdentifyIntr(TDM(i)))!=0) {
            //sp_SerialPutS("\r\n -------------------------- in TDM_isr() for port : ");
            //PRINT_DEC(i);
            /* clear it */
            sp_TdmIntrClr(TDM(i), intr_status);

            /* Keep port running */
            //sp_TdmStop(TDM(i));

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
        return (tdm_lpbk(EXTERNAL));
        PRINT_STR("TDM External loopback not supported on onboard slot");
}

int tdm_lpbk (int lpbk)
{
    uint32_t fail, no_ports;
    uint32_t *src_buf_base, *dst_buf_base;
    int32_t retval;
    int32_t i, j, k, wastetime;

    if (lpbk == INTERNAL) {
        if (dc_slot != 0)  {
            PRINT_STR("\r\n TDM Internal Loopback test supported in onboard slot only.");
            bsp_debug_printf("\r\n PID of NGVM = %s", pid);
            sprintf((char *)&(hd_if->errmsg), "TDM Internal Loopback test supported in onboard slot only. PID of NGVM = %s, DC slot %d", pid, dc_slot);
            return (0);
        }
    }
    /* Install the exception vectors */
    if ((retval = enable_pll(30, 50, 2)) != 0) {
        if (retval == -1) {
        /* second call required for SP2704 first silicon */
            retval = enable_pll(30, 50, 2);
        }
        if (retval != 0) {
            sprintf((char *)&(hd_if->errmsg), "enable_pll() failed tdm_lpbk\n");
            return (1);
            //while(1);
        }
    }

    /* enabling TDM interrupt */
    sp_InitInterruptController(); /* enable interrupt controller */

    /* Connect TDM_isr to TDM interrupt */
    sp_SetInterrupt(HW_INT_NUM_TO_ID(INT_TDM), TYPE_INT_TDM, PRIORITY0, 
                    &TDM_isr);

    /* initialize UART */
    SP_INIT_SERIAL(375, 9600); /* SYSCLK = 375MHz and baud rate = 9600 */

    PRINT_STR("\f");

    if (lpbk == INTERNAL) {
        PRINT_STR("** SIU port0~5 Internal loopback Test\r\r");
        no_ports = MAX_NUM_PORT;
    } else {
        PRINT_STR("** SIU port0~1 External loopback Test\r\r");
        no_ports = 2;
    }
    PRINT_STR("** Test environment:\r");
    PRINT_STR("** Number of ports tested: ");
    //PRINT_DEC(MAX_NUM_PORT);
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
    for (i=0; i<no_ports; i++) {
        PRINT_STR("  PORT");
        PRINT_DEC(i);
        PRINT_STR(" channel initialization starts..\r");

        PRINT_STR("  - Initialize source memory space at ");
        PRINT_HEX((uint32_t)&siu_src[i][0][0]);
        PRINT_STR(" .");

        /* fill out src space with SAMPLE number */
        for (j=0; j<NUMBER_OF_CHANNEL; j++) {
            if (j%10==0) {
                PRINT_STR(".");
            }

            for(k=0; k<NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1); k++) {
                siu_src[i][j][k] = j;
                siu_dst[i][j][k] = 0;
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
            (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER*(NUMBER_OF_CHANNEL-1)-1,             SIGCON_LCOL2_LROW_INTC, (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);
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
        src_buf_base = (uint32_t*)&siu_src[i][0][0];
        dst_buf_base = (uint32_t*)&siu_dst[i][0][0];

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
    for(i=0; i<no_ports; i++) {
        sp_TdmIntrInit(TDM(i), TDM_INTR_SWTU_DST);
    }

    for(i=0; i<no_ports; i++) {
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
   if (no_ports == MAX_NUM_PORT) {
        while (!(tdm_intr_received[TDM(0)]\
                &&tdm_intr_received[TDM(1)]\
                &&tdm_intr_received[TDM(2)]\
                &&tdm_intr_received[TDM(3)]\
                &&tdm_intr_received[TDM(4)]\
                &&tdm_intr_received[TDM(5)])) {
        if (wastetime == 0)
            break;
        wastetime--;
        }
    } else {
       while (!(tdm_intr_received[TDM(0)]\
                &&tdm_intr_received[TDM(1)])) {
            if (wastetime == 0)
                break;
            wastetime--;
        }
    }

    if (wastetime == 0) {
        PRINT_STR(" No TDM interrupt received \r");
        sprintf((char *)&(hd_if->errmsg), "TDM_lpbk() TDM interrupt Error. TDM interrupt for Port 0 = %d, and for port 1 = %d \n", (int)tdm_intr_received[TDM(0)], (int)tdm_intr_received[TDM(1)]);
        retval = 1;
        PRINT_STR(" Test FAILED");
        cterr('f', 0, "TDM Loopback Failed");
        //lsi_mg_test_fail();
        for (i=0; i<no_ports; i++)
            sp_TdmSIUResetInternalLB(i);
        return (retval);
    }
        
    PRINT_STR("\r\r** Check data @ destination \r");

    for (i=0; i<no_ports; i++) {
        PRINT_STR("  PORT ");
        PRINT_DEC(i);
        PRINT_STR(": ");

        fail = 0;

        /* fill out src space with SAMPLE number */
        for (j=0; j<NUMBER_OF_CHANNEL; j++) {
            for (k=0; k<NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1); k++) {
                if (siu_dst[i][j][k] != j) {
                    /* fail */
#if 0
                    PRINT_STR(" \r\n i,j,k = ");
                    PRINT_DEC(i);
                    PRINT_STR(", ");
                    PRINT_DEC(j);
                    PRINT_STR(", ");
                    PRINT_DEC(k);
                    PRINT_STR(" : = ");
                    PRINT_DEC(siu_dst[i][j][k]);
#endif
                    fail++;
                } else {
#if 0
                    PRINT_STR(" \r\n i,j,k = ");
                    PRINT_DEC(i);
                    PRINT_STR(", ");
                    PRINT_DEC(j);
                    PRINT_STR(", ");
                    PRINT_DEC(k);
                    PRINT_STR(" : = ");
                    PRINT_DEC(siu_dst[i][j][k]);
#endif
;
                }
            }
        }

        if (fail>0) {
            PRINT_DEC(fail);
            PRINT_STR(" Errors found\r");
            sprintf((char *)&(hd_if->errmsg), "TDM_lpbk() data compare failed for port%d, channel %d, sample %d, total failures = %d\n", (int)i, (int)j, (int)k, (int)fail);
            retval += fail;
        } else {
            PRINT_STR("No error found\r");
        }
    } 

    PRINT_STR("\r");

    if (retval) {
        PRINT_STR(" Test FAILED");
        cterr('f', 0, "TDM Loopback Failed");
        //lsi_mg_test_fail();
    } else {
        PRINT_STR(" Test PASSED");
        //lsi_mg_test_pass();
    }
    for (i=0; i<no_ports; i++)
        sp_TdmSIUResetInternalLB(i);
    return (retval);
}

/******** History ********
$Log: TDM_tests.c,v $
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

