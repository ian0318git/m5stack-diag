/* $Id: t1e1_common.h,v 1.2 2012/03/28 00:38:13 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/t1e1_common.h,v $
 *-------------------------------------------------------------------------
 * t1e1_common.h -- Common definitions for t1e1 configurations/setup.
 *
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-------------------------------------------------------------------------
 */

#ifndef T1E1_COMMON_DEFS
#define T1E1_COMMON_DEFS

/*
 * Classes of T1E1 Wics
 */
#define HUMVEE_CLASS_WIC     0x01
#define DINO_CLASS_WIC       0x02
#define SHAMU_CLASS_WIC      0x04
#define SOPRANO_CLASS_WIC    0x08

/*
 * Device Mode E1 or T1
 */
#define DEVICE_MODE_E1       0x00
#define DEVICE_MODE_T1       0x01
#define DEVICE_MODE_UNDEF    0x0FF

/*
 * Wic mode definitions
 */
#define WIC_MODE_UNDEF     0xFF
#define WIC_MODE_IS_E1     0x00
#define WIC_MODE_IS_T1     0x01
/*
 *  Line Speed Definitions
 */
#define LINE_SPEED_64K       0x00
#define LINE_SPEED_56K       0x01
/*
 * Bus Modes
 */
#define BUS_MODE_TE           0x00       /* IOM-2, TDM interface */
#define BUS_MODE_LT           0x01       /* IOM-2, TDM interface */
#define BUS_MODE_NMSI         0x02       /* Unstructured Data */
#define BUS_MODE_LT_C2600     0x03       /* Modified IOM-2 for Quake */
#define BUS_MODE_LT_8M        0x04       /* IOM-2, TDM interface */
#define BUS_MODE_LT_INT       0x05       /* IOM-2, Internal LT TDM interface */

#define INTF_E1_2048MODE      0x06       /* for PICARD interface */
#define INTF_T1_1544MODE      0x07       /* for PICARD interface */
#define INTF_T1_VICMODE       0x08       /* for PICARD VIC interface */
#define INTF_MOWGLI_T1MODE    0x09       /* for PICARD Mowgli interface */
#define INTF_MOWGLI_E1MODE    0x0a       /* for PICARD Mowgli interface */
#define BUS_MODE_LT_8M_SPLIT  0x0B       
#define BUS_MODE_TIC          0x0C

/*
 *  Framer Clock Modes
 */
#define T1E1_INTERNAL_CLK     0x00
#define T1E1_LINE_CLK         0x01

/*
 *  Framer Line Codes
 */
#define LINE_CODE_AMI          0x0000
#define LINE_CODE_B8ZS         0x0001
#define LINE_CODE_HBD3         0x0002

/*
 * Frame Modes 
 */
#define FRAMER_SF_MODE         0x0000
#define FRAMER_ESF_MODE        0x0001
#define FRAMER_CRC_MODE        0x0002
#define FRAMER_NOCRC_MODE      0x0003
#define FRAMER_TIC_MODE        0x0004

#define LPBK_PER_CHANNEL         0x00
#define LPBK_CHANNEL_GROUP       0x01
#define LPBK_ALL_CHANNELS        0x02

/*
 * Loopback Test Definitions.
 */
#define EXTERNAL_LOOPBACK        0x00
#define FRAMER_LIU_LOOPBACK      0x01
#define FRAMER_LOOPBACK          0x02
#define FRAMER_REMOTE_LOOPBACK   0x03
#define FRAMER_PAYLOAD_LOOPBACK  0x04
#define SIMPLE_FRAMER_LOOPBACK   0x05
#define FRAMER_LINE_LOOPBACK     0x06
/*
 *   Time Slot definitions
 */
#define T1_FIRST_TIMESLOT  0
#define T1_MAX_TIMESLOTS   24

#define E1_FIRST_TIMESLOT  1
#define E1_MAX_TIMESLOTS   32
/*
 * Config data area - how t1e1 is configured for test
 */
typedef struct t1e1_cfg_t_ {
    ushort  t1e1;
    ushort  clock;
    ushort  line_code;
    ushort  framing;
    ushort  bus_mode;
    ushort  line_speed;
    ushort  lpbk_mode;
    unsigned char  *ts_map;
} t1e1_cfg_t;

/*********************************************************************
 *         Controller IDS for T1E1 
 ********************************************************************/
#define  WIC_NOT_DEF          0x0000
#define  WIC_T1_CSU           0x0020
#define  WIC_E1               0x0021
#define  WIC_DUAL_T1_CSU      0x0022
#define  WIC_DUAL_E1          0x0023
#define  WIC_DUAL_T1_DI       0x0024
#define  WIC_DUAL_E1_DI       0x0025
#define  WIC_E1_G703          0x002f
#define  WIC_DUAL_E1_G703     0x0030
#define  WIC_DINO_T1          0x003E
#define  WIC_DINO_E1          0x003F
#define  VIC2_1MFT_T1E1       0x0473
#define  VIC2_2MFT_T1E1       0x0474
#define  NM_SOPRANO_0T1E1     0x03D9
#define  NM_SOPRANO_1T1E1     0x03DA
#define  NM_SOPRANO_2T1E1     0x03DB

#endif

/******** History ******** 
$Log: t1e1_common.h,v $
Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
