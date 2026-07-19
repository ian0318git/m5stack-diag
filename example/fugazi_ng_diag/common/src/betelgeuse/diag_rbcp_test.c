/* $Id: diag_rbcp_test.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_rbcp_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_rbcp_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <net/if.h>
#include <string.h>
#include "common_utils.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "router_if.h"
#include "platform_cookie.h"
#include "diag_enhance_err_msg_lib.h"
#include <stdio.h>
#include "pca.h"
#include "slot.h"
#include "ngvm_graffham.h"
#include "sgmii_defs.h"
#include <netinet/in.h>
#include "rbcp_lib.h"
#include "diag_rbcp_util.h"
#include "dev_ltc4215.h"
#include "diag_rbcp_test.h"

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int plat_rbcp_pwroff_intel(void);
int plat_rbcp_pwron_intel(void);
void clear_plat_regis_done_flag(int);
int plat_rbcp_margin_high (int);
int plat_rbcp_margin_low (int);
int plat_rbcp_recv_msgs(char *, ushort, int *);
int gshdsl_led_ctrl(int);


/***********************************************************************
 *  Externs
 ************************************************************************/
extern int do_all_menu_items(struct menuinfo *);

extern int plat_test_slot;
extern ushort plat_board_id;
extern void show_margins_cterr_wrapper(void);
extern int io_port_8bit_i2c_read (void *i2c, int32_t offset, uchar *data,  uchar flag);
extern int rbcp_eth_pkt_rx(eth_rx_pkt_t *);

extern int rbcp_recv_msgs(char *, ushort, int *);
extern int rbcp_send_msgs(char *, int, uchar, ushort, int);


/***********************************************************************
 *  Global Variable
 ************************************************************************/
static int regis_done_flag[MAX_NUM_PLAT_SLOTS]={FALSE};
static char buf[RBCP_MSG_BUF_SIZE];
#define ENHANCED_ERR_MSG_EXAMPLE 1

static dspif_ether_t cmd_packet_p;

static char dst_mac[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static char src_mac[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
static char snap_id[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x01, 0x1d };


int plat_test_slot = 1;
int shedir_test_slot = 1;

ushort plat_board_id;
  
/*
 * LED Utility menu table
 */

submenu_xtable_t  gshdsl_led_util_menu_table[] = {
    {"ATM", (PFT)gshdsl_led_ctrl, GSHDSL_ATM_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"EFM",(PFT)gshdsl_led_ctrl, GSHDSL_EFM_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L0", (PFT)gshdsl_led_ctrl, GSHDSL_L0_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L1", (PFT)gshdsl_led_ctrl, GSHDSL_L1_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L2", (PFT)gshdsl_led_ctrl, GSHDSL_L2_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
    {"L3", (PFT)gshdsl_led_ctrl, GSHDSL_L3_LED, 0,
     (type_t(*)())0,         0,
     (PFT)0,  0},
};


#define LED_UTIL_MENU_TABLE_SIZE \
        (sizeof(gshdsl_led_util_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gshdsl_led_util_menu_primary_items[LED_UTIL_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t gshdsl_led_util_menu_secondary_items[LED_UTIL_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static struct menuinfo gshdsl_led_util = {
  "LED Utility Menu",         /* title */
  0,                                      /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,                  /* shows major flags */
  0,                                      /* generic prompt */
  0,                                      /* size -- bumped by add_menu_item() */
  gshdsl_led_util_menu_primary_items,
};
static struct menuinfo *gshdsl_led_utilp = &gshdsl_led_util;

/***********************************************************************
 *  Functions
 ************************************************************************/

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
 *Function: plat_rbcp_recv_msgs
 *
 *Description: Recevie opcode from Gshdsl
 *
 *Inpunts: buf: send context
 *         opcode: RBCP operation code
 *         len: packet length
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int plat_rbcp_recv_msgs (char *buf, ushort opcode, int *len)
{
    struct cisco_scp_hdr *hdr;
    int ix=0;
    int msg_size;

    for (ix = 0; ix < PLAT_RBCP_PKT_RECV_TIMEOUT; ix++) {
        msg_size=0;
        /* See if we get any message */
        printf("\n%s(): Wait loop# %d\n",__FUNCTION__, ix);
        if (plat_rbcp_recv((uchar *)buf, &msg_size) == PASSED) {
            if (diagflag_xram & D_SET_OPTIONS) {
                printf("\n\n%s(): Received a pkt\n", __FUNCTION__);
            }
                if (diagflag_xram & D_SET_OPTIONS) {
                    printf("\n%s(): Received a RBCP pkt\n", __FUNCTION__);
                }
                msg_size = NTOHS(*(ushort *)(buf + (ETH_ALEN * 2)));
                hdr = (struct cisco_scp_hdr *)((uchar *)(buf +
                      HEADER_LEN_802 ));

                rbcp_ntoh_scp_hdr(hdr);

                if (diagflag_xram & D_SET_OPTIONS) {
                    printf ("\n%s(): msg_size = %#4x",__FUNCTION__, msg_size);
                    printf ("\n%s(): hdr->opcode = %x",__FUNCTION__,
                            hdr->opcode);
                    printf ("\n%s(): hdr->flags = %x\n",__FUNCTION__,
                            hdr->flags);
                }
                if (hdr->opcode == opcode) {
                    *len = msg_size;
                    return(PASSED);
                }
            }
        }

        msleep(1000);

    return(FAILED);
}

/**********************************************************************
 *
 *Function: plat_rbcp_send_msgs
 *
 *Description: Send rbcp opcode to BMC
 *
 *Inpunts: buf,
 *         id_flag:Determine whether it have done registration test or not
 *         req_resp: Determine whether it is Request or Response
 *         opcode
 *         len
 *
 *Outputs:rbcp_send(buf,total_size)
 **********************************************************************
 */

int plat_rbcp_send_msgs (char *buf, int id_flag, uchar req_resp, 
                             ushort opcode, int len)
{
    struct cisco_scp_hdr *hdr;
    int hdrlen;
    uint16 *etherlen;
    int total_size;

    /*open */
    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n\n%s(): Now Send opcode=%04x to BMC\n", 
                __FUNCTION__, opcode);
    }

    hdrlen = sizeof(struct cisco_scp_hdr);
    hdr = (struct cisco_scp_hdr *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Set Response flag bit */
    if (req_resp==RBCP_FLAG_RESP) {
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
    if (len==0) {
        *etherlen = hdrlen + LLC_LEN_802 + SNAP_LEN_802 ;
    } else {
        *etherlen = len;
    }
    /* Totol size is all frame size, must big than 60 Bytes (not include CRC) */
    if (len==0) {
        total_size = LLC_LEN_802 + SNAP_LEN_802
                   + hdrlen + RBCP_PKT_PADDING;
    } else {
        total_size = len;
    }
    if (diagflag_xram & D_SET_OPTIONS)
        printf ("\n%s(): total packet length = %d \n",__FUNCTION__,total_size);

    /* Send the message, to be implemented */
    return (plat_rbcp_send((uint8_t *)buf, total_size));
}

/*
 **********************************************************************
 *
 *  Function: plat_build_command_packet
 *
 *  Description: build command packet
 *
 *  Input: selected test; which DSP core
 *
 *  Returns: None
 *
 **********************************************************************
 */
void plat_build_command_packet (uint16_t select_test, uint32_t param1, 
                                    uint32_t param2, uint8_t core_id)
{
    cmd_packet_p.dspif_hdr.src_id = SWAP32(HOST_ID);
    /* 1 is NGVM number */
    cmd_packet_p.dspif_hdr.dest_id = SWAP32(1);

    cmd_packet_p.dspif_hdr.op_type = (OP_TEST_REQUEST);
    cmd_packet_p.dspif_hdr.data_len = (sizeof(dspif_info_t));
    cmd_packet_p.dspif_info.command = 0;
    cmd_packet_p.dspif_info.result = SWAP32(RESULT_RUNNING);
    cmd_packet_p.dspif_info.flags = SWAP32(FLAG_NULL);
    cmd_packet_p.dspif_info.select = select_test;
    cmd_packet_p.dspif_info.faults = 0;
    cmd_packet_p.dspif_info.location = 0;
    cmd_packet_p.dspif_info.expected = 0;
    cmd_packet_p.dspif_info.actual = 0;
    cmd_packet_p.dspif_info.extra = 0;
    cmd_packet_p.dspif_info.errorcount = 0;
    cmd_packet_p.dspif_info.testcounter = 0;
    cmd_packet_p.dspif_info.ReadyOnTest = 0;
    cmd_packet_p.dspif_info.TestCtrl = 0;
    cmd_packet_p.dspif_info.WhoAmI = 0;
    cmd_packet_p.dspif_info.ver_no = 0;
    cmd_packet_p.dspif_info.wait_states = 0;
    cmd_packet_p.dspif_info.param1 = param1;
    cmd_packet_p.dspif_info.param2 = param2;
    cmd_packet_p.dspif_info.param3 = 0;
    cmd_packet_p.dspif_info.param4 = 0;
#ifdef GE_DEBUG
    printf("\n cmd_packet_p.dspif_hdr.src_id = 0x%x", SWAP32(HOST_ID));
    printf("\n cmd_packet_p.dspif_hdr.dest_id = 0x%x", SWAP32(core_id));
    printf("\n cmd_packet_p.dspif_hdr.op_type = 0x%x", SWAP32(OP_TEST_REQUEST));
    printf("\n cmd_packet_p.dspif_hdr.data_len = %#lx", SWAP32(sizeof(dspif_info_t)));
#endif
    memset((cmd_packet_p.dspif_info.bufmsg), 0, 128);
    memset((cmd_packet_p.dspif_info.errmsg), 0, 128);
}


/**********************************************************************
 *
 *Function: plat_rbcp_register
 *
 *Description: Registration function between platform and module
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int plat_rbcp_register (void)
{
    int rc=0;
    struct cisco_scp_hdr *hdr = (struct cisco_scp_hdr *)
                                ((uchar *)(buf + HEADER_LEN_802 ));
    int pkt_len=0;

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
    plat_rbcp_clear_recv();

    /* Send RBCP registration request (opcode = 0x14) */
    if (plat_rbcp_send_msgs(buf, FALSE, RBCP_FLAG_REQ,CISCO_SCP_OP_REQ_REG,
                             pkt_len) == FAILED) {
        printf ("\n%s(): 0x14 req send failure\n",__FUNCTION__);
        return (RBCP_SEND_FAILURE | 0x10);
    }

    /* Check for the Ack from Gshdsl */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Register test response pkt from Gshdsl */
    if ((rc = plat_rbcp_recv_msgs(buf, CISCO_SCP_OP_REG, &pkt_len))) {
        printf("%s:%d:RBCP Register Test Failed\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Add our own length and sequence number */
    hdr->length = SCP_LEN;
    hdr->seq_num = SCP_START_SEQ_NUM;
    hdr->length = NTOHS(hdr->length);
    hdr->seq_num = NTOHS(hdr->seq_num);

    /* Send RBCP message */
    if (plat_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_RESP, CISCO_SCP_OP_REG, 
                             pkt_len) == FAILED) {
        printf("%s:%d:Send RBCP packet Failed\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_registration_test
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
int diag_rbcp_registration_test (int menu_option)
{
    testname(" Registration");
    int rc;

    #ifdef ENHANCED_ERR_MSG_EXAMPLE
     uchar ngwic_get_pid[FRU_SIZE] = {0};
     uchar ngwic_get_loc[FRU_SIZE] = {0};
    #endif
    /* Enabled RBCP registeration if called from utils */
    if (menu_option==TRUE) {
        regis_done_flag[plat_test_slot] = FALSE;
    }

    if (!regis_done_flag[plat_test_slot]) {
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

         memcpy(ngwic_get_pid,(char*)&plat_board_id,2); 
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

        /* RBCP Registration is not yet done */
        prpass(testpass, "RBCP");

        rc = plat_rbcp_register();

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
    regis_done_flag[plat_test_slot] = TRUE;
    return (PASSED);
}

/**********************************************************************
 *
 * Function: plat_rbcp_test
 *
 * Description: This function provides RBCP testing.
 *
 * Input :  opcode : test_name
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int plat_rbcp_test (int opcode)
{
    int rc;
    char *ptr, test_result; 
    cisco_scp_reply_data_t *receive_info;
    int pkt_len = 0, msglen;
    ucse_uart_msg_t *msg;

    /* call RBCP register to ensure we have registered RBCP into Gshdsl */
    diag_rbcp_registration_test(FALSE);

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_HOST);
    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + PLAT_RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    plat_rbcp_clear_recv();

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("%s(): line %d Transmit packet length is %d\n", 
               __FUNCTION__, __LINE__, pkt_len);
    }

    /* Send message */
    if (plat_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, opcode,
                             pkt_len) == FAILED) {
        printf("%s:%d:Send RBCP packet Failed\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Check for the Ack from Gshdsl */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the RBCP test response pkt from Gshdsl */
    if ((rc = plat_rbcp_recv_msgs(buf, opcode, &pkt_len))) {
        printf("%s:%d:RBCP Test Failed\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Point to SCP data, the first btye is test result.
     * If test result is failed, then display the error messages
     */
    ptr = buf + HEADER_LEN_802 + SCP_HEADER_LEN;  
    receive_info = (cisco_scp_reply_data_t *)ptr;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("%s(): line %d Recevie packet length is %d\n", 
               __FUNCTION__, __LINE__, pkt_len);
        dismem((uchar *)(buf), pkt_len, (ulong)(buf), 1);
    }

    test_result = receive_info->result;
   
    if (diagflag_xram & D_SET_OPTIONS) {
        printf("%s(): line %d receive_info->result is %d\n",
               __FUNCTION__, __LINE__, receive_info->result);
    }

    /* Check receive info or just display the info */
    if (test_result == FAILED) {
        printf("\nERROR LOG :\n%s",  receive_info->result_log); 
        return (FAILED);
    }
    

    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_rbcp_ecc_test
 *
 * Description: This function provides RBCP ECC Memory testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *                        1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_rbcp_ecc_test (int menu_option)
{
    testname(" ECC Memory");
    prpass(testpass,"RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_ECC_TEST) == FAILED) {
        cterr('f', 0, "RBCP ECC Memory Test Failed");
        return (FAILED);
    }
    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_memory_test
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
int diag_rbcp_memory_test (int menu_option)
{
    testname(" Memory");
    prpass(testpass,"RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_DRAM_TEST) == FAILED) {
        cterr('f', 0, "RBCP Memory Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_spi_flash_test
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
int diag_rbcp_spi_flash_test (int menu_option)
{
    testname(" SPI FLASH");
    prpass(testpass,"RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_SPI_FLASH_TEST) == FAILED) {
        cterr('f', 0, "RBCP SPI FLASH Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: plat_rbcp_spi_flash_protect
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
int plat_rbcp_spi_flash_protect (int menu_option)
{

    if (plat_rbcp_test(CISCO_SCP_NIM_SPI_FLASH_PROTECT) == FAILED) {
        printf("%s:%d:SPI FLASH IS NOT PROTECTED\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    printf("\nProvide data protection");

    msleep(RBCP_WAIT_TIME);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_etsec1_rmii_lpbk_test
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
int diag_rbcp_etsec1_rmii_lpbk_test (int menu_option)
{
    testname(" ETSEC1 RMII Loopback");
    prpass(testpass,"RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_ETSEC1_TEST) == FAILED) {
        cterr('f', 0, "RBCP ETSEC1 RMII Loopback Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_etsec3_rmii_lpbk_test
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
int diag_rbcp_etsec3_rmii_lpbk_test (int menu_option)
{
    testname(" ETSEC3 RMII loopback");
    prpass(testpass, "RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_ETSEC3_TEST) == FAILED) {
        cterr('f', 0, "RBCP ETSEC3 RMII loopback Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_ucc1_rmii_lpbk_test
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
int diag_rbcp_ucc1_rmii_lpbk_test (int menu_option)
{
    testname(" UCC1 RMII Loopback");
    prpass(testpass, "RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_UCC1_TEST) == FAILED) {
        cterr('f', 0, "RBCP UCC1 RMII Loopback Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_ucc5_rmii_lpbk_test
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
int diag_rbcp_ucc5_rmii_lpbk_test (int menu_option)
{
    testname(" UCC5 RMII Loopback");
    prpass(testpass, "RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_UCC5_TEST) == FAILED) {
        cterr('f', 0, "RBCP UCC5 RMII Loopback Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_rbcp_ucc3_utopia_lpbk_test
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
int diag_rbcp_ucc3_utopia_lpbk_test (int menu_option)
{
    testname(" UCC3 UTOPIA Loopback");
    prpass(testpass, "RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_UCC3_TEST) == FAILED) {
        cterr('f', 0, "RBCP UCC3 UTOPIA Loopback Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_rbcp_led_test
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
int diag_rbcp_led_test (int menu_option)
{
    testname(" LED");
    prpass(testpass, "RBCP"); 

    if (plat_rbcp_test(CISCO_SCP_NIM_LED_TEST) == FAILED) {
        cterr('f', 0, "RBCP LED Test Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: plat_rbcp_margin_high
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
int plat_rbcp_margin_high (int menu_option)
{

    if (plat_rbcp_test(CISCO_SCP_NIM_MARGIN_HIGH) == FAILED) {
        cterr('f', 0, "Set Power Supply to Margin High Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: plat_rbcp_margin_low
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
int plat_rbcp_margin_low (int menu_option)
{

    if (plat_rbcp_test(CISCO_SCP_NIM_MARGIN_LOW) == FAILED) {
        cterr('f', 0, "Set Power Supply to Margin Low Failed");
        return (FAILED);
    }

    msleep(RBCP_WAIT_TIME);

    return (PASSED);
}



/**********************************************************************
 *
 * Function: clear_plat_regis_done_flag
 *
 * Description: Clean Register test done flag
 *
 * Input :  None
 *
 * Output: None
 *
 **********************************************************************
 */
void clear_plat_regis_done_flag (int slot)
{
    regis_done_flag[slot] = FALSE;
}


/**********************************************************************
 *
 * Function: diag_terminate_rbcp_test 
 *
 * Description: This function terminates the RBCP test.
 *
 * Input :  menu_option - None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_terminate_rbcp_test (void)
{
    int pkt_len = 0, msglen;
    ucse_uart_msg_t *msg;

    /* call RBCP register to ensure we have registered RBCP into Gshdsl */
    diag_rbcp_registration_test(FALSE);

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));
    
    /* Clean payload data */
    memset(msg, 0, msglen);
    
    msg->payload.ucse_uart_type = htonl(UCSE_UART_HOST);
    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + PLAT_RBCP_PKT_PADDING;
    
    /* Clear not necessary packet first */
    plat_rbcp_clear_recv();
    
    if (diagflag_xram & D_SET_OPTIONS) {
        printf("%s(): line %d Transmit packet length is %d\n",
               __FUNCTION__, __LINE__, pkt_len);
    }

    /* Send message */
    if (plat_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_OP_TERMINATE_RBCP,
                             pkt_len) == FAILED) {
        printf("%s:%d:Send RBCP packet fail\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
  
    return (PASSED);
}

/**********************************************************************
 *
 * Function: gshdsl_led_utils_menu
 *
 * Description: Build LED utilities menu.
 *
 * Inputs: None.
 *
 * Outputs: PASSED.
 *
 **********************************************************************
 */
int gshdsl_led_utils_menu (void)
{
    build_primary_submenu(gshdsl_led_util_menu_table,
                          LED_UTIL_MENU_TABLE_SIZE,
                          "LED Utilities Menu",
                          &gshdsl_led_utilp);
    build_secondary_submenu(gshdsl_led_util_menu_table,
                            LED_UTIL_MENU_TABLE_SIZE,
                            gshdsl_led_util_menu_secondary_items);
    menu(&gshdsl_led_util, gshdsl_led_util_menu_secondary_items, 0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: gshdsl_led_ctrl
 *
 * Description: This function provides single LED control.
 *
 * Input :  menu_option : LED Name
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int gshdsl_led_ctrl (int menu_option)
{
    int choose = menu_option;
    int on_off;

     switch (choose) {
            case GSHDSL_ATM_LED:
                on_off = gethex_answer("\n1.Turn ATM On\n2.Turn ATM Off\n3.Exit\n",0, 0, 3);
                if (on_off == 1) {
                      if (plat_rbcp_test(CISCO_SCP_LED_ATM) == FAILED) {
                          printf("Gshdsl RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else if (on_off == 2) {
                      if (plat_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                          printf("Gshdsl RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else {
                       return (PASSED);
                }
                 msleep(LED_WAIT_TIME);
            break;
            case GSHDSL_EFM_LED:
                on_off = gethex_answer("\n1.Turn EFM On\n2.Turn EFM Off\n3.Exit\n",0, 0, 3);
                if (on_off == 1) {
                      if (plat_rbcp_test(CISCO_SCP_LED_EFM) == FAILED) {
                          printf("Gshdsl RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else if (on_off ==2) {
                      if (plat_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                          printf("Gshdsl RBCP LED Test Failed");
                          return (FAILED);
                      }
                } else {
                       return (PASSED);
                }
                 msleep(LED_WAIT_TIME);
            break;
            case GSHDSL_L0_LED:
                 on_off = gethex_answer("\n1.Turn L0 Green On\n2.Turn L0 Yellow On\n"
                                        "3.Turn L0 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L0_G) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L0_Y) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (plat_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep(LED_WAIT_TIME);
                 break;
            case GSHDSL_L1_LED:
                 on_off = gethex_answer("\n1.Turn L1 Green On\n2.Turn L1 Yellow On\n"
                                        "3.Turn L1 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L1_G) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L1_Y) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (plat_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep(LED_WAIT_TIME);
                 break;
            case GSHDSL_L2_LED:
                 on_off = gethex_answer("\n1.Turn L2 Green On\n2.Turn L2 Yellow On\n"
                                        "3.Turn L2 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L2_G) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L2_Y) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (plat_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep(LED_WAIT_TIME);
                 break;
            case GSHDSL_L3_LED:
                 on_off = gethex_answer("\n1.Turn L3 Green On\n2.Turn L3 Yellow On\n"
                                        "3.Turn L3 Off\n4.Exit\n",0, 0, 4);
                 if (on_off == 1) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L3_G) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 2) {
                       if (plat_rbcp_test(CISCO_SCP_LED_L3_Y) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else if (on_off == 3) {
                       if (plat_rbcp_test(CISCO_SCP_LED_OFF) == FAILED) {
                           printf("Gshdsl RBCP LED Test Failed");
                           return (FAILED);
                       }
                 } else {
                       return (PASSED);
                 }
                 msleep(LED_WAIT_TIME);
                 break;
            default:
                 printf("error#");
                 break;
     }
     return (PASSED);

}

/*-------------------------------------------------
 * $Log: diag_rbcp_test.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
