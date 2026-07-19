/* $Id: reason.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/reason.h,v $
 *------------------------------------------------------------------
 * reason.h -- reason why system reboots
 *
 * July 1988, Greg Satz
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

/*
 * Holdover data from last runable image
 */
#define BOOT_STACK_FRAMES	8	/* # of pairs of fp and pc to save */

/*
 * Definition of bootstuff[]:
 */
#define BOOT_COUNT		0	/* Length of bootstuff */
#define	BOOT_REASON		1	/* Reason for last reboot */
#define	BOOT_PC			2	/* PC at last reboot */
#define	BOOT_ADDRESS		3	/* Buserror address */
#define BOOT_STACK_TRACE	4	/* Saved stack trace (fp and pc) */
#define BOOT_STACK_LAST		((BOOT_STACK_FRAMES * 2)-1+BOOT_STACK_TRACE)

#define	BOOTDATA		BOOT_STACK_LAST	/* Length of bootstuff */

/*
 * Reason for last reboot:
 */
#define	EXC_RESET	0	/* Reset switch  */
#define	EXC_ABORT	1	/* Abort switch */
#define	EXC_BREAK	2	/* Breakpoint trap */
#define EXC_EXIT	3	/* Exit trap */
#define	EXC_TRACE	4	/* Trace trap */
#define	EXC_EMT		5	/* Emulator trap */
#define	EXC_BUSERR	6	/* Bus error */
#define	EXC_ADRERR	7	/* Address error */
#define	EXC_WATCHDOG	8	/* Watchdog timeout */
#define	EXC_RELOAD	9	/* Reload requested */
#define	EXC_PARITY	10	/* Parity error */
#define EXC_SHMEM_PE    11      /* shared memory parity error */
#define EXC_UNEXP_INT   12      /* unexpected user definable interrupt */
#define EXC_MAX         12      /* largest reboot reason code */

/*
 * When to give up network boot and boot from roms.
 */

#define	THRESHOLD 5

/*
 * stuff this in bootstuff[BOOT_ADDRESS] to request boot of ROM software
 * actually, stuff ROMBOOT_REQUEST-1 in bootstuff[BOOT_ADDRESS] to 
 * account for increment done by rom monitor before value is checked
 */

#define ROMBOOT_REQUEST (THRESHOLD+2)

/******** History ******** 
$Log: reason.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
