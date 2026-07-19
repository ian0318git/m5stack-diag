/* $Id: cpss_common.c,v 1.2 2021/09/24 01:21:39 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/cpss_common/cpss_common.c,v $
 *------------------------------------------------------------------
 *
 * Filename:	cpss_common.c
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>

#include "cpss_extserv.h"

CPSS_TASK int_tid = 0;
int int_exit = 0;

/*******************************************************************************
* intTask
*
* DESCRIPTION:
*       Interrupt handler task.
*
* INPUTS:
*       param1  - device number
*       param2  - ISR cookie
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
static unsigned __TASKCONV intTask(GT_VOID *param)
{
    int ret = 0;
    GT_VOIDFUNCPTR routine;
    GT_U32 parameter;
    GT_U32 main_cause, gpio_data;
    unsigned int *gpio_cause;

    routine = ((struct intTaskParams*)param)->routine;
    parameter = ((struct intTaskParams*)param)->parameter;

    free(param);

    while (1) {
        /*
         * Wait for interrupt
         */
        if ((ret = ioctl(sub_fd, DM_SUB_IOC_WAIT)) < 0) {
            ERR("Fail to do cmd. Error = %d ", ret);
            return (GT_FAIL);
        }

        if(int_exit) {
            return (GT_OK);
        }
	
	INFO("ISR thread receives hardware interrupt\n");

	main_cause = *(unsigned int *)(config_addr + DM_MAIN_INT_CAUSE_HI_REG);

	if (main_cause & (0x1 << DM_PCIE_SWITCH_MG_INT_SHIFT)) {
	    /* process PP related interrupts */
	    ((void(*)(unsigned int))routine)((unsigned int)parameter);
	}

	if (main_cause & (0x1 << DM_PCIE_GPIOLO24_31_INT_SHIFT)) {
	    /* process GPIO pin related interrupts */
	    gpio_data = *(unsigned int *)(config_addr + DM_GPIOLO_DATA_IN_REG);
	    gpio_cause = (unsigned int *)(config_addr + DM_GPIOLO_INT_CAUSE_REG);
	    INFO("gpio_data = %#x, gpio_cause = %#x\n", gpio_data, *gpio_cause);

	    /* POE interrupt */
	    if (*gpio_cause & (0x1 << DM_GPIOLO_POE_INT_SHIFT)) {
		/* need to deactivate POE interrupt line */
		*gpio_cause &= ~(0x1 << DM_GPIOLO_POE_INT_SHIFT);
	    }

	    /* PHY interrupt */
	    if (*gpio_cause & (0x1 << DM_GPIOLO_PHY_INT_SHIFT)) {
		/* need to deactivate PHY interrupt line */
		*gpio_cause &= ~(0x1 << DM_GPIOLO_PHY_INT_SHIFT);
	    }
	}
    }

    return (GT_OK);
}


pcie_mapping_array_t * cpss_pcie_get_pcie_mapping (void)
{

    pcie_mapping_array_t *map_array;
    pcie_switch_port_mapping_t *ents;
#ifdef DEBUG
    printf("The dm instance get the pcie port mapping.");
#endif
    map_array = malloc(sizeof(pcie_mapping_array_t));
    if (map_array == NULL) {
        printf("The dm instance fail to alloc memory for "
               "pcie port mapping.");
        return (NULL);
    }

    map_array->num = 1;
    ents = calloc(1, sizeof(pcie_switch_port_mapping_t));
    if (ents == NULL) {
        printf("The dm instance fail to alloc memory for "
               "pcie port mapping ents.");
        free(map_array);
        return (NULL);
    }

    ents[0].slot = 0;
    ents[0].bay = 1;
    ents[0].sec_bus = 1;
    ents[0].sub_bus = 1;
    map_array->ents = (unsigned long)ents;

    return (map_array);

}

void cpss_pcie_put_pcie_mapping (pcie_mapping_array_t *map_array)
{
#ifdef DEBUG
    INFO("Put the pcie pcie_mapport mapping.");
#endif
    free((void *)(unsigned long)map_array->ents);
    free(map_array);
}

/*******************************************************************************
* extDrvPciMapEx
*
* DESCRIPTION:
*       This routine maps PP registers and PCI registers into userspace
*
* INPUTS:
*
*
* OUTPUTS:
*       pciBaseAddr
*       pciSize
*       internalPciBase
*       internalSize
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciMapEx
(
    OUT GT_UINTPTR  *pciBaseAddr,
    OUT GT_SIZE_T   *pciSize,
    OUT GT_UINTPTR  *internalPciBase,
    OUT GT_SIZE_T   *internalSize
)
{
    int ret = 0;
    void *baseAddr;
    void *internalBase;
    config_ppregs_size_t size;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_CONFIG_PPREGS_SIZE, &size)) < 0) {
        printf("The dm instance fail to do cmd "
            "DM_SUB_IOC_CONFIG_PPREGS_SIZE. Error = %d", ret);
        return (GT_FAIL);
    }

    /*
     * map config space
     */
    internalBase = mmap(0,
                        size.config_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        sub_fd,
                        0);

    if (MAP_FAILED == internalBase) {
        printf("The dm instance  fail to map the config "
            "memory space.");
        return (GT_FAIL);
    }

    /*
     * map pp register space
     */
    baseAddr = mmap(0,
                    size.ppregs_size,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    sub_fd,
                    size.config_size);

    /*
     * config space base address and size
     * Note, the offset of config register is taken care by app
     */
    *internalPciBase = (GT_UINTPTR)internalBase;
    *internalSize = size.config_size;

    /*
     * pp register space base address and size
     */
    *pciBaseAddr = (GT_UINTPTR)baseAddr;
    *pciSize = size.ppregs_size;

    return (GT_OK);
}

/*******************************************************************************
* extDrvPciUnMapEx
*
* DESCRIPTION:
*       This routine unmaps PP registers and PCI registers from userspace
*
* INPUTS:
*       pciBaseAddr
*       pciSize
*       internalPciBase
*       internalSize
*
* OUTPUTS:
*
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciUnMapEx
(
    IN GT_UINTPTR  pciBaseAddr,
    IN GT_SIZE_T   pciSize,
    IN GT_UINTPTR  internalPciBase,
    IN GT_SIZE_T   internalSize
)
{
    int ret = 0;

    if (pciBaseAddr == 0 || pciSize == 0 ||
        internalPciBase == 0 || internalSize == 0) {
        ERR("Bad param (%lu, %lu, %lu, %lu)",
            (unsigned long)pciBaseAddr, (unsigned long)pciSize,
            (unsigned long)internalPciBase, (unsigned long)internalSize);
            return (GT_BAD_PARAM);
    }

    /*
     * unmap config space
     */
    ret = munmap((void*)internalPciBase, (size_t)internalSize);
    if (ret) {
        ERR("Fail to unmap the config memory space");
        return (GT_FAIL);
    }

    /*
     * unmap pp register space
     */
    ret = munmap((void*)pciBaseAddr, (size_t)pciSize);
    if (ret) {
        ERR("Fail to unmap the pp register memory space");
        return (GT_FAIL);
    }

    return (GT_OK);
}

void cpss_pcie_get_extserv (CPSS_EXT_DRV_FUNC_BIND_STC *extDrv,
                              CPSS_OS_FUNC_BIND_STC      *os,
                              CPSS_TRACE_FUNC_BIND_STC   *trace)

{
#ifdef DEBUG
    INFO("cpss external services.");
#endif

    if (extDrv != NULL) {
        memset(extDrv, 0, sizeof(CPSS_EXT_DRV_FUNC_BIND_STC));

        extDrv->extDrvDmaBindInfo.extDrvDmaReadFunc = extDrvDmaRead;
        extDrv->extDrvDmaBindInfo.extDrvDmaWriteDriverFunc = extDrvDmaWrite;

        extDrv->extDrvIntBindInfo.extDrvIntConnectFunc = extDrvIntConnect;
        extDrv->extDrvIntBindInfo.extDrvIntEnableFunc = extDrvIntEnable;
        extDrv->extDrvIntBindInfo.extDrvIntDisableFunc = extDrvIntDisable;
        extDrv->extDrvIntBindInfo.extDrvIntLockModeSetFunc =
                                  extDrvSetIntLockUnlock;

        extDrv->extDrvPciInfo.extDrvPciConfigWriteRegFunc =
                              extDrvPciConfigWriteReg;
        extDrv->extDrvPciInfo.extDrvPciConfigReadRegFunc =
                              extDrvPciConfigReadReg;
        extDrv->extDrvPciInfo.extDrvPciDevFindFunc = extDrvPciFindDev;
        extDrv->extDrvPciInfo.extDrvPciIntVecFunc = extDrvGetPciIntVec;
        extDrv->extDrvPciInfo.extDrvPciIntMaskFunc = extDrvGetIntMask;
        extDrv->extDrvPciInfo.extDrvPciCombinedAccessEnableFunc =
                              extDrvEnableCombinedPciAccess;
        extDrv->extDrvPciInfo.extDrvPciDoubleWriteFunc = extDrvPciDoubleWrite;
        extDrv->extDrvPciInfo.extDrvPciDoubleReadFunc = extDrvPciDoubleRead;
    }

    if (os != NULL) {
        memset(os, 0, sizeof(CPSS_OS_FUNC_BIND_STC));

        os->osMemBindInfo.osMemBzeroFunc = osMemBzero;
        os->osMemBindInfo.osMemSetFunc = osMemSet;
        os->osMemBindInfo.osMemCpyFunc = osMemCpy;
        os->osMemBindInfo.osMemCmpFunc = osMemCmp;
        os->osMemBindInfo.osMemStaticMallocFunc = osMemStaticMalloc;
        os->osMemBindInfo.osMemMallocFunc = osMemMalloc;
        os->osMemBindInfo.osMemReallocFunc = osMemRealloc;
        os->osMemBindInfo.osMemFreeFunc = osMemFree;
        os->osMemBindInfo.osMemCacheDmaMallocFunc = osMemCacheDmaMalloc;
        os->osMemBindInfo.osMemCacheDmaFreeFunc = osMemCacheDmaFree;
        os->osMemBindInfo.osMemPhyToVirtFunc = osMemPhyToVirt;
        os->osMemBindInfo.osMemVirtToPhyFunc = osMemVirtToPhy;

        os->osStrBindInfo.osStrlenFunc = osStrlen;
        os->osStrBindInfo.osStrCpyFunc = osStrCpy;
        os->osStrBindInfo.osStrNCpyFunc = osStrNCpy;
        os->osStrBindInfo.osStrChrFunc = osStrChr;
        os->osStrBindInfo.osStrCmpFunc = osStrCmp;
        os->osStrBindInfo.osStrNCmpFunc = osStrNCmp;
        os->osStrBindInfo.osStrCatFunc = osStrCat;
        os->osStrBindInfo.osStrStrNCatFunc = osStrNCat;
        os->osStrBindInfo.osStrChrToUpperFunc = osToUpper;
        os->osStrBindInfo.osStrTo32Func = osStrTo32;
        os->osStrBindInfo.osStrToU32Func = osStrToU32;

        os->osSemBindInfo.osMutexCreateFunc = osMutexCreate;
        os->osSemBindInfo.osMutexDeleteFunc = osMutexDelete;
        os->osSemBindInfo.osMutexLockFunc = osMutexLock;
        os->osSemBindInfo.osMutexUnlockFunc = osMutexUnlock;
        os->osSemBindInfo.osSigSemBinCreateFunc = osSemBinCreate;
        os->osSemBindInfo.osSigSemMCreateFunc = osSemMCreate;
        os->osSemBindInfo.osSigSemCCreateFunc = osSemCCreate;
        os->osSemBindInfo.osSigSemDeleteFunc = osSemDelete;
        os->osSemBindInfo.osSigSemWaitFunc = osSemWait;
        os->osSemBindInfo.osSigSemSignalFunc = osSemSignal;

        os->osIoBindInfo.osIoBindStdOutFunc = osBindStdOut;

        os->osIoBindInfo.osIoPrintfFunc = osPrintf;
        os->osIoBindInfo.osIoVprintfFunc = osVprintf;
        os->osIoBindInfo.osIoSprintfFunc = osSprintf;
        os->osIoBindInfo.osIoVsprintfFunc = osVsprintf;
	    os->osIoBindInfo.osIoSnprintfFunc =	osSnprintf;
	    os->osIoBindInfo.osIoVsnprintfFunc = osVsnprintf; 
        os->osIoBindInfo.osIoPrintSynchFunc = osIoPrintSynch;
        os->osIoBindInfo.osIoGetsFunc = osGets;

        os->osInetBindInfo.osInetNtohlFunc = osNtohl;
        os->osInetBindInfo.osInetHtonlFunc = osHtonl;
        os->osInetBindInfo.osInetNtohsFunc = osNtohs;
        os->osInetBindInfo.osInetHtonsFunc = osHtons;
        os->osInetBindInfo.osInetNtoaFunc = osInetNtoa;

        os->osTimeBindInfo.osTimeWkAfterFunc = osTimerWkAfter;
        os->osTimeBindInfo.osTimeTickGetFunc = osTickGet;
        os->osTimeBindInfo.osTimeGetFunc = osTimeGet;
        os->osTimeBindInfo.osTimeRTFunc = osTimeRT;
        os->osTimeBindInfo.osGetSysClockRateFunc = osGetSysClockRate;
        os->osTimeBindInfo.osDelayFunc = osDelay;

        os->osIntBindInfo.osIntEnableFunc =
                (CPSS_OS_INT_ENABLE_FUNC)extDrvIntEnable;
        os->osIntBindInfo.osIntDisableFunc =
                (CPSS_OS_INT_DISABLE_FUNC)extDrvIntDisable;
        os->osIntBindInfo.osIntModeSetFunc =
                (CPSS_OS_INT_MODE_SET_FUNC)extDrvSetIntLockUnlock;
        os->osIntBindInfo.osIntConnectFunc =
                (CPSS_OS_INT_CONNECT_FUNC)extDrvIntConnect;

        os->osRandBindInfo.osRandFunc = osRand;
        os->osRandBindInfo.osSrandFunc = osSrand;

        os->osTaskBindInfo.osTaskCreateFunc = osTaskCreate;
        os->osTaskBindInfo.osTaskDeleteFunc = osTaskDelete;
        os->osTaskBindInfo.osTaskGetSelfFunc = osTaskGetSelf;
        os->osTaskBindInfo.osTaskLockFunc = osTaskLock;
        os->osTaskBindInfo.osTaskUnLockFunc = osTaskUnLock;

        os->osStdLibBindInfo.osQsortFunc = osQsort;
        os->osStdLibBindInfo.osBsearchFunc = osBsearch;

        os->osMsgQBindInfo.osMsgQCreateFunc = osMsgQCreate;
        os->osMsgQBindInfo.osMsgQDeleteFunc = osMsgQDelete;
        os->osMsgQBindInfo.osMsgQSendFunc = osMsgQSend;
        os->osMsgQBindInfo.osMsgQRecvFunc = osMsgQRecv;
        os->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    }

    if (trace != NULL) {
        memset(trace, 0, sizeof(CPSS_TRACE_FUNC_BIND_STC));

        trace->traceHwBindInfo.traceHwAccessWriteFunc = traceHwAccessWrite;
        trace->traceHwBindInfo.traceHwAccessReadFunc = traceHwAccessRead;
        trace->traceHwBindInfo.traceHwAccessDelayFunc = traceHwAccessDelay;
    }
}

/*******************************************************************************
* extDrvDmaRead
*
* DESCRIPTION:
*       Read a memory block from a given address.
*
* INPUTS:
*       address     - The address to read from.
*       length      - Length of the memory block to read (in words).
*       burstLimit  - Number of words to be read on each burst.
*
* OUTPUTS:
*       buffer  - The read data.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       1.  The given buffer is always 4 bytes aligned, any further alignment
*           requirements should be handled internally by this function.
*       2.  The given buffer may be allocated from an uncached memory space, and
*           it's to the function to handle the cache flushing.
*       3.  The Prestera Driver assumes that the implementation of the DMA is
*           blocking, otherwise the Driver functionality might be damaged.
*
*******************************************************************************/
GT_STATUS extDrvDmaRead
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit,
    OUT GT_U32      *buffer
)
{
    /*
     * suppress compilation warning
     */
    (void)burstLimit;

    /*
     * fall back to memory copy
     * cpss lib handles the endian
     */
    memcpy(buffer, (void*)address, length * 4);
    return (GT_OK);
}

/*******************************************************************************
* extDrvDmaWrite
*
* DESCRIPTION:
*       Write a given buffer to the given address using the DMA.
*
* INPUTS:
*       address     - The destination address to write to.
*       buffer      - The buffer to be written.
*       length      - Length of buffer in words.
*       burstLimit  - Number of words to be written on each burst.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       1.  The given buffer is always 4 bytes aligned, any further alignment
*           requirements should be handled internally by this function.
*       2.  The given buffer may be allocated from an uncached memory space, and
*           it's to the function to handle the cache flushing.
*       3.  The Prestera Driver assumes that the implementation of the DMA is
*           blocking, otherwise the Driver functionality might be damaged.
*
*******************************************************************************/
GT_STATUS extDrvDmaWrite
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      *buffer,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit
)
{
    /*
     * suppress compilation warning
     */
    (void)burstLimit;
    /*
     * fall back to memory copy
     * cpss lib handles the endian
     */
    memcpy((void*)address, buffer, length*4);
    return (GT_OK);
}

/*******************************************************************************
* extDrvIntConnect
*
* DESCRIPTION:
*       Connect a specified C routine to a specified interrupt vector.
*
* INPUTS:
*       vector    - interrupt vector number to attach to
*       routine   - routine to be called
*       parameter - parameter to be passed to routine
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_BAD_PARAM             - on wrong device number or PFC enable option
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS extDrvIntConnect
(
    IN  GT_U32           intVec,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
    int ret = 0;
    char taskName[128];
    struct intTaskParams *param;

    if (routine == NULL) {
        ERR("Bad param (%p)", routine);
        return (GT_BAD_PARAM);
    }

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_INTCONNECT, intVec)) < 0) {
        ERR("Fail to do cmd. Error = %d", ret);
        return (GT_FAIL);
    }

    snprintf(taskName, sizeof(taskName), "intTask%u", (unsigned int)intVec);

    param = malloc(sizeof(struct intTaskParams));
    if (param == NULL) {
        ERR("Fail to malloc memory.");
        return (GT_FAIL);
    }
    param->routine = routine;
    param->parameter = parameter;

    int_exit = 0;
    osTaskCreate(taskName,
                 64,
                 0x2000,
                 intTask,
                 param,
                 &int_tid);

    if (0 == int_tid) {
        ERR("Fail to create a ISR thread.");

        free(param);
        return (GT_FAIL);
    }
#ifdef DEBUG
    INFO("Succeeds to create a ISR thread.");
#endif
    return (GT_OK);
}

/*******************************************************************************
* extDrvIntEnable
*
* DESCRIPTION:
*       Enable corresponding interrupt bits
*
* INPUTS:
*       intVecNum - new interrupt bits
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS extDrvIntEnable
(
    IN GT_U32   intVecNum
)
{
    int ret = 0;
    /*
     * Enable the IRQ
     */
    if ((ret = ioctl(sub_fd, DM_SUB_IOC_INTENABLE, intVecNum)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);

        return (GT_FAIL);
    }

    return (GT_OK);
}

/*******************************************************************************
* extDrvIntDisable
*
* DESCRIPTION:
*       Disable corresponding interrupt bits.
*
* INPUTS:
*       intVecNum - new interrupt bits
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS extDrvIntDisable
(
    IN GT_U32   intVecNum
)
{
    int ret = 0;
    /*
     * Disable the irq
     */
    if ((ret = ioctl(sub_fd, DM_SUB_IOC_INTDISABLE, intVecNum)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);

        return (GT_FAIL);
    }

    return (GT_OK);
}

/*******************************************************************************
* extDrvSetIntLockUnlock
*
* DESCRIPTION:
*       Lock/unlock interrupts
*
* INPUTS:
*       mode   - interrupt state lock/unlock
*       key    - if mode is INTR_MODE_UNLOCK, lock key returned by
*                preceding interrupt disable call
*
* OUTPUTS:
*       key    - if mode is INTR_MODE_LOCK lock key for the interrupt
*                level
*
* RETURNS:
*       Lock key for the interrupt level
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 extDrvSetIntLockUnlock
(
    IN      CPSS_OS_INTR_MODE_ENT mode,
    INOUT   GT_32          *key
)
{
    /*
     * suppress compilation warning
     */
    (void)mode;
    (void)key;
    /*
     * This api is used to protect int db from multiple ISR threads sync issue.
     * not used for dm case.
     */
    return (GT_OK);
}

/*******************************************************************************
* extDrvPciConfigWriteReg
*
* DESCRIPTION:
*       This routine write register to the PCI configuration space.
*
* INPUTS:
*       busNo    - PCI bus number.
*       devSel   - the device devSel.
*       funcNo   - function number.
*       regAddr  - Register offset in the configuration space.
*       data     - data to write.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciConfigWriteReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    int ret = 0;
    pcie_config_reg_t reg;

    reg.bus = busNo;
    reg.dev = devSel;
    reg.func = funcNo;
    reg.reg = regAddr;
    reg.data = data;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_PCIECONFIGWRITEREG, &reg)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);
        return (GT_FAIL);
    }

    return (GT_OK);
}

/*******************************************************************************
* extDrvPciConfigReadReg
*
* DESCRIPTION:
*       This routine read register from the PCI configuration space.
*
* INPUTS:
*       busNo    - PCI bus number.
*       devSel   - the device devSel.
*       funcNo   - function number.
*       regAddr  - Register offset in the configuration space.
*
* OUTPUTS:
*       data     - the read data.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciConfigReadReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    int ret = 0;
    pcie_config_reg_t reg;

    if (data == NULL) {
        ERR("Bad param %p", data);

        return (GT_BAD_PARAM);
    }

    reg.bus = busNo;
    reg.dev = devSel;
    reg.func = funcNo;
    reg.reg = regAddr;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_PCIECONFIGREADREG, &reg)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);

        return (GT_FAIL);
    }

    *data = reg.data;

    return (GT_OK);
}

/*******************************************************************************
* extDrvPciFindDev
*
* DESCRIPTION:
*       This routine returns the next instance of the given device (defined by
*       vendorId & devId).
*
* INPUTS:
*       vendorId - The device vendor Id.
*       devId    - The device Id.
*       instance - The requested device instance.
*
* OUTPUTS:
*       busNo    - PCI bus number.
*       devSel   - the device devSel.
*       funcNo   - function number.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
)
{
    int ret = 0;
    pcie_find_dev_t dev;

    if (busNo == NULL || devSel == NULL || funcNo == NULL) {
        ERR("Bad param (%p, %p, %p)", busNo, devSel, funcNo);
        return (GT_BAD_PARAM);
    }

    dev.vendorId = vendorId;
    dev.devId    = devId;
    dev.instance = instance;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_FIND_DEV, &dev)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);
        return (GT_FAIL);
    }

    *busNo  = dev.bus;
    *devSel = dev.dev;
    *funcNo = dev.func;

    return (GT_OK);
}

/*******************************************************************************
* extDrvEnableCombinedWrites
*
* DESCRIPTION:
*       This function enables / disables the Pci writes / reads combining
*       feature.
*       Some system controllers support combining memory writes / reads. When a
*       long burst write / read is required and combining is enabled, the master
*       combines consecutive write / read transactions, if possible, and
*       performs one burst on the Pci instead of two. (see comments)
*
* INPUTS:
*       enWrCombine - GT_TRUE enables write requests combining.
*       enRdCombine - GT_TRUE enables read requests combining.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on sucess,
*       GT_NOT_SUPPORTED    - if the controller does not support this feature,
*       GT_FAIL             - otherwise.
*
* COMMENTS:
*       1.  Example for combined write scenario:
*           The controller is required to write a 32-bit data to address 0x8000,
*           while this transaction is still in progress, a request for a write
*           operation to address 0x8004 arrives, in this case the two writes are
*           combined into a single burst of 8-bytes.
*
*******************************************************************************/
GT_STATUS extDrvEnableCombinedPciAccess
(
    IN  GT_BOOL     enWrCombine,
    IN  GT_BOOL     enRdCombine
)
{
    if(enWrCombine || enRdCombine) {
        ERR("Unsupported param (%u, %u)", enWrCombine, enRdCombine);
        return (GT_NOT_SUPPORTED);
    }

    return (GT_OK);
}

/*******************************************************************************
* extDrvPciDoubleWrite
*
* DESCRIPTION:
*        This routine will write a 64-bit data  to given address
*
* INPUTS:
*        address - address to write to
*       word1 - the first half of double word to write (MSW)
*       word2 - the second half of double word to write (LSW)
*
* OUTPUTS:
*      none
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciDoubleWrite
(
    IN  GT_U32 address,
    IN  GT_U32 word1,
    IN  GT_U32 word2
)
{
    int ret = 0;
    read_write_data_t rw_data;

    if ((address >= (GT_U32)config_addr) &&
       (address < (GT_U32)(config_addr + config_size))) {
           rw_data.type = READ_WRITE_TYPE_CONFIG;
           rw_data.offset = address - config_addr;
    } else if ((address >= (GT_U32)ppregs_addr) &&
              (address < (GT_U32)(ppregs_addr + ppregs_size))) {
                  rw_data.type = READ_WRITE_TYPE_PPREGS;
                  rw_data.offset = address - ppregs_addr;
    } else {
        ERR("Unsupported address (%u)", (unsigned int)address);
        return (GT_FAIL);
    }

    rw_data.data = word1;
    rw_data.data = ((rw_data.data << 32) | word2);

    rw_data.length = 8;
    rw_data.burst = 8;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_U64_WRITE,
                     (unsigned long)&rw_data)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);

        return (GT_FAIL);
    }

    return (GT_OK);
}

/*******************************************************************************
* extDrvPciDoubleRead
*
* DESCRIPTION:
*        This routine will read a 64-bit data  from given address
*
* INPUTS:
*        address - address to read from
*
* OUTPUTS:
*       data     -  pointer for the received data.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS extDrvPciDoubleRead
(
    IN  GT_U32  address,
    OUT GT_U64  *dataPtr
)
{
    int ret = 0;
    read_write_data_t rw_data;

    if (dataPtr == NULL) {
        ERR("Bad param (%p)", dataPtr);

        return (GT_BAD_PARAM);
    }

    if ((address >= (GT_U32)config_addr) &&
       (address < (GT_U32)(config_addr + config_size))) {
           rw_data.type = READ_WRITE_TYPE_CONFIG;
           rw_data.offset = address - config_addr;
    } else if ((address >= (GT_U32)ppregs_addr) &&
              (address < (GT_U32)(ppregs_addr + ppregs_size))) {
                  rw_data.type = READ_WRITE_TYPE_PPREGS;
                  rw_data.offset = address - ppregs_addr;
    } else {
        ERR("Unsupported address (%u)", (unsigned int)address);
        return (GT_FAIL);
    }

    rw_data.length = 8;
    rw_data.burst = 8;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_U64_READ,
                     (unsigned long)&rw_data)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);
        return (GT_FAIL);
    }

    (*dataPtr).l[0] = (rw_data.data & 0xFFFFFFFF);
    (*dataPtr).l[1] = (rw_data.data >> 32);

    return (GT_OK);
}

/*******************************************************************************
* extDrvGetPciIntVec
*
* DESCRIPTION:
*       This routine return the PCI interrupt vector.
*
* INPUTS:
*       pciInt - PCI interrupt number.
*
* OUTPUTS:
*       intVec - PCI interrupt vector.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS extDrvGetPciIntVec
(
    IN  CPSS_EXTDRV_PCI_INT_ENT  pciInt,
    OUT void        **intVec
)
{
    int ret = 0;
    intrline_to_vector_t int2vec;

    if (intVec == NULL) {
        ERR("Bad param(%p)", intVec);
        return (GT_BAD_PARAM);
    }
    /*
     * get the PCI interrupt vector
     */
    int2vec.intrline = (unsigned long)pciInt;
    int2vec.vector = 0;
    if ((ret = ioctl(sub_fd, DM_SUB_IOC_GETINTVEC, &int2vec)) < 0) {
        ERR("Fail to do cmd, Error = %d", ret);
        return (GT_FAIL);
    }
    *intVec = (void *)(unsigned long)int2vec.vector;

    /*
     * check whether a valid value
     */
    if ((*intVec) == NULL) {
        ERR("Fail to get a valid int vector.");
        return (GT_FAIL);
    }

    return (GT_OK);
}

/*******************************************************************************
* extDrvGetIntMask
*
* DESCRIPTION:
*       This routine return the PCI interrupt vector.
*
* INPUTS:
*       pciInt - PCI interrupt number.
*
* OUTPUTS:
*       intMask - PCI interrupt mask.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*
* COMMENTS:
*       PCI interrupt mask should be used for interrupt disable/enable.
*
*******************************************************************************/
GT_STATUS extDrvGetIntMask
(
    IN  CPSS_EXTDRV_PCI_INT_ENT  pciInt,
    OUT GT_UINTPTR      *intMask
)
{
    void *vector;
    if(intMask == NULL) {
        ERR("Bad param (%p)", intMask);

        return (GT_BAD_PARAM);
    }

    /*
     * get the PCI interrupt vector
     */
    extDrvGetPciIntVec(pciInt, (void**)&vector);

    *intMask = (GT_U32)(unsigned long)vector;

    return (GT_OK);
}

/******************************************************************************
* appDemoTraceHwAccessAction
*
* DESCRIPTION:
*       Trace HW Access action: print or store.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       isrContext  - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                                       cpssDrvPpHwIsrRead)
*                    GT_FALSE: called from not ISR context.
*       pciPexSpace - GT_TRUE: called for PCI/PEX registers address
*                     space (cpssDrvPpHwInternalPciRegWrite,
*                            cpssDrvPpHwInternalPciRegRead)
*                     GT_FALSE - called for usual  (not PCI/PEX) address space
*       addr        - start address that the access was made to
*       length      - length of the data that was written in words
*       dataPtr     - (pointer to) data that was written
*       outputMode  - output mode: print, synchronious print and store
*       accessType  - access type: read or write
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
******************************************************************************/
static GT_STATUS traceHwAccessAction
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroupId,
    IN GT_BOOL                     isrContext,
    IN GT_BOOL                     pciPexSpace,
    IN GT_U32                      addr,
    IN GT_U32                      length,
    IN GT_U32                      *dataPtr,
    IN TRACE_OUTPUT_MODE_ENT       outputMode,
    IN TRACE_HW_ACCESS_TYPE_ENT    accessType
)
{
    GT_STATUS ret = GT_OK;
    GT_U32 i;
    char *buf = NULL;
    int len = 0;

    buf = malloc(256 + length * 8 + length / 4);
    if(buf == NULL) {
        ERR("Fail to alloc memory for hardware access trace.");
        return (GT_FAIL);
    }

    INFO("Hardware Access Trace:");

    switch (outputMode) {
        case TRACE_OUTPUT_MODE_DIRECT_E:
        case TRACE_OUTPUT_MODE_DIRECT_SYNC_E:
            if (accessType == TRACE_HW_ACCESS_TYPE_DELAY_E) {
                INFO("Delay \t%u \t%u \t%8u",(unsigned int)devNum,
                         (unsigned int)portGroupId, (unsigned int)*dataPtr);
            } else {
                if (accessType == TRACE_HW_ACCESS_TYPE_READ_E) {
                    len += sprintf(buf+len, "Read ");
                } else {
                    len += sprintf(buf+len, "Write ");
                }

                len += sprintf(buf+len, "\t%u \t%u ", (unsigned int)devNum,
                               (unsigned int)portGroupId);

                if (isrContext) {
                    len += sprintf(buf+len, "\tISR ");
                } else {
                    len += sprintf(buf+len, "\tTSK ");
                }

                if (pciPexSpace) {
                    len += sprintf(buf+len, "\tPEX ");
                } else {
                    len += sprintf(buf+len, "\tREG ");
                }

                INFO("%s", buf);
                INFO("ADDR:  %-#8x", (unsigned int)addr);

                len = 0;
                for (i = 0; i < length; i++) {
                    len += sprintf(buf+len, "%-#8x  ", (unsigned int)dataPtr[i]);

                    if (((i+1) % 4 == 0) && (i != (length -1)))
                        len += sprintf(buf+len, "\n");
                }

                INFO("%s", buf);
            }

            break;

        case TRACE_OUTPUT_MODE_DB_E:
        case TRACE_OUTPUT_MODE_FILE_E:
            ret = GT_NOT_SUPPORTED;
            break;

        default:
            ret = GT_BAD_STATE;
            break;
    }

    free(buf);

    return ret;
}

/******************************************************************************
* traceHwAccessWrite
*
* DESCRIPTION:
*       Trace HW write access information.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       isrContext  - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                                       cpssDrvPpHwIsrRead)
*                    GT_FALSE: called from not ISR context.
*       pciPexSpace - GT_TRUE: called for PCI/PEX registers address
*                     space (cpssDrvPpHwInternalPciRegWrite,
*                            cpssDrvPpHwInternalPciRegRead)
*                     GT_FALSE - called for usual  (not PCI/PEX) address space
*       addr        - start address that the access was made to
*       length      - length of the data that was written in words
*       dataPtr     - (pointer to) data that was written
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on fail
*
* COMMENTS:
*       None.
*
******************************************************************************/
GT_STATUS traceHwAccessWrite
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT      pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr,
    /* Old version CPSS without the parameter, but new CPSS with the parameter */
    IN GT_U32      mask 


)
{
    return (traceHwAccessAction(devNum,
                               portGroupId,
                               isrContext,
                               pciPexSpace,
                               addr,
                               length,
                               dataPtr,
                               TRACE_OUTPUT_MODE_DIRECT_E,
                               TRACE_HW_ACCESS_TYPE_WRITE_E));
}

/******************************************************************************
* traceHwAccessRead
*
* DESCRIPTION:
*       Trace HW read access information.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       isrContext  - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                                       cpssDrvPpHwIsrRead)
*                     GT_FALSE: called from not ISR context.
*       pciPexSpace - GT_TRUE: called for PCI/PEX registers address
*                     space (cpssDrvPpHwInternalPciRegWrite,
*                            cpssDrvPpHwInternalPciRegRead)
*                     GT_FALSE - called for usual  (not PCI/PEX) address space
*       addr        - start address that the access was made to
*       length      - length of the data that was written in words
*       dataPtr     - (pointer to) data that was read
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on fail
*
* COMMENTS:
*       None.
*
******************************************************************************/
GT_STATUS traceHwAccessRead
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT      pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
)
{
    return (traceHwAccessAction(devNum,
                               portGroupId,
                               isrContext,
                               pciPexSpace,
                               addr,
                               length,
                               dataPtr,
                               TRACE_OUTPUT_MODE_DIRECT_E,
                               TRACE_HW_ACCESS_TYPE_READ_E));
}


/******************************************************************************
* traceHwAccessDelay
*
* DESCRIPTION:
*       Trace HW write access information.
*
* INPUTS:
*       devNum      - PP device number
*       portGroupId - ports group number
*       millisec   -  the delay in millisec
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on fail
*
* COMMENTS:
*       None.
*
******************************************************************************/
GT_STATUS traceHwAccessDelay
(
       IN GT_U8       devNum,
       IN GT_U32      portGroupId,
       IN GT_U32      millisec
)
{
    return (traceHwAccessAction(devNum,
                               ((portGroupId==0xFFFFFFFF)?0:portGroupId),
                               GT_FALSE,
                               GT_FALSE,
                               0,
                               1,
                               &millisec,
                               TRACE_OUTPUT_MODE_DIRECT_E,
                               TRACE_HW_ACCESS_TYPE_DELAY_E));
}


/*
 *------------------------------------------------------------------
 * $Log: cpss_common.c,v $
 * Revision 1.2  2021/09/24 01:21:39  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/04/23 02:46:54  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2021/04/12 08:38:21  illiu
 * Add file: cpss common code
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */







