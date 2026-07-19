/* $Id: ddr3_init.c,v 1.2 2017/07/28 07:58:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/ddr3_init.c,v $
 *------------------------------------------------------------------
 * ddr3_init.c 
 * Description: DDR3 configuration.
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2011 LSI Inc.  All Rights Reserved
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
 * lsiEvalBoardDDR3config.c:	configure DDR3 for LSI SP27XX Evaluation Boards
 *
 * 		NOTE: Handles 32-bit vs. 16-bit configuration automatically
 *
 * 		sp_DDR3init() & ddr3_hasECCerrors() are only external entry points
 *
 * Authors    :   JWB / LNJ
 *
 *************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libdma.h"


#define SIZE_OF_DENALI_ARRAY ( LSI_SP27XX_IS_V10_DEVICE() ? 169 : 176 )

uint32_t *p_Denali_Control_Vals = 0;

/*
 * g_BYTELANE_MASK is a bit mask with one bit for each lane:
 * 			MSK_ECC|MSK_LANE3|MSK_LANE2|MSK_LANE1|MSK_LANE0
 * set to 0x1F for for 32-bit SDRAM configurations with ECC
 * set to 0x0F for for 32-bit SDRAM configurations without ECC
 * set to 0x13 for for 16-bit SDRAM configurations with ECC
 * set to 0x03 for for 16-bit SDRAM configurations without ECC
 * */
uint32_t g_BYTELANE_MASK = 0;

int						/* ret: 0 => no errors */
ddr3_hasECCerrors(void)	/* check for any DDR3 controller caught single or multi-bit ECC errors */
{
	if (LSI_SP27XX_IS_V10_DEVICE()) {
		return(HW_REG_ACCESS(LSI_SP27XX_DDR3_DENALI_CTL_71_RA) & 0x3C);
	} else {
		return(HW_REG_ACCESS(LSI_SP27XX_DDR3_DENALI_CTL_71_RA) & 0x78);
	}
}


/******** History ********
$Log: ddr3_init.c,v $
Revision 1.2  2017/07/28 07:58:50  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.3  2012/08/15 14:52:23  srane
cleanup code.

Revision 1.2  2012/07/17 20:34:38  srane
cleanup

Revision 1.1  2012/06/07 22:34:29  srane
Initial checkin for ECC memory test.
 

$Endlog$
*/

