/* $Id: diag_bcm54194_api.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm54194_api.h,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.h - Header for BCM 10G PHY bcm54194 API.
 *
 *
 * June 2016, Mecca Ho
 *
 * Copyright (c) 2016 - 2021 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

/*
 * IEEE Standard Registers
 */

#define BCM54194_CTRL_REG 0x00
  #define BCM54194_RESET_BIT               (0x1 << 15)
  #define BCM54194_INTERNAL_LOOPBACK       (0x1 << 14)
  #define BCM54194_SPEED_MASK              0x2040
  #define BCM54194_SPEED_1000MBPS          0x0040
  #define BCM54194_SPEED_100MBPS           0x2000
  #define BCM54194_SPEED_10MBPS            0x0000
  #define BCM54194_AN_ENABLE               (0x1 << 12)
  #define BCM54194_POWER_DOWN_BIT          (0x1 << 11)
  #define BCM54194_RESTART_AN              (0x1 << 9)
  #define BCM54194_DUPLEX_BIT              (0x1 << 8)
  #define BCM54194_MSB_SPEED_SEL           (0x1 << 6)
  #define BCM54194_CTRL_REG_RESERVED_BITS  0x00

#define BCM54194_STAT_REG 0x01
  #define BCM54194_LINK_STAT_BIT           (0x1 << 2)

#define BCM54195_1000BASE_CTRL_REG 0x09
  #define BCM54194_TEST_MODE_1             (0x1)
  #define BCM54194_TEST_MODE_2             (0x2)
  #define BCM54194_TEST_MODE_3             (0x3)
  #define BCM54194_TEST_MODE_4             (0x4)
  #define BCM54194_TEST_MODE_MASK          (0x7 << 13)
  #define BCM54194_NORMAL_MODE             (0x0 << 13)
  #define BCM54194_TRANSMIT_WAVE_TEST      (0x1 << 13)
  #define BCM54194_MS_TRANSMIT_JITTER_TEST (0x2 << 13)
  #define BCM54194_SL_TRANSMIT_JITTER_TEST (0x3 << 13)
  #define BCM54194_TRANSMIT_DIST_TEST      (0x4 << 13)


/*
 * RDB Registers
 */

#define BCM54194_PATTERN_GEN_CTRL_REG     0x005

#define BCM54194_TEST_1_REG               0x00E
  #define BCM54194_FORCE_LINK_BIT   (0x1 << 12)

#define BCM54194_MODE_CTRL_REG                  0x021
  #define BCM54194_SERDES_LINK_UP         (0x1 << 6)
  #define BCM54194_MODE_SEL_MASK          (0x3 << 1)
  #define BCM54194_SGMII_TO_COPPER_MODE   (0x0 << 1)
  #define BCM54194_SGMII_TO_FIBER_MODE    (0x1 << 1)
  #define BCM54194_REG_1000X_EN_BIT       (0x1)

#define BCM54194_COPPER_AUXILIARY_CTRL_REG      0x028

#define BCM54194_COPPER_POWER_MII_CTRL_REG      0x02A
  #define BCM54194_SUPPER_ISOLATE         (0x1 << 5)

#define BCM54194_COPPER_MISCEL_TEST_REG         0x02C
  #define BCM54194_RMT_LPBK_EN             (0x1 << 15)

#define BCM54194_COPPER_MISCEL_CTRL_REG         0x02F
  #define BCM54194_RX_PKT_COUNTER_EN       (0x1 << 11)
  #define BCM54194_FORCE_AUTO_MDIX_BIT     (0x1 << 9)

#define BCM54194_EXPANSION_INTERRUPT_STATUS_REG 0x031

#define BCM54194_EXPANSION_INTERRUPT_MASK_REG   0x032
  #define BCM54194_SERDES_LINK_STATUS_CHANGE_INT_DIS  (0x1 << 6)

#define BCM54194_PORT_INTERRUPT_STATUS_REG      0x032

#define BCM54194_TIME_SYNC_REG                  0xF5
  #define BCM54194_TX_SOP_10BT_EN_BIT     (0x1 << 8)
  #define BCM54194_RX_SOP_10BT_EN_BIT     (0x1 << 7)
  #define BCM54194_TIME_SYNC_EN_BIT       (0x1)

#define BCM54194_PRBS_CTRL_REG                0x200
  #define TEST_PRBS_ERR_CNTR              0x20
  #define CLR_PRBS_ERR_CNTR               0x10
  #define BCM54194_PRBS_23                0x8
  #define BCM54194_PRBS_15                0x4
  #define BCM54194_PRBS_7                 0x0
  #define BCM54194_PRBS_INVERT            0x2
  #define PRBS_ENABLE                     0x1

#define BCM54194_PRBS_STATUS_REG                0x201
  #define PRBS_LOCKED                     0x800
  #define PRBS_LOST_LOCK                  0x1000
  #define PRBS_ERR_CNTR_MASK              0x07FF /* 10bits PRBS error count. */

#define BCM54194_SERDES_100FX_CTRL_REG          0x233
  #define BCM54194_100BASE_FX_MODE_EN     0x21

#define BCM54194_EXTERNAL_SERDES_CTRL_REG       0x234
  #define CU_FIBER_SGMII_REG_MASK         0x60
  #define CU_FIBER_SGMII_REG_SGMII_MODE   0x40

#define BCM54194_SGMII_SLAVE_REG                0x235
  #define BCM54194_SGMII_SLAVE_MODE_EN_BIT (0x1 << 1)

#define BCM54194_MISC_1000X_CTRL_2_REG          0x237
  #define BCM54194_SIG_DET_EN             (0x1 << 5)

#define BCM54194_AUTO_DETECT_MEDIUM_REG         0x23E
  #define BCM54194_AUTO_DET_MEDIUM_EN_BIT 0x1
  #define BCM54194_AUXILIARY_100X_SEL     0x7800
  #define BCM54194_FIBER_IN_USE_LED       (0x1 << 7)
  #define BCM54194_FIBER_LED              (0x1 << 6)
  #define BCM54194_FIBER_SD_SYNC_STATUS   (0x1 << 5)
  #define BCM54194_AUTO_DET_MEDIUM_EN_BIT 0x1

#define BCM54194_TOP_LEVEL_PIN_CTRL_REG         0x811
  #define BCM54194_SFP_TXDIS_EN             (0x1 << 15)
  #define BCM54194_SFP_TXFLT_RXLOS_EN       (0x1 << 14)

#define BCM54194_TOP_MISC_TOP_GBL_RST_REG       0x82B
  #define BCM54194_TOP_MII_REG_SOFT_RST_BIT (0x1 << 15)
  #define BCM54194_1588_RESET_BIT           (0x1 << 10)

#define BCM54194_TOP_INTERRUPT_MASK_REG         0x82D
  #define BCM54194_PORT_ALL_INT_MASK    0x00F0
  #define BCM54194_PORT0_INT_DIS        (0x1 << 4)

#define BCM54194_I2C_MASTER_CTRL_REG            0x885
  #define BCM54194_I2C_SOFT_RST_BIT      (0x1 << 15)
  #define BCM54194_I2C_SDA_DEGL_EN_BIT   (0x1 << 12)
  #define BCM54194_I2C_SCL_DEGL_EN_BIT   (0x1 << 11)
  #define BCM54194_I2C_CMD_DONE_BIT      (0x1 << 9)
  #define BCM54194_I2C_ENABLE_ALL_PORT   (0xF << 5)
  #define BCM54194_I2C_DISABLE_ALL_PORT  (0x8 << 5)
  #define BCM54194_I2C_CMD_MASK          (0x7 << 2)
  #define BCM54194_I2C_NO_OP_CMD         (0x0 << 2)
  #define BCM54194_I2C_READ_CURR_CMD     (0x1 << 2)
  #define BCM54194_I2C_WRITE_CMD         (0x2 << 2)
  #define BCM54194_I2C_READ_CMD          (0x3 << 2)
  #define BCM54194_I2C_FLUSH_CMD         (0x4 << 2)
  #define BCM54194_I2C_SPD_400KBPS       (0x1 << 1)
  #define BCM54194_I2C_MASTER_EN_BIT     (0x1)


#define BCM54194_I2C_MASTER_DEV_ADDR_REG        0x886
#define BCM54194_I2C_MASTER_REG_ADDR_REG        0x887
#define BCM54194_I2C_MASTER_WDAT_REG            0x888
#define BCM54194_I2C_MASTER_RDAT_REG            0x889
#define BCM54194_I2C_MASTER_STS_REG             0x88A

#define BCM54194_TOP_MISC_SFP_STS0_REG          0x890
  #define BCM54194_SFP_P1_CONN_STS        (0x1 << 13) /* port 1 SFP current connection status */
  #define BCM54194_SFP_P0_CONN_STS        (0x1 << 12) /* port 0 SFP current connection status */
  #define BCM54194_SFP_P1_RXL_CHG         (0x1 << 5)  /* port 1 SFP rx_los change */
  #define BCM54194_SFP_P1_TXF_CHG         (0x1 << 4)  /* port 1 SFP tx_fault change */
  #define BCM54194_SFP_P1_CONN_CHG        (0x1 << 3)  /* port 1 SFP sfp_abs change */
  #define BCM54194_SFP_P0_RXL_CHG         (0x1 << 2)  /* port 0 SFP rx_los change */
  #define BCM54194_SFP_P0_TXF_CHG         (0x1 << 1)  /* port 0 SFP tx_fault change */
  #define BCM54194_SFP_P0_CONN_CHG        (0x1)       /* port 0 SFP sfp_abs change */
  #define BCM54194_SFP_P0_P1_SIDE_BAND_CHG   (BCM54194_SFP_P1_CONN_STS | BCM54194_SFP_P0_CONN_STS | BCM54194_SFP_P1_RXL_CHG | BCM54194_SFP_P0_RXL_CHG)


#define BCM54194_SGMII_LN_CTRL_1G_REG           0x902
  #define BCM54194_SERDES_PORT_SEL_MASK  (0x7 << 12)
  #define BCM54194_SERDES_PORT0_FIBER    (0x4 << 12)
  #define BCM54194_SERDES_PORT1_FIBER    (0x5 << 12)

#define BCM54194_SGMII_TX_ACTRL_2_REG           0x923
  #define BCM54194_SERDES_RISE_FALL_MASK  (0x3 << 14)
  #define BCM54194_SERDES_RISE_FALL_142NS (0x2 << 14)

#define BCM54194_SGMII_RX_ACTRL_5_REG           0x945
  #define BCM54194_SD_THRESHOLD_MASK      0xF
  #define BCM54194_SD_THRESHOLD_100mV     0xD

#define BCM54194_P1588_SLICE_EN_CTRL_REG        0xA10
#define BCM54194_P1588_TX_SOP_TS_CAP_EN_REG     0xA21
#define BCM54194_P1588_RX_SOP_TS_CAP_EN_REG     0xA22
#define BCM54194_P1588_DPLL_DEBUG_SELECT_REG    0xA5B
#define BCM54194_P1588_PCH_TS_FIFO_RD_START_END_REG 0xA85
#define BCM54194_P1588_PCH_TS_FIFO_0_REG        0xA89
#define BCM54194_P1588_PCH_TS_FIFO_1_REG        0xA8A
#define BCM54194_P1588_PCH_TAGID_REG            0xA8B
#define BCM54194_P1588_PCH_TS_INFO_1_REG        0xA8C
#define BCM54194_P1588_PCH_TS_INFO_2_REG        0xA8D
#define BCM54194_P1588_PCH_TS_INFO_8_REG        0xAFE
#define BCM54194_P1588_CTRL_DEBUG_REG           0xA8E
#define BCM54194_P1588_MPLS_LABEL1_MASK_MSB_REG 0xA98
#define BCM54194_P1588_MPLS_LABEL2_MASK_MSB_REG 0xA9C
#define BCM54194_P1588_MPLS_LABEL3_MASK_MSB_REG 0xAA0
#define BCM54194_P1588_MPLS_LABEL4_MASK_MSB_REG 0xAA4
#define BCM54194_P1588_MPLS_LABEL5_MASK_MSB_REG 0xAA8
#define BCM54194_P1588_MPLS_LABEL6_MASK_MSB_REG 0xAAC
#define BCM54194_P1588_MPLS_LABEL7_MASK_MSB_REG 0xAB0
#define BCM54194_P1588_MPLS_LABEL8_MASK_MSB_REG 0xAB4
#define BCM54194_P1588_TIMECODE_SEL_REG         0xAC3
#define BCM54194_P1588_PCH_CRC8_MISMATCH_REG    0xAC4
#define BCM54194_P1588_INBAND_CTRL_PORT_REG     0xAEE
#define BCM54194_P1588_SOP_SELECTION_REG        0xAF8
#define BCM54194_P1588_PCH_TS_INFO_3_REG        0xAF9
#define BCM54194_P1588_PCH_TS_INFO_4_REG        0xAFA
#define BCM54194_P1588_PCH_TS_INFO_5_REG        0xAFB
#define BCM54194_P1588_PCH_TS_INFO_6_REG        0xAFC
#define BCM54194_P1588_PCH_TS_INFO_7_REG        0xAFD


/*
 * Others definitions
 */

#define BCM54194_INTF_ACCESS_SWITCH_DELAY 10

/* ioctl cmd for suspend/resume update GE phy link status */
#define SIOCNEPSUSPENDUPLINK  0x89F1
#define SIOCNEPRESUMEUPLINK   0x89F2

typedef enum bcm54194_intf
{
    BCM54194_SGMII_INTF = 0,
    BCM54194_COPPER_INTF,
    BCM54194_FIBER_INTF
} bcm54194_intf_t;

typedef enum {
    FUGAZI_1G_PHY_0,
    FUGAZI_1G_PHY_1,
    FUGAZI_1G_PHY_2,
    FUGAZI_1G_PHY_3,
    MAX_FUGAZI_1G_PHY
} fugazi_1g_phy_t;

typedef enum {
    FUGAZI_1G_eth_4 = 4,
    FUGAZI_1G_eth_5,
    FUGAZI_1G_eth_6,
    FUGAZI_1G_eth_7,
    FUGAZI_1G_eth_8,
    FUGAZI_1G_eth_9,
    FUGAZI_1G_eth_10,
    FUGAZI_1G_eth_11,
    MAX_FUGAZI_1G_ETH
} fugazi_1g_eth_t;

extern int bcm54194_switch_intf_access(int, bcm54194_intf_t);
extern int bcm54194_reg_1000x_en(int, int, int);
extern int bcm54194_soft_reset(void);
extern int bcm54194_cfg_setting(int, int, int, int, int, bcm54194_intf_t);
extern int bcm54194_config_loopback(int, int, int, bcm54194_intf_t, int, int);
extern int bcm54194_rdb_read(int, int , int , uint16_t *);
extern int bcm54194_rdb_write(int, int, int, uint16_t);
extern boolean bcm54194_is_linkup(int, int, bcm54194_intf_t);
extern int bcm54194_sig_pwr_ctrl(int, int, boolean, bcm54194_intf_t);
extern int enable_bcm54194_ibts_gm_sync_t1_ts_da_cap(int);
extern int enable_bcm54194_ibts_sc_sync_t2_ts_da_cap(int);
extern int enable_bcm54194_ibts_sc_dreq_t3_assist(int);
extern int enable_bcm54194_ibts_gm_rx_dreq_t4(int);
extern int dump_bcm54194_timestamp (void);
extern int enable_bcm54194_i2c_access (boolean);
extern int bcm54194_i2c_slave_read(int , int , ushort *);
extern int bcm54194_i2c_slave_write(int , int , ushort);
extern int bcm54194_i2c_slave_flush(int);
extern int bcm54194_suspend_lnx_link_polling(char *, int , boolean);
extern int bcm54194_transmit_test_pattern(int, int);
extern void init_bcm54194_macsec(uint);
extern void disable_bcm54194_macsec(uint);
extern void bcm54194_reset(int);
extern void bcm54194_init_script(void);
extern int bcm54194_sgmii_slave_mode(int, int, int);
extern int dump_bcm54194_loopback_config(int, int, int);
extern void check_link(void);
extern  int set_line_side_config(int, int);
extern void config_lpbk_mode(void);
extern int  bcm54194_recover_clock(int, int);
extern int bcm54194_config_prbs(void);
extern void packet_counter_util(void);
extern void bcm54194_interrupt_util (void);
extern int bcm54194_interrupt_test(int, uint16_t *);
extern int bcm54194_interrupt_get(int, int, uint16_t *);
extern int bcm54194_interrupt_generate(int, int, int);
extern int bcm54194_interrupt_set(int, int, int);
extern int bcm54194_interrupt_clear(int, int);
extern int bcm54194_config_interrupt(int, int, int);
extern boolean fugazi_is_1g_phy_linkup(int);

/*-------------------------------------------------
$Log: diag_bcm54194_api.h,v $
Revision 1.2  2021/06/02 08:22:34  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.1.4.3  2021/04/29 01:42:11  pdoong
Add checking if PHY Network side link is up in 'SyncE Recovered Clock Test'

Revision 1.1.4.2  2020/08/26 02:37:47  iachang
Merge Fugazi code into main trunk

Revision 1.1.2.6  2020/08/25 04:10:50  pdoong
Correct BCM54194_SFP_P0_P1_SIDE_BAND_CHG for checking side-band test bits.

Revision 1.1.2.5  2020/08/25 01:10:25  pdoong
Updated code from PRRQ comments.

Revision 1.1.2.4  2020/08/24 00:01:52  pdoong
Adde more BCM54194 register defintion macro for ER.

Revision 1.1.2.3  2020/04/06 07:05:10  iachang
Add BCM54194 LASI Interrupt Test

Revision 1.1.2.2  2020/02/25 02:48:24  pdoong
add utility to enable/generate 1G PHY interrupt to BCM57412 MAC LASI

Revision 1.1.2.1  2019/10/16 06:12:31  letsai
Modify file name

Revision 1.1.6.12  2019/09/23 07:38:25  letsai
Add packet counter utility of BCM54194 phy

Revision 1.1.6.11  2019/08/21 06:38:57  letsai
Add BCM54194 1G PHY PRBS utility

Revision 1.1.6.10  2019/06/15 03:48:42  letsai
1.Fix Rx mismatch error messgage showed in loopback test. 2.Removed Copper registers in BCM54194 phy register test. 3.Add print messgge when reset 1G phy.

Revision 1.1.6.9  2019/05/21 23:22:34  pdoong
Added SyncE recovered clock test from bcm54194 1G PHY output clock

Revision 1.1.6.8  2019/04/18 23:11:58  letsai
Add loopback mode config uyility and clean up code.

Revision 1.1.6.7  2019/04/18 01:21:30  letsai
1. Clean up code
2. Modify 1G phy address mapping
3. Modify print message of MCU FW opgrade

Revision 1.1.6.6  2019/04/12 23:03:25  letsai
Add utility to enable 1000BASE-X Line-Side Loopback

Revision 1.1.6.5  2019/04/10 16:29:30  letsai
1. Fix ethernet mapping.
2. Support all BCM54194 phy in utilities.
3. Remove unused functions.

Revision 1.1.6.4  2019/04/09 16:10:40  letsai
1. Support all BCM54194 PHY (0~3) Register Test.
2. Let utilities can dump each phy registers.
3. Check link status for each phy and each port(upstream and downstream).

Revision 1.1.6.3  2019/04/03 18:30:36  letsai
Add utility to check link status

Revision 1.1.6.2  2019/03/14 03:48:24  letsai
Initial check in.



$Endlog$
*/
