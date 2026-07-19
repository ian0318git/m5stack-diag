/* $Id: defs.h,v 1.1 2014/03/25 02:12:32 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** General defines.
*/
#define TRUE       1
#define FALSE      0

#define PASSED     0
#define FAILED     1

#define	WASTETIME_MAX	10000  /* wastetime maximum for diagmon kernel */

#define	REFRESHTIME	4			/* time in milliseconds 
						   for refresh */
#define	REFRFREQUENCY	(REFRESHTIME*1000)

#define ALARM_FREQUENCY (1000)       		/* 1ms */

#define ERRLOG_SIZE     (32 * 1024)

#define	BADADDRESS	0x2200000		/* address of bad location */

#define INITSTACKP	(ADRSPC_NVRAM + 0x2000)	/* stack pointer starts 
						   (goes down) */

#define ESCAPE_CHAR '\033'                      /* escape key code */

/*
** The following defines are used by a variety of routines to define
** bus width.
*/
#define BW_8BITS     0x01
#define BW_16BITS    0x02
#define BW_32BITS    0x04
#define BW_64BITS    0x08
#define BW_24BITS    0x10
#define BW_40BITS    0x20
#define BW_48BITS    0x40
#define BW_56BITS    0x80

#define BW_8BITS_LE     0x100
#define BW_16BITS_LE    0x200
#define BW_32BITS_LE    0x400
#define BW_64BITS_LE    0x800

/*
 *  for type in reg_info_t of common_utils.h
 *  These defines are bitwise. If not READ_ONLY and not WRITE_ONLY, then
 *  it is READ_WRITE.
 */
#define READ_WRITE 0
#define READ_ONLY  1
#define WRITE_ONLY 2
#define SAVE_RESTORE 4	/* For READ_WRITE only. Will not use resetval */
#define REG_ACCESS 8	/* Caller provided read/write access */
#define REG_DEV	   0x10	/* Device specifics */

/*
 * Definitions for possible system states
 */
#define SYSTEM_RUNNING   1              /* running the system code */
#define MONITOR_RUNNING  2              /* in the Monitor          */

/*
 * Definition for LED state
 */
#define OFF_LED        0
#define GREEN_LED      1
#define YELLOW_LED     2

/*
 * Define the PACKED macro
 */
#if defined(__mips) || defined(__mc68000)
#define PACKED(item) item __attribute__ ((packed))
#else
#define PACKED(item) item
#endif

/*
 * Macro for nop's surround accesses that may cause bus errors
 */
#ifdef __mips

#define BUSERR_NOP_X4()                                    \
    asm(".set noreorder");                                 \
    asm("nop"); asm("nop"); asm("nop"); asm("nop");        \
    asm(".set reorder"); 

/*
 * Define a write buffer flush macro
 */
#define FLUSH_BUFFERS	flush_all_wb

#define LD_ACCESS(a, t, tu)	ld_access(a, t, tu)
#define LDR_ACCESS(a, t, tu)	ldr_access(a, t, tu)
#define LDL_ACCESS(a, t, tu)	ldl_access(a, t, tu)
#define LWR_ACCESS(a, t)	lwr_access(a, t)
#define LWL_ACCESS(a, t)	lwl_access(a, t)

#define SD_ACCESS(a, t, tu)	sd_access(a, t, tu)
#define SDR_ACCESS(a, t, tu)	sdr_access(a, t, tu)
#define SDL_ACCESS(a, t, tu)	sdl_access(a, t, tu)
#define SWR_ACCESS(a, t)	swr_access(a, t)
#define SWL_ACCESS(a, t)	swl_access(a, t)

#ifndef ASMINCLUDE

/*
 * GET/PUT primitives
 */
static inline
unsigned long r4k_getlong(register void const *ptr)
{
    register unsigned long data;

    /* 
     * Load a mis-aligned ulong
     */
    asm volatile ("lwl %0,0(%1);lwr %0,3(%1)"	
	: "=&r" (data)			/* ouput variable		*/
	: "r"  (ptr));			/* input variable		*/

    return(data);
}

static inline
void r4k_putlong(register void *ptr, register unsigned long value)
{
    /* 
     * Store a mis-aligned long
     */
    asm volatile ("swl %1,0(%0);swr %1,3(%0)"	
	:				/* no outputs			*/
	: "r" (ptr), "r" (value));	/* input variables		*/
}

#define GETLONG(ptr)        r4k_getlong(ptr)
#define PUTLONG(ptr, val)   r4k_putlong(ptr, val)

#endif //ASMINCLUDE

#else /* __mips */

/*
 * GET/PUT primitives are null on 68k processors because long word
 * accesses on short word boundaries are OK (but not the swiftest).
 */
#define GETLONG(addr) (*((ulong *)(addr)))
#define PUTLONG(addr, value) (*((ulong *)(addr)) = (ulong)(value))

#define BUSERR_NOP_X4()                                    \
    asm("nop"); asm("nop"); asm("nop"); asm("nop");

#define FLUSH_BUFFERS	BUSERR_NOP_X4

/*
 * On M68k platforms, 3,5,6,7,8 byte accesses are impossible
 * so the macros expand out to nothing
 */

#define LD_ACCESS(a, t, tu)
#define LDR_ACCESS(a, t, tu)
#define LDL_ACCESS(a, t, tu)
#define LWR_ACCESS(a, t)
#define LWL_ACCESS(a, t)

#define SD_ACCESS(a, t, tu)
#define SDR_ACCESS(a, t, tu)
#define SDL_ACCESS(a, t, tu)
#define SWR_ACCESS(a, t)
#define SWL_ACCESS(a, t)

#endif /* __mips */

/* End of Module */

/* End of File */
/*------------------------------------------------------------------------------
 * $Log: defs.h,v $
 * Revision 1.1  2014/03/25 02:12:32  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.2  2011/08/18 19:43:22  huanngo
 * Update code to patriot2-branch
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

