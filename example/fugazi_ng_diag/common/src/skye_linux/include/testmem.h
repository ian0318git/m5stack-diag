/* $Id: testmem.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/testmem.h,v $
 *------------------------------------------------------------------
 * testmem.h - Definitions file for memory test
 *
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
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

/******** History ********/ 
/*
 * $Log: testmem.h,v $
 * Revision 1.2  2015/05/25 03:59:11  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:29  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:40  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */

