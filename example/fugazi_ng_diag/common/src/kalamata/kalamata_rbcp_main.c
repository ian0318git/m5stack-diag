/* $Id: kalamata_rbcp_main.c,v 1.8 2020/01/09 01:02:15 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/kalamata/kalamata_rbcp_main.c,v $
 *------------------------------------------------------------------
 * Filename: kalamata_rbcp_main.c
 *
 * Description: The RBCP main source code
 * Author: Kody Ko
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <net/if.h>
#include <string.h>
#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "router_if.h"
#include "nim_kalamata.h"
#include "kalamata_rbcp_main.h"
#include "rbcp_lib.h"
#include "rbcp_platform.h"
#include "platform_cookie.h"
#include "platform_margin_utils.h"
#include "module_fru.h"
#include <stdio.h>
#include "pca.h"
#include "dev_ltc4215.h"
#include "slot.h"
#include "ngvm_graffham.h"
#include "sgmii_defs.h"
#include <netinet/in.h>
#include "dash_fpga.h"
#include "common_utils.h"
#ifdef TABEIL
#include "dnv_eth_lib.h"
#endif

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/
#define KALAMATA_ATM_LED 0
#define KALAMATA_EFM_LED 1
#define KALAMATA_L0_LED  2
#define KALAMATA_L1_LED  3
#define KALAMATA_L2_LED  4
#define KALAMATA_L3_LED  5
/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

long build_kalamata_rbcp_menu(int);
int kalamata_rbcp_pwroff_intel(void);
int kalamata_rbcp_pwron_intel(void);
int kalamata_rbcp_registration_test(int);  /* also called by interface test */
void clear_kalamata_regis_done_flag(int);
int kalamata_rbcp_memory_test(int);
int kalamata_rbcp_ecc_test(int);
int kalamata_rbcp_etsec1_rmii_lpbk_test(int);
int kalamata_rbcp_etsec3_rmii_lpbk_test(int);
int kalamata_rbcp_ucc1_rmii_lpbk_test(int);
int kalamata_rbcp_ucc5_rmii_lpbk_test(int);
int kalamata_rbcp_ucc3_utopia_lpbk_test(int);
int kalamata_rbcp_spi_flash_test(int);
int kalamata_rbcp_spi_flash_protect_test(void);
int kalamata_rbcp_fpga_flash_protect_test(void);
int kalamata_rbcp_gshdsl_test(int);
int kalamata_rbcp_margin_high(int);
int kalamata_rbcp_margin_low(int);
static int terminate_rbcp_test(void);
int kalamata_rbcp_recv_msgs(char *, ushort, int *);
int kalamata_component_intr_test(int);
int kalamata_rbcp_register(void);
int kalamata_rbcp_ge_reg_test(int);
int kalamata_rbcp_led_test(int);
int kalamata_rbcp_fpga_reg_test(int);
int kalamata_setup_rbcp_ge_env(int);
int kalamata_cleanup_rbcp_ge_env(int);
int kalamata_rbcp_send(uint8_t *, int);
int kalamata_rbcp_recv(uint8_t *, int *);
int kalamata_rbcp_clear_recv(void);
int kalamata_led_utils_menu(void);
int kalamata_led_ctrl(int);

/***********************************************************************
 *  Externs
 ************************************************************************/
extern int do_all_menu_items(struct menuinfo *);

extern int kalamata_test_slot;
extern ushort kalamata_board_id;
extern void show_margins_cterr_wrapper(void);
extern int io_port_8bit_i2c_read (void *i2c, int32_t offset, uchar *data,  uchar flag);
extern int rbcp_eth_pkt_rx(eth_rx_pkt_t *);
extern n2g_i2c_if_t pca_i2c[];

/***********************************************************************
 *  Global Variable
 ************************************************************************/
static int pri_intf_rdy_chk (void);
static int regis_done_flag[MAX_NUM_KALAMATA_SLOTS]={FALSE};
static char buf[RBCP_MSG_BUF_SIZE];
static int socket_gl;
#define ENHANCED_ERR_MSG_EXAMPLE 1

static submenu_xtable_t kalamata_rbcp_submenu_tbl[] = {
    { "RBCP Registration Test", (type_t(*)())kalamata_rbcp_registration_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP ECC Memory Test", (type_t(*)())kalamata_rbcp_ecc_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Component Interrupt Test", (type_t(*)())kalamata_component_intr_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP FPGA Register Test", (type_t(*)())kalamata_rbcp_fpga_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP GE PHY register Test", (type_t(*)())kalamata_rbcp_ge_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP SPI Flash Test", (type_t(*)())kalamata_rbcp_spi_flash_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP ETSEC1 RMII loopback Test", (type_t(*)())kalamata_rbcp_etsec1_rmii_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP ETSEC3 RMII loopback Test", (type_t(*)())kalamata_rbcp_etsec3_rmii_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP UCC1 RMII loopback Test", (type_t(*)())kalamata_rbcp_ucc1_rmii_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP UCC5 RMII loopback Test", (type_t(*)())kalamata_rbcp_ucc5_rmii_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP UCC3 UTOPIA loopback Test", (type_t(*)())kalamata_rbcp_ucc3_utopia_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Memory Test", (type_t(*)())kalamata_rbcp_memory_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP LED Test", (type_t(*)())kalamata_rbcp_led_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to Kalamata", (type_t(*)())kalamata_rbcp_picocom_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Registration Util", (type_t(*)())kalamata_rbcp_registration_test, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "Check Ready Pin", (type_t(*)())pri_intf_rdy_chk, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "Terminate RBCP Test", (type_t(*)())terminate_rbcp_test, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Margin High", (type_t(*)())kalamata_rbcp_margin_high, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Margin Low", (type_t(*)())kalamata_rbcp_margin_low, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP LED Util", (type_t(*)())kalamata_led_utils_menu, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Check SPI Flash Protection Test", (type_t(*)())kalamata_rbcp_spi_flash_protect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Check FPGA Flash Protection Test", (type_t(*)())kalamata_rbcp_fpga_flash_protect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define KALAMATA_RBCP_SUBMENU_TABLE_SZ \
                (sizeof(kalamata_rbcp_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t kalamata_rbcp_primary_items[KALAMATA_RBCP_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t kalamata_rbcp_secondary_items[KALAMATA_RBCP_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

static menuinfo_t kalamata_rbcp_main_menu = {
    "Kalamata RBCP Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    kalamata_rbcp_primary_items,
};
static menuinfo_t *kalamata_rbcp_menup = &kalamata_rbcp_main_menu;

extern struct ngio_intf_t *kalamata_nim_iface;

static char dst_mac[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static char src_mac[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
static char snap_id[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x01, 0x1d };

/*
 * LED Utility menu table
 */

submenu_xtable_t  kalamata_led_util_menu_table[] = {
    {"ATM", (PFT)kalamata_led_ctrl, KALAMATA_ATM_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"EFM",(PFT)kalamata_led_ctrl, KALAMATA_EFM_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L0", (PFT)kalamata_led_ctrl, KALAMATA_L0_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L1", (PFT)kalamata_led_ctrl, KALAMATA_L1_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L2", (PFT)kalamata_led_ctrl, KALAMATA_L2_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L3", (PFT)kalamata_led_ctrl, KALAMATA_L3_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
};


#define LED_UTIL_MENU_TABLE_SIZE \
        (sizeof(kalamata_led_util_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t kalamata_led_util_menu_primary_items[LED_UTIL_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t kalamata_led_util_menu_secondary_items[LED_UTIL_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo kalamata_led_util = {
  "LED Utility Menu",         /* title */
  0,                                      /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,                  /* shows major flags */
  0,                                      /* generic prompt */
  0,                                      /* size -- bumped by add_menu_item() */
  kalamata_led_util_menu_primary_items,
};
static struct menuinfo *kalamata_led_utilp = &kalamata_led_util;



/***********************************************************************
 *  Functions
 ************************************************************************/
/***********************************************************************
 * Name: kalamata_rbcp_clear_recv
 *
 * Description:
 *      This test will clear all receive packet.
 *
 * Input: None
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */
int kalamata_rbcp_clear_recv (void)
{
    int wait_count = 1000000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status,recflag,loopcount;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    recflag = ENABLE;
    loopcount = 0;

    while (recflag) {
        /* clear buffer before use */
        memset((uchar *)recv_buffer, 0, RBCP_MSG_BUF_SIZE);
        memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

        /* setup rx stucture for receiving */
        rx_pkt_p->bufr_st_addr = recv_buffer;
        rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
        rx_pkt_p->pkt_num = 0;
        rx_pkt_p->wait_time = wait_count;
        rx_pkt_p->socket = socket_gl;

        /* now check receive packet */
        status = rbcp_eth_pkt_rx(rx_pkt_p);
        loopcount++;
        if (( status != PASSED) || (loopcount > RBCP_PKT_RECV_CLEAN)) {
            recflag = DISABLE;
        } else if (diagflag_xram & D_SET_OPTIONS) {
            printf("\nclearing recv packet queue: loopcount=%d\n", loopcount);
            dismem((uchar *)(recv_buffer), 0x50, (ulong)(recv_buffer), 1);
        }
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nKalamata RBCP clear receive packet. status = %d,"
               "loopcount = %d.\n",status, loopcount);
    }

    return (PASSED);
}


/***********************************************************************
 * Name: kalamata_rbcp_send
 *
 * Description:
 *      Send rbcp packet
 *
 * Input: uint8_t *buf, int length
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */

int kalamata_rbcp_send (uint8_t *buf, int length)
{

    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    int ret_val = PASSED;

    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr), /*dest_mac_addr*/buf,
            sizeof(mac_addr_t));
    memcpy((uchar *)(tx_pkt_p->src_addr), /*source_mac_addr*/buf + ETH_ALEN,
            sizeof(mac_addr_t));

    tx_pkt_p->pkt_type = *(uint16 *)(buf + (2 * ETH_ALEN));
    tx_pkt_p->payload_size = length;
    tx_pkt_p->bufr_st_addr = buf + MAC_HEADER_LEN;
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;
        
    if (!((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\ntx_pkt_p->pkt_type = 0x%04x", tx_pkt_p->pkt_type);
        printf("\ntx_pkt_p->tx_status  = 0x%04x", tx_pkt_p->tx_status);
        printf("\ntx_pkt_p->payload_size = 0x%04x", tx_pkt_p->payload_size);
        dismem((uchar *)tx_pkt_p, 0x40, (ulong)tx_pkt_p, 1);
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nTx data in %s() :\n", __FUNCTION__);
        dismem((unsigned char *)(buf), 0x40, (unsigned long)(buf), 1);
    }

    ret_val = eth_pkt_tx(tx_pkt_p);

    if (ret_val != ETH_PKT_TX_OK ) {
        cterr('f', 0, "%s: Failed send command : ret = 0x%x, status = 0x%x",
              __FUNCTION__, ret_val, tx_pkt_p->tx_status);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: kalamata_rbcp_recv
 *
 * Description:
 *      receive rbcp  packet.
 *
 * Input: uint8_t *buf, int *length
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */

int kalamata_rbcp_recv (uint8_t *buf, int *length)
{
    int wait_count = 1000000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    /* clear buffer before use */
    memset((uchar *)buf, 0, RBCP_MSG_BUF_SIZE);
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count * 2;
    rx_pkt_p->socket = socket_gl;

    /* now wait for */
    status = rbcp_eth_pkt_rx(rx_pkt_p);

    if (status) {
        if (diagflag_xram & D_SET_OPTIONS) {
            printf("\nKalamata RBCP receive packet fail. status = %d.\n",status);
        }
        return (FAILED); /* retry is provided by caller */
    };
    /* copy received to user buf */
    memcpy ((char *)buf, (uchar *)recv_buffer, sizeof(fe_packet_t));

    *length = sizeof(fe_packet_t);

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nRx data :\n");
        dismem((unsigned char *)(buf), 0x60, (unsigned long)(buf), 1);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: kalamata_cleanup_rbcp_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: slot - slot number
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int kalamata_cleanup_rbcp_ge_env (slot)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];
    
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        /* Gold beach has no GE Switch */  
        sgmii_port = get_sgmii_port_num(slot, TYPE_SWITCH);
    } else {
        /* All other ISR 4K family has GE Switch */
        sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);
    }

    if (sgmii_port == -1) {
        cterr('f', 0, "cleanup: Failed to get sgmii port number.");
        return (FAILED);
    }
    sprintf(if_name, "eth%d", sgmii_port);
    status = cleanup_eth_dev(if_name, socket_gl);

    if (status) {
        cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
        return (FAILED);
    }
    close(socket_gl);
    return (PASSED);
}

/***********************************************************************
 * Name: kalamata_setup_rbcp_ge_env
 *
 * Description:
 *      This test will set up GE operation environment.
 *
 * Input: slot - slot number 
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int kalamata_setup_rbcp_ge_env (int slot)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        /* Gold beach has no GE Switch */  
        sgmii_port = get_sgmii_port_num(slot, TYPE_SWITCH);
    } else {
        /* All other ISR 4K family has GE Switch */
        sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);
    }

    if (sgmii_port == -1) {
        cterr('f', 0, "Setup: Failed to get sgmii port number.");
        return (FAILED);
    }
#ifdef TABEIL
    sprintf(if_name, "%s", TABEI_ETH_BP);
    status = setup_eth_dev(if_name, &socket_gl);
#else
    sprintf(if_name, "eth%d", sgmii_port);
    status = setup_eth_dev(if_name, &socket_gl);
#endif

    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 *Function:rbcp_add_snap_hdr
 *
 *Description: Add rbcp snap header
 *
 *Inpunts: buf_snap
 *
 *Outputs:void
 **********************************************************************
 */

static void rbcp_add_snap_hdr (char *buf_snap)
{
    /* Append Snap Header */
    memcpy(buf_snap, snap_id, SNAP_LEN_802 + LLC_LEN_802);
}

/**********************************************************************
 *
 *Function: kalamata_rbcp_recv_msgs
 *
 *Description: Receive opcode from Kalamata
 *
 *Inputs: buf,opcode,len
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int kalamata_rbcp_recv_msgs (char *buf, ushort opcode, int *len)
{
    struct cisco_scp_hdr *hdr;
    int ix=0;
    int msg_size;

    for (ix = 0; ix < KALAMATA_RBCP_PKT_RECV_TIMEOUT; ix++) {
        msg_size=0;
        /* See if we get any message */
        printf("\n%s(): Wait loop# %d\n", __FUNCTION__, ix);
        if (kalamata_rbcp_recv((uchar *)buf, &msg_size) == PASSED) {
            if (diagflag_xram & D_VERBOSE) {
                printf("\n\n%s(): Received a pkt\n", __FUNCTION__);
            }
            if (is_rbcp_msg(buf) == TRUE) {
                if (diagflag_xram & D_VERBOSE) {
                    printf("\n%s(): Received a RBCP pkt\n", __FUNCTION__);
                }
                msg_size = NTOHS(*(ushort *)(buf + (ETH_ALEN * 2)));
                hdr = (struct cisco_scp_hdr *)((uchar *)(buf +
                      HEADER_LEN_802 ));

                rbcp_ntoh_scp_hdr(hdr);

                if (diagflag_xram & D_VERBOSE) {
                    printf ("\n%s(): msg_size = %#4x", __FUNCTION__, msg_size);
                    printf ("\n%s(): hdr->opcode = %x", __FUNCTION__,
                            hdr->opcode);
                    printf ("\n%s(): hdr->flags = %x\n", __FUNCTION__,
                            hdr->flags);
                }
                if (hdr->opcode == opcode) {
                    *len = msg_size;
                    return (PASSED);
                }
            }
        }

        msleep(1000);
    }

    return (FAILED);
}

/**********************************************************************
 *
 *Function: kalamata_rbcp_send_msgs
 *
 *Description: Send rbcp opcode to Kalamata
 *
 *Inputs: buf,
 *         id_flag:Determine whether it has done registration test or not
 *         req_resp: Determine whether it is Request or Response
 *         opcode
 *         len
 *
 *Outputs: PASSED/FAILED
 **********************************************************************
 */

int kalamata_rbcp_send_msgs (char *buf, int id_flag, uchar req_resp, 
                             ushort opcode, int len)
{
    struct cisco_scp_hdr *hdr;
    int hdrlen;
    uint16 *etherlen;
    int total_size;

    if (diagflag_xram & D_VERBOSE) {
        printf("\n\n%s(): Now Send opcode=%04x to Kalamata\n", 
                __FUNCTION__, opcode);
    }

    hdrlen = sizeof(struct cisco_scp_hdr);
    hdr = (struct cisco_scp_hdr *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Set Response flag bit */
    if (req_resp == RBCP_FLAG_RESP) {
        hdr->flags |= RBCP_FLAG_RESP;
    } else {
        hdr->flags &= ~RBCP_FLAG_RESP;
    }
    hdr->opcode = HTONS(opcode);

    /* Append Snap Header */
    rbcp_add_snap_hdr(buf + MAC_HEADER_LEN);

    /* Append MAC Header */
    memcpy(buf, dst_mac, ETH_ALEN);
    memcpy(buf + ETH_ALEN, src_mac, ETH_ALEN);

    /* Ethernet Len = LLC + SNAP + SCP */
    etherlen = (uint16 *)(buf + (2 * ETH_ALEN));
    if (len == 0) {
        *etherlen = hdrlen + LLC_LEN_802 + SNAP_LEN_802 ;
    } else {
        *etherlen = len;
    }
    /* Totol size is all frame size, must be bigger than 60 Bytes (not include CRC) */
    if (len == 0) {
        total_size = LLC_LEN_802 + SNAP_LEN_802 + hdrlen + RBCP_PKT_PADDING;
    } else {
        total_size = len;
    }
    if (diagflag_xram & D_VERBOSE) {
        printf ("\n%s(): total packet length = %d \n",__FUNCTION__,total_size);
    }

    /* Send the message */
    return (kalamata_rbcp_send((uint8_t *)buf, total_size));
}

/**********************************************************************
 *
 * Function: build_kalamata_rbcp_menu
 *
 * Description: Build Kalamata RBCP tests and utilities menu.
 *
 * Inputs:      show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_kalamata_rbcp_menu (int show_menu)
{

    build_primary_submenu(kalamata_rbcp_submenu_tbl, KALAMATA_RBCP_SUBMENU_TABLE_SZ,
                          "RBCP Main Menu", &kalamata_rbcp_menup);
    build_secondary_submenu(kalamata_rbcp_submenu_tbl, KALAMATA_RBCP_SUBMENU_TABLE_SZ,
                            kalamata_rbcp_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(kalamata_rbcp_menup, kalamata_rbcp_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(kalamata_rbcp_menup);
        /* prcomplete(testpass, errcount, 0); */
    }

    return (PASSED);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/

/**********************************************************************
 *
 *Function: kalamata_rbcp_register
 *
 *Description: Registration function between platform and module
 *
 *Inputs: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int kalamata_rbcp_register (void)
{
    int jx, rc = 0;
    struct cisco_scp_hdr *hdr = (struct cisco_scp_hdr *)
                                ((uchar *)(buf + HEADER_LEN_802 ));
    int pkt_len = 0;

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* O2 ethernet hdr will setup in rbcp_add_snap_hdr function */

    /* set up scp hdr */
    hdr->dst_slot_id = SCP_DSLOT_ID;
    hdr->src_slot_id = SCP_SSLOT_ID;
    hdr->dst_sap = SCP_DST_SAP;
    hdr->src_sap = SCP_SRC_SAP;
    hdr->length = NTOHS(SCP_LEN);
    hdr->seq_num = NTOHS(SCP_START_SEQ_NUM);

    /* Clear not necessary packet first */
    kalamata_rbcp_clear_recv();

    /* Send RBCP registration request (opcode = 0x14) */
    if (kalamata_rbcp_send_msgs(buf, FALSE, RBCP_FLAG_REQ, CISCO_SCP_OP_REQ_REG,
                                pkt_len) == FAILED) {
        printf ("\n%s(): 0x14 req send failure\n",__FUNCTION__);
        return (RBCP_SEND_FAILURE | FAILURE_OP);
    }

    /* See if we receive the registration request pkt from Kalamata */
    for (jx = 0; jx < RBCP_RETRIES; jx++) {
        rc = kalamata_rbcp_recv_msgs(buf, CISCO_SCP_OP_REG, &pkt_len);
        if (!rc) {
            break;
        }
    }
    if (rc) {
        printf("\nDid not receive any Register opcode from Module\n");
        return (FAILED);
    }

    /* Add our own length and sequence number */
    hdr->length = SCP_LEN;
    hdr->seq_num = SCP_START_SEQ_NUM;
    hdr->length = NTOHS(hdr->length);
    hdr->seq_num = NTOHS(hdr->seq_num);

    /* Send RBCP message */
    if (kalamata_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_RESP, CISCO_SCP_OP_REG, 
                                pkt_len) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_registration_test
 *
 * Description: This function provides RBCP testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *			              1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_registration_test (int menu_option)
{
    testname(" Registration");
    int rc;

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_pid[FRU_SIZE] = {0};
    uchar ngwic_get_loc[FRU_SIZE] = {0};
#endif
    /* Enabled RBCP registeration if called from utils */
    if (menu_option==TRUE) {
        regis_done_flag[kalamata_test_slot] = FALSE;
    }

    if (!regis_done_flag[kalamata_test_slot]) {
#ifdef ENHANCED_ERR_MSG_EXAMPLE
        /*
          * 1. Subtests of the test function will reuse all variables
          * 2. All variables will be cleared automatically when
          *    entering and leaving each menu item.
          */
         /* Segment 1: PID | Unique_string : slot_info */
         fru_table_offset = MB;
         /* fru_table_offset should be set, otherwise, it will not */
         /* go to enhanced error message format in cterr() */
         /* set fru_table_offset to get the predefine value */
         /* or change mb_pid & mb_loc below */

         memcpy(ngwic_get_pid,(char*)&kalamata_board_id,2); 
         strcpy((char *)ngwic_get_loc, "RBCP Registration Test");
         platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid ; 
         platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
 
         /* Segment 2: Test step captured from prpass */
         /* Segment 3: Failure message captured from cterr */

         /* Segment 4: Components used */
         cterr_add_component("E0 -> GE PHY -> NXP CPU");

         /* Segment 5: register and memory dump */

         /* Segment 6: Platform Environment initialized here*/
         //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

         /* Segment 7: Top 3 Debugging Steps */
         cterr_add_debug("Make sure the E0 to NXP CPU Path is okay",
                         "Make sure the NXP CPU to E0 path is okay ");
#endif

        if (regis_done_flag[kalamata_test_slot] != TRUE) {
            /* Need to send out a dummy packet to initialize to traffic path. */
            kalamata_rbcp_register();
            msleep (RBCP_WAIT_TIME);
        }
        
        /* RBCP Registration is not yet done */
        prpass(testpass, "RBCP");

        rc = kalamata_rbcp_register();

        if (rc) {
            cterr('f', 0, "RBCP registration test failed");
            return (FAILED);
        } else {
            prcomplete(testpass, errcount, 0);
        }
    } else {
        /* RBCP Registration was already done earlier*/
        printf("\nRegistration was already done. So nothing executed!\n");
    }
 
    /* Once RBCP registration is done, set this flag */
    regis_done_flag[kalamata_test_slot] = TRUE;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_test
 *
 * Description: Check RBCP ready to send and clear packet 
 *
 * Input : none 
 *
 * Output: none
 *
 **********************************************************************
 */
int kalamata_rbcp_test (ushort opcode)
{
    int rc;
    int pkt_len = 0, msglen;
    ucse_uart_msg_t *msg;
    char *ptr, test_result; 
    cisco_scp_reply_data_t *receive_info;

    /* call RBCP register to ensure we have registered RBCP into Kalamata */
    kalamata_rbcp_registration_test(FALSE);

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_HOST);
    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + KALAMATA_RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    kalamata_rbcp_clear_recv();

    if (diagflag_xram & D_VERBOSE) {
        printf("%s(): line %d Transmit packet length is %d\n", 
               __FUNCTION__, __LINE__, pkt_len);
    }

    /* Send message */
    if (kalamata_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, opcode,
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    /* Check for the Ack from Kalamata */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the test response pkt from Kalamata */
    if ((rc = kalamata_rbcp_recv_msgs(buf, opcode, &pkt_len))) {
        cterr('f', 0, "RBCP Test Failed");
        return (FAILED);
    }

    /* Point to SCP data, the first btye is test result.
     * If test result is failed, then display the error messages
     */
    ptr = buf + HEADER_LEN_802 + SCP_HEADER_LEN;  
    receive_info = (cisco_scp_reply_data_t *)ptr;

    if (diagflag_xram & D_VERBOSE) {
        printf("%s(): line %d Receive packet length is %d\n", 
               __FUNCTION__, __LINE__, pkt_len);
        dismem((uchar *)(buf), pkt_len, (ulong)(buf), 1);
    }

    test_result = receive_info->result;
   
    if (diagflag_xram & D_VERBOSE) {
        printf("%s(): line %d receive_info->result is %d\n",
               __FUNCTION__, __LINE__, receive_info->result);
    }

    /* Check receive info or just display the info */
    if (test_result == FAILED) {
        printf("\nERROR LOG :\n%s",  receive_info->result_log); 
        cterr('f', 0, "Kalamata RBCP Test Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_ecc_test
 *
 * Description: This function provides RBCP Memory testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_ecc_test (int menu_option)
{
    testname(" ECC Memory");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_ECC_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP ECC Memory Test Failed");
        return (FAILED);
    }
    
    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/**********************************************************************
 *
 * Function: kalamata_rbcp_memory_test
 *
 * Description: This function provides RBCP Memory testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_memory_test (int menu_option)
{
    testname(" Memory");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_DRAM_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP Memory Test Failed");
        return (FAILED);
    }
    
    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/**********************************************************************
 *
 * Function: kalamata_rbcp_spi_flash_test
 *
 * Description: This function provides RBCP SPI Flash testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_spi_flash_test (int menu_option)
{
    testname(" SPI Flash");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_SPI_FLASH_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP SPI Flash Test Failed");
        return (FAILED);
    }
    
    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_etsec1_rmii_lpbk_test
 *
 * Description: This function provides RBCP etsec1 RMII loopback testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_etsec1_rmii_lpbk_test (int menu_option)
{
    testname(" ETSEC 1 RMII");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_ETSEC1_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP ETSEC 1 RMII Test Failed");
        return (FAILED);
    }
    
    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_etsec3_rmii_lpbk_test
 *
 * Description: This function provides RBCP etsec3 RMII loopback testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_etsec3_rmii_lpbk_test (int menu_option)
{
    testname(" ETSEC 3 RMII");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_ETSEC3_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP ETSEC 3 RMII Test Failed");
        return (FAILED);
    }
    
    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/**********************************************************************
 *
 * Function: kalamata_rbcp_ucc1_rmii_lpbk_test
 *
 * Description: This function provides RBCP UCC1 RMII loopback testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_ucc1_rmii_lpbk_test (int menu_option)
{
    testname(" UCC 1 RMII");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_UCC1_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP UCC 1 RMII Test Failed");
        return (FAILED);
    }
    
    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_ucc5_rmii_lpbk_test
 *
 * Description: This function provides RBCP UCC5 RMII loopback testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_ucc5_rmii_lpbk_test (int menu_option)
{
    testname(" UCC 5 RMII");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_UCC5_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP UCC 5 RMII Test Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: kalamata_rbcp_ucc3_utopia_lpbk_test
 *
 * Description: This function provides RBCP UCC3 UTOPIA loopback testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_ucc3_utopia_lpbk_test (int menu_option)
{
    testname(" UCC 3 UTOPIA");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_UCC3_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP UCC 3 UTOPIA Test Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}
/**********************************************************************
 *
 * Function: kalamata_rbcp_ge_reg_test
 *
 * Description: This function provides RBCP GE PHY register testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_ge_reg_test (int menu_option)
{
    testname(" GE PHY REGISTER");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_GE_REG_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP GE PHY REGISTER Test Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}
/**********************************************************************
 *
 * Function: kalamata_rbcp_led_test
 *
 * Description: This function provides RBCP GE PHY register testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_led_test (int menu_option)
{
    testname(" LED");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_LED_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP LED Test Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_led_utils_menu
 *
 * Description: Build LED utilities menu.
 *
 * Inputs: None.
 *
 * Outputs: PASSED.
 *
 **********************************************************************
 */
int kalamata_led_utils_menu (void)
{
    build_primary_submenu(kalamata_led_util_menu_table,
                          LED_UTIL_MENU_TABLE_SIZE,
                          "LED Utilities Menu",
                          &kalamata_led_utilp);
    build_secondary_submenu(kalamata_led_util_menu_table,
                            LED_UTIL_MENU_TABLE_SIZE,
                            kalamata_led_util_menu_secondary_items);
    menu(&kalamata_led_util, kalamata_led_util_menu_secondary_items, 0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_led_ctrl
 *
 * Description: This function provides single LED control.
 *
 * Input :  menu_option : LED Name
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_led_ctrl (int menu_option)
{
    int choose = menu_option;
    int on_off;
   
     switch (choose) {
            case KALAMATA_ATM_LED:
                on_off = gethex_answer("\n1.Turn ATM On\n2.Turn ATM Off\n3.Exit\n",0, 0, 3);
                if (on_off == 1) {
                      if (kalamata_rbcp_test(CISCO_SCP_LED_ATM) == FAILED) {
                          printf("Kalamata RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else if (on_off == 2) {
                      if (kalamata_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                          printf("Kalamata RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else {
                       return (PASSED);
                }
                 msleep (LED_WAIT_TIME);
            break;
            case KALAMATA_EFM_LED:
                on_off = gethex_answer("\n1.Turn EFM On\n2.Turn EFM Off\n3.Exit\n",0, 0, 3);
                if (on_off == 1) {
                      if (kalamata_rbcp_test(CISCO_SCP_LED_EFM) == FAILED) {
                          printf("Kalamata RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else if (on_off ==2) {
                      if (kalamata_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                          printf("Kalamata RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else {
                       return (PASSED);
                }
                 msleep (LED_WAIT_TIME);
            break;
            case KALAMATA_L0_LED:
                 on_off = gethex_answer("\n1.Turn L0 Green On\n2.Turn L0 Yellow On\n3.Turn L0 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L0_G) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L0_Y) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep (LED_WAIT_TIME);
                 break;
            case KALAMATA_L1_LED:
                 on_off = gethex_answer("\n1.Turn L1 Green On\n2.Turn L1 Yellow On\n3.Turn L1 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L1_G) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L1_Y) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep (LED_WAIT_TIME);
                 break;
            case KALAMATA_L2_LED:
                 on_off = gethex_answer("\n1.Turn L2 Green On\n2.Turn L2 Yellow On\n3.Turn L2 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L2_G) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L2_Y) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep (LED_WAIT_TIME);
                 break;
            case KALAMATA_L3_LED:
                 on_off = gethex_answer("\n1.Turn L3 Green On\n2.Turn L3 Yellow On\n3.Turn L3 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L3_G) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_L3_Y) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (kalamata_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Kalamata RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep (LED_WAIT_TIME);
                 break;
            default:
                 printf("error#");
                 break;
     }
     return (PASSED);

}

/**********************************************************************
 *
 * Function: kalamata_rbcp_fpga_reg_test
 *
 * Description: This function provides RBCP FPGA Register testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_fpga_reg_test (int menu_option)
{
    testname(" FPGA REGISTER");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_FPGA_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCPFPGA REGISTER Test Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_component_intr_test
 *
 * Description: This function provides RBCP component interrupt testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_component_intr_test (int menu_option)
{
    testname(" COMPONENT INTERRUPT");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_INTR_TEST) == FAILED) {
        cterr('f', 0, "Kalamata RBCP COMPONENT INTERRUPT Test Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_margin_high
 *
 * Description: This function provides RBCP margin high setting.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_margin_high (int menu_option)
{
    testname(" MARGIN HIGH");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test( CISCO_SCP_NIM_MARGIN_HIGH) == FAILED) {
        cterr('f', 0, "Kalamata RBCP MARGIN HIGH Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_margin_low
 *
 * Description: This function provides RBCP margin low setting.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_margin_low (int menu_option)
{
    testname(" MARGIN LOW");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_MARGIN_LOW) == FAILED) {
        cterr('f', 0, "Kalamata RBCP MARGIN LOW Failed");
        return (FAILED);
    }

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: clear_kalamata_regis_done_flag
 *
 * Description: Clear Register test done flag
 *
 * Input : Slot
 *
 * Output: None
 *
 **********************************************************************
 */
void clear_kalamata_regis_done_flag (int slot)
{
    regis_done_flag[slot] = FALSE;
}

/*******************************************************************************
 *
 * Function   : pri_intf_rdy_chk
 *
 * Description: Check if EXP_PRI_RDY(IO3) is asserted.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pri_intf_rdy_chk (void)
{
    uint32_t         ctr = 0;
    uchar            data = 0, pri_rdy_mask = 0x08;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    if (io_port_8bit_i2c_read(io_exp, PCA9557_IN_PORT_REG, &data, TRUE)) {
        printf("\n\nFailed to read IO Expander(PCA9557)"
               " register 0x%02X.\n\n", ctr);
        return (FAILED);
    }

    /* Check EXP_PRI_RDY(IO3) */
    if (data & pri_rdy_mask) {
        return (PASSED);
    } else {
        return (FAILED);
    }

}

/**********************************************************************
 *
 * Function: terminate_rbcp_test 
 *
 * Description: This function terminates the RBCP test.
 *
 * Input :  menu_option - None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int terminate_rbcp_test (void)
{
    int pkt_len = 0, msglen;
    ucse_uart_msg_t *msg;

    /* call RBCP register to ensure we have registered RBCP into Kalamata */
    kalamata_rbcp_registration_test(FALSE);

    prpass(testpass, "RBCP GSHDSL"); 

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));
    
    /* Clean payload data */
    memset(msg, 0, msglen);
    
    msg->payload.ucse_uart_type = htonl(UCSE_UART_HOST);
    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + KALAMATA_RBCP_PKT_PADDING;
    
    /* Clear not necessary packet first */
    kalamata_rbcp_clear_recv();
    
    if (diagflag_xram & D_VERBOSE) {
        printf("%s(): line %d Transmit packet length is %d\n",
               __FUNCTION__, __LINE__, pkt_len);
    }

    /* Send message */
    if (kalamata_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_OP_TERMINATE_RBCP,
                             pkt_len) == FAILED) {
        return (FAILED);
    }
  
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_spi_flash_test
 *
 * Description: This function provides RBCP SPI Flash testing.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_spi_flash_protect_test (void)
{
    testname(" SPI Flash Protection");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_SPI_FLASH_PROTECT_TEST) == FAILED) {
        cterr('f', 0, "SPI FLASH IS NOT PROTECTED");
        return (FAILED);
    }
    
    printf("\nSPI Flash data protection is 'ON'");

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: kalamata_rbcp_spi_flash_test
 *
 * Description: This function provides RBCP SPI Flash testing.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int kalamata_rbcp_fpga_flash_protect_test (void)
{
    testname(" FPGA Flash Protection");
    prpass(testpass,"RBCP");

    if (kalamata_rbcp_test(CISCO_SCP_NIM_FPGA_FLASH_PROTECT_TEST) == FAILED) {
        cterr('f', 0, "FPGA FLASH IS NOT PROTECTED");
        return (FAILED);
    }
    
    printf("\nFPGA Flash data protection is 'ON'");

    msleep (RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*------------------------------------------------------------------
 * $Log: kalamata_rbcp_main.c,v $
 * Revision 1.8  2020/01/09 01:02:15  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.7  2019/11/25 08:55:50  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.6  2019/08/06 06:56:09  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.5  2019/02/20 12:17:44  letsai
 * Fix compile error
 *
 * Revision 1.4  2019/02/20 11:55:25  letsai
 * Add RBCP functions to support Kalamata NIM tests(CSCvo39487 & CSCvo39481)
 *
 * Revision 1.3  2018/04/19 09:15:37  letsai
 * Add LED utility of single color control
 *
 * Revision 1.2  2018/02/24 07:36:25  letsai
 * Collapse Kalamata-branch to Main Trunk.
 *
 * Revision 1.1.4.11  2017/12/28 03:13:55  letsai
 * Add RBCP FPGA register test
 *
 * Revision 1.1.4.10  2017/12/21 03:42:17  letsai
 * Add RBCP GE PHY register test/RBCP interrupt test/RBCP LED Test
 *
 * Revision 1.1.4.9  2017/12/11 11:24:07  letsai
 * Add RBCP UCC1 RMII loopback test
 *
 * Revision 1.1.4.8  2017/09/22 10:15:41  kodko
 * *** empty log message ***
 *
 * Revision 1.1.4.7  2017/09/21 06:27:43  letsai
 * Added RBCP function of Power Supply Margin
 *
 * Revision 1.1.4.6  2017/09/21 06:12:13  letsai
 * Added RBCP function of Power Supply Margin
 *
 * Revision 1.1.4.5  2017/09/08 11:10:00  kodko
 * Modify for P1B bring up.
 *
 * Revision 1.1.4.4  2017/08/22 12:27:49  kodko
 * Increase RBCP timeout time for Kalamata module test items.
 *
 * Revision 1.1.4.3  2017/08/17 13:01:51  kodko
 * Automation test bring up for Kalamata P1A.
 *
 * Revision 1.1.4.2  2017/06/16 07:17:02  kodko
 * Initial platform code commit for Kalamata project.
 *
 *------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------
 */

