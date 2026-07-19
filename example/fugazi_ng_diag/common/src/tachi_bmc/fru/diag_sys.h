/* $Id: diag_sys.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_sys.h,v $
 *
 *      File:   diag_sys.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

extern int		sys_max_slot_get();
extern int 		sys_card_present (uint8_t slot);
extern card_type_t	sys_card_type_get (uint8_t slot);
extern board_info_t*	sys_card_info_get (card_type_t card_type);
extern int		sys_dev_count_get (dev_type_t dev_type);
extern int		sys_port_count_get ();
#endif // _SYSTEM_H_
