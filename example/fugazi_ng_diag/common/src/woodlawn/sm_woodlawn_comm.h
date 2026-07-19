/* $Id: sm_woodlawn_comm.h,v 1.3 2015/02/14 12:48:41 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn_comm.h,v $
 *------------------------------------------------------------------
 * Filename: sm_woodlawn_comm.h
 *
 * Description: SM Woodlawn Communication Library
 * Author: Times Huang
 *
 * Copyright (c) 2013 - 2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef SM_WOODLAWN_COMM_H_
#define SM_WOODLAWN_COMM_H_

#define DIAG_KILL_NC_TMP_FILE               "/tmp/sm_woodlawm_rm.pid"
#define DIAG_COMMAND_DISPATCH_FILE          "/tmp/sm_woodlawn_comm_dispatch"

#define DIAG_RTN_PASS_STR                   "PASS"

#define DIAG_SEND_DIAG_VER_PORT_BASE                (2389)
#define DIAG_RUN_ALL_PORT_BASE                      (2390)
#define DIAG_RTN_STS_OUT_PORT_BASE                  (2391)
#define DIAG_VOLTAGE_MARGIN_STATUS_PORT_BASE        (2395)
#define DIAG_VOLTAGE_MARGIN_CTRL_PORT_BASE          (2396)
#define DIAG_EXECUTE_COMMAND_TRANSFER_PORT_BASE     (2398)
#define DIAG_EXECUTE_COMMAND_PORT_BASE              (2399)

/* NC Command Dispatch */
#define DIAG_COMMAND_VOLTAGE_MARGIN_SET             "volmarg_set"
#define DIAG_COMMAND_VOLTAGE_MARGIN_GET             "volmarg_get"
#define DIAG_COMMAND_LED_SET                        "led_set"
#define DIAG_COMMAND_1340_REG_READ                  "phy_1340_read"
#define DIAG_COMMAND_1340_REG_WRITE                 "phy_1340_write"
#define DIAG_COMMAND_1548_REG_READ                  "phy_1548_read"
#define DIAG_COMMAND_1548_REG_WRITE                 "phy_1548_write"
#define DIAG_COMMAND_1548_PTP_REG_READ              "phy_1548_ptp_read"
#define DIAG_COMMAND_1548_PTP_REG_WRITE             "phy_1548_ptp_write"
#define DIAG_COMMAND_2222_REG_READ                  "phy_2222_read"
#define DIAG_COMMAND_2222_REG_WRITE                 "phy_2222_write"
#define DIAG_COMMAND_2222_PTP_REG_READ              "phy_2222_ptp_read"
#define DIAG_COMMAND_2222_PTP_REG_WRITE             "phy_2222_ptp_write"
#define DIAG_COMMAND_SM_FPGA_REG_READ               "sm_fpga_read"
#define DIAG_COMMAND_SM_FPGA_REG_WRITE              "sm_fpga_write"
#define DIAG_COMMAND_10232_REG_READ                 "tlk10232_read"
#define DIAG_COMMAND_10232_REG_WRITE                "tlk10232_write"
#define DIAG_COMMAND_CAVIUM_SGMII_PORT_STATUS       "cavium_sgmii_port_status"
#define DIAG_COMMAND_CAVIUM_SGMII_PORT_CONFIG       "cavium_sgmii_port_CONFIG"
#define DIAG_COMMAND_CAVIUM_SGMII_REG_READ          "cavium_sgmii_reg_read"
#define DIAG_COMMAND_CAVIUM_XAUI_PORT_STATUS        "cavium_xaui_port_status"
#define DIAG_COMMAND_CAVIUM_XAUI_GMX_REG_READ       "cavium_xaui_gmx_reg_read"
#define DIAG_COMMAND_CAVIUM_XAUI_PCS_REG_READ       "cavium_xaui_pcs_reg_read"
#define DIAG_COMMAND_BOOTFLASH_GET_INFO             "bootflash_get_info"
#define DIAG_COMMAND_BOOTFLASH_OTP_TEST             "bootflash_otp_test"
#define DIAG_COMMAND_ENABLE_1548_PTP_ENGINE         "enable_1548_ptp_engine"
#define DIAG_COMMAND_VERIFY_1548_PTP                "verify_1548_ptp"
#define DIAG_COMMAND_VERIFY_1548_CLK_TRIG_IN        "verify_1548_clk_trig_in"
#define DIAG_COMMAND_VERIFY_2222_CLK_TRIG_IN        "verify_2222_clk_trig_in"
#define DIAG_COMMAND_CONFIG_1548_GEN_CLK_OUT        "config_1548_gen_clk_out"
#define DIAG_COMMAND_CONFIG_2222_GEN_CLK_OUT        "config_2222_gen_clk_out"
#define DIAG_COMMAND_CONFIG_1548_GEN_TRIG_OUT       "config_1548_gen_trig_out"
#define DIAG_COMMAND_CONFIG_2222_GEN_TRIG_OUT       "config_2222_gen_trig_out"
#define DIAG_COMMAND_VERIFY_FPGA_SYNC_CLK_OUT       "verify_fpga_sync_clk_out"
#define DIAG_COMMAND_VERIFY_FPGA_SYNC_TRIG_OUT      "verify_fpga_sync_trig_out"
#define DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_GE0        "config_fpga_clk_mux_ge0"
#define DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_GE1        "config_fpga_clk_mux_ge1"
#define DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_X2222P     "config_fpga_clk_mux_x2222p"
#define DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_GE0       "config_fpga_trig_mux_ge0"
#define DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_GE1       "config_fpga_trig_mux_ge1"
#define DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_X2222P    "config_fpga_trig_mux_x2222p"

/* NC LED Set opcode */
#define DIAG_LED_TURN_OP_ON                         (100)
#define DIAG_LED_TURN_OP_OFF                        (101)
#define DIAG_LED_TURN_OP_BLINKING                   (102)

#define DIAG_LED_SET_GE0_SPD                        (0)
#define DIAG_LED_SET_GE0_LINK                       (1)
#define DIAG_LED_SET_SFP0_EN                        (2)
#define DIAG_LED_SET_SFP0_S                         (3)
#define DIAG_LED_SET_GE1_SPD                        (4)
#define DIAG_LED_SET_GE1_LINK                       (5)
#define DIAG_LED_SET_SFP1_EN                        (6)
#define DIAG_LED_SET_SFP1_S                         (7)
#define DIAG_LED_SET_GE2_SPD                        (8)
#define DIAG_LED_SET_GE2_LINK                       (9)
#define DIAG_LED_SET_SFP2_EN                        (10)
#define DIAG_LED_SET_SFP2_S                         (11)
#define DIAG_LED_SET_GE3_SPD                        (12)
#define DIAG_LED_SET_GE3_LINK                       (13)
#define DIAG_LED_SET_SFP3_EN                        (14)
#define DIAG_LED_SET_SFP3_S                         (15)
#define DIAG_LED_SET_GE4_SPD                        (16)
#define DIAG_LED_SET_GE4_LINK                       (17)
#define DIAG_LED_SET_SFP4_EN                        (18)
#define DIAG_LED_SET_SFP4_S                         (19)
#define DIAG_LED_SET_GE5_SPD                        (20)
#define DIAG_LED_SET_GE5_LINK                       (21)
#define DIAG_LED_SET_SFP5_EN                        (22)
#define DIAG_LED_SET_SFP5_S                         (23)
#define DIAG_LED_SET_SFP_PLUS_EN                    (50)
#define DIAG_LED_SET_SFP_PLUS_SPD                   (51)

extern int woodlawn_do_all(void);
extern void woodlawn_transmit_nc_request(int);
extern void woodlawn_nc_dispatch_comm(char *);

/* NC Command Data Structure */
struct nc_command {
    char *cmd_str;
    long (*func)(char *);
};

#endif /* SM_WOODLAWN_COMM_H_ */



/*------------------------------------------------------------------
 * $Log: sm_woodlawn_comm.h,v $
 * Revision 1.3  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.2.8.2  2014/04/30 13:47:20  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.2.8.1  2014/01/03 06:40:31  leschen
 * Define ptp macro
 *
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.6  2013/07/02 07:57:31  leschen
 * Add bootflsh OTP nc command macros
 *
 * Revision 1.1.2.5  2013/06/17 10:24:51  leschen
 * Implement NC dispatch command
 *
 * Revision 1.1.2.4  2013/06/13 11:38:20  tirawan
 * Implement NC dispatch command
 *
 * Revision 1.1.2.3  2013/04/18 06:46:53  tirawan
 * Provide Voltage Margin Utility from O2
 *
 * Revision 1.1.2.2  2013/04/10 03:33:28  tirawan
 * Add GE backplane loopback test to verify GE0 connectivity
 *
 * Revision 1.1.2.1  2013/04/03 05:46:40  tirawan
 * Add auto boot by UART function, and auto run by nc utility
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
