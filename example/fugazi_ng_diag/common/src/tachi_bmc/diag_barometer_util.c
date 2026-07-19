/* $Id: diag_barometer_util.c,v 1.4 2016/06/30 02:06:51 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_barometer_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_barometer_util.c - Utility Function
 *
 * July 2015, benchen2
 * 
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "diag_barometer_util.h"
#include "diag_fpga_util.h"
#include "diag_mcu_lib.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "proto.h"
#include "platform_i2c.h"
#include "diag_fpga_lib.h"
#include "diag_barometer_lib.h"


typedef long int S32;
typedef short int S16;
static unsigned short int PressCntdec;
static unsigned short int TempCntdec;
static unsigned short int siPcomp;
static unsigned char I2CCoeff[12];


static int show_barometer_info (void);
static int get_barometer_values (int, uint *);
int diag_show_barometer (void);
int unreset_platform_ext_dev_barameter(int); 
/* Sub Menu used for Barometer utility.
 */
static submenu_xtable_t barometer_util_submenu_table[] = {
    {"Show Barometer Info", (type_t(*)())show_barometer_info,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define BAROMETER_UTIL_SUBMENU_TABLE_SIZE (sizeof(barometer_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

static mitem_t barometer_util_primary_items[BAROMETER_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t barometer_util_secondary_items[BAROMETER_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t barometer_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    barometer_util_primary_items,
};

menuinfo_t *barometer_util_submenup = &barometer_util_subtest_menu;

int diag_barometer_util (void)
{
    build_primary_submenu(barometer_util_submenu_table,
    		     BAROMETER_UTIL_SUBMENU_TABLE_SIZE,
                 "BAROMETER", &barometer_util_submenup);
    build_secondary_submenu(barometer_util_submenu_table,
    		     BAROMETER_UTIL_SUBMENU_TABLE_SIZE,
                 barometer_util_secondary_items);

    menu(barometer_util_submenup, barometer_util_secondary_items, '\0');
    return (PASSED);
}

int diag_show_barometer (void) {
    unsigned int sys_pressure = 0;
    set_nios_mode(NIOS_DISABLE_MODE);
    
    if (get_barometer_values(FALSE, &sys_pressure) ==FAILED ){
        return (FAILED);
    }
    printf("Current System Pressure(from barometer) is %dKpa\n", sys_pressure); 
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : get_barometer_values
 * Description: To read barometer and apply pressure compensation algorithm.
 * Inputs     : print_opt - To determine dump barometer info or not
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_barometer_values (int print_opt, uint *pressure_val)
{
    unsigned int siPcomp_int;
    signed char  sia0MSB;
    signed char  sia0LSB;
    signed char  sic12MSB;
    signed char  sic12LSB;
    signed char  sib1MSB;
    signed char  sib1LSB;
    signed char  sib2MSB;
    signed char  sib2LSB;
    signed char  sic11MSB;
    signed char  sic11LSB;
    signed char  sic22MSB;
    signed char  sic22LSB;

    signed short int sia0;
    signed short int sib1;
    signed short int sib2;
    signed short int sic12;
    signed short int sic11;
    signed short int sic22;
    signed int lt1;
    signed int lt2;
    signed int lt3;

    signed int si_c11x1;
    signed int si_a11;
    signed int si_c12x2;
    signed int si_a1;
    signed int si_c22x2;
    signed int si_a2;
    signed int si_a1x1;
    signed int si_y1;
    signed int si_a2x2;

    unsigned short int uiPadc;
    unsigned short int uiTadc;

    uchar i2c_data_val;
    int  i;

    n2g_i2c_if_t i2c_if;

    unreset_platform_ext_dev_barameter(FPGA_EXT_BAR_RST);

    /* Get Power Sequencer I2C interface structure */
    if (get_alt_sensor_i2c_struct(&i2c_if) != PASSED) {
        printf("%s:%d Failed to get sensor I2C structure.",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    i2c_if.buf =(char *)&i2c_data_val;

    /*
     * Start conversion and update shared memory space.
     */

    /* Start BOTH Conversion */
    i2c_data_val = 0x1;  /* Start both conversion */

    if (alt_sensor_write(&i2c_if, OVLD_BRMTR_BOTH) != PASSED) {
        printf("%s:%d Failed to start BOTH Conversion.(0x%02X)",
               __FUNCTION__, __LINE__, OVLD_BRMTR_BOTH);
        return (FAILED);
    }

    /* Read Pressure and Temps. */
    if (alt_sensor_read(&i2c_if, OVLD_BRMTR_POUTL) != PASSED) {
        printf("%s:%d Failed to read Barometer POUTL reg.(0x%02X)",
               __FUNCTION__, __LINE__, OVLD_BRMTR_POUTL);
        return (FAILED);
    }

    PressCntdec=  i2c_data_val;
    if (alt_sensor_read(&i2c_if, OVLD_BRMTR_POUTH) != PASSED) {
        printf("%s:%d Failed to read Barometer POUTH reg.(0x%02X)",
               __FUNCTION__, __LINE__, OVLD_BRMTR_POUTH);
        return (FAILED);
    }

    PressCntdec= PressCntdec + (i2c_data_val << 8);
    if (alt_sensor_read(&i2c_if, OVLD_BRMTR_TOUTL) != PASSED) {
        printf("%s:%d Failed to read Barometer TOUTL reg.(0x%02X)",
               __FUNCTION__, __LINE__, OVLD_BRMTR_TOUTL);
        return (FAILED);
    }

    TempCntdec= i2c_data_val;
    if (alt_sensor_read(&i2c_if, OVLD_BRMTR_TOUTH) != PASSED) {
        printf("%s:%d Failed to read Barometer TOUTH reg.(0x%02X)",
               __FUNCTION__, __LINE__, OVLD_BRMTR_TOUTH);
        return (FAILED);
    }
    TempCntdec= TempCntdec + (i2c_data_val << 8);

    /* Read Coefficients */
    for (i = 0; i < 12; i++) {
        i2c_if.buf = (char *)&I2CCoeff[i];

        if (alt_sensor_read(&i2c_if, (i + OVLD_BRMTR_COEF1)) != PASSED) {
            printf("%s:%d Failed to read Coefficient (0x%02X)",
                   __FUNCTION__, __LINE__, (i + OVLD_BRMTR_COEF1));
            return (FAILED);
        }
    }

    /* Check if coeff. have been populated in shared space. If not, update them. */

    /*
     * Routine below provided by Freescale to perform compensation.
     */

    /*====================================================
     * MPL115A Placing Coefficients into 16 bit variables
     *====================================================
     */
    /* coeff a0 16bit */
    sia0MSB= I2CCoeff[0];
    sia0LSB= I2CCoeff[1];
    sia0 = (S16)sia0MSB << 8;       /* s16 type: Shift to MSB */
    sia0 += (S16)sia0LSB & 0x00FF;  /* Add LSB to 16bit number */

    /* coeff b1 16bit */
    sib1MSB= I2CCoeff[2];
    sib1LSB= I2CCoeff[3];
    sib1 = sib1MSB << 8;       /* Shift to MSB */
    sib1 += sib1LSB & 0x00FF;  /* Add LSB to 16bit number */

    /* coeff b2 16bit */
    sib2MSB= I2CCoeff[4];
    sib2LSB= I2CCoeff[5];
    sib2 = sib2MSB << 8;       /* Shift to MSB */
    sib2 += sib2LSB & 0x00FF;  /* Add LSB to 16bit number */

    /* coeff c12 14bit */
    sic12MSB= I2CCoeff[6];
    sic12LSB= I2CCoeff[7];
    sic12 = sic12MSB << 8;        /* Shift to MSB only by 8 for MSB */
    sic12 += sic12LSB & 0x00FF;

    /* coeff c11 11bit */
    sic11MSB= I2CCoeff[8];
    sic11LSB= I2CCoeff[9];
    sic11 = sic11MSB << 8;        /* Shift to MSB only by 8 for MSB */
    sic11 += sic11LSB & 0x00FF;

    /* coeff c22 11bit */
    sic22MSB= I2CCoeff[10];
    sic22LSB= I2CCoeff[11];
    sic22 = sic22MSB << 8;        /* Shift to MSB only by 8 for MSB */
    sic22 += sic22LSB & 0x00FF;

    /*===================================================
     * Coefficient 9 equation compensation
     *===================================================
     */
    /*
     * Variable sizes:
     * For placing high and low bytes of the Memory addresses
     * for each of the 6 coefficients:
     * signed char (S8) sia0MSB, sia0LSB, sib1MSB,sib1LSB, sib2MSB,sib2LSB,
     * sic12MSB,sic12LSB, sic11MSB,sic11LSB, sic22MSB,sic22LSB;
     */
    /*
     * Variable for use in the compensation, this is the 6 coefficients
     * in 16bit form, MSB+LSB.
     * signed int (S16) sia0, sib1, sib2, sic12, sic11, sic22;
     */
    /*
     * Variable used to do large calculation as 3 temp variables in the process below
     * signed long (S32) lt1, lt2, lt3;
     */
    /*
     * Variables used for Pressure and Temperature Raw.
     * unsigned int (U16) uiPadc, uiTadc.
     * signed (N=number of bits in coefficient, F-fractional bits)
     * s(N,F)
     * The below Pressure and Temp or uiPadc and uiTadc are shifted from
     * the MSB+LSB values to remove the zeros in the LSB since this
     * 10bit number is stored in 16 bits. i.e 0123456789XXXXXX becomes 0000000123456789
     */

     uiPadc = PressCntdec >> 6;

    /* Note that the PressCntdec is the raw value from
     * the MPL115A data address. Its shifted >>6 since its 10 bit.
     */

     uiTadc = TempCntdec >> 6;

    /*
     * Note that the TempCntdec is the raw value from the MPL115A data address.
     * Its shifted >> 6 since its 10 bit.
     */

    /******* STEP 1 c11x1 = (c11 * Padc) */
    lt1 = (S32)sic11;        /* s(16,27) s(N,F+zeropad) goes from s(11,10)+11ZeroPad
                              * = s(11,22) => Left Justified = s(16,27).
                              */
    lt2 = (S32)uiPadc;       /* u(10,0) s(N,F) */
    lt3 = lt1 * lt2;         /* s(26,27) / c11 * Padc. */
    si_c11x1 = (S32)(lt3);   /* s(26,27) - EQ 1 = (c11x1 / checked). */

    /*
     * divide this hex number by 2^30 to get the correct decimal value.
     */
    /* b1 =s(14,11) => s(16,13) Left justified */

    /******* STEP 2 a11 = (b1 + c11x1) */
    lt1 = ((S32)sib1 << 14);   /* s(30,27) b1 = s(16,13) Shift b1 so that
                                * the F matches c11x1(shift by 14)
                                */
    lt2 = (S32)si_c11x1;       /* s(26,27) //ensure fractional bits are compatible */
    lt3 = lt1 + lt2;           /* s(30,27) / b1+c11x1 */
    si_a11 = (S32)(lt3 >> 14); /* s(16,13) - EQ 2 =a11 Convert this block back to s(16,X) */

    /******* STEP 3 c12x2 = (c12 * Tadc) */
    /* sic12 is s(14,13)+9zero pad = s(16,15)+9 => s(16,24) left justified */
    lt1 = (S32)sic12;        /* s(16,24) */
    lt2 = (S32)uiTadc;       /* u(10,0) */
    lt3 = lt1 * lt2;         /* s(26,24) */
    si_c12x2 = (S32)(lt3);   /* s(26,24) - EQ 3 = (c12x2 / checked) */

    /******* STEP 4 a1 = a11 + c12x2 */
    lt1 = ((S32)si_a11 << 11);  /* s(27,24) This is done by s(16,13) << 11
                                 * goes to s(27,24) to match c12x2's F part
                                 */
    lt2 = (S32)si_c12x2;        /* s(26,24) */
    lt3 = lt1 + lt2;            /* s(27,24) /a11 + c12x2 */
    si_a1 = (S32)(lt3 >> 11);   /* s(16,13) - EQ 4 = a1 / check */

    /******* STEP 5 c22x2= c22 * Tadc */
    /* c22 is s(11,10)+9zero pad = s(11,19) => s(16,24) left justified */
    lt1 = (S32)sic22;        /* s(16,30) This is done by s(11,10) + 15 zero pad
                              * goes to s(16,15)+15, to s(16,30)
                              */
    lt2 = (S32)uiTadc;       /* u(10,0) */
    lt3 = lt1 * lt2;         /* s(26,30) / c22*Tadc */
    si_c22x2 = (S32)(lt3);   /* s(26,30) - EQ 5 /= c22x2 */

    /******* STEP 6 a2= b2 + c22x2 */
    /* WORKS and loses the least in data. One extra execution.
     * Note how the 31 is really a 32 due to possible overflow.
     */
    /* b2 is s(16,14) User shifted left to => s(31,29) to match c22x2 F value */
    lt1 = ((S32)sib2 << 15);      /* s(31,29) */
    lt2 = ((S32)si_c22x2 >> 1);   /* s(25,29) s(26,30) goes to >> 16 s(10,14)
                                   * to match F from sib2
                                   */
    lt3 = lt1+lt2;                /* s(32,29) but really is a s(31,29) due to
                                   * overflow the 31 becomes a 32.
                                   */
    si_a2 = ((S32)lt3 >> 16);     /* s(16,13) */

    /******* STEP 7 a1x1= a1 * Padc */
    lt1 = (S32)si_a1;       /* s(16,13) */
    lt2 = (S32)uiPadc;      /* u(10,0) */
    lt3 = lt1 * lt2;        /* s(26,13) /a1*Padc */
    si_a1x1 = (S32)(lt3);   /* s(26,13) - EQ 7 /=a1x1 /check */

    /******* STEP 8 y1= a0 + a1x1 */
    /* a0 = s(16,3) */
    lt1 = ((S32)sia0 << 10);    /* s(26,13) This is done since has to match a1x1
                                 * F value to add. So S(16,3) << 10 = S(26,13)
                                 */
    lt2 = (S32)si_a1x1;         /* s(26,13) */
    lt3 = lt1 + lt2;            /* s(26,13) /a0+a1x1 */
    si_y1 = ((S32)lt3 >> 10);   /* s(16,3) - EQ 8 /=y1 /check */

    /******* STEP 9 a2x2= a2 *Tadc */
    lt1 = (S32)si_a2;       /* s(16,13) */
    lt2 = (S32)uiTadc;      /* u(10,0) */
    lt3 = lt1 * lt2;        /* s(26,13) /a2*Tadc */
    si_a2x2 = (S32)(lt3);   /* s(26,13) - EQ 9 /=a2x2 */

    /******* STEP 10 pComp = y1 + a2x2 */
    /* y1= s(16,3) */
    lt1 = ((S32)si_y1 << 10);  /* s(26,13) This is done to match a2x2
                                * F value so addition can match. s(16,3) << 10
                                */
    lt2 = (S32)si_a2x2;        /* s(26,13) */
    lt3 = lt1 + lt2;           /* s(26,13) /y1+a2x2 */

    /* FIXED POINT RESULT WITH ROUNDING: */
    siPcomp = (S16)(lt3 >> 13);  /* goes to no fractional parts
                                  * since this is an ADC count.
                                  */

    /* decPcomp is defined as a floating point number. */
    /* Conversion to Decimal value from 1023 ADC count value.
     * ADC counts are 0 to 1023. Pressure is 50 to 115kPa correspondingly.
     */
    /* decPcomp = ((65.0/1023.0)*siPcomp)+50; */
    siPcomp_int = (unsigned) siPcomp & 0x3FF;
    siPcomp_int = siPcomp_int * 65 / 1023 + 50;

    if (print_opt == TRUE) {
        /* Show info */
        printf("RawP=%d, RawT=%d, CompP=%5d or %3dkPa, Temp=%3dC.\n",
               PressCntdec, TempCntdec, siPcomp, siPcomp_int,
               (uiTadc*100-47200) /535 + 25);

        printf("Coeff=");
        for (i = 0; i < 12; i++) {
            printf(" 0x%02x",I2CCoeff[i]);
        }
    }

    *pressure_val = siPcomp_int;

    return (PASSED);
}

int unreset_platform_ext_dev_barameter(int bit) {
    
    int value;

    if (diag_fpga_reg_read(FPGA_EXT_RESET_REG, &value) == FAILED) {
            return (FAILED);
    }
    
    value &= ~bit;
    if (diag_fpga_reg_write(FPGA_EXT_RESET_REG, value) == FAILED) {
            return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : get_barometer_values
 * Description: To read barometer and apply pressure compensation algorithm.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_barometer_info (void)
{
    unsigned int sys_pressure = 0;

    set_nios_mode(NIOS_DISABLE_MODE);
    if (get_barometer_values(TRUE, &sys_pressure) != PASSED) {
        cterr('f', 0, "%s: Failed to get Barometer info.", __FUNCTION__);
    }
    set_nios_mode(NIOS_DIAG_MODE);

    if (sys_pressure <= TACHI_SYS_PRESS_THRE) {
        cterr('f', 0, "System Pressure (from Barometer) below %3dKpa"
                      " (now = %3dKpa)",
                      TACHI_SYS_PRESS_THRE, sys_pressure);
        return (FAILED);
    }
    
    
    return (PASSED);
}

/*---------------------------------------------------------------
$Log: diag_barometer_util.c,v $
Revision 1.4  2016/06/30 02:06:51  benchen2
remove the debug message

Revision 1.3  2016/06/17 07:08:22  benchen2
fix barameter show issue

Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.4  2016/04/07 03:57:50  benchen2
fix rtc utility

Revision 1.1.2.3  2016/04/01 09:19:04  benchen2
add barometer info

Revision 1.1.2.2  2015/08/24 06:39:56  meho
Removed unreset barometer in Show Barometer Info util.

Revision 1.1.2.1  2015/07/31 07:16:55  hondwang
barometer utility


$Endlog$
*/

