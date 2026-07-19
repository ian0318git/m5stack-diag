/* $Id: platform_barometer.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_barometer.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_barometer.c
 * Description: Operation Overlord I2C device Barometer
 *              related diag tests and utilities.
 *
 * Ported from fugazi_barometer.c
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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
#include "i2c_address.h"
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

static unsigned short int PressCntdec;
static unsigned short int TempCntdec;
static unsigned short int siPcomp;
static unsigned char I2CCoeff[12];

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
                                         MB_I2C_ADDR_SENSOR);

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
 * Function   : alt_sensor_write
 * Description: To write the data into expected register.
 * Inputs     : *i2c_if - Pointer to the expected I2C interface structure
 *              offset - the offset of the expected register to write
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t alt_sensor_write (n2g_i2c_if_t *i2c_if, uint32_t offset)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"env_write: buf is null");
    }
	
    i2c_if->offset = offset;

    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);  /* Env MCU I2C cycle time */
        return (FAILED);    
    }

    msleep(REN_I2C_PROC_TIME);  /* Env MCU I2C cycle time */
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

    /* Unreset Barometer */
    unreset_platform_ext_dev(FPGA_EXT_BAR_RST);

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

    if (alt_sensor_write(&i2c_if, FUGAZI_BRMTR_BOTH) != PASSED) {
        printf("%s:%d Failed to start BOTH Conversion.(0x%02X)",
               __FUNCTION__, __LINE__, FUGAZI_BRMTR_BOTH);
        return (FAILED);
    }

    /* Read Pressure and Temps. */
    if (alt_sensor_read(&i2c_if, FUGAZI_BRMTR_POUTL) != PASSED) {
        printf("%s:%d Failed to read Barometer POUTL reg.(0x%02X)",
               __FUNCTION__, __LINE__, FUGAZI_BRMTR_POUTL);
        return (FAILED);
    }

    PressCntdec=  i2c_data_val;
    if (alt_sensor_read(&i2c_if, FUGAZI_BRMTR_POUTH) != PASSED) {
        printf("%s:%d Failed to read Barometer POUTH reg.(0x%02X)",
               __FUNCTION__, __LINE__, FUGAZI_BRMTR_POUTH);
        return (FAILED);
    }

    PressCntdec= PressCntdec + (i2c_data_val << 8);
    if (alt_sensor_read(&i2c_if, FUGAZI_BRMTR_TOUTL) != PASSED) {
        printf("%s:%d Failed to read Barometer TOUTL reg.(0x%02X)",
               __FUNCTION__, __LINE__, FUGAZI_BRMTR_TOUTL);
        return (FAILED);
    }

    TempCntdec= i2c_data_val;
    if (alt_sensor_read(&i2c_if, FUGAZI_BRMTR_TOUTH) != PASSED) {
        printf("%s:%d Failed to read Barometer TOUTH reg.(0x%02X)",
               __FUNCTION__, __LINE__, FUGAZI_BRMTR_TOUTH);
        return (FAILED);
    }
    TempCntdec= TempCntdec + (i2c_data_val << 8);

    /* Read Coefficients */
    for (i = 0; i < 12; i++) {
        i2c_if.buf = (char *)&I2CCoeff[i];

        if (alt_sensor_read(&i2c_if, (i + FUGAZI_BRMTR_COEF1)) != PASSED) {
            printf("%s:%d Failed to read Coefficient (0x%02X)",
                   __FUNCTION__, __LINE__, (i + FUGAZI_BRMTR_COEF1));
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

/*******************************************************************************
 *
 * Function   : fugazi_check_system_pressure
 * Description: Function to check system pressure.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int fugazi_check_system_pressure (void)
{
    unsigned int sys_pressure = 0;

    if (get_barometer_values(FALSE, &sys_pressure) != PASSED) {
        cterr('f', 0, "%s: Failed to get Barometer info.", __FUNCTION__);
    }

    if (sys_pressure <= FUGAZI_SYS_PRESS_THRE) {
        cterr('f', 0, "System Pressure (from Barometer) below %3dKpa"
                      " (now = %3dKpa)",
                      FUGAZI_SYS_PRESS_THRE, sys_pressure);

        return (FAILED);
    }

    printf("Current System Pressure (from Barometer) is %3dKpa.\n",
           sys_pressure);

    return (PASSED);
}



/*-------------------------------------------------
 * $Log: platform_barometer.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:50  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
