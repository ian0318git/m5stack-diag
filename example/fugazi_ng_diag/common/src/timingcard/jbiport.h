/* $Id: jbiport.h,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/jbiport.h,v $
 */
/*******************************************************************************
 * Author: Kody Ko Ported from Altera
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 ******************************************************************************/
/****************************************************************************/
/*                                                                          */
/*  Module:        jbiport.h                                                */
/*                                                                          */
/*                 Copyright (C) Altera Corporation 2000-2001               */
/*                                                                          */
/*  Description:   Defines porting macros                                   */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JBIPORT_H
#define INC_JBIPORT_H

/*
*  PORT defines the target platform: DOS, WINDOWS, UNIX, or EMBEDDED
*
*  PORT = DOS      means a 16-bit DOS console-mode application
*
*  PORT = WINDOWS  means a 32-bit WIN32 console-mode application for
*                  Windows 95, 98, 2000, ME or NT.  On NT this will use the
*                  DeviceIoControl() API to access the Parallel Port.
*
*  PORT = UNIX     means any UNIX system.  BitBlaster access is support via
*                  the standard ANSI system calls open(), read(), write().
*                  The ByteBlaster is not supported.
*
*  PORT = EMBEDDED means all DOS, WINDOWS, and UNIX code is excluded. 
*                  Remaining code supports 16 and 32-bit compilers. 
*                  Additional porting steps may be necessary. See readme 
*                  file for more details.
*/

#define DOS      2
#define WINDOWS  3
#ifdef UNIX_PLATFORM
#define UNIX     4
#endif
#define EMBEDDED 5

#ifndef PORT
/* change this line to build a different port */
#define PORT EMBEDDED
#endif

#endif /* INC_JBIPORT_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: jbiport.h,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
