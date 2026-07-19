/* $Id: nim_dm_cpss_extdrv.h,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_cpss_extdrv.h,v $
 *------------------------------------------------------------------
 * DM CPSS lib external driver service
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *---------------------------------------------------------------------------
 */


#ifndef __NIM_DM_CPSS_EXTDRV_H__
#define __NIM_DM_CPSS_EXTDRV_H__

#define DM_MAIN_INT_CAUSE_HI_REG        0x20210
#define DM_PCIE_SWITCH_MG_INT_SHIFT     23
#define DM_PCIE_GPIOLO24_31_INT_SHIFT   6

#define DM_GPIOLO_DATA_IN_REG           0x10110
#define DM_GPIOLO_INT_CAUSE_REG         0x10114
#define DM_GPIOLO_POE_INT_SHIFT         25
#define DM_GPIOLO_PHY_INT_SHIFT         26

struct intTaskParams {
    GT_VOIDFUNCPTR   routine;
    GT_U32           parameter;
};

/*
 * CPSS_EXT_DRV_DMA_STC extDrvDmaBindInfo;
 */
GT_STATUS extDrvDmaWrite
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      *buffer,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit
);

GT_STATUS extDrvDmaRead
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit,
    OUT GT_U32      *buffer
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

GT_STATUS extDrvPciMapEx
(
    OUT GT_UINTPTR  *pciBaseAddr,
    OUT GT_SIZE_T   *pciSize,
    OUT GT_UINTPTR  *internalPciBase,
    OUT GT_SIZE_T   *internalSize,
    OUT GT_UINTPTR  *exPciBaseAddr,
    OUT GT_SIZE_T   *exPciSize
);

GT_STATUS extDrvPciMap
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
    IN GT_SIZE_T   internalSize,
    IN GT_UINTPTR  exPciBaseAddr,
    IN GT_SIZE_T   exPciSize
);

GT_STATUS extDrvPciUnMap
(
    IN GT_UINTPTR  pciBaseAddr,
    IN GT_SIZE_T   pciSize,
    IN GT_UINTPTR  internalPciBase,
    IN GT_SIZE_T   internalSize
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

GT_STATUS extDrvGetPciIntVec
(
    IN  CPSS_EXTDRV_PCI_INT_ENT  pciInt,
    OUT void **intVec
);

GT_STATUS extDrvGetIntMask
(
    IN  CPSS_EXTDRV_PCI_INT_ENT  pciInt,
    //OUT GT_U32 *intMask
    OUT GT_UINTPTR *intMask
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

extern int int_exit;
extern CPSS_TASK int_tid;
GT_STATUS extDrvIntConnect
(
    IN  GT_U32           intVec,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
);



#endif /*__NIM_DM_CPSS_EXTDRV_H__*/

/*------------------------------------------------------------------
 * $Log: nim_dm_cpss_extdrv.h,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
