/* $Id:
 * $Source:
 *------------------------------------------------------------------
 *
 * switzer_manhattan.h - Switzer-Manhattan interfaces.
 *
 * Feb. 2020, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_MANHATTAN_H__
#define __SWITZER_MANHATTAN_H__
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bits/socket.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>

#include "linux_pci.h"
#include "switzer_fpga.h"
#include "switzer_miura.h"
#include "switzer_priv.h"
#include "switzer_manhattan_x710_gpio.h"

#define SWITZER_MANHATTAN_I2C_ADDR_PCA1   0x1C
#define SWITZER_MANHATTAN_I2C_ADDR_PCA2   0x1D
#define SWITZER_MANHATTAN_I2C_ADDR_MUX    0x72
#define SWITZER_MANHATTAN_I2C_ADDR_PM1    0x30
#define SWITZER_MANHATTAN_I2C_ADDR_PM2    0x10
#define SWITZER_MANHATTAN_I2C_ADDR_POE1   0x20
#define SWITZER_MANHATTAN_I2C_ADDR_POE2   0x21
#define SWITZER_MANHATTAN_I2C_ADDR_LM75   0x4D
#define SWITZER_MANHATTAN_I2C_ADDR_SFP    0x50
#define SWITZER_MANHATTAN_I2C_ADDR_SFP_MEASURES    0x51

#define SWITZER_MANHATTAN_PCA9557_IN_REG                0
#define SWITZER_MANHATTAN_PCA9557_OUT_REG               1
#define SWITZER_MANHATTAN_PCA9557_POL_REG               2
#define SWITZER_MANHATTAN_PCA9557_CTL_REG               3

/* direction: 1-out 0-in; '_P' means port, '_D' means direction */
#define SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_P             3
#define SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_D             0
#define SWITZER_MANHATTAN_PCA2_PSE_RST_P                0
#define SWITZER_MANHATTAN_PCA2_PSE_RST_D                1
#define SWITZER_MANHATTAN_PCA2_PSE_OSS_P                1
#define SWITZER_MANHATTAN_PCA2_PSE_OSS_D                1
#define SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P      2
#define SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_D      1
#define SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P      3
#define SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_D      1
#define SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P      4
#define SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_D      1
#define SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P      5
#define SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_D      1

#define SWITZER_MANHATTAN_PCI_FUNC_PORT1  0x00
#define SWITZER_MANHATTAN_PCI_FUNC_PORT2  0x01
#define SWITZER_MANHATTAN_PCI_FUNC_PORT3  0x02
#define SWITZER_MANHATTAN_PCI_FUNC_PORT4  0x03

#define SWITZER_MAX_PORTS_OF_EACH_PAIR    2

/* to read/write i40e reg */
#define SIOCDEVREG  (SIOCDEVPRIVATE + 2)
struct i40e_ioctl_reg {
    uint8_t op;
    uint32_t off;
    uint32_t data;
};

typedef enum {
    SWITZER_MANHATTAN_2M,
    SWITZER_MANHATTAN_4T,
    SWITZER_MANHATTAN_1M,
    SWITZER_MANHATTAN_2T,
} switzer_manhattan_type_t;

typedef enum {
    SWITZER_LANE_0,
    SWITZER_LANE_1,
    SWITZER_LANE_2,
    SWITZER_LANE_3,
    MAX_NR_SWITZER_LANE
} switzer_lane_t;

struct switzer_manhattan_host_fpga {
    struct switzer_dash_i2c_master *i2c;
};

struct switzer_manhattan_poe {
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_manhattan_pm {
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_manhattan_i2c_mux {
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_manhattan_lm75 {
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_manhattan_x710_acc {
    #define SWITZER_MANHATTAN_PORT_NUM  4
    #define SWITZER_MANHATTAN_IPORT_NUM  2
    #define SWITZER_MANHATTAN_EPORT_NUM  2
    char ifname[SWITZER_MANHATTAN_PORT_NUM][IFNAMSIZ];
};

struct switzer_manhattan_phy_acc {
    char ifname[IFNAMSIZ];
    #define I40E_AQ_PHY_REG_ACCESS_INTERNAL         0
    #define I40E_AQ_PHY_REG_ACCESS_EXTERNAL         1
    #define I40E_AQ_PHY_REG_ACCESS_EXTERNAL_MODULE  2
    int  phy_id;
};

struct switzer_manhattan_eth_port {
    char intnl_port[SWITZER_MANHATTAN_IPORT_NUM][IFNAMSIZ];
    char extnl_port[SWITZER_MANHATTAN_EPORT_NUM][IFNAMSIZ];
};

enum {
    SWITZER_MANHATTAN_STAGE_INIT_0 = 0,
    SWITZER_MANHATTAN_STAGE_INIT_DONE ,
    SWITZER_MANHATTAN_STAGE_MAX
};

struct switzer_manhattan {
    int stage;
    int chk_4t; /* flag of doing actual checking if it is 4t, maybe removed after final decision of the ext phy */
    int mdio_dbg; /* flag to print mdio rd/wr params for debug */
    switzer_manhattan_type_t manhattan_type;
    struct switzer_manhattan_poe poe[2];
    struct switzer_manhattan_pm pm[2];
    struct switzer_manhattan_i2c_mux i2c_mux;
    struct switzer_manhattan_lm75 lm75;
    struct switzer_miura miura;
    /* platform */
    void   *seahawks; /* (struct switzer_manhattan_bcm54194 *) */
    struct ngio_intf_t *ngio;
    struct switzer_manhattan_host_fpga host_fpga;
    struct switzer_manhattan_x710_acc ctx_x710;
    struct switzer_manhattan_phy_acc  ctx_ephy;       /* For external phy */
    struct switzer_manhattan_phy_acc  ctx_iphy[SWITZER_MANHATTAN_IPORT_NUM]; /* For internal phy */
    struct switzer_manhattan_eth_port eth_port;
};


int switzer_manhattan_init(struct switzer_manhattan *mod,
                           struct switzer_settings *settings);
void switzer_manhattan_exit(struct switzer_manhattan *mod);

int switzer_manhattan_stage_get(struct switzer_manhattan *mod);
int switzer_manhattan_stage_set(struct switzer_manhattan *mod, int stage);

int is_switzer_manhattan_2m(struct switzer_manhattan *mod);
int is_switzer_manhattan_4t(struct switzer_manhattan *mod);
int is_switzer_manhattan_1m(struct switzer_manhattan *mod);
int is_switzer_manhattan_2t(struct switzer_manhattan *mod);

ssize_t switzer_manhattan_i2c_read(struct switzer_i2c_slave *slave,
                                   uint8_t cmd, void *buf, size_t count);
ssize_t switzer_manhattan_i2c_write(struct switzer_i2c_slave *slave,
                                    uint8_t cmd, const void *buf, size_t count);

int switzer_manhattan_ephy_read(struct switzer_manhattan *mod, switzer_lane_t lane,
                               switzer_if_side_t if_side, uint32_t devaddr,
                               uint32_t regaddr, uint32_t *data);
int switzer_manhattan_ephy_write(struct switzer_manhattan *mod, switzer_lane_t lane,
                                switzer_if_side_t if_side, uint32_t devaddr,
                                uint32_t regaddr, uint32_t data);

ssize_t switzer_manhattan_sfp_read(struct switzer_manhattan *mod,
                                   switzer_lane_t lane, uint8_t addr,
                                   uint8_t cmd, void *buf, size_t count);
ssize_t switzer_manhattan_sfp_write(struct switzer_manhattan *mod,
                                    switzer_lane_t lane, uint8_t addr,
                                    uint8_t cmd, const void *buf, size_t count);

int switzer_manhattan_iphy_read(struct switzer_manhattan *mod, uint32_t devaddr,
                               uint32_t regaddr, uint32_t *data);
int switzer_manhattan_iphy_write(struct switzer_manhattan *mod, uint32_t devaddr,
                                uint32_t regaddr, uint32_t data);

int switzer_manhattan_x710_read (struct switzer_manhattan *mod, int port, uint32_t off, uint32_t *data);
int switzer_manhattan_x710_write(struct switzer_manhattan *mod, int port, uint32_t off, uint32_t data);
int switzer_manhattan_x710_gpio_set(struct switzer_manhattan *mod, int port, int gpio_idx, int bitv);
int switzer_manhattan_x710_gpio_get(struct switzer_manhattan *mod, int port, int gpio_idx, uint32_t *st_reg);
int switzer_manhattan_x710_gpio_ctrl_set(struct switzer_manhattan *mod, int port, int gpio_idx, uint32_t ctl_val);
int switzer_manhattan_x710_gpio_ctrl_get(struct switzer_manhattan *mod, int port, int gpio_idx, uint32_t *ctl_val);

int switzer_manhattan_ephy_autoneg_remote_ability_get(struct switzer_manhattan *mod, switzer_lane_t lane,
                                                     switzer_if_side_t if_side, unsigned short *fec_ability,
                                                     unsigned short *pause_ability, bcm_plp_an_config_t *an_config);
int switzer_manhattan_registers_dump(struct switzer_manhattan *mod,
                                     switzer_lane_t lane, switzer_if_side_t if_side);
int switzer_manhattan_ephy_dump(struct switzer_manhattan *mod,
                               switzer_lane_t lane, switzer_if_side_t if_side);
int switzer_manhattan_ephy_mac_dump(struct switzer_manhattan *mod,
                                   switzer_lane_t lane, switzer_if_side_t if_side);
int switzer_manhattan_ephy_link_status(struct switzer_manhattan *mod, switzer_lane_t lane,
                                      switzer_if_side_t if_side, unsigned int *link_status);
int switzer_manhattan_display_eye_scan(struct switzer_manhattan *mod,
                                       switzer_lane_t lane, switzer_if_side_t if_side);
int switzer_manhattan_miura_loopback_set(struct switzer_manhattan *mod,
                                         switzer_lane_t lane, switzer_if_side_t if_side,
                                         unsigned int lb_mode, unsigned int enable);

int switzer_manhattan_miura_firmware_lane_get(struct switzer_manhattan *mod,
                                              switzer_lane_t lane, switzer_if_side_t if_side,
                                              bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);
int switzer_manhattan_miura_firmware_lane_set(struct switzer_manhattan *mod,
                                              switzer_lane_t lane, switzer_if_side_t if_side,
                                              bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);

int switzer_manhattan_miura_cl73_set(struct switzer_manhattan *mod, switzer_lane_t lane,
                                     switzer_if_side_t if_side, unsigned int enable);

void switzer_manhattan_miura_config_macsec_cleanup(struct switzer_manhattan *mod);
int switzer_manhattan_miura_config_macsec_bypass(struct switzer_manhattan *mod, switzer_lane_t lane, int port_speed);
int switzer_manhattan_miura_config_interface_macsec_bypass(struct switzer_manhattan *mod,
                                                           switzer_lane_t lane,
                                                           bcm_pm_interface_t sys_inf,
                                                           bcm_pm_interface_t line_inf,
                                                           int port_speed);
int switzer_manhattan_ephy_config_tx_disable(struct switzer_manhattan *mod,
                                            switzer_lane_t lane, int en);

int switzer_manhattan_miura_tx_get(struct switzer_manhattan *mod, switzer_lane_t lane,
                                   switzer_if_side_t if_side, bcm_plp_tx_t *tx_param);
int switzer_manhattan_miura_tx_set(struct switzer_manhattan *mod, switzer_lane_t lane,
                                   switzer_if_side_t if_side, bcm_plp_tx_t *tx_param);

int switzer_manhattan_miura_prbs_set(struct switzer_manhattan *mod,
                                     switzer_lane_t lane, switzer_if_side_t if_side,
                                     switzer_prbs_t prbs, unsigned int enable);
int switzer_manhattan_prbs_clear_rx_stat(struct switzer_manhattan *mod,
                                         switzer_lane_t lane, switzer_if_side_t if_side);
int switzer_manhattan_miura_prbs_check(struct switzer_manhattan *mod,
                                       switzer_lane_t lane, switzer_if_side_t if_side);

#include "switzer_manhattan_bcm54194_api.h"

#endif /* __SWITZER_MANHATTAN_H__ */
