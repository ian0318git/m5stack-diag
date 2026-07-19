/* $Id: libfdt_env.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/libfdt_env.h,v $
 *-----------------------------------------------------------------------------
 *
 * April 2013, Times Huang
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef _LIBFDT_ENV_H
#define _LIBFDT_ENV_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define _B(n)	((unsigned long long)((uint8_t *)&x)[n])
static inline uint32_t fdt32_to_cpu(uint32_t x)
{
	return (_B(0) << 24) | (_B(1) << 16) | (_B(2) << 8) | _B(3);
}
#define cpu_to_fdt32(x) fdt32_to_cpu(x)

static inline uint64_t fdt64_to_cpu(uint64_t x)
{
	return (_B(0) << 56) | (_B(1) << 48) | (_B(2) << 40) | (_B(3) << 32)
		| (_B(4) << 24) | (_B(5) << 16) | (_B(6) << 8) | _B(7);
}
#define cpu_to_fdt64(x) fdt64_to_cpu(x)
#undef _B

#endif /* _LIBFDT_ENV_H */

/*-------------------------------------------------
 * $Log: libfdt_env.h,v $
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:09  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:23  tirawan
 * Initial check-in for woodlawn linux code
 *
 * 
 *
 * $Endlog $
 *-------------------------------------------------
 */
