/* $Id: prvCpssDxChRouterTunnelTermTcam.h,v 1.1 2015/02/13 11:32:46 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h,v $
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
* prvCpssDxChRouterTunnelTermTcam.h
*
* DESCRIPTION:
*       CPSS DxCh private router / tunnel termination TCAM declarations.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __prvCpssDxChRouterTunnelTermTcamh
#define __prvCpssDxChRouterTunnelTermTcamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* mask of 7 bits for devNum field */
#define DEV_NUM_MASK_CNS            0x7f
/* mask of 8 bits for port/trunk field */
#define PORT_TRUNK_MASK_CNS         0xff
/* mask of 7 bits for dsa port/trunk field */
#define DSA_PORT_TRUNK_MASK_CNS     0x7f

/* mask of 4 bits for local port */
#define LOCAL_PORT_MASK_CNS         0xf
/* the size of tti rule key/mask in words */
#define TTI_RULE_SIZE_CNS           6

/* check rule index to be valid */
#define PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,index)                               \
    if(index >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm)        \
        return GT_BAD_PARAM;

/* get local port from the global port value */
#define PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devNum, globalPort) (((globalPort) >> 4) & \
             PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum))

/* the size of router / tunnel termination TCAM rule    */
/* (key/mask) of a whole line in words                  */
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_LINE_RULE_SIZE_CNS        6

/* the size of router / tunnel termination TCAM action  */
/* of a whole line in words                             */
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_LINE_ACTION_SIZE_CNS      4

/* the maximum size of router / tunnel termination TCAM action  */
/* of a whole line in words. Lion2 support 8 words              */
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS  8

/* maximum value for router / tunnel termination TCAM line index */
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_MAX_LINE_INDEX_CNS             1023

/* maximum value for router / tunnel termination TCAM column index */
#define PRV_CPSS_DXCH2_ROUTER_TUNNEL_TERM_MAX_COLUMN_INDEX_CNS             4

/* maximum value for router / tunnel termination TCAM column index */
#define PRV_CPSS_DXCH3_ROUTER_TUNNEL_TERM_MAX_COLUMN_INDEX_CNS             3

/* macro to validate the value of column index field (column in the TCAM)
    columnIndex - column index
*/
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_COLUMN_INDEX_MAC(columnIndex) \
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily) == CPSS_PP_FAMILY_CHEETAH2_E)           \
        {                                                                                \
            if (columnIndex > PRV_CPSS_DXCH2_ROUTER_TUNNEL_TERM_MAX_COLUMN_INDEX_CNS)    \
            {                                                                            \
                return GT_BAD_PARAM;                                                     \
            }                                                                            \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            if (columnIndex > PRV_CPSS_DXCH3_ROUTER_TUNNEL_TERM_MAX_COLUMN_INDEX_CNS)    \
            {                                                                            \
                return GT_BAD_PARAM;                                                     \
            }                                                                            \
        }                                                                                \

/* macro to get the maximum between router table size and TTI table size */
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum)          \
    (MAX(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.router,        \
         PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm))

/* macro to validate the value of row index field (row in the TCAM)
    rowIndex - row index
*/
#define PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,rowIndex)      \
    if (rowIndex >= PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum))           \
    {                                                                                    \
        return GT_BAD_PARAM;                                                             \
    }

/* macros to convert X/Y format to pattarn/mask format and
   convert pattern/mask format to X/Y format */
#define PRV_BIT_X_Y_TO_PATTERN_MAC(_x,_y) PRV_BIT_OPER_0010_MAC(_x,_y)
#define PRV_BIT_X_Y_TO_MASK_MAC(_x,_y)    PRV_BIT_OPER_0110_MAC(_x,_y)

#define PRV_BIT_PATTERN_MASK_TO_X_MAC(_x,_y) PRV_BIT_OPER_0001_MAC(_x,_y)
#define PRV_BIT_PATTERN_MASK_TO_Y_MAC(_x,_y) PRV_BIT_OPER_0100_MAC(_x,_y)


/*******************************************************************************
* prvCpssDxChLion2RouterTcamParityCalc
*
* DESCRIPTION:
*       Function calculates parity values for Router TCAM rules.
*
* APPLICABLE DEVICES:
*        Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion3. 
*
* INPUTS:
*       dataArr         - The data array to be written.
* OUTPUTS:
*       parityPtr       - (pointer to) calculated parity
*
* RETURNS :
*       None.
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvCpssDxChLion2RouterTcamParityCalc
(
    IN  GT_U32 dataArr[8],
    OUT GT_U32 *parityPtr
);

/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamSetLine
*
* DESCRIPTION:
*       Sets a single line in the router / tunnel termination TCAM.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum              - the device number
*       portGroupId         - The port group Id. relevant only to 'multi-port-groups' devices.
*                             supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       lineIndex           - line index in TCAM to write to
*       tcamKeyPtr          - TCAM key to write - hw format
*       tcamMaskPtr         - TCAM mask to write - hw format
*       tcamActionPtr       - TCAM action (rule) to write - hw format
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_INITIALIZED       - if the driver was not initialized
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The line is set as valid.
*
*       for Cheetah3:
*       X,Y to Data + Mask translation
*       ------------------------------
*       \   Data   \   Mask   \     X    \     Y    \  Result
*       ----------------------------------------------------------------
*       \ Dont Care\     0    \     0    \     0    \ Always Hit
*       \    0     \     1    \     0    \     1    \ Match 0 (no mask)
*       \    1     \     1    \     1    \     0    \ Match 1 (no mask)
*       \ Dont Care\ Dont Care\     1    \     1    \ Always Miss
*
*       When mask is 0 Data is always reffered as '0' during translation from
*       Data & Mask to X & Y.
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamSetLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr,
    IN  GT_U32          *tcamActionPtr
);

/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamGetLine
*
* DESCRIPTION:
*       Reads a tunnel termination entry from the router / tunnel termination
*       TCAM.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum              - the device number
*       portGroupId         - The port group Id. relevant only to 'multi-port-groups' devices.
*                             supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       lineIndex           - line index in TCAM to read
*
* OUTPUTS:
*       validPtr            - points to whether TCAM entry is valid or not
*       compareModePtr      - points to compare mode of the TCAM entry:
*                               0: single array compare mode
*                               1: row compare mode
*       tcamKeyPtr          - points to TCAM key read - hw format
*       tcamMaskPtr         - points to TCAM mask read - hw format
*       tcamActionPtr       - points to TCAM action (rule) read - hw format
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_INITIALIZED       - if the driver was not initialized
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamGetLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    OUT GT_U32          *validPtr,
    OUT GT_U32          *compareModePtr,
    OUT GT_U32          *tcamKeyPtr,
    OUT GT_U32          *tcamMaskPtr,
    OUT GT_U32          *tcamActionPtr
);


/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamInvalidateLine
*
* DESCRIPTION:
*       Invalidate a single line in the router / tunnel termination TCAM.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum              - the device number
*       portGroupId         - The port group Id. relevant only to 'multi-port-groups' devices.
*                             supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       lineIndex           - line index in TCAM to write to
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_INITIALIZED       - if the driver was not initialized
*       GT_HW_ERROR              - on hardware error
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamInvalidateLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex
);

/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamInvalidateEntry
*
* DESCRIPTION:
*       Invalidate a single entry in the router / tunnel termination TCAM.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum              - the device number
*       portGroupId         - The port group Id. relevant only to 'multi-port-groups' devices.
*                             supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       lineIndex           - line index in TCAM to write to
*       columnIndex         - column index in TCAM line to write to
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamInvalidateEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex
);

/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamKeyMaskWriteLine
*
* DESCRIPTION:
*       Write the TCAM mask and key. writing  operation is preformed on all
*       192 bits in the line.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum              - the device number
*       portGroupId         - The port group Id. relevant only to 'multi-port-groups' devices.
*                             supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       lineIndex           - line index in TCAM to write to.
*       tcamKeyPtr          - TCAM key to write - hw format 192 bit.
*       tcamMaskPtr         - TCAM mask to write - hw format 192 bit.
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_INITIALIZED       - if the driver was not initialized
*       GT_HW_ERROR              - on hardware error
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamKeyMaskWriteLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
);

/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamKeyMaskWriteEntry
*
* DESCRIPTION:
*       Write the TCAM mask and key.
*       Writing  operation is preformed on one column entry (out of 6).
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum              - the device number
*       portGroupId         - The port group Id. relevant only to 'multi-port-groups' devices.
*                             supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       lineIndex           - line index in TCAM to write to.
*       columnIndex         - Column index in the TCAM line (0 to 5).
*       tcamKeyPtr          - TCAM key to write - hw format 192 bit.
*       tcamMaskPtr         - TCAM mask to write - hw format 192 bit.
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_INITIALIZED       - if the driver was not initialized
*       GT_HW_ERROR              - on hardware error
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamKeyMaskWriteEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
);


/*******************************************************************************
* prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine
*
* DESCRIPTION:
*       This function gets the TTI Rule Pattern, Mask and Action for specific
*       TCAM location fore multi-port groups devices.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                           NOTEs:
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.

*       routerTtiTcamRow  - Index of the rule in the TCAM
*
* OUTPUTS:
*       validPtr          - points to whether TCAM entry is valid or not
*       compareModePtr    - points to compare mode of the TCAM entry:
*                               0: single array compare mode
*                               1: row compare mode
*       tcamKeyPtr        - points to TTI key in hardware format
*       tcamMaskPtr       - points to TTI mask in hardware format
*       tcamActionPtr     - points to TTI action in hw format.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    OUT GT_U32                              *validPtr,
    OUT GT_U32                              *compareModePtr,
    OUT GT_U32                              *tcamKeyPtr,
    OUT GT_U32                              *tcamMaskPtr,
    OUT GT_U32                              *tcamActionPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChRouterTunnelTermTcamh */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChRouterTunnelTermTcam.h,v $
 * Revision 1.1  2015/02/13 11:32:46  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
