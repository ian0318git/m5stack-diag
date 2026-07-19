/* $Id: plug_testcard_gpio_exp_lib.h,v 1.3 2018/11/23 09:10:40 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_gpio_exp_lib.h,v $
 *  
 * Filename   : plug_gpio_tc_exp_lib.h
 * Description: Header file of Pluggable GPIO Expander Library for Test Card
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_GPIO_TC_EXP_LIB_H__
#define __PLUG_GPIO_TC_EXP_LIB_H__

#include "dev_pca9557.h"
extern int plug_tc_gpio_exp_dev_create(dev_pca9557_object_t *);

#endif

/*-------------------------------------------------
$Log: plug_testcard_gpio_exp_lib.h,v $
Revision 1.3  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.1  2018/10/15 06:50:49  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 05:01:10  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
add pluggable testcard for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.2  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

