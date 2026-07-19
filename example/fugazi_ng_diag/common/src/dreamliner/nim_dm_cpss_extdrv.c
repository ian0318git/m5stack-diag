/* $Id: nim_dm_cpss_extdrv.c,v 1.2 2015/02/27 10:02:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/nim_dm_cpss_extdrv.c,v $
 *------------------------------------------------------------------
 * DM CPSS lib external driver service
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>

#include "nim_dm_cpss_extserv.h"
extern GT_BOOL phy_intr_happened;
extern GT_BOOL poe_intr_happened;

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
    return GT_OK;
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
    return GT_OK;
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
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_PCIECONFIGWRITEREG(%u, %u, %u, %u, %u). Error = %d",
            dm_cpss_slot, dm_cpss_bay,
            reg.bus, reg.dev, reg.func, reg.reg, reg.data, ret);
        return GT_FAIL;
    }

    return GT_OK;
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

    if(data == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p)",
            dm_cpss_slot, dm_cpss_bay, data);

        return GT_BAD_PARAM;
    }

    reg.bus = busNo;
    reg.dev = devSel;
    reg.func = funcNo;
    reg.reg = regAddr;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_PCIECONFIGREADREG, &reg)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_PCIECONFIGREADREG(%u, %u, %u, %u, %p). Error = %d",
            dm_cpss_slot, dm_cpss_bay,
            reg.bus, reg.dev, reg.func, reg.reg, data, ret);

        return GT_FAIL;
    }

    *data = reg.data;

    return GT_OK;
}

/*******************************************************************************
* extDrvPciMap
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
GT_STATUS extDrvPciMap
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

    if(pciBaseAddr == NULL || pciSize == NULL ||
       internalPciBase == NULL || internalSize == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param"
            "(%p, %p, %p, %p)", dm_cpss_slot, dm_cpss_bay,
            pciBaseAddr, pciSize, internalPciBase, internalSize);
        return GT_BAD_PARAM;
    }

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_CONFIG_PPREGS_SIZE, &size)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_CONFIG_PPREGS_SIZE. Error = %d",
            dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
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
        ERR("The dm instance of slot/bay(%u/%u) fail to map the config "
            "memory space.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
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

    if (MAP_FAILED == baseAddr) {
        ERR("The dm instance of slot/bay(%u/%u) fail to map the pp register "
            "memory space.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    /*
     * config space base address and size
     * Note, the offset of config register is taken care by app
     */
    *internalPciBase = (GT_UINTPTR)internalBase;
    *internalSize = (GT_SIZE_T)size.config_size;

    /*
     * pp register space base address and size
     */
    *pciBaseAddr = (GT_UINTPTR)baseAddr;
    *pciSize = size.ppregs_size;

    return GT_OK;
}

/*******************************************************************************
* extDrvPciUnMap
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
GT_STATUS extDrvPciUnMap
(
    IN GT_UINTPTR  pciBaseAddr,
    IN GT_SIZE_T   pciSize,
    IN GT_UINTPTR  internalPciBase,
    IN GT_SIZE_T   internalSize
)
{
    int ret = 0;

    if(pciBaseAddr == 0 || pciSize == 0 ||
       internalPciBase == 0 || internalSize == 0) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param"
            "(%lu, %lu, %lu, %lu)", dm_cpss_slot, dm_cpss_bay,
            (unsigned long)pciBaseAddr, (unsigned long)pciSize,
            (unsigned long)internalPciBase, (unsigned long)internalSize);
        return GT_BAD_PARAM;
    }

    /*
     * unmap config space
     */
    ret = munmap((void*)internalPciBase, (size_t)internalSize);
    if (ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to unmap the config "
            "memory space.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    /*
     * unmap pp register space
     */
    ret = munmap((void*)pciBaseAddr, (size_t)pciSize);
    if (ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to unmap the pp register "
            "memory space.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    return GT_OK;
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
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param"
            "(%p, %p, %p)", dm_cpss_slot, dm_cpss_bay, busNo, devSel, funcNo);
        return GT_BAD_PARAM;
    }

    dev.vendorId = vendorId;
    dev.devId    = devId;
    dev.instance = instance;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_FIND_DEV, &dev)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_FIND_DEV. Error = %d",
            dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    }

    *busNo  = dev.bus;
    *devSel = dev.dev;
    *funcNo = dev.func;

    return GT_OK;
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

    if(intVec == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p)",
            dm_cpss_slot, dm_cpss_bay, intVec);
        return GT_BAD_PARAM;
    }
    /*
     * get the PCI interrupt vector
     */
    int2vec.intrline = (unsigned long)pciInt;
    int2vec.vector = 0;
    if ((ret = ioctl(sub_fd, DM_SUB_IOC_GETINTVEC, &int2vec)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_GETINTVEC. Error = %d",
             dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    }
    *intVec = (void *)(unsigned long)int2vec.vector;

    /*
     * check whether a valid value
     */
    if((*intVec) == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to get a valid "
            "int vector.", dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
    }

    return GT_OK;
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
    OUT GT_U32      *intMask
)
{
    void *vector;
    if(intMask == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p)",
            dm_cpss_slot, dm_cpss_bay, intMask);

        return GT_BAD_PARAM;
    }

    /*
     * get the PCI interrupt vector
     */
    extDrvGetPciIntVec(pciInt, (void**)&vector);

    *intMask = (GT_U32)(unsigned long)vector;

    return GT_OK;
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
        ERR("The dm instance of slot/bay(%u/%u) get called with unsupported "
            "param(%u, %u)",
            dm_cpss_slot, dm_cpss_bay, enWrCombine, enRdCombine);
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
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

    if((address >= (GT_U32)config_addr) &&
       (address < (GT_U32)(config_addr + config_size))) {
        rw_data.type = READ_WRITE_TYPE_CONFIG;
        rw_data.offset = address - config_addr;
    } else if((address >= (GT_U32)ppregs_addr) &&
            (address < (GT_U32)(ppregs_addr + ppregs_size))) {
        rw_data.type = READ_WRITE_TYPE_PPREGS;
        rw_data.offset = address - ppregs_addr;
    } else {
        ERR("The dm instance of slot/bay(%u/%u) get called with unsupported "
            "address(%u)", dm_cpss_slot, dm_cpss_bay, (unsigned int)address);
        return GT_FAIL;
    }

    rw_data.data = word1;
    rw_data.data = ((rw_data.data << 32) | word2);

    rw_data.length = 8;
    rw_data.burst = 8;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_U64_WRITE,
                     (unsigned long)&rw_data)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) failto do cmd "
            "DM_SUB_IOC_U64_WRITE. Error = %d",
            dm_cpss_slot, dm_cpss_bay, ret);

        return GT_FAIL;
    }

    return GT_OK;
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

    if(dataPtr == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p)",
            dm_cpss_slot, dm_cpss_bay, dataPtr);

        return GT_BAD_PARAM;
    }

    if((address >= (GT_U32)config_addr) &&
       (address < (GT_U32)(config_addr + config_size))) {
        rw_data.type = READ_WRITE_TYPE_CONFIG;
        rw_data.offset = address - config_addr;
    } else if((address >= (GT_U32)ppregs_addr) &&
            (address < (GT_U32)(ppregs_addr + ppregs_size))) {
        rw_data.type = READ_WRITE_TYPE_PPREGS;
        rw_data.offset = address - ppregs_addr;
    } else {
        ERR("The dm instance of slot/bay(%u/%u) get called with unsupported "
            "address(%u)", dm_cpss_slot, dm_cpss_bay, (unsigned int)address);
        return GT_FAIL;
    }

    rw_data.length = 8;
    rw_data.burst = 8;

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_U64_READ,
                     (unsigned long)&rw_data)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_U64_READ. Error = %d",
            dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    }

    (*dataPtr).l[0] = (rw_data.data & 0xFFFFFFFF);
    (*dataPtr).l[1] = (rw_data.data >> 32);

    return GT_OK;
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
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_INTENABLE(%u). Error = %d",
            dm_cpss_slot, dm_cpss_bay, (unsigned int)intVecNum, ret);

        return GT_FAIL;
    }

    return GT_OK ;
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
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_INTDISABLE(%u). Error = %d",
            dm_cpss_slot, dm_cpss_bay, (unsigned int)intVecNum, ret);

        return GT_FAIL;
    }

    return GT_OK ;
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
    return GT_OK;
}


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
#ifdef DEBUG
    INFO("The dm instance of slot/bay(%u/%u) ISR thread starts to run.",
         dm_cpss_slot, dm_cpss_bay);
#endif
    while (1) {
        /*
         * Wait for interrupt
         */
        if ((ret = ioctl(sub_fd, DM_SUB_IOC_WAIT)) < 0) {
            ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
                "DM_SUB_IOC_WAIT. Error = %d",
                dm_cpss_slot, dm_cpss_bay, ret);

            return GT_FAIL;
        }

        if(int_exit) {
#ifdef DEBUG
            INFO("The dm instance of slot/bay(%u/%u) cancels the int thread.",
                 dm_cpss_slot, dm_cpss_bay);
#endif
            return GT_OK;
        }
	
	INFO("The dm instance of slot/bay(%u/%u) ISR thread receives hardware "
	     "interrupt\n", dm_cpss_slot, dm_cpss_bay);

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
		poe_intr_happened = GT_TRUE;
	    }

	    /* PHY interrupt */
	    if (*gpio_cause & (0x1 << DM_GPIOLO_PHY_INT_SHIFT)) {
		/* need to deactivate PHY interrupt line */
		*gpio_cause &= ~(0x1 << DM_GPIOLO_PHY_INT_SHIFT);
		phy_intr_happened = GT_TRUE;
	    }
	}
    }

    return GT_OK;
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
CPSS_TASK int_tid = 0;
int int_exit = 0;
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

    if(routine == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) get called with bad param(%p)",
            dm_cpss_slot, dm_cpss_bay, routine);
        return GT_BAD_PARAM;
    }

    if ((ret = ioctl(sub_fd, DM_SUB_IOC_INTCONNECT, intVec)) < 0) {
        ERR("The dm instance of slot/bay(%u/%u) fail to do cmd "
            "DM_SUB_IOC_INTCONNECT. Error = %d",
            dm_cpss_slot, dm_cpss_bay, ret);
        return GT_FAIL;
    }

    snprintf(taskName, sizeof(taskName), "intTask%u", (unsigned int)intVec);

    param = malloc(sizeof(struct intTaskParams));
    if(param == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to malloc memory.",
            dm_cpss_slot, dm_cpss_bay);
        return GT_FAIL;
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
        ERR("The dm instance of slot/bay(%u/%u) fail to create a ISR thread.",
            dm_cpss_slot, dm_cpss_bay);

        free(param);
        return GT_FAIL;
    }
#ifdef DEBUG
    INFO("The dm instance of slot/bay(%u/%u) succeeds to create a ISR thread.",
	 dm_cpss_slot, dm_cpss_bay);
#endif
    return  GT_OK;
}


/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_extdrv.c,v $
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
