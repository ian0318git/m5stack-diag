/* $Id: nim_dm_cpss_os.h,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_cpss_os.h,v $
 *------------------------------------------------------------------
 * DM CPSS lib external os service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *---------------------------------------------------------------------------
 */

#ifndef __NIM_DM_CPSS_OS_H__
#define __NIM_DM_CPSS_OS_H__

typedef struct count_sem_s {
    int binary_flag;
    int count;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} count_sem_t;

typedef struct dma_mem_info_s {
    dma_mem_t mem;
    void *user_addr;

    struct dma_mem_info_s *prev;
    struct dma_mem_info_s *next;
} dma_mem_info_t;


GT_VOID osMemBzero
(
    IN GT_CHAR *start,
    IN GT_U32 nbytes
);

GT_VOID *osMemSet
(
    IN GT_VOID *start,
    IN int symbol,
    IN GT_U32 size
);

GT_VOID *osMemCpy
(
    IN GT_VOID *destination,
    IN const GT_VOID *source,
    IN GT_U32 size
);

GT_32 osMemCmp
(
    IN const GT_VOID  *str1,
    IN const GT_VOID  *str2,
    IN GT_U32 size
);

GT_VOID *osMemStaticMalloc
(
    IN GT_U32 size
);

GT_VOID *osMemMalloc
(
    IN GT_U32 size
);

GT_VOID *osMemRealloc
(
    IN GT_VOID *ptr ,
    IN GT_U32 size
);

GT_VOID osMemFree
(
    IN GT_VOID* const memblock
);

GT_U32 osStrlen
(
    IN const GT_VOID *source
);

GT_VOID *osMemCacheDmaMalloc
(
    IN GT_U32 size
);

GT_STATUS osMemCacheDmaFree
(
    IN GT_VOID *ptr
);

GT_STATUS osMemPhyToVirt
(
    IN  GT_UINTPTR phyAddr,
    OUT GT_UINTPTR *virtAddr
);

GT_STATUS osMemVirtToPhy
(
    IN  GT_UINTPTR virtAddr,
    OUT GT_UINTPTR *phyAddr
);

GT_CHAR *osStrCpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source
);

GT_CHAR *osStrNCpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source,
    IN GT_U32          len
);

GT_CHAR *osStrChr
(
    IN const GT_CHAR   *source,
    IN GT_32           character
);

GT_32 osStrCmp
(
    IN const GT_CHAR   *str1,
    IN const GT_CHAR   *str2
);

GT_32 osStrNCmp
(
    IN const GT_CHAR   *str1,
    IN const GT_CHAR   *str2,
    IN GT_U32          len
);

GT_CHAR *osStrCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2
);

GT_CHAR *osStrNCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2,
    IN GT_U32          len
);

GT_32 osToUpper
(
    IN const GT_32 character
);

GT_32 osStrTo32
(
    IN const GT_CHAR *string
);

GT_U32 osStrToU32
(
    IN const GT_CHAR *string,
    IN GT_CHAR       **endPtr,
    IN GT_32         base
);

GT_STATUS osSemCCreate
(
    IN  const char        *name,
    IN  GT_U32            init,
    OUT CPSS_OS_SIG_SEM   *smid
);

GT_STATUS osSemBinCreate
(
    IN  const char                    *name,
    IN  CPSS_OS_SEMB_STATE_ENT  init,
    OUT CPSS_OS_SIG_SEM         *smidPtr
);

GT_STATUS osSemMCreate
(
    IN  const char      *name,
    OUT CPSS_OS_SIG_SEM *smid
);

GT_STATUS osSemDelete
(
    IN CPSS_OS_SIG_SEM smid
);

GT_STATUS osSemWait
(
    IN CPSS_OS_SIG_SEM  smid,
    IN GT_U32 timeOut
);

GT_STATUS osSemSignal
(
    IN CPSS_OS_SIG_SEM smid
);

GT_STATUS osMutexCreate
(
    IN  const char           *name,
    OUT CPSS_OS_MUTEX  *mtxidPtr
);

GT_STATUS osMutexDelete
(
    IN CPSS_OS_MUTEX mtxid
);

GT_STATUS osMutexLock
(
    IN CPSS_OS_MUTEX  mtxid
);

GT_STATUS osMutexUnlock
(
    IN CPSS_OS_MUTEX mtxid
);

GT_STATUS osBindStdOut
(
    IN CPSS_OS_BIND_STDOUT_FUNC_PTR writeFunction,
    IN void* userPtr
);

int osVprintf
(
    IN  const char* format,
    IN  va_list args
)__attribute__((format(printf, 1, 0)));

int osPrintf
(
    IN  const char* format,
    IN  ...
)__attribute__((format(printf, 1, 2)));

int osSprintf
(
    IN  char * buffer,
    IN  const char* format,
    IN  ...
)__attribute__((format(printf, 2, 3)));

int osVsprintf
(
    IN  char* buffer,
    IN  const char* format,
    IN  va_list args
)__attribute__((format(printf, 2, 0)));

int osSnprintf
(
    IN  char* buffer,
    IN  int size,
    IN  const char* format,
    IN  ...
)__attribute__((format(printf, 3, 4)));


int osVsnprintf
(
    IN char * buffer, 
    IN int size,
    IN const char* format,
    IN va_list args
)__attribute__((format(printf, 3, 0)));

int osIoPrintSynch(
    IN  const char* format,
    IN  ...
)__attribute__((format(printf, 1, 2)));

char *osGets
(
    INOUT char* bufferPtr
);

GT_U32 osNtohl
(
    IN GT_U32   data
);

GT_U32 osHtonl
(
    IN GT_U32   data
);

GT_U16 osNtohs
(
    IN GT_U16   data
);

GT_U16 osHtons
(
    IN GT_U16   data
);

void osInetNtoa
(
    IN  GT_U32  ipAddr,
    OUT GT_U8   *buf
);

GT_STATUS osTimerWkAfter
(
    IN GT_U32 mils
);

GT_U32 osTickGet
(
    void
);

GT_U32 osTimeGet
(
    void
);

GT_STATUS osTimeRT
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
);

GT_STATUS osGetSysClockRate
(
    OUT GT_U32  *ticks
);

GT_STATUS osDelay
(
    IN GT_U32 nanosec
);

GT_32 osRand(void);

void osSrand
(
    GT_U32 seed
);

GT_STATUS osTaskCreate
(
   // IN  GT_CHAR    *name,
    IN  const GT_CHAR *name,
    IN  GT_U32     prio,
    IN  GT_U32     stack,
    IN  unsigned   (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID    *arglist,
    OUT CPSS_TASK  *tid
);

GT_STATUS osTaskDelete
(
    IN CPSS_TASK tid
);

GT_STATUS osTaskWait
(
    IN CPSS_TASK tid,
    IN GT_VOID **th_ret
);

GT_STATUS osTaskGetSelf
(
    OUT CPSS_TASK *tid
);

GT_STATUS osTaskLock (GT_VOID);

GT_STATUS osTaskUnLock (GT_VOID);

GT_STATUS osQsort
(
    IN  GT_VOID_PTR                     array,
    IN  GT_U32                          nItems,
    IN  GT_U32                          itemSize,
    IN  CPSS_OS_COMPARE_ITEMS_FUNC_PTR  comparFunc
);

GT_STATUS osBsearch
(
    IN  const void *                    key,
    IN  const void *                    array,
    IN  GT_U32                          nItems,
    IN  GT_U32                          itemSize,
    IN  CPSS_OS_COMPARE_ITEMS_FUNC_PTR  comparFunc,
    OUT GT_VOID_PTR                     *result
);

GT_STATUS osMsgQCreate
(
    IN  const char      *name,
    IN  GT_U32          maxMsgs,
    IN  GT_U32          maxMsgSize,
    OUT CPSS_OS_MSGQ_ID *msgqId
);

GT_STATUS osMsgQDelete
(
    IN CPSS_OS_MSGQ_ID msgqId
);

GT_STATUS osMsgQSend
(
    IN CPSS_OS_MSGQ_ID  msgqId,
    IN GT_PTR           message,
    IN GT_U32           messageSize,
    IN GT_U32           timeOut
);

GT_STATUS osMsgQRecv
(
    IN    CPSS_OS_MSGQ_ID   msgqId,
    OUT   GT_PTR            message,
    INOUT GT_U32            *messageSize,
    IN    GT_U32            timeOut
);

GT_STATUS osMsgQNumMsgs
(
    IN    CPSS_OS_MSGQ_ID   msgqId,
    OUT   GT_U32            *numMessages
);



#endif /*__NIM_DM_CPSS_OS_H__*/

/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_os.h,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
