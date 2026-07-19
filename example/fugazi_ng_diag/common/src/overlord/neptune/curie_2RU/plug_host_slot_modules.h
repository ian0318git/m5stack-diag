/* $Id: plug_host_slot_modules.h,v 1.1 2020/01/09 01:02:06 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/plug_host_slot_modules.h,v $
 *------------------------------------------------------------------
 *
 * plug_host_slot_modules.h - Header file for Host Supported PLUGGABLE Slot 
 *                            Module Informations
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
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

/*
 *-----------------------------------------------------------------------------
$Log: plug_host_slot_modules.h,v $
Revision 1.1  2020/01/09 01:02:06  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
