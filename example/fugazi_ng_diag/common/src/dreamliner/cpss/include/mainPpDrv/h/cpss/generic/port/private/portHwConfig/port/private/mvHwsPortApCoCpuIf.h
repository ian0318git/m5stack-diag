/* $Id: mvHwsPortApCoCpuIf.h,v 1.1 2015/02/13 11:34:59 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/private/mvHwsPortApCoCpuIf.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsPortApCpCpuIf.h
*
* DESCRIPTION: Internal definition.
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwsPortApCpCpuIf_H
#define __mvHwsPortApCpCpuIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

typedef enum
{
  Extended_Global_Control = 0x5C,
  Confi_Processor_Global_Cfg = 0x500
}MV_HWS_CONFI_REGS;


typedef void (*MV_HWS_GET_Z80_CODE_FUNC_PTR)
(
	GT_U32  **z80Code,
	GT_U32  *size
);


GT_UINTPTR mvApGetSharedMemPtr(GT_U8 devNum, GT_U32 portGroup);
void mvApFwRun(GT_U8 devNum, GT_U32 portGroup);
void mvApFwStop(GT_U8 devNum, GT_U32 portGroup);
void mvApSetSharedMem(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 data);
void mvApGetSharedMem(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 *data);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortApCpCpuIf_H */


/*
 *------------------------------------------------------------------
 * $Log: mvHwsPortApCoCpuIf.h,v $
 * Revision 1.1  2015/02/13 11:34:59  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
