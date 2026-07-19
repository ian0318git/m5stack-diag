/* $Id: bcm82752_test.c,v 1.5 2021/10/18 06:28:59 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm82752_test.c,v $
*-----------------------------------------------------------------------------
* bcm82752_test.c - Diags Test for BCM 10G PHY bcm82752.
*
* Feb 2019, Leschen 
*
* Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#if 0
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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <poll.h>

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
#include "bcm57412_lib.h"
#include "bcm57412_test.h"
#include <assert.h>
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "eth_traf.h"
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

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

extern int bcm8275x_hw_init_done;
extern int is_item_available(void);
extern int set_bcm57412_1g_speed(int);
extern int curie_init(void);
extern void curie_exit(void);
extern struct curie_bcm82752 *curie;
extern int curie_bcm82752_check_cl37_an(struct curie_bcm82752 *, int, int *, int *, int *);
extern int curie_bcm82752_set_cl37_an(struct curie_bcm82752 *, int, int);
extern int curie_bcm82752_link_status(struct curie_bcm82752 *, int, curie_if_side_t, unsigned int *);

static int BCM8275x_utility(int);
static void read_bcm8275x_phy_reg(void);
static void write_bcm8275x_phy_reg(void);
static int ten_g_phy_register_tests(int, bcm82752_intf_t, const reg_info_t *);
static int BCM8275x_fw_download(void);
static int BCM8275x_eye_diagram(void);
static int BCM82752_led_test(void);
static int BCM8275x_register_test(void);
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
int curie_bcm82752_mode_config(struct curie_bcm82752 *, int, int);
static long bcm82752_mode_config(void);
static long bcm82752_reg_read(void);
static long bcm82752_reg_write(void);
static int curie_bcm82752_port_speed[] = {CURIE_BCM82752_PORT_SPEED_1G, CURIE_BCM82752_PORT_SPEED_10G};
static long bcm82752_prbs_line_side_test(int);
static long bcm82752_config_cl37(void);
static long bcm82752_check_cl37(void);
static long bcm82752_link_status(void);
static long glc_te_test(void);
int bcm57412_sideband_tx_dis(int);
static inline struct nlattr *nla_next (struct nlattr *, int *);
static inline uint8_t *get_genlmsg_data(bcm_nl_request_msg_t *);
static int send_gnl_msg(int,  bcm_nl_request_msg_t *,
                        bcm_nl_request_msg_t *);
static inline void *get_gelmnsg_nla_data(struct nlattr *);
long bcm82752_side_band_test(void);
int bcm57412_sideband_tx_dis_setup(int, int);
boolean bnxt_impl_init_netlink(void);
static int create_nl_socket(int, int);
int curie_eth_get_ifindex(uint16_t);
int bnxt_netlink_sideband_tx_dis(uint16_t, uint32_t, uint16_t);
void bnxt_impl_deinit_netlink(void);
static int get_family_id(int);
static int construct_hdrs(bcm_nl_request_msg_t*,
                           uint32_t, int *,
                           struct nlattr **);

typedef struct bnxt_ipc_mng_ {
    int      socket_fd;
    int      cpr_kernel_session;
    boolean  is_init_was_done;
} bnxt_ipc_mng_t;

static bnxt_ipc_mng_t g_mng;
static boolean validate_next_buf(int remaining, int next_len)
{
    return (remaining >= (NLA_HDRLEN + next_len));
}

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

/******************************************************************************
 *  List of Menu used for XFI BCM82752
 *****************************************************************************/
static submenu_xtable_t BCM8275x_tests_submenu_table[] = {
   {"PHY BCM8275x Utility", (type_t(*)())BCM8275x_utility,   FALSE,
    0, NULL, 0, (type_t(*)())BCM8275x_utility,   TRUE},
   {"PHY BCM8275x Register Test", (type_t(*)())BCM8275x_register_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PHY BCM8275x External Loopback Test", (type_t(*)())nep_xfi_int_ext_loopback_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    /* Deafult will not run this test for 3rd RDT because hasn't been verified before */
   {"BCM82752 Side Band Test", (type_t(*)())bcm82752_side_band_test, 0,
    MF_4, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  List of Utilities used for XFI BCM82752
 *****************************************************************************/
static submenu_xtable_t BCM8275x_util_items[] = {
    {"PHY BCM8275x Firmware Download", (type_t(*)())BCM8275x_fw_download, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BCM82752 Port 0 prbs line side Test", (type_t(*)())bcm82752_prbs_line_side_test, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BCM82752 Port 1 prbs line side Test", (type_t(*)())bcm82752_prbs_line_side_test, 1,
     0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BCM82752 Speed Config", bcm82752_mode_config, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump PHY BCM8275x registers", (type_t(*)())dump_bcm8275x_standard_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Read PHY BCM8275x Registers", (type_t(*)())read_bcm8275x_phy_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Write PHY BCM8275x Register", (type_t(*)())write_bcm8275x_phy_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY BCM8275x Internal Loopback Test", (type_t(*)())BCM8275x_internal_loopback_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY BCM8275x Eye Diagram", (type_t(*)())BCM8275x_eye_diagram, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read BCM82752 Register by Broadcom API", bcm82752_reg_read, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Write BCM82752 Register by Broadcom API", bcm82752_reg_write, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"10G PHY BCM82757 MACsec Test", (type_t(*)())ten_g_macsec_test_main, FALSE,
     0, (PFT)is_item_available, 0, (type_t(*)())ten_g_macsec_test_main, TRUE},
    {"PHY BCM8275x PTP1588 Test", (type_t(*)())ten_g_phy_ptp1588_test_main, FALSE,
     0, (PFT)is_item_available, 0, (type_t(*)())ten_g_phy_ptp1588_test_main, TRUE},
    {"PHY BCM8275x reset ", (type_t(*)())teng_phy_reset, 0, 0,
     (PFT)is_item_available, 0, (type_t(*)())0, 0},
    {"10G PHY LED Test", (type_t(*)())BCM82752_led_test, 0, 0,
     (PFT)is_item_available, 0, (type_t(*)())0, 0},
    {"PHY BCM8275x External Loopback Test", (type_t(*)())BCM8275x_external_loopback_test, 0, 0,
     (PFT)is_item_available, 0, (type_t(*)())0,   0},
    {"BCM82752 Config Clause 37", bcm82752_config_cl37, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 check Clause 37", bcm82752_check_cl37, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 link status", bcm82752_link_status, 0,
     0, NULL, 0, NULL, 0},
    {"BCM57412 side band enable tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, ENABLE,
     0, NULL, 0, NULL, 0},
    {"BCM57412 side band disable tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, DISABLE, 
     0, NULL, 0, NULL, 0},
    {"SFP GLC-TE Test", glc_te_test, 0,
     0, NULL, 0, NULL, 0},
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
    "PHY BCM8275x Utility Menu",
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
    system(SUPPRESS_MESG);

    /* Init bcm57412 SMI interface */
    curie_init();

    if (is_thallium()) {
        /* Thallium two ports are running at speed 1G */
        if (set_bcm57412_1g_speed(MB_57412_PORT1) == FAILED) {
            printf("Failed to set Thallium bcm57412 port1 at speed 1G\n");
            system(DISPLAY_PORT1_CAP);
        } 

        if (set_bcm57412_1g_speed(MB_57412_PORT2) == FAILED) {
            printf("Failed to set Thallium bcm57412 port2 at speed 1G\n");
            system(DISPLAY_PORT2_CAP);
        } 

        if (curie_bcm82752_mode_config(curie, BCM82752_PORT1, CURIE_BCM82752_PORT_SPEED_1G)) {
            printf("Failed to configure bcm82752 port1 1G speed\n");
        }

        if (curie_bcm82752_mode_config(curie, BCM82752_PORT2, CURIE_BCM82752_PORT_SPEED_1G)) {
            printf("Failed to configure bcm82752 port2 1G speed\n");
        }
    }

    build_primary_submenu(BCM8275x_tests_submenu_table,
                          BCM8275X_TESTS_SUBMENU_TABLE_SIZE,
                          "PHY BCM8275x", &BCM8275x_submenup);
    build_secondary_submenu(BCM8275x_tests_submenu_table,
                            BCM8275X_TESTS_SUBMENU_TABLE_SIZE,
                            BCM8275x_tests_secondary_items);

    if (show_menu) {
        menu(BCM8275x_submenup, BCM8275x_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(BCM8275x_submenup);
    }

    curie_exit();

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
                          "PHY BCM8275x Utilities Menu", &BCM8275x_util_menup);
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

int curie_bcm82752_read(struct curie_bcm82752 *curie,
                           int id, curie_if_side_t if_side,
                           uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    struct curie_quadra28 *q28 = &curie->quadra28[id];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);

    return bcmphy_quadra28_reg_read(q28, info->if_side, q28->id,
                                    info->lane_map, devaddr, &regaddr, data, 1);
}

int curie_bcm82752_write(struct curie_bcm82752 *curie,
                            int id, curie_if_side_t if_side,
                            uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    struct curie_quadra28 *q28 = &curie->quadra28[id];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);

    return bcmphy_quadra28_reg_write(q28, info->if_side, q28->id,
                                         info->lane_map, devaddr, &regaddr, &data, 1);
}

static long bcm82752_reg_read(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE_QUADRA28_DEV_PMA_PMD;
    int port;
    curie_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE_QUADRA28_DEV_PMA_PMD,
                            CURIE_QUADRA28_DEV_PCS,
                            CURIE_QUADRA28_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);

    rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);

    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 read error");
        return FAILED;
    }
    printf("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

static long bcm82752_reg_write(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE_QUADRA28_DEV_PMA_PMD;
    int port;
    curie_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE_QUADRA28_DEV_PMA_PMD,
                            CURIE_QUADRA28_DEV_PCS,
                            CURIE_QUADRA28_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = curie_bcm82752_write(curie, port, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 write error");
        return FAILED;
    }
    printf("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return PASSED;
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
    int data;
    uint dev_id = BCM82752_DEV_PMA, portnum, phy_intf;
    uint phy_addr, regnum, regnum_max = 0xFFFFFFFF;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface(XFI:0, SFI:1) ", 0, 0, 1);
    dev_id = gethex_answer("\nEnter DEV ID(PMA/PMD:1, PCS:3, 1GbE:7) ",
                            BCM82752_DEV_PMA, BCM82752_DEV_PMA, BCM82752_DEV_1GBE);
    regnum = gethex_answer("\nEnter PHY reg number(0x0 - 0xFFFFFFFF)", 0, 0, regnum_max);

    phy_addr = te_port_mapping_phy_addr[portnum];

    /* Switch to XFI/SFI mode */
    bcm82752_xfi_sfi_access(portnum, phy_intf);
    data = bcm82752_reg_rd(phy_addr, dev_id, regnum);
    printf("%d.%#.4x --> %#.8x\n", dev_id, regnum, data);
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
    int data;
    uint dev_id = BCM82752_DEV_PMA, portnum, phy_intf;
    uint phy_addr, regnum, regnum_max = 0xFFFFFFFF;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface(XFI:0, SFI:1) ", 0, 0, 1);
    dev_id = gethex_answer("\nEnter DEV ID(PMA/PMD:1, PCS:3, 1GbE:7) ",
                            BCM82752_DEV_PMA, BCM82752_DEV_PMA, BCM82752_DEV_1GBE);
    regnum = gethex_answer("\nEnter PHY reg number(0x0 - 0xFFFFFFFF)", 0, 0, regnum_max);
    wrval = gethex_answer("Enter value:", 0, 0, regnum_max);

    phy_addr = te_port_mapping_phy_addr[portnum];

    /* Switch to XFI/SFI mode */
    bcm82752_xfi_sfi_access(phy_addr, phy_intf);
    data = bcm82752_reg_rd(phy_addr, dev_id, regnum);
    printf("Before write - %d.%#.4x --> %#.8x\n", dev_id, regnum, data);

    bcm82752_reg_wr(phy_addr, dev_id, regnum, wrval);
    data = bcm82752_reg_rd(phy_addr, dev_id, regnum);
    printf("After write value %x - %d.%#.4x --> %#.8x\n", wrval, dev_id, regnum, data);
}

static int bcm82752_mode_clear_check(struct curie_quadra28 *q28)
{
    int timer_count = 200;
    bcm_plp_access_t *info = &q28->info;

    do {
        uint32_t data;
        bcm_plp_reg_value_get(q28->type, *info, 1, 0xC843, &data);
        if (!(data & 0x80))
            break;
        usleep(5000);
        timer_count--;
    } while (timer_count > 0);

    if (timer_count <= 0) {
        printf("failed to clear mode\n");
        return -1;
    }

    return 0;
}

static int bcm82752_mode_check(struct curie_quadra28 *q28, uint16_t mode)
{
    int timer_count = 200;
    bcm_plp_access_t *info = &q28->info;

    do {
        uint32_t data;
        bcm_plp_reg_value_get(q28->type, *info, 1, 0xC843, &data);
        if (data == mode)
            break;
        usleep(5000);
        timer_count--;
    } while (timer_count > 0);

    if (timer_count <= 0) {
        printf("failed to set mode %04x\n", mode);
        return -1;
    }

    return 0;
}

int curie_bcm82752_10g_config(void)
{
    int rv = FAILED;
    rv = BCM8275x_fw_download();
    return (rv);
}

int curie_bcm82752_1g_config(struct curie_bcm82752 *curie, int port, int recovered)
{
    uint32_t data;
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0xC8D8, &data);
        data &= ~(1 << 7);
    bcm_plp_reg_value_set(q28->type, *info, 1, 0xC8D8, data);

    if (bcm82752_mode_clear_check(q28) < 0)
        return -1;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0xC8D9, &data);
    if (recovered)
        data &= ~(1 << 4);
    else
        data |= (1 << 4);
    bcm_plp_reg_value_set(q28->type, *info, 1, 0xC8D9, data);

    bcm_plp_reg_value_set(q28->type, *info, 1, 0xC8D8, 0x0081);

    if (bcm82752_mode_check(q28, 0x0081) < 0)
        return -1;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0000, &data);
    data |= (1 << 15);
    bcm_plp_reg_value_set(q28->type, *info, 1, 0x0000, data);

    usleep(500 * 1000);
    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0000, &data);

    if (data & (1 << 15)) {
        printf("error: 1.0000.15 not self-cleard\n");
        return -1;
    }

    return 0;
}

int curie_bcm82752_mode_config(struct curie_bcm82752 *curie, int port, int speed)
{
    if (speed == CURIE_BCM82752_PORT_SPEED_1G)
        return curie_bcm82752_1g_config(curie, port, 0);
    else
        return curie_bcm82752_10g_config();
}

static long bcm82752_mode_config(void)
{
    int port, speed_flag, speed;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    speed_flag = gethex_answer("Enter speed(1G:0, 10G:1)", 0, 0, 1);
    speed = curie_bcm82752_port_speed[speed_flag];
    if (curie_bcm82752_mode_config(curie, port, speed)) {
        cterr('f', 0, "failed to configure bcm82752 mode");
        return FAILED;
    }
    return PASSED;
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
    struct curie_quadra28 *q28 = curie->quadra28;

    curie_quadra28_reset(q28);

    if (curie_quadra28_fw_download(q28)) {
        cterr('f', 0, "10G PHY FW Download failed");
        return (FAILED);
    }

    /* Emphasis setting */
    bcm82752_emphasis_setting();

    return (PASSED);
}

int curie_bcm82752_display_eye_scan(struct curie_bcm82752 *curie,
                                       int port, curie_if_side_t if_side)
{
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_display_eye_scan(q28->type, *info);
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
    int port;
    curie_if_side_t if_side;

    printf("BCM INFO - 10G PHY Eye Scan.\n");

    if (bcm8275x_hw_init_done != 1) {
        printf("\nPlease execute PHY BCM8275x firmware download first!!!\n");
        return (0);
    }

    if (is_bcm82752()) {
        //rc = quadra28_eye_diagram();

        port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
        if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

        rc = curie_bcm82752_display_eye_scan(curie, port, if_side);
        if (rc < 0) {
            printf("BCM82752 eye scan failed\n");
            return (rc);
        }
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
        /* Curie P2A bcm82752 PHY address is 0x10 and 0x11 */
        for (phy_addr = 0x10; phy_addr < 0x12; phy_addr++) {
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

    if (portnum) {
        portnum = te_port_mapping_phy_addr[portnum];
    } else {
        portnum = te_port_mapping_phy_addr[portnum];
    }

    ten_g_phy_reg_show(portnum);
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

int curie_bcm82752_prbs_check(struct curie_bcm82752 *curie,
                                 int port, curie_if_side_t if_side)
{
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);
    if ((rc = bcm_plp_prbs_status_get(q28->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        cterr('f', 0, "bcm_plp_prbs_status_get failed");
        return rc;
    }

    if (prbs_lock) {
        printf("prbs locked\n");
        printf("prbs error count: %d\n", error_count);
    } else {
        printf("prbs unlock\n");
    }
    if (prbs_lock_loss)
        cterr('f', 0, "prbs lock loss");
    return (!prbs_lock || prbs_lock_loss || error_count) ? -1 : 0;
}

int curie_bcm82752_prbs_clear_rx_stat(struct curie_bcm82752 *curie, int port, curie_if_side_t if_side)
{
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);
    if ((rc = bcm_plp_prbs_status_get(q28->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        cterr('f', 0, "bcm_plp_prbs_status_get failed");
        return rc;
    }
    return 0;
}

int curie_bcm82752_prbs_set(struct curie_bcm82752 *curie,
                               int port, curie_if_side_t if_side,
                               curie_prbs_t prbs, unsigned int enable)
{
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int poly;
    int rc;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);

    switch (prbs) {
    case CURIE_PRBS_7:
        poly = 0;
        break;
    case CURIE_PRBS_9:
        poly = 1;
        break;
    case CURIE_PRBS_11:
        poly = 2;
        break;
    case CURIE_PRBS_15:
        poly = 3;
        break;
    case CURIE_PRBS_23:
        poly = 4;
        break;
    default:
    case CURIE_PRBS_31:
        poly = 5;
        break;
    }

    rc = bcm_plp_prbs_set(q28->type, *info, 0, poly, 0, 0, enable);
    if (!rc && !enable)
        rc = bcm_plp_prbs_clear(q28->type, *info, 0);
    return rc;
}

static void curie_bcm82752_set_tx_serdes(struct curie_quadra28 *q28)
{
    int _rc;
    uint32_t data, regaddr, devaddr = CURIE_QUADRA28_DEV_PMA_PMD;

    regaddr = BCMI_QUADRA28_TX_CTRL_5r;
    data = 0x7000;
    _rc = bcm_plp_reg_value_set(q28->type, q28->info, devaddr, regaddr, data);
    if (_rc) {
        printf("warn: failed to set bcm82752 tx ctrl5r: %04x\n", data);
    }

    regaddr = BCMI_QUADRA28_TXFIR_CONTROL1r;
    data = 0x0100;
    _rc = bcm_plp_reg_value_set(q28->type, q28->info, devaddr, regaddr, data);
    if (_rc) {
        printf("warn: failed to set bcm82752 tx fir ctrl1r: %04x\n", data);
    }

    regaddr = BCMI_QUADRA28_TXFIR_CONTROL2r;
    data = 0x8026;
    _rc = bcm_plp_reg_value_set(q28->type, q28->info, devaddr, regaddr, data);
    if (_rc) {
        printf("warn: failed to set bcm82752 tx fir ctrl2r: %04x\n", data);
    }
}

static long __bcm82752_prbs_line_side_test(int port,
                                           curie_prbs_t prbs, uint32_t delay_sec)
{
    uint32_t enable = 1;
    curie_if_side_t if_side = CURIE_IF_SIDE_LINE;

    /* enable prbs */
    if (curie_bcm82752_prbs_set(curie, port, if_side, prbs, enable)) {
        cterr('f', 0, "BCM82752 PRBS set enable failed on port %d", port);
        return FAILED;
    }

    msleep(2000);
    /* clear prbs rx stat */
    if (curie_bcm82752_prbs_clear_rx_stat(curie, port, if_side)) {
        cterr('f', 0, "BCM82752 PRBS clear rx stat failed on port %d", port);
        return FAILED;
    }

    /* check prbs */
    msleep(delay_sec * 2000);
    if (curie_bcm82752_prbs_check(curie, port, if_side)) {
        cterr('f', 0, "BCM82752 PRBS check failed on port %d", port);
        return FAILED;
    }

    /* disable prbs */
    enable = 0;
    if (curie_bcm82752_prbs_set(curie, port, if_side, prbs, enable)) {
        cterr('f', 0, "BCM82752 PRBS set disable failed on port %d", port);
        return FAILED;
    }

    return (PASSED);
}

static long bcm82752_prbs_line_side_test(int port)
{
    curie_prbs_t prbs = CURIE_PRBS_7;
    struct curie_quadra28 *q28 = &curie->quadra28[port];

    testname("BCM82752 Port %d prbs line side", port);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the port %d external loopback test\n", port);
        return PASSED;
    }

    curie_bcm82752_set_tx_serdes(q28);

    msleep(500);
    if (__bcm82752_prbs_line_side_test(port, prbs, PRBS_TEST_DELAY) == FAILED) {
        cterr('f', 0, "port %d line sdie test Failed", port);
        return FAILED;
    }

    prpass(testpass, "port %d prbs line side Test Passed, ", port);

    return PASSED;
}

/***********************************************************************
 *
 * Function: glc_te_test
 *
 * Description: SFP GLC-TE test. BCM82752 external two ports connect
 *              with each other. Send packets from one side to another
 *              side.
 *
 * Inputs: NONE 
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static long glc_te_test(void)
{
    int rc = FAILED;
    char buf[128];
    FILE *fp;
    int ix = MB_57412_PORT1;

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    for (; ix <= MB_57412_PORT2; ix++) {
        if (ix == MB_57412_PORT1) {
            printf("Send packets from eth4 to eth5 via SFP GLC-TE\n");
            system(GLC_TE_TEST_1);
            msleep(200);
        } else if (ix == MB_57412_PORT2) {
            printf("Send packets from eth5 to eth4 via SFP GLC-TE\n");
            system(GLC_TE_TEST_2);
            msleep(200);
        } else {
            printf("Unknown port number %x\n", ix);
            return (rc);
        }

        fp = fopen(GLC_TE_TEST_RESULT, "r");

        if (fp == NULL) {
            printf("Failed to open file glc_te_test.txt");
            return (rc);
        }

        while (!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, GLC_TE_FAIL) != NULL) {
                printf("\nGLC-TE Test FAILED\n");
                fclose(fp);
                goto exit;
            } else {
                if (ix == MB_57412_PORT2) {
                    printf("\nGLC-TE Test PASSED\n");
                    rc = PASSED;
                }
                break;
            }
        }
        fclose(fp);
    }

exit:
    return (rc);
}

/***********************************************************************
 *
 * Function: bcm82752_link_status
 *
 * Description: Check BCM82752 link status
 *
 * Inputs: NONE 
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static long bcm82752_link_status(void)
{
    int rc;
    unsigned int link_status;
    int port;
    curie_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie_bcm82752_link_status(curie, port, if_side, &link_status);
    if (rc < 0) {
        printf("BCM82752 link down - status: %d\n", link_status);
        return (FAILED);
    }

    printf("phy link up - status: %d\n", link_status);
    return (PASSED);
}

/***********************************************************************
 *
 * Function: bcm82752_config_cl37
 *
 * Description: Config BCM82752 CL37 mode
 *
 * Inputs: NONE 
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static long bcm82752_config_cl37(void)
{
    int port;
    unsigned int enable;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (curie_bcm82752_set_cl37_an(curie, port, enable)) {
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: bcm82752_check_cl37
 *
 * Description: Check BCM82752 CL37 mode
 *
 * Inputs: NONE 
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static long bcm82752_check_cl37(void)
{
    int port, an, link, done;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);

    if (curie_bcm82752_check_cl37_an(curie, port, &an, &link, &done)) {
        return (FAILED);
    }

    printf("cl37 status: an enbale %d, link %d, done %d\n", an, link, done);

    return (PASSED);
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

long bcm82752_side_band_test (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE_QUADRA28_DEV_PMA_PMD;
    uint32_t rx_los_reg, tx_flt_reg, mod_abs;
    int port;
    curie_if_side_t if_side = CURIE_IF_SIDE_LINE;
    int ix, jx, result = PASSED;

    printf("SFP side band test is included in 3rd RDT release, hasn't been verified before!\n");
    printf("Please do not run it for critical phase now!\n");
    testname("BCM82752 Side Band");

    /* Optical configuration status reg 0xc8e4 */
    /* Write optical configuration control reg 0xc800 with val 0x383f */
    /* rx_los status reg. = 0xc8e4 bit6 */
    /* tx_fault status reg. = 0xc8e4 bit5 */
    /* mod_abs status reg. = 0xc834 bit3 (SFP present) */

    rx_los_reg = OPT_CONF_STAT_REG;
    tx_flt_reg = OPT_CONF_STAT_REG;
    mod_abs    = OPT_CONF_STAT_REG;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("\nPlease remove the SFP and loopback\n");
        /* No plug-in SFP, rx_los,tx_flt,mod_abs are high */
        for (ix = 0; ix < 2 ; ix++) {
            /* Write 0xc800 with val 0x383f */
            port = ix;
            data = OPT_CONF_CTRL_VAL;
            regaddr = OPT_CONF_CTRL_REG;
            rc = curie_bcm82752_write(curie, port, if_side, devaddr, regaddr, data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d write error reg %x", ix, regaddr);
                result = FAILED;
            }

            prpass(testpass, "Port %x Side Band Test, ",ix);

            /* RX_LOS Test */
            regaddr = rx_los_reg;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", ix);
                result = FAILED;
            }

            if (!(data & RX_LOS_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d rx_los (0x%x) error", ix, data);
                result = FAILED;
            }

            /* TX_FAULT Test */
            regaddr = tx_flt_reg;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 read error");
                result = FAILED;
            }

            if (!(data & TX_FLT_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d tx_fault (0x%x) error", ix, data);
                result = FAILED;
            }

            /* SFP_PRESENT Test */
            regaddr = mod_abs;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", ix);
                result = FAILED;
            }

            if (!(data & MOD_ABS_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d sfp_present (0x%x) error", ix, data);
                result = FAILED;
            }

            if (result == PASSED) {
                printf("\nPort %d Side Band Pass\n",ix);
            } else {
                goto exit;
            }
        }
    } else {
        printf("\nPlease plug-in the SFP and loopback\n");
        /* Test1: tx_dis enable, rx_los=high, tx_flt,sfp_present=low */
        for (ix = 0; ix < 2 ; ix++) {
            /* Write 0xc800 with val 0x383f */
            port = ix;
            data = OPT_CONF_CTRL_VAL;
            regaddr = OPT_CONF_CTRL_REG;
            rc = curie_bcm82752_write(curie, port, if_side, devaddr, regaddr, data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d write error reg %x", ix, regaddr);
                result = FAILED;
            }

            prpass(testpass, "Port %x TX_DIS Enable, ",ix);

            result = bcm57412_sideband_tx_dis_setup(ix, ENABLE);
            msleep(SIDEBAND_ASSERT_TIME);

            /* RX_LOS Test */
            regaddr = rx_los_reg;
            port = ix;
            printf("\n");
            for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
                rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
                if (rc < 0) {
                    cterr('f', 0, "BCM82752 port %d read error", ix);
                    result = FAILED;
                }
                if (!(data & RX_LOS_STATUS_MASK)) {
                    result = FAILED;
                    msleep(SIDEBAND_ASSERT_TIME);
                    continue;
                }
                break;
            }

            if ((jx == SIDEBAND_TIMEOUT)) {
                cterr('f', 0, "BCM82752 port %d rx_los (0x%x) error", ix, data);
                result = FAILED;
            }

            /* TX_FAULT Test */
            regaddr = tx_flt_reg;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", ix);
                result = FAILED;
            }

            if ((data & TX_FLT_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d tx_fault (0x%x) error", ix, data);
                result = FAILED;
            }

            /* SFP_PRESENT Test */
            regaddr = mod_abs;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", ix);
                result = FAILED;
            }

            if ((data & MOD_ABS_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d sfp_present (0x%x) error", ix,
                       data);
                result = FAILED;
            }

            if (result == PASSED) {
                printf("\nPort %d Side Band Pass, ",ix);
            } else {
                goto exit;
            }
        }


        /* Test2: tx_dis disable,  rx_los=low, tx_flt, sfp_present=low */
        for (ix = 0; ix < 2 ; ix++) {
            prpass(testpass, "Port %x TX_DIS Disable, ",ix);

            result = bcm57412_sideband_tx_dis_setup(ix, DISABLE);
            msleep(SIDEBAND_ASSERT_TIME);

            /* RX_LOS Test */
            regaddr = rx_los_reg;
            port = ix;
            printf("\n");
            for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
                rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
                if (rc < 0) {
                    cterr('f', 0, "BCM82752 port %d read error", ix);
                    result = FAILED;
                }
                if ((data & RX_LOS_STATUS_MASK)) {
                    result = FAILED;
                    msleep(SIDEBAND_ASSERT_TIME);
                    continue;
                }
                break;
            }

            if ((jx == SIDEBAND_TIMEOUT)) {
                cterr('f', 0, "BCM82752 port %d rx_los (0x%x) error", ix, data);
                result = FAILED;
            }

            /* TX_FAULT Test */
            regaddr = tx_flt_reg;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", ix);
                result = FAILED;
            }

            if ((data & TX_FLT_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d tx_fault (0x%x) error", ix, data);
                result = FAILED;
            }

            /* SFP_PRESENT Test */
            regaddr = mod_abs;
            rc = curie_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", ix);
                result = FAILED;
            }

            if ((data & MOD_ABS_STATUS_MASK)) {
                cterr('f', 0, "BCM82752 port %d sfp_present (0x%x) error", ix,
                       data);
                result = FAILED;
            }

            if (result == PASSED) {
                printf("\nPort %d Side Band Pass\n",ix);
            } else {
                goto exit;
            }
        }
    }
exit:
    return (result);
}

int bcm57412_sideband_tx_dis (int enable)
{
    int retval = PASSED;
    int ix = 0;
    int ifindex;

    bnxt_impl_init_netlink();
    /* To verify eth4 and eth5 */
    for (ix = 4; ix < 6; ix++) {
        ifindex = curie_eth_get_ifindex(ix);
        if (bnxt_netlink_sideband_tx_dis(ix, ifindex, enable)) {
            cterr('f',0,"Port %d: sideband read: Target not responding", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}

/******************************************************************************
 *
 * Function: bcm57412_sideband_tx_dis_setup
 *
 * Description: This function enalbe/disable the sideband tx_dis GPIO value
 *
 * Inputs      : port - port number
 *             : enable - 1: enable "tx_dis" to SFP, 0: disable "tx_dis" to SFP (enable SFP TX).
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_sideband_tx_dis_setup (int port, int enable)
{
    int retval = PASSED;
    int ix = 0;
    int ifindex;

    bnxt_impl_init_netlink();

    /* To verify eth4 and eth5 */
    for (ix = 4; ix < 6; ix++) {
        ifindex = curie_eth_get_ifindex(ix);
        if (bnxt_netlink_sideband_tx_dis(ix, ifindex, enable)) {
            cterr('f',0,"Port %d: sideband read: Target not responding", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}

/******************************************************************************
 *
 * Function: bnxt_impl_init_netlink
 *
 * Description: This function initial the netlink
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
boolean bnxt_impl_init_netlink (void)
{
    if (g_mng.is_init_was_done) {
        return (PASSED);
    }

    g_mng.socket_fd = create_nl_socket(NETLINK_GENERIC, 0);
    if( g_mng.socket_fd  < 0){
        printf("\nFUGAZI_NL: create_nl_socket failed");
        return (FAILED);
    }

    g_mng.cpr_kernel_session = get_family_id(g_mng.socket_fd);

    if( g_mng.cpr_kernel_session < 0){
        printf("\nFUGAZI_NL: get_family_id failed");
        return(FAILED);
    }
    g_mng.is_init_was_done = TRUE;
    return (PASSED);
}

/******************************************************************************
 *
 * Function: create_nl_socket
 *
 * Description: This function create a netlink socket
 *
 * Inputs      : protocol - netlonk protocol
 * Inputs      : groups   - netlonk group
 * Outputs     : Socket id
 *
 *****************************************************************************/
static int create_nl_socket(int protocol, int groups)
{
    int fd;
    struct sockaddr_nl local;

    fd = socket(AF_NETLINK, SOCK_RAW, protocol);
    if (fd < 0){
        printf("\nFUGAZI_NL: socket create error");
        return (-1);
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = groups;
    if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        close(fd);
        printf("\nFUGAZI_NL: socket bind error");
        return (-1);
    }
    return fd;
}
/******************************************************************************
 *
 * Function: curie_eth_get_ifindex
 *
 * Description: This function get the network index from /sys/class/net/%s/ifindex
 *
 * Inputs      : port - ethernet port number
 * Outputs     : ifindex
 *
 *****************************************************************************/
int curie_eth_get_ifindex (uint16_t port)
{
    char path[DEV_IFINDEX_PATH_SIZE];
    char dev_name[DEV_IFINDEX_PATH_SIZE];
    int ifindex;
    FILE *fp;

    sprintf(dev_name, "eth%d", port);
    snprintf(path, DEV_IFINDEX_PATH_SIZE, SYS_IFINDEX_PATH, dev_name);
    fp = fopen(path, "r");
    fscanf(fp, "%d", &ifindex);
    fclose(fp);
    return ifindex;
}
/******************************************************************************
 *
 * Function: bnxt_netlink_sideband_tx_dis
 *
 * Description: The firmware processes the HWRM command and payload contained
 *              in the message. Enable / Disable tx_dis sideband.
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 *             : enable - enable / disable flag
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bnxt_netlink_sideband_tx_dis (uint16_t port_id, uint32_t ifindex,
                                  uint16_t enable)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_sfp_sideband_cfg_input_t sideband_write_req;
    hwrm_port_sfp_sideband_cfg_output_t *sideband_write_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;

    memset(&sideband_write_req, 0, sizeof(sideband_write_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(sideband_write_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        sideband_write_req.req_type = HWRM_PORT_SFP_SIDEBAND_CFG;
        sideband_write_req.port_id = port_id;
        sideband_write_req.cmpl_ring = 0;
        sideband_write_req.seq_id = 0;
        sideband_write_req.target_id = 0;
        sideband_write_req.resp_addr = 0;
        sideband_write_req.enables |= PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_TX_DIS;
        if (enable) {
            sideband_write_req.flags |= PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS;
        } else {
            sideband_write_req.flags &= PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS;
        }
        memcpy((char *)get_gelmnsg_nla_data(na), &sideband_write_req,
               sizeof(sideband_write_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST",
                port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error",
                port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    sideband_write_resp = (hwrm_port_sfp_sideband_cfg_output_t *)
                           get_gelmnsg_nla_data(na);

    if (sideband_write_resp->error_code) {
        printf("\nFUGAZI_NL %d: write error_code 0x%x",
                port_id, sideband_write_resp->error_code);
        return (FAILED);
    }

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n FUGAZI Port %d: tx_dis %s", port_id,
                enable ? "enable" : "disable" );
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: bnxt_impl_deinit_netlink
 *
 * Description: This function deinit the netlink
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
void bnxt_impl_deinit_netlink (void)
{
    if (!g_mng.is_init_was_done) {
        return ;
    }
    if (g_mng.socket_fd > 0) {
        close(g_mng.socket_fd);
        g_mng.socket_fd = 0;
    }
    g_mng.is_init_was_done = FALSE;
}

/*
 * Probe the controller in genetlink to find the family id
 * for the CONTROL_EXMPL family
 */
/******************************************************************************
 *
 * Function: get_family_id
 *
 * Description: This function get the netlink family id
 *
 * Inputs      : sd - Socket ID
 * Outputs     : Family ID
 *
 *****************************************************************************/
static int get_family_id (int sd)
{
    bcm_nl_request_msg_t family_req;
    bcm_nl_request_msg_t ans;
    memset(&ans,0 ,sizeof(bcm_nl_request_msg_t));
    int fam_id = -1;

    /* Get family name */
    family_req.n.nlmsg_type  = GENL_ID_CTRL;
    family_req.n.nlmsg_flags = NLM_F_REQUEST;
    family_req.n.nlmsg_seq   = 0;
    family_req.n.nlmsg_pid   = getpid();
    family_req.n.nlmsg_len   = NLMSG_LENGTH(GENL_HDRLEN);
    family_req.g.cmd         = CTRL_CMD_GETFAMILY;
    family_req.g.version     = 0x1;

    struct nlattr *na = (struct nlattr *) get_genlmsg_data(&family_req);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;

    na->nla_len = strlen(BNXT_NL_NAME) + 1 + NLA_HDRLEN;

    strcpy((char *)get_gelmnsg_nla_data(na), BNXT_NL_NAME);

    family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (send_gnl_msg(sd, &family_req, &ans) < 0) {
        return (FAILED);
    }

    na = (struct nlattr *) get_genlmsg_data(&ans);
    na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
    if (na->nla_type == CTRL_ATTR_FAMILY_ID) {
        fam_id = *(__u16 *) get_gelmnsg_nla_data(na);
    }

    return fam_id;
}

/******************************************************************************
 *
 * Function: construct_hdrs
 *
 * Description: This function construct the netlink.
 *
 * Inputs      : req       - Send command structure
 *               *remaining - remaining parameter
 *               *naddr     - na pointer to starting of next header
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int construct_hdrs (bcm_nl_request_msg_t* req,
                           uint32_t ifindex, int *remaining,
                           struct nlattr **naddr)
{
    uint32_t data = 0;
    int next_len;
    struct nlattr *na;

    /* Send command needed */
    req->n.nlmsg_type   = g_mng.cpr_kernel_session;
    req->n.nlmsg_flags  = NLM_F_REQUEST;
    req->n.nlmsg_seq    = 0;
    req->n.nlmsg_pid    = getpid();
    req->n.nlmsg_len    = NLMSG_LENGTH(GENL_HDRLEN);
    req->g.cmd          = BNXT_CMD_HWRM;

    /* compose message */
    /* Add PID for get to the name-space */
    na = (struct nlattr *) get_genlmsg_data(req);
    na->nla_type = BNXT_ATTR_PID;
    na->nla_len = NLA_HDRLEN + sizeof(data);
    data = getpid();
    memcpy((char *)get_gelmnsg_nla_data(na), &data, sizeof(data));
    req->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

   /* Add IF_INDEX of the interface */
    na = nla_next(na, remaining);
    next_len = sizeof(data) + NLA_HDRLEN;
    if (validate_next_buf(*remaining, next_len)) {
        na->nla_type = BNXT_ATTR_IF_INDEX;
        na->nla_len = next_len; /* Message length */
        data = ifindex;
        memcpy((char *)get_gelmnsg_nla_data(na), &data, sizeof(data));
        req->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL: construct_hdrs: Failed to insert IF_INDEX");
        return (FAILED);
    }

    /* Set the na pointer to starting of next header */
    na = nla_next(na, remaining);
    *naddr = na;
    return (PASSED);
}

/******************************************************************************
 *
 * Function: get_gelmnsg_nla_data
 *
 * Description: get message from nal data
 *
 * Inputs      : na: netlink attribute
 *
 * Outputs     : Data address offset.
 *
 *****************************************************************************/
static inline void *get_gelmnsg_nla_data(struct nlattr *na)
{
    return ((void *)((char*)(na) + NLA_HDRLEN));
}

/******************************************************************************
 *
 * Function: send_gnl_msg
 *
 * Description: This function checks sfp+ cookie byte 0 and byte 1 to make
 *              sure the i2c bus between BCM57412 and SFP module is good.
 *
 * Inputs      : sd   - Socket ID
 *               *msg - command message
 *               *ans - response message
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int send_gnl_msg(int sd,  bcm_nl_request_msg_t *msg,
                        bcm_nl_request_msg_t *ans)
{
    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    char *buf = (char *)msg;
    int left_to_send = msg->n.nlmsg_len;
    int resp_len = 0;

    while (left_to_send) {
        int sent_len = sendto(sd, buf, left_to_send, 0,
                             (struct sockaddr *) &nladdr,
                              sizeof(nladdr));
        if (sent_len > left_to_send) {
            printf("\nFUGAZI_NL: send_gnl_msg, sent_len > left_to_send");
            return (FAILED);
        }

         if (sent_len <= 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                printf("FUGAZI_NL: sendto returned error");
                return (FAILED);
            }
        }

        buf += (sent_len);
        left_to_send -= (sent_len);
    }

    resp_len = recv(sd, ans, sizeof(bcm_nl_request_msg_t), 0);
    if (resp_len < 0){
        printf("\nFUGAZI_NL: recv failed");
        return (FAILED);
    }

     /* Validate response message */
     if (!NLMSG_OK((&ans->n), (uint32_t)resp_len)){
        printf("\nFUGAZI_NL: invalid reply message\n");
        return (FAILED);
     }

    if (ans->n.nlmsg_type == NLMSG_ERROR) { /* error */
        printf("\nFUGAZI_NL: received error\n");
        return (FAILED);
     }

     return (PASSED);
}

/******************************************************************************
 *
 * Function: get_genlmsg_data
 *
 * Description: get message from netlink attribute stream
 *
 * Inputs      : msg: message attribute stream
 *
 * Outputs     : Data address offset.
 *
 *****************************************************************************/
static inline uint8_t *get_genlmsg_data(bcm_nl_request_msg_t *msg) {
    uint8_t *t =(uint8_t *)NLMSG_DATA(msg);
    return (t + GENL_HDRLEN);
}

/******************************************************************************
 *
 * Function: nla_next
 *
 * Description: next netlink attribute in attribute stream
 *
 * Inputs      : nla       - netlink attribute
 *             : remaining - number of bytes remaining in attribute stream
 * Outputs     : Returns the next netlink attribute in the attribute stream and
 *               decrements remaining by the size of the current attribute.
 *****************************************************************************/
static inline struct nlattr *nla_next (struct nlattr *nla, int *remaining)
{
        unsigned int totlen = NLA_ALIGN(nla->nla_len);

        *remaining -= totlen;
        return (struct nlattr *) ((char *) nla + totlen);
}

/*-------------------------------------------------
$Log: bcm82752_test.c,v $
Revision 1.5  2021/10/18 06:28:59  leschen
Support SFP GLC-TE

Revision 1.4  2020/12/29 03:09:03  leschen
Remove bnxt_en operations.

Revision 1.3  2020/11/03 06:17:48  leschen
To support SFP side band test

Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.5  2019/06/27 06:30:17  leschen
Support CL73 utilities.

Revision 1.1.2.4  2019/06/12 06:14:45  leschen
Modify PRBS TX revise settings according to HW's requirement

Revision 1.1.2.3  2019/06/04 06:04:17  leschen
Support PHY PRBS feature

Revision 1.1.2.2  2019/04/09 08:51:06  leschen
Fix Thallium 1G internal loopback issue when external module is not present

Revision 1.1.2.1  2019/03/12 07:41:51  leschen
Initial check in to support BCM82752


$Endlog$
*/
