/* $Id: prvCpssBindFunc.h,v 1.1 2015/02/13 11:31:09 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/common/h/cpss/extServices/private/prvCpssBindFunc.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssBindFunc.h
*
* DESCRIPTION:
*       external functions wrapper. definitions for bind OS,extDrv,trace
*       dependent utilities to CPSS .
*
*       private CPSS file
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __prvCpssBindFunch
#define __prvCpssBindFunch

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <cpss/extServices/cpssExtServices.h>

/* macro to free allocation of pointer only in not NULL , and set it NULL */
#define FREE_PTR_MAC(ptr) if(ptr)cpssOsFree(ptr); ptr = NULL

/************* global *********************************************************/

extern CPSS_EXT_DRV_MGMT_CACHE_FLUSH_FUNC      cpssExtDrvMgmtCacheFlush;
extern CPSS_EXT_DRV_MGMT_CACHE_INVALIDATE_FUNC cpssExtDrvMgmtCacheInvalidate;

extern CPSS_EXTDRV_DMA_WRITE_FUNC  cpssExtDrvDmaWrite;
extern CPSS_EXTDRV_DMA_READ_FUNC   cpssExtDrvDmaRead;

extern CPSS_EXTDRV_ETH_PORT_RX_INIT_FUNC       cpssExtDrvEthPortRxInit;
extern CPSS_EXTDRV_ETH_RAW_SOCKET_MODE_SET_FUNC       cpssExtDrvEthRawSocketModeSet;
extern CPSS_EXTDRV_ETH_RAW_SOCKET_MODE_GET_FUNC       cpssExtDrvEthRawSocketModeGet;
extern CPSS_EXTDRV_LINUX_MODE_SET_FUNC       cpssExtDrvLinuxModeSet;
extern CPSS_EXTDRV_LINUX_MODE_GET_FUNC       cpssExtDrvLinuxModeGet;
extern CPSS_EXTDRV_HSU_MEM_BASE_ADDR_GET_FUNC       cpssExtDrvHsuMemBaseAddrGet;
extern CPSS_EXTDRV_HSU_WARM_RESTART_FUNC            cpssExtDrvHsuWarmRestart;
extern CPSS_EXTDRV_HSU_INBOUND_SDMA_ENABLE_FUNC     cpssExtDrvInboundSdmaEnable;
extern CPSS_EXTDRV_HSU_INBOUND_SDMA_DISABLE_FUNC    cpssExtDrvInboundSdmaDisable;
extern CPSS_EXTDRV_HSU_INBOUND_SDMA_STATE_GET_FUNC  cpssExtDrvInboundSdmaStateGet;
extern CPSS_EXTDRV_ETH_PORT_TX_INIT_FUNC       cpssExtDrvEthPortTxInit;
extern CPSS_EXTDRV_ETH_PORT_ENABLE_FUNC        cpssExtDrvEthPortEnable;
extern CPSS_EXTDRV_ETH_PORT_DISABLE_FUNC       cpssExtDrvEthPortDisable;
extern CPSS_EXTDRV_ETH_PORT_TX_FUNC            cpssExtDrvEthPortTx;
extern CPSS_EXTDRV_ETH_PORT_INPUT_HOOK_ADD_FUNC  cpssExtDrvEthInputHookAdd;
extern CPSS_EXTDRV_ETH_RAW_SOCKET_RX_HOOK_ADD_FUNC  cpssExtDrvEthRawSocketRxHookAdd;
extern CPSS_EXTDRV_ETH_PORT_TX_COMPLETE_HOOK_ADD_FUNC  cpssExtDrvEthTxCompleteHookAdd;
extern CPSS_EXTDRV_ETH_PORT_RX_PACKET_FREE_FUNC  cpssExtDrvEthRxPacketFree;
extern CPSS_EXTDRV_ETH_PORT_TX_MODE_SET_FUNC    cpssExtDrvEthPortTxModeSet;
extern CPSS_EXTDRV_ETH_CPU_CODE_TO_QUEUE        cpssExtDrvEthCpuCodeToQueue;
extern CPSS_EXTDRV_ETH_CPU_PREPEND_TWO_BYTES_FUNC cpssExtDrvEthPrePendTwoBytesHeaderSet;

extern CPSS_EXTDRV_INT_CONNECT_FUNC        cpssExtDrvIntConnect;
extern CPSS_EXTDRV_INT_ENABLE_FUNC         cpssExtDrvIntEnable;
extern CPSS_EXTDRV_INT_DISABLE_FUNC        cpssExtDrvIntDisable;
extern CPSS_EXTDRV_INT_LOCK_MODE_SET_FUNC  cpssExtDrvSetIntLockUnlock;

extern CPSS_EXTDRV_PCI_CONFIG_WRITE_REG_FUNC      cpssExtDrvPciConfigWriteReg;
extern CPSS_EXTDRV_PCI_CONFIG_READ_REG_FUNC       cpssExtDrvPciConfigReadReg;
extern CPSS_EXTDRV_PCI_DEV_FIND_FUNC              cpssExtDrvPciFindDev;
extern CPSS_EXTDRV_PCI_INT_VEC_GET_FUNC           cpssExtDrvGetPciIntVec;
extern CPSS_EXTDRV_INT_MASK_GET_FUNC              cpssExtDrvGetIntMask;
extern CPSS_EXTDRV_PCI_COMBINED_ACCESS_ENABLE_FUNC cpssExtDrvEnableCombinedPciAccess;
extern CPSS_EXTDRV_PCI_DOUBLE_WRITE_FUNC          cpssExtDrvPciDoubleWrite;
extern CPSS_EXTDRV_PCI_DOUBLE_READ_FUNC           cpssExtDrvPciDoubleRead;

extern CPSS_EXTDRV_HW_IF_SMI_INIT_DRIVER_FUNC             cpssExtDrvHwIfSmiInitDriver;
extern CPSS_EXTDRV_HW_IF_SMI_WRITE_REG_FUNC               cpssExtDrvHwIfSmiWriteReg;
extern CPSS_EXTDRV_HW_IF_SMI_READ_REG_FUNC                cpssExtDrvHwIfSmiReadReg;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_REG_RAM_READ_FUNC       cpssExtDrvHwIfSmiTskRegRamRead;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_REG_RAM_WRITE_FUNC      cpssExtDrvHwIfSmiTskRegRamWrite;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_REG_VEC_READ_FUNC       cpssExtDrvHwIfSmiTskRegVecRead;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_REG_VEC_WRITE_FUNC      cpssExtDrvHwIfSmiTskRegVecWrite;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_WRITE_REG_FUNC          cpssExtDrvHwIfSmiTaskWriteReg;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_READ_REG_FUNC           cpssExtDrvHwIfSmiTaskReadReg;
extern CPSS_EXTDRV_HW_IF_SMI_INTERRUPT_WRITE_REG_FUNC     cpssExtDrvHwIfSmiInterruptWriteReg;
extern CPSS_EXTDRV_HW_IF_SMI_INTERRUPT_READ_REG_FUNC      cpssExtDrvHwIfSmiInterruptReadReg;
extern CPSS_EXTDRV_HW_IF_SMI_DEV_VENDOR_ID_GET_FUNC       cpssExtDrvSmiDevVendorIdGet;
extern CPSS_EXTDRV_HW_IF_SMI_TASK_WRITE_REG_FIELD_FUNC    cpssExtDrvHwIfSmiTaskWriteRegField;

extern CPSS_EXTDRV_HW_IF_TWSI_INIT_DRIVER_FUNC  cpssExtDrvHwIfTwsiInitDriver;
extern CPSS_EXTDRV_HW_IF_TWSI_WRITE_REG_FUNC    cpssExtDrvHwIfTwsiWriteReg;
extern CPSS_EXTDRV_HW_IF_TWSI_READ_REG_FUNC     cpssExtDrvHwIfTwsiReadReg;

extern CPSS_EXTDRV_I2C_MGMT_MASTER_INIT_FUNC      cpssExtDrvI2cMgmtMasterInit;
extern CPSS_EXTDRV_MGMT_READ_REGISTER_FUNC        cpssExtDrvMgmtReadRegister;
extern CPSS_EXTDRV_MGMT_WRITE_REGISTER_FUNC       cpssExtDrvMgmtWriteRegister;
extern CPSS_EXTDRV_MGMT_ISR_READ_REGISTER_FUNC    cpssExtDrvMgmtIsrReadRegister;
extern CPSS_EXTDRV_MGMT_ISR_WRITE_REGISTER_FUNC   cpssExtDrvMgmtIsrWriteRegister;

extern CPSS_EXTDRV_DRAGONITE_GET_BASE_ADDRESS_FUNC cpssExtDrvDragoniteShMemBaseAddrGet;

extern CPSS_OS_INET_NTOHL_FUNC  cpssOsNtohl;
extern CPSS_OS_INET_HTONL_FUNC  cpssOsHtonl;
extern CPSS_OS_INET_NTOHS_FUNC  cpssOsNtohs;
extern CPSS_OS_INET_HTONS_FUNC  cpssOsHtons;
extern CPSS_OS_INET_NTOA_FUNC   cpssOsInetNtoa;

extern CPSS_OS_INT_ENABLE_FUNC    cpssOsIntEnable;
extern CPSS_OS_INT_DISABLE_FUNC   cpssOsIntDisable;
extern CPSS_OS_INT_MODE_SET_FUNC  cpssOsSetIntLockUnlock;
extern CPSS_OS_INT_CONNECT_FUNC   cpssOsInterruptConnect;

extern CPSS_OS_IO_BIND_STDOUT_FUNC cpssOsBindStdOut;
extern CPSS_OS_IO_PRINTF_FUNC      cpssOsPrintf;
extern CPSS_OS_IO_VPRINTF_FUNC     cpssOsVprintf;
extern CPSS_OS_IO_SPRINTF_FUNC     cpssOsSprintf;
extern CPSS_OS_IO_VSPRINTF_FUNC    cpssOsVsprintf;
extern CPSS_OS_IO_PRINT_SYNC_FUNC  cpssOsPrintSync;
extern CPSS_OS_IO_GETS_FUNC        cpssOsGets;

extern CPSS_OS_BZERO_FUNC             cpssOsBzero;
extern CPSS_OS_MEM_SET_FUNC           cpssOsMemSet;
extern CPSS_OS_MEM_CPY_FUNC           cpssOsMemCpy;
extern CPSS_OS_MEM_CMP_FUNC           cpssOsMemCmp;
/*extern CPSS_OS_STATIC_MALLOC_FUNC     cpssOsStaticMalloc; -- not to be used in CPSS !!*/
extern CPSS_OS_MALLOC_FUNC            cpssOsMalloc;
extern CPSS_OS_REALLOC_FUNC           cpssOsRealloc;
extern CPSS_OS_FREE_FUNC              cpssOsFree;
extern CPSS_OS_CACHE_DMA_MALLOC_FUNC  cpssOsCacheDmaMalloc;
extern CPSS_OS_CACHE_DMA_FREE_FUNC    cpssOsCacheDmaFree;
extern CPSS_OS_PHY_TO_VIRT_FUNC       cpssOsPhy2Virt;
extern CPSS_OS_VIRT_TO_PHY_FUNC       cpssOsVirt2Phy;

extern CPSS_OS_RAND_FUNC  cpssOsRand;
extern CPSS_OS_SRAND_FUNC cpssOsSrand;

extern CPSS_OS_MUTEX_CREATE_FUNC       cpssOsMutexCreate;
extern CPSS_OS_MUTEX_DELETE_FUNC       cpssOsMutexDelete;
extern CPSS_OS_MUTEX_LOCK_FUNC         cpssOsMutexLock;
extern CPSS_OS_MUTEX_UNLOCK_FUNC       cpssOsMutexUnlock;

extern CPSS_OS_SIG_SEM_BIN_CREATE_FUNC  cpssOsSigSemBinCreate;
extern CPSS_OS_SIG_SEM_M_CREATE_FUNC    cpssOsSigSemMCreate;
extern CPSS_OS_SIG_SEM_C_CREATE_FUNC    cpssOsSigSemCCreate;
extern CPSS_OS_SIG_SEM_DELETE_FUNC      cpssOsSigSemDelete;
extern CPSS_OS_SIG_SEM_WAIT_FUNC        cpssOsSigSemWait;
extern CPSS_OS_SIG_SEM_SIGNAL_FUNC      cpssOsSigSemSignal;

extern CPSS_OS_STR_LEN_FUNC     cpssOsStrlen;
extern CPSS_OS_STR_CPY_FUNC     cpssOsStrCpy;
extern CPSS_OS_STR_N_CPY_FUNC   cpssOsStrNCpy;
extern CPSS_OS_STR_CHR_FUNC     cpssOsStrChr;
extern CPSS_OS_STR_CMP_FUNC     cpssOsStrCmp;
extern CPSS_OS_STR_N_CMP_FUNC   cpssOsStrNCmp;
extern CPSS_OS_STR_CAT_FUNC     cpssOsStrCat;
extern CPSS_OS_STR_N_CAT_FUNC   cpssOsStrNCat;
extern CPSS_OS_TO_UPPER_FUNC    cpssOsToUpper;
extern CPSS_OS_STR_TO_32_FUNC   cpssOsStrTo32;
extern CPSS_OS_STR_TO_U32_FUNC  cpssOsStrToU32;

extern CPSS_OS_TIME_WK_AFTER_FUNC  cpssOsTimerWkAfter;
extern CPSS_OS_TIME_TICK_GET_FUNC  cpssOsTickGet;
extern CPSS_OS_TIME_GET_FUNC       cpssOsTime;
extern CPSS_OS_TIME_RT_FUNC        cpssOsTimeRT;
extern CPSS_OS_GET_SYS_CLOCK_RATE_FUNC cpssOsGetSysClockRate;
extern CPSS_OS_DELAY_FUNC          cpssOsDelay;

extern CPSS_OS_TASK_CREATE_FUNC    cpssOsTaskCreate;
extern CPSS_OS_TASK_DELETE_FUNC    cpssOsTaskDelete;
extern CPSS_OS_TASK_GET_SELF_FUNC  cpssOsTaskGetSelf;
extern CPSS_OS_TASK_LOCK_FUNC      cpssOsTaskLock;
extern CPSS_OS_TASK_UNLOCK_FUNC    cpssOsTaskUnLock;

extern CPSS_OS_STDLIB_QSORT_FUNC   cpssOsQsort;
extern CPSS_OS_STDLIB_BSEARCH_FUNC cpssOsBsearch;

extern CPSS_OS_MSGQ_CREATE_FUNC    cpssOsMsgQCreate;
extern CPSS_OS_MSGQ_DELETE_FUNC    cpssOsMsgQDelete;
extern CPSS_OS_MSGQ_SEND_FUNC      cpssOsMsgQSend;
extern CPSS_OS_MSGQ_RECV_FUNC      cpssOsMsgQRecv;
extern CPSS_OS_MSGQ_NUM_MSGS_FUNC  cpssOsMsgQNumMsgs;

extern CPSS_TRACE_HW_ACCESS_WRITE_FUNC  cpssTraceHwAccessWrite;
extern CPSS_TRACE_HW_ACCESS_READ_FUNC   cpssTraceHwAccessRead;
extern CPSS_TRACE_HW_ACCESS_DELAY_FUNC   cpssTraceHwAccessDelay;

#ifdef __cplusplus
}
#endif

#endif  /* __prvCpssBindFunch */


/*
 *------------------------------------------------------------------
 * $Log: prvCpssBindFunc.h,v $
 * Revision 1.1  2015/02/13 11:31:09  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
