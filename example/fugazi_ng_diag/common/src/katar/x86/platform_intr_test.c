/* $Id: platform_intr_test.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_intr_test.c,v $
 *------------------------------------------------------------------
 * 
 * Filename   : katar_platform_intr_test.c
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
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
#include <semaphore.h>
#include <time.h>
#include "common.h"
#include "proto.h"
#include "queryflags.h"
#include "platform_fpga.h"
#include "uio_utils.h"
#include "error.h"
#include "menu.h"
#include "platform_poe.h"
#include "platform_temp_sensor.h"

struct intr_t {
 	 void (*enable)(int);
	 void (*raise)(int);
	 void (*clr)(int);
	 void (*disable)(int);
	 void (*hndlr)(int, void*);
	 void *dev;
	 unsigned int irq;
	 unsigned int flag;
	 char name[30];
};

static  pthread_t threads;
static int ovrd = 0;
static int force_irq = 0;
static int intr_mb_test = 0;
static uint8 bInitThread = 0;
static uint8 bPauseintrCheck = 0;

/* for ctrl diag flow */
sem_t force_irq_finish;

static struct intr_t intr[INTR_MAX+2];

extern int dump_AQC_SFP_eeprom_data(int port_num);
extern void katar_AQR_set_force_intr(uint enable);
extern int katar_is_sfp_sku(void);

void platform_init_intr(void);
void platform_destory_intr(void);
int platform_intr_test(int dummy);
int platform_intr_mask(int dummy);
int platform_intr_manual_tests(int test_item);
int katar_mb_force_intr_test (int dummy);
void register_irq_all(void);
int katar_tirgger_intr(uint intr_typ,uint bTrigger);

extern unsigned long diagflag_xram;

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
    time_t clk = time(NULL);
	char cmd[1024];
	int irq_num = *(int *)arg;

	// Don't print message during mb test
	if(intr_mb_test!=TRUE)
	{
	    printf("Call %s time:%sirq:%d(%s) fpga IRQ %d;\nclr intr..", __FUNCTION__, ctime(&clk), 
			irq, intr[irq].name, irq_num);

		if(katar_clear_interupt(irq)==FAILED)
			printf("FAILED\n");
		else
			printf("PASSED\n");

		if(irq==INTR_RESET_BTN)
		{
			int btn_stat, btn_dur;
			if(katar_get_rst_btn_info(&btn_stat,&btn_dur,TRUE)==FAILED)
			printf("Reset RST_BTN_DUR failed\n");

			printf("Reset Momentary Press Push-Button now ");
			if(btn_stat)
				printf("asserted ");
			else
				printf("de-asserted ");
	
			if(btn_dur)
				printf("with long press\n");
			else
				printf("with short press\n");
		}

		if(irq_num != FPGA_IRQ_ALL)
		{
			sprintf(cmd,"cat /proc/interrupts |grep %d:|head -n 1| cut -c -15", irq_num);
			printf("IRQ");
			fflush(stdout);
			system(cmd);
			printf("\n");
		}
	}else
	{
		prpass(testpass, "%s ",intr[irq].name);
		katar_clear_interupt(irq);
	}
	return;
}

/*-------------------------------------------------------------------
 *
 * Function: normal_intr_hndlr
 * Description: normal intr handler
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
static void normal_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);
	char cmd[1024];
	int	irq_num = *(int *)arg;

    printf("Got irq:%d(%s) at %sclr intr..", irq, intr[irq].name, ctime(&clk));
	if(katar_clear_interupt(irq)==FAILED)
		printf("FAILED\n");
	else
		printf("PASSED\n");

	if(irq_num != FPGA_IRQ_ALL)
	{
		sprintf(cmd,"cat /proc/interrupts |grep %d:|head -n 1| cut -c -15", irq_num);
	    printf("IRQ");
	    fflush(stdout);
	    system(cmd);
		printf("\n");
	}

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function: silent_intr_hndlr
 * Description: silent intr handler
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
static void silent_intr_hndlr (int irq, void *arg) {
	//Only clear interrupt bit
    katar_clear_interupt(irq);
    return;
}

/*-------------------------------------------------------------------
 *
 * Function: reset_button_intr_hndlr
 * Description: intr handler for reset button
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
static void reset_button_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);
	int btn_stat, btn_dur;
	char cmd[1024];
	int irq_num = *(int *)arg;

    printf("Call %s time:%sirq:%d clr intr..", __FUNCTION__, ctime(&clk), irq);
	if(katar_clear_interupt(irq)==FAILED)
		printf("FAILED\n");
	else
		printf("PASSED\n");

	if(katar_get_rst_btn_info(&btn_stat,&btn_dur,TRUE)==FAILED)
		printf("Reset RST_BTN_DUR failed\n");

	printf("Reset Momentary Press Push-Button now ");
	if(btn_stat)
		printf("asserted ");
	else
		printf("de-asserted ");
	
	if(btn_dur)
		printf("with long press\n");
	else
		printf("with short press\n");

    if(irq_num != FPGA_IRQ_ALL)
    {
        sprintf(cmd,"cat /proc/interrupts |grep %d:|head -n 1| cut -c -15", irq_num);
        printf("IRQ");
        fflush(stdout);
        system(cmd);
        printf("\n");
    }

    return; 
}

static void usb_com_intr_hndlr (int irq, void *arg) {

	time_t clk = time(NULL);
	char cmd[1024];
    int irq_num = *(int *)arg;

	printf("Call %s time:%sirq:%d clr intr..", __FUNCTION__, ctime(&clk), irq);
	
	if(katar_clear_interupt(irq)==FAILED)
        printf("FAILED\n");
    else
        printf("PASSED\n");

	if(katar_get_usb_com_stat())
		katar_set_usb_com_control(TRUE);
	else
		katar_set_usb_com_control(FALSE);

    if(irq_num != FPGA_IRQ_ALL)
    {
        sprintf(cmd,"cat /proc/interrupts |grep %d:|head -n 1| cut -c -15", irq_num);
        printf("IRQ");
        fflush(stdout);
        system(cmd);
        printf("\n");
    }

	return;
}

static void register_irq(int irq)
{
	switch(irq)
	{
		case INTR_USB_COM:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)usb_com_intr_hndlr;
            sprintf(intr[irq].name, "Usb com");
            break;
		case INTR_DIMM_OVERHEAT:
			intr[irq].irq = irq;
			intr[irq].hndlr = (void *)normal_intr_hndlr;
			sprintf(intr[irq].name, "DIMM overheat");			
			break;
		case INTR_RESET_BTN:
			intr[irq].irq = irq;
			intr[irq].hndlr = (void *)reset_button_intr_hndlr;
			sprintf(intr[irq].name, "Reset button");				
			break;
		case INTR_FAN_TACH_LOW:
			intr[irq].irq = irq;
			intr[irq].hndlr = (void *)normal_intr_hndlr;
			sprintf(intr[irq].name, "Fan TACH low");					
			break;
		case INTR_POE:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "POE");
            break;
		case INTR_GE_SW:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "GE Switch");
            break;
		case INTR_SFP_P1_PRESENT:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "SFP Port 1 Presence Change");
            break;
        case INTR_SFP_P0_PRESENT:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "SFP Port 0 Presence Change");
            break;
		case INTR_CCCP_READY:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "CC CP change");
            break;
        case INTR_FPCP_READY:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "FP CP change");
            break;
        case INTR_PKT_READY:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "Packet ready change");
            break;
		case INTR_ILL_ACC:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)normal_intr_hndlr;
            sprintf(intr[irq].name, "Illegal access");
            break;
		//FPGA won't connect SFP_LOS SFP_FAULT after P1B
		case INTR_SFP_P1_LOS:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)silent_intr_hndlr;
            sprintf(intr[irq].name, "SFP Port 1 Loss of Signal");
            break;
		case INTR_SFP_P1_FAULT:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)silent_intr_hndlr;
            sprintf(intr[irq].name, "SFP Port 1 Transmitter Fault");
            break;
        case INTR_SFP_P0_LOS:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)silent_intr_hndlr;
            sprintf(intr[irq].name, "SFP Port 0 Loss of Signal");
            break;
        case INTR_SFP_P0_FAULT:
            intr[irq].irq = irq;
            intr[irq].hndlr = (void *)silent_intr_hndlr;
            sprintf(intr[irq].name, "SFP Port 0 Transmitter Fault");
            break;
		default:
			break;
	}
	return;
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
    int irq;
	uint8 bRemote = FALSE , bManual = FALSE ,bSkipWait = FALSE;
	struct timespec ts;
	int rc = FAILED;

	platform_init_intr();

    ovrd = 1;
	rc = sem_init(&force_irq_finish, 0, 0 );
    if (rc != PASSED) {
        printf("sem_init on force_irq_finish failed.\n");
        goto done;
    }
	
    while (1) {
		
		force_irq = INTR_MAX;
		for(irq=0;irq<INTR_MAX;irq++)
        {
            printf("%d) %s interrupt\n", irq, intr[irq].name);
        }
        irq = getdec_answer("Select interrupt, other value to quit: ", INTR_MAX, 0, 0xFF);
        if(irq>= INTR_MAX)
        {
			printf("exit from force interrupt test menu\n");
			goto done;
        }else if(katar_check_interupt_mask(irq))
		{
			
			printf("%s intr mask off is set ON\n", intr[irq].name);
			if (getc_answer("\nDo you want to trun it off? (y/n)", "yn", 'y')== 'y')
			{
				bPauseintrCheck = TRUE;
	            msleep(1);
				clear_fpga_status();
				katar_interupt_mask_control(irq,FALSE);
				msleep(1);
				bPauseintrCheck = FALSE;
			}else
	            goto done;
		}

		bRemote = FALSE;
		bManual = FALSE;
		switch(irq)
		{
			case INTR_FAN_TACH_LOW:
			case INTR_POE:
			case INTR_GE_SW:
				if (getc_answer("\nTrigger by force trigger reg? (y/n)", "yn", 'y')== 'y')
					bRemote = FALSE;
				else
					bRemote = TRUE;
				break;
			case INTR_RESET_BTN:
			case INTR_SFP_P1_PRESENT:
			case INTR_SFP_P0_PRESENT:
				if (getc_answer("\nManual trigger interrupt? (y/n)", "yn", 'n')== 'y')
					bManual = TRUE;
				else
					bManual = FALSE;
				break;
		}
        force_irq = irq;
		bSkipWait = FALSE;

		if(bManual)
		{
			switch(irq)
	        {
    	        case INTR_RESET_BTN:
        	        printf("\nPress reset button for more than 10 sec\n");
       		        break;
               	case INTR_SFP_P1_PRESENT:
                   	printf("\nPlug/unplug SFP module to port 1\n");
                    break;
    	        case INTR_SFP_P0_PRESENT:
       	            printf("\nPlug/unplug SFP module to port 0\n");
           	        break;
               	default:
	                printf("wrong parameter.\n");
     	            goto done;
       	            break;
           	}
		}else if(bRemote)
			katar_tirgger_intr(irq,TRUE);	
		else if(katar_force_interupt(irq)!=PASSED)
			bSkipWait = TRUE;

		if(bSkipWait == FALSE)
		{
			msleep(1);

			rc = clock_gettime(CLOCK_REALTIME, &ts);
        	if (rc != PASSED) {
            	printf("clock gettime failed..\n");
	            goto done;
    	    }
			ts.tv_sec += 20;
			/* wait for the setting of rx side */
	        rc = sem_timedwait(&force_irq_finish, &ts);

			if(bRemote)
                katar_tirgger_intr(irq,FALSE);

			if (rc != PASSED)
			{
				printf("wait for force_irq_finish failed..\n");
				goto done;
			}
			sleep(1);
			printf("\n%s interrupt mask is %s\n",intr[irq].name,katar_check_interupt_mask(irq)?"ON":"OFF");
		}else
			printf("Don't have force interrupt reg\n");
    }
 done:
    ovrd = 0;
	force_irq = INTR_MAX;
    sem_destroy(&force_irq_finish);
	platform_destory_intr();
    return PASSED;
}


/*-------------------------------------------------------------------
 *
 * Function:  platform_intr_mask
 *
 * set interrupt mask
 *
 * INPUT : dummy not used
 * OUTPUT: always return PASSED. this is utility.
 *------------------------------------------------------------------
 */
int
platform_intr_mask (int dummy)
{
    int irq;
	int bSet;

	register_irq_all();

    while (1) {
		for(irq=0;irq<INTR_MAX;irq++)
		{
			bSet = katar_check_interupt_mask(irq);
			printf("%d) %s intr mask off is set %s\n", irq, intr[irq].name,bSet?"ON":"OFF" );
		}
		irq = getdec_answer("Select mask to change, other value to quit: ", INTR_MAX, 0, 0xFF);
		if(irq>= INTR_MAX)
		{
			clear_fpga_status();
			printf("exit from set interrupt mask menu\n");
			return PASSED;
		}
		bSet = katar_check_interupt_mask(irq);
		clear_fpga_status();
        if(bSet)
            katar_interupt_mask_control(irq,FALSE);
        else
            katar_interupt_mask_control(irq,TRUE);
    }
    return PASSED;
}

int platform_intr_manual_tests (int test_item)
{
	struct timespec ts;
	int rc = FAILED;
	int bSFPpresent = -1;
	int sfp_port_num = 0;
	char *tname[] = {"reset button", "SFP p1 present", "SFP p0 present"};

	switch(test_item)
	{
		case 0:
			force_irq = INTR_RESET_BTN;
			break;
		case 1:
			force_irq = INTR_SFP_P1_PRESENT;
			sfp_port_num = 1;
			if(!katar_is_sfp_sku())
			{
				printf("------ Not SFP sku ------\n");	
				return rc;
			}
			break;
		case 2:
			force_irq = INTR_SFP_P0_PRESENT;
			sfp_port_num = 0;
            if(!katar_is_sfp_sku())
            {
                printf("------ Not SFP sku ------\n");
                return rc;
            }
			break;
		default:
			return rc;
			break;
	}

    testname("interrupt");
	printf("Start %s interrupt test\n",tname[test_item]);

    platform_init_intr();
    ovrd = 1;

	if ((diagflag_xram & D_DEBUG_OPTIONS)==0)
		intr_mb_test = 1;

    rc = sem_init(&force_irq_finish, 0, 0 );
    if (rc != PASSED) {
        cterr('f',0,"sem_init on force_irq_finish failed.");
        goto done;
    }

	if(katar_check_interupt_mask(force_irq))
	{
		bPauseintrCheck = TRUE;
		msleep(1);
        clear_fpga_status();
        katar_interupt_mask_control(force_irq,FALSE);
        msleep(1);
        bPauseintrCheck = FALSE;
    }

    rc = clock_gettime(CLOCK_REALTIME, &ts);
    if (rc != PASSED) {
        cterr('f',0,"clock gettime failed.");
        goto done;
    }
    ts.tv_sec += 30;

	switch(force_irq)
	{
		case INTR_RESET_BTN:
			printf("\nPress reset button for more than 10 sec\n");
			break;
		case INTR_SFP_P0_PRESENT:
		case INTR_SFP_P1_PRESENT:
			bSFPpresent = katar_is_sfp_present(sfp_port_num);
			if(bSFPpresent)
			{
				rc = dump_AQC_SFP_eeprom_data(sfp_port_num);
				if (rc != PASSED) {
     				cterr('f',0,"Get port %d eeprom data failed.",sfp_port_num);
			        goto done;
			    }
				printf("\nUnplug SFP module in port %d\n",sfp_port_num);
			}
			else
				printf("\nPlug SFP module to port %d\n",sfp_port_num);
			break;
		default:
            cterr('f',0,"wrong parameter.");
			rc = FAILED;
            goto done;
            break;
	}

	rc = sem_timedwait(&force_irq_finish, &ts);
	//wait 500 ms if got IRQ
	if(rc == PASSED)
		msleep(500);

	printf("\n");
	switch(force_irq)
    {
		case INTR_SFP_P0_PRESENT:
        case INTR_SFP_P1_PRESENT:
            if(bSFPpresent == katar_is_sfp_present(sfp_port_num))
			{
				cterr('f',0,"SFP port %d present signal check fail.",sfp_port_num);
				rc = FAILED;
				goto done;
			}else if(katar_is_sfp_present(sfp_port_num))
			{
				rc = dump_AQC_SFP_eeprom_data(sfp_port_num);
                if (rc != PASSED) {
					cterr('f',0,"Get port %d eeprom data failed.",sfp_port_num);
                    goto done;
                }
			}
            break;
        default:
            break;
    }
	if (rc != PASSED)
		cterr('f',0,"wait for %s interrupt fail.",tname[test_item]);

	//manual intr test move to utility
	prcomplete(testpass, errcount, "%s interrupt test passed\n",tname[test_item]);
done:
    ovrd = 0;
    sem_destroy(&force_irq_finish);
	platform_destory_intr();
	intr_mb_test = 0;

	return rc;
}

int katar_tirgger_intr(uint intr_typ,uint bTrigger)
{
	int rc = FAILED;

	switch(intr_typ)
	{
		case INTR_FAN_TACH_LOW:
			bPauseintrCheck = TRUE;
			msleep(1);
			clear_fpga_status();
			if(bTrigger)
				katar_set_fan_threshold(10000);
			else
			{
				katar_set_fan_threshold(0);
				sleep(1);
			}
			msleep(1);
			bPauseintrCheck = FALSE;
		break;
#ifdef ENABLE_POE_MODULE
		case INTR_POE:
			if(bTrigger)
				rc = katar_poe_set_force_intr();
			else
				katar_poe_clear_force_intr();
		break;
#endif
		case INTR_GE_SW:
			katar_AQR_set_force_intr(bTrigger);
		break;

		default:
			if(bTrigger)
				rc = katar_force_interupt(intr_typ);
		break;
	}
	return rc;
}

int katar_skip_mb_force_intr_test(uint intr_typ)
{
	int rc = FALSE;
	switch(intr_typ)
    {
		//Can't force trigger intr
		case INTR_DIMM_OVERHEAT:
			rc = TRUE;
			break;
		
		//Tested with manual intr test
		case INTR_RESET_BTN:
		case INTR_SFP_P1_PRESENT:
		case INTR_SFP_P0_PRESENT:
			rc = TRUE;
            break;

		//These signal won't connect to FPGA after P1B
		case INTR_SFP_P1_LOS:
		case INTR_SFP_P0_LOS:
		case INTR_SFP_P1_FAULT:
		case INTR_SFP_P0_FAULT:
			rc = TRUE;
            break;
	}
	return rc;
}

int
katar_mb_force_intr_test (int dummy)
{
    int irq,i;
    struct timespec ts;
    int rc = FAILED;
	int bKernelIRQCheck = FALSE;
	char *tname = "Auto interupt";
    
	testname("%s", tname);

    platform_init_intr();
    ovrd = 1;
	if ((diagflag_xram & D_DEBUG_OPTIONS)==0)
	    intr_mb_test = 1;
	
	rc = sem_init(&force_irq_finish, 0, 0 );
    if (rc != PASSED) 
	{
		cterr('f', 0, "sem_init on force_irq_finish failed.");
        goto done;
    }

    for(irq=0;irq<INTR_MAX;irq++)
    {
        force_irq = irq;
		if(katar_skip_mb_force_intr_test(irq)==FALSE)
		{
			if(katar_check_interupt_mask(irq))
			{
				bPauseintrCheck = TRUE;
				msleep(1);
				clear_fpga_status();
				katar_interupt_mask_control(irq,FALSE);
				msleep(1);
				bPauseintrCheck = FALSE;
			}
			katar_tirgger_intr(irq,TRUE);
        	msleep(5);

	        rc = clock_gettime(CLOCK_REALTIME, &ts);
    	    if (rc != PASSED) {
				cterr('f', 0, "clock gettime failed.");
				katar_tirgger_intr(irq,FALSE);
        	    goto done;
    	    }
	        ts.tv_sec += 20;
    	    /* wait for the setting of rx side */
	        rc = sem_timedwait(&force_irq_finish, &ts);

			msleep(5);

			katar_tirgger_intr(irq,FALSE);
	        if (rc != PASSED)
    	    {
	            printf("wait for force_irq_finish failed..\n");
				cterr('f', 0, "wait for %s force_irq_finish failed.",intr[irq].name);
        	    goto done;
    	    }

			//Kernel will set mask on when get IRQ			
			if(katar_check_interupt_mask(irq)==1)
				bKernelIRQCheck = TRUE;

			msleep(100);
		}
    }

	for (i=TS_BEZEL_SIDE0; i<=TS_BEZEL_SIDE1; i++) {
		prpass(testpass, "#%d temperature sensor alert test", i);
        rc = temperature_sensor_alert_test(i);
        if (rc != PASSED) {
            cterr('f', 0, "Test %d temperature sensor alert failed.",i);
			goto done;
        }
    }

	if(bKernelIRQCheck==FALSE)
	{
		cterr('f', 0, "Kernel IRQ check fail,please make sure you use latest kernel.");
	}
	prpass(testpass, NULL);
	rc = PASSED;
done:
    ovrd = 0;
    intr_mb_test = 0;
	sem_destroy(&force_irq_finish);
	platform_destory_intr();
	sleep(1);
	return rc;
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
    unsigned int fpga_irq = FPGA_IRQ_NONE;
	int irq;
	int forceirqcheck = FALSE;
    
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (bInitThread) {

		while((bPauseintrCheck) || (katar_check_interupt(INTR_ALL,FALSE) == FPGA_IRQ_NONE))
			 msleep(1); /* Wait 5ms */

		for(irq=0;irq<INTR_MAX;irq++)
		{
			fpga_irq = katar_check_interupt(irq,FALSE);
			if(fpga_irq != FPGA_IRQ_NONE)
			{
				//INTR_CCCP_READY && INTR_FPCP_READY share same bit
				if(force_irq == irq)
					forceirqcheck = TRUE;
				else if((force_irq==INTR_FPCP_READY)&&(irq==INTR_CCCP_READY))
					forceirqcheck = TRUE;

				if(ovrd && forceirqcheck)
				{
					force_intr_hndlr(irq,(void *)&fpga_irq);
					if (sem_post(&force_irq_finish)){
			            if (errno == EINVAL){
			                printf("The sem(force_irq_ready) does not refer to a valid semaphore \n");
			            } else {
			                printf("The function sem_post() is not supported by this implementation\n");              
			            }
			        }					
				}else if(intr[irq].hndlr)
				{
					if(intr[irq].dev == NULL)
						intr[irq].hndlr(irq, &fpga_irq);
					else
						intr[irq].hndlr(irq, (void *)intr[irq].dev);
				}else
					printf("Got %d irq without hndlr ; fpga_irq:%d\n", irq, fpga_irq);
			}
		}
    }
    return (void*)NULL;
}

void register_irq_all(void)
{
	static int bRegistered = FALSE;
	int irq;

	if(bRegistered)
		return;

	memset(intr, 0, sizeof(intr));

	for(irq=0;irq<INTR_MAX;irq++)
        register_irq(irq);

	bRegistered = TRUE;
	return;
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
	if(bInitThread == 1)
		return;
	//clear interrupt before test
	katar_clear_interupt(INTR_ALL);

	register_irq_all();
    
	bInitThread = 1;
    /* create thread to block waiting for interrupt */
    if(pthread_create(&threads, NULL, rx_intr, (void *)NULL)) {
        printf("pthread_create failed \n");
        exit(-1);
        return;
    }
	return;
}

void
platform_destory_intr (void)
{
	pthread_cancel(threads);
	
	bInitThread = 0;

	clear_fpga_status();
	return;
}

/*
 *------------------------------------------------------------------
 * $Log: platform_intr_test.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.8  2019/03/08 07:44:37  mikech2
 * Add SFP sku check for SFP utility
 *
 * Revision 1.1.2.7  2019/03/07 02:51:08  mikech2
 * Move reset button/SFP present test to utility
 *
 * Revision 1.1.2.6  2019/03/06 01:56:23  mikech2
 * Add dump SFP eeprom in SFP intr test
 *
 * Revision 1.1.2.5  2019/03/04 07:39:16  mikech2
 * Add temperature sensor alert test in Interrupt auto test
 *
 * Revision 1.1.2.4  2019/02/20 02:54:48  mikech2
 * Add SFP present test in SPF intr test
 *
 * Revision 1.1.2.3  2019/02/12 08:06:30  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/01/29 08:02:05  mikech2
 * remove POE test for katar P2 build
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.11  2019/01/21 07:29:20  mikech2
 * Add check kernel IRQ in mb_test
 *
 * Revision 1.1.2.10  2019/01/18 03:42:28  mikech2
 * Set mask off before interrupt test
 *
 * Revision 1.1.2.9  2018/12/27 03:49:00  mikech2
 * Modify prpass usage
 *
 * Revision 1.1.2.8  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.7  2018/11/16 07:55:47  mikech2
 * Add delay to avoid send interrupt twice
 *
 * Revision 1.1.2.6  2018/11/09 15:08:20  benlu
 * add AQR PHY interrupt auto test
 *
 * Revision 1.1.2.5  2018/11/09 07:12:04  mikech2
 * Fix auto interrupt test reset issue
 *
 * Revision 1.1.2.4  2018/11/08 06:00:15  mikech2
 * Add fan low and interrupt test in mb test and remove intr utility
 *
 * Revision 1.1.2.3  2018/11/05 07:37:59  mikech2
 * Add interrupt utility
 *
 * Revision 1.1.2.2  2018/10/26 02:39:34  mikech2
 * Fix typo
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.5  2018/10/11 06:49:04  mikech2
 * Update FPGA intr test for SERIRQ
 *
 * Revision 1.1.2.4  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.3  2018/09/07 02:16:52  mikech2
 * Fix FPGA util issue
 *
 * Revision 1.1.2.2  2018/06/28 03:32:56  mikech2
 * Add interrupt mask control menu
 *
 * Revision 1.1.2.1  2018/06/25 08:24:53  mikech2
 * Add interupt test menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
