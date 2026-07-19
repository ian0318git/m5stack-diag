/* $Id: tdm_intr_and_unicnt_cntl.h,v 1.2 2012/05/10 22:57:02 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/tdm_intr_and_unicnt_cntl.h,v $
 *------------------------------------------------------------------
 * tdm_intr_and_unicnt_cntl.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*
 * tdm_intr_cntl.h
 *
 *  Created on: Oct 5, 2009
 *      Author: dokim
 */

#ifndef TDM_INTR_CNTL_H_
#define TDM_INTR_CNTL_H_

/* controlling general TDM interrupts */
/* enable <intr_type> interrupt to <core> processor */
int __tdm_intr_enable(int port, int core, int intr_type);

/* disable <intr_type> interrupt to <core> processor */
int __tdm_intr_disable(int port, int core, int intr_type);

/* clearing interrupt */
int __tdm_intr_clr(int port, int core, int type_intr);

/* reset unicnt */
void __unicnt_reset(int unicnt);

/* set unicnt's limit */
void __unicnt_conf(int unicnt, int clk_src, int limit);

/* enable unicnt */
void __unicnt_en(int unicnt);

/* disable unicnt */
void __unicnt_dis(int unicnt);

#endif /* TDM_INTR_CNTL_H_ */

/******** History ********
$Log: tdm_intr_and_unicnt_cntl.h,v $
Revision 1.2  2012/05/10 22:57:02  srane
Add TDM support.

Revision 1.1  2012/04/18 09:47:32  srane
Initial checkin


$Endlog$
*/

