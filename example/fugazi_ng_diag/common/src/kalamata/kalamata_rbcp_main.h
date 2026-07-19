/* $Id: kalamata_rbcp_main.h,v 1.4 2019/02/20 11:55:25 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/kalamata/kalamata_rbcp_main.h,v $
 *------------------------------------------------------------------
 * Filename: kalamata_rbcp_main.h
 *
 * Description: The RBCP main code header file
 * Author: Kody Ko
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef KALAMATA_RBCP_MAIN_H_
#define KALAMATA_RBCP_MAIN_H_

#define MAX_NUM_KALAMATA_SLOTS          (1)

#define KALAMATA_BP_GE0                 (0)
#define KALAMATA_BP_GE1                 (1)
#define FAILURE_OP                      0x10
#define RBCP_WAIT_TIME                  5000
#define LED_WAIT_TIME                   2000

#define CISCO_SCP_NIM_DRAM_TEST                0x200
#define CISCO_SCP_NIM_SPI_FLASH_TEST           0x201
#define CISCO_SCP_NIM_ETSEC1_TEST              0x202
#define CISCO_SCP_NIM_ETSEC3_TEST              0x203
#define CISCO_SCP_NIM_UCC5_TEST                0x204
#define CISCO_SCP_NIM_UCC3_TEST                0x205
#define CISCO_SCP_OP_TERMINATE_RBCP            0x206
#define CISCO_SCP_NIM_INTR_TEST                0x207
#define CISCO_SCP_NIM_MARGIN_HIGH              0x208
#define CISCO_SCP_NIM_MARGIN_LOW               0x209
#define CISCO_SCP_NIM_GE_REG_TEST              0x210
#define CISCO_SCP_NIM_UCC1_TEST                0x211
#define CISCO_SCP_NIM_LED_TEST                 0x212
#define CISCO_SCP_NIM_FPGA_TEST                0x213
#define CISCO_SCP_NIM_ECC_TEST                 0x214
#define CISCO_SCP_LED_ATM                      0x215
#define CISCO_SCP_LED_EFM                      0x216
#define CISCO_SCP_LED_L0_G                     0x217
#define CISCO_SCP_LED_L0_Y                     0x218
#define CISCO_SCP_LED_L1_G                     0x219
#define CISCO_SCP_LED_L1_Y                     0x220
#define CISCO_SCP_LED_L2_G                     0x221
#define CISCO_SCP_LED_L2_Y                     0x222
#define CISCO_SCP_LED_L3_G                     0x223
#define CISCO_SCP_LED_L3_Y                     0x224
#define CISCO_SCP_LED_OFF                      0x225
#define CISCO_SCP_NIM_SPI_FLASH_PROTECT_TEST   0x226
#define CISCO_SCP_NIM_FPGA_FLASH_PROTECT_TEST  0x227


#define REPLY_RESULT_LOG_SIZE           128
#define REPLY_MSG_SIZE                  1347

#define MAC_HEADER_LEN                  14
#define LLC_LEN_802                     3
#define SNAP_LEN_802                    5
#define HEADER_LEN_802                  (MAC_HEADER_LEN + LLC_LEN_802 + SNAP_LEN_802)
#define SCP_HEADER_LEN                  16

/* PCA9557 Definition */
#define PCA9557_IN_PORT_REG             0x00
#define PCA9557_OUT_PORT_REG            0x01
#define PCA9557_POLAR_INV_P_REG         0x02
#define PCA9557_CFG_PORT_REG            0x03

#define PCA9557_PORT_MASK               0xFF
#define PCA9557_PORT_INIT               0x00

#define PCA9557_IO_INPUT                0x1
#define PCA9557_IO_OUTPUT               0x0
#define PCA9557_IO_HIGH                 0x1
#define PCA9557_IO_LOW                  0x0

#define KALAMATA_RBCP_PKT_PADDING       22

#define KALAMATA_RBCP_PKT_RECV_TIMEOUT  (300)
#define KALAMATA_RBCP_PKT_RECV_CLEAN    (100)
#define KALAMATA_RBCP_RETRIES           (6)


/* total scp payload size is 1476-byte */
typedef struct cisco_scp_reply_data {
    uint8_t result;
    uint8_t result_log[REPLY_RESULT_LOG_SIZE];
    uint8_t msg[REPLY_MSG_SIZE];
} cisco_scp_reply_data_t;

extern long build_kalamata_rbcp_menu(int);
extern int kalamata_rbcp_console_switch(void);
extern int kalamata_rbcp_heartbeat_test(int);
extern int kalamata_rbcp_registration_test(int);
extern void clear_kalamata_regis_done_flag(int);
extern int kalamata_setup_rbcp_ge_env(int);
extern int kalamata_cleanup_rbcp_ge_env(int);


#endif /* KALAMATA_RBCP_MAIN_H_ */
/*------------------------------------------------------------------
 * $Log: kalamata_rbcp_main.h,v $
 * Revision 1.4  2019/02/20 11:55:25  letsai
 * Add RBCP functions to support Kalamata NIM tests(CSCvo39487 & CSCvo39481)
 *
 * Revision 1.3  2018/04/19 09:15:37  letsai
 * Add LED utility of single color control
 *
 * Revision 1.2  2018/02/24 07:36:25  letsai
 * Collapse Kalamata-branch to Main Trunk.
 *
 * Revision 1.1.4.9  2017/12/28 03:13:55  letsai
 * Add RBCP FPGA register test
 *
 * Revision 1.1.4.8  2017/12/21 03:42:17  letsai
 * Add RBCP GE PHY register test/RBCP interrupt test/RBCP LED Test
 *
 * Revision 1.1.4.7  2017/12/11 11:24:07  letsai
 * Add RBCP UCC1 RMII loopback test
 *
 * Revision 1.1.4.6  2017/09/21 06:12:14  letsai
 * Added RBCP function of Power Supply Margin
 *
 * Revision 1.1.4.5  2017/09/08 11:10:00  kodko
 * Modify for P1B bring up.
 *
 * Revision 1.1.4.4  2017/08/22 12:27:49  kodko
 * Increase RBCP timeout time for Kalamata module test items.
 *
 * Revision 1.1.4.3  2017/08/17 13:01:51  kodko
 * Automation test bring up for Kalamata P1A.
 *
 * Revision 1.1.4.2  2017/06/16 07:17:03  kodko
 * Initial platform code commit for Kalamata project.
 *
 *------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------
 */
