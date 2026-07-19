/* $Id: prvCpssCommonDefs.h,v 1.1 2015/02/13 11:35:07 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/private/prvCpssCommonDefs.h,v $
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
* prvCpssCommonDefs.h
*
* DESCRIPTION:
*       Private common definitions.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssCommonDefsh
#define __prvCpssCommonDefsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssCommonDefs.h>

/* Converts packet command to hardware value */
#define PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(_val, _cmd)           \
    switch (_cmd)                                                       \
    {                                                                   \
        case CPSS_PACKET_CMD_FORWARD_E:                                 \
            _val = 0;                                                   \
            break;                                                      \
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:                           \
            _val = 1;                                                   \
            break;                                                      \
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:                             \
            _val = 2;                                                   \
            break;                                                      \
        case CPSS_PACKET_CMD_DROP_HARD_E:                               \
            _val = 3;                                                   \
            break;                                                      \
        case CPSS_PACKET_CMD_DROP_SOFT_E:                               \
            _val = 4;                                                   \
            break;                                                      \
        default:                                                        \
                return GT_BAD_PARAM;                                    \
    }

/* Converts packet command to hardware value */
#define PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(_cmd, _val)           \
    switch (_val)                                                       \
    {                                                                   \
        case 0:                                                         \
            _cmd = CPSS_PACKET_CMD_FORWARD_E;                           \
            break;                                                      \
        case 1:                                                         \
            _cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;                     \
            break;                                                      \
        case 2:                                                         \
            _cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;                       \
            break;                                                      \
        case 3:                                                         \
            _cmd = CPSS_PACKET_CMD_DROP_HARD_E;                         \
            break;                                                      \
        case 4:                                                         \
            _cmd = CPSS_PACKET_CMD_DROP_SOFT_E;                         \
            break;                                                      \
        default: _cmd =  (_val);                                        \
    }

/* Convert Vlan/QoS command to hardware value */
#define PRV_CPSS_CONVERT_ATTR_ASSIGN_CMD_TO_HW_VAL_MAC(_val, _cmd)      \
    switch (_cmd)                                                       \
    {                                                                   \
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E:                   \
            _val = 0;                                                   \
            break;                                                      \
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E:               \
            _val = 1;                                                   \
            break;                                                      \
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E:                 \
            _val = 2;                                                   \
            break;                                                      \
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E:                    \
            _val = 3;                                                   \
            break;                                                      \
        default:                                                        \
                return GT_BAD_PARAM;                                    \
    }

/* Convert hardware value to Vlan/QoS command */
#define PRV_CPSS_CONVERT_HW_VAL_TO_ATTR_ASSIGN_CMD_MAC(_cmd, _val)      \
    switch (_val)                                                       \
    {                                                                   \
        case 0:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;             \
            break;                                                      \
        case 1:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;         \
            break;                                                      \
        case 2:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;           \
            break;                                                      \
        case 3:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;              \
            break;                                                      \
        default:                                                        \
            _cmd =  (_val);                                             \
    }

/* Convert Vlan/QoS precedence to hardware value */
#define PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(_val, _prec)      \
    switch (_prec)                                                  \
    {                                                               \
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:        \
            _val = 0;                                               \
            break;                                                  \
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:        \
            _val = 1;                                               \
            break;                                                  \
        default:                                                    \
            return GT_BAD_PARAM;                                    \
    }

/* Convert hardware value to Vlan/QoS precedence */
#define PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(_prec, _val)      \
    switch (_val)                                                   \
    {                                                               \
        case 0:                                                     \
            _prec = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E; \
            break;                                                  \
        case 1:                                                     \
            _prec = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E; \
            break;                                                  \
        default:                                                    \
            _prec =  _val;                                          \
    }

/* check etherTypeMode value */
#define PRV_CPSS_ETHER_TYPE_MODE_CHECK_MAC(etherTypeMode)           \
    if((etherTypeMode != CPSS_VLAN_ETHERTYPE0_E) &&                 \
       (etherTypeMode != CPSS_VLAN_ETHERTYPE1_E)) return GT_BAD_PARAM


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonDefsh */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssCommonDefs.h,v $
 * Revision 1.1  2015/02/13 11:35:07  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
