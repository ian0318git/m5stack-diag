/* $Id: diag_smi_lib.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_smi_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_smi_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/* Common */
#define PLAT_SMI_RETRY_MAX   100
#define M7040_SMI_REG                0xF212A200
#define M7040_SMI_BUSY               (1 << 28)
#define M7040_SMI_READ_VALID         (1 << 27)
#define M7040_SMI_OPCODE_RD          (1 << 26)

/* Externs */
extern int plat_smi_read(int, int, ushort *);
extern int plat_smi_write(int, int, ushort);
extern int plat_smi_read_util(int);
extern int plat_smi_write_util(int);

/*-------------------------------------------------
 * $Log: diag_smi_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
