/* $Id: platform_vtg_mntr.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_vtg_mntr.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_vtg_mntr.h
 *
 * Description: Operation-Overlord Voltage Monitor. 
 *              This file is based on EDCS-618748.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_VTG_MNTR_H__
#define __PLATFORM_VTG_MNTR_H__

#include "dev_csco_10698.h"

/* Common defines */
#define VTG_MNTR_RTC_TEST_TIME	10	/* Run Time Counter wait time */
#define VTG_MNTR_STAT_CHECK_FAIL	(FAILED + 1)
#define VTG_MNTR_RUNTIME_RETRY	3	/* Wraparound retry count */
#define VTG_MNTR_BUF_SIZE 32


typedef struct dev_vtg_mntr_t_ {
    ren_t rev;			/* 0x00 Firmware Revision */
    ren_t stat_s;		/* 0x01 Status. Sticky */
    ren_t stat_c;		/* 0x02 Status. Clearable */
    ren_t v_fault_s;		/* 0x03 Voltage Fault. Sticky */
    ren_t v_fault_c;		/* 0x04 Voltage Fault. Clearable*/
    ren_t rtc_2;		/* 0x05 Run Time Counter Word 2 (MS) */
    ren_t rtc_1;		/* 0x06 Run Time Counter Word 1 */
    ren_t rtc_0;		/* 0x07 Run Time Counter Word 0 (LS) */
    ren_t rt_3v;		/* 0x08 3.0 Volts, Real Time */
    ren_t max_3v;		/* 0x09 3.0 Volts, Maximum */
    ren_t min_3v;		/* 0x0A 3.0 Volts, Minimum */
    ren_t rt_2_5v;		/* 0x0B 2.5 Volts, Real Time */
    ren_t max_2_5v;		/* 0x0C 2.5 Volts, Maximum */
    ren_t min_2_5v;		/* 0x0D 2.5 Volts, Minimum */
    ren_t rt_1_8v;		/* 0x0E 1.8 Volts, Real Time */
    ren_t max_1_8v;		/* 0x0F 1.8 Volts, Maximum */
    ren_t min_1_8v;		/* 0x10 1.8 Volts, Minimum */
    ren_t rt_tx_1_5v;		/* 0x11 1.5V_TX_QLM, Real Time */
    ren_t max_tx_1_5v;		/* 0x12 1.5V_TX_QLM, Maximum */
    ren_t min_tx_1_5v;		/* 0x13 1.5V_TX_QLM, Minimum */
    ren_t rt_1_2v;		/* 0x14 1.2 Volts, Real Time */
    ren_t max_1_2v;		/* 0x15 1.2 Volts, Maximum */
    ren_t min_1_2v;		/* 0x16 1.2 Volts, Minimum */
    ren_t rt_1_15v;		/* 0x17 1.15 Volts, Real Time */
    ren_t max_1_15v;		/* 0x18 1.15 Volts, Maximum */
    ren_t min_1_15v;		/* 0x19 1.15 Volts, Minimum */
    ren_t rt_1_1v;		/* 0x1A 1.1 Volts, Real Time */
    ren_t max_1_1v;		/* 0x1B 1.1 Volts, Maximum */
    ren_t min_1_1v;		/* 0x1C 1.1 Volts, Minimum */
    ren_t rt_1_xxv;		/* 0x1D 1.xx_VCORE_ICPU, Real Time */
    ren_t max_1_xxv;		/* 0x1E 1.xx_VCORE_ICPU, Maximum */
    ren_t min_1_xxv;		/* 0x1F 1.xx_VCORE_ICPU, Minimum */
    ren_t rt_1_0v;		/* 0x20 1.0 Volts, Real Time */
    ren_t max_1_0v;		/* 0x21 1.0 Volts, Maximum */
    ren_t min_1_0v;		/* 0x22 1.0 Volts, Minimum */
    ren_t rt_sa_v;		/* 0x23 VTT_SA_ICPU, Real Time */
    ren_t max_sa_v;		/* 0x24 VTT_SA_ICPU, Maximum */
    ren_t min_sa_v;		/* 0x25 VTT_SA_ICPU, Minimum */
    ren_t rt_i_0_75v;		/* 0x26 0.75V_VTT_ICPU, Real Time */
    ren_t max_i_0_75v;		/* 0x27 0.75V_VTT_ICPU, Maximum */
    ren_t min_i_0_75v;		/* 0x28 0.75V_VTT_ICPU, Minimum */
    ren_t rt_c_0_75v;		/* 0x29 0.75V_VTT_CCPU, Real Time */
    ren_t max_c_0_75v;		/* 0x2A 0.75V_VTT_CCPU, Maximum */
    ren_t min_c_0_75v;		/* 0x2B 0.75V_VTT_CCPU, Minimum */
    ren_t ramp_t_3_0v;		/* 0x2C 3.0 Volts Ramp time */
    ren_t ramp_t_2_5v;		/* 0x2D 2.5 Volts Ramp time */
    ren_t ramp_t_1_8v;		/* 0x2E 1.8 Volts Ramp time */
    ren_t ramp_tx_1_5v;		/* 0x2F 1.5V_TX_QLM Ramp time */
    ren_t ramp_t_1_2v;		/* 0x30 1.2 Volts Ramp time */
    ren_t ramp_t_1_15v;		/* 0x31 1.15 Volts Ramp time */
    ren_t ramp_t_1_1v;		/* 0x32 1.1 Volts Ramp time */
    ren_t ramp_t_1_xxv;		/* 0x33 1.xx_VCORE_ICPU Ramp time */
    ren_t ramp_t_1_0v;		/* 0x34 1.0 Volts Ramp time */
    ren_t ramp_t_sa_v;		/* 0x35 VTT_SA_ICPU Ramp time */
    ren_t ramp_t_i_0_75v;	/* 0x36 0.75V_VTT_ICPU Ramp time */
    ren_t ramp_t_c_0_75v;	/* 0x37 0.75V_VTT_CCPU Ramp time */
    ren_t d_sampling;		/* 0x38 Enable Double Sampling */
    ren_t margin;		/* 0x39 Voltage Margin Control */
    ren_t scratchpad0;		/* 0x3A Scratch Pad 0 */
    ren_t scratchpad1;		/* 0x3B Scratch Pad 1 */
    ren_t scratchpad2;		/* 0x3C Scratch Pad 2 */
    ren_t scratchpad3;		/* 0x3D Scratch Pad 3 */
    ren_t scratchpad4;		/* 0x3E Scratch Pad 4 */
    ren_t scratchpad5;		/* 0x3F Scratch Pad 5 */
    ren_t reserved0;		/* 0x40 Reserved */
    ren_t reserved1;		/* 0x41 Reserved */
    ren_t reserved2;		/* 0x42 Reserved */
    ren_t reserved3;		/* 0x43 Reserved */
    ren_t reserved4;		/* 0x44 Reserved */
    ren_t reserved5;		/* 0x45 Reserved */
    ren_t reserved6;		/* 0x46 Reserved */
} dev_vtg_mntr_t;


/*
 * Voltage test parameter struct
 */
typedef struct vtg_mntr_v_t_ {
    char *volt_p;		/* Voltage text pointer */
    ren_t over_v;		/* Over  Voltage ADC */
    ren_t under_v;		/* Under Voltage ADC */
    ren_o rt_o;			/* Real time register offset */
    ren_o max_o;		/* Maximum register offset */
    ren_o min_o;		/* Minimum register offset */
} vtg_mntr_v_t;


/* Registers offset defines */
#define VTG_MNTR_REV          0x00
#define VTG_MNTR_STA_S        0x01
#define VTG_MNTR_STA_C        0x02
#define VTG_MNTR_V_F_S        0x03
#define VTG_MNTR_V_F_C        0x04
#define VTG_MNTR_RTC_2        0x05
#define VTG_MNTR_RTC_1        0x06
#define VTG_MNTR_RTC_0        0x07
#define VTG_MNTR_3_0_RT       0x08
#define VTG_MNTR_3_0_MAX      0x09
#define VTG_MNTR_3_0_MIN      0x0A
#define VTG_MNTR_2_5_RT	      0x0B
#define VTG_MNTR_2_5_MAX      0x0C
#define VTG_MNTR_2_5_MIN      0x0D
#define VTG_MNTR_1_8_RT	      0x0E
#define VTG_MNTR_1_8_MAX      0x0F
#define VTG_MNTR_1_8_MIN      0x10
#define VTG_MNTR_1_5_TX_RT    0x11
#define VTG_MNTR_1_5_TX_MAX   0x12
#define VTG_MNTR_1_5_TX_MIN   0x13
#define VTG_MNTR_1_2_RT       0x14
#define VTG_MNTR_1_2_MAX      0x15
#define VTG_MNTR_1_2_MIN      0x16
#define VTG_MNTR_1_15_RT      0x17
#define VTG_MNTR_1_15_MAX     0x18
#define VTG_MNTR_1_15_MIN     0x19
#define VTG_MNTR_1_1_RT       0x1A
#define VTG_MNTR_1_1_MAX      0x1B
#define VTG_MNTR_1_1_MIN      0x1C
#define VTG_MNTR_1_XX_RT      0x1D
#define VTG_MNTR_1_XX_MAX     0x1E
#define VTG_MNTR_1_XX_MIN     0x1F
#define VTG_MNTR_1_0_RT       0x20
#define VTG_MNTR_1_0_MAX      0x21
#define VTG_MNTR_1_0_MIN      0x22
#define VTG_MNTR_SA_RT        0x23
#define VTG_MNTR_SA_MAX       0x24
#define VTG_MNTR_SA_MIN       0x25
#define VTG_MNTR_0_75_I_RT    0x26
#define VTG_MNTR_0_75_I_MAX   0x27
#define VTG_MNTR_0_75_I_MIN   0x28
#define VTG_MNTR_0_75_C_RT    0x29
#define VTG_MNTR_0_75_C_MAX   0x2A
#define VTG_MNTR_0_75_C_MIN   0x2B
#define VTG_MNTR_3_0_RPT      0x2C
#define VTG_MNTR_2_5_RPT      0x2D
#define VTG_MNTR_1_8_RPT      0x2E
#define VTG_MNTR_1_5_TX_RPT   0x2F
#define VTG_MNTR_1_2_RPT      0x30
#define VTG_MNTR_1_15_RPT     0x31
#define VTG_MNTR_1_1_RPT      0x32
#define VTG_MNTR_1_XX_RPT     0x33
#define VTG_MNTR_1_0_RPT      0x34
#define VTG_MNTR_SA_RPT       0x35
#define VTG_MNTR_0_75_I_RPT   0x36
#define VTG_MNTR_0_75_C_RPT   0x37
#define VTG_MNTR_EN_DS        0x38
#define VTG_MNTR_MRGN_CTRL    0x39
#define VTG_MNTR_SCR0         0x3A
#define VTG_MNTR_SCR1         0x3B
#define VTG_MNTR_SCR2         0x3C
#define VTG_MNTR_SCR3         0x3D
#define VTG_MNTR_SCR4         0x3E
#define VTG_MNTR_SCR5         0x3F
#define VTG_MNTR_RSV0         0x40
#define VTG_MNTR_RSV1         0x41
#define VTG_MNTR_RSV2         0x42
#define VTG_MNTR_RSV3         0x43
#define VTG_MNTR_RSV4         0x44
#define VTG_MNTR_RSV5         0x45
#define VTG_MNTR_RSV6         0x46


/*
 * Register Definitions
 */
/* Firmware Revision - 0x00 */
#define VTG_MNTR_REV_MAJOR      0xFF00   /* Major Revision */
#define VTG_MNTR_REV_MINOR      0x00FF   /* Minor Revision */

/* Status Registers, Sticky/Clearable - 0x01:0x02 */
#define VTG_MNTR_STAT_V_F_MASK  0x03FC  /* Voltage Fault Count mask */
#define VTG_MNTR_STAT_V_F_SHIFT 2	/* Voltage Fault Count bits shift */
#define VTG_MNTR_STAT_V_F_PU    0x0002  /* Voltage Fault During Power Up */
#define VTG_MNTR_STAT_V_F_OP    0x0001  /* Voltage Fault During Operation */

/* Voltage Fault Registers, Sticky/Clearable - 0x03:0x04 */
#define VTG_MNTR_V_F_0_75_C     0x0800  /* 0.75V VTT CCPU Fault */
#define VTG_MNTR_V_F_0_75_I     0x0400  /* 0.75V VTT ICPU Fault */
#define VTG_MNTR_V_F_SA_I       0x0200  /* VTT SA ICPU Fault */
#define VTG_MNTR_V_F_1_0V       0x0100  /* 1.0V Fault */
#define VTG_MNTR_V_F_1_XXV      0x0080  /* 1.xx VCORE ICPU Fault */
#define VTG_MNTR_V_F_1_1V       0x0040  /* 1.1V Fault */
#define VTG_MNTR_V_F_1_15V      0x0020  /* 1.15V Fault */
#define VTG_MNTR_V_F_1_2V       0x0010  /* 1.2V Fault */
#define VTG_MNTR_V_F_1_5V_TX    0x0008  /* 1.5V TX QLM Fault */
#define VTG_MNTR_V_F_1_8V       0x0004  /* 1.8V Fault */
#define VTG_MNTR_V_F_2_5V       0x0002  /* 2.5V Fault */
#define VTG_MNTR_V_F_3_0V       0x0001  /* 3.0V Fault */

/* Run Time Counter Registers - 0x05:0x07 */
#define VTG_MNTR_RTC_MASK	0xFFFF	/* Real Time Counter Mask */
#define VTG_MNTR_RTC_SHIFT	16	/* Real Time Counter registers size */

/* Voltage & Current Registers - 0x08:0x2B */
#define VTG_MNTR_ADC_READ_MASK	0x03FF  /* ADC Reading Mask */

/* Voltage Ramp Time Register - 0x2C:0x37 */
#define VTG_MNTR_MAX_WAIT_MASK  0xFF00  /* Maximum wait time for power up (ms) */
#define VTG_MNTR_T_PWR_UP_MASK  0x00FF  /* Time to power up (ms) */

/* Enable Double Sampling Regisgter - 0x38 */
#define VTG_MNTR_EN_DS_KEY      0xFE00  /* Enable Double Sampling Key */
#define VTG_MNTR_GFY_SCR_DS     0x0100  /* 1.05 Goofy SCROOGE CORE Double Sampling */
#define VTG_MNTR_1_XXV_CPU_DS	0x0080	/* 1.xxV CPU Core Double Sampling */
#define VTG_MNTR_1_2V_GESW_DS   0x0040  /* 1.2V GE Switch Core Double Sampling */
#define VTG_MNTR_1_8V_CPU_DS    0x0010  /* 1.8V CPU Double Sampling */
#define VTG_MNTR_2_5V_DS        0x0008  /* 2.5V Double Sampling */
#define VTG_MNTR_3_3V_DS        0x0004  /* 3.3V Double Sampling */
#define VTG_MNTR_5V_DS          0x0002  /* 5V Double Sampling */
#define VTG_MNTR_12V_DS         0x0001  /* 12V Double Sampling */

/* Voltage Margin Control Register - 0x39 */
#define VTG_MNTR_DDR_MRGN       0x0008  /* 1.8V/1.5V DDR Margin 3% */
#define VTG_MNTR_DDR_MRGN_EN    0x0004  /* 1.8V/1.5V DDR Margin Enable */
#define VTG_MNTR_3_3V_MRGN      0x0002  /* 3.3V/3.0V Margin 3% */
#define VTG_MNTR_3_3V_MRGN_EN   0x0001  /* 3.3V/3.0V Margin Enable */


/* Voltage Margin enums */
typedef enum {
    VTG_MRGN_GET_DDR = 0,    /* Display 1.8/1.5V DDR Margin */
    VTG_MRGN_GET_3_3V,       /* Display 3.3/3.0V Margin */
    VTG_MRGN_SET_DDR_NORM,   /* Set 1.8/1.5V Margin to normal */
    VTG_MRGN_SET_DDR_HI,     /* Set 1.8/1.5V Margin to high */
    VTG_MRGN_SET_DDR_LO,     /* Set 1.8/1.5V Margin to low */
    VTG_MRGN_SET_3_3V_NORM,  /* Set 3.3V/3.0V Margin to normal */
    VTG_MRGN_SET_3_3V_HI,    /* Set 3.3V/3.0V Margin to high */
    VTG_MRGN_SET_3_3V_LO,    /* Set 3.3V/3.0V Margin to low */
} voltage_margin_t;


/* Voltage Tests Definitions. 
 * These values are derived from Operation-Overlord Volage Monitor HFS(EDCS-xxxx).
 * Based on the document, 7% over/under will be flagged as error.
 */
/* Over Voltage ADC Values */       /* Orig. Volt - Over Volt - 7% Over Volt */
#define OVER_ADC_3_0V      0x02EE   /*    3.0V    -   3.45V   -    3.20V     */   
#define OVER_ADC_2_5V      0x034C   /*    2.5V    -   2.87V   -    2.68V     */
#define OVER_ADC_1_8V      0x02DA   /*    1.8V    -   2.07V   -    1.93V     */
#define OVER_ADC_1_5V      0x0260   /*    1.5V    -   1.73V   -    1.61V     */
#define OVER_ADC_1_2V      0x01E6   /*    1.2V    -   1.38V   -    1.28V     */
#define OVER_ADC_1_15V     0x01D2   /*   1.15V    -   1.32V   -    1.23V     */ 
#define OVER_ADC_1_1V      0x01BE   /*    1.1V    -   1.27V   -    1.18V     */
#define OVER_ADC_VCORE     0x0195   /* VCORE ICPU -   1.15V   -    1.07V     */
#define OVER_ADC_1_0V      0x0195   /*    1.0V    -   1.15V   -    1.07V     */
#define OVER_ADC_SA        0x016D   /*  SA ICPU   -   1.04V   -    0.96V     */
#define OVER_ADC_0_75V_I   0x0130   /* 0.75V ICPU -   0.86V   -    0.80V     */      
#define OVER_ADC_0_75V_C   0x0130   /* 0.75V CCPU -   0.86V   -    0.80V     */

/* Under Voltage ADC Values */      /* Orig. Volt - Under Volt - 7% Under Volt */
#define UNDER_ADC_3_0V     0x028C   /*    3.0V    -   2.55V    -   2.80V     */    
#define UNDER_ADC_2_5V     0x02DD   /*    2.5V    -   2.13V    -   2.32V     */ 
#define UNDER_ADC_1_8V     0x027A   /*    1.8V    -   1.53V    -   1.67V     */ 
#define UNDER_ADC_1_5V     0x0211   /*    1.5V    -   1.28V    -   1.40V     */ 
#define UNDER_ADC_1_2V     0x01A7   /*    1.2V    -   1.02V    -   1.12V     */ 
#define UNDER_ADC_1_15V    0x0195   /*   1.15V    -   0.98V    -   1.07V     */  
#define UNDER_ADC_1_1V     0x0183   /*    1.1V    -   0.94V    -   1.02V     */ 
#define UNDER_ADC_VCORE    0x0160   /* VCORE ICPU -   0.85V    -   0.93V     */  
#define UNDER_ADC_1_0V     0x0160   /*    1.0V    -   0.85V    -   0.93V     */ 
#define UNDER_ADC_SA       0x013D   /*  SA ICPU   -   0.77V    -   0.84V     */
#define UNDER_ADC_0_75V_I  0x0108   /* 0.75V ICPU -   0.64V    -   0.70V     */    
#define UNDER_ADC_0_75V_C  0x0108   /* 0.75V CCPU -   0.64V    -   0.70V     */


/* Functions prototype */
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *);
extern uint32_t err_report(dev_object_t *, char *, uint32_t);
extern void build_vtg_mntr_tst_menu(int);
extern int init_vtg_mntr(int);
extern int vtg_mrgn(int, int);
extern int vtg_get_version(uint16_t *);

#endif /* __PLATFORM_VTG_MNTR_H__ */

/*------------------------------------------------------------------
$Log: platform_vtg_mntr.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:25  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
