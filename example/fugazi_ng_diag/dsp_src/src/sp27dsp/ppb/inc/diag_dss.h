/* $Id: diag_dss.h,v 1.2 2012/10/04 23:36:15 srane Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/diag_dss.h,v $
 *------------------------------------------------------------------
 * diag_dss.h
 *      Defines related to DSS cores 
 *
 * March 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_DSS_H__
#define __DIAG_DSS_H__

#define LSI_MG_REG_STRUCT_INITIALIZE 1 /* For DSS_REG[] */
#define AG_MG_NUM_DSS 4 /* For DSS_REG[] */
#define SP_ALL_DSS_CORES_BM 0xF /* For DSS_REG[] */
#define SP_ALL_2702_DSS_CORES_BM 0x3 /* For DSS_REG[] */

enum
{
        CORE_PPB          = 0,
        CORE_DSS0         = 1,
        CORE_DSS1         = 2,
        CORE_DSS2         = 3,
        CORE_DSS3         = 4,
        CORE_ARM          = 5,
        CORE_DSS          = 6
};

extern volatile uint32_t *uart_getlock;
extern volatile uint32_t *uart_mem;

extern uint32_t sp_ReleaseDSS(uint32_t dssBitmask);
extern uint32_t sp_ResetDSS(uint32_t dssBitmask);

#define FREE 0
#define LOCKED 1

#endif /* __DIAG_DSS_H__ */

/******** History ********
$Log: diag_dss.h,v $
Revision 1.2  2012/10/04 23:36:15  srane
Add support for SP2702. Version control.

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

