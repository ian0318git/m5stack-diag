/* $Id: stubs.c,v 1.2 2020/03/13 12:06:54 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/stubs.c,v $
 *------------------------------------------------------------------
 * stubs.c
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "types.h"
#include "common.h"
#include "monitor.h"

int netflashbooted = 1;

struct monitem moncmd[] = { };
#define MONCMDSIZ (sizeof(moncmd)/sizeof(struct monitem))
int moncmdsiz = MONCMDSIZ;

int mb_board_type(void)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (0);
}

boolean does_hwic_multitask (int slotnum)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

boolean does_nm_multitask (int slotnum)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

uint pcie_config_read(uint p, uint bus, ushort dev, uint func, uint reg)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

int get_pci_dev_num (int slot, int dev)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

uchar get_pci_bus_no(uint slot)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

long timer_calibrate (long t) 
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 *
 **********************************************************************
 */

float get_mem_overhead_factor(void) {
    return 0.006;
}

/******** History ********
$Log: stubs.c,v $
Revision 1.2  2020/03/13 12:06:54  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.2  2020/01/15 03:30:11  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/
