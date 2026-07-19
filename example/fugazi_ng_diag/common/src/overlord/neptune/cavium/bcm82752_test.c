/* $Id: bcm82752_test.c,v 1.5 2018/10/03 09:53:57 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm82752_test.c,v $
*-----------------------------------------------------------------------------
* bcm82752_test.c - Diags Test for BCM 10G PHY bcm82752.
*
* June 2016, Bo Wang
*
* Copyright (c) 2016 - 2018 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  

#include "bcm82752_api.h"
#include "bcm82752_reg_def.h"
#include "bcm82752_test.h"
#include "platform_xfi.h"
#include "platform_ext_lpbk.h"
#include "sff_trans.h"
#include "bcm_common_defines.h"
#include "platform_sfp_cookie.h"
#include "platform_i2c.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

extern int bcm8275x_hw_init_done;
extern int is_item_available(void);
extern int miura_fw_download(void);
extern int miura_macsec_xfi_lrm_sr_config(int);
extern int miura_macsec_bypass_1000x(void);

static int BCM8275x_utility(int);
static void read_bcm8275x_phy_reg(void);
static void write_bcm8275x_phy_reg(void);
static int ten_g_phy_register_tests(int, bcm82752_intf_t, const reg_info_t *);
static int BCM8275x_fw_download(void);
static int BCM8275x_eye_diagram(void);
static int BCM82752_led_test(void);
static void BCM82757_led_test(void);
static int BCM8275x_register_test(void);
int neptune_cavium_xfi_lpbk_test(void);
int BCM8275x_internal_loopback_test(void);
int BCM8275x_external_loopback_test(void);
int nep_xfi_int_ext_loopback_test(void);
static int ten_g_macsec_test_main(int);
static int neptune_ten_g_macsec_test(void);
static int bcm82757_macsec_test(uint, uint);
static void dump_bcm8275x_standard_reg (void);
static int ten_g_phy_ptp1588_test_main (int);
static int neptune_ten_g_phy_ptp1588_test (void);
static void teng_phy_reset(void);     
static void dump_sfp_eeprom(void);
static void write_sfp_eeprom(void);
static int BCM8275x_xfi_lrm_sr_config (int);
static int BCM8275x_macsec_bypass_1000x(void);
static void bcm82757_loopback_setting (void);
static int bcm8275x_sfp_i2c_test_wrap(int);


static const reg_info_t bcm_82757_gen_cntrl_reg[] = {
    {"Main clock and reset ctrl",
     BCM82757_SFI_MAIN_CLK_RST_CTRL, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General control reg 1",
     BCM82757_SFI_GEN_CTRL_REG1, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General control reg 2",
     BCM82757_SFI_GEN_CTRL_REG2, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General control reg 3",
     BCM82757_SFI_GEN_CTRL_REG3, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_micro_boot_reg[] = {
    {"MDIO POR reg",
     BCM82757_SFI_MDIO_POR_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Micro Boot reg",
     BCM82757_SFI_MICRO_BOOT_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_chip_cntrl_reg[] = {
    {"Chip ID reg",
     BCM82757_SFI_CHIP_ID_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Chip Rev reg",
     BCM82757_SFI_CHIP_REV_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Chip Configuration reg",
     BCM82757_SFI_CHIP_CONFIG_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_module_ctrl_reg[] = {
    {"Module controller main control reg",
     BCM82757_SFI_MODULE_MAIN_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Module controller status reg",
     BCM82757_SFI_MODULE_MAIN_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_pad_cntrl_reg[] = {
    {"PAD mdio1 ctrl reg",
     BCM82757_SFI_PAD_MDIO1_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"PAD mdio2 ctrl reg",
     BCM82757_SFI_PAD_MDIO2_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"PAD adr0 ctrl reg",
     BCM82757_SFI_PAD_ADR0_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"PAD adr1 ctrl reg",
     BCM82757_SFI_PAD_ADR1_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"PAD adr2 ctrl reg",
     BCM82757_SFI_PAD_ADR2_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_chip_fw_reg[] = {
    {"Address for backdoor access to master code ram",
     BCM82757_SFI_MST_CODE_RAM_MEM_ADDR, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Address for backdoor access to master data ram",
     BCM82757_SFI_MST_DATA_RAM_MEM_ADDR, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Address for backdoor access to master vector table",
     BCM82757_SFI_MST_VECTOR_TBL_ADDR, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_lmi_reg[] = {
    {"LMI Reset Control",
     BCM82757_SFI_LMI_RESET_CTRL_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"LMI CMD", BCM82757_SFI_LMI_CMD_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"LMI Address", BCM82757_SFI_LMI_ADDR_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"LMI Data", BCM82757_SFI_LMI_DATA_REG, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82757_xfi_dev1_reg[] = {
    {"Scratch Pad 0", BCM82757_XFI_SCRATCH_PAD_REG0, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Scratch Pad 1", BCM82757_XFI_SCRATCH_PAD_REG1, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Scratch Pad 2", BCM82757_XFI_SCRATCH_PAD_REG2, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Scratch Pad 3", BCM82757_XFI_SCRATCH_PAD_REG3, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82752_sfi_dev1_reg[] = {
    {"PMD Control", BCM82752_SFI_PMD_CTRL_REG, READ_WRITE, {2}, 0x0000, 0x2040},
    {"PMD Status", BCM82752_SFI_PMD_STAT_REG, READ_ONLY,  {2}, 0x0000, 0x0082},
    {"PMD PHY ID 0", BCM82752_SFI_PMD_ID_0_REG, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PMD PHY ID 1", BCM82752_SFI_PMD_ID_1_REG, READ_ONLY,  {2}, 0x0000, 0x5250},
    {"PMD Speed Ability", BCM82752_SFI_PMD_SPEED_ABIL_REG, READ_ONLY, {2}, 0x0000, 0x0011},
    {"PMD Devices in Package 1", BCM82752_SFI_PMD_DEVICE_IN_PAK_1_REG, READ_ONLY,  {2}, 0x0000, 0x008A},
    {"PMD Devices in Package 2", BCM82752_SFI_PMD_DEVICE_IN_PAK_2_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Control 2", BCM82752_SFI_PMD_CTRL_2_REG, READ_WRITE,  {2}, 0x003F, 0x0008},
    {"PMD Status 2", BCM82752_SFI_PMD_STAT_2_REG, READ_ONLY, {2}, 0x0000, 0xBF01},
    {"PMD Transmit Disable", BCM82752_SFI_PMD_TRANSMIT_DIS_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Receive Signal Detect", BCM82752_SFI_PMD_RX_SIG_DETECT_REG, READ_ONLY, {2}, 0x0000, 0x0000},
    {"PMD Extended Ability", BCM82752_SFI_PMD_EXT_ABIL_REG, READ_ONLY,  {2}, 0x0000, 0x4002},
    {"PMD Organizationally Unique ID 0", BCM82752_SFI_PMD_ORG_UNI_ID_0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PMD Organizationally Unique ID 1", BCM82752_SFI_PMD_ORG_UNI_ID_1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82752_sfi_dev3_reg[] = {
    {"PCS Control 1", BCM82752_SFI_PCS_CTRL_1_REG, READ_WRITE, {2}, 0x0000, 0x2040},
    {"PCS Status 1", BCM82752_SFI_PCS_STAT_1_REG, READ_ONLY,  {2}, 0x0000, 0x8020},
    {"PCS PHY ID 0", BCM82752_SFI_PCS_ID_0_REG, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PCS PHY ID 1", BCM82752_SFI_PCS_ID_1_REG, READ_ONLY,  {2}, 0x0000, 0x5250},
    {"PCS Speed Ability", BCM82752_SFI_PCS_SPEED_ABIL_REG, READ_ONLY, {2}, 0x0000, 0x0001},
    {"PCS Devices in Package 1", BCM82752_SFI_PCS_DEVICE_IN_PAK_1_REG, READ_ONLY,  {2}, 0x0000, 0x0005},
    {"PCS Devices in Package 2", BCM82752_SFI_PCS_DEVICE_IN_PAK_2_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PCS Control 2", BCM82752_SFI_PCS_CTRL_2_REG, READ_WRITE, {2}, 0x0003, 0x0000},
    {"PCS Status 2", BCM82752_SFI_PCS_STAT_2_REG, READ_ONLY,  {2}, 0x0000, 0x8401},
    {"PCS 10GBASE-R EEE Capability", BCM82752_SFI_PCS_10G_R_EEE_CAP_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS Wake Error Counter", BCM82752_SFI_PCS_WAKE_ERR_CNT_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R STATUS", BCM82752_SFI_PCS_10G_R_STAT_REG, READ_ONLY, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R STATUS 2", BCM82752_XFI_PCS_10G_R_STAT_2_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A0", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A1", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A2", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A2_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A3", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A3_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B0", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B1", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B2", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B2_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B3", BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B3_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Test Control", BCM82752_SFI_PCS_10G_R_JIT_TEST_CTRL_REG, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Test Error Count", BCM82752_SFI_PCS_10G_R_JIT_TEST_ERR_CNT_REG, READ_ONLY, {2}, 0x0000, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82752_xfi_dev1_reg[] = {
    {"PMD Control", BCM82752_XFI_PMD_CTRL_REG, READ_WRITE, {2}, 0x0000, 0x2040},
    {"PMD Status", BCM82752_XFI_PMD_STAT_REG, READ_ONLY,  {2}, 0x0000, 0x0080},
    {"PMD PHY ID 0", BCM82752_XFI_PMD_ID_0_REG, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PMD PHY ID 1", BCM82752_XFI_PMD_ID_1_REG, READ_ONLY,  {2}, 0x0000, 0x5250},
    {"PMD Speed Ability", BCM82752_XFI_PMD_SPEED_ABIL_REG, READ_ONLY, {2}, 0x0000, 0x0011},
    {"PMD Devices in Package 1", BCM82752_XFI_PMD_DEVICE_IN_PAK_1_REG, READ_ONLY,  {2}, 0x0000, 0x008A},
    {"PMD Devices in Package 2", BCM82752_XFI_PMD_DEVICE_IN_PAK_2_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Status 2", BCM82752_XFI_PMD_STAT_2_REG, READ_ONLY, {2}, 0x0000, 0xBFE1},
    {"PMD Transmit Disable", BCM82752_XFI_PMD_TRANSMIT_DIS_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Receive Signal Detect", BCM82752_XFI_PMD_RX_SIG_DETECT_REG, READ_ONLY, {2}, 0x0000, 0x0000},
    {"PMD Extended Ability", BCM82752_XFI_PMD_EXT_ABIL_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Organizationally Unique ID 0", BCM82752_XFI_PMD_ORG_UNI_ID_0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PMD Organizationally Unique ID 1", BCM82752_XFI_PMD_ORG_UNI_ID_1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_82752_xfi_dev3_reg[] = {
    {"PCS Control 1", BCM82752_XFI_PCS_CTRL_1_REG, READ_WRITE, {2}, 0x0000, 0x2040},
    {"PCS Status 1", BCM82752_XFI_PCS_STAT_1_REG, READ_ONLY,  {2}, 0x0000, 0x8020},
    {"PCS PHY ID 0", BCM82752_XFI_PCS_ID_0_REG, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PCS PHY ID 1", BCM82752_XFI_PCS_ID_1_REG, READ_ONLY,  {2}, 0x0000, 0x5250},
    {"PCS Speed Ability", BCM82752_XFI_PCS_SPEED_ABIL_REG, READ_ONLY, {2}, 0x0000, 0x0001},
    {"PCS Devices in Package 1", BCM82752_XFI_PCS_DEVICE_IN_PAK_1_REG, READ_ONLY,  {2}, 0x0000, 0x0005},
    {"PCS Devices in Package 2", BCM82752_XFI_PCS_DEVICE_IN_PAK_2_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PCS Control 2", BCM82752_XFI_PCS_CTRL_2_REG, READ_WRITE, {2}, 0x0003, 0x0000},
    {"PCS Status 2", BCM82752_XFI_PCS_STAT_2_REG, READ_ONLY,  {2}, 0x0000, 0x8401},
    {"PCS Organizationally Unique ID 0", BCM82752_XFI_PCS_ORG_UNI_ID_0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS Organizationally Unique ID 1", BCM82752_XFI_PCS_ORG_UNI_ID_1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-X STATUS", BCM82752_XFI_PCS_10G_X_STAT_REG, READ_ONLY, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R STATUS", BCM82752_XFI_PCS_10G_R_STAT_REG, READ_ONLY, {2}, 0x0000, 0x000C},
    {"PCS 10GBASE-R STATUS 2", BCM82752_XFI_PCS_10G_R_STAT_2_REG, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A0", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A1", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A2", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A2_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A3", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A3_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B0", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B0_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B1", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B1_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B2", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B2_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B3", BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B3_REG, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Test Control", BCM82752_XFI_PCS_10G_R_JIT_TEST_CTRL_REG, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Test Error Count", BCM82752_XFI_PCS_10G_R_JIT_TEST_ERR_CNT_REG, READ_ONLY, {2}, 0x0000, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const bcm_phy_regs_t bcm82752_phy_standard_reg_tbl[] = {
    {"XFI",    BCM82752_XFI_INTF, bcm_82752_xfi_dev1_reg},
    {"XFI",    BCM82752_XFI_INTF, bcm_82752_xfi_dev3_reg},
    {"SFI",    BCM82752_SFI_INTF, bcm_82752_sfi_dev1_reg},
    {"SFI",    BCM82752_SFI_INTF, bcm_82752_sfi_dev3_reg},
};

#define BCM82752_NUM_PHY_INTF (sizeof(bcm82752_phy_standard_reg_tbl) /      \
                               sizeof(struct bcm_phy_regs_t_))

static const bcm_phy_regs_t bcm82757_phy_direct_reg_tbl[] = {
    {"GEN CNTRL",   BCM82752_SFI_INTF, bcm_82757_gen_cntrl_reg},
    {"MICRO BOOT",  BCM82752_SFI_INTF, bcm_82757_micro_boot_reg},
    {"CHIP CNTRL",  BCM82752_SFI_INTF, bcm_82757_chip_cntrl_reg},
    {"MODULE CTRL", BCM82752_SFI_INTF, bcm_82757_module_ctrl_reg},
    {"PAD CNTRL", BCM82752_SFI_INTF, bcm_82757_pad_cntrl_reg},
    {"CHIP FW", BCM82752_SFI_INTF, bcm_82757_chip_fw_reg},
    {"LMI", BCM82752_SFI_INTF, bcm_82757_lmi_reg},
};

#define BCM82757_NUM_PHY_INTF (sizeof(bcm82757_phy_direct_reg_tbl) /      \
                               sizeof(struct bcm_phy_regs_t_))

/******************************************************************************
 *  List of Menu used for XFI BCM82752
 *****************************************************************************/
static submenu_xtable_t BCM8275x_tests_submenu_table[] = {
   {"10G PHY BCM8275x Utility", (type_t(*)())BCM8275x_utility,   FALSE,
    0, NULL, 0, (type_t(*)())BCM8275x_utility,   TRUE},
   {"10G PHY BCM8275x Firmware Download", (type_t(*)())BCM8275x_fw_download,   0,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"10G PHY BCM8275x Register Test", (type_t(*)())BCM8275x_register_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"10G PHY Port0 SFP+ I2C Test", (type_t(*)())bcm8275x_sfp_i2c_test_wrap,   3,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"10G PHY Port0 SFP+ I2C Test", (type_t(*)())bcm8275x_sfp_i2c_test_wrap,   4,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"10G PHY BCM8275x SFP+ Ext/Internal  Loopback Test", (type_t(*)())nep_xfi_int_ext_loopback_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for XFI BCM82752
 *****************************************************************************/
static submenu_xtable_t BCM8275x_util_items[] = {
    {"10G PHY BCM8275x Firmware Download", (type_t(*)())BCM8275x_fw_download, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump PHY BCM8275x registers", (type_t(*)())dump_bcm8275x_standard_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Read PHY BCM8275x Registers", (type_t(*)())read_bcm8275x_phy_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Write PHY BCM8275x Register", (type_t(*)())write_bcm8275x_phy_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"XFI/SFP+ GE PHY loopback util", (type_t(*)()) neptune_ten_g_phy_lpbk_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Cavium XFI MAC loopback test", (type_t(*)()) neptune_cavium_xfi_lpbk_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"10G PHY BCM8275x Internal Loopback Test", (type_t(*)())BCM8275x_internal_loopback_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"10G PHY BCM8275x SFP+ External Loopback Test", (type_t(*)())BCM8275x_external_loopback_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"10G PHY BCM82757 MACsec Test", (type_t(*)())ten_g_macsec_test_main, FALSE,
     0, (PFT)is_item_available, 0, (type_t(*)())ten_g_macsec_test_main, TRUE},
    {"10G PHY BCM8275x PTP1588 Test", (type_t(*)())ten_g_phy_ptp1588_test_main, FALSE,
     0, (PFT)is_item_available, 0, (type_t(*)())ten_g_phy_ptp1588_test_main, TRUE},
    {"PHY BCM8275x reset ", (type_t(*)())teng_phy_reset, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump SFP+ EEPROM", (type_t(*)())dump_sfp_eeprom, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write SFP+ EEPROM", (type_t(*)())write_sfp_eeprom, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10G PHY BCM8275x Eye Diagram", (type_t(*)())BCM8275x_eye_diagram, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10G PHY LED Test", (type_t(*)())BCM82752_led_test, 0, 0,
     (PFT)is_bcm82752, 0, (type_t(*)())0, 0},
    {"10G PHY LED Test", (type_t(*)())BCM82757_led_test, 0, 0,
     (PFT)(not_bcm82752), 0, (type_t(*)())0, 0},
    {"10G PHY XFI-LRM Config", (type_t(*)())BCM8275x_xfi_lrm_sr_config, bcm_pm_InterfaceLRM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10G PHY XFI-SR Config", (type_t(*)())BCM8275x_xfi_lrm_sr_config, bcm_pm_InterfaceSR, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10G PHY 1GBASE Config", (type_t(*)())BCM8275x_macsec_bypass_1000x, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10G PHY loopback Config", (type_t(*)())bcm82757_loopback_setting, 0, 0,
     (PFT)(not_bcm82752), 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM8275X_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM8275x_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))

#define BCM8275X_TESTS_UTIL_SIZE (sizeof(BCM8275x_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM8275x_tests_primary_items[BCM8275X_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t BCM8275x_tests_secondary_items[BCM8275X_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM82752_tests_primary_util_items[BCM8275X_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t BCM8275x_tests_secondary_util_items[BCM8275X_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * XFI BCM82752 Utils submenu
 *****************************************************************************/
menuinfo_t BCM8275x_util_menu = {
    "10G PHY BCM8275x Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    BCM82752_tests_primary_util_items,
};
menuinfo_t *BCM8275x_util_menup = &BCM8275x_util_menu;

menuinfo_t BCM8275x_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    BCM8275x_tests_primary_items,
};
menuinfo_t *BCM8275x_submenup = &BCM8275x_subtest_menu;

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
int ten_g_bcm8275x_test (int show_menu)
{
    build_primary_submenu(BCM8275x_tests_submenu_table,
                          BCM8275X_TESTS_SUBMENU_TABLE_SIZE,
                          "10G PHY BCM8275x", &BCM8275x_submenup);
    build_secondary_submenu(BCM8275x_tests_submenu_table,
                            BCM8275X_TESTS_SUBMENU_TABLE_SIZE,
                            BCM8275x_tests_secondary_items);

    if (show_menu) {
        menu(BCM8275x_submenup, BCM8275x_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(BCM8275x_submenup);
}
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : BCM8275x_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM8275x_utility (int show_menu)
{
    build_primary_submenu(BCM8275x_util_items, BCM8275X_TESTS_UTIL_SIZE,
                          "10G PHY BCM8275x Utilities Menu", &BCM8275x_util_menup);
    build_secondary_submenu(BCM8275x_util_items, BCM8275X_TESTS_UTIL_SIZE,
                            BCM8275x_tests_secondary_util_items);

    menu(BCM8275x_util_menup, BCM8275x_tests_secondary_util_items, '\0' );

    return (PASSED);
}

static int macsec_xfi_eth_port_list[] = {XFI0, XFI1};
static int macsec_xfi_eth_speed_list[] = {SPEED_10G};

#define F_GRP        (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E      (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL        (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t ten_g_macsec_tests_submenu_table[] = {
    {"MACsec test on BCM82752 PHY", (type_t(*)())neptune_ten_g_macsec_test,   0,
        F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define TEN_G_MACSEC_TESTS_SUBMENU_TABLE_SIZE (sizeof(ten_g_macsec_tests_submenu_table) / \
                                         sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ten_g_macsec_tests_primary_items[TEN_G_MACSEC_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];
static mitem_t ten_g_macsec_tests_secondary_items[TEN_G_MACSEC_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];

menuinfo_t ten_g_macsec_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ten_g_macsec_tests_primary_items,
};
menuinfo_t *ten_g_macsec_submenup = &ten_g_macsec_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: macsec_ten_g_test_main
 *      This is the entry point for the macsec main test.
 *
 * Input:  dummy
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int ten_g_macsec_test_main (int dummy)
{

    build_primary_submenu(ten_g_macsec_tests_submenu_table,
                          TEN_G_MACSEC_TESTS_SUBMENU_TABLE_SIZE,
                          "MACsec", &ten_g_macsec_submenup);
    build_secondary_submenu(ten_g_macsec_tests_submenu_table,
                            TEN_G_MACSEC_TESTS_SUBMENU_TABLE_SIZE,
                            ten_g_macsec_tests_secondary_items);

    menu(ten_g_macsec_submenup, ten_g_macsec_tests_secondary_items, '\0' );

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: neptune_macsec_test
 *      a testing wrapper for macsec test
 *
 * Input: NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int neptune_ten_g_macsec_test (void)
{
    uint port, port_curr, rc = FAILED;
    uint speed, speed_curr;
    uint speed_cnt, port_cnt;
    uint try, retry_limit = 2;

    testname("BCM82757 MACsec");

    port_cnt = sizeof(macsec_xfi_eth_port_list) / sizeof(int);
    speed_cnt = sizeof(macsec_xfi_eth_speed_list) / sizeof(int);

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        port = macsec_xfi_eth_port_list[port_curr];

        for (speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
           speed = macsec_xfi_eth_speed_list[speed_curr];

           prpass(testpass, "Test port-%d speed-%d, ", port, speed);
           for (try=0; try < retry_limit; try++) {
               rc = bcm82757_macsec_test(port,speed);
               if ((rc == PASSED) || (try == (retry_limit - 1))) {
                    break;
               } else {
                    printf("####### retry the test #########\n");
                    //reset_quad_phy();
                    //bcm82752_soft_reset();
               }
           }

           if (rc != PASSED) {
               cterr('f',0,"MACsec test failed on port%d with spd%d\n", port, speed);
               return(FAILED);
           }

        }  /* for speed */
    }  /* for port */

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: check_status
 *
 * Description: read macsec counter to ensure packets are encrypted,
 *              decrypted and authenticated.
 *
 * Input:  port - setup port
 *         phy_addr - phy address
 *         manually - 1 for chech status manually
 *
 * Output: PASS/FAILED
 *
 *------------------------------------------------------------------
 */
static int check_status (uint port, uint phy_addr)
{
    /* check IGR_OK and IGR_MISS */

    /* IGR is not OK, packet is not decrypted or authenticated */

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: init_macsec
 *
 * Description: we configure macsec related setting on its mem/reg.
 *
 * Input:  port - setup port
 *         phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
static void init_macsec (uint port, uint phy_addr)
{
    return;
}

/*------------------------------------------------------------------
 *
 * Function: clr_macsec_cnt
 *
 * Description: cleanup macsec counter(statistic) via reading them.
 *
 * Input:  port - setup port
 *         phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
static void clr_macsec_cnt (uint port, uint phy_addr)
{

}

/*------------------------------------------------------------------
 *
 * Function: bcm82757_macsec_test
 *
 * Description: testing macsec on 88E1548P.
 *              steps: turn on macsec on PHY,
 *              disable drop_bad_tag, send packets and
 *              check the statistic bit on PHY.
 *
 * Input:  port - test port
 *         speed - test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int bcm82757_macsec_test (uint port, uint speed)
{
	int *eth_mapping_port_num = eth_mapping_sfp_num;
    uint phy_addr = eth_mapping_port_num[port], result = FAILED;

    prpass(testpass, "Test port-%d speed-%d, ", port, speed);

    /* init macsec setting and cleanup macsec counter(statistic) */
    init_macsec(port, phy_addr);
    clr_macsec_cnt(port, phy_addr);

    /* using internal loopback configuration so far */
    set_ten_g_phy_int_lpbk(SEL_PORT_XFI, port, speed, BCM82752_LOOPBACK_PCS);

    /* Enable macsec */

    msleep(1000);

    /* send packets */
    result = neptune_set_packet(SEL_PORT_XFI, port, speed);

    /* Disable MACsec and loopback before leaving test. */
    set_ten_g_phy_int_lpbk(SEL_PORT_XFI, port, speed, BCM82752_LOOPBACK_NONE);

    if (result != PASSED) {
        printf("xfi_set_packet failed %s\n",__FUNCTION__);
        return (result);
    }

    result |= check_status(port, phy_addr);

    if (result != PASSED) {
       printf("Statistics have failure case.\n");
       return (result);
    }

    return (result);
}

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t ten_g_phy_ptp1588_tests_submenu_table[] = {
    {"PTP1588 test on BCM82752 PHY", (type_t(*)())neptune_ten_g_phy_ptp1588_test,
     0, F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define TEN_G_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE (sizeof(ten_g_phy_ptp1588_tests_submenu_table) / \
                                                    sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ten_g_phy_ptp1588_tests_primary_items[TEN_G_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE +
                                                     MAX_BASE_ITEMS];
static mitem_t ten_g_phy_ptp1588_tests_secondary_items[TEN_G_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE +
                                                       MAX_BASE_ITEMS];

menuinfo_t ten_g_phy_ptp1588_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ten_g_phy_ptp1588_tests_primary_items,
};
menuinfo_t *ten_g_phy_ptp1588_submenup = &ten_g_phy_ptp1588_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: ten_g_phy_ptp1588_test_main
 *      This is the entry point for the macsec main test.
 *
 * Input:  dummy
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int ten_g_phy_ptp1588_test_main (int dummy)
{

    build_primary_submenu(ten_g_phy_ptp1588_tests_submenu_table,
                          TEN_G_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE,
                          "PTP1588", &ten_g_phy_ptp1588_submenup);
    build_secondary_submenu(ten_g_phy_ptp1588_tests_submenu_table,
                            TEN_G_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE,
                            ten_g_phy_ptp1588_tests_secondary_items);

    menu(ten_g_phy_ptp1588_submenup, ten_g_phy_ptp1588_tests_secondary_items,
         '\0' );

    return(PASSED);
}

static int bcm8275x_sfp_i2c_test_wrap(int sfp)
{
    int rc = FAILED, port = sfp - OVLD_CAVIUM_TWSI_SFP2;
    char *tname = "BCM8275x sfp+ i2c";
    testname("%s port %d", tname, port);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM PHY port %d sfp+ i2c test\n", port);
    }

    rc = bcm_sfp_i2c_test(sfp);
    if (rc != PASSED) {
        cterr('f', 0, "bcm_sfp_i2c_test failed on port", port);
        return (rc);
    } else {
        prpass(testpass, "port %d passed, ", port);
    }

    return (rc);
}

/*
 * Function: read_bcm8275x_phy_reg
 *
 * Description: Utility to do peek and poke to 10G PHY registers
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm8275x_phy_reg (void)
{
    int port_max = 1;
    int port_min = 0;
    uint rdval;
    uint dev_id = BCM82752_DEV_PMA, portnum, phy_intf;
    uint phy_addr, regnum, regnum_max = 0xFFFFFFFF;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface(XFI:0, SFI:1) ", 0, 0, 1);
    dev_id = gethex_answer("\nEnter DEV ID(PMA/PMD:1, PCS:3, 1GbE:7) ",
                            BCM82752_DEV_PMA, BCM82752_DEV_PMA, BCM82752_DEV_1GBE);
    regnum = gethex_answer("\nEnter PHY reg number(0x0 - 0xFFFFFFFF)", 0, 0, regnum_max);

    phy_addr = te_port_mapping_phy_addr[portnum];

    if (is_bcm82752()) {
        /* Switch to XFI/SFI mode */
        bcm82752_xfi_sfi_access(phy_addr, phy_intf);
        rdval = bcm82752_reg_rd(phy_addr, dev_id, regnum);
    } else {
        rdval = bcm82757_miura_reg_rd(phy_addr, phy_intf, dev_id, regnum);
    }
    if (rdval < 0) {
        printf("Failed to read 10GE PHY, TE%d, %d.0x%x\n", portnum, dev_id, regnum);
    } else {
        printf("TE%d, %d.%#.4x = %#.8x \n", portnum, dev_id, regnum, rdval);
    }
}

/*
 * Function: write_bcm8275x_phy_reg
 *
 * Description: Utility to do peek and poke to 10G PHY registers
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm8275x_phy_reg (void)
{
    int port_max = 1;
    int port_min = 0;
    ushort wrval;
    uint dev_id = BCM82752_DEV_PMA, portnum, phy_intf;
    uint rc, phy_addr, regnum, regnum_max = 0xFFFFFFFF;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface(XFI:0, SFI:1) ", 0, 0, 1);
    dev_id = gethex_answer("\nEnter DEV ID(PMA/PMD:1, PCS:3, 1GbE:7) ",
                            BCM82752_DEV_PMA, BCM82752_DEV_PMA, BCM82752_DEV_1GBE);
    regnum = gethex_answer("\nEnter PHY reg number(0x0 - 0xFFFFFFFF)", 0, 0, regnum_max);
    wrval = gethex_answer("Enter value:", 0, 0, regnum_max);
    phy_addr = te_port_mapping_phy_addr[portnum];

    if (is_bcm82752()) {
        /* Switch to XFI/SFI mode */
        bcm82752_xfi_sfi_access(portnum, phy_intf);
        rc = bcm82752_reg_wr(portnum, dev_id, regnum, wrval);
    } else {
        rc = bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, regnum, wrval);
    }
    if (rc != PASSED) {
        printf("Failed to write 10GE PHY, TE%d, %d.%#.8x\n", portnum, dev_id, regnum);
    } else {
        printf("TE%d, %d.%#.8x <-- %#.8x \n", portnum, dev_id, regnum, wrval);
    }
}

void reset_platform_ext_dev (int bit)
{
    assert(dash_fpga);

    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    sys->ext_rst |= bit;
}

void
unreset_platform_ext_dev (int bit)
{
    assert(dash_fpga);
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    sys->ext_rst &= ~bit;
}

/*
 * Function: bcm82757_loopback_setting
 *
 * Description: bcm82757 loopback configuration
 *
 * Input: none
 *
 * Return: none
 */
static void bcm82757_loopback_setting (void)
{
    uint lb_mode = 1, enable = TRUE, side;

    printf("1 - digital loopback (Deeper)\n");
    printf("2 - remote loopback (Shallow)\n");
    lb_mode = getdec_answer("\nEnter loopback mode ", 1, 0, 10);

    enable = getdec_answer("Enable:1, Disable:0 ", 0, 0, 1);
    side = getdec_answer("line:0, system:1 ", 0, 0, 1);
    bcm82757_config_loopback(side, lb_mode, enable);
}

/******************************************************************************
 *
 * Function: BCM8275x_xfi_lrm_sr_config
 *
 * Description: This function performs the BCM82757 XFI-LRM or XFI-SR configuration.
 *
 * Inputs      :
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM8275x_xfi_lrm_sr_config (int bcm_pm_intf)
{
	int rc = FAILED;

	printf("BCM INFO - 10G PHY XFI LRM/SR Configuration.\n");

    if (!is_bcm82752()) {
        /* Reset 10G PHY */
        reset_platform_ext_dev(0x0800);
        msleep(10);
        unreset_platform_ext_dev(0x0800);
        msleep(10);

        rc = miura_macsec_xfi_lrm_sr_config(bcm_pm_intf);
    }
    return (rc);
}

static int BCM8275x_macsec_bypass_1000x()
{
	int rc = FAILED;

	printf("BCM INFO - 10G PHY 1GBASE Configuration.\n");

    if (!is_bcm82752()) {
        /* Reset 10G PHY */
        reset_platform_ext_dev(0x0800);
        msleep(10);
        unreset_platform_ext_dev(0x0800);
        msleep(10);

        rc = miura_macsec_bypass_1000x();
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: BCM8275x_fw_download
 *
 * Description: This function performs the BCM82752 firmware download and init.
 *
 * Inputs      :
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM8275x_fw_download (void)
{
	printf("BCM INFO - 10G PHY Firmware Download.\n");

    if (is_bcm82752()) {
        int ix = 0;
        int *port_list = te_port_mapping_phy_addr;
        int port_cnt = sizeof(port_list) / sizeof(int);

        for (ix = 0; ix < port_cnt; ix++) {
            if (bcm82752_init(port_list[ix]) != PASSED) {
                cterr('f', 0, "10G PHY FW Download failed");
                return (FAILED);
            }
        }
    } else {
#ifdef DEBUG
        if (bcm8275x_hw_init_done == 1) {
    	    printf("\n10 PHY firmware already downloaded !!!\n");
    	    return (0);
        }
#endif
        /* Reset 10G PHY */
        reset_platform_ext_dev(0x0800);
        msleep(10);
        unreset_platform_ext_dev(0x0800);
        msleep(10);

        if (miura_fw_download() != PASSED) {
            return (FAILED);
        }
        bcm8275x_hw_init_done = 1;
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: BCM8275x_eye_diagram
 *
 * Description: This function performs the BCM82752 firmware download and init.
 *
 * Inputs      :
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM8275x_eye_diagram (void)
{
	int rc = FAILED;

	printf("BCM INFO - 10G PHY Eye Scan.\n");

    if (bcm8275x_hw_init_done != 1) {
        printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
        return (0);
    }

    if (is_bcm82752()) {
        rc = quadra28_eye_diagram();
    } else {
        rc = miura_eye_diagram();
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: BCM82752_led_test
 *
 * Description: This function performs the BCM82752 LED test.
 *
 * Inputs      :
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM82752_led_test (void)
{
	int rc = FAILED, ix = 0, dev_id = BCM82752_DEV_PMA;
    int *port_list = te_port_mapping_phy_addr;
    int port_cnt = sizeof(port_list) / sizeof(int);
    uint16_t opt_digital_ctrl_reg[] = {0, 0}, led_ctrl_reg[] = {0, 0}, offset;

    if (bcm8275x_hw_init_done != 1) {
        printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
        return (0);
    }

	printf("Please connect TE0 and TE1 to each other by SFP+ cable.\n");

    /* Save BCM82752 LED register original value */
    for (ix = 0; ix < port_cnt; ix++) {
        offset = BCM82752_LED_CONTROL_0_REG;
        if ((led_ctrl_reg[ix] = bcm82752_reg_rd(port_list[ix], dev_id, offset)) < 0) {
            printf("Failed to read 10GE PHY, TE%d, %d.0x%x\n", port_list[ix], dev_id, offset);
        }

        offset = BCM82752_PCS_OPTICS_DIGITAL_CONTROL_REG;
        if ((opt_digital_ctrl_reg[ix] = bcm82752_reg_rd(port_list[ix], dev_id, offset)) < 0) {
            printf("Failed to read 10GE PHY, TE%d, %d.0x%x\n", port_list[ix], dev_id, offset);
        }
    }

    /* Config BCM82752 in LED mode */
    for (ix = 0; ix < port_cnt; ix++) {
        offset = BCM82752_LED_CONTROL_0_REG;
        rc = bcm82752_reg_wr(port_list[ix], dev_id, offset, 0x1);
        if (rc != PASSED) {
            printf("Failed to write 10GE PHY, TE%d, %d.0x%x\n", port_list[ix], dev_id, offset);
        }

        offset = BCM82752_PCS_OPTICS_DIGITAL_CONTROL_REG;
        rc = bcm82752_reg_wr(port_list[ix], dev_id, offset, 0x60);
        if (rc != PASSED) {
            printf("Failed to write 10GE PHY, TE%d, %d.0x%x\n", port_list[ix], dev_id, offset);
        }
    }

    /* send packet from TE0 to TE1 */
    system("ifconfig xfi0 10.10.10.10");
    system("ifconfig xfi1 10.10.10.11");
    msleep(10);

    system("ping -c5 -I xfi0 10.10.10.11");

    system("ifconfig xfi0 down");
    system("ifconfig xfi1 down");

    /* Restore BCM82752 LED register value */
    for (ix = 0; ix < port_cnt; ix++) {
        offset = BCM82752_LED_CONTROL_0_REG;
        rc = bcm82752_reg_wr(port_list[ix], dev_id, offset, led_ctrl_reg[ix]);
        if (rc != PASSED) {
            printf("Failed to write 10GE PHY, TE%d, %d.0x%x\n", port_list[ix], dev_id, offset);
        }

        offset = BCM82752_PCS_OPTICS_DIGITAL_CONTROL_REG;
        rc = bcm82752_reg_wr(port_list[ix], dev_id, offset, opt_digital_ctrl_reg[ix]);
        if (rc != PASSED) {
            printf("Failed to write 10GE PHY, TE%d, %d.0x%x\n", port_list[ix], dev_id, offset);
        }
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: BCM82757_led_test
 *
 * Description: This function performs the BCM82757 LA LED test.
 *
 * Inputs      :
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static void BCM82757_led_test (void)
{
	int phy_addr = 0, dev_id = BCM82752_DEV_PMA, phy_intf = BCM82752_SFI_INTF;

    if (bcm8275x_hw_init_done != 1) {
        printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
        return;
    }

	printf("Please connect TE0 and TE1 to each other by SFP+ cable.\n");
    
    /* Save BCM82752 LED register original value */
    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PAD_GPIO0_0_CTRL_REG, 0x80E2);
    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PAD_GPIO0_1_CTRL_REG, 0x80E2);

    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PM_LED_PARAMS_REG, 0x10FF);
    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PM_LED_MODE_REG, 0x0A48);

    /* send packet from TE0 to TE1 */
    system("ifconfig xfi0 10.10.10.10");
    system("ifconfig xfi1 10.10.10.11");
    msleep(10);

    system("ping -c5 -I xfi0 10.10.10.11");

    system("ifconfig xfi0 down");
    system("ifconfig xfi1 down");

    /* Restore BCM82752 LED register value */
    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PAD_GPIO0_0_CTRL_REG, 0x00E5);
    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PAD_GPIO0_1_CTRL_REG, 0x00E5);

    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PM_LED_PARAMS_REG, 0x10FF);
    bcm82757_miura_reg_wr(phy_addr, phy_intf, dev_id, BCM82757_PM_LED_MODE_REG, 0x07C8);

    return;
}

/*******************************************************************************
 *
 * Function: ten_g_phy_register_tests
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
static int ten_g_phy_register_tests (int phy_addr, bcm82752_intf_t intf, const reg_info_t *reg_ptr)
{
    uint32_t ix;
    uint16_t retval = PASSED;
    uint16_t data, temp, tst_offset, save_val, readval = 0x0;
    int dev_id;

    if (is_bcm82752()) {
        /* Switch to XFI/SFI register space */
        bcm82752_xfi_sfi_access(phy_addr, intf);
    }

    while (reg_ptr->size.size != 0) {

    	dev_id = (reg_ptr->offset) >> 16;
        save_val = bcm82752_reg_rd(phy_addr, dev_id, ((reg_ptr->offset) & 0xFFFF));
        if (save_val < 0) {
            cterr('f', 0, "%s(): Error reading %s register offset %d.%#x"
                  "phy_addr %d\n", __FUNCTION__,  reg_ptr->name, dev_id,
                  (reg_ptr->offset & 0xFFFF), phy_addr);
            return (FAILED);
        }

        if (reg_ptr->type == READ_WRITE) {

            tst_offset = (reg_ptr->offset) & 0xFFFF;

            /*
             * ripple 1 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {

                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                /* Write to register under test */

                retval = bcm82752_reg_wr(phy_addr, dev_id, tst_offset, temp);
                /* Read back */
                if (retval == PASSED) {
                    readval = bcm82752_reg_rd(phy_addr, dev_id, tst_offset);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %d.%#x, phy_addr %d,Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, dev_id, tst_offset, phy_addr,
                          temp, readval);
                    return (FAILED);
                }
            }

            /*
             * ripple 0 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                temp = (~(1 << ix)) & reg_ptr->mask;
                /* Write to register under test */
                retval = bcm82752_reg_wr(phy_addr, dev_id, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    readval = bcm82752_reg_rd(phy_addr, dev_id, tst_offset);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %d.%#x, phy_addr %d, Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, dev_id, tst_offset, phy_addr,
                          temp, readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = NEP_PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                /* Write to register under test */
                retval = bcm82752_reg_wr(phy_addr, dev_id, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    readval = bcm82752_reg_rd(phy_addr, dev_id, tst_offset);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Pattern test failed when accessing %s "
                          "Register offset %d.%#x phy_addr %d, Expect %#x, "
                          "Read %#x", __FUNCTION__, reg_ptr->name, dev_id, tst_offset,
                          phy_addr, temp, readval);
                    return (retval);
                }

                data = ~NEP_PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            retval = bcm82752_reg_wr(phy_addr, dev_id, tst_offset, save_val);
            if (retval == FAILED) {
                cterr('f', 0, "%s(): Error restoring %s register "
                      "offset %d.%#x, phy_addr %d\n", __FUNCTION__,
                      reg_ptr->name, dev_id, ((reg_ptr->offset) & 0xF), phy_addr);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: BCM8275x_register_test
 *
 * Description: This function performs the BCM82752 register test.
 *
 * Inputs      : port - port number
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM8275x_register_test (void)
{
    int phy_addr = 0;
    testname("BCM8275x PHY Register");

    if (bcm8275x_hw_init_done != 1) {
	    printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
	    return (0);
    }

    if (is_bcm82752()) {
        int no_of_phy = 2;
        for (phy_addr = 0; phy_addr < no_of_phy; phy_addr++) {
            prpass(testpass, "XFI : ");
            if (ten_g_phy_register_tests(phy_addr, BCM82752_XFI_INTF,
                                         &bcm_82752_xfi_dev3_reg[0]) == FAILED) {
                cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
                return (FAILED);
            }

        	prpass(testpass, "SFI : ");
            if (ten_g_phy_register_tests(phy_addr, BCM82752_SFI_INTF,
                                         &bcm_82752_sfi_dev1_reg[0]) == FAILED) {
                cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
                return (FAILED);
            }
        }
    } else {
        prpass(testpass, "XFI : ");
        if (ten_g_phy_register_tests(phy_addr, BCM82752_XFI_INTF,
                                     &bcm_82757_xfi_dev1_reg[0]) == FAILED) {
            cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: ten_g_dump_phy_reg()
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
static void ten_g_dump_phy_reg(int phy_addr, const bcm_phy_regs_t *phy_reg_ptr)
{
    unsigned short rdval;
    const reg_info_t *reg_ptr;
    int dev_id, reg_offset;

    /* Switch to XFI/SFI register space */
    if (is_bcm82752()) {
        bcm82752_xfi_sfi_access(phy_addr, phy_reg_ptr->phy_intf);
    }

    reg_ptr = phy_reg_ptr->intfregs;

    while (reg_ptr->size.size != 0) {
        dev_id = (reg_ptr->offset) >> 16;
        reg_offset = (reg_ptr->offset) & 0xFFFF;
        if (is_bcm82752()) {
            rdval = bcm82752_reg_rd(phy_addr, dev_id, reg_offset);
        } else {
            rdval = bcm82757_miura_reg_rd(phy_addr, phy_reg_ptr->phy_intf,
                                          dev_id, reg_offset);
        }
        /* we don't check rdval is nagetive here,
         * some of registers will get '0xF' on MSB
         */

        printf("%s : %-32s reg %d.%#.4x = %#.4x\n", phy_reg_ptr->intfname, reg_ptr->name,
                dev_id, reg_offset, rdval);
        reg_ptr++;
        msleep(10); /* wait for a while for next register. */
    }
}

/*******************************************************************************
 *
 * Function: ten_g_phy_reg_show()
 *
 * This function get PHY page/regs info and call ten_g_dump_phy_reg() to prints
 * PHY registers.
 *
 * Input: port - current port (without offset)
 *        phy_sel - PHY offset value to get specifc PHY addr.
 *        page_sel - select PHY page.
 *        dump_type - dump on page or all pages.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ten_g_phy_reg_show (int port)
{
    uint ix, intf_move;
    const bcm_phy_regs_t *phy_reg_ptr;

    if (is_bcm82752()) {
        phy_reg_ptr = &bcm82752_phy_standard_reg_tbl[0];
        intf_move = BCM82752_NUM_PHY_INTF;
    } else {
        phy_reg_ptr = &bcm82757_phy_direct_reg_tbl[0];
        intf_move = BCM82757_NUM_PHY_INTF;
    }

    /* dump all page */
    for (ix = 0; ix < intf_move; ix++) {
        ten_g_dump_phy_reg(port, phy_reg_ptr);
        phy_reg_ptr++;
    }

    return (PASSED);
}

/*
 * Function: dump_bcm8275x_standard_reg
 *
 * This function displays the PHY setting of the requested SGMII port.
 * Using ten_g_phy_reg_show() to dump PHY page.
 *
 * Input: none.
 *
 * Output: void
 */
static void dump_bcm8275x_standard_reg (void)
{
    int port_max = 1;
    int port_min = 0;
    int portnum;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);

    ten_g_phy_reg_show(portnum);
}

/******************************************************************************
 *
 * Function: neptune_cavium_xfi_lpbk_test
 *
 * Description: This function perform the Cavium xfi internal loopback test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int neptune_cavium_xfi_lpbk_test (void)
{
    if (bcm8275x_hw_init_done != 1) {
	    printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
	    return (0);
    }

    return (neptune_ten_g_phy_lpbk_test(CAVIUM_INT_LPBK));
}

/******************************************************************************
 *
 * Function: BCM8275x_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              (Shallow Host Loopback) from Cavium to BCM82752.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int BCM8275x_internal_loopback_test (void)
{
    if (bcm8275x_hw_init_done != 1) {
	    printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
	    return (0);
    }

    return (neptune_ten_g_phy_lpbk_test(TEN_GE_PHY_INT_LPBK));
}

/******************************************************************************
 *
 * Function: BCM8275x_external_loopback_test
 *
 * Description: This function perform the external loopback test from Cavium to
 *              BCM82752 SFP+.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int BCM8275x_external_loopback_test (void)
{
    if (bcm8275x_hw_init_done != 1) {
	    printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
	    return (0);
    }

    return (neptune_ten_g_phy_lpbk_test(TEN_GE_PHY_SFP_EXT_LPBK));
}

/******************************************************************************
 *
 * Function: nep_xfi_int_ext_loopback_test
 *
 * Description: This function perform the external loopback test from Cavium to
 *              BCM8275x SFP+.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int nep_xfi_int_ext_loopback_test (void)
{
    if (bcm8275x_hw_init_done != 1) {
	    printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
	    return (0);
    }

    return (neptune_ten_g_phy_lpbk_test(XFI_INT_EXT_LPBK));
}

/***********************************************************************
 *
 * Function: neptune_ten_g_phy_ptp1588_test
 *
 * Description: Do GE PHY copper external loopback test
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static int neptune_ten_g_phy_ptp1588_test (void)
{
    if (bcm8275x_hw_init_done != 1) {
	    printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
	    return (0);
    }

    return (neptune_ten_g_phy_lpbk_test(PTP_XFI_SFP_EXT_LPBK));
}

/***********************************************************************
 *
 * Function: teng_phy_reset
 *
 * Description: Do 10G PHY reset/unreset
 *
 * Inputs: none
 *
 * Outputs: none
 *
 ***************************************************************************/
static void teng_phy_reset (void)
{
    int reset; 
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;

    assert(dash_fpga);

    reset = gethex_answer("\nPHY reset 1; unreset 0", 0, 0, 1);

    if (reset) {
        printf("Reset 10G PHY...\n");
        sys->ext_rst |= FPGA_EXT_10GE_DUAL_RST; 
    } else {
        printf("Unreset 10G PHY...\n");
        sys->ext_rst &= ~FPGA_EXT_10GE_DUAL_RST; 
    }
}

static void dump_sfp_eeprom (void)
{
    int port_max = 1;
    int port_min = 0;
    int port, size = SFF_EEPROM_SIZE, i2c_addr = 0xA0, offset = 0;
    sff_trans_map_t *sfp_map = malloc(sizeof(sff_trans_map_t));
    
    port = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);

    if (bcm82752_is_sfp_module_present(port)) {
        if ((bcm82752_twsi_mii_reg_rw(port, i2c_addr, offset, sfp_map->sff_eeprom,
                                      size, 0)) != 0) {
            printf("BCM ERR - EEPROM Read Failure on port %d\n", port);
        } else {
            int ix = 0;
            for (ix = 0; ix < SFF_EEPROM_SIZE; ix++) {
                if (!(ix%8)) {
                    printf("\n0x%.2x : ", ix);
                }
                printf("%.2x ", sfp_map->sff_eeprom[ix]);
            }
        }
    } else {
        printf("No SFP presence detected for port %d.\n", port);
    }
}

static void write_sfp_eeprom (void)
{
    int port_max = 1;
    int port_min = 0;
    int port, size = 1, i2c_addr = 0xA0;
    uchar wrval, offset;
    
    port = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    offset = gethex_answer("\nEnter offset", 0, 0, SFF_EEPROM_SIZE);
    wrval = gethex_answer("\nEnter write value", 0, 0, 0xFF);

    if (bcm82752_is_sfp_module_present(port)) {
        if ((bcm82752_twsi_mii_reg_rw(port, i2c_addr, offset, &wrval,
                                      size, 1)) != 0) {
            printf("BCM ERR - EEPROM Write Failure on port %d\n", port);
        }
    } else {
        printf("No SFP presence detected for port %d.\n", port);
    }
}

/*-------------------------------------------------
$Log: bcm82752_test.c,v $
Revision 1.5  2018/10/03 09:53:57  meho
Added test coverage between BCM PHY and SFP/SFP+ via I2C interface.

Revision 1.4  2018/06/07 01:35:36  meho
Added BCM82757 1GBASE configuration utility

Revision 1.3  2018/05/28 07:39:01  meho
Added BCM82757 LA LED blink util.

Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.31  2018/02/06 07:31:26  meho
Changed bcm82757 loopback util from global to remote lpbk

Revision 1.1.2.30  2018/01/24 01:38:53  meho
Added BCM82757 line side loopback configuration utility.

Revision 1.1.2.29  2017/10/17 07:49:52  meho
1. Upgraded BCM API: MIUR_1_4, epdm_1_4_8.
2. Added BCM82757 LRM/SR configuration utilities.

Revision 1.1.2.28  2017/09/07 06:46:46  meho
1. Fixed dump BCM82752 register bug.
2. Added dump BCM82757 register utility.
3. Added BCM82757 indirect register r/w utility.

Revision 1.1.2.27  2017/07/11 06:45:57  meho
Fixed PRRQ commnet.

Revision 1.1.2.26  2017/04/28 03:01:00  meho
Added TE0/TE1 LED utility from BCM82752.

Revision 1.1.2.25  2017/04/10 05:27:24  meho
Integrated BCM82752/82757 API.

Revision 1.1.2.24  2017/03/30 05:02:24  meho
Fixed BCM82752 register test bug.

Revision 1.1.2.23  2017/01/25 11:43:54  meho
Renaming the BCM API to 10G Eye Scan.

Revision 1.1.2.21  2017/01/16 09:11:31  meho
Removed 1G speed for 10G PHY loopback test.

Revision 1.1.2.20  2017/01/13 09:36:09  meho
Added 10G PHY accesses SFP+ eeprom utilies.

Revision 1.1.2.19  2017/01/11 02:16:51  meho
Return fail when 10G PHY FW download fail.

Revision 1.1.2.18  2017/01/09 09:56:04  alpeng
support 10g and ge phy reset and unreset utilities

Revision 1.1.2.17  2017/01/05 02:20:58  meho
Added message for 10G PHY FW download.

Revision 1.1.2.16  2016/12/27 08:22:42  meho
Corrected the print Pass location.

Revision 1.1.2.15  2016/12/27 02:01:42  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.1.2.14  2016/12/19 07:49:45  meho
Added VERBOSE flag in sending packet for loopback test.

Revision 1.1.2.13  2016/12/15 02:00:18  meho
Added check external flag for GE loopback test.

Revision 1.1.2.12  2016/11/29 06:27:52  meho
Changed submenu name and code clean up.

Revision 1.1.2.11  2016/11/28 03:43:55  meho
1. Fixed GE phy Mac/Int/Ext loopback test bugs.
2. Added 10G FW download.

Revision 1.1.2.10  2016/10/01 06:23:13  meho
Added BCM82752 FW download item in menu.

Revision 1.1.2.9  2016/07/26 10:09:43  meho
Added 10G PHY PTP1588 loopback test skeleton.

Revision 1.1.2.8  2016/07/25 11:28:30  meho
Added register dump utility for BCM82752.

Revision 1.1.2.7  2016/07/22 03:48:57  meho
Added BCM82757 MACsec skeleton.

Revision 1.1.2.6  2016/07/20 08:09:49  meho
1. Updated BCM82752 firmware array.
2. Added 10G PHY loopback debug utilities.

Revision 1.1.2.5  2016/07/20 01:44:59  meho
Added GE PHY loopback debug utilities.

Revision 1.1.2.4  2016/07/14 09:17:41  meho
Added internal/SFP-external loopback for BCM82752.

Revision 1.1.2.3  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.2  2016/07/07 09:04:30  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.

Revision 1.1.2.1  2016/06/12 10:31:07  bowang3
Add bcm82752 10G PHY code framework

$Endlog$
*/
