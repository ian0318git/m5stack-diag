/* $Id: nvmonvars.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/nvmonvars.h,v $
 *------------------------------------------------------------------
 *
 * Filename:	nvmonvars.h
 *
 * Paul Tong - Apr. 2012.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#ifndef __NVMONVARS_H__
#define __NVMONVARS_H__

/*
** This structure defines the layout of the monitor portion of the 
** memory, which may be in non-volatile storage or RAM. This area is
** used to store monitor information.
*/
#define EVTSIZ           768  /* the environment table size */
#define ALSSIZ           512  /* the alias table size */
#define NVMAGIC    -17958194  /* magic number for the NV RAM */

struct nvram {                  /* the first 6 items must not be moved! */
  long testarea;                /* transient area for testing, must be 1st */
  short autobootstate;          /* for the autoboot state machine */
  short autobootcount;          /* retry counter for the autoboot mechanism */
  char *autobootsptr;           /* string pointer for the autoboot mechanism */
  unsigned short confreg;       /* our virtual configuration register */
  unsigned short rconfreg;      /* our 1s complement virtual conf reg */
  long magic;                   /* NVMAGIC (checksumming starts here) */
  char condev, pollcon;         /* console flags */
  unsigned char conbaud;        /* the console baud rate (future use) */
  unsigned char auxbaud;        /* the auxillary port baud rate (future) */
  unsigned char killchar;       /* the kill line character (future)*/
  unsigned char erasechar;      /* the erase character (future) */
  unsigned char diagflag;       /* LOOPONERR | STOPONERR | CONTINUOUS, etc. */
  unsigned char iomempercent;   /* main mem % allocated for packet mem */
  unsigned char pad0[4];        /* pad for future expansion */
  unsigned short pad1[4];       /* for future expansion */
  int memopsiz;                 /* operation size for memory routines */
  unsigned long srcaddr;        /* source address for the menu diags */
  unsigned long destaddr;       /* destination address for the menu diags */
  unsigned long start;          /* start address for the menu diags */
  unsigned long length;         /* length for the menu diags */
  unsigned long value;          /* data value for the menu diags */
  unsigned long trigger;        /* scope trigger address */
#if 0  
  /* this is being removed because we are getting a warning */
  /* about BOOTDATA being multiply defined.  It is also defined */
  /* in mon_boot.h and the definitions to not match - since this*/
  /* field is not used at present - I will leave it to the      */
  /* implementor of the code that uses this field to figure out */
  /* what BOOTDATA needs to be                                  */
  long sys_ret_info[BOOTDATA+1];/* contains data about last reboot */
#endif
  unsigned long pad2[4];        /* for future expansion */
  char diaglist[64];            /* storage for our diag groups */
  char evartbl[EVTSIZ];         /* our monitor environment table */
  char aliastbl[ALSSIZ];        /* our monitor alias table */
  unsigned long pad3[4];        /* for future expansion */
  short chksum;                 /* area for table checksum - must be last */
};

extern struct nvram nvram;          /* the in-core copy of the structure */
extern struct nvram *nvram_ptr;     /* pointer to NVRAM copy of the structure */
extern unsigned long diagflag_xram; /* ram global for additional diag flags */

#define NVRAM           (&nvram)
#define NVRAM_SAV       (nvram_ptr)

/*
** defines for fields within the nvram structure
**
** This was done for portability.  Not all platforms have the same
** nvram structure - some are restricted to using a minimal amount
** of memory.  These defines are used in the C modules for those
** fields which may not be in every platforms nvram structure.
*/
#define ALIAS_TABLE     (NVRAM)->aliastbl
#define CONDEV          (NVRAM)->condev
#define DIAGFLAG        (NVRAM)->diagflag
#define MEMOP_SIZE      (NVRAM)->memopsiz
#define MEMOP_SRCADDR   (NVRAM)->srcaddr
#define MEMOP_DESTADDR  (NVRAM)->destaddr
#define MEMOP_LENGTH    (NVRAM)->length
#define MEMOP_START     (NVRAM)->start
#define MEMOP_VALUE     (NVRAM)->value
#define MEMOP_TRIGGER   (NVRAM)->trigger
#define IOMEM_PERCENT   (NVRAM)->iomempercent
#define CURHISTPTR      g_curhistptr
#define MONHISTCOUNT    g_monhistcount
#define CMDHIST         commandhistory
#define ERRLOGPTR       g_errlogptr

#define NV_RDSHORT(addr) ((((short)(*(char *)(addr)) << 8) & 0xff00) |  \
                          ((short)(*(char *)((int)addr+1)) & 0x00ff))
#define NV_RDLONG(addr) ((((long)(*(char *)(addr)) << 24) & 0xff000000) |    \
                         (((long)(*(char *)((int)addr+1)) << 16) & 0x00ff0000)  | \
                         (((long)(*(char *)((int)addr+2)) << 8) & 0x0000ff00)   |  \
                         ((long)(*(char *)((int)addr+3)) & 0x000000ff)) 


#define NV_CONFREG     NV_RDSHORT(&nvram_ptr->confreg)
#define NV_MAGIC       NV_RDLONG(&nvram_ptr->magic)
#define NV_RCONFREG    NV_RDSHORT(&nvram_ptr->rconfreg)

/* Macro for determining if the confreg is valid in NVRAM */
#define NV_VALID_CONFREG ((NVRAM_SAV)->magic == NVMAGIC && (NVRAM_SAV)->confreg == (unsigned short)(~(NVRAM_SAV)->rconfreg))
#define NV_WR_CONFREG(val) nv_wr_confreg(val)

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

/* related prototypes */    
extern char *g_errlogptr;     /* pointer to the end of the error log */
extern int nvramvalid(void);
extern void nvraminit(void);
extern int savenv(void);
extern int nvprotect(int argc, char *argv[]);
extern void nv_wr_confreg(unsigned short val);

/* alias.c */
extern char *getvar(char *tblptr, char *vname);
extern int setvar(char *tblptr, int tblsiz, char *strptr);
extern int unsetvar(char *tblptr, char *var);
extern int printtbl(char *tblptr);

#endif  /* __NVMONVARS_H__ */

/* ----------------- End of File -----------------*/
/* ------ History ------------ 
$Log: nvmonvars.h,v $
Revision 1.2  2013/10/08 08:48:30  tirawan
Woodlawn collapsed to main trunk

Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
Branch into woodlawn-branch2 and port woodlawn code

Revision 1.1.2.1  2013/04/24 10:37:23  tirawan
Initial check-in for woodlawn linux code

Revision 1.1  2013/03/13 06:43:00  kuangik
Add for the first time

Revision 1.5  2012/08/03 10:16:56  evanli
Mapping to latest O2 source code on 20120726

Revision 1.3  2012/07/25 01:34:39  leslie
Recover to previous revision 1.1.1.1

Revision 1.1.1.1  2012/02/10 05:59:50  kody
Initial imports Woodlawn project code base.

Revision 1.1.2.2  2011/06/01 19:23:29  mcharon
fix warnings

Revision 1.1.2.1  2011/03/24 06:59:42  mcharon
create overlord

Revision 1.1.2.1  2011/03/11 22:33:46  mcharon
iniitail support dyno

Revision 1.1.1.1  2009/10/17 02:05:54  huyhoang
Initial archive of diaglinux module

Revision 1.1.8.2  2009/06/04 09:37:32  sctsai
Sync with informers2-tag-060209 repository.

Revision 1.1.6.1  2009/02/18 02:57:06  sctsai
Sync informers-tag-021609 to informers2-branch.

Revision 1.1.2.4  2009/01/24 04:25:55  shhuang
+ Correct previous check-in comment as
+ Update according to the new layout of boot flash and nvram.

Revision 1.1.2.2  2008/12/08 03:19:58  shhuang
+ Added function description and clean up.

Revision 1.1.2.1  2008/06/02 17:45:25  shhuang
+ Initial check-in.

$Endlog$
*/

