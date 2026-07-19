/* $Id: monitor.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/monitor.h,v $
 *------------------------------------------------------------------
 * monitor.h
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

#ifndef __MONITOR_H__
#define __MONITOR_H__

#if defined(SIDEWINDER) || defined(CVM_XFORMERS) || defined(FSL_XFORMERS) || \
    defined(SHIN) || defined(BRYCE) || defined(PANOPTES) || defined(MIRAGE) || \
    defined(APEX_VEGA) || defined (APEX_ZETA) || defined(EAGLEEYE) || \
    defined(INFORMERS) || defined(CVM_THREEGORGES)

/*
** defines for the monitor command table
** including monitor flags
*/
/* must be privileged to view or exec privileged commands */
#define MF_PRIVILEGED 1
/* hidden commands never get displayed but can be exec'd anytime */
#define MF_HIDDEN     2

/* define command line size */
#include "mon_boot.h"

#if !defined(ATLANTIS)

struct monitem {
  char *command;
  int (*procedure)();
  char *description;

#if defined(NOTYET) || defined(SHIN) || defined(PANOPTES) || \
    defined(MIRAGE) || defined(EAGLEEYE)
  int flags;
#endif
};

/*
** Define the number of lines in monitor history buffer.
** This number must be 1 greater than the number of lines you wish
** displayed by the history command, the current monitor item takes
** one buffer.
*/
#define NUMMONHIST    17
#define MONLINESIZ  LINESIZE  /* monitor line size */

#if !defined(SIDEWINDER)
struct cmdhist {          /* our command history data */
    unsigned short histnum;
    char cmdbuf[MONLINESIZ];
};
#endif

extern struct monitem moncmd[];
#endif

extern int moncmdsiz;

/* defines for hkeepflags (housekeeping flags) */
#define H_USRINT        0x01        /* the user interrupt flag */
#define H_INCFILL       0x02        /* incrementing fill flag */
#define H_MORE          0x04        /* pagination flag */
#define H_PRIVILEGED    0x08        /* privileged mode */
#define H_SCRIPT        0x10        /* processing a monsh script */
#define H_BUSERR        0x80        /* bus error flag */

/* defines for envflag */
#define INPONCT         0x01        /* in power-on confidence test */
#define INRESET         0x02        /* in reset test */
#define INDIAG          0x04        /* in the menu driven diagnostics */
#define INMON           0x08        /* in the monitor */
#define INWARMBOOT      0x100       /* in a warm boot */

#endif  /* SHIN, CVM_XFORMERS, INFORMERS */

extern volatile unsigned char envflag, hkeepflags;

extern void monitor(int reset);
extern void shift(int count, int *argcp, char *argv[]);
extern int help(void);
extern int gdb_cntrl(int argc, char *argv[]);
extern int rom_reload(void);
extern int show_diag_ver(void);

#endif /* __MONITOR_H__ */
/* end of module */


/******** History ******** 
$Log: monitor.h,v $
Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

