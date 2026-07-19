/* $Id: m2_testcard_host_impl.h,v 1.1 2021/05/13 08:49:58 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/m2_testcard_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * m2_testcard_host_impl.h
 *
 * Copyright (c) 2021 by Cisco Systems, Inc
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __M2_TESTCARD_HOST_IMPL__
#define __M2_TESTCARD_HOST_IMPL__

#define M2_POWER_BIT         (0x1 << 0)
#define M2_PCIE_BUS2         0x2

boolean is_m2_testcard_in(void);
extern int m2_tc_host_get_nvme_dev(char *, int);
extern int m2_tc_host_get_eusb_dev(char *, int);
extern int m2_tc_host_get_i2c_dev(uint8_t *, uint8_t *);
extern int m2_tc_power_control(int);
extern int m2_tc_host_get_m2_pcie_config(int*, int*, int*, int*, int*);
#endif

/*-------------------------------------------------
 * $Log: m2_testcard_host_impl.h,v $
 * Revision 1.1  2021/05/13 08:49:58  kodko
 * Support M.2 testcard.
 *
 * $Endlog$
 *-------------------------------------------------
 */
