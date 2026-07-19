/* $Id: prvCpssDxChHsu.h,v 1.1 2015/02/13 11:32:54 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h,v $
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
* prvCpssDxChHsu.h
*
* DESCRIPTION:
*       CPSS DxCh HSU data.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __prvCpssDxChHsuh
#define __prvCpssDxChHsuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/systemRecovery/hsu/cpssGenHsu.h>
#include <cpss/generic/systemRecovery/hsu/private/prvCpssGenHsu.h>
#include <cpss/generic/interrupts/private/prvCpssGenIntDefs.h>
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/generic/port/private/prvCpssPortTypes.h>

#define PRV_CPSS_DXCH_HSU_GET_SIZE_IN_ONE_ITERATION_CNC 0xffffffff
#define PRV_CPSS_DXCH_HSU_ITERATOR_MAGIC_NUMBER_CNC     0xEFABCD90


 /*
 * typedef: enum PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ENT
 *
 * Description:
 *      it shows how to update stageArray structure.
 *
 * Enumerations:
 *
 *    PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_RXTX_E  - it used to update size and address only for rx/tx stages
 *    PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ALL_E   - it used to update size and address for all stages
 */

typedef enum
{
    PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_RXTX_E,
    PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ALL_E
} PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ENT;

/*
 * Typedef: enum PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT
 *
 * Description: It represents global shadow iterator stages
 *
 * values:
 *      PRV_CPSS_DXCH_HSU_SYS_GLOBAL_STAGE_E                            - global system  stage
 *      PRV_CPSS_DXCH_HSU_GEN_PP_CONFIG_STAGE_E                         - device generic stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE0_STAGE_E                         - rx queue0 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE1_STAGE_E                         - rx queue1 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE2_STAGE_E                         - rx queue2 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE3_STAGE_E                         - rx queue3 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE4_STAGE_E                         - rx queue4 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE5_STAGE_E                         - rx queue5 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE6_STAGE_E                         - rx queue6 stage
 *      PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE7_STAGE_E                         - rx queue7 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE0_STAGE_E                         - tx queue0 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE1_STAGE_E                         - tx queue1 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE2_STAGE_E                         - tx queue2 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE3_STAGE_E                         - tx queue3 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE4_STAGE_E                         - tx queue4 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE5_STAGE_E                         - tx queue5 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE6_STAGE_E                         - tx queue6 stage
 *      PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE7_STAGE_E                         - tx queue7 stage
 *
 *      PRV_CPSS_DXCH_HSU_GEN_TRUNK_ARRAY_STAGE_E                       - trunk array stage
 *      PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_ARRAY_STAGE_E           - all trunk members array stage
 *      PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_IS_LOCAL_ARRAY_STAGE_E  - trunk local indication array stage
 *      PRV_CPSS_DXCH_HSU_GEN_PORT_INFO_ARRAY_STAGE_E                   - port info array stage
 *
 *      PRV_CPSS_DXCH_HSU_BRIDGE_STAGE_E                                - brige info stage
 *      PRV_CPSS_DXCH_HSU_NETIF_STAGE_E                                 - network info stage
 *      PRV_CPSS_DXCH_HSU_POLICER_STAGE_E                               - policer info stage
 *      PRV_CPSS_DXCH_HSU_PORT_STAGE_E                                  - port info  stage
 *      PRV_CPSS_DXCH_HSU_PORT_GROUPS_STAGE_E                           - port groups info  stage
 *      PRV_CPSS_DXCH_HSU_GLOBAL_LAST_STAGE_E                           - last stage
 */
typedef enum
{
    PRV_CPSS_DXCH_HSU_SYS_GLOBAL_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_PP_CONFIG_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE0_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE1_STAGE_E ,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE2_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE3_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE4_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE5_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE6_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE7_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE0_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE1_STAGE_E ,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE2_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE3_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE4_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE5_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE6_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE7_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TRUNK_ARRAY_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_ARRAY_STAGE_E ,
    PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_IS_LOCAL_ARRAY_STAGE_E,
    PRV_CPSS_DXCH_HSU_GEN_PORT_INFO_ARRAY_STAGE_E,

    PRV_CPSS_DXCH_HSU_BRIDGE_STAGE_E,
    PRV_CPSS_DXCH_HSU_NETIF_STAGE_E,
    PRV_CPSS_DXCH_HSU_POLICER_STAGE_E,
    PRV_CPSS_DXCH_HSU_PORT_STAGE_E,
    PRV_CPSS_DXCH_HSU_PORT_GROUPS_STAGE_E,
    PRV_CPSS_DXCH_HSU_GLOBAL_LAST_STAGE_E
}PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT;



/*
 * typedef: enum CPSS_DXCH_HSU_DATA_TYPE_ENT
 *
 * Description:
 *      Types of cpss data for export/import/size_calculations actions during
 *      HSU process. It could be cpss internal data  structures, shadow tables,
 *      global variables, data per feature/mechanism.
 *
 * Enumerations:
 *
 *    PRV_CPSS_CPSS_DXCH_HSU_DATA_MAIN_PHASE_LPM_DB_E          - LPM DB HSU phase
 *    PRV_CPSS_CPSS_DXCH_HSU_DATA_MAIN_PHASE_GLOBAL_E          - Global HSU phase
 */

typedef enum
{
    PRV_CPSS_CPSS_DXCH_HSU_DATA_MAIN_PHASE_LPM_DB_E,
    PRV_CPSS_CPSS_DXCH_HSU_DATA_MAIN_PHASE_GLOBAL_E
} PRV_CPSS_CPSS_DXCH_HSU_DATA_MAIN_PHASE_TYPE_ENT;


/*
 * Typedef: struct PRV_CPSS_DXCH_HSU_MAIN_FRAME_ITERATOR_STC
 *
 * Description: This struct that holds HSU global shadow iterator
 *
 * Fields:
 *      hsuAllAccumulatedSize    - amount of HSU data that was processed so far
 *      hsuAllDataSize           - all hsu data size that should be treated
 *      hsuCurrentPhase          - hsu current phase .
 */
typedef struct
{
    GT_U32                                          hsuAllAccumulatedSize;
    GT_U32                                          hsuAllDataSize;
    PRV_CPSS_CPSS_DXCH_HSU_DATA_MAIN_PHASE_TYPE_ENT hsuCurrentPhase;
}PRV_CPSS_DXCH_HSU_MAIN_FRAME_ITERATOR_STC;


/*
 * Typedef: struct PRV_CPSS_DXCH_HSU_GLOBAL_ITERATOR_STC
 *
 * Description: This struct that holds HSU global shadow iterator
 *
 * Fields:
 *      genIterator              - generic iterator
 *      currStage                - the current stage
 *      devNum                   - current device number
 *      hsuBlockGlobalSize       - size of HSU data block
 *      accumulatedGlobalSize    - amount of HSU block data processed for this moment
 *      systemDataProcessed      - GT_TRUE, if all global system data is processed
 *                                 GT_FALSE - otherwise
 *      genStageComplete         - GT_TRUE, if all generic global data is processed
 *                                 GT_FALSE - otherwise
 *      **********************************************************************************
 *      The following export/import miscellaneous variables that should be stable between
 *      iterations
 *      **********************************************************************************
 *      rxOrigDescArray          - rx descriptor array saves original descriptors pointers
 *      txOrigDescArray          - tx descriptor array saves original descriptors pointers
 *      trunksArrayPtr           - pointer to trunk array data
 *      allMembersArrayPtr       - pointer to trunks member info
 *      allMembersIsLocalArray   - are trunk members 'local' to the device array
 *      phyPortInfoArrayPtr      - array of port info structures
 *      virtualFunctionsPtr      - pointer to the trunk functions.
 *      phyInfoPtr               - pointer to the port functions
 *      portMacObjPtr            - pointer and refernce to PHYMAC MACSEC object
 *      allDescListPtr           - pointer to RX/TX/AU/FU data
 */
typedef struct
{
    PRV_CPSS_HSU_GEN_ITERATOR_STC             genIterator;
    PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT currStage;
    GT_U8                                     devNum;
    GT_U32                                    hsuBlockGlobalSize;
    GT_U32                                    accumulatedGlobalSize;
    GT_BOOL                                   systemDataProcessed;
    GT_BOOL                                   genStageComplete;
    PRV_CPSS_SW_RX_DESC_STC                   *rxOrigDescArray[NUM_OF_RX_QUEUES][3];
    PRV_CPSS_SW_TX_DESC_STC                   *txOrigDescArray[NUM_OF_TX_QUEUES][3];
    PRV_CPSS_TRUNK_ENTRY_INFO_STC             *trunksArrayPtr;
    CPSS_TRUNK_MEMBER_STC                     *allMembersArrayPtr;
    GT_BOOL                                   *allMembersIsLocalArrayPtr;
    PRV_CPSS_PORT_INFO_ARRAY_STC              *phyPortInfoArrayPtr;
    PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC       *virtualFunctionsPtr;
    PRV_CPSS_PHY_INFO_STC                     *phyInfoPtr;
    CPSS_MACDRV_OBJ_STC                       **portMacObjPtr;
    PRV_CPSS_INTERRUPT_CTRL_STC               *allDescListPtr;
}PRV_CPSS_DXCH_HSU_GLOBAL_ITERATOR_STC;


/*
 * Typedef: struct PRV_CPSS_DXCH_HSU_LPM_DB_ITERATOR_STC
 *
 * Description: This struct that holds LPM DB shadow iterator
 *
 * Fields:
 *      accumulatedLpmDbSize    - hsu LPM DBs data that was processed so far
 *      lpmDbSize               - hsu LPM DBs data size that should be treated
 *      isLpmDbProcessed        - (pointer to) LPM DBs processing status array .
 */
typedef struct
{
    GT_U32                 accumulatedLpmDbSize;
    GT_U32                 lpmDbSize;
    GT_BOOL                *isLpmDbProcessed;
}PRV_CPSS_DXCH_HSU_LPM_DB_ITERATOR_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_HSU_STAGE_STC
 *
 * Description: The iteration stage information.
 *
 * Fields:
 *      stageSize       - stage size in bytes
 *      stageAddress    - stage address
 */
typedef struct
{
    GT_U32                      stageSize;
    GT_U8*                      stageAddress;
}PRV_CPSS_DXCH_HSU_STAGE_STC;

/*******************************************************************************
* prvCpssDxChHsuLpmDbDataBlockSizeGet
*
* DESCRIPTION:
*       This function gets the memory size needed to export the LPM DB HSU
*       data block.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
* OUTPUTS:
*       sizePtr                  - the LPM DB size calculated in bytes
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuLpmDbDataBlockSizeGet
(
    OUT   GT_U32    *sizePtr
);

/*******************************************************************************
* prvCpssDxChHsuGlobalDataBlockSizeGet
*
* DESCRIPTION:
*       This function gets the memory size needed to export the Global HSU
*       data block.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
* OUTPUTS:
*       sizePtr                  - the Global HSU data block size calculated in bytes
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuGlobalDataBlockSizeGet
(
    OUT   GT_U32    *sizePtr
);



/*******************************************************************************
* prvCpssDxChHsuGlobalDataBlockExport
*
* DESCRIPTION:
*       This function exports Global HSU data block to survived restart
*       memory area supplied by application.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       iteratorPtr            - pointer to iterator, to start - set iterator to 0.
*       hsuBlockMemSizePtr     - pointer hsu block data size supposed to be exported
*                                in current iteration. The minimal value is 1k bytes.
*       hsuBlockMemPtr         - pointer to HSU survived restart memory area
*
* OUTPUTS:
*       iteratorPtr            - the iterator - points to the point from which
*                                process will be continued in future iteration.
*       hsuBlockMemSizePtr     - pointer to hsu block data size exported in current iteration.
*       exportCompletePtr      - GT_TRUE -  HSU data export is completed.
*                                GT_FALSE - HSU data export is not completed.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuGlobalDataBlockExport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *exportCompletePtr
);


/*******************************************************************************
* prvCpssDxChHsuGlobalDataBlockImport
*
* DESCRIPTION:
*       This function imports Global HSU data block from survived restart
*       memory area supplied by application.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       iteratorPtr            - pointer to iterator, to start - set iterator to 0.
*       hsuBlockMemSizePtr     - pointer hsu block data size supposed to be exported
*                                in current iteration. The minimal value is 1k bytes.
*       hsuBlockMemPtr         - pointer to HSU survived restart memory area
*
* OUTPUTS:
*       iteratorPtr            - the iterator - points to the point from which
*                                process will be continued in future iteration.
*       hsuBlockMemSizePtr     - pointer to hsu block data size imported in current iteration.
*       importCompletePtr      - GT_TRUE -  HSU data import is completed.
*                                GT_FALSE - HSU data import is not completed.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuGlobalDataBlockImport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *importCompletePtr
);

/*******************************************************************************
* prvCpssDxChHsuLpmDbDataBlockExport
*
* DESCRIPTION:
*       This function exports LPM DB HSU data block to survived restart
*       memory area supplied by application.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       iteratorPtr            - pointer to iterator, to start - set iterator to 0.
*       hsuBlockMemSizePtr     - pointer hsu block data size supposed to be exported
*                                in current iteration. The minimal value is 1k bytes.
*       hsuBlockMemPtr         - pointer to HSU survived restart memory area
*
* OUTPUTS:
*       iteratorPtr            - the iterator - points to the point from which
*                                process will be continued in future iteration.
*       hsuBlockMemSizePtr     - pointer to hsu block data size exported in current iteration.
*       exportCompletePtr      - GT_TRUE -  HSU data export is completed.
*                                GT_FALSE - HSU data export is not completed.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong hsuBlockMemSize.
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuLpmDbDataBlockExport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *exportCompletePtr
);

/*******************************************************************************
* prvCpssDxChHsuLpmDbDataBlockImport
*
* DESCRIPTION:
*       This function imports LPM DB HSU data block from survived restart
*       memory area supplied by application.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       iteratorPtr            - pointer to iterator, to start - set iterator to 0.
*       hsuBlockMemSizePtr     - pointer hsu block data size supposed to be exported
*                                in current iteration. The minimal value is 1k bytes.
*       hsuBlockMemPtr         - pointer to HSU survived restart memory area
*
* OUTPUTS:
*       iteratorPtr            - the iterator - points to the point from which
*                                process will be continued in future iteration.
*       hsuBlockMemSizePtr     - pointer to hsu block data size exported in current iteration.
*       importportCompletePtr  - GT_TRUE -  HSU data import is completed.
*                                GT_FALSE - HSU data import is not completed.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong hsuBlockMemSize.
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuLpmDbDataBlockImport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *importCompletePtr
);



/*******************************************************************************
* prvCpssDxChHsuRxTxImportPreparation
*
* DESCRIPTION:
*       This function prepares for import stage.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                 - device number
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_OUT_OF_CPU_MEM        - on out of CPU memory
*       GT_NO_RESOURCE           - on NO_RESOURCE under cpssBmPoolCreate
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuRxTxImportPreparation
(
    IN   GT_U8                                   devNum
);

/*******************************************************************************
* prvCpssDxChHsuRestoreRxTxSwDescriptorsChain
*
* DESCRIPTION:
*       This function restores RxTx Sw descriptors chain.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                - device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuRestoreRxTxSwDescriptorsChain
(
    IN     GT_U8                         devNum
);

/*******************************************************************************
* prvCpssDxChHsuExportImportDataHandling
*
* DESCRIPTION:
*       This function handle import/export data.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       actionType             - PRV_CPSS_DXCH_HSU_EXPORT_E - export action
*                                PRV_CPSS_DXCH_HSU_IMPORT_E - import action
*       currentIterPtr         - points to the current iteration.
*       hsuBlockMemSizePtr     - pointer hsu block data size supposed to be exported
*                                in current iteration.
*       hsuBlockMemPtrPtr      - pointer to HSU survived restart memory area
*
* OUTPUTS:
*       currentIterPtr         - points to the current iteration
*       hsuBlockMemSizePtr     - pointer to hsu block data size exported in current iteration.
*       hsuBlockMemPtrPtr      - pointer to HSU survived restart memory area
*       accumSizePtr           - points to accumulated size
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuExportImportDataHandling
(
    IN     PRV_CPSS_HSU_ACTION_TYPE_ENT            actionType,
    INOUT  PRV_CPSS_DXCH_HSU_GLOBAL_ITERATOR_STC *currentIterPtr,
    INOUT  GT_U32                                  *hsuBlockMemSizePtr,
    INOUT  GT_U8                                   **hsuBlockMemPtrPtr,
    OUT    GT_U32                                  *accumSizePtr
);

/*******************************************************************************
* prvCpssDxChHsuConvertStageInQueueIndex
*
* DESCRIPTION:
*       This function converts stage enum to tx queue index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       stageIndex             - import/export tx stage
*
* OUTPUTS:
*       queueIndexPtr          - pointer to tx queue index
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on bad hsu data type
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuConvertStageInQueueIndex
(
    IN   PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT stageIndex,
    OUT  GT_U32                                  *queueIndexPtr
);


/*******************************************************************************
* prvCpssDxChSystemRecoveryDisableModeHandle
*
* DESCRIPTION:
*       Handling au and fu queues,Rx SDMA to provide smooth reprogramming in new immage.
*       It is applicable to system recovery mode when application is not interested
*       in getting Rx/Au/Fu messages during system recovery process.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
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
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChSystemRecoveryDisableModeHandle
(
    GT_VOID
);

/*******************************************************************************
* prvCpssDxChHsuInitStageSizeArray
*
* DESCRIPTION:
*       This function inits stage size array for given HSU data block.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                 - device number
*       dataType               - stageArray data type
* OUTPUTS:
*       stageArrayPtr          - pointer to stage array
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on bad hsu data type
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChHsuInitStageArray
(
    IN   GT_U8                                  devNum,
    IN   PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ENT dataType,
    OUT  PRV_CPSS_DXCH_HSU_STAGE_STC            stageSizeArrayPtr[]
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHsuh */
/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChHsu.h,v $
 * Revision 1.1  2015/02/13 11:32:54  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
