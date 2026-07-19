/* $Id: dsl_libs.h,v 1.5 2018/05/15 09:37:32 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/dsl_libs.h,v $ 
 *------------------------------------------------------------------
 * 
 * dsl_libs.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DSL_LIBS_H_
#define _DSL_LIBS_H_
#include "dsl_mib_defs.h"

#define ATM                             0
#define MII                             2

#define  QN_TO_STR_STR_LEN             32
#define  RET_STR_SIZE                  20
#define  TMP_STR_SIZE                  80
#define abs(n)   ( ((n) ^ ((n) >> 31)) - ((n) >> 31) )
#define DEC_POINT(n) (n) < 0 ? '-' : ' ', (int)(abs(n) / 10), (int)(abs(n) % 10)

#define BCM_DSL_LINE_0                 0
#define BCM_DSL_LINE_1                 1
#define BCM_DSL_LINE_BONDING           2

#define SEND_ALL_TONE_DELAY            5000
#define WAIT_SINGLE_SHOWTIME           6000
#define WAIT_BONDING_SHOWTIME          12000
#define GET_LINE_DELAY                 500
#define SHOWTIME_RETRY_TIMES           1
#define WAIT_BCM_RECONFIG              100

/* ADLS */
#define ANSI_T1_413_ID      0x00 /* ANSI T1.413 1998 */
#define ITU_G_992_1A_ID     0x01 /* G.992.1, Annex A */
#define ITU_G_992_3A_ID     0x02 /* G.992.3, Annex A */
#define ITU_G_992_5A_ID     0x03 /* G.992.5, Annex A */
#define ITU_G_992_3L_ID     0x04 /* G.992.3, Annex L */
#define ITU_G_992_3M_ID     0x05 /* G.992.3, Annex M */
#define ITU_G_992_5M_ID     0x06 /* G.992.5, Annex M */
#define ITU_G_992_1B_ID     0x07 /* G.992.1, Annex B */
#define ITU_G_992_3B_ID     0x08 /* G.992.3, Annex B */
#define ITU_G_992_5B_ID     0x09 /* G.992.5, Annex B */
#define ITU_G_992_3J_ID     0x0c /* G.992.3, Annex J */
#define ITU_G_992_5J_ID     0x0d /* G.992.5, Annex J */

/* VDSL */
#define ITU_G_993_1_ID      0x0a /* VDSL1 */
#define ITU_G_993_2_ID      0x0b /* VDSL2 */

#define INVALID_MODE        0xFF

typedef enum BcmAdslOpNum
{
	ANSI_T1_413 = 1,
	ITU_G_992_1A,
	ITU_G_992_3A,
	ITU_G_992_5A,
	ITU_G_992_3L,
	ITU_G_992_3M,
	ITU_G_992_5M,
	ITU_G_992_1B,
	ITU_G_992_3B,
	ITU_G_992_5B,
	ITU_G_993_1,
	ITU_G_993_2,
	ITU_G_992_3J,
	ITU_G_992_5J
} BCM_ADSL_OP_Num;

#define SET_OP_MODE_PARAMETER   4
#define ADSL_CONFIG_ITEMS   11

#define IDLE_LISTEN_TIME    25    /* Elapse time in s */
#define IDLE_LISTEN_FREQ    100  /* Frequencies in ms */

#define kAdslVersionStringSize  32

typedef struct bcm_id_string_t
{
    unsigned int    code; 
    unsigned char   *name;
} bcm_id_string_t;


typedef struct DDRControl {
    uint32    RevID;            /* 00 */
    uint32    PadSSTLMode;      /* 04 */
    uint32    CmdPadCntl;       /* 08 */
    uint32    DQPadCntl;        /* 0c */
    uint32    DQSPadCntl;       /* 10 */
    uint32    ClkPadCntl0;      /* 14 */
    uint32    MIPSDDRPLLCntl0;  /* 18 */
    uint32    MIPSDDRPLLCntl1;  /* 1c */
    uint32    MIPSDDRPLLConfig; /* 20 */
    uint32    MIPSDDRPLLMDiv;   /* 24 */
    uint32    DSLCorePhaseCntl; /* 28 */
    uint32    DSLCpuPhaseCntr;  /* 2c */
    uint32    MIPSPhaseCntl;    /* 30 */
    uint32    DDR1_2PhaseCntl0; /* 34 */
    uint32    DDR3_4PhaseCntl0; /* 38 */
    uint32    VCDLPhaseCntl0;   /* 3c */
    uint32    VCDLPhaseCntl1;   /* 40 */
    uint32    WSliceCntl;       /* 44 */
    uint32    DeskewDLLCntl;    /* 48 */
    uint32    DeskewDLLReset;   /* 4c */
    uint32    DeskewDLLPhase;   /* 50 */
    uint32    AnalogTestCntl;   /* 54 */
    uint32    RdDQSGateCntl;    /* 58 */
    uint32    PLLTestReg;       /* 5c */
    uint32    Spare0;           /* 60 */
    uint32    Spare1;           /* 64 */
    uint32    Spare2;           /* 68 */
    uint32    CLBist;           /* 6c */
    uint32    LBistCRC;         /* 70 */
    uint32    UBUSPhaseCntl;    /* 74 */
    uint32    UBUSPIDeskewLLMB0; /* 78 */
    uint32    UBUSPIDeskewLLMB1; /* 7C */

} DDRControl;


/* From bcmadsl.h */
// Return status values
typedef enum BcmAdslStatus
{
    BCMADSL_STATUS_SUCCESS = 0,
    BCMADSL_STATUS_ERROR
} BCMADSL_STATUS;

// Return status values
typedef enum AdslLinkState
{
    BCM_ADSL_LINK_UP = 0,
    BCM_ADSL_LINK_DOWN,
    BCM_ADSL_TRAINING_G992_EXCHANGE,
    BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS,
    BCM_ADSL_TRAINING_G992_STARTED,
    BCM_ADSL_TRAINING_G993_EXCHANGE,
    BCM_ADSL_TRAINING_G993_CHANNEL_ANALYSIS,
    BCM_ADSL_TRAINING_G993_STARTED,        
    BCM_ADSL_TRAINING_G994,
    BCM_ADSL_G994_NONSTDINFO_RECEIVED,
    BCM_ADSL_BERT_COMPLETE,
    BCM_ADSL_ATM_IDLE,
    BCM_ADSL_EVENT,
    BCM_ADSL_G997_FRAME_RECEIVED,
    BCM_ADSL_G997_FRAME_SENT
} ADSL_LINK_STATE;

#ifndef DISABLE_ADSL_OLD_DEF
#define    ADSL_LINK_UP        BCM_ADSL_LINK_UP
#define    ADSL_LINK_DOWN        BCM_ADSL_LINK_DOWN
#endif

/* ADSL test modes */
typedef enum AdslTestMode
{
    ADSL_TEST_NORMAL = 0,
    ADSL_TEST_REVERB,
    ADSL_TEST_MEDLEY,
    ADSL_TEST_SELECT_TONES,
    ADSL_TEST_NO_AUTO_RETRAIN,
    ADSL_TEST_MARGIN_TWEAK,
    ADSL_TEST_ESTIMATE_PLL_PHASE,
    ADSL_TEST_REPORT_PLL_PHASE_STATUS,
    ADSL_TEST_AFELOOPBACK,
    ADSL_TEST_L3,
    ADSL_TEST_DIAGMODE,
    ADSL_TEST_L0,
    ADSL_TEST_FREEZE_REVERB = 20,
    ADSL_TEST_FREEZE_MEDLEY
} ADSL_TEST_MODE;

/* VDSL test modes */
typedef enum VdslTestMode
{
    VDSL_TEST_NORMAL = 0,
    VDSL_TEST_REVERB,
    VDSL_TEST_MEDLEY,
    VDSL_TEST_SELECT_TONES,
    VDSL_TEST_NO_AUTO_RETRAIN,
    VDSL_TEST_MARGIN_TWEAK,
    VDSL_TEST_ESTIMATE_PLL_PHASE,
    VDSL_TEST_REPORT_PLL_PHASE_STATUS,
    VDSL_TEST_AFELOOPBACK,
    VDSL_TEST_L3,
    VDSL_TEST_DIAGMODE,
    VDSL_TEST_L0
} VDSL_TEST_MODE;

typedef struct _adsl_version_info {
    unsigned short   phyType;
    unsigned short   phyMjVerNum;
    unsigned short   phyMnVerNum;
    char                  phyVerStr[kAdslVersionStringSize];
    unsigned short   drvMjVerNum;
    unsigned short   drvMnVerNum;
    char                  drvVerStr[kAdslVersionStringSize];
    unsigned short   ciscoMjVerNum;   // Major
    unsigned short   ciscoMnVerNum;   // Minor
    unsigned short   ciscoRelVerNum;  // Release 
} adsl_version_info_t;

typedef struct _adsl_cfg_profile {
    long adslAnnexCParam;
    long adslAnnexAParam;
    long adslTrainingMarginQ4;
    long adslShowtimeMarginQ4;
    long adslLOMTimeThldSec;
    long adslDemodCapMask;
    long adslDemodCapValue;
    long adsl2Param;
    long adslPwmSyncClockFreq;
    long adslHsModeSwitchTime;
    long adslDemodCap2Mask;
    long adslDemodCap2Value;
    long vdslParam;
    long vdslParam1;    
} adsl_cfg_profile_t; 


// ADSL_CONNECTION_INFO Contains ADSL Connection Info
typedef struct _adsl_connection_info 
{
    ADSL_LINK_STATE LinkState; 
    unsigned long ShowtimeStart;
    uint32 ulFastUpStreamRate;
    uint32 ulFastDnStreamRate;
    uint32 ulInterleavedUpStreamRate;
    uint32 ulInterleavedDnStreamRate;
} adsl_connection_info_t;

extern int bcm63168_check_sku_type(void);
extern int bcm63168_set_line_id(void);
extern int bcm63168_initialize (void);
extern int bcm63168_uninitialize (void);
extern int bcm63168_configure (void);
extern int bcm63168_get_configure (void);
extern int bcm63168_get_version (void);
extern int bcm63168_connection_start (void);
extern int bcm63168_do_showtime(void);
extern int bcm63168_connection_stop (void);
extern int bcm63168_get_conn_info (void);
extern int bcm63168_set_tone (void);
extern int bcm63168_send_all_tone (void);
extern int bcm63168_idle_listen (void);
extern int bcm63168_get_idle_listen_result (void);
extern int bcm63168_showtime_test(void);
extern int bcm63168_get_adslmib_info(void);
extern int bcm63168_get_xtm_bonding_info (void);
extern int bcm63168_get_xdsl_info (void);
extern int bcm63168_showtime_no_retrain(void);
extern int bcm63168_get_line_mode (void);
extern int bcm63168_vdsl_test_option_select (void);
extern int bcm63168_set_test_mode (void);
extern int bcm63168_volt_normal (void);
extern int bcm63168_volt_high (void);
extern int bcm63168_volt_low (void);
extern int bcm63168_led_test (void);
extern int bcm63168_led_utils (int);
extern int bcm63168_show_profile (void);
extern int bcm63168_show_spi_flash_reg(void);
extern int bcm63168_en_wp_spi_flash_reg(void);
extern int bcm63168_dis_wp_spi_flash_reg(void);
extern char* QnToString1(long , int , int );
extern int bcm63138_set_gfast_relay(void);
extern void msleep(int );
#endif
/*-------------------------------------------------
$Log: dsl_libs.h,v $
Revision 1.5  2018/05/15 09:37:32  steja
CSCvj38863: Enhanced LED single test utility

Revision 1.4  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.3  2017/08/25 10:03:57  steja
1.Add Utility to restore back CFE IOS parameter for DF site(CSCvf70937)
2.Add Utility for SPI Write protect

Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:02  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:04  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2.4.2  2017/09/14 03:26:30  steja
1. Support Utility for CFE SPI write protect.
2. Fix CFE restore verification bug.

Revision 1.1.4.2.4.1  2017/05/03 02:07:11  steja
Add Relay pin switch utilities

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk


*/
