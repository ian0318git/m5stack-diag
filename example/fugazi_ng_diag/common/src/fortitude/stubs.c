/* $Id: stubs.c,v 1.6 2019/06/03 09:07:40 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/stubs.c,v $
 *------------------------------------------------------------------
 *
 * stubs.c - stub functions for Fortitude NPU application.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
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
Revision 1.6  2019/06/03 09:07:40  meho
Writing data to fpga_spi_ctrl register once at a time.

Revision 1.5  2018/05/18 09:24:50  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.4  2017/07/07 18:10:59  ptong
fixed a typo

Revision 1.3  2016/05/06 17:44:26  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.2.76.2  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.2.76.1  2017/04/05 09:12:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.4  2017/07/07 18:10:59  ptong
fixed a typo

Revision 1.3  2016/05/06 17:44:26  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
