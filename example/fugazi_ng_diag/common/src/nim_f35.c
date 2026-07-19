/* $Id: nim_f35.c,v 1.13 2019/11/25 08:55:49 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nim_f35.c,v $
 *******************************************************************************
 *
 * nim_f35.c - F35 code
 *
 * Jan 2014, Smita Rane
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <string.h>
#include <sys/wait.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "linux_api.h"
#include "router_if.h"
#include "platform_cookie.h"
#ifdef TACHI
#include "diag_lewis_gesw_test.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_utils.h"
#include "diag_console_util.h"
#include "diag_fpga_lib.h"
#include <linux/filter.h>  /* pkt filter */
#include <arpa/inet.h>
#elif TABEIL
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_pkt_txrx_utils.h"
#include "dnv_eth_lib.h"
#else
#include "platform_eth_pkt_txrx.h"
#endif
#include "cross_platform.h"
#include <nim_f35.h>
#include <stdlib.h>
#include "dash_fpga.h" 

#ifdef TACHI
#define MAC_FILTER_SIZE       (6)
#endif 

static int gpio_ltc_test(void);

//static f35_ds_t f35_ds[3];
static struct ngio_intf_t *f35_iface;
static void (*f35_saved_diag_exec)(void) = NULL;

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t f35_mod_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)f35_console_switch,0,   0,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"Power off F35 NGWIC",     (PFT)f35_power_off,   0,   0,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
};

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t f35_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)f35_console_switch, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off F35 NGWIC",     (PFT)f35_power_off,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on F35 NGWIC",      (PFT)f35_pwr_on,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle F35 NGWIC",   (PFT)f35_pwr_cycle,   0,   MM_2,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Enable Backplane GE loopback",  (PFT)enable_bp_ge_lpbk,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Disable Backplane GE loopback", (PFT)disable_bp_ge_lpbk,    0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)f35_uart_test,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Test",         (PFT)ltc4215_register_test, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Read",         (PFT)ltc4215_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Write",        (PFT)ltc4215_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"F35 Power Stat",        (PFT)f35_power_stat,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 Register Read",         (PFT)pca9557_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 Register Write",        (PFT)pca9557_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Escape to Shell (debugging only)", (PFT)f35_o2_shell,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)f35_o2_command,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},   
};

#define F35_UTILS_SUBMENU_TABLE_SZ (sizeof(f35_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t f35_utils_primary_items[F35_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t f35_utils_secondary_items[F35_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

char f35utiltitle[50];
menuinfo_t f35_util_submenu = {
    f35utiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    f35_utils_primary_items,
};

menuinfo_t *f35_util_submenup = &f35_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t f35_mainmenu_tbl[] = {
    {"F35 Interface Utilities",  (PFT)f35_utils,          0,  0,
     (type_t(*)())0, 0, (type_t(*)())f35_utils, 0},
    {"GPIO Expander/LTC4215 test",       (PFT)gpio_ltc_test,      0,  MM_2,
     (type_t(*)())0, 0, (type_t(*)())0,         0},
    {"F35 NIM test",             (PFT)f35_console_switch, 0,  MM_2,
     (type_t(*)())0, 0,	(PFT)f35_console_switch,          1},
};

#define F35_MAINMENU_TBL_SIZE (sizeof(f35_mainmenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[F35_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[F35_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
 
static char nimsubmenutitle[40];

static struct menuinfo maindiag = {
    nimsubmenutitle,	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;
  
ngio_priv_t ngio_priv_ds[MAX_WIC+1];  /* 3 for Overlord */
ngio_priv_t *ngio_priv_ds_p;  /* 3 for Overlord */

/* Variable to select to run tests on host using ethernet interface or run 
 * tests on the DSP using uart interface */
int f35_tests_use_enet = 1; /* continuos mode, run all tests on module */

/***********************************************************************
 * Name: cleanup_ge_env (common)
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: slot : NGIO slot
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int cleanup_ge_env (int slot)
{
    ngio_priv_t *ep;
    int status = PASSED;
    int sgmii_port = 0;
    char if_name[10];
    
    ep = &(ngio_priv_ds[slot]);
    assert(ep); 

    if (ep->ge_setup_flag == TRUE) {
        ep->ge_setup_flag = FALSE;

    sgmii_port = get_sgmii_port_num(slot, TYPE_SWITCH);
#ifdef TACHI
        sprintf(if_name, ETH1_MAC1);
#elif TABEIL
        sprintf(if_name, "%s", TABEI_ETH_BP);
#else
        sprintf(if_name, "eth%d", sgmii_port);
#endif 
        status = cleanup_eth_dev(if_name, ep->socket_gl);

        if (status) {
            cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
            return (FAILED);
        }
        close(ep->socket_gl);
        return (PASSED);
    }
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets (fe_packet_t)
 *               Copy the received packet into ngio_priv_t struct
 *               fields rx_pak and recv_packet
 *
 *  Input: slot - which slot to use for receive
 *         count - Number of retry times to receive the packet
 *                 = 1 ->display rx error if no ppkt received. 
 *         disp - display spinning wheel or not
 *
 *  Returns: PASSED if successful; 
 *           FAILED - If no pkt recvd, rx errors, or MAC of recvd pkt 
 *                    does not match expected
 *
 **********************************************************************
 */
int wait_for_ge_packet (int slot, int count, int disp)
{
    ngio_priv_t *ep;
    
    eth_rx_pkt_t * rx_pkt_p;
    //int wait_count = 1000;
    int wait_count = 600000;
    int status;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    /* Need to store the MAC address of the NGVM and host */
    ep = &ngio_priv_ds[slot];
    rx_pkt_p = (eth_rx_pkt_t *)ep->rx_pak;

    /* clear buffer before use */
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count;
    rx_pkt_p->socket = ep->socket_gl;
    rx_pkt_p->rx_chk = 1;
    /* now wait for rx from GEMAC attached to backplane GE switch 
     * return can be ETH_NO_PKT_RX (0x4), ETH_PKT_RX_ERR (0x1)
     * or ETH_PKT_RX_OK (0x0). Error message for ETH_PKT_RX_ERR will
     * be printed by the source function.
     */
    status = eth_pkt_rx(rx_pkt_p);
    //printf("\b");
    if (status) {
        if (disp) {
        switch (count%8) {
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
    }
        printf("\r ");
        /* count will always be a non-zero value */
        if (count == 1) {
            printf("\n HOST: %s(): eth_pkt_rx() returned error (no rx or rx errors) "
                   "%d\n", __FUNCTION__, status);
        }
        return (FAILED); /* retry is provided by caller */
    } 
    
    //printf("\n Received Packet verify the source/dest MAC");
    //dismem((uchar *)recv_buffer, 0x80, (ulong)recv_buffer, 1);
    /* Make sure we received packet from the expected destination */
    if ((chk_macaddr(recv_buffer, (uchar *)&(ep->eth_hdr.src_addr[0])) != 0) ||
        (chk_macaddr(&(recv_buffer[6]), (uchar *)&(ep->eth_hdr.dest_addr[0])) 
        != 0))
    {
        //if (disp_err == 1) /* This will always be a non-zero value */ {
            printf("\n HOST: Received MAC does not match MAC in cookie. Try again");
            printf("\n HOST: Expected MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                   ep->eth_hdr.dest_addr[0], ep->eth_hdr.dest_addr[1],
                   ep->eth_hdr.dest_addr[2], ep->eth_hdr.dest_addr[3],
                   ep->eth_hdr.dest_addr[4],
                   ep->eth_hdr.dest_addr[5]);
            printf("\n HOST: Received MAC : ");
            printf("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",  recv_buffer[6],
                   recv_buffer[7],  recv_buffer[8],
                   recv_buffer[9],  recv_buffer[10],
                   recv_buffer[11]);
        //}
        return (FAILED);
    }
    //printf("\n Received Ready Response from Graffam.");

    /* copy received to user pak */
    memcpy ((char *)&(ep->recv_packet), (uchar *)recv_buffer, 
            sizeof(fe_packet_t));
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: wait_result_packet
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
int wait_result_packet (int slot, uint16_t retry, int disp)
{
    int ret_val = PASSED;
    int count = retry;

    if (count < 2) {  /* warning programmer only */
        printf("HOST: retry count too small, not recommend!!!\n");
        return (FAILED);
    }

    /* Wait for Gige packet */
    while (count) {
        if (!(wait_for_ge_packet(slot, count, disp)))
        {
            break;
        }
        count -= 1;
        msleep(1000);
    }

    if (count == 0) {
        ret_val = FAILED;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nWait for result = %d\n",count);
        fflush(stdout);
    }
    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: send_tx_pkt (common)
 *
 *  Description: Fill the ethernet tx packet header and call tx
 *
 *  Input: None
 *
 *  Returns: PASSED
 *           FAILED - some tx error
 *
 **********************************************************************
 */
int send_tx_pkt (int slot, int size)
{
    ngio_priv_t *ep;
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    int ret_val = PASSED;

    ep = &ngio_priv_ds[slot];

    assert(f35_iface);

    ep = (ngio_priv_t *)f35_iface->priv;

    //get_local_mac_addr(CPU_SGMII_PORT1, (uchar *)&src_mac_addr[0]);
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    /* Write to routine to read the MAC address fromt the cookie from the 
     * NGVM. Till then use the bcast address.
     */
    printf("\n F35 module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
           ep->eth_hdr.dest_addr[0], ep->eth_hdr.dest_addr[1],
           ep->eth_hdr.dest_addr[2], ep->eth_hdr.dest_addr[3],
           ep->eth_hdr.dest_addr[4], ep->eth_hdr.dest_addr[5]);
    memcpy((uchar *)(tx_pkt_p->dest_addr),
           (uchar *)&(ep->eth_hdr.dest_addr),
           sizeof(mac_addr_t));
    //memcpy((uchar *)(tx_pkt_p->dest_addr), bcast_mac_addr, sizeof(mac_addr_t));
    dismem((uchar *)(tx_pkt_p->dest_addr), sizeof(mac_addr_t), 
           (ulong) (tx_pkt_p->dest_addr), 4);
    memcpy((uchar *)(tx_pkt_p->src_addr),
           (uchar *)&(ep->eth_hdr.src_addr),
           sizeof(mac_addr_t)); /* host MAC */
    //memset((uchar *)(tx_pkt_p->src_addr), 0, sizeof(mac_addr_t)); /* host MAC */
    tx_pkt_p->pkt_type = PKT_TYPE_IPV4;
    tx_pkt_p->payload_size = size;
    tx_pkt_p->bufr_st_addr = (uchar *)&(ep->tx_pak);
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = ep->socket_gl;

#if 0
    dismem((uchar *)(tx_pkt_p->src_addr), sizeof(mac_addr_t), 
           (ulong) (tx_pkt_p->src_addr), 4);
    printf("\n Packet to Send \n");
    printf("\n pkt_type = 0x%x", tx_pkt_p->pkt_type);
    printf("\n payload_size = 0x%x", tx_pkt_p->payload_size);
    printf("\n socket = 0x%x", tx_pkt_p->socket);
    printf("\n sending the packet\n");
    fflush(stdout);
#endif
    ret_val = eth_pkt_tx(tx_pkt_p);

    return (ret_val);
}

#if 0
/*
 **********************************************************************
 *  
 *  Function: f35_check_kernel_up
 *
 *  Description: Unreset F35, dnld firmware and check if ready pin set.
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
static int f35_check_kernel_up (void)
{
    n2g_i2c_if_t  *pca;
    f35_ds_t *ep;
    int i2c_dev, i;
    uchar *rx_pak, data = 0;

    /* For ethernet mode loopback is never enabled */
    if (f35_tests_use_enet == 0) {  /* for uart mode */
        printf("\n Make sure GE Loopback mode is disabled.");
        disable_bp_ge_lpbk(0);
        disable_bp_ge_lpbk(1);
    }

    ep = (f35_ds_t *)f35_iface->priv;
    assert(ep);

    if (ep->diag_kernel_flag == ON) {
        //printf(" DSP firmware is downloaded and DSP is booted up");
        return (PASSED);
    }
    /* reset PVDM */
    ep->reset_ngvm();

    rx_pak = ep->rx_pak;
    printf("\r Please wait for the Graffham Bootloader to dhcp the "
           "firmware");
    fflush(stdout);

    pca = (n2g_i2c_if_t *)f35_iface->pca;
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
    /* SR Please check if this is necessary */
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

    if (f35_tests_use_enet == 0) {
        printf("\n Enabling GE loopback mode ");
        enable_bp_ge_lpbk(0);
    }
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: f35_build_pkt
 *
 *  Description: Prepare the ethernet pkt with test data
 *
 *  Input: pkt_type - 
 *         test_cmd - which test to run on f35 module 
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int f35_build_cmd_pkt (int test_cmd, int indx)
{
    ipc_msg_t ipc_msg;

    ipc_msg->msg_id = f35_tests_tbl[indx].msg_id;
    ipc_msg->cmd_id = f35_tests_tbl[indx].cmd_id;
    ipc_msg->msg_len = f35_tests_tbl[indx].msg_len;
    ipc_msg->cmd_rslt = NO_RSLT;

    switch (test_cmd) {
    case f35_INT_REG_TEST:
    case f35_USB_TEST:
    case F35_SPI_FLASH_TEST:
    case F35_PHY_ADDR_INFO:         /* Done cmdline */
        break;
    case F35_MODEM_ENUM_TEST:
        /* Make sure modem is ENABLED or modem reset is disable */
        tesla_kw_modem_reset(iface, DISABLE_CHK);
        /* Make sure modem power is on */
        tesla_host_fpga_modem_pwr_on(iface);
        /* Give time for modem to come up in linux */
        msleep(300);
        break;
    case AT_COMMAND_TEST:   /* Done cmdline */
        cmd = (uchar *)(ipc_msg->msg_data);
        printf("\nEnter the AT command> ");
        (void) get_line(cmd, 64);
        len = strlen(cmd);
        cmd[len++] = '\n';
        cmd[len++] = '\0';
        printf("\nSending AT command (len=%d): \n", strlen(cmd));
        printf("\nSending AT command %s\n", cmd);
        ipc_msg->msg_len = strlen(cmd);
        break;
    case DRMW_FPGA_REG_ALT:
        size = sizeof(tesla_kw_fpga_regs_t)/sizeof(uint32_t);
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        prepare_ipc_msg_reg_alt_test(DRMW_FPGA_REG_ALT,
                                     (reg_alt_test_t *)(ipc_msg->msg_data));
        break;
    case KW_INT_REG_ALT:          /* Done IPC_CMD */
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        prepare_ipc_msg_reg_alt_test(KW_INT_REG_ALT,
                                     (reg_alt_test_t *)(ipc_msg->msg_data));
        break;
    case KW_INT_REG_DISP:         /* Done IPC_CMD */
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        prepare_ipc_msg_reg_alt_test(KW_INT_REG_DISP,
                                     (reg_alt_test_t *)(ipc_msg->msg_data));
        break;
    case ALTERA_PCI_REG_ALT:
    case ALT_PCI_REG_CON_ALT:   /* Done cmdline */
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        ipc_msg->msg_id = IPC_CMD; /* To execute the console command */
        prepare_ipc_msg_reg_alt_test(ALTERA_PCI_REG_ALT,
                                     (reg_alt_test_t *)(ipc_msg->msg_data));
        break;
    case DRMW_FPGA_REG_CON_ALT:
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        ipc_msg->msg_id = IPC_CMD; /* To execute the console command */
        prepare_ipc_msg_reg_alt_test(DRMW_FPGA_REG_CON_ALT,
                                     (reg_alt_test_t *)(ipc_msg->msg_data));
        break;
    case READ_SPI_PROM:
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        ipc_msg->msg_id = IPC_CMD; /* To execute the console command */
        reg_alt->offset = gethex_answer("\nEnter in hex SPI offset", 0,
                           0, 0xFFFF);
        reg_alt->op = 1;
        break;
    case WRITE_SPI_PROM:
        reg_alt = (reg_alt_test_t *)(ipc_msg->msg_data);
        ipc_msg->msg_id = IPC_CMD; /* To execute the console command */
        reg_alt->offset = gethex_answer("\nEnter in hex SPI offset", 0,
                           0, 0xFFFF);
        reg_alt->data = gethex_answer("Enter in hex the data to write ", 0,
                                      0, 0xFFFFFFFF);
        printf("\n data to write = 0x%x\n", reg_alt->data);
        break;
    }
}

/*
 **********************************************************************
 *
 *  Function: f35_select_test
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
static int f35_run_test (uint test_cmd, int wait_time)
{
    int      indx, ret_val = PASSED;
    uchar    print_str[IPC_MSG_STR], *cmd;

    indx = find_index_f35_test_tbl(test_cmd);
    if (indx == -1) {
        printf("\n%s() : Cannot find test cmd %d in f35_tests_tbl",
               __FUNCTION__, ipc_cmd);
        return (FAILED);
    }

    sprintf(print_str, "%s", f35_tests_tbl[indx].desc);
    testname_disp(print_str);

    /* Check if F35 Kernel code is up and running */
    if (f35_check_kernel_up() == FALSE) {
        printf("\n %s() : F35 Kernel is not up", __FUNCTION__);
        cterr('f', 0, "%s(): \n F35 READY PIN not set. Test : Please check "
              "if \"/firmware/f35_fw_diag.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (setup_ge_env(slot) == FAILED) {
        return (FAILED);
    }

    ret_val = f35_build_cmd_pkt(test_cmd, indx);

    if ((retval = f35_send_tx_pkt(slot, size)) != PASSED) {
            cterr('f', 0,"%s(): f35_send_command_packet() returned tx "
                  "error %d for test %d\n", __FUNCTION__, retval, test);
            return (FAILED);
        }
        if (wait_time == 0) /* do not need reply */
            return (PASSED);
        /* wait for result packet */
        if (wait_result_packet(slot, 15, 1)) {
            i = graff_get_testcommand_id(test);
            /* DSP probably not responding */
            graffham_reset_en();
            cterr('f', 0, "%s(): Timed out waiting for DSP test(0x%x):%s result."
                  " Waiting for %d secs", __FUNCTION__, test, graffham_command[i].test_name, (wait_time));
            return (FAILED);
        }
        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ether_t));
//#ifdef GRAFF_DEBUG
        dismem((char *)result_packet_p, 0x40, (ulong)result_packet_p, 1);
        printf("\n result = 0x%x, expected 0x%x",
               result_packet_p->dspif_info.result, (RESULT_SUCCESSFUL));
//#endif


    if (ret_val == RESULT_FAILED) {
        /* Please get, display register/memory dump to debug failure */
        graff_dsp_debug(test, wait_time);
        ret_val = FAILED;
    }

    cleanup_ge_env(slot);

    return (ret_val); 

}
#endif

/*
 **********************************************************************
 *
 *  Function: f35_utils
 *
 *  Description: F35 Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int f35_utils (void)
{

    assert(f35_iface);
    
    sprintf(f35utiltitle, "F35 NIM%d Host Interface Utilities Menu", 
            f35_iface->slot);
    build_primary_submenu(f35_utils_submenu_table,
                          F35_UTILS_SUBMENU_TABLE_SZ,
                          f35utiltitle, &f35_util_submenup);

    build_secondary_submenu(f35_utils_submenu_table,
                            F35_UTILS_SUBMENU_TABLE_SZ,
                            f35_utils_secondary_items);

    menu(f35_util_submenup, f35_utils_secondary_items, '\0');

    return (PASSED);
}

#if 0
/*
 **********************************************************************
 *
 *  Function: f35_mod_utils
 *
 *  Description: F35 Module Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *  
 **********************************************************************
 */ 
static int f35_mod_utils (void)
{   
    assert(f35_iface);

    sprintf(f35modutiltitle, "F35 Slot %d Module Utilities Menu",
            f35_iface->slot);
    build_primary_submenu(f35_mod_utils_submenu_table,
                          F35_MOD_UTILS_SUBMENU_TABLE_SZ,
                          f35modutiltitle, &f35_mod_util_submenup);

    build_secondary_submenu(f35_mod_utils_submenu_table,
                            F35_MOD_UTILS_SUBMENU_TABLE_SZ,
                            f35_mod_utils_secondary_items);

    menu(f35_mod_util_submenup, f35_mod_utils_secondary_items, '\0');

    return (PASSED);
}   
#endif


/*
 **********************************************************************
 * Function: f35_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void f35_cleanup (void)
{
    assert(f35_iface);

    if (f35_saved_diag_exec) {
        pre_diag_exec = f35_saved_diag_exec;
        f35_saved_diag_exec = NULL;
    }
}

/*
 *************************************************************************
 * Function: f35_uart_test
 *
 * Test the UART connection from the host to F35.
 * Also test the GE0 interface by checking diag image download successful or not.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int f35_uart_test (void)
{
    const int maxlen = 28;
    char test_if[maxlen];;
    int rv;

    /* 'n\n' for trigger F35 side diag sub-item,
     * which will invoke 'uname -a'.
     */
    assert(f35_iface);

#ifdef TACHI
    diag_uart_to_nim_cnnt(f35_iface->slot); 
    snprintf(test_if, maxlen-1, UART_TTYS2_DEV);
#else 
    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d", f35_iface->uart_ctrl); 
#endif

    prpass(testpass, "F35 UART ");

    rv = uart_msg_exh_test(test_if, "\n", "Submenuitem>", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"F35 UART test failed\n");
    }

    sleep(1);

    return (rv);
}

/*
 **********************************************************************
 * Function: gpio_exp_test
 *  
 * Description: Test the output pins 1 (boot select) and 4 (uart mux) 
 *              of the GPIO Expander register.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */ 
static int gpio_exp_test (void)
{
    n2g_i2c_if_t  *pca;     
    uchar save, data = 0;
    //prpass(testpass, "PCA9557 Addr 0x38 GPIO Expander Register test");
    /*
     * 1. Set polarity reg to 0x0
     * 2. Set the config reg to select the output pins
     * 3. read/write to data reg and check the value
     */
    assert(f35_iface);
    assert(f35_iface->priv);

    prpass(testpass,"GPIO Expander - ");

    pca = (n2g_i2c_if_t *)f35_iface->pca;
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 Polarity register \n",
              __FUNCTION__);
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, POLARITY_INV_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "%s(): Unable to read PCA9557 Polarity register \n",
              __FUNCTION__);
        return (FAILED);
    }
    data = 0x7D;
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n",
              __FUNCTION__);
        return (FAILED);
    }
    io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &data, TRUE);
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "%s(): Unable to read PCA9557 Input register \n",
              __FUNCTION__);
        return (FAILED);
    }
    //printf("\n Input Port Register = 0x%x", data);
    save = data;
    data = ((~(save&0x82))&0x82);
    //printf("\n Set Output Port Registe to 0x%x", data);
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT_REG, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 Output register \n",
              __FUNCTION__);
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "%s(): Unable to read PCA9557 Input register \n",
              __FUNCTION__);
        return (FAILED);
    }
    //printf("\n Read back Input Port Register = 0x%x\n", data);
    if ((data&0x82) != ((~(save&0x82))&0x82)) {
        cterr('f', 0, "%s(): Tried to write 0x%x but read back 0x%x from Input"
              "port register \n", __FUNCTION__, (((~(save&0x82))&0x82)), (data&0x82));
        return (FAILED);
    }
    printf("\n Write back default value to Output Port Register 0x%x", save);
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT_REG, &save) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 Output register \n",
              __FUNCTION__);
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "%s(): Unable to read PCA9557 Input register \n",
              __FUNCTION__);
        return (FAILED);
    }
    if ((data&0x82) != (save&0x82)) {
        cterr('f', 0, "%s(): Tried to write default value 0x%x but read back 0x%x\n", 
              __FUNCTION__, (data&0x82), (save&0x82));
        return (FAILED);
    }
    return (PASSED);
}   

/*
 *************************************************************************
 * Function: f35_iface_test
 *
 * Test entry for F35 interface test.
 *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int f35_iface_test (void)
{
    int wait_time = 3000;
    int i;
    uchar data;

    assert(f35_iface);
    if (gpio_exp_test()) {
        return (FAILED);
    }

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }

    printf("\nWait for F35 module to boot up.");
    fflush(stdout);
    f35_iface->unreset(f35_iface);
    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by F35 module side when the diag menu is up. */
    for (i = 0; i < wait_time; i++) {

        if (io_port_8bit_i2c_read(f35_iface->pca, 0x0, &data, TRUE) == FAILED) {
            cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
            return (FAILED);
        }

        if (data & 0x08) 
            break;

        msleep(200);
    }

    if (i == wait_time) {
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }
    printf("\r Interface ready bit set by F35");
    fflush(stdout);
    printf("\r Modem needs 10-15 sec to come up");
    fflush(stdout);
    msleep(150000); /* F35 module waits for 10-15 sec for modem to come up */
    printf("\r F35 Menu is up ... Ready for Uart test");
    fflush(stdout);

    /* Testing UART and GE0 interfaces */
    if (f35_uart_test()) {
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: get_host_module_mac
 *
 * F35 data structure store the host and f35 module MAC addr
 *
 * Input : None
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
static int get_host_module_mac (void)
{
    ngio_priv_t *ep;
    int i;
    uchar print_mac[6];

    assert(f35_iface);
    ep = (ngio_priv_t *) f35_iface->priv;
    assert(ep);

    get_ngio_mac_addr(f35_iface->slot, f35_iface->mod_type,  &print_mac[0]);

    for (i=0; i<6; i++) {
        ep->eth_hdr.dest_addr[i] = (uint8_t) print_mac[i];
    }
    printf("\n HOST: F35 module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
           ep->eth_hdr.dest_addr[0], ep->eth_hdr.dest_addr[1],
           ep->eth_hdr.dest_addr[2], ep->eth_hdr.dest_addr[3],
           ep->eth_hdr.dest_addr[4], ep->eth_hdr.dest_addr[5]);

#ifdef TACHI 
    system_mac_addr_get(ETH1_MAC1, (mac_addr_t *)print_mac); 
#else 
    get_host_mac_addr(f35_iface->slot, &print_mac[0]);
#endif 

    for (i=0; i<6; i++) {
        ep->eth_hdr.src_addr[i] = (uint8_t) print_mac[i];
    }
    printf("\n HOST: Host MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
           ep->eth_hdr.src_addr[0], ep->eth_hdr.src_addr[1],
           ep->eth_hdr.src_addr[2], ep->eth_hdr.src_addr[3],
           ep->eth_hdr.src_addr[4], ep->eth_hdr.src_addr[5]);
    fflush(stdout);
    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: ngio_priv_init
 *
 * F35 data structure init
 *
 * Input : None
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
static int ngio_priv_init (void)
{
    ngio_priv_t *ep;
    //f35_priv_ds *f35_ds;
    int slot;

    slot  = f35_iface->slot;
    f35_iface->priv = (void *) &ngio_priv_ds[slot];
    ep = (ngio_priv_t *) f35_iface->priv;
    ep->ngiop = (struct ngio_intf_t *) f35_iface;

    get_pid(f35_iface->cookie, ep->pid);

    ep->kernel_flag = 0;
    ep->uboot_flag = 0;
    ep->boot_dev_id = 0;
    ep->fw_dnlded = 0;
    ep->build_tx_pkt = 0;

    //f35_ds = ep->dev_priv;

    //f35_ds->modem_id[0] = 0;
    //f35_ds->uboot_info = 0;
    //f35_ds->modem_info[0] = 0;
    return (PASSED);
}

#if 0
static int f35_modem_test (void)
{
    return (PASSED);

}

static int f35_emmcflash_test (void)
{
    return (PASSED);

}

static int f35_bootflash_test (void)
{
    return (PASSED);

}

static int f35_soc_test (void)
{
    return (PASSED);

}

static int f35_mem_short_test (void)
{
    return (PASSED);

}

static int f35_led_test (void)
{
    return (PASSED);

}

static int f35_cpld_test (void)
{
    /* Verify power status of the board is good */
    /* Verify the imgage is downloaded and the diags on module is up */
    /* Send the cpld test command to the module */
    /* Wait for result from the module */
    /* Display result status */
    
    return (PASSED);    

}
#endif

/**********************************************************************
 *
 * Function: f35_test
 *  
 * This function invokes the F35 module specific tests
 *
 * Input : None
 *
 * Output: PASSED or FAILED
 *  
 ***********************************************************************/
static int f35_test (int menu_display)
{
    pid_t pid;
    const int maxlen = 128;
    char cmd[maxlen];
    int retval = PASSED;
    uint32_t status;
    int ix, iy, boot_timeout;

    assert(f35_iface);
    f35_iface->unreset(f35_iface);

    if (menu_display == 0)
        f35_tests_use_enet = 1;
    else
        f35_tests_use_enet = 0;

#if DEBUG_HOST_MOD_COMM
    printf("\n f35_tests_use_enet = %d", f35_tests_use_enet);
    fflush(stdout);
#endif

#ifdef TACHI
    diag_uart_to_nim_cnnt(f35_iface->slot); 
    snprintf(cmd, maxlen-1, UART_TTYS2_DEV);
#else 
    snprintf(cmd, maxlen-1, "/dev/ttyDASH%d", f35_iface->uart_ctrl);
#endif

    if (is_goldbeach()) {
        for (ix = 0; ix < RETRY_TIME; ix++) {
            if(ix > 1) {
                printf("\n*******************************\n");
                printf("*Retry No.%d after power cycle*\n", ix);
                printf("*******************************\n");
            }
            printf("F35 power cycle");
            if (f35_pwr_off()) {
                cterr('f', 0, "Failed to Power Off the F35 NGWIC");
                return(FAILED);
            }
            printf("\nLooking for UDP Port 69 Disable...");
            fflush(stdout);
            boot_timeout = UDP_UP_DOWN_TIMEOUT;
            /* Make sure UDP port 69 Disable*/
            do {
                status = utah_port_is_linkup(f35_iface->slot + 1);
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("\neth%d status  = %x", (f35_iface->slot) +1, 
        		             status);
                    fflush(stdout);
                }
                if (((netstat_main(FTP_SERVER)) == FAILED) && (status == FAILED)) {
                   if (diagflag_xram & D_DEBUG_OPTIONS) {
                       printf("\neth%d status  = %x\n", (f35_iface->slot) +1, 
                       		status);
                       printf("\n------ Host UDP Port 69 Disable (%f)s  -----\n",
                             (BOOT_TIMEOUT - boot_timeout) * DELAY_HELF_SEC);
                       fflush(stdout);
                   }
                   printf("OK\n");
                   fflush(stdout);
                   break;
                }
                msleep(BOOT_DELAY);
            } while (boot_timeout--);
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                if (boot_timeout < 0) {
                    printf("\nHost UDP Port 69 Not Disable\n");
                    fflush(stdout);
                } else {
                    printf("\nHost UDP Port 69 Disable\n");
                    fflush(stdout);
                }
            }        
            msleep(DELAY_1_SEC);
            if (f35_pwr_on()) {
                cterr('f', 0, "Failed to Power On the F35 NGWIC");
                return(FAILED);
            }
            printf("\nLooking for U-boot prompt ...");
            fflush(stdout);
            msleep(AUTOBOOT_STOP_TIME); /* Need add delay to avoid system hang up */
            boot_timeout = BOOT_TIMEOUT;
            do {
                dash_tx_uart(cmd, CR_C_STRING);    /* Issue ctrl+C to stop autoboot */
                if (dash_rx_polling_uart(cmd, UBOOT_PROMPT, UART_TIMEOUT) == PASSED) {
                    printf(" OK\n");
                    fflush(stdout);
                    break;
                }
                msleep(BOOT_DELAY);
            } while (boot_timeout--);
            if (boot_timeout <= 0) {
                printf("Failed to get '%s' bootloader prompt\n", "cisco-uboot");
                fflush(stdout);
            }    
            printf("\nLooking for UDP Port 69 Enable...");
            fflush(stdout);
            msleep(DELAY_1_SEC);
            /* CSCuz69331 : NIM Card TFTP Firmware Download Time Out Issue*/
            boot_timeout = UDP_UP_DOWN_TIMEOUT;
            do {
                status = utah_port_is_linkup(f35_iface->slot + 1);
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("\neth%d status  = %x", (f35_iface->slot) +1, 
        		       status);
                    fflush(stdout);
                }
                if (((netstat_main(FTP_SERVER)) == PASSED) && (status == PASSED)) {
                   if (diagflag_xram & D_DEBUG_OPTIONS) {
                       printf("\neth%d status  = %x\n", (f35_iface->slot) +1, 
                       		status);
                       printf("\n------ Host UDP Port 69 Enable (%f)s  -----\n",
                             (BOOT_TIMEOUT - boot_timeout) * DELAY_HELF_SEC);
                       fflush(stdout);
                   }
                   printf("OK\n");
                   fflush(stdout);
                   break;
                }
                msleep(BOOT_DELAY);
            } while (boot_timeout--);
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                if (boot_timeout < 0) {
                    printf("\nHost UDP Port 69 Not Enable\n");
                    fflush(stdout);
                } else {
                    printf("\nHost UDP Port 69 Enable\n");
                    fflush(stdout);
                }
            }
            printf("\nBooting F35 Image ...");
            fflush(stdout);
            msleep(DELAY_1_SEC);
            for (iy = 0; iy < BOOTD_CMD_COUNT; iy++) {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("*Retry No.%d %s", iy,BOOTD_COMMAND);
                    fflush(stdout);
                }
                dash_tx_uart(cmd, CR_C_STRING);
                msleep(BOOT_DELAY);
                dash_tx_uart(cmd, BOOTD_COMMAND);
                msleep(BOOT_DELAY);
                if (dash_rx_polling_uart(cmd, LOAD_STR, UART_TIMEOUT) == PASSED) {
                    printf(" OK\n");
                    fflush(stdout);
                    break;
                } else {
                    printf(".");
                    fflush(stdout);
                }   
            }   
            prpass(testpass, "\r HOST: Waiting for F35 Interface ready ... ");
            fflush(stdout);
            if (intf_ready(f35_iface) == PASSED) {
                break;
            } else {
                printf("\nF35 Primary Interface Not Ready\n");
            }
        }
    }

    pid = fork();
    if (pid < 0) {
        perror("Cannot fork in f35_test()");
        return (FAILED);
    } else if(pid == 0) { /* child */
        //execlp("picocom", "picocom", "-b9600", "-d8", "-pn", "-fn", 
                 //cmd, (char *)0);
        //execlp("ping", "ping", "14.6.48.1", (char *)0);
        printf("\nChild");
        execlp("microcom", "microcom", cmd, (char *)0);
        printf("\nChild: ERROR exec() failed");
        fflush(stdout);
    } else {
#ifdef TACHI
        msleep(200);
        if (diag_lewis_gesw_nim_iface_setting(TRUE,NIM_SGMII)) {
            cterr('f', 0, "Setting Lewis interface failed\n");
            kill(pid, SIGTERM);
            wait(NULL);
            return FAILED;
        }
#endif
        printf("\n In parent");
        fflush(stdout);
    }
#if 0
    if(pthread_create(&thread, NULL, start_picocom, (void *)0)) {
            printf("pthread_create failed \n");
            fflush(stdout);
    }
#endif

    prpass(testpass, "\r HOST: Waiting for F35 Interface ready ... ");
    if (intf_ready(f35_iface) == FAILED) {
        cterr('f', 0, "HOST: %s(): F35 Primary Interface Not Ready ", __FUNCTION__);
        retval = FAILED;
    } 
    printf("\n HOST: F35 Primary Interface ready in expander GPIO");

    sleep(3); /* module will wait 15 secs for host packet */

    if (f35_iface->test_type == IFACE_TEST) {
        kill(pid, SIGTERM);
        wait(NULL);
        return(retval);
    }

    /* store the module mac */
    get_host_module_mac();

    //msleep(1000);

    /* Indicate to F35 module that host is ready and sends information
       for the test environment - continuous run, menu display etc */

    if (f35_ready_test(f35_iface->slot) == FAILED) {
        printf("\n HOST:%s(): FAILED show F35 submenu ", __FUNCTION__);
        retval = 3; /* kill the shell and start picocom */
    } else {
        if ((retval = f35_rslt_test(f35_iface->slot)) == FAILED) {
            printf("\n HOST: Did not receive F35 tests result pkt");
            kill(pid, SIGTERM);
            wait(NULL);
            return (retval);
        }
    } 
    /* Tests passed and no STOP_ON_ERROR */
    /* find the process# of picocom */ 
    //system("killall picocom");
    printf("\n HOST: Now terminate the microcom process pid = %d ", pid);
    fflush(stdout);
    //system("killall picocom");
    //sprintf(cmd, " kill -9 %d", pid);
    //system(cmd);
    //pthread_cancel(thread);
    kill(pid, SIGTERM);
    //kill(0, SIGTERM);
    wait(NULL);
    if ((retval == 2) || (retval == 3)) { /* the tests results failed */
        //cterr('f', 0, "F35 Tests failed");
        /* If no stop on error continue tests so kill picocom */
        if ((DIAGFLAG & D_STOPONERR) || (retval == 3)) {
            printf("\n stop on error enterring picocom\n\n");
#ifdef TACHI
            diag_uart_to_nim_cnnt(f35_iface->slot); 
            snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2");
#else 
            snprintf(cmd,maxlen-1, 
                     "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
                     f35_iface->uart_ctrl);
#endif
            system(cmd);
            return (FAILED);
        }
    }
    return retval;
}

/*
 **********************************************************************
 *
 * Function: f35_main_test
 *
 * This function invokes the Dr Maxwell main diagnostics tests
 *
 * Input : None
 *
 * Output: PASSED or FAILED
 *
 ***********************************************************************/
static int f35_main_test (void)
{
    int  retval = PASSED;

    if (retval == PASSED) {
        retval = gpio_exp_test();
    }

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }

    if (retval == PASSED) {
        retval = f35_test(0);  /* 0:menu_display is off run all tests on F35 */
    }

    return retval;
}

/*  
 **********************************************************************
 *
 *  Function: f35_build_command_packet
 *  
 *  Description: f35_build test command 
 *    1. Fill cmd_hdr_t & cmd_info_t with select_test and other arguments
 *    2. Copy the above information into the struct ngio_priv_ti, tx_pak
 *       field
 *    3. size is sizeof(f35_cmd_pkt_t)
 *  
 *  Input: selected test; which DSP core
 *  
 *  Returns: None
 *  
 **********************************************************************
 */ 
void f35_build_command_packet (int slot, uint16_t select_test, uint32_t param1,
                               uint32_t param2, uint8_t mod_type, int *size)
{   
    ngio_priv_t *ep;
    f35_cmd_pkt_t *cmd_packet_p;
    int i;
    
    ep = &ngio_priv_ds[slot];

    if (is_goldbeach()) {
        /* If Host enable Continuous Flag, the NIM will continue test can't back to Host */
        param1 &= ~D_CONTINUOUS;
    }

    *size = sizeof(f35_cmd_pkt_t);
    cmd_packet_p = (f35_cmd_pkt_t *)&(ep->tx_pak[0]);
    //SR orig cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->cmd_hdr.src_id = HOST_ID;
    cmd_packet_p->cmd_hdr.dest_id = slot;

    cmd_packet_p->cmd_hdr.op_type = (OP_TEST_REQUEST);
    cmd_packet_p->cmd_hdr.data_len = (sizeof(f35_cmd_pkt_t));
    cmd_packet_p->cmd_info.command = 0;
    cmd_packet_p->cmd_info.result = SWAP32(RESULT_RUNNING);
    cmd_packet_p->cmd_info.flags = SWAP32(FLAG_NULL);
    cmd_packet_p->cmd_info.select = select_test;
    cmd_packet_p->cmd_info.faults = warncount;
    cmd_packet_p->cmd_info.location = 0;
    cmd_packet_p->cmd_info.expected = 0;
    cmd_packet_p->cmd_info.actual = err_accum;
    cmd_packet_p->cmd_info.extra = 0;
    cmd_packet_p->cmd_info.errorcount = errcount;
    cmd_packet_p->cmd_info.testcounter = testpass;
    cmd_packet_p->cmd_info.ReadyOnTest = 0;
    cmd_packet_p->cmd_info.TestCtrl = 0;
    cmd_packet_p->cmd_info.WhoAmI = 0;
    cmd_packet_p->cmd_info.ver_no = 0;
    cmd_packet_p->cmd_info.wait_states = 0;
    cmd_packet_p->cmd_info.param1 = param1;
    cmd_packet_p->cmd_info.param2 = param2;
    cmd_packet_p->cmd_info.param3 = mod_type;
    cmd_packet_p->cmd_info.param4 = 0;
#ifdef GE_DEBUG
    printf("\n cmd_packet_p->cmd_hdr.src_id = 0x%x", SWAP32(HOST_ID));
    //printf("\n cmd_packet_p->cmd_hdr.dest_id = 0x%x", SWAP32(core_id));
    printf("\n cmd_packet_p->cmd_hdr.op_type = 0x%x", SWAP32(OP_TEST_REQUEST));
    printf("\n cmd_packet_p->cmd_hdr.data_len = 0x%x", SWAP32(sizeof(dspif_info_t)));
#endif
unsigned long testpass = 0;
unsigned long errcount = 0;
unsigned long err_accum = 0;
unsigned long warncount = 0;

    memset((cmd_packet_p->cmd_info.bufmsg), 0, 128);
    memset((cmd_packet_p->cmd_info.errmsg), 0, 128);
    if (select_test == SELECT_READY) {
        cmd_packet_p->cmd_info.location = f35_tests_use_enet;
        cmd_packet_p->cmd_info.errorcount = errcount;
        cmd_packet_p->cmd_info.testcounter = testpass;
        cmd_packet_p->cmd_info.faults = warncount;
        cmd_packet_p->cmd_info.actual = err_accum;
        for (i = 0; i < 80; i++)
            cmd_packet_p->cmd_info.bufmsg[i] = ep->pid[i];
        printf("\n SELECT_READY Packet data:");
        printf("\n continuous mode = %d, errcount =%d, testpass = %d,"
               " warncount = %d, err_accum = %d, diagflag = %d", 
               cmd_packet_p->cmd_info.location, 
               cmd_packet_p->cmd_info.errorcount,
               cmd_packet_p->cmd_info.testcounter, 
               cmd_packet_p->cmd_info.faults, 
               cmd_packet_p->cmd_info.actual,
               cmd_packet_p->cmd_info.param1); 
        fflush(stdout);
    }
}

/*
 ***********************************************************************
 * Name: setup_ge_env (common)
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
static int setup_ge_env (int slot)
{
    ngio_priv_t *ep;
    int status = PASSED;
    int sgmii_port = 0;
    char if_name[10];
#ifdef TACHI  /* tachi has nc-si packets send frequently, using filter */
    uint32_t mac0 = 0; 
    uint32_t mac1 = 0; 
    struct sock_filter bpf_code[MAC_FILTER_SIZE];
    struct sock_fprog filter; 
#endif 

    ep = (ngio_priv_t *) &ngio_priv_ds[slot];
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        /* Linux socket already setup return */
        return (PASSED);
    }

    sgmii_port = get_sgmii_port_num(slot, TYPE_SWITCH);
#ifdef TACHI
    sprintf(if_name, ETH1_MAC1);
#elif TABEIL
    sprintf(if_name, "%s", TABEI_ETH_BP);
#else
    sprintf(if_name, "eth%d", sgmii_port);
#endif 
    status = setup_eth_dev(if_name, &(ep->socket_gl));

#ifdef TACHI
    memcpy(&mac1, &ep->eth_hdr.src_addr[2], 4); 
    memcpy(&mac0, &ep->eth_hdr.src_addr[0], 2); 
    mac1 = htonl(mac1);
    mac0 = htons(mac0);

    bpf_code[0] = (struct sock_filter){0x20, 0, 0, 0x00000002};
    bpf_code[1] = (struct sock_filter){0x15, 0, 3, mac1};
    bpf_code[2] = (struct sock_filter){0x28, 0, 0, 0x00000000};
    bpf_code[3] = (struct sock_filter){0x15, 0, 1, mac0};
    bpf_code[4] = (struct sock_filter){0x6, 0, 0, 0x0000ffff};
    bpf_code[5] = (struct sock_filter){0x6, 0, 0, 0x00000000};
    filter.len = MAC_FILTER_SIZE;
    filter.filter = bpf_code;

    if(setsockopt(ep->socket_gl, SOL_SOCKET, SO_ATTACH_FILTER, 
		  &filter, sizeof(filter)) < 0) {
	cterr('f', 0, "%s:setsockopt error\n", __FUNCTION__);
	return (FAILED);
    }
#endif 
//#ifdef GE_COMM
    printf("\n socket = %d", ep->socket_gl);
fflush(stdout);
//#endif

    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    ep->ge_setup_flag = TRUE;

    return (PASSED);
}

/*
 ***********************************************************************
 * Name: f35_rslt_test (common)
 *
 * Description:
 *      Wait to receive the result packet from NIM
 *  
 * Input: slot : NIM slot
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
static int f35_rslt_test (int slot) 
{
    struct ngio_intf_t *iface;
    ngio_priv_t *ep;
    fe_packet_t   *recv_packet_p;
    f35_cmd_pkt_t *ready_msg_p;
    f35_cmd_pkt_t ready_msg;
    int retval = PASSED;

    printf("\n HOST: Wait for Result Pkt From F35 ");

    ep = &ngio_priv_ds[slot];
    iface = ep->ngiop;
    assert(iface);
    
    ep = f35_iface->priv;
    printf("\n HOST: F35 module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
           ep->eth_hdr.dest_addr[0], ep->eth_hdr.dest_addr[1],
           ep->eth_hdr.dest_addr[2], ep->eth_hdr.dest_addr[3],
           ep->eth_hdr.dest_addr[4], ep->eth_hdr.dest_addr[5]);
    fflush(stdout);
    recv_packet_p = &(ep->recv_packet);
    ready_msg_p = &ready_msg;

    if (setup_ge_env(slot) == FAILED) {
        return (FAILED);
    }

    memset((uchar *)ready_msg_p, 0, (int)sizeof(f35_cmd_pkt_t));
    memset((uchar *)recv_packet_p, 0, (int)sizeof(fe_packet_t));

    /* wait for result packet */
    //if (wait_result_packet(slot, 280)) {
    if (wait_result_packet(slot, 3800, 0)) {
        cterr('f', 0, "\nHOST: %s(): Timed out waiting for RESULT PACKET from F35.",
               __FUNCTION__);
        return (FAILED);
    }
#ifdef TACHI
    memcpy((char *)ready_msg_p, (char *)(&(recv_packet_p->data[0])-2),
           sizeof(f35_cmd_pkt_t));
#else 
    memcpy((char *)ready_msg_p, (char *)(&(recv_packet_p->data[0])+0),
           sizeof(f35_cmd_pkt_t));
#endif 
//dismem((uchar *)ready_msg_p, 0x50, (ulong) ready_msg_p, 0x4);
    /* parse result */
    if (ready_msg_p->cmd_hdr.op_type != OP_RESPONSE) {
        cterr('f', 0, "HOST: %s(): Did not receive test result pkt op_type: 0x%x: "
              "expected 0x%x", __FUNCTION__, ready_msg_p->cmd_hdr.op_type, 
              OP_RESPONSE);
        return (FAILED);
    }

    if (cleanup_ge_env(slot) == FAILED) {
        cterr('f', 0, "HOST: %s(): cleanup_ge_env() failed", __FUNCTION__);
        return (FAILED);
    }

    if ((ready_msg_p->cmd_hdr.src_id !=
        ((f35_iface->slot << 24) | (f35_iface->id) << 16)) || 
        (ready_msg_p->cmd_info.param3 != f35_iface->mod_type) ||
        (ready_msg_p->cmd_hdr.dest_id != HOST_ID)) {
        cterr('f', 0, "HOST: %s(): Did not receive test result from correct NIM "
              "slot expected src_id 0x%x, received 0x%x OR module type expected"
              "0x%x, received 0x%x OR dest_id received 0x%x not equal to expected"
              " 0x%x", __FUNCTION__, 
              ((f35_iface->slot << 24) | (f35_iface->id) << 16),
              ready_msg_p->cmd_hdr.src_id, f35_iface->mod_type, 
              ready_msg_p->cmd_info.param3, ready_msg_p->cmd_hdr.dest_id, HOST_ID);
        return (FAILED);
    } else {
        printf("\r HOST: Received Test Result from F35 Armada ");
    }
    err_accum = ready_msg_p->cmd_info.actual;
    if (ready_msg_p->cmd_info.result == RESULT_FAILED) {
        return (2);
    }
    return retval;

}

/* Building packet to be sent to F35 module restrictions :-

 cmd_hdr.src_id : should be equal to HOST_ID (0xF35)
 cmd_info.select: tells what type of packet is this :- SELECT_READY etc
                  should be equal to the type expected by F35 module
   For READY packet host send the following information :-
        tests_use_enet = cmd_pkt.cmd_info.location;
        host_testpass = cmd_pkt.cmd_info.testcounter;
        host_warncount = cmd_pkt.cmd_info.faults;
        host_errcount = cmd_pkt.cmd_info.errorcount;
        host_err_accum = cmd_pkt.cmd_info.actual;
        host_diagflag = cmd_pkt.cmd_info.param1;
        nim_slot = cmd_pkt.cmd_hdr.dest_id;
        mod_type = cmd_pkt.cmd_info.param3;

   For non READY packets following is checked by the F35 module 
        cmd_pkt.cmd_hdr.dest_id != nim_slot || cmd_pkt.cmd_info.param3 != mod_type
   F35 module sends the packets to host with the following information:
        cmd_hdr.op_type = OP_RESPONSE (for ready response it is OP_READY)
        cmd_hdr.src_id = ((nim_slot) << 24) | ((BOARD_ID) << 16)
        cmd_hdr.dest_id = HOST_ID
*/

/*
 **********************************************************************
 *
 * Function: f35_ready_test.
 *
 * Description: Check if NIM is ready
 *
 * Input:  slot : NIM slot
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
static int f35_ready_test (int slot) 
{
    struct ngio_intf_t *iface;
    ngio_priv_t *ep;
    fe_packet_t   *recv_packet_p;
    f35_cmd_pkt_t *ready_msg_p;
    f35_cmd_pkt_t ready_msg;
    int size;
    int retval = PASSED;

    printf("\n HOST: Send Ready Info Packet to F35 ");

    ep = &ngio_priv_ds[slot];
    iface = ep->ngiop;
    assert(iface);

    ep = f35_iface->priv;
#ifdef DEBUG
    printf("\n HOST: F35 module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
           ep->eth_hdr.dest_addr[0], ep->eth_hdr.dest_addr[1],
           ep->eth_hdr.dest_addr[2], ep->eth_hdr.dest_addr[3],
           ep->eth_hdr.dest_addr[4], ep->eth_hdr.dest_addr[5]);
    fflush(stdout);
#endif
    recv_packet_p = &(ep->recv_packet);
    ready_msg_p = &ready_msg;

    if (setup_ge_env(slot) == FAILED) {
        return (FAILED);
    }

    memset((uchar *)ready_msg_p, 0, (int)sizeof(f35_cmd_pkt_t));
    memset((uchar *)recv_packet_p, 0, (int)sizeof(fe_packet_t));

    f35_build_command_packet(slot, SELECT_READY, (NVRAM)->diagflag, 
                             f35_iface->id, f35_iface->mod_type, &size);
    prpass(testpass, "\r HOST: Built Ready packet size = %d", size);
    fflush(stdout);
#ifdef DEBUG
    printf("\r HOST: F35 module MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",
           ep->eth_hdr.dest_addr[0], ep->eth_hdr.dest_addr[1],
           ep->eth_hdr.dest_addr[2], ep->eth_hdr.dest_addr[3],
           ep->eth_hdr.dest_addr[4], ep->eth_hdr.dest_addr[5]);
#endif

    /* Send the packet in struct ngio_priv_t, tx_pak field */
    if ((retval = send_tx_pkt(slot, size)) != PASSED) {
        cterr('f', 0, "\n HOST: %s(): send_command_packet() returned tx error "
              "%d\n", __FUNCTION__, retval);
        return (FAILED);
    }
    prpass(testpass, "\n HOST: Sent Ready packet ");
    fflush(stdout);
    /* wait for result packet */
    if (wait_result_packet(slot, 60, 1)) {
        cterr('f', 0, "\nHOST: %s(): Timed out waiting for READY message from "
              "F35.", __FUNCTION__);
        return (FAILED);
    }

    /* due to Tachi bmc is 32bit architecture, O2/USD is 64bit arch.
     * the size of member pkt_len on structure ether_hdr_t
     * will be 4 instead 2.
     * then sizeof(ether_hdr_t) equal to 14 on O2/USD;
     * and  sizeof(ether_hdr_t) equal to 16 on TACHI BMC.
     *
     * it will cause ready_msg_p->cmd_hdr.op_type cannot get correct
     * vaule from recv_packet_p->data[0];
     * To alias the pointer recv_packet_p->data[0],
     * we nimus 2 to match the location.
     *
     * on router_if.h
     *  typedef struct {
     *      mac_addr_t  dest_addr;
     *      mac_addr_t  src_addr;
     *      unsigned short      pkt_len;
     *  } ether_hdr_t;   // total size is 16 instead of 14 on BMC
     *
     *  typedef struct fe_packet_t {
     *      ether_hdr_t eth_hdr;
     *      unsigned char       data[1500];
     *  } fe_packet_t;
     *
     */

#ifdef TACHI
    memcpy((char *)ready_msg_p, (char *)(&(recv_packet_p->data[0])-2),
           sizeof(f35_cmd_pkt_t));
#else 
    memcpy((char *)ready_msg_p, (char *)(&(recv_packet_p->data[0])+0),
           sizeof(f35_cmd_pkt_t));
#endif 
    /* parse result */
    if (ready_msg_p->cmd_hdr.op_type != OP_READY) {
        cterr('f', 0, "HOST: %s(): Ready test failed op_type: 0x%x: expected "
              "0x%x", __FUNCTION__, ready_msg_p->cmd_hdr.op_type, OP_READY );
        return (FAILED);
    }
    /* check if the firmware version is correct */
    if ((ready_msg_p->cmd_info.param1 != DIAGFW_MAJ_REL) ||
        (ready_msg_p->cmd_info.param2 != DIAGFW_MIN_REL) ||
        (ready_msg_p->cmd_info.param3 != DIAGFW_DEBUG_VER)) {
        printf("\n HOST: DSP FW version does not match 0x%x:0x%x:0x%x, expected"
               " 0x%x:0x%x:0x%x", ready_msg_p->cmd_info.param1,
               ready_msg_p->cmd_info.param2, ready_msg_p->cmd_info.param3,
               DIAGFW_MAJ_REL, DIAGFW_MIN_REL, DIAGFW_DEBUG_VER);
      //cterr('f', 0, "HOST: %s(): Ready test failed DSP diag FW version does "
      //     "not match", __FUNCTION__);
      //return (FAILED);
    }
    /* Only one dsp */
    ep->major_rel = ready_msg_p->cmd_info.param1;
    ep->minor_rel = ready_msg_p->cmd_info.param2;
    ep->debug_ver = ready_msg_p->cmd_info.param3;

    if (cleanup_ge_env(slot) == FAILED) {
        cterr('f', 0, "HOST: %s(): cleanup_ge_env() failed", __FUNCTION__);
        return (FAILED);
    }

    if (ready_msg_p->cmd_hdr.src_id == 
        ((f35_iface->slot << 24) | (f35_iface->id) << 16))
        printf("\r HOST: Received Ready Response from F35 Armada ");
    return retval;

}

/*
 **********************************************************************
 *
 * Function: intf_ready().
 *
 * Description: Check if NIM is booted up. Read the GPIO ready bit.
 *
 * Input:  nim - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
static int intf_ready (struct ngio_intf_t *iface)
{
    int ix, retval, time_out;
    uchar data;

    retval = FAILED;

    if (is_goldbeach()) {
        time_out = GB_READY_TIME_OUT;/*CSCuy97529 : Goldbeach need more than 60 sec*/
    } else {
        time_out = READY_TIME_OUT;	
    }
    for (ix = 0; ix < time_out; ix++) {

        msleep(1000);
        /* Check if the interface is up */
        if (io_port_8bit_i2c_read(iface->pca, INPUT_PORT_REG,
                                  &data, TRUE)) {
            retval = FAILED;
            break;
        }

        if ((data & BIT3) == 0) {
            //cterr('f', 0, "Primary Interface Not Ready data read 0x%x", data);
            retval = FAILED;
            continue;
        } else {
            retval = PASSED;
            break;
        }
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nCheck interface time out = %d\n",ix);
    }
    /* SR turn on the green light wfter interface ready from GPIO expander */
    /* Make sure GPIO2 is used for LED control */
    if (retval == PASSED) {
        if (util_oir_ltc4215_led(iface->oir, OIR_LED_GREEN_ONLY))
            printf("\n intf_ready(): Cannot turn on green light after interface "
                   "ready");
    }
    return (retval);
}

/*
 **********************************************************************
 *
 * Function: f35_nim_test().
 *
 * Description: This function is the entry point for F35 NGWIC test .
 *
 * Input:  nim - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
int f35_nim_test (void *nim)
{ 
    struct ngio_intf_t *iface;
    int slot;
    int ret_val = PASSED;
    ushort board_id = 0;
    uchar data;

    assert(nim);

    iface = (struct ngio_intf_t *)nim;
    f35_iface = iface;

    slot = iface->slot;
    board_id = iface->id;

    iface->uart_on(nim);    
    
    if (f35_iface->mod_type == SM_DAUGHTER_CARD) {
        printf("\nF35 test, board_id %#x, SM slot %d\n", board_id, slot);
        sprintf(nimsubmenutitle, "F35 SM%d DC Host Main Menu", slot);
        testname("SM DC Slot%d F35", slot);
    } else {
        printf("\nF35 test, board_id %#x, NGWIC slot %d\n", board_id, slot);
        sprintf(nimsubmenutitle, "F35 NIM%d Host Main Menu", slot);
        testname("Slot%d F35 NGWIC ", slot);
    }
    /* init the NGIO private data structure */
    ngio_priv_init();

    if (tftp_get(0, "nim_cwan_fw.img", 0, "/firmware/nim_cwan_fw.img", 1) < 0) {
	cterr('f', 0, "Failed to tftp download firmware to local host");
	return (FAILED);
    }

    /* Setup GPIO to boot the upgrade image */
    data = 0;
    if (io_port_8bit_i2c_write(f35_iface->pca, POLARITY_INV_REG, &data) 
        == FAILED) {
        cterr('f', 0, "%s():Unable to write PCA9557 P0larity register \n",
              __FUNCTION__);
        return (FAILED);
    }
    data = 0xFD;
    //printf("\n Set Configuration Register to 0x%x", data);
    if (io_port_8bit_i2c_write(f35_iface->pca, CONFIGURATION_REG, &data) 
        == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n", 
              __FUNCTION__);
        return (FAILED);
    }
    data = 0x2;  /* Choose upgrade image for booting */
    if (io_port_8bit_i2c_write(f35_iface->pca, OUTPUT_PORT_REG, &data) 
        == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 Output register \n", 
              __FUNCTION__);
        return (FAILED);
    }

    if (iface->test_type == IFACE_TEST)
        return(f35_iface_test());

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    f35_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(f35_mainmenu_tbl, F35_MAINMENU_TBL_SIZE, "F35 Menu",
			  &maindiagp);
    build_secondary_submenu(f35_mainmenu_tbl, F35_MAINMENU_TBL_SIZE,
			    main_menu_secondary_items);

    /* turn on the green light */
    if (util_oir_ltc4215_led(iface->oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
    //get_host_module_mac();
    if (iface->menu_display) {
        menu(maindiagp, main_menu_secondary_items, '\0');
    } else {
        f35_tests_use_enet = 1;
        f35_main_test();
    }

    f35_cleanup();

    return (ret_val);
}

/*
 **********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_register_test (void)
{
    int ret;

    prpass(testpass, "LTC4215 OIR Register test ");

    ret = oir_ltc4215_register_test(f35_iface->oir);
    if (ret == FAILED)
	cterr('f',0,"LTC4215 register test failed.");

    return (ret);
}

int gpio_ltc_test (void)
{
    int retval;

    retval = PASSED;
    assert(f35_iface);
    retval = gpio_exp_test();

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
	return (FAILED);
    }
    return retval;
}

/*
 **********************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_reg_write(void)
{
    void *oir_if;

    oir_if = (void *)(f35_iface->oir);
    assert(oir_if);
    return(util_oir_ltc4215_reg_write(oir_if));
}

/*
 **********************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int ltc4215_reg_read(void)
{
    void *oir_if;

    oir_if = (void *)(f35_iface->oir);
    assert(oir_if);

    return(util_oir_ltc4215_reg_read(oir_if));
}

/*
 **********************************************************************
 *  
 * Function: f35_power_stat
 *
 * Description: Display the NIM's current power, current, voltage reading
 *
 * Input : None.
 *
 * Output: PASSED
 *  
 **********************************************************************
 */
static int f35_power_stat (void)
{
    void *oir_if;
    uchar reg_e, reg_f, reg_g;
    float i, pcard, pres, ptotal, vdd, ovoltsensor;

    assert(f35_iface);
    
    oir_if = (void *)(f35_iface->oir);
    assert(oir_if);

    oir_ltc4215_reg_read (oir_if, 0x4, &reg_e);
    oir_ltc4215_reg_read (oir_if, 0x5, &reg_f);
    oir_ltc4215_reg_read (oir_if, 0x6, &reg_g);
    printf("\n Sense Register E  (04h) = 0x%x (%d) microvolts (uV)", 
           reg_e, reg_e);
    printf("\n Source Register F (05h) = 0x%x (%d) millivolts (mV)", 
           reg_f, reg_f);
    printf("\n ADIN Register G   (06h) = 0x%x (%d) millivolts(mV)", 
           reg_g, reg_g);

    /* sense register resolution is 151 micro volts */
    vdd = reg_e * .151; /* in milli volts */
    ovoltsensor = reg_f * 60.5; /* in mvolts */

    /* i = NIM current = voltage across sense resistor (Reg E) divided 
       by value of the sense resistor */

    /* The sense resistor on F35 is .015 ohms */
    i = (vdd / 15);  /* mv/mohms = A */
 
    pcard = (ovoltsensor * i)/1000;  /* (mv * A) / 1000 = W */

    pres = (i * i * 0.015); /* 15 mohm = .015 ohm */
  
    /* ptotal = pres + pcard = (i^2 * R) + ( V * i)  */
    ptotal = pcard + pres;

    printf("\n NIM power (Ptotal) = %.2f W", ptotal);
    printf("\n NIM current = %.3f A", i);
    printf("\n NIM input voltage = %.2f V", ((ovoltsensor + vdd)/1000));
    printf("\n NIM operating voltage = %.2f V", (ovoltsensor)/1000);
    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: f35_pwr_off
 *
 * Description: This function power off F35 NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int f35_pwr_off (void)
{
    void *oir_if;
    uint8_t data = 0;

    oir_if = (void *)(f35_iface->oir);
    assert(oir_if);

    printf("\nPower Off the F35 NGWIC.\n");

    if (is_goldbeach()) {
        /* disable power interrupt avoid system alert */
        ngiowic_disable_intr(f35_iface->slot, NGIO_FLT_INTR);
    }

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: f35_power_off
 *
 * Description: This function is a wrapper to power off F35 NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int f35_power_off (void)
{
    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! F35 NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    return (f35_pwr_off());
}


/*
 **********************************************************************
 *
 * Function: f35_pwr_on
 *
 * Description: This function power on F35 NGWIC.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int f35_pwr_on (void)
{
    void *oir_if;
    uint8_t data = 0;

    assert(f35_iface);
    oir_if = (void *)(f35_iface->oir);
    assert(oir_if);

    printf("\nPower On the F35 NGWIC.\n");

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(f35_iface, f35_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power on NGWIC module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return(FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return(FAILED);
    }

    printf("Waiting for F35 NGWIC to Power-Up.\n");
    msleep(2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    f35_iface->uart_on(f35_iface);    

    /* take F35 NGWIC out of reset */
    f35_iface->unreset(f35_iface);

    printf("F35 NGWIC is powered up.\n");

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: f35_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int f35_pwr_cycle (void)
{
    uint8_t i;
//, ans;

    printf("\n");
    printf("Power Cycle the F35 NGWIC");

#if 0
    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "F35 is not Power Cycled.\n\n");
        return (PASSED);
    }
#endif

    if (f35_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the F35 NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (f35_pwr_on()) {
        cterr('f', 0, "Failed to Power On the F35 NGWIC");
        return(FAILED);
    }

#if DDR3_TRAINING_DEBUG /* For testing uboot DDR3 training capture */
    printf("\n Power on for 10 sec to capture log\n");
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }
#endif
    return(PASSED);
}

/*
 **********************************************************************
 *
 * Function: pca9557_reg_read
 *
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9557_reg_read (void)
{
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset;

    assert(f35_iface);
    pca = f35_iface->pca;
    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    printf("\nRegister @ %#x = %#x\n", offset, data);
    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: pca9557_reg_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9557_reg_write (void)
{
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset;

    assert(f35_iface);
    pca = f35_iface->pca;
    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    return (PASSED);
}

#if 0   /* SR Not for F35 */
static int 
set_ngnim_console ()
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    /* set GPIO pin 4 to 0 for NGWIC console redirect */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    return (PASSED);
}


static int 
set_ngvm_console ()
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    /* set GPIO pin 4 to 1 for daughtercard console redirect */
    data |= 0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    return (PASSED);
}
#endif

/*
 **********************************************************************
 *
 * Function: f35_console_switch
 *
 * Description: use picocom linux app to display the F35 console
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int f35_console_switch (void)
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(f35_iface);

    f35_iface->unreset(f35_iface);

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); 

#ifdef TACHI
    diag_uart_to_nim_cnnt(f35_iface->slot); 
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2");
#else 
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             f35_iface->uart_ctrl);
#endif 

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: f35_o2_shell
 *
 * Description: Display the platform linux shell prompt
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int f35_o2_shell (void)
{
    int slot;

    assert(f35_iface);
    slot = f35_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\nEscaping to Shell from NGWIC Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);    
}

/*
 **********************************************************************
 *
 * Function: f35_o2_command
 *
 * Description: Execute linux shell command on the platform linux
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int f35_o2_command (void)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return(PASSED);
}

/*
 **********************************************************************
 *
 * Function: enable_bp_ge_lpbk
 *
 * Description: Enable the platform GE switch port (connected to the
 *           particular NIM) loopback mode.
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int enable_bp_ge_lpbk (void)
{
    int ge_port;

    assert(f35_iface);
    if (is_goldbeach()) {
        /* Goldbeach platform didn't have GESW*/
        printf("\nGoldbeach Didn't Support GESW\n");
        return (PASSED);
    }
#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(f35_iface->slot, TGT_DEV_NGWIC, 0);
#else
    ge_port = ovld_get_ge_sw_port_num(f35_iface->slot, TGT_DEV_NGWIC, 0);
#endif
    set_gesw_line_loopback(ge_port, 1);

    return (PASSED);
}

/*
 **********************************************************************
 *
 * Function: disable_bp_ge_lpbk
 *
 * Description: Disable the platform GE switch port (connected to the
 *           particular NIM) loopback mode.
 *
 * Input : None.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int disable_bp_ge_lpbk (void)
{
    int ge_port;

    assert(f35_iface);
    if (is_goldbeach()) {
        /* Goldbeach platform didn't have GESW*/
        printf("\nGoldbeach Didn't Support GESW\n");
        return (PASSED);
    }
#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(f35_iface->slot, TGT_DEV_NGWIC, 0);
#else
    ge_port = ovld_get_ge_sw_port_num(f35_iface->slot, TGT_DEV_NGWIC, 0);
#endif
    set_gesw_line_loopback(ge_port, 0);

    return (PASSED);
}

/******** History ********
$Log: nim_f35.c,v $
Revision 1.13  2019/11/25 08:55:49  kehuang2
Collapse Tabei-L into main trunk

Revision 1.12  2018/05/22 02:31:11  alpeng
fixed compiler warning, CSCvj57934

Revision 1.11  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.10  2017/08/10 10:10:34  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.9  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.8  2016/10/16 12:28:12  iachang
Supported Goldbeach Platform.

Revision 1.7  2016/09/14 07:28:11  jimmyya
Add serdes setting for Tachi project

Revision 1.6.2.4  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.6.2.3  2017/06/13 09:25:58  alpeng
fix edvt issue for racing condition

Revision 1.6.2.2  2017/04/05 06:40:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.6.2.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.10  2017/08/10 10:10:34  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.9  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.8  2016/10/16 12:28:12  iachang
Supported Goldbeach Platform.

Revision 1.7  2016/09/14 07:28:11  jimmyya
Add serdes setting for Tachi project

Revision 1.6  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.5  2016/03/22 04:09:52  alpeng
using ipv4 as packet type

Revision 1.4  2015/03/20 17:29:21  srane
Flush printfs

Revision 1.3  2015/02/18 17:57:13  srane
F35 firmware name change per uboot image_name variable

Revision 1.2  2015/02/02 03:33:34  srane
Add tftp_get command, gpio expander register setup, fix uart test.

Revision 1.1  2014/11/01 05:04:26  srane
Initial commit for F35 4G NIM



$Endlog$
*/
