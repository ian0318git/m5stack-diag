/* $Id: prvCpssDxChPhySmi.h,v 1.1 2015/02/13 11:32:27 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPhySmi.h
*
* DESCRIPTION:
*        Private definitions for PHY SMI.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChPhySmih
#define __prvCpssDxChPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* offset of PHY SMI Group 1 registers */
#define CPSS_DX_PHY_ADDR_REG_OFFSET_CNS     0x0800000

/* offset of SMI Control registers */
#define CPSS_DX_SMI_MNG_CNTRL_OFFSET_CNS    0x1000000

/* number of tri-speed or FE ports with PHY poling support */
#define PRV_CPSS_DXCH_SMI_PPU_PORTS_NUM_CNS     24


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPhySmih */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChPhySmi.h,v $
 * Revision 1.1  2015/02/13 11:32:27  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
