/* $Id: regs.h,v 1.2 2017/07/28 07:58:38 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/regs.h,v $
 *------------------------------------------------------------------
 * regs.h
 *      sp27xx register mapping between LSI_SP27XX and AG_MG_REGS
 *
 * Mar 2012-2017, Smita Rane
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*------------------------------------------------------------------
 * regs.h - sp27xx register mapping between LSI_SP27XX and AG_MG_REGS
 *
 * March, 2011 pbecerra
 *
 * Copyright (c) 2011 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef _REGS_H_
#define _REGS_H_

#define LSI_MG_REG_STRUCT_INITIALIZE 1


#if defined(USE_AG_MG_REGS) && !defined(USE_LSI_SP27XX_REGS)

#include <ag_mg_regs_regops.h>
#include <ag_mg_regs_car.h>

#define SET_BITS_M(r,m)   REG32_SET_BITS(AG_MG_REGS_##r, AG_MG_REGS_##m)
#define SET_BITS(r,m)     REG32_SET_BITS(AG_MG_REGS_##r, m)
#define RESET_BITS_M(r,m) REG32_RESET_BITS(AG_MG_REGS_##r, AG_MG_REGS_##m)
#define RESET_BITS(r,m)   REG32_RESET_BITS(AG_MG_REGS_##r, m)
#define CHK_REG_M(r,m)    CHK_REG_MASK(AG_MG_REGS_##r,AG_MG_REGS_##m)
#define CHK_REG(r,m)      CHK_REG_MASK(AG_MG_REGS_##r,m)
#define READ(r,p)         REG32_READ(AG_MG_REGS_##r,p)
#define WRITE(r,v)        REG32_WRITE(AG_MG_REGS_##r,v)
#define ACCESS(r)         HW_REG_ACCESS(AG_MG_REGS_##r)
#define RM(r)             AG_MG_REGS_ ## r
#define CLR_REG_M(r,m)    (HW_REG_ACCESS(AG_MG_REGS_##r) &= ~(AG_MG_REGS_##m))
#define CLR_REG(r,m)      (HW_REG_ACCESS(AG_MG_REGS_##r) &= ~(m))

typedef ag_mg_regs_register REGISTER ;

#elif !defined(USE_AG_MG_REGS) && defined(USE_LSI_SP27XX_REGS)

#include <lsi_sp27xx_regops.h>
#include <lsi_sp27xx_car.h>

#define SET_BITS_M(r,m)   REG32_SET_BITS(LSI_SP27XX_##r, LSI_SP27XX_##m)
#define SET_BITS(r,m)     REG32_SET_BITS(LSI_SP27XX_##r, m)
#define RESET_BITS_M(r,m) REG32_RESET_BITS(LSI_SP27XX_##r, LSI_SP27XX_##m)
#define RESET_BITS(r,m)   REG32_RESET_BITS(LSI_SP27XX_##r, m)
#define CHK_REG_M(r,m)    CHK_REG_MASK(LSI_SP27XX_##r,LSI_SP27XX_##m)
#define CHK_REG(r,m)      CHK_REG_MASK(LSI_SP27XX_##r,m)
#define READ(r,p)         REG32_READ(LSI_SP27XX_##r,p)
#define WRITE(r,v)        REG32_WRITE(LSI_SP27XX_##r,v)
#define ACCESS(r)         HW_REG_ACCESS(LSI_SP27XX_##r)
#define RM(r)             LSI_SP27XX_ ## r
#define CLR_REG_M(r,m)    (HW_REG_ACCESS(LSI_SP27XX_##r) &= ~(LSI_SP27XX_##m))
#define CLR_REG(r,m)      (HW_REG_ACCESS(LSI_SP27XX_##r) &= ~(m))

typedef lsi_sp27xx_register REGISTER ;

/* Fixup naming differences between PDS and IDE defs */
#ifndef LSI_SP27XX_CAR_CHIPID_REV_ID_BO
#define LSI_SP27XX_CAR_CHIPID_REV_ID_BO LSI_SP27XX_CHIPID_REV_ID_BO
#define LSI_SP27XX_CAR_CHIPID_REV_ID_BM LSI_SP27XX_CHIPID_REV_ID_BM
#endif

#else
#error "One of USE_AG_MG_REGS or USE_LSI_SP27XX_REGS must be defined"
#endif


#define IS_V10_DEVICE() \
   ((ACCESS(CAR_CHIPID_RA) & RM(CAR_CHIPID_REV_ID_BM)) == 0)


#endif /* _REGS_H_ */

/******** History ********
$Log: regs.h,v $
Revision 1.2  2017/07/28 07:58:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/07/17 20:34:28  srane
cleanup

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

