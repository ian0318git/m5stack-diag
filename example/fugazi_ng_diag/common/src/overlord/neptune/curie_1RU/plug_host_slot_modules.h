/* $Id: plug_host_slot_modules.h,v 1.2 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/plug_host_slot_modules.h,v $
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
Revision 1.2  2019/08/06 06:56:15  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2018/10/16 09:05:39  meho
Pluggable re-structured



*/
