/* $Id: highrise_cpld_api.h,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/highrise_cpld_api.h,v $
 *******************************************************************************
 * File Name: highrise_cpld_api.h
 *
 * Description: Highrise CPLD api header file
 *
 * Author: Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef __HIGHRISE_CPLD_API_H__
#define __HIGHRISE_CPLD_API_H__

#include "types.h"

extern long hr_cpld_reset_act2(void);
extern long hr_cpld_unreset_act2(void);
#endif 

/*********************************************************************
 * $Log: highrise_cpld_api.h,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/12/09 07:29:50  alpeng
 * add function prologue; remove redundant header; adding ifdef for header files;
 *
 *
 *
 */

