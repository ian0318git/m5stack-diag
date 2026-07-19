/* $Id: pcmap.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/pcmap.h,v $
 *------------------------------------------------------------------
 *
 * pcmap.h - Memory map defines
 *
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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


