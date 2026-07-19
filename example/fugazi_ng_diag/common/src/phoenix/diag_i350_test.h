/* $Id: diag_i350_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i350_test.h,v $
 *------------------------------------------------------------------
 * i350_test.h - Header for Intel I350 Test.
 *
 *
 * July 2018-2019, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define I350_PORT0_INT_LPBK "/opt/script/i350_test.sh 1 lb"
#define I350_PORT1_INT_LPBK "/opt/script/i350_test.sh 2 lb"

#define I350_PORT0_EXT_LPBK "/opt/script/i350_test.sh 1 extlb"
#define I350_PORT1_EXT_LPBK "/opt/script/i350_test.sh 2 extlb"

#define I350_PORT0_1G_EXT_LPBK "/opt/script/i350_test.sh 1 extlb 1G"
#define I350_PORT1_1G_EXT_LPBK "/opt/script/i350_test.sh 2 extlb 1G"

#define I350_PORT0_REG_TEST "/opt/script/i350_test.sh 1 regs"
#define I350_PORT1_REG_TEST "/opt/script/i350_test.sh 2 regs"

#define I350_PORT0 0
#define I350_PORT1 1

#define I350_GET_TEST_RESULT    "cat /phoenix-diag/i350_test_result.txt"
#define I350_GET_TEST_LOG       "cat /phoenix-diag/i350_test.LOG"
#define I350_GET_TEST_PASS_LOG  "cat /phoenix-diag/i350_test.LOG | sed '1d'"


#define I350_GET_NIC "celo64e /devices | grep I350"
#define I350_TEST_RESULT "/phoenix-diag/i350_test_result.txt"
#define I350_TEST_LOG "/phoenix-diag/i350_test.LOG"
#define I350_FAIL "failed"

#define WAIT_I350_REG_TEST  200
#define WAIT_I350_ETH_TEST  3000

void phoenix_show_i350_ver(void);
