/* $Id: tsn_rbcp_main.h,v 1.3 2018/06/01 08:16:10 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/tsn_rbcp_main.h,v $
 *------------------------------------------------------------------
 * Filename: tsn_rbcp_main.h
 *
 * Description: The RBCP main code header file
 * Author: Kody Ko
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef TSN_RBCP_MAIN_H_
#define TSN_RBCP_MAIN_H_

#define MAX_NUM_TSN_SLOTS          (1)

#define TSN_BP_GE0                 (0)
#define TSN_BP_GE1                 (1)

#define RBCP_WAIT_TIME                  5000
#define LED_WAIT_TIME                   2000

#define CISCO_SCP_NIM_DRAM_TEST         0x200
#define CISCO_SCP_NIM_SPI_FLASH_TEST    0x201
#define CISCO_SCP_NIM_ETSEC1_TEST       0x202
#define CISCO_SCP_NIM_ETSEC3_TEST       0x203
#define CISCO_SCP_NIM_UCC5_TEST         0x204
#define CISCO_SCP_NIM_UCC3_TEST         0x205
#define CISCO_SCP_OP_TERMINATE_RBCP     0x206
#define CISCO_SCP_NIM_MARGIN_HIGH       0x208
#define CISCO_SCP_NIM_MARGIN_LOW        0x209
#define CISCO_SCP_NIM_UCC1_TEST         0x211
#define CISCO_SCP_NIM_LED_TEST          0x212
#define CISCO_SCP_NIM_ECC_TEST          0x214
#define CISCO_SCP_LED_ATM               0x215
#define CISCO_SCP_LED_EFM               0x216
#define CISCO_SCP_LED_L0_G              0x217
#define CISCO_SCP_LED_L0_Y              0x218
#define CISCO_SCP_LED_L1_G              0x219
#define CISCO_SCP_LED_L1_Y              0x220
#define CISCO_SCP_LED_L2_G              0x221
#define CISCO_SCP_LED_L2_Y              0x222
#define CISCO_SCP_LED_L3_G              0x223
#define CISCO_SCP_LED_L3_Y              0x224
#define CISCO_SCP_LED_OFF               0x225
#define CISCO_SCP_NIM_SPI_FLASH_PROTECT 0x226

#define GSHDSL_ATM_LED 0
#define GSHDSL_EFM_LED 1
#define GSHDSL_L0_LED  2
#define GSHDSL_L1_LED  3
#define GSHDSL_L2_LED  4
#define GSHDSL_L3_LED  5

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

#define TSN_RBCP_PKT_PADDING       22

#define TSN_RBCP_PKT_RECV_TIMEOUT  (150)
#define TSN_RBCP_PKT_RECV_CLEAN    (100)
#define TSN_RBCP_RETRIES           (6)

/* total scp payload size is 1476-byte */
typedef struct cisco_scp_reply_data {
    uint8_t result;
    uint8_t result_log[REPLY_RESULT_LOG_SIZE];
    uint8_t msg[REPLY_MSG_SIZE];
} cisco_scp_reply_data_t;

extern unsigned int fru_table_offset;
extern long build_tsn_rbcp_menu(int);
extern int tsn_rbcp_console_switch(void);
extern int tsn_rbcp_heartbeat_test(int);
extern int tsn_rbcp_registration_test(int);
extern void clear_tsn_regis_done_flag(int);

extern int tsn_rbcp_memory_test(int);
extern int tsn_rbcp_ecc_test(int);
extern int tsn_rbcp_etsec1_rmii_lpbk_test(int);
extern int tsn_rbcp_etsec3_rmii_lpbk_test(int);
extern int tsn_rbcp_ucc1_rmii_lpbk_test(int);
extern int tsn_rbcp_ucc5_rmii_lpbk_test(int);
extern int tsn_rbcp_ucc3_utopia_lpbk_test(int);
extern int tsn_rbcp_spi_flash_test(int);
extern int tsn_rbcp_spi_flash_protect(int);
extern int tsn_rbcp_led_test(int);
extern int terminate_rbcp_test(void);
extern int gshdsl_led_utils_menu(void);



            
#endif /* TSN_RBCP_MAIN_H_ */
/*------------------------------------------------------------------
 * $Log: tsn_rbcp_main.h,v $
 * Revision 1.3  2018/06/01 08:16:10  letsai
 * Added Check flash protection utility
 *
 * Revision 1.2  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 *
 *------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------
 */
