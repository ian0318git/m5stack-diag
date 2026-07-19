/* $Id: diag_ge_phy_88E1548L_lib.h,v 1.3 2015/02/14 12:48:41 kodko Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1548L_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1548L_lib.h 
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_88E1548L_LIB_H__
#define __DIAG_GE_PHY_88E1548L_LIB_H__

#include "ethernet.h"

typedef enum mrvl_88e1548_phy_t_ {
    MRVL_1548_PHY0,
    MRVL_1548_PHY1,
} mrvl_88e1548_phy_t;

typedef enum mrvl_88e1548_phy_led_mode_t_ {
    copper_mode,
    fiber_mode,
} mrvl_88e1548_phy_led_mode_t;

typedef enum mrvl_88e1548_ge_port_t_ {
    MRVL_1548_GE0,
    MRVL_1548_GE1,
    MRVL_1548_GE2,
    MRVL_1548_GE3,
    MRVL_1548_GE4,
    MRVL_1548_GE5,
} mrvl_88e1548_ge_port_t;

typedef enum mrvl_88e1548_test_mode_t_ {
    NORMAL_MODE,
    TRANSMIT_WAVEFORM_TEST,
    TRANSMIT_JITTER_TEST_MASTER_MODE,
    TRANSMIT_JITTER_TEST_SLAVE_MODE,
    TRANSMIT_DISTORTION_TEST,
} mrvl_88e1548_test_mode_t;

#define MRVL_88E1548_PHY0_SMI_ADDR          (0x0)
#define MRVL_88E1548_PHY1_SMI_ADDR          (0x4)

#define MRVL_88E1548_TEST_MODE_PAGE (0x0)
#define MRVL_88E1548_TEST_MODE_REG    (0x9)

#define MRVL_88E1548_NORMAL_MODE (0x9140)
#define MRVL_88E1548_TEST_MODE_1  (0x3f00)
#define MRVL_88E1548_TEST_MODE_2  (0x5f00)
#define MRVL_88E1548_TEST_MODE_3  (0x7700)
#define MRVL_88E1548_TEST_MODE_4  (0x9f00)
#define SET_PHY_TO_MASTER_MODE (0x1F00)
#define SET_PHY_TO_SLAVE_MODE (0x1700)
#define PHY_SOFT_RESET (0x9140)
#define DISABLE_CLOCK (0x3E80)
#define ENABLE_TX_TCLK (0x8000)

#define MRVL_88E1548_LED_CTRL_PAGE (0x3)
#define MRVL_88E1548_LED_FUNCTION_REG (0x10)
#define MRVL_88E1548_LED_POLARITY_REG (0x11)
#define MRVL_88E1548_FORCE_GE_LED_ON (0x8299)
#define MRVL_88E1548_FORCE_SFP_LED_ON (0x9289)
#define MRVL_88E1548_FORCE_GE_LED_OFF (0x1288)
#define MRVL_88E1548_FORCE_SFP_LED_OFF (0x8218)
#define MRVL_88E1548_FORCE_GE_LED_BLINK (0x12bb)
#define MRVL_88E1548_FORCE_SFP_LED_BLINK (0xb21b)
#define MRVL_88E1548_LED_SPEED_BLINK (0x0202)
#define MRVL_88E1548_LED_POLARITY (0x8844)
#define MRVL_88E1548_GE_ON_SFP_OFF (0x8201)
#define MRVL_88E1548_GE_OFF_SFP_ON (0x0281)

#define MRVL_88E1548_LED0_ON            (0x0009)
#define MRVL_88E1548_LED1_ON            (0x0090)
#define MRVL_88E1548_LED2_ON            (0x0900)
#define MRVL_88E1548_LED3_ON            (0x9000)

#define MRVL_88E1548_LED0_OFF           (0x0008)
#define MRVL_88E1548_LED1_OFF           (0x0080)
#define MRVL_88E1548_LED2_OFF           (0x0800)
#define MRVL_88E1548_LED3_OFF           (0x8000)

/************************* 88E1548L ************************/
#define MRV88E1548L_REG_PAGE_0                   0

#define MRV88E1548L_REG_PAGE_1                   1
#define MRV88E1548L_FIBER_CTRL_REG               0
#define MRV88E1548L_FIBER_SPECIFIC_CTRL_REG               0x10
#define MRV88E1548L_FIBER_POWER_DOWN             (1 << 11)
#define MRV88E1548L_PHY1_FIBER_PORT_2    2
#define MRV88E1548L_PHY1_FIBER_PORT_3    3

#define MRV88E1548L_REG_PAGE_2                   2

#define MRV88E1548L_REG_PAGE_3                   3

#define MRV88E1548L_REG_PAGE_4                   4

#define MRV88E1548L_REG_PAGE_5                   5

#define MRV88E1548L_REG_PAGE_6                   6

#define MRV88E1548L_REG_PAGE_7                   7

#define MRV88E1548L_REG_PAGE_8                   8

#define MRV88E1548L_REG_PAGE_9                   9

#define MRV88E1548L_REG_PAGE_12                  12

#define MRV88E1548L_REG_PAGE_14                  14

#define MRV88E1548L_REG_PAGE_16                  16

#define MRV88E1548L_REG_PAGE_18                  18
#define MRV88E1548L_PACKET_GEN                   0x10
#define MRV88E1548L_CRC_COUNTERS                 0x11
#define MRV88E1548L_CHECKER_CTRL_REG             0x12
#define MRV88E1548L_CRC_COUNTERS_RESET           (1 << 4)
#define MRV88E1548L_EN_CRC_CHECKER               0x4

#define MRV88E1548L_REG_PAGE_20                  20
#define MRV88E1548L_REG_PAGE_26                  26
#define MRV88E1548L_REG_PAGE_27                  27
#define MRV88E1548L_REG_PAGE_29                  29

#define MRV88E1548L_REG_PAGE_254                 254

#define MRV88E1548L_REG_PAGE_MAX                 255

/* Page 0 Register Offsets - Copper */
#define MRV88E1548L_PAGE_ADDRESS_REG             22

/* Page 3 Register Offsets */
#define MRV88E1548L_LED_CTRL_REG                 16
#define MRV88E1548L_LED_POLARITY_CTRL_REG        17

/* Page 16 Register Offsets */
#define MRV88E1548L_PTP_READ_ADDR                0
#define MRV88E1548L_PTP_WRITE_ADDR               1
#define MRV88E1548L_PTP_DATA_LO                  2
#define MRV88E1548L_PTP_DATA_HI                  3

/* Page 18 Register Offsets */
#define MRV88E1548L_GEN_CTRL_REG2                27

/* TOD Alignment Clock Cycle */
#define MRV88E1548L_CLOCK_CYC                    (0x232C)

/* TAI Clock Configurations */
#define MRV88E1548L_TOD_CFG_GEN                  (0x230B)

/* TOD/Pulse-Trigger Functions */
#define MRV88E1548L_TOD_FUNC_CFG                 (0x2323)

/* Pulse-Trigger Generation Mask */
#define MRV88E1548L_TRIG_GEN_MASK0               (0x2311)
#define MRV88E1548L_TRIG_GEN_MASK1               (0x2312)
#define MRV88E1548L_TRIG_GEN_MASK2               (0x2313)
#define MRV88E1548L_TRIG_GEN_MASK3               (0x2314)

/* Pulse-Trigger Generation */
#define MRV88E1548L_TRIG_GEN_TOD0                (0x230D)
#define MRV88E1548L_TRIG_GEN_TOD1                (0x230E)
#define MRV88E1548L_TRIG_GEN_TOD2                (0x230F)
#define MRV88E1548L_TRIG_GEN_TOD3                (0x2310)

/* extern function declaration */
int get_phy_port(int, int); 
int get_88e1548_bus_id(int);
int get_88e1548_phy_addr(int, int);
int get_88e1548_4ge_phy_addr(int, int);
int verify_1548_drift_adjustment_mode(void);
extern int verify_1548_clk_trig_in(void);
extern int config_1548_gen_clk_out(void);
extern int config_1548_gen_trig_out(void);
extern int check_1548_gen_clk_out_config(void);
extern int check_1548_gen_trig_out_config(int);

extern int diag_88e1548_init(void);

typedef unsigned char Enumeration8; /**< Local definition of 8  bit enumerated value */

/* L2 Ethernet Header */
typedef struct ether_header_t_ {
    uint8   dst_mac[6];
    uint8   src_mac[6];
    uint16  ether_type;
} ether_header_t;

typedef struct 
{
  unsigned short epoch_number;
  unsigned int seconds;
  signed int  nanoseconds;  
} V2TimeRepresentation;

typedef struct 
{
  unsigned char        clockClass;
  Enumeration8     clockAccuracy;
  unsigned short       offsetScaledLogVariance;
} ClockQuality;

typedef struct 
{
  char            clockIdentity[8];
  unsigned short       portNumber;
} volatile PortIdentity;

/** PTP Version 2 Announce message structure */
typedef struct 
{
  V2TimeRepresentation originTimestamp;           
  signed short            currentUTCOffset;            
  unsigned char            reserved;                    
  unsigned char            grandmasterPriority1;          
  ClockQuality         grandmasterClockQuality;     
  unsigned char            grandmasterPriority2;        
  char                grandmasterIdentity[8];      
  unsigned short           stepsRemoved;                 
  Enumeration8         timeSource;                   

/* *Note: stepsRemoved is a 16 bit field, but it is 
 * not 16 bit aligned in the announce message
 */
} MsgAnnounce;

/** IEEE 1588 and IEEE 802.1AS PTP Version 2 common Message header structure */
typedef struct 
{
  unsigned char transportSpecificAndMessageType;       // 00       1 (2 4-bit fields)
  unsigned char reserved1AndVersionPTP;                // 01       1 (2 4-bit fields)
  unsigned short messageLength;                         // 02       2
  unsigned char    domainNumber;                          // 04       1
  unsigned char    reserved2;                             // 05       1
  char        flags[2];                              // 06       2
  signed long long    correctionField;                       // 08       8
  unsigned int   reserved3;                             // 16       4
  PortIdentity sourcePortId;                          // 20      10
  unsigned short   sequenceId;                            // 30       2
  unsigned char    control;                               // 32       1
  unsigned char    logMeanMessageInterval;                // 33       1
} volatile V2MsgHeader;

#define PTP_MESSAGE_TYPE (0x0) 
#define PTP_VERSION    (0x2)
#define PTP_PACKET_LENGTH (0x40)
#define PTP_MESSAGE_LENGTH (0x34)
#define PTP_ETHERNET_TYPE (0x88f7)
#define PTP_PKT_CMP_START_LEN_1 (0x0)
#define PTP_PKT_CMP_END_LEN_1 (0xb)
#define PTP_PKT_CMP_START_LEN_2 (0x30)
#define PTP_PKT_CMP_END_LEN_2 (0x39)
#define PTP_PKT_LEN_BIT_31_MASK (0x80000000)

#define PTP_CFG_GEN_INRESS  (0x3400)
#define PTP_CFG_GEN_EGRESS  (0x3000)
#define PTP_READ_ADDRESS      (0x0)
#define PTP_WRITE_ADDRESS     (0x1)
#define PTP_DATA_LO           (0x2)
#define PTP_DATA_HI           (0x3)
#define PTP_VERIFY_NUM   (3)
#define WAIT_PHY_READY   (100) 
#define PTP_INIT_PULSE_IN_CNT   (0xff) 
#define PTP_PULSE_IN_CNT_FULL    (0xff)
#define PTP_INIT_CLOCK_IN_CNT    (0xff)
#define PTP_CLOCK_IN_CNT_FULL    (0xffff)
#define TRIG_IN_CNT_REG          (0x2322)
#define CLOCK_IN_CNT_REG         (0x232e)
#define TRIG_VERIFY_TIME         (10)
#define CLK_VERIFY_TIME          (10)
#define TRIG_VERIFY_NUM          (3)
#define CLK_VERIFY_NUM           (3)
#define PTP_CNT_DELAY            (1000)
#define PTP_CONFIG_DELAY         (1000)
#define PTP_READ_DELAY           (500)

extern void disable_ptp_engine(int);
#endif
/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1548L_lib.h,v $
 * Revision 1.3  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 *
 * Revision 1.2.8.3  2014/05/02 02:43:21  kodko
 * Modify the verify time from 10000 to 10.
 *
 * Revision 1.2.8.2  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.2.8.1  2014/03/11 02:27:58  leschen
 * Add macros to support 1588 [Cclk/trig verificatoin.
 *
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.3  2013/09/05 06:10:18  leschen
 * Modify PTP payload check length
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/08/06 09:32:29  leschen
 * Define PTP related macros and structs.
 *
 * Revision 1.1.2.2  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.1  2013/04/24 10:37:17  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/19 09:51:24  kuangik
 * Add retry mechanism (ported from O2) and reset quad phy if the test fails
 *
 * Revision 1.14  2013/02/19 08:40:07  leslie
 * Declare extern functions
 *
 * Revision 1.13  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.12  2012/12/11 01:57:19  leslie
 * Fix for fiber 88E1548L platform loopback test.
 *
 * Revision 1.11  2012/10/24 10:33:05  leslie
 * Update for 88E1548 test item.
 *
 * Revision 1.10  2012/10/18 12:55:02  kody
 * Add 88E1548L fiber line loopback between platform side.
 *
 * Revision 1.9  2012/10/08 09:54:04  leslie
 * Add PHY map table.
 *
 * Revision 1.8  2012/09/21 10:57:35  leslie
 * Add macros for test mode lib.
 *
 * Revision 1.7  2012/09/05 22:52:31  kody
 * Fix the incorrect struct name for 1548.
 *
 * Revision 1.6  2012/08/27 06:45:09  evanli
 * add MRV88E1548L_PAGE_ADDRESS_REG variable
 *
 * Revision 1.5  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/06/12 02:39:57  leslie
 * Update for 88E1548 test item
 *
 * Revision 1.2  2012/04/06 06:07:18  kuangik
 * Update for 88E1548 Test Item
 *
 * Revision 1.1  2012/02/10 06:58:25  leslie
 * Add Woodlawn phy 88E1548L lib header file.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
