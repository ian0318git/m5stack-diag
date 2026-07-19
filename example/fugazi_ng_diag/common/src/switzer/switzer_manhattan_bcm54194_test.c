#include <error.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "types.h"
#include "common.h"
#include "common_utils.h"
#include "menu.h"
#include "queryflags.h"
#include "nvmonvars.h"
#include "switzer_common.h"
#include "switzer_manhattan_bcm54194_api.h"
#include "switzer_manhattan_bcm54194_test.h"
#include "switzer_manhattan_bcm54194_reg_detail.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

extern int switzer_manhattan_sock_test(char *eth_port_s, char *eth_port_r);
extern manhattan_bcm54194_t g_seahawks;
static manhattan_bcm54194_t *_g_p_seahawks = &g_seahawks;

static int BCM541xx_utility (int show_menu);
static int BCM541xx_register_test (int);
static int bcm54194_intr_test(int);
static int bcm54194_lpbk_test(int);
static int bcm54194_test_mode_util(int arg);
static int bcm54194_sgmii_slave_mode_util(int arg);
static int manhattan_bcm54194_mdio22_rdwr_util(int);
static int manhattan_bcm54194_mdio45_rdwr_util(int);
static int manhattan_bcm54194_rdb_rdwr_util(int);
static int manhattan_bcm54194_ieee_reg_dump_util(int);
static int manhattan_bcm54194_per_port_reg_dump_util(int);
static int manhattan_bcm54194_global_reg_dump_util(int);
static int manhattan_bcm54194_clause45_reg_dump_util(int);
static int bcm54194_copper_lpbk_config_util(int arg);
//static int bcm54194_copper_lpbk_extnl_config(int arg);
static int bcm54194_fiber_lpbk_extnl_config_util(int arg);
static int bcm54194_sgmii_lpbk_config(int arg);
static int bcm54194_lpbk_extnl_cfg_dump(int arg);
static int bcm54194_lpbk_intnl_cfg_dump(int arg);
static int bcm54194_link_status(int arg);
static int bcm54194_pkt_cntr_util(int arg);
static int bcm54194_port_detail_status(int arg);
static int bcm54194_sfp_status(int arg);
       int bcm54194_sfp_reg_rdwr(int rdwr);
       int bcm54194_sfp_reg_dump(int arg);
static int bcm54194_reg_parse_dump(int phy_addr, struct __reg_parser *rp);


static const reg_info_t bcm54194_ieee_sgmii_reg[] = {
    {"SGMII Control",                          0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"SGMII Status",                           0x01, READ_ONLY,  {2}, 0x0000, 0x0149},
    {"SGMII AN Advertisement",                 0x04, READ_WRITE, {2}, 0x9E01, 0x0801},
    {"SGMII AN Link Partner Ability",          0x05, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm54194_ieee_copper_reg[] = {
    {"Copper MII Control",                     0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"Copper MII Status",                      0x01, READ_ONLY,  {2}, 0x0000, 0x79C9},
    {"PHY ID1 MSB",                            0x02, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PHY ID2 LSB",                            0x03, READ_ONLY,  {2}, 0x0000, 0x5018},
    {"Copper AN Advertisement",                0x04, READ_WRITE, {2}, 0xBFFF, 0x01E1},
    {"Copper AN Link Partner Ability",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper AN Expansion",                    0x06, READ_ONLY,  {2}, 0x0000, 0x0064},
    {"Copper Next Page Transmit",              0x07, READ_WRITE, {2}, 0xB7FF, 0x2000},
    {"Copper Link Partner Received Next Page", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",                         0x09, READ_WRITE, {2}, 0xFF00, 0x0F00},
    {"1000BT Status",                          0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"IEEE Extended Status",                   0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm54194_ieee_fiber_reg[] = {
    {"Fiber Control",                          0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"1000BX Status",                          0x01, READ_ONLY,  {2}, 0x0000, 0x0140},
    {"1000BX AN Advertisement",                0x04, READ_WRITE, {2}, 0x0000, 0x0000},
    {"1000BX AN Link Partner Ability",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const bcm54194_regs_t bcm54194_phy_ieee_reg_tbl[] = {
    {"Copper", MANHATTAN_BCM54194_INTF_COPPER, bcm54194_ieee_copper_reg},
    {"SGMII" , MANHATTAN_BCM54194_INTF_SGMII , bcm54194_ieee_sgmii_reg },
    {"Fiber" , MANHATTAN_BCM54194_INTF_FIBER , bcm54194_ieee_fiber_reg },
};

struct bcm54194_clause45_reg_s {
    char *name;
    uint16_t dev;
    uint16_t reg;
};

static struct bcm54194_clause45_reg_s bcm54194_clause45_regs[] = {
    {"IEEE_PMA_PMD_CONTROL_1         ", 0x1, 0x0   },
    {"IEEE_PMA_PMD_STATUS_1          ", 0x1, 0x01  },
    {"PCS_STATUS_1                   ", 0x3, 0x1   },
    {"EEE_CAPABILITY                 ", 0x3, 0x14  },
    {"EEE_WAKE_ERR_CNT               ", 0x3, 0x16  },
    {"EEE_ADVERTISEMENT              ", 0x7, 0x3C  },
    {"EEE_LINK_PARTNER_ADVERTISEMENT ", 0x7, 0x3D  },
    {"EEE_TEST_CONTROL               ", 0x7, 0x803D},
    {"EEE_RESOLUTION_STATUS          ", 0x7, 0x803E},
    {"EEE_LPI_COUNTER                ", 0x7, 0x803F},
    {NULL                             , 0  , 0     }
};

static struct bcm54194_per_port_regs_s {
    char         *name    ;
    uint16_t      rdb     ;
#define BCM54194_REG_DUMP_ATTR_PMSK         0xf
#define BCM54194_REG_DUMP_ATTR_PSHFT        0
#define BCM54194_REG_DUMP_ATTR_RDCLR_MSK    0x10
#define BCM54194_REG_DUMP_ATTR_RDCLR_SHFT   4
#define BCM54194_REG_DUMP_ATTR_WOCLR_MSK    0x20
#define BCM54194_REG_DUMP_ATTR_WOCLR_SHFT   5
#define BCM54194_REG_DUMP_ATTR_WZCLR_MSK    0x40
#define BCM54194_REG_DUMP_ATTR_WZCLR_SHFT   6
#define SWITZER_MANHATTAN_I2C_ADDR_SFP      0x50
#define SFP_REG_VENDOR_NAME_OFFSET          0x14
#define SFP_REG_VENDOR_NAME_LEN             16

    unsigned int  attr    ;
} bcm54194_per_port_regs[] = {
    {"PHY_EXTENDED_CTRL Register                        ", 0x000, 3}, /* 3 is 54194 port 0 and 54194 port 1 */
    {"COPPER_PHY_EXTENDED_STATUS Register               ", 0x001, 3},
    {"PATTERN_GENERATOR_CONTROL Register                ", 0x005, 3},
    {"PATTERN_GENERATOR_STATUS Register                 ", 0x006, 3},
    {"COPPER_AUXILIARY_STATUS_SUMMARY Register          ", 0x009, 3},
    {"COPPER_INTERRUPT_STATUS Register                  ", 0x00A, (3 | (1 << BCM54194_REG_DUMP_ATTR_RDCLR_SHFT))},
    {"COPPER_INTERRUPT_MASK Register                    ", 0x00B, 3},
    {"COPPER_HCD_STATUS Register                        ", 0x00C, 3},
    {"1000BASET_MASTER_SLAVE_SEED Register              ", 0x00D, 3},
    {"TEST_1 Register                                   ", 0x00E, 3},
    {"TEST_2 Register                                   ", 0x00F, 3},
    {"SPARE_CONTROL_1 Register                          ", 0x012, 3},
    {"SPARE_CONTROL_2 Register                          ", 0x014, 3},
    {"SPARE_CONTROL_3 Register                          ", 0x015, 3},
    {"COPPER_LED_STATUS Register                        ", 0x018, 3},
    {"COPPER_LED_CONTROL Register                       ", 0x019, 3},
    {"COPPER_AUTO_POWER_DOWN Register                   ", 0x01A, 3},
    {"COPPER_LED_SELECTOR_1 Register                    ", 0x01D, 3},
    {"COPPER_LED_SELECTOR_2 Register                    ", 0x01E, 3},
    {"LED_GPIO_CONTROL_STATUS Register                  ", 0x01F, 3},
    {"MODE_CONTROL Register                             ", 0x021, 3},
    {"COPPER_AUXILIARY_CONTROL Register                 ", 0x028, 3},
    {"10BASE-T Register                                 ", 0x029, 3},
    {"COPPER_POWER/MII_CONTROL Register                 ", 0x02A, 3},
    {"COPPER_MISCELLANEOUS_TEST Register                ", 0x02C, 3},
    {"COPPER_MISCELLANEOUS_CONTROL Register             ", 0x02F, 3},
    {"RX_TX_PACKET_COUNTER Register                     ", 0x030, 3},
    {"EXPANSION_INTERRUPT_STATUS Register               ", 0x031, (3 | (1 << (1 << BCM54194_REG_DUMP_ATTR_RDCLR_SHFT)))},
    {"EXPANSION_INTERRUPT_MASK Register                 ", 0x032, 3},
    {"MULTICOLOR_LED_SELECTOR Register                  ", 0x034, 3},
    {"MULTICOLOR_LED_FLASH_RATE_CONTROL Register        ", 0x035, 3},
    {"MULTICOLOR_LED_PROGRAMMABLE_BLINK_CONTROL Register", 0x036, 3},
    {"PORT_INTERRUPT_STATUS Register                    ", 0x03B, 3},
    {"SOFT_RESET Register                               ", 0x070, 3},
    {"LED_PROGRAMMABLE_CURRENT_MODE_CONTROL Register    ", 0x074, 3},
    {"RDB_ACCESS_MODE Register                          ", 0x087, 3},
    {"EEE_STATISTIC_TIMER_12HOURS_LPI Register          ", 0x0AA, 3},
    {"EEE_STATISTIC_TIMER_12HOURS_LOCAL Register        ", 0x0AB, 3},
    {"EEE_STATISTIC_LOC_LPI_REQ_0_TO_1_COUNTER Register ", 0x0AC, 3},
    {"EEE_STATISTIC_REM_LPI_REQ_0_TO_1_COUNTER Register ", 0x0AD, 3},
    {"EEE_STATISTIC_COUNTERS_CTRL_STATUS Register       ", 0x0AF, 3},
    {"TIME_SYNC Register                                ", 0x0F5, 3},
    {"EEE_LPI_TIMERS Register                           ", 0x152, 3},
    {"EEE_100TX_MODE_BW_CONTROL Register                ", 0x156, 3},
    {"PRBS_CONTROL Register                             ", 0x200, 3},
    {"PRBS_STATUS Register                              ", 0x201, 3},
    {"OPERATING_MODE_STATUS Register                    ", 0x202, 3},
    {"SGMII_LINESIDE_LOOPBACK_CONTROL Register          ", 0x204, 3},
    {"XGMII_AN_MODE_CNTRL_INFO Register                 ", 0x22D, 3},
    {"SERDES_100FX_STATUS Register                      ", 0x231, 3},
    {"SERDES_100FX_CONTROL Register                     ", 0x233, 3},
    {"EXTERNAL_SERDES_CONTROL Register (Port 0 Only)    ", 0x234, 1}, /* Only 54194 port 0 */
    {"SGMII_SLAVE Register                              ", 0x235, 3},
    {"MISC_1000X_CONTROL_1 Register                     ", 0x236, 3},
    {"MISC_1000X_CONTROL_2 Register                     ", 0x237, 3},
    {"AUTO_DETECT_SGMII_GBIC Register                   ", 0x238, 3},
    {"AUTONEG_1000_X_DEBUG Register                     ", 0x23A, 3},
    {"AUXILIARY_1000X_CONTROL Register                  ", 0x23B, 3},
    {"AUXILIARY_1000X_STATUS Register                   ", 0x23D, 3},
    {"AUTO_DETECT_MEDIUM Register                       ", 0x23E, 3},
    {"ECD_CONTROL_AND_STATUS Register                   ", 0x2A0, 3},
    {"ECD_FAULT_TYPE Register                           ", 0x2A1, 3},
    {"ECD_PAIR_A_LENGTH_RESULTS Register                ", 0x2A2, 3},
    {"ECD_PAIR_B_LENGTH_RESULTS Register                ", 0x2A3, 3},
    {"ECD_PAIR_C_LENGTH_RESULTS Register                ", 0x2A4, 3},
    {"ECD_PAIR_D_LENGTH_RESULTS Register                ", 0x2A5, 3},
    {NULL, 0, 0},
};

static struct bcm54194_global_reg_s {
    char         *name;
    uint16_t      rdb ;
    unsigned int  attr;
} bcm54194_global_regs[] = {
    {"MII_BUFFER_CONTROL_0 Register Port 0:          ", 0x800, 1},
    {"MII_BUFFER_CONTROL_0 Register Port 1:          ", 0x802, 1},
    {"MII_BUFFER_CONTROL_0 Register Port 2:          ", 0x804, 1},
    {"MII_BUFFER_CONTROL_0 Register Port 3:          ", 0x806, 1},
    {"TOP_LEVEL_CONFIGURATION Register               ", 0x810, 1},
    {"TOP_LEVEL_PIN_CONFIGURATION Register           ", 0x811, 1},
    {"LED_CONTROL_0 Register                         ", 0x820, 1},
    {"TOP_MISC_TOP_GLOBAL_RESET Register             ", 0x82B, 1},
    {"TOP_INTERRUPT_STATUS Register                  ", 0x82C, 1},
    {"TOP_INTERRUPT_MASK Register                    ", 0x82D, 1},
    {"TOP_MISC_MSPU_INTERRUPT Register               ", 0x82F, 1},
    {"VOLTAGE_TEMPERATURE_MONITOR_CONTROL Register   ", 0x831, 1},
    {"TEMPERATURE_MONITOR_VALUE Register             ", 0x832, 1},
    {"TEMPERATURE_MONITOR_HIGH_THRESHOLD Register    ", 0x833, 1},
    {"TEMPERATURE_MONITOR_LOW_THRESHOLD Register     ", 0x834, 1},
    {"VOLTAGE_MONITOR_1V_VALUE Register              ", 0x835, 1},
    {"VOLTAGE_MONITOR_1V_HIGH_THRESHOLD Register     ", 0x836, 1},
    {"VOLTAGE_MONITOR_1P0V_LOW_THRESHOLD Register    ", 0x837, 1},
    {"VOLTAGE_MONITOR_3P3V_VALUE Register            ", 0x838, 1},
    {"VOLTAGE_MONITOR_3P3V_HIGH_THRESHOLD Register   ", 0x839, 1},
    {"VOLTAGE_MONITOR_3P3V_LOW_THRESHOLD Register    ", 0x83A, 1},
    {"VOLTAGE_TEMPERATURE_MONITOR_INTERRUPT Register ", 0x83B, 1},
    {"SYNCE_RECOVERY_CLOCK Register                  ", 0x83C, 1},
    {"XTAL_CTRL Register                             ", 0x850, 1},
    {"LCPLL_CONFIGURATION Register                   ", 0x851, 1},
    {"LED_MATRIX_CONTROL_0 Register                  ", 0x85F, 1},
    {"TOP_MISC_MACSEC_CONFIG_2 Register Port 0:      ", 0x870, 1},
    {"TOP_MISC_MACSEC_CONFIG_3 Register Port 0:      ", 0x871, 1},
    {"I2C_MASTER_CONTROL Register                    ", 0x885, 1},
    {"I2C_MASTER_DEVICE_ADDRESS Register             ", 0x886, 1},
    {"I2C_MASTER_REGISTER_ADDRESS Register           ", 0x887, 1},
    {"I2C_MASTER_WRITE_DATA Register                 ", 0x888, 1},
    {"I2C_MASTER_READ_DATA Register                  ", 0x889, 1},
    {"I2C_MASTER_STATUS Register                     ", 0x88A, 1},
    {"I2C_MASTER_INTERRUPT_MASK Register             ", 0x88B, 1},
    {"TOP_MISC_SFP_STS0                              ", 0x890, 1},
    {"SGMII_LN_CONTROL Register                      ", 0x902, 1},
    {"TX_ANALOG_CONTROL_0 Register                   ", 0x921, 1},
    {"SGMII_TX_ACONTROL_1                            ", 0x922, 1},
    {"SGMII_TX_ACONTROL_2                            ", 0x923, 1},
    {"SGMII_RX_ACONTROL_5 Register                   ", 0x945, 1},
    {"RX_CONTROL_PCI Register                        ", 0x95A, 1},
    {NULL, 0, 0},
};


static struct __reg_parser sgmii_reg_detail_info[] = {
    REG_DETAIL__SGMII_CTRL,
    REG_DETAIL__SGMII_STATUS,
    REG_DETAIL__SGMII_AN_ADVERTISEMENT,
    REG_DETAIL__SGMII_AN_LINK_PARTNER_ABILITY,
    {-1, -1, "NULL",},
};

static struct __reg_parser copper_reg_detail_info[] = {
    REG_DETAIL__COPPER_MII_CONTROL,
    REG_DETAIL__COPPER_MII_STATUS,
    REG_DETAIL__COPPER_AN_ADVERTISEMENT,
    REG_DETAIL__COPPER_AN_LINK_PARTNER_ABILITY,
    REG_DETAIL__COPPER_AN_EXPANSION,
    REG_DETAIL__1000BASET_CONTROL,
    REG_DETAIL__1000BASE_T_STATUS,
    REG_DETAIL__IEEE_EXTENDED_STATUS,
    REG_DETAIL__COPPER_PHY_EXTENDED_STATUS,
    REG_DETAIL__COPPER_AUXILIARY_STATUS_SUMMARY,
    REG_DETAIL__MODE_CONTROL,
    REG_DETAIL__COPPER_AUXILIARY_CONTROL,
    {-1, -1, "NULL",},
};

static struct __reg_parser fiber_reg_detail_info[] = {
    REG_DETAIL__FIBER_CONTROL,
    REG_DETAIL__1000BASE_X_STATUS,
    REG_DETAIL__1000BASE_X_AN_ADVERTISEMENT,
    REG_DETAIL__1000BASE_X_AN_LINK_PARTNER_ABILITY,
    REG_DETAIL__MODE_CONTROL,
    {-1, -1, "NULL",},
};

static struct __reg_parser pkt_counter_reg_detail_info[] = {
    REG_DETAIL__COPPER_PHY_EXTENDED_STATUS,
    REG_DETAIL__COPPER_PHY_CRC_COUNTER,
    REG_DETAIL__TEST_1,
    REG_DETAIL__COPPER_MISCELLANEOUS_CONTROL,
    REG_DETAIL__RX_TX_PACKET_COUNTER,
    {-1, -1, "NULL",},
};

static struct __reg_parser sfp_stat_reg_detail_info[] = {
    REG_DETAIL__TOP_MISC_SFP_STS0,
    {-1, -1, "NULL",},
};

/******************************************************************************
 *  List of Menu used for SGMII BCM541xx
 *****************************************************************************/
static submenu_xtable_t BCM54194_tests_submenu_table[] = {
    {"BCM541xx Utility", (type_t(*)())BCM541xx_utility,   FALSE,
     0, NULL, 0, (type_t(*)())BCM541xx_utility,   TRUE},

    {"BCM541xx Register Test", (type_t(*)())BCM541xx_register_test,   0,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Interrupt Test", (type_t(*)())bcm54194_intr_test,   0,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Copper Port-0(Front RJ45 Port-1) Loopback Test", (type_t(*)())bcm54194_lpbk_test,  ((0 << 8) | MANHATTAN_BCM54194_INTF_COPPER),
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Copper Port-1(Front RJ45 Port-0) Loopback Test", (type_t(*)())bcm54194_lpbk_test,  ((1 << 8) | MANHATTAN_BCM54194_INTF_COPPER),
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Fiber  Port-0(Front SFP Port-1) Loopback Test", (type_t(*)())bcm54194_lpbk_test,  ((0 << 8) | MANHATTAN_BCM54194_INTF_FIBER),
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Fiber  Port-1(Front SFP Port-0) Loopback Test", (type_t(*)())bcm54194_lpbk_test,  ((1 << 8) | MANHATTAN_BCM54194_INTF_FIBER),
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},

};

/******************************************************************************
 *  List of Utilities used for SGMII BCM541xx
 *****************************************************************************/
static submenu_xtable_t BCM54194_util_items[] = {
    {"BCM541xx Reset", (type_t(*)())manhattan_bcm54194_reset, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Register Verbose Set", (type_t(*)())manhattan_bcm54194_reg_verbose, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Register Verbose Clear", (type_t(*)())manhattan_bcm54194_reg_verbose, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Register MDIO C22 Read", (type_t(*)())manhattan_bcm54194_mdio22_rdwr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BCM541xx Register MDIO C22 Write", (type_t(*)())manhattan_bcm54194_mdio22_rdwr_util, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Register MDIO C45 Read", (type_t(*)())manhattan_bcm54194_mdio45_rdwr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BCM541xx Register MDIO C45 Write", (type_t(*)())manhattan_bcm54194_mdio45_rdwr_util, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Register RDB Read", (type_t(*)())manhattan_bcm54194_rdb_rdwr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Register RDB Write", (type_t(*)())manhattan_bcm54194_rdb_rdwr_util, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx IEEE     Registers Dump", (type_t(*)())manhattan_bcm54194_ieee_reg_dump_util, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Per-port Registers Dump", (type_t(*)())manhattan_bcm54194_per_port_reg_dump_util, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Global   Registers Dump", (type_t(*)())manhattan_bcm54194_global_reg_dump_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Clause45 Registers Dump", (type_t(*)())manhattan_bcm54194_clause45_reg_dump_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx SGMII Loopback Config",          (type_t(*)())bcm54194_sgmii_lpbk_config, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Copper Internal Loopback Config", (type_t(*)())bcm54194_copper_lpbk_config_util, MANHATTAN_BCM54194_LPBK_INT, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Copper External Loopback Config", (type_t(*)())bcm54194_copper_lpbk_config_util, MANHATTAN_BCM54194_LPBK_EXT, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Fiber  External Loopback Config", (type_t(*)())bcm54194_fiber_lpbk_extnl_config_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Copper Internal Loopback Config Dump", (type_t(*)())bcm54194_lpbk_intnl_cfg_dump, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Copper External Loopback Config Dump", (type_t(*)())bcm54194_lpbk_extnl_cfg_dump, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Link Status", (type_t(*)())bcm54194_link_status, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Packet Counter Util", (type_t(*)())bcm54194_pkt_cntr_util, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Port Detail Status", (type_t(*)())bcm54194_port_detail_status, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx SFP Status", (type_t(*)())bcm54194_sfp_status, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx SFP Register Read", (type_t(*)())bcm54194_sfp_reg_rdwr, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx SFP Register Write", (type_t(*)())bcm54194_sfp_reg_rdwr, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx SFP Register Dump", (type_t(*)())bcm54194_sfp_reg_dump, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Loopback Test", (type_t(*)())bcm54194_lpbk_test, -1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx Test Mode", (type_t(*)())bcm54194_test_mode_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},

    {"BCM541xx SGMII Slave Mode", (type_t(*)())bcm54194_sgmii_slave_mode_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
};


/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM54194_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM54194_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))

#define BCM54194_TESTS_UTIL_SIZE (sizeof(BCM54194_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/

/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM54194_tests_primary_items[BCM54194_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t BCM54194_tests_secondary_items[BCM54194_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM54194_tests_primary_util_items[BCM54194_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t BCM54194_tests_secondary_util_items[BCM54194_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];



/******************************************************************************
 * BCM541xx Utils submenu
 *****************************************************************************/
menuinfo_t BCM54194_util_menu = {
    "GE PHY BCM541xx Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    BCM54194_tests_primary_util_items,
};
menuinfo_t *BCM54194_util_menup = &BCM54194_util_menu;

menuinfo_t BCM54194_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    BCM54194_tests_primary_items,
};
menuinfo_t *BCM54194_submenup = &BCM54194_subtest_menu;

/******************************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 ******************************************************************************/
long manhattan_bcm54194_test(int show_menu)
{
    build_primary_submenu(BCM54194_tests_submenu_table,
                          BCM54194_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY BCM541xx", &BCM54194_submenup);
    build_secondary_submenu(BCM54194_tests_submenu_table,
                            BCM54194_TESTS_SUBMENU_TABLE_SIZE,
                            BCM54194_tests_secondary_items);

    if (show_menu) {
        menu(BCM54194_submenup, BCM54194_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(BCM54194_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : BCM541xx_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM541xx_utility (int show_menu)
{
    manhattan_front_port_to_54194_port_map_show();
    printf("!!! NOTE: In utility context, 'port' refers to the BCM54194 port instead of front pannel port.\n");
    printf("Press 'enter' to continue...\n");
    getchar();

    build_primary_submenu(BCM54194_util_items, BCM54194_TESTS_UTIL_SIZE,
                          "GE PHY BCM541xx Utilities Menu", &BCM54194_util_menup);
    build_secondary_submenu(BCM54194_util_items, BCM54194_TESTS_UTIL_SIZE,
                            BCM54194_tests_secondary_util_items);

    menu(BCM54194_util_menup, BCM54194_tests_secondary_util_items, '\0' );

    return (PASSED);
}


static inline int __ask_for_port(void)
{
    const int port_min = 0;
    const int port_max = MANHATTAN_BCM54194_PORT_NUMB - 1;
    char      buf[64]  = {0,};

    manhattan_front_port_to_54194_port_map_show();
    printf("!!! NOTE: In utility context, 'port' refers to the BCM54194 port instead of front pannel port.\n");

    snprintf(buf, sizeof(buf), "Enter PHY port number(%d - %d)", port_min, port_max);
    return getdec_answer(buf, port_min, port_min, port_max);
}

static inline int __ask_for_intf(int num, ...)
{
    int     intf_min = 0;
    int     intf_max = 0;
    int     idx      = 0;
    int     val      = 0;
    va_list ap;

    va_start(ap, num);
    printf("Interfaces:\n");
    for(idx = 0; idx < num; idx++) {
        val = va_arg(ap, int);
        if (val > intf_max) intf_max = val;
        if (val < intf_min) intf_min = val;
        assert(val <= MANHATTAN_BCM54194_INTF_FIBER && val >= MANHATTAN_BCM54194_INTF_SGMII);
        printf("  %d: %s\n", val, MANHATTAN_INF_NAME(val));
    }
    va_end(ap);
    return getdec_answer("Enter enterface", intf_min, intf_min, intf_max);
}

static inline int __ask_for_speed(void)
{
    const int speed[] = {
        SPD_10MBPS,
        SPD_100MBPS,
        SPD_1000MBPS,
    };
    int i = 0;
    printf("Speed:\n");
    for (i = 0; i < sizeof(speed)/sizeof(int); i++) {
        printf("  %d: %-4d MBPS\n", i, speed[i]);
    }
    i = getdec_answer("  > ", 2, 0, 2);
    return speed[i];
}

static inline int __ask_for_mdio_c45_dev(void)
{
    int dev = 0;
    while(1) {
        dev = getdec_answer("Enter dev addr(1, 3, 7): ", 1, 1, 7);
        switch(dev) {
        case 1: case 3: case 7: return dev;
        }
        continue;
    }
    return 0;
}

/*******************************************************************************
 *
 * Function: phy_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : interface structure pointer, info for all registers
 *
 * Output: PASS/FAIL
 *
 *******************************************************************************/
static int phy_register_tests (int phy_addr, manhattan_bcm54194_intf_t intf, const reg_info_t *reg_ptr )
{
    uint32_t ix         = 0;
    uint16_t retval     = PASSED;
    uint16_t data       = 0;
    uint16_t temp       = 0;
    uint16_t tst_offset = 0;
    uint16_t save_val   = 0;
    uint16_t readval    = 0x0;

    /* Switch to Copper/Fiber register space */
    if (intf == MANHATTAN_BCM54194_INTF_FIBER) {
        manhattan_bcm54194_reg_1000x_en(phy_addr, TRUE);
    } else {
        manhattan_bcm54194_reg_1000x_en(phy_addr, FALSE);
    }

    while (reg_ptr->size.size != 0) {
        printf("  Test reg %-40s phyaddr-0x%02x regaddr-0x%04x test-mask-0x%04x ...\n",
            reg_ptr->name, phy_addr, reg_ptr->offset, reg_ptr->mask);
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_ptr->offset, &save_val), FAILED, "Failed.\n");
        printf("    Orig value 0x%04x\n", save_val);
        if (reg_ptr->type == READ_WRITE) {
            tst_offset = reg_ptr->offset;
            /*
             * ripple 1 test
             */
            printf("    Ripple 1 test.\n");
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                /* Write to register under test */

                printf("      Write 0x%04x\n", temp);
                ERET_COND(0 != MHT_MDIO_WR(phy_addr, tst_offset, temp), FAILED, "Failed.\n");
                /* Read back */
                ERET_COND(0 != MHT_MDIO_RD(phy_addr, tst_offset, &readval), FAILED, "Failed.\n");
                if ( ((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {

                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d,Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, temp,
                          readval);
                    return (FAILED);
                }
            }

            /*
             * ripple 0 test
             */
            printf("    Ripple 0 test.\n");
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                temp = (~(1 << ix)) & reg_ptr->mask;
                /* Write to register under test */
                printf("      Write 0x%04x\n", temp);
                ERET_COND(0 != MHT_MDIO_WR(phy_addr, tst_offset, temp), FAILED, "Failed.\n");

                /* Read back */
                ERET_COND(0 != MHT_MDIO_RD(phy_addr, tst_offset, &readval), FAILED, "Failed.\n");

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d, Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, temp,
                          readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = REG_TST_NEP_PATTERN;
            printf("    Pattern test:0x%04x.\n", data);
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                /* Write to register under test */
                printf("      Write 0x%04x\n", temp);
                ERET_COND(0 != MHT_MDIO_WR(phy_addr, tst_offset, temp), FAILED, "Failed.\n");
                /* Read back */
                ERET_COND(0 != MHT_MDIO_RD(phy_addr, tst_offset, &readval), FAILED, "Failed.\n");

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Pattern test failed when accessing %s "
                          "Register offset %#x phy_addr %d, Expect %#x, "
                          "Read %#x", __FUNCTION__, reg_ptr->name, tst_offset,
                          phy_addr, temp, readval);
                    return (retval);
                }

                data = ~REG_TST_NEP_PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            printf("    Restore 0x%04x\n", save_val);
            ERET_COND(0 != MHT_MDIO_WR(phy_addr, tst_offset, save_val),
                    FAILED, "Failed to restoring register.\n");
            printf("  OK\n\n");
        } else {
            printf("  OK\n\n");
        }
        reg_ptr++;
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: BCM541xx_register_test
 *
 * Description: This function performs the BCM541xx register test.
 * This test only performs BCM541xx IEEE register set.
 *
 * Inputs      : port - port number
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM541xx_register_test (int arg)
{
    int ix       = 0;
    int phy_addr = 0;
    testname("BCM541xx PHY IEEE Register");

    for (ix = 0; ix < MANHATTAN_BCM54194_PORT_NUMB; ix++) {
        phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(ix);

        prpass(testpass, "\nFiber : ");
        printf("\n");
        if (phy_register_tests(phy_addr, MANHATTAN_BCM54194_INTF_FIBER,
                               &bcm54194_ieee_fiber_reg[0]) == FAILED) {
            cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
            return (FAILED);
        }

        prpass(testpass, "\nCopper : ");
        printf("\n");
        if (phy_register_tests(phy_addr, MANHATTAN_BCM54194_INTF_COPPER,
                               &bcm54194_ieee_copper_reg[0]) == FAILED) {
            cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
            return (FAILED);
        }
    }

    return (PASSED);
}

static int bcm54194_copper_lpbk_config(int phy_addr, int speed, int extnl_intnl)
{
    char cmd[64];
    // ERET_COND(0 != manhattan_bcm54194_switch_to_fiber(phy_addr, 0),
    //         FAILED, "Failed to change to copper interface.\n");


    // Turn off fiber
    ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 0, MANHATTAN_BCM54194_INTF_FIBER),
            -(__LINE__), "Phy-addr-0x%02x: disable fiber failed", phy_addr);

    // Select copper reg space
    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, 0), -(__LINE__), "Failed.\n");
    sprintf(cmd, "ifconfig %s down", _g_p_seahawks->eth_map[MANHATTAN_BCM54194_PORT_BY_PHYAD(phy_addr)]);
    system(cmd);
    switzer_mdelay(1000);
    sprintf(cmd, "ifconfig %s up", _g_p_seahawks->eth_map[MANHATTAN_BCM54194_PORT_BY_PHYAD(phy_addr)]);
    system(cmd);
    switzer_mdelay(1000);

    ERET_COND(0 != manhattan_bcm54194_config_loopback(phy_addr, speed, extnl_intnl, 1),
            FAILED, "Failed to config internal loopback.\n");
    ERET_COND(!manhattan_bcm54194_is_linkup(phy_addr, MANHATTAN_BCM54194_INTF_COPPER),
            FAILED, "Wait for linkup failed.\n");
    return PASSED;
}

static int bcm54194_copper_lpbk_config_util(int arg)
{
    int      port        = __ask_for_port();
    int      speed       = __ask_for_speed();
    uint16_t phy_addr    = MANHATTAN_BCM54194_PHYAD_PORT(port);
    int      extnl_intnl = arg;
    return bcm54194_copper_lpbk_config(phy_addr, speed, extnl_intnl);
}

static int bcm54194_fiber_lpbk_extnl_config(int phy_addr, int speed)
{
    ERET_COND(0 != manhattan_bcm54194_switch_to_fiber(phy_addr, 1),
            FAILED, "Failed to switch to fiber interface..\n");
    ERET_COND(0 != manhattan_bcm54194_config_loopback(phy_addr, speed, MANHATTAN_BCM54194_LPBK_SFP_EXT, 1),
            FAILED, "Config loopback failed.\n");
    ERET_COND(!manhattan_bcm54194_is_linkup(phy_addr, MANHATTAN_BCM54194_INTF_FIBER),
            FAILED, "Wait for linkup failed.\n");
    return PASSED;
}

static int bcm54194_fiber_lpbk_extnl_config_util(int arg)
{
    int      port     = __ask_for_port();
    uint16_t phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);
    int      speed    = getdec_answer("Speed(0-100MBPS, 1-1000MBPS)", 1, 0, 1);
    return bcm54194_fiber_lpbk_extnl_config(phy_addr, speed ? SPD_1000MBPS : SPD_100MBPS);
}

static int bcm54194_sgmii_lpbk_config(int arg)
{
    int      port     = arg < 0 ? __ask_for_port() : arg;
    int      speed    = __ask_for_speed();
    uint16_t phy_addr = MANHATTAN_BCM54194_PHYAD_SGMII(port);;
#if 0
    uint16_t val      = 0

    // Write RDB register 0x021, bit[0] = 1'b1
    // Write RDB register 0x00E, bit[12] = 1'b1 (Force link in 10 Mb/s or 100 Mb/s mode. Not needed for 1000 Mb/s mode.)
    // Select the SGMII speed for the port.
    //   Write Register 0x0 = 0x4140 to enable 1000 Mb/s loopback or
    //   Write Register 0x0 = 0x6100 to enable 100 Mb/s loopback or
    //   Write Register 0x0 = 0x4100 to enable 10 Mb/s loopback

    ERET_COND(0 != MHT_RDB_RD(phy_addr, 0x21, &val), FAILED, "Failed.\n");
    val |= 1;
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0, 0x4140), FAILED, "Failed.\n");
    return PASSED;
#else
    ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 0, MANHATTAN_BCM54194_INTF_FIBER),
            FAILED, "Failed.\n");
    ERET_COND(0 != manhattan_bcm54194_config_loopback(phy_addr, speed, MANHATTAN_BCM54194_LPBK_SGMII, 1),
            FAILED, "Config loopback failed.\n");
    ERET_COND(!manhattan_bcm54194_is_linkup(phy_addr, MANHATTAN_BCM54194_INTF_FIBER),
            FAILED, "Wait for linkup failed.\n");
    return PASSED;
#endif
}


static int bcm54194_lpbk_extnl_cfg_dump(int arg)
{
    /* This func doesn't change configs, just read */
    int port = arg < 0 ? __ask_for_port() : arg;
    return manhattan_bcm54194_loopback_config_dump(MANHATTAN_BCM54194_PHYAD_PORT(port), MANHATTAN_BCM54194_LPBK_EXT);
}

static int bcm54194_lpbk_intnl_cfg_dump(int arg)
{
    /* This func doesn't change configs, just read */
    int port = arg < 0 ? __ask_for_port() : arg;
    return manhattan_bcm54194_loopback_config_dump(MANHATTAN_BCM54194_PHYAD_PORT(port), MANHATTAN_BCM54194_LPBK_INT);
}

static int bcm54194_link_status(int arg)
{
    /* This func doesn't change configs, just read */
    return manhattan_bcm54194_link_status() == 0 ? PASSED : FAILED;
}

static int bcm54194_pkt_cntr_util(int arg)
{
    /* This func doesn't change configs, just read */
    int phya = 0;
    int port = arg < 0 ? __ask_for_port() : arg & 0xf;
    int act  = arg < 0 ? getdec_answer(
                "Select action:\n"
                "  0: Enable RX counter\n"
                "  1: Enable TX counter\n"
                "  2: Show counters\n"
                "  Note: RX and TX are exlusive\n",
                2, 0, 2) : (arg & 0xf0) >> 4;

    if (act == 2) {
        printf("== Line side ==\n");
        phya = MANHATTAN_BCM54194_PHYAD_PORT(port);
        ERET_COND(PASSED != bcm54194_reg_parse_dump(phya, pkt_counter_reg_detail_info), FAILED, "Failed.\n");
        printf("\n== System side ==\n");
        phya = MANHATTAN_BCM54194_PHYAD_SGMII(port);
        ERET_COND(PASSED != bcm54194_reg_parse_dump(phya, pkt_counter_reg_detail_info), FAILED, "Failed.\n");
    }
    return manhattan_bcm54194_packet_counter_util(port, act) == 0 ? PASSED : FAILED;
}


static int __get_bit_comp_val(uint16_t regv, const char *desc, int *val)
{
    char _desc[128];
    char *p  = NULL;
    char *t  = NULL;
    char *pp = NULL;
    char *tt = NULL;
    int   b  = 0;
    int   e  = 0;
    int   s  = 0;
    int   v  = 0;
    int   flg= 0;

    memset(_desc, 0, sizeof(_desc));
    memcpy(_desc, desc, strlen(desc));
    forpart(p, _desc, ',', t) {
        //printf("    %s\n", p);
        flg = 0;
        forpart(pp, p, ':', tt) {
            if (flg > 1) {
                log_err("Wrong syntax in '%s' of '%s' .\n", p, desc);
                return -1;
            }
            if (1 != sscanf(pp, "%d", &b)) {
                log_err("Wrong syntax in '%s' of '%s' .\n", p, desc);
                return -2;
            }
            if (flg == 0) {
                e   = b;
                flg += 1;
            } else {
                s = b;
            }
            //printf("      bit-%d\n", b);
        }
        //printf("      e %d\n", e);
        //printf("      s %d\n", s);
        v <<= e - s + 1;
        v |= (regv & (((1 << (e - s + 1)) - 1) << s)) >> s;
    }
    *val = v;
    return 0;
}


static char *__get_val_desc(const uint16_t val, const char *desc, char *buf, const int buflen)
{
    char *p = NULL;
    char *q = NULL;
    char _desc[1024];
    char *t = NULL;
    int   v = 0;

    memset(_desc, 0, sizeof(_desc));
    strncpy(_desc, desc, sizeof(_desc));

    forpart_ext(p, _desc, ";;", t) {
        while(*p && isspace(*p)) {
            p++;
        }
        if (! *p) {
            p = NULL;
            continue;
        } //break loop
        if(strncmp("NOTE@@", p, sizeof("NOTE@@") - 1) == 0) {
            strncpy(buf, p + sizeof("NOTE@@") -1, buflen);
            return buf;
        }
        if(1 != sscanf(p, "%i", &v)) {
            log_err("Syntax error on '%s' of '%s'\n", p, desc);
            return NULL;
        }
        if (!(q = strstr(p, "@@"))) {
            log_err("Syntax error on '%s' of '%s'\n", p, desc);
            return NULL;
        }
        if (v == val) {
            strncpy(buf, q + 2, strlen(q + 2));
            return buf;
        }
    }
    strncpy(buf, "Udefined.", sizeof("Undefined."));
    return buf;
}

static int bcm54194_reg_parse_dump(int phy_addr, struct __reg_parser *rp)
{
    int      i    = 0;
    int      j    = 0;
    uint16_t regv = 0;
    int      val  = 0;
    int      maxl = 0;
    char buf[256];
    char fmt[64];

    for(i = 0; rp[i].type >=0; i++) {
        if (rp[i].type == __REG_TYPE_MDIO) {
            ERET_COND(0 != MHT_MDIO_RD(phy_addr, rp[i].offs, &regv), FAILED, "");
        } else {
            ERET_COND(0 != MHT_RDB_RD(phy_addr, rp[i].offs, &regv), FAILED, "");
        }
        printf("Reg %s %-s phy-addr-0x%02x off-0x%04x: 0x%04x (%s)\n",
            rp[i].type == __REG_TYPE_MDIO ? "REG" : "RDB",
            rp[i].name, phy_addr, rp[i].offs, regv, __binary_dump_16(regv, NULL));

        for(maxl = 0, j = 0; rp[i].fld_info[j].name; j++) {
            if (maxl < strlen(rp[i].fld_info[j].name))
                maxl = strlen(rp[i].fld_info[j].name);
        }
        memset(fmt, 0, sizeof(fmt));
        snprintf(fmt, sizeof(fmt), "  %%-%ds:", maxl);
        strcat(fmt, " 0x%-4x (%-10s %-6s)(%-s)\n");
        for(j = 0; rp[i].fld_info[j].name; j++) {
            memset(buf, 0, sizeof(buf));
            ERET_COND(0 != __get_bit_comp_val(regv, rp[i].fld_info[j].bit_comp, &val), FAILED, "Failed.\n");
            ERET_COND(NULL == __get_val_desc(val, rp[i].fld_info[j].val_desc, buf, sizeof(buf) - 1), FAILED, "Failed.\n");
            printf(fmt, rp[i].fld_info[j].name, val, rp[i].fld_info[j].bit_comp, rp[i].fld_info[j].attr, buf);
        }
        printf("\n");
    }
    return PASSED;
}

static int bcm54194_port_status_sgmii(int port)
{
    int phya =  MANHATTAN_BCM54194_PHYAD_SGMII(port);

    printf("SGMII Port-%d status dump\n", port);
    printf("=====================================================\n");

    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phya, 1), FAILED, "Failed to enable SGMII reg space.\n");
    ERET_COND(PASSED != bcm54194_reg_parse_dump(phya, sgmii_reg_detail_info), FAILED, "Failed.\n");

    return PASSED;
}

static int bcm54194_port_status_copper(int port)
{
    int phya =  MANHATTAN_BCM54194_PHYAD_PORT(port);
    printf("Copper Port-%d status dump\n", port);
    printf("=====================================================\n");

    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phya, 0), FAILED, "Failed to enable Copper reg space.\n");
    ERET_COND(PASSED != bcm54194_reg_parse_dump(phya, copper_reg_detail_info), FAILED, "Failed.\n");
    return PASSED;
}

static int bcm54194_port_status_fiber(int port)
{
    int phya =  MANHATTAN_BCM54194_PHYAD_PORT(port);
    printf("Copper Port-%d status dump\n", port);
    printf("=====================================================\n");

    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phya, 1), FAILED, "Failed to enable Fiber reg space.\n");
    ERET_COND(PASSED != bcm54194_reg_parse_dump(phya, fiber_reg_detail_info), FAILED, "Failed.\n");
    return PASSED;
}

static int bcm54194_port_detail_status(int arg)
{
    /* This func doesn't change configs, just read */
    int port = __ask_for_port();
    int intf = __ask_for_intf(3, MANHATTAN_BCM54194_INTF_SGMII, MANHATTAN_BCM54194_INTF_COPPER, MANHATTAN_BCM54194_INTF_FIBER);

    switch(intf) {
    case MANHATTAN_BCM54194_INTF_SGMII  : return bcm54194_port_status_sgmii(port);
    case MANHATTAN_BCM54194_INTF_COPPER : return bcm54194_port_status_copper(port);
    case MANHATTAN_BCM54194_INTF_FIBER  : return bcm54194_port_status_fiber(port);
    }
    return FAILED;
}

static int bcm54194_sfp_status(int arg)
{
    int phya =  MANHATTAN_BCM54194_PHYAD;
    printf("Copper SFP status dump\n");
    printf("=====================================================\n");
    ERET_COND(PASSED != bcm54194_reg_parse_dump(phya, sfp_stat_reg_detail_info), FAILED, "Failed.\n");
    return PASSED;
}

int bcm54194_sfp_reg_rdwr(int rdwr)
{
    int      port     = __ask_for_port();
    int      sfp_addr = 0;
    int      sfp_reg  = 0;
    ushort   val      = 0;

    //TODO: check sfp on-slot status
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x890, &val), FAILED, "");
    ERET_COND(!(val & (1 << (port + 12))), FAILED, "No SFP on port-%d\n", port);

    sfp_addr= gethex_answer("Sfp I2C addr", 0x50, 0, 0xff); //SWITZER_MANHATTAN_I2C_ADDR_SFP
    sfp_reg = gethex_answer("Sfp register addr", 0, 0, 0xffff);
    if (rdwr) {
        val = gethex_answer("Value to write", 0, 0, 0xff);
        printf("WR Port-%d SFP I2C Addr-0x%02x Reg-0x%04x: 0x%02x\n", port, sfp_addr, sfp_reg, val);
        ERET_COND(0 != bcm54194_i2c_slave_write(port, sfp_addr, sfp_reg, val), FAILED, "Failed.\n");
    } else {
        ERET_COND(0 != bcm54194_i2c_slave_read(port, sfp_addr, sfp_reg, &val, 1), FAILED, "Failed.\n");
        printf("RD Port-%d SFP I2C Addr-0x%02x Reg-0x%04x: 0x%02x\n", port, sfp_addr, sfp_reg, val);
    }
    return PASSED;
}

int bcm54194_sfp_reg_dump(int arg)
{
    int    port     = __ask_for_port();
    int    sfp_addr = 0;
    int    reg_off  = 0;
    int    numb     = 0;
    int    i        = 0;
    ushort val      = 0;

    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x890, &val), FAILED, "");
    ERET_COND(!(val & (1 << (port + 12))), FAILED, "No SFP on port-%d\n", port);

    sfp_addr= gethex_answer("Sfp I2C addr", 0x50, 0, 0xff); //SWITZER_MANHATTAN_I2C_ADDR_SFP
    reg_off = gethex_answer("Sfp register addr offset", 0, 0, 0xffff);
    numb    = getdec_answer("Number of registers to read", 1, 1, 256);

    printf("%6s", " ");
    for(i = 0; i < 16; i++)
        printf("%-2x ", i);
    printf("\n");
    printf("%04x: ", reg_off & 0xf);
    for(i = reg_off & 0xf; i < reg_off; i++)
        printf("%3s", " ");
    for(i = 0; i < numb;) {
        ERET_COND(0 != bcm54194_i2c_slave_read(port, sfp_addr, reg_off + i, &val, 1), FAILED, "Failed.\n");
        printf("%02x ", val&0xff);
        i += 1;
        if ((reg_off + i) % 16 == 0) {
            printf("\n%04x: ", reg_off + i);
        }
    }
    printf("\n");
    return PASSED;
}

static int bcm54194_lpbk_test_copper(int port)
{
    int idx      = 0;
    int spd      = 0;
    int phy_addr = 0;

    manhattan_bcm54194_reset(1);
    phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);

    printf("\nDiagflag:0x%x\n", (NVRAM)->diagflag);
    printf("!!! For BCM54194, external and internal loopback are exclusive each other.\n\n");
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, perform internal loopback test..\n");
        for_each(idx, spd, int, 3, (int)SPD_1000MBPS, (int)SPD_100MBPS, (int)SPD_10MBPS) {
            printf("Copper port-%d speed-%d Mbps internal loopback test...\n", port, spd);
            if (spd != SPD_1000MBPS) {
                printf("TODO: Speed-%d Loopback test is not available now.\n", spd);
                continue;
            }
            ERET_COND(0 != bcm54194_copper_lpbk_config(phy_addr, spd, MANHATTAN_BCM54194_LPBK_INT),
                FAILED, "Failed.\n");
            if (_g_p_seahawks->eth_map[port][0]) {
                ERET_COND(PASSED != switzer_manhattan_sock_test(_g_p_seahawks->eth_map[port], _g_p_seahawks->eth_map[port]),
                    FAILED, "Failed to run packet test on '%s' (BCM54194 port-%d)\n", _g_p_seahawks->eth_map[port], port);
            } else {
                printf("No ethernet port name for BCM54194 port-%d, skip packet test.\n", port);
            }
        }
    } else {
        printf("External loopback flag is on, perform external loopback test..\n");
        for_each(idx, spd, int, 3, (int)SPD_1000MBPS, (int)SPD_100MBPS, (int)SPD_10MBPS) {
            printf("Copper port-%d speed-%d Mbps external loopback test...\n", port, spd);
            if (spd != SPD_1000MBPS) {
                printf("TODO: Speed-%d Loopback test is not available now.\n", spd);
                continue;
            }
            ERET_COND(0 != bcm54194_copper_lpbk_config(phy_addr, spd, MANHATTAN_BCM54194_LPBK_EXT),
                FAILED, "Failed.\n");
            if (_g_p_seahawks->eth_map[port][0]) {
                ERET_COND(PASSED != switzer_manhattan_sock_test(_g_p_seahawks->eth_map[port], _g_p_seahawks->eth_map[port]),
                    FAILED, "Failed to run packet test on '%s' (BCM54194 port-%d)\n", _g_p_seahawks->eth_map[port], port);
            } else {
                printf("No ethernet port name for BCM54194 port-%d, skip packet test.\n", port);
            }
        }
    }
    manhattan_bcm54194_reset(1);
    return PASSED;
}

static int bcm54194_sfp_reg_dump_vendor_name(int fiber_port)
{
    int    port     = fiber_port;
    int    sfp_addr = SWITZER_MANHATTAN_I2C_ADDR_SFP;
    int    reg_off  = SFP_REG_VENDOR_NAME_OFFSET;
    int    numb     = SFP_REG_VENDOR_NAME_LEN;
    int    i        = 0;
    ushort val      = 0;
    char   buf[20];

    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x890, &val), FAILED, "");
    ERET_COND(!(val & (1 << (port + 12))), FAILED, "No SFP on port-%d\n", port);

    for(i = 0; i < numb; i++) {
        ERET_COND(0 != bcm54194_i2c_slave_read(port, sfp_addr, reg_off + i, &val, 1), FAILED, "Failed.\n");
        buf[i] = val & 0xff;
    }
    printf("\nSFP vendor name: %s\n",buf);

    return PASSED;
}

static int bcm54194_lpbk_test_fiber(int port)
{
    int idx      = 0;
    int spd      = 0;
    int phy_addr = 0;

    phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip on fiber port-%d\n", port);
        return PASSED;
    }

    manhattan_bcm54194_reset(1);
    ERET_COND(0 != bcm54194_sfp_reg_dump_vendor_name(port), FAILED, "dump SFP vendor name Failed,check SFP i2c\n");

    for_each(idx, spd, int, 2, (int)SPD_1000MBPS, (int)SPD_100MBPS) {
        if (spd != SPD_1000MBPS) {
            printf("TODO: Speed-%d Loopback test is not available now.\n", spd);
            continue;
        }
        printf("Fiber port-%d speed-%d Mbps exnternal loopback test...\n", port, spd);
        ERET_COND(0 != bcm54194_fiber_lpbk_extnl_config(phy_addr, spd), FAILED, "Failed.\n");
        if (_g_p_seahawks->eth_map[port][0]) {
            ERET_COND(PASSED != switzer_manhattan_sock_test(_g_p_seahawks->eth_map[port], _g_p_seahawks->eth_map[port]),
                FAILED, "Failed to run packet test on '%s' (BCM54194 port-%d)\n", _g_p_seahawks->eth_map[port], port);
        } else {
            printf("No ethernet port name for BCM54194 port-%d, skip packet test.\n", port);
        }
    }
    manhattan_bcm54194_reset(1);
    return PASSED;
}

static int bcm54194_lpbk_test(int arg)
{
    int                       port = 0;
    manhattan_bcm54194_intf_t intf = 0;

    printf("%s arg:0x%0x\n", __func__, arg);

    if (arg < 0) {
        port = __ask_for_port();
        intf = __ask_for_intf(2, MANHATTAN_BCM54194_INTF_COPPER, MANHATTAN_BCM54194_INTF_FIBER);
    } else {
        port = (arg & 0xff00) >> 8;
        intf = (arg & 0x00ff);
    }

    ERET_COND(port >= MANHATTAN_BCM54194_PORT_NUMB || intf > MANHATTAN_BCM54194_INTF_FIBER, FAILED, "Invalid argument 0x%x.\n", arg);

    printf("%s Interface:%s\n", __func__, MANHATTAN_INF_NAME(intf));

    if (intf == MANHATTAN_BCM54194_INTF_COPPER) {
        ERET_COND(PASSED != bcm54194_lpbk_test_copper(port), FAILED, "Failed.\n");
    } else {
        ERET_COND(PASSED != bcm54194_lpbk_test_fiber(port), FAILED, "Failed.\n");
    }
    return PASSED;
}

static int bcm54194_test_mode_util(int arg)
{
    int       port     = 0;
    int       phya     = 0;
    int       tstm     = 0;

    port = __ask_for_port();
    tstm = getdec_answer("Test mode:\n"
                         "  0: Normal Operation\n"
                         "  1: Transmit Waveform Test\n"
                         "  2: Master Transmit Jitter Test\n"
                         "  3: Slave Transmit Jitter Test\n"
                         "  4: Transmit Distortion Test\n"
                         "> ", 0, 0, 4);

    phya = MANHATTAN_BCM54194_PHYAD_PORT(port);
    ERET_COND(0 != bcm54194_transmit_test_pattern(phya, tstm), FAILED, "Failed to set test mode to '%d'\n", tstm);
    return PASSED;
}

static int bcm54194_sgmii_slave_mode_util(int arg)
{
    int port   = 0;
    int enable = 0;
    int phya   = 0;
    port   = __ask_for_port();
    enable = getdec_answer("0: Disable, 1: Enable ", 0, 0, 1);
    phya   = MANHATTAN_BCM54194_PHYAD_PORT(port);
    ERET_COND(0 != manhattan_bcm54194_sgmii_slave_mode(phya, enable), FAILED, "Failed to set sgmii slave mode.\n");
    return PASSED;
}


/*******************************************************************************
 *
 * Function: dump_phy_reg()
 *
 * This function prints the specific PHY register values
 *
 * Input: curr_port - current port (wit offset)
 *        page_reg_ptr - page table pointer.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_phy_reg(int port, const bcm54194_regs_t *phy_reg_ptr)
{
    short             rdval    = 0;
    const reg_info_t *reg_ptr  = NULL;
    int               phy_addr = 0;

    /* Switch to SGMII or Fiber register space */
    if (phy_reg_ptr->phy_intf == MANHATTAN_BCM54194_INTF_SGMII) {
        phy_addr = MANHATTAN_BCM54194_PHYAD_SGMII(port);
        //manhattan_bcm54194_switch_intf_access(phy_reg_ptr->phy_intf);
    } else if (phy_reg_ptr->phy_intf == MANHATTAN_BCM54194_INTF_FIBER) {
        phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);
        //manhattan_bcm54194_reg_1000x_en(phy_addr, 1);
    } else {
        phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);
    }
    manhattan_bcm54194_switch_intf_access(phy_reg_ptr->phy_intf);

    //printf("\n%s\n", phy_reg_ptr->intfname);
    reg_ptr = phy_reg_ptr->intfregs;

    while (reg_ptr->size.size != 0) {
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_ptr->offset, &rdval), FAILED, "Failed.\n");
        /* we don't check rdval is nagetive here,
         * some of registers will get '0xF' on MSB
         */

        printf("%-6s: %-40s port-0x%02x phy-addr-0x%02x reg-0x%02x = 0x%04x (%s)\n",
            phy_reg_ptr->intfname, reg_ptr->name, port, phy_addr, reg_ptr->offset, rdval & 0xffff,
            __binary_dump_16(rdval & 0xffff, NULL));
        reg_ptr++;
        switzer_mdelay(10); /* wait for a while for next register. */
    }

#ifdef BCM54194_A0_SILICON
    /* Restore to Copper register space */
    if (phy_reg_ptr->phy_intf == MANHATTAN_BCM54194_INTF_SGMII) {
        manhattan_bcm54194_switch_intf_access(MANHATTAN_BCM54194_INTF_COPPER);
    } else if (phy_reg_ptr->phy_intf == MANHATTAN_BCM54194_INTF_FIBER) {
        manhattan_bcm54194_reg_1000x_en(phy_addr, 0);
    }
#endif

    return PASSED;
}

static int manhattan_bcm54194_mdio22_rdwr_util (int rdwr)
{
    int      port = 0;
    int      intf = 0;
    int      phya = 0;
    int      rega = 0;
    uint16_t regv = 0;

    port = __ask_for_port();
    intf = __ask_for_intf(3, MANHATTAN_BCM54194_INTF_SGMII, MANHATTAN_BCM54194_INTF_COPPER, MANHATTAN_BCM54194_INTF_FIBER);
    phya = MANHATTAN_BCM54194_PHYAD_GET(port, intf);
    rega = gethex_answer("Enter register addr", 0, 0, 0xffff);

    ERET_COND(0 != manhattan_bcm54194_switch_intf_access(intf),
            FAILED, "Failed to switch to intface %u(%s)\n", intf, MANHATTAN_INF_NAME(intf));

    if (rdwr == 0) { /* read */
        ERET_COND(0 != MHT_MDIO_RD(phya, rega, &regv), FAILED, "Failed.\n");
        printf("RD port-0x%02x intf-%-6s phy-addr-0x%02x reg-0x%04x val:0x%04x (%s)\n",
            port, MANHATTAN_INF_NAME(intf), phya, rega, regv, __binary_dump_16(regv, NULL));
    } else { /* write */
        regv = gethex_answer("Enter register value(0x0): ", 0, 0, 0xffff);
        ERET_COND(0 != MHT_MDIO_WR(phya, rega,  regv), FAILED, "Failed.\n");
        printf("WR port-0x%02x intf-%-6s phy-addr-0x%02x reg-0x%04x val:0x%04x (%s)\n",
            port, MANHATTAN_INF_NAME(intf), phya, rega, regv, __binary_dump_16(regv, NULL));
    }

    return PASSED;
}

static int manhattan_bcm54194_mdio45_rdwr_util (int rdwr)
{
    int      port = 0;
    int      phya = 0;
    int      deva = 0;
    int      rega = 0;
    uint16_t regv = 0;

    port = __ask_for_port();
    deva = __ask_for_mdio_c45_dev();
    rega = gethex_answer("Enter register addr", 0, 0, 0xffff);

    phya = MANHATTAN_BCM54194_PHYAD_PORT(port);

    if (rdwr == 0) { /* read */
        ERET_COND(0 != MHT_MDIO_RD45(phya, deva, rega, &regv), FAILED, "Failed.\n");
        printf("RD port-0x%02x phy-addr-0x%02x dev-addr-0x%02x reg-0x%04x val:0x%04x (%s)\n",
            port, phya, deva, rega, regv, __binary_dump_16(regv, NULL));
    } else { /* write */
        regv = gethex_answer("Enter register value(0x0): ", 0, 0, 0xffff);
        ERET_COND(0 != MHT_MDIO_WR45(phya, deva, rega,  regv), FAILED, "Failed.\n");
        printf("WR port-0x%02x phy-addr-0x%02x dev-addr-0x%02x reg-0x%04x val:0x%04x (%s)\n",
            port, phya, deva, rega, regv, __binary_dump_16(regv, NULL));
    }
    return PASSED;
}

static int manhattan_bcm54194_rdb_rdwr_util (int rdwr)
{
    int      port = 0;
    int      phya = 0;
    int      rdb  = 0;
    uint16_t regv = 0;

    port = __ask_for_port();
    rdb  = gethex_answer("Enter register RDB offset", 0, 0, 0xffff);
    phya = MANHATTAN_BCM54194_PHYAD_PORT(port);

    if (rdwr == 0) { /* read */
        ERET_COND(0 != MHT_RDB_RD(phya, rdb, &regv), FAILED, "Failed.\n");
        printf("RD port-0x%02x phy-addr-0x%02x rdb-0x%04x val:0x%04x (%s)\n",
            port, phya, rdb, regv, __binary_dump_16(regv, NULL));
    } else { /* write */
        regv = gethex_answer("Enter register value(0x0): ", 0, 0, 0xffff);
        ERET_COND(0 != MHT_RDB_WR(phya, rdb,  regv), FAILED, "Failed.\n");
        printf("WR port-0x%02x phy-addr-0x%02x rdb-0x%04x val:0x%04x (%s)\n",
            port, phya, rdb, regv, __binary_dump_16(regv, NULL));
    }
    return PASSED;
}

static int manhattan_bcm54194_per_port_reg_dump_util(int arg)
{
    struct bcm54194_per_port_regs_s *reg  = NULL;
    int                              idx  = 0;
    uint16_t                         port = 0;
    uint16_t                         ports= 0;
    uint16_t                         porte= 0;
    uint16_t                         intf = 0;
    uint16_t                         intfs= 0;
    uint16_t                         intfe= 0;
    uint16_t                         phya = 0;
    uint16_t                         regv = 0;

    printf("%s\n", __func__);

    if (arg < 0) {
        ports = 0;
        porte = MANHATTAN_BCM54194_PORT_NUMB - 1;
        intfs = 0;
        intfe = MANHATTAN_BCM54194_INTF_FIBER;
    } else {
        porte = ports = __ask_for_port();
        intfe = intfs = __ask_for_intf(3, MANHATTAN_BCM54194_INTF_SGMII, MANHATTAN_BCM54194_INTF_COPPER, MANHATTAN_BCM54194_INTF_FIBER);
    }

    for(port = ports; port <= porte; port++) {
        for(intf = intfs; intf <= intfe; intf++) {
            phya = intf == MANHATTAN_BCM54194_INTF_SGMII ?
                           MANHATTAN_BCM54194_PHYAD_SGMII(port) :
                           MANHATTAN_BCM54194_PHYAD_PORT(port);

            printf("\n%s port-%d Register\n", MANHATTAN_INF_NAME(intf), port);
            printf("======================================================\n");
            for(idx = 0; bcm54194_per_port_regs[idx].name; idx++) {
                reg = &bcm54194_per_port_regs[idx];
                if ((reg->attr & BCM54194_REG_DUMP_ATTR_PMSK)  & ((1 << port) << BCM54194_REG_DUMP_ATTR_PSHFT)) {
                    ERET_COND(0 != MHT_RDB_RD(phya, reg->rdb, &regv), FAILED, "Failed.\n");
                    printf("%s port-%d phy-addr-0x%02x rdb-0x%04x : 0x%04x (%s) %s\n",
                            reg->name, port, phya, reg->rdb, regv, __binary_dump_16(regv, NULL),
                            reg->attr & BCM54194_REG_DUMP_ATTR_RDCLR_MSK ? "RDCLR" :
                            reg->attr & BCM54194_REG_DUMP_ATTR_WOCLR_MSK ? "WOCLR" :
                            reg->attr & BCM54194_REG_DUMP_ATTR_WZCLR_MSK ? "WZCLR" : "");
                } else {
                    printf("%s port-%d phy-addr-0x%02x rdb-0x%04x : Skip\n",
                            reg->name, port, phya, reg->rdb);
                }
            }
        }
    }
    return PASSED;
}

static int manhattan_bcm54194_global_reg_dump_util  (int arg)
{
    struct bcm54194_global_reg_s *reg  = NULL;
    int                           idx  = 0;
    uint16_t                      port = 0;
    uint16_t                      phya = 0;
    uint16_t                      regv = 0;

    printf("%s\n", __func__);
    port = 0;
    phya = MANHATTAN_BCM54194_PHYAD;

    for(idx = 0; bcm54194_global_regs[idx].name; idx++) {
        reg = &bcm54194_global_regs[idx];
        ERET_COND(0 != MHT_RDB_RD(phya, reg->rdb, &regv), FAILED, "Failed.\n");
        printf("%s port-%d phy-addr-0x%02x rdb-0x%04x : 0x%04x (%s) %s\n",
                reg->name, port, phya, reg->rdb, regv, __binary_dump_16(regv, NULL),
                reg->attr & BCM54194_REG_DUMP_ATTR_RDCLR_MSK ? "RDCLR" :
                reg->attr & BCM54194_REG_DUMP_ATTR_WOCLR_MSK ? "WOCLR" :
                reg->attr & BCM54194_REG_DUMP_ATTR_WZCLR_MSK ? "WZCLR" : "");
    }
    return PASSED;
}

static int manhattan_bcm54194_clause45_reg_dump_util(int arg)
{
    struct bcm54194_clause45_reg_s *reg = NULL;
    int                             idx = 0;
    uint16_t                        val = 0;

    printf("%s\n", __func__);
    for(idx = 0; bcm54194_clause45_regs[idx].name; idx++) {
        reg = &bcm54194_clause45_regs[idx];
        ERET_COND(0 != MHT_MDIO_RD45(MANHATTAN_BCM54194_PHYAD, reg->dev, reg->reg, &val), FAILED, "");
        printf("%-36s dev-0x%02x reg-0x%04x :0x%04x (%s)\n",
            reg->name, reg->dev, reg->reg, val, __binary_dump_16(val, NULL));
    }
    return 0;
}


/*
 * Function:manhattan_bcm54194_ieee_reg_dump_util
 *
 * This function displays the PHY setting of the requested SGMII port.
 *
 * Input: none.
 *
 * Output: void
 */
static int manhattan_bcm54194_ieee_reg_dump_util(int arg)
{
    int                    port;
    int                    ports;
    int                    porte;
    uint                   ix;
    uint                   numb;
    const bcm54194_regs_t *reg_ptr;

    printf("%s\n", __func__);

    if (arg < 0) {
        ports = 0;
        porte = MANHATTAN_BCM54194_PORT_NUMB - 1;
    } else {
        porte = ports = __ask_for_port();
    }
    numb     = sizeof(bcm54194_phy_ieee_reg_tbl) / sizeof(bcm54194_regs_t);

    /* dump all page */
    for(port = ports; port <= porte; port++) {
        reg_ptr  = &bcm54194_phy_ieee_reg_tbl[0];
        printf("\nIEEE Register dump on port-%d\n", port);
        printf("======================================================\n");
        for (ix = 0; ix < numb; ix++) {
            ERET_COND(PASSED != dump_phy_reg(port, reg_ptr), FAILED, "Failed.\n");
            reg_ptr++;
        }
    }

    return 0;
}

/*
 * Function: manhattan_bcm54194_interrupt_util
 *
 * Description:
 * Utility to Enable/Disable BCM54194 LASI Interrupt .
 *
 * Input: none
 *
 * Return: None
 */
int manhattan_bcm54194_interrupt_util (int port, int act)
{
    int phy_addr = 0;
    int phy_port = port;
    int action   = act;
    uint16_t int_status=0;

    phy_port = port>= 0 ? port : __ask_for_port();
    action   = act >= 0 ? act  : getdec_answer("Enter action\n"
                                               "    0:Disable\n"
                                               "    1:Enable\n"
                                               "    2:Clear\n"
                                               "    3:Status\n"
                                               "    4:Enable lpbk\n"
                                               "    5:Disable lpbk\n"
                                               "    6:init\n"
                                               ": ", 1, 0, 6);

    system("dmesg -n 7");

    phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(phy_port);
    switch (action) {
    case 6:
        /* Configure PHY core interrupts on the INTRP */
        ERET_COND(0 != manhattan_bcm54194_config_interrupt(phy_port, 1), -(__LINE__), "Failed.\n");
        break;
    case 0:
    case 1:
        /* Disable/Enable BCM54194 Link change Interrupt */
        ERET_COND(0 != manhattan_bcm54194_interrupt_set(phy_addr, action), -(__LINE__), "Failed.\n");
        break;
    case 2:
        /* Clear BCM54194 Link change Interrupt */
        ERET_COND(0 != manhattan_bcm54194_interrupt_clear(phy_addr), -(__LINE__), "Failed.\n");
        break;
    case 3:
        /* Read BCM54194 interrupt status */
        ERET_COND(0 != manhattan_bcm54194_interrupt_get(phy_addr, &int_status), -(__LINE__), "Failed.\n");
        break;
    case 4:
        /* Generate interrupts by enable System loopback */
        ERET_COND(0 != manhattan_bcm54194_interrupt_generate(phy_addr, 1, (NVRAM)->diagflag & D_EXT_LOOPBACK),
                -(__LINE__), "Failed.\n");
        break;
    case 5:
        /* Disable System loopback */
        ERET_COND(0 != manhattan_bcm54194_interrupt_generate(phy_addr, 0,(NVRAM)->diagflag & D_EXT_LOOPBACK),
                -(__LINE__), "Failed.\n");
        break;
    default:
        printf("ERROR: invalid action %d\n", action);
        break;
    } /* switch (action) */
    return 0;
}

static int bcm54194_intr_test(int arg)
{
    double   temp_curr = 0.0;
    uint16_t regv      = 0;

    printf("BCM541xx Interrupt Test, %s\n", __func__);
    manhattan_bcm54194_reset(1);

    #define __WAIT_FOR_INTR() do {                     \
        printf("%6d Wait for temperature calculation.\n%6s", __LINE__, " ");  \
        PROMPT_DELAY_MS(500, 10); \
    }while(0)

    //Trigger inter by temperature sensor

    //1, Unmask temperature intr to output to INTRP pin
    //   TOP_INTERRUPT_MASK Register                    0x82D
    //   VOLTAGE_TEMPERATURE_MONITOR_INTERRUPT Register 0x83B
    printf("%6d Unmask temperature intr  in 0x82D and 0x83B.\n", __LINE__);
    regv = ~(1 << 3);
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x82D,  regv), FAILED, "");
    regv = 0;
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x83B,  regv), FAILED, "");

    //2, Select temperature monitor only and release it
    //   VOLTAGE_TEMPERATURE_MONITOR_CONTROL Register 0x831
    printf("%6d Select temperature monitor only and release it in 0x83B.\n", __LINE__);
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x831, &regv), FAILED, "");
    regv &= ~0x7; // clear VTMON_SEL to 2'b00 = Temperature monitor and release the monitor
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x831,  regv), FAILED, "");
    __WAIT_FOR_INTR();

    //3, Get current temperature
    //   TEMPERATURE_MONITOR_VALUE Register           0x832
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x832, &regv), FAILED, "");
    temp_curr = 413.35 - (0.49055 * ((int)(regv & 0x3ff)));
    printf("%6d %-24s: %.3f Celcius Degree\n", __LINE__, "Current temperature", temp_curr);

    //4, Set threshold to +10/-10 to current temp
    //   TEMPERATURE_MONITOR_HIGH_THRESHOLD Register  0x833
    //   TEMPERATURE_MONITOR_LOW_THRESHOLD Register   0x834
    regv = (uint16_t)((413.5 - (temp_curr + 10.0)) / 0.49055);
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x833,  regv), FAILED, "");
    regv = (uint16_t)((413.5 - (temp_curr - 10.0)) / 0.49055);
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x834,  regv), FAILED, "");
    printf("%6d %-24s: [%.3f, %.3f]\n", __LINE__, "Set threshold to", temp_curr + 10, temp_curr - 10);

    //5, Read to clear intrs and check
    //   TOP_INTERRUPT_STATUS Register                0x82C
    printf("%6d Clear intr status.\n", __LINE__);
    __WAIT_FOR_INTR();
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x83B, &regv), FAILED, "");
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x82C, &regv), FAILED, "");
    printf("%6d Re-check intr status.\n", __LINE__);
    __WAIT_FOR_INTR();
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x83B, &regv), FAILED, "");
    ERET_COND(regv & (1 << 8), FAILED, "Temperature intr not cleared, reg-0x83B status:0x%04x (%s).\n",
            regv, __binary_dump_16(regv, NULL));
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x82C, &regv), FAILED, "");
    ERET_COND(regv != 0, FAILED, "Temperature intr not cleared, reg-0x82C status:0x%04x (%s).\n",
            regv, __binary_dump_16(regv, NULL));
    //5.1, Check external INTRP pin status
    if (_g_p_seahawks->intr) {
        printf("%6d Check externl INTRP pin status.\n", __LINE__);
        ERET_COND(0 != _g_p_seahawks->intr(_g_p_seahawks->priv, 0, (void *)0), FAILED, "Check INTRP pin failed.\n");
    }

    printf("%6d Adjust threshold to trigger intr.\n", __LINE__);
    //6, Get current temperature
    //   TEMPERATURE_MONITOR_VALUE Register           0x832
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x832, &regv), FAILED, "");
    temp_curr = 413.35 - (0.49055 * ((int)(regv & 0x3ff)));
    printf("%6d %-24s: %.3f Celcius Degree\n", __LINE__, "Current temperature", temp_curr);

    //4, Adjusting threshold to trigger intr
    //   TEMPERATURE_MONITOR_HIGH_THRESHOLD Register  0x833
    //   TEMPERATURE_MONITOR_LOW_THRESHOLD Register   0x834
    regv = (uint16_t)((413.5 - (temp_curr - 5.0)) / 0.49055);
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x833,  regv), FAILED, "");
    regv = (uint16_t)((413.5 - (temp_curr - 10.0)) / 0.49055);
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, 0x834,  regv), FAILED, "");
    printf("%6d %-24s: [%.3f, %.3f]\n", __LINE__, "Set threshold to", temp_curr - 5, temp_curr - 10);

    //8, Check intr status
    //   Check external INTRP pin status
    if (_g_p_seahawks->intr) {
        printf("%6d Check externl INTRP pin status.\n", __LINE__);
        ERET_COND(0 != _g_p_seahawks->intr(_g_p_seahawks->priv, 0, (void *)1), FAILED, "Check INTRP pin failed.\n");
    }
    //   TOP_INTERRUPT_STATUS Register                0x82C
    printf("%6d Check if intr occured.\n", __LINE__);
    __WAIT_FOR_INTR();
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x83B, &regv), FAILED, "");
    ERET_COND(!(regv & (1 << 0)), FAILED, "Temperature intr not triggered, reg-0x83B status:0x%04x (%s).\n",
            regv, __binary_dump_16(regv, NULL));
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, 0x82C, &regv), FAILED, "");
    ERET_COND(!(regv & (1 << 3)), FAILED, "Temperature intr not triggered, Intr status:0x%04x (%s).\n",
            regv, __binary_dump_16(regv, NULL));

    printf("PASSED.\n");
    return PASSED;
}


int manhattan_bcm54194_led_ctrl(unsigned int port, unsigned int led, unsigned int on_off)
{
    unsigned int       phya = MANHATTAN_BCM54194_PHYAD_PORT(port);
    unsigned int       rdba = led == 3 ? 0x1e : 0x1D;
    unsigned int       boff = led == 2 ? 7 : 0;
    const unsigned int wdth = 4;
    uint16_t           regv = 0;

    ERET_COND(0 != MHT_RDB_RD(phya, rdba, &regv), FAILED, "");
    regv &= ~(((1 << wdth) - 1) << boff);
    regv |= (!on_off ? 0xE : 0xF) << boff; //Negative output
    ERET_COND(0 != MHT_RDB_WR(phya, rdba, regv), FAILED, "");
    return PASSED;
}

static struct {
    char *name;
    int   port;
    int   led_port; //P1_1 led is used for P0 link, P0_1 led is for P1 link
    int   led_idx;
} _bcm54194_leds[] = {
    {
        "LED_PORT0B_LNK_GRN Port-0 Link LED(Green)",
        0,
        1,
        1,
    },

    {
        "LED_PORT1_LNK_GRN Port-1 Link LED(Green)",
        1,
        0,
        1,
    },

    {NULL, -1, -1}
};

int manhattan_bcm54194_led_test(int arg)
{
    int i    = 0;
    int j    = 0;
    int pt_s = 0;
    int pt_e = 0;
    int on   = 0;

    if(arg < 0) {
        pt_s = 0;
        pt_e = 1;
    } else {
        pt_s = arg;
        pt_e = arg;
    }

    for(i = 0; _bcm54194_leds[i].name; i++) {
        if (pt_s <= _bcm54194_leds[i].port && _bcm54194_leds[i].port <= pt_e) {
            for_each(j, on, int, 3, (int)0, (int)1, (int)0) {
                printf("..%s %s\n", _bcm54194_leds[i].name, on ? "On" : "Off");
                ERET_COND(PASSED != manhattan_bcm54194_led_ctrl(_bcm54194_leds[i].led_port, _bcm54194_leds[i].led_idx, on),
                    FAILED, "Failed.\n");
                switzer_mdelay(3000);
            }
        }
    }

    return PASSED;
}

int manhattan_bcm54194_led_util(int led, int on)
{
    ERET_COND(PASSED != manhattan_bcm54194_led_ctrl(_bcm54194_leds[led].led_port, _bcm54194_leds[led].led_idx, on),
            FAILED, "Failed.\n");

    return PASSED;
}
