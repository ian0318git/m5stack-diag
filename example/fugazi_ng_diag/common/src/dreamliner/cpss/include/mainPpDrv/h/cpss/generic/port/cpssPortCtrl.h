/* $Id: cpssPortCtrl.h,v 1.1 2015/02/13 11:34:13 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/cpssPortCtrl.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPortCtrl.h
*
* DESCRIPTION:
*       CPSS definitions for port configurations.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssPortCtrlh
#define __cpssPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

/*
 * typedef: enum CPSS_PORT_DUPLEX_ENT
 *
 * Description: Enumeration of port duplex modes
 *
 * Enumerations:
 *    CPSS_PORT_FULL_DUPLEX_E - full duplex mode
 *    CPSS_PORT_HALF_DUPLEX_E - half duplex mode
 */
typedef enum
{
    CPSS_PORT_FULL_DUPLEX_E,
    CPSS_PORT_HALF_DUPLEX_E

}CPSS_PORT_DUPLEX_ENT;

/*
 * typedef: enum CPSS_PORT_INTERFACE_MODE_ENT
 *
 * Description: Enumeration of port interface modes
 *
 * Enumerations:
 *    CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E  - Reduced 10-bit interface mode
 *    CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E   - Reduced GMII interface mode
 *    CPSS_PORT_INTERFACE_MODE_MII_E            - MII interface mode
 *    CPSS_PORT_INTERFACE_MODE_SGMII_E          - SGMII interface mode
 *    CPSS_PORT_INTERFACE_MODE_XGMII_E          - XGMII interface mode (aka XAUI, aka KX4)
 *    CPSS_PORT_INTERFACE_MODE_MGMII_E          - MGII interface mode
 *    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E     - 1000 BASE-X interface mode
 *    CPSS_PORT_INTERFACE_MODE_GMII_E           - GMII interface mode
 *    CPSS_PORT_INTERFACE_MODE_MII_PHY_E        - MII PHY interface mode
 *    CPSS_PORT_INTERFACE_MODE_QX_E             - Quarter X interface mode
 *    CPSS_PORT_INTERFACE_MODE_HX_E             - Half X interface mode (aka KX2)
 *    CPSS_PORT_INTERFACE_MODE_RXAUI_E          - RXAUI interface mode
 *    CPSS_PORT_INTERFACE_MODE_100BASE_FX_E     - 100 BASE-FX interface mode
 *    CPSS_PORT_INTERFACE_MODE_QSGMII_E         - Quad SGMII interface mode
 *    CPSS_PORT_INTERFACE_MODE_XLG_E            - XLG (40G) interface mode
 *    CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E    - XGMII interface mode (aka XAUI),
 *      this mode should be defined on special ports which serdeses occupied for 
 *      traffic forwarding by neighbor ports, but they need to get from them clock of 
 *      specific frequency. This mode enables to protect serdeses of port where it defined
 *      from reset and enables to check compatibility of clock frequency required by this 
 *      port and used by neigbor port that occupied its serdeses for his traffic.
 * 
 *    CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E - similar to "LOCAL_XGMII" above, 
 *      but more generic; real interface mode of port is board dependent: for 
 *      Lion2 - SR_LR. Purposed for "relay" ports using MAC loopback.
 * 
 *    CPSS_PORT_INTERFACE_MODE_KR_E             - KR i/f mode with 1/2/4/10 serdeses (FEC enabled)
 *    CPSS_PORT_INTERFACE_MODE_HGL_E            - HGL i/f mode 6 serdeses - 16G with serdes speed 3.333G, 
 *                                                  15G with 3.125; 4@12.5 - 40G; 10@12.5 - 100G
 *    CPSS_PORT_INTERFACE_MODE_CHGL_12_E        - CHGL i/f mode 12 serdeses*10.3125G - 100G
 *    CPSS_PORT_INTERFACE_MODE_ILKN12_E         - Interlaken Alliance i/f mode 12 serdeses
 *    CPSS_PORT_INTERFACE_MODE_ILKN16_E         - Interlaken Alliance i/f mode 16 serdeses
 *    CPSS_PORT_INTERFACE_MODE_ILKN24_E         - Interlaken Alliance i/f mode 24 serdeses
 *    CPSS_PORT_INTERFACE_MODE_SR_LR_E          - short/long reach i/f mode with 10.3125@1/2/4 serdeses 
 *                                                  (FEC disabled); specific for short and long reach
 *                                                      configurations done by auto-tuning
 *
 *    CPSS_PORT_INTERFACE_MODE_NA_E             - Interface mode is Not
 *                                                Applicable
 */
typedef enum
{
    CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E,   /* 0 */
    CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,    /* 1 */
    CPSS_PORT_INTERFACE_MODE_MII_E,             /* 2 */
    CPSS_PORT_INTERFACE_MODE_SGMII_E,           /* 3 */
    CPSS_PORT_INTERFACE_MODE_XGMII_E,           /* 4 */
    CPSS_PORT_INTERFACE_MODE_MGMII_E,           /* 5 */
    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,      /* 6 */
    CPSS_PORT_INTERFACE_MODE_GMII_E,            /* 7 */
    CPSS_PORT_INTERFACE_MODE_MII_PHY_E,         /* 8 */
    CPSS_PORT_INTERFACE_MODE_QX_E,              /* 9 */
    CPSS_PORT_INTERFACE_MODE_HX_E,              /* 10 */
    CPSS_PORT_INTERFACE_MODE_RXAUI_E,           /* 11 */
    CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,      /* 12 */
    CPSS_PORT_INTERFACE_MODE_QSGMII_E,          /* 13 */
    CPSS_PORT_INTERFACE_MODE_XLG_E,             /* 14 */
    CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,     /* 15 */
    CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E = 
                                        CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,

    CPSS_PORT_INTERFACE_MODE_KR_E,              /* 16 */
    CPSS_PORT_INTERFACE_MODE_HGL_E,             /* 17 */
    CPSS_PORT_INTERFACE_MODE_CHGL_12_E,         /* 18 */
    CPSS_PORT_INTERFACE_MODE_ILKN12_E,          /* 19 */
    CPSS_PORT_INTERFACE_MODE_SR_LR_E,           /* 20 */
    CPSS_PORT_INTERFACE_MODE_ILKN16_E,          /* 21 */
    CPSS_PORT_INTERFACE_MODE_ILKN24_E,          /* 22 */

    CPSS_PORT_INTERFACE_MODE_NA_E               /* 23 */

}CPSS_PORT_INTERFACE_MODE_ENT;

/*
 * typedef: enum CPSS_PORT_SPEED_ENT
 *
 * Description: Enumeration of port speeds
 *
 * Enumerations:
 *    CPSS_PORT_SPEED_10_E    - 10 Mbps
 *    CPSS_PORT_SPEED_100_E   - 100 Mbps
 *    CPSS_PORT_SPEED_1000_E  - 1 Gbps
 *    CPSS_PORT_SPEED_10000_E - 10 Gbps
 *    CPSS_PORT_SPEED_12000_E - 12 Gbps
 *    CPSS_PORT_SPEED_2500_E  - 2.5 Gbps
 *    CPSS_PORT_SPEED_5000_E  - 5 Gbps
 *    CPSS_PORT_SPEED_13600_E - 13.6 Gbps
 *    CPSS_PORT_SPEED_20000_E - 20 Gbps
 *    CPSS_PORT_SPEED_40000_E - 40 Gbps
 *    CPSS_PORT_SPEED_16000_E - 16 Gbps(XGMII@5G;HGL@3.333)
 *    CPSS_PORT_SPEED_15000_E - 15 Gbps(HGL@3.125)
 *    CPSS_PORT_SPEED_75000_E - 75 Gbps
 *    CPSS_PORT_SPEED_100G_E  - 100 Gbps
 *    CPSS_PORT_SPEED_140G_E  - 140 Gbps
 *
 *    CPSS_PORT_SPEED_NA_E    - for given port parameter speed isn't applicable
 */
typedef enum
{
    CPSS_PORT_SPEED_10_E,       /* 0 */ 
    CPSS_PORT_SPEED_100_E,      /* 1 */ 
    CPSS_PORT_SPEED_1000_E,     /* 2 */ 
    CPSS_PORT_SPEED_10000_E,    /* 3 */ 
    CPSS_PORT_SPEED_12000_E,    /* 4 */ 
    CPSS_PORT_SPEED_2500_E,     /* 5 */ 
    CPSS_PORT_SPEED_5000_E,     /* 6 */ 
    CPSS_PORT_SPEED_13600_E,    /* 7 */ 
    CPSS_PORT_SPEED_20000_E,    /* 8 */ 
    CPSS_PORT_SPEED_40000_E,    /* 9 */ 
    CPSS_PORT_SPEED_16000_E,    /* 10 */
    CPSS_PORT_SPEED_15000_E,    /* 11 */
    CPSS_PORT_SPEED_75000_E,    /* 12 */
    CPSS_PORT_SPEED_100G_E,     /* 13 */
    CPSS_PORT_SPEED_50000_E,    /* 14 */
    CPSS_PORT_SPEED_140G_E,     /* 15 */
                                
    CPSS_PORT_SPEED_NA_E        /* 16 */

}CPSS_PORT_SPEED_ENT;


/*
 * typedef: struct CPSS_PORT_ATTRIBUTES_STC
 *
 * Description: port attributes
 *
 * Fields:
 *  portLinkUp      - Is link up
 *  portSpeed       - Port speed
 *  portDuplexity   - Duplex mode
 *
 */
typedef struct
{
    GT_BOOL              portLinkUp;
    CPSS_PORT_SPEED_ENT  portSpeed;
    CPSS_PORT_DUPLEX_ENT portDuplexity;
} CPSS_PORT_ATTRIBUTES_STC;

/*
 * typedef: enum CPSS_PORT_MRU_SIZE_ENT
 *
 * Description: Enumeration of Maximal Receive packet sizes
 *
 * Enumerations:
 *    CPSS_PORT_MRU_1518_E  - Accepts packets up to 1518  bytes in length
 *    CPSS_PORT_MRU_1522_E  - Accepts packets up to 1522  bytes in length
 *    CPSS_PORT_MRU_1526_E  - Accepts packets up to 1526  bytes in length
 *    CPSS_PORT_MRU_1536_E  - Accepts packets up to 1536  bytes in length
 *    CPSS_PORT_MRU_1552_E  - Accepts packets up to 1552  bytes in length
 *    CPSS_PORT_MRU_1632_E  - Accepts packets up to 1632  bytes in length
 *    CPSS_PORT_MRU_1664_E  - Accepts packets up to 1664  bytes in length
 *    CPSS_PORT_MRU_2048_E  - Accepts packets up to 2048  bytes in length
 *    CPSS_PORT_MRU_9022_E  - Accepts packets up to 9022  bytes in length
 *    CPSS_PORT_MRU_9192_E  - Accepts packets up to 9192  bytes in length
 *    CPSS_PORT_MRU_9216_E  - Accepts packets up to 9216  bytes in length
 *    CPSS_PORT_MRU_9220_E  - Accepts packets up to 9220  bytes in length
 *    CPSS_PORT_MRU_10240_E - Accepts packets up to 10240 bytes in length
 */
typedef enum
{
    CPSS_PORT_MRU_1518_E = 0,
    CPSS_PORT_MRU_1522_E,
    CPSS_PORT_MRU_1526_E,
    CPSS_PORT_MRU_1536_E,
    CPSS_PORT_MRU_1552_E,
    CPSS_PORT_MRU_1632_E,
    CPSS_PORT_MRU_1664_E,
    CPSS_PORT_MRU_2048_E,
    CPSS_PORT_MRU_9022_E,
    CPSS_PORT_MRU_9192_E,
    CPSS_PORT_MRU_9216_E,
    CPSS_PORT_MRU_9220_E,
    CPSS_PORT_MRU_10240_E
}CPSS_PORT_MRU_SIZE_ENT;



/*
 * typedef: enum CPSS_PORT_XGMII_MODE_ENT
 *
 * Description: Enumeration of XGMII Transmit Inter-Packet Gap (IPG) modes
 *              that can be used in the Prestera system.
 *
 * Enumerations:
 *    CPSS_PORT_XGMII_LAN_E -
 *      LAN Mode Maintains a Deficit Idle Count (DIC), which is used to decide
 *      whether to add or delete idle characters and maintain an average IPG of
 *      96 bits
 *    CPSS_PORT_XGMII_WAN_E -
 *      IPG is stretched to adopt to OC-192 speed.
 *    CPSS_PORT_XGMII_FIXED_E -
 *      Adds up to three idle symbols to a Base IPG that ranges from 64 bits
 *      to 120 bits in steps of 32bits, to align the start symbol to Lane 0
  */
typedef enum
{
    CPSS_PORT_XGMII_LAN_E,
    CPSS_PORT_XGMII_WAN_E,
    CPSS_PORT_XGMII_FIXED_E
}CPSS_PORT_XGMII_MODE_ENT;


/*
 * typedef: struct CPSS_PORT_MAC_STATUS_STC
 *
 * Description: Structure for configuring Protocol based classification
 *
 * Fields:
 *    isPortRxPause     - whether port receives pause.
 *    isPortTxPause     - whether port transmit pause.
 *    isPortBackPres    - whether is in Back pressure.
 *    isPortBufFull     - whether port lacking rx buffers.
 *    isPortSyncFail    - whether port sync test failed.
 *    isPortHiErrorRate - whether the port in High Error rate.
 *    isPortAnDone      - whether port Auto Neg done.
 *    isPortFatalError  - whether port has fatal error.
 */
typedef struct
{
    GT_BOOL isPortRxPause;
    GT_BOOL isPortTxPause;
    GT_BOOL isPortBackPres;
    GT_BOOL isPortBufFull;
    GT_BOOL isPortSyncFail;
    GT_BOOL isPortHiErrorRate;
    GT_BOOL isPortAnDone;
    GT_BOOL isPortFatalError;

}CPSS_PORT_MAC_STATUS_STC;

/*
 * typedef: enum CPSS_PORT_DIRECTION_ENT
 *
 * Description: Enumeration of port direction.
 *
 * Enumerations:
 *    CPSS_PORT_DIRECTION_RX_E - port's Rx direction
 *    CPSS_PORT_DIRECTION_TX_E - port's Tx direction
 *    CPSS_PORT_DIRECTION_BOTH_E - port's Rx and Tx directions
 */
typedef enum{
    CPSS_PORT_DIRECTION_RX_E,
    CPSS_PORT_DIRECTION_TX_E,
    CPSS_PORT_DIRECTION_BOTH_E
}CPSS_PORT_DIRECTION_ENT;

/**********************************************************************
 * typedef: enum CPSS_PORT_RX_FC_PROFILE_SET_ENT
 *
 * Description: Enumeration of Flow control profile set
 *
 * Enumerations:
 *    CPSS_PORT_RX_FC_PROFILE_1_E - Flow Control profile 1
 *    CPSS_PORT_RX_FC_PROFILE_2_E - Flow Control profile 2
 *    CPSS_PORT_RX_FC_PROFILE_3_E - Flow Control profile 3
 *    CPSS_PORT_RX_FC_PROFILE_4_E - Flow Control profile 4
 *********************************************************************/
typedef enum
{
     CPSS_PORT_RX_FC_PROFILE_1_E =0,
     CPSS_PORT_RX_FC_PROFILE_2_E,
     CPSS_PORT_RX_FC_PROFILE_3_E,
     CPSS_PORT_RX_FC_PROFILE_4_E
} CPSS_PORT_RX_FC_PROFILE_SET_ENT;

/*
 * typedef: enum CPSS_PORT_XG_FIXED_IPG_ENT
 *
 * Description: Enumeration of The IPG base used to calculate the IPG when XG 
 * port is set to fixed mode.
 *
 * Enumerations:
 *    CPSS_PORT_XG_FIXED_IPG_8_BYTES_E -
 *      Fixed IPG uses 8 bytes as the base IPG.
 *    CPSS_PORT_XG_FIXED_IPG_12_BYTES_E -
 *      Fixed IPG uses 12 bytes as the base IPG.
 */
typedef enum
{
    CPSS_PORT_XG_FIXED_IPG_8_BYTES_E,
    CPSS_PORT_XG_FIXED_IPG_12_BYTES_E
} CPSS_PORT_XG_FIXED_IPG_ENT;


/*
 * typedef: enum CPSS_PORT_FLOW_CONTROL_ENT
 *
 * Description: Enumeration of Port FC Direction enablers.
 *
 * Enumerations:
 *  CPSS_PORT_FLOW_CONTROL_DISABLE_E  - Both disabled
 *  CPSS_PORT_FLOW_CONTROL_RX_TX_E    - Both enabled
 *  CPSS_PORT_FLOW_CONTROL_RX_ONLY_E  - Only Rx FC enabled
 *  CPSS_PORT_FLOW_CONTROL_TX_ONLY_E  - Only Tx FC enabled
 *
 * Comments:
 *         None.
 */
typedef enum
{
    CPSS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,
    CPSS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,
    CPSS_PORT_FLOW_CONTROL_RX_ONLY_E,  
    CPSS_PORT_FLOW_CONTROL_TX_ONLY_E
}CPSS_PORT_FLOW_CONTROL_ENT;


/*
 * typedef: struct CPSS_PORT_MODE_SPEED_STC
 *
 * Description: pair of port interface mode and speed 
 *              possible configuration
 *
 * Fields:
 *      ifMode - port interface mode 
 *      speed  - port speed
 *
 */
typedef struct
{
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT          speed;
}CPSS_PORT_MODE_SPEED_STC;

/*
 * typedef: enum CPSS_PORT_PCS_RESET_MODE_ENT
 *
 * Description: Enumeration of port PCS reset direction.
 *
 * Enumerations:
 *    CPSS_PORT_PCS_RESET_MODE_RX_E - reset/unreset PCS Rx only
 *    CPSS_PORT_PCS_RESET_MODE_TX_E - reset/unreset PCS Tx only
 *    CPSS_PORT_PCS_RESET_MODE_ALL_E - reset/unreset PCS in all modes
 */
typedef enum{
    CPSS_PORT_PCS_RESET_MODE_RX_E,
    CPSS_PORT_PCS_RESET_MODE_TX_E,
    CPSS_PORT_PCS_RESET_MODE_ALL_E
}CPSS_PORT_PCS_RESET_MODE_ENT;

/*******************************************************************************
* prvCpssGenCpssDevType2HwsTranslate
*
* DESCRIPTION:
*       Translate device family and revision to HWS type
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       device type in HWS format (GT_U32 used from internal path definition
*                                   considerations)
*
*******************************************************************************/
GT_U32 prvCpssGenCpssDevType2HwsTranslate
(
    GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortCtrlh */

/*
 *------------------------------------------------------------------
 * $Log: cpssPortCtrl.h,v $
 * Revision 1.1  2015/02/13 11:34:13  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
