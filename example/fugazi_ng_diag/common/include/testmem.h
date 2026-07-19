/* $Id: testmem.h,v 1.4 2012/08/15 14:26:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/testmem.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Structure for our memory test function.
*/

struct testmem {
  utype_t *start, *rdaddr;
  long length;
  ulong passcount;
  unsigned char flag;
};


/* defines for the above flag field */
#define QUITONERR  0x01    /* abort the test on first error */
#define ABBR_TEST  0x02    /* do only the abbreviated test */
#define INDIAGS    0x04    /* use the diagnostic environment provided */
#define RDADDR     0x08    /* separate read and write addresses */
#define MUTE       0x10    /* keep quiet about errors, etc. */
#define DRAM       0x20    /* DRAM present on the platform */
#define PARITY_EN  0x40    /* Parity enabled during a main or */
                           /* shared memory parity test (cancun only)*/

#define DR0_MEMORY    0
#define DR1_MEMORY    1
#define DR2_MEMORY    2

extern int testmem(struct testmem *tmemp);
extern int memrefresh(struct testmem *tmemp);
extern int meminfo();
extern int test_mem_6n(unsigned long *, unsigned long, char *);
extern int test_masked_mem_6n(unsigned long *, unsigned long, unsigned long, char *);
extern int testmem_byte(struct testmem *, uint8_t *);

extern char *memphmsg;
extern unsigned long num_pattern[];
extern unsigned long num_pats;

/******** History ******** 
$Log: testmem.h,v $
Revision 1.4  2012/08/15 14:26:15  alpeng
support CLI cmds memdebug, memloop and memtest

Revision 1.3  2012/06/06 15:00:06  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
