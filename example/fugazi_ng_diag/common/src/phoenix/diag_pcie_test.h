/* $Id: diag_pcie_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_pcie_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_pcie_test.h - PCIe test header file.
 *
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define PCIE_SCAN_CMD       "/opt/script/generic_pcie_lane.sh 2> /dev/null"
#define PCIE_LIST_LOG       "/tmp/phoenix_pcie_lane_err.txt"
#define PCIE_NAME           "C3000 Series PCI Express Root Port"

/* Phoenix PCIe Root Port Mapping */
#define PCIE_RP_M2          2
#define PCIE_RP_LOGIC_FPGA  3
#define PCIE_RP_NIM1        4
#define PCIE_RP_NIM0        5
#define PCIE_RP_I350        6
#define PCIE_RP_TDM_FPGA    7

extern int diag_pcie_scan_test(int);

