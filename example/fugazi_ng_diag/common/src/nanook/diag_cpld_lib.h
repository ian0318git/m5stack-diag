/* $Id: diag_cpld_lib.h,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_cpld_lib.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_cpld_lib.h
 * Description: CPLD library header file.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define NANOOK_CPLD_KLM               "cpld"
#define CPLD_SIZE                      0x1000

extern int cpld_read_reg(uint, uint32_t *);
extern int cpld_write_reg(uint, uint);
extern int open_cpld(void);
extern int cpld_reset_api(uint, uint, uint, uint);

/*-------------------------------------------------
 * $Log: diag_cpld_lib.h,v $
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
