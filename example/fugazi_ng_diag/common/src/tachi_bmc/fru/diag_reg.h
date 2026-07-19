/* $Id: diag_reg.h,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_reg.h,v $
 *
 *      File:   diag_reg.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#ifndef _DIAG_REG_H_
#define _DIAG_REG_H_

// Register types
#define TYP_NONE 	0
#define TYP_REG	1
#define TYP_FLD	2

// Reg defines

#define REG_DEF( blk, adr, name, desc, mask, rst_val, typ ) \
  {  TYP_REG, blk, adr, 4, name, desc, mask, rst_val, typ },

#define REG_DEF_FLD( adr, name, hi, lo, desc, cause ) \
  { TYP_FLD, "fld",  adr, 0, name, desc, hi, lo, cause },

#define REG_DEF_RNG( adr, name, rng, desc, cause ) \
  { TYP_FLD, "fld",  adr, 0, name, desc, rng, cause },

#define BFLD(regname, fld) \
REG_DEF_FLD(regname, #fld, regname##_FLD_##fld, regname##_FLD_##fld, "", "")

#define RFLD(regname, fld) \
REG_DEF_FLD(regname, #fld, regname##_FLD_##fld##_E, regname##_FLD_##fld##_S, "", "")

#define MFLD(regname, fld) \
REG_DEF_RNG(regname, #fld, regname##_FLD_##fld, "", "")

#define WCFG(regname)   \
REG_DEF("asic", regname*4, #regname, "", 0xFFFF, 0x00, "cfg")

#define RCFG(regname)   \
REG_DEF("asic", regname, #regname, "", 0xFFFF, 0x00, "cfg")

#define RSHD(regname)   \
REG_DEF("asic", regname, #regname, "", 0xFFFF, 0x00, "shadow")

#define REXP(regname)   \
REG_DEF("asic", regname, #regname, "", 0xFFFF, 0x00, "exp")

#define RAUX(regname)   \
REG_DEF("asic", regname, #regname, "", 0xFFFF, 0x00, "aux")

#define RSTA(regname)   \
REG_DEF("asic", regname, #regname, "", 0xFFFF, 0x00, "status")

#define RIRQ(regname)   \
REG_DEF("asic", regname, #regname, "", 0xFFFF, 0x00, "irq")


typedef int (*REG_RD)(unsigned int, unsigned int*);
typedef int (*REG_WR)(unsigned int, unsigned int);

// Register Descriptor
typedef struct _reg_desc_s {
	int          desc_type; // TYP_REG or TYP_FLD
	char        *blk;       // unused for TYP_FLD
	unsigned int addr;      // matches owning reg for TYP_FLD
	int          len;       // unused for TYP_FLD
	char        *name;
	char        *desc;
	unsigned int mask;      // hi bit pos for TYP_FLD
	unsigned int rst_val;   // lo bit pos for TYP_FLD
	char        *typ;       // "cause" for TYP_FLD
} reg_desc_t;

#endif //_DIAG_REG_H_
