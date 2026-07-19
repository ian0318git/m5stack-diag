/* $Id: pcmap.h,v 1.2 2017/08/02 14:21:47 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/pcmap.h,v $
 *------------------------------------------------------------------
 *
 * pcmap.h - Memory map defines for TSN.
 *
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
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


/******** History ******** 
*/
/*************************************************************
$Log: pcmap.h,v $
Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/08 09:55:11  steja
Initial Check-in


$Endlog$
*/
