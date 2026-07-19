/* $Id: mpcore_dist_intrc_io.h,v 1.2 2017/07/28 07:58:49 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/mpcore_dist_intrc_io.h,v $
 *------------------------------------------------------------------
 * mpcore_dist_intrc_io.h
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
 * mpcore_dist_intrc_io.h
 *
 *  Created on: Aug 26, 2009
 *      Author: dokim
 */



#ifndef INTC_IO_H_
#define INTC_IO_H_

#include "lsi_sp27xx_regops.h"

#define ALL					0xa11

#define EDGE_SENSITIVE	1
#define LEVEL_SENSITIVE	0

#define N_BY_N		0
#define ONE_BY_N	1

#define CPU_TARGET_ARM0 	1
#define CPU_TARGET_ARM1 	2

#define INT_STATUS_PENDING			1
#define INT_STATUS_NON_PENDING		0

#define INT_STATUS_ACTIVE			1
#define INT_STATUS_NON_ACTIVE		0


void __armif_enable_irq(void);
void __armif_disable_irq(void);

int __armif_read_intid(void);
int __armif_clr_int(int int_id);
int __armif_set_pri_msk(int pri_level);

void __dic_en(void);
void __dic_disable(void);

int __dic_int_en(int int_id);
int __dic_int_dis(int int_id);

int __dic_int_set_cpu(int int_id, int cpu_id);
int __dic_int_reset_cpu(int int_id, int cpu_id);
int __dic_int_set_pri(int int_id, int priority);

int __dic_int_conf(int int_id, int detect_type, int sw_model);
int __dic_int_chk_pen(int int_id);
int __dic_int_clr_pen(int int_id);
int __dic_int_chk_active(int int_id);
int __dic_int_read_active(int reg_offset);

void __dic_sw_intr_gen(int sw_int_id);
void __arm_intr_gen_to_dss(int dss_core_id, int intr_num);

#endif /* DIST_INTC_IO_H_ */

/******** History ********
$Log: mpcore_dist_intrc_io.h,v $
Revision 1.2  2017/07/28 07:58:49  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:37  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

