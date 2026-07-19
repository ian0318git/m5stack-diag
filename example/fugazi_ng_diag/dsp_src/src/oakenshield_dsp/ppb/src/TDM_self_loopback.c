/* $Id: TDM_self_loopback.c,v 1.2 2017/07/28 07:58:50 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/TDM_self_loopback.c,v $
 *------------------------------------------------------------------
 * TDM_self_loopback.c
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

#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"

#include "libtdm.h"
#include "libuart.h"

#include "libppbint.h"

#define NUMBER_OF_CHANNEL	256
#define N_BITS_PER_SAMPLE_PER_CH	8
#define NUMBER_OF_SAMPLE_BUFFER	8

#ifdef LSI_SP27XX_BUILT_FOR_DSS
#define ADDR_CONVERT_DSSLMEM_TO_DBM(addr)	(((0xf0000000&(addr))==0x40000000)\
											?(0x80000000|(sp_read_cpuid()<<25)|((addr)&0x3FFFF)):(addr))
#endif

/* TDM configuration option */
uint32_t swtu_conf = TWO_DIMENSIONAL;

#define BUFFERING_OPTION DOUBLE_BUFFERING

volatile uint8_t siu_src[MAX_NUM_PORT][NUMBER_OF_CHANNEL][NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1)];
volatile uint8_t siu_dst[MAX_NUM_PORT][NUMBER_OF_CHANNEL][NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1)];

uint32_t tdm_intr_received[MAX_NUM_PORT] = {0};

void TDM_isr(void)
{
	int i;
	uint32_t intr_status;

sp_SerialPutS("\r\n -------------------------- in TDM_isr() ");
	/* check which port received interrupt */
	for(i=0; i<MAX_NUM_PORT; i++)
	{
		if((intr_status = sp_TdmIdentifyIntr(TDM(i)))!=0)
		{
			/* clear it */
			sp_TdmIntrClr(TDM(i), intr_status);

			/* and stop the port */
			sp_TdmStop(TDM(i));

			/* set the flag */
			tdm_intr_received[TDM(i)] = 1;
		}
	}

	/* clear interrupt in the core */
#ifdef LSI_SP27XX_BUILT_FOR_DSS
	DSS_REG->clearint1.fields.tdm_int = 1;
#endif
}

int tdm_int_lpbk(void)
{
	uint32_t fail;
	int32_t retval;
	int32_t i, j, k;
	uint32_t *src_buf_base, *dst_buf_base;
    uint32_t intr_status;


#if 0
// SR already done 
	if ((retval = enable_pll(30, 50, 2)) != 0) {
		if (retval == -1) {
			/* second call required for SP2704 first silicon */
			retval = enable_pll(30, 50, 2);
		}
		if (retval != 0) {
			while(1);
		}
	}
#endif

/* enabling TDM interrupt */

#ifdef LSI_SP27XX_BUILT_FOR_DSS

	di();

	/* clear any pending core ints */
	DSS_REG->clearint0.reg = 0xFFFFFFFF;
	DSS_REG->clearint1.reg = 0xFFFFFFFF;
	DSS_REG->clearint2.reg = 0xFFFFFFFF;

	ICU_REG->mipl[TDM_offset].fields.ipl= 6;
	DSS_REG->maskstat1.fields.tdm_int = 1;

	ei();

#else
	sp_InitInterruptController(); /* enable interrupt controller */

	/* Connect TDM_isr to TDM interrupt */
	sp_SetInterrupt(HW_INT_NUM_TO_ID(INT_TDM), TYPE_INT_TDM, PRIORITY0, &TDM_isr);
#endif

	/* initialize UART */
	SP_INIT_SERIAL(375, 9600); /* SYSCLK = 375MHz and baud rate = 9600 */

	PRINT_STR("\f");

	PRINT_STR("** SIU port0~5 self loopback Test\r\r");
	PRINT_STR("** Test environment:\r");
	PRINT_STR("** Number of ports tested: ");
	PRINT_DEC(MAX_NUM_PORT);
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
	for (i=0; i<MAX_NUM_PORT; i++)
	{
		PRINT_STR("  PORT");
		PRINT_DEC(i);
		PRINT_STR(" channel initialization starts..\r");

		PRINT_STR("  - Initialize source memory space at ");
		PRINT_HEX((uint32_t)&siu_src[i][0][0]);
		PRINT_STR(" .");

		/* fill out src space with SAMPLE number */
		for (j=0; j<NUMBER_OF_CHANNEL; j++)
		{
			if(j%10==0)
			{
				PRINT_STR(".");
			}

			for (k=0; k<NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1); k++)
			{
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
		sp_TdmSIUConfInternalLB(TDM(i), NUMBER_OF_CHANNEL, BIT_SIZE_8, CH_MODE, 23 /* for 16.3MHz of SIU clk */);
		PRINT_STR("... Done\r");

		PRINT_STR("  - Enabling channels .");
		/* enable channels */
		sp_TdmEnableInChan(TDM(i), 0, NUMBER_OF_CHANNEL-1);
		sp_TdmEnableOutChan(TDM(i), 0, NUMBER_OF_CHANNEL-1);
		PRINT_STR("... Done\r");

		PRINT_STR("  - SWTU initialize .");

		if (swtu_conf==ONE_DIMENSIONAL) { /* configuration for one-dimensional mode */

				sp_TdmSWTU1Dconfig(TDM(i), AUTOLOAD_ON, SIGCON_LCOL2_LROW_INTC, (BUFFERING_OPTION+1)*NUMBER_OF_CHANNEL);

				PRINT_STR("    SWTU port");
				PRINT_DEC(TDM(i));
				PRINT_STR(" initialized for 1D mode\r");
		} else if (swtu_conf==TWO_DIMENSIONAL_BACKWARD_COMP) { /* configuration for old two-dimensional mode */

				/* configure SWTU for 2D backward compatible mode */
				sp_TdmSWTU2D_backward_config(TDM(i), AUTOLOAD_ON, (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER,
						(BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER*(NUMBER_OF_CHANNEL-1)-1,
                        SIGCON_LCOL2_LROW_INTC, (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);
				PRINT_STR("    SWTU port");
				PRINT_DEC(TDM(i));
				PRINT_STR(" initialized for 2D Backward Compatible mode\r");
		} else { /* swtu_conf==TWO_DIMENSIONAL */ /* configuration for new two-dimensional mode */

				sp_TdmSWTU2Dconfig(TDM(i), AUTOLOAD_ON, SIGCON_LCOL2_LROW_INTC,
                                (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);
				PRINT_STR("    SWTU port");
				PRINT_DEC(TDM(i));
				PRINT_STR(" initialized for 2D Normal mode\r");
		}
/* address translation for DSS core */
#ifdef LSI_SP27XX_BUILT_FOR_DSS
		src_buf_base = (uint32_t*)ADDR_CONVERT_DSSLMEM_TO_DBM((uint32_t)&siu_src[i][0][0]);
		dst_buf_base = (uint32_t*)ADDR_CONVERT_DSSLMEM_TO_DBM((uint32_t)&siu_dst[i][0][0]);
#else
		src_buf_base = (uint32_t*)&siu_src[i][0][0];
		dst_buf_base = (uint32_t*)&siu_dst[i][0][0];
#endif

		if (swtu_conf == ONE_DIMENSIONAL) {
			sp_Tdm1D_InitChan(TDM(i), NUMBER_OF_CHANNEL, NUMBER_OF_SAMPLE_BUFFER,\
							src_buf_base, dst_buf_base, BUFFERING_OPTION);
		} else if (swtu_conf == TWO_DIMENSIONAL_BACKWARD_COMP) {
			sp_Tdm2D_Backward_InitChan(TDM(i), NUMBER_OF_CHANNEL, NUMBER_OF_SAMPLE_BUFFER,\
							src_buf_base, dst_buf_base, BUFFERING_OPTION);
		} else { /* swtu_conf == TWO_DIMENSIONAL */
			sp_Tdm2DInitChan(TDM(i), 0, NUMBER_OF_CHANNEL-1, N_BITS_PER_SAMPLE_PER_CH, NUMBER_OF_SAMPLE_BUFFER, \
						src_buf_base, dst_buf_base, NOT_USING_UNIVERSAL_COUNTER, BUFFERING_OPTION);
		}
		PRINT_STR("... Done\r\r");
	}

	/* Enable interrupt before we start TDM */
	for(i=0; i<MAX_NUM_PORT; i++) {
		sp_TdmIntrInit(TDM(i), TDM_INTR_SWTU_DST);
	}

	PRINT_STR("\r** Now we start all 6 TDM ports...\r");

	for(i=0; i<MAX_NUM_PORT; i++) {
		sp_TdmInternalClkRun(TDM(i));
		sp_TdmRun(TDM(i));

		PRINT_STR("  - PORT ");
		PRINT_DEC(i);
		PRINT_STR(" is free running..\r");
	}

	fail = 0;
    /* SR New code */
    for (i=0;i<100;i++) {
        PRINT_STR(".");
    }
    sp_SerialPutS("\r\n Before interrupt");
        /* check which port received interrupt */
    for(i=0; i<MAX_NUM_PORT; i++) {
        PRINT_STR("\r\n intr_status  for port ");
        PRINT_DEC(i);
        if((intr_status = sp_TdmIdentifyIntr(TDM(i)))!=0) {
            PRINT_STR("\r\n intr_status  for port ");
            PRINT_DEC(i);
            PRINT_STR("\r\n Received intr_status  = ");
            PRINT_HEX(intr_status);

            /* clear it */
            sp_TdmIntrClr(TDM(i), intr_status);

            /* and stop the port */
            sp_TdmStop(TDM(i));

            /* set the flag */
            tdm_intr_received[TDM(i)] = 1;
        }
    PRINT_STR("\r\n Received intr_status  = ");
    PRINT_HEX(intr_status);
    }
    /* SR New code end */

	/* wait until core receives interrupt signals from all 6 ports */
	while (!(tdm_intr_received[TDM(0)]\
	      &&tdm_intr_received[TDM(1)]\
	      &&tdm_intr_received[TDM(2)]\
	      &&tdm_intr_received[TDM(3)]\
	      &&tdm_intr_received[TDM(4)]\
	      &&tdm_intr_received[TDM(5)]));
    sp_SerialPutS("\r\n After interrupt");

	PRINT_STR("\r\r** Check data @ destination \r");

	for(i=0; i<MAX_NUM_PORT; i++) {
		PRINT_STR("  PORT ");
		PRINT_DEC(i);
		PRINT_STR(": ");

		fail = 0;

		/* fill out src space with SAMPLE number */
		for(j=0; j<NUMBER_OF_CHANNEL; j++) {
			for(k=0; k<NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1); k++) {
				if(siu_dst[i][j][k] != j) {
					/* fail */
					fail++;
				}
			}
		}

		if(fail>0) {
			PRINT_DEC(fail);
			PRINT_STR(" Errors found\r");
			retval += fail;
		} else {
			PRINT_STR("No error found\r");
		}
	}

	PRINT_STR("\r");

	if (retval) {
		lsi_mg_test_fail();
	} else {
		lsi_mg_test_pass();
	}
	return (retval);
}
/******** History ********
$Log: TDM_self_loopback.c,v $
Revision 1.2  2017/07/28 07:58:50  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:44:02  srane
Initial checkin


$Endlog$
*/

