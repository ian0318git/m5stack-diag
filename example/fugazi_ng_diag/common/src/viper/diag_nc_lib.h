 /* $Id: diag_nc_lib.h,v 1.3 2018/08/31 03:59:30 chieyang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_nc_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_nc_lib.h
 *
 * Copyright (c) 2017-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DIAG_NC_LIB_H_
#define _DIAG_NC_LIB_H_

/* Common */
//#define DEBUG_MESSAGE
#define ONE_B   1
#define TWO_B   2


#define DIAG_VIPER_NC_KILL_TMP_FILE          "/tmp/viper_nc_rm.pid"
#define DIAG_VIPER_NC_COMMAND_DISPATCH_FILE  "/tmp/viper_nc_comm_dispatch"
#define DIAG_VIPER_NC_TMP_PARMS_FILE         "/tmp/viper_nc.parms"
#define VIPER_NC_EXEC_LOG_FILE               "/tmp/viper_nc_exec_log.txt"
#define VIPER_NC_DONE_FILE                   "/tmp/viper_nc_exec_done.txt"

#define DIAG_VIPER_NC_RTN_PASS_STR           "PASS"
#define DIAG_VIPER_NC_ACK_STR                "ACK"
#define DIAG_VIPER_NC_NACK_STR               "NACK"

#define DIAG_VIPER_NC_RTN_PARMS_PORT_BASE                    (2288)
#define DIAG_VIPER_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE     (2291)
#define DIAG_VIPER_NC_EXECUTE_COMMAND_PORT_BASE              (2292)
#define DIAG_VIPER_NC_RET_EXEC_DONE_PORT                     (2293)

#define DIAG_DSL_SUBNET_STR     "192.168.2"
#define DIAG_HOST_IP_ADDR        (100)
#define DIAG_GATEWAY_IP_ADDR     (100)
#define DIAG_MODULE_IP_ADDR      (101)

#define VIPER_IOS_DSL_SUBNET_STR     "192.168.2"
#define VIPER_IOS_HOST_IP_ADDR        (100)
#define VIPER_IOS_GATEWAY_IP_ADDR     (100)
#define VIPER_IOS_MODULE_IP_ADDR      (101)

#define VIPER_NC_MAX_STR_SIZE          (256)


/* NC Command Data Structure */
struct nc_command {
    char *cmd_str;
    long (*func)(char *);
};


/* Externs */
extern void viper_nc_dispatch_comm(char *, char *);
extern void viper_nc_init_parms_file(void);
extern int  viper_nc_get_parms(int, char *);
extern int  viper_nc_dispatch_comm_is_ok(void);
extern int  check_ext_lpbk_flag(void);

#endif /* __DIAG_NC_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_nc_lib.h,v $
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.2  2018/06/11 02:39:14  olin2
 * Updated restore CFE IOS parameter util
 *
 * Revision 1.1.2.1  2018/02/27 08:06:46  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
