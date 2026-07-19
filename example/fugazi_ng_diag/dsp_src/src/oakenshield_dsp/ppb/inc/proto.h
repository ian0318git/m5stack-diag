/* $Id: proto.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/proto.h,v $
 *------------------------------------------------------------------
 * proto.h
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

#ifndef __PROTO_H__
#define __PROTO_H__

#ifndef LINUX_APP
#include "setjmps.h"
#endif

/*
** prototype file, external variables are in extern.h
*/

/* berrtest.c */
extern int berrtest(void);
extern int chkberr(volatile unsigned int *address, int size, unsigned readonly);

/* builtin.c */
extern int echo(int argc,char *argv[]);
extern int unset(int argc, char *argv[]);
extern int setalias(int argc, char *argv[]);
extern int msleep_cmd(int argc, char *argv[]);
extern int reset(int argc, char *argv[]);
extern int wrloop(int argc, char *argv[]);

/* cache.c */
extern int cachetest(int argc, char *argv[]);

/* diagtools.c */
extern int pstat(unsigned int *location, int size, int eq,
		    unsigned int mask, unsigned int cmpval, int msecs, 
		    void *retptr);
extern int pstat_le(unsigned int *location, int size, int eq,
		    unsigned int mask, unsigned int cmpval, int msecs, 
		    void *retptr);

/* epromtest.c */
extern int epromtest(void);

/* main.c */
extern int main(void);

/* main_net.c */
extern int _start (int code, unsigned char *arg);

/* memops.c */
extern int cmp_mem(int argc, char *argv[]);
extern int cmpbyte(unsigned char *addr0, unsigned char *addr1, int length);
extern int cmpword(unsigned short *addr0, unsigned short *addr1, int length);
extern int cmplword(unsigned *addr0, unsigned *addr1, int length);
extern int mov_mem(int argc, char *argv[]);
extern void movbyte(unsigned char *addr0, unsigned char *addr1, int length);
extern void movword(unsigned short *addr0, unsigned short *addr1, int length);
extern void movlword(unsigned *addr0, unsigned *addr1, int length);
extern int fil_mem(int argc, char *argv[]);
extern void filbyte(unsigned char *addr, int length, unsigned char val);
extern void filword(unsigned short *addr, int length, unsigned short val);
extern void fillword(unsigned int *addr, int length, unsigned int val);
extern int dis_mem(int argc, char *argv[]);
extern void dismem(unsigned char *addr, int length, 
		   unsigned long disaddr, int fldsize);
extern int alt_mem(int argc, char *argv[]);
extern int jump(int argc, char *argv[]);
extern int call(int argc, char *argv[]);
extern int memdebug(int argc, char *argv[]);
extern int memtest(int argc, char *argv[]);
extern int memloop(int argc, char *argv[]);
extern void addrtest(unsigned char *addr, int length);
extern int addrloop(int argc, char *argv[]);
extern int berrscan(int argc, char *argv[]);
extern int paritytest(int argc, char *argv[]);
extern void quitmsg(void);
extern int memory_checksum(int argc, char *argv[]);
extern int flush_io_wb(void);
extern void *malloc_dev(unsigned long nbytes);

/* regtest.c */
extern int reg_test(unsigned char *regname, unsigned int *regaddr, 
		    unsigned char size, unsigned int mask, 
		    unsigned char *errmsg);

/* tcalibrate.c */
extern long timer_calibrate (long t); 
extern int time_it (int cnt);
extern void wastetime (long n);
extern int tcal(int argc, char *argv[]);

/* timer.c */
extern void msleep(int msecs);
extern void scopetimer(void);
extern int  t_disable(void);
extern unsigned int get_cpu_clock(void);

extern char atoh(char );
#ifndef LINUX_APP
extern int  get_line(char *buffer, int bufsiz);
extern int iisdigit (char i);
extern int is_space (char c);
extern int  alarm(int cnt);
extern int  getopt(int argc, char *argv[], char *optstr);
extern void srand(unsigned long data);
extern unsigned long rand(void);
#endif /* LINUX_APP */
extern int  getnum(char *cptr, int base, unsigned int *longret);
extern int  getnnum(char *cptr, int base, unsigned int *longret, int maxchars);
extern int strncasecmp (const char *s1, const char *s2, unsigned long length);
extern int strcasecmp (const char *s1, const char *s2);

/*--------------------------------------------------------------------*/
/* Cancun routines                                                    */
/*--------------------------------------------------------------------*/

/* sizemem.c */
extern long shmemstart(void);
extern long sizeshmem(void);

#endif /* __PROTO_H__ */
/* End of File */

/******** History ******** 
$Log: proto.h,v $
Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/07/17 20:34:28  srane
cleanup

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

