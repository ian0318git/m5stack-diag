/* $Id: platform_pwr_seq.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pwr_seq.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_pwr_seq.h
 *
 * Description: Informers Power Sequencer. This file is  based on EDCS-618748.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PWR_SEQ_H__
#define __PLATFORM_PWR_SEQ_H__

#include "dev_csco_10698.h"

/* Common defines */
#define OVLD_PWR_CONSUMP_CONST   614

#define PWR_SEQ_RTC_TEST_TIME	10	/* Run Time Counter wait time */
#define PWR_SEQ_STAT_CHECK_FAIL	(FAILED + 1)
#define PWR_SEQ_RUNTIME_RETRY	3	/* Wraparound retry count */
#define PWR_SEQ_BUF_SIZE 32

typedef struct dev_pwr_seq_t_ {
    ren_t rev;                   /* 0x00 - Firmware Revision */
    ren_t stat_s;                /* 0x01 - Status. Sticky */
    ren_t stat_c;                /* 0x02 - Status. Clearable */
    ren_t v_fault_s;             /* 0x03 - Voltage Fault. Sticky */
    ren_t v_fault_c;             /* 0x04 - Voltage Fault. Clearable*/
    ren_t rtc_2;                 /* 0x05 - Run Time Counter Word 2 (MS) */
    ren_t rtc_1;                 /* 0x06 - Run Time Counter Word 1 */
    ren_t rtc_0;                 /* 0x07 - Run Time Counter Word 0 (LS) */
    ren_t rt_12v;                /* 0x08 - 12 Volts, Real Time */
    ren_t max_12v;               /* 0x09 - 12 Volts, Maximum */
    ren_t min_12v;               /* 0x0A - 12 Volts, Minimum */
    ren_t rt_5v;                 /* 0x0B - 5.0 Volts, Real Time */
    ren_t max_5v;                /* 0x0C - 5.0 Volts, Maximum */
    ren_t min_5v;                /* 0x0D - 5.0 Volts, Minimum */
    ren_t rt_3_3v;               /* 0x0E - 3.3 Volts, Real Time1 */
    ren_t max_3_3v;              /* 0x0F - 3.3 Volts, Maximum */
    ren_t min_3_3v;              /* 0x10 - 3.3 Volts, Minimum */
    ren_t rt_1_8v_pch;           /* 0x11 - 1.8V_PCH Volts, Real Time */
    ren_t max_1_8v_pch;          /* 0x12 - 1.8V_PCH Volts, Maximum */
    ren_t min_1_8v_pch;          /* 0x13 - 1.8V_PCH, Minimum */
    ren_t rt_1_5v_pch;           /* 0x14 - 1.5V_PCH Volts, Real Time */
    ren_t max_1_5v_pch;          /* 0x15 - 1.5V_PCH Volts, Maximum */
    ren_t min_1_5v_pch;          /* 0x16 - 1.5V_PCH, Minimum */
    ren_t rt_12v_cur;            /* 0x17 - 12 Volts, Current Real Time */
    ren_t max_12v_cur;           /* 0x18 - 12 Volts, Current Maximum */
    ren_t min_12v_cur;           /* 0x19 - 12 Volts, Current Minimum */
    ren_t rt_1_5v_ccpu;          /* 0x1A - 1.5V_CCPU, Real Time */
    ren_t max_1_5v_ccpu;         /* 0x1B - 1.5V_CCPU, Maximum */
    ren_t min_1_5v_ccpu;         /* 0x1C - 1.5V_CCPU, Minimum */
    ren_t rt_1_5v_icpu;          /* 0x1D - 1.5V_ICPU, Real Time */
    ren_t max_1_5v_icpu;         /* 0x1E - 1.5V_ICPU, Maximum */
    ren_t min_1_5v_icpu;         /* 0x1F - 1.5V_ICPU, Minimum */
    ren_t rt_1_05v;              /* 0x20 - 1.05 Volts, Real Time1 */
    ren_t max_1_05v;             /* 0x21 - 1.05 Volts, Maximum */
    ren_t min_1_05v;             /* 0x22 - 1.05 Volts, Minimum */
    ren_t rt_1_0v_pch;           /* 0x23 - 1.0V_PCH Volts, Real Time */
    ren_t max_1_0v_pch;          /* 0x24 - 1.0V_PCH Volts, Maximum */
    ren_t min_1_0v_pch;          /* 0x25 - 1.0V_PCH, Minimum */
    ren_t scratchpad0;           /* 0x26 - Scratch Pad 0 */
    ren_t scratchpad1;           /* 0x27 - Scratch Pad 1 */
    ren_t scratchpad2;           /* 0x28 - Scratch Pad 2 */
    ren_t scratchpad3;           /* 0x29 - Scratch Pad 3 */
    ren_t fault_det;             /* 0x2A - Disable Voltage Fault Detection */
    ren_t d_sampling;            /* 0x2B - Enable Double Sampling */
    ren_t en_wdg;                /* 0x2C - Enable System Watchdog Timer */
    ren_t wdg_ref;               /* 0x2D - Watchdog Refresh */
    ren_t wdt_to;                /* 0x2E - Watchdog Timeout Value */
    ren_t pwr_dwn_en;            /* 0x2F - Power Down Enable Register */
    ren_t pwr_dwn_time;          /* 0x30 - Power Down Count Register */
    ren_t ramp_t_12v;            /* 0x31 - 12 Volts Ramp time */
    ren_t ramp_t_3_3v;           /* 0x32 - 3.3 Volts Ramp time */
    ren_t ramp_t_5_0v;           /* 0x33 - 5.0 Volts Ramp time */
    ren_t ramp_t_1_8v;           /* 0x34 - 1.8V_PCH  Ramp time */
    ren_t ramp_t_1_5v;           /* 0x35 - 1.5V_PCH Ramp time */
    ren_t ramp_t_1_5v_ccpu;      /* 0x36 - 1.5V_CCPU Ramp time */
    ren_t ramp_t_1_5v_icpu;      /* 0x37 - 1.5V_ICPU Ramp time */
    ren_t ramp_t_1_05v;          /* 0x38 - 1.05V Ramp time */
    ren_t ramp_t_1_0v_pch;       /* 0x39 - 1.0V_PCH Ramp time */
    ren_t pwr_up_loops_num;      /* 0x3A - Number of Power up Loops */
    ren_t pwr_up_flow;           /* 0x3B - Power up Flow */
    ren_t max_12v_pwr_up_c;      /* 0x3C - Max. 12V Power Up Current */
    ren_t pwr_up_curr_cnt;       /* 0x3D - Power up Current Count */
    ren_t pwr_up_1st_vtg_l;      /* 0x3E - Power up 1st Voltage level */
    ren_t pwr_up_vtg_cnt;        /* 0x3F - Power up Voltage Count */
    ren_t pwr_up_loops_num_s;    /* 0x40 - Num of Power up Loops - Saved */
    ren_t pwr_up_flow_s;         /* 0x41 - Power up Flow - Saved */
    ren_t max_12v_pwr_curr_s;    /* 0x42 - Max. 12V Power up Current - Saved */
    ren_t pwr_up_curr_cnt_s;     /* 0x43 - Power up Current Count - Saved */
    ren_t pwr_up_1st_vtg_l_s;    /* 0x44 - Power up 1st Voltage Level - Saved */
    ren_t pwr_up_vtg_cnt_s;      /* 0x45 - Power up Voltage Count - Saved */
    ren_t pwr_led_ctrl;          /* 0x46 - Power LED Control Reg. */
    ren_t pwr_cyc_req_cnt;       /* 0x47 - Power Cycle Request Count Reg. */
    ren_t pwe_ctrl;              /* 0x48 - Power Control Reg. */
} dev_pwr_seq_t;

/*
 * Voltage test parameter struct
 */
typedef struct pwr_seq_v_t_ {
    char *volt_p;               /* Voltage text pointer */
    ren_t over_v;               /* Over  Voltage ADC */
    ren_t under_v;              /* Under Voltage ADC */
    ren_o rt_o;                 /* Real time register offset */
    ren_o max_o;                /* Maximum register offset */
    ren_o min_o;                /* Minimum register offset */
} pwr_seq_v_t;

/* 
 * Registers offset defines 
 * Update from Overlord_Power_Sequencer_Spec
 * Changed register:
 * 0x11 - 0x16, 0x1A - 0x29, 0x33 - 0x37, 0x44 - 0x47
 */
#define PWR_SEQ_REV                   0x00
#define PWR_SEQ_STA_S                 0x01
#define PWR_SEQ_STA_C                 0x02
#define PWR_SEQ_V_F_S                 0x03
#define PWR_SEQ_V_F_C                 0x04
#define PWR_SEQ_RTC_2                 0x05
#define PWR_SEQ_RTC_1                 0x06
#define PWR_SEQ_RTC_0                 0x07
#define PWR_SEQ_12_RT                 0x08
#define PWR_SEQ_12_MAX                0x09
#define PWR_SEQ_12_MIN                0x0A
#define PWR_SEQ_5_RT                  0x0B
#define PWR_SEQ_5_MAX                 0x0C
#define PWR_SEQ_5_MIN                 0x0D
#define PWR_SEQ_3_3_RT                0x0E
#define PWR_SEQ_3_3_MAX               0x0F    
#define PWR_SEQ_3_3_MIN               0x10
#define PWR_SEQ_PCHV_1_8_RT           0x11
#define PWR_SEQ_PCHV_1_8_MAX          0x12
#define PWR_SEQ_PCH_1_8_MIN           0x13
#define PWR_SEQ_PCH_1_5_RT            0x14
#define PWR_SEQ_PCH_1_5_MAX           0x15
#define PWR_SEQ_PCH_1_5_MIN           0x16
#define PWR_SEQ_12_C_RT               0x17
#define PWR_SEQ_12_C_MX               0x18
#define PWR_SEQ_12_C_MN               0x19
#define PWR_SEQ_CC_1_5_RT             0x1A
#define PWR_SEQ_CC_1_5_MAX            0x1B
#define PWR_SEQ_CC_1_5_MIN            0x1C
#define PWR_SEQ_IC_1_5_RT             0x1D
#define PWR_SEQ_IC_1_5_MAX            0x1E
#define PWR_SEQ_IC_1_5_MIN            0x1F
#define PWR_SEQ_1_05_RT               0x20
#define PWR_SEQ_1_05_MAX              0x21
#define PWR_SEQ_1_05_MIN              0x22
#define PWR_SEQ_PCH_1_RT              0x23
#define PWR_SEQ_PCH_1_MAX             0x24
#define PWR_SEQ_PCH_1_MIN             0x25
#define PWR_SEQ_SCR0                  0x26
#define PWR_SEQ_SCR1                  0x27
#define PWR_SEQ_SCR2                  0x28
#define PWR_SEQ_SCR3                  0x29
#define PWR_SEQ_F_DET                 0x2A
#define PWR_SEQ_D_SAMP                0x2B
#define PWR_SEQ_WDG_EN                0x2C
#define PWR_SEQ_WDG_RF                0x2D
#define PWR_SEQ_WDG_TO                0x2E
#define PWR_SEQ_PD_EN                 0x2F
#define PWR_SEQ_PD_TIMER              0x30
#define PWR_SEQ_12_RPT                0x31
#define PWR_SEQ_3P3_RPT               0x32
#define PWR_SEQ_5_RPT                 0x33
#define PWR_SEQ_PCH_1P8_RPT           0x34
#define PWR_SEQ_PCH_1P5_RPT           0x35  
#define PWR_SEQ_CC_1P5_RPT            0x36
#define PWR_SEQ_IC_1P5_RPT            0x37
#define PWR_SEQ_1P05_V_RPT            0x38 
#define PWR_SEQ_PCH_1_V_RPT           0x39
#define PWR_SEQ_NUM_PWR_UP_LP         0x3A
#define PWR_SEQ_PWR_UP_FLOW           0x3B
#define PWR_SEQ_MX_12V_PWR_CUR        0x3C
#define PWR_SEQ_PWR_UP_CUR_CNT        0x3D 
#define PWR_SEQ_PWR_UP_1ST_VTG_L      0x3E
#define PWR_SEQ_PWR_UP_VTG_CNT        0x3F
#define PWR_SEQ_NUM_PWR_UP_LP_S       0x40
#define PWR_SEQ_PWR_UP_FLOW_S         0x41
#define PWR_SEQ_MX_12V_PWR_CUR_S      0x42
#define PWR_SEQ_PWR_UP_CUR_CNT_S      0x43
#define PWR_SEQ_PWR_UP_1ST_VTG_L_S    0x44
#define PWR_SEQ_PWR_UP_VTG_CNT_S      0x45
#define PWR_SEQ_PWR_LED_CTRL          0x46
#define PWR_SEQ_PWR_CYC_REQ_CNT       0x47
#define PWR_SEQ_PWR_CTRL              0x48


/* Firmware Revision - 0x00 */
#define PS_REV_MAJOR_MSK     0xFF00 /* Major Revision */
#define PS_REV_MAJOR_SHIFT        8 /* Major Revision Shift */
#define PS_REV_MINOR_MSK     0x00FF /* Minor Revision */

/* Status Registers, Sticky/Clearable - 0x01/0x02 */
#define PS_STAT_SYS_DWN_VIA_SWITCH_MSK   0x4000 /* System powered down via Switch */
#define PS_STAT_DIS_12V_S_CHK_MSK        0x2000 /* 12V short checking disabled */
#define PS_STAT_PWR_UP_DELAY_MSK         0x1000 /* Power up Delayed */
#define PS_STAT_WDOG_TO_CNTR_MSK         0x0C00 /* Watch Dog Time out Counter */
#define PS_STAT_WDOG_TO_CNTR_SHIFT           10 /* Watch Dog Time out Counter Shift */
#define PS_STAT_12V_SHORT_DET_MSK        0x0300 /* 12V short circuit detect during pwoer up */
#define PS_STAT_12V_SHORT_DET_SHIFT           8 /* 12v short circuit detect during power up  Shift */
#define PS_STAT_VTG_FAULT_CNTR_MSK       0x00C0 /* Voltage Fault Counter */
#define PS_STAT_VTG_FAULT_CNTR_SHIFT          6 /* Voltage Fault Counter Shift */
#define PS_STAT_VTG_FAULT_PU_MSK         0x0020 /* Voltage Fault During Power Up */
#define PS_STAT_VTG_FAULT_OPT_MSK        0x0010	/* Voltage Fault During Operation */
#define PS_STAT_SYS_PWR_DWN_MSK          0x0008	/* System Power Down */
#define PS_STAT_WDOG_TO_MSK              0x0006 /* Watchdog Timeout */
#define PS_STAT_WDOG_TO_SHIFT                 1 /* Watchdog Timeout Shift */
#define PS_STAT_SYS_PWR_CYC_MSK          0x0001 /* System Power cycle by FPGA */

/* Voltage Fault Registers, Sticky/Clearable - 0x03/0x04 */
#define PS_VF_1_0V_PCH_FAULT_MSK         0x0100 /* 1.0V PCH Fault */
#define PS_VF_1_05V_FAULT_MSK            0x0080 /* 1.05V Fault */
#define PS_VF_1_5V_ICPU_FAULT_MSK        0x0040 /* 1.5V ICPU Fault */
#define PS_VF_1_5V_CCPU_FAULT_MSK        0x0020 /* 1.5V CCPU Fault */
#define PS_VF_1_5V_PCH_FAULT_MSK         0x0010 /* 1.5V PCH Fault */
#define PS_VF_1_8V_PCH_FAULT_MSK         0x0008 /* 1.8V PCH Fault */
#define PS_VF_3_3V_FAULT_MSK             0x0004 /* 3.3V Fault */
#define PS_VF_5V_FAULT_MSK               0x0002 /* 5V Fault */
#define PS_VF_12V_FAULT_MSK              0x0001 /* 12V Fault */

/* Run Time Counter - 0x05-07 */
#define PS_RTC_MSK   0xFFFF /* Real Time Counter */

/* Voltage Registers (12 Volts, 5.0 Volts, 3.3 Volts, 1.8 Volts PCH, 1.5 Volts PCH
 * 1.5 Volts CCPU, 1.5 Volts ICPU, 1.05 Volts , 1.0 Volts PCH) 
 * Real Time/Maximum/Minimum - 0x08-25
 */
#define PS_VC_ADC_MSK   0x03FF /* ADC Reading */

/* Scratch Pad Register - 0x26-29 */
#define PS_SCR_PAD_MSK  0xFFFF /* Scratch Pad */

/* Disable Voltage Fault Detection Register - 0x2A */
#define PS_DVFD_DIS_VTG_SD_KEY_MSK     0xFE00 /* Disable Voltage Shutdown Key */
#define PS_DVFD_DIS_VTG_SD_KEY_SHIFT        9 /* Disable Voltage Shutdown Key Shift */
#define PS_DVFD_DIS_1_0V_PCH_SD_MSK    0x0100 /* Disable 1.0V PCH Shutdown */
#define PS_DVFD_DIS_1_05V_SD_MSK       0x0080 /* Disable 1.05V Shutdown */
#define PS_DVFD_DIS_1_5V_ICPU_SD_MSK   0x0040 /* Disable 1.5V ICPU Shutdown */
#define PS_DVFD_DIS_1_5V_CCPU_SD_MSK   0x0020 /* Disable 1.5V CCPU Shutdown */
#define PS_DVFD_DIS_1_5V_PCH_SD_MSK    0x0010 /* Disable 1.5V PCH Shutdown */
#define PS_DVFD_DIS_1_8V_PCH_SD_MSK    0x0008 /* Disable 1.8V PCH Shutdown */
#define PS_DVFD_DIS_3_3V_SD_MSK        0x0004 /* Disable 3.3V Shutdown */
#define PS_DVFD_DIS_5V_SD_MSK          0x0002 /* Disable 5V Shutdown */
#define PS_DVFD_DIS_12V_SD_MSK         0x0001 /* Disable 12V Shutdown */

/* Enable Double Sampling Regisgter - 0x2B */
#define PS_EDS_EN_DS_KEY_MSK      0xFE00 /* Enable Double Sampling Key */
#define PS_EDS_EN_DS_KEY_SHIFT         9 /* Enable Double Sampling Key Shift */
#define PS_EDS_1_0V_PCH_DS_MSK    0x0100 /* 1.0V PCH core Double Sampling */
#define PS_EDS_1_05V_DS_MSK       0x0080 /* 1.05V Double Sampling */
#define PS_EDS_1_5V_ICPU_DS_MSK   0x0040 /* 1.5V ICPU Double Sampling */
#define PS_EDS_1_5V_CCPU_DS_MSK   0x0020 /* 1.5V CCPU Double Sampling */
#define PS_EDS_1_5V_PCH_DS_MSK    0x0010 /* 1.5V PCH Double Sampling */
#define PS_EDS_1_8V_PCH_DS_MSK    0x0008 /* 1.8V PCH Double Sampling */
#define PS_EDS_3_3V_DS_MSK        0x0004 /* 3.3V Double Sampling */
#define PS_EDS_5V_DS_MSK          0x0002 /* 5V Double Sampling */
#define PS_EDS_12V_DS_MSK         0x0001 /* 12V Double Sampling */

/* Watchdog Enable Register - 0x2C */
#define PS_WDE_EN_WDOG_KEY_MSK       0x01FF /* Enable Watchdog Key */

/* Watchdog Refresh Register - 0x2D */
#define PS_WDR_WDOG_REFRESH_MSK      0xFFFF /* Watchdog Refresh */

/* Watchdog Timeout Value Register - 0x2E */
#define PS_WDTO_WDOG_TO_MSK          0xFFFF /* Watchdog Timeout Value in seconds */

/* Power Down Enable Register - 0x2F */
#define PS_PDE_EN_PWR_DWN_MSK        0xFFFF /* Power Down Enable */

/* Power Down Time Value Register - 0x30 */
#define PS_PDT_PWR_DWN_TIME_MSK      0xFFFF /* Power Down Time Value in seconds */

/* Voltage Ramp Time Registers - 0x31 to 0x39 */
#define PS_VRT_MAX_WAIT_MSK          0xFF00 /* Max wait time for Power up */
#define PS_VRT_MAX_WAIT_SHIFT             8 /* Max wait time for Power up Shift */
#define PS_VRT_TIME_TO_PWR_UP_MSK    0x00FF /* Time to Power up */

/* Number of Power Up Loops Register - 0x3A, 0x40 */
#define PS_NUM_12V_CHK_LP_MSK        0xFFFF /* Number of 12V turned on */

/* Power Up Flow Register - 0x3B, 0x41 */
#define PS_SHORT_CHK_LP_CNTR_MSK     0xFF00 /* Short check loop count */
#define PS_SHORT_CHK_LP_CNTR_SHIFT        8 /* Short check loop count shift */
#define PS_DET_HI_CUR_VTG_2_MSK      0x0008 /* Detect High Current & Voltage (Second) */
#define PS_DET_HI_CUR_VTG_1_MSK      0x0004 /* Detect High Current & Voltage (First) */
#define PS_DET_HI_CUR_NO_VTG_MSK     0x0002 /* Detect High Current & No Voltage */
#define PS_DET_HI_CUR_MSK            0x0001 /* Detect High Current */

/* Max. 12V Power Up Current Register - 0x3C, 0x42 */
#define PS_MAX_12V_PU_CUR_MSK        0x03FF /* Max. 12V Power up Current */

/* Power up Current Count Register - 0x3D, 0x43 */
#define PS_PWR_UP_CUR_CNT_MSK        0x00FF /* Power up Current Count */

/* Power Up First Voltage Level Register - 0x3E, 0x44 */
#define PS_PU_FST_VTG_MSK            0x03FF /* Power up first Voltage Level */

/* Power up Voltage Count Register - 0x3F, 0x45 */
#define PS_PWR_UP_VTG_CNT_MSK        0xFFFF /* Power up Votage Count */

/* Power LED Control Register - 0x46 */
#define PS_PLC_YELLOW_TIME_MSK       0xFE00 /* Yellow LED ON/OFF Time (ms) */
#define PS_PLC_YELLOW_TIME_SHIFT          9 /* Yellow LED ON/OFF Time (ms) shift */
#define PS_PLC_GREEN_TIME_MSK        0x01FC /* Green LED ON/OFF Time (ms) */
#define PS_PLC_GREEN_TIME_SHIFT           2 /* Yellow LED ON/OFF Time (ms) shift */
#define PS_PLC_YELLOW_CTRL_MSK       0x0002 /* Yellow LED Control */
#define PS_PLC_GREEN_CTRL_MSK        0x0001 /* Green LED Control */

/* Power Cycle Request Count Register - 0x47 */
#define PS_PCRC_PWR_CYC_REQ_CNT_MSK  0xFFFF /* Power Cycle Request Count */

/* Power Control Register - 0x48 */
#define PS_PC_POE_PWR_CTRL           0x0010 /* bit[4]: PoE Power Control Bit */
#define PS_PC_POE_PWR_CTRL_SHIFT          4 /* PoE Power Control Bit shift */
#define PS_PC_GSHUT_DWN_TIMER_MSK    0x000F /* bit[3:0]: Graceful Shutdown Timer */


/* Update from Overlord_Power_Sequencer_Spec 
 * Voltage Tests defines. These values are derived from HFS (EDCS-618748) Rev.
 * 4.0, Table 6 and Table 7.
 * Numbers in comments are: Voltage - Over/Under Voltage - 7% Over/Under Voltage.
 * According to HFS, 7% over/under will be flagged as error
 */
#define PWR_12V_OVER		0x0335	/* 12.15 - 13.97 - 13.00 */
#define PWR_12V_UNDER		0x0238	/* 12.15 - 8.99  - 8.99  */
#define PWR_5V_OVER		  0x031F	/* 5.15  - 5.92  - 5.51  */
#define PWR_5V_UNDER		0x02B7	/* 5.15  - 4.37  - 4.79  */
#define PWR_3P3V_OVER		0x033A	/* 3.30  - 3.79  - 3.53  */
#define PWR_3P3V_UNDER	0x02CE	/* 3.30  - 2.80  - 3.07  */

#define PWR_1P8V_PCH_OVER		0x02DA	/* 1.80  - 2.07  - 1.93  */
#define PWR_1P8V_PCH_UNDER	0x027A	/* 1.80  - 1.53  - 1.67  */
#define PWR_1P5V_PCH_OVER	  0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_PCH_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P5V_ICPU_OVER	0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_ICPU_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P5V_CCPU_OVER	0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_CCPU_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P05V_OVER	    0x01A9	/* 1.05  - 1.21  - 1.12  */
#define PWR_1P05V_UNDER	    0x0172	/* 1.05  - 0.8925- 0.98  */
#define PWR_1P0V_PCH_OVER	  0x0196	/* 1.00  - 1.15  - 1.07  */
#define PWR_1P0V_PCH_UNDER	0x0160	/* 1.00  - 0.85  - 0.93  */


/* Functions prototype */
extern int init_pwr_seq(int err_log);
extern void build_pwr_tst_menu(int submenu);

#endif /* __PLATFORM_PWR_SEQ_H__ */

/*------------------------------------------------------------------
$Log: platform_pwr_seq.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.6  2012/07/19 07:02:14  palin2
Update Register 0x48, Power Control Register.

Revision 1.5  2012/06/26 12:18:42  palin2
Support to show the Power consumption of Overlord motherboard.

Revision 1.4  2012/06/26 03:59:45  palin2
Update registers map.

Revision 1.3  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
