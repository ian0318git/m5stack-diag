/* $Id: platform_stub.c,v 1.2 2016/05/06 17:44:26 huanngo Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_stub.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2013 - 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <setjmp.h>
#include <assert.h>
#include <stdio.h>

void   (*should_pause_diag)(void) = NULL;

int netflashbooted = 1;

jmp_buf monjmpbuf, *monjmpptr;

void
get_pci_bus_no (int slot, int i, int j)
{
    assert(!"not supported\n");
    
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
Revision 1.2  2016/05/06 17:44:26  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.1  2013/07/22 19:37:02  mcharon
move hts to utah dir/add platform_stub


$Endlog$
*/
