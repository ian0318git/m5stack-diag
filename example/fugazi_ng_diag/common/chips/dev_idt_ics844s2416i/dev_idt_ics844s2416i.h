/* $Id: dev_idt_ics844s2416i.h,v 1.2 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_idt_ics844s2416i/dev_idt_ics844s2416i.h,v $
 *-----------------------------------------------------------------------------
 * Filename:	idt_ics844s2416i.h
 *
 * Description:	IDT FEMTOCLOCK Crystal-to-Differential HCSL/LVCMOS Frequency
 *		Synthesizer. This header file defines registers offset,
 *		defaults, read & write bitmasks, & bit locations.
 *		Meant to use as pointer or offset from a particular chip's
 *		base address.
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#ifndef __ICS844S2416I_H__
#define __ICS844S2416I_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE	80

/*
 * dev_error_report message codes
 */
typedef enum {
    ICS2416_DEV_STATE = 0,
    ICS2416_ATTACH,
    ICS2416_DETACH,
    ICS2416_INIT,
    ICS2416_OPER_EN,
    ICS2416_OPER_DIS,
    ICS2416_SHOW,
    ICS2416_REG_TEST,
    ICS2416_DESTROY,
    ICS2416_I2C_READ,
    ICS2416_I2C_WRITE,
}ics844s2416i_report_code_t;


/*
 * device callin function - service provided and defined by the device
 */
typedef struct ics2416i_callin_fvt_t_ {
    int	(*register_test)(dev_object_t *);
}ics2416i_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct ics2416i_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
}ics2416i_callout_fvt_t;

/* Registers bits text descriptor */
/* Each Qn has different function in different platforms. This struct
 * provides mechanism allowing the caller to provide special text for
 * each pin.
 */
typedef struct ics2416_bit_t_ {
    char*	q16;	/* Q16_EN */
    char*	q15;	/* Q15_EN */
    char*	q14;	/* Q14_EN */
    char*	q13;	/* Q13_EN */
    char*	q12;	/* Q12_EN */
    char*	q11;	/* Q11_EN */
    char*	q10;	/* Q10_EN */
    char*	q9;	/* Q9_EN  */
    char*	q8;	/* Q9_EN  */
    char*	q7;	/* Q9_EN  */
    char*	q6;	/* Q9_EN  */
    char*	q5;	/* Q9_EN  */
    char*	q4;	/* Q9_EN  */
    char*	q3;	/* Q9_EN  */
    char*	q2;	/* Q9_EN  */
    char*	q1;	/* Q9_EN  */
    char*	q0;	/* Q9_EN  */
} ics2416_bit_t;

/* Registers struct */
typedef struct ics844s2416_reg_t_  {
    volatile charint data;	/* 0 - Data Bytes 0 - 3 */
} ics844s2416_reg_t;

/*
** Define the FEMTOCLOCK device object structure.
*/
typedef struct dev_844s2416i_object_t_ {
    dev_object_t		base;
    ics2416i_callin_fvt_t	*callin_fvt;
    ics2416i_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t		*i2c_p;		/* I2C API interface pointer */
    reg_info_t			*reg_table_p;	/* Register test table pointer*/
    ics2416_bit_t		*bit_p;		/* Bit text pointer */
    ics844s2416_reg_t		enable;		/* Bit enable/disable param */
}dev_844s2416i_object_t;

extern void dev_ics844s2416i_create(dev_object_t *dev,
				    dev_error_report_t error_report_fn);

/* Defines for Data Byte 0 */ 
#define ICS844S2416_DB0_Q16_EN	0x80	/* Q16_EN */
#define ICS844S2416_DB0_Q15_EN	0x40	/* Q15_EN */
#define ICS844S2416_DB0_Q14_EN	0x20	/* Q14_EN */
#define ICS844S2416_DB0_Q13_EN	0x10	/* Q13_EN */
#define ICS844S2416_DB0_Q12_EN	0x08	/* Q12_EN */
#define ICS844S2416_DB0_Q11_EN	0x04	/* Q11_EN */
#define ICS844S2416_DB0_Q10_EN	0x02	/* Q10_EN */
#define ICS844S2416_DB0_Q9_EN	0x01	/* Q9_EN  */

/* Defines for Data Byte 1 */
#define ICS844S2416_DB1_Q8_EN	0x80	/* Q8_EN */
#define ICS844S2416_DB1_Q7_EN	0x40	/* Q7_EN */
#define ICS844S2416_DB1_Q6_EN	0x20	/* Q6_EN */
#define ICS844S2416_DB1_Q5_EN	0x10	/* Q5_EN */
#define ICS844S2416_DB1_Q4_EN	0x08	/* Q4_EN */
#define ICS844S2416_DB1_Q3_EN	0x04	/* Q3_EN */
#define ICS844S2416_DB1_Q2_EN	0x02	/* Q2_EN */
#define ICS844S2416_DB1_Q1_EN	0x01	/* Q1_EN */

/* Defines for Data Byte 2 */
#define ICS844S2416_DB2_Q0_EN	0x80	/* Q0_EN */
#define ICS844S2416_DB2_M4	0x40	/* M4 */
#define ICS844S2416_DB2_M3	0x20	/* M3 */
#define ICS844S2416_DB2_M2	0x10	/* M2 */
#define ICS844S2416_DB2_M1	0x08	/* M1 */
#define ICS844S2416_DB2_M0	0x04	/* M0 */
#define DB2_M_SHIFT		2	/* left shift 2 bits */

#define ICS844S2416_DB2_M_MASK	0x7C	/* M4-M0 mask */
#define ICS844S2416_DIV_72	0x00	/* 1800 MHz VCO, 90 MHz Q12     */
#define ICS844S2416_DIV_73	0x04	/* 1825 MHz VCO, 91.25 MHz Q12  */
#define ICS844S2416_DIV_74	0x08	/* 1850 MHz VCO, 92.5 MHz Q12   */
#define ICS844S2416_DIV_75	0x0C	/* 1875 MHz VCO, 93.75 MHz Q12  */
#define ICS844S2416_DIV_76	0x10	/* 1900 MHz VCO, 95 MHz Q12     */
#define ICS844S2416_DIV_77	0x14	/* 1925 MHz VCO, 96.25 MHz Q12  */
#define ICS844S2416_DIV_78	0x18	/* 1950 MHz VCO, 97.5 MHz Q12   */
#define ICS844S2416_DIV_79	0x1C	/* 1975 MHz VCO, 98.75 MHz Q12  */
#define ICS844S2416_DIV_80	0x20	/* 2000 MHz VCO, 100 MHz Q12    */
#define ICS844S2416_DIV_81	0x24	/* 2025 MHz VCO, 101.25 MHz Q12 */
#define ICS844S2416_DIV_82	0x28	/* 2050 MHz VCO, 102.5 MHz Q12  */
#define ICS844S2416_DIV_83	0x2C	/* 2075 MHz VCO, 103.75 MHz Q12 */
#define ICS844S2416_DIV_84	0x30	/* 2100 MHz VCO, 105 MHz Q12    */
#define ICS844S2416_DIV_85	0x34	/* 2125 MHz VCO, 106.25 MHz Q12 */
#define ICS844S2416_DIV_86	0x38	/* 2150 MHz VCO, 107.5 MHz Q12  */
#define ICS844S2416_DIV_87	0x3C	/* 2175 MHz VCO, 108.75 MHz Q12 */
#define ICS844S2416_DIV_88	0x40	/* 2200 MHz VCO, 110 MHz Q12    */

#define ICS844S2416_DB2_SSC	0x02	/* PLL2 SSC - Down spread - 0.5% */
#define ICS844S2416_DB2_BYPASS	0x01	/* BYPASS - Reference Clock (XTAL)*/

/* Defines for Data Byte 3 */
#define ICS844S2416_DB3_Q12_100	0x80	/* Q12 Frequency - 100 MHz */
#define ICS844S2416_DB3_REV_C	0x40	/* Revision C and later */
#define ICS844S2416_DB3_UNUSE_BITS_MSK   0x3f /* Byte 3 reserved bits */

/* Functions prototype */
extern void ics844s2416i_dev_create(dev_object_t *, dev_error_report_t);


#endif /* __ICS844S2416I_H__ */

/*------------------------------------------------------------------
$Log: dev_idt_ics844s2416i.h,v $
Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
