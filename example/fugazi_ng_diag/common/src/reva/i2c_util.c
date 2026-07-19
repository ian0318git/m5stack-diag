/* $Id: i2c_util.c,v 1.3 2016/05/09 05:51:57 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/i2c_util.c,v $
 *
 * i2c_util.c - i2c diagnostic utilities, referenced in Utilities Menu
 *              devices: voltage margin
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
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
        printf("e: exit\n");
        printf("\nplease input: ");

        get_line(buffer, sizeof(buffer));

        if (buffer[0] == 'e') {
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
    if (volt != VOLTAGE_10_NORMAL) {
        cterr('f', 0, "1.0V margin error.");
        return FAILED;
    }

    voltage_no_margin();

    return PASSED;
}

/******** History ******** 
$Log: i2c_util.c,v $
Revision 1.3  2016/05/09 05:51:57  umlin
Reva:
common/src/reva/diag.c        => Change wording in comment
common/src/reva/reva_ge_dma.c => Change wording in comment
common/src/reva/i2c_util.c    => Change wording for function name
common/src/reva/i2c_util.h    => Change wording for function name
common/src/reva/reva_ge_phy.c => Polling to check copper link status
utils/banner.sh               => Add banner BOX_TYPE for Reva
common/src/reva/Makefile      => Update FPGA bin file: reva_sb_mboot_rel

Revision 1.2  2016/05/06 03:43:52  umlin
Reva: Commit Reva module side diag codes to main trunk


$Endlog$
*/
