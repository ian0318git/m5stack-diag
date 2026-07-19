/* $Id: tsn_comm.h,v 1.4 2018/02/09 09:56:55 hondwang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/tsn_comm.h,v $ 
 *------------------------------------------------------------------
 * 
 * tsn_comm.h
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _TSN_COMM_H_
#define _TSN_COMM_H_

/* Common */
#define ONE_B   1
#define TWO_B   2

#define TSN_WIFI_MODULE                    (1)
#define TSN_DSL_MODULE                     (2)

#define DIAG_TSN_NC_KILL_TMP_FILE          "/tmp/tsn_nc_rm.pid"
#define DIAG_TSN_NC_COMMAND_DISPATCH_FILE  "/tmp/tsn_nc_comm_dispatch"
#define DIAG_TSN_NC_TMP_PARMS_FILE         "/tmp/tsn_nc.parms"
#define TSN_NC_EXEC_LOG_FILE               "/tmp/tsn_nc_exec_log.txt"
#define TSN_NC_DONE_FILE                   "/tmp/tsn_nc_exec_done.txt"

#define DIAG_TSN_NC_RTN_PASS_STR           "PASS"
#define DIAG_TSN_NC_ACK_STR                "ACK"
#define DIAG_TSN_NC_NACK_STR               "NACK"

#define DIAG_TSN_NC_RTN_PARMS_PORT_BASE                    (2288)
#define DIAG_TSN_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE     (2291)
#define DIAG_TSN_NC_EXECUTE_COMMAND_PORT_BASE              (2292)
#define DIAG_TSN_NC_RET_EXEC_DONE_PORT                     (2293)

#define TSN_DIAG_WIFI_SUBNET_STR    "192.168.1"
#define TSN_DIAG_DSL_SUBNET_STR     "192.168.2"
#define TSN_DIAG_HOST_IP_ADDR        (100)
#define TSN_DIAG_GATEWAY_IP_ADDR     (100)
#define TSN_DIAG_MODULE_IP_ADDR      (101)

#define TSN_IOS_DSL_SUBNET_STR     "10.100.0"
#define TSN_IOS_HOST_IP_ADDR        (1)
#define TSN_IOS_GATEWAY_IP_ADDR     (1)
#define TSN_IOS_MODULE_IP_ADDR      (10)

#define TSN_NC_MAX_STR_SIZE          (256)

/* Marvell ARMADA CPU 88F7040 */
/* South Bridge: CP110N */
#define CP_MPP_CTRL_REG(n)           (0xF2440000 + (0x4 * n))
#define CP_GPIO_DATA_OUT_EN_REG(n)   (0xF2440104 + (0x40 * n))
#define CP_GPIO_DATA_IN_POLAR_REG(n) (0xF244010C + (0x40 * n))
#define CP_GPIO_DATA_IN_REG(n)       (0xF2440110 + (0x40 * n))
#define CP0_GPIO8                    0x100
#define CP0_GPIO50                   0x40000    /* 1 << 18 star from bit32*/
#define TSN_FPGA_CPU_INTR            CP0_GPIO8
#define TSN_PLUG_FPGA_CPU_INTR       CP0_GPIO50

#define M7040_SMI_REG                0xF212A200
#define M7040_SMI_BUSY               (1 << 28)
#define M7040_SMI_READ_VALID         (1 << 27)
#define M7040_SMI_OPCODE_RD          (1 << 26)

#define M7040_GENRATION_2_SET_REG(n)	(0xF21208F8 + (0x1000 * n))
#define G2_TX_SSC_AMP_OFFSET			9
#define G2_TX_SSC_AMP_MASK				0x00007E00

#define M7040_USB_PHY2_TX_CTRL_REG(n)	(0xF258000c + (0x1000 * n))
#define USB_PHY2_TX_CTRL_DRV_EN_LS_OFFSET	12
#define USB_PHY2_TX_CTRL_DRV_EN_LS_MASK	0x0000F000

#define CP_PORT_AN_CONFIG_BASEADDR   0xF2130E0C
#define CP_PORT_AN_CONFIG_REG(n)     (CP_PORT_AN_CONFIG_BASEADDR + (0x1000 * n))

#define TSN_UART_BUF_SIZE            1024
#define SKYE_UART_READ_TIMEOUT       1   /* 1 sec */

typedef struct tsn_uart_ {
    char *dev;
    char buf[TSN_UART_BUF_SIZE];
} tsn_uart;

extern void tsn_nc_dispatch_comm(char *, char *);
extern void tsn_nc_init_parms_file(void);
extern int tsn_nc_get_parms(int, char *);
extern int tsn_nc_dispatch_comm_is_ok(void);

/* NC Command Data Structure */
struct nc_command {
    char *cmd_str;
    long (*func)(char *);
};

/* Externs */
extern int  tsn_module;
extern int  tsn_mem_read32(uint, uint *);
extern int  tsn_mem_write32(uint, uint);
extern int  tsn_cpureg_rd_util(int);
extern int  tsn_cpureg_wr_util(int);
extern int  tsn_cpureg_utils(int);
extern int  tsn_tx_uart(char *, char *);
extern int  tsn_rx_polling_uart(char *, char *, int);
extern void tsn_print_spining_wheel(int);
extern int  tsn_rx_uart(char *, int, char *, int);
extern int  check_ext_lpbk_flag(void);

#endif /* TSN_COMM_H_ */

/*-------------------------------------------------
 * $Log: tsn_comm.h,v $
 * Revision 1.4  2018/02/09 09:56:55  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.3.16.1  2018/01/20 06:29:54  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.3  2017/08/25 10:03:57  steja
 * 1.Add Utility to restore back CFE IOS parameter for DF site(CSCvf70937)
 * 2.Add Utility for SPI Write protect
 *
 * Revision 1.2.4.2  2017/12/15 06:36:58  lucywang
 * Modified USB related registers for compliance test
 *
 * Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:50  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.4  2017/08/01 08:32:35  palin2
 * Enhanced TSN WiFi NC mechanism.
 *
 * Revision 1.1.8.3  2017/07/31 16:35:47  palin2
 * Updated WiFi Diag kernel boot up process based on Cisco WiFi bootloader.
 *
 * Revision 1.1.8.2  2017/07/29 03:41:21  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/24 14:14:11  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:08  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.8.6.1  2017/06/19 14:00:28  hondwang
 * Add plug FPGA interrupt testing function
 *
 * Revision 1.1.4.8  2016/10/03 00:38:49  palin2
 * Added utility to dump PSE registers.
 *
 * Revision 1.1.4.7  2016/10/02 20:32:27  palin2
 * Enhanced WiFi uart code to fix CSCvb53793.
 *
 * Revision 1.1.4.6  2016/09/13 08:14:23  palin2
 * Added CPU to GE PHY MAC loopback test.
 *
 * Revision 1.1.4.5  2016/08/16 03:08:17  palin2
 * Unified test pass print outs.
 *
 * Revision 1.1.4.4  2016/07/17 11:15:16  palin2
 * Added function to distinguish bwteen TSN-H and TSN-M.
 *
 * Revision 1.1.4.3  2016/06/30 14:06:32  steja
 * Pick up the latest from tsn-branch1
 *
 * Revision 1.1.4.2  2016/06/30 06:22:52  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.6  2016/06/29 14:14:51  palin2
 * 1. Updated code to support TSN-M.
 * 2. Added utility to set LAN PHY 1000Base-T Test mode.
 *
 * Revision 1.1.2.5  2016/06/17 15:26:25  palin2
 * Added WLAN module diags and utilities.
 *
 * Revision 1.1.2.4  2016/04/24 12:42:56  palin2
 * 1. Updated FPGA registers map.
 * 2. Fixed FPGA force interrupt test.
 * 3. Added FPGA registers dump utility.
 *
 * Revision 1.1.2.3  2016/04/22 12:28:37  palin2
 * Updated code after bring up GE PHY external loopback test.
 *
 * Revision 1.1.2.2  2016/04/19 07:37:59  palin2
 * Added utilities to access TSN CPU register(s).
 *
 * $Endlog$
 *-------------------------------------------------
 */
