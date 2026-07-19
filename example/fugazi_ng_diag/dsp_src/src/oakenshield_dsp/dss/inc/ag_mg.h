/* $Id: ag_mg.h,v 1.2 2017/07/28 07:58:33 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/ag_mg.h,v $
 *------------------------------------------------------------------
 * ag_mg.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>    NOTIFICATION    <<<<<<<<<<<<<<<<<<<<<<<<<<<<
 *
 * Copyright (©) 2006 Agere Systems Inc.
 * All Rights Reserved
 *
 * This is unpublished proprietary information of Agere Systems Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of Agere Systems Inc. and treated accordingly.
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#ifndef AG_MG_H_
#define AG_MG_H_

#include "ag_typedefs.h"
#include <stddef.h>
#include <stdio.h>
#include "ag_mg_config.h"

/*----------------------------------*/
/*   	CONSTANTS    				*/
/*----------------------------------*/

/*----------------------------------*/
/*   	TYPE DEFINITIONS    		*/
/*----------------------------------*/

enum
{
	AG_MG_DEV_ICU_PPBDSS_0_INT		= 32,
	AG_MG_DEV_ICU_PPBDSS_1_INT		= 33,
	AG_MG_DEV_ICU_PPBDSS_2_INT		= 34,
	AG_MG_DEV_ICU_PCE0_EOF_INT		= 78,
	AG_MG_DEV_ICU_PCE1_EOF_INT		= 80,
	AG_MG_DEV_ICU_TXD0_EXC_0_INT	= 83,
	AG_MG_DEV_ICU_TXD0_EOF_0_INT	= 84,
	AG_MG_DEV_ICU_TXD1_EXC_0_INT	= 85,
	AG_MG_DEV_ICU_TXD1_EOF_0_INT	= 86,
	AG_MG_DEV_ICU_TXD0_EXC_1_INT	= 114,
	AG_MG_DEV_ICU_TXD0_EOF_1_INT	= 115,
	AG_MG_DEV_ICU_TXD1_EXC_1_INT	= 117,
	AG_MG_DEV_ICU_TXD1_EOF_1_INT	= 118
};

/*----------------------------------*/
/*   	MACROS    					*/
/*----------------------------------*/

/*----------------------------------*/
/* 		HEADER INCLUSION	 		*/
/*----------------------------------*/
//#include "heap.h"	// OSEck
#include "ag_mg_regs.h"
#include <string.h>					// For memcpy prototype

/*----------------------------------*/
/* 		FUNCTION DECLARATIONS 		*/
/*----------------------------------*/

/*----------------------------------*/
/*   	MACROS after headers		*/
/*----------------------------------*/

#define getDssCoreId() 														\
	((*((uint32_t *)AG_MG_REGS_DSS_IDCODE_RA)								\
				>> AG_MG_REGS_DSS_IDCODE_DSSID_BO)							\
				& AG_MG_REGS_DSS_IDCODE_DSSID_BM)

/* For cycle profiling */
#define getCycles_init() {													\
						REG32_WRITE(AG_MG_REGS_OCE_ECNT_VAL_RA, 0);			\
						REG32_WRITE(AG_MG_REGS_OCE_ECNT_CTRL_RA, 0x000001FC); }
#define getCycles()		(0x7FFFFFFF - HW_REG_ACCESS(AG_MG_REGS_OCE_ECNT_VAL_RA))

#endif

/******** History ********
$Log: ag_mg.h,v $
Revision 1.2  2017/07/28 07:58:33  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:24  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 18:08:25  srane
Initial checkin


$Endlog$
*/

