/* $Id: diag_backplane_xaui_test.c,v 1.2 2015/05/25 03:59:15 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/diag_backplane_xaui_test.c,v $ 
 *-----------------------------------------------------------------------------
 *
 * diag_backplane_xaui_test.c - Backplane XAUI Loopback Test
 *
 * May 2013, Steja
 * Copyright (c) 2013~2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
 
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "defs.h"
#include "diag_common_lib.h"
#include "skye_xaui.h"
#include "skye_smi_lib.h"
#include "common_utils.h"
#include "diag_tlk10232_lib.h"

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

#include "monitor.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "skye_eth.h"
#include "skye_xaui.h"
#include "nvmonvars.h"
#include "platform_fru.h"

int xaui_backplane_loopback_test(void);
int xaui_cpu0_chb_to_cha_loopback_test(void);
extern int tlk_init_config(int);
extern int cpu0_xaui_bp_lp_test(void);
extern int xaui_lpbk_test(int);
extern int tlk10232_setup_data_path_xaui_b_to_xaui_a(void);
extern int tlk10232_set_ch_a_host_ge_loopback(void);
extern int tlk10232_disable_high_speed_tx_rx(void);
extern int tlk10232_disable_stats_check_for_high_speed(void);
extern int tlk_init_config_2(void);
extern int tlk_init_config_3(boolean, boolean);
extern int tlk10232_kr_fifo_ctrl(void);
extern void msleep(unsigned long);
extern int tlk_init_config_10gkr(void);
extern int tlk10232_dump_all_reg(void);

/******************************************************************************
 *
 * Function: xaui_backplane_loopback_test
 *
 * Description: This function perform the backplane loopback test from Tilera to
 *                    backplane XAUI.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
xaui_backplane_loopback_test (void)
{
    char cmd[32];
    testname("XAUI full path loopback");
    prpass(testpass, "Backplane loopback Test");

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_BP_XAUI;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "TLK 10232 Ch-B LS ", "TLK 10232 Ch-A LS", "Backplane Switch");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)tlk10232_dump_all_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check TLK10232 Register dump on segment no.5",
                    "Check Backplane GE switch loopback setup properly",
                    "Check TLK10232 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    sprintf(cmd, "ifconfig xgbe2 up");
    system(cmd);
    sprintf(cmd, "ifconfig gbe4 down");
    system(cmd);
    msleep(100);

    /* Config TLK10232 to operate in XAUIB <-> XAUIA */
    prpass(testpass, "Config TLK10232 to operate in XAUIB <-> XAUIA");
    if (config_tlk_10232_mode(XAUIB_TO_XAUIA) == FAILED) {
        cterr('f', 0, "Config TLK10232 to operate in XAUIB <-> XAUIA failed");
        return (FAILED);
    }

    msleep(1000);
    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    sprintf(cmd, "ifconfig xgbe2 down");
    system(cmd);
    sprintf(cmd, "ifconfig gbe4 up");
    system(cmd);

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_cpu0_chb_to_cha_loopback_test
 *
 * Description: This function perform the debug CPU0 through XAUI Ch-B to
 *              Ch-A back to Ch-B and back to CPU0.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int 
xaui_cpu0_chb_to_cha_loopback_test (void)
{
    testname("CPU0 <--> TLK CH-B <--> TLK CH-A loopback");

    /* Init TLK10232 for XAUI Backplane Loopback */
    if(tlk_init_config_2() != PASSED) {
        printf("failed tlk_init_config");
        return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI B to XAUI A */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if(tlk10232_set_ch_a_host_ge_loopback() != PASSED) {
        cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
        return (FAILED);
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_cpu0_chb_to_cha_polarity_test
 *
 * Description: This function perform the debug CPU0 through XAUI Ch-B to
 *              Ch-A back to Ch-B and back to CPU0 with polarity.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int 
xaui_cpu0_chb_to_cha_polarity_test (void)
{
    testname("CPU0 <--> TLK CH-B <--> TLK CH-A with polarity");

    /* Init TLK10232 for XAUI Backplane Loopback */
    if(tlk_init_config_3(TRUE, FALSE) != PASSED) {
        printf("failed tlk_init_config");
        return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI B to XAUI A */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if(tlk10232_set_ch_a_host_ge_loopback() != PASSED) {
        cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
        return (FAILED);
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_cpu0_chb_to_cha_no_polarity_test
 *
 * Description: This function perform the debug CPU0 through XAUI Ch-B to
 *              Ch-A back to Ch-B and back to CPU0 no polarity
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int 
xaui_cpu0_chb_to_cha_no_polarity_test (void)
{
    testname("CPU0 <--> TLK CH-B <--> TLK CH-A no polarity");

    /* Init TLK10232 for XAUI Backplane Loopback */
    if(tlk_init_config_3(FALSE, FALSE) != PASSED) {
        printf("failed tlk_init_config");
        return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI B to XAUI A */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if(tlk10232_set_ch_a_host_ge_loopback() != PASSED) {
        cterr('f', 0, "failed tlk10232_set_ch_a_loopback");
        return (FAILED);
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_cpu0_chb_to_cha_to_bp_no_polarity_test
 *
 * Description: This function perform the debug CPU0 through XAUI Ch-B to
 *              Ch-A through to host backplane back to Ch-B and back to CPU0 no polarity.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
xaui_cpu0_chb_to_cha_to_bp_no_polarity_test (void)
{
    testname("CPU0 <--> TLK CH-B <--> TLK CH-A <-> Host BP no polarity");

    /* Init TLK10232 for XAUI Backplane Loopback */
    if(tlk_init_config_3(FALSE, FALSE) != PASSED) {
        printf("failed tlk_init_config");
        return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI B to XAUI A */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_cpu0_chb_to_cha_to_bp_no_polarity_test
 *
 * Description: This function perform the debug CPU0 through XAUI Ch-B to
 *              Ch-A through to host backplane back to Ch-B and back to CPU0 with polarity.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
xaui_cpu0_chb_to_cha_to_bp_polarity_test (void)
{
    testname("CPU0 <--> TLK CH-B <--> TLK CH-A <-> Host BP with polarity");

    /* Init TLK10232 for XAUI Backplane Loopback */
    if(tlk_init_config_3(TRUE, FALSE) != PASSED) {
        printf("failed tlk_init_config");
        return (FAILED);
    }

    if(tlk10232_kr_fifo_ctrl() != PASSED) {
        printf("failed tlk10232_kr_fifo_ctrl");
        return (FAILED);
    }

    /* Set up  data path. 0x4c20->0xac20 XAUI B to XAUI A */
    if (tlk10232_setup_data_path_xaui_b_to_xaui_a() != PASSED){
        printf("failed config_tlk_10232_mode");
        return (FAILED);
    }
    msleep(1000);
    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

#ifdef DBG_10GKR
/******************************************************************************
 *
 * Function: ten_g_kr_bp_loopback_test
 *
 * Description: This function perform the backplane loopback test from Tilera to
 *              backplane XAUI.
 *  This below is the config script from TI FAE
 *     tlk_write 1 0x1e 0 0x8610       // Reset TLK10232
 *     tlk_write 1 0x1 0x9001 0x0201   //  Sets link training mode to full region search of all pre/post values
 *     tlk_write 0 0x1e 0x0001 0x8b24 //  Power down Channel A
 *     tlk_write 0 0x1e 0x000d 0x3f80  // Disable ChA CLKOUT
 *     tlk_write 1 0x1e 0x000d 0x3f80  // Disable ChB CLKOUT
 *     tlk_write 0 0x01 0x0000 0x0800 // Disable ChA PMA
 *     tlk_write 0 0x03 0x0000 0x0800 // Disable ChA PCS
 *     tlk_write 1 0x7 0 0x2000                // Disable Auto Negotiation
 *     tlk_write 1 0x1 0x96 0                  // Disable Link Training
 *     tlk_write 1 0x1e 0x000e 0x0008 // Data path Reset
 *     tlk_write 1 0x1e 0x0002 0x811c // Set lowest PLL loop bandwidth
 *     tlk_write 1 0x1e 0x0003 0xe888 // Set higher swing, set AGC_CTRL to disable receiver attenuation
 *     tlk_write 1 0x1e 0x0004 0x5252 // EQPRE = 101, CDRFMULT = 00, CDRTHR = 10, PK_DISABLE = 1
 *
 *     tlk_write 1 0x1e 0x000e 0x0008 // Data path Reset
 *     tlk_write 1 0x1 0x9002 0    // Disable Link training timeout
 *     tlk_write 1 0x1 0x9003 0    // Disable Link training timeout
 *     tlk_write 1 0x1e 0x8101 0x4 // Enable loading of default TX values
 *     tlk_write 1 0x1e 0x8100 0x4 // Load default TX values
 *     tlk_write 1 0x1e 0x8100 0   // Load default TX values (bit needs to be written high then low)
 *     tlk_write 1 0x1e 0x9005 0x1c00  // Set link training PRBS packet count (1c00 is the default)
 *     tlk_write 1 0x1e 0x96 0x3   // Enable link training and restart
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
ten_g_kr_bp_loopback_test (void)
{
    boolean check1 = TRUE;

    testname("XAUI full path loopback");
    prpass(testpass, "10G-KR backplane loopback");

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_BP_GE0;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "TLK 10232 Ch-B LS ", "TLK 10232 Ch-B HS", "10G-KR Backplane Switch");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)tlk10232_dump_all_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check TLK10232 Register dump on segment no.5",
                    "Check Backplane GE switch loopback setup properly",
                    "Check TLK10232 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    if (diagflag_xram & D_SET_OPTIONS) {
    if (getc_answer("want to run packet?(y/n)", "yn",'n') == 'y')
        check1 = TRUE;
    else
        check1 = FALSE;
    }
    /* TLK First Init configuration */
    if (tlk_init_config(TLK10G) != PASSED) {
        cterr('f', 0, "tlk_init_config failed");
        return (FAILED);
    }
    /* TLK Second Init configuration */
    if (tlk_init_config_10gkr() != PASSED) {
        cterr('f', 0, "tlk_init_config_10gkr failed");
        return (FAILED);
    }

    if (check1 == TRUE) {
    msleep(2000);
    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}
#endif

/*-------------------------------------------------
 * $Log: diag_backplane_xaui_test.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.4  2015/05/20 09:43:14  steja
 * Fix TLK missing code after code review <CDETS CSCuu01237>
 *
 * Revision 1.1.4.3  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *-------------------------------------------------
 * Revision 1.1.2.9  2015/02/12 12:42:03  steja
 * Code clean up
 *
 * Revision 1.1.2.8  2014/09/18 07:18:43  steja
 * 1.Update NC command codei
 * 2.Update enhanced error message
 *
 * Revision 1.1.2.7  2014/09/17 11:13:15  palin2
 * Removed unused code.
 *
 * Revision 1.1.2.6  2014/09/15 07:58:58  steja
 * Code Clean up
 *
 * Revision 1.1.2.5  2014/08/31 23:01:13  palin2
 * Updated enhanced error message FRU table offset.
 *
 * Revision 1.1.2.4  2014/08/31 15:59:28  steja
 * Add enhanced error messages
 *
 * Revision 1.1.2.3  2014/08/12 12:33:14  steja
 * Update 10GKR loopback test code
 *
 * Revision 1.1.2.2  2014/08/08 11:49:57  steja
 * Add 10G-KR loopback test
 *
 * Revision 1.1.2.1  2014/07/21 01:56:52  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:44  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.8  2014/01/28 07:39:40  steja
 * Code clean up
 *
 * Revision 1.1.4.7  2013/12/18 05:03:11  steja
 * 1. support PSE2 backplane loopback test
 * 2. support BIB change MAC address utility
 *
 * Revision 1.1.4.6  2013/11/29 07:08:55  steja
 * 1. Fix the full data path TLK working.
 * 2. add USB test
 * 3. add read BIB MAC utility
 *
 * Revision 1.1.4.5  2013/11/19 14:36:46  steja
 * Provide TLK utility for debugging
 * Update the BTK TLK into coded
 *
 * Revision 1.1.4.4  2013/11/05 09:17:53  steja
 * 1. Fix the MDIO not stable issue
 * 2. debug tlk log
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

