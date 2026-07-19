/* $Id: diag_ver.h,v 1.2 2012/10/04 23:36:15 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/diag_ver.h,v $
 *------------------------------------------------------------------
 * diag_ver.h
 *      Graffham firmware version
 *
 * Mar 2012, Smita Rane
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _DIAG_FW_VER
#define _DIAG_FW_VER

#define DIAGFW_MAJ_REL     atoi(MAJOR)  /* official rel; major function change */
#define DIAGFW_MIN_REL     atoi(MINOR)  /* pre-official, minor fix */
#define DIAGFW_DEBUG_VER   atoi(BUILD)  /* debug version used for the same release */

#endif /* _DIAG_FW_VER */

/******** History ********
$Log: diag_ver.h,v $
Revision 1.2  2012/10/04 23:36:15  srane
Add support for SP2702. Version control.

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

