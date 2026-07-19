/* $Id: mem_mgr.h,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: mem_mgr.h
 *
 * Description: header file for mem_mgr.c
 *
 *      
 * Original author mcharon
 * Copyright (c)2011-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef __MEM_MGR__
#define __MEM_MGR__

#define MAX_LIST_SIZE 512
#define MEM_MGR_MAX_KMALLOC_SIZE  0x20000
#define GETSTARTFREEMEM_SIZE   0x2C00000 /* has to be > 128K */

#define GET_START_FREEMEM         0x8000000
#define MEM_MGR_START_FREEMEM     GET_START_FREEMEM + GETSTARTFREEMEM_SIZE

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

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: mem_mgr.h,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.4  2012/04/12 18:37:03  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.3  2012/03/27 07:45:06  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.2  2011/08/18 19:43:28  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.1  2011/07/21 20:06:27  huanngo
 * Add support for mem_mgr.ko
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
