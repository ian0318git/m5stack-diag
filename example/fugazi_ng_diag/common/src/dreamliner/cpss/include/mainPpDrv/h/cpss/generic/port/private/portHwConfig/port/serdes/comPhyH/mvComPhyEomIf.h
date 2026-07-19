/* $Id: mvComPhyEomIf.h,v 1.1 2015/02/13 11:35:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyH/mvComPhyEomIf.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell 
* International Ltd. and/or its affiliates ("Marvell") under the following 
* alternative licensing terms.  
* If you received this File from Marvell, you may opt to use, redistribute 
* and/or modify this File under the following licensing terms. 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer. 
*  -   Redistributions in binary form must reproduce the above copyright 
*       notice, this list of conditions and the following disclaimer in the 
*       documentation and/or other materials provided with the distribution. 
*  -    Neither the name of Marvell nor the names of its contributors may be 
*       used to endorse or promote products derived from this software without 
*       specific prior written permission. 
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
* mvComPhyEomIf.h
*
* DESCRIPTION:
*       
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __mvComPhyEOMIf_H
#define __mvComPhyEOMIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	

#define  EOM_VOLT_RANGE (32)
#define  EOM_PHASE_RANGE (128)


typedef enum {SearchLeft, SearchRight, Done} MV_STATE_MODE;

typedef struct  
{
  GT_U32 totalError;
}MV_EOM_RESULTS;


/*******************************************************************************
* mvHwsEomInit
*
* DESCRIPTION:
*       Init EOM serdes mechanism.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsEomInit
(
	GT_U8                     devNum,
	GT_U32                    portGroup,
	GT_U32                    serdesNum
);

/*******************************************************************************
* mvHwsEomClose
*
* DESCRIPTION:
*       Disable EOM serdes mechanism.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsEomClose
(
	GT_U8                     devNum,
	GT_U32                    portGroup,
	GT_U32                    serdesNum
);


/*******************************************************************************
* mvHwsEomGetUi
*
* DESCRIPTION:
*       Returns the current system baud rate.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       current system baud rate.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsEomGetUi
(
	GT_U8      devNum,               
	GT_U32     portGroup,
	GT_U32     serdesNum,
	GT_U32*		 curUi
);

/*******************************************************************************
* mvHwsEomGetDfeRes
*
* DESCRIPTION:
*       Returns the current DFE parameters.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       current DFE resolution.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsEomGetDfeRes
(
	GT_U8      devNum,               
	GT_U32     portGroup,
	GT_U32     serdesNum,
	GT_U32*		 dfeRes
);

/*******************************************************************************
* mvHwsEomGetMatrix
*
* DESCRIPTION:
*       Calculate and returns the eye mapping matrix.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       timeout   - wait time in msec
*
* OUTPUTS:
*       horizontal/vertical Rx eye matrix
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsEomGetMatrix
(
	GT_U8     devNum,
	GT_U32    portGroup,
	GT_U32    serdesNum,
	GT_U32    timeout,
	GT_U32    *rowSize,
	GT_U32		*upMatrix,
	GT_U32    *loMatrix
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvComPhyEOMIf_H */

/*
 *------------------------------------------------------------------
 * $Log: mvComPhyEomIf.h,v $
 * Revision 1.1  2015/02/13 11:35:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
