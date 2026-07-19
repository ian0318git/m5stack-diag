/* $Id: platform_debug.h,v 1.1 2013/05/09 05:42:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_debug.h,v $
 *------------------------------------------------------------------
 *
 * platform_debug.h - Defines platform specific debug info.
 *
 * July 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_DEBUG_H_
#define _PLATFORM_DEBUG_H_

#include "common.h"
#include "proto.h"

#define isdbg(_dbg) \
	    ((_dbg) & platform_debug)

#define dbgprt(_dbg, _fmt, _args...) \
	    if (isdbg(_dbg)) { printf(_fmt, ## _args); }

#define ISDBG(_dbg) \
	    isdbg(_dbg)

#define DBGPRT(_dbg, _fmt, _args...) \
	dbgprt(_dbg, _fmt, ## _args)

#define DASSERT(_fmt , _args...) \
	    { printf(_fmt, ## _args); assert(0); }
	
#define PLAT_DBG_INIT		0

#define PLAT_DBG_STR \
	"Debug bit 0:CPU 1:APIC 2:PCI 3:INTR 4:PIRQ\n"

/*
 * platform_debug bit defines
 */
#define PLAT_DBG_NONE		0x00000000
#define PLAT_DBG_CPU		0x00000001
#define PLAT_DBG_APIC		0x00000002
#define PLAT_DBG_PCI		0x00000004
#define PLAT_DBG_INTR		0x00000008
#define PLAT_DBG_PIRQ		0x00000010
#define PLAT_DBG_MCHK		0x00000020
/* Don't forget to update the PLAT_DBG_STR above as well */ 
#define PLAT_DBG_ALL		0xffffffff

extern uint platform_debug;

#endif /* _PLATFORM_DEBUG_H_ */
