/* $Id: i350_test.h,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/i350_test.h,v $
 *-----------------------------------------------------------------------------
 * i350_test.h - Header for Intel I350 Test.
 *
 *
 * July 2018, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define INSERT_NAL_DRIVER "/diag_utils/intel-eeupdate-tool/nal"

#define I350_PORT1_INT_LPBK "/diag_utils/curie/scripts/i350_test.sh 1 lb"
#define I350_PORT2_INT_LPBK "/diag_utils/curie/scripts/i350_test.sh 2 lb"
#define I350_PORT3_INT_LPBK "/diag_utils/curie/scripts/i350_test.sh 3 lb"
#define I350_PORT4_INT_LPBK "/diag_utils/curie/scripts/i350_test.sh 4 lb"

#define I350_PORT1_EXT_LPBK "/diag_utils/curie/scripts/i350_test.sh 1 extlb"
#define I350_PORT2_EXT_LPBK "/diag_utils/curie/scripts/i350_test.sh 2 extlb"
#define I350_PORT3_EXT_LPBK "/diag_utils/curie/scripts/i350_test.sh 3 extlb"
#define I350_PORT4_EXT_LPBK "/diag_utils/curie/scripts/i350_test.sh 4 extlb"

#define I350_PORT1_REG_TEST "/diag_utils/curie/scripts/i350_test.sh 1 regs"
#define I350_PORT2_REG_TEST "/diag_utils/curie/scripts/i350_test.sh 2 regs"
#define I350_PORT3_REG_TEST "/diag_utils/curie/scripts/i350_test.sh 3 regs"
#define I350_PORT4_REG_TEST "/diag_utils/curie/scripts/i350_test.sh 4 regs"

#define I350_PORT1_EEPROM_TEST "/diag_utils/curie/scripts/i350_test.sh 1 eeprom"
#define I350_PORT2_EEPROM_TEST "/diag_utils/curie/scripts/i350_test.sh 2 eeprom"
#define I350_PORT3_EEPROM_TEST "/diag_utils/curie/scripts/i350_test.sh 3 eeprom"
#define I350_PORT4_EEPROM_TEST "/diag_utils/curie/scripts/i350_test.sh 4 eeprom"

#define RM_CELO_LOGS_FILE "rm /celo_i350_logs.txt"
#define CELO_LOGS_FILE "/celo_i350_logs.txt"

#define I350_SKIP_PORT0 0
#define I350_REST_PORT 1
#define DO_I350_PORT0 2

#define I350_PORT0 0
#define I350_PORT1 1
#define I350_PORT2 2
#define I350_PORT3 3
#define I350_PORT4 4

#define DISPLAY_I350_PORT1_CAP "ethtool eth0"
#define DISPLAY_I350_PORT2_CAP "ethtool eth1"
#define DISPLAY_I350_PORT3_CAP "ethtool eth2"
#define DISPLAY_I350_PORT4_CAP "ethtool eth3"

#define I350_SHOW_RUNNING_ETH "/diag_utils/curie/scripts/detect_nic_external_module.sh -d"
#define I350_GET_NIC "/diag_utils/curie/scripts/get_i350_nic_number.sh all"
#define I350_TEST_RESULT "/curie-1RU-diag/i350_test_result.txt"
#define I350_PASS "passed"

#define I350_FW_PROGRAMMING "/diag_utils/curie/scripts/i350_fw_program.sh"
#define BUFFER_ARRAY_SIZE 0x80
extern int i350_test (int);

/*-------------------------------------------------
$Log: i350_test.h,v $
Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.4  2019/06/04 06:49:15  leschen
Remove celo logs file at the beginning of lpbk test

Revision 1.1.2.3  2018/12/27 09:45:56  leschen
Based on PRRQ CSCvn30794-2 comments to modify the codes.

Revision 1.1.2.2  2018/08/29 07:07:14  leschen
Modify I350 tests/menu acrhitecture to be more reasonable and support polonium.

Revision 1.1.2.1  2018/08/14 19:11:17  leschen
Supporting Curie I350 diag tests and utility


$Endlog $
*/
