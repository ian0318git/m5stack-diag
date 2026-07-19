/* $Id: skye_gpio.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_gpio.c,v $
 *------------------------------------------------------------------
 *
 * skye_gpio.c: Skye GPIO related functions. 
 *
 * Oct 2013 - Iachang porting the code
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <gxio/common.h>
#include <gxio/gpio.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "types.h" 
#include "queryflags.h" 
#include "nvmonvars.h"


/*******************************************************************************
 *                           Function Prototypes
 *******************************************************************************
 */
boolean    is_cpu0(void);
boolean    cpu_gpio_pin_state(int);

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int check_cpu1_wdt(void);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
static  gxio_gpio_context_t gc;
boolean cpu_id;

/**********************************************************************
 *
 * Function: gpio_init
 *
 * Description: Initialize a GPIO context
 *
 * Input : NONE
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
gpio_init (void)
{
    int status;
    uint64_t o_pins = O_PINS; /* output */
    uint64_t p_pins = P_PINS; /* all */
    uint64_t val;
    printf("\nDBG : %s %d:\n", __FUNCTION__, __LINE__); 
    /*
     * Open a GPIO context, attach to the pins we want to use, and correctly
     * set pin directions.
     */
    status = gxio_gpio_init(&gc, 0);
    if (status != 0) {
        cterr('f', 0, "%s GPIO Initial FAILED",__FUNCTION__);
        return (FAILED);
    }

    status = gxio_gpio_attach(&gc, p_pins);
    if (status != 0) {
        cterr('f', 0, "%s GPIO Attach FAILED",__FUNCTION__);
        return (FAILED);
    }

    status = gxio_gpio_set_dir(&gc, 0, p_pins & ~o_pins, p_pins & o_pins, 0);
    if (status != 0) {
        cterr('f', 0, "%s GPIO Setup input/output FAILED",__FUNCTION__);
        return (FAILED);
    }

    val = gxio_gpio_get(&gc);
    printf("GPIO  values 0x%016lx\n", val);   

    /* Get the CPU ID */
    is_cpu0();
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: is_cpu0
 *
 * Description: Function to determine if this is main CPU by check CPU ID,
 *              and set global variable, cpu_id, with corresponding CPU ID.
 *
 * Input : NONE
 *                     
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean
is_cpu0 (void)
{
    uint64_t val = 0;

    val = gxio_gpio_get(&gc);
    if (DIAGFLAG & D_VERBOSE) {
	    printf("GPIO  values 0x%016lx", val);   
    }
    if ((val & CPU_CPU_ID_REGISTER) == CPU_CPU_ID_REGISTER) {
        if (DIAGFLAG & D_VERBOSE) {
            printf("\nI am CPU #1\n");   
        }
        cpu_id = 1;
        return (FALSE);
    }
    if (DIAGFLAG & D_VERBOSE) {
	    printf("\nI am CPU #0\n");   
    }
    cpu_id = 0;
    return (TRUE);
}

/**********************************************************************
 *
 * Function: check_cpu
 *
 * Description: Check CPU number. 
 *
 * Input : cpu_no : 0=Master ; 1=Slave
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
boolean
check_cpu (int cpu_no)
{
    uint64_t val = 0;
    boolean  cpu_check;
    val = gxio_gpio_get(&gc);
    if ((val & CPU_CPU_ID_REGISTER) == CPU_CPU_ID_REGISTER) {
        cpu_check = 1;
    } else {
        cpu_check = 0;
    }
    if (cpu_check == cpu_no) {
        return (TRUE);
    }
    return (FALSE);
}

/**********************************************************************
 *
 * Function: get_gxio_gpio
 *
 * Description: Get GPIO value
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
uint64_t
get_gxio_gpio (void)
{
    uint64_t val = 0;

    val = gxio_gpio_get(&gc);
    if (DIAGFLAG & D_VERBOSE) {
	    printf("GPIO  values 0x%016lx", val);   
    }

    return (val);
}

/**********************************************************************
 *
 * Function: wdt_act
 *
 * Description: Activity signal to WDT . 
 *
 * Input : time : The gpio signal durning time
 *                     
 * Output: None
 *
 **********************************************************************
 */
void
wdt_act (int time_out)
{
    uint64_t val = 0;
    boolean  gpio_val = 0;
    val = gxio_gpio_get(&gc);
    time_out = time_out * 2;
    while (time_out) {
        if (gpio_val == 0) {
            val |= CPU_ACT_WDT_REGISTER;
            gpio_val = 1;
        } else {
            val &= ~(CPU_ACT_WDT_REGISTER);
            gpio_val = 0;
        }    
        gxio_gpio_set(&gc, val, CPU_ACT_WDT_REGISTER);
        if (DIAGFLAG & D_VERBOSE) {
	        printf("\nGPIO values 0x%016lx", val);   
        }
        check_cpu1_wdt();
        msleep(500);
        time_out--;
    }
}

/*******************************************************************************
 *
 * Function   : cpu_gpio_pin_state
 * Description: Get CPU GPIO pin state.
 * Inputs     : gpio_num - GPIO bit number that want to check
 * Outputs    : ENABLE(1)/DISABLE(0)
 *
 *******************************************************************************
 */
boolean
cpu_gpio_pin_state (int gpio_num)
{
    uint64_t val = 0, gpio_mask = 0;

    val = get_gxio_gpio();
    if (DIAGFLAG & D_VERBOSE) {
        printf("CPU GPIOs = 0x%016lx\n", val);
    }

    gpio_mask = (uint64_t)(1 << gpio_num);

    if (val & gpio_mask) {
        return (ENABLE);
    }

    return (DISABLE);
}

/*******************************************************************************
 *
 * Function   : cpu_gpio_pin_state_wrap
 * Description: Wrap function to check CPU GPIO pin state.
 * Inputs     : NONE
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int
cpu_gpio_pin_state_wrap (void)
{
    int      gpio_num = 13, ret_val = 0;

    gpio_num = getdec_answer("Enter GPIO number you want to read", 13, 0, 63);

    ret_val = cpu_gpio_pin_state(gpio_num);

    printf("CPU GPIO(%02d) = %d\n", gpio_num, ret_val);

    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: skye_gpio.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/05/11 13:45:46  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:36:35  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.3  2014/09/29 18:36:22  palin2
 * Added utility to get CPU GPIO pin state.
 *
 * Revision 1.1.2.2  2014/08/13 11:51:49  steja
 * Add 10GKR determine function
 *
 * Revision 1.1.2.1  2014/07/21 01:56:55  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * skye_gpio.c:
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.9  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.2.8  2014/01/28 13:49:45  steja
 * Update GE Frontpanel test code
 *
 * Revision 1.1.2.7  2014/01/08 03:42:12  iachang
 * Moved CPU number item to basic utilities
 *
 * Revision 1.1.2.6  2013/12/25 08:54:59  iachang
 * Check CPU1 WDT timeout reg. during toggle GPIO pin
 *
 * Revision 1.1.2.5  2013/12/10 15:26:22  iachang
 * WDT Activity GPIO signal to CPU0
 * Support CPU0/1 Szalinski WDT Test
 * Szalinski FPGA Flash Image verify
 *
 * Revision 1.1.2.4  2013/11/18 03:28:13  iachang
 * Support CPU0 Szalinski watchdog test.
 *
 * Revision 1.1.2.3  2013/11/07 06:46:39  iachang
 * Support Szalinski interrupt test.
 *
 * Revision 1.1.2.2  2013/11/06 09:08:26  iachang
 * Provide CPU check function to display/hide menu
 *
 * Revision 1.1.2.1  2013/10/30 10:49:20  iachang
 * Modify CPU GPIO initial
 *
 *------------------------------------------------------------------
 * $Endlog$
 */



