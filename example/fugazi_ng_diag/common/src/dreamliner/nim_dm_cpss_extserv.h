/* $Id: nim_dm_cpss_extserv.h,v 1.4 2020/05/22 02:28:26 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/nim_dm_cpss_extserv.h,v $
 *------------------------------------------------------------------
 * DM CPSS lib external service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *---------------------------------------------------------------------------
 */

#ifndef __NIM_DM_CPSS_EXTSERV_H__
#define __NIM_DM_CPSS_EXTSERV_H__

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/cpssExtServices.h>

#include "nim_dm_ioctl.h"
#include "nim_dm_cpss_os.h"
#include "nim_dm_cpss_trace.h"
#include "nim_dm_cpss_extdrv.h"

#define PCI_RESOURCE_RANGE 32 
#define TOTAL_NGIO_SLOT_NUM  8

extern int main_fd;
extern int sub_fd;
extern unsigned int dm_cpss_slot;
extern unsigned int dm_cpss_bay;
extern unsigned long config_addr;
extern unsigned long config_size;
extern unsigned long ppregs_addr;
extern unsigned long ppregs_size;

#define ERR printf
#define INFO printf

typedef struct{
    CPSS_EXT_DRV_FUNC_BIND_STC   extDrv;
    CPSS_OS_FUNC_BIND_STC        os;
    CPSS_TRACE_FUNC_BIND_STC     trace;
}nim_dm_cpss_bind_func;

pcie_mapping_array_t * nim_dm_get_pcie_mapping(void);

void nim_dm_put_pcie_mapping(pcie_mapping_array_t *map_array);

int nim_dm_cpss_extserv_init(unsigned int slot, unsigned int bay);

void nim_dm_cpss_extserv_cleanup(void);

void nim_dm_cpss_get_extserv (CPSS_EXT_DRV_FUNC_BIND_STC *extDrv,
                              CPSS_OS_FUNC_BIND_STC      *os,
                              CPSS_TRACE_FUNC_BIND_STC   *trace);

void nim_dm_cpss_get_pciemap(GT_UINTPTR *pciBaseAddr,
                             GT_UINTPTR *internalPciBase);

void nim_dm_cpss_event_signal(void);


#endif /*__NIM_DM_CPSS_EXTSERV_H__*/
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_extserv.h,v $
 * Revision 1.4  2020/05/22 02:28:26  qingcwan
 * Merge switzer-carrier code into main chunk.
 *
 * Revision 1.3  2018/05/18 09:24:49  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.2.20.1  2016/12/07 05:59:00  alpeng
 *  update dreamliner setting for supporting neptune
 *
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 *
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 *
 * Revision 1.1.2.1  2014/12/02 08:04:11  iachang
 * Dreamliner Diag initial check-in.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
