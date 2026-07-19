/* $Id: platform_env.h,v 1.1 2013/05/09 05:42:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_env.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_env.h
 *
 * Description: Informers Environmental Control Unit. This file is
 *		based on EDCS-534569.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ENV_H__
#define __PLATFORM_ENV_H__

#ifndef LINUX_KLM
#include "dev_csco_10698.h"
#endif


/* Common defines */
#define NO_OF_FANS	4
#define FAN_MASK	0xF     /* Fan mask for PWM sweep */
#define MAX_PWM		100
#define MIN_PWM		0
#define MAX_RPM		0xFFFF
#define ENV_TEST_VER	0x0300	/* Fully Env tests version supports */
#define ENV_TEST_VER4P4	0x0404	/* Force Fan Alert/Temp Offset tests version */
#define ENV_MCU_BUF_SIZE 32 


typedef struct dev_env_mcu_t_ {
    ren_t rev;			/* 0 - Firmware Revision */
    ren_t stat1;		/* Status 1 */
    ren_t stat2;		/* Status 2 */
    ren_t in1_temp;		/* Inlet 1 Temperature */
    ren_t in2_temp;		/* Inlet 2 Temperature */
    ren_t out1_temp;		/* Outlet 1 Temperature */
    ren_t out2_temp;		/* Outlet 2 Temperature */
    ren_t fan1_tach;		/* Fan 1 Tachometer RPM */
    ren_t fan2_tach;		/* 8 - Fan 2 Tachometer RPM */
    ren_t fan3_tach;		/* Fan 3 Tachometer RPM */
    ren_t fan4_tach;		/* Fan 4 Tachometer RPM */
    ren_t fan5_tach;		/* Fan 5 Tachometer RPM */
    ren_t cur_12v;		/* +12V Current Measurement */
    ren_t max_12v;		/* +12V Maximum Current Measurement */
    ren_t ctrl1;		/* Control 1 */
    ren_t ctrl2;		/* Control 2 */
    ren_t temp1_mid2lo;		/* 10 - Temperature 1 Mid to Low */
    ren_t temp2_lo2mid;		/* Temperature 2 Low to Mid */
    ren_t temp3_hi2mid;		/* Temperature 3 High to Mid */
    ren_t temp4_mid2hi;		/* Temperature 4 Mid to High */
    ren_t fan1_lo;		/* Fan 1 Low Speed */
    ren_t fan1_mid;		/* Fan 1 Mid Speed */
    ren_t fan1_hi;		/* Fan 1 High Speed */
    ren_t fan2_lo;		/* Fan 2 Low Speed */
    ren_t fan2_mid;		/* 18 - Fan 2 Mid Speed */
    ren_t fan2_hi;		/* Fan 2 High Speed */
    ren_t fan3_lo;		/* Fan 3 Low Speed */
    ren_t fan3_mid;		/* Fan 3 Mid Speed */
    ren_t fan3_hi;		/* Fan 3 High Speed */
    ren_t fan4_lo;		/* Fan 4 Low Speed */
    ren_t fan4_mid;		/* Fan 4 Mid Speed */
    ren_t fan4_hi;		/* Fan 4 High Speed */
    ren_t fan5_lo;		/* 20 - Fan 5 Low Speed */
    ren_t fan5_mid;		/* Fan 5 Mid Speed */
    ren_t fan5_hi;		/* Fan 5 High Speed */
    ren_t prop;			/* Proportional Constant */
    ren_t fan_pwm_slope;	/* Fan PWM Slope */
    ren_t hyst_timeout;		/* Fan Hysteresis Timeout */
    ren_t tray_rem_timeout;	/* Fan Tray Removal Timeout */
    ren_t in1_alert;		/* Inlet 1 Alert Temperature */
    ren_t in2_alert;		/* 28 - Inlet 2 Alert Temperature */
    ren_t out1_alert;		/* Outlet 1 Alert Temperature */
    ren_t out2_alert;		/* Outlet 2 Alert Temperature */
    ren_t alert_shut;		/* Alert Shutdown Timer */
    ren_t pwr_seq_reset;	/* Power Sequencer Reset */
    ren_t pwr_seq_rst_to;	/* Power Sequencer Reset Timeout */
    ren_t wd_rst_to;		/* Watchdog Reset Counter */
    ren_t scratchpad;		/* Scratchpad */
    ren_t fan_dyn_prop_rpm_th;	/* 30 - Fan Dynamic Proportional RPM Threshold*/
    ren_t fan_dyn_prop_hi;	/* Fan Dynamic Proportional Upper Value */
    ren_t fan_dyn_prop_lo;	/* Fan Dynamic Proportional Lower Value */
    ren_t fan_dyn_prop_step;	/* Fan Dynamic Proportional Step value */
    ren_t temp5_l4_l3;		/* Temperature 5 L4 to L3 */
    ren_t temp6_l3_l4;		/* Temperature 6 L3 to L4 */
    ren_t temp7_l5_l4;		/* Temperature 7 L5 to L4 */
    ren_t temp8_l4_l5;		/* Temperature 8 L4 to L5 */
    ren_t fan1_l4;		/* 38 - Fan 1 Level 4 Speed */
    ren_t fan1_l5;		/* Fan 1 Level 5 Speed */
    ren_t fan2_l4;		/* Fan 2 Level 4 Speed */
    ren_t fan2_l5;		/* Fan 2 Level 5 Speed */
    ren_t fan3_l4;		/* Fan 3 Level 4 Speed */
    ren_t fan3_l5;		/* Fan 3 Level 5 Speed */
    ren_t fan4_l4;		/* Fan 4 Level 4 Speed */
    ren_t fan4_l5;		/* Fan 4 Level 5 Speed */
    ren_t fan5_l4;		/* 40 - Fan 5 Level 4 Speed */
    ren_t fan5_l5;		/* Fan 5 Level 5 Speed */
    ren_t soft_reset;		/* Soft Reset/Watchdog Test */
    ren_t max_in1_temp;		/* Maximum Inlet 1 Temperature */
    ren_t max_in2_temp;		/* Maximum Inlet 2 Temperature */
    ren_t max_out1_temp;	/* Maximum Outlet 1 Temperature */
    ren_t max_out2_temp;	/* Maximum Outlet 2 Temperature */
    ren_t work_temp;		/* Working Temperature */
    ren_t in1_temp_off;		/* 48 - Inlet 1 Temperature Offset */
    ren_t in2_temp_off;		/* Inlet 2 Temperature Offset */
    ren_t out1_temp_off;	/* Outlet 1 Temperature Offset */
    ren_t out2_temp_off;	/* Outlet 2 Temperature Offset */
} dev_env_mcu_t;

/* Simple Register Read/Write test options */
typedef enum {
    ENV_REG_TEST_OP1 = 0,	/* Read only register */
    ENV_REG_TEST_OP2,		/* Write -25 and 120. same result */
    ENV_REG_TEST_OP3,		/* Write 0 and 30000. same result */
    ENV_REG_TEST_OP4,		/* Write 0 and 30000. 0 to 1 */
    ENV_REG_TEST_INVALID,	/* End of table */
} env_reg_test_opt_t;

/* Special Environmental Control Unit diag return code */
typedef enum {
    ENV_WR_RD_PASSED = 0,	/* Read/write successful */
    ENV_WR_FAILED,		/* Write failed */
    ENV_RD_FAILED,		/* Read failed */
} env_reg_ret_t;

/* Simple Register Read/Write test table struct */
typedef struct env_reg_test_t_ {
    uint	option;
    ren_o	offset;
} env_reg_test_t;

/* Temperature Alert Test table struct */
typedef struct env_temp_alert_test_t_ {
    ren_o	current_t;	/* Current temperature offset */
    ren_o	alert_t;	/* Alert temperature offset */
    ren_t	stat1_mask;	/* Status 1 mask bit tested */
} env_temp_alert_test_t;

/* Hysteresis test table struct */
typedef struct env_hyst_test_t_ {
    int		wait;		/* Wait time */
    ren_t	level;		/* Expected level */
    ren_t	diff;		/* Temperature diff */
    ren_o	offset;		/* Test temperature offset */
} env_hyst_test_t;

/* Temperature Offset test table struct */
typedef struct env_tmp_off_test_t_ {
    char *	reg_name;	/* Register string */
    ren_o	temp;		/* Temperature register offset */
    ren_o	off;		/* Temperature offset register offset */
} env_tmp_off_test_t;

/* Platform Wattage parameters table struct */
typedef struct env_watt_t_ {
    int bit_res;	/* Bit Resolution */
    int rin;		/* RIn */
    int rout;		/* ROut */
    int rsense;		/* RSense */
    int const_i;	/* CONST_I */
    int const_p;	/* CONST_P */
} env_watt_t;

/* Registers offset defines */
#define ENV_MCU_REV	0x00
#define ENV_MCU_STA1	0x01
#define ENV_MCU_STA2	0x02
#define ENV_MCU_I1T	0x03
#define ENV_MCU_I2T	0x04
#define ENV_MCU_O1T	0x05
#define ENV_MCU_O2T	0x06
#define ENV_MCU_F1T	0x07
#define ENV_MCU_F2T	0x08
#define ENV_MCU_F3T	0x09
#define ENV_MCU_F4T	0x0A
#define ENV_MCU_F5T     0x0B
#define ENV_MCU_CUR	0x0C
#define ENV_MCU_MCM	0x0D
#define ENV_MCU_CTL1	0x0E
#define ENV_MCU_CTL2	0x0F    
#define ENV_MCU_T1	0x10
#define ENV_MCU_T2	0x11
#define ENV_MCU_T3	0x12
#define ENV_MCU_T4	0x13
#define ENV_MCU_F1L	0x14
#define ENV_MCU_F1M	0x15
#define ENV_MCU_F1H	0x16
#define ENV_MCU_F2L	0x17
#define ENV_MCU_F2M	0x18
#define ENV_MCU_F2H	0x19
#define ENV_MCU_F3L	0x1A
#define ENV_MCU_F3M	0x1B
#define ENV_MCU_F3H	0x1C
#define ENV_MCU_F4L	0x1D
#define ENV_MCU_F4M	0x1E
#define ENV_MCU_F4H	0x1F
#define ENV_MCU_F5L	0x20
#define ENV_MCU_F5M	0x21
#define ENV_MCU_F5H	0x22
#define ENV_MCU_PROP	0x23
#define ENV_MCU_PWS	0x24
#define ENV_MCU_HTO	0x25
#define ENV_MCU_TRT	0x26
#define ENV_MCU_I1A	0x27
#define ENV_MCU_I2A	0x28
#define ENV_MCU_O1A	0x29
#define ENV_MCU_O2A	0x2A
#define ENV_MCU_AST	0x2B
#define ENV_MCU_PSR	0x2C
#define ENV_MCU_PRT	0x2D
#define ENV_MCU_WRT	0x2E
#define ENV_MCU_SCR	0x2F
#define ENV_MCU_DPT	0x30
#define ENV_MCU_DPH	0x31
#define ENV_MCU_DPL	0x32
#define ENV_MCU_DPS	0x33
#define ENV_MCU_T5	0x34
#define ENV_MCU_T6	0x35
#define ENV_MCU_T7	0x36
#define ENV_MCU_T8	0x37
#define ENV_MCU_F1L4	0x38
#define ENV_MCU_F1L5	0x39
#define ENV_MCU_F2L4	0x3A
#define ENV_MCU_F2L5	0x3B
#define ENV_MCU_F3L4	0x3C
#define ENV_MCU_F3L5	0x3D
#define ENV_MCU_F4L4	0x3E
#define ENV_MCU_F4L5	0x3F
#define ENV_MCU_F5L4	0x40
#define ENV_MCU_F5L5	0x41
#define ENV_MCU_SFT_WDG	0x42
#define ENV_MCU_MI1T	0x43
#define ENV_MCU_MI2T	0x44
#define ENV_MCU_MO1T	0x45
#define ENV_MCU_MO2T	0x46
#define ENV_MCU_WRKT	0x47
#define ENV_MCU_I1TO	0x48
#define ENV_MCU_I2TO	0x49
#define ENV_MCU_O1TO	0x4A
#define ENV_MCU_O2TO	0x4B

/* Firmware Revistion - 0x00 */
#define ENV_MCU_REV_MAJOR	0x3F00	/* Major Revision */
#define ENV_MCU_REV_MINOR	0x00FF	/* Minor Revision */
#define ENV_VER_MAJOR_SHIFTS	8	/* Shift count for the Major version */
#define ENV_MCU_REV_MEG		0x8000	/* Megatron Firmware */
#define ENV_MCU_REV_DEBUG	0x4000	/* Debug version */

/* Status 1 - 0x01 */
#define ENV_MCU_STAT1_AG_TEMP	0x8000	/* Aggregate Temperature Alert */
#define ENV_MCU_STAT1_AG_ROT	0x4000	/* Aggregate Rotation Alert */
#define ENV_MCU_STAT1_AG_SW_ER	0x2000	/* Aggregate Software Error */
#define ENV_MCU_STAT1_FAN5_ROT	0x1000	/* Fan 5 Rotating */
#define ENV_MCU_STAT1_FAN4_ROT	0x0800	/* Fan 4 Rotating */
#define ENV_MCU_STAT1_FAN3_ROT	0x0400	/* Fan 3 Rotating */
#define ENV_MCU_STAT1_FAN2_ROT	0x0200	/* Fan 2 Rotating */
#define ENV_MCU_STAT1_FAN1_ROT	0x0100	/* Fan 1 Rotating */
#define ENV_MCU_STAT1_1617_ALT	0x0080	/* Max1617A Alert Input Status */
#define ENV_MCU_STAT1_OUT2_ALT	0x0020	/* Outlet 2 Temperature Alert */
#define ENV_MCU_STAT1_OUT1_ALT	0x0010	/* Outlet 1 Temperature Alert */
#define ENV_MCU_STAT1_IN2_ALT	0x0008	/* Inlet 2 Temperature Alert */
#define ENV_MCU_STAT1_IN1_ALT	0x0004	/* Inlet 1 Temperature Alert */
#define ENV_MCU_STAT1_FAN_T_D	0x0002	/* Fan Tray Dirflow Direction */
#define ENV_MCU_STAT1_FAN_TRAY	0x0001	/* Fan Tray Present */

/* Status 2 - 0x02 */
#define ENV_MCU_STAT2_ADC_TO	0x8000	/* ADC Timeout Error */
#define ENV_MCU_STAT2_HYS_TEMP	0x4000	/* Hysteresis Temp. Out of Order */
#define ENV_MCU_STAT2_WD_RST	0x2000	/* Watchdog Reset Flag */
#define ENV_MCU_STAT2_NO_FAN	0x1000	/* No Fan Enabled */
#define ENV_MCU_STAT2_FAN_ST_M	0x0E00	/* Current Fan Status mask */
#define ENV_MCU_STAT2_FAN_FULL	0x0E00	/* Alert state, running at full speed */
#define ENV_MCU_STAT2_FAN_LH	0x0A00	/* Fan speed is based on Linear Hyst */
#define ENV_MCU_STAT2_FAN_HI	0x0800	/* Fans are running at Level 5 (high) */
#define ENV_MCU_STAT2_FAN_L4	0x0600	/* Fans are running at Level 4 */
#define ENV_MCU_STAT2_FAN_MID	0x0400	/* Fans are running at Level 3 */
#define ENV_MCU_STAT2_FAN_L2	0x0200	/* Fans are running at Level 2 */
#define ENV_MCU_STAT2_FAN_LO	0x0000	/* Fans are running at Level 1 (low) */
#define ENV_MCU_STAT2_REV_AIR	0x0100	/* Reverse Airflow Indication */
#define ENV_MCU_STAT2_OT_SD	0x0090	/* Over Temperature Shutdown */

/* Control 1 - 0x0E */
#define ENV_MCU_CTRL1_PWM	0x8000	/* Fan Speed PWM */
#define ENV_MCU_CTRL1_HI_FAN	0x4000	/* Fan High Speed register */
#define ENV_MCU_CTRL1_FAN5_EN	0x1000	/* Enable Fan 5 */
#define ENV_MCU_CTRL1_FAN4_EN	0x0800	/* Enable Fan 4 */
#define ENV_MCU_CTRL1_FAN3_EN	0x0400	/* Enable Fan 3 */
#define ENV_MCU_CTRL1_FAN2_EN	0x0200	/* Enable Fan 2 */
#define ENV_MCU_CTRL1_FAN1_EN	0x0100	/* Enable Fan 1 */
#define ENV_MCU_CTRL1_1617_EN	0x0080	/* Enable Max1617A Alert */
#define ENV_MCU_CTRL1_O2_AL_EN	0x0020	/* Enable Outlet 2 Temperature Alert */
#define ENV_MCU_CTRL1_O1_AL_EN	0x0010	/* Enable Outlet 1 Temperature Alert */
#define ENV_MCU_CTRL1_I2_AL_EN	0x0008	/* Enable Inlet 2 Temperature Alert */
#define ENV_MCU_CTRL1_I1_AL_EN	0x0004	/* Enable Inlet 1 Temperature Alert */
#define ENV_MCU_CTRL1_CPU_D_EN	0x0002	/* Enable CPU Die Temp Alert Shutdown */
#define ENV_MCU_CTRL1_48V_EN	0x0001	/* Negative 48V Enable */

/* Control 2 - 0x0F */
#define ENV_MCU_CTRL2_FAN_IN_M	0xC000	/* Fan Control Temperature Inputs Mask*/
/*	Fan Control Temperature Inputs */
#define ENV_MCU_CTRL2_FCTL_SC	0x0000	/* Based on Scratchpad register */
#define ENV_MCU_CTRL2_FCTL_IN2	0x4000	/* Based on inlet 2 */
#define ENV_MCU_CTRL2_FCTL_IN1	0x8000	/* Based on inlet 1 */
#define ENV_MCU_CTRL2_FCTL_AV	0xC000	/* Based on average of inlets 1 and 2 */

#define ENV_MCU_CTRL2_PWM_D	0x2000	/* Dynamic proportional Controller */
#define ENV_MCU_CTRL2_FT_SHUT	0x0800	/* Fan Tray Removal shutdown enable */
#define ENV_MCU_CTRL2_HYS_M	0x0600	/* Hysteresis Select mask */
#define ENV_MCU_CTRL2_HYS_2	0x0400	/* Timed hysteresis method */
#define ENV_MCU_CTRL2_LI	0x0200	/* Linear interpolation */
#define ENV_MCU_CTRL2_NORM_HYS	0x0000	/* Normal hysteresis method */
#define ENV_MCU_CTRL2_REV_IN_O	0x0100	/* Reverse Inlet Outlet */
#define ENV_MCU_CTRL2_FT_PR	0x0080	/* Always Fan Tray Present */
#define ENV_MCU_CTRL2_FR_FN_AL	0x0040	/* Force Fans Alert */
/*#define ENV_MCU_CTRL2_ACT_LED	0x0002	 * LED Activity Yellow */
/*#define ENV_MCU_CTRL2_SYS_LED	0x0001	 * LED System Yellow */

/* Temperture (Inlet 1/2, Outlet 1/2) - 0x03-06 */
/* Temperature 1 Mid to Low - 0x10 */
/* Temperature 2 Low to Mid - 0x11 */
/* Temperature 3 High to Mid - 0x12 */
/* Temperature 4 Mid to High - 0x13 */
/* Inlet 1 Alert Temperature - 0x27 */
/* Inlet 2 Alert Temperature - 0x28 */
/* Outlet 1 Alert Temperature - 0x29 */
/* Outlet 2 Alert Tempearture - 0x2A */
#define ENV_MCU_TEMP_MASK	0xFFFF	/* Temperature in Celcius mask */
#define ENV_MCU_TEMP_MAX	127	/* Maximum temperature */
#define ENV_MCU_TEMP_MIN	-128	/* Minimum temperature */
#define ENV_ALERT_MAX_TEMP	90	/* Maximum alert temperature. Used to
					 * avoid components been damaged */

/* Fan Tachometer RPM (Fan 1-5) - 0x07-0B */
/* Fan Low/Middle/High Speeds (Fan 1-5) - 0x14-22 */
#define ENV_MCU_FAN_SPEED_MASK	0xFFFF	/* Fan speed in RPM or PWM */
#define ENV_MCU_FAN_RPM_MAX	0xFFFF	/* Maximum RPM */
#define ENV_MCU_FAN_RPM_MIN	0	/* Minimum RPM */
#define ENV_MCU_FAN1_LOWER_PWM	2	/* Lower PWM for Fans 1-4, Timer B/C */
#define ENV_MCU_FAN1_UPPER_PWM	99	/* Upper PWM for Fans 1-4, Timer B/C */
#define ENV_MCU_FAN5_LOWER_PWM	30	/* Lower PWM for Fan 5, Timer E */
#define ENV_MCU_FAN5_UPPER_PWM	100	/* Upper PWM for Fan 5, Timer E */

/* Fan Control Loop Proportional Constant - 0x23 */
#define ENV_MCU_PROP_MASK	0xFFFF	/* Proportional Constant mask */
#define ENV_MCU_PROP_MAX	0xFFFF	/* Maximum constant */
#define ENV_MCU_PROP_MIN	0x0001	/* Minimum constant */

/* Fan PWM Slope - 0x24 */
#define ENV_MCU_PWM_SLOPE_MASK	0xFFFF	/* Fan PWM Slope mask */
#define ENV_MCU_PWM_SLOPE_MAX	0xFFFF	/* Maximum slope */
#define ENV_MCU_PWM_SLOPE_MIN	0x0001	/* Minimum slope */

/* Fan Hysteresis Timeout - 0x25 */
#define ENV_MCU_HYS_TO_MASK	0xFFFF	/* Fan Hysteresis Timeout mask */
#define ENV_MCU_HYS_TO_MAX	0xFFFF	/* Maximum Timeout in seconds */
#define ENV_MCU_HYS_TO_MIN	0x0000	/* Minimum Timeout in seconds */

/* Fan Tray Removal Timeout - 0x26 */
#define ENV_MCU_REM_TO_MASK	0xFFFF	/* Fan Tray Removal Timeout mask */
#define ENV_MCU_REM_TO_MAX	0xFFFF	/* Maximum Timeout in seconds */
#define ENV_MCU_REM_TO_MIN	0x0000	/* Minimum Timeout in seconds */

/* +12V Current Measurement - 0x0C */
/* +12V Maximum Current Measurement - 0x0D */
#define ENV_MCU_12V_CUR_MASK	0x00FF	/* ADC reading of +12V current */
#define ENV_MCU_12V_MAX_MASK	0x00FF	/* Maximum ADC reading of +12V current*/

/* Fan Hysteresis Timeout - 0x25 */
#define HYST_FAN_TIMEOUT	3	/* Used in Timed Hysteresis test */

/* Alert Shutdown Timer - 0x2B */
#define ENV_MCU_ALERT_TO_MASK	0x00FF	/* Alert Shutdown Timer mask */
#define ENV_MCU_ALERT_TO_MAX	0x00FF	/* Maximum shutdown timer in seconds */
#define ENV_MCU_ALERT_TO_MIN	0x0000	/* Minimum shutdown timer in seconds */

/* Power Sequencer Reset - 0x2C */
#define ENV_MCU_PWR_SEQ_RST_1	0xB0EF	/* Reset to Power Sequencer - first */
#define ENV_MCU_PWR_SEQ_RST_2	0xAEDE	/* Second write to reset power seq. */

/* Power Sequencer Reset Timeout - 0x2D */
#define ENV_MCU_PWR_SEQ_TO_MASK	0xFFFF	/* Reset Timeout to Power Sequencer */
#define ENV_MCU_PWR_SEQ_TO_MAX	0xFFFF	/* Maximum timeout value in seconds */
#define ENV_MCU_PWR_SEQ_TO_MIN	0	/* Minimum timeout value in seconds */

/* Watchdog Reset Counter - 0x2E */
#define ENV_MCU_WD_RST_CTR_MASK	0xFFFF	/* Watchdog Reset Counter mask */
#define ENV_MCU_WD_RST_CTR_MAX	0xFFFF	/* Maximum reset counter */
#define ENV_MCU_WD_RST_CTR_MIN	0x0000	/* Minimum reset counter */

/* Soft Reset/Watchdog Test - 0x42 */
#define ENV_MCU_SFT_RST1	0xB0EF
#define ENV_MCU_SFT_RST2	0xAEDE
#define ENV_MCU_WDG_TST1	0xBABE
#define ENV_MCU_WDG_TST2	0xC0FE

/* Inlet  1 Temperature Offset - 0x48 */
/* Inlet  2 Temperature Offset - 0x49 */
/* Outlet 1 Temperature Offset - 0x4A */
/* Outlet 2 Temperature Offset - 0x4B */
#define ENV_MCU_TMP_OFF_MASK	0x00FF	/* Temperature Offset mask */

/* Test patterns used in Simple Registers Read/Write */
#define ENV_REG_TEST_OPTION1_PATTERN	0	/* test option 1 pattern */
#define ENV_REG_TEST_OPTION2_PATTERN1	-25	/* test option 2 low pattern */
#define ENV_REG_TEST_OPTION2_PATTERN2	120	/* test option 2 high pattern */
#define ENV_REG_TEST_OPTION3_PATTERN1	0	/* test option 3 low pattern */
#define ENV_REG_TEST_OPTION3_PATTERN2	30000	/* test option 3 high pattern */
#define ENV_REG_TEST_OPTION4_PATTERN1	0	/* test option 4 low pattern */
#define ENV_REG_TEST_OPTION4_PATTERN2	30000	/* test option 4 high pattern */
#define ENV_REG_TEST_OPTION4_PATTERN1_EXP 1	/* test option 4 low read */

#define ENV_ALERT_TEST_DELTA	10	/* Less than current temp to cause
					 * alert condition
					 */
#define ENV_TEMP_OFF_TOLR	3	/* Temperature offset test tolerance */
#define ENV_ALERT_TEST_DELTA_LM75 0x0A00 /* LM75 delta */
#define ENV_48V_ACTIVE_TIME	50	/* -48V activation time. 20 ms */
#define ENV_ROT_WAIT_TIME	2500	/* Rotation Alert test wait 2 seconds */
#define ENV_48V_DEACTIVE_COUNT	200	/* -48V deactivate - 75 ms. Will use
					 * 10 seconds per HW */
#define ENV_HYST_TIME		7000	/* Hysteresis test wait time */
#define ENV_HYST_TIM2		9000	/* Timed Hysteresis test wait time */
#define ENV_FORCE_FAN_AL_TIME	3000	/* Force Fans Alert time - 3 seconds */
#define MS_PER_SECOND		1000	/* milliseconds per second */

/* Functions prototype */
extern void build_fan_utils_menu(void);
#ifndef LINUX_KLM
extern void build_env_test_menu(int submenu);
extern int init_env(void);
extern int show_env_temp(int err_log, int format);
extern int env_get_version(uint16_t *);
#endif

#endif /* __PLATFORM_ENV_H__ */

/*------------------------------------------------------------------
$Log: platform_env.h,v $
Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
