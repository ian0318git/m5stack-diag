/* $Id: testcard_tlk_10232.h,v 1.14 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_tlk_10232.h,v $
 *--------------------------------------------------------------------
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
#define TLK10232_LS_CONFIG_CONTROL  0x000C
#define TLK10232_CLK_CONTROL        0x000D
#define TLK10232_RESET_CTRL         0x000E
#define TLK10232_CHANNEL_STATUS_1   0x000F
#define TLK10232_HS_ERROR_COUNTER   0x0010
#define TLK10232_LS_LN0_ERROR_COUNTER 0x0011
#define TLK10232_LS_LN1_ERROR_COUNTER 0x0012
#define TLK10232_LS_LN2_ERROR_COUNTER 0x0013
#define TLK10232_LS_LN3_ERROR_COUNTER 0x0014
#define TLK10232_LS_STATUS_1        0x0015
#define TLK10232_HS_STATUS_1        0x0016
#define TLK10232_DST_CTRL_2_REG     0x0018
#define TLK10232_HS_CH_CONTROL_1    0x001D
#define TLK10232_VDR_HEX0096        0x0096
#define TLK10232_DST_DATA_SRC_SEL   0xC000
#define TLK10232_DSR_CTRL_2_REG     0x001A
#define TLK10232_DSR_DATA_SRC_SEL   0xC000
#define TLK10232_TI_RESERVED_CONTROL 0x8020
#define TLK10232_MC_AUTO_CONTROL    0x8021
#define TLK10232_TRIGGER_LOAD_CTRL  0x8100
#define TLK10232_TRIGGER_EN_CTRL    0x8101
#define TLK10232_VDR_HEX9000        0x9000
#define TLK10232_VDR_HEX9001        0x9001
#define TLK10232_VDR_HEX9005        0x9005

/* PMA/PMD register */
#define TLK10232_PMA_CONTROL_1     0x0000
#define TLK10232_PMA_STATUS_1      0x0001
#define TLK20232_PMA_STATUS_2      0x0008
#define TLK10232_LT_TRAIN_CONTROL  0x0096
#define TLK10232_LT_TRAIN_STATUS   0x0097
#define TLK10232_KX_STATUS_1       0x00a1
#define TLK10232_KR_FEC_CONTROL    0x00ab
#define TLK10232_LT_VS_CTRL_2      0x9001
#define TLK10232_PMA_HEX9002       0x9002
#define TLK10232_PMA_HEX9003       0x9003

/* PCS register */
#define TLK10232_PCS_CONTROL   0x0000
#define TLK10232_PCS_STATUS_1   0x0001
#define TLK10232_PCS_STATUS_2   0x0008
#define TLK10232_KR_PCS_STATUS_1 0x0020
#define TLK10232_PCS_TP_CONTROL 0x002A
#define TLK10232_PCS_TP_ERR_COUNT 0x002B

/* autoneg register */
#define TLK10232_AN_CONTROL    0x0000
#define TLK10232_AN_STATUS     0x0001
#define TLK10232_AN_ADV_3      0x0012
#define TLK10232_AN_BP_STATUS  0x0030
/* autoneg */
#define TLK10232_AN_CTRL    0
#define TLK10232_AN_1GKX    0x2
#define TLK10232_AN_10GKR   0x8

/* this counter is shared for each check link status stage 
 * each stage shared 3-4 times */
#define RETRY_CNT_10G_KR_TC  (12) 

typedef enum _TLK10232_MODE_TYPE {
    TLK10232_MODE_1GKX,
    TLK10232_MODE_10GKR_NO_AN,
    TLK10232_MODE_10GKR,
    TLK10232_MODE_NUMBER
}TLK10232_MODE_TYPE_E;

typedef enum _TLK10232_STATUS_TYPE {
    TLK10232_HS_AZ_DONE,
    TLK10232_AGC_LOCKED,
    TLK10232_PLL_STATUS_LOCKED,
    TLK10232_AUTO_NEGOTIATION,
    TLK10232_LINK_TRAINING,
    TLK10232_KR_MODE,
    TLK10232_KR_PCS_RX_LINK_STATUS,
    TLK10232_KR_PCS_BLOCK_LOCK
}TLK10232_STATUS_TYPE_E;

typedef enum _TLK10232_REGISTER_TEST_TYPE {
    TLK10232_REGISTER_READ_WRITE_TEST,
    TLK10232_REGISTER_DUMP_TEST
} TLK10232_REGISTER_TEST_TYPE_E;

typedef enum _TLK10232_PRBS_PATTERN_TYPE {
    TLK10232_PRBS_PATTERN_31,
    TLK10232_PRBS_PATTERN_23,
    TLK10232_PRBS_PATTERN_7,
    TLK10232_PRBS_PATTERN_NUMBER
} TLK10232_PRBS_PATTERN_TYPE_E;

extern void build_tc_tlk10232_menu(int);
extern void tc_tlk10232_init_10gkr_ti_setting(void);
extern void tc_tlk10232_set_mode(int);
extern void tmp_set(void);
extern void tc_tlk10232_cleanup(int);
extern void tc_tlk10232_power_off(int);
extern int tc_tlk10232_internal_loopback_test(int);
extern int tc_tlk10232_chk_gesw_link_status(int);
extern void tc_tlk10232_reg_rw(TLK10232_REGISTER_TEST_TYPE_E type);
extern int tc_tlk10232_set_mode_type(int channel_addr, TLK10232_MODE_TYPE_E mode_type);
extern void tc_tlk10232_clear_latched_registers(int channel_addr, TLK10232_MODE_TYPE_E mode_type);
extern int tc_tlk10232_set_prbs(int channel_addr, TLK10232_PRBS_PATTERN_TYPE_E prbs_pattern);
extern int tc_tlk10232_clear_error_counters(int channel_addr);
extern int tc_tlk10232_prbs_generate(int channel_addr, int enable);
extern int tc_tlk10232_prbs_verify(int channel_addr, int enable);
extern void tc_tlk10232_reset_datapath(int channel_addr);
extern int tc_tlk10232_check_error_counters(int channel_addr);
extern int tc_tlk10232_check_status(int channel_addr, TLK10232_STATUS_TYPE_E type);

#endif /* __TESTCARD_ETH_H__ */

/* ------- End of file ------- */

/******** History ******** 
$Log: testcard_tlk_10232.h,v $
Revision 1.14  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.13  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.12.2.1  2018/08/28 17:44:39  alpeng
miss to check in header file for MARCO definition

Revision 1.12  2018/05/18 09:24:52  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.11  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.10  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.9.30.3  2018/05/17 10:50:23  alpeng
 sync with trunk <trunk-051618>

Revision 1.9.30.2  2018/02/05 09:48:26  alpeng
add check link speed before sending packet; skip aux -54v detection check

Revision 1.9.30.1  2017/04/05 09:15:17  leschen
Sync with <ng_diag-tag-032917>

Revision 1.9  2014/10/17 07:18:32  alpeng
supporting retry in case the gesw link is unstable packet cannot be sent

Revision 1.8  2014/08/20 06:21:18  alpeng
support new testcard on non-GH platforms

Revision 1.7  2014/07/28 03:43:46  alpeng
check gesw link status before sending packet

Revision 1.6  2014/07/25 01:36:57  alpeng
support xaui loopback and sort out the test item for new testcard

Revision 1.5  2014/07/22 09:40:13  alpeng
support 10g-kr on ge0 and 1g-kx on ge1 for new testcard

Revision 1.4  2014/06/25 06:17:04  alpeng
a new item for init tlk chip

Revision 1.3  2014/06/24 02:30:25  alpeng
update channel a and b address

Revision 1.2  2014/06/06 07:04:45  alpeng
put plx and tlk10232 test into menu

Revision 1.1  2014/05/15 07:49:55  alpeng
first check in for tlk10232 chip on new testcard

$Endlog$
*/

