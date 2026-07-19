/* $Id: bcm63268_lib.h,v 1.3 2015/02/13 12:26:49 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/goldschlager/bcm63268_lib.h,v $
 *------------------------------------------------------------------------------
 *
 * bcm63268_lib.h: Definition file for communicate with bcm63268
 *
 * Oct. 2013 - James Lin
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */ 
#ifndef _BCM63268_LIB_H_
#define _BCM63268_LIB_H_
#include "bcm63268_adslmib_def.h"

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

/* ADLS */
#define ANSI_T1_413_ID      0x00 /* ANSI T1.413 1998 */
#define ITU_G_992_1A_ID     0x01 /* G992.1 Annex A */
#define ITU_G_992_3A_ID     0x02 /* G.992.3, Annex A */
#define ITU_G_992_5A_ID     0x03 /* G.992.5, Annex A */
#define ITU_G_992_3L_ID     0x04 /* G.992.3, Annex L */
#define ITU_G_992_3M_ID     0x05 /* G.992.3, Annex M */
#define ITU_G_992_5M_ID     0x06 /* G.992.5, Annex M */
#define ITU_G_992_1B_ID     0x07 /* G992.1 Annex B */
#define ITU_G_992_3B_ID     0x08 /* G.992.3, Annex B */
#define ITU_G_992_5B_ID     0x09 /* G.992.5, Annex B */
#define ITU_G_992_3J_ID     0x0c /* G.992.3, Annex J */
#define ITU_G_992_5J_ID     0x0d /* G.992.5, Annex J */

/* VDSL */
#define ITU_G_993_1_ID      0x0a /* VDSL1 */
#define ITU_G_993_2_ID      0x0b /* VDSL2 */

#define INVALID_MODE        0xFF

/* Goldschlager SKUs */
#define GS_NIM_VAB_A        0xC42A
#define GS_NIM_VA_B         0xC42B
#define GS_NIM_VAB_M        0xC42C

#define SET_OP_MODE_PARAMETER   4
#define ADSL_CONFIG_ITEMS   11

#define IDLE_LISTEN_TIME    25    /* Elapse time in s */
#define IDLE_LISTEN_FREQ    1000    /* Frequencies in ms */

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

extern int bcm63268_check_sku_type(ngio_if *);
extern int bcm63268_set_line_id(ngio_if *);
extern int bcm63268_initialize (ngio_if *);
extern int bcm63268_uninitialize (ngio_if *);
extern int bcm63268_configure (ngio_if *);
extern int bcm63268_get_configure (ngio_if *); 
extern int bcm63268_get_version (ngio_if *);
extern int bcm63268_connection_start (ngio_if *);
extern int bcm63268_do_showtime(ngio_if *);
extern int bcm63268_connection_stop (ngio_if *);
extern int bcm63268_get_conn_info (ngio_if *);
extern int bcm63268_set_tone (ngio_if *);
extern int bcm63268_send_all_tone (ngio_if *);
extern int bcm63268_idle_listen (ngio_if *);
extern int bcm63268_get_idle_listen_result (ngio_if *);
extern int bcm63268_showtime(ngio_if * );
extern int bcm63268_get_adslmib_info(ngio_if * );
extern int bcm63268_get_xtm_bonding_info (ngio_if *);
extern int bcm63268_get_xdsl_info (ngio_if *);
extern int bcm63268_showtime_no_retrain(ngio_if *);
extern int bcm63268_get_line_mode (ngio_if *);
extern int bcm63268_vdsl_test_option_select (ngio_if *);
extern int bcm63268_set_test_mode (ngio_if *, ushort );
extern int bcm63268_volt_normal (ngio_if *);
extern int bcm63268_volt_high (ngio_if *);
extern int bcm63268_volt_low (ngio_if *);
extern int bcm63268_led_test (ngio_if *);
extern int bcm63268_show_profile (ngio_if *);
extern int bcm63268_show_spi_flash_reg (ngio_if *);
extern char* QnToString1(long , int , int );
extern void msleep(int );
#endif


/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: bcm63268_lib.h,v $
 * Revision 1.3  2015/02/13 12:26:49  meho
 * Added the utility to read Boardcom SPI flash registers
 *
 * Revision 1.2  2014/09/17 03:32:16  jamlin
 * Add support for Goldschlager NIM.
 *
 * Revision 1.1.6.2  2014/08/08 02:43:57  jamlin
 * goladschlager-branch3 initail commit.
 *
 * Revision 1.1.4.6  2014/04/11 03:50:04  jamlin
 * GS annexB PID changes from NIM-VAB-B tp NIM-VA-B
 *
 * Revision 1.1.4.5  2014/02/10 04:17:03  jamlin
 * added get_xdsl_profile function
 *
 * Revision 1.1.4.4  2014/02/10 04:01:11  jamlin
 * rename nc_dispatch_linkstatus to nc_dispatch_return_value function and added check_sku_type function
 *
 * Revision 1.1.4.3  2014/02/10 03:32:21  jamlin
 * added bcm_bonding_state_get function and fixed showtime bonding issue
 *
 * Revision 1.1.4.2  2014/01/07 01:54:52  jamlin
 * Goldschlager new branch goldschlager-branch2
 *
 * Revision 1.1.2.4  2013/12/04 01:38:52  jamlin
 * Support Bonding channels showtime status display.
 *
 * Revision 1.1.2.3  2013/11/22 14:09:37  jamlin
 * optimize test options in parametric test.
 *
 * Revision 1.1.2.2  2013/11/22 12:50:21  jamlin
 * Add PID check to differentiate GS SKUs.
 *
 * Revision 1.1.2.1  2013/11/02 13:39:51  jamlin
 * Initial commit for bringup.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

