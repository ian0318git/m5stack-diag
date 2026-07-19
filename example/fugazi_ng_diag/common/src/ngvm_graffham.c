/* $Id: ngvm_graffham.c,v 1.45 2021/01/07 06:23:26 jiajliu Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/ngvm_graffham.c,v $ 
 *******************************************************************************
 * ngvm_graffham.c : Graffham code
 *
 * Apr 2012,  Smita Rane
 *
 * Copyright (c) 2012-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <fcntl.h>
#include <net/if.h>
#include <string.h>

#include "common.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "assert.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "router_if.h"
#include "platform_i2c.h"
#include "cli_cmd.h"
#include "platform_cookie.h"
#include "ngio.h"
#include "cross_platform.h"
#include "pca.h"
#include "slot.h"
#include "cookie_4.h"
#include "plat_defs.h"
#ifndef TABEIL
#include "sgmii_defs.h"
#endif
#include "i2c_api.h"
#include "ngvm_graffham.h"
#include "common_utils.h"
#ifdef TABEIL
#include "dnv_eth_lib.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_fpga.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_pkt_txrx_utils.h"
#else
#include "platform_eth_pkt_txrx.h"
#endif
#include "linux_ntwk.h"
#include "linux_api.h"
#include "bcm_gesw_defs.h"
#include "dash_fpga.h"
#include "adapter_fpga.h"

//#define GRAFFHAM_USE_UART_MENU 1  /* NGVM menu executed on module itself */

static struct ngio_intf_t *ngio_ptr;
static ngvm_entity_t lsi_entity[MAX_NUM_NGVM];
static n2g_i2c_if_t pca_i2c[MAX_NUM_NGVM];
static char pca_buff[256];

int graff_dsp_test(int test, int param0, int param1, int wait_time);
static int graff_wait_result_packet(uint8_t *, int, uint16_t, int);
static int graffham_con_test(void);
static int graff_dsp_ddr3_sdram_test_wrapper(void);
static int graff_ge_intlpbk_test_wrapper(int);
static int graff_ge_lpbk_test_wrapper(int);
static int graffham_reset_en (void);
static int enable_bp_ge_lpbk(int);
static int disable_bp_ge_lpbk(int);
#ifdef TDM_INT_LPBK
static int graff_tdm_int_lpbk_test_wrapper(void);
#endif
static int graff_tdm_ext_lpbk_test_wrapper(void);
static int ecc_mem_test(void);
static int graff_arm11_cpu1_boot_test_wrapper(void);
static int graff_dss_core0_sanity_test_wrapper(void);
static int graff_dss_core1_sanity_test_wrapper(void);
static int graff_dss_core2_sanity_test_wrapper(void);
static int graff_dss_core3_sanity_test_wrapper(void);
static int graffham_unreset_en(void);
static int graff_disp_mac(void);
static int graff_disp_mem(void);
static int graff_disp_fw_ver(void);
static int gpio_resistor_test(void);
static int gpio_exp_read(void);
static int gpio_exp_write(void);
static int ppb_con(void);
static int graff_dac_1dot5vm_high(void);
static int graff_dac_1dot5vm_low(void);
static int graff_dac_no_1dot5vm(void);
static int graff_dac_dot93vm_high(void);
static int graff_dac_dot93vm_low(void);
static int graff_dac_no_dot93vm(void);
static int graff_select_test(int, int, int, int);
static int graffham_test(int);
static int ngvm_utils_test(void);
static int graff_intf_sync_test(void);
static int graff_uart_test(void);
static int gpio_exp_test(void);
//static int graffham_ready(void);
static int graffham_bringup_dsp(void);
static int graffham_ready_test(int dsp_no);
static int graff_setup_ge_env(void);
static void graff_clear_rx_buf(uchar *, int);
static void graff_build_command_packet(uint16_t, uint32_t, uint32_t, uint8_t);
static void graff_build_config_packet(void);
static void (*graffham_saved_diag_exec)(void) = NULL;
static int graff_send_command_packet(int dsp_no);
static int graff_cleanup_ge_env(void);
static int graff_eth_frames_test(uint32, uint32, int, mac_addr_t, int);
static int graff_wait_for_ge_packet(uchar *, int, int, int, int);

/* Platform motherboard HW revision
 * Added to deal with differences between platform revisions
 */
static unsigned int plat_bd_rev = 999;

/* Variable to select to run tests on host using ethernet interface or run 
 * tests on the DSP using uart interface */
int dsp_tests_use_enet = 1;

submenu_xtable_t graff_tests_submenu_table[] = {
    {"NGVM Utilities",              (PFT)ngvm_utils_test,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"NGVM Interface SYNC Signal test",
                                    (PFT)graff_intf_sync_test,            0,MM_2,
                         (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"DDR3 Memory test", (PFT)graff_dsp_ddr3_sdram_test_wrapper,         0,MM_2, 
                         (type_t(*)())0, 0, (type_t(*)())0, 0},
#ifdef UART_MODE
    {"EMAC0 Internal loopback test", (PFT)graff_ge_intlpbk_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"EMAC1 Internal loopback test", (PFT)graff_ge_intlpbk_test_wrapper, 1,MM_2, 
                         (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
    {"EMAC0 loopback test", (PFT)graff_ge_lpbk_test_wrapper,    0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"EMAC1 loopback test", (PFT)graff_ge_lpbk_test_wrapper,    1,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ARM11 CPU1 Boot test ", (PFT)graff_arm11_cpu1_boot_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core0 Sanity test ", (PFT)graff_dss_core0_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core1 Sanity test ", (PFT)graff_dss_core1_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core2 Sanity test ", (PFT)graff_dss_core2_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core3 Sanity test ", (PFT)graff_dss_core3_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
#ifdef TDM_INT_LPBK
    {"TDM Internal Loopback test ",(PFT)graff_tdm_int_lpbk_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
#endif
    {"TDM External Loopback test ", (PFT)graff_tdm_ext_lpbk_test_wrapper,0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ECC Memory test ",            (PFT)ecc_mem_test,                   0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Uart Loopack test",           (PFT)graff_uart_test,                0,MM_2,
                         (type_t(*)())0, 0,     (type_t(*)())0},
};

#define GRAFF_TESTS_SUBMENU_TABLE_SIZE (sizeof(graff_tests_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t graff_tests_primary_items[GRAFF_TESTS_SUBMENU_TABLE_SIZE + 10];
#ifndef GRAFFHAM_USE_UART_MENU
static mitem_t graff_tests_secondary_items[GRAFF_TESTS_SUBMENU_TABLE_SIZE + 10];
#endif

menuinfo_t graff_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    graff_tests_primary_items,
};
menuinfo_t *graff_submenup = &graff_subtest_menu;

submenu_xtable_t ngvm_utils_submenu_table[] = {
    {"Graffham Console",            (PFT)ppb_con,            0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"NGVM reset",                  (PFT)graffham_reset_en,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"NGVM unreset",                (PFT)graffham_unreset_en,0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Picocom Console ",            (PFT)graffham_con_test,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Graffham MAC",        (PFT)graff_disp_mac,     0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"Display Graffham Firmware Version",     
                                    (PFT)graff_disp_fw_ver,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Graffham Memory",     (PFT)graff_disp_mem,     0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"PCA9557 Register Read",       (PFT)gpio_exp_read,      0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"PCA9557 Register Write",      (PFT)gpio_exp_write,     0,0,(type_t(*)())0,                                      0, (type_t(*)())0},
    {"Enable GE0 Loopback",         (PFT)enable_bp_ge_lpbk,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Disable GE0 Loopback",        (PFT)disable_bp_ge_lpbk, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Enable GE1 Loopback",         (PFT)enable_bp_ge_lpbk,  1,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Disable GE1 Loopback",        (PFT)disable_bp_ge_lpbk, 1,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"1.5 Voltage Margin High", (PFT)graff_dac_1dot5vm_high, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"1.5 Voltage Margin Low",  (PFT)graff_dac_1dot5vm_low,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"1.5 No Voltage Margin",  (PFT)graff_dac_no_1dot5vm,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {".93 Voltage Margin High", (PFT)graff_dac_dot93vm_high, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {".93 Voltage Margin Low",  (PFT)graff_dac_dot93vm_low,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {".93 No Voltage Margin",  (PFT)graff_dac_no_dot93vm,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
};

#define NGVM_UTILS_SUBMENU_TABLE_SZ (sizeof(ngvm_utils_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ngvm_utils_primary_items[NGVM_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t ngvm_utils_secondary_items[NGVM_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
char graffutiltitle[50];

menuinfo_t ngvm_util_submenu = {
    //"NGVM Utilites Menu",
    graffutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ngvm_utils_primary_items,
};

menuinfo_t *ngvm_util_submenup = &ngvm_util_submenu;

static submenu_xtable_t graffham_mainmenu_tbl[] = {
    {"NGVM Utilities",              (PFT)ngvm_utils_test,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"GPIO Resistor test",          (PFT)gpio_resistor_test, 0,MM_2,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"GPIO Expander test",          (PFT)gpio_exp_test,      0,MM_2,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Graffham test",               (PFT)graffham_test,   TRUE,MM_2,(type_t(*)())0,
                                    0, (PFT)graffham_test, FALSE},
};

#define GRAFFHAM_MAINMENU_TBL_SIZE \
        (sizeof(graffham_mainmenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
5B
 */
static mitem_t main_menu_primary_items[GRAFFHAM_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[GRAFFHAM_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];

static title_buf_t  maindiag_header;

char graffmainmenutitle[50];
char graffsubmenutitle[50];
static struct menuinfo graffham_mainmenu = {
    /*"Graffham VM Main Menu",      *//* title */
    graffmainmenutitle,          /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};

static struct menuinfo *maindiagp = &graffham_mainmenu;

static test_commands_t graffham_command[] = {
    {SELECT_DSP_SANITY,        "SELECT_DSP_SANITY"},  /* 01 DSP sanity */
    {SELECT_DSP_SDRAM,         "SELECT_DSP_SDRAM"},    /* 02 SDRAM test */
    {SELECT_DSS0_SANITY,       "SELECT_DSS0_SANITY"}, /* 03 DSS Integrity */
    {SELECT_DSS1_SANITY,       "SELECT_DSS1_SANITY"}, /* 04 */
    {SELECT_DSS2_SANITY,       "SELECT_DSS2_SANITY"}, /* 05 */
    {SELECT_DSS3_SANITY,       "SELECT_DSS3_SANITY"}, /* 06 */
    {SELECT_UART_TEST,         "SELECT_UART_TEST"},   /* 07 Test uart intf */
    {SELECT_DAC_1DOT5VM_HIGH,  "SELECT_DAC_1DOT5VM_HIGH"}, /* 0x0008 */
    {SELECT_DAC_1DOT5VM_LOW,   "SELECT_DAC_1DOT5VM_LOW"},  /* 0x0009 */
    {SELECT_DAC_NO_1DOT5VM,    "SELECT_DAC_NO_1DOT5VM"},   /* 0x000A */
    {SELECT_ECC_MEM,           "SELECT_ECC_MEM"},          /* 0x000B */
    {SELECT_DSP_CONSOLE,       "SELECT_DSP_CONSOLE"},      /* 0x000C */
    {SELECT_UART_LPBK,         "SELECT_UART_LPBK"},        /* 0x000D */
    {SELECT_UART_LPBK_RESULT,  "SELECT_UART_LPBK_RESULT"}, /* 0x0011 */
    {SELECT_UART_LPBK_STOP,    "SELECT_UART_LPBK_STOP"},   /* 0x0012 */
    {SELECT_UART_LPBK_RX,      "SELECT_UART_LPBK_RX"},     /* 0x0013 */
    {SELECT_DAC_DOT93VM_HIGH,  "SELECT_DAC_DOT93VM_HIGH"}, /* 0x0014 */
    {SELECT_DAC_DOT93VM_LOW,   "SELECT_DAC_DOT93VM_LOW"},  /* 0x0015 */
    {SELECT_DAC_NO_DOT93VM,    "SELECT_DAC_NO_DOT93VM"},   /* 0x0016 */
    {SELECT_ARM11CPU1_BOOT,    "SELECT_ARM11CPU1_BOOT"},   /* 0x0017 */
    {SELECT_GE0_LPBK,          "SELECT_GE0_LPBK"}, /* 0x20 Host-DSP Ge0 lpbk */
    {SELECT_GE1_LPBK,          "SELECT_GE1_LPBK"}, /* 0x21 Host-DSP GE1 lpbk */
    {SELECT_MEM_DISP,          "SELECT_MEM_DISP"}, /* 0x22 */
    {SELECT_FW_VER_DISP,       "SELECT_FW_VER_DISP"}, /* 0x23 */
    {SELECT_GE_LPBK_PF,        "SELECT_GE_LPBK_PF"},  /* 0x40 */
    {SELECT_GE0_LPBK_PT,       "SELECT_GE0_LPBK_PT"}, /* 0x80 pass through */
    {SELECT_GE1_LPBK_PT,       "SELECT_GE1_LPBK_PT"}, /* 0x81 pass through */
    {SELECT_INTF_SYNC,         "SELECT_INTF_SYNC"},   /* 0x82 SYNC signals */
    {SELECT_TDM_INTLPBK,       "SELECT_TDM_INTLPBK"}, /* 0x100 TDM int lpbk */
    {SELECT_TDM_EXTLPBK,       "SELECT_TDM_EXTLPBK"}, /* 0x200 TDM ext lpbk */
    {SELECT_READY,             "SELECT_READY"},       /* 0x300 DSP is ready */
    {SELECT_NULL,              "UNKNOWN_TEST"},       /* */
};


#if 0
/********************************************************
 *
 * Function: graff_submenu_prcomplete
 *
 * Description: For submenu, print out test complete message.
 *
 * Input:    iface - Pointer to interface data structure.
 *  
 * Outputs:  None.
 *  
 * Assumptions:
 *  
 ********************************************************
 */
static void graff_submenu_prcomplete (void)
{   
    assert(ngio_ptr);

printf("\n in prcomplete\n");
    if (ngio_ptr->menu_display) {
        prcomplete(testpass, errcount, (char *)0);
    }
}
#endif

/**********************************************************************
 *
 * Function: lsi_sp27xx_vm_reset
 *
 * This function reset and unrerset the LSI SP27XX VM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
int lsi_sp27xx_vm_reset (void)
{
    ngvm_entity_t *graffham_ep;
    int ret, dsp;

    prpass(testpass, " LSI SP27XX VM reset ");

    assert(ngio_ptr);

    /* Pull reset then release the reset*/ 
    ret = ngio_ptr->reset(ngio_ptr);
    msleep(50);

    graffham_ep = (ngvm_entity_t *)ngio_ptr->priv;

    for (dsp=0; dsp<MAX_DSPS_PER_NGVM; dsp++)
        graffham_ep->dsp_downloaded[dsp] = FALSE;

    if ((ngio_ptr->on(ngio_ptr)) < 0) {
        cterr('f', 0, "%s(): Unable to power on module", __FUNCTION__);
        return FAILED;
    }
    ret |= ngio_ptr->unreset(ngio_ptr);

    msleep(50);
    
    return (ret);
}

/**********************************************************************
 *
 * Function: graffham_reset_en
 *
 * This function reset the LSI SP27XX VM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
static int graffham_reset_en (void)
{
    ngvm_entity_t *graffham_ep;
    int ret, dsp;

    assert(ngio_ptr);

    //printf("\nGraffham VM Enabling Reset\n ");
    /* Pull reset enable */
    ret = ngio_ptr->reset(ngio_ptr);
    msleep(50);

    graffham_ep = (ngvm_entity_t *)ngio_ptr->priv;

    for (dsp=0; dsp<MAX_DSPS_PER_NGVM; dsp++)
        graffham_ep->dsp_downloaded[dsp] = FALSE;
    return (ret);

}

/**********************************************************************
 *
 * Function: graffham_unreset_en
 *
 * This function unresets the LSI SP27XX VM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
static int graffham_unreset_en (void)
{
    int ret;

    assert(ngio_ptr);

    //printf("\nGraffham VM release reset");
    /* release the reset*/
    ret = ngio_ptr->unreset(ngio_ptr);
    msleep(50);
    return (ret);

}

/**********************************************************************
 *
 * Function: graff_disp_mac
 *  
 *  
 * Description: Display Graffham and Host MAC
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int graff_disp_mac (void)
{
    ngvm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Display the NGVM module details */
    printf("\n%s", ep->name);
    printf("\nPID: %s", ep->pid);
    if (ep->ngvm_id == PFUSE123_SP2702)
        printf("\nGraffham SP2702\n");
    else if (ep->ngvm_id == PFUSE123_SP2704)
        printf("\nGraffham SP2704\n");
    else {
        printf("\nUnknown LSI SP27XX (Make sure firmware is downloaded "
               "first)\n");
    }
    if (ep->init_ngvm()) {
        return (FAILED);
    }
    printf("\n Graffham module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
               ep->eth_hdr[0].dest_addr[0], ep->eth_hdr[0].dest_addr[1],
               ep->eth_hdr[0].dest_addr[2], ep->eth_hdr[0].dest_addr[3], 
               ep->eth_hdr[0].dest_addr[4], ep->eth_hdr[0].dest_addr[5]);

    printf("\n Host  MAC           : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
               ep->eth_hdr[0].src_addr[0], ep->eth_hdr[0].src_addr[1],
               ep->eth_hdr[0].src_addr[2], ep->eth_hdr[0].src_addr[3],
               ep->eth_hdr[0].src_addr[4], ep->eth_hdr[0].src_addr[5]);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: graff_disp_fw_ver
 *  
 *  
 * Description: Display Firmware version 
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int graff_disp_fw_ver (void)
{
    ngvm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int ret_val;
    uint8_t bufmsg[128];

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Bringup DSP */
    if (graffham_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (graff_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = graff_dsp_test(SELECT_FW_VER_DISP, 0, 0, 100);
    if (ret_val == PASSED) {
        recv_packet_p = &(ep->recv_packet);
        result_packet_p = &(ep->result_packet);

        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ether_t));

        memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));
        printf("\n %s\n", bufmsg);
    }
    return (ret_val);
}

/**********************************************************************
 *
 * Function: graff_disp_mem
 *  
 * Description: Display SP2704 memory
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int graff_disp_mem (void)
{
    ngvm_entity_t *ep;
    dspif_mem_t   *mem_p;
    int ret_val, offset, len;
 
    /* Bringup DSP */
    if (graffham_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    offset = gethex_answer("Memory Start Address: ", 0, 0, 0xffffFD7C);
    len = gethex_answer("Length : ", 0, 0, 644); /* 0x284 */

    if (graff_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = graff_dsp_test(SELECT_MEM_DISP, offset, len, 100);
    if (ret_val == PASSED) {
        assert(ngio_ptr);

        ep = (ngvm_entity_t *)ngio_ptr->priv;
        assert(ep);

        mem_p = (dspif_mem_t *)&(ep->recv_packet.data);
        printf("\n DSP address = 0x%x, size = %d\n", mem_p->dspif_info.param1, 
               mem_p->dspif_info.param2);
        dismem((uchar *)(mem_p->pkt_data), mem_p->dspif_info.param2, 
               (ulong)(mem_p->pkt_data), 4);
    }
    return (ret_val);
}

/**********************************************************************
 *
 * Function: gpio_exp_read
 *  
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int gpio_exp_read (void)
{
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset, i2c_dev;
    
    assert(ngio_ptr);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    assert(pca);

    i2c_dev = pca->i2c_dev;
    if (ngio_ptr->mod_type != VM_MODULE) {
        pca->i2c_dev = NGDC_I2C_ADDR_IO_PORT;
    }

    //offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    for (offset = 0; offset <= 0x3; offset++) {
        if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
            pca->i2c_dev = i2c_dev ;
            cterr('f', 0, "%s(): Unable to read PCA9557 register @ %#x\n", 
                  offset, __FUNCTION__);
            return (FAILED);
        }
        printf("\nRegister @ %#x = %#x\n", offset, data);
    }
    pca->i2c_dev = i2c_dev ;
    return (PASSED);
}

/**********************************************************************
 *
 * Function: gpio_exp_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int gpio_exp_write (void)
{   
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset, i2c_dev;

    assert(ngio_ptr);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    assert(pca);

    i2c_dev = pca->i2c_dev;
    if (ngio_ptr->mod_type != VM_MODULE) {
        pca->i2c_dev = NGDC_I2C_ADDR_IO_PORT;
    }

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 register @ %#x\n", offset,
              __FUNCTION__);
        return (FAILED);
    }
    pca->i2c_dev = i2c_dev;
    return (PASSED);
}

/*
 **********************************************************************
 *  
 *  Function: enable_bp_ge_lpbk
 *  
 *  Description: Enable GE switch loopback.
 *
 *  Input: None
 *
 *  Returns: PASSED 
 *
 **********************************************************************
 */
static int enable_bp_ge_lpbk (int port)
{
    ngvm_entity_t *ngvm_ep;
    int ge_port;

    assert(ngio_ptr);

    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    ge_port = ovld_get_ge_sw_port_num(ngvm_ep->pslot, 
                                      ngvm_ep->ge_tgt_dev, port);
    printf("\n NGVM eth port = %d, parent slot = %d, tgt_dev = %d \n", ge_port,
           ngvm_ep->pslot, ngvm_ep->ge_tgt_dev);
 
    set_gesw_line_loopback(ge_port, 1);

    return (PASSED);
}

/*
 **********************************************************************
 *  
 *  Function: disable_bp_ge_lpbk
 *  
 *  Description: This function disables the GE switch loopback mode
 *
 *  Input: None
 *
 *  Returns: PASSED 
 *
 **********************************************************************
 */
static int disable_bp_ge_lpbk (int port)
{
    ngvm_entity_t *ngvm_ep;
    int ge_port;

    assert(ngio_ptr);

    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    ge_port = ovld_get_ge_sw_port_num(ngvm_ep->pslot, 
                                      ngvm_ep->ge_tgt_dev, port);
    set_gesw_line_loopback(ge_port, 0);

    return (PASSED);
}

/*-----------------------------------------------------------------------
 *
 * Function: get_ngio_mac_addr
 *
 * This functions read MAC addr from Module cookie on Switzer Carrier.
 *
 * Input : None
 *
 * Output: MAC address.
 *
 *------------------------------------------------------------------------
 */
static int carrier_get_ngio_mac(unsigned char *mac_addr)
{
    uchar i, num_byte, *data_ptr;

    assert(ngio_ptr);

    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
         (ngio_ptr->cookie, (uchar) BOARD_MAC_ADDR,
          &num_byte, FALSE)) == (uchar *) NULL) {
        /*Search BOARD_MAC_ADDR failed. */
        for (i = 0; i < 6; i++) {         /* illegal code */
            mac_addr[i] = 0xff;
        }
        return FAILED;
    } else {
        for (i = 0; i < num_byte; i++) {
            mac_addr[i] = *data_ptr;
            data_ptr++;
        }
        return PASSED;
    }
}

/*
 **********************************************************************
 *  
 *  Function: lsi_sp27xx_init_ngvm
 *  
 *  Description: This function initialize internal stuctures/data
 *
 *  Input: None
 *
 *  Returns: PASSED 
 *
 **********************************************************************
 */
int lsi_sp27xx_init_ngvm (void)
{
    ngvm_entity_t *ep;
    int i, j, no_dsp;
    uchar print_mac[6];

    assert(ngio_ptr);
    
    /* Need to store the MAC address of the NGVM and host */ 
    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    ep->num_dsp = NUM_DSP_GRAFFHAM;
    no_dsp = ep->num_dsp;

    for (i=0; i<no_dsp; i++) {

        if (ngio_ptr->pc && ngio_ptr->pc->mod_type == DAUGHTER_CARD) {
            carrier_get_ngio_mac(&print_mac[0]);
        } else {
            get_ngio_mac_addr(ep->pslot, ngio_ptr->mod_type,  &print_mac[0]);
        }

        for (j=0; j<6; j++) {
            ep->eth_hdr[i].dest_addr[j] = (uint8_t) print_mac[j];
        }
        //prpass(testpass, " Graffham module MAC : ");
        printf("\r Graffham module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
               ep->eth_hdr[i].dest_addr[0], ep->eth_hdr[i].dest_addr[1],  
               ep->eth_hdr[i].dest_addr[2], ep->eth_hdr[i].dest_addr[3],  
               ep->eth_hdr[i].dest_addr[4], ep->eth_hdr[i].dest_addr[5]);

        get_host_mac_addr(0, &print_mac[0]);

        for (j=0; j<6; j++) {
            ep->eth_hdr[i].src_addr[j] = (uint8_t) print_mac[j]; 
        }
        //prpass(testpass, " Host  MAC           : ");
        printf("\r Host  MAC           : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
               ep->eth_hdr[i].src_addr[0], ep->eth_hdr[i].src_addr[1],  
               ep->eth_hdr[i].src_addr[2], ep->eth_hdr[i].src_addr[3],  
               ep->eth_hdr[i].src_addr[4], ep->eth_hdr[i].src_addr[5]);
    }
    printf("\r                                                              "
           "                             \r");
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: lsi_sp27xx_get_pid
 *
 *  Description: This function returns the PID string
 *
 *  Input: char *
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
static int lsi_sp27xx_get_pid (char *pid)
{
    uchar i, num_byte, *data_ptr;

    assert(ngio_ptr);

    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
        (ngio_ptr->cookie, (uchar) PRODUCT_ID,
        &num_byte, FALSE)) == (uchar *) NULL) {
            /*Search CONTROLLER_TYPE failed. */
            pid[0] = 0;                /* illegal code */
            return(FAILED);
        } else {
            for (i = 0; i < num_byte; i++) {
            pid[i] = *data_ptr++;
        }

    }
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: init_ngio_priv_struct
 *
 *  Description: This function initializes the ngvm specific structures.
 *
 *  Input: None
 *
 *  Returns: FAILED - If slot type not WIC_DC or SM_DC or VM_MODULE
 *           PASSED - otherwise
 *
 **********************************************************************
 */
static int init_ngio_priv_struct (void)
{
    ngvm_entity_t *graffham_ep;
    int dsp, ngvm_num, tgt_dev, i2c_dev, pslot;
    char slot_type_str[20], tty_dev[20];

    assert(ngio_ptr);

    switch (ngio_ptr->mod_type) {
        case SM_DAUGHTER_CARD:  
            assert(ngio_ptr->pc);
            ngvm_num = ngio_ptr->pc->slot;
            if (ngio_ptr->id == GRAFFHAM_VM) 
                sprintf(slot_type_str, "NGSM%d Graffham", ngvm_num);
            else
                sprintf(slot_type_str, "NGSM%d Graffham TestCard", ngvm_num);
            ngvm_num = ngvm_num + MAX_VM + MAX_WIC - FIRST_SLOT;
            tgt_dev = TGT_DEV_NGSM;
            i2c_dev = SM_I2C_ADDR_IO_PORT;
            sprintf(tty_dev, "NOTKNOWN");
            assert(ngio_ptr->pc);
            pslot = ngio_ptr->pc->slot;
            break;
        case WIC_DAUGHTER_CARD: 
            assert(ngio_ptr->pc);
            ngvm_num = ngio_ptr->pc->slot;
            if (ngio_ptr->id == GRAFFHAM_VM) 
                sprintf(slot_type_str, "NGWIC%d Graffham", ngvm_num);
            else
                sprintf(slot_type_str, "NGWIC%d Graffham TestCard", ngvm_num);
            ngvm_num = ngvm_num + MAX_VM - FIRST_SLOT;
            ngio_ptr->priv = (void *) &lsi_entity[ngvm_num];
            tgt_dev = TGT_DEV_NGWIC;
            assert(ngio_ptr->pc);
            if (ngio_ptr->pc->slot == FIRST_SLOT)
                sprintf(tty_dev, "/dev/ttyDASH2");
            else if (ngio_ptr->pc->slot == FIRST_SLOT+1)
                sprintf(tty_dev, "/dev/ttyDASH3");
            else
                sprintf(tty_dev, "/dev/ttyDASH4");
            i2c_dev = NGWIC_I2C_ADDR_IO_PORT;
            assert(ngio_ptr->pc);
            pslot = ngio_ptr->pc->slot;
            break;
        case VM_MODULE: 
            ngvm_num = ngio_ptr->slot;
            if (ngio_ptr->id == GRAFFHAM_VM) 
                sprintf(slot_type_str, "Onboard Graffham");
            else
                sprintf(slot_type_str, "Onboard Graffham TestCard");
            ngvm_num = ngvm_num - FIRST_SLOT;
            ngio_ptr->priv = (void *) &lsi_entity[ngvm_num];
            tgt_dev = TGT_DEV_NGVM;
            sprintf(tty_dev, "/dev/ttyDASH5");
            i2c_dev = NGDC_I2C_ADDR_IO_PORT;
            pslot = ngio_ptr->slot;  /* does not matter */
            break;
        default:
            ngvm_num = INVALID_NGVM_NUM;
            printf("\n %s(): INVALID NGVM slot type %d for NGVM slot %d, parent"
                   " slot %d", __FUNCTION__, ngio_ptr->mod_type, ngio_ptr->slot, 
                   ngio_ptr->pc->slot);
            return (FAILED);
    }
    testname(slot_type_str);
    /* point to Graffham structure */
    ngio_ptr->priv = (void *) &lsi_entity[ngvm_num];

    /* Init parameters for I2C access to the GPIO expander */
    pca_init_i2c((void *)&pca_i2c[ngvm_num]);
    pca_i2c[ngvm_num].i2c_ctrl = ngio_ptr->i2c_ctrl;
    pca_i2c[ngvm_num].i2c_dev = i2c_dev;
    pca_i2c[ngvm_num].buf = &pca_buff[ngvm_num];
    ngio_ptr->pca = (void *) &pca_i2c[ngvm_num];

    if (ngio_ptr->pc && ngio_ptr->pc->mod_type == DAUGHTER_CARD) {
        memcpy(&pca_i2c[ngvm_num], ngio_ptr->pc->pca, sizeof(n2g_i2c_if_t));
        pca_i2c[ngvm_num].i2c_dev = i2c_dev;
    }

    /* Init Graffham struct parameters */
    graffham_ep = (ngvm_entity_t *) ngio_ptr->priv; 
    /* Will populate id after firmware download */
    graffham_ep->cookie_id = ngio_ptr->id; 
    graffham_ep->ngvm_id = PFUSE123_UNKNOWN; 
    graffham_ep->plat_ngvm_num = ngvm_num; 
    graffham_ep->ge_tgt_dev = tgt_dev; 
    graffham_ep->pslot = pslot; 
    sprintf(graffham_ep->tty_dev, "%s", tty_dev);
    sprintf(graffham_ep->slot_type_str, "%s", slot_type_str);

    if (graffham_ep->cookie_id == GRAFFHAM_TESTCARD) { 
        sprintf(graffham_ep->name, "GRAFFHAM TestCard LSI SP27XX, board_id %#x,"
                " %s slot %d (ngvm #%d)", ngio_ptr->id, slot_type_str, 
                ngio_ptr->slot, ngvm_num);
    } else {
        sprintf(graffham_ep->name, "GRAFFHAM VM LSI SP27XX, board_id %#x, "
                "%s slot %d (ngvm #%d)", ngio_ptr->id, slot_type_str, 
                ngio_ptr->slot, ngvm_num);
    }

    for (dsp=0; dsp<MAX_DSPS_PER_NGVM; dsp++)
        graffham_ep->dsp_downloaded[dsp] = FALSE;
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: graffham_init_fp
 *
 *  Description: This function initialize callout functions
 *
 *  Input: None
 *
 *  Returns: None 
 *
 **********************************************************************
 */
static void graffham_init_fp (void)
{
    ngvm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    ep->reset_ngvm = lsi_sp27xx_vm_reset;
    ep->init_ngvm = lsi_sp27xx_init_ngvm;
    ep->get_pid = lsi_sp27xx_get_pid;

}

/**********************************************************************
 * Function: graffham_vm_cleanup
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void graffham_vm_cleanup (void)
{
    assert(ngio_ptr);

    // SR please add back disable_bp_ge_lpbk(0);
    // SR please add back disable_bp_ge_lpbk(1);

    // SR please add back graffham_reset_en();

    if (graffham_saved_diag_exec) {
        pre_diag_exec = graffham_saved_diag_exec;
        graffham_saved_diag_exec = NULL;
    }
}

#if 0
static int graffham_ready (void)
{
    ngvm_entity_t *ngvm_ep;
    assert(ngio_ptr);

    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);
    if (ngvm_ep->dsp_downloaded[0] == TRUE) {
        //printf(" DSP firmware is downloaded and DSP is booted up");
        return (1);
    }
    return (0);
}
#endif

/**********************************************************************
 * Function: get_uart_port
 *
 * Description: This function will get the corresponding port index
 *              NGSM  1: 0
 *              NGSM  2: 1
 *              NGWIC 1: 2
 *              NGWIC 2: 3
 *              NGWIC 3: 4
 *              NGVM   : 5
 *
 * Input:  None
 *
 * Output: If valid port it return port number. Otherwise return -1.
 **********************************************************************
*/
 static int get_uart_port(void)
{
    int port = -1;
    assert(ngio_ptr);

    switch (ngio_ptr->mod_type) {
        case SM_DAUGHTER_CARD:  /* pasthru */
        case WIC_DAUGHTER_CARD:  
            assert(ngio_ptr->pc);
            port = ngio_ptr->pc->uart_ctrl; 
            break;
        case VM_MODULE: 
            port = 5;
            break;
        default:
            printf("\n %s() failed: Unknown slot type (%d)\n",
                    __FUNCTION__, ngio_ptr->mod_type);
            return (-1);
    }

    return port;
}

/**********************************************************************
 * Function: adapter_graff_uart_test
 *
 * Description: This function performs the uart interface test for the
                NGVM on Module on Carrier Adapter
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int adapter_graff_uart_test(void)
{
    int ret = PASSED;;
    int port;
    char *tx_str = "abcdefgh";
    int tx_len = strlen(tx_str);
    int rx_sz;
    char rx_str[64];
    struct adapter_uart_t *adap;

    assert(ngio_ptr);

    adap = get_current_adapter_uart();
    port = ngio_ptr->pc->uart_ctrl;

    memset(rx_str, 0, sizeof(rx_str));
    rx_sz = 0;
    ret = adap->adapter_uart_lpbk_txrx(port, tx_str, tx_len, rx_str, &rx_sz, 9600, 0);
    if (ret == FAILED) {
        //Do not reset if test fails graffham_reset_en();
        cterr('f', 0, "%s(): uart_lpbk: failed to tx and rx.\
                Please reset/unreset NGVM. ", __FUNCTION__);
    } else {
        if (!strstr(rx_str, tx_str)) {
            ret = FAILED;
            cterr('f', 0, "rx/tx string differ [rx = %s] [expect str = %s] [tx = %s].",
                  rx_str, tx_str, tx_str);
        }
    }

    return ret;
}

/**********************************************************************
 * Function: uart_lpbk
 *
 * Description: This function performs the uart interface test for the 
                NGVM
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int graff_uart_test (void)
{
    ngvm_entity_t *ngvm_ep;
    int ret = PASSED;
    int port;
    char *tx_str = "abcdefgh";
    int tx_len = strlen(tx_str);
    int rx_sz;
    char rx_str[64];

#ifdef GRAFFHAM_USE_UART_MENU
    printf("\n uart is implicity tested with the menu running on NGVM "
           "module");
    return (PASSED);
#endif

    prpass(testpass, "Uart Loopback - ");
    assert(ngio_ptr);

    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    if (graffham_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }
    /* Run uart loopback test when the DSP diag application is booted */
    graff_select_test(SELECT_UART_TEST, 0, 0, 0);
    fflush(stdout);
    fflush(stderr);
    msleep(3000);
    fflush(stdout);
    fflush(stderr);
    msleep(3000);
    assert(ngio_ptr);
    assert(ngio_ptr->priv);
    ngvm_ep = (ngvm_entity_t *) ngio_ptr->priv;

    if (ngio_ptr->pc && ngio_ptr->pc->mod_type == DAUGHTER_CARD) {
        ret = adapter_graff_uart_test();
    } else {
    port = get_uart_port();
    memset(rx_str, 0, sizeof(rx_str));
    rx_sz = 0;
    if ((ret = uart_lpbk_txrx(port, tx_str, tx_len, rx_str, &rx_sz,
                              9600, 0))==FAILED) {
        //Do not reset if test fails graffham_reset_en();
        cterr('f', 0, "%s(): uart_lpbk: failed to tx and rx.\
                Please reset/unreset NGVM. ", __FUNCTION__);
    } else {
        if (!strstr(rx_str, tx_str)) {
            ret = FAILED;
            cterr('f', 0, "rx/tx string differ [rx = %s] [expect str = %s] [tx = %s].",
                  rx_str, tx_str, tx_str);
        }
    }
    }
    graffham_reset_en();
    return (ret);

}

/**********************************************************************
 * Function: gpio_resistor_test
 *
 * Description: This function test the resistor circuitry for the GPIO 
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int gpio_resistor_test (void)
{
    n2g_i2c_if_t  *pca;        
    int i2c_dev, ret;
    uchar data, config, inv, output;

    assert(ngio_ptr);
    assert(ngio_ptr->priv);

    ret = PASSED;
    prpass(testpass, "GPIO Resistor - ");

    /* This test should be conducted with the NGVM in reset to test
       the resistor stuffing used for pull up/down */
    graffham_reset_en();

    /* Get the correct I2C address to use */
    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    i2c_dev = pca->i2c_dev; 
    if (ngio_ptr->mod_type != VM_MODULE) {
        pca->i2c_dev = NGDC_I2C_ADDR_IO_PORT;
    }

    /* Please set the GPIO register to default first */
    if (io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &config, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 CONFIGURATION_REG, \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, POLARITY_INV_REG, &inv, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 POLARITY_INV_REG, \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, OUTPUT_PORT_REG, &output, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 OUTPUT_PORT_REG, \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    data = 0xFF;
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n", 
              __FUNCTION__);
        ret = (FAILED);
    }
    data = 0xF0;
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Polarity Inversion "
              "register \n", __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 INPUT_PORT_REG \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    if (data != 0xF3) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): GPIO Resistor test failed, expected 0xF3 read"
              " 0x%x \n", __FUNCTION__, data);
        ret = (FAILED);
    }
    /* Restore save values */
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &config) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n", 
              __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &inv) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Polarity Inversion "
              "register \n", __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT_REG, &output) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 OUTPUT_PORT_REG,"
              "register \n", __FUNCTION__);
        ret = (FAILED);
    }
    pca->i2c_dev = i2c_dev;
    //graff_submenu_prcomplete();
    return (ret);
     
}

/**********************************************************************
 * Function: gpio_exp_test
 *
 * Description: Test the output pins 1 and 7 of the GPIO Expander 
 *              Register.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int gpio_exp_test (void)
{
    n2g_i2c_if_t  *pca;        
    int i2c_dev;
    uchar save, data = 0;

    //prpass(testpass, "GPIO Expander Register test");
    /*
     * 1. Set polarity reg to 0x0
     * 2. Set the config reg to select the output pins
     * 3. read/write to data reg and check the value
     */
    assert(ngio_ptr);
    assert(ngio_ptr->priv);

    prpass(testpass,"GPIO Expander - ");

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    i2c_dev = pca->i2c_dev; 
    pca->i2c_dev = NGDC_I2C_ADDR_IO_PORT;
    //printf("\n Bug : Set Polarity Inversion Register to 0x%x\n", data);
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Polarity register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    //printf("\n Test GPIO o/p pins 1 (Boot Select) and pin 7 (Debug mode)");
    if (io_port_8bit_i2c_read(pca, POLARITY_INV_REG, &data, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 Polarity register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    //printf("\n Polarity Inversion Register = 0x%x", data);
    data = 0x7D;
    //printf("\n Set Configuration Register to 0x%x", data);
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &data, TRUE);
    //printf("\n Read back Configuration Register = 0x%x", data);
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 Input register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    //printf("\n Input Port Register = 0x%x", data);
    save = data;
    data = ((~(save&0x82))&0x82);
    //printf("\n Set Output Port Registe to 0x%x", data);
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Output register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 Input register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    //printf("\n Read back Input Port Register = 0x%x\n", data);
    if ((data&0x82) != ((~(save&0x82))&0x82)) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Tried to write 0x%x but read back 0x%x from Input"
              "port register \n", __FUNCTION__, (((~(save&0x82))&0x82)), (data&0x82));
        return (FAILED);
    }
    //printf("\n Write back default value to Output Port Register 0x%x", save);
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT_REG, &save) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Output register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 Input register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    if ((data&0x82) != (save&0x82)) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Tried to write default value 0x%x but read back 0x%x\n", __FUNCTION__, 
               (data&0x82), (save&0x82));
        return (FAILED);
    }
    pca->i2c_dev = i2c_dev;
    //graff_submenu_prcomplete();
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: graffham_run_test
 *
 *  Description: Run all the Graffham NGVM specific tests.
 *
 *  Input: ngio interface struct *
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *  
 **********************************************************************
 */
static int graffham_run_test ( void)
{
    int  retval = PASSED;

    assert(ngio_ptr);

    /* All these tests are executed by sending a ethernet test command
       packet to the Graffham ARM CPU0 */
    if (retval == PASSED) {
        retval = graff_intf_sync_test();
    } 
    if (retval == PASSED) {
        retval = graff_dsp_ddr3_sdram_test_wrapper();
    } 
    if (retval == PASSED) {
        retval = graff_ge_intlpbk_test_wrapper(0);
    }
    if (retval == PASSED) {
       retval = graff_ge_lpbk_test_wrapper(0);
    }
    if (retval == PASSED) {
        retval = graff_ge_intlpbk_test_wrapper(1);
    }
    if (retval == PASSED) {
       retval = graff_ge_lpbk_test_wrapper(1);
    }
    if (retval == PASSED) {
        retval = graff_arm11_cpu1_boot_test_wrapper();
    }
    if (retval == PASSED) {
        retval = graff_dss_core0_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = graff_dss_core1_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = graff_dss_core2_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = graff_dss_core3_sanity_test_wrapper();
    }
#ifdef TDM_INT_LPBK
    if (retval == PASSED) {
        retval = graff_tdm_int_lpbk_test_wrapper();
    }
#endif
    if (retval == PASSED) {
        retval = graff_tdm_ext_lpbk_test_wrapper();
    }
    if (retval == PASSED) {
        retval = ecc_mem_test();
    }
    if (retval == PASSED) {
        retval = graff_uart_test();
    }

    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: ngvm_main_test
 *
 *  Description: This is the test entry point for Graffham NGVM.
 *
 *  Input: ngio interface struct *
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *  
 **********************************************************************
 */
static int ngvm_main_test ( void)
{
    int  retval = PASSED;

    /* Will test the Host-Side FPGA Reg, Interrupts, Uart, GDF Int lpbk,
       pcie link */
    if (retval == PASSED) {
        retval = gpio_resistor_test();
    }

    if (retval == PASSED) {
        retval = gpio_exp_test();
    }

    if (retval == PASSED) {
        retval = graffham_test(TRUE);
    }

    return (retval);
}

#if 0
/*
 **********************************************************************
 *  
 *  Function: graff_disp_rev
 *  
 *  Description: Dsiplay the DSP FW version 
 *
 *  Input: None
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_disp_rev (int dsp)
{
    ngvm_entity_t *ngvm_ep;

    assert(ngio_ptr);

    /* check if DSP is already booted  only one dsp on Graffham */
    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    if (ngvm_ep->dsp_downloaded[0] == TRUE) {
        printf("\n DSP firmware is downloaded and DSP is booted up\n");
        printf("\n DSP FW Rev. major.minor.debug = %d.%d.%d \n",
               ngvm_ep->major_rel[0], ngvm_ep->minor_rel[0], 
               ngvm_ep->debug_ver[0]);
    } else {
        printf("\n DSP firmware is not downloaded. DSP not up."
               " Cannot display firmware version\n");
        return (FAILED);
    }
    return (PASSED);
}
#endif

/*
 **********************************************************************
 *
 *  Function: graffham_bringup_dsp
 *
 *  Description: Unreset DSP, dnld, firmware and check if dsp is ready
 *
 *  Input: None
 *  
 *  Returns: PASSED if successful; 
 *           FAILED  - (I2C GPIO access error or DSP not set READY PIN)
 *                     or DSP up but not in READY mode or DSP up but
 *                     firmware not correct.
 *
 **********************************************************************
 */
static int graffham_bringup_dsp (void)
{
    n2g_i2c_if_t  *pca;
    ngvm_entity_t *ngvm_ep;
    int i2c_dev, i;
    uchar data = 0;

    assert(ngio_ptr);

    /* For ethernet mode loopback is never enabled */
    if (dsp_tests_use_enet == 0) {  /* for uart mode */
        printf("\n Make sure GE Loopback mode is disabled.");
        disable_bp_ge_lpbk(0);
        disable_bp_ge_lpbk(1);
    }

    /* check if DSP is already booted  only one dsp on Graffham */
    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    if (ngvm_ep->dsp_downloaded[0] == TRUE) {
        //printf(" DSP firmware is downloaded and DSP is booted up");
        return (PASSED);
    }
    /* reset PVDM */
    ngvm_ep->reset_ngvm();

    printf("\r Please wait for the Graffham Bootloader to dhcp the "
           "firmware");
    fflush(stdout);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    i2c_dev = pca->i2c_dev; 
    pca->i2c_dev = NGDC_I2C_ADDR_IO_PORT;
    for (i = 0; i < 60; i++) {
        if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
            cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", 
                  INPUT_PORT_REG);
            return (FAILED);
        }
        if (data & PRIM_INTF_READY) {
            printf("\r GPIO Expander bit 3(READY Bit) Set by Graffham");
            break;
        } else if (i == 59) {
            printf("\n*** %s(): GPIO Expander bit 3 not set by DSP\n"
                   "Graffham not booted up, Tests will fail.", __FUNCTION__);
            return (FAILED);
        } 
        //prpass(testpass, " .");fflush(stdout);
    //printf("\b");
        switch (i%8) {
        case 0:
            printf("\r|");
            break;
        case 1:
            printf("\r/");
            break;
        case 2:
            printf("\r-");
            break;
        case 3:
            printf("\r\\");
            break;
        case 4:
            printf("\r|");
            break;
        case 5:
            printf("\r/");
            break;
        case 6:
            printf("\r-");
            break;
        case 7:
            printf("\r\\");
            break;
        default:
            break;
        }
        fflush(stdout);
        //printf("\r ");

        msleep(400);
    }
    pca->i2c_dev = i2c_dev;
    msleep(1000);
    if ((graffham_ready_test(0)) == PASSED)  {
        ngvm_ep->dsp_downloaded[0] = TRUE;
        //printf("\r Received Ready Packet from Graffham \n");
        printf("\r");
    } else {
        printf("\n %s(): graffham_ready_test() failed\n", __FUNCTION__);
        return (FAILED);
    }
    
    /* SR add later display board and DSP FW revision */
    //graff_disp_rev(0);

    if (dsp_tests_use_enet == 0) {
        printf("\n Enabling GE loopback mode ");
        enable_bp_ge_lpbk(0);
    }
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: graffham_vm_test
 *
 *  Description: This is the test entry point for Graffham NGVM.
 *
 *  Input: ngio interface struct *
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
int graffham_vm_test (void *vm)
{
    ngvm_entity_t *ngvm_ep;
    int ret_val;
    ushort board_id = 0;

    /* Called when NGVM present either in onboard slot or as a daughter-card in 
       one of the NGIOs */

    assert(vm);

    ngio_ptr = (struct ngio_intf_t *)vm;
    board_id = ngio_ptr->id;
    if ((board_id == GRAFFHAM_VM) || (board_id == GRAFFHAM_TESTCARD)) { 
        if (init_ngio_priv_struct() == FAILED) {
            cterr('f', '0', " %s(): Unknown slot type for Graffham NGVM %d \n", 
                  __FUNCTION__, ngio_ptr->slot);
            return (FAILED);
        }
        /* initialize Graffham internal entities for operations */
        graffham_init_fp();
    } else {
        cterr('f', '0', "\n %s(): NGVM Board ID = 0x%x not Graffham board"
              " ID", __FUNCTION__, board_id);
        return (FAILED);
    }

    assert(ngio_ptr->priv);
    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;

    /* Display the NGVM module details */
    printf("\r %s", ngvm_ep->name);

    /* ngvm_num : onboard = 0, NGWIC1 DC = 1, ... */
#ifdef DEBUG_GRAFF
    /* The actual number if all the slots(onboard, NGIOs)  
       were populated with NGVM */
    printf("\n NGVM_NUM = %d", ngvm_ep->ngvm_num);
#endif

    /* Find and display the pid for the NGVM */
    if ((ngvm_ep->get_pid(ngvm_ep->pid)) == FAILED) {
        cterr('f', 0, "%s(): Invalid NGVM PID %s", __FUNCTION__, ngvm_ep->pid);
        return (FAILED);
    }

#if 0
    /* PID will change for next NGVM build so remove the specific
       check */
    /* Make sure correct PID for onboard Vs DB */
    if (ngio_ptr->mod_type == VM_MODULE) {
        if (strstr(ngvm_ep->pid, "-MB-") == NULL) {
            cterr('f', 0, "%s(): Incorrect NGVM PID:%s in onboard slot", 
                  __FUNCTION__, ngvm_ep->pid);
            return (FAILED);
        }
    } 
    /* Manufacturing will test MB PID in NGWIC slot so remove check for DC */
      else if ((ngio_ptr->mod_type == SM_DAUGHTER_CARD) || 
               (ngio_ptr->mod_type == WIC_DAUGHTER_CARD)) {
        if (strstr(ngvm_ep->pid, "PVDM4-TDM") == NULL) {
            cterr('f', 0, "%s(): Incorrect NGVM PID in DB slot %s",
                  __FUNCTION__, ngvm_ep->pid);
            return (FAILED);
        }
    }
#endif
    sprintf(maindiag_header.title, "%s", ngvm_ep->pid);
    prpass(testpass, " PID: %s", ngvm_ep->pid);

    /* TFTP download the NGVM application fw to the platform */
    /* tftp_get does not return error */
    if (tftp_get(0, "dsp_sp2700_fw_diag.img", 0, "/firmware/dsp_sp2700_fw.img",
                 1) < 0) {
        cterr('f', 0, "Failed to tftp download firmware to local host");
        return (FAILED);
    }


    /* Init the NGVM module specific parameters mac etc */
    if (ngvm_ep->init_ngvm()) {
        return (FAILED);
    }

    graffham_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;
    sprintf(graffmainmenutitle, "%s Main Menu", ngvm_ep->slot_type_str);
    sprintf(graffsubmenutitle, "%s", ngvm_ep->slot_type_str);
    build_primary_submenu(graffham_mainmenu_tbl, GRAFFHAM_MAINMENU_TBL_SIZE, 
                          "Graffham Menu", &maindiagp);
    build_secondary_submenu(graffham_mainmenu_tbl, GRAFFHAM_MAINMENU_TBL_SIZE,
                            main_menu_secondary_items);

    if (ngio_ptr->menu_display) {
        menu(&graffham_mainmenu, main_menu_secondary_items, '\0');
        pre_diag_exec = graffham_saved_diag_exec;
        ret_val = PASSED;
    } else {
        ret_val = ngvm_main_test();
    }

    graffham_vm_cleanup();
    return (ret_val);

}

/**********************************************************************
 *
 * Function: graffham_ready_test 
 * This function test to make sure the Graffham is ready to accept commands
 *  
 * Input : dsp #
 *  
 * Output: PASSED/FAILED
 *         FAILED - tx READY command failed, did not recv resp, resp 
 *                  recevied does not match READY opcode, dsp firmware 
 *                  version returned is incorrect, ge setup/cleanup 
 *                  failed
 *
 **********************************************************************
 */ 
static int graffham_ready_test (int dsp_no)
{
    ngvm_entity_t *ep; 
    fe_packet_t   *recv_packet_p;
    dspif_ready_t *ready_msg_p;
    dspif_ready_t ready_msg;
    int retval = PASSED;
    
    printf("\r Send Ready Packet to Graffham ");

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    recv_packet_p = &(ep->recv_packet);
    ready_msg_p = &ready_msg;

    if (graff_setup_ge_env() == FAILED) {
        return (FAILED);
    }
    
    graff_clear_rx_buf((uchar *)ready_msg_p, (int)sizeof(dspif_ready_t));
    graff_clear_rx_buf((uchar *)recv_packet_p, (int)sizeof(fe_packet_t));
    
    graff_build_command_packet(SELECT_READY, 0, 0, DSS_CORE0);
    graff_build_config_packet();
    /* need to know which DSP on the PVDM */
    if ((retval = graff_send_command_packet(dsp_no)) != PASSED) {
        cterr('f', 0, "\n %s(): graff_send_command_packet() returned tx error "
              "%d\n", __FUNCTION__, retval);
        return (FAILED);
    }
    /* wait for result packet */
    if (graff_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 360,
                                 dsp_no)) {
        cterr('f', 0, "\n%s(): Timed out waiting for READY message from DSP.", 
               __FUNCTION__);
        return (FAILED);
    }
    memcpy((char *)ready_msg_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ready_t));
    //printf("\n op_type = 0x%x, expected 0x%x",
    //       ready_msg_p->dspif_hdr.op_type, OP_READY);
    /* parse result */
    if (ready_msg_p->dspif_hdr.op_type != OP_READY) {
        cterr('f', 0, "%s(): Ready test failed op_type: 0x%x: expected 0x%x",
              ready_msg_p->dspif_hdr.op_type, OP_READY, __FUNCTION__);
        return (FAILED);
    }
    /* check if the firmware version is correct */
    if ((ready_msg_p->fw_ver.major_num != DIAGFW_MAJ_REL) || 
        (ready_msg_p->fw_ver.minor_num != DIAGFW_MIN_REL) ||
        (ready_msg_p->fw_ver.debug_num != DIAGFW_DEBUG_VER)) {
        printf("\n DSP FW version does not match 0x%x:0x%x:0x%x, expected "
               "0x%x:0x%x:0x%x", ready_msg_p->fw_ver.major_num,
               ready_msg_p->fw_ver.minor_num, ready_msg_p->fw_ver.debug_num,
               DIAGFW_MAJ_REL, DIAGFW_MIN_REL, DIAGFW_DEBUG_VER);
        cterr('f', 0, "%s(): Ready test failed DSP diag FW version does not "
              "match", __FUNCTION__);
        return (FAILED);
    }
    /* Only one dsp */
    ep->major_rel[0] = ready_msg_p->fw_ver.major_num;
    ep->minor_rel[0] = ready_msg_p->fw_ver.minor_num;
    ep->debug_ver[0] = ready_msg_p->fw_ver.debug_num;

    if (graff_cleanup_ge_env() == FAILED) {
        cterr('f', 0, "%s(): graff_cleanup_ge_env() failed", __FUNCTION__);
        return (FAILED);
    }

    ep->ngvm_id = ready_msg_p->dsp_device.core_id;

    if (ready_msg_p->dsp_device.core_id == PFUSE123_SP2702)
        printf("\r Received Ready Response from Graffham SP2702     ");
    else if (ready_msg_p->dsp_device.core_id == PFUSE123_SP2704)
        printf("\r Received Ready Response from Graffham SP2704     ");
    else 
        printf("\r Received Ready Response from Graffham (unknown SP27XX) ");
    return retval;

}

/***********************************************************************
 * Name: graff_setup_ge_env (common)
 *
 * Description:
 *      This test will set up GE operation environment. 
 *
 * Input: None
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
static int graff_setup_ge_env (void)
{
    ngvm_entity_t *ep;
    int status = PASSED;
    int sgmii_port = 0;
    char if_name[IFNAMSIZ];
    
    assert(ngio_ptr);
    ep = (ngvm_entity_t *) ngio_ptr->priv;
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        /* Linux socket already setup return */
        return (PASSED);
    }

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);
#ifdef TABEIL
    sprintf(if_name, "%s", TABEI_ETH_BP);
#else
    sprintf(if_name, "eth%d", sgmii_port);
#endif
    status = setup_eth_dev(if_name, &(ep->socket_gl));

#ifdef GE_COMM
    printf("\n socket = %d", ep->socket_gl);
#endif

    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    ep->ge_setup_flag = TRUE;

    return (PASSED);
}

/***********************************************************************
 * Name: graff_cleanup_ge_env (common)
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: None
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int graff_cleanup_ge_env (void)
{
    ngvm_entity_t *ep;
    int status = PASSED;
    int sgmii_port = 0;
    char if_name[IFNAMSIZ];

    assert(ngio_ptr);
    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        ep->ge_setup_flag = FALSE;

	sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);
#ifdef TABEIL
        sprintf(if_name, "%s", TABEI_ETH_BP);
#else
        sprintf(if_name, "eth%d", sgmii_port);
#endif
        status = cleanup_eth_dev(if_name, ep->socket_gl);

        if (status) {
            cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
            return (FAILED);
        }
        return (PASSED);
    }
    return (PASSED);

}

/***********************************************************************
  * Name: graff_clear_rx_buf (common)
  *
  * Description:
  *      Clear receiver buffer before testing.
  *
  * Input: char * - array ptr
  *        int    - number of bytes to clear.
  *
  * Output: NONE.
  *
  ***********************************************************************
  */
void graff_clear_rx_buf (uchar *c_ptr, int num_bytes)
{
    int i;

    for (i = 0; i < num_bytes; i++) {
        *c_ptr++ = 0;
    }
}

/*
 **********************************************************************
 *
 *  Function: graff_build_config_packet
 *
 *  Description: PID info etc 
 *
 *  Input: None 
 *
 *  Returns: None
 *
 **********************************************************************
 */
void graff_build_config_packet (void)
{
    ngvm_entity_t *ep;
    dspif_ether_t *cmd_packet_p;
    uint i;
    uchar *ngvm_pid;

    assert(ngio_ptr);

    ep = ngio_ptr->priv;
    assert(ep);

    cmd_packet_p = &(ep->cmd_packet);
    ngvm_pid = (uchar *)&(cmd_packet_p->dspif_info.bufmsg[0]);
    for (i = 0; i < 128; i++)
        ngvm_pid[i] = ep->pid[i];
#ifdef DSP_DEBUG
    printf("\n In graff_build_config_packet() pid = %s\n", ngvm_pid);
#endif
    /* 0 - dsp_tests_use_enet : 1= enet intf 0= uart intf
     * 1 - menu_display   : If uart intf, display menu [1] or run the tests [0]
     * 2 - module_type  : 6= VM_MODULE etc
     * 3 - parent slot  : applicable if DC
     */
    cmd_packet_p->dspif_info.errmsg[2] = ngio_ptr->mod_type;
    if (ngio_ptr->mod_type != VM_MODULE) {
        assert(ngio_ptr->pc);
        /* Parent slot type */
        cmd_packet_p->dspif_info.errmsg[3] = ngio_ptr->pc->slot;  
    }
    // dsp_tests_use_enet = 0; /* Use ether pkts for running tests */
    cmd_packet_p->dspif_info.errmsg[0] = dsp_tests_use_enet; //eth or uart interface
    cmd_packet_p->dspif_info.errmsg[1] = ngio_ptr->menu_display; /*if uart then run tests or just display the menu */
    dsp_tests_use_enet = cmd_packet_p->dspif_info.errmsg[0]; 
#ifdef DSP_DEBUG
    dismem((char *)cmd_packet_p, sizeof(dspif_ether_t), (ulong)cmd_packet_p, 1);
    printf("\n In graff_build_config_packet() dc_slot = %d\n", 
           cmd_packet_p->dspif_info.errmsg[0]);
#endif
}

/*
 **********************************************************************
 *
 *  Function: graff_build_command_packet
 *
 *  Description: build test command
 *
 *  Input: selected test; which DSP core
 *
 *  Returns: None
 *
 **********************************************************************
 */
void graff_build_command_packet (uint16_t select_test, uint32_t param1, 
                                 uint32_t param2, uint8_t core_id)
{
    ngvm_entity_t *ep;
    dspif_ether_t *cmd_packet_p;

    assert(ngio_ptr);

    ep = ngio_ptr->priv;
    assert(ep);

    cmd_packet_p = &(ep->cmd_packet);
    //SR orig cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->dspif_hdr.dest_id = SWAP32(ep->plat_ngvm_num);

    cmd_packet_p->dspif_hdr.op_type = (OP_TEST_REQUEST);
    cmd_packet_p->dspif_hdr.data_len = (sizeof(dspif_info_t));
    cmd_packet_p->dspif_info.command = 0;
    cmd_packet_p->dspif_info.result = SWAP32(RESULT_RUNNING);
    cmd_packet_p->dspif_info.flags = SWAP32(FLAG_NULL);
    cmd_packet_p->dspif_info.select = select_test;
    cmd_packet_p->dspif_info.faults = 0;
    cmd_packet_p->dspif_info.location = 0;
    cmd_packet_p->dspif_info.expected = 0;
    cmd_packet_p->dspif_info.actual = 0;
    cmd_packet_p->dspif_info.extra = 0;
    cmd_packet_p->dspif_info.errorcount = 0;
    cmd_packet_p->dspif_info.testcounter = 0;
    cmd_packet_p->dspif_info.ReadyOnTest = 0;
    cmd_packet_p->dspif_info.TestCtrl = 0;
    cmd_packet_p->dspif_info.WhoAmI = 0;
    cmd_packet_p->dspif_info.ver_no = 0;
    cmd_packet_p->dspif_info.wait_states = 0;
    cmd_packet_p->dspif_info.param1 = param1;
    cmd_packet_p->dspif_info.param2 = param2;
    cmd_packet_p->dspif_info.param3 = 0;
    cmd_packet_p->dspif_info.param4 = 0;
#ifdef GE_DEBUG
    printf("\n cmd_packet_p->dspif_hdr.src_id = 0x%x", SWAP32(HOST_ID));
    printf("\n cmd_packet_p->dspif_hdr.dest_id = 0x%x", SWAP32(core_id));
    printf("\n cmd_packet_p->dspif_hdr.op_type = 0x%x", SWAP32(OP_TEST_REQUEST));
    printf("\n cmd_packet_p->dspif_hdr.data_len = 0x%x", SWAP32(sizeof(dspif_info_t)));
#endif
    memset((cmd_packet_p->dspif_info.bufmsg), 0, 128);
    memset((cmd_packet_p->dspif_info.errmsg), 0, 128);
}

/*
 **********************************************************************
 *
 *  Function: graff_send_command_packet (not common specific)
 *
 *  Description: build test command
 *
 *  Input: Slot; Which DSP
 *
 *  Returns: PASSED
 *           FAILED - some tx error
 *
 **********************************************************************
 */
int graff_send_command_packet (int dsp_no)
{
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    ngvm_entity_t *ep;
    //mac_addr_t  src_mac_addr;
    int ret_val = PASSED;

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    //get_local_mac_addr(CPU_SGMII_PORT1, (uchar *)&src_mac_addr[0]);
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
 
    /* Write to routine to read the MAC address fromt the cookie from the 
     * NGVM. Till then use the bcast address.
     */
    memcpy((uchar *)(tx_pkt_p->dest_addr), 
           (uchar *)&(ep->eth_hdr[dsp_no].dest_addr),
           sizeof(mac_addr_t));
    //memcpy((uchar *)(tx_pkt_p->dest_addr), bcast_mac_addr, sizeof(mac_addr_t));
    memcpy((uchar *)(tx_pkt_p->src_addr), 
           (uchar *)&(ep->eth_hdr[dsp_no].src_addr), 
           sizeof(mac_addr_t)); /* host MAC */
    //memset((uchar *)(tx_pkt_p->src_addr), 0, sizeof(mac_addr_t)); /* host MAC */
    tx_pkt_p->pkt_type = PKT_TYPE_IPV4; 
    tx_pkt_p->payload_size = sizeof(dspif_ether_t);
    tx_pkt_p->bufr_st_addr = (uchar *)&(ep->cmd_packet);
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = ep->socket_gl;

#if 0
    printf("\n Packet to Send \n");
    printf("\n pkt_type = 0x%x", tx_pkt_p->pkt_type);
    printf("\n payload_size = 0x%x", tx_pkt_p->payload_size);
    printf("\n sending the packet\n");
#endif
    ret_val = eth_pkt_tx(tx_pkt_p);

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: graff_sgmii_dsp_lpbk_test
 *
 *  Description: test ge loopback with different frames 
 *
 *  Input:  dsp number
 *          GE port number
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int graff_sgmii_dsp_lpbk_test (int dsp_no, int port_num)
{
    ngvm_entity_t *ep;
    int retval = PASSED;
    uint32 frame_num, frm_size;
    mac_addr_t mac_da;
    unsigned short pak_size[32] = {64, 108, 512, 256,
                1500, 65, 1511, 128,
                66, 719, 100, 1513,
                1000, 200, 78, 1514,
                300, 400, 312, 168,
                67, 955, 60, 512,
                333, 888, 83, 128,
                135, 531, 99, 1024};

    retval = PASSED;

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    memcpy((uchar *)mac_da, (uchar *)&(ep->eth_hdr[dsp_no].dest_addr),
           sizeof(mac_addr_t));

    for (frame_num = 0; frame_num < 3; frame_num++) {
        frm_size = pak_size[frame_num];
        prpass(testpass,"GE%d Ext Loopback test, frame%d, size %d ",
               port_num, frame_num, frm_size);
        if (graff_eth_frames_test(frame_num, frm_size, ep->socket_gl,
                                mac_da, dsp_no)) {
            retval = FAILED;
            break;
        }
    }

    return(retval);
}

/*
 **********************************************************************
 *
 *  Function: graff_build_lpbk_frame
 *
 *  Description: build the test data frame for ge loopback
 *
 *  Input: frame buffer pointer; desitnation MAC, size, test base value
 *                and  increment value;
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int graff_build_lpbk_frame (dspif_lpbk_t *frame_ptr, mac_addr_t dst_mac_addr,
                            uint16 frm_size, char base_val, char inc_val)
{
        uint32 data_len, count;
    uchar data;
    uchar *datap;
    dspif_lpbk_t *framep;

    /* build ethernet frame header */
    framep = (dspif_lpbk_t *)frame_ptr;

    /* build ethernet frame payload */

    data = base_val;
    datap = (uchar *)&framep->data;
    data_len = frm_size - sizeof(ether_hdr_t);
    for (count = 0; count < data_len; count++) {
        *datap++ = data;
        data += inc_val;
    }

        return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: c2ru_graff_check_rx_frame
 *
 *  Description: Check if the recevied frame test data is expected.
 *               Curie 2RU uses this function to drop unrelated packets.
 *
 *  Input: test frame pointer; recv frame pointer; packet number and size
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int c2ru_graff_check_rx_frame(volatile dspif_lpbk_t * test_frame_p,
                                     volatile uchar *recv_frame_p, int packet_num,
                                     uint32 frm_size)
{
    int count;
    uchar *rd_ptr, *wr_ptr;

    if (!is_curie_2ru())
        return PASSED;

    rd_ptr = (uchar *)(recv_frame_p + sizeof(ether_hdr_t));
    wr_ptr = (uchar *)(test_frame_p->data);

    for (count = 0; count < (frm_size - sizeof(ether_hdr_t)); count++) {
        if (*rd_ptr != *wr_ptr) {
            printf("\ndrop unrelated packet: %x: %02x %02x %02x %02x\n",
                   count, rd_ptr[0], rd_ptr[1], rd_ptr[2], rd_ptr[3]);
            return FAILED;
        }
        rd_ptr++;
        wr_ptr++;
    }

    return PASSED;
}

/*
 **********************************************************************
 *
 *  Function: graff_check_rx_frame
 *
 *  Description: Check the recevied frame test data 
 *
 *  Input: test frame pointer; recv frame pointer; packet number and size  
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int graff_check_rx_frame (volatile dspif_lpbk_t * test_frame_p, 
                          volatile uchar *recv_frame_p, int packet_num, 
                          uint32 frm_size)
{
    int count, error = 0;
    uchar *rd_ptr, *wr_ptr;

    /*
     * verify that we received the correct number of bytes
     * the byte count in tx_bd->length does not include 4 bytes of CRC
     * while the byte count in rx_bd->length does include it
     */
        rd_ptr = (uchar *)(recv_frame_p + sizeof(ether_hdr_t));
        wr_ptr = (uchar *)(test_frame_p->data);
  
        for (count = 0; count < (frm_size - sizeof(ether_hdr_t)); count++) {
            if (*rd_ptr != *wr_ptr) {
                /* dump packet data */
                printf("\n Ether header for received pkt-");
                dismem((uchar *)recv_frame_p, sizeof(ether_hdr_t), 
                       (ulong)(recv_frame_p), 1);
                printf("\n Rx pkt data -");
                dismem((uchar *)(recv_frame_p + sizeof(ether_hdr_t)), count+4,
                       (ulong)(recv_frame_p+ sizeof(ether_hdr_t)), 1);
                printf("\n Tx pkt data -");
                dismem((uchar *)test_frame_p->data, count+4,
                       (ulong)(test_frame_p->data), 1);
                cterr('f', 0, "Packet%d data mismatch at offset %#x, "
                      "sent %#.8x, rcvd %#.8x\ntx bd @%#.8x, rx bd @%#.8x",
                      packet_num, count, *wr_ptr, *rd_ptr, test_frame_p, recv_frame_p);
                error = FAILED;
                break;
            }
            rd_ptr++;
            wr_ptr++;
        }

    return (error);
}

/*
 **********************************************************************
 *
 *  Function: graff_eth_frames_test
 *
 *  Description: send and check received test frames
 *
 *  Input: test frame number, destination MAC,
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int graff_eth_frames_test (uint32 frame_num, uint32 frm_size, int socket_gl, 
                           mac_addr_t dst_mac_addr, int dsp)
{
    mac_addr_t   src_mac_addr;
    dspif_lpbk_t test_frame;
    dspif_lpbk_t *test_frame_p = &test_frame;
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    int   result = PASSED, wait_time = 0;
    uchar recv_frame_p[2048];
    char  base_val, inc_val;
    int retry_count = 0;

    memset((uchar *)test_frame_p, 0, sizeof(dspif_lpbk_t));
    memset((uchar *)recv_frame_p, 0, sizeof(recv_frame_p));

    /* make ethernet frame, data pat used dependent on odd/even frame size */
    if (frm_size & 1) {
        base_val = 0;
        inc_val = 1;
    } else {
        base_val = 0xff;
        inc_val = -1;
    }

    /* make ethernet frame, data pat dependent on odd/even frame size */
    if (graff_build_lpbk_frame(test_frame_p, dst_mac_addr, frm_size,
                        base_val, inc_val) == FAILED) {
        return (FAILED);
    }

    get_host_mac_addr(0, (uchar *)&src_mac_addr[0]);
   
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    //memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)dst_mac_addr, 
    //       sizeof(mac_addr_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr), "FFFFFFFFFFFF", 
           sizeof(mac_addr_t));
    memset((uchar *)(tx_pkt_p->src_addr), 0, sizeof(mac_addr_t)); /* host MAC */
    memcpy((uchar *)(tx_pkt_p->src_addr), src_mac_addr, sizeof(mac_addr_t)); /* host MAC */

    tx_pkt_p->pkt_type = PKT_TYPE_IPV4; 
    tx_pkt_p->payload_size = frm_size - sizeof(ether_hdr_t); /* payload size */
    tx_pkt_p->bufr_st_addr = (uchar *)&(test_frame_p->data); /* payload */
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;

    msleep(2000); /* Wait till DSP is ready for the loopback frames */
    msleep(2000); /* Wait till DSP is ready for the loopback frames */
    prpass(testpass, "Sending lpbk frame#%d, size = %d ... ", frame_num, frm_size);
    //getchar();
    result = eth_pkt_tx(tx_pkt_p);
    if (result != ETH_PKT_TX_OK ) {
        cterr('f', 0, "%s: Failed to TX lpbk Frame#%d, size = %d : ret = 0x%x,"
              " status = 0x%x", __FUNCTION__, frame_num, frm_size, result, 
              tx_pkt_p->tx_status);
        return (FAILED);
    }

    wait_time = 0x1000;
    prpass(testpass, "Waiting for lpbk response for frame#%d ", frame_num);
    //getchar();
retry:
    while ((result = graff_wait_for_ge_packet(recv_frame_p, socket_gl, 
            INTR_MODE, dsp, wait_time)) == FAILED) {
        if (--wait_time <= 0) {
            cterr('f', 0, "Failed to RX lpbk Frame#%d, size = %d \n", frame_num, frm_size);
            return (FAILED);
        }
        msleep(1);
    }

    /* Same workaround on Oakenshield on Curie 2RU
     * Found one unrelated packet received. We display and
     * drop it, and continue to receive next packet. if there is no next
     * packet received, timeout will be returned at above location */
    if (c2ru_graff_check_rx_frame(test_frame_p, recv_frame_p, frame_num,
                                  frm_size) != PASSED) {
        retry_count++;

        if (retry_count < 5)
            goto retry;
    }

    if (result == PASSED) {
        result = graff_check_rx_frame (test_frame_p, recv_frame_p, frame_num, 
                                       frm_size);
        if (result == PASSED) {
            prpass(testpass, "RX frame #%d matches for size %d ", frame_num, 
                   frm_size);
        }
    }
    printf("\r                                                               ");
    return (result);
}

/*
 **********************************************************************
 *
 *  Function: graff_build_stop_command_packet
 *          
 *  Description: build test command
 *
 *  Input: Selected test; which dsp core
 *          
 *  Returns: None
 *
 **********************************************************************
 */
void graff_build_stop_command_packet (uint16_t select_test, uint8_t core_id)
{
    ngvm_entity_t *ep;
    dspif_ether_t *cmd_packet_p;

    assert(ngio_ptr);
    ep = ngio_ptr->priv;

    assert(ep);
    cmd_packet_p = &(ep->cmd_packet);

    //cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->dspif_hdr.dest_id = SWAP32(ep->plat_ngvm_num);
    //cmd_packet_p->dspif_hdr.op_type = SWAP32(OP_TEST_STOP);
    cmd_packet_p->dspif_hdr.op_type = (OP_TEST_STOP);
    //cmd_packet_p->dspif_hdr.data_len = SWAP32(sizeof(dspif_info_t));
    cmd_packet_p->dspif_info.command = 0;
    cmd_packet_p->dspif_info.result = SWAP32(RESULT_RUNNING);
    cmd_packet_p->dspif_info.flags = SWAP32(FLAG_NULL);
    cmd_packet_p->dspif_info.select = (select_test);
    cmd_packet_p->dspif_info.faults = 0;
    cmd_packet_p->dspif_info.location = 0;
    cmd_packet_p->dspif_info.expected = 0;
    cmd_packet_p->dspif_info.actual = 0;
    cmd_packet_p->dspif_info.extra = 0;
    cmd_packet_p->dspif_info.errorcount = 0;
    cmd_packet_p->dspif_info.testcounter = 0;
    cmd_packet_p->dspif_info.ReadyOnTest = 0;
    cmd_packet_p->dspif_info.TestCtrl = 0;
    cmd_packet_p->dspif_info.WhoAmI = 0;
    cmd_packet_p->dspif_info.ver_no = 0;
    cmd_packet_p->dspif_info.wait_states = 0;
    cmd_packet_p->dspif_info.param1 = 0;
    cmd_packet_p->dspif_info.param2 = 0;
    cmd_packet_p->dspif_info.param3 = 0;
    cmd_packet_p->dspif_info.param4 = 0;
    memset((cmd_packet_p->dspif_info.bufmsg), 0, 128);
    memset((cmd_packet_p->dspif_info.errmsg), 0, 128);
}

/*
 **********************************************************************
 *
 *  Function: graff_wait_result_packet
 *
 *  Description: wait for result
 *
 *  Input: packet buffer; number of retry
 *
 *  Returns: PASSED - pkt received
 *           FAILED - pkt not received within the delay time
 *
 **********************************************************************
 */
int graff_wait_result_packet (uint8_t *pak, int socket_gl, uint16_t retry, int dsp)
{
    int ret_val = PASSED;
    int count = retry;

    memset(pak, 0, sizeof(dspif_ether_t));

    if (count < 2) {  /* warning programmer only */
        printf("retry count too small, not recommend!!!\n");
        return (FAILED);
    }

    /* Wait for Gige packet */
    while (count) {
        if (!(graff_wait_for_ge_packet(pak, socket_gl, INTR_MODE, dsp, count))) 
        {
            break;
        }
        count -= 1;
        msleep(1000);
    }

    if (count == 0) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: graff_ge_lpbk_test
 *
 *  Description: Send ge lpbk command and invoke DSP prepare to echo
 *               input packet.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int graff_ge_lpbk_test (int local_port)
{
    ngvm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int retval = PASSED;

    prpass(testpass, "Host <--> Grahham GE%d Ext Loopback test", local_port);

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    recv_packet_p = &(ep->recv_packet);
    result_packet_p = &(ep->result_packet);

#ifdef SR_ADD_LATER
    if (ep->fw_dnlded == FALSE) {
        if (lsi_sp27xx_app_fw_dnld(ngvm_num)) {
            return(FAILED);
        }
    }
#endif
/* SR add rx check for mac as well as slot id */
    graff_clear_rx_buf((uchar *)result_packet_p, (int)sizeof(dspif_ether_t));
    graff_clear_rx_buf((uchar *)recv_packet_p, (int)sizeof(fe_packet_t));
    /*
     * build command packet to run dsp sanity test
     * test result will be handle in processing receiving packets
     */
    /* DSS_CORE0 does not matter here */
    if (local_port == 0)
        graff_build_command_packet(SELECT_GE0_LPBK, 0, 0, DSS_CORE0);
    else
        graff_build_command_packet(SELECT_GE1_LPBK, 0, 0, DSS_CORE0);
    /* need to know which DSP on the PVDM */
    if ((retval = graff_send_command_packet(0)) != PASSED) {
        printf("\n %s(): graff_send_command_packet() returned tx error %d\n",
               __FUNCTION__, retval);
        return (FAILED);
    }
    /* allow DSP to set up loopback connection */
    msleep(10);

    retval = graff_sgmii_dsp_lpbk_test(0, local_port);

    if (local_port == 0)
        graff_build_stop_command_packet(SELECT_GE0_LPBK, DSS_CORE0);
    else
        graff_build_stop_command_packet(SELECT_GE1_LPBK, DSS_CORE0);
    if ((retval = graff_send_command_packet(0)) != PASSED) {
        printf("\n %s(): graff_send_command_packet() returned tx error %d\n",
               __FUNCTION__, retval);
        return (FAILED);
    }
    prpass(testpass, " Sent STOP GE %d Loopback command packet ", local_port);

    if (graff_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 20, 0)) {
        cterr('f', 0, "Timed out waiting for GE loopback test result.");
               return (FAILED);
    };
    if (retval != PASSED) {
        /* SGMII loopback failed */
        cterr('f', 0, "graff_ge_lpbk_test() SGMII loopback failed retval = %#x",
               retval);
        return(FAILED);
    }
    prpass(testpass, " Host <--> Graffham GE %d Loopback completed ", local_port);
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: graff_dac_1dot5vm_high
 *
 *  Description: Set 1.5 voltage margin high.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dac_1dot5vm_high (void)
{
    prpass(testpass, "1.5 Voltage Margin High");
    return (graff_select_test(SELECT_DAC_1DOT5VM_HIGH, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: graff_dac_1dot5vm_low
 *
 *  Description: Set 1.5 voltage margin low.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dac_1dot5vm_low (void)
{
    prpass(testpass, "1.5 Voltage Margin Low");
    return (graff_select_test(SELECT_DAC_1DOT5VM_LOW, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: graff_dac_no_1dot5vm
 *
 *  Description: No 1.5 voltage margin.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dac_no_1dot5vm (void)
{
    prpass(testpass, "1.5 No Voltage Margin");
    return (graff_select_test(SELECT_DAC_NO_1DOT5VM, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: graff_dac_dot93vm_high
 *
 *  Description: Set .93 voltage margin high.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dac_dot93vm_high (void)
{
    prpass(testpass, ".93 Voltage Margin High");
    return (graff_select_test(SELECT_DAC_DOT93VM_HIGH, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: graff_dac_dot93vm_low
 *
 *  Description: Set .93 voltage margin low.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dac_dot93vm_low (void)
{
    prpass(testpass, ".93 Voltage Margin Low");
    return (graff_select_test(SELECT_DAC_DOT93VM_LOW, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: graff_dac_no_dot93vm
 *
 *  Description: No .93 voltage margin.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dac_no_dot93vm (void)
{
    prpass(testpass, ".93 No Voltage Margin");
    return (graff_select_test(SELECT_DAC_NO_DOT93VM, 0, 0, 100));
}


/*
 **********************************************************************
 *
 *  Function: graff_dsp_ddr3_sdram_test_wrapper
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dsp_ddr3_sdram_test_wrapper (void)
{
    prpass(testpass, "DDR3 Memory - ");
    return (graff_select_test(SELECT_DSP_SDRAM, 0, 0, 800));

}

static int graff_intf_sync_test (void)
{
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;
    ngvm_entity_t *ngvm_ep;
    unsigned long fpga_addr;
    volatile uint32_t *sync_out_trig_out, *sync_out1, *sync_in_trig_in;
    uint32_t dummy_rd, host_fpga_rev =0;
    int result, ret = PASSED;

    assert(ngio_ptr);

    if (ngio_ptr->mod_type != VM_MODULE) {
        printf("\rGraffham NGVM in NGIO slot does not support SYNC signal tests");
        return (PASSED);
    } 

    /* Check Host(Overlord) FPGA revision */
    host_fpga_rev = (uint32_t)fpga->ver;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Host(Overlord) Master FPGA Revision register: 0x%04X.\n",
               __FUNCTION__, host_fpga_rev);
    }

    if (is_overlord() || is_juno()) {
        if ((host_fpga_rev & 0x7FFFFF) < 0x020500) {
            printf("To support this TestCard Sync Signal test, you need to upgrade "
                   "Host(Overlord) FPGA revision to 2.5 or higher.\n");
            return (PASSED);
        }
    } else {
        if ((host_fpga_rev & 0x7FFFFF) < 0x000300) {
            printf("To support this TestCard Sync Signal test, you need to upgrade "
                   "Host(USD series) FPGA revision to 0.3 or higher.\n");
            return (PASSED);
        }
    }

    fpga_addr = get_platform_net_clk_ptp_conf_base();
    sync_out_trig_out = (volatile uint32_t *)(fpga_addr+NGVM_SYNC_OUT_SYNC_TRIG_OUT);
    sync_out1 = (volatile uint32_t *)(fpga_addr+NGVM_SYNC_OUT1_CTRL);
    sync_in_trig_in = (volatile uint32_t *)(fpga_addr+SYNC_TRIG_IN_SYNC_IN_DBG);

    dummy_rd = *sync_out_trig_out;

#ifdef SYNC_DEBUG
    printf("\n In graff_intf_sync_test() @sync_out_trig_out = 0x%lx, "
           "*sync_out_trig_out = 0x%x\n", (unsigned long)sync_out_trig_out, dummy_rd);
#endif

    if (ngio_ptr->id == GRAFFHAM_TESTCARD) {
        /* SYNC_OUT high test */
        *sync_out_trig_out = 0x118; /* Select multiplexor port 24 */
        dummy_rd = *sync_out_trig_out;
        sleep(2);
        *sync_out_trig_out |= 0xff; /* bit 7-0 255=1 */
        dummy_rd = *sync_out_trig_out;
#ifdef SYNC_DEBUG
        printf("\n *sync_out_trig_out = 0x%x\n", dummy_rd);
#endif

        prpass(testpass, "Host --> Graffham SYNC_OUT signal High ");
    
        result =  graff_select_test(SELECT_INTF_SYNC, SYNC_OUT, CHECK_HIGH, 50);
        if (result == FAILED) {
            ret |= result;
            cterr('f', 0,"%s(): SYNC_OUT high failed", __FUNCTION__);
        }     

        /* SYNC_OUT low test */
        prpass(testpass, "Host --> Graffham SYNC_OUT signal Low ");
        *sync_out_trig_out = 0x118; /* Select multiplexor port 24 */
        *sync_out_trig_out |= 0xFE; /* bit 7-0 254=0 */
        dummy_rd = *sync_out_trig_out;
#ifdef SYNC_DEBUG
        printf("\n *sync_out_trig_out = 0x%x\n", dummy_rd);
#endif
        sleep(2);

        result =  graff_select_test(SELECT_INTF_SYNC, SYNC_OUT, CHECK_LOW, 50);
        if (result == FAILED) {
            ret |= result;
            cterr('f', 0,"%s(): SYNC_OUT low failed", __FUNCTION__);
        }  

        /* SYNC_TRIG_OUT high test */
        prpass(testpass, "Host --> Graffham SYNC_TRIG_OUT High ");
        *sync_out_trig_out = 0x1180000; /* bit 24 trig_out OE */
        dummy_rd = *sync_out_trig_out;
        sleep(2);
        *sync_out_trig_out |= 0xff0000; /* bit 23-16 255=1 */
        dummy_rd = *sync_out_trig_out;
        sleep(2);
#ifdef SYNC_DEBUG
        printf("\n *sync_out_trig_out = 0x%x\n", dummy_rd);
#endif

        result = graff_select_test(SELECT_INTF_SYNC, SYNC_TRIG_OUT, CHECK_HIGH,50);
        if (result == FAILED) {
            ret |= result;
            cterr('f', 0,"%s(): SYNC_TRIG_OUT High failed", __FUNCTION__);
        } 

        /* SYNC_TRIG_OUT low test */
        prpass(testpass, "Host --> Graffham SYNC_TRIG_OUT Low ");
        *sync_out_trig_out = 0x1180000; /* bit 23-16 255=1 */
        dummy_rd = *sync_out_trig_out;
        *sync_out_trig_out |= 0xFE0000; /* bit 23-16 255=1 */
        dummy_rd = *sync_out_trig_out;
#ifdef SYNC_DEBUG
        printf("\n *sync_out_trig_out = 0x%x\n", dummy_rd);
#endif
        sleep(2);

        result =  graff_select_test(SELECT_INTF_SYNC, SYNC_TRIG_OUT, CHECK_LOW, 50);
        if (result == FAILED) {
            ret |= result; 
            cterr('f', 0,"%s(): SYNC_TRIG_OUT Low failed", __FUNCTION__);
        } 
    }

    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    if (ngvm_ep->ngvm_id == PFUSE123_SP2702) {
        prpass(testpass, "LSI SP2702 does not support SYNC_OUT1 signal ");
    } else {
        prpass(testpass, "Host --> Graffham SYNC_OUT1 Low ");
        *sync_out1 = 0x118; /* bit 7-0 254=0 */
        dummy_rd = *sync_out1;
        sleep(10);
        *sync_out1 |= 0xFE; /* bit 7-0 254=0 */
#ifdef SYNC_DEBUG
        printf("\n *sync_out1 = 0x%x\n", dummy_rd);
#endif

        result =  graff_select_test(SELECT_INTF_SYNC, SYNC_OUT1, CHECK_LOW, 
                                    50);
        if (result == FAILED) {
            ret |= result;
            cterr('f', 0,"%s(): SYNC_OUT1 low failed", __FUNCTION__);
        }

        /* SYNC_OUT1 high test */
        prpass(testpass, "Host --> Graffham SYNC_OUT1 High ");
        *sync_out1 = 0x118; /* bit 8 sync_out OE */
        dummy_rd = *sync_out1;
        sleep(10);
        *sync_out1 |= 0xff; /* bit 7-0 255=1 */
        dummy_rd = *sync_out1;
#ifdef SYNC_DEBUG
        printf("\n *sync_out1 = 0x%x\n", dummy_rd);
#endif
        sleep(2);

        result =  graff_select_test(SELECT_INTF_SYNC, SYNC_OUT1, CHECK_HIGH, 
                                    50);
        if (result == FAILED) {
            ret |= result;
            cterr('f', 0,"%s(): SYNC_OUT1 High failed", __FUNCTION__);
        }
    }

    prpass(testpass, "Host <-- Graffham SYNC_IN signal Low ");
    graff_select_test(SELECT_INTF_SYNC, SYNC_IN, SET_LOW, 50);
    sleep(2);
    dummy_rd = *sync_in_trig_in;

    if (dummy_rd & 0x40) /* bit 6 = sync_in */ {
        cterr('f', 0,"%s(): SYNC_IN low failed, @0x%x = 0x%x", __FUNCTION__,
              sync_in_trig_in, dummy_rd);
        ret |= FAILED;
    } 

    /* Triton platform P1A is missing the SUNC_IN high signal
     */
    if (is_triton()) {
	get_platform_bd_rev(&plat_bd_rev);
	if (plat_bd_rev < 2) {
	    printf("\n-------------------------------------\n");
	    printf("\nSkip SYNC_IN high test on Triton P1A.\n");
	    printf("\n-------------------------------------\n");
	}
    } else {
        prpass(testpass, "Host <-- Graffham SYNC_IN signal High ");
        graff_select_test(SELECT_INTF_SYNC, SYNC_IN, SET_HIGH, 50);
        sleep(2);
        dummy_rd = *sync_in_trig_in;

        if (dummy_rd & 0x40) /* bit 6 = sync_in */ {
            ret |= PASSED;
        } else {
            cterr('f', 0,"%s(): SYNC_IN high failed, @0x%x = 0x%x", __FUNCTION__,
                  sync_in_trig_in, dummy_rd);
            ret |= FAILED;
        }
    }

    if (ngvm_ep->ngvm_id == PFUSE123_SP2702) {
        prpass(testpass, "LSISP2702 does not support SYNC_TRIG_IN signal ");
    } else {
        /* SYNC_TRIG_IN low test */
        prpass(testpass, "Host <-- Graffham SYNC_TRIG_IN Low ");
        graff_select_test(SELECT_INTF_SYNC, SYNC_TRIG_IN, SET_LOW, 50);
        sleep(2);
        dummy_rd = *sync_in_trig_in;
        if (dummy_rd & 0x80) /* bit 7 = trig_in */ {
            cterr('f', 0,"%s(): SYNC_TRIG_IN low failed, @0x%x = 0x%x", 
                  __FUNCTION__, sync_in_trig_in, dummy_rd);
            ret |= FAILED;
        } 

        /* SYNC_TRIG_IN high test */
        prpass(testpass, "Host <-- Graffham SYNC_TRIG_IN High ");
        graff_select_test(SELECT_INTF_SYNC, SYNC_TRIG_IN, SET_HIGH, 50);
        sleep(2);
        dummy_rd = *sync_in_trig_in;
        if (dummy_rd & 0x80) /* bit 7 = trig_in */ {
            ret |= PASSED;
        } else {
            cterr('f', 0,"%s(): SYNC_TRIG_IN high failed, @0x%x = 0x%x", 
                  __FUNCTION__, sync_in_trig_in, dummy_rd);
            ret |= FAILED;
        }
    }
    return ret;
}

int graff_dsp_debug (int test, int wait_time)
{
    ngvm_entity_t *ep;
    dspif_mem_t   *mem_p;
    uint32_t ret_val, param1, param2;

    /* Register, buffer display for GE0, GE1 loopback, TDM Loopback,
       DDR3 memory, ECC memory */
    switch (test) {
    case SELECT_GE1_LPBK: 
        /* 		MAC0 0x30044000
           		MAC1 0x3004C000
           MAC Control and Status Registers : 0x0 - 0x60 
           MAC Transmit and Receive Counters : 0x800 - 0x818 
           MAC Receive Only Counters : 0x81C - 0x85C 
           MAC Transmit Only Counters : 0x860 - 0x8AC 
           MAC Counter Carry and Interrupt Mask Registers : 0x8B0 - 0x8C0 
         */

        /* 		PCE0 0x30040000
           		PCE1 0x30048000
           DLT<0-2047> : 0x0 - 0x1FFC
           Configuration Control Registers : 0x2000 - 0x208C
           Status Registers : 0x2200 - 0x2228
           Counter Registers : 0x2240 - 0x22D4
           Queue Register Set 0-8 : 0x2400-241C, 2420-243C, ... 24A0-24BC
           Match and Mask RAM : 0x2800-28FC
           UDL Look-Up table<0-255> : 0x2C00-2FFC
         */

        /* 		Ethernet Transmit DMA
           		Ethernet TXD0 Registers 0x30043000
           		Ethernet TXD1 Registers 0x3004B000
            0x0 - 0x10C
            0x800 - 0xA18 TXD Queue Register Set 0-8 (800-818, 840-858 ...)
         */
        param1 = 0x30044000; /* Specify Address */
        param2 = 0x60; /* Size in bytes */
        break;
    case SELECT_TDM_EXTLPBK:
        /*
          		TDM SIU
          SIU<0-5> : 0x98010000, 0x98010100 ... 0x98010500
          Registers : 0x0 - FC
         */
        /*
          		TDM SWTU
          Status and control Registers<0-5> : 0x98010800, 0x98010900 ... 0x98010D00
            Registers : 0x0 - 2C

          Source Channel Register Set<0-5> : 0x98020000, 0x98022000 ... 0x9802A000
            Register Set<0-255> : 0x0-0C, 0x10-1C, 0x20-2C ... 0xFF0-FFC

          Destination Channel Register Set<0-5> : 0x98021000, 0x98023000 ... 0x9802B000
            Register Set<0-255> : 0x0-0C, 0x10-1C, 0x20-2C ... 0xFF0-FFC
         */
        /*
          		TDM Interrupt control: 0x98011000

          Interrupt Registers for DSS0: 0x00-14
          Interrupt Registers for DSS1: 0x18-2C
          Interrupt Registers for DSS2: 0x30-44
          Interrupt Registers for DSS3: 0x48-5C
          Interrupt Registers for PPB: 0x60-80
          Port Synchronization control Registers: 0x90-94
            Registers : 0x0 - 2C
         */
        /*
          		TDM Universal Counters: 0x98011000
           Universal counter Register Set 0: 0x100-104
           Universal counter Register Set 1: 0x108-10C
           ...
           Universal counter Register Set 31: 0x1F8-0x1FC
         */
        param1 = 0x98010000;
        param2 = 0xFC;
        break;
    }
    ret_val = graff_dsp_test(SELECT_MEM_DISP, param1, param2, wait_time);
    if (ret_val == PASSED) {
        assert(ngio_ptr);

        ep = (ngvm_entity_t *)ngio_ptr->priv;
        assert(ep);

        mem_p = (dspif_mem_t *)&(ep->recv_packet.data);
        printf("\n DSP address = 0x%x, size = %d\n", mem_p->dspif_info.param1, 
               mem_p->dspif_info.param2);
        dismem((uchar *)(mem_p->pkt_data), param2, 
               (ulong)(mem_p->pkt_data), 4);
    }

    graff_cleanup_ge_env();

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: graff_get_testcommand_id
 *
 *  Description: get the index of the test command
 *
 *  Input: SELECT command id 
 *  
 *  Returns: index in teh graffham_command table
 *
 **********************************************************************
 */
int graff_get_testcommand_id (int test)
{
    int i;

    for (i = 0; i < sizeof(graffham_command); i++) {
        if (graffham_command[i].test_id == test)
            return i;
   }
   return (i);
}

/*
 **********************************************************************
 *
 *  Function: graff_dsp_test
 *
 *  Description: Send command and invoke DSP DDR2 SDRAM test on DSPs.
 *
 *  Input: slot - slot number
 *  
 *  Returns: PASSED if successful; 
 *           FAILED - tx of command failed, rx time out, test failed
 *                    (RESULT_FAILED), test of rx not the same as test 
 *                    of tx 
 *
 **********************************************************************
 */
int graff_dsp_test (int test, int param0, int param1, int wait_time)
{  
    ngvm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int i, num_dsp = 0, dsp = 0, retval;
    uint8_t errbuf[128];

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    result_packet_p = &(ep->result_packet);
    recv_packet_p = &(ep->recv_packet);

    num_dsp = ep->num_dsp;

    for (dsp = 0; dsp < num_dsp; dsp++) {
// SR ?? Please add back the check for dsp fw dnled or not
#if 0
        if (graff_dsp_download(pvdm_slot, dsp)) {
            return (FAILED);
#endif
        graff_clear_rx_buf((uchar *)result_packet_p, 
                           (int)sizeof(dspif_ether_t));
        graff_clear_rx_buf((uchar *)recv_packet_p, 
                           (int)sizeof(fe_packet_t));
        /* 
         * build command packet to run dsp sanity test 
         * test result will be handle in processing receiving packets
        */
        graff_build_command_packet(test, param0, param1, DSS_CORE0);
        /* need to know which DSP on the PVDM */
        if ((retval = graff_send_command_packet(dsp)) != PASSED) {
            cterr('f', 0,"%s(): graff_send_command_packet() returned tx "
                  "error %d for test %d\n", __FUNCTION__, retval, test);
            return (FAILED);
        }
        if (wait_time == 0) /* do not need reply */
            return (PASSED);
        /* wait for result packet */
        if (graff_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 
                                     wait_time, dsp)) {
            i = graff_get_testcommand_id(test);
            /* DSP probably not responding */
            graffham_reset_en();
            cterr('f', 0, "%s(): Timed out waiting for DSP test(0x%x):%s result."
                  " Waiting for %d secs", __FUNCTION__, test, graffham_command[i].test_name, (wait_time));
            return (FAILED);
        }
        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ether_t));
#ifdef GRAFF_DEBUG
        dismem((char *)result_packet_p, 0x40, (ulong)result_packet_p, 1);
        printf("\n result = 0x%x, expected 0x%x", 
               result_packet_p->dspif_info.result, (RESULT_SUCCESSFUL));
#endif
        /* parse result */
        if (result_packet_p->dspif_info.result != (RESULT_SUCCESSFUL)) {
            /* need to copy out errmsg and display here */
            memcpy(errbuf, result_packet_p->dspif_info.errmsg, sizeof(errbuf));
            /* Call routine to dump registers/memory for debug */
            cterr('f', 0, "%s(): Failed on dsp%d test(%d), result: 0x%x: %s",
                  __FUNCTION__, dsp, test, (result_packet_p->dspif_info.result)
                  , errbuf);
            return (result_packet_p->dspif_info.result);
        }
        /* parse select command */
        if (result_packet_p->dspif_info.select != (test)) {
            printf("\n dspif_info.select = 0x%x, expected 0x%x", 
                   result_packet_p->dspif_info.select, test);
            /* need to copy out errmsg and display here */
            memcpy(errbuf, result_packet_p->dspif_info.errmsg, sizeof(errbuf));
            cterr('f', 0, "%s(): Failed on dsp%d, test command: 0x%x: %s",
                  __FUNCTION__, dsp, (result_packet_p->dspif_info.select), 
                  errbuf);
            return (FAILED);
        }
    }
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: graff_select_test
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: test - test to run on DSP
 *         param1, param2 - parameters if any for the test
 *         wait_time - response time for the result from DSP
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_select_test (int test, int param1, int param2, int wait_time)
{
    int ret_val = PASSED;

    assert(ngio_ptr);

    /* Bringup DSP */
    if (graffham_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (graff_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = graff_dsp_test(test, param1, param2, wait_time);

    if (ret_val == RESULT_FAILED) {
        /* Please get, display register/memory dump to debug failure */
        graff_dsp_debug(test, wait_time);
        ret_val = FAILED;
    }

    graff_cleanup_ge_env();

    /* Add delay after SDRAM test for curie2ru, or subsequent EMAC test will fail */
    if (test == SELECT_DSP_SDRAM && is_curie_2ru())
        msleep(10000);

    return (ret_val);

}

/*
 **********************************************************************
 *
 *  Function: graff_tdm_ext_lpbk_test_wrapper
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int graff_tdm_ext_lpbk_test_wrapper (void)
{
    prpass(testpass, " TDM External Loopback - ");

    assert(ngio_ptr);
 
    if (ngio_ptr->mod_type == VM_MODULE) {
        printf("\r On-board Graffham NGVM does not support TDM tests ");
        return (PASSED);
    } else
        return (graff_select_test(SELECT_TDM_EXTLPBK, 0, 0, 50));
}

/*
 **********************************************************************
 *
 *  Function: ecc_mem_test 
 *
 *  Description: wrapper to run the ECC memory test
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int ecc_mem_test (void)
{
    prpass(testpass, " ECC Memory - ");

    assert(ngio_ptr);

    return (graff_select_test(SELECT_ECC_MEM, 0, 0, 100));
}

#ifdef TDM_INT_LPBK
/*  
 **********************************************************************
 *  
 *  Function: graff_tdm_int_lpbk_test_wrapper
 *  
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_tdm_int_lpbk_test_wrapper (void)
{
    prpass(testpass, "TDM Internal Loopback test");
    return (graff_select_test(SELECT_TDM_INTLPBK, 0, 0, 50));
}
#endif

/*  
 **********************************************************************
 *  
 *  Function: graff_arm11_cpu1_boot_test_wrapper
 *  
 *  Description: wrapper to test the CPU1 for boot up
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_arm11_cpu1_boot_test_wrapper (void)
{
    prpass(testpass, " ARM11 CPU1 Boot - ");
    return (graff_select_test(SELECT_ARM11CPU1_BOOT, 0, 0, 50));
}

/*  
 **********************************************************************
 *  
 *  Function: graff_dss_core0_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_dss_core0_sanity_test_wrapper (void)
{
    prpass(testpass, " DSS Core0 Sanity - ");
    return (graff_select_test(SELECT_DSS0_SANITY, 0, 0, 50));
}

/*  
 **********************************************************************
 *  
 *  Function: graff_dss_core1_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
static int graff_dss_core1_sanity_test_wrapper (void)
{   
    prpass(testpass, " DSS Core1 Sanity - ");
    return (graff_select_test(SELECT_DSS1_SANITY, 0, 0, 50));
}   

/*  
 **********************************************************************
 *  
 *  Function: graff_dss_core2_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
static int graff_dss_core2_sanity_test_wrapper (void)
{   
    ngvm_entity_t *ngvm_ep;

    assert(ngio_ptr);
    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    prpass(testpass, " DSS Core2 Sanity - ");
    if (ngvm_ep->ngvm_id == PFUSE123_SP2702) {
        printf("\r LSI SP2702 does not support DSS2");
        return (PASSED);
    }
    return (graff_select_test(SELECT_DSS2_SANITY, 0, 0, 50));
}   

/*  
 **********************************************************************
 *  
 *  Function: graff_dss_core3_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
static int graff_dss_core3_sanity_test_wrapper (void)
{   
    ngvm_entity_t *ngvm_ep;

    assert(ngio_ptr);
    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    prpass(testpass, " DSS Core3 Sanity - ");
    if (ngvm_ep->ngvm_id == PFUSE123_SP2702) {
        printf("\r LSI SP2702 does not support DSS3");
        return (PASSED);
    }

    return (graff_select_test(SELECT_DSS3_SANITY, 0, 0, 50));
}   

/*  
 **********************************************************************
 *  
 *  Function: graff_ge_intlpbk_test_wrapper
 *  
 *  Description: wrapper so we can set up and clear up ge.
 *  
 *  Input: slot - slot number
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_ge_intlpbk_test_wrapper (int port)
{   
    if ((dsp_tests_use_enet == 1)) {
        prpass(testpass, " Internal GE%d loopback only for uart mode ", port);
        return (PASSED);
    }
    prpass(testpass, " EMAC%d Internal Loopback test", port);
    if (port == 1)
        return (graff_select_test(SELECT_GE1_LPBK_PT, 0, 0, 300));
    else
        printf("\n %s(): Unknown GE port %d\n", __FUNCTION__, port);
    return (FAILED);
}  

/*
 **********************************************************************
 *
 *  Function: graff_ge_lpbk_test_wrapper
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int graff_ge_lpbk_test_wrapper(int port)
{
    int ret_val = PASSED;
 
    assert(ngio_ptr);

    if ((port == 1) && (ngio_ptr->mod_type != VM_MODULE)) {
        printf("\rGraffham in NGWIC DC slot does not support GE%d Ext loopback"
               " test", port);
        return (PASSED);
    } 

    if (graff_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    if (graff_ge_lpbk_test(port)) {
        ret_val = FAILED;
    }
    graff_cleanup_ge_env();

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: ngvm_utils_test
 *
 *  Description: NGVM utilities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int ngvm_utils_test (void)
{
    ngvm_entity_t *ngvm_ep;

    assert(ngio_ptr);
    ngvm_ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ngvm_ep);

    sprintf(graffutiltitle, "%s Utilities Menu", ngvm_ep->slot_type_str);
    build_primary_submenu(ngvm_utils_submenu_table,
                          NGVM_UTILS_SUBMENU_TABLE_SZ,
                          graffutiltitle, &ngvm_util_submenup);

    build_secondary_submenu(ngvm_utils_submenu_table,
                            NGVM_UTILS_SUBMENU_TABLE_SZ,
                            ngvm_utils_secondary_items);

    menu(ngvm_util_submenup, ngvm_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: graff_iface_test
 *
 *  Description: Run interface test 
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *           FAILED - otherwise 
 *
 **********************************************************************
 */
int graff_iface_test (void)
{
    int retval = PASSED;

    retval = gpio_exp_test();

    if (retval == PASSED) {
        retval = graff_intf_sync_test();
    }

    if (retval == PASSED) {
        retval = graff_ge_lpbk_test_wrapper(1);
    }

    if (retval == PASSED) {
        retval = graff_tdm_ext_lpbk_test_wrapper();
    }

    if (retval == PASSED) {
        retval = graff_uart_test();
    }

    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: graffham_test
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *           FAILED - DSP ready pin not set, the test failed
 *
 **********************************************************************
 */
static int graffham_test (int run_tests)
{
    int retval = PASSED;

    /* Bringup DSP */
    if (graffham_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (ngio_ptr->test_type == IFACE_TEST)
        return(graff_iface_test());

    if (dsp_tests_use_enet == 1) {
        if (run_tests == 1) {
            retval = graffham_run_test();
        } else {
            build_primary_submenu(graff_tests_submenu_table,
                                  GRAFF_TESTS_SUBMENU_TABLE_SIZE,
                                  graffsubmenutitle, &graff_submenup);
            build_secondary_submenu(graff_tests_submenu_table,
                                    GRAFF_TESTS_SUBMENU_TABLE_SIZE,
                                    graff_tests_secondary_items);

            menu(&graff_subtest_menu, graff_tests_secondary_items, '\0');
        }
    } else {
    /* SR please set the diag_flag details for the DSP to know
       how to set up the menu and which tests to run */ 
        graffham_con_test();
    }
    return retval;
}

/*
 **********************************************************************
 *
 *  Function: ppb_con
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *           FAILED, otterwise
 *
 **********************************************************************
 */
static int ppb_con (void)
{
    int retval = PASSED;

    prpass(testpass, "SP2700 Console");
    /* If DSP diags firmware is downloaded then can send ethernet command
       packet else just goto picocom directly SR add this code later */
    printf("\n Please enter <ESC> to exit DSP console prompt.");
    retval =  graff_select_test(SELECT_DSP_CONSOLE, 0, 0, 0);
    graffham_con_test();
    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: graffham_con_test
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *
 **********************************************************************
 */
static int graffham_con_test (void)
{
    ngvm_entity_t *ep;
    char disp[128]; 

    assert(ngio_ptr);

    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Connect to the LSI SP27XX console using uart interface. 
     * LSI SP27XX is out of resest and its bootloader has downloaded the 
     * application firmware and diags menu is up. 
     */
    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);

    sprintf(disp, "picocom -b 9600 -d8 -pn -fn %s", ep->tty_dev);
    printf("\n %s\n\n\n", disp);
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(disp);
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: graff_wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets
 *
 *  Input: pak - received packet buffer
 *         mode - POLL_MODE or INTR_MODE
 *
 *  Returns: PASSED if successful; 
 *           FAILED - If no pkt recvd, rx errors, or MAC of recvd pkt 
 *                    does not match expected
 *
 **********************************************************************
 */
int graff_wait_for_ge_packet (uchar *pak, int socket_gl, int mode, int dsp,
                              int disp_err)
{
    ngvm_entity_t *ep;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int wait_count = 1000;
    int status; 
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    assert(ngio_ptr);

    /* Need to store the MAC address of the NGVM and host */
    ep = (ngvm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* clear buffer before use */
    memset((uchar *)pak, 0, sizeof(fe_packet_t));
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count;
    rx_pkt_p->socket = socket_gl;
    rx_pkt_p->rx_chk = 1;
    /* now wait for rx from GEMAC attached to backplane GE switch 
     * return can be ETH_NO_PKT_RX (0x4), ETH_PKT_RX_ERR (0x1)
     * or ETH_PKT_RX_OK (0x0). Error message for ETH_PKT_RX_ERR will
     * be printed by the source function.
     */
    status = eth_pkt_rx(rx_pkt_p);
    //printf("\b");
    if (status) {
        switch (disp_err%8) {
        case 0:
            printf("\r|");
            break;
        case 1:
            printf("\r/");
            break;
        case 2:
            printf("\r-");
            break;
        case 3:
            printf("\r\\");
            break;
        case 4:
            printf("\r|");
            break;
        case 5:
            printf("\r/");
            break;
        case 6:
            printf("\r-");
            break;
        case 7:
            printf("\r\\");
            break;
        default:
            break;
        }
        printf("\r ");
        /* disp_err will always be a non-zero value */ {
        if (disp_err == 1) 
            printf("\n %s(): eth_pkt_rx() returned error (no rx or rx errors) "
                   "%d\n", __FUNCTION__, status);
        }
        return (FAILED); /* retry is provided by caller */
    }
    //printf("\n Received Packet verify the source/dest MAC");
    //dismem((uchar *)recv_buffer, 0x80, (ulong)recv_buffer, 1);
    /* Make sure we received packet from the expected destination */
    if ((chk_macaddr(recv_buffer, (uchar *)&(ep->eth_hdr[dsp].src_addr[0])) != 0) ||
        (chk_macaddr(&(recv_buffer[6]), (uchar *)&(ep->eth_hdr[dsp].dest_addr[0])) != 0))
    {
        //if (disp_err == 1) /* This will always be a non-zero value */ {
            printf("\n Received MAC does not match MAC in cookie. Try again");
            printf("\n Expected MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
                   ep->eth_hdr[dsp].dest_addr[0], ep->eth_hdr[dsp].dest_addr[1],
                   ep->eth_hdr[dsp].dest_addr[2], ep->eth_hdr[dsp].dest_addr[3],
                   ep->eth_hdr[dsp].dest_addr[4], 
                   ep->eth_hdr[dsp].dest_addr[5]);
            printf("\n Received MAC : ");
            printf("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",  recv_buffer[6],
                   recv_buffer[7],  recv_buffer[8],
                   recv_buffer[9],  recv_buffer[10],
                   recv_buffer[11]);
        //}
        return (FAILED);
    }
    //printf("\n Received Ready Response from Graffam.");

    /* copy received to user pak */
    memcpy ((char *)pak, (uchar *)recv_buffer, sizeof(fe_packet_t));

    return (PASSED);
}

/******** History ********
$Log: ngvm_graffham.c,v $
Revision 1.45  2021/01/07 06:23:26  jiajliu
CSCvu31200-6: Fix Graffham segmentation fault on overlord and utah

Revision 1.44  2020/12/22 14:30:19  jiajliu
CSCvu31200-5: Switer Carrier: Add support for Grimlock and DC Graffham

Revision 1.43  2019/12/30 05:59:18  kehuang2
CSCvs55860: Support Gaffham

Revision 1.42  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.41  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.40  2017/02/07 03:05:24  alpeng
add packet type on packet format for graffham

Revision 1.39.28.6  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.39.28.5  2017/10/06 22:44:09  ptong
Skip SYNC_IN high signal test on Triton P1A

Revision 1.39.28.4  2017/04/19 01:04:36  alpeng
skip SYNC_IN high which is HW issue

Revision 1.39.28.3  2017/04/05 06:40:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.39.28.2  2017/02/10 06:54:23  alpeng
add fix for graffham pkt_type to branch

Revision 1.39.28.1  2016/12/15 05:34:21  alpeng
update graffham get uart ctrl function to get uart_ctrl from host api

Revision 1.39  2014/11/26 07:00:42  alpeng
Support NGSM+NGWIC+NGVM case

Revision 1.38  2014/11/26 04:11:06  alpeng
reverting to version 1.36

Revision 1.36  2014/06/10 23:40:03  mcharon
remove redundant call to close()

Revision 1.35  2014/05/03 14:52:48  mcharon
use IFNAMSIZE; cache uio dir name in uio_utils

Revision 1.34  2014/02/22 05:06:07  mcharon
add uart test that bypassse tty driver

Revision 1.33  2014/01/27 21:36:00  mcharon
break uart_lpbk_test into 3 separate functions: init/send/receive

Revision 1.32  2014/01/09 11:07:03  danchung
Correct the FPGA version number to be checked before running ngvm
sync signal test for USD

Revision 1.31  2013/12/18 06:32:46  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.30  2013/12/16 02:06:56  hroni
do uart test using uart_lpbk_test()

Revision 1.29  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.28  2013/11/11 21:18:39  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.27  2013/10/07 21:17:05  ptong
Use get_host_mac_addr() and get_sgmii_port_num() api to replace hardcoded CPU_SGMII_PORT1 value to support Utah platform

Revision 1.26  2013/03/13 08:42:57  srane
Reduce the time out for DSP test result packet. Add test name in debug
printf.

Revision 1.25  2013/02/28 00:36:06  srane
Add support for NGVM testcard.

Revision 1.24  2013/02/15 10:31:47  palin2
Update UART test by add a new parameter to allow using specific baud rate.

Revision 1.23  2013/02/07 22:51:05  srane
DF site needs DC interface test. Add tdm and gpio expander to the iface test.

Revision 1.22  2013/02/01 19:47:36  srane
Add back the memory test.

Revision 1.21  2013/01/29 02:05:16  srane
Add check for LSI chipset to decide the SYNC gpio pins available for tests.
Clean up the printfs.

Revision 1.20  2012/12/24 10:00:11  srane
P2 Graffham build added new design to be able to test SYNC signals, also
needs 2.5 and up O2 firmware. Add few uitls - show ver, mac address.

Revision 1.19  2012/11/28 00:54:19  srane
Display rx/tx packet data for comparison.

Revision 1.18  2012/10/24 21:32:19  srane
Remove PID check for the onboard slot (new PIDs released for REV 2).
Add picocom console to the util menu.

Revision 1.17  2012/10/04 19:58:21  srane
Add GE 1 loopback test to the Graffham onboard interface test.

Revision 1.16  2012/10/04 19:15:11  srane
Add support for SP2702 (CSCuc51339), change the PID string search - new
PID will be finalized shortly.

Revision 1.15  2012/10/02 23:15:52  srane
Execute uart loopback test at the Graffham DSP firmware level.
Boot loader intermittently hangs CSCuc07162.

Revision 1.14  2012/09/24 01:10:46  srane
Add interface test, util to display dsp registers/memory,
change dsp firmware name.

Revision 1.13  2012/09/10 05:56:21  srane
Add test for ARM11 CPU1, add dsp memory display pkt infrastructure,
secure bootloader code breaks uart loopback test - add fix, general
cleanup.

Revision 1.12  2012/08/28 18:47:31  srane
Add defines for voltage margin tests.

Revision 1.11  2012/08/22 17:41:29  mcharon
add one more argument to uart_intf_test

Revision 1.10  2012/08/15 16:13:06  srane
Add support for ETH1 Loopback.

Revision 1.9  2012/07/25 00:04:59  srane
check the PID of the module. Keep NGVM in reset after tests.

Revision 1.8  2012/07/17 20:15:29  srane
Add DAC voltage margin support, use ethernet for sending DSP commands,
DSP firmware version check.

Revision 1.7  2012/06/28 21:38:29  srane
Add code to support the DSP READY message exchange, new boot loader filename
change etc.

Revision 1.6  2012/06/09 02:16:29  srane
cleanup console redirect code.

Revision 1.5  2012/06/07 23:16:12  srane
Remove TDM internal loopback test

Revision 1.4  2012/06/07 22:48:28  srane
Add card info packet exchange, use TFTPDIR env, TDM external loopback, ECC
memory test.

Revision 1.3  2012/06/02 00:57:11  srane
Fix warnings.

Revision 1.2  2012/05/24 23:27:20  srane
Add support for both ethernet and uart moder, GPIO expander, uart tests.
General cleanup

Revision 1.1  2012/05/16 07:26:38  srane
Initial commit for Graffham NGVM.


$Endlog$
*/


