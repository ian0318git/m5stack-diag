/* $Id: m2_testcard_host_impl.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/m2_testcard_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * m2_testcard_host_impl.h
 *
 * Apr 2021, Ian Chang <iachang@cisco.com>
 *
 * Copyright (c) 2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __M2_TESTCARD_HOST_IMPL__
#define __M2_TESTCARD_HOST_IMPL__

#define M2_POWER_BIT         (0x1 << 0)

boolean is_m2_testcard_in (void);
extern int m2_tc_host_get_nvme_dev(char *dev_name,int length);
extern int m2_tc_host_get_eusb_dev(char *dev_name,int length);
extern int m2_tc_host_get_i2c_dev(uint8_t *i2c_ctrl, uint8_t *i2c_mux);
extern int m2_tc_power_control(int flag);
extern int m2_tc_host_get_m2_pcie_config(int*, int*, int*, int*, int*);
#endif
/*
 *------------------------------------------------------------------
 * $Log: m2_testcard_host_impl.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.2.1  2021/04/26 08:15:25  iachang
 * CSCvy10910:Fugazi Diag supportted M.2 test card
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
