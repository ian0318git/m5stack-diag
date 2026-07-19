/* $Id: plug_host_slot_modules.h,v 1.2 2019/10/17 02:16:27 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_host_slot_modules.h,v $
 *------------------------------------------------------------------
 *
 * plug_host_slot_modules.h - Header file for Host Supported PLUGGABLE Slot 
 *                            Module Informations
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __PLUG_HOST_SLOT_MODULES__
#define __PLUG_HOST_SLOT_MODULES__

struct plug_module_info {
    char   *name;         /* module name */
    PFT    diag;          /* the diagnostic for this module */
    ushort id;            /* the port module id */
    PFT    intf_diag;     /* the pci test for this module */
    uint mod_info_flags;  /* These flags provide more info of the module */
};

extern struct plug_module_info plug_host_module_tbl[];
extern int MAX_MOD_IDS;

#endif



/*-------------------------------------------------
$Log: plug_host_slot_modules.h,v $
Revision 1.2  2019/10/17 02:16:27  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.2  2019/07/29 06:13:52  kodko
Clean up code based on off-line code review

Revision 1.1.2.1  2018/10/26 08:40:50  kodko
Add support for PIM LTE and test card modules.

$Endlog$
*/
