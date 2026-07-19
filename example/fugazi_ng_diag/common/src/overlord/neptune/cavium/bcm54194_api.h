/* $Id: bcm54194_api.h,v 1.3 2018/07/23 07:38:47 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm54194_api.h,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.h - Header for BCM 10G PHY bcm54194 API.
 *
 *
 * June 2016, Mecca Ho
 *
 * Copyright (c) 2016 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define BCM54194_INTF_ACCESS_SWITCH_DELAY 10

#define BCM54194_CTRL_REG 0x00
#define BCM54194_STAT_REG 0x01
#define BCM54195_1000BASE_CTRL_REG 0x09
#define BCM54194_COPPER_AUXILIARY_CTRL_REG 0x028
#define BCM54194_TEST_1_REG 0x00E

#define BCM54194_INTERNAL_LOOPBACK (0x1 << 14)
#define BCM54194_AN_ENABLE (0x1 << 12)
#define BCM54194_SPEED_MASK 0x2040
#define BCM54194_SPEED_1000MBPS 0x0040
#define BCM54194_SPEED_100MBPS 0x2000
#define BCM54194_SPEED_10MBPS 0x0000
#define BCM54194_RESET_BIT (0x1 << 15)
#define BCM54194_DUPLEX_BIT (0x1 << 8)
#define BCM54194_POWER_DOWN_BIT (0x1 << 11)
#define BCM54194_FORCE_LINK_BIT (0x1 << 12)
#define BCM54194_LINK_STAT_BIT (0x1 << 2)

#define BCM54194_TOP_MII_REG_SOFT_RST_BIT (0x1 << 15)
#define BCM54194_1588_RESET_BIT (0x1 << 10)
#define BCM54194_TX_SOP_10BT_EN_BIT (0x1 << 8)
#define BCM54194_RX_SOP_10BT_EN_BIT (0x1 << 7)
#define BCM54194_TIME_SYNC_EN_BIT (0x1)

#define BCM54194_REG_1000X_EN_BIT (0x1)

#define BCM54194_I2C_MASTER_EN_BIT (0x1)
#define BCM54194_I2C_SDA_DEGL_EN_BIT (0x1 << 12)
#define BCM54194_I2C_SCL_DEGL_EN_BIT (0x1 << 11)
#define BCM54194_I2C_SOFT_RST_BIT (0x1 << 15)
#define BCM54194_I2C_DISABLE_ALL_PORT (0x8)
#define BCM54194_I2C_ENABLE_ALL_PORT (0xF)
#define BCM54194_I2C_SPD_400KBPS (0x1 << 1)
#define BCM54194_I2C_CMD_MASK (0x7 << 2)
#define BCM54194_I2C_NO_OP_CMD (0x0)
#define BCM54194_I2C_READ_CURR_CMD (0x1 << 2)
#define BCM54194_I2C_WRITE_CMD (0x2 << 2)
#define BCM54194_I2C_READ_CMD (0x3 << 2)
#define BCM54194_I2C_FLUSH_CMD (0x4 << 2)
#define BCM54194_I2C_CMD_DONE_BIT (0x1 << 9)

#define BCM54194_FORCE_AUTO_MDIX_BIT (0x1 << 9)

#define BCM54194_PATTERN_GEN_CTRL_REG 0x5
#define BCM54194_TIME_SYNC_REG 0xF5
#define BCM54194_TOP_MISC_TOP_GBL_RST_REG 0x82B
#define BCM54194_P1588_MPLS_LABEL1_MASK_MSB_REG 0xA98
#define BCM54194_P1588_MPLS_LABEL2_MASK_MSB_REG 0xA9C
#define BCM54194_P1588_MPLS_LABEL3_MASK_MSB_REG 0xAA0
#define BCM54194_P1588_MPLS_LABEL4_MASK_MSB_REG 0xAA4
#define BCM54194_P1588_MPLS_LABEL5_MASK_MSB_REG 0xAA8
#define BCM54194_P1588_MPLS_LABEL6_MASK_MSB_REG 0xAAC
#define BCM54194_P1588_MPLS_LABEL7_MASK_MSB_REG 0xAB0
#define BCM54194_P1588_MPLS_LABEL8_MASK_MSB_REG 0xAB4
#define BCM54194_P1588_SLICE_EN_CTRL_REG 0xA10
#define BCM54194_P1588_SOP_SELECTION_REG 0xAF8
#define BCM54194_P1588_TIMECODE_SEL_REG 0xAC3
#define BCM54194_P1588_DPLL_DEBUG_SELECT_REG 0xA5B
#define BCM54194_P1588_PCH_TS_FIFO_RD_START_END_REG 0xA85
#define BCM54194_P1588_PCH_TS_FIFO_0_REG 0xA89
#define BCM54194_P1588_PCH_TS_FIFO_1_REG 0xA8A
#define BCM54194_P1588_PCH_TAGID_REG 0xA8B
#define BCM54194_P1588_PCH_CRC8_MISMATCH_REG 0xAC4
#define BCM54194_P1588_PCH_TS_INFO_1_REG 0xA8C
#define BCM54194_P1588_PCH_TS_INFO_2_REG 0xA8D
#define BCM54194_P1588_PCH_TS_INFO_3_REG 0xAF9
#define BCM54194_P1588_PCH_TS_INFO_4_REG 0xAFA
#define BCM54194_P1588_PCH_TS_INFO_5_REG 0xAFB
#define BCM54194_P1588_PCH_TS_INFO_6_REG 0xAFC
#define BCM54194_P1588_PCH_TS_INFO_7_REG 0xAFD
#define BCM54194_P1588_PCH_TS_INFO_8_REG 0xAFE
#define BCM54194_P1588_CTRL_DEBUG_REG 0xA8E
#define BCM54194_P1588_INBAND_CTRL_PORT_REG 0xAEE
#define BCM54194_P1588_TX_SOP_TS_CAP_EN_REG 0xA21
#define BCM54194_P1588_RX_SOP_TS_CAP_EN_REG 0xA22

#define BCM54194_EXTERNAL_SERDES_CTRL_REG 0x234
#define BCM54194_SGMII_SLAVE_REG 0x235

#define BCM54194_I2C_MASTER_CTRL_REG 0x885
#define BCM54194_I2C_MASTER_DEV_ADDR_REG 0x886
#define BCM54194_I2C_MASTER_REG_ADDR_REG 0x887
#define BCM54194_I2C_MASTER_WDAT_REG 0x888
#define BCM54194_I2C_MASTER_RDAT_REG 0x889
#define BCM54194_I2C_MASTER_STS_REG 0x88A

#define BCM54194_COPPER_MISCEL_CTRL_REG 0x2F

#define BCM54194_TEST_MODE_1 (0x1)
#define BCM54194_TEST_MODE_2 (0x2)
#define BCM54194_TEST_MODE_3 (0x3)
#define BCM54194_TEST_MODE_4 (0x4)

#define BCM54194_TEST_MODE_MASK          (0x7 << 13)
#define BCM54194_NORMAL_MODE             (0x0 << 13)
#define BCM54194_TRANSMIT_WAVE_TEST      (0x1 << 13)
#define BCM54194_MS_TRANSMIT_JITTER_TEST (0x2 << 13)
#define BCM54194_SL_TRANSMIT_JITTER_TEST (0x3 << 13)
#define BCM54194_TRANSMIT_DIST_TEST      (0x4 << 13)

#define BCM54194_SGMII_SLAVE_MODE_EN_BIT (0x1 << 1)
#define BCM54194_AUTO_DET_MEDIUM_EN_BIT 0x1
#define BCM54194_MODE_SEL_MASK (0x3 << 1)
#define BCM54194_SGMII_TO_COPPER_MODE (0x0 << 1)
#define BCM54194_SGMII_TO_FIBER_MODE (0x1 << 1)
#define BCM54194_100BASE_FX_MODE_EN 0x21
#define BCM54194_SERFES_PORT_SEL_MASK (0x7 << 12)
#define BCM54194_SERFES_PORT0_FIBER (0x4 << 12)
#define BCM54194_SERFES_PORT1_FIBER (0x5 << 12)
#define BCM54194_SERDES_RISE_FALL_MASK (0x3 << 14)
#define BCM54194_SERDES_RISE_FALL_142NS (0x2 << 14)
#define BCM54194_SD_THRESHOLD_MASK 0xF
#define BCM54194_SD_THRESHOLD_100mV 0xD

#define BCM54194_AUTO_DETECT_MEDIUM_REG 0x23E
#define BCM54194_MODE_CTRL_REG 0x021
#define BCM54194_SERDES_100FX_CTRL_REG 0x233
#define BCM54194_SGMII_LN_CTRL_1G_REG 0x902
#define BCM54194_SGMII_TX_ACTRL_2_REG 0x923
#define BCM54194_SGMII_RX_ACTRL_5_REG 0x945

/* Neptune ioctl cmd for suspend/resume update GE phy link status */
#define SIOCNEPSUSPENDUPLINK  0x89F1
#define SIOCNEPRESUMEUPLINK   0x89F2

typedef enum bcm54194_intf
{
    BCM54194_SGMII_INTF = 0,
    BCM54194_COPPER_INTF,
    BCM54194_FIBER_INTF
} bcm54194_intf_t;

extern int bcm54194_switch_intf_access(bcm54194_intf_t);
extern int bcm54194_reg_1000x_en(int , int);
extern int bcm54194_soft_reset(void);
extern int bcm54194_cfg_setting(int, int, int, int, bcm54194_intf_t);
extern int bcm54194_config_loopback(int, int, bcm54194_intf_t, int, int);
extern int bcm54194_rdb_read(int , int , int , uint16_t *);
extern int bcm54194_rdb_write(int, int, int, uint16_t);
extern boolean bcm54194_is_linkup(int, bcm54194_intf_t);
extern int bcm54194_sig_pwr_ctrl(int, boolean, bcm54194_intf_t);
extern int enable_bcm54194_ibts_gm_sync_t1_ts_da_cap(int);
extern int enable_bcm54194_ibts_sc_sync_t2_ts_da_cap(int);
extern int enable_bcm54194_ibts_sc_dreq_t3_assist(int);
extern int enable_bcm54194_ibts_gm_rx_dreq_t4(int);
extern int dump_bcm54194_timestamp (void);
extern int enable_bcm54194_i2c_access (boolean);
extern int bcm54194_i2c_slave_read (int , int , ushort *);
extern int bcm54194_i2c_slave_write (int , int , ushort);
extern int bcm54194_i2c_slave_flush (int);
extern int bcm54194_suspend_lnx_link_polling (char *, int , boolean);
extern int bcm54194_transmit_test_pattern(int , int, int);
extern void init_bcm54194_macsec(uint);
extern void disable_bcm54194_macsec(uint);
extern void bcm54194_reset(void);
extern void bcm54194_init_script(void);
extern int bcm54194_sgmii_slave_mode(int, int);
extern int dump_bcm54194_loopback_config(int, int);
/*-------------------------------------------------
$Log: bcm54194_api.h,v $
Revision 1.3  2018/07/23 07:38:47  meho
Added dump bcm54194 internal loopback setting when failure occur.

Revision 1.2  2018/05/18 09:24:53  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.18  2018/04/16 08:42:19  meho
Added GLC-GE-100FX SFP loopback utility.

Revision 1.1.2.17  2018/01/10 09:23:17  meho
Added 100M SFP external loopback utility.

Revision 1.1.2.16  2017/12/29 06:28:11  meho
Workaround for BCM54194 B0 silicon MDIO address issue.

Revision 1.1.2.15  2017/10/30 08:52:57  meho
Added 1588 config script for BCM54194.

Revision 1.1.2.14  2017/10/18 09:18:20  meho
Added BCM54194 reset by FPGA.

Revision 1.1.2.13  2017/10/17 09:58:46  meho
Added bcm54194 MACsec test.

Revision 1.1.2.12  2017/01/11 03:40:08  meho
Added GE PHY Test Mode Util.

Revision 1.1.2.11  2016/11/28 03:43:55  meho
1. Fixed GE phy Mac/Int/Ext loopback test bugs.
2. Added 10G FW download.

Revision 1.1.2.10  2016/09/14 02:44:28  meho
Added BCM54194 I2C r/w utilities.

Revision 1.1.2.9  2016/08/18 06:57:49  meho
Code clean up.

Revision 1.1.2.8  2016/08/04 03:39:38  meho
Added the enable BCM54194 PTP function in loopback test.

Revision 1.1.2.7  2016/08/03 06:25:19  meho
Added enable PTP1588 sequence for BCM54195.

Revision 1.1.2.6  2016/07/26 07:54:26  meho
Added GE PHY PTP1588 loopback test skeleton.

Revision 1.1.2.5  2016/07/20 01:44:59  meho
Added GE PHY loopback debug utilities.

Revision 1.1.2.4  2016/07/13 08:28:09  meho
1. Added Cavium PCS internal loopback.
2. Added check link up function for bcm54194.

Revision 1.1.2.3  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.2  2016/07/07 09:04:29  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.

Revision 1.1.2.1  2016/06/23 12:44:54  meho
Added bcm54194 soft-reset function.



$Endlog$
*/
