/* $Id: platform_pem_fan.h,v 1.2 2013/09/11 02:25:08 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/platform_pem_fan.h,v $
 *------------------------------------------------------------------
 *
 * platform_pem_fan.h: Header for Fan Control Utilities
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Dave DeSimone
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
 *------------------------------------------------------------------
 * $Log: platform_pem_fan.h,v $
 * Revision 1.2  2013/09/11 02:25:08  alpeng
 * 1. support Juno fan info and display on initialize stage.
 * 2. support fedora rootfs
 *
 * Revision 1.1  2013/05/31 12:43:15  danchung
 * Porting PSU source code from Nightster for Juno.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
