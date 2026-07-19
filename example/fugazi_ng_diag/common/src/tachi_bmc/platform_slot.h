/* $Id: platform_slot.h,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_slot.h,v $
 *------------------------------------------------------------------
 *
 * platform_slot.h - Contains structure of all supported NIM on TACHI
 * 
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Alan O'Sullivan
 */

#ifndef __PLATFORM_SLOT_H__
#define __PLATFORM_SLOT_H__

extern struct module_info *get_platform_slot_table(int *index, unsigned short id);
extern int get_wic_serdes_no(int slot);
extern int get_max_sm_slots(void);
extern int get_max_wic_slots(void);
extern int get_sm_real_slot(int slot);
extern int get_wic_real_slot(int slot);
extern int get_vm_real_slot(int slot);
extern int get_wic_device_no(int slot);
extern int get_sm_device_no(int slot);
extern int get_max_vm_slots(void);
extern int hwic_slot_start_with(void);
extern int get_max_hwic_slots(void);
extern int get_dc_real_slot(int);

#endif  /* end __PLATFORM_SLOT_H */

/* ------ End of Module ------ */



/*
 *------------------------------------------------------------------
$Log: platform_slot.h,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

$Endlog$
*/
