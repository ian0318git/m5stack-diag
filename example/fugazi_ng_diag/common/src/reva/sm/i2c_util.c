/* $Id: i2c_util.c,v 1.2 2017/03/16 05:20:22 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/sm/i2c_util.c,v $
 *
 * i2c_util.c - i2c diagnostic utilities, referenced in Utilities Menu
 *              devices: voltage margin
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "i2c_util.h"

/**********************************************************************
 * Function   : voltage_no_margin
 *
 * Description: remove the voltage and margin value
 *
 * Input      : none
 * Output     : PASSED/FAILED
 **********************************************************************/
int voltage_no_margin(void)
{
    /* Voltage 1 : V3.3, V1.8, V1.5 V1.0 */
    if (zynq_i2c_write_byte(OUT0_33, VOLTAGE_33_NO)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT1_18, VOLTAGE_18_NO)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT2_15, VOLTAGE_15_NO)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT3_10, VOLTAGE_10_NO)) {
        return FAILED;
    }

    /* Voltage 2: V1.2, V1.0(GTP) */
    if (zynq2_i2c_write_byte(OUT0_12, VOLTAGE2_12_NO)) {
        return FAILED;
    }
    
    if (zynq2_i2c_write_byte(OUT1_10, VOLTAGE2_10_NO)) {
        return FAILED;
    }

    return PASSED;
}

/**********************************************************************
 * Function   : voltage_margin_high
 *
 * Description: change the voltage and margin value to high
 *
 * Input      : none
 * Output     : PASSED/FAILED
 **********************************************************************/
int voltage_margin_high(void)
{
    /* Voltage 1 : V3.3, V1.8, V1.5 V1.0 */
    if (zynq_i2c_write_byte(OUT0_33, VOLTAGE_33_HIGH)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT1_18, VOLTAGE_18_HIGH)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT2_15, VOLTAGE_15_HIGH)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT3_10, VOLTAGE_10_HIGH)) {
        return FAILED;
    }

    /* Voltage 2: V1.2, V1.0(GTP) */
    if (zynq2_i2c_write_byte(OUT0_12, VOLTAGE2_12_HIGH)) {
        return FAILED;
    }    
    if (zynq2_i2c_write_byte(OUT1_10, VOLTAGE2_10_HIGH)) {
        return FAILED;
    }

    return PASSED;
}

/**********************************************************************
 * Function   : voltage_margin_normal
 *
 * Description: change the voltage and margin value to normal
 *
 * Input      : none
 * Output     : PASSED/FAILED
 **********************************************************************/
int voltage_margin_normal(void)
{
    /* Voltage 1 : V3.3, V1.8, V1.5 V1.0 */
    if (zynq_i2c_write_byte(OUT0_33, VOLTAGE_33_NORMAL)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT1_18, VOLTAGE_18_NORMAL)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT2_15, VOLTAGE_15_NORMAL)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT3_10, VOLTAGE_10_NORMAL)) {
        return FAILED;
    }

    /* Voltage 2: V1.2, V1.0(GTP) */
    if (zynq2_i2c_write_byte(OUT0_12, VOLTAGE2_12_NORMAL)) {
        return FAILED;
    }
    if (zynq2_i2c_write_byte(OUT1_10, VOLTAGE2_10_NORMAL)) {
        return FAILED;
    }

    return PASSED;
}

/**********************************************************************
 * Function   : voltage_margin_low
 *
 * Description: change the voltage and margin value to low
 *
 * Input      : none
 * Output     : PASSED/FAILED
 **********************************************************************/
int voltage_margin_low(void)
{
    /* Voltage 1 : V3.3, V1.8, V1.5 V1.0 */
    if (zynq_i2c_write_byte(OUT0_33, VOLTAGE_33_LOW)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT1_18, VOLTAGE_18_LOW)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT2_15, VOLTAGE_15_LOW)) {
        return FAILED;
    }
    if (zynq_i2c_write_byte(OUT3_10, VOLTAGE_10_LOW)) {
        return FAILED;
    }

    /* Voltage 2: V1.2, V1.0(GTP) */
    if (zynq2_i2c_write_byte(OUT0_12, VOLTAGE2_12_LOW)) {
        return FAILED;
    }    
    if (zynq2_i2c_write_byte(OUT1_10, VOLTAGE2_10_LOW)) {
        return FAILED;
    }
    return PASSED;
}


/**********************************************************************
 * Function: voltage_margin_specific
 *
 * Description: allow the user to specify the voltage and margin value
 *
 * Input : none
 * Output: PASSED/FAILED
 **********************************************************************/
int voltage_margin_specific(void)
{
    char buffer[4];
    int ret = 0;
    uchar val;

    while (1) {
        printf("\nSelect which voltage to margin\n");
        printf("\na: 3.3V\n");
        printf("b: 1.8V\n");
        printf("c: 1.5V\n");
        printf("d: 1.0V\n");
        printf("e: 1.2V\n");  
        printf("f: 1.0V (GTP)\n");
        printf("g: exit\n");
        printf("\nplease input: ");

        get_line(buffer, sizeof(buffer));

        if (buffer[0] == 'g') {
            break;
        }
        val =
            (uchar) getdec_answer("please input value to margin : ", 0, 0,
                                  255);

        switch (buffer[0]) {
        case 'a':
            ret = zynq_i2c_write_byte(OUT0_33, val);
            break;
        case 'b':
            ret = zynq_i2c_write_byte(OUT1_18, val);
            break;
        case 'c':
            ret = zynq_i2c_write_byte(OUT2_15, val);
            break;
        case 'd':
            ret = zynq_i2c_write_byte(OUT3_10, val);
            break;
        case 'e':
            ret = zynq2_i2c_write_byte(OUT0_12, val);
            break;
        case 'f':
            ret = zynq2_i2c_write_byte(OUT1_10, val);
            break;
        default:
            break;
        }
        if (ret) {
            cterr('f', 0, "Margin set error\n");
            return (FAILED);
        }
    }
    return (PASSED);
}

/***************************************************************************
 * Function: voltage_margin_display
 *
 * Description: This function shows current margins of all kinds of voltage
 *
 * Input : None
 * Output: None
 ***************************************************************************/
void voltage_margin_display(void)
{
    volatile uchar volt;

    /* Voltage 1 : V3.3, V1.8, V1.5 V1.0 */
    if (!zynq_i2c_read_byte(OUT0_33, &volt)) {
        printf("\n3.3V current margin: %d\n", volt);
    } else {
        printf("\nMargin read error\n");
    }

    if (!zynq_i2c_read_byte(OUT1_18, &volt)) {
        printf("1.8V current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }

    if (!zynq_i2c_read_byte(OUT2_15, &volt)) {
        printf("1.5V current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }
    if (!zynq_i2c_read_byte(OUT3_10, &volt)) {
        printf("1.0V current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }

    /* Voltage 2: V1.2, V1.0 (GTP) */
    if (!zynq2_i2c_read_byte(OUT0_12, &volt)) {
        printf("1.2V current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }

    if (!zynq2_i2c_read_byte(OUT1_10, &volt)) {
        printf("1.0V (GTP) current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }

}

/***************************************************************************
 * Function: margin_test
 * Description: To do margin test
 *         Change the margin to normal and check the value is correct not not
 *
 * Input : None
 * Output: PASSED/FAILED
 ***************************************************************************/
int margin_test(void)
{
    volatile uchar volt = 0;

    prpass(testpass, "Margin test.");

    voltage_margin_normal();

    /* Voltage 1 : V3.3, V1.8, V1.5 V1.0 */
    zynq_i2c_read_byte(OUT0_33, &volt);
    if (volt != VOLTAGE_33_NORMAL) {
        cterr('f', 0, "3.3V margin error.");
        return FAILED;
    }

    zynq_i2c_read_byte(OUT1_18, &volt);
    if (volt != VOLTAGE_18_NORMAL) {
        cterr('f', 0, "1.8V margin error.");
        return FAILED;
    }
    
    zynq_i2c_read_byte(OUT2_15, &volt);
    if (volt != VOLTAGE_15_NORMAL) {
        cterr('f', 0, "1.5V margin error.");
        return FAILED;
    }

    zynq_i2c_read_byte(OUT3_10, &volt);
    if (volt != VOLTAGE_15_NORMAL) {
        cterr('f', 0, "1.0V margin error.");
        return FAILED;
    }

    /* Voltage 2: V1.2, V1.0(GTP) */
    zynq2_i2c_read_byte(OUT0_12, &volt);
    if (volt != VOLTAGE2_12_NORMAL) {
        cterr('f', 0, "1.2V margin error.");
        return FAILED;
    }

    zynq2_i2c_read_byte(OUT1_10, &volt);
    if (volt != VOLTAGE2_10_NORMAL) {
        cterr('f', 0, "1.0V (GTP) margin error.");
        return FAILED;
    }

    voltage_no_margin();

    return PASSED;
}

/*********************************************************************
 *
 * Function: zynq2_i2c_read_byte for Voltage 2
 *
 * Description: This function read 1 byte from I2C slave dev speicified
 *              used for register device
 *
 * Inputs: offset  - offset within I2C dev
 *         buf     - buffer to save data read
 *
 * Outputs: return PASSED/FAILED
 *
 *********************************************************************
 */
int zynq2_i2c_read_byte(uchar offset, volatile uchar *buf)
{
    uchar wr_val = offset;
    volatile uchar *rd_val = buf;
    if (zynq_i2c_write(ZYNQ2_I2C_ADDR_DS4424, &wr_val, 1)) {
        return (FAILED);
    }
    if (zynq_i2c_read(ZYNQ2_I2C_ADDR_DS4424, rd_val, 1)) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: zynq_i2c_write_byte for voltage 2
 *
 * Description: This function write 1 byte to I2C slave device specified offset
 *              used for register device
 * Inputs: offset  - offset within I2C dev
 *         value   - value to write
 *
 * Outputs: return PASSED/FAILED
 *
 ******************************************************************************
 */
int zynq2_i2c_write_byte(uchar offset, uchar value)
{
    uchar wr_val[2];
    wr_val[0] = offset;
    wr_val[1] = value;

    if (zynq_i2c_write(ZYNQ2_I2C_ADDR_DS4424, wr_val, 2)) {
        return (FAILED);
    }
    return (PASSED);
}

/******** History ******** 
$Log: i2c_util.c,v $
Revision 1.2  2017/03/16 05:20:22  umlin
Reva-SM: Commit Reva-SM module side diag codes to main trunk

Revision 1.1.2.1  2016/11/02 18:00:16  umlin
Reva-SM: Add 1.2V and 1.0V voltage margin. Fine-tune voltage margin value for Reva-SM


$Endlog$
*/
