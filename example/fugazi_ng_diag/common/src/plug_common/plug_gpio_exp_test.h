/* $Id: plug_gpio_exp_test.h,v 1.2 2018/01/20 04:53:29 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_gpio_exp_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : plug_gpio_exp_test.h
 * Description: Header file of Pluggable GPIO Expander Test
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_GPIO_EXP_TEST_H__
#define __PLUG_GPIO_EXP_TEST_H__

typedef enum {
    MANDATORY,
    OPTIONAL
} plug_gpio_type;

extern int plug_gpio_exp_reg_test(int);
extern int plug_gpio_exp_show_reg(int);
extern int plug_gpio_exp_alter_reg(int);

#endif

/*-------------------------------------------------
$Log: plug_gpio_exp_test.h,v $
Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:40:40  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:18  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.3  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

