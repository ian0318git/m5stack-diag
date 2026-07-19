/* $Id: prvCpssHwInit.h,v 1.1 2015/02/13 11:33:20 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/cpssHwInit/private/prvCpssHwInit.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssHwInit.h
*
* DESCRIPTION:
*       CPSS definitions for HW init.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssHwInith
#define __prvCpssHwInith

#include <cpss/generic/interrupts/private/prvCpssGenIntDefs.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/config/cpssGenCfg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* This is used to indicate the specific data types (pointer fields or write
   only data) in the registers addresses struct */
#define PRV_CPSS_SW_PTR_BOOKMARK_CNS     0xFFFFFFFF

/* This is used to mark pointer fields or registers that are not initialized in
   the registers addresses struct */
#define PRV_CPSS_SW_PTR_ENTRY_UNUSED     0xFFFFFFFB

/* This is used to mark write only data in the registers addresses struct */
#define PRV_CPSS_SW_TYPE_WRONLY_CNS     0x1

/* This is used to mark dynamically allocated pointer fields in the registers
   addresses struct */
#define PRV_CPSS_SW_TYPE_PTR_FIELD_CNS  0x2

/* macro to free an allocated pointer, and set it to NULL */
#if __WORDSIZE != 64
#define FREE_PTR_IF_NOT_NULL_MAC(ptr)       \
        if(ptr && (((GT_U32)ptr) != PRV_CPSS_SW_PTR_ENTRY_UNUSED)) \
        {                                   \
            cpssOsFree(ptr);                \
            ptr=NULL;                       \
        }
#else
#define FREE_PTR_IF_NOT_NULL_MAC(ptr)       \
        if(ptr && (((GT_UINTPTR)ptr) != 0xFFFFFFFFFFFFFFFB)) \
        {                                   \
            cpssOsFree(ptr);                \
            ptr=NULL;                       \
        }
#endif

/*
 * Typedef: struct PRV_CPSS_BOOKMARK_STC
 *
 * Description: This is used to indicate the specific data types (pointer fields or write only data) in the registers addresses struct
 *
 * Fields:
 *
 *              bookmark - bookmark itself
 *              type - SW type (PRV_CPSS_SW_TYPE_WRONLY_CNS or PRV_CPSS_SW_TYPE_PTR_FIELD_CNS)
 *              size - size of bookmark
 *              nextPtr - pointer to the next bookmark
  */
typedef struct GT_PACKED bkmark
{
                GT_U32  bookmark;
                GT_U32  type;
                GT_U32  size;
                GT_U32  *nextPtr;
} PRV_CPSS_BOOKMARK_STC;

/*
 * Typedef: struct PRV_CPSS_RE_INIT_RX_DESC_INFO_STC
 *
 * Description: The information block of a single Rx descriptors list used to re-initialise.
 *
 * Fields:
 *
 *      rxDescBlock         - (Points to) the HW descriptors block.
 *      totalDescNum        - total Number of Descriptors
 *      freeDescNum         - Number of all descriptors in list.
 *      next2ReturnOffset   - Offset in the list of the descriptor to which the next returned
 *                            buffer should be attached.
 *      next2ReceiveOffset  - Offset in the list of the descriptor from which the next packet
 *                            will be fetched when an Rx interrupt is received.
 *      freeDescNum         - Number of free descriptors in list.
 *      headerOffset        - Number of reserved bytes before each buffer, to be
 *                            kept for application and internal use.
 *      forbidQEn           - When set to GT_TRUE enabling the Rx SDMA on buffer
 *                            release is forbidden.
 */
typedef struct
{
    void                *rxDescBlock;
    GT_U32              totalDescNum;
    GT_U32              next2ReturnOffset;
    GT_U32              next2ReceiveOffset;

    GT_U32              freeDescNum;
    GT_U32              headerOffset;
    GT_BOOL             forbidQEn;
}PRV_CPSS_RE_INIT_RX_DESC_INFO_STC;

/*
 * Typedef: struct PRV_CPSS_DMA_RE_INIT_INFO_STC
 *
 * Description:
 *      The structure that hold info for DMA related settings
 *      done during 'PP logical init'.
 *      This info needed for devices that doing re-init
 *      without resetting the device HW.
 *
 * Fields:
 *
 *      netIfCfg                  - network interface configuration
 *      auqCfg                    - Address Update Messages Queue CFG
 *      fuqUseSeparate            - Use separate Queue for FDB Upload Messages
 *      fuqCfg                    - FDB Upload Messages Queue CFG
 *      auqInternalInfo           - Address Update Messages Queue info
 *      fuqInternalInfo           - FDB Upload Messages Queue info
 *      secondaryAuqInternalInfo  - Address Update Messages Queue seconary info
 *      rxDescInfo                - RX Descriotors info per RX Queue
 *      auq1InternalInfo          - Address Update Messages Queue1 info
 *      secondaryAuq1InternalInfo - Address Update Messages Queue1 seconary info
 */
typedef struct{
    CPSS_NET_IF_CFG_STC     netIfCfg;       /* parameter as given in 'phase 2' from application */
    CPSS_AUQ_CFG_STC        auqCfg;         /* parameter as given in 'phase 2' from application */
    GT_BOOL                 fuqUseSeparate; /* parameter as given in 'phase 2' from application */
    CPSS_AUQ_CFG_STC        fuqCfg;         /* parameter as given in 'phase 2' from application */

    PRV_CPSS_AU_DESC_CTRL_STC   auqInternalInfo[CPSS_MAX_PORT_GROUPS_CNS];/* AUQ cpss internal info -- per port group */
    PRV_CPSS_AU_DESC_CTRL_STC   fuqInternalInfo[CPSS_MAX_PORT_GROUPS_CNS];/* FUQ cpss internal info -- per port group */
    PRV_CPSS_AU_DESC_CTRL_STC   secondaryAuqInternalInfo[CPSS_MAX_PORT_GROUPS_CNS];/* secondary AUQ cpss internal info -- per port group */
    PRV_CPSS_RE_INIT_RX_DESC_INFO_STC   rxDescInfo[NUM_OF_RX_QUEUES]; /* Rx descriptors internal info */
    PRV_CPSS_AU_DESC_CTRL_STC   auq1InternalInfo[CPSS_MAX_PORT_GROUPS_CNS];/* AUQ cpss internal info -- per port group */
    PRV_CPSS_AU_DESC_CTRL_STC   secondaryAuq1InternalInfo[CPSS_MAX_PORT_GROUPS_CNS];/* secondary AUQ cpss internal info -- per port group */
}PRV_CPSS_DMA_RE_INIT_INFO_STC;

/*
 * typedef: enum PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT
 *
 * Description: Defines pre phase 1 init mode .
 *
 * Enumerations:
 *  PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_DEFAULT_INIT_E - regular pre phase 1 init mode
 *  PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_NO_PP_INIT_E   - no Pp initialization should be done
 *
 */
typedef enum
{
    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_DEFAULT_INIT_E,
    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_NO_PP_INIT_E
}
PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT;

/*******************************************************************************
* prvCpssPrePhase1PpInit
*
* DESCRIPTION:
*       private (internal) function to make phase1 pre-init
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       prePhase1InitMode  - pre phase 1 init mode.
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK                    - on success.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPrePhase1PpInit
(
    IN    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT     prePhase1InitMode
);


/*******************************************************************************
* prvCpssPrePhase1PpInitModeGet
*
* DESCRIPTION:
*       private (internal) function retrieving  pre phase1 init mode
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
* OUTPUTS:
*       prePhase1InitModePtr  - pointer to pre phase 1 init mode .
*
* RETURNS:
*       GT_OK                    - on success.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPrePhase1PpInitModeGet
(
    OUT PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT     *prePhase1InitModePtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbRelease
*
* DESCRIPTION:
*       private (internal) function to release all the DB of the device.
*       NOTE: function 'free' the allocated memory ,buffers, semaphores
*             and restore DB to 'pre-init' state
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number.
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbRelease
(
    IN    GT_U8     devNum
);

/*******************************************************************************
* prvCpssPpConfigDevDbBusBaseAddrAttach
*
* DESCRIPTION:
*       private (internal) function to attach the base address to devNum
*
*       this needed for the 'initialization after the DB release processing'
*       when the initialization for a device during 'phase 1' will be we will
*       know what 'old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number.
*       busBaseAddr -  the address of the device on the interface bus (pci/pex/smi/twsi)
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbBusBaseAddrAttach
(
    IN GT_U8        devNum,
    IN GT_UINTPTR   busBaseAddr
);

/*******************************************************************************
* prvCpssPpConfigDevDbInfoSet
*
* DESCRIPTION:
*       private (internal) function to :
*       1. first call for device or device did HW reset :
*           function set the info about the device regarding DMA allocations
*           given by the application.
*       2. else the function check that the info given now match the info given
*          of first time after the HW reset.
*
*       this needed for the 'Initialization after the DB release processing'
*       when the initialization for a device during 'Phase 1' will be we will
*       know what 'Old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number.
*       netIfCfgPtr - (pointer to)Network interface configuration.
*       auqCfgPtr - (pointer to)AU queue info
*       fuqUseSeparate - do we have another Queue for FU message
*       fuqCfgPtr - (pointer to)FU queue info
* OUTPUTS:
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_STATE             - the new info it not the same info that was
*                                  set for this device (according to the bus
*                                  base address) before the 'shutdown'
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*       GT_BAD_PTR               - one of the pointers is NULL
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbInfoSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_IF_CFG_STC          *netIfCfgPtr,
    IN CPSS_AUQ_CFG_STC             *auqCfgPtr,
    IN GT_BOOL                      fuqUseSeparate,
    IN CPSS_AUQ_CFG_STC             *fuqCfgPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbHwResetSet
*
* DESCRIPTION:
*       private (internal) function to state that the device did HW reset or that
*       the device finished the 'pp logic init' function
*
*       this needed for the 'initialization after the DB release processing'
*       when the initialization for a device during 'phase 1' will be we will
*       know what 'old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number
*       didHwReset - GT_TRUE - Did HW reset ,
*                    GT_FALSE -  finished pp logical init.
* OUTPUTS:
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbHwResetSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   didHwReset
);

/*******************************************************************************
* prvCpssPpConfigDevDbHwResetGet
*
* DESCRIPTION:
*       private (internal) function get the state that the device did HW reset
*       or that the device finished the 'pp logic init' function.
*
*       this needed for the 'initialization after the DB release processing'
*       when the initialization for a device during 'phase 1' will be we will
*       know what 'old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number
* OUTPUTS:
*       didHwResetPtr - GT_TRUE - Did HW reset ,
*                       GT_FALSE -  finished pp logical init.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbHwResetGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *didHwResetPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbNetIfInitSet
*
* DESCRIPTION:
*       Private (internal) function to set if network initialization is needed or
*       if configuration will be based on previously saved re-initialization parameters.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - The device number
*       needNetIfInit   - GT_TRUE  - Need network interface init
*                         GT_FALSE - Network interface init using previously saved values.
* OUTPUTS:
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbNetIfInitSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   needNetIfInit
);

/*******************************************************************************
* prvCpssPpConfigDevDbNetIfInitGet
*
* DESCRIPTION:
*       Private (internal) function to get whether network initialization is needed or
*       whether configuration will be based on previously saved re-initialization parameters.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number
* OUTPUTS:
*       needNetIfInitPtr    - (pointer to):
*                             GT_TRUE  - Need network interface init
*                             GT_FALSE - Network interface init using previously saved values.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbNetIfInitGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *needNetIfInitPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbPrvInfoSet
*
* DESCRIPTION:
*       private (internal) function to :
*       set the AUQ,FUQ private cpss info.
*
*       this needed for the 'Initialization after the DB release processing'
*       when the initialization for a device during 'Phase 1' will be we will
*       know what 'Old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number.
*       auqInternalInfoPtr - (pointer to)AU queue private info , of all port groups
*       fuqInternalInfoPtr - (pointer to)FU queue private info , of all port groups
*       secondaryAuqInternalInfoPtr - (pointer to)secondary AU queue private info , of all port groups
*       rxDescListPtr      - (pointer to)Rx descriptors list private info
*       auq1InternalInfoPtr - (pointer to)AU1 queue private info , of all port groups
*       secondaryAuq1InternalInfoPtr - (pointer to)secondary AU1 queue private info , of all port groups
* OUTPUTS:
*       none
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*
* COMMENTS:
*      NOTE: not checking for NULL pointers to speed up performances
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbPrvInfoSet
(
    IN GT_U8                        devNum,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *auqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *fuqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuqInternalInfoPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *auq1InternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuq1InternalInfoPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbPrvInfoGet
*
* DESCRIPTION:
*       private (internal) function to :
*       get the AUQ,FUQ private cpss info.
*
*       this needed for the 'Initialization after the DB release processing'
*       when the initialization for a device during 'Phase 1' will be we will
*       know what 'Old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number.
*       portGroupId - The port group Id , to support multi-port-groups device
* OUTPUTS:
*       auqInternalInfoPtr - (pointer to)AU queue private info
*       fuqInternalInfoPtr - (pointer to)FU queue private info (can be NULL when FUQ not used)
*       secondaryAuqInternalInfoPtr - (pointer to)secondary AU queue private info (can be NULL when queue not used)
*       auq1InternalInfoPtr - (pointer to)AU1 queue private info
*       secondaryAuq1InternalInfoPtr - (pointer to)secondary AU1 queue private info (can be NULL when queue not used)
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*       GT_BAD_PTR               - one of the pointers is NULL
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbPrvInfoGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *auqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *fuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *auq1InternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuq1InternalInfoPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbPrvNetInfInfoSet
*
* DESCRIPTION:
*       private (internal) function to :
*       set the Rx descriptors list private cpss info.
*
*       this needed for the 'Initialization after the DB release processing'
*       when the initialization for a device during 'Phase 1' will be we will
*       know what 'Old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum             - The device number.
*       rxDescPtr          - (pointer to) the start of Rx descriptors block.
*       rxDescListPtr      - (pointer to)Rx descriptors list private info.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*       GT_BAD_PTR               - one of the pointers is NULL
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfInfoSet
(
    IN GT_U8                        devNum,
    IN PRV_CPSS_RX_DESC_STC         *rxDescPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbPrvNetInfInfoGet
*
* DESCRIPTION:
*       private (internal) function to :
*       get the Rx descriptors list private cpss info.
*
*       this needed for the 'Initialization after the DB release processing'
*       when the initialization for a device during 'Phase 1' will be we will
*       know what 'Old devNum' it used and so accessing the special DB for this
*       device.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - The device number.
* OUTPUTS:
*       rxDescListPtr      - (pointer to) Rx descriptors list private info
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device Number
*       GT_NOT_FOUND             - device not found in the DB
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfInfoGet
(
    IN GT_U8                        devNum,
    OUT PRV_CPSS_RX_DESC_LIST_STC   *rxDescListPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbRenumber
*
* DESCRIPTION:
*       the function set parameters for the driver to renumber it's DB.
*       the function "renumber" the current device number of the device to a
*       new device number.
*       NOTE:
*       this function MUST be called under 'Interrupts are locked'
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       oldDevNum  - The PP's "old" device number .
*       newDevNum  - The PP's "new" device number swap the DB to.
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL     - on error
*       GT_ALREADY_EXIST - the new device number is already used
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevDbRenumber
(
    IN GT_U8    oldDevNum,
    IN GT_U8    newDevNum
);

/*******************************************************************************
* prvCpssPpConfigDevInfoGet
*
* DESCRIPTION:
*    Private (internal) function returns device static information
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - physical device number
*
* OUTPUTS:
*       devInfoPtr   - (pointer to) device information
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device number
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_GEN_CFG_DEV_INFO_STC   *devInfoPtr
);

/*******************************************************************************
* prvCpssPpConfigPortToPhymacObjectBind
*
* DESCRIPTION:
*         The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*    devNum         - device number
*    portNum        - port number
*    macDrvObjPtr   - port phymac object pointer
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK       - success
*       GT_FAIL     - error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssPpConfigPortToPhymacObjectBind
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssHwInith */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssHwInit.h,v $
 * Revision 1.1  2015/02/13 11:33:20  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
