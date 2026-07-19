/* $Id: diag_i350_test.h,v 1.2 2019/10/17 02:16:22 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i350_test.h,v $
 *-----------------------------------------------------------------------------
 * i350_test.h - Header for Intel I350 Test.
 *
 *
 * July 2018-2019, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define INSERT_NAL_DRIVER "nal"

#define I350_PORT0_INT_LPBK "/opt/script/i350_test.sh 1 lb"
#define I350_PORT1_INT_LPBK "/opt/script/i350_test.sh 2 lb"
#define I350_PORT2_INT_LPBK "/opt/script/i350_test.sh 3 lb"
#define I350_PORT3_INT_LPBK "/opt/script/i350_test.sh 4 lb"

#define I350_PORT0_EXT_LPBK "/opt/script/i350_test.sh 1 extlb"
#define I350_PORT1_EXT_LPBK "/opt/script/i350_test.sh 2 extlb"

#define I350_PORT0_REG_TEST "/opt/script/i350_test.sh 1 regs"
#define I350_PORT1_REG_TEST "/opt/script/i350_test.sh 2 regs"
#define I350_PORT2_REG_TEST "/opt/script/i350_test.sh 3 regs"
#define I350_PORT3_REG_TEST "/opt/script/i350_test.sh 4 regs"

#define I350_PORT0_EEPROM_TEST "/opt/script/i350_test.sh 1 eeprom"
#define I350_PORT1_EEPROM_TEST "/opt/script/i350_test.sh 2 eeprom"
#define I350_PORT2_EEPROM_TEST "/opt/script/i350_test.sh 3 eeprom"
#define I350_PORT3_EEPROM_TEST "/opt/script/i350_test.sh 4 eeprom"

#define I350_PORT0 0
#define I350_PORT1 1
#define I350_PORT2 2
#define I350_PORT3 3

#define I350_GET_TEST_RESULT    "cat /tabei-diag/i350_test_result.txt"
#define I350_GET_TEST_LOG       "cat /tabei-diag/i350_test.LOG"
#define I350_GET_TEST_PASS_LOG  "cat /tabei-diag/i350_test.LOG | sed '1d'"


#define I350_GET_NIC "celo64e /devices | grep I350"
#define I350_TEST_RESULT "/tabei-diag/i350_test_result.txt"
#define I350_TEST_LOG "/tabei-diag/i350_test.LOG"
#define I350_FAIL "failed"

#define I350_FW_PROGRAMMING "/opt/script/i350_fw_program.sh"

#define WAIT_I350_REG_TEST  200
#define WAIT_I350_ETH_TEST  3000

#define TABEIL_I350_PCIE_BUS                        "/sys/devices/pci0000:00/0000:00:10.0/"
#define TABEIL_I350_PCIE_PORT2                      "0000:0a:00.2/resource0"
#define TABEIL_I350_PCIE_PORT3                      "0000:0a:00.3/resource0"
#define PROMETHIUM_I350_PCIE_BUS                    "/sys/devices/pci0000:00/0000:00:10.0/"
#define PROMETHIUM_I350_PCIE_PORT2                  "0000:23:00.2/resource0"
#define PROMETHIUM_I350_PCIE_PORT3                  "0000:23:00.3/resource0"
#define TABEIL_I350_CTRL                             0x0
#define TABEIL_I350_CTRL_EXT                         0x18
#define TABEIL_I350_CTRL_EXT_LINK_MODE_MASK          0xC00000
#define TABEIL_I350_CTRL_EXT_LINK_MODE_SGMII         0x800000
#define TABEIL_I350_SFP_PRESENT                      0x40000
#define TABEIL_I350_SFP_DISABLE                      0x80000
#define TABEIL_I350_SFP_TX_FAULT                     0x40
#define PHY_IDENTIFIER_REG                           0x3
#define PHY_IDENTIFIER_MASK                          0xFFF0
#define PHY88E1111_IDENTIFIER                        0xCC0

#define PHY88E1111_CTRL_REG                             0x0
#define PHY88E1111_1000BASET_CTRL_REG                   0x9
#define PHY88E1111_INTERRUPT_EN_REG                     0x12
#define PHY88E1111_EXTEND_PHY_SPE_STATE_REG             0x1B
#define PHY88E1111_EXTEND_ADDR_REG                      0x1D
#define PHY88E1111_EXTENDED_REG                         0x1E

#define PHY88E1111_DISABLE_INTERRUPT                    0x0000
#define PHY88E1111_EXTEND_PAGE7                         0x0007
#define PHY88E1111_EXTEND_PAGE16                        0x0010
#define PHY88E1111_EXTEND_PAGE18                        0x0012
#define PHY88E1111_CHANGE_NON_BGIC                      0x808C
#define PHY88E1111_SOFTWARE_INIT                        0x8140
#define PHY88E1111_DIASABLE_LPBK                        0x0000
#define PHY88E1111_MASTER_CONFIG                        0x1800
#define PHY88E1111_SOFTWARE_RESET                       0x8000
#define PHY88E1111_FORCE_GIGABIT_MODE                   0x0008
#define PHY88E1111_EN_GIGABIT_STUB_LPBK                 0x0002
#define PHY88E1111_DISABLE_NEXT_CANCELLER               0x0001

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

extern int diag_i350_test (int);

/*-------------------------------------------------
$Log: diag_i350_test.h,v $
Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.10  2019/09/18 01:27:06  kehuang2
Suppout the PHY chip config on sfp for I350

Revision 1.1.2.9  2019/09/03 09:04:03  olin2
Enhance SFP loopback test

Revision 1.1.2.8  2019/07/30 02:03:44  olin2
Enhance SFP loopback test

Revision 1.1.2.7  2019/07/26 08:25:33  olin2
Code clean up

Revision 1.1.2.6  2019/03/27 08:56:16  kehuang2
Clean up code for Promethium

Revision 1.1.2.5  2019/02/25 09:47:34  harrchan
For different network config in Cisco Bios0.5

Revision 1.1.2.4  2018/12/04 03:11:19  olin2
Update I350 interface name for Cisco BIOS

Revision 1.1.2.3  2018/11/09 07:16:43  olin2
Update I350 ethernet name

Revision 1.1.2.2  2018/11/01 01:48:26  olin2
Support I350 test

Revision 1.1.2.1  2018/10/03 06:06:38  olin2
Initial commit for I350 test



$Endlog $
*/
