/* $Id: monitor.h,v 1.15 2021/06/02 07:42:56 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/monitor.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

#ifndef __MONITOR_H__
#define __MONITOR_H__

#if defined(OVERLORD) || defined(FORTITUDE) || defined(PRINCE) || defined(WOODLAWN) || defined(WALLANDER) \
    || defined(TACHI) || defined(TACHI_INTEL) || defined(TSN) || defined(VIPER) || defined(NUTELLA) \
    || defined(TABEIL) || defined(NANOOK) || defined(HIGHRISE) || defined(PHOENIX) || defined(FUGAZI)


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

/*
** Define the number of lines in monitor history buffer.
** This number must be 1 greater than the number of lines you wish
** displayed by the history command, the current monitor item takes
** one buffer.
*/
#define NUMMONHIST    17
#define MONLINESIZ  1600

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

#endif  /* SHIN, CVM_XFORMERS, INFORMERS */

extern volatile unsigned char envflag, hkeepflags;

extern void monitor(int reset);
extern void shift(int count, int *argcp, char *argv[]);
extern int help();
extern int gdb_cntrl(int argc, char *argv[]);
extern int rom_reload();
extern int show_diag_ver(void);

#endif /* __MONITOR_H__ */
/* end of module */


/******** History ******** 
$Log: monitor.h,v $
Revision 1.15  2021/06/02 07:42:56  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.14  2021/04/14 09:10:13  achiu2
[PRRQ:CSCvx56970-2] Phoenix code review for ER

Revision 1.13  2020/08/19 09:48:59  markzha
*** empty log message ***

Revision 1.12  2019/12/11 10:10:22  lucywang
Merged Nanook to main trunk

Revision 1.11  2019/10/17 02:16:15  kehuang2
Collapse Tabei-L into main trunk

Revision 1.10  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.9.40.1  2019/01/25 02:11:06  harrchan
Add definition of NUTELLA

Revision 1.9  2018/08/06 02:31:00  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.8  2017/08/02 14:21:28  steja
Support TSN-H/M platform code

Revision 1.7.22.1  2017/07/29 03:40:43  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.7  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.6.4.2  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

Revision 1.6.4.1  2015/06/11 02:01:04  tirawan
Add files for Tachi BMC project

Revision 1.6  2015/02/26 07:27:14  xiaoyizh
Add Wallander support.

Revision 1.5  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.4  2013/10/08 08:48:26  tirawan
Woodlawn collapsed to main trunk

Revision 1.3  2013/04/23 07:28:36  xiaoyizh
Add Prince support.

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

