/* $Id: linux_mem.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/linux_mem.c,v $
 *------------------------------------------------------------------
 *
 * 5/2008 mcharon
 *     Leverage from common/src/linux_mem.c
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <linux/kernel.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "types.h"
#include "mem_mgr.h"  /* klm header files */
#include "klm_defs.h" /* klm header files */
#include "linux_api.h"
#include "cross_platform.h"
#include "common.h"
#include "common_utils.h"

/* here's the explanation of how memory management works:

suppose we set limit on how much memory linux can occupy to 96M.
that means anything above 96M is a sandbox for the user app,
garanteed not to be touched by linux kernel.
first memory region above 96M is reserved for getfreememstart,
next memory region reserved for goofy device objects,
with he rest available to IO modules.

we begin by calling malloc_nm before creating object device drivers.
the first malloc_nm call will result in reserving a small region to be
used by 'getfreememstart' call. this region starts at GET_START_FREEMEM
and has the size of GETSTARTFREEMEM_SIZE. If getfreemem needs more memory,
just bump up GETSTARTFREEMEM_SIZE.

next goofy device objects will call malloc_nm....goofy device objects will never
free its memory, requiring  special region reserved for goofy device objects.
as goofy device objects make malloc_nm call, 'malloc_free_nm' keeps track of the next available
memory location, with the first avaiable memory address being
GET_START_FREEMEM + GETSTARTFREEMEM_SIZE, which is store inside variable 'start_malloc_free_mem'.

after all the goofy device objects are created and before jumping to our menu, 'malloc_free_mem'
is the next memory address available to IO modules.  We need to call
init_large_memory_usage_cnt(), so that 'start_malloc_free_mem' is set to 'malloc_free_mem', the
next available memory, ensuring that when IO modules call malloc_nm, requesting
large memory (> 128K), we are using memory region that's truly available,
not used by getfreememstart and not used by goofy device objects.

when IO module calls free_nm, the last invokation of free_nm will make sure that
variable 'malloc_free_mem' which is used to keep track of next available memory  is reset to the
'start_malloc_free_mem', which has the start address of the memory not used by getfreemem
and not used by goofy device objects, to avoid memory leak.
free_nm doesn't reset 'malloc_free_mem' until memory allocation reference count is
decrement to 0. (this means that memory is consumed, and not released!!! until free_nm
with a reference count of 1 is called...)
*/
//#define MEM_DEBUG
static uint large_memory_usage_cnt = 0;
static ulong ptr_getfreemem = 0;
/* pointer to free mem...used when user needs large memory (> 128K) */
static ulong start_malloc_free_mem = (MEM_MGR_START_FREEMEM);
static ulong malloc_free_mem = (MEM_MGR_START_FREEMEM);

typedef struct mem_list_t_ {
    struct mem_info_t_ addr;
    struct mem_list_t_ *next;
    struct mem_list_t_ *prev;
    uint32_t in_use;
} mem_list_t;

static mem_list_t mem_list[MAX_LIST_SIZE];

static int fd_mem =0;       /* dev/mem --manages mmap */
static int fd_mem_mgr =0;   /* /dev/mem_mgr --manages malloc_nm */


void mmap_close(void)
{
    if (fd_mem) {
        close(fd_mem);
    }
}

void
unmap_dev(mem_info_t *addr, ulong virt_addr, int size)
{
    assert(!"unmap_dev not supported\n");
}

/* used by memops.c */
unsigned long
mmap_dev(mem_info_t *addr, const uint64_t phy_addr, const uint32_t size)
{
    assert(!"mmap_dev not supported\n");
    return 0;
}

void *
malloc_dev (unsigned long nbytes)
{
    return ((void *)(malloc((size_t)nbytes)));
}

void init_large_memory_usage_cnt(void)
{
    large_memory_usage_cnt = 0;
    start_malloc_free_mem = malloc_free_mem;
#ifdef MEM_DEBUG
    printf("....start_malloc_free_mem for IO use @%p....reference cnt=0\n",
	   (void *)start_malloc_free_mem);
    fflush(0);
#endif

}

void *
malloc_nm (unsigned long size)
{
    assert(!"malloc_nm not supported");
    return (void *)0;
}

void
free_nm (void *s)
{
    assert(!"free_nm not supported");
    return ;
    
}

/* becarefule!!!! in diagmon, getfreemem returns mem address in kseg0
   (bit 32 set) but in linux there's no way to return this address so getfreemem just returns
   virtual address.
*/
/* statements below do not apply:
   this means that where in the diagmon code we do MEM_TO_PCI on the address
   returned by getfreememstart, we don't need to do MEM_TO_PCI....;otherwise, things can get messey
*/
long
getfreememstart(void)
{
    /* a kludge to support getfreememstart */
    return (long)((VIR_ADDR(ptr_getfreemem)));
#ifdef MEM_DEBUG
    printf("getfreemem start : %p\n", ptr_getfreemem);
#endif

}

void mem_mgr_open(void){
    
    if (fd_mem_mgr)
        return;
    if ((fd_mem_mgr = open("/dev/mem_mgr", O_RDWR)) < 0) {
        perror("mem_mgr_open: Can't open /dev/mem_mgr");
        exit(EXIT_FAILURE);
    }

}
void mem_mgr_close(void)
{
    if (fd_mem_mgr) {
        if (close(fd_mem_mgr) < 0) {
            perror("mem_mgr_close: Can't close");
            exit (EXIT_FAILURE);
        }
    }
    fd_mem_mgr = 0;
}


unsigned long XKPHYS_PCI_ADDR(void *mm)
{
    printf("fix me!!! linux_api.c xkphys_pci_addr\n");
    exit(0);
}


unsigned long PHY_ADDR(unsigned long s)
{
    unsigned long virt_addr = (unsigned long)s;
    unsigned long start, end, offset;
    uint32_t i = 0;

    for (i=0;i<MAX_LIST_SIZE;i++) {
        start  = mem_list[i].addr.virt_addr;
        end = start + mem_list[i].addr.size;
        if ( (virt_addr >= start) && (virt_addr < end)) {
            offset = virt_addr - start;
            break;
        }
    }

    if (i >= MAX_LIST_SIZE) {
        printf("Unable to get physical address using virtual address %p\n", (void *)virt_addr);
        for (i=0;i<MAX_LIST_SIZE;i++) {
            start  = mem_list[i].addr.virt_addr;
            end = start + mem_list[i].addr.size;
            if (mem_list[i].in_use) {
                printf("virtual[%d]: %p to %p; phsical: %p to %p; size %d [%#x]\n", i,
                       (void *)(mem_list[i].addr.virt_addr),
                       (void *)(mem_list[i].addr.size + mem_list[i].addr.virt_addr),
                       (void *)(mem_list[i].addr.phy_addr),
                       (void *)(mem_list[i].addr.size + mem_list[i].addr.phy_addr),
                       mem_list[i].addr.size, mem_list[i].addr.size);
                fflush(0);
            }
                    
        }
        printf("....exiting program...hit return to continue...\n");getchar();
        exit(0);
    }

    /* all's well...return physical address */
    return (unsigned long)(mem_list[i].addr.phy_addr + offset);

}

unsigned long MEM_TO_PCI(unsigned long s)
{
    return PHY_ADDR(s);
}
unsigned long PCI_TO_MEM(unsigned long s)
{
    return VIR_ADDR(s);
}


unsigned long PHY_TO_KSEG1(unsigned long s)
{
    return VIR_ADDR(s);
}

unsigned long VIR_ADDR(ulong s)
{
    unsigned long phy_addr = (unsigned long)s;
    unsigned long start, end, offset;
    uint32_t i = 0;

    for (i=0;i<MAX_LIST_SIZE;i++) {
        start  = mem_list[i].addr.phy_addr;
        end = start + mem_list[i].addr.size;
        if ( (phy_addr >= start) && (phy_addr < end)) {
            offset = phy_addr - start;
            break;          
        }
    }

    if (i>=MAX_LIST_SIZE) {
        printf("Unable to get virtual address using physical address %p\n", (void *)phy_addr);
        for (i=0;i<MAX_LIST_SIZE;i++) {
            start  = mem_list[i].addr.virt_addr;
            end = start + mem_list[i].addr.size;
            if (mem_list[i].in_use) {
                printf("virtual[%d]: %p to %p; phsical: %p to %p; size %d [%#x]\n", i,
                       (void *)(mem_list[i].addr.virt_addr),
                       (void *)(mem_list[i].addr.size + mem_list[i].addr.virt_addr),
                       (void *)mem_list[i].addr.phy_addr,
                       (void *)(mem_list[i].addr.size + mem_list[i].addr.phy_addr),
                       mem_list[i].addr.size, mem_list[i].addr.size);
                fflush(0);
            }
                    
        }
        printf("....exiting program...hit return to continue...\n");getchar();
        exit(0);
    }

    /* all's well...return physical address */
    return (unsigned long)(mem_list[i].addr.virt_addr+offset);
}


/**********************************************************************
 *
 * Function: addr_vtop
 *
 * Description: Use the vtop klm to find the physical address
 *              corresponding to the virtual address from the page
 *              table of the current process.
 *
 * Input: vir_addr - A virtual address of the current process
 *        phy_addr - pointer to the caller's buffer for holding the
 *                   output result (i.e. physical address)
 *
 * Return: PASS/FAIL
 */
int
addr_vtop(ulong vir_addr, ulong *phy_addr)
{
    int fd_vtop;
    volatile char *addr;
    ulong page_mask;

    /* Open the device file for operation
     */
    if ((fd_vtop = open("/dev/addr_vtop", O_RDWR)) < 0) {
        printf("%s(): Fail to open /dev/addr_vtop", __FUNCTION__);
        return(FAIL);
    }

    addr = (char *)vir_addr;

    /* The user virtual address is passed in addr to the vtop driver.
     * the pfn of the physical page is returned in addr by the driver.
     */
    if (read(fd_vtop, &addr, sizeof(addr)) < 0) {
        printf("%s(): Fail to read /dev/addr_vtop", __FUNCTION__);
        close(fd_vtop);
        return(FAIL);
    }

    /* Compose the actual physical address by adding the low bits
     */
    page_mask = (ulong)getpagesize() - 1;
    *phy_addr = (ulong)addr | (vir_addr & page_mask);

    if (close(fd_vtop) < 0) {
        printf("%s(): Fail to close /dev/addr_vtop", __FUNCTION__);
        return(FAIL);
    }

    return(PASS);
}

/**********************************************************************
 *
 * Function: ioptov_mmap
 *
 * Description: Use the ioptov klm to map a virtual memory range
 *              in the user space to the physical address of an
 *              IO module or device (e.g. PCi device BAR0 value) 
 *
 * Input: phy_addr - The physical address of an IO device
 *        size - the size of the address range in bytes
 *        vir_addr - pointer to the caller's buffer for holding the
 *                   output result (i.e. virtual address)
 *
 * Return: PASS/FAIL
 */
int
ioptov_mmap(ulong phy_addr, ulong size, ulong *vir_addr)
{
    int fd_ioptov = 0;
    void *mmap_rtn = 0;

    if ((fd_ioptov = open("/dev/mem", O_RDWR)) < 0) {
        printf("Failed to open /dev/mem\n");
        perror((char *)gettestname());
        return(FAIL);
    }

    /* The phy_addr value must be Linux PAGE_SIZE aligned
     */

    mmap_rtn = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_ioptov, phy_addr);

    if (mmap_rtn == MAP_FAILED) {
        perror("linux_mem.c: ioptov");
        close(fd_ioptov);
        return(FAIL);
    }

    *vir_addr = (ulong)mmap_rtn;

    if (close(fd_ioptov) < 0) {
        perror("Failed to close /dev/mem");
        return(FAIL);
    }

    return(PASS);
}

/**********************************************************************
 *
 * Function: pg_align_addr
 *
 * Description: Mask off the lower bit from 'addr' and return
 *              the page aligned address. 
 *
 * Input: addr - Any address
 *
 * Return: the page aligned address
 */
ulong
pg_align_addr(ulong addr)
{
    ulong align_addr, page_mask;

    page_mask = (getpagesize() - 1);
    align_addr = addr & ~page_mask;
    return(align_addr);
}

/**********************************************************************
 *
 * Function: pg_merge_addr
 *
 * Description: Merge the page_addr and the offset_addr to form
 *              the actual address
 *
 * Input: page_addr - Address contain the page address bits
 *        offset_addr - Address contains the offest bits
 *
 * Return: the complete address
 */
ulong
pg_merge_addr(ulong page_addr, ulong offset_addr)
{
    ulong addr, page_mask;

    page_mask = (getpagesize() - 1);
    addr = (page_addr & ~page_mask)  | (offset_addr & page_mask);
    return(addr);
}

/**********************************************************************
 *
 * Function: getmemfree
 *
 * Description: Get the MemFree value from /pfoc/meminfo 
 *
 * Input: none
 *
 * Return: Free memory reported by reading /proc/meminfo
 */
ulong
getmemfree(void)
{
    struct sysinfo sys_info;

    if(sysinfo(&sys_info) != 0) {
        printf("%s() sysinfo call failed\n", __FUNCTION__);
	return -1;
    }
    /*
    printf("MemTotal= %ld KB %ld MB\n",
	   sys_info.totalram/ONE_K, sys_info.totalram/ONE_MEG);
    printf("MemFree= %ld %ld MB\n",
	   sys_info.freeram/ONE_K, sys_info.freeram/ONE_MEG);
    */
    return(sys_info.freeram);
}

/**********************************************************************
 *
 * Function: main_mem_test
 *
 * Description: Linux level (platform independent) main memory test
 *
 * Input: none
 *
 * Return: pass/fail
 */
int main_mem_test(void)
{
// FIXME: May need to adjust OVRHD_FACTOR below
#define OVRHD_FACTOR    0.05

    char *origaddr;
    ulong start, end, freememsz, ovrhd_sz, adjust_size;
    int result = FAIL;
    ulong page_mask;

    /* Testing DRAM in Linux has constrants.
     * We can malloc all the free mem available but we can't
     * test all of it. We must adjust the free memory size that
     * the malloc gave us by a factor of 0.6%.
     * This only happened on the Cavium eval
     * board. Our ngd Linux server do not have this issue. 
     */
    freememsz = getmemfree();
    ovrhd_sz = freememsz * OVRHD_FACTOR;
    adjust_size = freememsz - ovrhd_sz;
    printf("Testing %dMB free memory\n", (unsigned int)(adjust_size/ONE_MEG));

    origaddr = (char *)malloc(adjust_size);
    if (!origaddr) {
        printf("%s() malloc failed\n",__FUNCTION__);
	    return(FAIL);
    }

    /* Align the start and end address at page boundary
     */
    page_mask = (getpagesize() - 1);    
    start = ((ulong)origaddr + page_mask) & ~page_mask;
    end = ((ulong)origaddr + adjust_size) & ~page_mask;

    result = mem_addr_eq_data_test(start, end, start);
    if (result == PASS) {
        result = mem_march_test(LONG_UNCACHE, start, end);
    }

    free(origaddr);

    if (result == PASS) {
      printf("Main memory test passed\n");
    }
    else {
      printf("Main memory test failed\n");
    }
    return(result);
}

/*-------------------------------------------------
$Log: linux_mem.c,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.4  2017/07/11 06:45:57  meho
Fixed PRRQ commnet.

Revision 1.1.2.3  2017/07/05 02:38:12  alpeng
fixed for prrq comments

Revision 1.1.2.2  2017/02/21 02:55:45  meho
Increased test coverage to 95% in Cavium memory test.

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

Revision 1.4  2014/01/10 21:30:58  mcharon
remove header files that no longer exist for new gnu

Revision 1.3  2012/06/05 09:33:44  aarwang
- Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module

$Endlog$
*/
