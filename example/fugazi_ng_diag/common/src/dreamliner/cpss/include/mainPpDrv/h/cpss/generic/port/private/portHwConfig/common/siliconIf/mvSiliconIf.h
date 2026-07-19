/* $Id: mvSiliconIf.h,v 1.1 2015/02/13 11:34:19 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/siliconIf/mvSiliconIf.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* siliconIf.h
*
* DESCRIPTION:
*       Application IF defintion.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __siliconIf_H
#define __siliconIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

/* Returns the info located at the specified offset & length in data.   */
#define U32_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & ((1 << (length)) - 1))

#undef HWS_DEBUG
#ifdef HWS_DEBUG
#include <gtOs/gtOsIo.h>
extern GT_FILEP fdHws;
#endif

extern GT_VOID gtBreakOnFail
(
    GT_VOID
);

extern GT_STATUS gtStatus;

/* Global SysConf control library code for sysLog registration */
#ifdef CHECK_STATUS
#undef CHECK_STATUS
#endif
#define CHECK_STATUS(origFunc) \
{ \
    gtStatus = origFunc; \
    if (GT_OK != gtStatus) \
    { \
        gtBreakOnFail(); \
        return gtStatus; \
    } \
}

#define ABS(val) ((val) < 0) ? -(val) : (val)

/* max number of devices supported by driver */
#define HWS_MAX_DEVICE_NUM (128)
/* max number of SERDESes in one device */
#define HWS_MAX_DEV_SERDES_NUM (96)

typedef enum
{
    Lion2A0,
    Lion2B0,
    Puma3A0,
    HooperA0,
    Puma3B0,

    LAST_SIL_TYPE

}MV_HWS_DEV_TYPE;

typedef enum
{
    GEMAC_UNIT,
    XLGMAC_UNIT,
    HGLMAC_UNIT,
    XPCS_UNIT,
    MMPCS_UNIT,

    CG_UNIT,
    INTLKN_UNIT,
    INTLKN_RF_UNIT,
    SERDES_UNIT,
    ETI_UNIT,
    ETI_ILKN_RF_UNIT,

    LAST_UNIT

}MV_HWS_UNITS_ID;

typedef struct
{
    GT_U32 baseAddr;
    GT_U32 regOffset;
}HWS_UNIT_INFO;

/* max port number in core of specified device */

typedef struct
{
    GT_U8               devNum;
    GT_U32              portsNum;
    MV_HWS_DEV_TYPE     devType;
    GT_U32 serdesType;
    GT_U32 lastSupPortMode;

}HWS_DEVICE_INFO;

typedef struct intlknIpRegsDef
{
    GT_U32 regOffset;
    GT_U32 defValue;
}MV_INTLK_REG_DEF;


extern HWS_DEVICE_INFO hwsDeviceSpecInfo[HWS_MAX_DEVICE_NUM];

#define HWS_CORE_PORTS_NUM(devNum) hwsDeviceSpecInfo[devNum].portsNum
#define HWS_DEV_SERDES_TYPE(devNum) hwsDeviceSpecInfo[devNum].serdesType
#define HWS_DEV_SILICON_TYPE(devNum) hwsDeviceSpecInfo[devNum].devType
#define HWS_DEV_PORT_MODES(devNum) hwsDeviceSpecInfo[devNum].lastSupPortMode

typedef GT_STATUS (*MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  *sdVector
);


typedef struct
{
  MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR     redundVectorGetFunc;

}MV_HWS_DEV_FUNC_PTRS;


/* os wrapper function prototypes */
/*******************************************************************************
* MV_OS_MALLOC_FUNC
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
*******************************************************************************/
typedef void * (*MV_OS_MALLOC_FUNC)
(
    GT_U32 size
);
/*******************************************************************************
* MV_OS_FREE_FUNC
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
*******************************************************************************/
typedef void (*MV_OS_FREE_FUNC)
(
    IN void* const memblock
);

/*******************************************************************************
* MV_OS_MEM_SET_FUNC
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
*******************************************************************************/
typedef void * (*MV_OS_MEM_SET_FUNC)
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/*******************************************************************************
* MV_OS_TIME_WK_AFTER_FUNC
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
*******************************************************************************/
typedef GT_STATUS (*MV_OS_TIME_WK_AFTER_FUNC)
(
    IN GT_U32 mils
);

/*******************************************************************************
* MV_OS_EXACT_DELAY_FUNC
*
* DESCRIPTION:
*       Implement exact time delay for specified number of millisecond.
*
* INPUTS:
*       mils - time to delay in milliseconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_OS_EXACT_DELAY_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 mils
);

/*******************************************************************************
* MV_SYS_DEVICE_INFO
*
* DESCRIPTION:
*       Return silicon ID and revision ID for current device number.
*
* INPUTS:
*       devNum - system device number
*
* OUTPUTS:
*       devId  - silicon ID
*       revNum - revision number
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_SYS_DEVICE_INFO)
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
);

/* structure that hold the "os" functions needed be bound to HWS */
typedef struct {
  MV_OS_EXACT_DELAY_FUNC   osExactDelayPtr;
  MV_OS_TIME_WK_AFTER_FUNC osTimerWkPtr;
  MV_OS_MEM_SET_FUNC       osMemSetPtr;
  MV_OS_FREE_FUNC          osFreePtr;
  MV_OS_MALLOC_FUNC        osMallocPtr;
  MV_SYS_DEVICE_INFO       sysDeviceInfo;

}HWS_OS_FUNC_PTR;

extern MV_OS_EXACT_DELAY_FUNC   hwsOsExactDelayPtr;
extern MV_OS_TIME_WK_AFTER_FUNC hwsOsTimerWkFuncPtr;
extern MV_OS_MEM_SET_FUNC       hwsOsMemSetFuncPtr;
extern MV_OS_FREE_FUNC          hwsOsFreeFuncPtr;
extern MV_OS_MALLOC_FUNC        hwsOsMallocFuncPtr;

/*******************************************************************************
* mvUnitInfoGet
*
* DESCRIPTION:
*       Return silicon specific base address and index for specified unit
*
* INPUTS:
*       devNum    - Device Number
*       unitId    - unit ID (MAC, PCS, SERDES)
*
* OUTPUTS:
*       baseAddr  - unit base address in device
*       unitIndex - unit index in device
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void  mvUnitInfoGet
(
    GT_U8           devNum,
    MV_HWS_UNITS_ID unitId,
    GT_U32          *baseAddr,
    GT_U32          *unitIndex
);

/*******************************************************************************
* mvUnitInfoSet
*
* DESCRIPTION:
*       Init silicon specific base address and index for specified unit
*
* INPUTS:
*       devType   - Device type
*       unitId    - unit ID (MAC, PCS, SERDES)
*       baseAddr  - unit base address in device
*       unitIndex - unit index in device
*
* OUTPUTS:
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS  mvUnitInfoSet
(
    MV_HWS_DEV_TYPE devType,
    MV_HWS_UNITS_ID unitId,
    GT_U32          baseAddr,
    GT_U32          unitIndex
);

/*******************************************************************************
* mvUnitInfoGetByAddr
*
* DESCRIPTION:
*       Return unit ID by unit address in device
*
* INPUTS:
*       devNum    - Device Number
*       baseAddr  - unit base address in device
*
* OUTPUTS:
*       unitId    - unit ID (MAC, PCS, SERDES)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void  mvUnitInfoGetByAddr
(
    GT_U8           devNum,
    GT_U32          baseAddr,
    MV_HWS_UNITS_ID *unitId
);

/*******************************************************************************
* genRegisterSet
*
* DESCRIPTION:
*       Implement write access to device registers.
*
* INPUTS:
*       devNum    - Device Number
*       portGroup - port group (core) number
*       address   - address to access
*       data      - data to write
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS genRegisterSet
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 address,
    GT_U32 data,
    GT_U32 mask
);

/*******************************************************************************
* genRegisterGet
*
* DESCRIPTION:
*       Read access to device registers.
*
* INPUTS:
*       devNum    - Device Number
*       portGroup - port group (core) number
*       address   - address to access
*
* OUTPUTS:
*       data      - read data
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS genRegisterGet
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address,
    GT_U32 *data,
    GT_U32 mask
);

/*******************************************************************************
* genInterlakenRegSet
*
* DESCRIPTION:
*       Implement write access to INERLAKEN IP registers.
*
* INPUTS:
*       devNum    - Device Number
*       portGroup - port group (core) number
*       address   - address to access
*       data      - data to write
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS genInterlakenRegSet
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address,
    GT_U32 data,
    GT_U32 mask
);

/*******************************************************************************
* genInterlakenRegGet
*
* DESCRIPTION:
*       Implement read access to INERLAKEN IP registers.
*
* INPUTS:
*       devNum    - Device Number
*       portGroup - port group (core) number
*       address   - address to access
*
* OUTPUTS:
*       data      - read data
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS genInterlakenRegGet
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address,
    GT_U32 *data,
    GT_U32 mask
);

/*******************************************************************************
* hwsLion2IfInit
*
* DESCRIPTION:
*       Init all supported units needed for ports initialization
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsLion2IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/*******************************************************************************
* hwsLion2IfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsLion2IfClose(GT_U8 devNum);

/*******************************************************************************
* hwsPuma3IfInit
*
* DESCRIPTION:
*       Init all supported units needed for ports initialization
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsPuma3IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/*******************************************************************************
* hwsPuma3IfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsPuma3IfClose(GT_U8 devNum);

/*******************************************************************************
* hwsHooperIfInit
*
* DESCRIPTION:
*       Init all supported units needed for ports initialization
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsHooperIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/*******************************************************************************
* hwsHooperIfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsHooperIfClose(GT_U8 devNum);

/*******************************************************************************
* mvHwsRedundancyVectorGet
*
* DESCRIPTION:
*       Get SD vector.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsRedundancyVectorGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  *sdVector
);

/*******************************************************************************
* hwsDeviceSpecGetFuncPtr
*
* DESCRIPTION:
*       Get function structure pointer.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsDeviceSpecGetFuncPtr(MV_HWS_DEV_FUNC_PTRS **hwsFuncsPtr);


#ifdef __cplusplus
}
#endif

#endif /* __siliconIf_H */


/*
 *------------------------------------------------------------------
 * $Log: mvSiliconIf.h,v $
 * Revision 1.1  2015/02/13 11:34:19  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
