/* $Id: queryflags.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/queryflags.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** queryflags.h - defines for query_user() in query.c
**
** Mechanism to query the user for test parameters.
** Note that the order of the assigned bits is critical
** to the proper functioning of the query() routine
** (esp. QU_R_WR, QU_VALUE, and QU_OPSIZ).
*/

typedef unsigned int QUERYFLAG;

#define QU_SOURCE      0x01
#define QU_SOURCE_BIT  0
#define QU_DEST        0x02
#define QU_DEST_BIT    1
#define QU_START       0x04
#define QU_START_BIT   2
#define QU_SIZE        0x08
#define QU_SIZE_BIT    3
#define QU_R_WR        0x10
#define QU_R_WR_BIT    4
#define QU_VALUE       0x20
#define QU_VALUE_BIT   5
#define QU_PASSES      0x40
#define QU_PASSES_BIT  6
#define QU_OPSIZ       0x80
#define QU_OPSIZ_BIT   7
#define QU_INCPAT      0x100
#define QU_INCPAT_BIT  8
#define QU_TRIGGER     0x200
#define QU_TRIGGER_BIT 9
#define QU_ABBREV      0x400
#define QU_ABBREV_BIT  10  
#define QU_PARITY_EN   0x800      /* Cancun only */ 
#define QU_PARITY_EN_BIT  11  

extern char * take_0x_addr(char *);
extern void query_user(QUERYFLAG query, ...);
extern int getc_answer(char *msg, char *cmpstr, char curval);
extern unsigned long gethex_answer(char *msgstr, unsigned long currentval, 
				   unsigned long min, unsigned long max);
extern int getdec_answer(char *msgstr, uint, uint, uint);
extern int getdec_token(char **);

/******** History ******** 
$Log: queryflags.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
