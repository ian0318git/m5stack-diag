/* $Id: diag_rtc_lib.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_rtc_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_rtc_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "types.h"
#include "common.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "proto.h"
#include "platform_i2c.h"
#include "diag_i2c_lib.h"
#include "dev_1337.h"
#include "diag_rtc_lib.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int diag_rtc_dev_create(dev_ds1337_object_t *, n2g_i2c_if_t *);
int clear_rtc_osf_bit(void);


/*******************************************************************************
 *
 * Function    : diag_rtc_dev_create
 * Description : Function to create RTC, Maxim ds1337, device object.
 *               It includes to create common device object, attach device,
 *               and setup call-out function vectors.
 * Inputs      : rtc_obj - Pointer of ds1337 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_rtc_dev_create (dev_ds1337_object_t *rtc_obj, n2g_i2c_if_t *rtc_i2c_if)
{
    dev_object_t *dev = (dev_object_t *)rtc_obj;

    /* Create common device object */
    dev_1337_create(dev, (dev_error_report_t)err_report);

    /* Setup call-out function vectors */
    rtc_obj->callout_fvt->open = n2g_i2c_open;
    rtc_obj->callout_fvt->close = n2g_i2c_close;
    rtc_obj->callout_fvt->rd = n2g_i2c_read;
    rtc_obj->callout_fvt->wr = n2g_i2c_write;

    /* Setup I2C API parameter struct */
    rtc_i2c_if->i2c_bus_type = CPU_I2C2;      /* I2C bus number */
    rtc_i2c_if->i2c_dev = MB_I2C_ADDR_RTC;    /* I2C device enum */

    rtc_obj->i2c_p = rtc_i2c_if;
    rtc_obj->dt = 0;

    /* Attach deivce */
    if (rtc_obj->base.dev_object_fvt->dev_attach(dev) != PASSED) {
        printf("%s: Failed to attach RTC, Maxim ds1337, chip.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : clear_rtc_osf_bit
 * Description: Function to clear RTC status reg.(offset: 0xF) OSF bit(bit7).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int clear_rtc_osf_bit (void)
{
    int ret_val = FAILED;
    n2g_i2c_if_t rtc_i2c_if;
    n2g_i2c_if_t *rtc_i2c_if_p = &rtc_i2c_if;
    uint8_t reg_val = 0;
    int ctr = 0;

    /* Setup I2C API parameter structure */
    rtc_i2c_if_p = (n2g_i2c_if_t *)(get_n2g_i2c_if(I2C_CTRL_TWO,
                                                   I2C_MUX_ZERO,
                                                   MB_I2C_ADDR_RTC));

    rtc_i2c_if_p->offset = DS1337_STATUS_REG;
    rtc_i2c_if_p->buf = (char *)&reg_val;

    /* Read out current RTC status register value */
    if (n2g_i2c_read(rtc_i2c_if_p) != PASSED) {
        printf("%s(%d): Failed to read RTC reg. 0x%02Xh.\n",
               __func__, __LINE__, rtc_i2c_if_p->offset);
        return (FAILED);
    }

    /* Clear RTC OSF bit if needed */
    if ((reg_val & (uint8_t)DS1337_STAT_OSF) != 0) {
        reg_val &= (uint8_t)(~DS1337_STAT_OSF);

        if (n2g_i2c_write(rtc_i2c_if_p) != PASSED) {
            printf("%s(%d): Failed to write RTC reg. 0x%02Xh.\n",
                   __func__, __LINE__, rtc_i2c_if_p->offset);
            return (FAILED);
        }

        /* Comfirm if RTC OSF bit is cleared by polling RTC status reg. */
        for (ctr = 0; ctr < POLL_1SEC_W_10MS_INVL_COUNT; ctr++) {
            reg_val = DS1337_STAT_OSF;
    
            if (n2g_i2c_read(rtc_i2c_if_p) != PASSED) {
                printf("%s(%d): Failed to read RTC reg. 0x%02Xh.\n",
                       __func__, __LINE__, rtc_i2c_if_p->offset);
                return (FAILED);
            }

            if ((reg_val & (uint8_t)DS1337_STAT_OSF) == 0) {
                ret_val = PASSED;
                break;
            }
            msleep(POLL_INVL_10MS);   /* Polling interval: 10ms */
        }

        if (ret_val != PASSED) {
            printf("%s(%d): Failed to clear RTC reg.(0x%02Xh) OSF bit.\n",
                   __func__, __LINE__, rtc_i2c_if_p->offset);
            return (FAILED);
        }
    }
    return (PASSED);
}
 
/*-------------------------------------------------
 * $Log: diag_rtc_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
