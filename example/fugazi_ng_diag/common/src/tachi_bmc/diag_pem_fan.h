/* $Id: diag_pem_fan.h,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_pem_fan.h,v $
 *------------------------------------------------------------------
 *
 * diag_pem_fan.h: Header for Fan Control Utilities
 *
 * porintg from o2 platform_pem_fan.h 
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
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
 * $Log: diag_pem_fan.h,v $
 * Revision 1.2  2016/04/20 11:25:30  benchen2
 * add tachi fru portion
 *
 * Revision 1.1.2.1  2015/12/23 11:16:13  alpeng
 * support PEM(PSU) utility and its fan utils
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
