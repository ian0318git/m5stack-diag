/* $Id: stubs.c,v 1.2 2016/05/06 17:44:27 huanngo Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/stubs.c,v $
 *------------------------------------------------------------------
 *
 * stubs.c - stub functions for Prince NIM application.
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2012-2016 by Cisco Systems, Inc.
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
Revision 1.2  2016/05/06 17:44:27  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.1  2013/04/19 07:17:52  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
