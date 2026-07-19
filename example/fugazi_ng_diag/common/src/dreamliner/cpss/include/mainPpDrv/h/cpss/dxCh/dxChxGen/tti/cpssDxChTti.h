/* $Id: cpssDxChTti.h,v 1.1 2015/02/13 11:33:00 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti/cpssDxChTti.h,v $
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
* cpssDxChTti.h
*
* DESCRIPTION:
*       CPSS tunnel termination declarations.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __cpssDxChTtih
#define __cpssDxChTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/generic/ip/cpssIpTypes.h>

/*******************************************************************************
* cpssDxChTtiMacToMeSet
*
* DESCRIPTION:
*       function sets a TTI MacToMe entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       entryIndex        - Index of mac and vlan in MacToMe table (APPLICABLE RANGES: 0..7)
*       valuePtr          - points to Mac To Me and Vlan To Me
*       maskPtr           - points to mac and vlan's masks
*       interfaceInfoPtr  - points to source interface info (APPLICABLE DEVICES: Lion3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiMacToMeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);


/*******************************************************************************
* cpssDxChTtiMacToMeGet
*
* DESCRIPTION:
*       This function gets a TTI MacToMe entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       entryIndex        - Index of mac and vlan in MacToMe table (APPLICABLE RANGES: 0..7)
*
* OUTPUTS:
*       valuePtr          - points to Mac To Me and Vlan To Me
*       maskPtr           - points to mac and vlan's masks
*       interfaceInfoPtr  - points to source interface info (APPLICABLE DEVICES: Lion3)
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong parameter's value
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiMacToMeGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);

/*******************************************************************************
* cpssDxChTtiPortLookupEnableSet
*
* DESCRIPTION:
*       This function enables/disables the TTI lookup for the specified key
*       type at the port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       keyType       - TTI key type
*       enable        - GT_TRUE: enable TTI lookup
*                       GT_FALSE: disable TTI lookup
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortLookupEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             enable
);
/*******************************************************************************
* cpssDxChTtiPortLookupEnableGet
*
* DESCRIPTION:
*       This function gets the port's current state (enable/disable) of  the
*       TTI lookup for the specified key type.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       keyType       - TTI key type
*
* OUTPUTS:
*       enablePtr     - points to enable/disable TTI lookup
*                       GT_TRUE: TTI lookup is enabled
*                       GT_FALSE: TTI lookup is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id, port or key type
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortLookupEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChTtiPortIpv4OnlyTunneledEnableSet
*
* DESCRIPTION:
*       This function enables/disables the IPv4 TTI lookup for only tunneled
*       packets received on port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       enable        - GT_TRUE: enable IPv4 TTI lookup only for tunneled packets
*                       GT_FALSE: disable IPv4 TTI lookup only for tunneled packets
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/*******************************************************************************
* cpssDxChTtiPortIpv4OnlyTunneledEnableGet
*
* DESCRIPTION:
*       This function gets the port's current state (enable/disable) of the
*       IPv4 TTI lookup for only tunneled packets received on port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable IPv4 TTI lookup only for
*                       tunneled packets
*                       GT_TRUE: IPv4 TTI lookup only for tunneled packets is enabled
*                       GT_FALSE: IPv4 TTI lookup only for tunneled packets is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChTtiPortIpv4OnlyMacToMeEnableSet
*
* DESCRIPTION:
*       This function enables/disables the IPv4 TTI lookup for only mac to me
*       packets received on port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       enable        - GT_TRUE: enable IPv4 TTI lookup only for mac to me packets
*                       GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/*******************************************************************************
* cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
*
* DESCRIPTION:
*       This function gets the port's current state (enable/disable) of the
*       IPv4 TTI lookup for only mac to me packets received on port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable IPv4 TTI lookup only for
*                       mac to me packets
*                       GT_TRUE: IPv4 TTI lookup only for mac to me packets is enabled
*                       GT_FALSE: IPv4 TTI lookup only for mac to me packets is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChTtiIpv4McEnableSet
*
* DESCRIPTION:
*       This function enables/disables the TTI lookup for IPv4 multicast
*       (relevant only to IPv4 lookup keys).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       enable        - GT_TRUE: enable TTI lookup for IPv4 MC
*                       GT_FALSE: disable TTI lookup for IPv4 MC
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiIpv4McEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/*******************************************************************************
* cpssDxChTtiIpv4McEnableGet
*
* DESCRIPTION:
*       This function gets the current state (enable/disable) of TTI lookup for
*       IPv4 multicast (relevant only to IPv4 lookup keys).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable TTI lookup for IPv4 MC
*                       GT_TRUE: TTI lookup for IPv4 MC enabled
*                       GT_FALSE: TTI lookup for IPv4 MC disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiIpv4McEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/*******************************************************************************
* cpssDxChTtiPortMplsOnlyMacToMeEnableSet
*
* DESCRIPTION:
*       This function enables/disables the MPLS TTI lookup for only mac to me
*       packets received on port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       enable        - GT_TRUE: enable MPLS TTI lookup only for mac to me packets
*                       GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/*******************************************************************************
* cpssDxChTtiPortMplsOnlyMacToMeEnableGet
*
* DESCRIPTION:
*       This function gets the port's current state (enable/disable) of the
*       MPLS TTI lookup for only mac to me packets received on port.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable IPv4 TTI lookup only for
*                       mac to me packets
*                       GT_TRUE: MPLS TTI lookup only for mac to me packets is enabled
*                       GT_FALSE: MPLS TTI lookup only for mac to me packets is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChTtiPortMimOnlyMacToMeEnableSet
*
* DESCRIPTION:
*       This function enables/disables the MIM TTI lookup for only mac to me
*       packets received on port.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       enable        - GT_TRUE:  enable MIM TTI lookup only for mac to me packets
*                       GT_FALSE: disable MIM TTI lookup only for mac to me packets
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
);

/*******************************************************************************
* cpssDxChTtiPortMimOnlyMacToMeEnableGet
*
* DESCRIPTION:
*       This function gets the port's current state (enable/disable) of the
*       MIM TTI lookup for only mac to me packets received on port.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable MIM TTI lookup only for
*                       mac to me packets
*                       GT_TRUE:  MIM TTI lookup only for mac to me packets is enabled
*                       GT_FALSE: MIM TTI lookup only for mac to me packets is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChTtiRuleSet
*
* DESCRIPTION:
*       This function sets the TTI Rule Pattern, Mask and Action for specific
*       TCAM location according to the rule Key Type.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       routerTtiTcamRow  - Index of the tunnel termination entry in the
*                           the router / tunnel termination TCAM
*       keyType           - TTI key type
*       patternPtr        - points to the rule's pattern
*       maskPtr           - points to the rule's mask. The rule mask is "AND STYLED
*                           ONE". Mask bit's 0 means don't care bit (corresponding
*                           bit in the pattern is not used in the TCAM lookup).
*                           Mask bit's 1 means that corresponding bit in the pattern
*                           is using in the TCAM lookup.
*       actionType        - type of the action to use
*       actionPtr         - points to the TTI rule action that applied on packet
*                           if packet's search key matched with masked pattern.
*
* OUTPUTS:
*       None.
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
GT_STATUS cpssDxChTtiRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/*******************************************************************************
* cpssDxChTtiRuleGet
*
* DESCRIPTION:
*       This function gets the TTI Rule Pattern, Mask and Action for specific
*       TCAM location according to the rule Key Type.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       routerTtiTcamRow  - Index of the rule in the TCAM
*       keyType           - TTI key type
*       actionType        - type of the action to use
*
* OUTPUTS:
*       patternPtr        - points to the rule's pattern
*       maskPtr           - points to the rule's mask. The rule mask is "AND STYLED
*                           ONE". Mask bit's 0 means don't care bit (corresponding
*                           bit in the pattern is not used in the TCAM lookup).
*                           Mask bit's 1 means that corresponding bit in the pattern
*                           is using in the TCAM lookup.
*       actionPtr         - points to the TTI rule action that applied on packet
*                           if packet's search key matched with masked pattern.
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
GT_STATUS cpssDxChTtiRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/*******************************************************************************
* cpssDxChTtiRuleActionUpdate
*
* DESCRIPTION:
*       This function updates rule action.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum           - device number
*       routerTtiTcamRow - Index of the rule in the TCAM
*       actionType       - type of the action to use
*       actionPtr        - points to the TTI rule action that applied on packet
*                          if packet's search key matched with masked pattern.
*
* OUTPUTS:
*       None.
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
GT_STATUS cpssDxChTtiRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/*******************************************************************************
* cpssDxChTtiRuleValidStatusSet
*
* DESCRIPTION:
*       This function validates / invalidates the rule in TCAM.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       routerTtiTcamRow  - Index of the tunnel termination entry in the
*                           the router / tunnel termination TCAM
*       valid             - GT_TRUE - valid, GT_FALSE - invalid
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  GT_BOOL                             valid
);

/*******************************************************************************
* cpssDxChTtiRuleValidStatusGet
*
* DESCRIPTION:
*       This function returns the valid status of the rule in TCAM
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       routerTtiTcamRow  - Index of the tunnel termination entry in the
*                           the router / tunnel termination TCAM
*
* OUTPUTS:
*       validPtr          - GT_TRUE - valid, GT_FALSE - invalid
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
GT_STATUS cpssDxChTtiRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    OUT GT_BOOL                             *validPtr
);

/*******************************************************************************
* cpssDxChTtiMacModeSet
*
* DESCRIPTION:
*       This function sets the lookup Mac mode for the specified key type.
*       This setting controls the Mac that would be used for key generation
*       (Source/Destination).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       keyType       - TTI key type; valid values:
*                           CPSS_DXCH_TTI_KEY_IPV4_E
*                           CPSS_DXCH_TTI_KEY_MPLS_E
*                           CPSS_DXCH_TTI_KEY_ETH_E
*                           CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
*       macMode       - MAC mode to use; valid values:
*                           CPSS_DXCH_TTI_MAC_MODE_DA_E
*                           CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not needed for TRILL key
*
*******************************************************************************/
GT_STATUS cpssDxChTtiMacModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
);

/*******************************************************************************
* cpssDxChTtiMacModeGet
*
* DESCRIPTION:
*       This function gets the lookup Mac mode for the specified key type.
*       This setting controls the Mac that would be used for key generation
*       (Source/Destination).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       keyType       - TTI key type; valid values:
*                           CPSS_DXCH_TTI_KEY_IPV4_E
*                           CPSS_DXCH_TTI_KEY_MPLS_E
*                           CPSS_DXCH_TTI_KEY_ETH_E
*                           CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
*
* OUTPUTS:
*       macModePtr    - MAC mode to use; valid values:
*                           CPSS_DXCH_TTI_MAC_MODE_DA_E
*                           CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or key type
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not needed for TRILL key
*
*******************************************************************************/
GT_STATUS cpssDxChTtiMacModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
);

/*******************************************************************************
* cpssDxChTtiPclIdSet
*
* DESCRIPTION:
*       This function sets the PCL ID for the specified key type. The PCL ID
*       is used to distinguish between different TTI keys in the TCAM.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum        - device number
*       keyType       - TTI key type
*       pclId         - PCL ID value (10 bits)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_U32                          pclId
);

/*******************************************************************************
* cpssDxChTtiPclIdGet
*
* DESCRIPTION:
*       This function gets the PCL ID for the specified key type. The PCL ID
*       is used to distinguish between different TTI keys in the TCAM.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum        - device number
*       keyType       - TTI key type
*
* OUTPUTS:
*       pclIdPtr      - (points to) PCL ID value (10 bits)
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_U32                          *pclIdPtr
);

/*******************************************************************************
* cpssDxChTtiExceptionCmdSet
*
* DESCRIPTION:
*       Set tunnel termination exception command.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       exceptionType - tunnel termination exception type to set command for
*       command       - command to set
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       For the following exceptions:
*           CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*           CPSS_PACKET_CMD_TRAP_TO_CPU_E
*           CPSS_PACKET_CMD_DROP_HARD_E
*
*       For the following exceptions:
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       The commands are:
*           CPSS_PACKET_CMD_FORWARD_E
*           CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*           CPSS_PACKET_CMD_TRAP_TO_CPU_E
*           CPSS_PACKET_CMD_DROP_HARD_E
*           CPSS_PACKET_CMD_DROP_SOFT_E
*
*******************************************************************************/
GT_STATUS cpssDxChTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/*******************************************************************************
* cpssDxChTtiExceptionCmdGet
*
* DESCRIPTION:
*       Get tunnel termination exception command.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - physical device number
*       exceptionType - tunnel termination exception type to set command for
*
* OUTPUTS:
*       commandPtr    - points to the command for the exception type
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       For the following exceptions:
*           CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*           CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*           CPSS_PACKET_CMD_TRAP_TO_CPU_E
*           CPSS_PACKET_CMD_DROP_HARD_E
*
*       For the following exceptions:
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       The commands are:
*           CPSS_PACKET_CMD_FORWARD_E
*           CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*           CPSS_PACKET_CMD_TRAP_TO_CPU_E
*           CPSS_PACKET_CMD_DROP_HARD_E
*           CPSS_PACKET_CMD_DROP_SOFT_E
*
*******************************************************************************/
GT_STATUS cpssDxChTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/*******************************************************************************
* cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
*
* DESCRIPTION:
*       IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*           - IPv4 header <Checksum> is correct
*           - IPv4 header <Version> = 4
*           - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*           - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*           - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*           - IPv4 header <SIP> != IPv4 header <DIP>
*
*       This function globally enables/disables bypassing IPv4 header length criteria checks as part of
*       IPv4 header exception checking.
*
* APPLICABLE DEVICES:
*        xCat.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum        - device number
*       enable        - GT_TRUE:  enable bypass header length check
*                       GT_FALSE: disable bypass header length check
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*******************************************************************************/
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
);

/*******************************************************************************
* cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
*
* DESCRIPTION:
*       IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*           - IPv4 header <Checksum> is correct
*           - IPv4 header <Version> = 4
*           - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*           - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*           - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*           - IPv4 header <SIP> != IPv4 header <DIP>
*
*       This function gets the globally bypassing IPv4 header length criteria check as part of
*       IPv4 header exception checking.
*
* APPLICABLE DEVICES:
*        xCat.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum        - device number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable bypass header length check
*                       GT_TRUE:  enable bypass header length check
*                       GT_FALSE: disable bypass header length check
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*******************************************************************************/
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChTtiPortIpv4TotalLengthDeductionEnableSet
*
* DESCRIPTION:
*       For MACSEC packets over IPv4 tunnel, that are to be tunnel terminated,
*       this configuration enables aligning the IPv4 total header length to the
*       correct offset (taking into account the additional 32B MACSEC header).
*
* APPLICABLE DEVICES:
*       Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*       portNum - port number
*       enable  - Enable/disable Ipv4 Total Length Deduction.
*                 GT_TRUE - When enabled, and Global configuration<IPv4 Total Length
*                          Deduction Enable> == Enabled:
*                 <IPv4 Total Length> = Packet IPv4 header <Total Length> - Global
*                 configuration < IPv4 Total Length Deduction Value>
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - failed to write to hardware
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant for Ethernet-over-IPv4-GRE packets
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortIpv4TotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChTtiPortIpv4TotalLengthDeductionEnableGet
*
* DESCRIPTION:
*       Get if IPv4 total header length is aligned to the correct offset
*
* APPLICABLE DEVICES:
*       Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum - device number
*       portNum- port number
*
* OUTPUTS:
*       enablePtr -  Enable/disable Ipv4 Total Length Deduction.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - failed to write to hardware
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant for Ethernet-over-IPv4-GRE packets
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortIpv4TotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChTtiIpv4TotalLengthDeductionEnableSet
*
* DESCRIPTION:
*       Enable/Disable Global configuration<IPv4 Total Length Deduction Enable>
*
* APPLICABLE DEVICES:
*       Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*       enable  - Enable/disable Global Ipv4 Total Length Deduction.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - failed to write to hardware
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant for Ethernet-over-IPv4-GRE packets
*
*******************************************************************************/
GT_STATUS cpssDxChTtiIpv4TotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChTtiIpv4TotalLengthDeductionEnableGet
*
* DESCRIPTION:
*       Get Global configuration<IPv4 Total Length Deduction Enable>
*
* APPLICABLE DEVICES:
*       Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum - device number
*
* OUTPUTS:
*       enablePtr -  Enable/disable Global Ipv4 Total Length Deduction.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - failed to write to hardware
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant for Ethernet-over-IPv4-GRE packets
*
*******************************************************************************/
GT_STATUS cpssDxChTtiIpv4TotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChTtiIpv4TotalLengthDeductionValueSet
*
* DESCRIPTION:
*       Set Global configuration < IPv4 Total Length Deduction Value>
*
* APPLICABLE DEVICES:
*       Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*       value   - IPv4 Total Length Deduction Value (APPLICABLE RANGES: 0..63)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - failed to write to hardware
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant for Ethernet-over-IPv4-GRE packets
*
*******************************************************************************/
GT_STATUS cpssDxChTtiIpv4TotalLengthDeductionValueSet
(
    IN GT_U8            devNum,
    IN GT_U32           value
);

/*******************************************************************************
* cpssDxChTtiIpv4TotalLengthDeductionValueGet
*
* DESCRIPTION:
*       Get Global configuration<IPv4 Total Length Deduction Value>
*
* APPLICABLE DEVICES:
*       Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum - device number
*
* OUTPUTS:
*       valuePtr -  (pointer to) IPv4 Total Length Deduction Value
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - failed to write to hardware
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant for Ethernet-over-IPv4-GRE packets
*
*******************************************************************************/
GT_STATUS cpssDxChTtiIpv4TotalLengthDeductionValueGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *valuePtr
);

/*******************************************************************************
* cpssDxChTtiEthernetTypeSet
*
* DESCRIPTION:
*       This function sets the TTI Ethernet type value that is used
*       to identify packets for TTI triggering.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       ethertypeType - Ethernet type
*       ethertype     - Ethernet type value (range 16 bits)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   If there are 2 registers used for ethertype configuration,
*   one for ethertype identification of incoming tunneled packets in TTI
*   and one for setting the ethertype for outgoing packets in tunnel start
*   header alteration, these registers are configured to have the same value.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    IN  GT_U32                              ethertype
);

/*******************************************************************************
* cpssDxChTtiEthernetTypeGet
*
* DESCRIPTION:
*       This function gets the TTI Ethernet type value that is used
*       to identify packets for TTI triggering.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       ethertypeType - Ethernet type
*
* OUTPUTS:
*       ethertypePtr  - Points to Ethernet type value (range 16 bits)
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*
* COMMENTS:
*   If there are 2 registers used for ethertype configuration,
*   one for ethertype identification of incoming tunneled packets in TTI
*   and one for setting the ethertype for outgoing packets in tunnel start
*   header alteration, these registers are configured to have the same value.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    OUT GT_U32                              *ethertypePtr
);

/*******************************************************************************
* cpssDxChTtiTrillCpuCodeBaseSet
*
* DESCRIPTION:
*       This function sets the Trill cpu code base.
*       TRILL CPU codes are relative to global configurable CPU code base value.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum        - device number
*       cpuCodeBase   - The base CPU code value for the TRILL engine (APPLICABLE RANGES: 192..255)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_OUT_OF_RANGE          - parameter not in valid range.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
);

/*******************************************************************************
* cpssDxChTtiTrillCpuCodeBaseGet
*
* DESCRIPTION:
*       This function sets the Trill cpu code base.
*       TRILL CPU codes are relative to global configurable CPU code base value.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - device number
*
* OUTPUTS:
*       cpuCodeBasePtr - Points to the base CPU code value for the TRILL engine
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_OUT_OF_RANGE          - parameter not in valid range.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
);

/*******************************************************************************
* cpssDxChTtiTrillAdjacencyCheckEntrySet
*
* DESCRIPTION:
*       This function sets entry in the TRILL Adjacency Check dedicated TCAM.
*       A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*       The TRILL engine uses a single TCAM lookup to implement
*          the following TRILL adjacency checks:
*          1. TRILL IS-IS Adjacency check -
*             Checks that the single-destination TRILL frame arrives from a
*             {neighbor, port} for which an IS-IS adjacency exists.
*          2. TRILL Tree Adjacency Check -
*             Checks that the multi-destination TRILL frame arrives from a
*             {neighbor, port} that is a branch on the given TRILL distribution tree.
*       If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum            - device number
*       entryIndex        - entry index in TRILL Adjacency TCAM (APPLICABLE RANGES: 0..255)
*       valuePtr          - points to TRILL Adjacency STC
*       maskPtr           - points to TRILL Adjacency STC's mask
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong parameter's value
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *valuePtr,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *maskPtr
);

/*******************************************************************************
* cpssDxChTtiTrillAdjacencyCheckEntryGet
*
* DESCRIPTION:
*       This function gets entry in the TRILL Adjacency Check dedicated TCAM.
*       A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*       The TRILL engine uses a single TCAM lookup to implement
*          the following TRILL adjacency checks:
*          1. TRILL IS-IS Adjacency check -
*             Checks that the single-destination TRILL frame arrives from a
*             {neighbor, port} for which an IS-IS adjacency exists.
*          2. TRILL Tree Adjacency Check -
*             Checks that the multi-destination TRILL frame arrives from a
*             {neighbor, port} that is a branch on the given TRILL distribution tree.
*       If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum            - device number
*       entryIndex        - entry index in TRILL Adjacency TCAM (APPLICABLE RANGES: 0..255)
*
* OUTPUTS:
*       valuePtr          - points to TRILL Adjacency STC
*       maskPtr           - points to TRILL Adjacency STC's mask
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong parameter's value
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *valuePtr,
    OUT CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *maskPtr
);

/*******************************************************************************
* cpssDxChTtiTrillMaxVersionSet
*
* DESCRIPTION:
*       This function sets the max Trill version.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum            - device number
*       maxTrillVersion   - max TRILL version value (range 2 bits)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillMaxVersionSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          maxTrillVersion
);

/*******************************************************************************
* cpssDxChTtiTrillMaxVersionGet
*
* DESCRIPTION:
*       This function gets the max Trill version.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum               - device number
*
* OUTPUTS:
*       maxTrillVersionPtr   - Points to max TRILL version value (range 2 bits)
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
);

/*******************************************************************************
* cpssDxChTtiTrillRbidLttEntrySet
*
* DESCRIPTION:
*    Sets a LookUp Translation Table Entry.
*    The RBID LTT Table has 64K entries which maps to 4K RBID table pointer.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum                - the device number.
*       rbid                  - the entry's index in RBID LTT table (16 bits)
*       rbidLttEntryPtr       - the pointer into RBID table
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - if succeeded
*       GT_BAD_PARAM    - on bad parameter's value
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillRbidLttEntrySet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          rbid,
    IN  CPSS_DXCH_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC   *rbidLttEntryPtr
);


/*******************************************************************************
* cpssDxChTtiTrillRbidLttEntryGet
*
* DESCRIPTION:
*    Gets a LookUp Translation Table Entry.
*    The RBID LTT Table has 64K entries which remaps to 4K RBID table pointer.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum            - the device number.
*       rbid              - the entry's index in RBID LTT table (16 bits)
*
* OUTPUTS:
*       rbidLttEntryPtr - the pointer into RBID table
*
* RETURNS:
*       GT_OK           - if succeeded
*       GT_BAD_PARAM    - on bad parameter's value
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillRbidLttEntryGet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         rbid,
    OUT CPSS_DXCH_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC   *rbidLttEntryPtr
);

/*******************************************************************************
* cpssDxChTtiTrillRbidEntrySet
*
* DESCRIPTION:
*   Sets an entry in the RBID Table.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number.
*       rbidEntryIndex  - the entry's index in RBID table (APPLICABLE RANGES: 0..4095)
*       rbidEntryPtr    - pointer to the value written into RBID table
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - if succeeded
*       GT_BAD_PARAM    - on bad parameter's value
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillRbidEntrySet
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    rbidEntryIndex,
    IN  CPSS_DXCH_TTI_TRILL_RBID_TABLE_ENTRY_STC  *rbidEntryPtr
);

/*******************************************************************************
* cpssDxChTtiTrillRbidEntryGet
*
* DESCRIPTION:
*   Gets an entry in the RBID Table.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number.
*       rbidEntryIndex  - the entry's index in RBID table (APPLICABLE RANGES: 0..4095)
*
* OUTPUTS:
*       rbidEntryPtr    - pointer to the value written into RBID table
*
* RETURNS:
*       GT_OK           - if succeeded
*       GT_BAD_PARAM    - on bad parameter's value
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillRbidEntryGet
(
    IN  GT_U8                                     devNum,
    IN  GT_32                                     rbidEntryIndex,
    OUT CPSS_DXCH_TTI_TRILL_RBID_TABLE_ENTRY_STC  *rbidEntryPtr
);

/*******************************************************************************
* cpssDxChTtiTrillDropCounterSet
*
* DESCRIPTION:
*      Sets the TRILL drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - the device number
*       exceptionType - tunnel termination exception type to set counter for; valid options:
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       counter - value to set counter.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillDropCounterSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  GT_U32                              counter
);

/*******************************************************************************
* cpssDxChTtiTrillDropCounterGet
*
* DESCRIPTION:
*      Gets the TRILL drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - the device number
*       exceptionType - tunnel termination exception type to get counter for; valid options:
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*
* OUTPUTS:
*       counterPtr - (pointer to) counter value.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiTrillDropCounterGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT GT_U32                              *counterPtr
);

/*******************************************************************************
* cpssDxChTtiPortTrillEnableSet
*
* DESCRIPTION:
*       This function enables/disables the TRILL engine on the port.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*       enable        - GT_TRUE: enable TRILL engine on port
*                       GT_FALSE: disable TRILL engine on port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Configure TRILL Interface Enable bit for Egress ePort in table “HA Egress ePort Attribute Table 1”
*       Configure TRILL Engine Enable bit for Ingress ePort in table “Pre-TTI Lookup Ingress ePort Table”
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortTrillEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/*******************************************************************************
* cpssDxChTtiPortTrillEnableGet
*
* DESCRIPTION:
*       This function gets the current state enables/disables of TRILL engine
*       on the port.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*
* OUTPUTS:
*       enablePtr     - points to enable/disable TRILL
*                       GT_TRUE:  TRILL engine enabled on port
*                       GT_FALSE: TRILL engine disabled on port
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Same value is configured for Egress ePort in table “HA Egress ePort Attribute Table 1”
*       and Ingress ePort in table “Pre-TTI Lookup Ingress ePort Table”
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortTrillEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
);

/*******************************************************************************
* cpssDxChTtiPortTrillOuterVid0Set
*
* DESCRIPTION:
*       This function sets the Outer Tag0 VID that must be for all TRILL packets
*       from the port.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum    - device number
*       portNum   - port number
*       vlanId    - TRILL Outer Tag0 VID (APPLICABLE RANGES: 0..4095)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - parameter not in valid range.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortTrillOuterVid0Set
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
);

/*******************************************************************************
* cpssDxChTtiPortTrillOuterVid0Get
*
* DESCRIPTION:
*       This function gets the Outer Tag0 VID that must be for all TRILL packets
*       from the port.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum    - device number
*       portNum   - port number
*
* OUTPUTS:
*       vlanIdPtr - (pointer to) TRILL Outer Tag0 VID
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - parameter not in valid range.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vlanIdPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupMacToMeSet
*
* DESCRIPTION:
*       This function sets a TTI MacToMe entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       entryIndex        - Index of mac and vlan in MacToMe table (APPLICABLE RANGES: 0..7)
*       valuePtr          - points to Mac To Me and Vlan To Me
*       maskPtr           - points to mac and vlan's masks
*       interfaceInfoPtr  - points to source interface info (APPLICABLE DEVICES: Lion3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupMacToMeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_U32                                          entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupMacToMeGet
*
* DESCRIPTION:
*       This function gets a TTI MacToMe entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       entryIndex        - Index of mac and vlan in MacToMe table (APPLICABLE RANGES: 0..7)
*
* OUTPUTS:
*       valuePtr          - points to Mac To Me and Vlan To Me
*       maskPtr           - points to mac and vlan's masks
*       interfaceInfoPtr  - points to source interface info (APPLICABLE DEVICES: Lion3)
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupMacToMeGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_U32                                          entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupMacModeSet
*
* DESCRIPTION:
*       This function sets the lookup Mac mode for the specified key type.
*       This setting controls the Mac that would be used for key generation
*       (Source/Destination).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portGroupsBmp - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       keyType       - TTI key type; valid values:
*                           CPSS_DXCH_TTI_KEY_IPV4_E
*                           CPSS_DXCH_TTI_KEY_MPLS_E
*                           CPSS_DXCH_TTI_KEY_ETH_E
*                           CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
*       macMode       - MAC mode to use; valid values:
*                           CPSS_DXCH_TTI_MAC_MODE_DA_E
*                           CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not needed for TRILL key
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupMacModeSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
);

/*******************************************************************************
* cpssDxChTtiPortGroupMacModeGet
*
* DESCRIPTION:
*       This function gets the lookup Mac mode for the specified key type.
*       This setting controls the Mac that would be used for key generation
*       (Source/Destination).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum        - device number
*       portGroupsBmp - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       keyType       - TTI key type; valid values:
*                           CPSS_DXCH_TTI_KEY_IPV4_E
*                           CPSS_DXCH_TTI_KEY_MPLS_E
*                           CPSS_DXCH_TTI_KEY_ETH_E
*                           CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
*
* OUTPUTS:
*       macModePtr    - MAC mode to use; valid values:
*                           CPSS_DXCH_TTI_MAC_MODE_DA_E
*                           CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong device id or key type
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not needed for TRILL key
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupMacModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupRuleSet
*
* DESCRIPTION:
*       This function sets the TTI Rule Pattern, Mask and Action for specific
*       TCAM location according to the rule Key Type.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       routerTtiTcamRow  - Index of the tunnel termination entry in the
*                           the router / tunnel termination TCAM
*       keyType           - TTI key type
*       patternPtr        - points to the rule's pattern
*       maskPtr           - points to the rule's mask. The rule mask is "AND STYLED
*                           ONE". Mask bit's 0 means don't care bit (corresponding
*                           bit in the pattern is not used in the TCAM lookup).
*                           Mask bit's 1 means that corresponding bit in the pattern
*                           is using in the TCAM lookup.
*       actionType        - type of the action to use
*       actionPtr         - points to the TTI rule action that applied on packet
*                           if packet's search key matched with masked pattern.
*
* OUTPUTS:
*       None.
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
GT_STATUS cpssDxChTtiPortGroupRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupRuleGet
*
* DESCRIPTION:
*       This function gets the TTI Rule Pattern, Mask and Action for specific
*       TCAM location according to the rule Key Type.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       routerTtiTcamRow  - Index of the rule in the TCAM
*       keyType           - TTI key type
*       actionType        - type of the action to use
*
* OUTPUTS:
*       patternPtr        - points to the rule's pattern
*       maskPtr           - points to the rule's mask. The rule mask is "AND STYLED
*                           ONE". Mask bit's 0 means don't care bit (corresponding
*                           bit in the pattern is not used in the TCAM lookup).
*                           Mask bit's 1 means that corresponding bit in the pattern
*                           is using in the TCAM lookup.
*       actionPtr         - points to the TTI rule action that applied on packet
*                           if packet's search key matched with masked pattern.
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
GT_STATUS cpssDxChTtiPortGroupRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupRuleActionUpdate
*
* DESCRIPTION:
*       This function updates rule action.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum           - device number
*       portGroupsBmp    - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       routerTtiTcamRow - Index of the rule in the TCAM
*       actionType       - type of the action to use
*       actionPtr        - points to the TTI rule action that applied on packet
*                          if packet's search key matched with masked pattern.
*
* OUTPUTS:
*       None.
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
GT_STATUS cpssDxChTtiPortGroupRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupRuleValidStatusSet
*
* DESCRIPTION:
*       This function validates / invalidates the rule in TCAM.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       routerTtiTcamRow  - Index of the tunnel termination entry in the
*                           the router / tunnel termination TCAM
*       valid             - GT_TRUE - valid, GT_FALSE - invalid
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong value in any of the parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    IN  GT_BOOL                             valid
);

/*******************************************************************************
* cpssDxChTtiPortGroupRuleValidStatusGet
*
* DESCRIPTION:
*       This function returns the valid status of the rule in TCAM
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum            - device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       routerTtiTcamRow  - Index of the tunnel termination entry in the
*                           the router / tunnel termination TCAM
*
* OUTPUTS:
*       validPtr          - GT_TRUE - valid, GT_FALSE - invalid
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
GT_STATUS cpssDxChTtiPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    OUT GT_BOOL                             *validPtr
);

/*******************************************************************************
* cpssDxChTtiPortGroupTrillDropCounterSet
*
* DESCRIPTION:
*      Sets the TRILL drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum - the device number
*       portGroupsBmp - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       exceptionType - tunnel termination exception type to set counter for; valid options:
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       counter - value to set counter.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupTrillDropCounterSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT  exceptionType,
    IN  GT_U32                       counter
);

/*******************************************************************************
* cpssDxChTtiPortGroupTrillDropCounterGet
*
* DESCRIPTION:
*      Gets the TRILL drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum - the device number
*       portGroupsBmp - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       exceptionType - tunnel termination exception type to get counter for; valid options:
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*           CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*
* OUTPUTS:
*       counterPtr - (pointer to) counter value.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChTtiPortGroupTrillDropCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT     exceptionType,
    OUT GT_U32                          *counterPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTtih */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChTti.h,v $
 * Revision 1.1  2015/02/13 11:33:00  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
