/* $Id: ag_mg_regs_regops.h,v 1.1 2012/04/18 18:08:26 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_mg_regs_regops.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_regops.h
 *      Graffham - DSS uart 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef AG_MG_REGS_REGOPS_H
#define AG_MG_REGS_REGOPS_H

/* NOTE: users should redefine as suitable for their compiler;
 *       this is used below, and in peripheral-specific
 *       header files to define register contents
 */
typedef volatile unsigned int  ag_mg_regs_register;

/*
 * macro definition for correct access of HW registers; ra is physical
 *  register address (LSI_SP27xx_RA_XXX from peripheral-specific header file)
 * NOTE: volatile, 32-bit access is always necessary for HW
 *  registers on ARM, even if less than 32 bits are read/written!!!
 */
#define HW_REG_ACCESS(ra)  (*(ag_mg_regs_register *)(ra))

#ifndef SUCCESS
#define SUCCESS		0
#endif
#ifndef ERROR
#define ERROR		1
#endif

/* use LSI_SP27xx_XXX_RA, LSI_SP27xx_XXX_BM from peripheral-specific header */
#define SET_REG_MASK(ra,bm) (HW_REG_ACCESS(ra) |=  (bm))
#define CLR_REG_MASK(ra,bm) (HW_REG_ACCESS(ra) &= ~(bm))
#define MSK_REG_MASK(ra,bm) (HW_REG_ACCESS(ra) &=  (bm))
#define CHK_REG_MASK(ra,bm) ((HW_REG_ACCESS(ra) & (bm)) == (bm))

#define REG32_READ(addr, data)	(data = *(ag_mg_regs_register*)(addr))
#define REG32_WRITE(addr, data)	(*(ag_mg_regs_register*)(addr) = (unsigned long)(data))

#define REG32_RESET_BITS(addr, reset_bits)	(*(ag_mg_regs_register*)(addr) &= ~(reset_bits))
#define REG32_SET_BITS(addr, set_bits)	(*(ag_mg_regs_register*)(addr) |= (set_bits))

#endif

/* 
 * $Log: ag_mg_regs_regops.h,v $
 * Revision 1.1  2012/04/18 18:08:26  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

