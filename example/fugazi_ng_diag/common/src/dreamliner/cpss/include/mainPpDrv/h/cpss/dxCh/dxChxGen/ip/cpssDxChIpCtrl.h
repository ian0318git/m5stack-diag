/* $Id: cpssDxChIpCtrl.h,v 1.1 2015/02/13 11:31:59 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h,v $
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
* cpssDxChIpCtrl.h
*
* DESCRIPTION:
*       the CPSS DXCH Ip HW control registers APIs
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssDxChIpCtrlh
#define __cpssDxChIpCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>


/*******************************************************************************
* cpssDxChIpSpecialRouterTriggerEnable
*
* DESCRIPTION:
*       Sets the special router trigger enable modes for packets with bridge
*       command other then FORWARD or MIRROR.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum              - the device number
*       bridgeExceptionCmd  - the bridge exception command the packet arrived to
*                             the router with.
*       enableRouterTrigger - enable /disable router trigger.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong devNum or bridgeExceptionCmd
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpSpecialRouterTriggerEnable
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd,
    IN GT_BOOL                         enableRouterTrigger
);

/*******************************************************************************
* cpssDxChIpSpecialRouterTriggerEnableGet
*
* DESCRIPTION:
*       Gets the special router trigger enable modes for packets with bridge
*       command other then FORWARD or MIRROR.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum              - the device number
*       bridgeExceptionCmd  - the bridge exception command the packet arrived to
*                             the router with.
*
* OUTPUTS:
*       enableRouterTriggerPtr - router trigger state (enable/disable).
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong devNum or bridgeExceptionCmd
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpSpecialRouterTriggerEnableGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd,
    OUT GT_BOOL                         *enableRouterTriggerPtr
);

/*******************************************************************************
* cpssDxChIpExceptionCommandSet
*
* DESCRIPTION:
*       set a specific exception command.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devNum        - the device number
*       exceptionType  - the exception type
*       protocolStack - whether to set for ipv4/v6 or both
*       command       - the command, for availble commands see
*                       CPSS_DXCH_IP_EXCEPTION_TYPE_ENT.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpExceptionCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN CPSS_PACKET_CMD_ENT              command
);

/*******************************************************************************
* cpssDxChIpExceptionCommandGet
*
* DESCRIPTION:
*       get a specific exception command.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devNum        - the device number
*       exceptionType  - the exception type
*       protocolStack - whether to get for ipv4/v6
*   
* OUTPUTS:
*       exceptionCmdPtr       - the command, for availble commands see
*                       CPSS_DXCH_IP_EXCEPTION_TYPE_ENT
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer
*       GT_BAD_STATE             - on invalid hardware value read
*
* COMMENTS:
*       None.
* 
*******************************************************************************/
GT_STATUS cpssDxChIpExceptionCommandGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT CPSS_PACKET_CMD_ENT             *exceptionCmdPtr
);

/*******************************************************************************
* cpssDxChIpUcRouteAgingModeSet
*
* DESCRIPTION:
*      Sets the global route aging modes.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum        - the device number
*       refreshEnable - Enables the global routing activity refresh mechanism
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpUcRouteAgingModeSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  refreshEnable
);

/*******************************************************************************
* cpssDxChIpUcRouteAgingModeGet
*
* DESCRIPTION:
*      Gets the global route aging modes.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*
* OUTPUTS:
*       refreshEnablePtr - Enables the global routing activity refresh mechanism
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpUcRouteAgingModeGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *refreshEnablePtr
);

/*******************************************************************************
* cpssDxChIpRouterSourceIdSet
*
* DESCRIPTION:
*      set the router source id assignmnet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum        - the device number
*       ucMcSet       - whether to set it for unicast packets or multicast.
*       sourceId      - the assigned source id.
*
* OUTPUTS:
*      None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_OUT_OF_RANGE          - on out of range parameter
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId
);

/*******************************************************************************
* cpssDxChIpRouterSourceIdGet
*
* DESCRIPTION:
*      get the router source id.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum        - the device number
*       ucMcSet       - whether to set it for unicast packets or multicast.
*
* OUTPUTS:
*       sourceIdPtr   - the assigned source id.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_OUT_OF_RANGE          - on out of range parameter
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr
);

/*******************************************************************************
* cpssDxChIpPortGroupRouterSourceIdSet
*
* DESCRIPTION:
*      set the router source id assignmnet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum        - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*       ucMcSet       - whether to set it for unicast packets or multicast.
*       sourceId      - the assigned source id.
*
* OUTPUTS:
*      None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_OUT_OF_RANGE          - on out of range parameter
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId
);

/*******************************************************************************
* cpssDxChIpPortGroupRouterSourceIdGet
*
* DESCRIPTION:
*      get the router source id assignmnet.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum        - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*       ucMcSet       - whether to set it for unicast packets or multicast.
*
* OUTPUTS:
*       sourceIdPtr   - the assigned source id.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_OUT_OF_RANGE          - on out of range parameter
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr
);

/*******************************************************************************
* cpssDxChIpRouterSourceIdOverrideEnableSet
*
* DESCRIPTION:
*     Enable/Disable overriding of source id by routing engine.
*
* APPLICABLE DEVICES:
*        Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion3.
*
* INPUTS:
*       devNum   - the device number
*       enable   - GT_TRUE  - enable override
*                  GT_FALSE - disable override
*
* OUTPUTS:
*      None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Lion2 device: applicable starting from revision B1
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterSourceIdOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);

/*******************************************************************************
* cpssDxChIpRouterSourceIdOverrideEnableGet
*
* DESCRIPTION:
*     Get Enable/Disable status of overriding of source id
*     by routing engine.
*
* APPLICABLE DEVICES:
*        Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion3.
*
* INPUTS:
*       devNum    - the device number
*
* OUTPUTS:
*       enablePtr - (pointer to)enable:
*                    GT_TRUE  - enable override
*                    GT_FALSE - disable override
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Lion2 device: applicable starting from revision B1
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterSourceIdOverrideEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
);

/*******************************************************************************
* cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet
*
* DESCRIPTION:
*      Sets the multi-target TC queue assigned to multi-target Control
*      packets and to RPF Fail packets where the RPF Fail Command is assigned
*      from the MLL entry.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum                    - the device number
*       ctrlMultiTargetTCQueue    - The multi-target TC queue for control
*                                   traffic. There are 4 possible queues (APPLICABLE RANGES: 0..3)
*       failRpfMultiTargetTCQueue - the multi-target TC queue for fail rpf
*                                   traffic. There are 4 possible queues (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       A packet is considered as a multi-target control if it is a FROM_CPU DSA
*       Tagged with DSA<Use_Vidx> = 1 or a multi-target packet that is also to
*       be mirrored to the CPU
*
*******************************************************************************/
GT_STATUS cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ctrlMultiTargetTCQueue,
    IN  GT_U32  failRpfMultiTargetTCQueue
);

/*******************************************************************************
* cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet
*
* DESCRIPTION:
*      Gets the multi-target TC queue assigned to multi-target Control
*      packets and to RPF Fail packets where the RPF Fail Command is assigned
*      from the MLL entry.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum                    - the device number
*
* OUTPUTS:
*       ctrlMultiTargetTCQueuePtr    - The multi-target TC queue for control
*                                      traffic. There are 4 possible queues
*       failRpfMultiTargetTCQueuePtr - the multi-target TC queue for fail rpf
*                                      traffic. There are 4 possible queues
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL Ptr
*
* COMMENTS:
*       A packet is considered as a multi-target control if it is a FROM_CPU DSA
*       Tagged with DSA<Use_Vidx> = 1 or a multi-target packet that is also to
*       be mirrored to the CPU
*
*******************************************************************************/
GT_STATUS cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet
(
    IN   GT_U8   devNum,
    OUT  GT_U32  *ctrlMultiTargetTCQueuePtr,
    OUT  GT_U32  *failRpfMultiTargetTCQueuePtr
);

/*******************************************************************************
* cpssDxChIpQosProfileToMultiTargetTCQueueMapSet
*
* DESCRIPTION:
*     Sets the Qos Profile to multi-target TC queue mapping.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum             - the device number
*       qosProfile         - QOS Profile index
*       multiTargetTCQueue - the multi-target TC queue. There are 4 possible
*                            queues.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpQosProfileToMultiTargetTCQueueMapSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    IN  GT_U32  multiTargetTCQueue
);

/*******************************************************************************
* cpssDxChIpQosProfileToMultiTargetTCQueueMapGet
*
* DESCRIPTION:
*     gets the Qos Profile to multi-target TC queue mapping.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum             - the device number
*       qosProfile         - QOS Profile index
*
* OUTPUTS:
*       multiTargetTCQueuePtr - the multi-target TC queue. There are 4 possible
*                               queues.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpQosProfileToMultiTargetTCQueueMapGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    OUT GT_U32  *multiTargetTCQueuePtr
);

/*******************************************************************************
* cpssDxChIpMultiTargetQueueFullDropCntGet
*
* DESCRIPTION:
*      Get the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum      - the device number
*
* OUTPUTS:
*       dropPktsPtr - the number of counted dropped packets.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetQueueFullDropCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *dropPktsPtr
);

/*******************************************************************************
* cpssDxChIpMultiTargetQueueFullDropCntSet
*
* DESCRIPTION:
*      set the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum      - the device number
*       dropPkts    - the counter value to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetQueueFullDropCntSet
(
    IN GT_U8     devNum,
    IN GT_U32    dropPkts
);

/*******************************************************************************
* cpssDxChIpMultiTargetTCQueueSchedModeSet
*
* DESCRIPTION:
*      sets the multi-target TC queue scheduling mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum             - the device number
*       multiTargetTcQueue - the multi-target TC queue. There are 4 possible
*                            queues.
*       schedulingMode     - the scheduling mode.
*       queueWeight        - the queue weight for SDWRR scheduler (APPLICABLE RANGES: 0..255)
*                           (relvant only if schedMode =
*                            CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetTCQueueSchedModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  multiTargetTcQueue,
    IN  CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT schedulingMode,
    IN  GT_U32                                  queueWeight
);

/*******************************************************************************
* cpssDxChIpMultiTargetTCQueueSchedModeGet
*
* DESCRIPTION:
*      gets the multi-target TC queue scheduling mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum             - the device number
*       multiTargetTcQueue - the multi-target TC queue. There are 4 possible
*                            queues.
*
* OUTPUTS:
*       schedulingModePtr     - the scheduling mode.
*       queueWeightPtr        - the queue weight for SDWRR scheduler
*                               (relvant only if schedMode =
*                               CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E).
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetTCQueueSchedModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  multiTargetTcQueue,
    OUT CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT *schedulingModePtr,
    OUT GT_U32                                  *queueWeightPtr
);

/*******************************************************************************
* cpssDxChIpBridgeServiceEnable
*
* DESCRIPTION:
*      enable/disable a router bridge service.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum            - the device number
*       bridgeService     - the router bridge service
*       enableDisableMode - Enable/Disable mode of this function (weather
*                           to enable/disable for ipv4/ipv6/arp)
*       enableService     - weather to enable the service for the above more.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpBridgeServiceEnable
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode,
    IN  GT_BOOL                                             enableService
);

/*******************************************************************************
* cpssDxChIpBridgeServiceEnableGet
*
* DESCRIPTION:
*      enable/disable a router bridge service.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum            - the device number
*       bridgeService     - the router bridge service
*       enableDisableMode - Enable/Disable mode of this function (weather
*                           to enable/disable for ipv4/ipv6/arp)
*
* OUTPUTS:
*       enableServicePtr  - weather to enable the service for the above more.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpBridgeServiceEnableGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode,
    OUT GT_BOOL                                             *enableServicePtr
);

/*******************************************************************************
* cpssDxChIpRouterBridgedPacketsExceptionCntGet
*
* DESCRIPTION:
*      Get exception counter for Special Services for Bridged Traffic.
*      it counts the number of Bridged packets failing any of the following checks:
*        - SIP Filter check for bridged IPv4/6 packets
*        - IP Header Check for bridged IPv4/6 packets
*        - Unicast RPF check for bridged IPv4/6 and ARP packets
*        - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum      - the device number
*
* OUTPUTS:
*       routerBridgedExceptionPktsPtr - the number of counted router bridged
*                                       exception  packets.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on bad  devNum
*       GT_BAD_PTR               - on illegal pointer value
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterBridgedPacketsExceptionCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *routerBridgedExceptionPktsPtr
);

/*******************************************************************************
* cpssDxChIpRouterBridgedPacketsExceptionCntSet
*
* DESCRIPTION:
*      Set exception counter for Special Services for Bridged Traffic.
*      it counts the number of Bridged packets failing any of the following checks:
*        - SIP Filter check for bridged IPv4/6 packets
*        - IP Header Check for bridged IPv4/6 packets
*        - Unicast RPF check for bridged IPv4/6 and ARP packets
*        - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum                        - the device number
*       routerBridgedExceptionPkts    - the counter value to set
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on bad  devNum
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterBridgedPacketsExceptionCntSet
(
    IN   GT_U8    devNum,
    IN  GT_U32    routerBridgedExceptionPkts
);

/*******************************************************************************
* cpssDxChIpMllBridgeEnable
*
* DESCRIPTION:
*      enable/disable MLL based bridging.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number
*       mllBridgeEnable - enable /disable MLL based bridging.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMllBridgeEnable
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    mllBridgeEnable
);

/*******************************************************************************
* cpssDxChIpMllBridgeEnableGet
*
* DESCRIPTION:
*      get state of MLL based bridging.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum             - the device number
*
* OUTPUTS:
*       mllBridgeEnablePtr - enable /disable MLL based bridging.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL prt
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMllBridgeEnableGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *mllBridgeEnablePtr
);

/*******************************************************************************
* cpssDxChIpMultiTargetRateShaperSet
*
* DESCRIPTION:
*      set the Multi target Rate shaper params.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum     - the device number
*       multiTargetRateShaperEnable - weather to enable (disable) the Multi
*                                     target Rate shaper
*       windowSize - if enabled then this is the shaper window size (APPLICABLE RANGES: 0..0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetRateShaperSet
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    multiTargetRateShaperEnable,
    IN   GT_U32     windowSize
);

/*******************************************************************************
* cpssDxChIpMultiTargetRateShaperGet
*
* DESCRIPTION:
*      set the Multi target Rate shaper params.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum     - the device number
*
* OUTPUTS:
*       multiTargetRateShaperEnablePtr - weather to enable (disable) the Multi
*                                        target Rate shaper
*       windowSizePtr - if enabled then this is the shaper window size
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetRateShaperGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *multiTargetRateShaperEnablePtr,
    OUT   GT_U32     *windowSizePtr
);

/*******************************************************************************
* cpssDxChIpMultiTargetUcSchedModeSet
*
* DESCRIPTION:
*      set the Multi target/unicast sheduler mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum    - the device number
*       ucSPEnable- weather the Unicast uses SP , if GT_FALSE then it means
*                   both the Unicast and multi target uses DSWRR scheduling
*       ucWeight  - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                   is the unicast weight (APPLICABLE RANGES: 0..255).
*       mcWeight  - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                   is the multi target weight (APPLICABLE RANGES: 0..255).
*       schedMtu  - The MTU used by the scheduler
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetUcSchedModeSet
(
    IN   GT_U8                              devNum,
    IN   GT_BOOL                            ucSPEnable,
    IN   GT_U32                             ucWeight,
    IN   GT_U32                             mcWeight,
    IN   CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   schedMtu
);

/*******************************************************************************
* cpssDxChIpMultiTargetUcSchedModeGet
*
* DESCRIPTION:
*      get the Multi target/unicast sheduler mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum    - the device number
*
* OUTPUTS:
*       ucSPEnablePtr- weather the Unicast uses SP , if GT_FALSE then it means
*                      both the Unicast and multi target uses DSWRR scheduling
*       ucWeightPtr  - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                      is the unicast weight (APPLICABLE RANGES: 0..255).
*       mcWeightPtr  - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                      is the multi target weight (APPLICABLE RANGES: 0..255).
*       schedMtuPtr  - The MTU used by the scheduler
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL ptr
*       GT_BAD_STATE             - on invalid hardware value read
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMultiTargetUcSchedModeGet
(
    IN   GT_U8                              devNum,
    OUT  GT_BOOL                            *ucSPEnablePtr,
    OUT  GT_U32                             *ucWeightPtr,
    OUT  GT_U32                             *mcWeightPtr,
    OUT  CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   *schedMtuPtr
);

/*******************************************************************************
* cpssDxChIpArpBcModeSet
*
* DESCRIPTION:
*       set a arp broadcast mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum       - the device number
*       arpBcMode    - the arp broadcast command. Possible Commands:
*                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
* GalTis:
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpArpBcModeSet
(
    IN GT_U8               devNum,
    IN CPSS_PACKET_CMD_ENT arpBcMode
);

/*******************************************************************************
* cpssDxChIpArpBcModeGet
*
* DESCRIPTION:
*       get a arp broadcast mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum       - the device number
*
* OUTPUTS:
*       arpBcModePtr - the arp broadcast command. Possible Commands:
*                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       None.
* GalTis:
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpArpBcModeGet
(
    IN  GT_U8               devNum,
    OUT CPSS_PACKET_CMD_ENT *arpBcModePtr
);

/*******************************************************************************
* cpssDxChIpPortRoutingEnable
*
* DESCRIPTION:
*       Enable multicast/unicast IPv4/v6 routing on a port.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum        - the device number
*       portNum       - the port to enable on
*       ucMcEnable    - routing type to enable Unicast/Multicast
*       protocolStack - what type of traffic to enable ipv4 or ipv6 or both.
*       enableRouting - enable IP routing for this port
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortRoutingEnable
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN GT_BOOL                          enableRouting
);

/*******************************************************************************
* cpssDxChIpPortRoutingEnableGet
*
* DESCRIPTION:
*       Get status of multicast/unicast IPv4/v6 routing on a port.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum        - the device number
*       portNum       - the port to enable on
*       ucMcEnable    - routing type to enable Unicast/Multicast
*       protocolStack - what type of traffic to enable ipv4 or ipv6 or both.
*
* OUTPUTS:
*       enableRoutingPtr - (pointer to)enable IP routing for this port
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PARAM             - wrong protocolStack
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       CPSS_IP_PROTOCOL_IPV4V6_E is not supported in this get API.
*       Can not get both values for ipv4 and ipv6 in the same get.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortRoutingEnableGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable,
    IN  CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT GT_BOOL                          *enableRoutingPtr
);

/*******************************************************************************
* cpssDxChIpQosProfileToRouteEntryMapSet
*
* DESCRIPTION:
*       Sets the QoS profile to route entry offset mapping table.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       qosProfile       - QOS Profile index
*       routeEntryOffset - The offset in the array of route entries to be
*                          selected for this QOS profile
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       In QoS-based routing, the Route table entry is selected according to the
*       following index calculation:
*       1. in DxCh2, DxCh3, xCat: <Route Entry Index> +
*                               (QoSProfile-to-Route-Block-Offset(QoSProfile) %
*                               (<Number of Paths> + 1)
*       2. in Lion : <Route Entry Index> +
*                    Floor(QoSProfile-to-Route-Block-Offset(QoSProfile) *
*                         (<Number of Paths> + 1) / 8)
*
*******************************************************************************/
GT_STATUS cpssDxChIpQosProfileToRouteEntryMapSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               qosProfile,
    IN  GT_U32               routeEntryOffset
);

/*******************************************************************************
* cpssDxChIpQosProfileToRouteEntryMapGet
*
* DESCRIPTION:
*       gets the QoS profile to route entry offset mapping table.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       qosProfile       - QOS Profile index
*
* OUTPUTS:
*       routeEntryOffsetPtr - The offset in the array of route entries to be
*                             selected for this QOS profile
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       In QoS-based routing, the Route table entry is selected according to the
*       following index calculation:
*       1. in DxCh2, DxCh3, xCat: <Route Entry Index> +
*                               (QoSProfile-to-Route-Block-Offset(QoSProfile) %
*                               (<Number of Paths> + 1)
*       2. in Lion : <Route Entry Index> +
*                    Floor(QoSProfile-to-Route-Block-Offset(QoSProfile) *
*                         (<Number of Paths> + 1) / 8)
*
*******************************************************************************/
GT_STATUS cpssDxChIpQosProfileToRouteEntryMapGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               qosProfile,
    OUT GT_U32               *routeEntryOffsetPtr
);

/*******************************************************************************
* cpssDxChIpRoutingEnable
*
* DESCRIPTION:
*       globally enable/disable routing.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum        - the device number
*       enableRouting - enable /disable global routing
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PARAM             - wrong devNum
*
* COMMENTS:
*       the ASIC defualt is routing enabled.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpRoutingEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enableRouting
);

/*******************************************************************************
* cpssDxChIpRoutingEnableGet
*
* DESCRIPTION:
*       Get global routing status
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum              - the device number
*
* OUTPUTS:
*       enableRoutingPtr    -  (pointer to)enable /disable global routing
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PARAM             - wrong devNum
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChIpRoutingEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enableRoutingPtr
);

/*******************************************************************************
* cpssDxChIpCntGet
*
* DESCRIPTION:
*       Return the IP counter set requested.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum  - the device number
*       cntSet - counters set to retrieve.
*
* OUTPUTS:
*       countersPtr - (pointer to)struct contains the counter values.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - on error
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       This function doesn't handle counters overflow.
*
*******************************************************************************/
GT_STATUS cpssDxChIpCntGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_CNT_SET_ENT          cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/*******************************************************************************
* cpssDxChIpCntSetModeSet
*
* DESCRIPTION:
*      Sets a counter set's bounded inteface and interface mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum        - the device number
*       cntSet        - the counter set
*       cntSetMode    - the counter set bind mode (interface or Next hop)
*       interfaceModeCfgPtr - if cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E
*                          this is the interface configuration
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpCntSetModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_IP_CNT_SET_ENT                        cntSet,
    IN  CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode,
    IN  CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceModeCfgPtr
);

/*******************************************************************************
* cpssDxChIpCntSetModeGet
*
* DESCRIPTION:
*      Gets a counter set's bounded inteface and interface mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum        - the device number
*       cntSet        - the counter set
*
* OUTPUTS:
*       cntSetModePtr - the counter set bind mode (interface or Next hop)
*       interfaceModeCfgPtr - if cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E
*                             this is the interface configuration
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpCntSetModeGet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_IP_CNT_SET_ENT                        cntSet,
    OUT CPSS_DXCH_IP_CNT_SET_MODE_ENT              *cntSetModePtr,
    OUT CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceModeCfgPtr
);

/*******************************************************************************
* cpssDxChIpCntSet
*
* DESCRIPTION:
*       set the requested IP counter set.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum       - the device number
*       cntSet       - counters set to clear.
*       countersPtr  - the counter values to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpCntSet
(
    IN GT_U8                         devNum,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/*******************************************************************************
* cpssDxChIpSetMllCntInterface
*
* DESCRIPTION:
*      Sets a mll counter set's bounded inteface.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number
*       mllCntSet       - the mll counter set out of the 2.
*       interfaceCfgPtr - the mll counter interface configuration
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpSetMllCntInterface
(
    IN GT_U8                                       devNum,
    IN GT_U32                                      mllCntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
);

/*******************************************************************************
* cpssDxChIpMllCntGet
*
* DESCRIPTION:
*      Get the mll counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum     - the device number.
*       mllCntSet  - the mll counter set out of the 2
*
* OUTPUTS:
*       mllOutMCPktsPtr - According to the configuration of this cnt set, The
*                      number of routed IP Multicast packets Duplicated by the
*                      MLL Engine and transmitted via this interface
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMllCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
);

/*******************************************************************************
* cpssDxChIpMllCntSet
*
* DESCRIPTION:
*      set an mll counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number
*       mllCntSet       - the mll counter set out of the 2
*       mllOutMCPkts    - the counter value to set
*
* OUTPUTS:
*      None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMllCntSet
(
    IN GT_U8    devNum,
    IN GT_U32   mllCntSet,
    IN GT_U32   mllOutMCPkts
);

/*******************************************************************************
* cpssDxChIpDropCntSet
*
* DESCRIPTION:
*      set the drop counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum      - the device number
*       dropPkts - the counter value to set
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpDropCntSet
(
    IN GT_U8 devNum,
    IN GT_U32 dropPkts
);

/*******************************************************************************
* cpssDxChIpSetDropCntMode
*
* DESCRIPTION:
*      Sets the drop counter count mode.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum      - the device number
*       dropCntMode - the drop counter count mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpSetDropCntMode
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_IP_DROP_CNT_MODE_ENT dropCntMode
);

/*******************************************************************************
* cpssDxChIpDropCntGet
*
* DESCRIPTION:
*      Get the drop counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum    - the device number
*
* OUTPUTS:
*       dropPktsPtr - the number of counted dropped packets according to the
*                     drop counter mode.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *dropPktsPtr
);

/*******************************************************************************
* cpssDxChIpMtuProfileSet
*
* DESCRIPTION:
*       Sets the global MTU profile limit values.
*
* APPLICABLE DEVICES:
*       DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number
*       mtuProfileIndex - the mtu profile index out of the possible 8 (APPLICABLE RANGES: 0..7).
*       mtu             - the maximum transmission unit (APPLICABLE RANGES: 0..0x3FFF).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMtuProfileSet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    IN GT_U32 mtu
);

/*******************************************************************************
* cpssDxChIpMtuProfileGet
*
* DESCRIPTION:
*       Gets the global MTU profile limit values.
*
* APPLICABLE DEVICES:
*       DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number
*       mtuProfileIndex - the mtu profile index out of the possible 8 (APPLICABLE RANGES: 0..7).
*
* OUTPUTS:
*       mtuPtr             - the maximum transmission unit
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpMtuProfileGet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    OUT GT_U32 *mtuPtr
);


/*******************************************************************************
* cpssDxChIpv6AddrPrefixScopeSet
*
* DESCRIPTION:
*       Defines a prefix of a scope type.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       prefix           - an IPv6 address prefix
*       prefixLen        - length of the prefix (APPLICABLE RANGES: 0..16)
*       addressScope     - type of the address scope spanned by the prefix
*       prefixScopeIndex - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_BAD_PARAM             - case that the prefix length is out of range.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Configures an entry in the prefix look up table
*
*******************************************************************************/
GT_STATUS cpssDxChIpv6AddrPrefixScopeSet
(
    IN  GT_U8                           devNum,
    IN  GT_IPV6ADDR                     prefix,
    IN  GT_U32                          prefixLen,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope,
    IN  GT_U32                          prefixScopeIndex
);

/*******************************************************************************
* cpssDxChIpv6AddrPrefixScopeGet
*
* DESCRIPTION:
*       Get a prefix of a scope type.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       prefixScopeIndex - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       prefixPtr        - an IPv6 address prefix
*       prefixLenPtr     - length of the prefix
*       addressScopePtr  - type of the address scope spanned by the prefix
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_BAD_PARAM             - case that the prefix length is out of range.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpv6AddrPrefixScopeGet
(
    IN  GT_U8                           devNum,
    OUT GT_IPV6ADDR                     *prefixPtr,
    OUT GT_U32                          *prefixLenPtr,
    OUT CPSS_IPV6_PREFIX_SCOPE_ENT      *addressScopePtr,
    IN  GT_U32                          prefixScopeIndex
);

/*******************************************************************************
* cpssDxChIpv6UcScopeCommandSet
*
* DESCRIPTION:
*       sets the ipv6 Unicast scope commands.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       addressScopeSrc  - IPv6 address scope of source address
*       addressScopeDest - IPv6 address scope of destination address
*       borderCrossed    - GT_TRUE if source site ID is diffrent to destination
*                          site ID
*       scopeCommand     - action to be done on a packet that match the above
*                          scope configuration.
*                          possible commands:
*                           CPSS_PACKET_CMD_ROUTE_E,
*                           CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                           CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                           CPSS_PACKET_CMD_DROP_SOFT_E,
*                           CPSS_PACKET_CMD_DROP_HARD_E
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error.
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_NO_RESOURCE           - if failed to allocate CPU memory
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpv6UcScopeCommandSet
(
    IN GT_U8                      devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest,
    IN GT_BOOL                    borderCrossed,
    IN CPSS_PACKET_CMD_ENT        scopeCommand
);

/*******************************************************************************
* cpssDxChIpv6UcScopeCommandGet
*
* DESCRIPTION:
*       gets the ipv6 Unicast scope commands.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       addressScopeSrc  - IPv6 address scope of source address
*       addressScopeDest - IPv6 address scope of destination address
*       borderCrossed    - GT_TRUE if source site ID is diffrent to destination
*                          site ID
* OUTPUTS:
*       scopeCommandPtr  - action to be done on a packet that match the above
*                          scope configuration.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error.
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_NO_RESOURCE           - if failed to allocate CPU memory
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpv6UcScopeCommandGet
(
    IN  GT_U8                      devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest,
    IN  GT_BOOL                    borderCrossed,
    OUT CPSS_PACKET_CMD_ENT        *scopeCommandPtr
);

/*******************************************************************************
* cpssDxChIpv6McScopeCommandSet
*
* DESCRIPTION:
*       sets the ipv6 Multicast scope commands.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       addressScopeSrc  - IPv6 address scope of source address
*       addressScopeDest - IPv6 address scope of destination address
*       borderCrossed    - GT_TRUE if source site ID is diffrent to destination
*                          site ID
*       scopeCommand     - action to be done on a packet that match the above
*                          scope configuration.
*                          possible commands:
*                           CPSS_PACKET_CMD_ROUTE_E,
*                           CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                           CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                           CPSS_PACKET_CMD_DROP_SOFT_E,
*                           CPSS_PACKET_CMD_DROP_HARD_E,
*                           CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                           CPSS_PACKET_CMD_BRIDGE_E
*       mllSelectionRule - rule for choosing MLL for IPv6 Multicast propogation
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error.
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_NO_RESOURCE           - if failed to allocate CPU memory
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpv6McScopeCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN GT_BOOL                          borderCrossed,
    IN CPSS_PACKET_CMD_ENT              scopeCommand,
    IN CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule
);

/*******************************************************************************
* cpssDxChIpv6McScopeCommandGet
*
* DESCRIPTION:
*       gets the ipv6 Multicast scope commands.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       addressScopeSrc  - IPv6 address scope of source address
*       addressScopeDest - IPv6 address scope of destination address
*       borderCrossed    - GT_TRUE if source site ID is diffrent to destination
*                          site ID
* OUTPUTS:
*       scopeCommandPtr     - action to be done on a packet that match the above
*                             scope configuration.
*                             possible commands:
*                             CPSS_PACKET_CMD_ROUTE_E,
*                             CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                             CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                             CPSS_PACKET_CMD_DROP_SOFT_E,
*                             CPSS_PACKET_CMD_DROP_HARD_E,
*                             CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                             CPSS_PACKET_CMD_BRIDGE_E
*       mllSelectionRulePtr - rule for choosing MLL for IPv6 Multicast propogation
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_FAIL                  - on error.
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_NO_RESOURCE           - if failed to allocate CPU memory
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_BAD_PTR               - on NULL ptr
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpv6McScopeCommandGet
(
    IN  GT_U8                            devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN  GT_BOOL                          borderCrossed,
    OUT CPSS_PACKET_CMD_ENT              *scopeCommandPtr,
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT *mllSelectionRulePtr
);

/*******************************************************************************
* cpssDxChIpRouterMacSaBaseSet
*
* DESCRIPTION:
*       Sets 40 MSBs of Router MAC SA Base address on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - the device number
*       macPtr - (pointer to)The system Mac address to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
);

/*******************************************************************************
* cpssDxChIpRouterMacSaBaseGet
*
* DESCRIPTION:
*       Get 40 MSBs of Router MAC SA Base address on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - the device number
*
* OUTPUTS:
*       macPtr - (pointer to)The system Mac address to set.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
);

/*******************************************************************************
* cpssDxChIpRouterMacSaLsbModeSet
*
* DESCRIPTION:
*       Sets the mode in which the device sets the packet's MAC SA least
*       significant bytes.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - the device number
*       saLsbMode       - The MAC SA least-significant bit mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum/saLsbMode.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*
*       The least significant bytes are set by cpssDxChIpRouterPortVlanMacSaLsbSet().
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterMacSaLsbModeSet
(
    IN  GT_U8                        devNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT     saLsbMode
);

/*******************************************************************************
* cpssDxChIpRouterMacSaLsbModeGet
*
* DESCRIPTION:
*       Gets the mode in which the device sets the packet's MAC SA least
*       significant bytes.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - the device number
*
* OUTPUTS:
*       saLsbModePtr    - (pointer to) The MAC SA least-significant bit mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum/saLsbMode.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterMacSaLsbModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT     *saLsbModePtr
);


/*******************************************************************************
* cpssDxChIpPortRouterMacSaLsbModeSet
*
* DESCRIPTION:
*       Sets the mode, per port, in which the device sets the packet's MAC SA
*       least significant bytes.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - the device number
*       portNum         - the port number
*       saLsbMode       - The MAC SA least-significant bit mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum/saLsbMode.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*
*       The least significant bytes are set by:
*       Port mode is set by cpssDxChIpRouterPortMacSaLsbSet().
*       Vlan mode is set by cpssDxChIpRouterVlanMacSaLsbSet().
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortRouterMacSaLsbModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_NUM                 portNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode
);


/*******************************************************************************
* cpssDxChIpPortRouterMacSaLsbModeGet
*
* DESCRIPTION:
*       Gets the mode, per port, in which the device sets the packet's MAC SA least
*       significant bytes.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - the device number
*       portNum         - the port number
*
* OUTPUTS:
*       saLsbModePtr    - (pointer to) The MAC SA least-significant bit mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum/saLsbMode.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None;
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortRouterMacSaLsbModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_NUM                 portNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT    *saLsbModePtr
);

/*******************************************************************************
* cpssDxChIpRouterPortVlanMacSaLsbSet
*
* DESCRIPTION:
*       Sets the 8 LSB Router MAC SA for this VLAN / EGGRESS PORT.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*     devNum          - the device number
*     vlanPortId      - VLAN Id or Eggress Port Id
*     saMac           - The 8 bits SA mac value to be written to the SA bits of
*                       routed packet if SA alteration mode is configured to
*                       take LSB according to VLAN id / Eggress Port Id.
*
* OUTPUTS:
*     None.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_FAIL                  - on error.
*     GT_HW_ERROR              - on hardware error
*     GT_BAD_PARAM             - wrong devNum or vlanId
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterPortVlanMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanPortId,
    IN GT_U8   saMac
);

/*******************************************************************************
* cpssDxChIpRouterPortVlanMacSaLsbGet
*
* DESCRIPTION:
*       Gets the 8 LSB Router MAC SA for this VLAN / EGGRESS PORT.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*     devNum          - the device number
*     vlanPortId      - VLAN Id or Eggress Port Id
*
* OUTPUTS:
*     saMacPtr        - (pointer to) The 8 bits SA mac value written to the SA
*                       bits of routed packet if SA alteration mode is configured
*                       to take LSB according to VLAN id / Eggress Port Id.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_FAIL                  - on error.
*     GT_HW_ERROR              - on hardware error
*     GT_BAD_PARAM             - wrong devNum or vlanId
*     GT_BAD_PTR               - one of the parameters is NULL pointer
*     GT_TIMEOUT       - after max number of retries checking if PP ready
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterPortVlanMacSaLsbGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanPortId,
    OUT GT_U8   *saMacPtr
);

/*******************************************************************************
* cpssDxChIpRouterPortMacSaLsbSet
*
* DESCRIPTION:
*       Sets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*     devNum          - the device number
*     portNum         - Eggress Port number
*     saMac           - The 8 bits SA mac value to be written to the SA bits of
*                       routed packet if SA alteration mode is configured to
*                       take LSB according to Eggress Port number.
*
* OUTPUTS:
*     None.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_FAIL                  - on error.
*     GT_HW_ERROR              - on hardware error
*     GT_BAD_PARAM             - wrong devNum
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterPortMacSaLsbSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    saMac
);

/*******************************************************************************
* cpssDxChIpRouterPortMacSaLsbGet
*
* DESCRIPTION:
*       Gets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*     devNum          - the device number
*     portNum         - Eggress Port number
*
* OUTPUTS:
*     saMacPtr        - (pointer to) The 8 bits SA mac value written to the SA
*                       bits of routed packet if SA alteration mode is configured
*                       to take LSB according to Eggress Port number.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_FAIL                  - on error.
*     GT_HW_ERROR              - on hardware error
*     GT_BAD_PARAM             - wrong devNum
*     GT_BAD_PTR               - one of the parameters is NULL pointer
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterPortMacSaLsbGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8                   *saMacPtr
);

/*******************************************************************************
* cpssDxChIpRouterVlanMacSaLsbSet
*
* DESCRIPTION:
*       Sets the 8 LSB Router MAC SA for this VLAN.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*     devNum          - the device number
*     vlan            - VLAN Id
*     saMac           - The 8 bits SA mac value to be written to the SA bits of
*                       routed packet if SA alteration mode is configured to
*                       take LSB according to VLAN.
*
* OUTPUTS:
*     None.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_FAIL                  - on error.
*     GT_HW_ERROR              - on hardware error
*     GT_BAD_PARAM             - wrong devNum or vlanId
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterVlanMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlan,
    IN GT_U8   saMac
);

/*******************************************************************************
* cpssDxChIpRouterVlanMacSaLsbGet
*
* DESCRIPTION:
*       Gets the 8 LSB Router MAC SA for this VLAN.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*     devNum          - the device number
*     vlan            - VLAN Id
*
* OUTPUTS:
*     saMacPtr        - (pointer to) The 8 bits SA mac value written to the SA
*                       bits of routed packet if SA alteration mode is configured
*                       to take LSB according to VLAN.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_FAIL                  - on error.
*     GT_HW_ERROR              - on hardware error
*     GT_BAD_PARAM             - wrong devNum or vlanId
*     GT_BAD_PTR               - one of the parameters is NULL pointer
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterVlanMacSaLsbGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlan,
    OUT GT_U8   *saMacPtr
);

/*******************************************************************************
* cpssDxChIpRouterMacSaModifyEnable
*
* DESCRIPTION:
*      Per Egress port bit Enable Routed packets MAC SA Modification
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - the device number
*       portNum  - physical or CPU port number.
*       enable   - GT_FALSE: MAC SA Modification of routed packets is disabled
*                  GT_TRUE: MAC SA Modification of routed packets is enabled
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum/portNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterMacSaModifyEnable
(
    IN  GT_U8                      devNum,
    IN  GT_PORT_NUM                portNum,
    IN  GT_BOOL                    enable
);

/*******************************************************************************
* cpssDxChIpRouterMacSaModifyEnableGet
*
* DESCRIPTION:
*      Per Egress port bit Get Routed packets MAC SA Modification State
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum        - the device number
*       portNum       - physical or CPU port number.
*
* OUTPUTS:
*       enablePtr     - GT_FALSE: MAC SA Modification of routed packets is disabled
*                  GT_TRUE: MAC SA Modification of routed packets is enabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum/portNum.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpRouterMacSaModifyEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_NUM                 portNum,
    OUT GT_BOOL                     *enablePtr
);

/*******************************************************************************
* cpssDxChIpEcmpUcRpfCheckEnableSet
*
* DESCRIPTION:
*       Globally enables/disables ECMP/QoS unicast RPF check.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* INPUTS:
*       devNum    - the device number
*       enable    - enable / disable ECMP/Qos unicast RPF check
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpEcmpUcRpfCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/*******************************************************************************
* cpssDxChIpEcmpUcRpfCheckEnableGet
*
* DESCRIPTION:
*       Gets globally enables/disables ECMP/QoS unicast RPF check state.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* INPUTS:
*       devNum    - the device number
*
* OUTPUTS:
*       enablePtr - (points to) enable / disable ECMP/Qos unicast RPF check
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS cpssDxChIpEcmpUcRpfCheckEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/*******************************************************************************
* cpssDxChIpPortGroupCntSet
*
* DESCRIPTION:
*       set the requested IP counter set.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       cntSet       - counters set to clear.
*       countersPtr  - the counter values to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupCntSet
(
    IN GT_U8                         devNum,
    IN GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/*******************************************************************************
* cpssDxChIpPortGroupCntGet
*
* DESCRIPTION:
*       Return the IP counter set requested.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       cntSet - counters set to retrieve.
*
* OUTPUTS:
*       countersPtr - (pointer to)struct contains the counter values.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PTR               - on NULL pointer
*       GT_FAIL                  - on error
*       GT_NOT_INITIALIZED       - The library was not initialized.
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       This function doesn't handle counters overflow.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupCntGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN  CPSS_IP_CNT_SET_ENT          cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/*******************************************************************************
* cpssDxChIpPortGroupDropCntSet
*
* DESCRIPTION:
*      set the drop counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       dropPkts - the counter value to set
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupDropCntSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               dropPkts
);

/*******************************************************************************
* cpssDxChIpPortGroupDropCntGet
*
* DESCRIPTION:
*      Get the drop counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       dropPktsPtr - the number of counted dropped packets according to the
*                     drop counter mode.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *dropPktsPtr
);

/*******************************************************************************
* cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet
*
* DESCRIPTION:
*      Get the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       dropPktsPtr - the number of counted dropped packets.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet
(
    IN   GT_U8                  devNum,
    IN   GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT  GT_U32                 *dropPktsPtr
);

/*******************************************************************************
* cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet
*
* DESCRIPTION:
*      set the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       dropPkts    - the counter value to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet
(
    IN GT_U8                 devNum,
    IN GT_PORT_GROUPS_BMP    portGroupsBmp,
    IN GT_U32                dropPkts
);

/*******************************************************************************
* cpssDxChIpPortGroupMllCntGet
*
* DESCRIPTION:
*      Get the mll counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number.
*       portGroupsBmp   - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       mllCntSet  - the mll counter set out of the 2
*
* OUTPUTS:
*       mllOutMCPktsPtr - According to the configuration of this cnt set, The
*                      number of routed IP Multicast packets Duplicated by the
*                      MLL Engine and transmitted via this interface
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupMllCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
);

/*******************************************************************************
* cpssDxChIpPortGroupMllCntSet
*
* DESCRIPTION:
*      set an mll counter.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum          - the device number
*       portGroupsBmp   - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       mllCntSet       - the mll counter set out of the 2
*       mllOutMCPkts    - the counter value to set
*
* OUTPUTS:
*      None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - Illegal parameter in function called
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupMllCntSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN GT_U32                   mllCntSet,
    IN GT_U32                   mllOutMCPkts
);

/*******************************************************************************
* cpssDxChIpUcRpfVlanModeSet
*
* DESCRIPTION:
*       Defines the uRPF check mode for a given VID.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum    - the device number
*       vid       - Vlan Id
*       uRpfMode  - CPSS_DXCH_IP_VLAN_URPF_DISABLE_MODE_E = uRPF check is disabled
*                        for this VID.
*                   CPSS_DXCH_IP_VLAN_URPF_VLAN_MODE_E= Vlan: If ECMP uRPF is
*                                        globally enabled, then uRPF check is done
*                                        by comparing the packet VID to the VID
*                                        in the additional route entry, otherwise
*                                        it is done using the SIP-Next Hop Entry VID.
*                   CPSS_DXCH_IP_VLAN_URPF_PORT_MODE_E = uRPF check is done by
*                        comparing the packet source {device,port}/Trunk to
*                              the SIP-Next Hop Entry {device,port}/Trunk.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device, vid or uRpfMode
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*******************************************************************************/
GT_STATUS cpssDxChIpUcRpfVlanModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    IN  CPSS_DXCH_IP_VLAN_URPF_MODE_ENT uRpfMode
);

/*******************************************************************************
* cpssDxChIpUcRpfVlanModeGet
*
* DESCRIPTION:
*       Read uRPF check mode for a given VID.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum    - the device number
*       vid       - Vlan Id
*
* OUTPUTS:
*       uRpfModePtr  -  CPSS_DXCH_IP_VLAN_URPF_DISABLE_MODE_E = uRPF check is disabled
*                           for this VID.
*                       CPSS_DXCH_IP_VLAN_URPF_VLAN_MODE_E= Vlan: If ECMP uRPF is
*                                        globally enabled, then uRPF check is done
*                                        by comparing the packet VID to the VID
*                                        in the additional route entry, otherwise
*                                        it is done using the SIP-Next Hop Entry VID.
*                       CPSS_DXCH_IP_VLAN_URPF_PORT_MODE_E = uRPF check is done by
*                           comparing the packet source {device,port}/Trunk to
*                           the SIP-Next Hop Entry {device,port}/Trunk.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*
* COMMENTS:
*       If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*******************************************************************************/
GT_STATUS cpssDxChIpUcRpfVlanModeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vid,
    OUT CPSS_DXCH_IP_VLAN_URPF_MODE_ENT     *uRpfModePtr
);

/*******************************************************************************
* cpssDxChIpPortSipSaEnableSet
*
* DESCRIPTION:
*       Enable SIP/SA check for packets received from the given port.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum    - the device number
*       portNum   - the port number (including CPU port)
*       enable    - GT_FALSE: disable SIP/SA check on the port
*                   GT_TRUE:  enable SIP/SA check on the port
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device or port
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortSipSaEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
);

/*******************************************************************************
* cpssDxChIpPortSipSaEnableGet
*
* DESCRIPTION:
*       Return the SIP/SA check status for packets received from the given port.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum    - the device number
*       portNum   - the port number (including CPU port)
*
* OUTPUTS:
*       enablePtr - GT_FALSE: SIP/SA check on the port is disabled
*                   GT_TRUE:  SIP/SA check on the port is enabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device or port
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*
* COMMENTS:
*       SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortSipSaEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
);

/*******************************************************************************
* cpssDxChIpPortGroupMultiTargetRateShaperSet
*
* DESCRIPTION:
*      set the Multi target Rate shaper params.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum     - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       multiTargetRateShaperEnable - weather to enable (disable) the Multi
*                                     target Rate shaper
*       windowSize - if enabled then this is the shaper window size (APPLICABLE RANGES: 0..0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       for multi port-group device, the multi-target rate shaping
*       shapes a traffic on per-port-group basis according to the
*       source port of the packet.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupMultiTargetRateShaperSet
(
    IN  GT_U8              devNum,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_BOOL            multiTargetRateShaperEnable,
    IN  GT_U32             windowSize
);

/*******************************************************************************
* cpssDxChIpPortGroupMultiTargetRateShaperGet
*
* DESCRIPTION:
*      get the Multi target Rate shaper params.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum     - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       multiTargetRateShaperEnablePtr - weather to enable (disable) the Multi
*                                        target Rate shaper
*       windowSizePtr - if enabled then this is the shaper window size
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       for multi port-group device, the multi-target rate shaping
*       shapes a traffic on per-port-group basis according to the
*       source port of the packet.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupMultiTargetRateShaperGet
(
    IN  GT_U8              devNum,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    OUT GT_BOOL            *multiTargetRateShaperEnablePtr,
    OUT GT_U32             *windowSizePtr
);

/*******************************************************************************
* cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet
*
* DESCRIPTION:
*      Set exception counter for Special Services for Bridged Traffic.
*      it counts the number of Bridged packets failing any of the following checks:
*        - SIP Filter check for bridged IPv4/6 packets
*        - IP Header Check for bridged IPv4/6 packets
*        - Unicast RPF check for bridged IPv4/6 and ARP packets
*        - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum                    - the device number
*       portGroupsBmp             - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       routerBridgedExceptionPkts - the counter value to set
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on bad  devNum
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               routerBridgedExceptionPkts
);

/*******************************************************************************
* cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet
*
* DESCRIPTION:
*      Get exception counter for Special Services for Bridged Traffic.
*      it counts the number of Bridged packets failing any of the following checks:
*        - SIP Filter check for bridged IPv4/6 packets
*        - IP Header Check for bridged IPv4/6 packets
*        - Unicast RPF check for bridged IPv4/6 and ARP packets
*        - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       routerBridgedExceptionPktsPtr - the number of counted router bridged
*                                       exception  packets.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on bad  devNum
*       GT_BAD_PTR               - on illegal pointer value
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *routerBridgedExceptionPktsPtr
);


/*******************************************************************************
* cpssDxChIpTcamEccDaemonEnableSet
*
* DESCRIPTION:
*       Enables or disables Router TCAM ECC Daemon.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum        - device number
*       enable        - GT_TRUE  - Router TCAM ECC Daemon enable
*                       GT_FALSE - Router TCAM ECC Daemon disable
* OUTPUTS:
*      None
*
* RETURNS :
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong parameter
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamEccDaemonEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/*******************************************************************************
* cpssDxChIpTcamEccDaemonEnableGet
*
* DESCRIPTION:
*       Gets status (Enabled/Disabled) of Router TCAM ECC Daemon.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum        - device number
*
* OUTPUTS:
*       enablePtr     - (pointer to) Router TCAM ECC Daemon status:
*                       GT_TRUE  - Router TCAM ECC Daemon enable
*                       GT_FALSE - Router TCAM ECC Daemon disable
*
* RETURNS :
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong parameter
*       GT_BAD_PTR               - null pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamEccDaemonEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpCtrlh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChIpCtrl.h,v $
 * Revision 1.1  2015/02/13 11:31:59  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
