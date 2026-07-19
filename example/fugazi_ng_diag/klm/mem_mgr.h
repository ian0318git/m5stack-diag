/* $Id: mem_mgr.h,v 1.2 2012/03/28 00:38:26 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/mem_mgr.h,v $
 *-----------------------------------------------------------------------------
 * File: mem_mgr.h main header file for mem_mgr.c
 *
 * March. 2008, mcharon
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#ifndef __MEM_MGR__
#define __MEM_MGR__

#define MAX_LIST_SIZE 512
#define MEM_MGR_MAX_KMALLOC_SIZE  0x20000
#define GETSTARTFREEMEM_SIZE   0x2C00000 /* has to be > 128K */

#define GET_START_FREEMEM         0x8000000
#define MEM_MGR_START_FREEMEM     GET_START_FREEMEM + GETSTARTFREEMEM_SIZE
//#define CASE2 2

#ifdef LINUX_APP
#include <inttypes.h>
#endif

#define MEM_MGR_IOC _IOWR(0xC0, 5, int)

typedef struct mem_info_t_
{
    uint32_t size;   /* size of of memory that user */
    unsigned long phy_addr; /* phys addr of location requested by user. used by i/o */
    unsigned long virt_addr; /* virt addr of location requested by user. used by i/o */
    unsigned long kernel_virt_addr; /* kernel virt addr of location requested by user.
                                       klm uses this */
    unsigned long start_phy_addr; /* phy start address of mapped region. */
    unsigned long start_virt_addr; /* virt start address of mapped region. */
    unsigned long mmaped_size;     /* actual mmaped size. minimum is a page size (4k)*/
    unsigned int in_use;
} mem_info_t;

#endif

/******** History ******** 
$log: $
$Endlog: $
*/
