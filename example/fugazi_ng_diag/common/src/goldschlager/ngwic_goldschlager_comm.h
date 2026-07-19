/* $Id: ngwic_goldschlager_comm.h,v 1.3 2015/02/13 12:26:49 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/goldschlager/ngwic_goldschlager_comm.h,v $
 *------------------------------------------------------------------------------
 *
 * ngwic_goldschlager_comm.h: Goldschlager Communication Library
 *
 * Oct. 2013 - James Lin
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#ifndef NGWIC_GOLDSCHLAGER_COMM_H_
#define NGWIC_GOLDSCHLAGER_COMM_H_

#define DIAG_KILL_NC_TMP_FILE               "/tmp/ngwic_goldschlager_rm.pid"
#define DIAG_COMMAND_DISPATCH_FILE          "/tmp/ngwic_goldschlager_comm_dispatch"
#define DIAG_RTN_PASS_STR                   "PASS"

#define DIAG_RUN_ALL_PORT_BASE                      (2390)
#define DIAG_RTN_STS_OUT_PORT_BASE                  (2391)
#define DIAG_RETURN_VLAUE_PORT_BASE                 (2392)
#define DIAG_EXECUTE_COMMAND_TRANSFER_PORT_BASE     (2398)
#define DIAG_EXECUTE_COMMAND_PORT_BASE              (2399)

/***********************************************************************
 *       Test commands:  HOST Messages to the BCM63x68 (NC Command Dispatch)
 ***********************************************************************
*/
#define DIAG_COMMAND_BCM63268_FLASH_TEST            "bcm63268_flash_test"
#define DIAG_COMMAND_MRVL1512_REG_TEST              "mrvl1512_reg_test"
#define DIAG_COMMAND_BCM63268_DRAM_TEST             "bcm63268_dram_test"
#define DIAG_COMMAND_BCM63268_VOLT_NORMAL           "bcm63268_volt_normal"
#define DIAG_COMMAND_BCM63268_VOLT_HIGH             "bcm63268_volt_high"
#define DIAG_COMMAND_BCM63268_VOLT_LOW              "bcm63268_volt_low"
#define DIAG_COMMAND_BCM63268_LED_TEST              "bcm63268_led_test"
#define DIAG_COMMAND_BCM63268_RESET                 "bcm63268_reset"
#define DIAG_COMMAND_BOOT_SEL_PIN_HIGH                "bcm63268_boot_sel_pin_high"
#define DIAG_COMMAND_BOOT_SEL_PIN_LOW                "bcm63268_boot_sel_pin_low"
#define DIAG_COMMAND_BCM63268_SHOW_PROFILE      "bcm63268_show_profile"
#define DIAG_COMMAND_BCM63268_SHOW_SPI_FLASH_REG    "bcm63268_show_spi_flash_reg"

#define  FRM_HOST_INIT_BCM63268             "frm_host_init_bcm63268"
#define  FRM_HOST_CONFIG_BCM63268           "frm_host_config_bcm63268"
#define  FRM_HOST_GET_VERSION               "frm_host_get_version"
#define  FRM_HOST_CONN_START                "frm_host_conn_start"
#define  FRM_HOST_CONN_STOP                 "frm_host_conn_stop"
#define  FRM_HOST_GET_CONN_INFO             "frm_host_get_conn_info"
#define  FRM_HOST_GET_SKU_TYPE             "frm_host_get_sku_type"
#define  FRM_HOST_GET_BONDING_STATE         "frm_host_get_bonding_state"
#define  FRM_HOST_SET_LINE_MODE             "frm_host_set_line_mode"
#define  FRM_HOST_GET_LINE_MODE             "frm_host_get_line_mode"
#define  FRM_HOST_SET_TEST_MODE             "frm_host_set_test_mode"
#define  FRM_HOST_ALL_TONE                  "frm_host_all_tone"
#define  FRM_HOST_SET_IDLE_LISTEN           "frm_host_set_idle_listen"
#define  FRM_HOST_DO_IDLE_LISTEN            "frm_host_do_idle_listen"
#define  FRM_HOST_UNINIT_BCM63268           "frm_host_uninit_bcm63268"
#define  FRM_HOST_GET_SW_INFO               "frm_host_get_sw_info"
#define  FRM_HOST_GET_RGMII_STATUS          "frm_host_get_rgmii_status"
#define  FRM_HOST_SHOWTIME_CONT             "frm_host_showtime_cont"
#define  FRM_HOST_SET_TONES                 "frm_host_set_tones"
#define  FRM_HOST_PRINT_IDLE_LISTEN         "frm_host_print_idle_listen"
#define  FRM_HOST_GET_CONFIG_INFO           "frm_host_get_config_info"
#define  FRM_HOST_GET_DRAM_INFO             "frm_host_get_dram_info"
#define  FRM_HOST_GET_ADSLMIB_INFO          "frm_host_get_adslmib_info"
#define  FRM_HOST_GET_XTM_BONDING_INFO      "frm_host_get_xtm_bonding_info"
#define  FRM_HOST_GET_XDSL_INFO             "frm_host_get_xdsl_info"
#define  FRM_HOST_RESET_STATCOUNTERS        "frm_host_reset_statcounters"

/***********************************************************************
 *       ADSL LINK STATE
 ***********************************************************************
*/
#define NC_BCM_ADSL_LINK_UP                         "bcm_adsl_link_up"
#define NC_BCM_ADSL_LINK_DOWN                       "bcm_adsl_link_down"
#define NC_BCM_ADSL_TRAINING_G992_EXCHANGE          "bcm_adsl_training_g992_exange"
#define NC_BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS  "bcm_adsl_training_g992_channel_analysis"
#define NC_BCM_ADSL_TRAINING_G992_STARTED           "bcm_adsl_training_g992_started"
#define NC_BCM_ADSL_TRAINING_G993_EXCHANGE          "bcm_adsl_training_g993_exange"
#define NC_BCM_ADSL_TRAINING_G993_CHANNEL_ANALYSIS  "bcm_adsl_training_g993_channel_analysis"
#define NC_BCM_ADSL_TRAINING_G993_STARTED           "bcm_adsl_training_g993_started"
#define NC_BCM_ADSL_TRAINING_G994                   "bcm_adsl_training_g994"
#define NC_BCM_ADSL_G994_NONSTDINFO_RECEIVED        "bcm_adsl_g994_nonstdinfo_received"
#define NC_BCM_ADSL_BERT_COMPLETE                   "bcm_adsl_bert_complete"
#define NC_BCM_ADSL_ATM_IDLE                        "bcm_adsl_atm_idle"
#define NC_BCM_ADSL_EVENT                           "bcm_adsl_event"
#define NC_BCM_ADSL_G997_FRAME_RECEIVED             "bcm_adsl_g997_frame_received"
#define NC_BCM_ADSL_G997_FRAME_SENT                 "bcm_adsl_g997_frame_sent"

extern void goldschlager_transmit_nc_request(int);
extern int goldschlager_nc_dispatch_comm(char *, uint, uint,uint);
extern int goldschlager_nc_dispatch_return_value(char *, uint, uint, uint);

/* NC Command Data Structure */
struct nc_command {
    char *cmd_str;
    long (*func)(char *);
};

#endif /* NGWIC_GOLDSCHLAGER_COMM_H_ */


/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngwic_goldschlager_comm.h,v $
 * Revision 1.3  2015/02/13 12:26:49  meho
 * Added the utility to read Boardcom SPI flash registers
 *
 * Revision 1.2  2014/09/17 03:32:16  jamlin
 * Add support for Goldschlager NIM.
 *
 * Revision 1.1.6.2  2014/08/08 02:43:58  jamlin
 * goladschlager-branch3 initail commit.
 *
 * Revision 1.1.4.6  2014/04/08 13:12:31  jamlin
 * Checkin enhanced error message.
 *
 * Revision 1.1.4.5  2014/02/10 04:17:03  jamlin
 * added get_xdsl_profile function
 *
 * Revision 1.1.4.4  2014/02/10 04:03:18  jamlin
 * rename nc_dispatch_linkstatus to nc_dispatch_return_value function and added check_sku_type function
 *
 * Revision 1.1.4.3  2014/02/10 03:32:21  jamlin
 * added bcm_bonding_state_get function and fixed showtime bonding issue
 *
 * Revision 1.1.4.2  2014/01/07 01:54:52  jamlin
 * Goldschlager new branch goldschlager-branch2
 *
 * Revision 1.1.2.2  2013/12/04 01:38:52  jamlin
 * Support Bonding channels showtime status display.
 *
 * Revision 1.1.2.1  2013/11/02 13:39:52  jamlin
 * Initial commit for bringup.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

