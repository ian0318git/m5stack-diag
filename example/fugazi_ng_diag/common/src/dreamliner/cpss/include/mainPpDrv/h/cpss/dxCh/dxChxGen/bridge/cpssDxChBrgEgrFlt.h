/* $Id: cpssDxChBrgEgrFlt.h,v 1.1 2015/02/13 11:31:30 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h,v $
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
* cpssDxChBrgEgrFlt.h
*
* DESCRIPTION:
*       Egress filtering facility DxCh cpss implementation
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDxChBrgEgrFlth
#define __cpssDxChBrgEgrFlth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

/*******************************************************************************
* cpssDxChBrgPortEgrFltUnkEnable
*
* DESCRIPTION:
*      Enable/Disable egress Filtering for bridged Unknown Unicast packets
*      on the specified egress port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev      - device number
*       portNum  - CPU port, port number
*       enable   - GT_TRUE: Unknown Unicast packets are filtered and are
*                           not forwarded to this port.
*                  GT_FALSE: Unknown Unicast packets are not filtered and may
*                           be forwarded to this port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltUnkEnable
(
    IN GT_U8            dev,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChBrgPortEgrFltUnkEnableGet
*
* DESCRIPTION:
*      This function gets the egress Filtering current state (enable/disable)
*      for bridged Unknown Unicast packets on the specified egress port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev         - device number
*       portNum     - CPU port, port number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) bridged unknown unicast packets filtering
*                   GT_TRUE:  Unknown Unicast packets are filtered and are
*                             not forwarded to this port.
*                   GT_FALSE: Unknown Unicast packets are not filtered and may
*                             be forwarded to this port.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltUnkEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChBrgPortEgrFltUregMcastEnable
*
* DESCRIPTION:
*      Enable/Disable egress Filtering for bridged Unregistered Multicast packets
*      on the specified egress port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev      - device number
*       portNum  - CPU port, port number
*       enable   - GT_TRUE: Unregistered Multicast packets are filtered and
*                           are not forwarded to this port.
*                  GT_FALSE:Unregistered Multicast packets are not filtered
*                           and may be forwarded to this port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltUregMcastEnable
(
    IN GT_U8            dev,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChBrgPortEgrFltUregMcastEnableGet
*
* DESCRIPTION:
*      This function gets the egress Filtering current state (enable/disable)
*      for bridged Unregistered Multicast packets on the specified egress port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev         - device number
*       portNum     - CPU port, port number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) bridged unregistered multicast packets filtering
*                   GT_TRUE:  Unregistered Multicast packets are filtered and
*                             are not forwarded to this port.
*                   GT_FALSE: Unregistered Multicast packets are not filtered
*                             and may be forwarded to this port.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltUregMcastEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChBrgVlanEgressFilteringEnable
*
* DESCRIPTION:
*       Enable/Disable VLAN Egress Filtering on specified device for Bridged
*       Known Unicast packets.
*       If enabled the VLAN egress filter verifies that the egress port is a
*       member of the VID assigned to the packet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev      - device number
*       enable   - GT_TRUE: VLAN egress filtering verifies that the egress
*                          port is a member of the packet's VLAN classification
*                 GT_FLASE: the VLAN egress filtering check is disabled.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong dev
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       VLAN egress filtering is required by 802.1Q, but if desired, this
*       mechanism can be disabled, thus allowing "leaky VLANs".
*
*******************************************************************************/
GT_STATUS cpssDxChBrgVlanEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChBrgVlanEgressFilteringEnableGet
*
* DESCRIPTION:
*      This function gets the VLAN Egress Filtering current state (enable/disable)
*      on specified device for Bridged Known Unicast packets.
*      If enabled the VLAN egress filter verifies that the egress port is a
*      member of the VID assigned to the packet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev         - device number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) bridged known unicast packets filtering
*                   GT_TRUE:   VLAN egress filtering verifies that the egress
*                              port is a member of the packet's VLAN classification
*                   GT_FLASE: the VLAN egress filtering check is disabled.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgVlanEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
);

/*******************************************************************************
* cpssDxChBrgRoutedUnicastEgressFilteringEnable
*
* DESCRIPTION:
*       Enable/Disable VLAN egress filtering on Routed Unicast packets.
*       If disabled, the destination port may or may not be a member of the VLAN.
*       If enabled, the VLAN egress filter verifies that the egress port is a
*       member of the VID assigned to the packet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev      - device number
*       enable   - GT_TRUE: Egress filtering is enabled
*                  GT_FLASE: Egress filtering is disabled
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong dev
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChBrgRoutedUnicastEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);


/*******************************************************************************
* cpssDxChBrgRoutedUnicastEgressFilteringEnableGet
*
* DESCRIPTION:
*      This function gets the VLAN Egress Filtering current state (enable/disable)
*      on Routed Unicast packets.
*      If disabled, the destination port may or may not be a member of the VLAN.
*      If enabled, the VLAN egress filter verifies that the egress port is a
*      member of the VID assigned to the packet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev         - device number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) routed unicast packets filtering
*                   GT_TRUE:  Egress filtering is enabled
*                   GT_FLASE: Egress filtering is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgRoutedUnicastEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
);

/*******************************************************************************
* cpssDxChBrgRoutedSpanEgressFilteringEnable
*
* DESCRIPTION:
*       Enable/Disable STP egress Filtering on Routed Packets.
*       If disabled the packet may be forwarded to its egress port,
*       regardless of its Span State.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev      - device number
*       enable   - GT_TRUE: Span state filtering is enabled
*                  GT_FLASE: Span state filtering is disabled
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong dev
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChBrgRoutedSpanEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChBrgRoutedSpanEgressFilteringEnableGet
*
* DESCRIPTION:
*      This function gets the STP egress Filtering current state (enable/disable)
*      on Routed packets.
*      If disabled the packet may be forwarded to its egress port,
*      regardless of its Span State.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev         - device number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) routed packets filtering
*                   GT_TRUE:  Span state filtering is enabled
*                   GT_FLASE: Span state filtering is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgRoutedSpanEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
);


/*******************************************************************************
* cpssDxChBrgPortEgrFltUregBcEnable
*
* DESCRIPTION:
*       Enables or disables egress filtering of unregistered broadcast packets.
*       Unregistered broadcast packets are:
*        - packets with destination MAC ff-ff-ff-ff-ff-ff
*        - destination MAC address lookup not finds matching entry
*        - packets were not routed
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       dev     - device number
*       portNum - port number
*       enable  - GT_TRUE  - enable filtering of unregistered broadcast packets.
*                            unregistered broadcast packets are dropped
*                 GT_FALSE - disable filtering of unregistered broadcast packets.
*                            unregistered broadcast packets are not dropped
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong dev
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltUregBcEnable
(
    IN GT_U8            dev,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChBrgPortEgrFltUregBcEnableGet
*
* DESCRIPTION:
*      This function gets the egress Filtering current state (enable/disable)
*      of unregistered broadcast packets.
*      Unregistered broadcast packets are:
*      - packets with destination MAC ff-ff-ff-ff-ff-ff
*      - destination MAC address lookup not finds matching entry
*      - packets were not routed
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       dev         - device number
*       portNum     - port number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) unregistered broadcast packets filtering
*                   GT_TRUE  - enable filtering of unregistered broadcast packets.
*                              unregistered broadcast packets are dropped
*                   GT_FALSE - disable filtering of unregistered broadcast packets.
*                              unregistered broadcast packets are not dropped
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltUregBcEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChBrgPortEgressMcastLocalEnable
*
* DESCRIPTION:
*       Enable/Disable sending Multicast packets back to its source
*       port on the local device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev     - device number
*       portNum - port number
*       enable  - Boolean value:
*                 GT_TRUE  - Multicast packets may be sent back to
*                            their source port on the local device.
*                 GT_FALSE - Multicast packets are not sent back to
*                            their source port on the local device.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong dev
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       For xCat and above devices to enable local switching of Multicast,
*       unknown Unicast, and Broadcast traffic, both egress port configuration
*       and the field in the VLAN entry (by function
*       cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgressMcastLocalEnable
(
    IN GT_U8            dev,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChBrgPortEgressMcastLocalEnableGet
*
* DESCRIPTION:
*      This function gets current state (enable/disable) for sending
*      Multicast packets back to its source port on the local device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev         - device number
*       portNum     - port number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) sending Multicast packets back to its source
*                   GT_TRUE  - Multicast packets may be sent back to
*                              their source port on the local device.
*                   GT_FALSE - Multicast packets are not sent back to
*                              their source port on the local device.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgressMcastLocalEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* cpssDxChBrgPortEgrFltIpMcRoutedEnable
*
* DESCRIPTION:
*      Enable/Disable egress filtering for IP Multicast Routed packets
*      on the specified egress port.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       dev      - device number
*       portNum  - port number
*       enable   - GT_TRUE: IP Multicast Routed packets are filtered and are
*                           not forwarded to this port.
*                  GT_FALSE: IP Multicast Routed packets are not filtered and may
*                           be forwarded to this port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltIpMcRoutedEnable
(
    IN GT_U8            dev,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/*******************************************************************************
* cpssDxChBrgPortEgrFltIpMcRoutedEnableGet
*
* DESCRIPTION:
*      This function gets the egress Filtering current state (enable/disable)
*      for IP Multicast Routed packets on the specified egress port.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       dev             - device number
*       portNum         - port number
*
* OUTPUTS:
*       enablePtr   - points to (enable/disable) IP Multicast Routed packets filtering
*                   GT_TRUE:  IP Multicast Routed packets are filtered and are
*                             not forwarded to this port.
*                   GT_FALSE: IP Multicast Routed packets are not filtered and may
*                             be forwarded to this port.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong dev or port
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*         None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgPortEgrFltIpMcRoutedEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);


/*******************************************************************************
* cpssDxChBrgEgrFltBypassMcSourceFilteringWhenEportAssignedSet
*
* DESCRIPTION:
*       Determines if MC source filtering is performed on this packet incase
*       the SRCePort was assigned for the current packet by TTI/Rbridge engine.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum - device number
*       enable - GT_TRUE: if the SRCePort was assigned for the current packet,
*                         MC source filtering is NOT performed on this packet.
*                GT_FALSE: if the SRCePort was assigned for the current packet,
*                         MC source filtering is performed on this packet.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device or port.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChBrgEgrFltBypassMcSourceFilteringWhenEportAssignedSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChBrgEgrFltBypassMcSourceFilteringWhenEportAssignedGet
*
* DESCRIPTION:
*       Returns if MC source filtering is performed on this packet incase
*       the SRCePort was assigned for the current packet by TTI/Rbridge engine.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - device number
*
* OUTPUTS:
*       enablePtr - (pointer to)
*                GT_TRUE: if the SRCePort was assigned for the current packet,
*                         MC source filtering is NOT performed on this packet.
*                GT_FALSE: if the SRCePort was assigned for the current packet,
*                         MC source filtering is performed on this packet.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong device or port.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChBrgEgrFltBypassMcSourceFilteringWhenEportAssignedGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgEgrFlth */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChBrgEgrFlt.h,v $
 * Revision 1.1  2015/02/13 11:31:30  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
