/* $Id: nvmonvars.h,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/nvmonvars.h,v $
 *------------------------------------------------------------------
 *
 * nvmonvars.h - The Monitor's NVRAM structure
 *
 * March 2016, Steja adapted from Xformers.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef _NVMONVARS_H_
#define _NVMONVARS_H_

#include "types.h"
#include "reason.h"

#define PLATFORM_MB_COOKIE_SIZE	0x200

/*
 ** Special nvram cookie stuff.
 ** The Sapphire platform keeps the cookie info in NVRAM as it does
 ** not have a separate cookie PROM or SEEPROM.
 */
struct nvcookie {
    unsigned short magic;
    uchar data[PLATFORM_MB_COOKIE_SIZE];
    unsigned short chksum;
};

/*
 ** This structure defines the layout of the monitor portion of the 
 ** non-volatile RAM on the board.  This area is used to store monitor
 ** information.
 */
#define EVTSIZ           	768     /* the environment table size */
#define ALSSIZ           	512     /* the alias table size */
#define FLTSIZ           	2048    /* Fault history buffer */
#define ROMNVSIZE        	4096    /* size of nvram reserved for rommon */
#define NVMAGIC    		-17958194       /* magic number for the NV RAM */
#define NVCOOKIE_MAGIC   	0x8d74  /* magic number for nvram cookie */
#define VSTRING_BUFF_SIZE       257     /* From IOS boot/buff.h */


struct nvram {                  /* the first 8 items must not be moved! */
    struct nvcookie nvcookie;   /* mfg programs cookie info here */
    long testarea;              /* transient area for testing, must be 1st */
    short autobootstate;        /* for the autoboot state machine */
    short autobootcount;        /* retry counter for the autoboot mechanism */
    char *autobootsptr;         /* string pointer for the autoboot mechanism */
    unsigned short confreg;     /* our virtual configuration register */
    unsigned short rconfreg;    /* our ones complement virtual configuration register */
    long magic;                 /* NVMAGIC (checksumming starts here) */
    unsigned char cur_rommon;   /* currently running rommon - linked to 
                                 * NVRAM_MON_CURMON_OFFSET define in 
                                 * c1800_pcmap.h.  HAS to be at this 
                                 * offset (536) */
    char condev, pollcon;       /* console flags (future use) */
    unsigned char conbaud;      /* the console baud rate (future use) */
    unsigned char auxbaud;      /* the auxillary port baud rate (future) */
    unsigned char killchar;     /* the kill line character (future) */
    unsigned char erasechar;    /* the erase character (future) */
    unsigned char diagflag;     /* LOOPONERR | STOPONERR | CONTINUOUS, etc. */
    unsigned char iomempercent; /* main mem % allocated for packet mem */
    unsigned char passwdprotect;        /* enable/disable password recovery */
    unsigned char pref_rommon;  /* select which rommon to use */
    unsigned char pad0[4];      /* pad for future expansion */
    unsigned char reboot_type;  /* how did we reload? */
    unsigned char *ro_version;  /* banner of readonly rommon */
    unsigned char *upg_version; /* banner of upgrade rommon */
    unsigned long memopsiz;     /* operation size for memory routines */
    unsigned long srcaddr;      /* source address for the menu diags */
    unsigned long destaddr;     /* destination address for the menu diags */
    unsigned long start;        /* start address for the menu diags */
    unsigned long length;       /* length for the menu diags */
    unsigned long value;        /* data value for the menu diags */
    unsigned long trigger;      /* scope trigger address */
    long sys_ret_info[BOOTDATA + 1];    /* contains data about last reboot */
    unsigned long pad2[4];      /* for future expansion */
    char diaglist[64];          /* storage for our diag groups */
    char evartbl[EVTSIZ];       /* our monitor environment table */
    char aliastbl[ALSSIZ];      /* our monitor alias table */
    unsigned long pad3[4];      /* for future expansion */
    char vstring_sys[VSTRING_BUFF_SIZE];        /* save previous booted IOS version */
    char fault_history[FLTSIZ]; /* fault history buffer */
    unsigned long upgrade_rommon_start; /* where upgrade rommon can be burnt */
    unsigned long ios_nvram_start;      /* start of IOS NV config space */
    unsigned long upgrade_checksum;     /* checksum of upgrade rommon image */
    unsigned long pad5[0x20];   /* pad for future expansion; readonly and 
                                 * upgrade need to have similiar structure. 
                                 * Once a readonly version is released, the 
                                 * upgrade rommon can only use the space 
                                 * reserved padx[] for adding  new fields
                                 */
    short chksum;               /* area for table checksum - must be last */
};

extern struct nvram nvram;      /* the in-core copy of the structure */
extern struct nvram *nvram_ptr; /* pointer to NVRAM copy of the structure */
extern struct nvram nvram_sav;  /* in-core copy of the latest in the NVRAM */
extern unsigned long diagflag_xram;     /* ram global for additional diag flags */

#define NVRAM           (&nvram)
#define NVRAM_SAV       (&nvram_sav)

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

/* defines for diagflag */
#define D_STOPONERR     0x01    /* stop when an error occurs */
#define D_LOOPONERR     0x02    /* loop when an error occurs */
#define D_CONTINUOUS    0x04    /* loop when an error occurs */
#define D_QUIETMODE     0x08    /* quiet all messages */
#define D_VERBOSE       0x20
#define D_ABBR_TEST     0x40
#define D_EXT_LOOPBACK  0x80

#endif                          /* _NVMONVARS_H_ */


/*-------------------------------------------------
$Log: nvmonvars.h,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
