/* $Id: cpssDxChBrgMc.h,v 1.1 2015/02/13 11:31:30 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h,v $
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
* cpssDxChBrgMc.h
*
* DESCRIPTION:
*       CPSS DXCh Multicast Group facility API.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/
#ifndef __cpssDxChBrgMch
#define __cpssDxChBrgMch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

/*******************************************************************************
* cpssDxChBrgMcInit
*
* DESCRIPTION:
*       Initializes Bridge engine Multicast Library.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcInit
(
    IN GT_U8    devNum
);

/*******************************************************************************
* cpssDxChBrgMcIpv6BytesSelectSet
*
* DESCRIPTION:
*
*       Sets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*       are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*       to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*       DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum              - PP's device number.
*       dipBytesSelectMapArr- array, which contains numbers of 4 bytes from the
*                             DIP IPv6 address are used upon IPv6 MC bridging.
*                             The first array element contains DIP byte 0,
*                             respectively the fourth array element contains
*                             DIP byte 3.
*
*                             The numbers of IPv6 address bytes are in the
*                             Network format and in the range [0:15], where 0
*                             is MSB of IP address and 15 is LSB of IP address.
*
*       sipBytesSelectMapArr- array, which contains numbers of 4 bytes from the
*                             SIP IPv6 address are used upon IPv6 MC bridging.
*                             The first array element contains SIP byte 0,
*                             respectively the fourth array element contains
*                             SIP byte 3.
*
*                             The numbers of IPv6 address bytes are in the
*                             Network format and in the range [0:15], where 0
*                             is MSB of IP address and 15 is LSB of IP address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PTR               - on NULL pointers.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcIpv6BytesSelectSet
(
    IN GT_U8  devNum,
    IN GT_U8  dipBytesSelectMapArr[4],
    IN GT_U8  sipBytesSelectMapArr[4]
);

/*******************************************************************************
* cpssDxChBrgMcIpv6BytesSelectGet
*
* DESCRIPTION:
*
*       Gets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*       are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*       to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*       DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum              - PP's device number.
*
* OUTPUTS:
*       dipBytesSelectMapArr- array, which contains numbers of 4 bytes from the
*                             DIP IPv6 address are used upon IPv6 MC bridging.
*                             The first array element contains DIP byte 0,
*                             respectively the fourth array element contains
*                             DIP byte 3.
*
*                             The numbers of IPv6 address bytes are in the
*                             Network format and in the range [0:15], where 0
*                             is MSB of IP address and 15 is LSB of IP address.
*
*       sipBytesSelectMapArr- array, which contains numbers of 4 bytes from the
*                             SIP IPv6 address are used upon IPv6 MC bridging.
*                             The first array element contains SIP byte 0,
*                             respectively the fourth array element contains
*                             SIP byte 3.
*
*                             The numbers of IPv6 address bytes are in the
*                             Network format and in the range [0:15], where 0
*                             is MSB of IP address and 15 is LSB of IP address.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PTR               - on NULL pointers.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcIpv6BytesSelectGet
(
    IN  GT_U8  devNum,
    OUT GT_U8  dipBytesSelectMapArr[4],
    OUT GT_U8  sipBytesSelectMapArr[4]
);

/*******************************************************************************
* cpssDxChBrgMcEntryWrite
*
* DESCRIPTION:
*       Writes Multicast Group entry to the HW.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - PP's device number.
*       vidx            - multicast group index.
*                         valid range 0..(Layer-2 Multicast groups number - 1).
*                         see datasheet of specific device to get number of
*                         Layer-2 Multicast groups.
*                         The flood VIDX entry is not allowed for configuration.
*                         The flood VIDX is 0x3FF for xCat2 devices and
*                         0xFFF for other devices.
*       portBitmapPtr   - pointer to the bitmap of ports are belonged to the
*                         vidx MC group of specified PP device.
*                         (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..26;
*                                             DxCh2, DxCh3, xCat, Lion, xCat2 0..27;
*                                             Lion2, Lion3 0..127)
*                         CPU port supported
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum, ports bitmap value or
*                                  vidx is larger than the allowed value.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcEntryWrite
(
    IN GT_U8                devNum,
    IN GT_U16               vidx,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
);

/*******************************************************************************
* cpssDxChBrgMcEntryRead
*
* DESCRIPTION:
*       Reads the Multicast Group entry from the HW.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - PP's device number.
*       vidx            - multicast group index.
*                         valid range 0..(Layer-2 Multicast groups number - 1).
*                         see datasheet of specific device to get number of
*                         Layer-2 Multicast groups.
*                         The flood VIDX entry is allowed for reading.
*                         The flood VIDX is 0x3FF for xCat2 devices and
*                         0xFFF for other devices.
*
* OUTPUTS:
*       portBitmapPtr   - pointer to the bitmap of ports are belonged to the
*                         vidx MC group of specified PP device.
*                         CPU port supported
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum or
*                                  vidx is larger than the allowed value.
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcEntryRead
(
    IN  GT_U8                devNum,
    IN  GT_U16               vidx,
    OUT CPSS_PORTS_BMP_STC   *portBitmapPtr
);

/*******************************************************************************
* cpssDxChBrgMcGroupDelete
*
* DESCRIPTION:
*       Deletes the Multicast Group entry from the HW.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - PP's device number.
*       vidx            - multicast group index.
*                         valid range 0..(Layer-2 Multicast groups number - 1).
*                         see datasheet of specific device to get number of
*                         Layer-2 Multicast groups.
*                         The flood VIDX entry is not allowed for configuration.
*                         The flood VIDX is 0x3FF for xCat2 devices and
*                         0xFFF for other devices.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum or
*                                  vidx is larger than the allowed value.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcGroupDelete
(
    IN GT_U8    devNum,
    IN GT_U16   vidx
);

/*******************************************************************************
* cpssDxChBrgMcMemberAdd
*
* DESCRIPTION:
*       Add new port member to the Multicast Group entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - PP's device number.
*       vidx            - multicast group index.
*                         valid range 0..(Layer-2 Multicast groups number - 1).
*                         see datasheet of specific device to get number of
*                         Layer-2 Multicast groups.
*                         The flood VIDX entry is not allowed for non CPU port
*                         configuration. CPU port may be added to the flood
*                         VIDX entry.
*                         The flood VIDX is 0x3FF for xCat2 devices and
*                         0xFFF for other devices.
*       portNum         - physical port number.
*                         (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..26;
*                                             DxCh2, DxCh3, xCat, Lion, xCat2 0..27;
*                                             Lion2, Lion3 0..127)
*                         CPU port supported
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum, portNum or
*                                  vidx is larger than the allowed value.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_U16                   vidx,
    IN GT_PHYSICAL_PORT_NUM     portNum
);

/*******************************************************************************
* cpssDxChBrgMcMemberDelete
*
* DESCRIPTION:
*       Delete port member from the Multicast Group entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - PP's device number.
*       vidx            - multicast group index.
*                         valid range 0..(Layer-2 Multicast groups number - 1).
*                         see datasheet of specific device to get number of
*                         Layer-2 Multicast groups.
*                         The flood VIDX entry is not allowed for non CPU port
*                         configuration. CPU port may be deleted from the flood
*                         VIDX entry.
*                         The flood VIDX is 0x3FF for xCat2 devices and
*                         0xFFF for other devices.
*       portNum         - physical port number.
*                         (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..26;
*                                             DxCh2, DxCh3, xCat, Lion, xCat2 0..27;
*                                             Lion2, Lion3 0..127)
*                         CPU port supported
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum, portNum or
*                                  vidx is larger than the allowed value.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChBrgMcMemberDelete
(
    IN GT_U8                        devNum,
    IN GT_U16                       vidx,
    IN GT_PHYSICAL_PORT_NUM         portNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgMch */
/*
 *------------------------------------------------------------------
 * $Log: cpssDxChBrgMc.h,v $
 * Revision 1.1  2015/02/13 11:31:30  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
