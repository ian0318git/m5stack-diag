 /* $Id: diag_dsl_util.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_dsl_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_dsl_util.h 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DIAG_DSL_UTIL_H_
#define _DIAG_DSL_UTIL_H_


/* NC Command Dispatch */
#define DIAG_BCM63268_LED_OFF              "bcm63268_led_off"
#define DIAG_BCM63268_LED_CD_ON            "bcm63268_led_cd_on"
#define DIAG_BCM63268_LED_DATA_ON          "bcm63268_led_data_on"

#define DSL_LED_OFF     0
#define DSL_LED_CD_ON   1
#define DSL_LED_DATA_ON 2


extern int xdsl_bcm63168_led_utils(int);
extern int marvell_1512_reset(void);


#endif /* DIAG_DSL_UTIL_H */

/*-------------------------------------------------
 * $Log: diag_dsl_util.h,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.2  2018/06/25 07:02:35  olin2
 * Remove Viper-Intel P0 DSL support
 *
 * Revision 1.1.2.1  2018/05/21 08:42:36  olin2
 * Support DSL LED on/off utility
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
