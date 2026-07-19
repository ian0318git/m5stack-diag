/* $Id: highrise_cpld_diag.h,v 1.1 2020/08/19 09:49:35 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/highrise_cpld_diag.h,v $
 *******************************************************************************
 * File Name: highrise_cpld_diag.h
 *
 * Description: Highrise CPLD main header file
 *
 * Author: Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef HIGHRISE_CPLD_DIAG_H_
#define HIGHRISE_CPLD_DIAG_H_

#define SKU_30361               0x1
#define SKU_30363               0x2

/* Define upgrade interface */
typedef enum {
    UPGRADE_FROM_CPLD = 0,
    UPGRADE_FROM_IO_EXPANDER,
    SPEED_UP_UPGRADE_FROM_CPLD,
} cpld_interface_t;

extern long build_hr_cpld_menu(int);

#endif /* HIGHRISE_CPLD_DIAG_H_ */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: highrise_cpld_diag.h,v $
 * Revision 1.1  2020/08/19 09:49:35  markzha
 * *** empty log message ***
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
