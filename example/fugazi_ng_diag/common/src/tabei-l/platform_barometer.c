/* $Id: platform_barometer.c,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_barometer.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_barometer.c
 * Description: Operation I2C device Barometer
 *              related diag tests and utilities.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "proto.h"
#include "goofy_i2c.h"
#include "n2g_api_rc.h"
#include "i2c_api.h"
#include "diag_i2c_addr.h"
#include "dev_csco_10698.h"
#include "dash_fpga.h"
#include "platform_i2c.h"
#include "platform_barometer.h"
#include "plat_defs.h"


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
typedef long int  S32;
typedef short int S16;

/* LPS25H register table */
static barometer_reg_info_t lps25h_reg_tbl[] = {
    {"Register 0x8: REF_P_XL", ST_REF_P_XL},
    {"Register 0x9: REF_P_L",  ST_REF_P_L},
    {"Register 0xA: REF_P_H",  ST_REF_P_H},
    {"Register 0xF: WHO_AM_I", ST_WHO_AM_I},
    {"Register 0x10: RES_CONF", ST_RES_CONF},
    {"Register 0x20: STRL_REG1", ST_STRL_REG1},
    {"Register 0x21: STRL_REG2", ST_STRL_REG2},
    {"Register 0x22: STRL_REG3", ST_STRL_REG3},
    {"Register 0x23: STRL_REG4", ST_STRL_REG4},
    {"Register 0x24: INT_CFG", ST_INT_CFG},
    {"Register 0x25: INT_SOURCE", ST_INT_SOURCE},
    {"Register 0x27: STATUS_REG", ST_STATUS_REG},
    {"Register 0x28: PRESS_OUT_XL", ST_PRESS_OUT_XL},
    {"Register 0x29: PRESS_OUT_L", ST_PRESS_OUT_L},
    {"Register 0x2A: PRESS_OUT_H", ST_PRESS_OUT_H},
    {"Register 0x2B: TEMP_OUT_L", ST_TEMP_OUT_L},
    {"Register 0x2C: TEMP_OUT_H", ST_TEMP_OUT_H},
    {"Register 0x2E: FIFO_CTRL", ST_FIFO_CTRL},
    {"Register 0x2F: FIFO_STATUS", ST_FIFO_STATUS},
    {"Register 0x30: THS_P_L", ST_THS_P_L},
    {"Register 0x31: THS_P_H", ST_THS_P_H},
    {"Register 0x39: RPDS_L", ST_RPDS_L},
    {"Register 0x3A: RPDS_H", ST_RPDS_H},
};

/*******************************************************************************
 *
 * Function   : get_alt_sensor_i2c_struct
 * Description: To get Altitude Sensor I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_alt_sensor_i2c_struct (n2g_i2c_if_t *alt_sensor_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                         MB_I2C_ADDR_BAROMETER);

    if (tmp == NULL) {
        printf("%s: Failed to get Altitude Sensor I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(alt_sensor_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : alt_sensor_read
 * Description: To read the expected offset register data out.
 * Inputs     : *i2c_if - Pointer to the expected I2C interface structure
 *              offset - the offset of the expected register to read
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t alt_sensor_read (n2g_i2c_if_t *i2c_if, uint32_t offset)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"env_read: buf is null");
    }

    i2c_if->offset = offset;
	
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        /* Unable to read data */
        printf("*** %s: Unable to read %s Register 0x%02x(rc = %#x).\n",
               __FUNCTION__, i2c_if->dev_name, i2c_if->offset, rc);
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);  /* I2C cycle time */

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : get_barometer_st_info
 * Description: To read barometer pressure.
 * Inputs     : pressure_val : To read barometer pressure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_barometer_st_info (uint *pressure_val)
{
    uchar i2c_data_val;
    unsigned int st_press_out;

    n2g_i2c_if_t i2c_if;

    /* Get Power Sequencer I2C interface structure */
    if (get_alt_sensor_i2c_struct(&i2c_if) != PASSED) {
        cterr('f', 0, "%s:%d Failed to get sensor I2C structure.",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    i2c_if.buf =(char *)&i2c_data_val;

    /* Read Pressure */
    if (alt_sensor_read(&i2c_if, ST_PRESS_OUT_XL) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read Barometer reg.(0x%02X)",
               __FUNCTION__, __LINE__, ST_PRESS_OUT_XL);
        return (FAILED);
    }

    st_press_out = i2c_data_val;

    if (alt_sensor_read(&i2c_if, ST_PRESS_OUT_L) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read Barometer reg.(0x%02X)",
               __FUNCTION__, __LINE__, ST_PRESS_OUT_L);
        return (FAILED);
    }

    st_press_out |= (i2c_data_val << 8);

    if (alt_sensor_read(&i2c_if, ST_PRESS_OUT_H) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read Barometer reg.(0x%02X)",
               __FUNCTION__, __LINE__, ST_PRESS_OUT_H);
        return (FAILED);
    }

    st_press_out |= (i2c_data_val << 16);
    *pressure_val = st_press_out/40960;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : show_barometer_info
 * Description: To read barometer and apply pressure compensation algorithm.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_barometer_info (void)
{
    unsigned int sys_pressure = 0;

        if (get_barometer_st_info(&sys_pressure) != PASSED) {
            cterr('f', 0, "%s: Failed to get Barometer info.", __FUNCTION__);
            return (FAILED);
        } else {
            printf("Current System Pressure (from Barometer) is %3dKpa.\n", sys_pressure);
        }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : display_barometer_reg
 * Description: To display barometer registers.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int display_barometer_reg (void)
{
    int ix;
    uchar i2c_data_val;
    barometer_reg_info_t *reg_info_p;

    n2g_i2c_if_t i2c_if;

    /* Get Power Sequencer I2C interface structure */
    if (get_alt_sensor_i2c_struct(&i2c_if) != PASSED) {
        cterr('f', 0, "%s:%d Failed to get sensor I2C structure.",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    i2c_if.buf = (char *)&i2c_data_val;
    reg_info_p = &lps25h_reg_tbl[0];

    for (ix = 0; ix < sizeof(lps25h_reg_tbl)/sizeof(barometer_reg_info_t); ix ++, reg_info_p ++) {
        /* Read register value */
        if (alt_sensor_read(&i2c_if, reg_info_p->offset) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read Barometer reg.(0x%02X)",
                   __FUNCTION__, __LINE__, reg_info_p->offset);
            return (FAILED);
        } else {
            printf("%-32s, value = 0x%02x\n", reg_info_p->name, i2c_data_val);
        }
    }
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: platform_barometer.c,v $
Revision 1.2  2019/10/17 02:16:25  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.1  2019/07/15 11:28:47  kehuang2
Support Barometer test and utility

Revision 1.3.2.1  2018/08/17 18:32:37  alpeng
fixed barometer for support curie 1ru

Revision 1.3  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.2.54.5  2017/11/22 08:15:55  leschen
Support Barsoom VG450.

Revision 1.2.54.4  2017/04/05 09:15:16  leschen
Sync with <ng_diag-tag-032917>

Revision 1.2.54.3  2017/03/23 06:30:38  leschen
Support barometer LPS25H.

Revision 1.2.54.2  2017/03/13 07:43:31  leschen
Support Triton system.

Revision 1.2.54.1  2017/01/04 08:49:07  leschen
Temporary hide unsupport utility.

Revision 1.2  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.4  2012/09/19 07:58:06  palin2
Add to display system pressure info in x86 Diag boot message.

Revision 1.3  2012/09/18 07:47:32  palin2
Add function to check system pressure in Diag boot-up process.

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
