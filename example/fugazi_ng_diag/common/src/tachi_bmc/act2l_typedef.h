/* $Id: act2l_typedef.h,v 1.2 2016/04/20 11:25:24 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/act2l_typedef.h,v $
 *----------------------------------------------------------------------------
 * act2l_typedef.h  Support for ACT2/Ruby API code.
 *
 * May 2011: Alan O'Sullivan  
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
 * All rights reserved.
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
#define AUTHENTICATION_CREDENTIAL u1  // For declaring arrays
#define PAUTHENTICATION_CREDENTIAL p_u1

#define KEY_OBJECT_ID u4
#define PKEY_OBJECT_ID p_u4

#define DATA_BLOCK u1
#define PDATA_BLOCK p_u1

#define PTOTAL_REMAINING p_u2
#define SEQUENCE_COUNTER u1

#endif

/******** History ********
$Log: act2l_typedef.h,v $
Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/24 03:39:35  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/
