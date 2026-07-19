/* $Id: errrtn.c,v 1.2 2017/07/28 07:58:39 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/errrtn.c,v $
 *------------------------------------------------------------------
 * errrtn.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * errrtn.c
 *
 *  Created on: Nov 10, 2009
 *      Author: bryansch
 */

#include <stdint.h>
#include "libgeneric.h"

/* Default error handler called from init.s */
void errrtn(uint32_t failnumber) {
	lsi_mg_test_fail();
}
/******** History ********
$Log: errrtn.c,v $
Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:48:10  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:30  srane
Initial checkin


$Endlog$
*/

