/* $Id: madPlatformDeps.h,v 1.1 2015/02/13 11:35:39 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/phy/madPlatformDeps.h,v $
 *------------------------------------------------------------------
 *
 * madPlatformDeps.h - platform dependent definitions
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "madCopyright.h"



/********************************************************************************

* madPlatformDeps.h

*

* DESCRIPTION:

*       platform dependent definitions

*

* DEPENDENCIES:   Platform.

*

* FILE REVISION NUMBER:

*

*******************************************************************************/



#ifndef __madPlatformDepsh

#define __madPlatformDepsh



#include "madApi.h"



#ifdef __cplusplus

extern "C" {

#endif



MAD_BOOL madDefaultMiiRead (unsigned int portNumber , unsigned int miiReg, unsigned int* value);

MAD_BOOL madDefaultMiiWrite (unsigned int portNumber , unsigned int miiReg, unsigned int value);



#ifdef __cplusplus

}

#endif



#endif   /* madPlatformDepsh */

/*
 *------------------------------------------------------------------
 * $Log: madPlatformDeps.h,v $
 * Revision 1.1  2015/02/13 11:35:39  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
