/* $Id: dev_print.h,v 1.4 2013/10/08 11:03:47 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/dev_print.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: dev_print.h
 * Support the dev_print.c. Bring extern prototype from other .h
 * in here to minimize the impact of warning when compiling.
 *
 * January 2006 - Anh Dang
 *
 * Copyright (c) 2008-2012, 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _DEV_PRINT_H_
#define _DEV_PRINT_H_

#include <stdint.h> /* for uint32_t */
#include <stdio.h>
#include "types.h"  /* for boolean */

#define DEV_PRINT_PTR  g_dev_print_ptr
#define DEV_PRINT_SIZE 0x100000 /* 1MEG */

/* 
 * This struct dev_prstuff has to be exactly the same as 
 * struct prstuff in printf.c for dev_print() to leverage 
 * the existing code printf(), sprintf()in library lib/src
 */
struct dev_prstuff { 
  char ljust, padchar, *bufptr;
  long precision, fldwidth;
  int count;
};

extern char *dev_print_start;
extern char *g_dev_print_ptr;
extern int sh_poll_slot;

extern int dumpdevprint(void);
extern int clrdevprint(void);
extern void endofdebuginfo(void);
extern uint32 db_print(char *fmtptr, ...);

typedef uint32 (*print_fn_t)(char *, ...);
extern void (*poll_slot_fptr)(print_fn_t); 

/* bring these ..._print_...() function over to avoid those warning */
extern void oc3_vpd_print_wvic_slot(int, print_fn_t);
extern void soprano_print_vic_slots(int, print_fn_t);
extern void soprano_print_vic_daughter(int, print_fn_t);
extern void soprano_print_pvdm_slots(int slot, print_fn_t print_fn);
extern void print_pvdm_info(void);
extern void print_vwic_daughter_modules(print_fn_t);
extern void fecpm_print_vwic_daughter(int, print_fn_t);
extern void print_mb_vwic_daughtercard_info(print_fn_t);
extern void print_daughtercard_modules(print_fn_t);
extern void vpm_print_vic_slots(int, print_fn_t); 
extern void copland_print_vic_slot (int, print_fn_t);
extern void copland_print_spmm_slots(int, print_fn_t);
extern void copland_print_vic_daughter(int, print_fn_t);
extern void guido_print_vic_slots (int slot, print_fn_t);
extern void guido_print_vic_daughter(int slot, print_fn_t);
extern int  print_nm_em_cookie (int slot, int em_slot, print_fn_t);
extern int  bigband_print_daughtercard(int slot, print_fn_t);
extern int  volant_print_daughtercard(int, print_fn_t);
extern void venom_print_daughtercard(int, print_fn_t);
extern void dis_pci_regs(int slot, uchar bus, uchar dev, print_fn_t print_fn);
extern int  print_aim_info(print_fn_t);
extern int print_all_slots(int);
extern void print_mb_slots(print_fn_t print_fn);
extern int  bryce_print_daughtercard(int slot, int em_slot, 
				     print_fn_t print_fn);

extern void stack_trace(ulong *pc, ulong *sp, ulong ra, 
			int frames, print_fn_t print_fn);
extern long getcpu_sp();

#endif  /* _DEV_PRINT_H_ */

/******** History ******** 
$Log: dev_print.h,v $
Revision 1.4  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.3  2012/09/18 19:19:54  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
