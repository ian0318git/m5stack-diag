/* $Id: prvCpssDxChDiagDataIntegrityMainMappingDb.h,v 1.1 2015/02/13 11:31:55 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h,v $
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
* prvCpssDxChDiagDataIntegrityMainMappingDb.h
*
* DESCRIPTION:
*       Internal header with DFX Data Integrity module main mapping batabase.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChDiagataIntegrityMainMappingDbh
#define __prvCpssDxChDiagataIntegrityMainMappingDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Lion2 max port group number */
#define LION2_MAX_PORT_GROUP_NUM_CNS                    8
/* Lion2 max number of DFX pipes */
#define LION2_MAX_NUMBER_OF_PIPES_CNS                   8
/* Maximal DFX cause index  */
#define MAX_DFX_INT_CAUSE_NUM_CNS                       27
/* indicator of unused DB entry */
#define DATA_INTEGRITY_ENTRY_NOT_USED_CNS               0xCAFECAFE  
/* Lion2 Hooper max port group number */
#define HOOPER_MAX_PORT_GROUP_NUM_CNS                   4



/*
 * Typedef: struct PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_MAP_DB_STC
 *
 * Description: Bitmap of memories in DFX Client
 *
 * Fields:
 *      key     - DB key - consist of DFX pipe number, Client Number and Memory number
 *      memType - memory type 
 *      protectionType - memory protection type
 *      externalProtectionType - memory external protection type (non-DFX mechanism)
 *      causePortGroupId - port group ID of event has happened
 *      firstTableDataBit - first table data bit
 *      lastTableDataBit - last table data bit
 *
 * Comments:
 *      None
 */
typedef struct
{
    GT_U32                                                  key;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT              memType;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  externalProtectionType;
    GT_U32                                                  causePortGroupId;
    GT_U32                                                  firstTableDataBit;
    GT_U32                                                  lastTableDataBit;
}PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_MAP_DB_STC;

extern PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_MAP_DB_STC lion2DataIntegrityDbArray[];
extern GT_U32 lion2DataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_MAP_DB_STC hooperDataIntegrityDbArray[];
extern GT_U32 hooperDataIntegrityDbArrayEntryNum;

/*
 * Typedef: struct PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FIRST_STAGE_MAPPING_DATA_STC
 *
 * Description: First stage map entry.
 *
 * Fields:
 *      dfxPipeIndex     - DFX pipe index
 *      dfxClientIndex   - DXF client index
 *
 * Comments:
 *      None
 */
typedef struct
{
    GT_U32 dfxPipeIndex;
    GT_U32 dfxClientIndex;
}PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FIRST_STAGE_MAPPING_DATA_STC;

extern PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FIRST_STAGE_MAPPING_DATA_STC lion2DataIntegrityFirstStageMappingArray[LION2_MAX_PORT_GROUP_NUM_CNS][MAX_DFX_INT_CAUSE_NUM_CNS];
extern PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FIRST_STAGE_MAPPING_DATA_STC hooperDataIntegrityFirstStageMappingArray[HOOPER_MAX_PORT_GROUP_NUM_CNS][MAX_DFX_INT_CAUSE_NUM_CNS];

extern GT_STATUS prvCpssDxChDiagDataIntegrityMemoryIndexesGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT              memType,
    IN  GT_BOOL                                                 isPerPortGroup,
    IN  GT_U32                                                  portGroupId,
    INOUT GT_U32                                                *arraySizePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC            *memLocationArr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
);

extern GT_STATUS prvCpssDfxMemoryRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    OUT GT_U32 *regDataPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChDiagataIntegrityMainMappingDbh */
    

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChDiagDataIntegrityMainMappingDb.h,v $
 * Revision 1.1  2015/02/13 11:31:55  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
