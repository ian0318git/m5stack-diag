/* $Id: cpss_extserv.h,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/cpss_extserv.h,v $
 *------------------------------------------------------------------
 *
 * Filename:	cpss_extserv.h
 *
 *------------------------------------------------------------------
 */

#ifndef __CPSS_EXTSERV_H__
#define __CPSS_EXTSERV_H__

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/cpssExtServices.h>

#include "cpss_ioctl.h"
#include "cpss_os.h"
#include "cpss_common.h"

#define PCI_RESCAN                 "echo 1 > /sys/bus/pci/rescan"

#define PCI_RESOURCE_RANGE         32 
#define TOTAL_NGIO_SLOT_NUM        6 
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

typedef struct {
    CPSS_EXT_DRV_FUNC_BIND_STC   extDrv;
    CPSS_OS_FUNC_BIND_STC        os;
    CPSS_TRACE_FUNC_BIND_STC     trace;
} cpss_pcie_bind_func;


int cpss_pcie_extserv_init_ex ();
void cpss_pcie_bar2_resize ();
void cpss_pcie_extserv_cleanup_ex (void);
void cpss_pcie_get_pciemap_ex (GT_UINTPTR *pciBaseAddr,
                               GT_UINTPTR *internalPciBase);

/* For Elixir AC5 */
void cpss_pcie_get_pciemap_size_ex (GT_UINTPTR *pciBaseAddr_size,
                                    GT_UINTPTR *internalPciBase_size);



#endif /*__CPSS_EXTSERV_H__*/
/*
 *------------------------------------------------------------------
 * $Log: cpss_extserv.h,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2021/04/12 08:41:16  illiu
 * Add file: cpss platform code
 *
 * Revision 1.1.2.1  2021/01/26 02:55:09  illiu
 * Rename nim_dm prefix file to cpss prefix
 *
 * Revision 1.1.2.3  2020/10/26 07:54:49  harrchan
 * Base on AC5 bring up reseult to modify AC5 code
 *
 * Revision 1.1.2.2  2020/10/07 09:12:55  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2020/09/09 09:18:40  illiu
 * Modified to support Dreamliner with CPSS 4.2 library.
 *
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
