/* $Id: nim_dm_cpss_os.c,v 1.2 2015/02/27 10:02:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/nim_dm_cpss_os.c,v $
 *------------------------------------------------------------------
 * DM CPSS lib external os service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#define _GNU_SOURCE

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/times.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include <pthread.h>

#include "nim_dm_cpss_extserv.h"

/*******************************************************************************
* osMemBzero
*
* DESCRIPTION:
*       Fills the first nbytes characters of the specified buffer with 0
*
* INPUTS:
*       start  - start address of memory block to be zeroed
*       nbytes - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID osMemBzero
(
    IN GT_CHAR * start,
    IN GT_U32 nbytes
)
{
    memset((GT_VOID *)start, 0,  nbytes);
}

/*******************************************************************************
* osMemSet
*
* DESCRIPTION:
*       Stores 'symbol' converted to an unsigned char in each of the elements
*       of the array of unsigned char beginning at 'start', with size 'size'.
*
* INPUTS:
*       start  - start address of memory block for setting
*       symbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemSet
(
    IN GT_VOID *start,
    IN int symbol,
    IN GT_U32 size
)
{
    if(start == NULL)
        return start;

    return memset(start, symbol, size);
}

/*******************************************************************************
* osMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemCpy
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32       size
)
{
    if(destination == NULL || source == NULL)
        return destination;

    return memcpy(destination, source, size);
}

/*******************************************************************************
* osMemCmp
*
* DESCRIPTION:
*       Compare 'size' characters from the object pointed to by 'str1' to
*       the object pointed to by 'str2'.
*
* INPUTS:
*       str1 - first memory area
*       str2 - second memory area
*       size - size of memory to compare
*
* OUTPUTS:
*       None
*
* RETURNS:
*       > 0  - if str1 is alfabetic bigger than str2
*       == 0 - if str1 is equal to str2
*       < 0  - if str1 is alfabetic smaller than str2
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osMemCmp
(
    IN const GT_VOID  *str1,
    IN const GT_VOID  *str2,
    IN GT_U32       size
)
{
    if(str1 == NULL || str2== NULL)
        return GT_BAD_PARAM;

    return (GT_32) memcmp(str1, str2, size);
}


/*******************************************************************************
* osMemStaticMalloc
*
* DESCRIPTION:
*       Allocates memory block of specified size, this memory will not be free.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available or calling after first init.
*
* COMMENTS:
*       Usage of this function is only on FIRST initialization.
*
*******************************************************************************/
GT_VOID *osMemStaticMalloc
(
    IN GT_U32 size
)
{
    return malloc(size);
}

/*******************************************************************************
* osMemMalloc
*
* DESCRIPTION:
*       Allocates memory block of specified size.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
GT_VOID *osMemMalloc
(
    IN GT_U32 size
)
{
    return malloc(size);
}

/*******************************************************************************
* osMemRealloc
*
* DESCRIPTION:
*       Reallocate memory block of specified size.
*
* INPUTS:
*       ptr  - pointer to previously allocated buffer
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*       Recommended to avoid usage of this function.
*
*******************************************************************************/
GT_VOID *osMemRealloc
(
    IN GT_VOID * ptr ,
    IN GT_U32    size
)
{
    return realloc(ptr, size);
}

/*******************************************************************************
* CPSS_OS_FREE_FUNC
*
* DESCRIPTION:
*       De-allocates or frees a specified memory block.
*
* INPUTS:
*       memblock - previously allocated memory block to be freed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
GT_VOID osMemFree
(
    IN GT_VOID* const memblock
)
{
    free(memblock);
}

/*******************************************************************************
* osMemCacheDmaMalloc
*
* DESCRIPTION:
*       Allocate a cache-safe buffer  of specified size for DMA devices
*       and drivers
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
static dma_mem_info_t *g_dma_mem = NULL;
static pthread_mutex_t dma_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

GT_VOID *osMemCacheDmaMalloc
(
    IN GT_U32 size
)
{
    dma_mem_info_t *dma_mem = NULL;

    dma_mem = osMemMalloc(sizeof(dma_mem_info_t));
    if(dma_mem == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc memory for "
            "dma_mem_info_t.", dm_cpss_slot, dm_cpss_bay);

        return NULL;
    }

    dma_mem->mem.size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    if (ioctl(sub_fd, DM_SUB_IOC_ALLOC_DMA_MEM,
              (unsigned long)&dma_mem->mem) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to get dma memory from "
            "kernel driver.", dm_cpss_slot, dm_cpss_bay);

        return NULL;
    }

    dma_mem->user_addr = mmap(0,
                              dma_mem->mem.size,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED,
                              sub_fd,
                              config_size + ppregs_size +
                              dma_mem->mem.kernel_addr);

    if (MAP_FAILED == dma_mem->user_addr) {
        ERR("The dm instance of slot/bay(%u/%u) fail to map dma memory to user "
            "space.", dm_cpss_slot, dm_cpss_bay);

        return NULL;
    }

    pthread_mutex_lock(&dma_mutex);

    if(g_dma_mem == NULL) {
        g_dma_mem = dma_mem;
        dma_mem->next = dma_mem;
        dma_mem->prev = dma_mem;
    } else {
        dma_mem->prev = g_dma_mem->prev;
        g_dma_mem->prev->next = dma_mem;
        dma_mem->next = g_dma_mem;
        g_dma_mem->prev = dma_mem;
    }

    pthread_mutex_unlock(&dma_mutex);


    return dma_mem->user_addr;
}

/*******************************************************************************
* osMemCacheDmaFree
*
* DESCRIPTION:
*       Free the buffer acquired with osCacheDmaMalloc()
*
* INPUTS:
*       ptr - pointer to malloc/free buffer
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osMemCacheDmaFree
(
    IN GT_VOID *ptr
)
{
    dma_mem_info_t *dma_mem;
    if(ptr == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, ptr);

        return GT_BAD_PARAM;
    }

    if(g_dma_mem == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad env(%p).",
            dm_cpss_slot, dm_cpss_bay, g_dma_mem);

        return GT_FAIL;
    }

    pthread_mutex_lock(&dma_mutex);

    if (g_dma_mem->user_addr == ptr) {
        dma_mem = g_dma_mem;

        if(g_dma_mem->next != g_dma_mem)
            g_dma_mem = g_dma_mem->next;
        else
            g_dma_mem = NULL;

    } else {
        while((dma_mem = g_dma_mem->next) != g_dma_mem) {
            if(dma_mem->user_addr == ptr)
                break;
        }

        if(dma_mem == g_dma_mem) {
            ERR("The dm instance of slot/bay(%u/%u) fail to find the dma "
                "memory info.", dm_cpss_slot, dm_cpss_bay);
            pthread_mutex_unlock(&dma_mutex);
            return GT_FAIL;
        }
    }

    dma_mem->prev->next = dma_mem->next;
    dma_mem->next->prev = dma_mem->prev;

    munmap(dma_mem->user_addr,  dma_mem->mem.size);
    if (ioctl(sub_fd, DM_SUB_IOC_FREE_DMA_MEM,
              (unsigned long)&dma_mem->mem) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to free dma memory in "
            "kernel driver.", dm_cpss_slot, dm_cpss_bay);

        osMemFree(dma_mem);
        pthread_mutex_unlock(&dma_mutex);
        return GT_FAIL;
    }

    osMemFree(dma_mem);
    pthread_mutex_unlock(&dma_mutex);

    return GT_OK;
}

/*******************************************************************************
* osMemPhyToVirt
*
* DESCRIPTION:
*       Converts physical address to virtual.
*
* INPUTS:
*       phyAddr  - physical address
*
* OUTPUTS:
*       virtAddr - virtual address
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS osMemPhyToVirt
(
    IN  GT_UINTPTR phyAddr,
    OUT GT_UINTPTR *virtAddr
)
{
    dma_mem_info_t *dma_mem;
    if(virtAddr == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, virtAddr);

        return GT_BAD_PARAM;
    }

    if(g_dma_mem == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad env(%p).",
            dm_cpss_slot, dm_cpss_bay, g_dma_mem);

        return GT_FAIL;
    }

    pthread_mutex_lock(&dma_mutex);

    if((phyAddr >= (GT_UINTPTR)g_dma_mem->mem.bus_addr) &&
       (phyAddr < (GT_UINTPTR)(g_dma_mem->mem.bus_addr + g_dma_mem->mem.size))) {
        *virtAddr = (GT_UINTPTR)(unsigned long)g_dma_mem->user_addr +
                    (phyAddr - (GT_UINTPTR)g_dma_mem->mem.bus_addr);
        pthread_mutex_unlock(&dma_mutex);
        return GT_OK;
    }

    while((dma_mem = g_dma_mem->next) != g_dma_mem) {
        if((phyAddr >= (GT_UINTPTR)dma_mem->mem.bus_addr) &&
           (phyAddr < (GT_UINTPTR)(dma_mem->mem.bus_addr + dma_mem->mem.size))) {
            *virtAddr = (GT_UINTPTR)(unsigned long)dma_mem->user_addr +
                        (phyAddr - (GT_UINTPTR)dma_mem->mem.bus_addr);
            pthread_mutex_unlock(&dma_mutex);
            return GT_OK;
        }

    }

    pthread_mutex_unlock(&dma_mutex);

    ERR("The dm instance of slot/bay(%u/%u) fail to do convert the physical "
        "address(%lu) to virtual address. ",
        dm_cpss_slot, dm_cpss_bay, (unsigned long)phyAddr);

    return GT_FAIL;

}

/*******************************************************************************
* osMemVirtToPhy
*
* DESCRIPTION:
*       Converts virtual address to physical.
*
* INPUTS:
*       virtAddr - virtual address
*
* OUTPUTS:
*       phyAddr  - physical address
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS osMemVirtToPhy
(
    IN  GT_UINTPTR virtAddr,
    OUT GT_UINTPTR *phyAddr
)
{
    dma_mem_info_t *dma_mem;
    if(phyAddr == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, phyAddr);

        return GT_BAD_PARAM;
    }

    if(g_dma_mem == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad env(%p).",
            dm_cpss_slot, dm_cpss_bay, g_dma_mem);

        return GT_FAIL;
    }

    pthread_mutex_lock(&dma_mutex);

    if((virtAddr >= (GT_UINTPTR)g_dma_mem->user_addr) &&
       (virtAddr < (GT_UINTPTR)((unsigned long)g_dma_mem->user_addr +
                                g_dma_mem->mem.size))){
        *phyAddr = (GT_UINTPTR)(unsigned long)g_dma_mem->mem.bus_addr +
                    (virtAddr - (GT_UINTPTR)g_dma_mem->user_addr);
        pthread_mutex_unlock(&dma_mutex);
        return GT_OK;
    }

    while((dma_mem = g_dma_mem->next) != g_dma_mem) {
        if((virtAddr >= (GT_UINTPTR)dma_mem->user_addr) &&
           (virtAddr < (GT_UINTPTR)((unsigned long)dma_mem->user_addr +
                                     dma_mem->mem.size))) {
            *phyAddr = (GT_UINTPTR)(unsigned long)dma_mem->mem.bus_addr +
                        (virtAddr - (GT_UINTPTR)dma_mem->user_addr);
            pthread_mutex_unlock(&dma_mutex);
            return GT_OK;
        }

    }

    pthread_mutex_unlock(&dma_mutex);

    ERR("The dm instance of slot/bay(%u/%u) fail to do convert the virtual "
        "address(%lu) to physical address.",
        dm_cpss_slot, dm_cpss_bay, (unsigned long)virtAddr);

    return GT_FAIL;
}



/*******************************************************************************
* osStrlen
*
* DESCRIPTION:
*       Determine the length of a string.
*
* INPUTS:
*       source  - string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       size    - number of characters in string, not including EOS.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 osStrlen
(
    IN const GT_VOID * source
)
{
    return strlen(source);
}

/*******************************************************************************
* osStrCpy
*
* DESCRIPTION:
*       Copies string 'source' (including EOS) to string 'dest'.
*
* INPUTS:
*       dest    - pointer to a buffer for the copied string
*       source  - string to copy
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the 'dest'.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrCpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source
)
{
    return strcpy(dest, source);
}

/*******************************************************************************
* osStrNCpy
*
* DESCRIPTION:
*       Copies string 'source' (including EOS) to string 'dest'.
*
* INPUTS:
*       dest    - pointer to a buffer for the copied string
*       source  - string to copy
*       len     - copy no more than len characters
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the 'dest'.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrNCpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source,
    IN GT_U32          len
)
{
    return strncpy(dest, source, len);
}

/*******************************************************************************
* osStrChr
*
* DESCRIPTION:
*       Find the first occurrence of a character in a string.
*
* INPUTS:
*       source      - string to look in to
*       character   - character to look for
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the found character or
*       NULL - if character were not found
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrChr
(
    IN const GT_CHAR   *source,
    IN GT_32           character
)
{
    return strchr(source, character);
}

/*******************************************************************************
* osStrCmp
*
* DESCRIPTION:
*       Compares lexicographically the null terminating strings str1 and str2.
*
* INPUTS:
*       str1   - string to look in to
*       str2   - character to look for
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       > 0  - if str1 is alfabetic bigger than str2
*       == 0 - if str1 is equal to str2
*       < 0  - if str1 is alfabetic smaller than str2
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osStrCmp
(
    IN const GT_CHAR   *str1,
    IN const GT_CHAR   *str2
)
{
    return strcmp(str1, str2);
}

/*******************************************************************************
* osStrNCmp
*
* DESCRIPTION:
*       Compares lexicographically the null terminating strings str1 and str2.
*
* INPUTS:
*       str1   - string to look in to
*       str2   - character to look for
*       len    - number of characters to compare
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       > 0  - if str1 is alfabetic bigger than str2
*       == 0 - if str1 is equal to str2
*       < 0  - if str1 is alfabetic smaller than str2
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osStrNCmp
(
    IN const GT_CHAR   *str1,
    IN const GT_CHAR   *str2,
    IN GT_U32          len
)
{
    return strncmp(str1, str2, len);
}

/*******************************************************************************
* osStrCat
*
* DESCRIPTION:
*       Appends a copy of string 'str2' to the end of string 'str1'.
*
* INPUTS:
*       str1   - destination string
*       str2   - sthring to add the destination string
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the destination string (str1)
*
* COMMENTS:
*       The resulting string is null-terminated.
*
*******************************************************************************/
GT_CHAR *osStrCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2
)
{
    return strcat(str1, str2);
}

/*******************************************************************************
* osStrNCat
*
* DESCRIPTION:
*       Appends up to 'len' characters from string 'str1' to the end
*       of string 'str2'.
*
* INPUTS:
*       str1   - destination string
*       str2   - string to add the destination string
*       len    - number of characters to cat
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the destination string (str1)
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrNCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2,
    IN GT_U32          len
)
{
    return strncat(str1, str2, len);
}

/*******************************************************************************
* osToUpper
*
* DESCRIPTION:
*       Converts a lower-case letter to the corresponding upper-case letter.
*
* INPUTS:
*       character   - a character to convert to upper case
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The upper case character
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osToUpper
(
    IN const GT_32 character
)
{
    return toupper(character);
}

/*******************************************************************************
* osStrTo32
*
* DESCRIPTION:
*       Converts the initial portion of the string s to long integer
*       representation.
*
* INPUTS:
*       string   - a string to convert
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The converted value represented as a long.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osStrTo32
(
    IN const GT_CHAR *string
)
{
    return atoi(string);
}

/*******************************************************************************
* osStrToU32
*
* DESCRIPTION:
*       Converts the initial portion of the string s to unsigned long integer
*       representation.
*
* INPUTS:
*       string   - a string to convert
*       endPtr   - ptr to final string
*       base     - radix
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The converted value or ZERO, if no conversion could be performed.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 osStrToU32
(
    IN const GT_CHAR *string,
    IN GT_CHAR **    endPtr,
    IN GT_32         base
)
{
    return strtoul(string, endPtr, base);
}

/*******************************************************************************
* osSemCCreate
*
* DESCRIPTION:
*       Create counting semaphore.
*
* INPUTS:
*       name   - semaphore Name
*       init   - init value of semaphore
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS osSemCCreate
(
    IN  const char              *name,
    IN  GT_U32                  init,
    OUT CPSS_OS_SIG_SEM         *smid
)
{
    count_sem_t *csem;

    /*
     * not used, suppress compilation warning
     */
    (void)name;

    if(smid == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, smid);

        return GT_BAD_PARAM;
    }
    csem = malloc(sizeof(count_sem_t));
    if(csem == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc memory to create "
            "a counting semaphore.", dm_cpss_slot, dm_cpss_bay);

        return GT_FAIL;
    }

    pthread_mutex_init(&csem->mtx, NULL);
    pthread_cond_init(&csem->cond, NULL);

    csem->binary_flag = 0;
    csem->count = init;

    *smid = (CPSS_OS_SIG_SEM)csem;
    return GT_OK;
}


/*******************************************************************************
* osSemBinCreate
*
* DESCRIPTION:
*       Create and initialize a binary semaphore for signaling between tasks or
*       between Interrupt context and tasks.
*
* INPUTS:
*       name   - "signaling" semaphore Name
*       init   - init value of "signaling" semaphore (full or empty)
*
* OUTPUTS:
*       smidPtr - (pointer to)semaphore "signaling" Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
GT_STATUS osSemBinCreate
(
    IN  char                    *name,
    IN  CPSS_OS_SEMB_STATE_ENT  init,
    OUT CPSS_OS_SIG_SEM         *smidPtr
)
{
    count_sem_t *csem;

    /*
     * not used, suppress compilation warning
     */
    (void)name;

    if(smidPtr == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, smidPtr);

        return GT_BAD_PARAM;
    }
    csem = malloc(sizeof(count_sem_t));
    if(csem == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc memory to create "
            "a binary semaphore.", dm_cpss_slot, dm_cpss_bay);

        return GT_FAIL;
    }

    pthread_mutex_init(&csem->mtx, NULL);
    pthread_cond_init(&csem->cond, NULL);

    csem->binary_flag = 1;
    csem->count = (init == CPSS_OS_SEMB_EMPTY_E)? 0 : 1;

    *smidPtr = (CPSS_OS_SIG_SEM)csem;
    return GT_OK;
}

/*******************************************************************************
* osSemMCreate
*
* DESCRIPTION:
*       Create and initialize a mutext semaphore.
*
* INPUTS:
*       name   - semaphore Name
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       This is not API. Should be used as development tool only.
*       Will be removed.
*
*******************************************************************************/
GT_STATUS osSemMCreate
(
    IN  const char      *name,
    OUT CPSS_OS_SIG_SEM *smid
)
{
    /*
     * not used, suppress compilation warning
     */
    (void)name;

    return osSemBinCreate(NULL, CPSS_OS_SEMB_EMPTY_E, smid);
}

/*******************************************************************************
* osSemDelete
*
* DESCRIPTION:
*       Delete "signaling" semaphore.
*
* INPUTS:
*       smid - semaphore Id (as created by
*                 CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
GT_STATUS osSemDelete
(
    IN CPSS_OS_SIG_SEM smid
)
{
    int ret = 0;
    count_sem_t *csem = (count_sem_t*)smid;

    ret = pthread_mutex_destroy(&csem->mtx);
    if(ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to delete the mutex of a "
            "semaphore. ERROR = %d", dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    };

    ret = pthread_cond_destroy(&csem->cond);
    if(ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to delete the cond of a "
            "semaphore. ERROR = %d", dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    };

    free(csem);

    return GT_OK;
}

/*******************************************************************************
* osSemWait
*
* DESCRIPTION:
*       Wait on "signaling" semaphore.
*
* INPUTS:
*       smid    - "signaling" semaphore Id (as created by
*                 CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
*       timeOut - time out in milliseconds or 0 to wait forever
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_TIMEOUT - on time out
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
GT_STATUS osSemWait
(
    IN CPSS_OS_SIG_SEM  smid,
    IN GT_U32       timeOut
)
{
    int ret = 0;
    struct timespec out, now;
    count_sem_t *csem = (count_sem_t*)smid;

    pthread_mutex_lock(&csem->mtx);
    if (csem->count) {
        /* wait successful */
        csem->count--;

        pthread_mutex_unlock(&csem->mtx);
        return GT_OK;
    }

    if (timeOut == CPSS_OS_SEM_NO_WAIT_CNS) {
        /* no wait */
        ERR("The dm instance of slot/bay(%u/%u) fail to wait a semaphore "
            "without wait.", dm_cpss_slot, dm_cpss_bay);
        pthread_mutex_unlock(&csem->mtx);
        return GT_FAIL;
    }


    if (timeOut == CPSS_OS_SEM_WAIT_FOREVER_CNS) {
        /* wait forever */
        while (csem->count == 0 && ret == 0)
            ret = pthread_cond_wait(&csem->cond, &csem->mtx);

    } else {
        /* wait no more than timeOut milliseconds */
        clock_gettime(CLOCK_REALTIME, &now);

        if (timeOut < 1000) {
            out.tv_sec  = now.tv_sec;
            out.tv_nsec = now.tv_nsec + timeOut * 1000000;
        } else {
            out.tv_sec = now.tv_sec + timeOut / 1000;
            out.tv_nsec = now.tv_nsec + (timeOut % 1000) * 1000000;
        }

        if (out.tv_nsec >= 1000000000) {
            out.tv_sec++;
            out.tv_nsec -= 1000000000;
        }

        while (csem->count == 0 && ret == 0)
            ret = pthread_cond_timedwait(&csem->cond, &csem->mtx, &out);

        if(ret == ETIMEDOUT) {
            /* wait failed (timeout) */
            ERR("The dm instance of slot/bay(%u/%u) fail to wait a semaphore "
                "with a timeout.", dm_cpss_slot, dm_cpss_bay);
            pthread_mutex_unlock(&csem->mtx);
            return GT_TIMEOUT;
        }
    }

    if (csem->count > 0 && ret == 0) {
        /* wait successful */
        csem->count--;
        pthread_mutex_unlock(&csem->mtx);
        return GT_OK;
    } else {
        ERR("The dm instance of slot/bay(%u/%u) fail to wait a semaphore "
            "with error(%d).", dm_cpss_slot, dm_cpss_bay, ret);
        pthread_mutex_unlock(&csem->mtx);
        return GT_FAIL;
    }
}

/*******************************************************************************
* osSemSignal
*
* DESCRIPTION:
*       Signal a "signaling" semaphore.
*
* INPUTS:
*       smid    - "signaling" semaphore Id (as created by
*                 CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
GT_STATUS osSemSignal
(
    IN CPSS_OS_SIG_SEM smid
)
{
    count_sem_t *csem = (count_sem_t*)smid;

    pthread_mutex_lock(&csem->mtx);

    if(csem->binary_flag)
        csem->count = 1;
    else
        csem->count++;

    pthread_cond_signal(&csem->cond);
    pthread_mutex_unlock(&csem->mtx);

    return GT_OK;
}

/*******************************************************************************
* osMutexCreate
*
* DESCRIPTION:
*       Create and initialize a mutex (mutual exclusion object).
*       This object used for protecting resources in HW or in SW , that
*       accessed from more then one SW task.
*
* INPUTS:
*       name   - mutex Name
*
* OUTPUTS:
*       mtxidPtr - (pointer to)mutex Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
GT_STATUS osMutexCreate
(
    IN  char                    *name,
    OUT CPSS_OS_MUTEX           *mtxidPtr
)
{
    pthread_mutex_t *mtx;
    pthread_mutexattr_t mattr;

    /*
     * not used, suppress compilation warning
     */
    (void)name;

    mtx = malloc(sizeof(pthread_mutex_t));
    if(mtx == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc a memory buffer"
            "for mutex.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    /*
     * initialize mutex to a recursive mutex
     */
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mtx, &mattr);
    pthread_mutexattr_destroy(&mattr);

    *mtxidPtr = (CPSS_OS_MUTEX)mtx;
    return GT_OK;
}

/*******************************************************************************
* osMutexDelete
*
* DESCRIPTION:
*       Delete mutex object
*
* INPUTS:
*       mtxid - mutex Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
GT_STATUS osMutexDelete
(
    IN CPSS_OS_MUTEX mtxid
)
{
    int ret = 0;
    pthread_mutex_t *mtx = (pthread_mutex_t *)mtxid;

    pthread_mutex_destroy(mtx);
    if(ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to delete the mutex. "
            "ERROR = %d", dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    };

    free(mtx);

    return GT_OK;
}

/*******************************************************************************
* osMutexLock
*
* DESCRIPTION:
*       Lock resource using specified mutex
*
* INPUTS:
*       mtxid    - mutex Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_TIMEOUT - on time out
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
GT_STATUS osMutexLock
(
    IN CPSS_OS_MUTEX  mtxid
)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mtxid;

    pthread_mutex_lock(mtx);

    return GT_OK;
}

/*******************************************************************************
* osMutexUnlock
*
* DESCRIPTION:
*       Unlock mutex
*
* INPUTS:
*       mtxid    - mutex Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
GT_STATUS osMutexUnlock
(
    IN CPSS_OS_MUTEX mtxid
)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mtxid;

    pthread_mutex_unlock(mtx);

    return GT_OK;
}



/*******************************************************************************
* osBindStdOut
*
* DESCRIPTION:
*       Bind Stdout to handler
*
* INPUTS:
*       writeFunction - function to call for output
*       userPtr       - first parameter to pass to write function
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CPSS_OS_BIND_STDOUT_FUNC_PTR writeFuncPtr = NULL;
static void* writeFuncParam = NULL;

GT_STATUS osBindStdOut
(
    IN CPSS_OS_BIND_STDOUT_FUNC_PTR writeFunction,
    IN void* userPtr
)
{
    writeFuncPtr = writeFunction;
    writeFuncParam = userPtr;
    return GT_OK;
}

/*******************************************************************************
* osVprintf
*
* DESCRIPTION:
*       Write a formatted string to the standard output stream.
*
* INPUTS:
*       format  - format string to write
*       args    - va_list
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters written, or a negative value if
*       an output error occurs.
*
* COMMENTS:
*       None
*******************************************************************************/
int osVprintf
(
    IN  const char* format,
    IN  va_list args
)
{
    int ret;
    char buff[2048];

    if (writeFuncPtr != NULL) {
        ret = vsnprintf(buff, sizeof(buff), format, args);
        if (ret <= 0)
            return ret;

        return writeFuncPtr(writeFuncParam, buff, ret);
    }

    ret = vprintf(format, args);
    if (ret <= 0) {
        return ret;
    }

    /* should be checked if needed */
    fflush(stdout);

    return ret;
}

/*******************************************************************************
* osPrintf
*
* DESCRIPTION:
*       Write a formatted string to the standard output stream.
*
* INPUTS:
*       format  - format string to write
*       ... - parameters of the 'format'
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters written, or a negative value if
*       an output error occurs.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int osPrintf
(
    IN  const char* format,
    IN  ...
)
{
    va_list args;
    int     ret;

    va_start(args, format);
    ret = osVprintf(format, args);
    va_end(args);

    return ret;
}

/*******************************************************************************
* osSprintf
*
* DESCRIPTION:
*       Write a formatted string to a buffer.
*
* INPUTS:
*       buffer  - buffer to write to
*       format  - format string
*       ... - parameters of the 'format'
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters copied to buffer, not including
*       the NULL terminator.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int osSprintf
(
    IN  char * buffer,
    IN  const char* format,
    IN  ...
)
{
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buffer, format, args);
    va_end(args);

    return i;
}

/*******************************************************************************
* osVsprintf
*
* DESCRIPTION:
*       Write a formatted string to a buffer.
*
* INPUTS:
*       buffer  - buffer to write to
*       format  - format string
*       args    - va_list
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters copied to buffer, not including
*       the NULL terminator.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int osVsprintf
(
    IN  char * buffer,
    IN  const char* format,
    IN  va_list args
)
{
    return vsprintf(buffer, format, args);
}

/*******************************************************************************
* osIoPrintSynch
*
* DESCRIPTION:
*       Write a formatted string to the standard output stream.
*
* INPUTS:
*       format  - format string to write
*       ... - parameters of the 'format'
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters written, or a negative value if
*       an output error occurs.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int osIoPrintSynch(
    IN  const char* format,
    IN  ...
)
{
    va_list args;
    int     ret;

    va_start(args, format);
    ret = osVprintf(format, args);
    va_end(args);

    fflush(stdout);

    return ret;
}


/*******************************************************************************
* osGets
*
* DESCRIPTION:
*       Reads characters from the standard input stream into the array
*       'bufferPtr' until end-of-file is encountered or a new-line is read.
*       Any new-line character is discarded, and a null character is written
*       immediately after the last character read into the array.
*
* INPUTS:
*       bufferPtr  - (pointer to) buffer to write to
*
* OUTPUTS:
*       bufferPtr  - (pointer to)buffer with read data
*
* RETURNS:
*       A pointer to 'bufferPtr', or a null pointer if end-of-file is
*       encountered and no characters have been read, or there is a read error.
*
* COMMENTS:
*       None
*
*******************************************************************************/
char *osGets
(
    INOUT char * bufferPtr
)
{
    return fgets(bufferPtr, 1024, stdin);
}


/*******************************************************************************
* osNtohl
*
* DESCRIPTION:
*       convert long integer from network order to host order
*
* INPUTS:
*       data - long integer in network order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       long integer in host order
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 osNtohl
(
    IN GT_U32   data
)
{
    return ntohl(data);
}

/*******************************************************************************
* osHtonl
*
* DESCRIPTION:
*       convert long  integer from host order to network order
*
* INPUTS:
*       long integer in host order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       long integer in network order
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 osHtonl
(
    IN GT_U32   data
)
{
    return htonl(data);
}

/*******************************************************************************
* osNtohs
*
* DESCRIPTION:
*       convert short integer from network order to host order
*
* INPUTS:
*       data - short integer in network order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       short integer in host order
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U16 osNtohs
(
    IN GT_U16   data
)
{
    return ntohs(data);
}

/*******************************************************************************
* osHtons
*
* DESCRIPTION:
*       convert short integer from host order to network order
*
* INPUTS:
*       short integer in host order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       short integer in network order
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U16 osHtons
(
    IN GT_U16   data
)
{
    return htons(data);
}

/*******************************************************************************
* osInetNtoa
*
* DESCRIPTION:
*       convert an network address to dot notation, store it in a buffer.
*
* INPUTS:
*       ipAddr -  inet address
*
* OUTPUTS:
*       buf    -  where to return ASCII string
*
* RETURNS:
*
*
* COMMENTS:
*       None
*
*******************************************************************************/
void osInetNtoa
(
    IN  GT_U32      ipAddr,
    OUT GT_U8       *buf
)
{
    struct in_addr in;
    char *addr;

    in.s_addr = htonl(ipAddr);
    addr = inet_ntoa(in);

    if(buf != NULL) {
        strcpy((char*)buf, addr);
    }
}


/*******************************************************************************
* osTimerWkAfter
*
* DESCRIPTION:
*       Puts current task to sleep for specified number of millisecond.
*
* INPUTS:
*       mils - time to sleep in milliseconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osTimerWkAfter
(
    IN GT_U32 mils
)
{
    int ret;
    GT_U32 sec = mils / 1000;
    GT_U32 msec =  mils % 1000;

    if(sec > 0) {
        ret = sleep(sec);
        if(ret > 0)
            sleep(ret);
    }

    if(msec)
        usleep(msec*1000);

    return GT_OK;
}

/*******************************************************************************
* osTickGet
*
* DESCRIPTION:
*       Gets the value of the kernel's tick counter.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The tick counter value.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 osTickGet
(
    void
)
{
    return (GT_U32)(times(NULL));
}


/*******************************************************************************
* osTimeGet
*
* DESCRIPTION:
*       Gets number of seconds passed since system boot
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The second counter value.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 osTimeGet
(
    void
)
{
    return (GT_U32)time(NULL);
}

/*******************************************************************************
* osTimeRT
*
* DESCRIPTION:
*       Get the current time with nanoseconds
*
* INPUTS:
*       None
*
* OUTPUTS:
*       seconds     - elapsed time in seconds
*       nanoSeconds - elapsed time within a second
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS osTimeRT
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
)
{
    struct timespec tv;

    if (clock_gettime(CLOCK_REALTIME, &tv) < 0)
        return GT_FAIL;

    if (seconds != NULL)
        *seconds = (GT_U32)tv.tv_sec;
    if (nanoSeconds != NULL)
        *nanoSeconds = (GT_U32)tv.tv_nsec;

    return GT_OK;
}

/*******************************************************************************
* osGetSysClockRate
*
* DESCRIPTION:
*       Get the system clock rate
*
* INPUTS:
*       None
*
* OUTPUTS:
*       ticks - The number of ticks per second of the system clock.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS osGetSysClockRate
(
    OUT GT_U32  *ticks
)
{
    *ticks = (GT_U32)sysconf(_SC_CLK_TCK);
    return GT_OK;
}

/*******************************************************************************
* osDelay
*
* DESCRIPTION:
*       System Delay nanoseconds.
*
* INPUTS:
*       delay - non operational time in nanoseconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS osDelay
(
    IN GT_U32 nanosec
)
{
    struct timespec timeout, remain ;

    timeout.tv_nsec = nanosec % 1000000000;
    timeout.tv_sec  = nanosec / 1000000000;

    while(nanosleep(&timeout, &remain)) {
        if(errno != EINTR)
            return GT_FAIL;

        timeout = remain;
    }

    return GT_OK;
}



/*******************************************************************************
* osRand
*
* DESCRIPTION:
*       Generates a pseudo-random integer between 0 and RAND_MAX
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       rValue    - pseudo-random integer
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osRand(void)
{
    return rand();
}

/*******************************************************************************
* osSrand
*
* DESCRIPTION:
*       Reset the value of the seed used to generate random numbers.
*
* INPUTS:
*       seed  - random number seed .
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void osSrand
(
    GT_U32 seed
)
{
    srand(seed);
    return;
}


/*******************************************************************************
* osTaskCreate
*
* DESCRIPTION:
*       Create OS Task/Thread and start it.
*
* INPUTS:
*       name    - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
*       prio    - task priority 0: SCHED_OTHER, 1 - 64 SCHED_RR => HIGH
*       stack   - task Stack Size
*       start_addr - task Function to execute
*       arglist    - pointer to list of parameters for task function
*
* OUTPUTS:
*       tid   - Task ID
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osTaskCreate
(
    IN  GT_CHAR    *name,
    IN  GT_U32     prio,
    IN  GT_U32     stack,
    IN  unsigned   (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID    *arglist,
    OUT CPSS_TASK  *tid
)
{
    int ret = 0;
    pthread_t pid;
    pthread_attr_t attr;
    struct sched_param param;
    GT_U32 stack_min;

    /*
     * not used, suppress compilation warning
     */
    (void)name;

    pthread_attr_init(&attr);
    stack_min = PTHREAD_STACK_MIN;

    if (stack < stack_min)
        stack = stack_min;
    pthread_attr_setstacksize(&attr, stack);

    /*
     * if prio is not 0, set the sched policy to RR
     * it is only for ISR routine.
     */
    if(prio != 0) {
        param.sched_priority = prio;
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    }

    ret = pthread_create(&pid, &attr, (void*(*)(void*))start_addr, arglist);
    if (ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to create a thread(%d).",
            dm_cpss_slot, dm_cpss_bay, ret);

        *tid = 0;
        pthread_attr_destroy(&attr);
        return GT_FAIL;
    }

    *tid = (CPSS_TASK)pid;

    pthread_attr_destroy(&attr);

    return GT_OK;

}

/*******************************************************************************
* osTaskDelete
*
* DESCRIPTION:
*       Deletes existing task/thread.
*
* INPUTS:
*       tid   - Task ID
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       If tid = 0, delete calling task (itself)
*
*******************************************************************************/
GT_STATUS osTaskDelete
(
    IN CPSS_TASK tid
)
{
    if(tid == 0) {
        pthread_exit(0);
    } else {
        if(pthread_cancel((pthread_t)tid)) {
            ERR("The dm instance of slot/bay(%u/%u) fail to delete a thread.",
                dm_cpss_slot, dm_cpss_bay);

            return GT_FAIL;
        }
    }

    return GT_OK;
}

/*******************************************************************************
* osTaskWait
*
* DESCRIPTION:
*       Wait task/thread to exit.
*
* INPUTS:
*       tid   - Task ID
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS osTaskWait
(
    IN CPSS_TASK tid,
    IN GT_VOID **th_ret
)
{
    int ret = 0;
    if(tid == 0) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad tid.",
            dm_cpss_slot, dm_cpss_bay);

        return GT_BAD_PARAM;
    }

    ret = pthread_join((pthread_t)tid, th_ret);
    if(ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to wait a thread(%d, %d).",
            dm_cpss_slot, dm_cpss_bay, ret, errno);

        return GT_FAIL;
    }

    return GT_OK;
}

/*******************************************************************************
* osTaskGetSelf
*
* DESCRIPTION:
*       returns the current task (thread) id
*
* INPUTS:
*       none
*
* OUTPUTS:
*       tid -  the current task (thread) id
*
* RETURNS:
*       GT_OK - on success.
*       GT_FAIL - if parameter is invalid
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osTaskGetSelf
(
    OUT CPSS_TASK *tid
)
{
    if(tid == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, tid);

        return GT_BAD_PARAM;
    }

    *tid = (CPSS_TASK)pthread_self();

    return GT_OK;
}

/*******************************************************************************
* CPSS_OS_TASK_LOCK_FUNC
*
* DESCRIPTION:
*       Disable task rescheduling of current task.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
static pthread_mutex_t task_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

GT_STATUS osTaskLock (GT_VOID)
{
    if(pthread_mutex_lock(&task_mutex)) {
        ERR("The dm instance of slot/bay(%u/%u) fail to lock a thread.",
            dm_cpss_slot, dm_cpss_bay);

        return GT_FAIL;
    }

    return GT_OK;
}

/*******************************************************************************
* osTaskUnLock
*
* DESCRIPTION:
*       Enable task rescheduling.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/

GT_STATUS osTaskUnLock (GT_VOID)
{
    if(pthread_mutex_unlock(&task_mutex)) {
        ERR("The dm instance of slot/bay(%u/%u) fail to unlock a thread.",
            dm_cpss_slot, dm_cpss_bay);

        return GT_FAIL;
    }

    return GT_OK;
}


/*******************************************************************************
* osQsort
*
* DESCRIPTION:
*       Sort an array
*
* INPUTS:
*       array      - Address of first element in array
*       nItems     - Number of items in array
*       itemSize   - Size of one member
*       comparFunc - Function which compares two items
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osQsort
(
    IN  GT_VOID_PTR                     array,
    IN  GT_U32                          nItems,
    IN  GT_U32                          itemSize,
    IN  CPSS_OS_COMPARE_ITEMS_FUNC_PTR  comparFunc
)
{
    qsort(array, nItems, itemSize, comparFunc);
    return GT_OK;
}

/*******************************************************************************
* osBsearch
*
* DESCRIPTION:
*       binary search of a sorted array
*
* INPUTS:
*       key        - key to search
*       array      - Address of first element in array
*       nItems     - Number of items in array
*       itemSize   - Size of one member
*       comparFunc - Function which compares two items
*
* OUTPUTS:
*       result     - pointer to item found or NULL
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osBsearch
(
    IN  const void *                    key,
    IN  const void *                    array,
    IN  GT_U32                          nItems,
    IN  GT_U32                          itemSize,
    IN  CPSS_OS_COMPARE_ITEMS_FUNC_PTR  comparFunc,
    OUT GT_VOID_PTR                     *result
)
{
    void *ret;

    ret = bsearch(key, array, nItems, itemSize, comparFunc);

    if (result != NULL)
        *result = ret;

    return (ret == NULL) ? GT_FAIL : GT_OK;
}


/*******************************************************************************
* osMsgQCreate
*
* DESCRIPTION:
*       Create and initialize a message queue.
*
* INPUTS:
*       name       - message queue name
*       maxMsgs    - max messages in queue
*       maxMsgSize - max length of single message
*
* OUTPUTS:
*       msgqId - message queue id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osMsgQCreate
(
    IN  const char      *name,
    IN  GT_U32          maxMsgs,
    IN  GT_U32          maxMsgSize,
    OUT CPSS_OS_MSGQ_ID *msgqId
)
{
    int qid;
    struct msqid_ds ds;
    GT_U32 bytes = maxMsgs * maxMsgSize;

    /*
     * not used, suppress compilation warning
     */
    (void)name;

    if(msgqId == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, msgqId);

        return GT_BAD_PARAM;
    }
    qid = msgget(IPC_PRIVATE, 0);
    if(qid < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to create a MsgQ.",
            dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    if(msgctl(qid, IPC_STAT, &ds) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to get the info of "
            "a MsgS(%d).", dm_cpss_slot, dm_cpss_bay, qid);

        return GT_FAIL;
    }

    if (ds.msg_qbytes < bytes) {
        ds.msg_qbytes = bytes;
        if(msgctl(qid, IPC_SET, &ds) < 0) {
            ERR("The dm instance of slot/bay(%u/%u) fail to set the max "
                "bytes(%u) of MsgQ(%d). ",
                dm_cpss_slot, dm_cpss_bay, (unsigned int)bytes, qid);

            return GT_FAIL;
        }
    }

    *msgqId = (CPSS_OS_MSGQ_ID)qid;
    return GT_OK;
}

/*******************************************************************************
* osMsgQDelete
*
* DESCRIPTION:
*       Delete message queue
*
* INPUTS:
*       msgqId - message queue Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osMsgQDelete
(
    IN CPSS_OS_MSGQ_ID msgqId
)
{
    if(msgctl((int)msgqId, IPC_RMID, NULL) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to delete a MsgQ(%d).",
            dm_cpss_slot, dm_cpss_bay, (int)msgqId);

        return GT_FAIL;
    }

    return GT_OK;
}


/*******************************************************************************
* osMsgQSend
*
* DESCRIPTION:
*       Send message to queue
*
* INPUTS:
*       msgqId       - Message queue Id
*       message      - message data pointer
*       messageSize  - message size
*       timeOut      - time out in milliseconds or
*                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_TIMEOUT   - on time out
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osMsgQSend
(
    IN CPSS_OS_MSGQ_ID  msgqId,
    IN GT_PTR           message,
    IN GT_U32           messageSize,
    IN GT_U32           timeOut
)
{
    struct msgbuf *buf;

    buf = malloc(sizeof(struct msgbuf) + messageSize);
    if(buf == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc a message buffer "
            "for sending.", dm_cpss_slot, dm_cpss_bay);

        return GT_FAIL;
    }

    /*
     * message type is not used, but considered as msg len
     */
    buf->mtype = messageSize;
    memcpy(buf->mtext, message, messageSize);

    while(msgsnd((int)msgqId, buf, messageSize, IPC_NOWAIT) < 0) {
        if(errno != EAGAIN) {
            ERR("The dm instance of slot/bay(%u/%u) fail to send a message to "
                "a MsgQ(%d).", dm_cpss_slot, dm_cpss_bay, (int)msgqId);
            free(buf);
            return GT_FAIL;
        }

        if(timeOut == CPSS_OS_MSGQ_NO_WAIT) {
            ERR("The dm instance of slot/bay(%u/%u) fail to send a message to "
                "a MsgQ(%d) without wait.",
                dm_cpss_slot, dm_cpss_bay, (int)msgqId);
            free(buf);
            return GT_FAIL;
        } else if(timeOut == CPSS_OS_MSGQ_WAIT_FOREVER) {
            continue;
        } else {
            if(timeOut-- == 0) {
                ERR("The dm instance of slot/bay(%u/%u) fail to send a message "
                    "to a MsgQ(%d) with timeout(%u).",
                    dm_cpss_slot, dm_cpss_bay,
                    (int)msgqId, (unsigned int)timeOut);
                free(buf);
                return GT_TIMEOUT;
            }
            osTimerWkAfter(1);
        }
    }

    free(buf);
    return GT_OK;
}

/*******************************************************************************
* osMsgQRecv
*
* DESCRIPTION:
*       Receive message from queue
*
* INPUTS:
*       msgqId       - Message queue Id
*       messageSize  - size of buffer pointed by message
*       timeOut      - time out in milliseconds or
*                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* OUTPUTS:
*       message      - message data pointer
*       messageSize  - actual message size
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_TIMEOUT   - on time out
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osMsgQRecv
(
    IN    CPSS_OS_MSGQ_ID   msgqId,
    OUT   GT_PTR            message,
    INOUT GT_U32            *messageSize,
    IN    GT_U32            timeOut
)
{
    struct msgbuf *buf;

    if(messageSize == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, messageSize);

        return GT_BAD_PARAM;
    }

    buf = osMemMalloc(sizeof(struct msgbuf) + *messageSize);
    if(buf == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc a message buffer "
            "for receiving.", dm_cpss_slot, dm_cpss_bay);

        return GT_FAIL;
    }

    while(msgrcv((int)msgqId, buf, *messageSize, 0, IPC_NOWAIT) < 0) {
        if(errno != ENOMSG) {
            ERR("The dm instance of slot/bay(%u/%u) fail to receive a message "
                "from a MsgQ(%d).", dm_cpss_slot, dm_cpss_bay, (int)msgqId);
            free(buf);
            return GT_FAIL;
        }

        if(timeOut == CPSS_OS_MSGQ_NO_WAIT) {
            ERR("The dm instance of slot/bay(%u/%u) fail to receive a message "
                "from a MsgQ(%d) without wait.",
                dm_cpss_slot, dm_cpss_bay, (int)msgqId);
            free(buf);
            return GT_FAIL;
        } else if(timeOut == CPSS_OS_MSGQ_WAIT_FOREVER) {
            continue;
        } else {
            if(timeOut-- == 0) {
                ERR("The dm instance of slot/bay(%u/%u) fail to receive a "
                    "message from a MsgQ(%d) with timeout(%u).",
                    dm_cpss_slot, dm_cpss_bay,
                    (int)msgqId, (unsigned int)timeOut);
                free(buf);
                return GT_TIMEOUT;
            }
            osTimerWkAfter(1);
        }
    }

    memcpy(message, buf->mtext, (GT_U32)buf->mtype);
    *messageSize = (GT_U32)buf->mtype;

    free(buf);
    return GT_OK;
}

/*******************************************************************************
* osMsgQNumMsgs
*
* DESCRIPTION:
*       Return number of messages pending in queue
*
* INPUTS:
*       msgqId       - Message queue Id
*
* OUTPUTS:
*       numMessages  - number of messages pending in queue
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS osMsgQNumMsgs
(
    IN    CPSS_OS_MSGQ_ID   msgqId,
    OUT   GT_U32            *numMessages
)
{
    struct msqid_ds ds;

    if(numMessages == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p).",
            dm_cpss_slot, dm_cpss_bay, numMessages);

        return GT_BAD_PARAM;
    }

    if(msgctl((int)msgqId, IPC_STAT, &ds) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to get the info of a msg "
            "queue(%d).", dm_cpss_slot, dm_cpss_bay, (int)msgqId);

        return GT_FAIL;
    }

    *numMessages = ds.msg_qnum;
    return GT_OK;
}
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_os.c,v $
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 *
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 *
 * Revision 1.1.2.1  2014/12/02 08:04:11  iachang
 * Dreamliner Diag initial check-in.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


