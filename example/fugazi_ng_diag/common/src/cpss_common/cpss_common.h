/* $Id: cpss_common.h,v 1.2 2021/09/24 01:21:39 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/cpss_common/cpss_common.h,v $
 *------------------------------------------------------------------
 *
 * Filename:	cpss_common.h
 *
 *------------------------------------------------------------------
 */

#ifndef __CPSS_COMMON_H__
#define __CPSS_COMMON_H__

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/cpssExtServices.h>

#define DM_MAIN_INT_CAUSE_HI_REG        0x20210
#define DM_PCIE_SWITCH_MG_INT_SHIFT     23
#define DM_PCIE_GPIOLO24_31_INT_SHIFT   6

#define DM_GPIOLO_DATA_IN_REG           0x10110
#define DM_GPIOLO_INT_CAUSE_REG         0x10114
#define DM_GPIOLO_POE_INT_SHIFT         25
#define DM_GPIOLO_PHY_INT_SHIFT         26


extern int int_exit;
extern CPSS_TASK int_tid;

struct intTaskParams {
    GT_VOIDFUNCPTR   routine;
    GT_U32           parameter;
};

pcie_mapping_array_t * cpss_pcie_get_pcie_mapping (void);
void cpss_pcie_put_pcie_mapping (pcie_mapping_array_t *map_array);
void cpss_pcie_get_extserv (CPSS_EXT_DRV_FUNC_BIND_STC *extDrv,
                            CPSS_OS_FUNC_BIND_STC      *os,
                            CPSS_TRACE_FUNC_BIND_STC   *trace);

GT_STATUS extDrvPciMapEx
(
    OUT GT_UINTPTR  *pciBaseAddr,
    OUT GT_SIZE_T   *pciSize,
    OUT GT_UINTPTR  *internalPciBase,
    OUT GT_SIZE_T   *internalSize
);

GT_STATUS extDrvPciUnMapEx
(
    IN GT_UINTPTR  pciBaseAddr,
    IN GT_SIZE_T   pciSize,
    IN GT_UINTPTR  internalPciBase,
    IN GT_SIZE_T   internalSize
);

GT_STATUS extDrvGetPciIntVec
(
    IN  CPSS_EXTDRV_PCI_INT_ENT  pciInt,
    OUT void **intVec
);

GT_STATUS extDrvGetIntMask
(
    IN  CPSS_EXTDRV_PCI_INT_ENT  pciInt,
    OUT GT_UINTPTR *intMask
);

GT_STATUS extDrvDmaRead
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit,
    OUT GT_U32      *buffer
);

GT_STATUS extDrvDmaWrite
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      *buffer,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit
);

GT_STATUS extDrvIntConnect
(
    IN  GT_U32           intVec,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
);

/*
 * CPSS_EXT_DRV_INT_STC extDrvIntBindInfo;
 */
GT_STATUS extDrvIntEnable
(
    IN GT_U32  intVecNum
);

GT_STATUS extDrvIntDisable
(
    IN GT_U32  intVecNum
);

GT_32 extDrvSetIntLockUnlock
(
    IN      CPSS_OS_INTR_MODE_ENT mode,
    INOUT   GT_32 *key
);

/*
 * CPSS_EXT_DRV_PCI_STC  extDrvPciInfo;
 */
GT_STATUS extDrvPciConfigWriteReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

GT_STATUS extDrvPciConfigReadReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

GT_STATUS extDrvPciFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
);

GT_STATUS extDrvEnableCombinedPciAccess
(
    IN  GT_BOOL enWrCombine,
    IN  GT_BOOL enRdCombine
);

GT_STATUS extDrvPciDoubleWrite
(
    IN  GT_U32 address,
    IN  GT_U32 word1,
    IN  GT_U32 word2
);

GT_STATUS extDrvPciDoubleRead
(
    IN  GT_U32  address,
    OUT GT_U64  *dataPtr
);


/*
 * typedef: enum TRACE_HW_ACCESS_TYPE_ENT
 *
 * Description: PP access type enumeration
 *
 * Fields:
 *      TRACE_HW_ACCESS_TYPE_READ_E  - PP access type is read.
 *      TRACE_HW_ACCESS_TYPE_WRITE_E - PP access type is write
 *      TRACE_HW_ACCESS_TYPE_DELAY_E - PP access type is (write/)delay
 */
typedef enum
{
    TRACE_HW_ACCESS_TYPE_READ_E,
    TRACE_HW_ACCESS_TYPE_WRITE_E,
    TRACE_HW_ACCESS_TYPE_DELAY_E
} TRACE_HW_ACCESS_TYPE_ENT;

/*
 * typedef: enum  APP_DEMO_TRACE_OUTPUT_MODE_ENT
 *
 * Description: PP access type enumeration
 *
 * Fields:
 *      TRACE_OUTPUT_MODE_DIRECT_E         - use osPrintf.
 *      TRACE_OUTPUT_MODE_DIRECT_SYNC_E    - use osPrintSync need for ISR debug
 *      TRACE_OUTPUT_MODE_DB_E             - store the data in db
 *      TRACE_OUTPUT_MODE_FILE_E           - store the data in file
 */

typedef enum
{
    TRACE_OUTPUT_MODE_DIRECT_E,
    TRACE_OUTPUT_MODE_DIRECT_SYNC_E,
    TRACE_OUTPUT_MODE_DB_E,
    TRACE_OUTPUT_MODE_FILE_E
} TRACE_OUTPUT_MODE_ENT;

GT_STATUS traceHwAccessWrite
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr,
    /* Old version CPSS without the parameter, but new CPSS with the parameter */
    IN GT_U32      mask  
);

GT_STATUS traceHwAccessRead
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  pciPexSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
);

GT_STATUS traceHwAccessDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);





#endif
/*
 *------------------------------------------------------------------
 * $Log: cpss_common.h,v $
 * Revision 1.2  2021/09/24 01:21:39  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/04/23 02:46:57  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2021/04/12 08:38:26  illiu
 * Add file: cpss common code
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
