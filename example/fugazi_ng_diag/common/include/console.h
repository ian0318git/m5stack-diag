/* $Id: console.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/console.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __CONSOLE_H_
#define __CONSOLE_H_

#include "s2681.h"

extern unsigned char UARTspeeds[];
extern char caphexnums[];

#define uartaddr	((struct uartdevice *)ADRSPC_DUART)

struct uartdevice {
    struct regpair a;		/* channel a */
    struct regpair b;		/* channel b */
};

#define PUTHEXNUM(hexnum) \
    PUTDIGIT(hexnum,28); \
    PUTDIGIT(hexnum,24); \
    PUTDIGIT(hexnum,20); \
    PUTDIGIT(hexnum,16); \
    PUTDIGIT(hexnum,12); \
    PUTDIGIT(hexnum,8); \
    PUTDIGIT(hexnum,4); \
    PUTDIGIT(hexnum,0);

/* console.c */
extern void conout(char c);
extern int conin();
extern int constat();
//extern char getchar();
//extern void putchar(char c);
extern void setmore();
extern long conint();
extern void default_break();
extern int conbrkinttest();
extern int testbrkint();

/* auxport.c */
extern void initaux(); 
extern void auxout(int c);
extern int auxin();
extern int auxstat();

/* auxtest.c */
extern int auxecho();
extern int auxloopback();
extern int auxinttest();
extern long auxint();


#endif /* __CONSOLE_H_ */
/* End of Module */

/******** History ******** 
$Log: console.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
