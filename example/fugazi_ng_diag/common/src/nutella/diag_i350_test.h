/* $Id: diag_i350_test.h,v 1.5 2019/12/19 07:16:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_i350_test.h,v $
 *-----------------------------------------------------------------------------
 * i350_test.h - Header for Intel I350 Test.
 *
 * July 2018, Leschen
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define INSERT_NAL_DRIVER "nal"

#define I350_PORT1_INT_LPBK "/opt/script/i350_test.sh 1 lb"
#define I350_PORT2_INT_LPBK "/opt/script/i350_test.sh 2 lb"

#define I350_PORT1_EXT_LPBK "/opt/script/i350_test.sh 1 extlb"
#define I350_PORT2_EXT_LPBK "/opt/script/i350_test.sh 2 extlb"

#define I350_PORT1_REG_TEST "/opt/script/i350_test.sh 1 regs"
#define I350_PORT2_REG_TEST "/opt/script/i350_test.sh 2 regs"

#define I350_PORT1_EEPROM_TEST "/opt/script/i350_test.sh 1 eeprom"
#define I350_PORT2_EEPROM_TEST "/opt/script/i350_test.sh 2 eeprom"

#define I350_PORT1 1
#define I350_PORT2 2

#define DISPLAY_I350_PORT1_CAP "ethtool enp3s0f0"
#define DISPLAY_I350_PORT2_CAP "ethtool enp3s0f1"

#define I350_PORT1_UP          "ifconfig enp3s0f0 up"
#define I350_PORT2_UP          "ifconfig enp3s0f1 up"
#define I350_PORT1_DOWN        "ifconfig enp3s0f0 down"
#define I350_PORT2_DOWN        "ifconfig enp3s0f1 down"

#define I350_INTERFACE_NAME_PORT1  "enp3s0f0"
#define I350_INTERFACE_NAME_PORT2  "enp3s0f1"

#define I350_SHOW_RUNNING_ETH "/opt/script/detect_nic_external_module.sh -d"
#define I350_GET_NIC "/opt/script/get_i350_nic_number.sh all"
#define I350_TEST_RESULT "/nutella-diag/i350_test_result.txt"
#define I350_TEST_LOG "/nutella-diag/i350_test.LOG"
#define I350_FAIL "failed"

#define I350_FW_PROGRAMMING "/opt/script/i350_fw_program.sh"

#define WAIT_I350_TEST      200
#define WAIT_I350_ETH_TEST  3000

#define I350_IOCTL_COMMAND2    (SIOCDEVPRIVATE+2)
#define I350_IOCTL_COMMAND3    (SIOCDEVPRIVATE+3)



#define NUTELLA_I350_PCIE_BUS                        "/sys/devices/pci0000:00/0000:00:10.0/"
#define NUTELLA_I350_PCIE_PORT1                      "0000:03:00.0/resource0"
#define NUTELLA_I350_PCIE_PORT2                      "0000:03:00.1/resource0"
#define NUTELLA_I350_CTRL                             0x0
#define NUTELLA_I350_CTRL_EXT                         0x18
#define NUTELLA_I350_CTRL_EXT_LINK_MODE_MASK          0xC00000
#define NUTELLA_I350_CTRL_EXT_LINK_MODE_SGMII         0x800000
#define NUTELLA_I350_SFP_PRESENT                      0x40000
#define NUTELLA_I350_SFP_DISABLE                      0x80000
#define NUTELLA_I350_SFP_TX_FAULT                     0x40

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

extern int diag_i350_test (int);

/*-------------------------------------------------
$Log: diag_i350_test.h,v $
Revision 1.5  2019/12/19 07:16:51  harrchan
1.Add utility to dump SFP present pin status 2.Add utility to enable/disable SFP module(CSCvs46746)

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
