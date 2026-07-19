/* $Id: irqrtn.c,v 1.2 2017/07/28 07:58:47 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/irqrtn.c,v $
 *------------------------------------------------------------------
 * irqrtn.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2010 LSI Inc.
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * irqrtn.c - Interrupt routing routine
 * 	NOTE: placed in a separate file so it can be replaced by user version
 *
 *  Created on: Nov 10, 2009
 *      Author: dokim
 */

#include "libppbint.h"
#include "libgeneric.h"
#include "mpcore_dist_intrc_io.h"

void irqrtn()
{
	uint32_t int_id_temp;
	int_id_temp = __armif_read_intid();

	if(_intr_isr[sp_readMPcpuid()][int_id_temp] != 0) {
		(*_intr_isr[sp_readMPcpuid()][int_id_temp])();
	}

	sp_ClearInterrupt(int_id_temp);
}

/******** History ********
$Log: irqrtn.c,v $
Revision 1.2  2017/07/28 07:58:47  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

