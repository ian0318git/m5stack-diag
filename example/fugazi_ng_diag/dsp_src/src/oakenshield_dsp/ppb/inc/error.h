/* $Id: error.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/error.h,v $
 *------------------------------------------------------------------
 * error.h : error defines
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

#ifndef _ERROR_H_
#define _ERROR_H_

#define TESTNAMEBUFSIZ 80

typedef char testnamebuf_t[TESTNAMEBUFSIZ];

extern unsigned long multi_testpass[];
extern unsigned long multi_errcount[];
extern unsigned long multi_err_accum[];
extern unsigned long multi_warncount[];
extern testnamebuf_t multi_testname[];

extern char *errlog_start;
extern char testnamebuf[];
extern unsigned long testpass, errcount, err_accum, warncount, menu_display;
extern char *banner_string;

extern void testname(void), errleds(void), clrline(void), prpass(unsigned long, ...);
extern void logprintf(void), bell(void), clrerrlog(void), scanerrlog(void);
extern int stoponerr(void), dumperrlog(void);
extern void dump_n_flush(void);

void cterr(char, int, const char*, ...);
int cterr_clear_debug(void);
int cterr_add_debug(char *fmtptr, ...);
int cterr_clear_componet(void);
int cterr_add_componet(char *fmtptr, ...);

/* Structure used to hold componet list for cterr */
typedef struct cterr_componet_ {
    unsigned char *comp_name;
    struct cterr_componet_ *next;
} cterr_componet_t;

/* Structure used to hold debug info list for cterr */
typedef struct cterr_debug_ {
    unsigned char *debug_comment;
    struct cterr_debug_ *next;
} cterr_debug_t;

#if defined(INTEL_ICC) || defined(LINUX_APP)
extern void cterr(char, int, char*, ...);
#else
extern void cterr(char, int,const char*, ...);
#endif

#endif /* _ERROR_H_ */

/******** History ******** 
$Log: error.h,v $
Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.3.86.1  2017/03/30 10:25:49  harrchan
Add fpga upgrade utility

Revision 1.3  2012/07/17 20:34:28  srane
cleanup

Revision 1.2  2012/06/28 21:24:38  srane
add variables needed for Host-DSP READY message exchange.

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

