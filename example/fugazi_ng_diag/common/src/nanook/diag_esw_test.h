/* $Id: diag_esw_test.h,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_esw_test.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_esw_test.h
 * Description: header file of Nanook ethernet switch,
 *              Marvell 98DXC323.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_ESW_TEST_H__
#define __DIAG_ESW_TEST_H__

#define NANOOK_SNAKE_TEST_FORWARD    0
#define NANOOK_SNAKE_TEST_BACKWARD    1

#define NANOOK_SNAKE_PAIR_NUM    13
#define NANOOK_SNAKE_IXIA_PAIR_NUM    12

#define NANOOK_SNAKE_PHY_PAIR_NUM    12

#define NANOOK_LINK_UP_TOUT    100

#define NANOOK_PHY_INTR_TEST_PORT_NUM    3 
#define NANOOK_PHY_INTR_TEST_PORT_0    0 
#define NANOOK_PHY_INTR_TEST_PORT_8    8 
#define NANOOK_PHY_INTR_TEST_PORT_16    16 

#endif   /* __DIAG_ESW_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_esw_test.h,v $
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
