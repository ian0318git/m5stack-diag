/* $Id: ngio_testcard.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/ngio_testcard.h,v $
 *------------------------------------------------------------------
 * Filename   : ngio_testcard.h
 *
 * Description: Head file of TestCard to put those common definition.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __NGIO_TESTCARD_H__
#define __NGIO_TESTCARD_H__

/* Common */
int ntc_gesw_p0_type; 
int ntc_gesw_p1_type; 

#define TC_BUF_SIZE   256

#define TC_NGSM_PCIE_LANE_NUM  2

/* TestCard Interface Type */
#define TC_NGSM       0x01
#define TC_NGWIC      0x02

/* TestCard Loopback Type */
#define TC_INT_LPBK   0x00
#define TC_EXT_LPBK   0x01

/* TestCard UART Loopback Type */
#define TC_UART_HOST_LPBK   0x00
#define TC_UART_LINE_LPBK   0x01

/* Testcard status+type */
#define TC_LEGACY_NIM_OR_SM    0x00 /* old NIM TC or SM TC */ 
#define TC_10GKR_NIM_ON_NIM    0x01 /* 10GKR TC plug to NIM slot */
#define TC_10GKR_NIM_ON_SM     0x02 /* 10GKR TC + Thule (SM slot) */ 
#define TC_10GKR_SM_ON_SM      0x03 /* 10GKR SM TC */

typedef struct testcard_if {
    int                type;
    char               *type_name;
    int                slot;
    int                uart_ctrl; 
    int                is_10gkr; 
} testcard_if_t;


/* TestCard I2C device address  */
#define TESTCARD_FPGA_I2C_ADDR      0x65
#define TC_PCIE_REDRIVER_I2C_ADDR   0x60

/* Check eth port link status */
#define NTC_RETRY_CNTER  (60) 
#define NTC_DELAY_TIME   (1000)

/* Externs */
extern testcard_if_t *testcard_if_p;
extern uint32_t tc_real_pcie_port;
extern void get_tc_i2c_struct(n2g_i2c_if_t *);
extern void get_testcard_if_info(testcard_if_t *);
extern uint32_t ngwic_testcard(void *);
extern uint32_t ngsm_testcard(void *);
extern int for_10gkr_testcard(int);
extern int ntc_gesw_ptype_chk(int); 
extern int ntc_chk_eth_linkup(int); 


#endif /* __NGIO_TESTCARD_H__ */

/* ------- End of file ------- */

