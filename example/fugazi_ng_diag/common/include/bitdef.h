/* $Id: bitdef.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/bitdef.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Support for printf's ability to print a string for
** register bits.
** The actstate (active state) field defines the state of
** the bit when active.
** The string associated with the bit will be printed if
** the bit is active.
*/

typedef struct bitdef bitdef_t;

struct bitdef {
    int actstate;
    char *string;
};

/*
 * Prototypes that use bitdef_t
 */

extern unsigned long get_user_test_options(unsigned char *diag_id,
					   bitdef_t option_defs[],
					   unsigned long options_mask,
					   unsigned long test_options);

/******** History ******** 
$Log: bitdef.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
