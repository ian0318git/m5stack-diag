/* $Id: nim_dm_cpss_extserv.h,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_cpss_extserv.h,v $
 *------------------------------------------------------------------
 * DM CPSS lib external service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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

#define PCI_RESCAN          "echo 1 > /sys/bus/pci/rescan"

#define PCI_RESOURCE_RANGE 32 
#define TOTAL_NGIO_SLOT_NUM  6 
#define TOTAL_NGIO_SLOT_NUM_ONE    1 

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

int nim_dm_cpss_extserv_init_ex(unsigned int slot, unsigned int bay);

int nim_dm_cpss_extserv_init(unsigned int slot, unsigned int bay);

void nim_dm_cpss_extserv_cleanup_ex(void);

void nim_dm_cpss_extserv_cleanup(void);

void nim_dm_cpss_get_extserv (CPSS_EXT_DRV_FUNC_BIND_STC *extDrv,
                              CPSS_OS_FUNC_BIND_STC      *os,
                              CPSS_TRACE_FUNC_BIND_STC   *trace);

void nim_dm_cpss_get_pciemap(GT_UINTPTR *pciBaseAddr,
                             GT_UINTPTR *internalPciBase);

void nim_dm_cpss_get_pciemap_ex(GT_UINTPTR *pciBaseAddr,
                             GT_UINTPTR *internalPciBase, GT_UINTPTR *expciBaseAddr);


void nim_dm_cpss_event_signal(void);


#endif /*__NIM_DM_CPSS_EXTSERV_H__*/
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_extserv.h,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
