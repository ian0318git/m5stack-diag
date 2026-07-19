/* $Id: dnld.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/dnld.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

/*
** Defines for the download mechanism
*/

/*
** The following are the target handshake characters.
** The target sends the appropriate char to the host to confirm
** reciept of a packet or to inform of other status.
*/
#define DNLD_HNDSHK_START    '~'  /* target start character */
#define DNLD_HNDSHK_ACK      '+'  /* packet recieved ok */
#define DNLD_HNDSHK_RESEND   '-'  /* resend a packet */
#define DNLD_HNDSHK_ABORT    '!'  /* abort the transmission */

/* general defines */
#define DNLD_MAXRETRIES      5    /* packet retry count */
#define DNLD_MAXPACKET       256  /* maximum packet size */

/* flag for datacount field to indicate 32 bit address field */
#define DNLD_32BITFLG        0x8000

/* host sends to target in address field of termination record to abort */
#define DNLD_ABORT_ADDR      -1

/*
** defines for the host side state machine
*/
typedef enum {
    DNLD_IDLE,         /* not doing a download */
    DNLD_STARTWAIT,    /* waiting for a start from the target */
    DNLD_ACKWAIT,      /* waiting for an ack from the target */
    DNLD_SENDPKT,      /* send the packet in the filebuf */
    DNLD_SENDNEXTPKT,  /* read and send the next packet */
    DNLD_SENDLASTPKT,  /* send this packet and quit */
    DNLD_LASTACKWAIT,  /* wait for last ack */
} DNLD_STATE;

#ifndef UNIX
/* dnld.c */
extern int dnld(int argc, char *argv[]);
extern int download(char *progname, int verbose);
#endif

/******** History ******** 
$Log: dnld.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
