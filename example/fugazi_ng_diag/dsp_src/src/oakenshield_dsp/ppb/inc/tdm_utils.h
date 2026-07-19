/* $Id: tdm_utils.h,v 1.3 2021/04/15 00:52:45 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/tdm_utils.h,v $
 *------------------------------------------------------------------------
 * tdm_utils.h - Header file for TDM utils
 *
 * Oct 2016 - Owen Lin
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------
 */

#ifndef TDM_UTILS_H
#define TDM_UTILS_H

#include "libtdm.h"

/* Defines */
#define NUMBER_OF_CHANNEL   256
#define N_BITS_PER_SAMPLE_PER_CH    8
#define NUMBER_OF_SAMPLE_BUFFER 40
#define BUFFERING_OPTION DOUBLE_BUFFERING

#define COLS NUMBER_OF_CHANNEL
#define ROWS NUMBER_OF_SAMPLE_BUFFER
#define SIU_CHANNELS 256				/* number of time slots on tdm bus  */

/* buffer offset */
#define CHANNEL_BUFFER	 ROWS * 2 /* 80 bytes each SWTU buffer */
#define PORT_BUFFER		CHANNEL_BUFFER * 4 /* 4 channels per port */
#define VOICE0_OFFSET	0	 /* the 1st buffer */
#define MONITOR_OFFSET	CHANNEL_BUFFER * 2 /* the 3rd buffer */
#define SI32261_PORT_OFFSET    PORT_BUFFER


/* Function Prototypes */
volatile uint8_t SWTU0_SOURCEBUFFER[2][NUMBER_OF_CHANNEL][NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1)];
volatile uint8_t SWTU0_DESTINATIONBUFFER[2][NUMBER_OF_CHANNEL][NUMBER_OF_SAMPLE_BUFFER*(BUFFERING_OPTION+1)];

#endif /* TDM_UTILS_H */

/************* History ************
$Log: tdm_utils.h,v $
Revision 1.3  2021/04/15 00:52:45  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:33  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.1  2016/12/14 05:03:50  olin2
Initial commit code for Oakenshield





$Endlog$
*/

