/* $Id: platform_dev.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/platform_dev.c,v $
 *
 *      File:   platform.c
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include "diag_main.h"
#include "diag_dev.h"

#include "diag_platform.h"
#include "diag_reg.h"

board_info_t platform_info[] = 
{
	{NULL, DEV_UNKNOWN, 0, NULL, NULL, (section_toc_t*)NULL, 
	 NULL, NULL, NULL, NULL, 0},
};

int sys_max_slot_get()
{
	return (1);
}

card_type_t sys_card_type_get (uint8_t slot)
{
	return (slot ? CARD_UNKNOWN : CARD_BMC);
}

int  sys_card_present (uint8_t slot)
{
	return (slot ? 0 : 1);
}

board_info_t* sys_card_info_get (card_type_t card_type)
{
	board_info_t *pinfo = NULL;
	switch(card_type) {
		case	CARD_BMC:
			pinfo = platform_info;
			break;

		default:
			break;
	}
	return (pinfo);
}

int sys_dev_count_get (dev_type_t dev_type)
{
	board_info_t *pinfo = sys_card_info_get(CARD_BMC);

	while (pinfo && pinfo->name) {
		if (pinfo->dev_type == dev_type)
			return (pinfo->dev_cnt);
		pinfo++;
	}
	return (0);
}
