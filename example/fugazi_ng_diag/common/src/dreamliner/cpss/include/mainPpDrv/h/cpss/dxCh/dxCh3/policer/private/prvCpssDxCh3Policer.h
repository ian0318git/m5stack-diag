/* $Id: prvCpssDxCh3Policer.h,v 1.1 2015/02/13 11:31:26 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer/private/prvCpssDxCh3Policer.h,v $
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
* prvCpssDxCh3Policer.h
*
* DESCRIPTION:
*       Private CPSS DxCh3 Policing Engine declarations.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssDxCh3Policerh
#define __prvCpssDxCh3Policerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/* Check Policer stage number, for DxCh3 it is always 0 */
#define PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(_devNum, _stage)                                  \
if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(_devNum))                                       \
{                                                                                       \
    switch ((_stage))                                                                   \
    {                                                                                   \
        case CPSS_DXCH_POLICER_STAGE_INGRESS_0_E:                                       \
            if (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policersNum == 0)   \
            {                                                                           \
                return GT_BAD_PARAM;                                                    \
            }                                                                           \
            break;                                                                      \
        case CPSS_DXCH_POLICER_STAGE_INGRESS_1_E:                                       \
            if (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.featureInfo.iplrSecondStageSupported == GT_FALSE) \
            {                                                                           \
                return GT_BAD_PARAM;                                                    \
            }                                                                           \
            break;                                                                      \
        case CPSS_DXCH_POLICER_STAGE_EGRESS_E:                                          \
            if (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.egressPolicersNum == 0) \
            {                                                                           \
                return GT_BAD_PARAM;                                                    \
            }                                                                           \
            break;                                                                      \
        default:                                                                        \
            return GT_BAD_PARAM;                                                        \
    }                                                                                   \
}                                                                                       \
else                                                                                    \
    (_stage) = 0; /* the CH3 ignores stage, use stage 0 */


/*
 * typedef: enum PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT
 *
 * Description: Enumeration for Type of triggered action that the Policer
 *              needs to perform. It using upon accessing the metering and
 *              counting table entries for read, write and update operations.
 *
 * Enumerations:
 *  PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E  - The counter entry will be
 *                                                  read to the IPLR Table
 *                                                  Access Data Reg. Then reset
 *                                                  and written back to memory.
 *
 *  PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E       - The counter entry will be
 *                                                  read to the IPLR Table
 *                                                  Access Data Register. No
 *                                                  write back is performed.
 *
 *  PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E       - Flush the Counter
 *                                                  write-back buffer.
 *
 *  PRV_CPSS_DXCH3_POLICER_METERING_UPDATE_E      - Sets metering entry fields,
 *                                                  according to the information
 *                                                  placed in the IPLR Table
 *                                                  Access Data Register.
 *
 *  PRV_CPSS_DXCH3_POLICER_METERING_REFRESH_E     - Refresh the metering entry
 *                                                  status fields.
 *  PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E  - The counter entry will be 
 *                                                  loaded to the PLR Table 
 *                                                  Access Data Regs.
 *                                                  And content of  PLR Table Access
 *                                                  Data Regs is written to memory.
 *                                                  Supported for Lion and above 
 *                                                  devices.
 *
 */
typedef enum {
    PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E = 0,
    PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E,
    PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E,
    PRV_CPSS_DXCH3_POLICER_METERING_UPDATE_E = 4,
    PRV_CPSS_DXCH3_POLICER_METERING_REFRESH_E,
    PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E = 0x40 
} PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT;

/*******************************************************************************
* prvCpssDxCh3PolicerInternalTableAccess
*
* DESCRIPTION:
*       Triggers access operation in policer internal table.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       portGroupId     - the portGroupId. to support multi-port-groups device
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above
*                         devices and ignored by DxCh3.
*       entryOffset     - The offset from the base address of the
*                         accessed entry.
*       triggerCmd      - Triggered command that the Policer needs to perform.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_TIMEOUT                  - on time out.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong input parameters.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS prvCpssDxCh3PolicerInternalTableAccess
(
    IN GT_U8                                        devNum,
    IN  GT_U32                                      portGroupId,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN GT_U32                                       entryOffset,
    IN PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT   triggerCmd
);

/*******************************************************************************
* prvCpssDxCh3PolicerAccessControlBusyWait
*
* DESCRIPTION:
*       Do busy wait on Policer Access Control Register.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above
*                         devices and ignored by DxCh3.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success.
*       GT_TIMEOUT      - on time out.
*       GT_HW_ERROR     - on Hardware error.
*       GT_BAD_PARAM    - on wrong input parameters.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS prvCpssDxCh3PolicerAccessControlBusyWait
(
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxCh3Policerh */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxCh3Policer.h,v $
 * Revision 1.1  2015/02/13 11:31:26  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
