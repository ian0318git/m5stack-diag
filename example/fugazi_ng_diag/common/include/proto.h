/* $Id: proto.h,v 1.4 2013/05/02 17:27:48 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/proto.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
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
extern unsigned char swapbyte(unsigned char c);


/*--------------------------------------------------------------------*/
/* Cancun routines                                                    */
/*--------------------------------------------------------------------*/

/* sizemem.c */
extern long shmemstart();
extern long sizeshmem();

#endif /* __PROTO_H__ */
/* End of File */

/******** History ******** 
$Log: proto.h,v $
Revision 1.4  2013/05/02 17:27:48  mcharon
move ttf2array to linux_api.c

Revision 1.3  2012/08/14 09:46:38  alpeng
support CLI cmd addrloop

Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
