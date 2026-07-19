/* $Id: ngio_testcard.h,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/ngio_testcard.h,v $
 *--------------------------------------------------------------------
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

/******** History ******** 
$Log: ngio_testcard.h,v $
Revision 1.2  2019/10/17 02:16:25  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.6  2019/03/14 06:14:47  olin2
Clean up code

Revision 1.1.2.5  2019/03/13 09:14:14  olin2
Update eth setting

Revision 1.1.2.4  2018/12/22 07:20:12  olin2
Clean up code

Revision 1.1.2.3  2018/11/06 07:02:11  olin2
Update eth port

Revision 1.1.2.2  2018/11/02 10:08:35  olin2
Support testcard xaui test

Revision 1.1.2.1  2018/10/09 09:22:05  olin2
Initial commit for NIM test

Revision 1.4.2.1  2018/08/28 16:31:18  alpeng
stablize testcard loobpack test on curie

Revision 1.4  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.3  2016/10/16 12:28:17  iachang
Supported Goldbeach Platform.

Revision 1.2.36.5  2018/01/16 06:46:30  alpeng
first check in for 10G-KR SM testcard; we need to apply correct id once hw ready for it

Revision 1.2.36.4  2017/04/17 10:13:24  alpeng
add gesw ptype check

Revision 1.2.36.3  2017/04/06 02:09:45  leschen
Fix testcard xaui issue.

Revision 1.2.36.2  2017/04/05 06:41:58  leschen
Sync with <ng_diag-tag-032917>

Revision 1.2.36.1  2016/12/15 03:42:04  alpeng
 fix old testcard uart issue, using api to get uart_ctrl num

Revision 1.3  2016/10/16 12:28:17  iachang
Supported Goldbeach Platform.

Revision 1.2  2014/07/25 01:36:57  alpeng
support xaui loopback and sort out the test item for new testcard

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.6  2012/11/07 10:58:15  alpeng
remove useless file and clean up code

Revision 1.5  2012/10/10 16:57:01  palin2
Fixed NGWIC TestCard PCIe loopback test based on IDT FAE's suggestion.

Revision 1.4  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

Revision 1.3  2012/09/12 10:26:08  palin2
Add NGWIC TestCard support from Host side(Overlord) DiagMenu.

Revision 1.2  2012/08/20 13:22:58  palin2
Add NGSM TestCard support from Host side(Overlord) DiagMenu.

Revision 1.1  2012/08/14 11:30:55  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.4  2012/08/08 22:19:41  palin2
1. Move TestCard UART external loopback test to "testcard_uart.c".
2. Add support TestCard UART internal loopback test and related utilities.

Revision 1.3  2012/07/31 17:08:20  palin2
Initial check-in for TestCard PCIe tests.

Revision 1.2  2012/07/30 15:47:09  palin2
Add support TestCard UART loopback test.

Revision 1.1  2012/07/23 17:33:54  palin2
Initial check-in for Overlord Test Card diag tests.


$Endlog$
*/
