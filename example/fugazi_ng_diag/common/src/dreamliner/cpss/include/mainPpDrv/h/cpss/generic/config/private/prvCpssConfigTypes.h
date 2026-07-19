/* $Id: prvCpssConfigTypes.h,v 1.1 2015/02/13 11:33:14 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/config/private/prvCpssConfigTypes.h,v $
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
* prvCpssConfigTypes.h
*
* DESCRIPTION:    Private Driver facility types definitions
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __prvCpssConfigTypesh
#define __prvCpssConfigTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
/* get public types */
#include <cpss/generic/cpssTypes.h>
/* get the OS , extDrv functions*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
/* get common defs */
#include <cpssCommon/cpssPresteraDefs.h>
/* get generic common defs */
#include <cpss/generic/cpssCommonDefs.h>
/* include HW init definitions */
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
/* get multi-port group utilities*/
#include <cpss/generic/cpssHwInit/private/prvCpssHwMultiPortGroups.h>
/* get registers access utilities : counters ,busy wait , multi-port groups */
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>

/* get the interrupts control info */
#include <cpss/generic/interrupts/private/prvCpssGenIntDefs.h>
/* include the "cpss drivers APIs" */
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
/* include the FDB types for the mac Addr Query Info */
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
/* get the port(s) info */
#include <cpss/generic/port/private/prvCpssPortTypes.h>
/* get trunk info */
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfMii.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>

/* check if device is EXMX , according to it's family */
#define CPSS_IS_EXMX_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PP_FAMILY_START_EXMX_E &&    \
      (devFamily) < CPSS_PP_FAMILY_END_EXMX_E)?1:0)


/* check if device is EXMXMP , according to it's family */
#define CPSS_IS_EXMXPM_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PP_FAMILY_START_EXMXPM_E &&    \
      (devFamily) < CPSS_PP_FAMILY_END_EXMXPM_E)?1:0)

/* check if device is DXCH , according to it's family */
#define CPSS_IS_DXCH_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PP_FAMILY_START_DXCH_E &&    \
      (devFamily) < CPSS_PP_FAMILY_END_DXCH_E)?1:0)

/* check if device is DXSAL , according to it's family */
#define CPSS_IS_DXSAL_FAMILY_MAC(devFamily)      \
    (((devFamily) == CPSS_PP_FAMILY_SALSA_E) ? 1 : 0)

/*
 * Typedef: struct PRV_CPSS_FAMILY_INFO_STC
 *
 * Description: A structure to hold common PP Family needed in CPSS
 *
 * Fields:
 *      trunkInfo - info of trunks common to all PP of a specific family
 *      numReferences - number of references by devices that associated with this family
 *                      needed to support the ability to release this DB
*/
typedef struct{
    PRV_CPSS_FAMILY_TRUNK_INFO_STC trunkInfo;
    GT_U32                         numReferences;
}PRV_CPSS_FAMILY_INFO_STC;

/* array of pointers to the valid families
(index to array is family type - one of CPSS_PP_FAMILY_TYPE_ENT) */
extern PRV_CPSS_FAMILY_INFO_STC* prvCpssFamilyInfoArray[CPSS_PP_FAMILY_LAST_E];


/* check that the device exists */
#define PRV_CPSS_IS_DEV_EXISTS_MAC(devNum)       \
    (((devNum) >= PRV_CPSS_MAX_PP_DEVICES_CNS || \
        prvCpssPpConfig[devNum] == NULL) ? 0 : 1)

/* check that the device exists
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_DEV_CHECK_MAC(devNum)                  \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))         \
        return GT_BAD_PARAM



/* macro to get a pointer on the ExMx device (exMx format)
    devNum - the device id of the exMx device

    NOTE : the macro do NO validly checks !!!!
           (caller responsible for checking with other means/macro)
*/
#define PRV_CPSS_PP_MAC(devNum) \
    ((PRV_CPSS_GEN_PP_CONFIG_STC*)prvCpssPpConfig[devNum])

/* check that the port number is valid virtual port (CPU port is invalid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_VIRTUAL_PORT_CHECK_MAC(devNum,portNum)        \
    if(portNum >= PRV_CPSS_PP_MAC(devNum)->numOfVirtPorts) \
    {                                 \
        return GT_BAD_PARAM;          \
    }

/* check that the port number is valid physical port (CPU port is invalid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,portNum)            \
    if((portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts) ||     \
        (0 == (CPSS_PORTS_BMP_IS_PORT_SET_MAC(                 \
            &(PRV_CPSS_PP_MAC(devNum)->existingPorts) , portNum)))) \
    {                                 \
        return GT_BAD_PARAM;          \
    }

/* check that port is exist in the existing ports bitmap,       */
/* the macro does not check that port not exeeds the numOfPorts */
#define PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum)              \
        (0 != (CPSS_PORTS_BMP_IS_PORT_SET_MAC(                       \
            &(PRV_CPSS_PP_MAC(devNum)->existingPorts) , portNum)))

/* Check for device's port existence and skip if port does not exist */
#define PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(_dev, _port) \
    if(!PRV_CPSS_PHY_PORT_IS_EXIST_MAC(_dev, _port)) \
        continue;

/* check that the port number is valid virtual port (CPU port is valid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_VIRTUAL_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum) \
    if ((portNum) != CPSS_CPU_PORT_NUM_CNS)                 \
        PRV_CPSS_VIRTUAL_PORT_CHECK_MAC(devNum,portNum)

/* check that the port number is valid physical port (CPU port is valid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum)     \
    if ((portNum) != CPSS_CPU_PORT_NUM_CNS)                 \
        PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,portNum)

/* access to the device's trunk info of the device */
#define PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)  \
    (&(PRV_CPSS_PP_MAC(devNum)->trunkInfo))

/* bmp for hw interfaces */
#define PRV_CPSS_HW_IF_PCI_BMP_CNS      (1 << CPSS_CHANNEL_PCI_E)
#define PRV_CPSS_HW_IF_SMI_BMP_CNS      (1 << CPSS_CHANNEL_SMI_E)
#define PRV_CPSS_HW_IF_TWSI_BMP_CNS     (1 << CPSS_CHANNEL_TWSI_E)
#define PRV_CPSS_HW_IF_PEX_BMP_CNS      (1 << CPSS_CHANNEL_PEX_E)

/*set the HW interface bmp */
#define PRV_CPSS_HW_IF_BMP_MAC(mngInterfaceType)\
    (1 << (GT_U32)mngInterfaceType)

/* bmp of "PCI" compatible behavior */
#define PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_PCI_BMP_CNS) ? 1 : 0)

/* bmp of "SMI" compatible behavior */
#define PRV_CPSS_HW_IF_SMI_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_SMI_BMP_CNS) ? 1 : 0)

/* bmp of "TWSI" compatible behavior */
#define PRV_CPSS_HW_IF_TWSI_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_TWSI_BMP_CNS) ? 1 : 0)

/* bmp of "PEX" compatible behavior */
#define PRV_CPSS_HW_IF_PEX_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_PEX_BMP_CNS) ? 1 : 0)

/* Current primary AUQ index */
#define PRV_CPSS_AUQ_INDEX_MAC(devNum, portGroup)\
    (PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroup])

/* Current secondary AUQ index */
#define PRV_CPSS_SECONDARY_AUQ_INDEX_MAC(devNum, portGroup)\
    (PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroup].activeSecondaryAuqIndex)

/* Secondary AUQ check condition */
#define PRV_CPSS_SECONDARY_AUQ_CHECK_MAC(devNum, portGroup)\
    ((PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroup] == \
      PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroup].activeSecondaryAuqIndex) \
      ? GT_TRUE : GT_FALSE)

/* check that the AU descriptors initialized (relevant only to DMA supporting device) */
#define PRV_CPSS_AUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum)        \
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)].blockAddr == 0)  \
        return GT_NOT_INITIALIZED
/* check that the secondary AU descriptors initialized (relevant only to DMA supporting device) */
#define PRV_CPSS_SECONDARY_AUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum)        \
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)].blockAddr == 0)  \
        return GT_NOT_INITIALIZED

/* check that the FU descriptors initialized (relevant only to DMA supporting device) */
#define PRV_CPSS_FUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum)        \
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)].blockAddr == 0)  \
        return GT_NOT_INITIALIZED

/* use HW devNum and not the 'SW devNum' */
#define PRV_CPSS_HW_DEV_NUM_MAC(devNum)  \
    (PRV_CPSS_PP_MAC(devNum)->hwDevNum)

/* macro to check if current iteration is not valid any more
    devNum - the device number to check with it it's max number
             the device number is controlling the "HW interface bus" that allow
             us to tune the needed value for max number of iterations
    currIter - the current iteration number , should be 0...max in increasing
               manner
*/
#define  PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter)    \
    if((currIter) >= PRV_CPSS_PP_MAC(devNum)->maxIterationsOfBusyWait) \
        return GT_TIMEOUT


/* max number of ports in single port group.
   NOTE: event that the Lion actually support only 12 ports,
        still it 'reserve' 16 ports for calculations:
            <<4 , & 0xf
*/
#define PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS   16

/*
 * Typedef: struct PRV_CPSS_GEN_PP_PORT_GROUPS_INFO_STC
 *
 * Description: A structure to hold info about the port groups of a device
 *              NOTE: this structure relevant ALSO to a 'non-multi port groups' device
 *                  -- currently used only in the DXCH and not in the exmxpm,exmx,dxsal
 * Fields:
 *      isMultiPortGroupDevice - is the device : 'multi-port-groups' device.
 *      numOfPortGroups        - The number of port groups that the device support.
 *                     NOTE: this number not depend on the number of active port groups ,
 *                     for example this value is 4 always for Lion.
 *      activePortGroupsBmp    - Bitmap of active port groups
 *      firstActivePortGroup   - The portGroupId of the first active port group id
 *      lastActivePortGroup    - The portGroupId of the last active port group id,
 *                     Note that there can be non active port groups
 *                     between this port group and the firstActivePortGroup
 *                     used for loops and for allocations for port groups.
 *                     --> used instead of : 'num Of port groups'
 *                     dynamic allocate fields that are 'per port group' , according
 *                     to (lastValidPortGroup + 1)
 *      cpuPortPortGroupId - The portGroupId that used for 'cpu port' as SDMA/RGMII.
 *                for 'non multi port group' device --> 0
 *          NOTE: 1. Lion: For RGMII should be 3
 *                2. Lion: For SDMA should be CPSS_PORT_GROUP_UNAWARE_MODE_CNS
 *                3. This portGroupId used only for 'cpu port' and not for all networkIf
 *                   configurations that may use firstActivePortGroup!!!
 *                4. in Lion2/Puma3 the physical CPU port is valid only
 *                          in one of the network ports groups.
 *      loopbackPortGroupId - in Puma3 the physical loopback port is valid only
 *                          in one of the network ports groups.
 *      auqPortGroupIdLastServed - 'last served' port group# for AUQ .
 *                  needed for the 'Round Robin' of the AUQ manager
 *                for 'non multi port groups' device --> 0
 *      fuqPortGroupIdLastServed - 'last served' port group# for FUQ .
 *                  needed for the 'Round Robin' of the FUQ manager
 *                for 'non multi port groups' device --> 0
 *      secureBreachNextPortGroupToServe - 'Next to serve' port group# for Secure Breach
 *                  messages. Needed for the 'Round Robin' of the Secure Breach messages
 *                  manager. for 'non multi port groups' device --> 0
 *      ptpNextPortGroupToServe - 'Next to serve' port group# for PTP FIFO entries.
 *                  Needed for the 'Round Robin' of the PTP FIFO entries
 *                  manager. for 'non multi port groups' device --> 0
 *
*/
typedef struct{
    GT_BOOL     isMultiPortGroupDevice;
    GT_U32      numOfPortGroups;
    GT_U32      activePortGroupsBmp;
    GT_U32      firstActivePortGroup;
    GT_U32      lastActivePortGroup;
    GT_U32      cpuPortPortGroupId;
    GT_U32      loopbackPortGroupId;
    GT_U32      auqPortGroupIdLastServed;
    GT_U32      fuqPortGroupIdLastServed;
    GT_U32      secureBreachNextPortGroupToServe;
    GT_U32      ptpNextPortGroupToServe;
    GT_U32      portGroupPizzaProfileID[PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS];  /* 0 - default profile*/
} PRV_CPSS_GEN_PP_PORT_GROUPS_INFO_STC;


/* macro to start a loop on active port groups */
#define PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)   \
    /* loop on all port groups */                                     \
    for((portGroupId) = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup; \
        (portGroupId) <= PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup; \
        (portGroupId)++)                                                     \
    {                                                                    \
        if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupId))))  \
        {                                                                \
            continue;                                                    \
        }

/* macro to end a loop on active port groups */
#define PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)     \
    } /* end of 'for' */


/* macro to start to loop on port groups according to portGroupsBmp , but only
   on active port groups */
#define PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)   \
    /* loop on all port groups */                                        \
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)       \
    {                                                                    \
        /* skip port groups that are not in bmp */                       \
        if(0 == (portGroupsBmp & (1<<(portGroupId))))                    \
        {                                                                \
            continue;                                                    \
        }

/* macro to end a loop on port groups according to portGroupsBmp */
#define PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)     \
    }                                                               \
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)


/* macro to start a loop on NON-active port groups */
#define PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum,portGroupId)     \
    /* loop on all port groups */                                     \
    for((portGroupId) = 0; (portGroupId) < PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups; (portGroupId)++)\
    {                                                                    \
        if(0 != (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupId))))  \
        {                                                                \
            continue;                                                    \
        }

/* macro to end a loop on NON-active port groups */
#define PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum,portGroupId)     \
    } /* end of 'for' */


/* check if the device is 'multi-port groups' device */
#define PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) \
    ((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE) ? 1 : 0)

/* macro to get the first active port group */
#define PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup

/* macro to get the last active port group */
#define PRV_CPSS_LAST_ACTIVE_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup

/* macro to get the 'CPU port' portGroupId.
    NOTE: for RGMII interface it will be single portGroupId (in Lion portGroupId #3)
          for SDMA interface it will be 'All port groups'
*/
#define PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.cpuPortPortGroupId

/* macro to get the physical 'loopback port' portGroupId. */
#define PRV_CPSS_LOOPBACK_PORT_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.loopbackPortGroupId


/* macro to get the portGroupId for SDMA configurations .
   that way we allow application to give SDMA memory to single port group instead
   of split it between all active port groups.

   ONLY the settings of descriptors that relate to RX/TX packets, will use this
   portGroupId.

   NOTE:
       1. all other non-SDMA relate registers, are set to all active port groups.
       2. this is not relate to 'CPU port' configurations
*/
#define PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum)   \
    PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId

/* check if port number in range that need local port conversion */

#define PRV_CPSS_PORT_GROUP_CONVERT_CHECK_MAC(portNum)  \
    (((portNum) < CPSS_NULL_PORT_NUM_CNS || (portNum) > CPSS_CPU_PORT_NUM_CNS)? 1 : 0)

/* convert 'global' port number to 'portGroupId' */
#define PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum) \
    ((PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0) ?             \
      PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum) :                    \
      PRV_CPSS_PORT_GROUP_CONVERT_CHECK_MAC(portNum) ?                     \
      (((GT_U32)portNum) >> 4) :                                           \
      CPSS_PORT_GROUP_UNAWARE_MODE_CNS)

/* convert 'global' port number to 'local' port number */
#define PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum)     \
    (GT_U8)((PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0) ?       \
      portNum: /*no conversion */                                           \
      PRV_CPSS_PORT_GROUP_CONVERT_CHECK_MAC(portNum) ?                      \
      ((portNum)& 0xf) :                                                    \
      (portNum))
/*
NOTE: the #define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
    should be only in C files that handle the MAC for the CPU port

    and they need the difference between
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC ,
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC
*/

#ifdef PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
    /* convert 'Physical global' port number to 'portGroupId'
        this macro must be used for 'MAC registers' for physical ports.
        (in the 'Port Library')

        this macro different from the PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC
        for the treatment for CPU port.
    */
    #define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum) \
          (((portNum) == CPSS_CPU_PORT_NUM_CNS) ?                        \
           PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum) :                       \
           PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum))
#else  /*!PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS*/
    /* Not allowing this C file to use this macro since it is not dealing with
        the MAC of the port.

        !!! So we create compilation error !!!   'Undeclared identifier'

        caller must instead use PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum)
    */
    #define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum) \
        use_of_this_macro_not_allowed_from_this_C_file  /* undeclared identifier */

#endif /*!PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS*/

/* convert 'local' port number to 'global' port number */
#define PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum, portGroupId, portNum) \
    (GT_U8)((PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0) ?      \
      portNum: /*no conversion */                                   \
      ((portNum) < CPSS_NULL_PORT_NUM_CNS) ?                        \
        ((((portGroupId)      )<< 4) | ((portNum)& 0xf)) :          \
      (portNum))

/* convert bmp of ports of the device , to bmp for the relevant port group
    NOTE: macro get pointer to BMP of ports --> ports[0],ports[1]
    but return GT_U32 as ports bmp relevant to the port group !


    -- macro must NOT be called for 'non-multi port groups' device
*/
#define PRV_CPSS_DEVICE_PORTS_BMP_TO_PORT_GROUP_PORTS_BMP_CONVERT_MAC(devNum,portsBitmapPtr,portGroupId) \
        /* each port group cover 16 ports of the full bmp of 64 ports */                 \
        (U32_GET_FIELD_MAC(portsBitmapPtr->ports[(portGroupId) / 2],                    \
                          (PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS * ((portGroupId) % 2)),\
                           PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS ))

/* convert bmp of ports of specific port group , and add it to bmp of ports of device
    NOTE: macro get pointer to GT_32 of ports
    and fill pointer to ports BMP --> ports[0],ports[1]

    -- macro must NOT be called for 'non-multi port groups' device
*/
#define PRV_CPSS_PORT_GROUP_PORTS_BMP_TO_DEVICE_PORTS_BMP_CONVERT_MAC(devNum,portGroupId,portGroupPortsBmp,portsBitmapPtr) \
        /* each port group cover 16 ports of the full bmp of 64 ports */          \
        (portsBitmapPtr)->ports[(portGroupId) / 2] |= (portGroupPortsBmp) <<          \
            (PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS * ((portGroupId) % 2))

/* check and return 'not applicable device' when device is NOT multi-port group device */
#define PRV_CPSS_MULTI_PORT_GROUPS_DEVICE_CHECK_MAC(devNum)  \
    if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0)   \
        return GT_NOT_APPLICABLE_DEVICE

/* check and return 'bad param' when device is multi-port group device , but
   portGroupsBmp indicate non active port groups
   for non-multi port groups device --> macro SETs the  portGroupsBmp to
   CPSS_PORT_GROUP_UNAWARE_MODE_CNS

   macro's logic :
   1. for non-multi port group device --> SETs the  portGroupsBmp to
      CPSS_PORT_GROUP_UNAWARE_MODE_CNS
   2. when portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS --> never error
   3. when portGroupsBmp == 0 --> error
   4. when ((portGroupsBmp &activePortGroupsBmp) != portGroupsBmp)   --> error
*/

#define PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp)  \
    if(!PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))               \
    {                                                                   \
        /* we ignore parameter from application and use 'unaware' */    \
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;               \
    }                                                                   \
    else if ((portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS) &&     \
             (portGroupsBmp == 0 ||                                     \
             ((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & portGroupsBmp) != portGroupsBmp)))  \
                return GT_BAD_PARAM;

/*
    This macro assings the portGroup variable to first port group
    of the given device contained in given bitmap.
    If there is no such it returns GT_BAD_PARAM.
*/
#define PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(            \
    devNum, portGroupsBmp, portGroupId)                                 \
    if (GT_OK != prvCpssPpConfigPortGroupFirstActiveGet(                \
        devNum, portGroupsBmp, &portGroupId))                           \
    {                                                                   \
        return GT_BAD_PARAM;                                            \
    }

/* check device by not applicable families bitmap */
#define PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(_devNum, _unappFamBmp)  \
   if (PRV_CPSS_PP_MAC(_devNum)->appDevFamily & (_unappFamBmp))       \
        return GT_NOT_APPLICABLE_DEVICE /* ; in caller */


extern CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT prvCpssHwDevNumModeDb[BIT_12];

/* check if HW device number represents device that uses dual device Id
   input : HW Device Number (not SW devNum !)
   return :
   0 - not dual device Id
   1 - dual device Id
*/
#define PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(hwDevNum) \
    ((((hwDevNum) < BIT_12) && (prvCpssHwDevNumModeDb[(hwDevNum)] == CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E) ?\
      1 : 0))

/* check that when HW device number represents device with dual device Id used
   only even number (not odd)
   returns GT_BAD_PARAM when : dual device Id with odd number
*/
#define PRV_CPSS_DUAL_HW_DEVICE_CHECK_MAC(hwDevNum) \
        if (PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(hwDevNum) && ((hwDevNum) %2)) \
            return GT_BAD_PARAM


/*
 * Typedef: enumeration CPSS_PP_FAMILY_BIT_ENT
 *
 * Description: The PP family representing bit.
 *              Used to check the device in not applicable families bitmap.
 *              The same values used for DxCh and for ExMxPm families
 *
 * Fields:
 *      CPSS_CH1_E          - (DXCH) Cheetah family devices
 *      CPSS_CH1_DIAMOND_E  - (DXCH) Cheetah Diamond family devices
 *      CPSS_CH2_E          - (DXCH) Cheetah 2 family devices
 *      CPSS_CH3_E          - (DXCH) Cheetah 3 family devices
 *      CPSS_XCAT_E         - (DXCH) xCat family devices
 *      CPSS_LION_E         - (DXCH) Lion family devices
 *      CPSS_XCAT2_E        - (DXCH) xCat2 family devices
 *      CPSS_LION2_E        - (DXCH) Lion2 family devices
 *      CPSS_PUMA_E         - (EXMXPM) Puma(2) family devices
 *      CPSS_PUMA3_E        - (EXMXPM) Puma3 family devices
 *      CPSS_LION3_E        - (DXCH) Lion3 family devices
 *
 *      CPSS_PP_FAMILY_BIT_LAST_E - last in the enumeration (NOT TO BE USED)
 */
typedef enum
{
    CPSS_CH1_E         = BIT_0,
    CPSS_CH1_DIAMOND_E = BIT_1,
    CPSS_CH2_E         = BIT_2,
    CPSS_CH3_E         = BIT_3,
    CPSS_XCAT_E        = BIT_4,
    CPSS_LION_E        = BIT_5,
    CPSS_XCAT2_E       = BIT_6,
    CPSS_LION2_E       = BIT_7,
    CPSS_PUMA_E        = BIT_8,
    CPSS_PUMA3_E       = BIT_9,
    CPSS_LION3_E       = BIT_10,

    CPSS_PP_FAMILY_BIT_LAST_E

} CPSS_PP_FAMILY_BIT_ENT;

/*
 * Typedef: struct PRV_CPSS_GEN_PP_CONFIG_STC
 *
 * Description: A structure to hold all PP data needed in CPSS
 *
 * Fields:
 *      devNum       - PP's device number represented by this info.
 *      functionsSupportedBmp - bmp of types of supported functions
 *                    each CPSS API function will check if the device can "run"
 *                    it , so this BMP will allow the device to be called for
 *                    ExMx / Tiger / DxChx functions ..
 *      devType      - device type of the PP.
 *      revision     - The device's revision number.
 *      devFamily    - CPSS's device family that current device belongs to.
 *      appDevFamily - device family bit to check in unapplicable families bitmap
 *      numOfPorts   - number of ports in this device
 *      numOfVirtPorts - number of virtual ports.
 *                       For Value Blade capable devices the value of
 *                       numOfVirtPorts may be different from numOfPorts.
 *                       For not VB devices numOfVirtPorts = numOfPorts.
 *      existingPorts - bmp of actual exiting ports
 *                      needed for devices that their physical ports not
 *                      continues from first port num to last port num
 *      baseCoreClock - the 'base core'  clock , as documented for the theoretical
 *                      calculations of 'clock' : shaper,Policer,aging...
 *      coreClock - device's core clock frequency and not system (in Mhz)
 *      hwIfSupportedBmp  - bmp of supported Management interface types.
 *                          this BMP allow to support "old" hw if with it's
 *                          newer protocol version, like in the case of "PCI"
 *                          and "PEX"(PCI Express)
 *      intCtrl - Includes control structures needed for managing the
 *                    interrupts received as a result of Rx / Tx / AU ...
 *      hwDevNum      - HW device number.
 *      macAddrQueryInfo - info from the PP about the query that SW triggered
 *      configInitAfterHwResetDone - set to GT_FALSE at "phase 2" and "shutdown
 *                                   Warm Hw Reset"
 *                                   set to GT_TRUE at "pp logical init"
 *                                   Only after HW reset or "Shutdown Warm Hw
 *                                   Reset" need to do full initialization
 *      ppHAState    - PP's High availability mode : Active / Standby
 *      trunkInfo    - DB of the about trunks in that device
 *      phyPortInfoArray - array of port info structures
 *      phyInfo      - PHY related information
 *      useIsr        - indication that the device connected to the ISR of the
 *                      cpss. meaning that the device get ISR services from ISR.
 *                      This also imply that the events of the device managed
 *                      by the cpss of externally.
 *                      GT_TRUE - the cpss manage the ISR and the events of the
 *                                device.
 *                      GT_FALSE - the cpss NOT manage the ISR and NOT the events
 *                                of the device, and the application responsible
 *                                to have it's own ISR and manage the events.
 *                      NOTE: this relate to the
 *                      CPSS_DXCH_PP_PHASE1_INIT_INFO_STC::intVecNum ==
 *                       CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS
 *     portGroupsInfo -   Info about the port groups of the device.
 *                   NOTE: it is relevant also to 'non multi port groups' devices
 *                         for the simplicity of the macros that needed to
 *                         support loops on active port groups !!!
 *                         for example see macro PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC
 *     cpuPortMode - hold current CPU network interface mode sdma/mii/none
 *     netifSdmaPortGroupId - the port group on which CPU network interface is operational.
 *                            relevant to CPSS_NET_CPU_PORT_MODE_SDMA_E mode only.
 *      maxIterationsOfBusyWait - maximum iterations when doing busy wait on
 *                  value/filed in all loop. after that number of loops ,
 *                  the function will return GT_TIMEOUT
 *      hitlessWriteMethodEnable - In some cases under reconfiguration hitless write methood is used.
 *                                 This methood is in verifying memory contents before writing.
 *                                 If memory contents and writing data are the same no write is performed,
 *                                 otherwise write is done.
 *                                 GT_TRUE: hitless write methood is enable.
 *                                 GT_FALSE: hitless write methood is disable.
 */
typedef struct
{
    GT_U8                           devNum;
    GT_U32                          functionsSupportedBmp;
    CPSS_PP_DEVICE_TYPE             devType;
    GT_U8                           revision;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    CPSS_PP_FAMILY_BIT_ENT          appDevFamily;
    GT_U8                           numOfPorts;
    GT_U8                           numOfVirtPorts;
    CPSS_PORTS_BMP_STC              existingPorts;
    GT_U32                          baseCoreClock;
    GT_U32                          coreClock;
    GT_U32                          hwIfSupportedBmp;
    GT_HW_DEV_NUM                   hwDevNum;
    PRV_CPSS_INTERRUPT_CTRL_STC     intCtrl;
    GT_BOOL                         configInitAfterHwResetDone;
    CPSS_SYS_HA_MODE_ENT            ppHAState;
    PRV_CPSS_TRUNK_DB_INFO_STC      trunkInfo;
    PRV_CPSS_PORT_INFO_ARRAY_STC    *phyPortInfoArray;
    PRV_CPSS_PHY_INFO_STC           phyInfo;
    GT_BOOL                         useIsr;
    PRV_CPSS_GEN_PP_PORT_GROUPS_INFO_STC  portGroupsInfo;
    CPSS_NET_CPU_PORT_MODE_ENT      cpuPortMode;
    GT_U32                          netifSdmaPortGroupId;
    GT_U32                          maxIterationsOfBusyWait;
    GT_BOOL                         hitlessWriteMethodEnable;

} PRV_CPSS_GEN_PP_CONFIG_STC;

/* array of pointers to the valid devices */
extern void* prvCpssPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

/*
 * typedef: struct PRV_CPSS_HA_CONFIG_INFO_STC
 *
 * Description: structure for export/import data fro device config during HA catch up process
 *
 * Fields:
 *      deviceId       - device type of PP.
 *      revision       - revision of PP.
 */
typedef struct
{
    CPSS_PP_DEVICE_TYPE deviceId;
    GT_U8               revision;
}PRV_CPSS_PP_HA_CONFIG_INFO_STC;

/*
 * typedef: struct RE_INITIALZATION_DEV_INFO_STC
 *
 * Description: This structure holds per device the DMA pointers allocated for it,
 *              for the 'reuse' after re-initialization due to 'shutdown and restart'
 *
 * Fields:
 *      busBaseAddr           - bus base address is used as 'key' for attaching a device number
 *      deviceDidHwReset      - if device HW reset occured
 *      deviceNeedNetIfInit   - indicate if network initialization is needed or
 *                              configuration based on previously saved parameters
 *      dmaInfoValid          - shows whether dma info valid
 *      dmaInfo               - dma info
 */

typedef struct{
    GT_UINTPTR busBaseAddr;
    GT_BOOL deviceDidHwReset;
    GT_BOOL deviceNeedNetIfInit;
    GT_BOOL dmaInfoValid;
    PRV_CPSS_DMA_RE_INIT_INFO_STC   dmaInfo;
}RE_INITIALZATION_DEV_INFO_STC;

/*
 * Typedef: struct PRV_CPSS_GEN_GLOBAL_DATA_STC
 *
 * Description: This structure contains all Cpss Generic global data
 *
 * Fields:
 *   cpssPpHaDevData           - array of pointers to application supplied info about HA for the PPs
 *   prvCpssAfterDbRelease     -  flag that state the DB of cpss was released and
 *                                  now we are after that stage.
 *                                  NOTE: the flag start with GT_FALSE , but once changed
 *                                  to GT_TRUE it will stay that way forever.
 *   reInitializationDbPtr     - DB to hold per device the DMA pointers allocated for it,
 *                                  for the 'reuse' after re-initialization due to
 *                                  'shutdown and restart'
 *                                  NOTE : this DB must NOT be dynamic allocated ,
 *                                  because all the pointers from cpssOsMalloc considered
 *                                  'not valid' after the 'release DB' operation
 *   prvMiiDevNum              -  Dev num conected to CPU by MII port
 *   prvCpssGenNetIfMiiTxCtrl  -  mii tx control info DB
 *   prvCpssGenNetIfMiiRxQCtrl -  mii rx control info DB
 *   prvCpssGenNetIfMiiCtrl    -  shows that mii control info DB's are initialized
 *
 *      cpssPpInitCalled    - indication that cpssPpInit(...) was called
 *      supportDualDeviceId - indication that the system is in 'dual deviceId' mode.
 *               GT_TRUE - This system support dual device Id
 *                   This means that application MUST not use 'odd' hwDevNum to any device in the system.
 *                   Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
 *               GT_FALSE - This system NOT support dual device Id
 */

typedef struct
{
    PRV_CPSS_PP_HA_CONFIG_INFO_STC *cpssPpHaDevData[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_BOOL  prvCpssAfterDbRelease;
    RE_INITIALZATION_DEV_INFO_STC reInitializationDbPtr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32 prvMiiDevNum;
    PRV_CPSS_NETIF_MII_TX_CTRL_STC   prvCpssGenNetIfMiiTxCtrl;
    PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC  prvCpssGenNetIfMiiRxQCtrl;
    PRV_CPSS_GEN_NETIF_ETH_PORT_CTRL_VALID_STC prvCpssGenNetIfMiiCtrl;
    GT_BOOL                 cpssPpInitCalled;
    GT_BOOL                 supportDualDeviceId;

} PRV_CPSS_GEN_GLOBAL_DATA_STC;

extern PRV_CPSS_GEN_GLOBAL_DATA_STC sysGenGlobalInfo;
/*******************************************************************************
* prvCpssSysConfigPhase1
*
* DESCRIPTION:
*       This function sets cpss system level system configuration parameters
*       before any of the device's phase1 initialization .
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success,
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssSysConfigPhase1
(
    void
);

/*******************************************************************************
* prvCpssHwIfSet
*
* DESCRIPTION:
*       This function sets cpss HW interface parameters.
*       called during "phase1" initialization .
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device number
*       mngInterfaceType  - Management interface type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success,
*       GT_BAD_PARAM             - on bad device number or interface type
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssHwIfSet(
    IN GT_U8                            devNum,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT    mngInterfaceType
);

/*******************************************************************************
* prvCpssNonMultiPortGroupsDeviceSet
*
* DESCRIPTION:
*       This function sets cpss DB to be 'non-multi-port-groups' device.
*       It is called during "phase1" initialization.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success,
*       GT_BAD_PARAM             - on bad device number
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssNonMultiPortGroupsDeviceSet(
    IN GT_U8                            devNum
);


/*******************************************************************************
* prvCpssPpConfigBitmapFirstActiveBitGet
*
* DESCRIPTION:
*       Get the first active bit from the given bitmap.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       bitmap          - sorce bitmap.
* OUTPUTS:
*       firstBitPtr     - pointer to the index of the found bit
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum or portGroupsBmp
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigBitmapFirstActiveBitGet
(
    IN  GT_U32     bitmap,
    OUT GT_U32     *firstBitPtr
);

/*******************************************************************************
* prvCpssPpConfigPortGroupFirstActiveGet
*
* DESCRIPTION:
*       Get the first active port group from the given bitmap.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - The PP's device number .
*       portGroupsBmp    - bitmap of Port Groups.
*                      NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                          bitmap must be set with at least one bit representing
*                          valid port group(s). If a bit of non valid port group
*                          is set then function returns GT_BAD_PARAM.
*                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       portGroupIdPtr  - pointer to port Group Id.
*                         for non multi-port groups device
*                         gets the CPSS_PORT_GROUP_UNAWARE_MODE_CNS value
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum or portGroupsBmp
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigPortGroupFirstActiveGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT GT_U32                 *portGroupIdPtr
);

/*******************************************************************************
* prvCpssPpConfigBitmapNumBitsGet
*
* DESCRIPTION:
*       Get the number of bits that set set to 1 in the given bitmap.
*
* INPUTS:
*       bitmap          - source bitmap.
* OUTPUTS:
*       None
*
* RETURNS:
*       numbert of bits that are set to 1 in bitmap
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 prvCpssPpConfigBitmapNumBitsGet
(
    IN  GT_U32     bitmap
);

/*******************************************************************************
* prvCpssPpConfigNumActivePortGroupsInBmpGet
*
* DESCRIPTION:
*       Get the number of active port groups in the given bitmap.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - The PP's device number .
*       portGroupsBmp    - bitmap of Port Groups.
*                      NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                          bitmap must be set with at least one bit representing
*                          valid port group(s). If a bit of non valid port group
*                          is set then function returns GT_BAD_PARAM.
*                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       numActivePortGroupsPtr  - (pointer to)number of active port groups in the BMP.
*                          for non multi-port groups device : value 1 is returned.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum or portGroupsBmp
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigNumActivePortGroupsInBmpGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT GT_U32                 *numActivePortGroupsPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssConfigTypesh */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssConfigTypes.h,v $
 * Revision 1.1  2015/02/13 11:33:14  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
