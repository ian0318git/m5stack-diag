/* $Id: oir_ltc4215_api.c,v 1.2 2012/03/28 00:38:14 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/oir_ltc4215_api.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:          oir_test.c
 *
 * Description:       Common file for the OIR on the SM / SM adapter card
 *
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */
#include "endians.h"
#include "common.h"
#include "proto.h"
#include "error.h"
#include "types.h"
#include "queryflags.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"

static int  oir_ltc4215_dev_attach(dev_object_t *dev, void *);
static void oir_ltc4215_dev_detach(dev_object_t *dev);
static uint32 oir_err_report (dev_object_t *dev, char *err_msg,
                    uint32 err_type);


/**********************************************************************
*
* Function: oir_ltc4215_dev_attach
*
* Description: Create an object of the ltc4215 device driver
*
* Input: dev - pointer to the device driver object
*           slot - slot number
*
* Output: PASS/FAIL
*
**********************************************************************
*/
static int
oir_ltc4215_dev_attach (dev_object_t *dev, void *p)
{
    dev_ltc4215_object_t *dev_oir = (dev_ltc4215_object_t *)dev;
    int rc = PASSED;
    n2g_i2c_if_t *i2c_if = (n2g_i2c_if_t *)p;
    
    i2c_if = dev_oir->i2c_p;
    i2c_if->offset = LTC4215_FAULT_REG;
    i2c_if->i2c_speed = N2G_I2C_400KHZ;

    rc = dev_ltc4215_create(dev, (dev_error_report_t)oir_err_report);
    if (rc != PASSED) {
        return (FAILED);
    }
    /*
     * Initialize the call-out function vectors:
     */
    dev_oir->callout_fvt->open   = n2g_i2c_open;
    dev_oir->callout_fvt->close  = n2g_i2c_close;
    dev_oir->callout_fvt->init   = n2g_i2c_init;
    dev_oir->callout_fvt->rd	  = n2g_i2c_read;
    dev_oir->callout_fvt->wr	  = n2g_i2c_write;

    rc = dev_oir->base.dev_object_fvt->dev_attach(dev);

    return (rc);
}

/**********************************************************************
*
* Function: oir_ltc4215_dev_detach
*
* Description: Detach an object of the ltc4215 device driver
*
* Input: dev - pointer to the device driver object
*
* Output: None
*
**********************************************************************
*/
static void
oir_ltc4215_dev_detach (dev_object_t *dev)
{
    dev_ltc4215_object_t *dev_oir = (dev_ltc4215_object_t *)dev;

    /* detach all device specific function vectors */
    dev_oir->base.dev_object_fvt->dev_detach((dev_object_t *)dev_oir);
    /* free all the memory from malloc in device_attach() */
    dev_oir->base.dev_object_fvt->dev_destroy((dev_object_t **)&dev_oir);
}

/*------------------------------------------------------------------------------
* Function: oir_err_report
*
* Description: 
*  This function is called by device driver code, and is used to 
*  report error to the host.
*
* Input : 
*  dev	  - Device driver object
*  err_msg  - pointer to error message
*  err_type - error type
*
* Output: PASSED/FAILED
*
*------------------------------------------------------------------------------
*/
static uint32
oir_err_report (dev_object_t *dev, char *err_msg, uint32 err_type)
{
    switch(err_type) {
        case WARNING:
            cterr('w', 0, "%s", err_msg);
            break;
        case RETRY:
            printf("\nRetry: %s\n", err_msg);
            break;
        case FATAL:
            printf("\nFatal Error: %s\n", err_msg);
            break;
        default:
            cterr('f', 0, "%s", err_msg);
            break;
    }

    return(PASSED);
}

/**********************************************************************
*
* Function: oir_ltc4215_register_test
*
* Description: A function for LTC4215 register test.
*
* Input : slot - slot number 
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
int
oir_ltc4215_register_test (void *p)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    dev_ltc4215_object_t dev_ltc4215_object;
    dev_ltc4215_object_t *dev_oir = &dev_ltc4215_object;

    int rc = PASSED;

    dev_oir->i2c_p = i2c_p;
    if (oir_ltc4215_dev_attach((dev_object_t *)dev_oir, i2c_p) == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f',0,"Failed to attach OIR LTC4215 device");
        return(FAILED);
    }

    if (dev_oir->base.dev_object_fvt->dev_init((dev_object_t *)dev_oir)
                == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Failed to init OIR LTC4215 device");
        return (FAILED);
    }

    rc = dev_oir->callin_fvt->reg_test((dev_object_t *)dev_oir);
    if (rc != PASSED) {
        cterr('f',0,"LTC4215 register test failed");
        rc = FAILED;
    }

    oir_ltc4215_dev_detach((dev_object_t *)&dev_ltc4215_object);

    return(rc);
}

/**********************************************************************
*
* Function: oir_ltc4215_led_test
*
* Description: A function for LTC4215 LED test.
*
* Input : slot - slot number 
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
int
oir_ltc4215_leds_test (void *p)
{
    uint8_t i, data_alert, data_fault;
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    printf("\n\nExercise each LED in the following sequence:\n"
           "Off, Green On, Green Blink, Amber On, Amber Blink, Normal.\n");
    printf("\n!!! This is a visual test. User needs to decide whether "
           " the LED works correctly or not !!!\n\n");

    /* Save previous led values */
    if (oir_ltc4215_reg_read(i2c_p, LTC4215_ALERT_REG, &data_alert)) {
        return(FAILED);
    }
    if (oir_ltc4215_reg_read(i2c_p, LTC4215_FAULT_REG, &data_fault)) {
        return(FAILED);
    }

    prpass(testpass, "Exercise LED_OFF on LED");
    if (util_oir_ltc4215_led(i2c_p, OIR_LED_OFF)) {
        return(FAILED);
    }
    msleep(1000);

    prpass(testpass, "Exercise GREEN_ON on LED");
    if (util_oir_ltc4215_led(i2c_p, OIR_LED_GREEN)) {
        return(FAILED);
    }
    msleep(1000);

    prpass(testpass, "Exercise GREEN_BLINK on LED");
    for (i = 0; i < 10; i++) {
        if (util_oir_ltc4215_led(i2c_p, OIR_LED_GREEN)) {
            return(FAILED);
        }
        msleep(100);
        if (util_oir_ltc4215_led(i2c_p, OIR_LED_GREEN_OFF)) {
            return(FAILED);
        }
        msleep(100);
    }

    prpass(testpass, "Exercise AMBER_ON on LED");
    if (util_oir_ltc4215_led(i2c_p, OIR_LED_AMBER)) {
        return(FAILED);
    }
    msleep(1000);

    prpass(testpass, "Exercise AMBER_BLINK on LED");
    for (i = 0; i < 10; i++) {
        if (util_oir_ltc4215_led(i2c_p, OIR_LED_AMBER)) {
            return(FAILED);
        }
        msleep(100);
        if (util_oir_ltc4215_led(i2c_p, OIR_LED_AMBER_OFF)) {
            return(FAILED);
        }
        msleep(100);
    }

    printf("\n\nAfter the LED test, restore the original LED setting.\n");

    /* Restore previous led values */
    if (oir_ltc4215_reg_write(i2c_p, LTC4215_ALERT_REG, &data_alert)) {
        return(FAILED);
    }
    if (oir_ltc4215_reg_write(i2c_p, LTC4215_FAULT_REG, &data_fault)) {
        return(FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: oir_ltc4215_reg_write
 *
 * Description: LTC4215 Register Write.
 *
 * Input : slot -   slot number
 *            reg -   register
 *            data - data to write
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
oir_ltc4215_reg_write (void *p, uchar reg, uchar *data)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    dev_ltc4215_object_t dev_ltc4215_object;
    dev_ltc4215_object_t *dev_oir = &dev_ltc4215_object;
    int rc = PASSED;

    dev_oir->i2c_p = i2c_p;
    if (oir_ltc4215_dev_attach((dev_object_t *)dev_oir, i2c_p) == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f',0,"Failed to attach OIR LTC4215 device");
		return(FAILED);
    }

    if (dev_oir->base.dev_object_fvt->dev_init((dev_object_t *)dev_oir)
                == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Failed to init OIR LTC4215 device");
        return (FAILED);
    }

    dev_oir->i2c_p->offset = reg;

    rc = dev_oir->callin_fvt->reg_wr((dev_object_t *)dev_oir, data,
                dev_oir->i2c_p->offset);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to write i2c ret_code = %#x\n", rc);
        rc = FAILED;
    }

    oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
    return(PASSED);
}

/**********************************************************************
 *
 * Function: util_oir_ltc4215_reg_read
 *
 * Description: LTC4215 Register Read.
 *
 * Input : slot -   slot number
 *            reg -   register
 *            data - data to read
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
oir_ltc4215_reg_read (void *p, uchar reg, uchar *data)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    dev_ltc4215_object_t dev_ltc4215_object;
    dev_ltc4215_object_t *dev_oir = &dev_ltc4215_object;
    int rc = PASSED;

    dev_oir->i2c_p = i2c_p;
    if (oir_ltc4215_dev_attach((dev_object_t *)dev_oir, i2c_p) == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f',0,"Failed to attach OIR LTC4215 device");
        return(FAILED);
    }

    if (dev_oir->base.dev_object_fvt->dev_init((dev_object_t *)dev_oir)
                == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Failed to init OIR LTC4215 device");
        return (FAILED);
    }

    dev_oir->i2c_p->offset = reg;

    rc = dev_oir->callin_fvt->reg_rd((dev_object_t *)dev_oir, data,
                dev_oir->i2c_p->offset);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to write i2c ret_code = %#x\n", rc);
        rc = FAILED;
    }

    oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
    return(rc);
}




/**********************************************************************
*
* Function: util_oir_ltc4215_led
*
* Description: A function for lighting LTC4215 LEDs.
*
* Input : slot - slot number
*            led_color - led color
*            en  - led enable
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
int
util_oir_ltc4215_led (void *p, uchar led_color)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    dev_ltc4215_object_t dev_ltc4215_object;
    dev_ltc4215_object_t *dev_oir = &dev_ltc4215_object;
    int rc = PASSED;

    assert(p);
    
    dev_oir->i2c_p = i2c_p;
    if (oir_ltc4215_dev_attach((dev_object_t *)dev_oir, i2c_p) == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f',0,"Failed to attach OIR LTC4215 device");
        return(FAILED);
    }

    if (dev_oir->base.dev_object_fvt->dev_init((dev_object_t *)dev_oir)
                == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Failed to init OIR LTC4215 device");
        return (FAILED);
    }

    switch(led_color) {
        case OIR_LED_AMBER:
            rc = dev_oir->callin_fvt->pg_led((dev_object_t *)dev_oir, TRUE);
            break;
		case OIR_LED_GREEN:
            rc = dev_oir->callin_fvt->gpio2_led((dev_object_t *)dev_oir, TRUE);
            break;
		case OIR_LED_AMBER_ONLY:
		    rc = dev_oir->callin_fvt->pg_led((dev_object_t *)dev_oir, TRUE);
            rc |= dev_oir->callin_fvt->gpio2_led((dev_object_t *)dev_oir,
                        FALSE);
		    break;
        case OIR_LED_GREEN_ONLY:
            rc = dev_oir->callin_fvt->pg_led((dev_object_t *)dev_oir, FALSE);
            rc |= dev_oir->callin_fvt->gpio2_led((dev_object_t *)dev_oir,
                        TRUE);
            break;
        case OIR_LED_AMBER_OFF:
            rc = dev_oir->callin_fvt->pg_led((dev_object_t *)dev_oir, FALSE);
            break;
        case OIR_LED_GREEN_OFF:
            rc = dev_oir->callin_fvt->gpio2_led((dev_object_t *)dev_oir, FALSE);
            break;
        case OIR_LED_OFF:
        default:
            rc = dev_oir->callin_fvt->pg_led((dev_object_t *)dev_oir, FALSE);
            rc |= dev_oir->callin_fvt->gpio2_led((dev_object_t *)dev_oir,
                        FALSE);
            break;
    }

    oir_ltc4215_dev_detach((dev_object_t *)dev_oir);

    return(rc);
}

/**********************************************************************
 *
 * Function: util_oir_ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : slot - slot number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
util_oir_ltc4215_reg_write (void *p)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    dev_ltc4215_object_t dev_ltc4215_object;
    dev_ltc4215_object_t *dev_oir = &dev_ltc4215_object;
    uint8_t data = 0;
    int rc = PASSED;

    printf("\n\nLTC4215 OIR Register Write\n\n");

    dev_oir->i2c_p = i2c_p;
    if (oir_ltc4215_dev_attach((dev_object_t *)dev_oir, i2c_p) == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f',0,"Failed to attach OIR LTC4215 device");
		return(FAILED);
    }

    if (dev_oir->base.dev_object_fvt->dev_init((dev_object_t *)dev_oir)
                == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Failed to init OIR LTC4215 device");
        return (FAILED);
    }

    dev_oir->i2c_p->offset = gethex_answer("Reg offset to write", 0, 0, 0x6);

    rc = dev_oir->callin_fvt->reg_rd((dev_object_t *)dev_oir, &data,
                dev_oir->i2c_p->offset);
    if (rc != PASSED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Unable to read i2c ret_code = %#x\n", rc);
        return (FAILED);
    }

    data = gethex_answer("Data to write", data, 0, 0xff);

    rc = dev_oir->callin_fvt->reg_wr((dev_object_t *)dev_oir, &data,
                dev_oir->i2c_p->offset);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to write i2c ret_code = %#x\n", rc);
        rc = FAILED;
    }

    oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
    return(rc);
}

/**********************************************************************
 *
 * Function: util_oir_ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : slot - slot number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
util_oir_ltc4215_reg_read (void *p)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    dev_ltc4215_object_t dev_ltc4215_object;
    dev_ltc4215_object_t *dev_oir = &dev_ltc4215_object;
    uint8_t data = 0;
    int rc = PASSED;

    printf("\n\nLTC4215 OIR Register Read\n\n");

    dev_oir->i2c_p = i2c_p;
    if (oir_ltc4215_dev_attach((dev_object_t *)dev_oir, i2c_p) == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f',0,"Failed to attach OIR LTC4215 device");
        return(FAILED);
    }

    if (dev_oir->base.dev_object_fvt->dev_init((dev_object_t *)dev_oir)
                == FAILED) {
        oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
        cterr('f', 0, "Failed to init OIR LTC4215 device");
        return (FAILED);
    }

    dev_oir->i2c_p->offset = gethex_answer("Reg offset to read", 0, 0, 0x6);

    rc = dev_oir->callin_fvt->reg_rd((dev_object_t *)dev_oir, &data, 
                dev_oir->i2c_p->offset);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to read i2c ret_code = %#x\n", rc);
        rc = FAILED;
    } else {
        printf("\nRegister @ %#x = %#x\n", dev_oir->i2c_p->offset, data);
    }

    oir_ltc4215_dev_detach((dev_object_t *)dev_oir);
    return(rc);
}


/******** History ******** 
$Log: oir_ltc4215_api.c,v $
Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
