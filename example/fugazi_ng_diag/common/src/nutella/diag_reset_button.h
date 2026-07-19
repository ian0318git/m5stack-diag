/* $Id: diag_reset_button.h,v 1.5 2019/10/16 23:46:24 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_reset_button.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_reset_button.h
 * Description: Header file of Nutella RESET button Diag test.
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RESET_BUTTON_H__
#define __DIAG_RESET_BUTTON_H__

/* Nutella HW request */
#define WAIT_RESET_BUTTON_STATUS    5000

/* Extern */
extern int nutella_reset_button_test(void);

#endif

/*------------------------------------------------------------------
$Log: diag_reset_button.h,v $
Revision 1.5  2019/10/16 23:46:24  alicehua
CSCvr66516: Fix register offset and delay range in reset button test.

Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
