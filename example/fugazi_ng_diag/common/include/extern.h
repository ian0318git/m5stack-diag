/* $Id: extern.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/extern.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: ptong
 *------------------------------------------------------------------
 */

/*
** external variable file, function prototypes are in proto.h 
*/

/* atm/atmfmwimg.c */
extern char atmdiag_firmware_version[];
extern unsigned short atmdiag_firmware[]; /* portion of diag code
					   * resides in priram
					   */
extern int atmdiag_firmware_words; /* size of atmdiag_firmware[] */
extern unsigned short atmdiag_iram[]; /* portion of diag code resides
				       * in IRAM
				       */
extern int atmdiag_iram_words; /* size of atmdiag_iram[] */

/* atm/ldrfmwimg.c */
extern char atmloader_firmware_version[];
extern unsigned short atmloader_firmware[];
extern int atmloader_firmware_words;

/* atmxilinx.S */
extern char atm_xilinx_version[];       /* The ATM xilinx version */
extern int atm_xilinx, atm_endxilinx;   /* These are the xilinx image
					   start and end addresses */

/* memops.c */
extern long shmemsize;

extern int optind;
extern unsigned long memsize;
extern long memsize_mapped;
extern int netflashbooted;
extern int netboot_memorysize;
extern long getfreememstart(void);

/* bring in for VOLCANO */
extern char ciscopro_flag;              /* CiscoPro platform */
extern volatile unsigned char envflag, hkeepflags;
extern unsigned long *shmemaddr;
extern struct cookie_fmt c1500_cookie_fmt;

/* misc */
extern char *banner_string;
extern char *end, *_fbss;
extern int sys_state;

/* platform specific */
extern unsigned long platform_get_memsize(void);

/* End of File */


/******** History ******** 
$Log: extern.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
