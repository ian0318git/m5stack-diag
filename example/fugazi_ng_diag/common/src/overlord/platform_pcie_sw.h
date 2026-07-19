/* $Id: platform_pcie_sw.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pcie_sw.h,v $
 *------------------------------------------------------------------
 *
 * platform_pcie_sw.h - Overlord PCIe switch, IDT PES16NT16G2,
 *                      related definitions.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_PCIE_SW_H_
#define _PLATFORM_PCIE_SW_H_

/* Common */
#define OVLD_PCIE_LPBK_TEST_TIME   1
#define PCIE_SW_EXT_TEST           0
#define OVLD_PCIE_TEST_NUM         4

typedef struct port_info {
    char       *name;     /* port name */
    uint32_t   port_num;  /* number of the port */
} port_info_t;


/* Port Number */
#define PCIE_SW_P0       0
#define PCIE_SW_P1       1
#define PCIE_SW_P2       2
#define PCIE_SW_P3       3
#define PCIE_SW_P4       4
#define PCIE_SW_P5       5
#define PCIE_SW_P6       6
#define PCIE_SW_P7       7
#define PCIE_SW_P8       8
#define PCIE_SW_P9       9
#define PCIE_SW_P10      10
#define PCIE_SW_P11      11
#define PCIE_SW_P12      12
#define PCIE_SW_P13      13
#define PCIE_SW_P14      14
#define PCIE_SW_P15      15
#define PCIE_SW_P16      16
#define PCIE_SW_P17      17
#define PCIE_SW_P18      18
#define PCIE_SW_P19      19

/* Base Addr. for Port Config. Space Reg. of IDT PES16NT16G2 */
#define P0_CFG_ADDR      0x00000
#define P1_CFG_ADDR      0x02000
#define P2_CFG_ADDR      0x04000
#define P3_CFG_ADDR      0x06000
#define P4_CFG_ADDR      0x08000
#define P5_CFG_ADDR      0x0A000
#define P6_CFG_ADDR      0x0C000
#define P7_CFG_ADDR      0x0E000
#define P8_CFG_ADDR      0x10000
#define P9_CFG_ADDR      0x12000
#define P10_CFG_ADDR     0x14000
#define P11_CFG_ADDR     0x16000
#define P12_CFG_ADDR     0x18000
#define P13_CFG_ADDR     0x1A000
#define P14_CFG_ADDR     0x1C000
#define P15_CFG_ADDR     0x1E000
#define P16_CFG_ADDR     0x20000
#define P17_CFG_ADDR     0x22000
#define P18_CFG_ADDR     0x24000
#define P19_CFG_ADDR     0x26000


/* Registers Map of IDT PES16NT16G2 */
#define SWCONF_STS_BASE   0x3E000


#define SWCTL_OFF           0x0000
#define PCIELCTL_OFF        0x0050
#define PCIELSTS_OFF        0x0052
#define SERDESCFG_OFF       0x0510
#define PHYLCFG0_OFF        0x0530
#define STMCTL_OFF          0x0E54
#define STMSTS_OFF          0x0E58
#define STMTCTL_OFF         0x0E5C
#define STMTSTS_OFF         0x0E60
#define STMECNT0_OFF        0x0E64
#define GASAADDR_OFF        0x0FF8
#define GASADATA_OFF        0x0FFC


/* SWCTL - Switch Control Reg. (0x0000) */
#define SWCTL_RSTHALT       0x00000004
#define SWCTL_REGUNLOCK     0x00000008
#define SWCTL_BDISCARD      0x00080000

/* PCIELCTL - PCI Express Link Control Reg. (0x050) */
#define PCIELCTL_ASPM_MSK     0x0003
#define PCIELCTL_RCB          0x0008
#define PCIELCTL_LDIS         0x0010
#define PCIELCTL_LRET         0x0020
#define PCIELCTL_CCLK         0x0040
#define PCIELCTL_ESYNC        0x0080
#define PCIELCTL_CLKPWRMGT    0x0100
#define PCIELCTL_HAWD         0x0200
#define PCIELCTL_LBWINTEN     0x0400
#define PCIELCTL_LABWINTEN    0x0800

/* PCIELSTS - PCI Express Link Status Reg. (0x052) */
#define PCIELSTS_CLS_MSK      0x000F
#define PCIELSTS_NLW_MSK      0x03F0
#define PCIELSTS_LTRAIN       0x0800
#define PCIELSTS_SCLK         0x1000
#define PCIELSTS_DLLLA        0x2000
#define PCIELSTS_LBWSTS       0x4000
#define PCIELSTS_LABWSTS      0x8000

#define PCIELSTS_DLLLA_OFF    13

/* SERDESCFG - SerDes Configuration Reg. (0x510) */
#define SERDESCFG_RCVD_MSK    0x000000FF
#define SERDESCFG_FEID        0x00000100
#define SERDESCFG_EIDD        0x00000200
#define SERDESCFG_P1D         0x00000400
#define SERDESCFG_P2D         0x00000800
#define SERDESCFG_ILPBK_MSK   0x00007000
#define SERDESCFG_ILPBK_OFF   12
#define SERDESCFG_LSE         0x00010000

/* SERDESCFG[14:12] - Internal Loopback Selection */
#define LPBK_DIS              0x0
#define TXRX_PMA_LPBK         0x1
#define SER_CDR_PMA_LPBK      0x2

/* SERDESCFG[16] - Low-Swing Mode Enable */
#define FULL_SWING_MOD        0x0
#define LOW_SWING_MOD         0x1

/* PHYLCFG0 - PHY Link Configuration 0 Reg. (0x530) */
#define PHYLCFG0_LNKNUM_MSK   0x000000FF
#define PHYLCFG0_G1CME        0x00000100
#define PHYLCFG0_SRMBLDIS     0x00000400
#define PHYLCFG0_CLINKDIS     0x00000800
#define PHYLCFG0_PCEC         0x00001000
#define PHYLCFG0_SCLINKEN     0x00002000
#define PHYLCFG0_ILSCC        0x00004000
#define PHYLCFG0_ILTERR       0x00020000
#define PHYLCFG0_ECFGAREC     0x00040000
#define PHYLCFG0_TLW_MSK      0x00380000
#define PHYLCFG0_SLANEREV     0x00400000
#define PHYLCFG0_FLANEREV     0x00800000
#define PHYLCFG0_RDETECT_MSK  0x30000000

/* STMCTL - SerDes Test Mode Control Reg. (0xE54) */
#define STMCTL_CMD_MSK      0x00000007
#define STMCTL_SPEED        0x00000008
#define STMCTL_SPEED_OFF    3

/* STMCTL[2:0] - CMD */
#define NO_OPERATION        0x0
#define ENT_TEST_MOD        0x1
#define EXT_TEST_MOD        0x2
#define FORCE_TEST_MOD      0x5
#define ABORT_CMD           0x7

/* STMCTL[3] - SPEED */
#define GEN1_2_5GTS         0x0
#define GEN2_5_0GTS         0x1


/* STMSTS - SerDes Test Mode Status Reg. (0xE58) */
#define STMSTS_CC           0x00000001
#define STMSTS_PSTATE_MSK   0x00000006
#define STMSTS_PSTATE_OFF   1

/* STMSTS[2:1] - Port State */
#define NORMAL_MOD          0x0
#define ENTER_TEST_MOD      0x1
#define SERDES_TEST_MOD     0x2
#define EXIT_TEST_MOD       0x3


/* STMTCTL - SerDes Test Mode Test Control Reg. (0xE5C) */
#define STMTCTL_TSEL_MSK    0x0000000F
#define STMTCTL_TSYNCP_MSK  0x00000030
#define STMTCTL_TSYNCP_OFF  4

/* STMTCTL[3:0] - Test Select */
#define TSEL_IDLE           0x0
#define LPBK_PCIE_MST       0x1
#define LPBK_10PRBS_MST     0x2
#define LPBK_8PRBS_MST      0x3
#define LPBK_USR_DEF_MST    0x4
#define LPBK_10PCS_SLAVE    0x7

/* STMTCTL[5:4] - TSYNCP */
#define TSYNCP_5US          0x0
#define TSYNCP_10US         0x1
#define TSYNCP_100US        0x2
#define TSYNCP_1MS          0x3

/* STMTSTS - SerDes Test Mode Test Status Reg. (0xE60) */
#define STMTSTS_SYNC_MSK    0x000000FF
#define STMTSTS_TR          0x80000000

/* STMECNT0 - SerDes Test Mode Lane 0 Error Count Reg. (0xE64) */
#define STMECNT_ERR_INJ     0x00010000
#define STMECNT_ERR_DET     0x00000800
#define STMECNT_OVR         0x00000400
#define STMECNT_CNT_MSK     0x000003FF


/* Externs */


#endif	/* _PLATFORM_PCIE_SW_H_ */

/******** History ******** 
$Log: platform_pcie_sw.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/10/10 16:57:02  palin2
Fixed NGWIC TestCard PCIe loopback test based on IDT FAE's suggestion.

Revision 1.1  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

$Endlog$
*/

