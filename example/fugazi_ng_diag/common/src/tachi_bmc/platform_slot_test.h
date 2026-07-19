/* $Id: platform_slot_test.h,v 1.3 2017/03/30 08:34:08 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_slot_test.h,v $
 *------------------------------------------------------------------
 *
 * platform_slot_test.c - Platform specific slot test functions.
 *                        An entry for NIM test on BMC.
 *
 * Oct 2015, Alan Peng
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SLOT_TEST_H__
#define __PLATFORM_SLOT_TEST_H__

enum {
    NIM_GPIO_TEST = 1,
    NIM_OIR_TEST, 
    NIM_DL_GENARAL_TEST,
    NIM_DL_GE0_INT_LPBK_TEST,
    NIM_DL_GE1_INT_LPBK_TEST,
    NIM_DL_EXT_LPBK_TEST,

};

#define NIM_DL_GE0_PHY_INTR_LPBK     (0)
#define NIM_DL_GE1_PHY_INTR_LPBK     (1)
#define NIM_DL_PHY_EXT_LPBK          (2)

#define NIM_DL_SEPAR_LB_TEST_DELAY   (10)
#endif  /* end __PLATFORM_SLOT_TEST_H */

/* ------ End of Module ------ */



/*
 *------------------------------------------------------------------
$Log: platform_slot_test.h,v $
Revision 1.3  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/12/09 10:35:57  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.1  2015/09/26 05:22:35  alpeng
update nim test entry


$Endlog$
*/
