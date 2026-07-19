/* $Id: act2l_typedef.h,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/act2l_typedef.h,v $
 *------------------------------------------------------------------
 * 
 * act2l_typedef.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __ACT2L_TYPEDEF_H__
#define __ACT2L_TYPEDEF_H__

#define IN
#define OUT
#define STATIC 

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long  long u8;

typedef unsigned char * p_u1;
typedef unsigned short * p_u2;
typedef unsigned int * p_u4;
typedef unsigned long long * p_u8;

typedef unsigned int ACT2_STATUS;
typedef unsigned char BOOL;


/*Not sure if this is the right place. But base.h seems not to be the right
 * place*/

#define USER_ID u1
#define PUSER_ID p_u1
#define SESSION_ID u4
#define PSESSION_ID p_u4
#define AUTHENTICATION_CREDENTIAL u1  /* For declaring arrays */
#define PAUTHENTICATION_CREDENTIAL p_u1

#define KEY_OBJECT_ID u4
#define PKEY_OBJECT_ID p_u4

#define DATA_BLOCK u1
#define PDATA_BLOCK p_u1

#define PTOTAL_REMAINING p_u2
#define SEQUENCE_COUNTER u1

#endif

/*-------------------------------------------------
 * $Log: act2l_typedef.h,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:50  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
