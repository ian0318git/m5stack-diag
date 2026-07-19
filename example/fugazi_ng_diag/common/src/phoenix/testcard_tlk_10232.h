/* $Id: testcard_tlk_10232.h,v 1.2 2021/04/15 00:52:30 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/testcard_tlk_10232.h,v $
 *------------------------------------------------------------------
 * Filename   : testcard_tlk10232.h
 *
 * Description: Head file for TestCard tlk10232 related definition.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __TESTCARD_TLK10232_H__
#define __TESTCARD_TLK10232_H__

/* Common */
#define TLK_MAX_RETRY      10
#define GB_TC_MAX_ETH_PORT  1
#define TC_MAX_ETH_PORT     2

#define TC_TLK10232_CHA_ADDR     0xA
#define TC_TLK10232_CHB_ADDR     0xB

#define SAME_LS_INPUT    0
#define SAME_HS_INPUT    1
#define ALT_LS_INPUT     2
#define ALT_HS_OUTPUT    3

/* Device Type Option */
#define TLK_VDR_SPE_OPT     1
#define TLK_PMAPMD_OPT      2
#define TLK_PCS_OPT         3
#define TLK_AUTO_NEG_OPT    4
#define TLK_USER_DEF_OPT    5

/* Device Type */
#define TLK10232_VDR_SPE_DEV_ADDR       0x1E
#define TLK10232_PMAPMD_DEV_ADDR        0x1
#define TLK10232_PCS_DEV_ADDR           0x3
#define TLK10232_AUTO_NEG_DEV_ADDR      0x7

/* Register definition */
/* vendor specific device addr */
#define TLK10232_GLOBAL_CTRL_REG    0x0000
#define TLK10232_CHANNEL_CTRL       0x0001
#define TLK10232_HS_SERDES_CTRL_1   0x0002
#define TLK10232_HS_SERDES_CTRL_2   0x0003
#define TLK10232_HS_SERDES_CTRL_3   0x0004
#define TLK10232_HS_SERDES_CTRL_4   0x0005
#define TLK10232_LOOPBACK_TP_CTRL   0x000B
#define TLK10232_CLK_CONTROL        0x000D
#define TLK10232_RESET_CTRL         0x000E
#define TLK10232_DST_CTRL_2_REG     0x0018
#define TLK10232_VDR_HEX0096        0x0096
#define TLK10232_DST_DATA_SRC_SEL   0xC000
#define TLK10232_DSR_CTRL_2_REG     0x001A
#define TLK10232_DSR_DATA_SRC_SEL   0xC000
#define TLK10232_TRIGGER_LOAD_CTRL  0x8100
#define TLK10232_TRIGGER_EN_CTRL    0x8101
#define TLK10232_VDR_HEX9000        0x9000
#define TLK10232_VDR_HEX9001        0x9001
#define TLK10232_VDR_HEX9005        0x9005

/* PMA/PMD register */
#define TLK10232_PMA_CONTROL_1     0x0000
#define TLK10232_LT_TRAIN_CONTROL  0x0096
#define TLK10232_KR_FEC_CONTROL    0x00ab
#define TLK10232_LT_VS_CTRL_2      0x9001
#define TLK10232_PMA_HEX9002       0x9002
#define TLK10232_PMA_HEX9003       0x9003

/* PCS register */
#define TLK10232_PCS_CONTROL   0x0000

/* autoneg register */
#define TLK10232_AN_CONTROL    0x0000
#define TLK10232_AN_ADV_3      0x0012
#define TLK10232_AN_BP_STATUS  0x0030
/* autoneg */
#define TLK10232_AN_CTRL    0
#define TLK10232_AN_1GKX    0x2
#define TLK10232_AN_10GKR   0x8

/* this counter is shared for each check link status stage 
 * each stage shared 3-4 times */
#define RETRY_CNT_10G_KR_TC  (12) 


extern void build_tc_tlk10232_menu(int);
extern void tc_tlk10232_init_10gkr_ti_setting(void);
extern void tc_tlk10232_set_mode(int);
extern void tmp_set(void);
extern void tc_tlk10232_cleanup(int);
extern void tc_tlk10232_power_off(int);
extern int tc_tlk10232_internal_loopback_test(int);
extern int tc_tlk10232_chk_gesw_link_status(int);

#endif /* __TESTCARD_ETH_H__ */

/* ------- End of file ------- */

