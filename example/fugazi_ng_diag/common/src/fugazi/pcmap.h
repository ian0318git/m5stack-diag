/* $Id: pcmap.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/pcmap.h,v $
 *------------------------------------------------------------------
 *
 * pcmap.h - Memory map defines for Fugazi.
 *
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PCMAP_H_
#define _PCMAP_H_


/*
 * Macro to map DRAM virtual space to physical space and vice versa. 
 */
#define PHYSICAL_ADDR_START     0

/*
 * Physical DRAM definitions.
 */
#define ADRSPC_RAM		PHYSICAL_ADDR_START		/* start of RAM */
#define PHY_ADRSPC_RAM		PHYSICAL_ADDR_START	/* Start of RAM */
#define ADRSPC_RAM_END		(PHYSICAL_ADDR_START + 0x80000000) 
#define ADRSPC_RAM_SIZE		(ADRSPC_RAM_END - ADRSPC_RAM)

/*
 * Top memory reserved by BIOS+ROMMON
 */
#define BIOS_TOP_MEM_RSV_SZ	0x4000000			/* Top mem reserved by BIOS */

/*
 * Definition for ROMMON used for bus error testing. On this platform
 * there is no real KSEG space but these definitions
 * help facilitate porting the existing code from MARs which is
 * a MIPs based platform.
 */ 
#define ADRSPC_BAD_ADDR		0xbad0add0			/* for stack.c */
#define KSEG1_ADRSPC_BAD_ADDR	0xbad0add0		/* for bus error test */
#define ADRSPC_K1BASE		0x0					/* for bus error test */


#endif /* _PCMAP_H_ */


/*-------------------------------------------------
 * $Log: pcmap.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:50  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
