/* $Id: diag_main.c,v 1.2 2017/07/28 07:58:36 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/src/diag_main.c,v $
 *------------------------------------------------------------------
 * diag_main.c
 *      Oakenshield - DSP DSS Core diagnostics main entry
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "stdio.h"
#include "string.h"
#include "ag_mg_regs.h"
#include "graffdsp.h"
#include "diag_dss.h"

volatile UINT16 dsp_freq_value = 300;

//#define DSS_DEBUG 1
/* function proto */
static void init_id (void);
static void diag_main(void);
static ulong runtest(void);
static uint stop_on_error(void);
uint test_gpio(void);


uint32_t uart_puts (int core, const char *str);
extern void bsp_debug_printf (int core, const char *fmt, ...);
extern void uart_put_long (int core, uint32_t val, uint32_t base);
extern int tdm_lpbk(void);
extern int test_sanity();


/* diag test table */
struct diag_table montalvo_test_table[]= {
    SELECT_DSP_SANITY,                  (PFI)test_sanity,
    SELECT_INTF_SYNC,                   (PFI)test_gpio,
};
#define MAX_NUM_TEST sizeof(montalvo_test_table)/sizeof(struct diag_table)

/* store magic number to tell ARM DSS core is ready */
UINT32	*dss0_ppb_if = (UINT32 *)0x002FE000;
UINT32	*dss1_ppb_if = (UINT32 *)0x002FE200;
UINT32	*dss2_ppb_if = (UINT32 *)0x002FE400;
UINT32	*dss3_ppb_if = (UINT32 *)0x002FE600;
volatile uint32_t	*uart_mem = (uint32_t *)0x002FF800;
volatile uint32_t	*uart_getlock = (uint32_t *)0x002FF840;
/* store arm-dss interface for command/result exchange */
dspif_info_t *dss0_dh_if = (dspif_info_t *)0x002FE800;
dspif_info_t *dss1_dh_if = (dspif_info_t *)0x002FEC00;
dspif_info_t *dss2_dh_if = (dspif_info_t *)0x002FF000;
dspif_info_t *dss3_dh_if = (dspif_info_t *)0x002FF400;
dspif_info_t *dh_if;

volatile ag_mg_regs_dss_reg_s *dss_regs =
    (volatile ag_mg_regs_dss_reg_s *)AG_MG_REGS_DSS_BASE;
volatile ag_mg_regs_tdm_reg_s *tdm_regs =
    (volatile ag_mg_regs_tdm_reg_s *)AG_MG_REGS_TDM_BASE;
volatile ag_mg_regs_car_reg_s *car_regs =
    (volatile ag_mg_regs_car_reg_s *)(AG_MG_REGS_CAR_BASE);
volatile ag_mg_regs_gpio_reg_s *gpio_regs =
    (volatile ag_mg_regs_gpio_reg_s *)AG_MG_REGS_GPIO_BASE;
volatile ag_mg_regs_gpio_reg_s *ppb_gpio_regs =
    (volatile ag_mg_regs_gpio_reg_s *)AG_MG_REGS_PPB_GPIO_BASE;
volatile ag_mg_regs_timer_reg_s *timer_regs =
    (volatile ag_mg_regs_timer_reg_s *)AG_MG_REGS_TIMER_BASE;
volatile ag_mg_regs_ddr3_reg_s *ddr3_regs =
    (volatile ag_mg_regs_ddr3_reg_s *)AG_MG_REGS_DDR3_BASE;
volatile ag_mg_regs_pcc_reg_s *pcc_regs =
    (volatile ag_mg_regs_pcc_reg_s *)AG_MG_REGS_PCC_BASE;


/* identify different DSPs */
dsp_type_t dsp_device;
dsp_type_t *dsp_device_p = &dsp_device;

//extern UINT32 init_gpio(void);
  
uint test_gpio (void)
{
    int i, data;
    volatile uint32_t *addr;
    uint32_t dss_gpio;

    dss_gpio = 0xcf004000;
    addr = (volatile uint32_t *)dss_gpio;

    addr = (volatile uint32_t *) (dss_gpio+0x400);

    bsp_debug_printf(CORE_DSS0, "\r\n DSS> In test_gpio sig = 0x%x, status = 0x%x, *gpio dir = 0x%x", dh_if->param1, dh_if->param2, *addr);

    if (dh_if->param2 & SET_LEVEL) { /* output signal */
        *addr = dh_if->param1;
        bsp_debug_printf(CORE_DSS0, "\r\n ouput direction *gpio_dir @0x%x = 0x%x", addr, *addr);
        for (i=0;i<100;i++)
            data = i;
        if (dh_if->param2 == SET_HIGH) {
            addr = (volatile uint32_t *)(dss_gpio+(dh_if->param1<<2));
            *addr = dh_if->param1;
bsp_debug_printf(CORE_DSS0, "\r\n DSS> written high at addr = 0x%x data = 0x%x", addr, *addr);
            //sp_SetGPIODataHigh(dh_if->param1);
        } else {
            addr = (volatile uint32_t *)(dss_gpio+(dh_if->param1<<2));
            *addr = *addr & ~(0x1 << (dh_if->param1-1));
bsp_debug_printf(CORE_DSS0, "\r\n DSS> written low at addr = 0x%x data = 0x%x", addr, *addr);
            //sp_SetGPIODataLow(dh_if->param1);
        }
    } else {
        *addr = *addr & ~(0x1 << (dh_if->param1-1));
        bsp_debug_printf(CORE_DSS0, "\r\n input direction *gpio_dir @0x%x = 0x%x", addr, *addr);
        for (i=0;i<100;i++)
            data = i;
        addr = (volatile uint32_t *)(dss_gpio+(dh_if->param1<<dh_if->param1));
        data = *addr;
bsp_debug_printf(CORE_DSS0, "\r\n data read @0x%x = 0x%x\n", addr, data);
        data = data & dh_if->param1;
        //data = sp_GetGPIOData(dh_if->param1);
bsp_debug_printf(CORE_DSS0, "\r\n data read = 0x%x\n", data);
        if (((data >> (dh_if->param1-1)) & 0x1) == (dh_if->param2 & 0x1)) {
bsp_debug_printf(CORE_DSS0, "\r\n DSS> test for sig = %d , status 0x%x PASSED\n", dh_if->param1, dh_if->param2);
            return (PASSED);
        } else {
            sprintf((char *)&(dh_if->errmsg), "Failed branch instruction");
bsp_debug_printf(CORE_DSS0, "\r\n DSS> test for sig = %d , status 0x%x FAILED\n", dh_if->param1, dh_if->param2);
            return (FAILED);
        }
    }
    return (PASSED);

}

/***********************************************************************
 *
 *  Function: main
 *
 *  Description: main entry for diag running on LSI StarPro26xx DSP
 *		 do some module initialization here first before jumping
 *		 into the diag main code.
 *
 **********************************************************************
 */
int main()
{   
#ifdef DSS_DEBUG
    //int num = 0;
#endif
    
	/*
     * identiy DSP type to find out Clock speed;
     * identy core number since only Core 0 does
     */
//uart_puts(CORE_DSS, "\r\n DSS> In _main");
    
    dsp_device_p = &dsp_device;
#ifdef DSS_DEBUG
//uart_puts(CORE_DSS, "\r\n DSS> In _main");
#endif

    init_id();
#ifdef DSS_DEBUG
    //num = dsp_device_p->core_id;
//bsp_debug_printf(CORE_DSS, "\nDSS%d> After _main NUM \n", num);
#endif

    /* only core 0 does init/config */
    if (dsp_device_p->core_id == DSS_CORE0) {
		/* 
		 * DSP Bootloader takes the configuration
		 * input from host and does the settings
		 * for PLL, DDR2.
		 */
		//if (init_gpio()){
		//	return (FAILED);
		//}
    }
    
    /* now it's ready to go to run diag tests */
    diag_main();

    //tdm_lpbk();

	return (0); /* main must return int, but will never reach here */
}

/***********************************************************************
 *
 * Function: init_id
 *
 * Description: Need to identify which DSP chip this is 
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
void init_id(void)
{
        /* read reg (DSP type, DSP FREQ) 
         * DEVICEID[7,1,0] = [0,0,0]. SP2603
         * DEVICEID[7,1,0] = [0,0,1]. SP2601
         * DEVICEID[7,1,0] = [0,1,0]. SP2602
         * PFUSE24[2:0] = [000], 500 MHz Changed to 550 MHz
         * PFUSE24[2:0] = [001], 350 MHz Changed to 400 MHz
         * PFUSE24[2:0] = [010], 250 MHz Changed to 300 MHz
         */
        /* store in DSP type struct */
car_regs = (volatile ag_mg_regs_car_reg_s *)(AG_MG_REGS_CAR_BASE);
dss_regs = (volatile ag_mg_regs_dss_reg_s *)AG_MG_REGS_DSS_BASE;

        dsp_device_p->device_type =
                car_regs->deviceid.fields.deviceid & (0x3);
#if 0
        dsp_device_p->device_freq =
                car_regs->pfuse[24].fields.pfuse & (0x7);
		if (dsp_device_p->device_freq == DSP_550MHZ) {
			dsp_freq_value = 550;
		}
		if (dsp_device_p->device_freq == DSP_400MHZ) {
			dsp_freq_value = 400;
		}
		if (dsp_device_p->device_freq == DSP_300MHZ) {
			dsp_freq_value = 300;
		}
#endif
        /* now, identify which DSP; it matters for MAC, TDM */
        /*  DSP CHIP_ID[0] : 0 chip 0, 1 chip 1 */
        dsp_device_p->chip_id =
                car_regs->chipid.fields.chipid & (0x1);
        /*
         * One of the DSS Control Registers, IDCODE (DSS Identification Register)
         * provides DSS core ID. Bits10-7 (DSSID) of IDCODE are 
         * hardwired physical ID for DSS core:
         * DSS0: DSSID[10-7] = 0000
         * DSS1: DSSID[10-7] = 0001
         * DSS2: DSSID[10-7] = 0010
         */
        dsp_device_p->core_id =
        	(dss_regs->idcode.fields.dssid);
        
#if 0
        /* identify number of channel supported */
        if ((dsp_device_p->device_type == DSP_SP2601_ID) && 
        	(dsp_device_p->device_freq == DSP_250MHZ)) {
        	dsp_device_p->num_chan = 16;
        }
        if ((dsp_device_p->device_type == DSP_SP2601_ID) && 
            (dsp_device_p->device_freq == DSP_350MHZ)) {
            dsp_device_p->num_chan = 32;
        }
        if (dsp_device_p->device_type == DSP_SP2602_ID){
        	dsp_device_p->num_chan = 64;
        }
        if (dsp_device_p->device_type == DSP_SP2603_ID){
        	dsp_device_p->num_chan = 128;
        }
#endif
}

#if 0
/***********************************************************************
 *
 * Function: time_delay
 *
 * Description: delay specified number of ms  
 *
 * Input : timeout value in number of 1ms
 *
 * Returns: none
 *
 **********************************************************************
 */
void time_delay (UINT32 onems) 
{
	UINT32 timer = 0;
	UINT32 timer_value = 0;

	/* calculate timer value:
	 * DSPCLK/4
	 * To get 1ms = (1/(DSPCLK/4) * timer_value
	 */
	timer_value = (dsp_freq_value / 4) * 1000;

	/* Init the timer first */
	timer_regs->timer[0].ctrl.reg = 
		(AGR_SP26XX_TIMER_CTRL_ONESH_BM  |       
		 AGR_SP26XX_TIMER_CTRL_SIZE_BM); 

	/* 
	 * Load the timeout value to the timer value register - 
	 */
	timer = onems * timer_value;
	timer_regs->timer[0].load.fields.loadval = timer;
	/* Enable the timer */
	timer_regs->timer[0].ctrl.reg |= AGR_SP26XX_TIMER_CTRL_EN_BM; /*0x80*/
 
	/* 
	 * Wait for the timeout
	 */
	while (timer != 0)
	{
        timer = timer_regs->timer[0].val.fields.currentval; 
	}
}

/***********************************************************************
 * us_delay()
 *
 * Description: delay specified number of us
 *
 * Input : timeout value in number of 1us
 *
 * Returns: none
 *
 **********************************************************************
 */
void us_delay(UINT32 oneus)
{
        UINT32 timer = 0;
        UINT32 timer_value = 0;

        /* calculate timer value:
         * DSPCLK/4
         * To get 1us = (1/(DSPCLK/4) * timer_value
         */
        timer_value = (dsp_freq_value / 4) ;

        /* Init the timer first */
        timer_regs->timer[0].ctrl.reg =
                (AGR_SP26XX_TIMER_CTRL_ONESH_BM  |
                 AGR_SP26XX_TIMER_CTRL_SIZE_BM);

        /*
         * Load the timeout value to the timer value register -
         */
        timer = oneus * timer_value;
        timer_regs->timer[0].load.fields.loadval = timer;
        /* Enable the timer */
        timer_regs->timer[0].ctrl.reg |= AGR_SP26XX_TIMER_CTRL_EN_BM; /*0x80*/

        /*
         * Wait for the timeout
         */
        while (timer != 0)
        {
        timer = timer_regs->timer[0].val.fields.currentval;
        }
}
#endif

/*
 **********************************************************************
 *
 *  Function: diag_main
 *
 *  Description: process test command and invoke test funtions
 *				 Ported from 5510 logic.
 *
 *	Input: None.
 *
 *	Output: None.
 *
 **********************************************************************
 */
static void diag_main(void)
{
    ulong counter, nerrors;
    uint idle;
    char core_str[10];
//uint32_t addr;

#ifdef DSS_DEBUG
//int i;
//UINT32 *uart_addr;

//uart_addr = (UINT32 *)0xC3047000;
    //counter = 0;
//uart_puts(CORE_DSS, " DSS> In diag_main");
#endif
dss0_ppb_if = (UINT32 *)0x002FE000;
//bsp_debug_printf("\n DSS> In diag_main dss0_ppb_if = 0x%x", dss0_ppb_if);
//addr = (uint32_t )dss0_ppb_if;
//uart_put_long(addr, 16);
dss1_ppb_if = (UINT32 *)0x002FE200;
dss2_ppb_if = (UINT32 *)0x002FE400;
dss3_ppb_if = (UINT32 *)0x002FE600;

    
	/* tell ARM/PPB that DSS Cores are ready */
    switch (dsp_device_p->core_id) {
    case DSS_CORE0:
                sprintf(core_str, "DSS_CORE0");
		dh_if = dss0_dh_if;
#ifdef DSS_DEBUG
bsp_debug_printf(CORE_DSS0, "                                                 ");
bsp_debug_printf(CORE_DSS0, "\r\n DSS0> In diag_main dss0_ppb_if = 0x%x", dss0_ppb_if);
/*
bsp_debug_printf(CORE_DSS0, "\r\n Dump UART Register \n");
for (i=0; i<0x45;i++) {
    bsp_debug_printf(CORE_DSS0, "\r\n @0x%x = 0x%x", uart_addr, *uart_addr);
    uart_addr++;
}
*/
#endif
    	*dss0_ppb_if = MAGIC;
    	break;
    case DSS_CORE1:
                sprintf(core_str, "DSS_CORE1");
		dh_if = dss1_dh_if;
#ifdef DSS_DEBUG
bsp_debug_printf(CORE_DSS1, "\n DSS1> In diag_main dss1_ppb_if = 0x%x", dss1_ppb_if);
#endif
        *dss1_ppb_if = MAGIC;
        break;
    case DSS_CORE2:
#ifdef DSS_DEBUG
bsp_debug_printf(CORE_DSS2, "\n DSS2> In diag_main dss2_ppb_if = 0x%x", dss2_ppb_if);
#endif
                sprintf(core_str, "DSS_CORE2");
		dh_if = dss2_dh_if;
        *dss2_ppb_if = MAGIC;

        break;
    case DSS_CORE3:
#ifdef DSS_DEBUG
bsp_debug_printf(CORE_DSS3, "\n DSS3> In diag_main dss3_ppb_if = 0x%x", dss3_ppb_if);
#endif
                sprintf(core_str, "DSS_CORE3");
		dh_if = dss2_dh_if;
		dh_if = dss3_dh_if;
        *dss3_ppb_if = MAGIC;
        break;
    }
    
	memset(dh_if, 0, sizeof(dspif_info_t));
//#if 0

    /* initialize the host interface */
    /* should ARM or DSS core init this??? */
    // SR done in ARM dh_if->command = CMD_RUN;
    dh_if->ack = ACK_NULL;
    dh_if->result = RESULT_RUNNING;
    dh_if->flags = FLAG_NULL;
    dh_if->select = SELECT_NULL;
    dh_if->faults = 0;
    dh_if->location = 0;
    dh_if->expected = 0;
    dh_if->actual = 0;
    dh_if->extra = 0;
    dh_if->errorcount = 0;
    dh_if->testcounter = 0;
    dh_if->ReadyOnTest = 0;
    dh_if->TestCtrl = 0;
    dh_if->WhoAmI = 0;
	
//bsp_debug_printf(CORE_DSS0, "\n *******DSS0> In diag_main dss0_ppb_if = 0x%x", dss0_ppb_if);
	sprintf((char *)&(dss0_dh_if->bufmsg), 
	"deviceid %x, chipid %x, freq %x",
	car_regs->deviceid.fields.deviceid,
	car_regs->chipid.fields.chipid,
	car_regs->pfuse[24].fields.pfuse);
    idle = TRUE;
//bsp_debug_printf(CORE_DSS0, "\n *******DSS0> before for loop");
    for (;;) {
        counter++;
        if ((counter >= 0x10000L) ||
                ((dh_if->flags & FLAG_CONT_RUN) &&
                        (dh_if->select != SELECT_NULL) &&
                        (dh_if->command == CMD_RUN))) {
            counter = 0;
        }
	    /* waiting for packets; parse it */ 
	    switch (dh_if->command) {
            /**
             * CMD_ABORT allows the host to break the DSP out of a
             * continuous loop caused by setting FLAG_CONT_RUN or
             * FLAG_LOOP_ON_ERROR.
             */

        case CMD_ABORT:
            if (!idle) {
                dh_if->ack = ACK_OK;
                dh_if->result = RESULT_ABORTED;
            }
            idle = TRUE;
            break;

            /**
             * CMD_RESET allows the host to reset the DSP to its initial
             * configuration.
             */

        case CMD_RESET:
            dh_if->result = RESULT_RUNNING;
            dh_if->ack = ACK_OK;
            idle = FALSE;
            dh_if->result = RESULT_SUCCESSFUL;
            dh_if->command = CMD_ABORT;
            idle = TRUE;
            break;

            /**
             * CMD_RUN allows the host to issue an 'execute' command to the
             * DSP, so that it will 'run' the selected tests.
             */

        case CMD_RUN:
            dh_if->result = RESULT_RUNNING;
            dh_if->ack = ACK_OK;

            if (idle) {
                dh_if->errorcount = 0;
                dh_if->testcounter = 0;
                dh_if->location = 0;
                dh_if->expected = 0;
                dh_if->actual = 0;
                dh_if->extra = 0;
            }
            idle = FALSE;

            dh_if->testcounter++;
bsp_debug_printf(CORE_DSS0, "\n *******%s> before runtest", core_str);
            nerrors = runtest();

            if ((nerrors != 0) && (dh_if->flags & FLAG_STOP_ON_ERROR)) {
                dh_if->flags &= ~FLAG_CONT_RUN;
            }
            if ((dh_if->flags & FLAG_CONT_RUN) != FLAG_CONT_RUN) {
                if (dh_if->errorcount == 0) {
bsp_debug_printf(CORE_DSS0, "\n *******%s> RESULT_SUCCESSFUL ", core_str);
                    dh_if->result = RESULT_SUCCESSFUL;
                } else {
bsp_debug_printf(CORE_DSS0, "\n *******%s> RESULT_RESULT_FAILED ", core_str);
                    dh_if->result = RESULT_FAILED;
                }
                dh_if->command = CMD_ABORT;
                idle = TRUE;
            }
            break;
        default:
            dh_if->ack = ACK_ERROR;
            dh_if->result = RESULT_ABORTED;
            dh_if->command = CMD_ABORT;
            idle = TRUE;
        }/* switch */
    }/* for */
//#endif
}

/*
 **********************************************************************
 *
 *  Function: runtest
 *
 *  Description: run DSP diagnostic test cases
 *  All tests return 0 if successful.  If a test fails the return value is
 *  the number of errors detected by the given test case. 
 *  If the specified test doesn't exist, it returns FAILED.
 *
 **********************************************************************
 */
static ulong runtest()
{
    uint lnerrors;
    uint test_select, i, ran_cmd;

    lnerrors = 0;
    ran_cmd  = FALSE;

    for (i=0; i<MAX_NUM_TEST; i++){
        test_select = montalvo_test_table[i].test_select;
        if (dh_if->select == test_select){
            ran_cmd = TRUE;
            lnerrors = (*montalvo_test_table[i].diag)();
            if (lnerrors != 0) {
                dh_if->faults |= test_select;
                dh_if->errorcount += lnerrors;
                while (((dh_if->flags & FLAG_LOOP_ON_ERROR) == FLAG_LOOP_ON_ERROR) &&
                       (dh_if->command != CMD_ABORT)) {
                    lnerrors = (*montalvo_test_table[i].diag)();
                    dh_if->testcounter++;
                    dh_if->errorcount += lnerrors;
                }
                if (stop_on_error()) {
                    return lnerrors;
                }
            }
            return lnerrors;
        }  /* end if select */
    }

    if (ran_cmd == FALSE) {
        dh_if->errorcount++;
        return(FAILED);
    }

    return(lnerrors);
}

/***********************************************************************
 *
 * Function: stop_on_error
 *
 * This program stops the running DSP test if there is an error 
 *
 * Input : none
 *
 * Returns: 
 *
 **********************************************************************
 */
static uint stop_on_error()
{
    uint status;

    status = dh_if->flags & FLAG_STOP_ON_ERROR;
    if (dh_if->command == CMD_ABORT) {
        status = 1;
    }
    return(status);
}


/* 
 * $Log: diag_main.c,v $
 * Revision 1.2  2017/07/28 07:58:36  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:30  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.3  2012/12/24 00:07:08  srane
 * Support NGVM interface SYNC signal test.
 *
 * Revision 1.2  2012/05/10 22:45:14  srane
 * Add DSS sanity test for all cores.
 *
 * Revision 1.1  2012/04/18 18:08:36  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
