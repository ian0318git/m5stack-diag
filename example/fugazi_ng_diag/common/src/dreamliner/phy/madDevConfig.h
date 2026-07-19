/* $Id: madDevConfig.h,v 1.1 2015/02/13 11:35:39 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/phy/madDevConfig.h,v $
 *------------------------------------------------------------------
 * madDevConfig.h - Includes device specific configurations.
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

* madDevConfig.h

*

* DESCRIPTION:

*       Includes device specific configurations.

*

* DEPENDENCIES:

*       None.

*

* FILE REVISION NUMBER:

*       $Revision: 1.1 $

*

*******************************************************************************/



#ifndef __madDevConfigh

#define __madDevConfigh



#include "madApi.h"



#ifdef __cplusplus

extern "C" {

#endif



/*******************************************************************************

* madDevPreInit

*

* DESCRIPTION:

*       This function configures Marvell Phy Device to function properly.

*       This function applied only once when driver is loaded. 

*

* INPUTS:

*        None.

* OUTPUTS:

*        None.

*

* RETURNS:

*       MAD_OK               - on success, or

*       MAD_FAIL             - otherwise.

*

* COMMENTS:

*

*******************************************************************************/

MAD_STATUS madDevPreInit

(

    IN MAD_DEV    *dev

);



#ifdef __cplusplus

}

#endif



#endif /* __madDevConfigh */

/*
 *------------------------------------------------------------------
 * $Log: madDevConfig.h,v $
 * Revision 1.1  2015/02/13 11:35:39  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
