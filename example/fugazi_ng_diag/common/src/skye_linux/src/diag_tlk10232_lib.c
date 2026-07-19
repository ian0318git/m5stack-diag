/* $Id: diag_tlk10232_lib.c,v 1.2 2015/05/25 03:59:15 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/diag_tlk10232_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_tlk10232_lib.c - Utility Menu and Functions for Skye TLK10232
 *
 * Ported from Woodlawn Project
 * May 2013, steja
 * Copyright (c) 2015 by Cisco Systems, Inc.
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
#include <linux/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "queryflags.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "skye_eth.h"
#include "skye_smi_lib.h"
#include "diag_tlk10232_lib.h"
#include <gxio/mpipe.h>
#include "skye_xaui.h"
#include "nvmonvars.h"

int config_tlk_10232_mode(int);
int read_tlk_10232_reg(int, uint *);
int write_tlk_10232_reg(int, int);
int set_tlk10232_lpbk_bit(void);
int tlk10232_mode_select(void);
int tlk10232_global_reset (void);
int tlk10232_path_reset(void);
int config_tlk_10232_pll(void);
int config_tlk_10232_set_ref_clock(void);
int tlk10232_setup_data_path_xaui_b_to_xaui_a(void);
int tlk10232_disable_clock_out(void);
int config_tlk_10232_set_ref_clock(void);
int config_tlk_10232_polarity_switch_lane_0_tx(void);
int config_tlk_10232_polarity_switch_lane_1_rx(void);
int config_tlk_10232_polarity_switch_lane_3_tx_rx(void);
int tlk10232_xaui_host_to_tlk_cha_to_chb_lpbk_setup(void);
int tlk10232_check_ls_status_ln(int, int, int);
int tlk10232_polarity_switch_hs_tx_rx_channel_b(void);
int tlk_init_config_10gkr_for_host_lbpk(boolean);
int tlk_init_config(int);
int tlk_config_10gkr(void);
int tlk10232_set_ch_b_loopback(void);
int tlk10232_optimize_ch_a(void);


extern void msleep(unsigned long);
extern int cpu0_xaui_bp_lp_test(void);
extern boolean check_10gcap(void);

/******************************************************************************
 *
 * Function: config_tlk_10232_set_ref_clock
 *
 * Description: This function perform the config TLK setting reference clock
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_set_ref_clock (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_set_ref_clock\n");
    }
    /*  Set channel A&B clock to 312.5MHz because we us 312.5MHz reference clock
     *  for TLK10232.
     *  0x1e.0x001d, val 0x0000->0x1000 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_HS_CH_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_HS_CH_CTRL1_MASK)) |
            TLK_10232_HS_CH_CTRL_1_REF_CLK);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_HS_CH_CTRL_1_REF_CLK)) !=
            (TLK_10232_HS_CH_CTRL_1_REF_CLK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_HS_CH_CTRL_1_REF_CLK)),
             (TLK_10232_HS_CH_CTRL_1_REF_CLK));
        return (FAILED);
    }

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_HS_CH_CTRL1_MASK)) |
            TLK_10232_HS_CH_CTRL_1_REF_CLK);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_HS_CH_CTRL_1_REF_CLK)) !=
            (TLK_10232_HS_CH_CTRL_1_REF_CLK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_HS_CH_CTRL_1_REF_CLK)),
             (TLK_10232_HS_CH_CTRL_1_REF_CLK));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_set_ref_clock\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_0_tx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 0 tx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_0_tx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_0_tx\n");
    }
    /*  Polarity switch Lane 0 TX. 0x1e.0x0006, val 0xF115->0x1111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE0_PLL);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_VAL)) !=
            (TLK_10232_LS_SERDES_CTRL_1_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_VAL)),
             (TLK_10232_LS_SERDES_CTRL_1_VAL));
        return (FAILED);
    }

    /* Polarity switch Lane 0 TX. 0x1e.0x0008, val 0x000D->0x400D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | (TLK_10232_LS_SERDES_CTRL_3_INVERT_POLAR);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_VAL)) !=
            (TLK_10232_LS_SERDES_CTRL_3_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_VAL)),
             (TLK_10232_LS_SERDES_CTRL_3_VAL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_0_tx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_0_rx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 0 rx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_0_rx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_0_rx\n");
    }
    /*  Polarity switch Lane 0 RX. 0x1e.0x0006, val 0xF115->0x1111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE0_PLL);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x",
                reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_VAL)) !=
            (TLK_10232_LS_SERDES_CTRL_1_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_VAL)),
             (TLK_10232_LS_SERDES_CTRL_1_VAL));
        return (FAILED);
    }

    /* Polarity switch Lane 0 RX. 0x1e.0x0008, val 0x000D->0x800D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | (TLK_10232_LS_SERDES_CTRL_3_INVERT_POLAR);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_VAL)) !=
            (TLK_10232_LS_SERDES_CTRL_3_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_VAL)),
             (TLK_10232_LS_SERDES_CTRL_3_VAL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_0_rx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_1_rx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 1 rx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_1_rx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_1_rx\n");
    }
    /*  Polarity switch Lane 1 RX. 0x1e.0x0006, val 0xF115->0x2111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE1_RX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX)) !=
            (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX)),
             (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX));
        return (FAILED);
    }

    /* Polarity switch Lane 1 RX. 0x1e.0x0008, val 0x000D->0x800D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_3_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_3_LANE1_RX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX)) !=
            (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX)),
             (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_1_rx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_1_tx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 1 tx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_1_tx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_1_tx\n");
    }
    /*  Polarity switch Lane 1 TX. 0x1e.0x0006, val 0xF115->0x2111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE1_TX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE1_TX)) !=
            (TLK_10232_LS_SERDES_CTRL_1_LANE1_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE1_TX)),
             (TLK_10232_LS_SERDES_CTRL_1_LANE1_TX));
        return (FAILED);
    }

    /* Polarity switch Lane 1 TX. 0x1e.0x0008, val 0x000D->0x400D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_3_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_3_LANE1_TX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE1_TX)) !=
            (TLK_10232_LS_SERDES_CTRL_3_LANE1_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE1_TX)),
             (TLK_10232_LS_SERDES_CTRL_3_LANE1_TX));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_1_tx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_3_tx_rx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 3 tx and rx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_3_tx_rx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_3_tx_rx\n");
    }
    /*  Polarity switch Lane 3 TX RX. 0x1e.0x0006, val 0xF115->0x8111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE1_RX_TX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX_TX)) !=
            (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX_TX)),
             (TLK_10232_LS_SERDES_CTRL_1_LANE1_RX_TX));
        return (FAILED);
    }

    /* Polarity switch Lane 3 TX RX. 0x1e.0x0008, val 0x000D->0xC00D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_3_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_3_LANE1_RX_TX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX_TX)) !=
            (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX_TX)),
             (TLK_10232_LS_SERDES_CTRL_3_LANE1_RX_TX));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_3_tx_rx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_2_tx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 2 tx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_2_tx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_2_tx\n");
    }
    /*  Polarity switch Lane 2 TX. 0x1e.0x0006, val 0xF115->0x4111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE2_TX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE2_TX)) !=
            (TLK_10232_LS_SERDES_CTRL_1_LANE2_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE2_TX)),
             (TLK_10232_LS_SERDES_CTRL_1_LANE2_TX));
        return (FAILED);
    }

    /* Polarity switch Lane 2 TX. 0x1e.0x0008, val 0x000D->0x400D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_3_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_3_LANE2_TX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE2_TX)) !=
            (TLK_10232_LS_SERDES_CTRL_3_LANE2_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE2_TX)),
             (TLK_10232_LS_SERDES_CTRL_3_LANE2_TX));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_2_tx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_polarity_switch_lane_3_rx
 *
 * Description: This function perform the config TLK
 *              setting polarity switch lane 3 rx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_polarity_switch_lane_3_rx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_polarity_switch_lane_3_rx\n");
    }
    /*  Polarity switch Lane 3 RX. 0x1e.0x0006, val 0xF115->0x8111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_LANE3_RX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE3_RX)) !=
            (TLK_10232_LS_SERDES_CTRL_1_LANE3_RX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_LANE3_RX)),
             (TLK_10232_LS_SERDES_CTRL_1_LANE3_RX));
        return (FAILED);
    }

    /* Polarity switch Lane 3 RX. 0x1e.0x0008, val 0x000D->0x800D */
    reg_addr = TLK_10232_LS_SERDES_CTRL_3_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_3_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_3_LANE3_RX);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE3_RX)) !=
            (TLK_10232_LS_SERDES_CTRL_3_LANE3_RX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_3_LANE3_RX)),
             (TLK_10232_LS_SERDES_CTRL_3_LANE3_RX));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_polarity_switch_lane_3_rx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_pll
 *
 * Description: This function perform the config TLK
 *              setting PLL
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_pll (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_pll\n");
    }
    /*  Set channel  A &B LS PLL multiplier factor is 5, for 312.5MHz clock.
     *  0x1e.0x0006, val 0xF115->0xF111 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_LS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_PLL);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_PLL)) !=
            (TLK_10232_LS_SERDES_CTRL_1_PLL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_PLL)),
             (TLK_10232_LS_SERDES_CTRL_1_PLL));
        return (FAILED);
    }

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_LS_SERDES_CTRL_1_PLL_MASK)) |
            TLK_10232_LS_SERDES_CTRL_1_PLL);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LS_SERDES_CTRL_1_PLL)) !=
            (TLK_10232_LS_SERDES_CTRL_1_PLL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_LS_SERDES_CTRL_1_PLL)),
             (TLK_10232_LS_SERDES_CTRL_1_PLL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_pll\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_mode
 *
 * Description: This function config tlk_10232 to operate in XAUI mode or
 *              10GBASE-KR mode.
 *              1. Internal loopback test path : tilera <-> CH. B
 *              2. Backplane loopback test path : tilera <-> CH A <-> CH B
 *              3. TLK10232 Default mode : XAUI CH. A <-> 10G-KR
 *
 * Inputs      : mode
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_mode (int mode)
{
    switch (mode) {
        case XAUIB_TO_10GKR:
            /* TLK First Init configuration */
            if (tlk_init_config(TLK10G) != PASSED) {
                cterr('f', 0, "tlk_init_config failed");
                return (FAILED);
            }
            /* Second Init config */
            if (tlk_config_10gkr() != PASSED) {
                cterr('f', 0, "tlk_init_config_10gkr failed");
                return (FAILED);
            }
            break;
        case XAUIB_TO_XAUIB:  /* Tilera <-> TLK10232 CHB */
            /* Init TLK10232 for XAUI Backplane Loopback */
            if(tlk_init_config(NONTLK10G) != PASSED) {
                cterr('f', 0, "failed tlk_init_config");
                return (FAILED);
            }
            msleep(1000);

            /* XAUI B to XAUI B */
            if(tlk10232_set_ch_b_loopback() != PASSED) {
                cterr('f', 0, "failed tlk10232_set_ch_b_loopback");
                return (FAILED);
            }
            msleep(10);
            break;
        case XAUIB_TO_XAUIA:
            /* Tilera <-> TLK10232 CHB <-> TLK10232 CHA <-> BP */
            /* Init TLK10232 for XAUI Backplane Loopback */
            if(tlk_init_config(NONTLK10G) != PASSED) {
                printf("failed tlk_init_config");
                return (FAILED);
            }

            /* Optimization CH-A LS */
            if (tlk10232_optimize_ch_a() != PASSED) {
                cterr('f', 0, "failed tlk10232_optimize_ch_a");
                return (FAILED);
            }

            /* Set up  data path. 0x4c20->0xac20 XAUI B to XAUI A */
            if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED) {
                cterr('f', 0, "TLK10232 setup data path xaui B to xaui A "
                        "failed");
                return (FAILED);
            }

            msleep(10);
            break;
        default :
            cterr('f', 0, "TLK10232 doesn't support this mode");
            return (FAILED);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_auto_negotiation
 *
 * Description: This function perform the config TLK disable auto negotiation
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_auto_negotiation(void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_auto_negotiation\n");
    }
    /*  Disable auto-negotiation. 0x07.0x0000.bit 12, val 0x3000->0x2000 */
    /* Disable CHA auto-negotiation */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_AN_CTRL;
    reg_addr = TLK_10232_AN_CTRL_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_AN_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_DISABLE_AUTO_NEGOTIATION)) !=
            (TLK_DISABLE_AUTO_NEGOTIATION))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_DISABLE_AUTO_NEGOTIATION)),
                (TLK_DISABLE_AUTO_NEGOTIATION));
        return (FAILED);
    }

    /* Disable CHB auto-negotiation */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_AN_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_DISABLE_AUTO_NEGOTIATION) !=
            (TLK_DISABLE_AUTO_NEGOTIATION))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_DISABLE_AUTO_NEGOTIATION)),
                (TLK_DISABLE_AUTO_NEGOTIATION));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_auto_negotiation\n");
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk10232_disable_link_training
 *
 * Description: This function perform the config TLK disable link training
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_link_training (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_link_training\n");
    }
    /* Disable link training. 0x01.0x0096 val 0x0000 */
    /* Disable CHA link training */
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_LT_TRAIN_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_LINK_TRAIN_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_LT_TRAIN_CTRL_VAL) !=
            (TLK_10232_LT_TRAIN_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_LT_TRAIN_CTRL_VAL)),
                (TLK_10232_LT_TRAIN_CTRL_VAL));
        return (FAILED);
    }

    /* Disable CHB link training */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_LINK_TRAIN_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_LT_TRAIN_CTRL_VAL)) !=
            (TLK_10232_LT_TRAIN_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_LT_TRAIN_CTRL_VAL)),
                (TLK_10232_LT_TRAIN_CTRL_VAL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_link_training\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_stats_check_for_high_speed
 *
 * Description: This function perform the config TLK disable status check for
 *              high speed
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_stats_check_for_high_speed (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_stats_check_for_high_speed\n");
    }
    /* Disable auto HS status check. 0x1E.0x8021. val 0x000f->0x0007f */
    /* Disable CHA auto HS status check */
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_TI_RESERVED_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE) !=
            (TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE)),
             (TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE));
        return (FAILED);
    }

    /* Disable CHB auto HS status check */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE) !=
            (TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE)),
             (TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_stats_check_for_high_speed\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_high_speed_tx_rx
 *
 * Description: This function perform the config TLK disable
 *              high speed tx and rx
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_high_speed_tx_rx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_high_speed_tx_rx\n");
    }
    /* Disable High Speed Tx and Rx 0x1E.0x0003. val 0xA848->0xA040 */
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_HS_SERDES_CTRL_2;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_HS_SERDES_CTRL_2_MASK)) |
            TLK_10232_HS_SERDES_CTRL_2_VAL_3_11);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_HS_SERDES_CTRL_2_VAL_3_11)) !=
            TLK_10232_HS_SERDES_CTRL_2_VAL_3_11)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_HS_SERDES_CTRL_2_VAL_3_11)),
             (TLK_10232_HS_SERDES_CTRL_2_VAL_3_11));
        return (FAILED);
    }

    /* Disable CHB High Speed Tx and Rx */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_HS_SERDES_CTRL_2_MASK)) |
            TLK_10232_HS_SERDES_CTRL_2_VAL_3_11);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_HS_SERDES_CTRL_2_VAL_3_11)) !=
            TLK_10232_HS_SERDES_CTRL_2_VAL_3_11)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_HS_SERDES_CTRL_2_VAL_3_11)),
             (TLK_10232_HS_SERDES_CTRL_2_VAL_3_11));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_high_speed_tx_rx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_clock_out
 *
 * Description: This function perform the config TLK disable
 *              clock out
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_clock_out (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_clock_out\n");
    }
    /* Disable clock out on both channels since clkout pins not terminated
     * 0x1E.0x000d. val 0x2f80->0x3f80 */
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_CLK_CTRL;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | (TLK_10232_CLK_CTRL_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    msleep(1000);
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_CLK_CTRL_CLK_OUT_DISABLE) !=
            TLK_10232_CLK_CTRL_CLK_OUT_DISABLE)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_CLK_CTRL_CLK_OUT_DISABLE)),
             (TLK_10232_CLK_CTRL_CLK_OUT_DISABLE));
        return (FAILED);
    }

    /* Disable clock out on both channels */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | (TLK_10232_CLK_CTRL_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    msleep(1000);
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_CLK_CTRL_CLK_OUT_DISABLE) !=
            TLK_10232_CLK_CTRL_CLK_OUT_DISABLE)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_CLK_CTRL_CLK_OUT_DISABLE)),
             (TLK_10232_CLK_CTRL_CLK_OUT_DISABLE));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End of tlk10232_disable_clock_out\n");
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk10232_xaui_to_xaui_configuration
 *
 * Description: Set up TLK10232 registers for xaui path loopback test
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_xaui_to_xaui_configuration (int mode)
{
    boolean check1 = TRUE;  /* disable high speed tx rx */
    boolean check2 = TRUE;  /* disable stats check high speed */

    if (mode == TLK10G) {
        check1 = FALSE;
        check2 = FALSE;
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        if (getc_answer("disable high speed tx rx ?(y/n)", "yn",'n') == 'y')
            check1 = TRUE;
        else
            check1 = FALSE;

        if (getc_answer("disable stats check high speed ?"
                "(y/n)", "yn",'n') == 'y')
            check2 = TRUE;
        else
            check2 = FALSE;
    }

    if (tlk10232_disable_auto_negotiation() != PASSED) {
        cterr('f', 0, "Failed Disable Auto Negotiation");
        return (FAILED);
    }


    if (tlk10232_disable_link_training() != PASSED) {
        cterr('f', 0, "Failed Disable Link Training");
        return (FAILED);
    }

    if (check1 == TRUE) {
        if (tlk10232_disable_high_speed_tx_rx() != PASSED) {
            cterr('f', 0, "Failed Disable High Speed");
            return (FAILED);
        }
    }

    if (check2 == TRUE) {
        if (tlk10232_disable_stats_check_for_high_speed()) {
            cterr('f', 0, "Failed Disable Status Check For High Speed");
            return (FAILED);
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: set_tlk10232_lpbk_bit
 *
 * Description: Set up TLK10232 deep remote lpbk bit for GE BP loopback test
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
set_tlk10232_lpbk_bit (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    /* Set deep remote lpbk bit */
    dev_id = TLK_10232_LPBK_TP_CTRL;
    reg_addr = TLK_10232_LPBK_TP_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val = mii_value | TLK_10232_LPBK_TP_VAL;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_mode_select
 *
 * Description: Set up TLK10232 by mode selection
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_mode_select (void)
{
    int rv;
    char val;
    printf("TLK10232 mode selection\n");
    val = getdec_answer("1-XAUI <-> 10GKR, 2-XAUIB <-> XAUIB, "
            "3-XAUIB <-> XAUIA", 1, 1, 3);
    rv = config_tlk_10232_mode(val);
    if (rv != PASSED) {
        cterr('f', 0, "TLK10232 configuration failed");
        return (FAILED);
    }
    
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_global_reset
 *
 * Description: Set up TLK10232 by global reset
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_global_reset (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_global_reset\n");
    }
    /* Do TLK10232 global reset
    0x1E.0x0000.bit 15, val 0x0610->0x8610 */
    dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
    reg_addr = TLK_10232_GLOBAL_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_GLOBAL_RESET;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_GLOBAL_RESET_DEFAULT)) !=
            (TLK_10232_GLOBAL_RESET_DEFAULT))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_GLOBAL_RESET_DEFAULT)),
                (TLK_10232_GLOBAL_RESET_DEFAULT));
        return (FAILED);
    }

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_GLOBAL_RESET;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_GLOBAL_RESET_DEFAULT)) !=
            (TLK_10232_GLOBAL_RESET_DEFAULT))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_GLOBAL_RESET_DEFAULT)),
                (TLK_10232_GLOBAL_RESET_DEFAULT));
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_global_reset\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_setup_data_path_xaui_b_to_xaui_a
 *
 * Description: Set up TLK10232 data path xaui Ch-B to xaui Ch-A
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_setup_data_path_xaui_b_to_xaui_a (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_setup_data_path_xaui_b_to_xaui_a\n");
    }
    /* Set up CHA data path. 0x1E.0x001A. val 0x4c20->0xac20 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
    reg_addr = TLK_10232_DSR_CONTROL_2_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    val = (mii_value & (~TLK_10232_DSR_CONTROL_2_MASK_VAL_2)) |
                (~TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHA);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHA) !=
            (TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHA))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHA),
             (TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHA));
        return (FAILED);
    }

    /* Set up CHB data path. 0x1E.0x001A. val 0x4c20->0xac20*/
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    val = (mii_value & (~TLK_10232_DSR_CONTROL_2_MASK_VAL_2)) |
                (~TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHB);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHB) !=
            (TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHB))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHB),
             (TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHB));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_setup_data_path_xaui_b_to_xaui_a\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_path_reset
 *
 * Description: Set up TLK10232 by resetting the path
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_path_reset (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_path_reset\n");
    }
    /* Clear data path. 0x1E.0x000e. val 0x0000->0x0008 */
    /* CHA data path reset */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_RESET_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PATH_RESET;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_RESET_CTRL_VAL)) != (TLK_10232_RESET_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_RESET_CTRL_VAL)),
                (TLK_10232_RESET_CTRL_VAL));
        return (FAILED);
    }
    
    /* CHB data path reset */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PATH_RESET;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_RESET_CTRL_VAL)) != (TLK_10232_RESET_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_RESET_CTRL_VAL)),
                (TLK_10232_RESET_CTRL_VAL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_path_reset\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_ch_a_loopback
 *
 * Description: Set up TLK10232 Ch-A loopback
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_a_loopback (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_set_ch_a_loopback\n");
    }
    /* Clear data path. 0x1E.0x001a. val 0x4C20->0x0c20 */
    /* CHA loopback */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_DSR_CONTROL_2_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = ((mii_value & (~TLK_10232_DSR_CTRL_LOOPBACK_MASK)) |
            TLK_10232_DSR_CTRL_LOOPBACK);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_DSR_CTRL_LOOPBACK)) !=
            (TLK_10232_DSR_CTRL_LOOPBACK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_DSR_CTRL_LOOPBACK)),
                (TLK_10232_DSR_CTRL_LOOPBACK));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_ch_a_loopback\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_ch_b_loopback
 *
 * Description: Set up TLK10232 Ch-B loopback
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_b_loopback (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;

    /* Clear data path. 0x1E.0x001a. val 0x4C20->0x0c20 */
    /* CHB loopback */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_DSR_CONTROL_2_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val = ((mii_value & (~TLK_10232_DSR_CONTROL_2_MASK_VAL_2)) |
            TLK_10232_DSR_DAT_SW_MODE_ANYDATA);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_DSR_ANY_CTRL_LBPK)) !=
            (TLK_10232_DSR_ANY_CTRL_LBPK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_DSR_ANY_CTRL_LBPK)),
                (TLK_10232_DSR_ANY_CTRL_LBPK));
        return (FAILED);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_ch_a_host_ge_loopback
 *
 * Description: Set up TLK10232 Ch-A to host GE loopback
 *              CPU0 XAUI -> TLK CH-B -> CH-A -> CH-B -> CPU0 XAUI
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_a_host_ge_loopback (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_ch_a_loopback\n");
    }
    /* Clear data path. 0x1E.0x000b. val 0x0d10->0x0d18 */
    /* CHA host loopback */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LPBK_TP_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_LPBK_TP_VAL;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL)) !=
            (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL)),
                (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_ch_a_loopback\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_ch_b_host_ge_loopback
 *
 * Description: Set up TLK10232 Ch-B to host GE loopback
 *              CPU0 XAUI -> TLK CH-B -> CH-B -> CPU0 XAUI
 * Inputs      : enable / disable loopback bit
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_b_host_ge_loopback (boolean enable)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_ch_b_host_ge_loopback\n");
    }
    /* Clear data path. 0x1E.0x000b. val 0x0d10->0x0d18 */
    /* CHB host loopback */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LPBK_TP_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    if (enable == TRUE) {
        val = mii_value | TLK_10232_LPBK_TP_VAL;
    } else {
        val = mii_value & (~TLK_10232_LPBK_TP_VAL);
    }
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if (enable == TRUE) {
    if((mii_value & (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL)) !=
            (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL)),
                (TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL));
        return (FAILED);
    }
    } else {
        if((mii_value & (0x0d10)) != (0x0d10))
        {
            cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                    (mii_value & (0x0d10)), (0x0d10));
            return (FAILED);
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_ch_b_host_ge_loopback\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_chb_hs_tx_rx
 *
 * Description: Set up TLK10232 Ch-B high speed Tx and Rx
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_chb_hs_tx_rx (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_chb_hs_tx_rx\n");
    }
    /* Clear data path. 0x1E.0x0005. val 0x2000->0xe000 */
    /* Set Channel B HS TX & RX polarity switch */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_HS_SERDES_CTRL_4_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_HS_SERDES_CTRL_4_LPBK;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_HS_SERDES_CTRL_4_VAL)) !=
            (TLK_10232_HS_SERDES_CTRL_4_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_HS_SERDES_CTRL_4_VAL)),
                (TLK_10232_HS_SERDES_CTRL_4_VAL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_chb_hs_tx_rx\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_dump_all_reg
 *
 * Description: Dump all TLK register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_dump_all_reg (void)
{
    int phy_id, dev_id, reg_addr, mii_value;
    int port, ix;
    /* TLK 10232 xgbe2 */
    port = 2;
    cterr_db_print("Dump all Reg\n");

    for (phy_id = 0x10; phy_id < 0x12; phy_id++) {
        dev_id = TLK_10232_RESET_CTRL;
        cterr_db_print("Dev_id(0x%x)\n", dev_id);
        for (ix = 0; ix < (0x20); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        for (ix = 0x8003; ix < (0x8005); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        reg_addr = 0x8021;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x", reg_addr,
                    dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);

        for (ix = 0x802A; ix < (0x8030); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        for (ix = 0x8040; ix < (0x8043); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        for (ix = 0x8100; ix < (0x8102); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        /* dev 0x01 */
        dev_id = 0x01;
        cterr_db_print("Dev_id(0x%x)\n", dev_id);
        for (ix = 0; ix < (0xC); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        for (ix = 0x96; ix < (0x9C); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        for (ix = 0xA1; ix < (0xB0); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        for (ix = 0x8001; ix < (0x801F); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        /* LT_VS_CONTROL_2 */
        reg_addr = 0x9001;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x",
                    reg_addr, dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);

        /* PCS Registers */
        dev_id = 0x03;
        for (ix = 0x0; ix < (0x2); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        reg_addr = 0x0008;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x",
                    reg_addr, dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);

        for (ix = 0x20; ix < (0x2C); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        reg_addr = 0x8000;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x", reg_addr,
                    dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);

        reg_addr = 0x8010;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x", reg_addr,
                    dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);

        /* Auto Negotiation Register */
        dev_id = 0x07;
        for (ix = 0x0; ix < (0x2); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        reg_addr = 0x5;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x", reg_addr,
                    dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);

        for (ix = 0x10; ix < (0x1C); ix++) {
            reg_addr = ix;
            mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
            if (mii_value < 0) {
                cterr_db_print("Read reg %x error from device %x, phy %x",
                        reg_addr, dev_id, phy_id);
                return (FAILED);
            }
            cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
        }

        reg_addr = 0x30;
        mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
        if (mii_value < 0) {
            cterr_db_print("Read reg %x error from device %x, phy %x",
                    reg_addr, dev_id, phy_id);
            return (FAILED);
        }
        cterr_db_print("Reg(0x%x)=0x%x\n", reg_addr, mii_value);
    }
#ifdef DEBUG
    printf("\n\n");
    dev_id = TLK_10232_RESET_CTRL;
    cterr_db_print("Ch-B (0x%x)\n", dev_id);
    for (ix = 0; ix < (0x20); ix++) {
    reg_addr = ix;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr_db_print("Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    }
#endif
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_dump_all_reg\n");
    }
    return (PASSED);
}


/*******************************************************************
 *
 * Function    : read_tlk_10232_reg
 * Description : SMI read funtion for tlk_10232 reg test.
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
int
read_tlk_10232_reg (int addr, uint *buff)
{
    uint mii_value;
    int phy_id, dev_id;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_REG_DEVICE_30;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read error from device %x(0x%x)", dev_id, phy_id);
        return (mii_value);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("%s() link is xgbe%d phy_id is %x dev_id is %x "
                    "mii_value is %x\n",
                __FUNCTION__, port, phy_id, dev_id, mii_value);
        }
        buff[0] = mii_value;
        return (PASSED);
    }
}

/*******************************************************************
 *
 * Function    : write_tlk_10232_reg
 * Description : SMI write funtion for tlk_10232 reg test.
 * Input       : addr  - register offset.
 *               value - data to be written.
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
int
write_tlk_10232_reg (int addr, int val)
{
    int status;
    int phy_id, dev_id;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_REG_DEVICE_30;

    status = skye_tlk_reg_wr(port, phy_id, dev_id, addr, val);
    if (status < 0) {
        cterr('f', 0, "Write error to device %x(0x%x)", dev_id, phy_id);
        return (status);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("%s() link is xgbe%d phy_id is %x dev_id is %x val is %x\n",
                __FUNCTION__, port, phy_id, dev_id, val);
        }
        return(PASSED);
    }

}


/******************************************************************************
 *
 * Function: tlk10232_polarity_configs
 *
 * Description: TLK Polarity configuration
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tlk10232_polarity_configs (void)
{
    if (tlk10232_disable_clock_out() != PASSED){
        cterr('f', 0, "failed disable clock out");
        return (FAILED);
    }
    if (config_tlk_10232_set_ref_clock() != PASSED) {
        cterr('f', 0, "failed config_tlk_10232_set_ref_clock");
        return (FAILED);
    }
    if (config_tlk_10232_pll() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_pll");
        return (FAILED);
    }
#ifdef SKYE_TLK_POLARITY
    if (config_tlk_10232_polarity_switch_lane_0_rx() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_0_rx");
        return (FAILED);
    }
    if (config_tlk_10232_polarity_switch_lane_1_tx() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_1_tx");
        return (FAILED);
    }
    if (config_tlk_10232_polarity_switch_lane_3_tx_rx() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_3_tx_rx");
        return (FAILED);
    }

    if (tlk10232_polarity_switch_hs_tx_rx_channel_b() != PASSED) {
        cterr('f', 0, "failed config polarity switch hs tx rx ch b");
        return (FAILED);
    }
#endif
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk_init_config
 *
 * Description: TLK init configuration
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk_init_config (int mode)
{
    boolean check_reset = TRUE;  /* reset data path for both channels */

    if (mode == TLK10G) {
        check_reset = FALSE;
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        if (getc_answer("want to reset data path for both channels?(y/n)",
                "yn",'n') == 'y')
            check_reset = TRUE;
        else
            check_reset = FALSE;
    }

     /* Init TLK10232 */
    if (tlk10232_global_reset()!= PASSED) {
        cterr('f', 0, "failed global reset");
        return (FAILED);
    }
    msleep(100);

    /* Shrinkray HW need Polarity reverse, Skye P1A and P1B no need
     * Polarity reverse */
    if (tlk10232_polarity_configs() != PASSED) {
        cterr('f', 0, "failed setting polarity configs");
        return (FAILED);
    }

    if (tlk10232_disable_auto_negotiation() != PASSED) {
        cterr('f', 0, "Failed Disable Auto Negotiation");
        return (FAILED);
    }

    if (tlk10232_disable_link_training() != PASSED) {
        cterr('f', 0, "Failed Disable Link Training");
        return (FAILED);
    }

    if (tlk10232_xaui_to_xaui_configuration(mode) != PASSED ){
        cterr('f', 0, "failed tlk10232_xaui_to_xaui_configuration");
        return (FAILED);
    }

    if (check_reset == TRUE) {
        if (tlk10232_path_reset() != PASSED ){
            cterr('f', 0, "failed tlk10232_path_reset");
            return (FAILED);
        }
    }
    msleep(100);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk_init_config_2
 *
 * Description: TLK init configuration 2
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk_init_config_2 (void)
{
    /* Init TLK10232 for XAUI Backplane Loopback */
    if (tlk10232_global_reset()!= PASSED) {
        cterr('f', 0, "failed global reset");
        return (FAILED);
    }
    msleep(100);

    if (tlk10232_disable_high_speed_tx_rx() != PASSED) {
        cterr('f', 0, "Failed Disable High Speed");
        return (FAILED);
    }

    if (tlk10232_disable_stats_check_for_high_speed()) {
        cterr('f', 0, "Failed Disable Status Check For High Speed");
        return (FAILED);
    }

    if (tlk10232_disable_clock_out() != PASSED){
        cterr('f', 0, "failed disable clock out");
        return (FAILED);
    }
    if (config_tlk_10232_set_ref_clock() != PASSED) {
        cterr('f', 0, "failed config_tlk_10232_set_ref_clock");
        return (FAILED);
    }
    if (config_tlk_10232_pll() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_pll");
        return (FAILED);
    }
#ifdef SKYE_TLK_POLARITY
    if (config_tlk_10232_polarity_switch_lane_0_tx() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_0_tx");
        return (FAILED);
    }
    if (config_tlk_10232_polarity_switch_lane_1_rx() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_1_rx");
        return (FAILED);
    }
    if (config_tlk_10232_polarity_switch_lane_3_tx_rx() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_3_tx_rx");
        return (FAILED);
    }
#endif
    if (tlk10232_disable_auto_negotiation() != PASSED) {
        cterr('f', 0, "Failed Disable Auto Negotiation");
        return (FAILED);
    }

    if (tlk10232_disable_link_training() != PASSED) {
        cterr('f', 0, "Failed Disable Link Training");
        return (FAILED);
    }

#ifdef DEBUG
    if (tlk10232_xaui_to_xaui_configuration(NONTLK10G) != PASSED ){
        cterr('f', 0, "failed tlk10232_xaui_to_xaui_configuration");
        return (FAILED);
    }
#endif
    if (tlk10232_path_reset() != PASSED ){
        cterr('f', 0, "failed tlk10232_path_reset");
        return (FAILED);
    }
    msleep(100);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk_init_config_3
 *
 * Description: TLK init configuration 3
 * Inputs      : polarity - set polarity
 *               interlpbk - set internal loopback
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk_init_config_3 (boolean polarity, boolean interlpbk)
{
    /* Init TLK10232 for XAUI Backplane Loopback */
    if (tlk10232_global_reset()!= PASSED) {
        cterr('f', 0, "failed global reset");
        return (FAILED);
    }
    msleep(100);
    /* internal loopback cpu0 -> CHB will fail if disable high speed */
    if (interlpbk == FALSE) {
        if (tlk10232_disable_high_speed_tx_rx() != PASSED) {
            cterr('f', 0, "Failed Disable High Speed");
            return (FAILED);
        }
    }

    if (tlk10232_disable_stats_check_for_high_speed()) {
        cterr('f', 0, "Failed Disable Status Check For High Speed");
        return (FAILED);
    }

    if (tlk10232_disable_clock_out() != PASSED){
        cterr('f', 0, "failed disable clock out");
        return (FAILED);
    }
    if (config_tlk_10232_set_ref_clock() != PASSED) {
        cterr('f', 0, "failed config_tlk_10232_set_ref_clock");
        return (FAILED);
    }
    if (config_tlk_10232_pll() != PASSED ){
        cterr('f', 0, "failed config_tlk_10232_pll");
        return (FAILED);
    }
    if (polarity == TRUE) {
#ifdef SKYE_TLK_POLARITY
        if (config_tlk_10232_polarity_switch_lane_0_rx() != PASSED ){
            cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_0_rx");
            return (FAILED);
        }

        if (config_tlk_10232_polarity_switch_lane_1_tx() != PASSED ){
            cterr('f', 0, "failed config_tlk_10232_polarity_switch_lane_1_rx");
            return (FAILED);
        }

        if (config_tlk_10232_polarity_switch_lane_3_tx_rx() != PASSED ){
            cterr('f', 0, "failed config_tlk_10232_polarity_switch_"
                    "lane_3_tx_rx");
            return (FAILED);
        }
#endif
    }

    if (tlk10232_disable_auto_negotiation() != PASSED) {
        cterr('f', 0, "Failed Disable Auto Negotiation");
        return (FAILED);
    }

    if (tlk10232_disable_link_training() != PASSED) {
        cterr('f', 0, "Failed Disable Link Training");
        return (FAILED);
    }

#ifdef DEBUG
    if (tlk10232_xaui_to_xaui_configuration(NONTLK10G) != PASSED ){
        cterr('f', 0, "failed tlk10232_xaui_to_xaui_configuration");
        return (FAILED);
    }
#endif
    if (tlk10232_path_reset() != PASSED ){
        cterr('f', 0, "failed tlk10232_path_reset");
        return (FAILED);
    }
    msleep(100);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_ge_host_lpbk_setup
 *
 * Description: Setup TLK ge host loopback
 * Inputs      : enable / disable loopback bit
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_ge_host_lpbk_setup (boolean enable)
{
    if (check_10gcap() == TRUE) {
        if (enable == TRUE) {
            tlk_init_config_10gkr_for_host_lbpk(enable);
        } else {
            if (tlk10232_set_ch_b_host_ge_loopback(enable) != PASSED ){
                cterr('f', 0, "failed set ch b ge loopback bit");
                return (FAILED);
            }
        }
    } else {
        if (enable == TRUE) {
            /* Init TLK10232 for XAUI Backplane Loopback */
            if (tlk10232_global_reset()!= PASSED) {
                cterr('f', 0, "failed global reset");
                return (FAILED);
            }

            if (config_tlk_10232_set_ref_clock() != PASSED) {
                cterr('f', 0, "failed config_tlk_10232_set_ref_clock");
                return (FAILED);
            }
#ifdef DBG_PING /* Possible issue the intermittent ping */
            if (config_chb_hs_tx_rx() != PASSED) {
                cterr('f', 0, "failed config_tlk_10232_set_ref_clock");
                return (FAILED);
            }
#endif
            if (tlk10232_path_reset() != PASSED ){
                cterr('f', 0, "failed tlk10232_path_reset");
                return (FAILED);
            }

            if (tlk10232_set_ch_b_host_ge_loopback(enable) != PASSED ){
               cterr('f', 0, "failed tlk10232_set_1gkx_loopback");
               return (FAILED);
            }
        } else {
            if (tlk10232_set_ch_b_host_ge_loopback(enable) != PASSED ){
               cterr('f', 0, "failed tlk10232_set_1gkx_loopback");
               return (FAILED);
            }
        }

    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_xaui_host_lpbk_setup
 *
 * Description: Setup TLK Xaui host loopback
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_xaui_host_lpbk_setup (void)
{
    if (tlk_init_config_2() != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if (tlk10232_set_ch_a_loopback() != PASSED ){
       cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
       return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: host_full_path_to_CPU0_polarity_set
 *
 * Description: Setup TLK full path to test loopback from host to CPU0
 *              w/ polarity
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
host_full_path_to_CPU0_polarity_set (void)
{
    if (tlk_init_config_3(TRUE, FALSE) != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: host_full_path_to_CPU0_no_polarity_set
 *
 * Description: Setup TLK full path to test loopback from host to CPU0
 *              w/o polarity
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
host_full_path_to_CPU0_no_polarity_set (void)
{
    if (tlk_init_config_3(FALSE, FALSE) != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: host_to_cha_to_chb_to_cha_to_host_polarity_set
 *
 * Description: Setup TLK from host --> ch-A --> ch-B --> ch-A --> host
 *              w/ polarity
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
host_to_cha_to_chb_to_cha_to_host_polarity_set (void)
{
    if (tlk_init_config_3(TRUE, FALSE) != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if (tlk10232_set_ch_b_host_ge_loopback(TRUE) != PASSED ){
       cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
       return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: host_to_cha_to_chb_to_cha_to_host_polarity_set
 *
 * Description: Setup TLK from host --> ch-A --> ch-B --> ch-A --> host
 *              w/o polarity
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
host_to_cha_to_chb_to_cha_to_host_no_polarity_set (void)
{
    if (tlk_init_config_3(FALSE, FALSE) != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if (tlk10232_set_ch_b_host_ge_loopback(TRUE) != PASSED ){
       cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
       return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_xaui_host_polarity_set
 *
 * Description: Setup TLK from xaui to host
 *              w/ polarity
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_xaui_host_polarity_set (void)
{
    if (tlk_init_config_3(TRUE, FALSE) != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if (tlk10232_set_ch_a_loopback() != PASSED ){
       cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
       return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_xaui_host_polarity_set
 *
 * Description: Setup TLK from xaui to host
 *              w/o polarity
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_xaui_host_no_polarity_set (void)
{
    if (tlk_init_config_3(FALSE, FALSE) != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if (tlk10232_set_ch_a_loopback() != PASSED ){
       cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
       return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_xaui_host_to_tlk_cha_to_chb_lpbk_setup
 *
 * Description: Setup TLK from xaui host -> Ch-A -> Ch-B -> ChA -> host
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_xaui_host_to_tlk_cha_to_chb_lpbk_setup (void)
{
    if (tlk_init_config_2() != PASSED ){
       cterr('f', 0, "failed tlk_init_config");
       return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI A to XAUI B */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }
    /* Set Deep remote loopback bit Ch B */
    if (tlk10232_set_ch_b_host_ge_loopback(TRUE) != PASSED ){
       cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
       return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_check_status_ls_pll_lock
 *
 * Description: Setup TLK check status PLL Lock
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_check_status_ls_pll_lock (void)
{
    int phy_id, dev_id, reg_addr, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_check_status_ls_pll_lock\n");
    }
    /* Check MC_AUTO_CONTROL Reg. 0x1E.0x000F */
    /* CHA check status pll lock check */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_CHANNEL_STATUS_1_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    printf("[%s](0x%x) CH-A read[2] = %x\n","MC_AUTO_CONTROL", reg_addr,
            mii_value);

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    printf("[%s](0x%x) CH-B read[2] = %x\n","MC_AUTO_CONTROL", reg_addr,
            mii_value);

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_check_status_ls_pll_lock\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_check_ls_ln_err_count
 *
 * Description: Setup TLK check lane n error counter
 * Inputs      : reg_addr - register address
 *               lane     - each lane
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_check_ls_ln_err_count (int reg_addr, int lane)
{
    int phy_id, dev_id, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_check_ls_ln%d_err_count\n", lane);
    }
    /* Check LS_LN0_ERROR_COUNTER Reg. 0x1E.0x0011 */
    dev_id = TLK_10232_RESET_CTRL;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* FAE Suggest read 2nd times */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    printf("[LS_LN%d_ERROR_COUNTER](0x%x) CH-A read = %x\n", lane, reg_addr,
            mii_value);

    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    printf("[LS_LN%d_ERROR_COUNTER](0x%x) CH-B read = %x\n", lane, reg_addr,
            mii_value);

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_check_ls_ln%d_err_count\n", lane);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_check_status
 *
 * Description: Setup TLK check status
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_check_status (void)
{
    if(tlk10232_check_status_ls_pll_lock() != PASSED) {
        cterr('f', 0, "failed tlk10232_check_status_ls_pll_lock");
        return (FAILED);
    }

    if(tlk10232_check_ls_ln_err_count(TLK_10232_LS_LN0_ERROR_COUNTER_REG, 0) !=
            PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_ln0_err_count");
        return (FAILED);
    }
    if(tlk10232_check_ls_ln_err_count(TLK_10232_LS_LN1_ERROR_COUNTER_REG, 0) !=
            PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_ln1_err_count");
        return (FAILED);
    }
    if(tlk10232_check_ls_ln_err_count(TLK_10232_LS_LN2_ERROR_COUNTER_REG, 0) !=
            PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_ln2_err_count");
        return (FAILED);
    }
    if(tlk10232_check_ls_ln_err_count(TLK_10232_LS_LN3_ERROR_COUNTER_REG, 0) !=
            PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_ln3_err_count");
        return (FAILED);
    }

    if(tlk10232_check_ls_status_ln(TLK_10232_CHK_LS_STATUS_LANE0,
            TLK_10232_CHK_LS_STATUS_LANE0, 0) != PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_status_ln0");
        return (FAILED);
    }
    if(tlk10232_check_ls_status_ln(TLK_10232_LANE1,
            TLK_10232_CHK_LS_STATUS_LANE1, 1) != PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_status_ln1");
        return (FAILED);
    }
    if(tlk10232_check_ls_status_ln(TLK_10232_LANE2,
            TLK_10232_CHK_LS_STATUS_LANE2, 2) != PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_status_ln2");
        return (FAILED);
    }
    if(tlk10232_check_ls_status_ln(TLK_10232_LANE3,
            TLK_10232_CHK_LS_STATUS_LANE3, 3) != PASSED) {
        cterr('f', 0, "failed tlk10232_check_ls_status_ln3");
        return (FAILED);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_ctc
 *
 * Description: Setup TLK disable ctc
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_ctc (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_disable_ctc\n");
    }
    /* Set up CHA data path. 0x1E.0x001d. val 0x1000->0x1c00 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
    reg_addr = TLK_10232_HS_CH_CTRL_1_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    val = mii_value | TLK_10232_CTC_BYPASS_DISABLE;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & TLK_10232_CTC_BYPASS_DISABLE) !=
            (TLK_10232_CTC_BYPASS_DISABLE))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & TLK_10232_CTC_BYPASS_DISABLE),
             (TLK_10232_CTC_BYPASS_DISABLE));
        return (FAILED);
    }

    /* Set up CHB data path. 0x1E.0x001d. val 0x1000->0x1c00 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    val = mii_value | TLK_10232_CTC_BYPASS_DISABLE;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & TLK_10232_CTC_BYPASS_DISABLE) !=
            (TLK_10232_CTC_BYPASS_DISABLE))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
              (mii_value & TLK_10232_CTC_BYPASS_DISABLE),
              (TLK_10232_CTC_BYPASS_DISABLE));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_ctc\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_kr_fifo_ctrl
 *
 * Description: Setup TLK kr fifo control
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_kr_fifo_ctrl (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_kr_fifo_ctrl\n");
    }
    /* Set up CHA data path. 0x01.0x8001. val 0xcc4c->0xa020 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_KR_FIFO_CTRL_1_REG;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    val =(((~mii_value) | TLK_10232_KR_FIFO_CTRL1_MASK) &
            TLK_10232_KR_FIFO_CTRL);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & TLK_10232_KR_FIFO_CTRL) != (TLK_10232_KR_FIFO_CTRL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & TLK_10232_KR_FIFO_CTRL), (TLK_10232_KR_FIFO_CTRL));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_kr_fifo_ctrl\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_ch_b_deep_local_loopback
 *
 * Description: Set up TLK10232 Ch-B deep local loopback
 *              CPU0 XAUI -> TLK CH-B -> CH-B -> CPU0 XAUI
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_b_deep_local_loopback (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_ch_b_deep_local_loopback\n");
    }
    /* Clear data path. 0x1E.0x000b. val 0x0d10->0x0d12 */
    /* CHB host loopback */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LPBK_TP_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_DEEP_LBPK;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_LBPK_TP_CTRL_DEEP_LBPK)) !=
            (TLK_10232_LBPK_TP_CTRL_DEEP_LBPK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_LBPK_TP_CTRL_DEEP_LBPK)),
                (TLK_10232_LBPK_TP_CTRL_DEEP_LBPK));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_ch_b_deep_local_loopback\n");
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk10232_set_ch_b_shallow_local_loopback
 *
 * Description: Set up TLK10232 Ch-B shallow local loopback
 *              CPU0 XAUI -> TLK CH-B -> CH-B -> CPU0 XAUI
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_b_shallow_local_loopback (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_ch_b_shallow_local_loopback\n");
    }
    /* Clear data path. 0x1E.0x000b. val 0x0d10->0x0d11 */
    /* CHB host loopback */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LPBK_TP_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_SHALLOW_LOCAL_LBPK;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_LBPK_TP_CTRL_SHALLOW_LBPK)) !=
            (TLK_10232_LBPK_TP_CTRL_SHALLOW_LBPK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_LBPK_TP_CTRL_SHALLOW_LBPK)),
                (TLK_10232_LBPK_TP_CTRL_SHALLOW_LBPK));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_ch_b_shallow_local_loopback\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_ch_b_pma_loopback
 *
 * Description: Set up TLK10232 Ch-B pma loopback
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_ch_b_pma_loopback (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_ch_b_pma_loopback\n");
    }
    /* Clear data path. 0x01.0x0000. val 0x0000->0x0001 */
    /* CHB host loopback */
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_PMA_CTRL_1_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PMA_CTRL_1_LBPK;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_PMA_CTRL_1_LBPK)) != (TLK_10232_PMA_CTRL_1_LBPK))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_PMA_CTRL_1_LBPK)),
                (TLK_10232_PMA_CTRL_1_LBPK));
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_ch_b_pma_loopback\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_check_ls_status_ln
 *
 * Description: Setup TLK check lane n ls status
 * Inputs      : lane - each lane
 *               checkvalue - check value
 *               value - lane value
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_check_ls_status_ln (int lane, int checkvalue, int value)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("Start tlk10232_check_ls_status_ln%d\n", value);
    }
    /* Select LANE 0, 0x1E. 0x000C <- 0x0330 */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LS_CONFIG_CONTROL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val = mii_value | lane;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & checkvalue) != checkvalue)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & checkvalue), checkvalue);
        return (FAILED);
    }

    /* Check LS_STATUS_1 Reg. 0x1E.0x0015 */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LS_STATUS_1_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    printf("[LS_STATUS_1_LANE%d](0x%x) CH-A read = %x\n",value, reg_addr,
            mii_value);

    /* Select LANE 0, 0x1E. 0x000C <- 0x0330 */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LS_CONFIG_CONTROL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val = mii_value | lane;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & checkvalue) != checkvalue)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & checkvalue), checkvalue);
        return (FAILED);
    }

    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_LS_STATUS_1_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    printf("[LS_STATUS_1_LANE%d](0x%x) CH-B read = %x\n",value, reg_addr,
            mii_value);

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_check_ls_status_ln%d\n", value);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_polarity_switch_hs_tx_rx_channel_b
 *
 * Description: This function TLK config high speed channel b tx and rx
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_polarity_switch_hs_tx_rx_channel_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_polarity_switch_hs_tx_rx_channel_b\n");
    }
    /*  Config polarity switch. 0x1e.0x0005, val 0x2000->0xc005 */
    /* Disable CHB auto-negotiation */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_HS_CH_CTRL_1_DEV;
    reg_addr = TLK_10232_HS_SERDES_CTRL_4_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_HS_SERDES_CTRL_4_MASK);
    val |= TLK_10232_HS_TWPOST2_RX_TX;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_10232_HS_TWPOST2_RX_TX)) !=
            (TLK_10232_HS_TWPOST2_RX_TX))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_HS_TWPOST2_RX_TX)),
                (TLK_10232_HS_TWPOST2_RX_TX));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_polarity_switch_hs_tx_rx_channel_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_chb_ls_hs_switch
 *
 * Description: Sets channel b LH / HS switch
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_chb_ls_hs_switch (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_chb_ls_hs_switch\n");
    }
    /*
    0x1e.0x001a default 0x4c20 */
    dev_id = TLK_10232_HS_CH_CTRL_1_DEV;
    reg_addr = TLK_10232_DSR_CONTROL_2_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_DST_CTRL_2_VAL;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_DST_CTRL_2_VAL)) != (TLK_10232_DST_CTRL_2_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_DST_CTRL_2_VAL)),
                (TLK_10232_DST_CTRL_2_VAL));
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_chb_ls_hs_switch\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_set_link_training
 *
 * Description: Sets link training mode to full region search of all pre/post
 *              values
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_set_link_training (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_set_link_training\n");
    }
    /*
    0x1.0x9001 default 0x0200 -> 0x0201 */
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_TI_RSVD_CTRL2;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_RSVD_CTRL_VAL;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_RSVD_CTRL_VAL)) != (TLK_10232_RSVD_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_RSVD_CTRL_VAL)),
                (TLK_10232_RSVD_CTRL_VAL));
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_set_link_training\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_power_down_ch_a
 *
 * Description: Power down channel A
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_power_down_ch_a (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_power_down_ch_a\n");
    }
    /*
    0x1E.0x0001 default 0x0b00 -> 0x8b24 */
    dev_id = TLK_10232_REG_DEVICE_30;
    reg_addr = TLK_10232_CH_CTRL_1_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PWR_DOWN_CHA;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_CH_CTRL_1_PWR_DOWN_CHA)) !=
            (TLK_10232_CH_CTRL_1_PWR_DOWN_CHA))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_CH_CTRL_1_PWR_DOWN_CHA)),
                (TLK_10232_CH_CTRL_1_PWR_DOWN_CHA));
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_power_down_ch_a\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_pma_ch_a
 *
 * Description: disable pma channel A
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_pma_ch_a (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_pma_ch_a\n");
    }
    /*
    0x1.0x0000 default 0x0000 -> 0x0800 */
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_PMA_CTRL_1_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_DISABLE_PMA;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_DISABLE_PMA)) != (TLK_10232_DISABLE_PMA))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_DISABLE_PMA)),
                (TLK_10232_DISABLE_PMA));
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_pma_ch_a\n");
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk10232_enable_pcs_ch_a
 *
 * Description: disable pcs channel A
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_enable_pcs_ch_a (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_enable_pcs_ch_a\n");
    }
    /*
    0x3.0x0000 default 0x0000 -> 0x0800 */
    dev_id = TLK_10232_PCS_CTRL;
    reg_addr = TLK_10232_PCS_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PCS_CTRL_EN_PCS;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_PCS_CTRL_EN_PCS)) != (TLK_10232_PCS_CTRL_EN_PCS))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
                (mii_value & (TLK_10232_PCS_CTRL_EN_PCS)),
                (TLK_10232_PCS_CTRL_EN_PCS));
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_enable_pcs_ch_a\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_an_channel_b
 *
 * Description: This function TLK disable auto negotiation channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_an_channel_b(void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_an_channel_b\n");
    }
    /*  Disable auto-negotiation. 0x07.0x0000.bit 12, val 0x3000->0x2000 */
    /* Disable CHB auto-negotiation */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_AN_CTRL;
    reg_addr = TLK_10232_AN_CTRL_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_AN_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & (TLK_DISABLE_AUTO_NEGOTIATION)) !=
            (TLK_DISABLE_AUTO_NEGOTIATION))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_DISABLE_AUTO_NEGOTIATION)),
                (TLK_DISABLE_AUTO_NEGOTIATION));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_an_channel_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_disable_lt_channel_b
 *
 * Description: This function config TLK disable link training channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_disable_lt_channel_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_disable_lt_channel_b\n");
    }
    /* Disable link training. 0x01.0x0096 val 0x0000 */
    /* Disable CHB link training */
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_LT_TRAIN_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_LINK_TRAIN_DISABLE);
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_LT_TRAIN_CTRL_VAL) !=
            (TLK_10232_LT_TRAIN_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_LT_TRAIN_CTRL_VAL)),
                (TLK_10232_LT_TRAIN_CTRL_VAL));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_disable_lt_channel_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_path_reset_channel_b
 *
 * Description: Set up TLK10232 by resetting the path channel b
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_path_reset_channel_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_path_reset_channel_b\n");
    }
    /* Clear data path. 0x1E.0x000e. val 0x0000->0x0008 */
    /* Now is using val 0x0000->0x000e */
    /* CHB data path reset */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_RESET_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* val = mii_value | TLK_10232_PATH_RESET; */
    val = mii_value | 0x000e;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (TLK_10232_RESET_CTRL_VAL)) != (TLK_10232_RESET_CTRL_VAL))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_RESET_CTRL_VAL)),
                (TLK_10232_RESET_CTRL_VAL));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_path_reset_channel_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_set_lowest_pll_ch_b
 *
 * Description: This function Set lowest PLL loop bandwidth channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_set_lowest_pll_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_set_lowest_pll_ch_b\n");
    }
    /*  Set channel B to lowest PLL loop bandwidth
     *  0x1e.0x0002, val 0x831D->0x811c */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_HS_SERDES_CTRL_1_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    // Mask PLL Multi and HS loop bandwidth.
    val &= ~TLK_10232_PLL_MULTI_HS_LOOP_MASK;
    val |= TLK_10232_PLL_MULTI_HS_LOOP;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_PLL_MULTI_HS_LOOP) != TLK_10232_PLL_MULTI_HS_LOOP)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_PLL_MULTI_HS_LOOP)),
             (TLK_10232_PLL_MULTI_HS_LOOP));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_set_lowest_pll_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_set_higher_swing_ch_b
 *
 * Description: This function Set higher swing, set AGC_CTRL to
 *              disable receiver attenuation channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_set_higher_swing_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_set_higher_swing_ch_b\n");
    }
    /*  Set channel B to higher swing, set AGC_CTRL to disable rx att ch b
     *  0x1e.0x0003, val 0xA848->0xe888  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_HS_SERDES_CTRL_2;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    // MASK ADAPTIVE GAIN & AUTO ZERO CALIBRATE
    val &= ~TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO_MASK;
    val |= TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO) !=
            TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO)),
             (TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_set_higher_swing_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_set_hs_serdes_ctrl3_ch_b
 *
 * Description: This function
 *              Set EQPRE = 101, CDRFMULT = 00, CDRTHR = 10, PK_DISABLE = 1
 *              channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_set_hs_serdes_ctrl3_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_set_hs_serdes_ctrl3_ch_b\n");
    }
    /*  Set channel B to set hs serdes control 3
     *  0x1e.0x0004, val 0x1500->0x5252  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_LS_SERDES_CTRL_1_DEV;
    reg_addr = TLK_10232_HS_SERDES_CTRL_3;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    // MASK EQPRE, CDRFMULT, CDRTHR, PK_DISABLE
    val &= ~TLK_10232_EQPRE_CDRFMULT_CDRTHR_PK_MASK;
    val |= TLK_10232_SET_HS_SERDES_CTRL_3;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_SET_HS_SERDES_CTRL_3) !=
            TLK_10232_SET_HS_SERDES_CTRL_3)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_SET_HS_SERDES_CTRL_3)),
             (TLK_10232_SET_HS_SERDES_CTRL_3));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_set_hs_serdes_ctrl3_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_set_hs_serdes_ctrl3_ch_b
 *
 * Description: This function
 *              Enable FEC channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_set_kr_fec_ctrl_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_set_kr_fec_ctrl_ch_b\n");
    }
    /*  Set channel B to enable fec 10GBASE-R FEC function
     *  0x1.0x00AB, val 0x0000->0x0001  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_KR_FEC_CTRL_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    // MASK EQPRE, CDRFMULT, CDRTHR, PK_DISABLE
    val &= ~TLK_10232_KR_FEC_CTRL_MASK;
    val |= TLK_10232_EN_FEC;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_EN_FEC) != TLK_10232_EN_FEC)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_EN_FEC)), (TLK_10232_EN_FEC));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_set_kr_fec_ctrl_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_disable_link_training_timeout_ch_b
 *
 * Description: This function disable link training time out channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_disable_link_training_timeout_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_disable_link_training_timeout_ch_b\n");
    }
    /*  Set channel B to disable link training time out
     *  0x1.0x9002, val 0x1335->0x0000  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_TI_RSVD_CTRL3;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val &= ~TLK_10232_TI_RSD_CTRL_3_MASK;

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_DIS_LINK_TRAIN) != TLK_10232_DIS_LINK_TRAIN)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_DIS_LINK_TRAIN)),
             (TLK_10232_DIS_LINK_TRAIN));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_disable_link_training_timeout_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_disable_link_training2_timeout_ch_b
 *
 * Description: This function disable link training2 time out channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_disable_link_training2_timeout_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_disable_link_training2_timeout_ch_b\n");
    }
    /*  Set channel B to disable link training 2 time out
     *  0x1.0x9003, val 0x5E29->0x0000  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_TI_RSVD_CTRL4;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val &= ~TLK_10232_TI_RSD_CTRL_4_MASK;

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_DIS_LINK_TRAIN_2) != TLK_10232_DIS_LINK_TRAIN_2)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_DIS_LINK_TRAIN_2)),
             (TLK_10232_DIS_LINK_TRAIN_2));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_disable_link_training2_timeout_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_en_load_tx_default_ch_b
 *
 * Description: This function enable loading of default TX value channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_en_load_tx_default_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_en_load_tx_default_ch_b\n");
    }
    /*  Set channel B to disable link training 2 time out
     *  0x1e.0x8101, val 0x0000->0x0004  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_TI_RSVD_2_CTRL_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val &= ~TLK_10232_EN_LOAD_TX_DEFAULT_MASK;
    val |= TLK_10232_DIS_EN_LOAD_TX_DEFAULT;

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_DIS_EN_LOAD_TX_DEFAULT) !=
            TLK_10232_DIS_EN_LOAD_TX_DEFAULT)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_DIS_EN_LOAD_TX_DEFAULT)),
             (TLK_10232_DIS_EN_LOAD_TX_DEFAULT));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_en_load_tx_default_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_en_load_tx_default2_ch_b
 *
 * Description: This function enable loading of default 2 TX value channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_en_load_tx_default2_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_en_load_tx_default_ch_b\n");
    }
    /*  Set channel B to disable link training 2 time out
     *  0x1e.0x8100, val 0x0000->0x0004  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_TI_RSVD_1_CTRL_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val &= ~TLK_10232_EN_LOAD_TX_DEFAULT_MASK;
    val |= TLK_10232_DIS_EN_LOAD_TX_DEFAULT;

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_DIS_EN_LOAD_TX_DEFAULT) !=
            TLK_10232_DIS_EN_LOAD_TX_DEFAULT)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_DIS_EN_LOAD_TX_DEFAULT)),
             (TLK_10232_DIS_EN_LOAD_TX_DEFAULT));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_en_load_tx_default_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_en_load_tx_default3_ch_b
 *
 * Description: This function enable loading of default 3 TX value channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_en_load_tx_default3_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_en_load_tx_default3_ch_b\n");
    }
    /*  Set channel B to disable link training 2 time out
     *  0x1e.0x8100, val 0x0000->0x0000  */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_TI_RSVD_1_CTRL_REG;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val &= ~TLK_10232_EN_LOAD_TX_DEFAULT_MASK;
    val |= TLK_10232_EN_LOAD_TX_DEF_3;

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_EN_LOAD_TX_DEF_3) != TLK_10232_EN_LOAD_TX_DEF_3)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_EN_LOAD_TX_DEF_3)),
             (TLK_10232_EN_LOAD_TX_DEF_3));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_en_load_tx_default3_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: config_tlk_10232_set_link_training_prbs_ch_b
 *
 * Description: This function set link training PRBS packet count channel b
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
config_tlk_10232_set_link_training_prbs_ch_b (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    val = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("config_tlk_10232_set_link_training_prbs_ch_b\n");
    }
    /*  Set channel B to set link training PRBS packet count
     *  0x1e.0x9005, val 0x1c00 */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_TI_RSVD_CTRL6;

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val |= TLK_10232_SET_PRBS;

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if((mii_value & TLK_10232_SET_PRBS) != TLK_10232_SET_PRBS)
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x", reg_addr,
             (mii_value & (TLK_10232_SET_PRBS)), (TLK_10232_SET_PRBS));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End config_tlk_10232_set_link_training_prbs_ch_b\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_en_link_training
 *
 * Description: This function perform enable link training and restart
 *
 * Inputs      : on / off
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_en_link_training (boolean onoff)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    reg_addr = 0;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_en_link_training\n");
    }
    /* Enable link training. 0x1e.0x0096 val 0x0003 */
    /* Enable CHB link training */
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_LT_TRAIN_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    if (onoff) {
        val = mii_value & (~TLK_10232_LINK_TRAIN_DISABLE);
        val |= TLK_10232_EN_LINK_TRAIN;
    } else {
        val = mii_value & (~(0x0002));
    }

    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /* read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    /*compare*/
    if (onoff) {
        if((mii_value & TLK_10232_LT_TRAIN_CTRL_DEFAULT) !=
                (TLK_10232_LT_TRAIN_CTRL_DEFAULT))
        {
            cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (TLK_10232_LT_TRAIN_CTRL_DEFAULT)),
                (TLK_10232_LT_TRAIN_CTRL_DEFAULT));
            return (FAILED);
        }
    } else {
        if((mii_value & 0x0000) != (0x0000))
        {
            cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (0x0000)), (0x0000));
            return (FAILED);
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_en_link_training\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_path_reset_channel_b_2
 *
 * Description: Set up TLK10232 by resetting the path channel b mode 2
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_path_reset_channel_b_2 (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_path_reset_channel_b_2\n");
    }
    /* Clear data path. 0x1E.0x9000. val 0x024d */
    /* CHB data path reset */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = 0x9000;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val = mii_value | 0x024d;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*Read back */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    /*compare*/
    if((mii_value & (0x024d)) != (0x024d))
    {
        cterr('f', 0, "Read back reg %x value from %x, expect %x",
                reg_addr, (mii_value & (0x024d)), (0x024d));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_path_reset_channel_b_2\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_write_reg
 *
 * Description: Sets TLK 10232 values
 * Inputs      : phy_id = physical id
 *               dev_id = device id
 *               reg_addr = register address
 *               val      = register value
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_write_reg (int phy_id, int dev_id, int reg_addr, int val)
{
    int mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_write_reg\n");
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | val;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_write_reg\n");
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_read_reg
 *
 * Description: Read TLK 10232 values
 * Inputs      : phy_id = physical id
 *               dev_id = device id
 *               reg_addr = register address
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk10232_read_reg (int phy_id, int dev_id, int reg_addr)
{
    int mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_read_reg\n");
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    msleep(100);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_read_reg\n");
    }
    return (mii_value);
}


/******************************************************************************
 *
 * Function: tlk_init_config_10gkr
 *
 * Description: TLK init configuration for 10G-KR
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk_init_config_10gkr (void)
{
    /* CHB LS / HS Switch */
    if (tlk10232_set_chb_ls_hs_switch() != PASSED) {
        cterr('f', 0, "failed set channel b LS / HS switch");
        return (FAILED);
    }

    /* Test card script init */
    if (tlk10232_set_link_training()!= PASSED) {
        cterr('f', 0, "failed set link training");
        return (FAILED);
    }

    if (tlk10232_power_down_ch_a()!= PASSED) {
        cterr('f', 0, "failed power down channel a");
        return (FAILED);
    }

    if (tlk10232_disable_pma_ch_a()!= PASSED) {
        cterr('f', 0, "failed disable pma channel a");
        return (FAILED);
    }

    if (tlk10232_enable_pcs_ch_a()!= PASSED) {
        cterr('f', 0, "failed disable pcs channel a");
        return (FAILED);
    }

    if (tlk10232_disable_an_channel_b() != PASSED) {
        cterr('f', 0, "Failed Disable Auto Negotiation Ch B");
        return (FAILED);
    }
    /* TI Recommend */
    if (config_tlk_10232_set_lowest_pll_ch_b()!= PASSED) {
        cterr('f', 0, "failed set lowest pll channel b");
        return (FAILED);
    }

    if (config_tlk_10232_set_higher_swing_ch_b()!= PASSED) {
        cterr('f', 0, "failed set higher swing channel b");
        return (FAILED);
    }

    if (config_tlk_10232_set_hs_serdes_ctrl3_ch_b()!= PASSED) {
        cterr('f', 0, "failed set hs serdes ctrl3 channel b");
        return (FAILED);
    }

    if (config_tlk_10232_set_kr_fec_ctrl_ch_b()!= PASSED) {
        cterr('f', 0, "failed set kr fec control channel b");
        return (FAILED);
    }
    /* TI Recommend ---- */
    if (tlk10232_path_reset_channel_b()!= PASSED) {
        cterr('f', 0, "failed data path reset channel b");
        return (FAILED);
    }
    /* Disable Timer MS */
    if (config_tlk_10232_disable_link_training_timeout_ch_b()!= PASSED) {
        cterr('f', 0, "failed disable link training channel b");
        return (FAILED);
    }
    /* Disable Timer LS */
    if (config_tlk_10232_disable_link_training2_timeout_ch_b()!= PASSED) {
        cterr('f', 0, "failed disable link training 2 channel b");
        return (FAILED);
    }
    /* TI Init script */
    if (config_tlk_10232_en_load_tx_default_ch_b()!= PASSED) {
        cterr('f', 0, "failed load tx default channel b");
        return (FAILED);
    }

    if (config_tlk_10232_en_load_tx_default2_ch_b()!= PASSED) {
        cterr('f', 0, "failed load tx default 2 channel b");
        return (FAILED);
    }

    if (config_tlk_10232_en_load_tx_default3_ch_b()!= PASSED) {
        cterr('f', 0, "failed load tx default 3 channel b");
        return (FAILED);
    }

    if (config_tlk_10232_set_link_training_prbs_ch_b()!= PASSED) {
        cterr('f', 0, "failed set link training prbs channel b");
        return (FAILED);
    }

    if (tlk10232_en_link_training(TRUE)!= PASSED) {
        cterr('f', 0, "failed enable link training channel b");
        return (FAILED);
    }
    /* TI Init script ---- */
    return (PASSED);
}

/******************************************************************************
 *
 * Function: skye_kr_restart_an
 *
 * Description: Function for TLK KR check restart auto negotiation bit
 *              FAE recommended settings.
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
void skye_kr_restart_an (void)
{
    int data, ix;
    int retry_count = 50;

    data = tlk10232_read_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x07, 0x0000);
retry_again:
    for (ix = 0; ix < 50; ix++) {
        tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x07, 0x0000, 0x3200);
        msleep (100);

        /* if set, read is required to clear AN_RESTAR bit */

        data = tlk10232_read_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x07, 0x0000);

        /* Sleep 1 second, wait for AN complete */
        msleep(1000);

        data = tlk10232_read_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x07, 0x0001);

        tlk10232_path_reset();

        if (data & 0x0020) {
            printf("KR-AN-link-complete, data = 0x%x\n", data);
            fflush(stdout);
            break;
        } else {
            tlk10232_path_reset();
            printf("KR-AN link not finished - %d, data = 0x%x, retry=%d\n", ix,
                    data, retry_count);
            fflush(stdout);
            retry_count--;
            if (retry_count == 0) {
                printf("KR-AN link time out !\n");
                return;
            } else {
                goto retry_again;
            }
        }
    }
}


/******************************************************************************
 *
 * Function: tlk10232_fae_recom_reg
 *
 * Description: TLK configuration for 10G-KR.
 *              FAE recommended settings.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tlk10232_fae_recom_reg (void)
{
    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x1e, 0x9001, 0x0201)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }

    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x07, 0x0000, 0x3000)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }

    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x01, 0x0096, 0x0002)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }

    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x1e, 0x9005, 0x1c00)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }
    msleep(1000);
    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x1e, 0x8100, 0x0001)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }

    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x1e, 0x0004, 0x5540)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }

    if (tlk10232_write_reg(TLK_10232_PHY_ADDR_CHANNEL_B, 0x01, 0x0096, 0x0003)!=
            PASSED) {
        cterr('f', 0, "failed write reg for tlk fae recommend");
        return (FAILED);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk_config_10gkr
 *
 * Description: TLK configuration for 10G-KR
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk_config_10gkr (void)
{
    /* Pwr down Channel A*/
    if (tlk10232_power_down_ch_a()!= PASSED) {
        cterr('f', 0, "failed power down channel a");
        return (FAILED);
    }

    /* Enable CLKOUTx_P/N power down for channel A*/
    /*Enable CLKOUTx_P/N power down for channel B*/
    if (tlk10232_disable_clock_out() != PASSED){
        cterr('f', 0, "failed disable clock out");
        return (FAILED);
    }

    /*PMA power down for channel A*/
    if (tlk10232_disable_pma_ch_a()!= PASSED) {
        cterr('f', 0, "failed disable pma channel a");
        return (FAILED);
    }

    /* Enable PCS power down */
    if (tlk10232_enable_pcs_ch_a()!= PASSED) {
        cterr('f', 0, "failed disable pcs channel a");
        return (FAILED);
    }

    /* Disable AN */
    if (tlk10232_disable_auto_negotiation() != PASSED) {
        cterr('f', 0, "Failed Disable Auto Negotiation");
        return (FAILED);
    }

    /* Disable LT */
    if (tlk10232_disable_link_training() != PASSED) {
        cterr('f', 0, "Failed Disable Link Training");
        return (FAILED);
    }

    /* Enable FEC */
    if (config_tlk_10232_set_kr_fec_ctrl_ch_b()!= PASSED) {
        cterr('f', 0, "failed set kr fec control channel b");
        return (FAILED);
    }

    /* Data path reset */
    if (tlk10232_path_reset_channel_b()!= PASSED) {
        cterr('f', 0, "failed data path reset channel b");
        return (FAILED);
    }

    if (tlk10232_path_reset_channel_b_2()!= PASSED) {
        cterr('f', 0, "failed data path reset channel b mode 2");
        return (FAILED);
    }

    /* TI init script */
    if (config_tlk_10232_en_load_tx_default_ch_b()!= PASSED) {
        cterr('f', 0, "failed load tx default channel b");
        return (FAILED);
    }

    if (config_tlk_10232_en_load_tx_default2_ch_b()!= PASSED) {
        cterr('f', 0, "failed load tx default 2 channel b");
        return (FAILED);
    }

    if (config_tlk_10232_en_load_tx_default3_ch_b()!= PASSED) {
        cterr('f', 0, "failed load tx default 3 channel b");
        return (FAILED);
    }

    /* Config TLK Register Recommend from FAE */
    if (tlk10232_fae_recom_reg()!= PASSED) {
        cterr('f', 0, "failed to config TLK Reg Recommend from FAE channel b");
        return (FAILED);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk_init_config_10gkr_for_host_lbpk
 *
 * Description: TLK init configuration for 10G-KR from host lpbk
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
tlk_init_config_10gkr_for_host_lbpk (boolean host_test)
{
    boolean run_packet = FALSE;

    if (diagflag_xram & D_SET_OPTIONS) {
        if (getc_answer("want to run packet?(y/n)", "yn",'n') == 'y')
            run_packet = TRUE;
        else
            run_packet = FALSE;

        if (getc_answer("want to enable TLK CH-B HS loopback bit?(y/n)",
                "yn",'n') == 'y')
            host_test = TRUE;
        else
            host_test = FALSE;
    }

    if (config_tlk_10232_mode(XAUIB_TO_10GKR) == FAILED) {
        cterr('f', 0, "Config TLK10232 into XAUIB <->  XAUIB mode failed");
        return (FAILED);
    }

    /* kr restart auto negotiation */
    skye_kr_restart_an();
    msleep(3000);  /* Wait 3 second for KR link stable */
    if (run_packet == TRUE) {
        if (cpu0_xaui_bp_lp_test() == FAILED) {
            cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
            return (FAILED);
        }
    } else {
        if (tlk10232_set_ch_b_host_ge_loopback(host_test) != PASSED ){
            cterr('f', 0, "failed tlk10232_path_reset");
            return (FAILED);
        }
    }
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: tlk10232_optimize_ch_a
 *
 * Description: It enables the 1050mV p-p amplitude and -1.83dB de-emphasis
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tlk10232_optimize_ch_a (void)
{
    int phy_id, dev_id, reg_addr, val, mii_value;
    int port;
    /* TLK 10232 xgbe2 */
    port = 2;
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("tlk10232_optimize_ch_a\n");
    }
    /* Optimization the Ch-A LS. 0x1E.0x0007. val 0xfc44 */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = 0x0007;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    val = mii_value | 0xfc44;
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr,
                dev_id, phy_id);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("End tlk10232_optimize_ch_a\n");
    }
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: diag_tlk10232_lib.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.6  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.5  2015/05/05 11:53:11  steja
 * CDETS[CSCuu01237] Solving TLK intermittent loopback issue on GH platform.
 *
 * Revision 1.1.4.4  2015/04/30 08:33:52  steja
 * Clean up code
 *
 * Revision 1.1.4.3  2015/04/29 13:30:37  steja
 * Update TLK 10G-KR test path
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *-------------------------------------------------
 * Revision 1.1.2.16  2015/04/15 07:15:00  steja
 * Update TLK10232 10GKR configuration based on Woodlawn 10G-KR settings
 *
 * Revision 1.1.2.15  2015/03/27 06:46:59  steja
 * Optimization XAUI Channel A LS based on HW register
 *
 * Revision 1.1.2.14  2015/02/12 12:42:03  steja
 * Code clean up
 *
 * Revision 1.1.2.13  2015/01/13 08:31:14  steja
 * Print Reg address
 *
 * Revision 1.1.2.12  2014/11/12 09:30:43  steja
 * Update setup TLK loopback bit
 *
 * Revision 1.1.2.11  2014/11/11 07:59:00  steja
 * Fix the enable loopback function.
 *
 * Revision 1.1.2.10  2014/11/10 09:42:44  steja
 * Update TLK10232 10G KR loopback setup
 *
 * Revision 1.1.2.9  2014/09/26 09:05:53  steja
 * (CSCuq98591)Fix GBE4 link issue
 *
 * Revision 1.1.2.8  2014/09/24 01:44:14  steja
 * update cterr_db_print for register dump
 *
 * Revision 1.1.2.7  2014/09/18 07:18:43  steja
 * 1.Update NC command codei
 * 2.Update enhanced error message
 *
 * Revision 1.1.2.6  2014/09/15 07:58:59  steja
 * Code Clean up
 *
 * Revision 1.1.2.5  2014/08/26 06:32:40  steja
 * Add define for polarity flag
 *
 * Revision 1.1.2.4  2014/08/12 12:33:14  steja
 * Update 10GKR loopback test code
 *
 * Revision 1.1.2.3  2014/08/08 11:49:57  steja
 * Add 10G-KR loopback test
 *
 * Revision 1.1.2.2  2014/08/08 09:47:52  steja
 * Fix internal loopback test [CSCup56604]
 *
 * Revision 1.1.2.1  2014/07/21 01:56:52  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:46  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.13  2014/01/28 07:39:40  steja
 * Code clean up
 *
 * Revision 1.1.4.12  2014/01/06 13:00:33  steja
 * 1. clean up code
 * 2. Add header TLK code
 *
 * Revision 1.1.4.11  2013/12/18 05:03:11  steja
 * 1. support PSE2 backplane loopback test
 * 2. support BIB change MAC address utility
 *
 * Revision 1.1.4.10  2013/11/29 07:08:55  steja
 * 1. Fix the full data path TLK working.
 * 2. add USB test
 * 3. add read BIB MAC utility
 *
 * Revision 1.1.4.9  2013/11/19 14:47:56  steja
 * update tlk_init_config_2
 *
 * Revision 1.1.4.7  2013/11/05 09:17:54  steja
 * 1. Fix the MDIO not stable issue
 * 2. debug tlk log
 *
 * Revision 1.1.4.6  2013/10/10 00:36:22  steja
 * 1. Add TLK Utility PLL and Polarity TX RX switch
 * 2. Code update
 *
 * Revision 1.1.4.5  2013/10/05 06:20:24  steja
 * Update for debug
 *
 * Revision 1.1.4.4  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.3  2013/09/16 09:50:15  iachang
 * Code review and update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:07  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 */
