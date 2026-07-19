/* $Id: proto.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/proto.h,v $
 *------------------------------------------------------------------
 * proto.h Definitions file for proto.
 *
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PROTO_H__
#define __PROTO_H__

#include "types.h"
/*
** prototype file, external variables are in extern.h
*/

/* berrtest.c */
extern int berrtest();
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
extern int epromtest();

/* main.c */
extern int main();

/* main_net.c */
extern int _start (int code, unsigned char *arg);

/* memops.c */
extern int cmp_mem(int argc, char *argv[]);
extern int cmpbyte(unsigned char *addr0, unsigned char *addr1, int length);
extern int cmpword(unsigned short *addr0, unsigned short *addr1, int length);
extern int cmplword(unsigned *addr0, unsigned *addr1, int length);
extern int cmpdlword(unsigned long *addr0, unsigned long *addr1, int length);
extern int mov_mem(int argc, char *argv[]);
extern void movbyte(unsigned char *addr0, unsigned char *addr1, int length);
extern void movword(unsigned short *addr0, unsigned short *addr1, int length);
extern void movlword(unsigned *addr0, unsigned *addr1, int length);
extern void movdlword(unsigned long *addr0, unsigned long *addr1, int length);
extern int fil_mem(int argc, char *argv[]);
extern void filbyte(unsigned char *addr, int length, unsigned char val);
extern void filword(unsigned short *addr, int length, unsigned short val);
extern void fillword(unsigned int *addr, int length, unsigned int val);
extern void fildlword(unsigned long *addr, int length, unsigned long val);
extern int dis_mem(int argc, char *argv[]);
extern void dismem(unsigned char *addr, int length, 
		   unsigned long disaddr, int fldsize);
extern int alt_mem(int argc, char *argv[]);
extern int jump(int argc, char *argv[]);
extern int call(int argc, char *argv[]);
extern int memdebug(int argc, char *argv[]);
extern int memtest(int argc, char *argv[]);
extern int memloop(int argc, char *argv[]);
extern void addrtest(unsigned long , unsigned long , unsigned long);
extern int addrloop(int argc, char *argv[]);
extern int berrscan(int argc, char *argv[]);
extern int paritytest(int argc, char *argv[]);
extern void quitmsg();
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
extern void scopetimer();
extern int  t_disable();
extern unsigned int get_cpu_clock(void);

extern int  getnum(char *cptr, int base, unsigned int *longret);
extern int  getnnum(char *cptr, int base, utype_t *longret, int maxchars);


/*--------------------------------------------------------------------*/
/* Cancun routines                                                    */
/*--------------------------------------------------------------------*/

/* sizemem.c */
extern long shmemstart();
extern long sizeshmem();

#endif /* __PROTO_H__ */
/* End of File */

/******** History ********/ 
/*
 * $Log: proto.h,v $
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

