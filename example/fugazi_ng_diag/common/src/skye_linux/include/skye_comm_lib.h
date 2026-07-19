/* $Id: skye_comm_lib.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_comm_lib.h,v $
 *------------------------------------------------------------------
 * Filename: skye_comm_lib.h
 *
 * Description: Skye Communication Library related definitions
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef SKYE_COMM_LIB_H_
#define SKYE_COMM_LIB_H_

/* NC Command Data Structure */
struct nc_command {
    char *cmd_str;
    long (*func)(char *);
};

/* Common Definitions */
#define SKYE_NC_CONN_ITVL                 1
#define DIAG_RTN_PASS_STR                 "PASS"
#define DIAG_RTN_FAIL_STR                 "FAIL"
#define SKYE_NC_SET_SIZE                  3

/* Skye NC file path Definitions */
#define DIAG_KILL_NC_TMP_FILE             "/tmp/skye_rm.pid"
#define DIAG_CMD_DISPATCH_FILE            "/tmp/skye_comm_dispatch"
#define SKYE_NC_DONE_FP                   "/tmp/skye_nc_done"
#define SKYE_NC_DBLOG_FILE                "/dblog.txt"
#define FREQ_MARGIN_TMP_RESULT            "/tmp/skye_fm_result"

/* Skye NC port number Definitions */
#define DIAG_RUN_ALL_PORT_BASE            (2390)
#define DIAG_RTN_STS_OUT_PORT_BASE        (2391)
#define DIAG_RTN_DBLOG_PORT_BASE          (2392)
#define DIAG_EXEC_CMD_TRANS_PORT_BASE     (2398)
#define DIAG_EXEC_CMD_PORT_BASE           (2399)
#define SKYE_NC_DONE_PORT                 (2400)

/* NC Command Dispatch */
#define DIAG_CMD_ALIVE_CHECK              "alive_check"
#define DIAG_DO_ALL_TEST                  "do_all_test"
#define DIAG_DO_MEM_TEST                  "do_mem_test"
#define DIAG_DO_FPGA_TEST                 "do_fpga_test"
#define DIAG_DO_SPIROM_TEST               "do_spirom_test"
#define DIAG_DO_I2CDEV_TEST               "do_i2cdev_test"
#define DIAG_DO_TLK_TEST                  "do_tlk_test"
#define DIAG_DO_PCIE_TEST                 "do_pcie_test"

/*
 * Externs
 */
extern void skye_nc_dispatch_comm(char *);

#endif   /* SKYE_COMM_LIB_H_ */


/*------------------------------------------------------------------
 * $Log: skye_comm_lib.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:28  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.7  2015/01/26 01:14:14  steja
 * Add function for frequency margin to host side menu utilities through NC
 *
 * Revision 1.1.2.6  2014/11/27 07:25:01  palin2
 * Added PCIe lanes Scan test to 2-CPUs Skye default tests.
 *
 * Revision 1.1.2.5  2014/11/27 02:31:09  steja
 * Fix the intermittent failure to run do all test(CSCur27613)
 *
 * Revision 1.1.2.4  2014/09/18 07:18:12  steja
 * Update NC command code
 *
 * Revision 1.1.2.3  2014/08/28 02:54:09  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.2  2014/08/21 02:22:30  palin2
 * Support passing Flags setup from host side by NC command in Skye.
 *
 * Revision 1.1.2.1  2014/08/15 03:27:37  palin2
 * Initial check-in to support NC command on Skye.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

