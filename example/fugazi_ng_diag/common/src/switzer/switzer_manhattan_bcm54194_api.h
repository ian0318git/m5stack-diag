#ifndef __SWITZER_MANHATTAN_BCM54194_API_H__
#define __SWITZER_MANHATTAN_BCM54194_API_H__
/*
 *-----------------------------------------------------------------------------
 * switzer_manhattan_bcm54194_api.h
 * APIs for BCM54194.
 *
 * Copyright (c) 2016 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <types.h>
#include <sys/socket.h>
#include <bits/socket.h>
#include <linux/if.h>
#include "switzer_common.h"

typedef enum manhattan_bcm54194_intf
{
    MANHATTAN_BCM54194_INTF_SGMII = 0,
    MANHATTAN_BCM54194_INTF_COPPER,
    MANHATTAN_BCM54194_INTF_FIBER
} manhattan_bcm54194_intf_t;

#define MANHATTAN_INF_NAME(INTF) ({                     \
    assert((INTF) <= MANHATTAN_BCM54194_INTF_FIBER);    \
    (INTF) == MANHATTAN_BCM54194_INTF_SGMII ? "SGMII" : \
    (INTF) == MANHATTAN_BCM54194_INTF_COPPER? "COPPER": \
    (INTF) == MANHATTAN_BCM54194_INTF_FIBER ? "FIBER" : "Unknown"; })

#define MANHATTAN_FRONT_RJ45_PORT_NUMB      2
#define MANHATTAN_FRONT_SFP_PORT_NUMB       2
#define MANHATTAN_BCM54194_PORT_NUMB        2
#define MANHATTAN_BCM54194_PHYAD            0xF
#define MANHATTAN_BCM54194_PORT_NUMB_TOTAL  4
#define MANHATTAN_BCM54194_PHYAD_PORT(PORT)         ({assert((PORT) < MANHATTAN_BCM54194_PORT_NUMB_TOTAL); MANHATTAN_BCM54194_PHYAD + (PORT);})
#define MANHATTAN_BCM54194_PHYAD_SGMII(PORT)        ({assert((PORT) < MANHATTAN_BCM54194_PORT_NUMB_TOTAL); MANHATTAN_BCM54194_PHYAD + (PORT) + 4;})
#define MANHATTAN_BCM54194_PORT_BY_PHYAD(PHYAD)     (PHYAD - MANHATTAN_BCM54194_PHYAD)
#define MANHATTAN_BCM54194_PHYAD_GET(PORT, INTF)    ({   \
        assert((INTF) <= MANHATTAN_BCM54194_INTF_FIBER); \
        ((INTF) == MANHATTAN_BCM54194_INTF_COPPER || (INTF) == MANHATTAN_BCM54194_INTF_FIBER) ? \
            MANHATTAN_BCM54194_PHYAD_PORT(PORT) :        \
            MANHATTAN_BCM54194_PHYAD_SGMII(PORT);})

#define MANHATTAN_FRONT_PORT_TO_54194_PORT(PORT) ({assert((PORT) < MANHATTAN_BCM54194_PORT_NUMB); (PORT) == 0 ? 1: 0;})
#define MANHATTAN_54194_PORT_TO_FRONT_PORT(PORT) ({assert((PORT) < MANHATTAN_BCM54194_PORT_NUMB); (PORT) == 0 ? 1: 0;})


#define BCM54194_INTF_ACCESS_SWITCH_DELAY   10

#define BCM54194_ORG_UNIQ_ID                0xAE02

#define BCM54194_CTRL_REG                   0x00
#define BCM54194_STAT_REG                   0x01
#define BCM54194_PHY_IDENTIFIER_MSB         0x02
#define BCM54194_PHY_IDENTIFIER_LSB         0x03
#define BCM54195_1000BASE_CTRL_REG          0x09
#define BCM54194_C45_BY_C22_DEVAD_REG       0x0D
#define BCM54194_C45_BY_C22_DATA_REG        0x0E
#define BCM54194_COPPER_AUXILIARY_CTRL_REG  0x028
#define BCM54194_TEST_1_REG                 0x00E

#define BCM54194_INTERNAL_LOOPBACK     (0x1 << 14)
#define BCM54194_AN_ENABLE             (0x1 << 12)
#define BCM54194_SPEED_MASK             0x2040
#define BCM54194_SPEED_1000MBPS         0x0040
#define BCM54194_SPEED_100MBPS          0x2000
#define BCM54194_SPEED_10MBPS           0x0000
#define BCM54194_RESET_BIT             (0x1 << 15)
#define BCM54194_DUPLEX_BIT            (0x1 << 8)
#define BCM54194_POWER_DOWN_BIT        (0x1 << 11)
#define BCM54194_FORCE_LINK_BIT        (0x1 << 12)
#define BCM54194_LINK_STAT_BIT         (0x1 << 2)

#define BCM54194_TOP_MII_REG_SOFT_RST_BIT (0x1 << 15)
#define BCM54194_1588_RESET_BIT           (0x1 << 10)
#define BCM54194_TX_SOP_10BT_EN_BIT       (0x1 << 8)
#define BCM54194_RX_SOP_10BT_EN_BIT       (0x1 << 7)
#define BCM54194_TIME_SYNC_EN_BIT         (0x1)

#define BCM54194_REG_1000X_EN_BIT         (0x1)

#define BCM54194_I2C_MASTER_EN_BIT        (0x1)
#define BCM54194_I2C_SDA_DEGL_EN_BIT      (0x1 << 12)
#define BCM54194_I2C_SCL_DEGL_EN_BIT      (0x1 << 11)
#define BCM54194_I2C_SOFT_RST_BIT         (0x1 << 15)
#define BCM54194_I2C_DISABLE_ALL_PORT     (0x8)
#define BCM54194_I2C_ENABLE_ALL_PORT      (0xF)
#define BCM54194_I2C_SPD_400KBPS          (0x1 << 1)
#define BCM54194_I2C_CMD_MASK             (0x7 << 2)
#define BCM54194_I2C_NO_OP_CMD            (0x0)
#define BCM54194_I2C_READ_CURR_CMD        (0x1 << 2)
#define BCM54194_I2C_WRITE_CMD            (0x2 << 2)
#define BCM54194_I2C_READ_CMD             (0x3 << 2)
#define BCM54194_I2C_FLUSH_CMD            (0x4 << 2)
#define BCM54194_I2C_CMD_DONE_BIT         (0x1 << 9)

#define BCM54194_FORCE_AUTO_MDIX_BIT      (0x1 << 9)

#define BCM54194_PATTERN_GEN_CTRL_REG               0x5
#define BCM54194_TIME_SYNC_REG                      0xF5
#define BCM54194_TOP_MISC_TOP_GBL_RST_REG           0x82B
#define BCM54194_P1588_MPLS_LABEL1_MASK_MSB_REG     0xA98
#define BCM54194_P1588_MPLS_LABEL2_MASK_MSB_REG     0xA9C
#define BCM54194_P1588_MPLS_LABEL3_MASK_MSB_REG     0xAA0
#define BCM54194_P1588_MPLS_LABEL4_MASK_MSB_REG     0xAA4
#define BCM54194_P1588_MPLS_LABEL5_MASK_MSB_REG     0xAA8
#define BCM54194_P1588_MPLS_LABEL6_MASK_MSB_REG     0xAAC
#define BCM54194_P1588_MPLS_LABEL7_MASK_MSB_REG     0xAB0
#define BCM54194_P1588_MPLS_LABEL8_MASK_MSB_REG     0xAB4
#define BCM54194_P1588_SLICE_EN_CTRL_REG            0xA10
#define BCM54194_P1588_SOP_SELECTION_REG            0xAF8
#define BCM54194_P1588_TIMECODE_SEL_REG             0xAC3
#define BCM54194_P1588_DPLL_DEBUG_SELECT_REG        0xA5B
#define BCM54194_P1588_PCH_TS_FIFO_RD_START_END_REG 0xA85
#define BCM54194_P1588_PCH_TS_FIFO_0_REG            0xA89
#define BCM54194_P1588_PCH_TS_FIFO_1_REG            0xA8A
#define BCM54194_P1588_PCH_TAGID_REG                0xA8B
#define BCM54194_P1588_PCH_CRC8_MISMATCH_REG        0xAC4
#define BCM54194_P1588_PCH_TS_INFO_1_REG            0xA8C
#define BCM54194_P1588_PCH_TS_INFO_2_REG            0xA8D
#define BCM54194_P1588_PCH_TS_INFO_3_REG            0xAF9
#define BCM54194_P1588_PCH_TS_INFO_4_REG            0xAFA
#define BCM54194_P1588_PCH_TS_INFO_5_REG            0xAFB
#define BCM54194_P1588_PCH_TS_INFO_6_REG            0xAFC
#define BCM54194_P1588_PCH_TS_INFO_7_REG            0xAFD
#define BCM54194_P1588_PCH_TS_INFO_8_REG            0xAFE
#define BCM54194_P1588_CTRL_DEBUG_REG               0xA8E
#define BCM54194_P1588_INBAND_CTRL_PORT_REG         0xAEE
#define BCM54194_P1588_TX_SOP_TS_CAP_EN_REG         0xA21
#define BCM54194_P1588_RX_SOP_TS_CAP_EN_REG         0xA22

#define BCM54194_EXTERNAL_SERDES_CTRL_REG           0x234
#define BCM54194_SGMII_SLAVE_REG                    0x235

#define BCM54194_I2C_MASTER_CTRL_REG                0x885
#define BCM54194_I2C_MASTER_DEV_ADDR_REG            0x886
#define BCM54194_I2C_MASTER_REG_ADDR_REG            0x887
#define BCM54194_I2C_MASTER_WDAT_REG                0x888
#define BCM54194_I2C_MASTER_RDAT_REG                0x889
#define BCM54194_I2C_MASTER_STS_REG                 0x88A

#define BCM54194_COPPER_MISCEL_CTRL_REG             0x2F

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

#define BCM54194_SGMII_SLAVE_MODE_EN_BIT    (0x1 << 1)
#define BCM54194_AUTO_DET_MEDIUM_EN_BIT      0x1
#define BCM54194_MODE_SEL_MASK              (0x3 << 1)
#define BCM54194_SGMII_TO_COPPER_MODE       (0x0 << 1)
#define BCM54194_SGMII_TO_FIBER_MODE        (0x1 << 1)
#define BCM54194_100BASE_FX_MODE_EN          0x21
#define BCM54194_SERFES_PORT_SEL_MASK       (0x7 << 12)
#define BCM54194_SERFES_PORT0_FIBER         (0x4 << 12)
#define BCM54194_SERFES_PORT1_FIBER         (0x5 << 12)
#define BCM54194_SERDES_RISE_FALL_MASK      (0x3 << 14)
#define BCM54194_SERDES_RISE_FALL_142NS     (0x2 << 14)
#define BCM54194_SD_THRESHOLD_MASK           0xF
#define BCM54194_SD_THRESHOLD_100mV          0xD

#define BCM54194_AUTO_DETECT_MEDIUM_REG 0x23E
#define BCM54194_MODE_CTRL_REG          0x021
#define BCM54194_SERDES_100FX_CTRL_REG  0x233
#define BCM54194_SGMII_LN_CTRL_1G_REG   0x902
#define BCM54194_SGMII_TX_ACTRL_2_REG   0x923
#define BCM54194_SGMII_RX_ACTRL_5_REG   0x945

/* PRBS */
# define  BCM54194_PRBS_LOCKED         0x800
# define  BCM54194_PRBS_LOST_LOCK      0x1000
# define  BCM54194_TEST_PRBS_ERR_CNTR  0x20
# define  BCM54194_CLR_PRBS_ERR_CNTR   0x10
# define  BCM54194_PRBS_7              0x0
# define  BCM54194_PRBS_15             0x4
# define  BCM54194_PRBS_23             0x8
# define  BCM54194_PRBS_INVERT         0x2
# define  BCM54194_PRBS_ENABLE         0x1


//TODO??????
/* ioctl cmd for suspend/resume update GE phy link status */
#define SIOCNEPSUSPENDUPLINK  0x89F1
#define SIOCNEPRESUMEUPLINK   0x89F2

#define  SPD_10MBPS       10
#define  SPD_100MBPS      100
#define  SPD_1000MBPS     1000
#define  SPD_10000MBPS    10000

#define  HALF_DUPLEX      0
#define  FULL_DUPLEX      1

#define  AUTONEG_OFF      0
#define  AUTONEG_ON       1

#define  PRBS_PATTERN_7   7
#define  PRBS_PATTERN_15  15
#define  PRBS_PATTERN_23  23

typedef enum {
    MANHATTAN_BCM54194_LPBK_INT = 1,
    MANHATTAN_BCM54194_LPBK_EXT    ,
    MANHATTAN_BCM54194_LPBK_SFP_EXT,
    MANHATTAN_BCM54194_LPBK_SGMII
} manhattan_bcm54194_lpbk_t;

typedef struct manhattan_bcm54194 {
    #define MANHATTAN_BCM54194_INIT_STAGE_NOT_INIT    0
    #define MANHATTAN_BCM54194_INIT_STAGE_DONE_INIT   1
    int   init_stage;
    void *priv;
    char eth_map[MANHATTAN_BCM54194_PORT_NUMB][IFNAMSIZ];
    int (*mdio_rd)(void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data);
    int (*mdio_wr)(void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int  data);
    int (*intr   )(void *ctx, int act, void *arg); /* Intr check outside */
    int (*reset  )(void *ctx, int rst);
} manhattan_bcm54194_t;

int manhattan_bcm54194_init(void *priv,
    int (*rd)  (void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data),
    int (*wr)  (void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int  data),
    int (*intr)(void *ctx, int act, void *arg),
    int (*rst) (void *ctx, int rst),
    char eth_map[MANHATTAN_BCM54194_PORT_NUMB][IFNAMSIZ]);

int manhattan_bcm54194_exit(void *priv);
int manhattan_front_port_to_54194_port_map_show(void);

int manhattan_bcm54194_rdb_access_enable (int phy_addr);
int manhattan_bcm54194_rdb_access_disable (int phy_addr);
int manhattan_bcm54194_rdb_read (int phy_addr, int rdb_offset, uint16_t *reg_val);
int manhattan_bcm54194_rdb_write (int phy_addr, int rdb_offset, uint16_t reg_val);
int manhattan_bcm54194_switch_intf_access (manhattan_bcm54194_intf_t intf);
int manhattan_bcm54194_reg_1000x_en (int phy_addr, int enable);
int manhattan_bcm54194_per_port_reset (int phy_addr, int intf);
int manhattan_bcm54194_soft_reset (void);
int manhattan_bcm54194_init_script (void);
int manhattan_bcm54194_reset (int print_msg);
int manhattan_bcm54194_sgmii_slave_mode (int phy_addr, int enable);
int manhattan_bcm54194_loopback_config_dump (int phy_addr, int loopback_mode);
int manhattan_bcm54194_cfg_setting (int phy_addr, int speed, int auto_neg, int duplex, manhattan_bcm54194_intf_t intf);
int manhattan_bcm54194_config_loopback (int phy_addr, int speed, int loopback_mode, int enable);
int manhattan_bcm54194_is_linkup (int phy_addr, manhattan_bcm54194_intf_t intf);
int manhattan_bcm54194_sig_pwr_ctrl(int phy_addr, int enable, manhattan_bcm54194_intf_t intf);
int manhattan_bcm54194_switch_to_fiber(int phy_addr, int onoff);
int manhattan_bcm54194_mdio45_reg_rd (int phy_addr, int dev, int reg, ushort *data);
int manhattan_bcm54194_mdio45_reg_wr (int phy_addr, int dev, int reg, ushort data);
int manhattan_bcm54194_check_link (void);
int manhattan_bcm54194_link_status (void);
int manhattan_bcm54194_line_side_config (int port);
int manhattan_bcm54194_config_prbs (int port, int action, int invert, int pattern);
int manhattan_bcm54194_packet_counter_util (int port, int action);
int manhattan_bcm54194_config_interrupt (int port, int enable);
int manhattan_bcm54194_interrupt_generate (int phy_addr, int enable, int ext_lpbk);
int manhattan_bcm54194_interrupt_set(int phy_addr, int action);
int manhattan_bcm54194_interrupt_clear (int phy_addr);
int manhattan_bcm54194_interrupt_get (int phy_addr, uint16_t *int_status);
int bcm54194_interrupt_util (void);
int bcm54194_interrupt_test(int phy_port, uint16_t *int_status);
int bcm54194_i2c_slave_write (int port, int slave_addr, int offset, ushort wrval);
int bcm54194_i2c_slave_read (int port, int slave_addr, int offset, ushort *buf, int len);
int bcm54194_transmit_test_pattern(int phy_addr, int mode);

#endif

static inline char *__binary_dump_16(uint16_t val, char *buf)
{
    static char _buf[32] = {0,};
    int   i = 0;
    int   j = 0;
    char *p = buf ? &buf[0] : &_buf[0];

    for(i = 15, j = 0; i >= 0; i--, j++) {
        p[j] = '0' + !!(val & (1 << i));
        if ((i & 0x3) == 0 && i > 0) { /* i % 4 == 0 */
            p[++j] = ' ';
        }
    }
    p[j] = 0;
    return p;
}

int manhattan_bcm54194_reg_verbose(int set);
#define MANHATTAN_BCM54194_REG_VERBOSE manhattan_bcm54194_reg_verbose(-1)
#define MHT_MDIO_RD(PHYAD___, REGAD___, PVAL___)                                                    \
({                                                                                                  \
    static int rc___ = 0;                                                                           \
    static unsigned int VAL___ = 0;                                                                 \
    rc___ = _g_p_seahawks->mdio_rd(_g_p_seahawks->priv, PHYAD___, REGAD___, &VAL___);               \
    if (rc___ != 0) {                                                                               \
        cterr('f', 0, "Failed to read  BCM54194, phy_addr:%#x, reg:%#x; rc:%d\n",                   \
            PHYAD___, REGAD___, rc___);                                                             \
    } else                                                                                          \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                           \
        printf("%-40s RD phyad-0x%02x reg-0x%04x : 0x%04x (%s)\n",                                  \
            __func__, PHYAD___, REGAD___, VAL___, __binary_dump_16(VAL___, NULL));                  \
    }                                                                                               \
    *(PVAL___) = VAL___ & 0xFFFF;                                                                   \
    rc___;                                                                                          \
})

#define MHT_MDIO_WR(PHYAD___, REGAD___, VAL___)                                                     \
({                                                                                                  \
    static int rc___ = 0;                                                                           \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                           \
        printf("%-40s WR phyad-0x%02x reg-0x%04x : 0x%04x (%s)\n",                                  \
            __func__, PHYAD___, REGAD___, VAL___, __binary_dump_16(VAL___, NULL));                  \
    }                                                                                               \
    rc___ = _g_p_seahawks->mdio_wr(_g_p_seahawks->priv, PHYAD___, REGAD___, (VAL___) & 0xffff);     \
    if (rc___ != 0) {                                                                               \
        cterr('f', 0, "Failed to write BCM54194, phy_addr:%#x, reg:%#x, val:%#x; rc:%d\n",          \
            PHYAD___, REGAD___, VAL___, rc___);                                                     \
    }                                                                                               \
    rc___;                                                                                          \
})

#define MHT_MDIO_RD45(PHYAD___, DEVAD___, REGAD___, PVAL___)                                 \
({                                                                                           \
    static int rc___ = 0;                                                                    \
    static ushort VAL___ = 0;                                                                \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                    \
        printf("45 RD >>>>{\n");                                                             \
    }                                                                                        \
    rc___ = manhattan_bcm54194_mdio45_reg_rd(PHYAD___, DEVAD___, REGAD___, &VAL___);         \
    if (rc___ != 0) {                                                                        \
        cterr('f', 0, "Failed to read BCM54194, phy_addr:%#x, dev:%#x, reg:%#x; rc:%d\n",    \
            PHYAD___, DEVAD___, REGAD___, rc___);                                            \
    } else                                                                                   \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                    \
        printf("}<<<<\n");                                                                   \
        printf("%-40s RD45 phyad-0x%02x devad-0x%04x reg-0x%04x : 0x%04x (%s)\n",            \
            __func__, PHYAD___, DEVAD___, REGAD___, VAL___, __binary_dump_16(VAL___, NULL)); \
    }                                                                                        \
    *(PVAL___) = VAL___ & 0xFFFF;                                                            \
    rc___;                                                                                   \
})

#define MHT_MDIO_WR45(PHYAD___, DEVAD___, REGAD___,  VAL___)                                    \
({                                                                                              \
    static int rc___ = 0;                                                                       \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                       \
        printf("%-40s WR45 phyad-0x%02x devad-0x%04x reg-0x%04x : 0x%04x (%s)\n",               \
            __func__, PHYAD___, DEVAD___, REGAD___, VAL___, __binary_dump_16(VAL___, NULL));    \
        printf("45 WR >>>>{\n");                                                                \
    }                                                                                           \
    rc___ = manhattan_bcm54194_mdio45_reg_wr(PHYAD___, DEVAD___, REGAD___,  (VAL___) & 0xffff); \
    if (rc___ != 0) {                                                                           \
        cterr('f', 0, "Failed to write BCM54194, phy_addr:%#x, dev:%#x, reg:%#x; rc:%d\n",      \
            PHYAD___, DEVAD___, REGAD___, rc___);                                               \
    }                                                                                           \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                       \
        printf("}<<<<\n");                                                                      \
    }                                                                                           \
    rc___;                                                                                      \
})

#define MHT_RDB_RD(PHYAD___, RDBOFF___, PVAL___)                                      \
({                                                                                    \
    static int rc___ = 0;                                                             \
    static uint16_t VAL___;                                                           \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                             \
        printf("RDB RD >>>>{\n");                                                     \
    }                                                                                 \
    rc___ = manhattan_bcm54194_rdb_read(PHYAD___, RDBOFF___, &VAL___);                \
    if (rc___ != 0) {                                                                 \
        cterr('f', 0, "BCM54194 RDB read failed, phy_addr:%#x, rdb_off:%#x; rc:%d\n", \
                PHYAD___, RDBOFF___, rc___);                                          \
    } else                                                                            \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                             \
        printf("}<<<<\n");                                                            \
        printf("%-40s RD phyad-0x%02x RDB-0x%04x : 0x%04x (%s)\n",                    \
            __func__, PHYAD___, RDBOFF___, VAL___, __binary_dump_16(VAL___, NULL));   \
    }                                                                                 \
    *(PVAL___) = VAL___;                                                              \
    rc___;                                                                            \
})

#define MHT_RDB_WR(PHYAD___, RDBOFF___, VAL___)                                                 \
({                                                                                              \
    static int rc___ = 0;                                                                       \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                       \
        printf("%-40s WR phyad-0x%02x RDB-0x%04x : 0x%04x (%s)\n",                              \
            __func__, PHYAD___, RDBOFF___, VAL___, __binary_dump_16(VAL___, NULL));             \
        printf("RDB WR >>>>{\n");                                                               \
    }                                                                                           \
    rc___ = manhattan_bcm54194_rdb_write(PHYAD___, RDBOFF___, (VAL___) & 0xFFFF);               \
    if (rc___ != 0) {                                                                           \
        cterr('f', 0, "BCM54194 RDB write failed, phy_addr:%#x, rdb_off:%#x, val:%#x; rc:%d\n", \
                PHYAD___, RDBOFF___, VAL___, rc___);                                            \
    }                                                                                           \
    if (MANHATTAN_BCM54194_REG_VERBOSE) {                                                       \
        printf("}<<<<\n");                                                                      \
    }                                                                                           \
    rc___;                                                                                      \
})
