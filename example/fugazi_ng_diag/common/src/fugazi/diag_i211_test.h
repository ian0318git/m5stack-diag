/* $Id: diag_i211_test.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_i211_test.h,v $
 *------------------------------------------------------------------
 * i211_test.h - Header for Intel I211 Test.
 *
 *
 * Jan 2019, Ian Chang
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define INSERT_NAL_DRIVER "/diag_utils/intel-eeupdate-tool/nal"

#define I211_PORT1_INT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 12 lb"
#define I211_PORT2_INT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 2 lb"
#define I211_PORT3_INT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 3 lb"
#define I211_PORT4_INT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 4 lb"

#define I211_PORT1_EXT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 12 extlb"
#define I211_PORT2_EXT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 2 extlb"
#define I211_PORT3_EXT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 3 extlb"
#define I211_PORT4_EXT_LPBK "/diag_utils/fugazi/scripts/i211_test.sh 4 extlb"

#define I211_PORT1_REG_TEST "/diag_utils/fugazi/scripts/i211_test.sh 12 regs"
#define I211_PORT2_REG_TEST "/diag_utils/fugazi/scripts/i211_test.sh 2 regs"
#define I211_PORT3_REG_TEST "/diag_utils/fugazi/scripts/i211_test.sh 3 regs"
#define I211_PORT4_REG_TEST "/diag_utils/fugazi/scripts/i211_test.sh 4 regs"

#define I211_PORT1_EEPROM_TEST "/diag_utils/fugazi/scripts/i211_test.sh 12 eeprom"
#define I211_PORT2_EEPROM_TEST "/diag_utils/fugazi/scripts/i211_test.sh 2 eeprom"
#define I211_PORT3_EEPROM_TEST "/diag_utils/fugazi/scripts/i211_test.sh 3 eeprom"
#define I211_PORT4_EEPROM_TEST "/diag_utils/fugazi/scripts/i211_test.sh 4 eeprom"

#define I211_SKIP_PORT0 0
#define I211_REST_PORT  1
#define DO_I211_PORT0   0

#define I211_PORT0      0
#define I211_PORT1      1
#define I211_PORT2      2
#define I211_PORT3      3
#define I211_PORT4      4

#define DISPLAY_I211_PORT1_CAP "ethtool eth12"
#define DISPLAY_I211_PORT2_CAP "ethtool eth1"
#define DISPLAY_I211_PORT3_CAP "ethtool eth2"
#define DISPLAY_I211_PORT4_CAP "ethtool eth3"

#define I211_SHOW_RUNNING_ETH "/diag_utils/fugazi/scripts/detect_nic_external_module.sh -d"
#define I211_GET_NIC "/diag_utils/fugazi/scripts/get_i211_nic_number.sh all"
#define I211_TEST_RESULT "/fugazi-diag/I211_test_result.txt"
#define I211_PASS "passed"

#define I211_FW_PROGRAMMING "source /diag_utils/fugazi/scripts/function.sh; i211_upgrade"
extern int i211_test (int);

/*-------------------------------------------------
 * $Log: diag_i211_test.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.5  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.4  2019/03/29 18:28:44  iachang
 * support i211 firmware upgrade
 *
 * Revision 1.1.6.3  2019/03/21 21:44:12  iachang
 * Bring up I211 test, skip loopabck test with default test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */

