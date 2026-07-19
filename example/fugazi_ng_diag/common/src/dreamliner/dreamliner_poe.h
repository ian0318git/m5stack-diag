/* $Id: dreamliner_poe.h,v 1.2 2015/02/27 10:02:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/dreamliner_poe.h,v $
 *------------------------------------------------------------------
 *
 * dreamliner_poe.h - This file contains defines for Dreamliner POE 
 *                    controller.
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/* need to check with HW */
#define POE_I2C_ADDR0                0x20            /* for first PoE controller */
#define POE_I2C_ADDR1                0x28            /* for second PoE controller */
#define POE_I2C_ADDR2                0x29            /* for second PoE controller */

#define TPS2386B_ID                  0x52            /* TI SN2386B */
#define SI3457_ID                    0x42            /* Silocon Lab Si3457 */
#define TI_MSR_ID                    0x50            /* TI SN2386B */
#define SI_MSR_ID                    0x40            /* Silocon Lab Si3457 */
#define POE_PORTS                    4
#define POE0_EXT_START_PORT          0
#define POE1_EXT_START_PORT          (POE0_EXT_START_PORT + POE_PORTS)

/************************* SN2386B ************************/
/* SN2386B Registers map */
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
#define ILP_PWRPR_ICUT_DISABLE              0x15    /* power priority/ICUT disable */
#define ILP_TIMING_CONFIG                   0x16    /* Timing Config */
#define ILP_MISC_CONFIG                     0x17    /* Misc Config */
#define ILP_DET_CLASS_RESTART_PB            0x18    /* Det/Class Restart PB */
#define ILP_POWER_ENABLE_PB                 0x19    /* Power Enable PB */
#define ILP_GLOBAL_PB                       0x1A    /* Global PB */
#define ILP_ID                              0x1B    /* ID */
#define ILP_POLICE_21_CONFIG                0x1E    /* police 21 config */
#define ILP_POLICE_43_CONFIG                0x1F    /* police 43 config */
#define ILP_IEEE_PWR_ENABLE                 0x23    /* IEEE power enable */
#define ILP_PWR_ON_FAULT                    0x24    /* power-on fault */
#define ILP_PWR_ON_FAULT_COR                0x25    /* power-on fault CoR */
#define ILP_TEMPERATURE                     0x2C    /* temperature */
#define ILP_INPUT_VDC_LSB                   0x2E    /* input voltage LSB */
#define ILP_INPUT_VDC_MSB                   0x2E    /* input voltage MSB */
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
#define ILP_COOL_DOWN                       0x45    /* cool down */

#define PORTS_PER_ILP                       4   /* Ports per PoE controller */

/* Defines for Interrupt and Int Mask registers */
#define SUPPLY_FAULT                        0x80    /* Supply Fault */
#define TSTART_FAULT                        0x40    /* tSTART Fault */
#define IMAX_FAULT                          0x20    /* IMAX Fault */
#define CLASS_COMPLETE                      0x10    /* Class Complete */
#define DETECT_COMPLETE                     0x08    /* Detect Complete */
#define DISCONNECT                          0x04    /* Disconnect */
#define PWR_GOOD_EVENT                      0x02    /* Pwr Good Event */
#define POWER_ENABLE                        0x01    /* Power Enable */

/* Defines for Power Event and Power Event CoR registers */
#define PWR_GOOD_CHG4                       0x80    /* Pwr Good Chg 4 */
#define PWR_GOOD_CHG3                       0x40    /* Pwr Good Chg 3 */
#define PWR_GOOD_CHG2                       0x20    /* Pwr Good Chg 2 */
#define PWR_GOOD_CHG1                       0x10    /* Pwr Good Chg 1 */
#define PWR_ENABLE_CHG4                     0x08    /* Pwr Enable Chg 4 */
#define PWR_ENABLE_CHG3                     0x04    /* Pwr Enable Chg 3 */
#define PWR_ENABLE_CHG2                     0x02    /* Pwr Enable Chg 2 */
#define PWR_ENABLE_CHG1                     0x01    /* Pwr Enable Chg 1 */

/* Defines for Detect Event and Detect Event CoR registers */
#define CLASS_COMPLETE_4                    0x80    /* Class Complete 4 */
#define CLASS_COMPLETE_3                    0x40    /* Class Complete 3 */
#define CLASS_COMPLETE_2                    0x20    /* Class Complete 2 */
#define CLASS_COMPLETE_1                    0x10    /* Class Complete 1 */
#define DETECT_COMPLETE_4                   0x08    /* Detect Complete 4 */
#define DETECT_COMPLETE_3                   0x04    /* Detect Complete 3 */
#define DETECT_COMPLETE_2                   0x02    /* Detect Complete 2 */
#define DETECT_COMPLETE_1                   0x01    /* Detect Complete 1 */

/* Defines for Fault Event and Fault Event CoR registers */
#define DISCONNECT_4                        0x80    /* Disconnect 4 */
#define DISCONNECT_3                        0x40    /* Disconnect 3 */
#define DISCONNECT_2                        0x20    /* Disconnect 2 */
#define DISCONNECT_1                        0x10    /* Disconnect 1 */
#define IMAX_FAULT_4                        0x08    /* IMAX Fault 4 */
#define IMAX_FAULT_3                        0x04    /* IMAX Fault 3 */
#define IMAX_FAULT_2                        0x02    /* IMAX Fault 2 */
#define IMAX_FAULT_1                        0x01    /* IMAX Fault 1 */

/* Defines for tSTART Event and tSTART Event CoR registers */
#define TSTART_FAULT_4                      0x08    /* tSTART Fault 4 */
#define TSTART_FAULT_3                      0x04    /* tSTART Fault 3 */
#define TSTART_FAULT_2                      0x02    /* tSTART Fault 2 */
#define TSTART_FAULT_1                      0x01    /* tSTART Fault 1 */

/* Defines for Supply Event and Supply Evetn CoR registers */
#define OVER_TEMP                           0x80    /* Over Temp */
#define VDD_UVLO                            0x20    /* VDD UVLO */
#define VEE_UVLO                            0x10    /* VEE UVLO */

/* Defines for Ports 1 -4 Status registers */
#define CLASS_MASK                          0x70    /* Class status mask */
#define UNKNOWN_CLASS                       0       /* Class Status Unknown */
#define CLASS1                              1       /* Class 1 */
#define CLASS2                              2       /* Class 2 */
#define CLASS3                              3       /* Class 3 */
#define CLASS4                              4       /* Class 4 */
#define CLASS0                              6       /* Class 0 */
#define OVERCURRENT                         7       /* Overcurrent */

#define DETECT_MASK                         0x0f    /* Detect status mask */
#define UNKNOWN_DETECT                      0       /* Detect Statusw Unknown */
#define SHORT_CIRCUIT                       1       /* Short Circuit (<1V) */
#define RLOW                                3       /* RLOW (<15K) */
#define DETECT_GOOD                         4   /* Detect Good (15K < R < 33K)*/
#define RHIGH                               5       /* RHIGH (>33K) */
#define OPEN_CIRCUIT                        6       /* Open Circuit */
#define MOSFET_FAULT                        0xe     /* MOSFET fault */

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

/* Defines for Pin Status register */
#define PIN_STAT_ADDR_MASK                  0x3C    /* A3 - A0 Pin Status */
#define AUXIN                               0x02    /* AUXIN Pin Status */
#define PIN_STAT_AUTO                       0x01    /* Auto Pin Status */

/* Defines for Operating Mode register */
#define MODE_MASK                           0x03    /* Operating mode mask */
#define SHUTDOWN                            0       /* Shutdown - power off, 
                                                       detection and class off*/
#define MANUAL                              1       /* Manual - will not advance
                                                       between states */
#define SEMIAUTO                            2       /* Semiauto - detect and 
                                                       class but wait to turn 
                                                       on power */
#define MODE_AUTO                           3       /* Auto - detect, class and
                                                       power automatically */
/* Defines for Disconnect Enable register */
#define DC_DIS_ENABLE_4                     0x08    /* DC Dis Enable 4 */
#define DC_DIS_ENABLE_3                     0x04    /* DC Dis Enable 3 */
#define DC_DIS_ENABLE_2                     0x02    /* DC Dis Enable 2 */
#define DC_DIS_ENABLE_1                     0x01    /* DC Dis Enable 1 */
#define DC_DIS_ALL                          0x0F    /* DC Dis Enable all */

/* Defines for Detect/Class Enable register */
#define CLASS_ENABLE_4                      0x80    /* Class Enable 4 */
#define CLASS_ENABLE_3                      0x40    /* Class Enable 3 */
#define CLASS_ENABLE_2                      0x20    /* Class Enable 2 */
#define CLASS_ENABLE_1                      0x10    /* Class Enable 1 */
#define DETECT_ENABLE_4                     0x08    /* Detect Enable 4 */
#define DETECT_ENABLE_3                     0x04    /* Detect Enable 3 */
#define DETECT_ENABLE_2                     0x02    /* Detect Enable 2 */
#define DETECT_ENABLE_1                     0x01    /* Detect Enable 1 */

/* Defines for Timing Config register */
#define TIMING_CONFIG_MASK  0x03    /* Timing Config mask */
/*  tSTART      */
#define TSTART_60MS                         0       /* 60 ms */
#define TSTART_30MS                         0x10    /* 30 ms */
#define TSTART_120MS                        0x20    /* 120 ms */

/*  tICUT       */
#define TICUT_60MS                          0x00    /* 60 ms */
#define TICUT_30MS                          0x04    /* 30 ms */
#define TICUT_120MS                         0x08    /* 120 ms */
#define TICUT_240MS                         0x0C    /* 240 ms */
/*  tDIS        */
#define TDIS_360MS                          0x00    /* 360 ms */
#define TDIS_90MS                           0x01    /* 90 ms */
#define TDIS_180MS                          0x02    /* 180 ms */
#define TDIS_720MS                          0x03    /* 720 ms */

/* Defines for Misc Config register */
#define INTERRUPT_PIN_ENABLE                0x80    /* Interrupt Pin Enable */
#define CLASS_CHANGE_ENABLE                 0x08    /* class change enable */
#define DETECT_CHANGE_ENABLE                0x04    /* detect change enable */

/* Det/Class Restart PB register */
/* Same as Detect/Class Enable register */

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

/* Defines for Reset register */
#define CLEAR_ALL_INTS                      0x80
#define CLEAR_INT_PIN                       0x40
#define RESET_IC                            0x10
#define RESET_PORT4                         0x08
#define RESET_PORT3                         0x04
#define RESET_PORT2                         0x02
#define RESET_PORT1                         0x01

/* Power 2X mode register */
#define ICUT_MAX                            0x0f    /* ICut maxmum */
#define POWER_2X                            0x10    /* Power plus */

#define NO_PHONE                            0x00    /* Detect None */
#define CISCO_PHONE                         0x01    /* Cisco Phone */
#define IEEE_PHONE                          0x02    /* IEEE Phone */

#define POE_INTR_DELAY                      500

/*
 *------------------------------------------------------------------
 * $Log: dreamliner_poe.h,v $
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 *
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 * 
 * Revision 1.1.2.2  2015/01/28 20:28:34  iachang
 * Fixed 2nd source SI PoE hang up on with FPGA(7/22/2014) issue.
 * 
 * Revision 1.1.2.1  2014/12/02 08:04:11  iachang
 * Dreamliner Diag initial check-in.
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */
