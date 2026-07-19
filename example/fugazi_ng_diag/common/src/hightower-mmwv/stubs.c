/* $Id: stubs.c,v 1.2 2021/06/02 02:56:22 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/stubs.c,v $
 *********************************************************************
 *
 * stubs.c - stub functions for HT-mmwv
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */



#include "types.h"
#include "common.h"
#include "monitor.h"
#include <stdio.h>

int netflashbooted = 0; /* menu.c need this */

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
    return 0.2;
}

/*********************************************************************
 * $Log: stubs.c,v $
 * Revision 1.2  2021/06/02 02:56:22  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

