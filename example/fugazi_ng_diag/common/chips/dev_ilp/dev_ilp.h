/* $Id: dev_ilp.h,v 1.2 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_ilp/dev_ilp.h,v $
 *------------------------------------------------------------------
 *
 * File name: dev_ilp.h
 *
 * Description: The device driver of In Line Power (PoE)
 *              (ltc4659, TI sn2385d, Si3456D)
 *              Source: lediag and Firebee
 *
 * April 2010 by tirawan 
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_ILP_H__
#define __DEV_ILP_H__

#define ILP_PORT_OFF_TIMEOUT                100    /* 100 msec */

#define PORTS_PER_ILP                       4
#define ONE_SECOND                          1000
#define I2C_REG_WIDTH                       8

#define ILP_YELLOW_LED                      0x10
#define ILP_GREEN_LED                       0x20
#define ILP_LINK_LED                        0x40

#define ILP_LED_ON                          0x01
#define ILP_LED_OFF                         0x02
#define ILP_LED_NORMAL                      0x04
#define ILP_LED_BLINK                       0x08
#define ILP_LED_DEFAULT                     0x80

#define NO_PHONE                            0x00    /* Detect None */
#define CISCO_PHONE                         0x01    /* Cisco Phone */
#define IEEE_PHONE                          0x02    /* IEEE Phone */

#define ILP_INTERRUPT                       0x00    /* Interrupt */
#define ILP_INT_MASK                        0x01    /* Int Mask */
#define ILP_POWER_EVENT                     0x02    /* Power Event */
#define ILP_POWER_EVENT_COR                 0x03    /* Power Event CoR */
#define ILP_DETECT_EVENT                    0x04    /* Detect Event */
#define ILP_DETECT_EVENT_COR                0x05    /* Detect Event CoR */
#define ILP_FAULT_EVENT                     0x06    /* Fault Event */
#define ILP_FAULT_EVENT_COR                 0x07    /* Fault Event CoR */
#define ILP_TSTART_EVENT                    0x08    /* tSTART Event */
#define ILP_TSTART_EVENT_COR                0x09    /* tSTART Event CoR */
#define ILP_SUPPLY_EVENT                    0x0A    /* Supply Event */
#define ILP_SUPPLY_EVENT_COR                0x0B    /* Supply Event CoR */
#define ILP_PORT1_STATUS                    0x0C    /* Port 1 Status */
#define ILP_PORT2_STATUS                    0x0D    /* Port 2 Status */
#define ILP_PORT3_STATUS                    0x0E    /* Port 3 Status */
#define ILP_PORT4_STATUS                    0x0F    /* Port 4 Status */
#define ILP_POWER_STATUS                    0x10    /* Power Status */
#define ILP_PIN_STATUS                      0x11    /* Pin Status */
#define ILP_OPERATING_MODE                  0x12    /* Operating Mode */
#define ILP_DISCONNECT_ENABLE               0x13    /* Disconnect Enable */
#define ILP_DETECT_CLASS_ENABLE             0x14    /* Detect/Class Enable */
#define ILP_TIMING_CONFIG                   0x16    /* Timing Config */
#define ILP_MISC_CONFIG                     0x17    /* Misc Config */
#define ILP_DET_CLASS_RESTART_PB            0x18    /* Det/Class Restart PB */
#define ILP_POWER_ENABLE_PB                 0x19    /* Power Enable PB */
#define ILP_GLOBAL_PB                       0x1A    /* Global PB */
#define ILP_ID                              0x1B    /* ID */
#define ILP_PWRPR                           0x22    /* PWRPR */
#define ILP_ICUT_P1_P2                      0x2A    /* iCut limit of port 1, 2*/
#define ILP_ICUT_P3_P4                      0x2B    /* iCut limit of port 3, 4*/
#define ILP_IDC_P1_LSB                      0x30    /* IDC P1 LSB */
#define ILP_IDC_P1_MSB                      0x31    /* IDC P1 MSB */
#define ILP_VDC_P1_LSB                      0x32    /* VDC P1 LSB */
#define ILP_VDC_P1_MSB                      0x33    /* VDC P1 MSB */
#define ILP_IDC_P2_LSB                      0x34    /* IDC P2 LSB */
#define ILP_IDC_P2_MSB                      0x35    /* IDC P2 MSB */
#define ILP_VDC_P2_LSB                      0x36    /* VDC P2 LSB */
#define ILP_VDC_P2_MSB                      0x37    /* VDC P2 MSB */
#define ILP_IDC_P3_LSB                      0x38    /* IDC P3 LSB */
#define ILP_IDC_P3_MSB                      0x39    /* IDC P3 MSB */
#define ILP_VDC_P3_LSB                      0x3A    /* VDC P3 LSB */
#define ILP_VDC_P3_MSB                      0x3B    /* VDC P3 MSB */
#define ILP_IDC_P4_LSB                      0x3C    /* IDC P4 LSB */
#define ILP_IDC_P4_MSB                      0x3D    /* IDC P4 MSB */
#define ILP_VDC_P4_LSB                      0x3E    /* VDC P4 LSB */
#define ILP_VDC_P4_MSB                      0x3F    /* VDC P4 MSB */
#define ILP_2X_MODE                         0x40    /* PoE Plus Power(2x mode)*/
#define ILP_FIRMWARE_REV                    0x41    /* Firmware Revision */
#define ILP_WATCHDOG                        0x42    /* Watch Dog */
#define ILP_DEVICE_ID                       0x43    /* Device ID */
#define ILP_PUSHBOTTON1                     0x50    /* PUSHBOTTON1 */
#define ILP_PUSHBOTTON2                     0x51    /* PUSHBOTTON2 */
#define ILP_PUSHBOTTON3                     0x52    /* PUSHBOTTON3 */
#define ILP_PUSHBOTTON4                     0x53    /* PUSHBOTTON4 */
#define ILP_PWR_PD_DIS1                     0x54    /* ILP_PWR_PD_DIS1 */
#define ILP_PWR_PD_DIS2                     0x55    /* ILP_PWR_PD_DIS2 */
#define ILP_PWR_PD_DIS3                     0x56    /* ILP_PWR_PD_DIS3 */
#define ILP_PWR_PD_DIS4                     0x57    /* ILP_PWR_PD_DIS4 */
#define ILP_PORT_RST_INTR                   0x58    /* ILP_PORT_RST_INTR */
#define ILP_INTR_CNTL                       0x59    /* ILP_INTR_CNTL */
#define ILP_TIMER_VALUE                     0x5A    /* ILP_TIMER_VALUE */
#define ILP_MAX_REG                         ILP_ICUT_P3_P4

/* Defines for Operating Mode register */
#define MODE_MASK                           0x03    /* Operating mode mask */
#define SHUTDOWN                            0       /* Shutdown - power off, 
                                                       detection and class off*/
#define MANUAL                              1       /* Manual - will not advance
                                                       betweenstates */
#define SEMIAUTO                            2       /* Semiauto - detect and 
                                                       class but waitto turn 
                                                       on power */
#define MODE_AUTO                           3       /* Auto - detect, class and
                                                       power automatically */

/* Defines for Power Status register */
#define POWER_GOOD_4                        0x80    /* Power Good 4 */
#define POWER_GOOD_3                        0x40    /* Power Good 3 */
#define POWER_GOOD_2                        0x20    /* Power Good 2 */
#define POWER_GOOD_1                        0x10    /* Power Good 1 */
#define POWER_GOOD_MASK                     0xF0
#define POWER_ENABLE_4                      0x08    /* Power Enable 4 */
#define POWER_ENABLE_3                      0x04    /* Power Enable 3 */
#define POWER_ENABLE_2                      0x02    /* Power Enable 2 */
#define POWER_ENABLE_1                      0x01    /* Power Enable 1 */
#define POWER_ENABLE_MASK                   0x0F

/* Defines for Power Enable PB register */
#define POWER_OFF_4                         0x80    /* Power off 4 */
#define POWER_OFF_3                         0x40    /* Power off 3 */
#define POWER_OFF_2                         0x20    /* Power off 2 */
#define POWER_OFF_1                         0x10    /* Power off 1 */
#define POWER_OFF_ALL                       0xF0    /* Power off all ports */
#define POWER_ON_4                          0x08    /* Power on 4 */
#define POWER_ON_3                          0x04    /* Power on 3 */
#define POWER_ON_2                          0x02    /* Power on 2 */
#define POWER_ON_1                          0x01    /* Power on 1 */
#define POWER_ON_ALL                        0x0F    /* Power on all ports */

/* Defines for Ports 1 -4 Status registers */
#define CLASS_MASK                          0x70    /* Class status mask */
#define UNKNOWN_CLASS                       0       /* Class Status Unknown */
#define CLASS1                              1       /* Class 1 */
#define CLASS2                              2       /* Class 2 */
#define CLASS3                              3       /* Class 3 */
#define CLASS4                              4       /* Class 4 */
#define CLASS0                              6       /* Class 0 */
#define OVERCURRENT                         7       /* Overcurrent */

/* Defines for Detect/Class Enable register */
#define CLASS_ENABLE_4                      0x80    /* Class Enable 4 */
#define CLASS_ENABLE_3                      0x40    /* Class Enable 3 */
#define CLASS_ENABLE_2                      0x20    /* Class Enable 2 */
#define CLASS_ENABLE_1                      0x10    /* Class Enable 1 */
#define DETECT_ENABLE_4                     0x08    /* Detect Enable 4 */
#define DETECT_ENABLE_3                     0x04    /* Detect Enable 3 */
#define DETECT_ENABLE_2                     0x02    /* Detect Enable 2 */
#define DETECT_ENABLE_1                     0x01    /* Detect Enable 1 */

/* Defines for Disconnect Enable register */
#define AC_DISCON_EN_4                      0x80    /* AC Discon En 4 */
#define AC_DISCON_EN_3                      0x40    /* AC Discon En 3 */
#define AC_DISCON_EN_2                      0x20    /* AC Discon En 2 */
#define AC_DISCON_EN_1                      0x10    /* AC Discon En 1 */
#define AC_DIS_ALL                          0xF0    /* AC Discon En all */
#define DC_DIS_ENABLE_4                     0x08    /* DC Dis Enable 4 */
#define DC_DIS_ENABLE_3                     0x04    /* DC Dis Enable 3 */
#define DC_DIS_ENABLE_2                     0x02    /* DC Dis Enable 2 */
#define DC_DIS_ENABLE_1                     0x01    /* DC Dis Enable 1 */
#define DC_DIS_ALL                          0x0F    /* DC Dis Enable all */

/* Defines for Detect Event and Detect Event CoR registers */
#define CLASS_COMPLETE_4                    0x80    /* Class Complete 4 */
#define CLASS_COMPLETE_3                    0x40    /* Class Complete 3 */
#define CLASS_COMPLETE_2                    0x20    /* Class Complete 2 */
#define CLASS_COMPLETE_1                    0x10    /* Class Complete 1 */
#define DETECT_COMPLETE_4                   0x08    /* Detect Complete 4 */
#define DETECT_COMPLETE_3                   0x04    /* Detect Complete 3 */
#define DETECT_COMPLETE_2                   0x02    /* Detect Complete 2 */
#define DETECT_COMPLETE_1                   0x01    /* Detect Complete 1 */

#define DETECT_MASK                         0x07    /* Detect status mask */
#define UNKNOWN_DETECT                      0       /* Detect Statusw Unknown */
#define SHORT_CIRCUIT                       1       /* Short Circuit (<1V) */
#define RLOW                                3       /* RLOW (<15K) */
#define DETECT_GOOD                         4   /* Detect Good (15K < R < 33K)*/
#define RHIGH                               5       /* RHIGH (>33K) */
#define OPEN_CIRCUIT                        6       /* Open Circuit */

/* Defines for Timing Config register */
#define TIMING_CONFIG_MASK	            0x03	/* Timing Config mask */
/*	tSTART		*/
#define TSTART_60MS                         0	/* 60 ms */
#define TSTART_30MS		0x10	/* 30 ms */
#define TSTART_120MS		0x20	/* 120 ms */
#define TSTART_240MS		0x30	/* 240 ms */
/*	tICUT		*/
#define TICUT_60MS		0x00	/* 60 ms */
#define TICUT_30MS		0x04	/* 30 ms */
#define TICUT_120MS		0x08	/* 120 ms */
#define TICUT_240MS		0x0C	/* 240 ms */
/*	tDIS		*/
#define TDIS_360MS		0x00	/* 360 ms */
#define TDIS_90MS		0x01	/* 90 ms */
#define TDIS_180MS		0x02	/* 180 ms */
#define TDIS_720MS		0x03	/* 720 ms */

/* Defines for Interrupt and Int Mask registers */
#define SUPPLY_FAULT                        0x80    /* Supply Fault */
#define TSTART_FAULT                        0x40    /* tSTART Fault */
#define IMAX_FAULT                          0x20    /* IMAX Fault */
#define CLASS_COMPLETE                      0x10    /* Class Complete */
#define DETECT_COMPLETE                     0x08    /* Detect Complete */
#define DISCONNECT                          0x04    /* Disconnect */
#define PWR_GOOD_EVENT                      0x02    /* Pwr Good Event */
#define POWER_ENABLE                        0x01    /* Power Enable */

/* Power 2X mode register */
#define ICUT_MAX                            0x07    /* ICut maxmum */
#define POWER_2X                            0x10    /* Power plus */

typedef struct ilp_reg_info {
    char        *name;      /* Pointer to the text of the register name */
    uchar       offset;     /* Offset into the device */
    uint        type;       /* Read only, Read/Write, etc */
    uchar       mask;       /* Operation mask */
    uchar       reset_val;  /* Default value */
} ilp_reg_info_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    DEV_ILP_DEV_STATE = 0,
    DEV_ILP_NOT_PRESENT,
    DEV_ILP_INVALID_MODE,
    DEV_ILP_MANUAL_MODE,
    DEV_ILP_UNABLE_READ_CLASS,
    DEV_ILP_UNABLE_DETECT,
    DEV_ILP_POWER_STATUS,
    DEV_ILP_UNABLE_POWER_ON,
    DEV_ILP_SKIPPED,
    DEV_ILP_REGISTER_TEST,
} ltc4259_report_code_t;

/*
 * Device callin functions - Service provided and defined by the device
 */
typedef struct dev_ilp_callin_fvt_t_ {
    uint32 (*is_ilp_present)(dev_object_t *);
    uint32 (*ilp_init)(dev_object_t *);
    uint32 (*ilp_power_port)(dev_object_t *);
    uint32 (*dc_disc)(dev_object_t *);
    uint32 (*ac_disc)(dev_object_t *);
    uint32 (*show_ilp_regs)(dev_object_t *);
    uint32 (*alter_ilp_reg)(dev_object_t *);
    uint32 (*register_test)(dev_object_t *);
    uint32 (*poe_2x_mode)(dev_object_t *);
    uint32 (*force_interrupt)(dev_object_t *);
} dev_ilp_callin_fvt_t;

/*
 * Device callout functions - Service needed by the device and
 *                            defined by platform
 */
typedef struct dev_ilp_callout_fvt_t_ {
    uint32 (*i2c_read)(uchar, uchar *, int, boolean);
    uint32 (*i2c_write)(uchar, uchar *, int);
    uint32 (*phone_detect)(uchar);
    uint32 (*get_max_port)(void);
    void   (*ilp_led)(int, int);
} dev_ilp_callout_fvt_t;

/*
 * Define the ILP device object structure.
 */
typedef struct dev_ilp_object_t_ {
    dev_object_t            base;
    dev_ilp_callin_fvt_t    *callin_fvt;
    dev_ilp_callout_fvt_t   *callout_fvt;
} dev_ilp_object_t;


extern void dev_ilp_create(dev_object_t *, dev_error_report_t);

#endif /* __DEV_ILP_H__ */

/******** History ******** 
$Log: dev_ilp.h,v $
Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
