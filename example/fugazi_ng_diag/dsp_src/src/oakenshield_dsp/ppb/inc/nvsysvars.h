/* $Id: nvsysvars.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/nvsysvars.h,v $
 *------------------------------------------------------------------
 * nvsysvars.h
 * 
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __NVSYSVARS_H__
#define __NVSYSVARS_H__

#if defined(EAGLEEYE)

#include "mon_boot.h"
#include "../src/eagleeye/nvmonvars.h"

#else /* EAGLEEYE */

#if defined(APEX_ZETA)

#include "../src/apexzeta/include/nvmonvars.h"

#else /* APEX_ZETA */

#if defined(APEX_VEGA)
 
#include "../src/apex_vega/include/nvmonvars.h"

#else /* APEX_VEGA */

#if defined(BRYCE)
 
#include "../src/bryce/include/nvmonvars.h"

#else /* BRYCE */

#if defined(SHIN)
#include "mon_boot.h"
#include "../../common/src/shinkansen/nvmonvars.h"

#else /* SHIN */

#if defined(CVM_XFORMERS)
 
#include "mon_boot.h"
#include "../../common/src/xformers/mips/nvmonvars.h"
 
#else /* CVM_XFORMERS */

#if defined(CVM_THREEGORGES)
 
#include "mon_boot.h"
#include "../../common/src/xformers/mips/nvmonvars.h"
 
#else /* CVM_THREEGORGES */

#if defined(FSL_XFORMERS)
 
#include "mon_boot.h"
#include "../../common/src/xformers/ppc/nvmonvars.h"
 
#else /* FSL_XFORMERS */

#if defined(INFORMERS)
 
#include "mon_boot.h"
#include "../../common/src/informers/nvmonvars.h"
 
#else /* INFORMERS */

#if defined(MIRAGE)

#include "mon_boot.h"
#include "../src/mirage/nvmonvars.h"

#else /* MIRAGE */

#if defined(PANOPTES)
#include "mon_boot.h"
#include "../src/panoptes/nvmonvars.h"

#else /* PANOPTES */

#define EVTSIZ           768  /* the environment table size */
#define ALSSIZ           512  /* the alias table size */
#define NVMAGIC    -17958194  /* magic number for the NV RAM */
#define TRAPMAGIC 1414676816  /* magic number for a trap (warm boot) */
#define DIAGMAGIC 1145651527  /* magic number for a diagnostic entry */
#define MONLINESIZ      1600  /* monitor line size */

/*
** Define the number of lines in monitor history buffer.
** This number must be 1 greater than the number of lines you wish
** displayed by the history command, the current monitor item takes
** one buffer.
*/
#define NUMMONHIST    17

/*
** This structure defines the layout of the non-volatile RAM
** on the board.  This area is used to store system information.
** The tables, environment and alias are checksummed.
** They must be contiguous with the environment table first and
** the checksum variable last.
*/

struct nvram {                  /* the first item must not be moved! */
  long testarea;                /* transient area for testing, must be 1st */
  long magic;                   /* NVMAGIC */
  char condev, pollcon;         /* console flags */
  unsigned char conbaud;        /* the console baud rate */
  unsigned char auxbaud;        /* the auxillary port baud rate */
  unsigned char killchar;       /* the kill line character */
  unsigned char erasechar;      /* the erase character */
  unsigned char pad0[2];        /* pad for future expansion */
  unsigned short diagflag;      /* LOOPONERR | STOPONERR | CONTINUOUS, etc. */
  unsigned short monhistcount;  /* monitor valid history count */
  unsigned short pad1[1];       /* pad for future expansion */
  unsigned long srcaddr;        /* source address for the menu diags */
  unsigned long destaddr;       /* destination address for the menu diags */
  unsigned long start;          /* start address for the menu diags */
  unsigned long length;         /* length for the menu diags */
  unsigned long value;          /* data value for the menu diags */
  unsigned long trigger;        /* scope trigger address */
  unsigned long memopsiz;            /* operation size for memory routines */
  char *errlogptr;              /* pointer to the end of the error log */
  char evartbl[EVTSIZ];         /* our monitor environment table */
  char aliastbl[ALSSIZ];        /* our monitor alias table */
  unsigned char pad2[4];        /* pad to longword boundary */
  struct cmdhist {              /* our command history data */
    unsigned short histnum;
    char cmdbuf[MONLINESIZ];
  } cmdhist[NUMMONHIST];
  struct cmdhist *curhistptr;   /* current history slot */
  char diaglist[160];           /* storage for list of menu indices */
  short chksum;                 /* area for table checksum */
};

extern struct nvram nvram;
extern struct nvram *nvram_ptr;
extern unsigned long diagflag_xram; /* ram global for additional diag flags */

/* defines for hflags (housekeeping flags) */
#define H_USRINT        0x01        /* the user interrupt flag */
#define H_INCFILL       0x02        /* incrementing fill flag */
#define H_MORE          0x04        /* pagination flag */
#define H_BUSERR        0x80        /* bus error flag */

/* defines for envflag */
#define INPONCT         0x01        /* in power-on confidence test */
#define INRESET         0x02        /* in reset test */
#define INDIAG          0x04        /* in the menu driven diagnostics */
#define INMON           0x08        /* in the monitor */
#define INWARMBOOT      0x100       /* in a warm boot */

/* defines for diagflag */
#define D_STOPONERR     0x01        /* stop when an error occurs */
#define D_LOOPONERR     0x02        /* loop when an error occurs */
#define D_CONTINUOUS    0x04        /* restart diag upon completion */
#define D_QUIETMODE     0x08        /* quiet all messages */
#define D_NESTED        0x10        /* this is a nested diagnostic */
#define D_VERBOSE       0x20
#define D_ABBR_TEST     0x40
#define D_EXT_LOOPBACK  0x80

/* defines for diagflag_xram */
#define D_SET_OPTIONS   0x00000001  /* test interacts with user for options */
#define D_TRACE         0x00000002  /* display for tracing code flow */
#define D_WARNING       0x00000004  /* display accumulated warnings */
#define D_PR_TASKSWAPS  0x00000008  /* print multitasking task swaps */
#define D_MIN_TEST_TIME 0x00000010  /* minimize test time of diag */

/* defines for teststat */
#define TS_CONPORT      0x01        /* console port */
#define TS_AUXPORT      0x02        /* auxilliary port */

extern void nvraminit(void);
extern int sync(int argc, char *argv[]);
extern int savenv(void);
extern short chksum(unsigned short *addr, int size);
//extern int nvprotect(int argc, char *argv[]);

#endif /* APEX_ZETA */
#endif /* SHIN */
#endif /* APEX_VEGA */
#endif /* BRYCE */
#endif /* CVM_XFORMERS */
#endif /* FSL_XFORMERS */
#endif /* CVM_THREEGORGES */
#endif /* INFORMERS */
#endif /* MIRAGE */
#endif /* PANOPTES */
#endif /* EAGLEEYE */

#endif /* __NVSYSVARS_H__ */

/* end of module */

/******** History ******** 
$Log: nvsysvars.h,v $
Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

