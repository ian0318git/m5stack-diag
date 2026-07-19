/* $Id: diag_dev.h,v 1.2 2016/04/20 08:41:37 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_dev.h,v $
 *
 *      File:   diag_dev.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#ifndef _DIAG_DEV_H_
#define _DIAG_DEV_H_

typedef enum {
	DEV_BOARD	= 0,
	DEV_CPU,
	DEV_CHASSIS,
	DEV_SYSTEM,
	DEV_REDWOOD,
	DEV_MV88E1111,
	DEV_MV88E6095,
	DEV_ADM1066,
	DEV_PHY,
	DEV_CUPHY,
	DEV_KRPHY,
	DEV_OCTPHY,
	DEV_SERDES,
	DEV_BCM5221,
	DEV_BCM54980,
	DEV_RTC,
	DEV_MIC,
        DEV_PALO,
        DEV_UCD,
        DEV_LED,
        DEV_CAT,
        DEV_WOO,
        DEV_PMBUS,
        DEV_LTC,
        DEV_PSU,
        DEV_FAN,
        DEV_PRT,
        DEV_HWMON,
	DEV_MVSW,
	DEV_MVPHY,
	DEV_MAX16046,
	DEV_PCA9555,
	DEV_BET,
	DEV_UCD90XXX,
	DEV_IR3536,
	DEV_POE,
	DEV_MAX6654,
	DEV_UNKNOWN,
} dev_type_t;

typedef enum {
	CARD_IOM	=0,
	CARD_BMC	=1,
	CARD_FEX	=2,
	CARD_GOODING	=3,
        CARD_VENTURA    =4,
        CARD_LA         =5,
        CARD_SANDIEGO1  =6,
        CARD_SANDIEGO2  =7,
        CARD_SANFRANCISCO =8,
        CARD_ALPINE =9,
        CARD_MARIN  =10,

	CARD_UNKNOWN,
} card_type_t;
#endif // _DIAG_DEV_H_
