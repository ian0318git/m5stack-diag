/* $Id: wifi_temp_sensor_lib.c,v 1.4 2019/01/18 05:54:47 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/wifi_temp_sensor_lib.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : wifi_temp_sensor_lib.c
 * Description: WiFi module Temperature Sensor Library
 *
 * Copyright (c) 2018 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "byteswap.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "i2c_address.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "tsn_comm.h"
#include "proto.h"
#include "platform_i2c.h"
#include "platform_fpga.h"
#include "platform_stub.h"
#include "dev_nxp_lm75b.h"
#include "plug_host_fpga_lib.h"
#include "wifi_temp_sensor_lib.h"

extern uint32 err_report(dev_object_t *, char *, uint32);
extern int get_i2c_fd(int);

static uint32_t wifi_ts_i2c_rd(uint32, ushort *);
static uint32_t wifi_ts_i2c_wr(uint32, ushort *);

int wifi_ts_dev_create(dev_lm75b_object_t *);
int wifi_ts_show_temp(void);

/*******************************************************************************
 *
 * Function    : wifi_ts_dev_create
 * Description : Function to create TMPX75 Device Object
 * Inputs      : gpio_obj - Pointer of TMPX75 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_ts_dev_create (dev_lm75b_object_t *ts_obj)
{
    dev_object_t *dev = (dev_object_t *)ts_obj;

    /* Create common device object */
    lm75b_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    ts_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    ts_obj->callout_fvt->rd = wifi_ts_i2c_rd;
    ts_obj->callout_fvt->wr = wifi_ts_i2c_wr;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : wifi_ts_show_temp
 * Description : Function to display WiFi temperature
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_ts_show_temp (void) 
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int ret;

    wifi_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret = ts_obj->callin_fvt->show_temp((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret);
}

 /*******************************************************************************
 *
 * Function    : wifi_ts_i2c_rd
 * Description : Function implementation of TMPX75 I2C Read
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t wifi_ts_i2c_rd (uint32 offset, ushort *data) 
{
    n2g_i2c_if_t *i2c_if;
    
    if (this_is_star() || this_is_supernova()) {
        i2c_if = (n2g_i2c_if_t *)(get_n2g_i2c_if(I2C_CTRL_TWO,
                                             I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP));
    } else {
        i2c_if = (n2g_i2c_if_t *)(get_n2g_i2c_if(I2C_CTRL_TWO,
                                             I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP));
    }                                     
                                             
    i2c_if->size = sizeof(ushort);
    i2c_if->buf = (char *)data;
    i2c_if->offset = offset;

    if (n2g_i2c_open(i2c_if) != PASSED) {
        printf("%s: n2g_i2c_open failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_read(i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_close(i2c_if) != PASSED) {
        printf("%s: n2g_i2c_close failed\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : wifi_ts_i2c_wr
 * Description : Function implementation of TMPX75 I2C Write
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t wifi_ts_i2c_wr (uint32 offset, ushort *data)
{
    int      wifi_i2c_fd = get_i2c_fd(CPU_I2C2);
    int      i2c_dev_addr = WIFI_I2C_ADDR_TEMP, ret_code = 0;

    if (this_is_star() || this_is_supernova()) {
        i2c_dev_addr = WIFI_I2C_STAR_ADDR_TEMP;
    } else {
        i2c_dev_addr = WIFI_I2C_ADDR_TEMP;
    }     
    
    if (wifi_i2c_fd < 0) {
        printf("%s:%d i2c-2 descriptor is not exists.\n", __func__, __LINE__);
        return (FAILED);
    }

    ret_code = ioctl(wifi_i2c_fd, I2C_SLAVE, i2c_dev_addr);
    if (ret_code < 0) {
        printf("%s:%d Failed to connect to device %#x(rc = %#x).",
               __func__, __LINE__, i2c_dev_addr, ret_code);
        return (FAILED);
    }

    if (i2c_smbus_write_word_data(wifi_i2c_fd, offset, *data) < 0) {
        printf("%s %d Failed to do I2C write %#x to %#x of device %#x.",
               __func__, __LINE__, *data, offset, i2c_dev_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: wifi_temp_sensor_lib.c,v $
Revision 1.4  2019/01/18 05:54:47  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.3  2018/11/23 08:49:53  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.52.1  2018/10/15 06:53:08  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/02/09 09:56:57  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.3  2018/01/20 07:21:47  hondwang
Fix some merge branch issue

Revision 1.1.6.2  2018/01/20 05:57:49  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/04 15:08:39  palin2
Added Star wifi temperature sensor diag tests.

$Endlog$
*/

