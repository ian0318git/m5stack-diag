/* $Id: bcm82752_reg_def.h,v 1.3 2020/11/03 06:17:48 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm82752_reg_def.h,v $
*-----------------------------------------------------------------------------
* bcm82752_reg_def.h - Register definitions for BCM 10G PHY bcm82752.
*          Leverage from KP
*
* Feb 2019, Leschen
*
* Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include "types.h"
#define BCM82752_DEV_UNDEFINED      0
#define BCM82752_DEV_PMA            1
#define BCM82752_DEV_PCS            3
#define BCM82752_DEV_1GBE           7
#define BCM82752_DEV_DEFAULT        0xFF

/* start of bcm82752 MMD1(PMD) register address defination */
#define BCM82752_PMD_CONTROL_REG                        0x0000
#define BCM82752_PMD_STATUS_REG                         0x0001
#define BCM82752_PMD_ID_MSB_REG                         0x0002
#define BCM82752_PMD_ID_LSB_REG                         0x0003
#define BCM82752_PMD_SPEED_ABILITY_REG                  0x0004
#define BCM82752_DEVICES_IN_PACKAGE_1_REG               0x0005
#define BCM82752_DEVICES_IN_PACKAGE_2_REG               0x0006
#define BCM82752_PMD_CONTROL_2_REG                      0x0007
#define BCM82752_PMD_STATUS_2_REG                       0x0008
#define BCM82752_PMD_TRANSMIT_DISABLE_REG               0x0009
#define BCM82752_PMD_RECEIVE_SIGNAL_DETECT_REG          0x000A
#define BCM82752_PMD_EXTENDED_ABILITY_REG               0x000B
#define BCM82752_PMD_ORGANIZATIONALLY_UNIQUE_ID_MSB_REG 0x000E
#define BCM82752_PMD_ORGANIZATIONALLY_UNIQUE_ID_LSB_REG 0x000F
#define BCM82752_TWO_WIRE_CONTROL_REG                   0x8000
#define BCM82752_TWO_WIRE_CHECK_SUM_REG                 0x8001
#define BCM82752_TWO_WIRE_TRANSFER_SIZE_REG             0x8002
#define BCM82752_TWO_WIRE_NVM_ADDRESS_REG               0x8003
#define BCM82752_TWO_WIRE_INTERNAL_ADDRESS_REG          0x8004
#define BCM82752_TWO_WIRE_SLAVE_ID_ADDRESS_REG          0x8005
#define BCM82752_TWO_WIRE_USER_ADDR_WRITE_POINTER_REG   0x8006
#define BCM82752_I2C_MASTER_DMA_SW_SEM_CONTROL_REG      0x8800

#define BCM82752_RX_ALARM_CONTROL_REG                   0x9000
#define BCM82752_TX_ALARM_CONTROL_REG                   0x9001
#define BCM82752_LASI_CONTROL_REG                       0x9002
#define BCM82752_RX_ALARM_STATUS_REG                    0x9003
#define BCM82752_TX_ALARM_STATUS_REG                    0x9004
#define BCM82752_LASI_STATUS_REG                        0x9005

#define BCM82752_SLICE_ENABLE_CONTRL_REG                0xC600
#define BCM82752_TX_EVENT_MESSAGE_MODE_SEL_REG_MODE     0xC601
#define BCM82752_TX_MODE2_SEL_REG                       0xC602
#define BCM82752_RX_EVENT_MESSAGE_MODE_SEL_REG          0xC603
#define BCM82752_RX_MODE2_SEL_REG                       0xC604
#define BCM82752_RX_LINK_DELAY_SEL_REG                  0xC605
#define BCM82752_RX_LINK_DELAY_MSB_SEL_REG              0xC606
#define BCM82752_TXRX_SOP_TS_CAPTURE_ENABLE_REG         0xC607
#define BCM82752_TX_OPTION_SEL_REG                      0xC608
#define BCM82752_RX_OPTION_SEL_REG                      0xC609
#define BCM82752_TXRX_TS_OFFSET_REG                     0xC60A
#define BCM82752_RX_TS_OFFSET_REG                       0xC60B
#define BCM82752_TIME_CODE_1_REG                        0xC60C
#define BCM82752_TIME_CODE_2_REG                        0xC60D
#define BCM82752_TIME_CODE_3_REG                        0xC60E
#define BCM82752_TIME_CODE_4_REG                        0xC60F
#define BCM82752_TIME_CODE_5_REG                        0xC610
#define BCM82752_DPLL_DB_1_REG                          0xC611
#define BCM82752_DPLL_DB_2_REG                          0xC612
#define BCM82752_DPLL_DEBUG_SEL_REG                     0xC613
#define BCM82752_SHADOW_REG_CONTRL_1_REG                0xC614
#define BCM82752_SHADOW_REG_CONTRL_2_REG                0xC615
#define BCM82752_INTERRUPT_MASK_REG                     0xC616
#define BCM82752_INTERRUPT_STATUS_REG                   0xC617
#define BCM82752_TX_CONTRL_REG                          0xC618
#define BCM82752_TX_DEBUG_REG                           0xC619
#define BCM82752_TX_DEBUG_REG                           0xC619
#define BCM82752_RX_CONTRL_REG                          0xC61A
#define BCM82752_RX_DEBUG_REG                           0xC61B
#define BCM82752_RX_TX_CONTRL_REG                       0xC61C
#define BCM82752_VLAN_TAG_REG                           0xC61D
#define BCM82752_OUTER_VLAN_TAG_REG                     0xC61E
#define BCM82752_INNER_VLAN_TAG_REG                     0xC61F
#define BCM82752_NSE_DPLL_1_REG                         0xC620
#define BCM82752_NSE_DPLL_2_REG                         0xC621
#define BCM82752_NSE_DPLL_3_REG                         0xC622
#define BCM82752_NSE_DPLL_4_REG                         0xC623
#define BCM82752_NSE_DPLL_5_REG                         0xC624
#define BCM82752_NSE_DPLL_6_REG                         0xC625
#define BCM82752_NSE_DPLL_7_REG                         0xC626
#define BCM82752_NSE_DPLL_8_REG                         0xC627
#define BCM82752_NSE_DPLL_9_REG                         0xC628
#define BCM82752_NSE_DPLL_10_REG                        0xC629
#define BCM82752_NSE_DPLL_11_REG                        0xC62A
#define BCM82752_NSE_DPLL_12_REG                        0xC62B
#define BCM82752_NSE_DPLL_13_REG                        0xC62C
#define BCM82752_NSE_DPLL_14_REG                        0xC62D
#define BCM82752_NSE_NCO_1_REG                          0xC62E
#define BCM82752_NSE_NCO_2_REG                          0xC62F
#define BCM82752_NSE_NCO_3_REG                          0xC630
#define BCM82752_NSE_NCO_4_REG                          0xC631
#define BCM82752_NSE_NCO_5_REG                          0xC632
#define BCM82752_NSE_SC_1_REG                           0xC633
#define BCM82752_NSE_SC_2_REG                           0xC634
#define BCM82752_NSE_SC_3_REG                           0xC635
#define BCM82752_NSE_SC_4_REG                           0xC636
#define BCM82752_NSE_SC_5_REG                           0xC637
#define BCM82752_NSE_SC_6_REG                           0xC638
#define BCM82752_NSE_SC_7_REG                           0xC639
#define BCM82752_NSE_SC_8_REG                           0xC63A
#define BCM82752_NSE_SC_9_REG                           0xC63B
#define BCM82752_NSE_SC_10_REG                          0xC63C
#define BCM82752_TS_HB_SEL_1_REG                        0xC63D
#define BCM82752_TS_HB_SEL_2_REG                        0xC63E
#define BCM82752_TS_HB_SEL_3_REG                        0xC63F
#define BCM82752_TS_HB_SEL_4_REG                        0xC640
#define BCM82752_TS_HB_SEL_5_REG                        0xC641
#define BCM82752_TS_HB_SEL_6_REG                        0xC642
#define BCM82752_TS_HB_SEL_7_REG                        0xC643
#define BCM82752_TS_HB_SEL_8_REG                        0xC644
#define BCM82752_TS_HB_SEL_9_REG                        0xC645
#define BCM82752_TS_HB_SEL_10_REG                       0xC646
#define BCM82752_TS_HB_SEL_11_REG                       0xC647
#define BCM82752_RX_SOP_COUNTER_REG                     0xC648
#define BCM82752_RX_EOP_COUNTER_REG                     0xC649
#define BCM82752_TX_SOP_COUNTER_REG                     0xC64A
#define BCM82752_TX_EOP_COUNTER_REG                     0xC64B
#define BCM82752_RXPKT_SOP_10G_COUNTER_REG              0xC64C
#define BCM82752_RXPKT_EOP_10G_COUNTER_REG              0xC64D
#define BCM82752_TXPKT_SOP_10G_COUNTER_REG              0xC64E
#define BCM82752_TXPKT_EOP_10G_COUNTER_REG              0xC64F
#define BCM82752_PKT_COUNT_SEL_REG                      0xC650
#define BCM82752_MPLS_CONTRL_REG                        0xC651
#define BCM82752_MPLS_TX_SPECIAL_LABEL_1_REG            0xC652
#define BCM82752_MPLS_TX_SPECIAL_LABEL_2_REG            0xC653
#define BCM82752_MPLS_LABEL_VALUE_1_REG                 0xC654
#define BCM82752_MPLS_LABEL_VALUE_2_REG                 0xC655
#define BCM82752_MPLS_LABEL_VALUE_3_REG                 0xC656
#define BCM82752_MPLS_LABEL_VALUE_4_REG                 0xC657
#define BCM82752_MPLS_LABEL_VALUE_5_REG                 0xC658
#define BCM82752_MPLS_LABEL_VALUE_6_REG                 0xC659
#define BCM82752_MPLS_LABEL_VALUE_7_REG                 0xC65A
#define BCM82752_MPLS_LABEL_VALUE_8_REG                 0xC65B
#define BCM82752_MPLS_LABEL_VALUE_9_REG                 0xC65C
#define BCM82752_MPLS_LABEL_VALUE_10_REG                0xC65D
#define BCM82752_MPLS_LABEL_VALUE_11_REG                0xC65E
#define BCM82752_MPLS_LABEL_VALUE_12_REG                0xC65F
#define BCM82752_MPLS_LABEL_VALUE_13_REG                0xC660
#define BCM82752_MPLS_LABEL_MASK_1_REG                  0xC661
#define BCM82752_MPLS_LABEL_MASK_2_REG                  0xC662
#define BCM82752_MPLS_LABEL_MASK_3_REG                  0xC663
#define BCM82752_MPLS_LABEL_MASK_4_REG                  0xC664
#define BCM82752_MPLS_LABEL_MASK_5_REG                  0xC665
#define BCM82752_MPLS_LABEL_MASK_6_REG                  0xC666
#define BCM82752_MPLS_LABEL_MASK_7_REG                  0xC667
#define BCM82752_MPLS_LABEL_MASK_8_REG                  0xC668
#define BCM82752_MPLS_LABEL_MASK_9_REG                  0xC669
#define BCM82752_MPLS_LABEL_MASK_10_REG                 0xC66A
#define BCM82752_MPLS_LABEL_MASK_11_REG                 0xC66B
#define BCM82752_MPLS_LABEL_MASK_12_REG                 0xC66C
#define BCM82752_MPLS_LABEL_MASK_13_REG                 0xC66D
#define BCM82752_MPLS_LABEL_DIR_1_REG                   0xC66E
#define BCM82752_MPLS_LABEL_DIR_2_REG                   0xC66F
#define BCM82752_DUAL_CLOCK_CONTRL_1_REG                0xC670
#define BCM82752_DUAL_CLOCK_MAC_ADDR_1_REG              0xC671
#define BCM82752_DUAL_CLOCK_MAC_ADDR_2_REG              0xC672
#define BCM82752_DUAL_CLOCK_MAC_ADDR_3_REG              0xC673
#define BCM82752_DUAL_CLOCK_IPV4_ADDR_1_REG             0xC674
#define BCM82752_DUAL_CLOCK_IPV4_ADDR_2_REG             0xC675
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_1_REG             0xC676
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_2_REG             0xC677
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_3_REG             0xC678
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_4_REG             0xC679
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_5_REG             0xC67A
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_6_REG             0xC67B
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_7_REG             0xC67C
#define BCM82752_DUAL_CLOCK_IPV6_ADDR_8_REG             0xC67D
#define BCM82752_PHY_FCMAC_STATUS_1_REG                 0xC67E
#define BCM82752_PHY_FCMAC_CONTRL_1_REG                 0xC67F
#define BCM82752_MAC_PORT_ID_REG                        0xC680
#define BCM82752_FCMAC_FC_CDR_CNTL_1_REG                0xC681
#define BCM82752_PSM10G_DEBUG_CONTRL_1_REG              0xC682
#define BCM82752_PSM10G_DEBUG_STATUS_1_REG              0xC683
#define BCM82752_PSM10G_DEBUG_STATUS_2_REG              0xC684
#define BCM82752_GPREG_1_REG                            0xC685
#define BCM82752_GPREG_2_REG                            0xC686
#define BCM82752_GPREG_3_REG                            0xC687
#define BCM82752_GPREG_4_REG                            0xC688
#define BCM82752_GPREG_5_REG                            0xC689

#define BCM82752_PHY_IDENTIFIER_REG                     0xC800
#define BCM82752_CHIP_REVISION_REG                      0xC801
#define BCM82752_CHIP_ID_LSB_REG                        0xC802
#define BCM82752_CHIP_ID_MSB_REG                        0xC803
#define BCM82752_USER_PMD_STATUS_REG                    0xC804
#define BCM82752_CHIP_MODE_REG                          0xC805 /* New for 82xxx */ 
#define BCM82752_DATAPATH_AND_CDR_SELECTION_REG         0xC806
#define BCM82752_PCS_OPTICS_DIGITAL_CONTROL_REG         0xC808
#define BCM82752_PCS_DIGITAL_STATUS_REG                 0xC809
#define BCM82752_LAN_TEST_CONTROL_REG                   0xC80A
#define BCM82752_BIST_CONTROL_STATUS_REG                0xC80B
#define BCM82752_GENERAL_PURPOSE_IO_CONTROL_REG         0xC80E
#define BCM82752_GENERAL_PURPOSE_IO_MACSEC_REG          0xC80D
#define BCM82752_PCS_RX_RECEIVED_IDLE_COUNT_0_REG       0xC813
#define BCM82752_PCS_RX_RECEIVED_IDLE_COUNT_1_REG       0xC814
#define BCM82752_PCS_TX_RECEIVED_IDLE_COUNT_0_REG       0xC815
#define BCM82752_PCS_TX_RECEIVED_IDLE_COUNT_1_REG       0xC816
#define BCM82752_PCS_BIST_DATA_LENGTH_REG               0xC817
#define BCM82752_PCS_BIST_TEST_LENGTH_REG               0xC818
#define BCM82752_XFI_BIST_ERROR_VECTOR_COUNT_REG        0xC81A
#define BCM82752_PCS_BIST_ERROR_VECTOR_COUNT_REG        0xC81B
#define BCM82752_SPEED_LINK_DETECT_STATUS_REG           0xC820
#define BCM82752_TX_SYNC_MODE_EN_REG                    0xC840
#define BCM82752_PMD_CONTROL_MODE_0_COPY_REG            0xC843
#define BCM82752_SPI_PORT_CONTROL_STATUS_REG            0xC848
#define BCM82752_LED_CONTROL_0_REG                      0xC8A0
#define BCM82752_PMD_CONTROL_MODE_0_REG                 0xC8D8
#define BCM82752_APPS_MODE_0_REG                        0xC8D9
#define BCM82752_APPS_MODE_2_REG                        0xC8DA
#define BCM82752_OPTICAL_CONFIGURATION_REG              0xC8E4
#define BCM82752_MACSEC_CONTROL_REG                     0xC8F0
#define BCM82752_BROADCAST_CONTROL_REG                  0xC8FE
#define BCM82752_TX_CONTROL_0_REG                       0xCA01
#define BCM82752_TX_CONTROL_1_REG                       0xCA02
#define BCM82752_TX_CONTROL_2_REG                       0xCA05
#define BCM82752_TX_CONTROL_2_EXTENSION_REG             0xCA06
#define BCM82752_TX_CONTROL_3_EXTENSION_REG             0xCA07
#define BCM82752_GENERAL_CONTROL_STATUS_REG             0xCA10
#define BCM82752_BOOT_STATUS_REG                        0xCA11
#define BCM82752_MESSAGE_IN_REG                         0xCA12
#define BCM82752_MESSAGE_OUT_REG                        0xCA13
#define BCM82752_GENERAL_PURPOSE_REG_0                  0xCA18
#define BCM82752_GENERAL_PURPOSE_REG_1                  0xCA19
#define BCM82752_GENERAL_PURPOSE_REG_2                  0xCA1A
#define BCM82752_GENERAL_PURPOSE_REG_3                  0xCA1B
#define BCM82752_GENERAL_PURPOSE_REG_4                  0xCA1C
#define BCM82752_MISCELLANEOUS_CONTROL_REG_1            0xCA23
#define BCM82752_MISCELLANEOUS_CONTROL_REG_2            0xCA24
#define BCM82752_EQ_CONTROL_REG                         0xCA65
#define BCM82752_EQ_STATUS_REG                          0xCA6D
#define BCM82752_MISCELLANEOUS_CONTROL_REG_3            0xCA85
#define BCM82752_GENERAL_PURPOSE_REG_5                  0xCA88
#define BCM82752_GENERAL_PURPOSE_REG_6                  0xCA89
#define BCM82752_GENERAL_PURPOSE_REG_7                  0xCA8A
#define BCM82752_GENERAL_PURPOSE_REG_8                  0xCA8B
#define BCM82752_GENERAL_PURPOSE_REG_9                  0xCA8C
#define BCM82752_GENERAL_CONTROL_REG                    0xCAA1
#define BCM82752_PMD_AND_PCS_STATUS_REG                 0xCD04
#define BCM82752_PMD_DIGITAL_CONTROL_REG                0xCD08
#define BCM82752_PMD_DIGITAL_STATUS_REG                 0xCD09
#define BCM82752_PMD_AND_PCS_TEST_CONTROL_REG           0xCD0A
#define BCM82752_PMD_GENERAL_PURPOSE_TX_PATTERN_REG_0   0xCD0C
#define BCM82752_PMD_GENERAL_PURPOSE_TX_PATTERN_REG_1   0xCD0D
#define BCM82752_PRBS31_TEST_WINDOW_0_REG               0xCD0F
#define BCM82752_PRBS31_TEST_WINDOW_1_REG               0xCD10
#define BCM82752_PRBS31_TEST_WINDOW_2_REG               0xCD11
#define BCM82752_USER_PRBS_CONTROL_0_REG                0xCD14
#define BCM82752_USER_PRBS_STATUS_0_REG                 0xCD15

/* start of bcm82752 MMD3(PCS) register address defination */
#define BCM82752_PCS_CONTROL_1_REG                      0x0000
#define BCM82752_PCS_STATUS_1_REG                       0x0001
#define BCM82752_PCS_ID_MSB_REG                         0x0002
#define BCM82752_PCS_ID_LSB_REG                         0x0003
#define BCM82752_SPEED_ABILITY_REG                      0x0004
#define BCM82752_DEVICES_IN_PACKAGE_1_REG               0x0005
#define BCM82752_DEVICES_IN_PACKAGE_2_REG               0x0006
#define BCM82752_PCS_CONTROL_2_REG                      0x0007
#define BCM82752_PCS_STATUS_2_REG                       0x0008
#define BCM82752_ORGANIZATIONALLY_UNIQUE_ID_MSB_REG     0x000E
#define BCM82752_ORGANIZATIONALLY_UNIQUE_ID_LSB_REG     0x000F
#define BCM82752_10GBASE_R_PCS_EEE_CAPABILITY_REG       0x0014
#define BCM82752_10GBASE_R_PCS_WAKE_ERROR_COUNTER_REG   0x0016
#define BCM82752_10GBASE_X_PCS_STATUS_REG               0x0018
#define BCM82752_10GBASE_R_PCS_STATUS_REG               0x0020
#define BCM82752_10GBASE_R_PCS_STATUS_2_REG             0x0021
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A0_REG  0x0022
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A1_REG  0x0023
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A2_REG  0x0024
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A3_REG  0x0025
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B0_REG  0x0026
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B1_REG  0x0027
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B2_REG  0x0028
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B3_REG  0x0029
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_CONTROL_REG  0x002A
#define BCM82752_10GBASE_R_PCS_JITTER_TEST_ERR_CNT_REG  0x002B

/* start of bcm82752 MMD7(1GBE) register address defination */
#define BCM82752_MII_CONTROL_REG                        0xFFE0
#define BCM82752_MII_STATUS_REG                         0xFFE1
#define BCM82752_AUTO_NEGOTIATION_ADVERTISEMENT_REG     0xFFE4
#define BCM82752_AUTO_NEGOTIATION_LINK_ABILITY_REG      0xFFE5
#define BCM82752_AUTO_NEGOTIATION_EXPANSION_REG         0xFFE6
#define BCM82752_AUTO_NEGOTIATION_NEXT_PAGE_REG         0xFFE7
#define BCM82752_AUTO_NEGOTIATION_LINK_RCVD_NEXT_PG_REG 0xFFE8
#define BCM82752_AUTO_NEGOTIATION_CONTROL_REG           0xFFEF
#define BCM82752_MISCELLANEOUS_2_REG                    0x8309
#define BCM82752_HOST_SIDE_1G_BIST_PATTERN_GEN_CTRL_REG 0x830A
#define BCM82752_HOST_SIDE_1G_BIST_CONTROL_REG          0x830C
#define BCM82752_HOST_SIDE_1G_BIST_TX_PACKET_COUNT_REG  0x830D
#define BCM82752_HOST_SIDE_1G_BIST_RX_PACKET_COUNT_REG  0x830E

#define BCM82752_PORT_NUM              2
#define BCM82752_PORT_LINE_LOCK_MASK   0x4
#define BCM82752_PORT_HOST_LOCK_MASK   0x4
#define BCM82752_XFI_SFI_SWITCH_REG    0xFFFF
#define BCM82752_XFI_SWITCH_MASK       0x1

#define BCM82752_MCODE_CKSUM_VERIFY_VALUE               0x600D
#define BCM82752_TX_REPEATERMODE_MASK  0x4
#define BCM82752_RX_REPEATERMODE_MASK  0x2

extern phy_reg_tbl_t bcm82752_global_reg_tbl[];
extern uint32_t bcm82752_global_reg_tbl_size;

extern phy_reg_tbl_t bcm82752_port_line_reg_tbl[];
extern uint32_t bcm82752_port_line_reg_tbl_size;

extern phy_reg_tbl_t bcm82752_port_host_reg_tbl[];
extern uint32_t bcm82752_port_host_reg_tbl_size;

/* 82752_PMD_ID_MSB AND 82758_PMD_ID_LSB */
#define PMD_PHY_ID                     0xAE025250

/*----------------------- Bitmasks ------------------------------ */

/* SFI_PMD_CTRL */
#define EDC_SOFT_RESET_BIT              BIT32(15)
#define SPEED_10G_MSK                   BIT32(13)

/* SFI_XFI_SWITCH_CTRL */
#define SFI_XFI_SWITCH_MSK              0x1

/* SFI_PMD_PCS_TEST_CTRL, XFI_PMD_PCS_TEST_CTRL */
#define LINE_LPBK_MSK                   0x1

/* SFI_PMD_CTRL, XFI_PMD_CTRL */
#define PMA_LPBK_MSK                    0x1

#define PRBS_LPBK_MSK                   0x180

/* SFI_PCS_CTRL_1, XFI_PCS_CTRL_1 */
#define SYSTEM_LPBK_MSK                 BIT32(14)

/* SFI_BIST_CTRL_STS */
#define PBIST_DETECT_BIT                BIT32(15)
#define XBIST_DETECT_BIT                BIT32(14)
#define XFI_BIST_TX_EN                  BIT32(9)
#define XFI_BIST_RX_CHECK_EN            BIT32(8)
#define PMD_BIST_TX_EN                  BIT32(7)
#define PMD_BIST_RX_CHECK_EN            BIT32(6)
#define BIST_PKT_SEQ_MSK                BIT32(3)
#define BIST_PKT_CNT_MSK                BIT32(1)

/* SFI_PCS_10G_R_JITTER_CTRL  */
#define PRBS31_RX_CHECK_EN              BIT32(5)
#define PRBS31_TX_EN                    BIT32(4)

/* SFI_SPI_PORT_CTRL_STS */
#define SPI_DWLD_DONE                   BIT32(13)

/* SFI_MII_CTRL */
#define AUTO_NEG_EN                     BIT32(12)
#define SPEED_SEL_MSB                   BIT32(6)
#define SPEED_SEL_LSB                   BIT32(13)

/* SFI_AN_AD */
#define HALF_DUPLEX_AD                  BIT32(6)
#define FULL_DUPLEX_AD                  BIT32(5)

/* SFI_MISC_2 */
#define FORCE_SPEED_ENC_EN              BIT32(5)

/* SFI_TX_CTRL_2 */
#define TAP_SEL_BIT                     BIT32(15)

/* SFI_SPEED_LINK_STS */
#define PCS_LKDWN10G                    BIT32(15)
#define PCS_LKDWN1G                     BIT32(13)

#define PBIST                           0x1
#define XBIST                           0x2
#define RANDOM_MODE                     0x1
#define SEQ_MODE                        0x2

#define EDC_RESET                       BIT32(16)
#define EDC_MCODE_LOAD                  BIT32(16)

/*  Bit field definition */
/* _PMD_RX_SIGNAL_DETECT        0xA */
#define GLOBAL_PMD_RX_SIGNAL_OK_VAL    0x1
/* _PCS_10G_R_STS               0x20 */
#define PCS_RX_BLOCK_LOCK_VAL    0x1      /* bit-0  1: 10GBASE-R PCS 64-66B sync locked to received blocks */
#define PCS_RX_HIGH_BER_VAL      0x2      /* bit-1  1: High BER */
#define PCS_RX_PRBS31_ABLE_VAL   0x4      /* bit-2  1: PRBS31 pattern testing ability */
#define PCS_RX_PRBS9_ABLE_VAL    0x8      /* bit-3  1: PRBS9 pattern testing ability */
#define PCS_RX_LINK_STAT_VAL     0x1000   /* bit-12 1: 10GBASE-R PCS receive linkup */

#define BCM82757_TX_CTRL5_REG 0x4800D0A5
#define BCM82757_TX_FIR_CTRL1_REG 0x4800D110
#define BCM82757_TX_FIR_CTRL2_REG 0x4800D111
#define BCM82757_PAD_GPIO0_0_CTRL_REG 0x8A82
#define BCM82757_PAD_GPIO0_1_CTRL_REG 0x8A86
#define BCM82757_PAD_GPIO1_0_CTRL_REG 0x8A84
#define BCM82757_PAD_GPIO1_1_CTRL_REG 0x8A88
#define BCM82757_PM_LED_MODE_REG 0x8BA0
#define BCM82757_PM_LED_PARAMS_REG 0x8BA1

enum {
        BNXT_CMD_UNSPEC,
        BNXT_CMD_HWRM,
        BNXT_NUM_CMDS
};

#define HWRM_PORT_SFP_SIDEBAND_CFG      0xd6UL
/* attributes */
enum {
        BNXT_ATTR_UNSPEC,
        BNXT_ATTR_PID,
        BNXT_ATTR_IF_INDEX,
        BNXT_ATTR_REQUEST,
        BNXT_ATTR_RESPONSE,
        BNXT_NUM_ATTRS
};

typedef uint8_t u8;
#define OPT_CONF_STAT_REG 0xc8e4
#define OPT_CONF_CTRL_REG 0xc800 
#define OPT_CONF_CTRL_VAL 0x383f 
#define RX_LOS_STATUS_MASK           0x40
#define TX_FLT_STATUS_MASK           0x20
#define MOD_ABS_STATUS_MASK          0x8
#define PIN_ACERT_VALUE             0x4
#define SIDEBAND_ASSERT_TIME    500
#define SIDEBAND_TIMEOUT        30 
#define BCM57412_SFP_PORT               12
#define DEV_IFINDEX_PATH_SIZE           64
#define BNXT_NL_NAME "bnxt_netlink"
#define BNXT_BUF_MAX 256
#define SYS_IFINDEX_PATH "/sys/class/net/%s/ifindex"
/* hwrm_port_sfp_sideband_cfg_input (size:256b/32B) */
typedef struct hwrm_port_sfp_sideband_cfg_input_ {
        __le16  req_type;
        __le16  cmpl_ring;
        __le16  seq_id;
        __le16  target_id;
        __le64  resp_addr;
        __le16  port_id;
        u8      unused_0[6];
        __le32  enables;
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RS0         0x1UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RS1         0x2UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_TX_DIS      0x4UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_MOD_SEL     0x8UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RESET_L     0x10UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_LP_MODE     0x20UL
        __le32  flags;
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RS0         0x1UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RS1         0x2UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS      0x4UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_MOD_SEL     0x8UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RESET_L     0x10UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_LP_MODE     0x20UL
} hwrm_port_sfp_sideband_cfg_input_t;

/* hwrm_port_sfp_sideband_cfg_output (size:128b/16B) */
typedef struct hwrm_port_sfp_sideband_cfg_output_ {
        __le16  error_code;
        __le16  req_type;
        __le16  seq_id;
        __le16  resp_len;
        u8      unused[7];
        u8      valid;
} hwrm_port_sfp_sideband_cfg_output_t;

typedef struct bcm_nl_request_msg_ {
    struct nlmsghdr n;
    struct genlmsghdr g;
    char buf[BNXT_BUF_MAX];
} bcm_nl_request_msg_t;

/*-------------------------------------------------
$Log: bcm82752_reg_def.h,v $
Revision 1.3  2020/11/03 06:17:48  leschen
To support SFP side band test

Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/03/12 07:41:51  leschen
Initial check in to support BCM82752


$Endlog$
*/
