/* $Id: diag_arch.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_arch.c,v $
 *
 *      File:   diag_arch.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Diag infra structure 
 *
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/


/*************************************************************** 
 *                                                             * 
 * File Name   : diag_arch.c                                   * 
 *                                                             * 
 * Description : This file includes the code for the entry     * 
 *               point for Off-line diags. It also includes    * 
 *               code to build all the required test           * 
 *               structures. However the code in this file     * 
 *               should be platform independant.               * 
 *                                                             * 
 *                                                             * 
 ***************************************************************/

#include "diag_main.h"

static diag_dev_t** psystem = NULL;
static diag_dev_t*  pboard = NULL;

uint32_t diag_board_dev_set (uint32_t slot)
{
	if (psystem && psystem[slot]) {
		pboard =  psystem[slot];
	} else {
		printf(" ERROR: Invalid dev tree @ slot%d\n", slot);
		return (DIAG_SYSTEM_ERROR);
	}
	return (0);
}

diag_dev_t* diag_board_dev_get ()
{
	return pboard;
}

/*************************************************************** 
 *                                                             * 
 * Function : diag_main()                                      * 
 *                                                             * 
 * Description : This function is the entry point for the      * 
 *               Offline diags. The required memory is         * 
 *               allocated and the structres are initialized.  * 
 *               After initializing, it enters into the cli    * 
 *               routine.                                      * 
 *                                                             * 
 ***************************************************************/

int diag_main()
{
	int 		rc = 0, max_slots, slot;
	card_type_t	card_type;
	board_info_t	*pboard_info;

	max_slots = sys_max_slot_get();
	if (max_slots <= 0) {
		printf("System is unsupported. max_slots = %d\n",
				max_slots);
		return (DIAG_SYSTEM_ERROR);
	}

	psystem = (diag_dev_t**)malloc(max_slots * sizeof(diag_dev_t*));
	if (!psystem) {
		printf(" ERROR: Failed to allocate %d * %d\n",
				(int)sizeof(diag_dev_t*), max_slots);
		return (DIAG_MALLOC_ERROR);
	}

	memset(psystem, 0, max_slots * sizeof(diag_dev_t*));

	printf(" max_slots = %d\n", max_slots);
	for (slot = 0; slot < max_slots; slot++) {

		if (!sys_card_present(slot))
			continue;
		card_type = sys_card_type_get(slot);
		if (card_type == CARD_UNKNOWN) {
			printf(" ERROR: Unknown card Slot%dn", slot);
			return (DIAG_SYSTEM_ERROR);
		}

		pboard_info = sys_card_info_get(card_type);	
		if (!pboard_info) {
			printf(" ERROR: Unsupported card %d in slot%d\n",
					card_type, slot);
			return (DIAG_SYSTEM_ERROR);
		}

		rc = diag_dev_init(pboard_info, &psystem[slot]);
		if (rc) {
			printf(" ERROR: Failed to create tree\n");
			return (DIAG_SOFTWARE_ERROR);
		}
	}

	diag_board_dev_set(0);
	return (rc);
}

/***************************************************************
 *                                                             *
 *  Function  : ExitDiags                                      *
 *                                                             *
 *  This function is used to free all the allocated memory and *
 *  remove the fifo's created before quitting the program.     *
 *                                                             *
 ***************************************************************/

void ExitDiags(int sig)
{
}

