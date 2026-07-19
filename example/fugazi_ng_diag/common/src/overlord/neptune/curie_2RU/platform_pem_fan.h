/* $Id: platform_pem_fan.h,v 1.1 2020/01/09 01:02:03 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_pem_fan.h,v $
 *------------------------------------------------------------------
 *
 * platform_pem_fan.h: Header for Fan Control Utilities
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Dave DeSimone
 *------------------------------------------------------------------
 */
#ifndef __RP1RUVE_FAN_H__
#define __RP1RUVE_FAN_H__

#define BANK_TEST_DELAY 4000
#define RP1RUVE_MIN_FAN_SPEED        60 // Minimum Fan Speed (60%)
#define RP1RUVE_MAX_FAN_SPEED       100 // Maximum Fan Speed (100%)
#define RP1RULVE_FAN_GET_DATA     0
#define RP1RULVE_FAN_GET_SPEED    1

typedef struct rp1ruve_fan_info_st {
    uint32 data;
    uchar speed;
    int   temp_lo;
    int   temp_hi;
} rp1ruve_fan_info_t;


extern void display_pem_fan_spd(void);


#endif /* __RP1RUVE_FAN_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: platform_pem_fan.h,v $
Revision 1.1  2020/01/09 01:02:03  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
