/* $Id: platform_stub.c,v 1.2 2018/05/18 09:25:00 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/platform_stub.c,v $
 *------------------------------------------------------------------
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <setjmp.h>
#include <assert.h>
#include <stdio.h>
#include "types.h" /* cli_cookie_cmd */
#include "cli_cmd.h" /*cli_cookie_cmd */


void   (*should_pause_diag)(void) = NULL;

int netflashbooted = 1;

jmp_buf monjmpbuf, *monjmpptr;

int pem_show_cookie_x (boolean mode, cli_cookie_cmd *cmd)
{
    /* not used for ntpn, but need to define for command code
     * platform_psu.c 
     */
    return 0; 
}

void
get_pci_bus_no (int slot, int i, int j)
{
    assert(!"not supported\n");
    
}

void
dma_intr (int irq, void *a)
{

}

void
hts_intr (void)
{

}

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 * **********************************************************************
 */

float get_mem_overhead_factor(void) {
    return 0.006;
}


//git clone  get://busybox.net/busybox.git
//cd busybox
//git pearl
//git branch bcm_rootfs
//git checkout bcm_rootf
//make CROSS_COMPILE=.... defconfig
/*
#define mb()    asm volatile("mfence":::"memory")
#define rmb()   asm volatile("lfence":::"memory")
#define wmb()   asm volatile("sfence":::"memory")
*/
/******** History ******** 
$Log: platform_stub.c,v $
Revision 1.2  2018/05/18 09:25:00  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.3  2017/07/05 06:31:15  alpeng
update fan info, update PSU and remove pem files

Revision 1.1.2.2  2017/04/05 08:29:45  leschen
Sync with <ng_diag-tag-032917>

Revision 1.1.2.1  2016/06/02 22:04:02  jskow
Move Overlord/x86 specific files to Neptune/x86.

Revision 1.4  2013/07/22 19:37:03  mcharon
move hts to utah dir/add platform_stub


$Endlog$
*/
