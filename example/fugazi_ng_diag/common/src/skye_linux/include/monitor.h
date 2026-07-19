/* $Id: monitor.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/monitor.h,v $
 *------------------------------------------------------------------
 *
 * monitor.h : Defines for the monitor command table
 *
 * May 09, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 */

#ifndef __MONITOR_H__
#define __MONITOR_H__

#if defined(LINUX_APP)

/*
** defines for the monitor command table
** including monitor flags
*/
/* must be privileged to view or exec privileged commands */
#define MF_PRIVILEGED 1
/* hidden commands never get displayed but can be exec'd anytime */
#define MF_HIDDEN     2


struct monitem {
  char *command;
  int (*procedure)();
  char *description;
};

/* define the Command Line size on monitor and Argument size 
 * Minimum size for 1551
 */
#define LINESIZE 1600
/*
** Define the number of lines in monitor history buffer.
** This number must be 1 greater than the number of lines you wish
** displayed by the history command, the current monitor item takes
** one buffer.
*/
#define NUMMONHIST    17
#define MONLINESIZ  LINESIZE  /* monitor line size */

struct cmdhist {          /* our command history data */
    unsigned short histnum;
    char cmdbuf[MONLINESIZ];
};

extern struct monitem moncmd[];

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

#endif  /* LINUX_APP */

extern volatile unsigned char envflag, hkeepflags;

extern void monitor(int reset);
extern void shift(int count, int *argcp, char *argv[]);
extern int help();
extern int gdb_cntrl(int argc, char *argv[]);
extern int rom_reload();
extern int show_diag_ver(void);

#endif /* __MONITOR_H__ */
/* end of module */

/******** History *********/
/*
 * $Log: monitor.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:27  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:38  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */
