/* $Id: ag_mg_config.h,v 1.2 2017/07/28 07:58:33 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/ag_mg_config.h,v $
 *------------------------------------------------------------------
 * ag_mg_config.h
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
 * Author:	 Said Belhaj 4/27/2006
 * Content:  Project configuration
 *
 * $History: $
 *
 ******************************************************************************/
#ifndef AG_MG_CONFIG_H_
#define AG_MG_CONFIG_H_

/* !MANFILE:	Project configuration file */

/******************************************************************************
*                          Hardware Configuration
*******************************************************************************/
#define AG_MG_NUM_DSS			4
#define SP_NUM_DSSs				AG_MG_NUM_DSS
#define SP_NUM_EMACs			2

#define AG_MG_CYCLES_PER_TICK	(16)

#define _AG_MG_XTALFREQ			(50000000)
#define _AG_MG_CLKFREQ			(750000000)
#define _AG_MG_SYSCLK			(_AG_MG_CLKFREQ/AG_MG_CYCLES_PER_TICK)

#endif

/******** History ********
$Log: ag_mg_config.h,v $
Revision 1.2  2017/07/28 07:58:33  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:24  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 18:08:25  srane
Initial checkin


$Endlog$
*/
