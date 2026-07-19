/* $Id: tftpclient.c,v 1.6 2014/07/17 23:07:06 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tftpclient.c,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 12/2012
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * tftp/tftp.c --
 *
 * A TFTP (RFC 1350) over IPv4/IPv6 client. This code is intended to
 * demonstrate (i) how to write encoding/decoding functions, (ii) how
 * to implement a simple state machine, and (iii) how to use a
 * select() mainloop to implement timeouts and retransmissions.
 */

#define _POSIX_C_SOURCE 200112L
#define _BSD_SOURCE
#define _DARWIN_C_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * See RFC 1350 section 5 and the appendix.
 */

#define TFTP_OPCODE_RRQ		1
#define TFTP_OPCODE_WRQ		2
#define TFTP_OPCODE_DATA	3
#define TFTP_OPCODE_ACK		4
#define TFTP_OPCODE_ERROR	5

#define TFTP_DEF_RETRIES    6 	
#define TFTP_DEF_TIMEOUT_SEC	1
#define TFTP_DEF_TIMEOUT_USEC	0
#define TFTP_BLOCKSIZE		512
#define TFTP_MAX_MSGSIZE	(4 + TFTP_BLOCKSIZE)

#define TFTP_MODE_OCTET		"octet"
#define TFTP_MODE_NETASCII	"netascii"
#define TFTP_MODE_MAIL		"mail"

#define TFTP_ERR_NOT_DEFINED	0
#define TFTP_ERR_NOT_FOUND	1
#define TFTP_ERR_ACCESS_DENIED	2
#define TFTP_ERR_DISK_FULL	3
#define TFTP_ERR_UNKNOWN_TID	4
#define TFTP_ERR_ILLEGAL_OP	5
#define TFTP_ERR_FILE_EXISTS	6
#define TFTP_ERR_NO_SUCH_USER	7

#define TFTP_STATE_CLOSED	  0
#define TFTP_STATE_RRQ_SENT	  1
#define TFTP_STATE_WRQ_SENT	  2
#define TFTP_STATE_DATA_SENT	  3
#define TFTP_STATE_LAST_DATA_SENT 4
#define TFTP_STATE_ACK_SENT	  5
#define TFTP_STATE_LAST_ACK_SENT  6

/*
 * Structure used to keep track of a TFTP protocol session.
 */

typedef struct tftp {
    char         *host;		/* hostname of the tftp server */
    char         *port;		/* port number / service name */
    char         *mode;		/* tftp transfer mode */
    char         *file;		/* tftp file name */
    char         *local;	/* local file name */
    int           sd;		/* socket descriptor */
    int           fd;		/* file descriptor */
    int		  state;	/* state of the TFTP state machine */
    uint16_t      blkno;	/* current block number */
    struct timeval backoff;
    struct timeval timer;
    struct sockaddr_storage addr;		/* address of the server */
    socklen_t               addrlen;		/* length of the address */
    unsigned char msg[TFTP_MAX_MSGSIZE];	/* tftp msg send buffer */
    size_t        msglen;			/* tftp msg send buffer len */
} tftp_t;

static const char *progname = "tftpc";
static int vflag = 0;

/*
 * Some systems do not define these preprocessor symbols - hence we
 * provide fallback definitions for portability.
 */

#ifndef NI_MAXHOST
#define NI_MAXHOST	1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV	32
#endif

/*
 * A helper function to print messages in verbose mode. An enhanced
 * version would use varargs but we keep it simple for now.
 */

static void
verbose(char *msg)
{
    if (vflag) {
	fprintf(stderr, "%s: %s\n", progname, msg);
    }
}

/*
 * Create a TFTP message according to the state information maintained
 * in the tftp data structure and the parameters provided. Since all
 * TFTP message are very similar, we use a single function to encode
 * all message types.
 */

static size_t
tftp_enc_packet(tftp_t *tftp, const uint16_t opcode, uint16_t blkno,
		unsigned char *data, size_t datalen)
{
    unsigned char *p = tftp->msg;
    size_t len;

    *p = ((opcode >> 8) & 0xff); p++;
    *p = (opcode & 0xff); p++;

    switch (opcode) {
    case TFTP_OPCODE_RRQ:
    case TFTP_OPCODE_WRQ:

	len = strlen(tftp->file) + 1 + strlen(tftp->mode) + 1;
	if (4 + len > TFTP_MAX_MSGSIZE) {
	    verbose("encoding error: filename too long");
	    return -1;
	}

	len = strlen(tftp->file);
	memcpy(p, tftp->file, len + 1);
	p += len + 1;

	len = strlen(tftp->mode);
	memcpy(p, tftp->mode, len + 1);
	p += len + 1;
	break;
	
    case TFTP_OPCODE_DATA:
	*p = ((blkno >> 8) & 0xff); p++;
	*p = (blkno & 0xff); p++;

	if ((4 + datalen) > TFTP_MAX_MSGSIZE) {
	    verbose("encoding error: data too big");
	    return -1;
	}
	memcpy(p, data, datalen);
	p += datalen;
	break;
	
    case TFTP_OPCODE_ACK:
	*p = ((blkno >> 8) & 0xff); p++;
	*p = (blkno & 0xff); p++;
	break;
	
    case TFTP_OPCODE_ERROR:
	/* blkno contains an error code and data is a NUL-terminated
	   string with an error message */
	*p = ((blkno >> 8) & 0xff); p++;
	*p = (blkno & 0xff); p++;

	len = strlen((char *) data);
	if ((4 + len + 1) > TFTP_MAX_MSGSIZE) {
	    verbose("encoding error: error message too big");
	    return -1;
	}
	memcpy(p, data, len + 1);
	p += len + 1;
	break;
    }

    tftp->msglen = (p - tftp->msg);
    return tftp->msglen;
}

/*
 * Utility functions to decode fields from a received TFTP message.
 */

static int
tftp_dec_opcode(unsigned char *buf, size_t buflen, uint16_t *opcode)
{
    if (buflen < 2) {
	return 0xffff;
    }

    *opcode = (buf[0] << 8) + buf[1];
    return 0;
}

static int
tftp_dec_blkno(unsigned char *buf, size_t buflen, uint16_t *blkno)
{
    if (buflen < 4) {
	return 0xffff;
    }

    *blkno = (buf[2] << 8) + buf[3];
    return 0;
}

static int
tftp_dec_data(unsigned char *buf, size_t buflen,
	      unsigned char **data, size_t *datalen)
{
    if (buflen < 5) {
	*data = NULL;
	*datalen = 0;
	return 0xffff;
    }

    *data = buf+4;
    *datalen = buflen - 4;
    return 0;
}

static int
tftp_dec_error(unsigned char *buf, size_t buflen,
	       uint16_t *errcode, char **msg)
{
    int i;
    
    if (buflen < 5) {
	*msg = NULL;
	return 0xffff;
    }

    /* sanity check: the error message must be nul-terminated inside
       of the buffer buf, otherwise the packet is invalid */
    
    for (i = 4; i < buflen; i++) {
	if (buf[i] == 0) break;
    }
    if (i == buflen) {
	verbose("error message is not a nul-terminated string");
	return -1;
    }

    *errcode = (buf[2] << 8) + buf[3];
    *msg = (char *) buf + 4;
    return 0;
}

/*
 * Open a socket for TFTP communication. We save the initial
 * destination address in the tftp data structure.
 */

static void
tftp_socket(tftp_t *tftp)
{
    struct addrinfo hints, *ai_list, *ai;
    int n, fd = 0;

    assert(tftp && tftp->host && tftp->port);
    tftp->sd = -1;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    n = getaddrinfo(tftp->host, tftp->port, &hints, &ai_list);
    if (n) {
        fprintf(stderr, "%s: getaddrinfo: %s\n",
                progname, gai_strerror(n));
        return;  /*ZZZ */
    }

    for (ai = ai_list; ai; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
	    switch (errno) {
	    case EAFNOSUPPORT:
	    case EPROTONOSUPPORT:
		continue;
		
	    default:
		fprintf(stderr, "%s: socket: %s\n",
			progname, strerror(errno));
		continue;
	    }
        }
	memcpy(&tftp->addr, ai->ai_addr, ai->ai_addrlen);
	tftp->addrlen = ai->ai_addrlen;
	break;	/* still here? we were successful and we are done */
    }

    freeaddrinfo(ai_list);

    if (ai == NULL) {
        fprintf(stderr, "%s: could not connect to %s port %s\n",
                progname, tftp->host, tftp->port);
        return; /*ZZZ*/
    }

    tftp->sd = fd;
    tftp->state = TFTP_STATE_CLOSED;
}

/*
 * Cleanup everything. We essentially only close the socket
 * and file descriptors.
 */

static void
tftp_close(tftp_t *tftp)
{
    assert(tftp);
    
    (void) close(tftp->sd);
    (void) close(tftp->fd);
}

/*
 * The mainloop implements the TFTP protocol machine, assuming
 * the initial message (read or write request) has already been
 * encoded into the send buffer.
 */

static int
tftp_mainloop (tftp_t *tftp)
{
    unsigned char buf[TFTP_MAX_MSGSIZE];
    size_t buflen;
    uint16_t opcode, blkno, errcode;
    char *errmsg;
    fd_set fdset;
    int retries;
    int rc = -EXIT_SUCCESS;
    //int total_secs = 0;
    struct timeval timeout, now, prev;

    retries = TFTP_DEF_RETRIES;
    timerclear(&tftp->timer);
    timerclear(&prev);
    while (tftp->state != TFTP_STATE_CLOSED) {

	if (gettimeofday(&now, NULL) == -1) {
	    fprintf(stderr, "%s: gettimeofday: %s\n",
		    progname, strerror(errno));
	    tftp_close(tftp);
	    return -EXIT_FAILURE;
	}

	if (! timerisset(&tftp->timer)
	    || timercmp(&now, &tftp->timer, >)) {
	    if (-1 == sendto(tftp->sd, tftp->msg, tftp->msglen, 0,
			     (const struct sockaddr *) &tftp->addr, tftp->addrlen)) {
		fprintf(stderr, "%s: sendto: %s\n",
			progname, strerror(errno));
		tftp_close(tftp);
		return -EXIT_FAILURE;
	    }
	}

        if (now.tv_sec >= (prev.tv_sec+1)) {
            //printf("Elapsed time tftp:  %d secs\r", total_secs++);fflush(stdout);
            prev.tv_sec = now.tv_sec;
        }
        
	if (tftp->state == TFTP_STATE_LAST_ACK_SENT) {
	    tftp->state = TFTP_STATE_CLOSED;
            /* need to clear prevous message */
            printf("                           \r");fflush(stdout);
	    break;
	}
	
	FD_ZERO(&fdset);
	FD_SET(tftp->sd, &fdset);

	if (! timerisset(&tftp->timer)) {
	    /* Start a new timer with the default interval. */
	    tftp->backoff.tv_sec = TFTP_DEF_TIMEOUT_SEC;
	    tftp->backoff.tv_usec = TFTP_DEF_TIMEOUT_USEC;
	    timeout = tftp->backoff;
	    timeradd(&now, &tftp->backoff, &tftp->timer);
	} else if (timercmp(&now, &tftp->timer, >)) {
	    /* We just retransmitted. Double the interval. */
	    timeradd(&tftp->backoff, &tftp->backoff, &tftp->backoff);
	    timeout = tftp->backoff;
	    timeradd(&now, &tftp->backoff, &tftp->timer);
	} else {
	    /* We did not wait long enough yet. Calculate the
	       remaining time to block. */
	    timersub(&tftp->timer, &now, &timeout);
	}

	if (select(1 + tftp->sd, &fdset, NULL, NULL, &timeout) == -1) {
	    fprintf(stderr, "%s: select: %s\n",
		    progname, strerror(errno));
	    tftp_close(tftp);
	    return -EXIT_FAILURE;
	}

	if (! FD_ISSET(tftp->sd, &fdset)) {
	    retries--;
	    if (! retries) {
		fprintf(stderr,
			"%s: timeout, aborting data transfer errno=%d\n",
			progname, errno);
		tftp_close(tftp);
		return -EXIT_FAILURE;
	    }
	    continue;
	}

	buflen = recvfrom(tftp->sd, buf, sizeof(buf), 0,
		  (struct sockaddr *) &tftp->addr, &tftp->addrlen);
	if (buflen == -1) {
	    fprintf(stderr, "%s: recvfrom: %s\n",
		    progname, strerror(errno));
	    tftp_close(tftp);
	    return -EXIT_FAILURE;
	}
	if (tftp_dec_opcode(buf, buflen, &opcode) != 0) {
	    verbose("failed to parse opcode in message");
	    continue;
	}

	switch (tftp->state) {
	case TFTP_STATE_WRQ_SENT:
	case TFTP_STATE_DATA_SENT:
	case TFTP_STATE_LAST_DATA_SENT:
	    switch (opcode) {
	    case TFTP_OPCODE_ACK:
                //                if (blkno == 100000)
                //  printf("\rhggg%d\r", blkno);
		if (tftp_dec_blkno(buf, buflen, &blkno) != 0) {
		    verbose("failed to decode block number in ack packet");
		    continue;
		}
		if (blkno != tftp->blkno) {
		    verbose("ignoring unexpected bock number in ack packet");
		    continue;
		}
		if (tftp->state == TFTP_STATE_LAST_DATA_SENT) {
		    tftp->state = TFTP_STATE_CLOSED;
		} else {
		    ssize_t len;
		    len = read(tftp->fd, buf, TFTP_BLOCKSIZE);
		    if (len == -1) {
			fprintf(stderr, "%s: read: %s\n",
				progname, strerror(errno));
			tftp_close(tftp);
			return -EXIT_FAILURE;
		    }
		    if (tftp_enc_packet(tftp, TFTP_OPCODE_DATA, ++tftp->blkno,
					buf, len) == -1) {
			fprintf(stderr, "%s: encoding error\n", progname);
			return -EXIT_FAILURE;
		    }
		    timerclear(&tftp->timer);
		    retries = TFTP_DEF_RETRIES;
		    tftp->state = (len == TFTP_BLOCKSIZE) ?
			TFTP_STATE_DATA_SENT : TFTP_STATE_LAST_DATA_SENT;

		}
                  /*Display the transfer complete percent*/


		break;
	    case TFTP_OPCODE_ERROR:
		if (tftp_dec_error(buf, buflen, &errcode, &errmsg) != 0) {
		    verbose("failed to decode error message");
		    continue;
		}
		fprintf(stdout, "%s: ---tftp error %d: %s\n",
			progname, errcode, errmsg);
		rc = -errcode;
		tftp->state = TFTP_STATE_CLOSED;
		break;
	    default:
		verbose("unexpected message ignored");
		continue;
	    }
	    break;
	case TFTP_STATE_RRQ_SENT:
	case TFTP_STATE_ACK_SENT:
	    switch (opcode) {
	    case TFTP_OPCODE_DATA:
		if (tftp_dec_blkno(buf, buflen, &blkno) != 0) {
		    verbose("failed to decode block number in data packet");
		    continue;
		}
		if (blkno == tftp->blkno) {
		    unsigned char *data;
		    size_t len, datalen;
		    if (tftp_dec_data(buf, buflen, &data, &datalen) != 0) {
		    }
		    len = write(tftp->fd, data, datalen);
		    if (len == -1) {
			fprintf(stderr, "%s: write: %s\n",
				progname, strerror(errno));
			tftp_close(tftp);
			return -EXIT_FAILURE;
		    }
		    if (tftp_enc_packet(tftp, TFTP_OPCODE_ACK, tftp->blkno,
					NULL, 0) == -1) {
			fprintf(stderr, "%s: encoding error\n", progname);
			return -EXIT_FAILURE;
		    }
		    tftp->blkno++;
		    timerclear(&tftp->timer);
		    retries = TFTP_DEF_RETRIES;
		    tftp->state = (datalen == TFTP_BLOCKSIZE) ?
			TFTP_STATE_ACK_SENT : TFTP_STATE_LAST_ACK_SENT;
		}
		break;
	    case TFTP_OPCODE_ERROR:
		if (tftp_dec_error(buf, buflen, &errcode, &errmsg) != 0) {
                    verbose("failed to decode error message");
		    continue;
		}
		fprintf(stdout, "%s: ...tftp error %d: %s\n",
			progname, errcode, errmsg);
		rc = -errcode;
		tftp->state = TFTP_STATE_CLOSED;
		break;
	    default:
		verbose("unexpected message ignored");
		continue;
	    }
	}
    }

    return rc;
}
#ifdef TFTP_CLIENT
int
tftp_main(int argc, char *argv[])
#else
int    
main(int argc, char *argv[])
#endif
{
    int c, opcode = 0;

    tftp_t tftp = {
	.host = "localhost",
	.port = "69",
	.mode = TFTP_MODE_OCTET,
	.state = TFTP_STATE_CLOSED,
    };
    optind = 0; /* need to init this ourselves cause we call optopt more than once */
    while ((c = getopt(argc, argv, "p:h:wrv")) >= 0) {
	switch (c) {
	case 'p':
	    tftp.port = optarg;
	    break;
	case 'h':
	    tftp.host = optarg;
	    break;
	case 'r':
	    opcode = TFTP_OPCODE_RRQ;
	    break;
	case 'w':
	    opcode = TFTP_OPCODE_WRQ;
	    break;
	case 'v':
	    vflag = 1;
	    break;
	case ':':
	case '?':
	    return(-EXIT_FAILURE);
	}
    }

    if (optind >= argc) {
	goto usage;
    }

    tftp.file = argv[optind];

    tftp.local = argv[optind];
    if (argv[optind+1]) {
	if (opcode == TFTP_OPCODE_WRQ) {
	    tftp.file = argv[optind+1];
	}
	if (opcode == TFTP_OPCODE_RRQ) {
	    tftp.local = argv[optind+1];
	}
    }

    if (opcode == TFTP_OPCODE_RRQ || opcode == TFTP_OPCODE_WRQ) {
	int rc = 0;
	int flags;
	int state;
	
	tftp_socket(&tftp);
    
	flags = (opcode == TFTP_OPCODE_RRQ)
	    ? O_WRONLY | O_CREAT : O_RDONLY;
	state = (opcode == TFTP_OPCODE_RRQ)
	    ? TFTP_STATE_RRQ_SENT : TFTP_STATE_WRQ_SENT;
	
	tftp.fd = open(tftp.local, flags, 0666);
	if (tftp.fd == -1) {
	    fprintf(stderr, "%s: failed to open '%s': %s\n",
		    progname, tftp.local, strerror(errno));
	    tftp_close(&tftp);
	    return -EXIT_FAILURE;
	}

	if (tftp_enc_packet(&tftp, opcode, 0, NULL, 0) == -1) {
	    fprintf(stderr, "%s: encoding error\n", progname);
	    tftp_close(&tftp);
	    return -EXIT_FAILURE;
	}
	tftp.state = state;
	tftp.blkno = (opcode == TFTP_OPCODE_RRQ) ? 1 : 0;
	rc = tftp_mainloop(&tftp);
	tftp_close(&tftp);
	return rc;
    }
    
usage:
#ifndef TFTP_CLIENT

    fprintf(stderr,
	    "usage: %s [-v] [-h host] [-p port] -r <file> [localname]\n"
	    "       %s [-v] [-h host] [-p port] -w [localname] <file>\n",
	    progname, progname);
#endif
    return -EXIT_FAILURE;
}

/*-------------------------------------------------
$Log: tftpclient.c,v $
Revision 1.6  2014/07/17 23:07:06  mcharon
if tftp failes before file not found, don't retry

Revision 1.4  2013/10/31 18:32:40  mcharon
fix the name tftp to tftpc

Revision 1.3  2013/03/14 02:01:11  mcharon
support comiling as standalone

Revision 1.2  2012/12/20 21:23:59  mcharon
need to init optin because getopt is called more than once

Revision 1.1  2012/12/20 05:42:01  mcharon
bring in source for tftp client


$Endlog$
*/
