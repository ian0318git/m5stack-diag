/* $Id: stubs.c,v 1.1 2015/02/26 07:18:30 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/stubs.c,v $
 *------------------------------------------------------------------
 *
 * stubs.c - stub functions for Wallander NIM application.
 *
 * April 2014, Xiaoying Zhang
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "types.h"
#include "common.h"
#include "monitor.h"

int netflashbooted = 1;

struct monitem moncmd[] = { };
#define MONCMDSIZ (sizeof(moncmd)/sizeof(struct monitem))
int moncmdsiz = MONCMDSIZ;

int 
mb_board_type(void)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (0);
}

boolean
does_hwic_multitask (int slotnum)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

boolean
does_nm_multitask (int slotnum)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

uint 
pcie_config_read(uint p, uint bus, ushort dev, uint func, uint reg)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

int
get_pci_dev_num (int slot, int dev)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

uchar
get_pci_bus_no(uint slot)
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}

long
timer_calibrate (long t) 
{
    printf("In stubbed function: %s\n",__FUNCTION__);
    return (PASSED);
}


/******** History ********
$Log: stubs.c,v $
Revision 1.1  2015/02/26 07:18:30  xiaoyizh
Initial check in for Wallander.

Revision 1.1  2013/04/19 07:17:52  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
