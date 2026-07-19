/* $Id: patriot_framer_spi.c,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_framer_spi.c
 *
 * Description: FRAMER SPI DRIVER
 *
 *
 * Author: Sofian Teja, port from IOS
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/*************************************************/
/* Header files */
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <asm/qe.h>
#include "patriot_framer_spi.h"
#include "klm_defs.h"

/*************************************************/
/* Variables */
int patriot_framer_spi_debug = 0;
module_param(patriot_framer_spi_debug, int, S_IRUGO);

static const char this_driver_name[] = "framer";
struct patriot_framer_dev {
    struct spi_device *spi_device;
    struct mutex lock;
    struct work_struct work;
    int irq_recvd;
    patriot_intr_dev_t *intr_dev;
};

static struct patriot_framer_dev *patriot_framer_device;

extern patriot_intr_dev_t *glob_intr_dev;

/*************************************************/
/* Function declarations */
static irqreturn_t patriot_framer_irq(int irq, void *context_data);
static void patriot_framer_work(struct work_struct *work);
static int patriot_framer_probe(struct spi_device *spi_device);
static int patriot_framer_remove(struct spi_device *spi_device);

/*************************************************/
/* Function definitions */


/******************************************************************
 * Name : patriot_framer_count
 * Description : increment framer int count
 * Input       : None
 * Output      : None
 ******************************************************************
 */
void patriot_framer_count (void)
{
    patriot_framer_device->intr_dev->framer_intr_cnt++;
    return;
}

/******************************************************************
 * Name : patriot_framer_irq
 * Description : framer irq routine
 * Input       : irq = irq to disable
 *               context_data = framer dev pointer
 * Output      : IRQ_HANDLED
 ******************************************************************
 */
static irqreturn_t patriot_framer_irq(int irq, void *context_data)
{
    struct patriot_framer_dev *pfd = context_data;

    /* disable irq */
    disable_irq_nosync(irq);

    if (pfd) {
    	printk("\n Framer recvd interrupt");
        schedule_work(&pfd->work);
    }

    return IRQ_HANDLED;
}

/******************************************************************
 * Name : patriot_framer_work
 * Description : framer work routine
 * Input       : work_struct
 * Output      : None
 ******************************************************************
 */
static void patriot_framer_work(struct work_struct *work)
{
    int ret, i;
    struct patriot_framer_dev *pfd =
            container_of(work, struct patriot_framer_dev, work);

    if (pfd) {
        /* check Framer id */
#ifdef DEBUG
        patriot_framer_id();
#endif
        /* check handle interrupt status */
        ret = patriot_framer_interrupt_handler();

        if (ret < 0) {
        	printk("\nCheck interrupt handler fail !");
        	return;
        }
        /* return how many interrupt is detected */
        patriot_framer_device->intr_dev->framer_intr_bit = ret;

        /* Make sure the interrupt bit is cleared before enable interrupt */
        for (i = 0; i < 5; i++) {
            mdelay(100);
            ret = patriot_framer_interrupt_disabled();
            if (ret == 0) {
      	        break;
            }
        }

        if (ret < 0) {
            printk("\nCheck interrupt handler disable fail !");
            return;
        }

        /* re-enable irq */
        if (ret == 0) {
	        printk("\nenabling interrupt");
            enable_irq(pfd->spi_device->irq);
        }
    }
    return;
}

/******************************************************************
 * Name : patriot_framer_probe
 * Description : framer probe routine
 * Input       : spi_device structure
 * Output      : 0/1/-ENOMEM
 ******************************************************************
 */
static int patriot_framer_probe(struct spi_device *spi_device)
{
    int retval = 0;

    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s\n", __FUNCTION__);
    }

    /* Initialize Patriot Framer device data structure */
    patriot_framer_device = kzalloc(sizeof(struct patriot_framer_dev),GFP_KERNEL);
    if (!patriot_framer_device) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s\n",
                    __FUNCTION__,
                    "Out of memory");
        }
        return -ENOMEM;
    }

    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s: %s\n",
                    __FUNCTION__,
                    "initialize patriot framer device");
    }
    memset(patriot_framer_device, 0, sizeof(struct patriot_framer_dev));
    INIT_WORK(&patriot_framer_device->work, patriot_framer_work);
    mutex_init(&patriot_framer_device->lock);
    patriot_framer_device->irq_recvd = 0;
    patriot_framer_device->spi_device = spi_device;

    patriot_framer_device->intr_dev = glob_intr_dev;

    patriot_framer_device->intr_dev->framer_intr_cnt = 0;

    /* Request irq */
    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s: %s: %d\n",
                    __FUNCTION__,
                    "request irq:",
                    patriot_framer_device->spi_device->irq);
    }
    retval = request_irq(patriot_framer_device->spi_device->irq,
                         patriot_framer_irq,
                         0,
                         this_driver_name,
                         patriot_framer_device);
    if (retval != 0) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s: %d\n",
                    __FUNCTION__,
                    "error from request_irq",
                    retval);
        }
        patriot_framer_device->spi_device = NULL;
        kfree(patriot_framer_device);
        patriot_framer_device = 0;
        return retval;
    }

    return 0;

}

/******************************************************************
 * Name : patriot_framer_remove
 * Description : framer remove routine
 * Input       : spi_device structure
 * Output      : 0
 ******************************************************************
 */
static int patriot_framer_remove(struct spi_device *spi_device)
{
    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s\n", __FUNCTION__);
    }
    if (patriot_framer_device) {
        free_irq(patriot_framer_device->spi_device->irq, patriot_framer_device);
        patriot_framer_device->spi_device = NULL;
        kfree(patriot_framer_device);
        patriot_framer_device = 0;
    }
    return 0;
}

static struct spi_driver patriot_framer_driver = {
    .driver = {
        .name = this_driver_name,
        .owner = THIS_MODULE,
    },
    .probe = patriot_framer_probe,
    .remove = __devexit_p(patriot_framer_remove),
};

/******************************************************************
 * Name : patriot_framer_spi_init
 * Description : framer spi init routine
 * Input       : None
 * Output      : 0/error
 ******************************************************************
 */
int patriot_framer_spi_init(void)
{
    int error;

    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s\n", __FUNCTION__);
    }
    error = spi_register_driver(&patriot_framer_driver);
    if (error < 0) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s: %d\n",
            __FUNCTION__, "spi_register_driver() failed",
            error);
        }
        return error;
    }
    return 0;
}

/******************************************************************
 * Name : patriot_framer_spi_exit
 * Description : framer spi exit routine
 * Input       : None
 * Output      : None
 ******************************************************************
 */
void patriot_framer_spi_exit(void)
{
    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s\n", __FUNCTION__);
    }
    spi_unregister_driver(&patriot_framer_driver);
}

/******************************************************************
 * Name : patriot_framer_write
 * Description : framer spi write routine
 * Input       : wraddr , wrdata
 * Output      : 0/-ENOMEM
 ******************************************************************
 */
int patriot_framer_write(unsigned short wraddr, unsigned char wrdata)
{
    ssize_t retval = 0;
    u8 hi_byte = 0;
    u8 lo_byte = 0;
    u8 dummy_byte = wrdata;
    struct spi_message  message;
    struct spi_transfer x;
    u8 *local_buf;

    /* 14-bit address:
     * low byte: b6..b0 burst_bit
     * high byte: rw_bit b13..b7
     */
    hi_byte = ((wraddr & 0x3f80) >> 7) & (~0x80);
    lo_byte = (wraddr & 0x7f) << 1; /* burst bit not set */
    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s: "
            "wraddr=0x%x, hi_byte=0x%x, lo_byte=0x%x, wrdata=0x%x\n",
            __FUNCTION__, wraddr, hi_byte, lo_byte, wrdata);
    }

    spi_message_init(&message);
    memset(&x, 0, sizeof x);
    x.len = 3;  /* write 3 bytes: hi_byte, lo_byte, dummy_byte */
    spi_message_add_tail(&x, &message);
    x.delay_usecs = 100;

    local_buf = kzalloc(32, GFP_KERNEL);
    if (!local_buf) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s\n",
                __FUNCTION__, "Cannot allocate local buffer.");
        }
        return -ENOMEM;
    }

    /* write 3 bytes: hi_byte, lo_byte, dummy_byte */
    memcpy(local_buf, &hi_byte, 1);
    memcpy((local_buf+1), &lo_byte, 1);
    memcpy((local_buf+2), &dummy_byte, 1);
    x.tx_buf = local_buf;
    x.rx_buf = 0;

    /* do the i/o */
    retval = spi_sync(patriot_framer_device->spi_device, &message);
    kfree(local_buf);
    if (retval < 0) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s: %d\n",
                __FUNCTION__, "spi_sync() failed", retval);
        }
    }
    return retval;
}

/******************************************************************
 * Name : patriot_framer_read
 * Description : framer spi read routine
 * Input       : rxaddr , rxdata
 * Output      : 0/-ENOMEM
 ******************************************************************
 */
int patriot_framer_read(unsigned short rxaddr, unsigned char *rxdata)
{
    ssize_t retval = 0;
    u8 hi_byte = 0;
    u8 lo_byte = 0;
    u8 dummy_byte = 0xff;
    struct spi_message  message;
    struct spi_transfer x;
    u8 *local_buf;
    u8 val[3];

    /* 14-bit address:
     * low byte: b6..b0 burst_bit
     * high byte: rw_bit b13..b7
     */
    hi_byte = ((rxaddr & 0x3f80) >> 7) | 0x80;
    lo_byte = (rxaddr & 0x7f) << 1;
    if (patriot_framer_spi_debug) {
        printk(KERN_ALERT "Patriot Framer SPI: %s: rxaddr=0x%x, "
            "hi_byte=0x%x, lo_byte=0x%x\n", __FUNCTION__,
            rxaddr, hi_byte, lo_byte);
    }

    spi_message_init(&message);
    memset(&x, 0, sizeof x);
    x.len = 3;  /* write 3 bytes: hi_byte, lo_byte, dummy_byte */
    spi_message_add_tail(&x, &message);
    x.delay_usecs = 100;

    local_buf = kzalloc(32, GFP_KERNEL);
    if (!local_buf) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s\n",
                __FUNCTION__, "Cannot allocate local buffer.");
        }
        return -ENOMEM;
    }

    /* write 3 bytes: hi_byte, lo_byte, dummy_byte */
    memcpy(local_buf, &hi_byte, 1);
    memcpy((local_buf+1), &lo_byte, 1);
    memcpy((local_buf+2), &dummy_byte, 1);
    x.tx_buf = local_buf;
    x.rx_buf = local_buf + 3;

    /* do the i/o */
    retval = spi_sync(patriot_framer_device->spi_device, &message);
    if (retval == 0) {
        memcpy(&val[0], x.rx_buf, 3);
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s 0x%x, 0x%x, 0x%x\n",
                __FUNCTION__,
                "spi_sync() done: received data = ",
                val[0], val[1], val[2]);
        }
    }

    kfree(local_buf);
    *rxdata = val[2];   /* return the 3rd byte */
    if (retval < 0) {
        if (patriot_framer_spi_debug) {
            printk(KERN_ALERT "Patriot Framer SPI: %s: %s: %d\n",
                __FUNCTION__, "spi_sync() failed", retval);
        }
    }
    return retval;
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: patriot_framer_spi.c,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.4  2014/03/06 01:56:52  steja
 * 1. added cli command for margining patriot voltage
 * 2. enhance framer interrupt and ecc memory test timing
 *
 * Revision 1.3  2012/08/07 07:19:34  steja
 * Update the framer interrupt handler to add more time to check the interrupt bit is cleared.
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
 * Revision 1.1.2.1  2012/03/13 13:31:09  steja
 * Support Framer Interrupt
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
