/* $Id: vm_timingcard_cpld_diag.h,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_cpld_diag.h,v $
 *******************************************************************************
 * File Name: vm_timingcard_cpld_diag.h
 *
 * Description: Timing Card NGVM CPLD main header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_TIMING_CPLD_DIAG_H_
#define VM_TIMING_CPLD_DIAG_H_

#define SKU_30361               0x1
#define SKU_30363               0x2

/* Define CPLD GPIO */
typedef enum {
    CPLD_GPIO0 = 0,
    CPLD_GPIO1,
    CPLD_GPIO2,
    CPLD_GPIO3,
    CPLD_GPIO4,
    CPLD_GPIO5,
    CPLD_GPIO6,
} cpld_gpio_t;

/* Define upgrade interface */
typedef enum {
    UPGRADE_FROM_CPLD = 0,
    UPGRADE_FROM_IO_EXPANDER,
    SPEED_UP_UPGRADE_FROM_CPLD,
} cpld_interface_t;

extern long build_timingcard_cpld_menu(int);

#endif /* VM_TIMING_CPLD_DIAG_H_ */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_cpld_diag.h,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.4  2014/04/22 06:06:02  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.3  2014/03/07 07:39:58  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.2  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
