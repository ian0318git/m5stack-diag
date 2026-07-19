/* $Id: patriot_framer_main.c,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_framer_main.c
 *
 * Description: FRAMER init main module
 *
 *
 * Author: Sofian Teja, port from IOS
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/*****************************************/
/* header files */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include "../apps/ds3170.h"
#include "patriot_framer_spi.h"
#include "klm_defs.h"

/*****************************************/
/* defines */
#define DS3170_DEVICE_ID   0x004F


/*****************************************/
/* data structures */

/*****************************************/
/* static/global variables */

int patriot_framer_debug = 0;

/******************************************************************
 * Name : patriot_framer_debug_onoff
 * Description : Patriot Framer Debug Flag
 * Input       : debug_onoff
 *                 - 1 (turn on)
 *                 - 0 (turn off)
 * Output      : return 0
 ******************************************************************
 */
int patriot_framer_debug_onoff(unsigned int debug_onoff)
{
    if (patriot_framer_debug) {
        printk(KERN_ALERT "Patriot Framer: %s: %s %d\n",
            __FUNCTION__,
            "debug option:",
            debug_onoff);
    }
    if (debug_onoff) {
        patriot_framer_debug = 1;
        patriot_framer_spi_debug = 1;
    } else {
        patriot_framer_debug = 0;
        patriot_framer_spi_debug = 0;
    }
    return 0;
}
EXPORT_SYMBOL_GPL(patriot_framer_debug_onoff);

/******************************************************************
 * Name : patriot_framer_id
 * Description : Read and verify the Framer id
 * Input       : None
 * Output      : 0/1/-1
 ******************************************************************
 */
int patriot_framer_id(void)
{
    int retval = 0;
    unsigned char temp = 0x0;
    unsigned char temp1 = 0x0;
    
    /* Debug on */
    patriot_framer_debug_onoff(1);
    
    /* STEP 1: Check Device ID Code */
    if (patriot_framer_debug) {
        printk(KERN_ALERT "Patriot Framer: %s: Read Framer device id\n",
            __FUNCTION__);
    }
    if ((retval = patriot_framer_read(IDR_ADDR_L, &temp)) < 0) {
        if (patriot_framer_debug) {
            printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
                __FUNCTION__, IDR_ADDR_L);
        }
        return retval;
    }
    if ((retval = patriot_framer_read(IDR_ADDR_H, &temp1)) < 0) {
        if (patriot_framer_debug) {
            printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
                __FUNCTION__, IDR_ADDR_H);
        }
        return retval;
    }
    if ((temp != (DS3170_DEVICE_ID & 0xff)) ||
        (temp1 != ((DS3170_DEVICE_ID & 0xff00) >> 8))) {
        if (patriot_framer_debug) {
            printk(KERN_ALERT "Patriot Framer: %s: %s: 0x%x, 0x%x\n",
                __FUNCTION__, "Incorrect Framer device id",
                temp, temp1);
        }
        return -1;
    }

    return retval;

}

/******************************************************************
 * Name : patriot_framer_interrupt_handler
 * Description : Framer Interrupt Handler check this register framer
 * Input       : None
 * Output      : 0/1
 ******************************************************************
 */
int patriot_framer_interrupt_handler(void)
{
    int retval = 0, temp = 0;
    unsigned char port_isr_l = 0x0;
    unsigned char t3_rsrie_l = 0x0;
    unsigned char t3_rsrl_l = 0x0;
    unsigned char gl_isr_l = 0x0;

    /* check port interrupt active */
    if ((retval = patriot_framer_read(GL_ISR_ADDR_L, &gl_isr_l)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
	       __FUNCTION__, GL_ISR_ADDR_L);
        return retval;
    }
    
    if (gl_isr_l & ISR_PISR) {
        printk(KERN_ALERT "Patriot Framer SPI: %s, ISR_PISR detected\n",
             __FUNCTION__);
        patriot_framer_count();
        temp |= CHECK_ISR_PISR;
    }

    /* check framer block interrupt active */
	if ((retval = patriot_framer_read(PORT_ISR_ADDR_L, &port_isr_l)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
            __FUNCTION__, PORT_ISR_ADDR_L);
        return retval;
	}

	if (port_isr_l & ISR_FMSR) {
	    printk(KERN_ALERT "Patriot Framer SPI: %s, ISR_FMSR detected\n",
		   __FUNCTION__);
	    patriot_framer_count();
	    temp |= CHECK_ISR_FMSR;
	}

	/* Check T3 Interrupt disable */
	if ((retval = patriot_framer_read(T3_RSRIE1_ADDR_L, &t3_rsrie_l)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
            __FUNCTION__, T3_RSRL1_ADDR_L);
        return retval;
	}
    /* Check OOFIE, SEFIE, OOMFIE */

	if (t3_rsrie_l & (T3_RSRIE1_OOFIE | T3_RSRIE1_SEFIE | T3_RSRIE1_OOMFIE)) {
	    printk(KERN_ALERT "Patriot Framer SPI: %s, OOF, SEF, OOMF Interupt enable detected\n",
		   __FUNCTION__);
	    patriot_framer_count();
	    temp |= CHECK_OOFIE_SEFIE_OOMFIE;
	}

	/* Check T3 Latched out of frame */
	if ((retval = patriot_framer_read(T3_RSRL1_ADDR_L, &t3_rsrl_l)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
            __FUNCTION__, T3_RSRL1_ADDR_L);
        return retval;
	}

    /* Check OOFL, SEFL, OOMFL */
	if (t3_rsrl_l & (T3_RSRL1_OOFL | T3_RSRL1_SEFL | T3_RSRL1_OOMFL)) {
	    printk(KERN_ALERT "Patriot Framer SPI: %s, OOFL, SEFL, OOFML detected\n",
		   __FUNCTION__);
	    patriot_framer_count();
	    temp |= CHECK_OOFL_SEFL_OOMFL;
	}
	retval = temp;

	return retval;
}

/******************************************************************
 * Name : patriot_framer_interrupt_disabled
 * Description : Disable Framer Interrupt Handler then check status reg
 * Input       : None
 * Output      : 0/1
 ******************************************************************
 */
int patriot_framer_interrupt_disabled(void)
{
    unsigned char temp = 0x0;
    int retval = 0;

    /* disabled interrupt
     * Global status interrupt &
     * Port Interrupt Status Register interrupt disable */
    if ((retval = patriot_framer_read(ISRIE_ADDR_L, &temp)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
	       __FUNCTION__, ISRIE_ADDR_L);
        return retval;
    }
    temp &= ~ISRIE_GSRIE;
    temp &= ~ISRIE_PISRIE;
    if ((retval = patriot_framer_write(ISRIE_ADDR_L, temp)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to write 0x%x\n",
	       __FUNCTION__, ISRIE_ADDR_L);
        return retval;
    }
    /* OOF, SEF, OOMF interrupt disable */
    if ((retval = patriot_framer_read(T3_RSRIE1_ADDR_L, &temp)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to read 0x%x\n",
	       __FUNCTION__, T3_RSRIE1_ADDR_L);
        return retval;
    }
    
    temp &= ~T3_RSRIE1_OOFIE;
    temp &= ~T3_RSRIE1_SEFIE;
    temp &= ~T3_RSRIE1_OOMFIE;
    if ((retval = patriot_framer_write(T3_RSRIE1_ADDR_L, temp)) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: Failed to write 0x%x\n",
	       __FUNCTION__, T3_RSRIE1_ADDR_L);
        return retval;
    }
    
    /* check interrupt status */
    if ((retval = patriot_framer_interrupt_handler()) < 0) {
        printk(KERN_ALERT "Patriot Framer: %s: check interrupt status failed\n",
	       __FUNCTION__);
        return retval;
    }
    
    return retval;

}

/******************************************************************
 * Name : patriot_framer_init
 * Description : Framer module init
 * Input       : None
 * Output      : 0/1
 ******************************************************************
 */
static int __init patriot_framer_init(void)
{
    /* Debug turn off */
    patriot_framer_debug_onoff(0);

    if (patriot_framer_debug) {
        printk(KERN_ALERT "Patriot Framer: %s: ***START***\n", __FUNCTION__);
    }

    /* Init SPI */
    if (patriot_framer_debug) {
        printk(KERN_ALERT "Patriot Framer: %s: Init SPI\n",
            __FUNCTION__);
    }
    if (patriot_framer_spi_init() < 0) {
        if (patriot_framer_debug) {
            printk(KERN_ALERT "Patriot Framer: %s: Init SPI failed.\n",
                __FUNCTION__);
        }
        return -1;
    }

    if (patriot_framer_debug) {
        printk(KERN_ALERT "Patriot Framer: %s: ***END***\n", __FUNCTION__);
    }

    return 0;
}
module_init(patriot_framer_init);

/******************************************************************
 * Name : patriot_framer_exit
 * Description : Framer module exit
 * Input       : None
 * Output      : 0/1
 ******************************************************************
 */
static void __exit patriot_framer_exit(void)
{
    if (patriot_framer_debug) {
        printk(KERN_ALERT "Patriot Framer: %s\n", __FUNCTION__);
    }
    patriot_framer_spi_exit();
}
module_exit(patriot_framer_exit);

MODULE_AUTHOR("Patriot");
MODULE_DESCRIPTION("Patriot Framer");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: patriot_framer_main.c,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.4  2014/03/06 01:56:52  steja
 * 1. added cli command for margining patriot voltage
 * 2. enhance framer interrupt and ecc memory test timing
 *
 * Revision 1.3  2012/12/03 09:07:47  steja
 * add check realtime register Framer interrupt handler
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.2.3  2012/04/12 18:37:03  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.2.2  2012/03/27 07:45:06  steja
 * Fix Warning compilation
 *
 * Revision 1.1.2.1  2012/03/13 13:31:53  steja
 * Support Framer Interrupt
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
