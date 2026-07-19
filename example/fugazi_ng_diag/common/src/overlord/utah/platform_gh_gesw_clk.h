/* $Id: platform_gh_gesw_clk.h,v 1.1 2014/08/14 10:27:00 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_gh_gesw_clk.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_gh_gesw_clk.h
 *
 * Description: Definitions for Greyhound GESW clock gen. 
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_GH_GESW_CLK_H__
#define __PLATFORM_GH_GESW_CLK_H__

#define IDT8T49N4811_GH_GESW_CLK_REG_SIZE     0x6
#define IDT8T49N4811_FREQ_SEL_OUTPUT_REG      0x0
#define IDT8T49N4811_FREQ_SEL_MISC_REG        0x1
#define IDT8T49N4811_OUTPUT_EN_BANK_AB_REG    0x2
#define IDT8T49N4811_OUTPUT_EN_BANK_CD_REG    0x3
#define IDT8T49N4811_OUTPUT_TYPE_SEL_REG      0x4
#define IDT8T49N4811_MISC_CTRL_REG            0x5


#endif   /* __PLATFORM_GH_GESW_CLK_H__ */ 


/*------------------------------------------------------------------
$Log: platform_gh_gesw_clk.h,v $
Revision 1.1  2014/08/14 10:27:00  alpeng
support greyhound gesw clk gen on i2c scan test and its util


$Endlog$
*/
