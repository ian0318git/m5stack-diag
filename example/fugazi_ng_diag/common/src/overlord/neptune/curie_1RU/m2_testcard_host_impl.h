/*------------------------------------------------------------------
 *
 * m2_testcard_host_impl.h
 *
 * Feb 2021, Xiaolan Yang <xiaolaya@cisco.com>
 *
 * Copyright (c) 2021 by Cisco Systems, Inc
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __M2_TESTCARD_HOST_IMPL__
#define __M2_TESTCARD_HOST_IMPL__

boolean is_m2_testcard_in (void);
extern int m2_tc_host_get_nvme_dev(char *dev_name,int length);
extern int m2_tc_host_get_eusb_dev(char *dev_name,int length);
extern int m2_tc_host_get_i2c_dev(uint8_t *i2c_ctrl, uint8_t *i2c_mux);
extern int m2_tc_power_control(int flag);
extern int m2_tc_host_get_m2_pcie_config(int*, int*, int*, int*, int*);
#endif
