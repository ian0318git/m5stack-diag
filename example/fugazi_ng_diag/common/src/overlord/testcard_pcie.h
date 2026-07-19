/* $Id: testcard_pcie.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_pcie.h,v $
 *--------------------------------------------------------------------
 * Filename   : testcard_pcie.h
 *
 * Description: Specific header file of Overlord Test Card PCIe.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __TESTCARD_PCIE_H__
#define __TESTCARD_PCIE_H__

/* Common definition */
#define PCIE_LANE0   0
#define PCIE_LANE1   1
#define PCIE_LANE2   2
#define PCIE_LANE3   3

#define TC_PCIE_LPBK_OVSWING   0x03

/* PCIe ReDriver, PI2EQX5964, Registers */
#define PI2EQX5964_REG_SIZE  12

/* PCIe ReDriver, PI2EQX5964, Reg. offset */
#define SIG_REG_OFFSET        0
#define RX50_REG_OFFSET       1
#define LBEC_REG_OFFSET       2
#define INDIS_REG_OFFSET      3
#define OUTDIS_REG_OFFSET     4
#define RESET_REG_OFFSET      5
#define PWR_REG_OFFSET        6
#define RXDETEN_REG_OFFSET    7
#define AEOC_REG_OFFSET       8
#define BEOC_REG_OFFSET       9
#define RESV_REG_OFFSET      10
#define IDL_DET_REG_OFFSET   11

/* Byte2 - Loopback and Emphasis Control Reg.(LBEC) */
#define LB_A0B0       0x80
#define LB_A1B1       0x40
#define LB_A2B2       0x20
#define LB_A3B3       0x10
#define DE_A          0x08
#define DE_B          0x04
#define LB_A0B0_SFT   7
#define LB_A1B1_SFT   6
#define LB_A2B2_SFT   5
#define LB_A3B3_SFT   4

/* Byte3 - Channel Input Disable Reg.(INDIS) */
#define INDIS_A0      0x80
#define INDIS_B0      0x40
#define INDIS_A1      0x20
#define INDIS_B1      0x10
#define INDIS_A2      0x08
#define INDIS_B2      0x04
#define INDIS_A3      0x02
#define INDIS_B3      0x01
#define INDIS_A0_SFT  7
#define INDIS_B0_SFT  6
#define INDIS_A1_SFT  5
#define INDIS_B1_SFT  4
#define INDIS_A2_SFT  3
#define INDIS_B2_SFT  2
#define INDIS_A3_SFT  1

/* Byte4 - Channel Output Disable Reg.(OUTDIS) */
#define ODIS_A0   0x80
#define ODIS_B0   0x40
#define ODIS_A1   0x20
#define ODIS_B1   0x10
#define ODIS_A2   0x08
#define ODIS_B2   0x04
#define ODIS_A3   0x02
#define ODIS_B3   0x01

#if 0
/* FPGA Reg. definition */
/* FPGA ID Reg. (+0x00h) */
#define FPGA_DEBUG_SHIFT     15
#define FPGA_DEBUG_MSK       0x8000
#define FPGA_MAJ_REV_SHIFT   8
#define FPGA_MAJ_REV_MSK     0x7F00
#define FPGA_MIN_REG_MSK     0x00FF
#endif


/* Externs */
extern int  tc_read_pcie_redriver_reg(void);
extern int  tc_alter_pcie_redriver_reg(uint, uint8_t, boolean);


#endif /* __TESTCARD_PCIE_H__ */

/* ------- End of file ------- */

/******** History ******** 
$Log: testcard_pcie.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

Revision 1.1  2012/07/31 17:08:20  palin2
Initial check-in for TestCard PCIe tests.

$Endlog$
*/

