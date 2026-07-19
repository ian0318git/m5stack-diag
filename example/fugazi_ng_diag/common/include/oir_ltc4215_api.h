/* $Id: oir_ltc4215_api.h,v 1.2 2012/03/28 00:38:11 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/oir_ltc4215_api.h,v $
 *------------------------------------------------------------------
 * Filename: oir_test.h
 *
 * Description: header file of Xformer OIR test.
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __OIR_TEST_H__
#define __OIR_TEST_H__

/*******************************************************************************
 constants
*******************************************************************************/
enum {
    OIR_LED_OFF = 0,
    OIR_LED_AMBER,
    OIR_LED_GREEN,
    OIR_LED_AMBER_ONLY,
    OIR_LED_GREEN_ONLY,
    OIR_LED_AMBER_OFF,
    OIR_LED_GREEN_OFF,
};

/*******************************************************************************
 prototypes
*******************************************************************************/
int  oir_ltc4215_register_test(void *);
int  oir_ltc4215_leds_test(void *);
int  oir_ltc4215_reg_write(void *, uchar reg, uchar *data);
int  oir_ltc4215_reg_read(void *, uchar reg, uchar *data);
int  util_oir_ltc4215_led(void *, uchar led_color);
int  util_oir_ltc4215_reg_read(void *);
int  util_oir_ltc4215_reg_write(void *);

#endif /* __OIR_TEST_H__ */


/******** History ******** 
$Log: oir_ltc4215_api.h,v $
Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
