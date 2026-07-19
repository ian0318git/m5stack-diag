/* $Id: cpssGenSystemRecovery.h,v 1.1 2015/02/13 11:35:11 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/systemRecovery/cpssGenSystemRecovery.h,v $
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
* cpssGenSystemRecovery.h
*
* DESCRIPTION:
*       Includes generic system recovery definitions.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __cpssGenSystemRecoveryh
#define __cpssGenSystemRecoveryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

 /*
 * Typedef enum CPSS_SYSTEM_RECOVERY_STATE_ENT
 *
 * Description: Indicates in which state system recovery process is.
 *
 * Enumerations:
 *      CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E - system is preparing to recovery process
 *      CPSS_SYSTEM_RECOVERY_INIT_STATE_E        - system is going through recovery init
 *      CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E  - system recovery is completed
 *      CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E - system is going to make hw catch up

 */
typedef enum
{
    CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E,
    CPSS_SYSTEM_RECOVERY_INIT_STATE_E,
    CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E,
    CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
} CPSS_SYSTEM_RECOVERY_STATE_ENT;

 /*
 * Typedef enum CPSS_SYSTEM_RECOVERY_PROCESS_ENT
 *
 * Description: Indicates which system recovery process is running.
 *
 * Enumerations:
 *      CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E        - HSU process
 *      CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E  - Fast Boot process
 *      CPSS_SYSTEM_RECOVERY_PROCESS_HA_E         - recovery process after HA event
 *      CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E - there is no active system recovery process
 *
 */
typedef enum
{
    CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E,
    CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E,
    CPSS_SYSTEM_RECOVERY_PROCESS_HA_E,
    CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E
} CPSS_SYSTEM_RECOVERY_PROCESS_ENT;


/*
 * Typedef: struct CPSS_SYSTEM_RECOVERY_MODE_STC
 *
 * Description: This struct containes description of system recovery modes.
 *
 * Fields:
 *      continuousRx             - This mode describes what memory allocation is for Rx descriptors and Rx buffers:
 *                                 GT_TRUE:  Rx descriptors and Rx buffers are allocated in the same memory
 *                                           before and after system recovery process.
 *                                 GT_FALSE: Rx descriptors and Rx buffers are allocated in different memory
 *                                           before and after system recovery process.
 *      continuousTx             - This mode describes what memory allocation is for Tx descriptors:
 *                                 GT_TRUE:  Tx descriptors are allocated in the same memory
 *                                           before and after system recovery process.
 *                                 GT_FALSE: Tx descriptors are allocated in different memories
 *                                           before and after system recovery process.
 *      continuousAuMessages     - This mode describes what memory allocation is for AUQ descriptors:
 *                                 GT_TRUE:  AUQ descriptors are allocated in the same memory
 *                                           before and after system recovery process.
 *                                 GT_FALSE: AUQ descriptors are allocated in different memories
 *                                           before and after system recovery process.
 *      continuousFuMessages     - This mode describes what memory allocation is for FUQ descriptors:
 *                                 GT_TRUE:  FUQ descriptors are allocated in the same memory
 *                                           before and after system recovery process.
 *                                 GT_FALSE: FUQ descriptors are allocated in different memories
 *                                           before and after system recovery process.
 *      haCpuMemoryAccessBlocked - GT_TRUE:  special mode after HA event when PP can't access CPU memory but CPU can access PP.
 *                               - GT_FALSE: regular mode; both CPU and PP can access each other memories.
 *                                 It is actually when not the same memory address spaces are used for AUQ/FUQ/RX messages
 *                                 before and after HA event.
 *                                 This mode should be set by Application after HA event before any usage of
 *                                 AUQ/FUQ/RX memories allocated before HA event, in order to prevent sending by PP
 *                                 messages to these memories already not related to PP.
 *                                 It is obligement of Application to revert  memory access to normal mode after cpssInit.
 *                                 (During cpssInit PP is initialized with new AUQ/FUQ/RX pointers )
 */
typedef struct
{
    GT_BOOL continuousRx;
    GT_BOOL continuousTx;
    GT_BOOL continuousAuMessages;
    GT_BOOL continuousFuMessages;
    GT_BOOL haCpuMemoryAccessBlocked;
}CPSS_SYSTEM_RECOVERY_MODE_STC;


/*******************************************************************************
* PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN
*
* DESCRIPTION:
*       Handling au and fu queues,Rx SDMA under non-continues mode (all AU/FU/RX messages
*       during system recovery process are disregarded) to provide smooth reprogramming in new immage.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; .
*
* NOT APPLICABLE DEVICES:
*       none
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN)(GT_VOID);


/*******************************************************************************
* PRV_CPSS_SYSTEM_RECOVERY_CATCH_UP_FUN
*
* DESCRIPTION:
*       Perform synchronization of hardware data and software DB after special init sequence.
*
* APPLICABLE DEVICES:
*         Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Puma2; Puma3.
*
* INPUTS:
*       systemRecoveryModePtr - pointer to system recovery mode data.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_SYSTEM_RECOVERY_CATCH_UP_FUN)(GT_VOID);



/*
 * Typedef: struct CPSS_SYSTEM_RECOVERY_INFO_STC
 *
 * Description: This struct containes information about system recovery process.
 *
 * Fields:
 *      systemRecoveryState                                 - Indicates in which state system recovery process is
 *      systemRecoveryMode                                  - Indicates in which working mode recovery process is.
 *      systemRecoveryProcess                               - Indicates which recovery process is taking place.
 */
typedef struct
{
    CPSS_SYSTEM_RECOVERY_STATE_ENT                    systemRecoveryState;
    CPSS_SYSTEM_RECOVERY_MODE_STC                     systemRecoveryMode;
    CPSS_SYSTEM_RECOVERY_PROCESS_ENT                  systemRecoveryProcess;
} CPSS_SYSTEM_RECOVERY_INFO_STC;

/* define the HSU  global DB */
extern CPSS_SYSTEM_RECOVERY_INFO_STC   systemRecoveryInfo;
extern PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN     prvCpssSystemRecoveryMsgDisableModeHandleFuncPtr;
extern PRV_CPSS_SYSTEM_RECOVERY_CATCH_UP_FUN             prvCpssSystemRecoveryCatchUpHandleFuncPtr;


/*******************************************************************************
* cpssSystemRecoveryStateSet
*
* DESCRIPTION:
*       This function inform cpss about system recovery state
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2; Lion3; Puma2; Puma3.
*
* NOT APPLICABLE DEVICES:
*       None
*
* INPUTS:
*       systemRecoveryInfoPtr           - pointer to system recovery information
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                  - on success.
*       GT_BAD_PTR             - on NULL pointer.
*       GT_BAD_PARAM           - on bad systemRecoveryState.
*
* COMMENTS:
*        None.
*
*******************************************************************************/
GT_STATUS cpssSystemRecoveryStateSet
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenSystemRecoveryh */

/*
 *------------------------------------------------------------------
 * $Log: cpssGenSystemRecovery.h,v $
 * Revision 1.1  2015/02/13 11:35:11  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
