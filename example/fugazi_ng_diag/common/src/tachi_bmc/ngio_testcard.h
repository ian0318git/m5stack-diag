/* $Id: ngio_testcard.h,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/ngio_testcard.h,v $
 *--------------------------------------------------------------------
 * Filename   : ngio_testcard.h
 *
 * Description: Head file of TestCard to put those common definition.
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __NGIO_TESTCARD_H__
#define __NGIO_TESTCARD_H__

/* Common */
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


typedef struct testcard_if {
    int                type;
    char               *type_name;
    int                slot;
} testcard_if_t;


/* TestCard I2C device address  */
#define TESTCARD_FPGA_I2C_ADDR      0x65
#define TC_PCIE_REDRIVER_I2C_ADDR   0x60


/* Externs */
extern testcard_if_t *testcard_if_p;
extern void get_tc_i2c_struct(n2g_i2c_if_t *);
extern void get_testcard_if_info(testcard_if_t *);
extern uint32_t ngwic_testcard(void *);
extern int for_10gkr_testcard(int);
char testcard_10gkr_pid[80];
int tc_fru_table_offset; 

#endif /* __NGIO_TESTCARD_H__ */

/* ------- End of file ------- */

/******** History ******** 
$Log: ngio_testcard.h,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.2  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.1  2015/07/31 10:39:59  alpeng
first check in for testcard

$Endlog$
*/
