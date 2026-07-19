/* $Id: jbicomp.h,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/jbicomp.h,v $
 */
/*******************************************************************************
 * Author: Kody Ko Ported from Altera
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 ******************************************************************************/
/****************************************************************************/
/*																			*/
/*	Module:			jbicomp.h												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1997-2001				*/
/*																			*/
/*	Description:	Contains the function prototypes for compressing		*/
/*					and uncompressing Boolean array data.					*/
/*																			*/
/****************************************************************************/

#ifndef INC_JBICOMP_H
#define INC_JBICOMP_H

#if PORT==DOS

void jbi_uncompress_page
(
	int variable_id,
	int page,
	int version
);

#else

unsigned int jbi_uncompress
(
	unsigned char *in, 
	unsigned int in_length,
	unsigned char *out, 
	unsigned int out_length,
	int version
);

#endif /* PORT==DOS */

#endif /* INC_JBICOMP_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: jbicomp.h,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.2  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
