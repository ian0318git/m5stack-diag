/* $Id: lebowski_host.c,v 1.16 2018/05/22 02:31:10 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_host.c,v $
 *------------------------------------------------------------------
 *
 * lebowski_host.c: main source file for Lebowski host diag.
 *
 * Feb. 2012 - Ian Chang(Ported from EagleEye)
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Charlie Yao
 */
 
/*------------------------------------------------------------------------------
 * includes
 *------------------------------------------------------------------------------
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <pthread.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "strings.h"
#include "signals.h"
#include "nvmonvars.h"
#include "linux_api.h"
#include "proto.h"
#include "error.h"
#include "free.h"
#include "cpu.h"
#include "menu.h"
#include "pm_utils.h"
#include "queryflags.h"
#include "mon_plat_defs.h"
#include "slot.h"
#include "sm_slot.h"
#include "cross_platform.h"
#include "pci.h"
#include "dev_object.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "lebowski_host.h"
#include "sgmii_defs.h"
#include "plat_defs.h"
#include "ethernet.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "pca.h"
#include <stdio.h>
#include <assert.h>
#include "platform_poe_psu.h"

#define LEBOWSKI_TEST_ALL_TEST      MIRAGE_TEST_ALL_TEST
#define LEBOWSKI_TEST_MAX_WAIT_TIME 5000
#define LEBOWSKI_WAIT_TIME_PER_TEST 120
#define LEBOWSKI_WAIT_RETRY         80
/*------------------------------------------------------------------------------
 * globals
 *------------------------------------------------------------------------------
 */
static void (*lebowski_saved_diag_exec)(void) = NULL;
static int (*savfcn)() = NULL;
static boolean lebowski_console_disp;
static lebowski_ds_t lebowski_info;
static lebowski_ds_t *lebowski_iface_p;
lebowski_ds_t lebowski_iface[MAX_SM+1];
static int socket_gl;
static fe_packet_t recv_packet;
static fe_packet_t * recv_packet_p = &recv_packet;
static fe_packet_t cmd_packet;
static fe_packet_t *cmd_packet_p = &cmd_packet;
static fe_packet_t result_packet;
static fe_packet_t *result_packet_p = &result_packet;
static mac_addr_t bcast_mac_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0xf0};
static mac_addr_t src_mac_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1a};
static int main_thread_wait_time = 0;
static short linepos;
static speed_t slot1_uart = LEBOWSKI_B9600;
static speed_t slot2_uart = LEBOWSKI_B9600;
static speed_t slot3_uart = LEBOWSKI_B9600;

/*------------------------------------------------------------------------------
 * prototypes
 *------------------------------------------------------------------------------
 */
static int  util_lebowski_sm_reset(void);
static int  util_lebowski_sm_power_off(void);
static int  util_lebowski_sm_pwr_off(void);
static int  util_lebowski_sm_pwr_on(void);
static int  util_lebowski_sm_pwr_cycle(void);
static int  util_lebowski_overlord_gesw(void);
static int  util_lebowski_uart_baud_rate_set(void);
static int  util_lebowski_mc_reset(void);
static int  util_lebowski_sm_mode_switch(void);
static int  util_lebowski_sm_config_reset(void);
static void lebowski_mc_reset(void);
static void *lebowski_poll_rx_data(void *);
static int  lebowski_load_sm_diag(lebowski_ds_t *lebowski_ptr);
static int  lebowski_all_tests(lebowski_ds_t *lebowski_ptr);
static void lebowski_sm_cleanup(void);
static int  lebowski_lebowski_sm_exit(int show_menu);
static int  lebowski_sm_console_switch(int show_menu);
static int  lebowski_host_utility(int show_menu);
static int  lebowski_lebowski_sm_test(int show_menu);
static int  lebowski_sm_init(void);
static int  ltc4215_register_test(void);
static int  ltc4215_led_test(void);
static int  ltc4215_oir_test(int show_menu);
static int  util_ltc4215_reg_read(void);
static int  util_ltc4215_reg_write(void);
static int  util_lebowski_sm_disp_pwr(void);
static uint32_t lebowski_sm_get_current(uint8_t data);
static int lebowski_setup_ge_env(void);
static int lebowski_cleanup_ge_env(void);
static long configure_i2c_expander(void);
static int lebowski_i2c_ioe_reg_test(void);
static void lebowski_print_spining_wheel(int);
static void setup_uart(void);
static void lebowski_clrline(char *);

/*------------------------------------------------------------------------------
 * constants
 *------------------------------------------------------------------------------
 */
static uart_baud_info lebowski_uart_baud[] = {
    {"115200",   B115200},
    {"9600",     B9600}
};


/* 
 * Sub Menu used for Utility.
 */
static mitem_t lebowski_util_submenu_table[] = {
    { "LTC4215 Register Read",          0, 0,   (PFT)util_ltc4215_reg_read,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "LTC4215 Register Write",         0, 0,   (PFT)util_ltc4215_reg_write,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Display Lebowski SM Power Meas",	0, 0,	(PFT)util_lebowski_sm_disp_pwr,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Toggle Lebowski SM Mode Switch", 0, 0,   (PFT)util_lebowski_sm_mode_switch,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski SM Config/Passwd Reset",        0, 0,
      (PFT)util_lebowski_sm_config_reset,    (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski SM Yeti3 Reset",        0, 0,
      (PFT)util_lebowski_mc_reset,           (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski SM Switch Reset",       0, 0,   (PFT)util_lebowski_sm_reset,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski SM Switch Power Off",   0, 0,   (PFT)util_lebowski_sm_power_off,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski SM Switch Power On",    0, 0,   (PFT)util_lebowski_sm_pwr_on,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski SM Switch Power Cycle", 0, 0,   (PFT)util_lebowski_sm_pwr_cycle,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski Switch Overlord GESW", 0, 0,    (PFT)util_lebowski_overlord_gesw,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Lebowski UART Baud Rate Set", 0, 0, (PFT)util_lebowski_uart_baud_rate_set,
      (type_t *)&zero, 0, (type_t(*)())0, 0 },
};

#define LEBOWSKI_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(lebowski_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t lebowski_util_subtest_menu = {
    "Lebowski Host Utilities Menu",
    (type_t)0,                                        /* title param */
    (PFT)menu_show_dflags,                         /* show diag flags */
    0,
    LEBOWSKI_UTIL_SUBMENU_TABLE_SZ,
    lebowski_util_submenu_table,
};

static menuinfo_t *lebowski_util_submenup = &lebowski_util_subtest_menu;

/* 
 * Sub Menu used for OIR LTC4215 tests.
 */
static submenu_xtable_t oir_submenu_table[] = {
    {"OIR (LTC4215) Register Test",     (PFT)ltc4215_register_test,  0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,   0,      (type_t(*)())0,   0},
    {"OIR LED Test",                    (PFT)ltc4215_led_test,       0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,   0,      (type_t(*)())0,   0},
};

#define OIR_SUBMENU_TABLE_SIZE (sizeof(oir_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t oir_tests_primary_items[OIR_SUBMENU_TABLE_SIZE + 
                                       MAX_BASE_ITEMS];
static mitem_t oir_tests_secondary_items[OIR_SUBMENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static menuinfo_t oir_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    oir_tests_primary_items,
};

static menuinfo_t *oir_submenup = &oir_subtest_menu;

/* 
 * Sub Menu used for Lebowski Host side tests.
 */
static submenu_xtable_t lebowski_submenu_table[] = {
    {"OIR (LTC4215) Test",      (PFT)ltc4215_oir_test,  0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0, 0,            (PFT)ltc4215_oir_test,  TRUE},
    {"Lebowski SM Test",        (PFT)lebowski_lebowski_sm_test,  0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,            (type_t(*)())0, 0},
    {"Lebowski Host Utilities", (PFT)lebowski_host_utility, 0,
     0,		(type_t(*)())0, 0,     (PFT)lebowski_host_utility, 0},
    {"Lebowski Switch Console", (PFT)lebowski_sm_console_switch, 0,
     0,     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Lebowski Exit Host_Lebowski Communication",
     (PFT)lebowski_lebowski_sm_exit, 0,
     0,     (type_t(*)())0, 0,     (type_t(*)())0, 0},
};

#define LEBOWSKI_SUBMENU_TABLE_SIZE (sizeof(lebowski_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lebowski_tests_primary_items[LEBOWSKI_SUBMENU_TABLE_SIZE + 
                                            MAX_BASE_ITEMS];
static mitem_t lebowski_tests_secondary_items[LEBOWSKI_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

static menuinfo_t lebowski_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    lebowski_tests_primary_items,
};

static menuinfo_t *lebowski_submenup = &lebowski_subtest_menu;

static n2g_i2c_if_t pca_i2c[] = {
    {
        .i2c_dev = SM_I2C_ADDR_IO_PORT,
    },
    {
        .i2c_dev = PCA9555_I2C_ADDRESS,
    },
};

static char pca_buff0[256];
static n2g_i2c_if_t *oir;

/*------------------------------------------------------------------------------
 * functions
 *------------------------------------------------------------------------------
 */
/***********************************************************************
 * Name: lebowski_clear_rx_buf
 *
 * Description:
 *      Clear receiver buffer before testing.
 *
 * Input: NONE.
 *
 * Output: NONE.
 *
 ***********************************************************************
*/
void
lebowski_clear_rx_buf(void)
{
    uchar *c_ptr = (uchar *)result_packet_p;
    int i;

    for (i = 0; i < sizeof(fe_packet_t); i++) {
        *c_ptr++ = 0;
    }
    return;
}

/*
 **********************************************************************
 *
 *  Function: lebowski_wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets
 *
 *  Input: pak - received packet buffer
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int
lebowski_wait_for_ge_packet(uchar *pak)
{
    int wait_count = 10000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status;
    int ix;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

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

    /* now wait for */
    status = eth_pkt_rx(rx_pkt_p);

    if (status) {
        if (diagflag_xram & D_SET_OPTIONS) {
        	memcpy ((char *)pak, (uchar *)recv_buffer, sizeof(fe_packet_t));
            printf("status = %x", status); fflush(0);
            printf("\nRx data :\n"); fflush(0);
            dismem((unsigned char *)(pak), 0x40,
                (unsigned long)(pak), 1);
        }
        return (FAILED); /* retry is provided by caller */
    };
    /* copy received to user pak */
    memcpy ((char *)pak, (uchar *)recv_buffer, sizeof(fe_packet_t));

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nRx data :");
        dismem((unsigned char *)(pak), 0x40,(unsigned long)(pak), 1);
    }
    /* SM response packet type 0xCCCC*/
    if ((recv_buffer[12] != 0xCC ) && (recv_buffer[13] != 0xCC )) {
        return (FAILED);
    }
    /* Avoid the interference from Broadcast packet */
    for (ix = 0; ix < 6; ix++) {
        if (recv_buffer[ix] == 0xff)
        {
            continue;
        }
        break;
    }
    if (ix == 6) {
        printf("Skip Broadcast Packet MAC Address :"
                "0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x \n",
                recv_buffer[0],
                recv_buffer[1],
                recv_buffer[2],
                recv_buffer[3],
                recv_buffer[4],
                recv_buffer[5]);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nRx data :");
            dismem((unsigned char *)(pak), 0x40, (unsigned long)(pak), 1);
        }
        return (FAILED);
    }
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: lebowski_send_command_packet
 *
 *  Description: This function sends a packet with the command to
 *               Lebowski SM
 *
 *  Input: iface - Patriot data structure info pointer
 *         cmd - command
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int lebowski_send_command_packet (lebowski_ds_t *iface, uchar cmd, int param)
{
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    uchar recv_frame_p[2048] = {0};
    int ret_val = PASSED,wait_time = 0;
    ushort pkt_type  = 0xffff; //0x0800;
    lebowski_ds_t    *lebowski_info_p;
    lebowski_info_p  = &lebowski_info;
    int real_slot    = lebowski_info_p->slot;
    
    get_host_mac_addr(0, (uchar *)&src_mac_addr[0]);

    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));
    memset((uchar *)cmd_packet_p, 0, sizeof(fe_packet_t));

    cmd_packet_p->data[0] = 0xbbbbbbbb;
    cmd_packet_p->data[1] = (cmd);
    cmd_packet_p->data[2] = (0);
    cmd_packet_p->data[3] = (0xBAD);
    cmd_packet_p->data[4] = (0x00112233);
    cmd_packet_p->data[5] = (0x44556677);
    cmd_packet_p->data[6] = (diagflag_xram);
    cmd_packet_p->data[7] = (DIAGFLAG);

    bcast_mac_addr[5] = 0xf0 + real_slot;

#ifdef DEBUG
    printf("\ncmd_packet_p->data[0] = 0x%02x\n", cmd_packet_p->data[0]);
    printf("\ncmd_packet_p->data[1] = 0x%02x\n", cmd_packet_p->data[1]);
    printf("\ncmd_packet_p->data[6] = 0x%02x\n", cmd_packet_p->data[6]);
    printf("\ncmd_packet_p->data[7] = 0x%02x\n", cmd_packet_p->data[7]);
#endif
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr), bcast_mac_addr,
	   sizeof(mac_addr_t));
    memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)src_mac_addr,
	   sizeof(mac_addr_t));

    tx_pkt_p->pkt_type = pkt_type;
    tx_pkt_p->payload_size = sizeof(fe_packet_t) - sizeof(ether_hdr_t) - 4;
    tx_pkt_p->bufr_st_addr = (uchar *)&cmd_packet_p->pad_data[0];
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;
#ifdef DEBUG
    printf("\ntx_pkt_p->pkt_type = 0x%04x", tx_pkt_p->pkt_type);
    printf("\ntx_pkt_p->tx_status  = 0x%04x", tx_pkt_p->tx_status);
    printf("\ntx_pkt_p->payload_size = 0x%04x", tx_pkt_p->payload_size);
    printf("\ntx_pkt_p->bufr_st_addr = 0x%08x", tx_pkt_p->bufr_st_addr);
    dismem((uchar *)tx_pkt_p, 0x40, (ulong)tx_pkt_p, 0x04);
#endif
    ret_val = eth_pkt_tx(tx_pkt_p);
    if (ret_val != ETH_PKT_TX_OK ) {
	cterr('f', 0, "%s: Failed send command : ret = 0x%x, status = 0x%x",
	      __FUNCTION__, ret_val, tx_pkt_p->tx_status);
        return (FAILED);
    }

    wait_time = 800;
    while ((ret_val = lebowski_wait_for_ge_packet(recv_frame_p)) == FAILED) {
        if (--wait_time <= 0) {
    	    printf("\nTimeout on receiving frame, wait_time = %d\n",wait_time);
	        return (FAILED);
        }
        msleep(10);
    }
    recv_packet_p = (fe_packet_t *)recv_frame_p;
    if (ret_val == PASSED) {
        printf("\nReceive packet (ACK) 0x%02x ",recv_packet_p->data[1]);
	    if (recv_packet_p->data[1] != CMD_ACK) {
	        cterr('f', 0,"Wrong response, expected = 0x%02x, received = 0x%02x",
	    		    CMD_ACK, recv_packet_p->data[1]);
	        return (FAILED);
	    }
    }
    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: lebowski_rcv_cmd_result_packet
 *
 *  Description: This function wait for the result packet after
 *               the command has been sent
 *
 *  Input: iface - Lebowski data structure info pointer
 *         cmd - command
 *
 *  Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int
lebowski_rcv_cmd_result_packet(lebowski_ds_t *lebowski_ptr, uchar cmd)
{
    uchar recv_frame_p[2048];
    int ret_val = PASSED, wait_time = 0, fd = 0;;
    int recv_data = 0;
    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));
    pthread_t threads;
    lebowski_ds_t    *lebowski_info_p;
    lebowski_info_p  = &lebowski_info;
    const int maxlen = 128;
    char tty[maxlen];

    if (cmd == LEBOWSKI_TEST_ALL_TEST) {
        snprintf(tty, maxlen-1, "/dev/ttyDASH%d", lebowski_iface_p->uart);
        fd = open(tty, O_RDWR|O_NOCTTY);
        main_thread_wait_time = LEBOWSKI_TEST_MAX_WAIT_TIME;
        /* if Lebowski is 48 Port, double the wait time */ 
        if (lebowski_ptr->num_asic > 1) { 
            wait_time = wait_time * 2;
        }
        if(pthread_create(&threads, NULL, lebowski_poll_rx_data, (void *)&fd)) {
            perror("pthread_create failed.");
            printf("%s: pthread_create failed.\n", __FUNCTION__);
            close(fd);
            return FAILED;
        }

        while (--main_thread_wait_time ) {
            if ((ret_val = lebowski_wait_for_ge_packet(recv_frame_p)) == PASSED)
            {
                recv_packet_p = (fe_packet_t *)recv_frame_p;
                recv_data = recv_packet_p->data[1];
                printf("\nReceive packet (Result) 0x%02x \n",recv_packet_p->data[1]);
                break;
            }
            msleep(10);
        }
#ifdef DEBUG
        printf("\nwait_time (%d) \n",wait_time);
#endif
        if (pthread_cancel(threads)!=0) {
            printf("pthread_cancel error");
        }
        close(fd);
    }
    if ((cmd == NO_CMD) || (cmd == EXIT)) {
        wait_time = 500;
        while ((ret_val = lebowski_wait_for_ge_packet(recv_frame_p)) == FAILED) 
        {
            if (--wait_time <= 0) {
    	        printf("\nTimeout on receiving frame \n");
		        return (FAILED);
            }
            msleep(10);
        }
        recv_packet_p = (fe_packet_t *)recv_frame_p;
        recv_data = recv_packet_p->data[1];
        printf("\nReceive packet (Result) 0x%02x \n",recv_packet_p->data[1]);
    }
    if (recv_data != TEST_DONE) {
        if (recv_data != TEST_FAIL) {
            cterr('f', 0,"SM Wrong Response, received = 0x%02x",
                   recv_packet_p->data[1]);
            return (FAILED);
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nRx TEST_FAIL :\n");
            dismem((unsigned char *)(recv_packet_p), 0x100, (unsigned long)(recv_packet_p), 1);
        }    
        cterr('f', 0,"Lebowski SM %d : %s",lebowski_info_p->slot, 
                                          &recv_packet_p->data[8]);
        return (FAILED);
    }
    return (PASSED);
}
/**********************************************************************
 *
 * Function: lebowski_send_cmd
 *
 * This function sends commands to the Lebowski SM
 *
 * Input : iface - Patriot data structure info pointer
 *         cmd - command
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
lebowski_send_cmd(lebowski_ds_t *iface, uchar cmd, int param)
{
    int retval = PASSED;

    if (lebowski_send_command_packet(iface, cmd, param)) {
	    retval = FAILED;
    }
    return retval;
}
/**********************************************************************
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
static int
ltc4215_register_test (void)
{
    testname("LTC4215 OIR Register");

    return(oir_ltc4215_register_test(oir));
}

/**********************************************************************
 *
 * Function: ltc4215_led_test
 *
 * Description: A wrapper function for LTC4215 LED test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_led_test (void)
{
    testname("LTC4215 OIR LED");

    return(oir_ltc4215_leds_test(oir));
}

/*------------------------------------------------------------------------------
 *
 * Function: ltc4215_oir_test().
 *
 * This function implements the ltc4215 oir test/menu for main menu.
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
ltc4215_oir_test (int show_menu)
{
    build_primary_submenu(oir_submenu_table, 
                          OIR_SUBMENU_TABLE_SIZE,
                          "LTC4215 OIR", &oir_submenup);
    build_secondary_submenu(oir_submenu_table,
                            OIR_SUBMENU_TABLE_SIZE,
                            oir_tests_secondary_items);

    if (show_menu) {
        menu(oir_submenup, oir_tests_secondary_items, '\0' );
    } else {
        do_all_menu_items(oir_submenup);
    }

    testname("LTC4215 OIR");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_ltc4215_reg_write (void)
{
    return(util_oir_ltc4215_reg_write(oir));
}

/**********************************************************************
 *
 * Function: util_ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_ltc4215_reg_read (void)
{
    return(util_oir_ltc4215_reg_read((void *)oir));
}

/**********************************************************************
 *
 * Function: util_lebowski_sm_disp_pwr
 *
 * Description: Display power of SM.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_sm_disp_pwr (void)
{
    uint32_t voltage, current, power;
    uint8_t data = 0;

    printf("\n\nLebowski SM Power Measure:\n\n");

    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data)) {
        return(FAILED);
    }
    voltage = (data * SINGLE_SM_VOL) / 100;

    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data)) {
        return(FAILED);
    }
    current = lebowski_sm_get_current(data) / 100;

    power = voltage * current;

    printf("Voltage = %d.%02d V\n", (voltage / 100), (voltage % 100));
    printf("Current = %d.%02d A\n", (current / 100), (current % 100));
    printf("Power = %d.%02d W\n", (power / 10000), ((power % 10000) / 100));

    return (PASSED);
}

/**********************************************************************
 *
 * Function: lebowski_sm_get_current
 *
 * Description: convert sense register value into current.
 *
 * Input : Sense Register value.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static uint32_t
lebowski_sm_get_current (uint8_t data)
{
    lebowski_ds_t *lebowski_info_p = &lebowski_info;
    ushort cookie_id = lebowski_info_p->cookie_id;
    uint32_t current;

    if (data) {
        switch (cookie_id) {
        case LEBOWSKI_X_ES3D_48_P_ID:
            current = (data - 1) * DWIDE_SM_CURRENT;
            break;
        default:
            current = (data - 1) * SINGLE_SM_CURRENT;
        }
    } else {
        current = 0;
    }

    return (current);
}

/**********************************************************************
 *
 * Function: util_lebowski_sm_power_off
 *
 * Description: This function is a warper of power off Lebowski SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_sm_power_off (void)
{
    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Lebowski SM Switch Still Powered On.\n\n");
        return (PASSED);
    }

    return (util_lebowski_sm_pwr_off());
}

/**********************************************************************
 *
 * Function: util_lebowski_sm_pwr_off
 *
 * Description: This function power off Lebowski SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_sm_pwr_off (void)
{
    uint8_t data = 0;

    printf("\nPower Off the Lebowski SM.\n");

    if (util_oir_ltc4215_led(oir, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    // power off sm module
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    printf("\nLebowski SM is turned off.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_lebowski_sm_pwr_on
 *
 * Description: This function power on Lebowski SM.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_sm_pwr_on (void)
{
    uint8_t data = 0;

    printf("\nPower On the Lebowski SM.\n");

    assert(oir);
    assert(lebowski_iface_p);

    /* turn on board power and take I2C out of reset */
    if (slot_i2c_unreset(lebowski_iface_p->lebowski_sm_iface , lebowski_iface_p->slot, "SM")) {
        return (FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    // power on sm module
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        return (FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return (FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return (FAILED);
    }

    printf("Waiting for Lebowski SM to Power-Up.\n");
    msleep(2000);

    printf("Reinitializing the Lebowski SM.\n");

    if (lebowski_sm_init()) {
        lebowski_sm_cleanup();
        return (FAILED);
    }

    // turn on the green light status if PSE2 re-init successfully
    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    printf("\nLebowski SM is turned on.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_lebowski_sm_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_sm_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    testname("Power Cycle the Lebowski SM");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Lebowski SM Switch is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (util_lebowski_sm_pwr_off()) {
        cterr('f', 0, "Failed on Power Off the Lebowski SM");
        return(FAILED);
    }

    // msleep for 10 seconds.
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }
    printf("\n");

    if (util_lebowski_sm_pwr_on()) {
        cterr('f', 0, "Failed on Power On the Lebowski SM");
        return(FAILED);
    }

    return(PASSED);
}
/**********************************************************************
 *
 * Function: util_lebowski_overlord_gesw
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_overlord_gesw (void)
{
    uint8_t ans;
    uint    port;
    int     ge_port, ge_port0, ge_port1;
    lebowski_ds_t     *lebowski_info_p;
    lebowski_info_p       = &lebowski_info;
    int real_slot = lebowski_info_p->slot;

    printf("\n");
    testname("Swtich the Overlord GESW");

    port = gethex_answer("Overlord GESW port E0/E1", 0, 0, 1);
    if (port) {
        ge_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1); //BP PHY
    } else {	
        ge_port = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0); //Serdes Mux
    } 
    printf("\nEnable the backplane loopback? (y/n) ");
    fflush(0);
    ans = getchar();
    if (ans != 'y' && ans != 'Y') {
        set_gesw_line_loopback(ge_port, 0);
    } else {
        set_gesw_line_loopback(ge_port, 1);
    }
    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0);
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1);
    printf("\nSM slot %d, GE0 : GESW port %d Line Loopback = %s",real_slot, 
            ge_port0 ,get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
    printf("\nSM slot %d, GE1 : GESW port %d Line Loopback = %s\n",real_slot, 
            ge_port1 ,get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
    fflush(0);

    return(PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: util_lebowski_uart_baud_rate_set().
 *
 * This function sets the Yeti3 uart baud rate
 *
 * Input : None 
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
util_lebowski_uart_baud_rate_set (void)
{
    const   int maxlen = 128;
    char    tty[maxlen];
    int     fd, slot;
    struct  termios  newtio, ori_conf;
    lebowski_ds_t   *iface;
    speed_t new_baud = 0, uart_baud_rate = 0;

    iface = lebowski_iface_p;
    assert(iface);
    slot = lebowski_iface_p->slot;

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", lebowski_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }
    tcgetattr(fd, &newtio);

    printf("\n\n Set Yeti3 UART Baud Rate: \n");
    uart_baud_rate = getdec_answer("\nBaudrate (0-115200, 1-9600):", 0, 0, 1);
    if (slot == LEBOWSKI_SLOT1) {
        slot1_uart = uart_baud_rate;
    } else if (slot == LEBOWSKI_SLOT2) {
        slot2_uart = uart_baud_rate;
    } else {
        slot3_uart = uart_baud_rate;
    }    
    
    new_baud = lebowski_uart_baud[uart_baud_rate].baud_rate;

    /* Backup default config for recover after test */
    memcpy(&ori_conf, &newtio, sizeof(newtio));

    if ((newtio.c_cflag & CBAUD) != new_baud) {
        if (cfsetospeed(&newtio, new_baud) < 0) {
            tcsetattr(fd, TCSAFLUSH, &ori_conf);
            close(fd);
            cterr('f', 0, "Failed to set output speed.");
            return (FAILED);
        }

        if (cfsetispeed(&newtio, new_baud) < 0) {
            tcsetattr(fd, TCSAFLUSH, &ori_conf);
            close(fd);
            cterr('f', 0, "Failed to set intput speed.");
            return (FAILED);
        }
    }
    close(fd);
    return (PASSED);
}
/**********************************************************************
 *
 * Function: util_lebowski_sm_reset
 *
 * Description: This function resets Lebowski SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_sm_reset (void)
{

    printf("\n\nLebowski SM Reset.\n\n");

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    lebowski_mc_reset();

    /* Pull reset then release the reset*/
    msleep(500);

    printf("Reinitialize Lebowski SM.\n\n");

    if (lebowski_sm_init()) {
        lebowski_sm_cleanup();
        return(FAILED);
    }

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_lebowski_mc_reset
 *
 * Description: This function resets Lebowski Yeti3.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_lebowski_mc_reset (void)
{
    printf("\n\nLebowski SM Yeti3 Reset.\n\n");

    lebowski_mc_reset();

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_lebowski_mode_switch
 *
 * Description: This function toggles Lebowski mode switch.
 *
 * Input :  None
 *
 * Output: None
 *
 **********************************************************************
 */
static int
util_lebowski_sm_mode_switch (void)
{
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];
    static uint32_t enable = ENABLE;

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
        return (FAILED);
    }
        if (enable) {
            data &= ~((1 << PCA9555_MODE_REGISTER));
            if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
                return (FAILED);
            }
            enable = DISABLE;
            printf("\n\nThe Lebowski SM Mode Switch is set to ON (Low).\n");
        } else {
            data |= ((1 << PCA9555_MODE_REGISTER));
            if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
                return (FAILED);
            }
            enable = ENABLE;
            printf("\n\nThe Lebowski SM Mode Switch is set to OFF (High).\n");
        }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_lebowski_sm_config_reset
 *
 * Description: This function resets the configuration of Lebowski switch.
 *
 * Input :  None
 *
 * Output: None
 *
 **********************************************************************
 */
static int
util_lebowski_sm_config_reset (void)
{
    int i;
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];

    printf("\n\nReset the Lebowski SM Configuration/Password.\n");
    printf("This step will take about 50 seconds to finish.\n");

    printf("\nTriggering the Mode Switch.\n");
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG,  &data, TRUE)) {
        return (FAILED);
    }
    data &= ~((1 << PCA9555_MODE_REGISTER));
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        return (FAILED);
    }
    printf("Resetting the Lebowski SM MicroController.\n");
    lebowski_mc_reset();

    printf("Booting Up the Lebowski SM Switch.\n");
    for (i = 0; i < YETI3_CONFIG_RESET_TIME; i++) {
        printf(".");
        msleep(1000);
    }
    printf("\n");
    data |= ((1 << PCA9555_MODE_REGISTER));
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
        return (FAILED);
    }
    printf("Releasing the Mode Switch.\n");
    printf("Done.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: lebowski_mc_reset
 *
 * Description: This function resets Lebowski Microcontroller.
 *
 * Input :  None
 *
 * Output: None
 *
 **********************************************************************
 */
static void
lebowski_mc_reset (void)
{
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];

    /* Yeti3 GPIO reset is inverted. Keep in reset (high) for 
     * 180 msec minimum, then un-reset (low) and wait 500 msec 
     * minimum before proceeding */

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG,  &data, TRUE)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }

    /* Yeti3 GPIO reset is inverted. Keep in reset (high) for 
     * 180 msec minimum, then un-reset (low) and wait 500 msec 
     * minimum before proceeding */

    data &= ~((1 << PCA9555_SYS_RET_REGISTER));
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    msleep(500);
    data |= ((1 << PCA9555_SYS_RET_REGISTER));
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    msleep(500);
}


/***********************************************************************
 * Name: lebowski_setup_ge_env
 *
 * Description:
 *      This test will set up GE operation environment. 
 *
 * Input: iface - Lebowski data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
static int lebowski_setup_ge_env(void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];
    
    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
        cterr('f', 0, "Setup: Failed to get sgmii port number.");
        return (FAILED);
    }
    sprintf(if_name, "eth%d", sgmii_port);
    status = setup_eth_dev(if_name, &socket_gl);
#ifdef DEBUG
    printf("\nsocket_gl = 0x%02x\n", socket_gl);
#endif
    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    return (PASSED);
}
/***********************************************************************
 * Name: lebowski_cleanup_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: iface - Patriot data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
static int lebowski_cleanup_ge_env(void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

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
/*****************************************************************
 *
 * Function: setup_uart()
 *
 * Description: Setup UART Interface Parameter
 *  
 * Input:  None
 *
 * Output: None 
 *
 *****************************************************************
 */
static void
setup_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd, slot;
    struct termios oldtio, newtio;
    lebowski_ds_t   *iface;
    int new_baud = 0;

    iface = lebowski_iface_p;
    assert(iface);
    slot = lebowski_iface_p->slot;

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", lebowski_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    /* Get current BAUD setting */
    if (slot == LEBOWSKI_SLOT1) {
        new_baud = slot1_uart;
    } else if (slot == LEBOWSKI_SLOT2) {
        new_baud = slot2_uart;
    } else if (slot == LEBOWSKI_SLOT3) {
        new_baud = slot3_uart;
    } else { 
        cterr('f',0,"Invalid slot number: %d.", slot);
        return;
    }
    
    if ( new_baud == LEBOWSKI_B115200) {
        newtio.c_cflag = B115200|CS8|CLOCAL|CREAD; /* control mode flags */
    } else if (new_baud == LEBOWSKI_B9600) {
        newtio.c_cflag = B9600|CS8|CLOCAL|CREAD;   /* control mode flags */
    } else {
        cterr('f',0,"Invalid Baud: %d.", new_baud);
        return;
    }

    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    newtio.c_iflag = IGNPAR | ICRNL;  /* input mode flags  */
    newtio.c_oflag = 0;               /* output mode flags */
    newtio.c_lflag = ICANON;          /* local mode flags  */
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    close(fd);
    return;
}
/*------------------------------------------------------------------------------
 *
 * Function: lebowski_uart_send_info().
 *
 * This function sends slot number to SM side via UART interface
 *
 * Input:  slot
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
lebowski_uart_send_info (int slot)
{
    int  ret_val = PASS;
    char uart_buff[2];      //CSCum79984
    const int maxlen = 128;
    char tty[maxlen];
    int fd;

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", lebowski_iface_p->uart);
    uart_buff[0] = slot;
    uart_buff[1] = '\r';
    printf("\nHost send slot (%x) to SM:  \n",uart_buff[0]);
    fd = open(tty, O_RDWR|O_NOCTTY);
    ret_val = write(fd, uart_buff, strlen(uart_buff));
    close(fd);

    msleep(1000 * 3);
    return (ret_val);
}
/*****************************************************************
 *
 * Function: lebowski_poll_rx_data()
 *
 * Description: polls for any incoming traffic. The wait time (timeout)
 *              is in seconds.
 *  
 * Input:  lebowski instance
 * Output: PASS/FAIL
 *
 *****************************************************************
 */
static void
* lebowski_poll_rx_data (void *fd)
{
    char waiting_str[] = "Waiting for command from host";
    int res;
    char buf[256];
    /*
     * we are probing status every 2 sec (min) and the timeout
     * is in second.
     */
    while(1) {
        res = read(*(int *)fd, buf, 255);
        if (lebowski_console_disp == TRUE) {
            if (res > 1) {
            lebowski_clrline(buf);
#ifdef DEBUG
              printf("(%d)%s\n", res, buf);
#endif
                /* update the main thread wait time */
                main_thread_wait_time = LEBOWSKI_TEST_MAX_WAIT_TIME;
            }
            if (strstr(buf,waiting_str) != NULL) {
                printf("\nFind : %s\n", waiting_str);
                /* tell main thread time should be don */
                main_thread_wait_time = 5;
                lebowski_console_disp = FALSE;
                break;
            }
        }
    }
    pthread_exit(NULL);
}

/**********************************************************************
 *
 * Function: lebowski_clrline()
 *
 * Description: This function checks input string for newline, clears it
 *
 * Input:  string to check for newline
 *
 * Output: void
 *
 **********************************************************************
 */
static void
lebowski_clrline(char *string)
{
    int     ix;  /* get line position */
    char    *cptr;
    int     length;

    length = strlen(string);
    ix = linepos;
    cptr = strchr(string, '\n');  /* does the string have a newline? */
    if(cptr) {
        *cptr = '\0';  /* replace the first one with NULL */
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nlength(%d)", length);
            dismem((unsigned char *)(string), length + 4, (unsigned long)(string), 1);
        }
    }
    putchar('\r');
    ix -= printf(string); /* ix = strlen(previous) - strlen(current) */
    fflush(0);
    linepos = length;
    if(ix > 0) {  /* last line was longer - wipe the rest of it */
        while(ix --) { 
            putchar(' '); /* clear rest of line */
        }
    }
    printf("\r");
}
/*****************************************************************
 *
 * Function: lebowski_print_spining_wheel()
 *
 * Description: Display the spining wheel during the waiting time.
 *  
 * Input:  Ring cycle
 * Output: none
 *
 *****************************************************************
 */
static void lebowski_print_spining_wheel (int pass)
{
    printf("\b");
    switch (pass%8) {
    case 0:
        printf("|");
        break;
    case 1:
        printf("/");
        break;
    case 2:
        printf("-");
        break;
    case 3:
        printf("\\");
        break;
    case 4:
        printf("|");
        break;
    case 5:
        printf("/");
        break;
    case 6:
        printf("-");
        break;
    case 7:
        printf("\\");
        break;
    default:
        break;
    }
    fflush(stdout);
    printf("\r");
}

/* **********************************************************************
 *
 *  Function: lebowski_load_sm_diag
 *
 *  Description: This function resets the SM then boots diag and sets up
 *               Host-Lebowski communication
 *               It forwards the EXTERNAL and MIN_TEST flags to tne NM side.
 *
 *  Input: Lebowski Instance
 *
 *  Returns: 0 (PASSED) for no errors
 *           1 (FAILED) for test failure
 *
 **********************************************************************
 */
static int 
lebowski_load_sm_diag (lebowski_ds_t *lebowski_ptr)
{
    int ix;
    const int maxlen = 128;
    char tty[maxlen];
    char diagmon_str[] = "diagmon";
    char bootloader_str[] = "switch";
    char boot_buff[] = "boot flash:lebowskinet";
    char diag_buff[] = "menu";
    char test_buff[] = "f";
    char cret_buff[] = "\r";
    int fd, pass = 0;
    char buf[256];

    assert(lebowski_iface_p); 

    testname(lebowski_ptr->testname);

    /*
     * wait for Lebowski side boot up, Lebowski side take a while to
     * come up to switch: prompt. During the boot up period and reset Yeti3,
     * the Yeti3 hang up. It's a workaround.
     */
    pass = 0;
    for (ix = 0; ix < 45; ix++) {
        lebowski_print_spining_wheel(pass++);
        msleep(1000);
    }

    /*
     * reset Lebowski
     */
    lebowski_mc_reset();

    prpass(testpass, "Waiting for Lebowski Side to be Powered Up,");
    printf("\n");
    /*
     * wait for Lebowski side powered up, Lebowski side take a while to
     * come up to switch: prompt, 45 -60 sec depending on #files in flash.
     * for loop is needed because msleep value is greater than MAX_SLEEP_VALUE
     */
    for (ix = 0; ix < 90; ix++) {
        lebowski_print_spining_wheel(pass++);
        msleep(1000);
    }

    printf("Start UART...\n");
    snprintf(tty, maxlen-1, "/dev/ttyDASH%d", lebowski_iface_p->uart);
    fd = open(tty, O_RDWR|O_NOCTTY);

    /* Step 1 : Find "switch:" */
    for (ix = 0; ix < LEBOWSKI_WAIT_RETRY; ix++) {
        /*
         * CSCta31550: if partial prompt in rc_data,
         * send carriage return to Lebowski, so that it
         * will output another prompt string.
         */
        write(fd, cret_buff, strlen(cret_buff));
        read(fd, buf, 255);
        if (strstr(buf, bootloader_str) != NULL) {
            printf("\nFind : %s\n", bootloader_str);
            break;      /* proceed when prompt string found */
        }
        /* wait for Lebowski to get CR and respond with prompt */
        lebowski_print_spining_wheel(pass++);
        msleep(2000);
    }
    if (ix == LEBOWSKI_TEST_MAX_WAIT_TIME) {
        cterr('f',0,"Failed get bootloader prompt");
        close(fd);
        return (FAIL);
    }

    /* Step 2 : boot Lebowski diag image  */
    write(fd, boot_buff, strlen(boot_buff));
    printf("Send [%s]\n", boot_buff);
    fflush(0);
    msleep(5000);
    /* Step 3 : Find "diagmon"  */
    for (ix = 0; ix < LEBOWSKI_WAIT_RETRY; ix++) {
        write(fd, cret_buff, strlen(cret_buff));
        read(fd, buf, 255);
        if (strstr(buf, diagmon_str) != NULL) {
            printf("\nFind : %s\n", diagmon_str);
            break;      /* proceed when prompt string found */
        }
        lebowski_print_spining_wheel(pass++);
        msleep(1000);
    }
    /* Step 4 : Into Menu   */
    write(fd, diag_buff, strlen(diag_buff));
    printf("Send [%s]\n", diag_buff);
    fflush(0);
    write(fd, cret_buff, strlen(cret_buff));
    msleep(1000);
    /* Step 5 : build test command to setup Host-Lebowski commnication  */
    write(fd, test_buff, strlen(test_buff));
    printf("Send [%s]\n", test_buff);
    fflush(0);

    write(fd, cret_buff, strlen(cret_buff));

    prpass(testpass, "Waiting for Lebowski Side Host Communication Setup,");
    printf("\n");
    for (ix = 0; ix < lebowski_ptr->num_asic; ix++) {
        msleep(1000*10);
    }
    close(fd);
    return (PASS);

}
/**********************************************************************
 *
 *  Function: lebowski_all_tests
 *
 *  Description: sends cmd to Lebowski to run all tests.
 *
 *  Input : Lebowski Instance
 *
 *  Returns: PASS/FAIL
 *
 **********************************************************************
 */
static int
lebowski_all_tests (lebowski_ds_t *lebowski_ptr)
{
    lebowski_ds_t *lebowski_info_p = &lebowski_info;
    int retval = PASSED;

    prpass(testpass, "Running Lebowski All Tests");

    lebowski_clear_rx_buf();

    if (lebowski_send_cmd(lebowski_info_p, LEBOWSKI_TEST_ALL_TEST, 0)) {
        cterr('f', 0,"Test All Timeout On Receiving ACK");
        return (FAILED);
    }
    lebowski_clear_rx_buf();
    if (retval == PASSED) {
        if (lebowski_rcv_cmd_result_packet(lebowski_info_p, 
	                                        LEBOWSKI_TEST_ALL_TEST)) {
	        retval = FAILED;
	    }
    }
    return(retval);
}

/**********************************************************************
 * Function: lebowski_sm_cleanup()
 *
 * Description: This function perform the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void
lebowski_sm_cleanup (void)
{
    lebowski_ds_t     *lebowski_info_p = &lebowski_info;
    uint8_t data = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];

    int ge_port0, ge_port1;

    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG, &data, TRUE)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    data |= ((1 << PCA9555_SYS_RET_REGISTER));
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG, &data)) {
	    cterr('f', 0, "GPIO read failed Reg: %x \n", PCA9555_OUT_PORT0_REG);
        return;
    }
    msleep(500);

    /* Mask host side slot interrupt */

    ge_port0 = ovld_get_ge_sw_port_num(lebowski_info_p->slot, TGT_DEV_NGSM, 0);
    set_gesw_line_loopback(ge_port0, 0);
    ge_port1 = ovld_get_ge_sw_port_num(lebowski_info_p->slot, TGT_DEV_NGSM, 1);
    set_gesw_line_loopback(ge_port1, 0);

    if (savfcn) {
        savfcn = NULL;
    }

    if (lebowski_saved_diag_exec) {
        pre_diag_exec = lebowski_saved_diag_exec;
        lebowski_saved_diag_exec = NULL;
    }
}

/*------------------------------------------------------------------------------
 *
 * Function: lebowski_lebowski_sm_exit().
 *
 * This function sends cmd to Lebowski to exit Host-Lebowski communication
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
lebowski_lebowski_sm_exit (int menu_option)
{
    lebowski_ds_t *lebowski_info_p = &lebowski_info;
    int retval = PASSED;
    n2g_i2c_if_t *pca= &pca_i2c[0];
    uint8_t data = 0;

    prpass(testpass, "Running Lebowski Exit Host-Lebowski Communication");

    /* Check if the interface is ready. I2C Expander GIPIO-03 : IN_RDY */
    if (io_port_8bit_i2c_read(pca, PCA9555_IN_PORT0_REG, &data, TRUE)) {
        return (FAILED);
    }
    if ((data & BIT3) == 0) {
        printf("\nLebowski Module Is Not Ready\n");
    } else {
        printf("\nLebowski Module Is Ready\n");
    }   

    if (lebowski_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    lebowski_clear_rx_buf();

    lebowski_uart_send_info(lebowski_info_p->slot);

    if (lebowski_send_cmd(lebowski_info_p, EXIT, 0)) {
        cterr('f', 0, "Timeout on receiving ACK\n");
        retval = FAILED;
    }

    if (retval == PASSED) {
        if (lebowski_rcv_cmd_result_packet(lebowski_info_p, EXIT)) {
            cterr('f', 0, "Timeout on receiving result\n");
            retval = FAILED;
        }
    }

    if (lebowski_cleanup_ge_env() == FAILED) {
        retval = FAILED;
    }
    return(retval);
}

/*------------------------------------------------------------------------------
 *
 * Function: lebowski_lebowski_sm_test().
 *
 * This function implements the lebowski sm test
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
lebowski_lebowski_sm_test (int show_menu)
{
    lebowski_ds_t *lebowski_info_p = &lebowski_info;
    int  ret_val, wait_time = 6000;
    uint8_t data = 0;
    n2g_i2c_if_t *pca= &pca_i2c[0];
    testname("Lebowski SM");

    if (lebowski_setup_ge_env() == FAILED) {
        return (FAILED);
    }
    /* Check if the interface is ready. I2C Expander GIPIO-03 : IN_RDY */
    if (io_port_8bit_i2c_read(pca, PCA9555_IN_PORT0_REG, &data, TRUE)) {
        return (FAILED);
    }
    lebowski_console_disp = FALSE;
    lebowski_clear_rx_buf();

    /* Waiting for SM initialize */
    msleep(wait_time);
    /*
      * Sending ACK command to SM to see if the Host-SM communication
      * is up. If not, try to boot SM diags image and set up the Host-SM
      * communication.
      */
    if ((data & BIT3) == 0) {
        printf("\nLebowski Module Is Not Ready\n");
        if (lebowski_load_sm_diag(lebowski_info_p)) {
            return (FAIL);
        }
        /*
         * Host send test all command
         *
         * Clear pending data and enable interrupt so the console output
         * of Lebowski's side can be displayed on the router's console.
         */
        /* Waiting for SM initialize */
        msleep(3000);
        lebowski_uart_send_info(lebowski_info_p->slot);
        printf("\nHost send NO_CMD : %x", NO_CMD);
        if (lebowski_send_cmd(lebowski_info_p, NO_CMD, 0)) {
        }
    } else {
        printf("\nLebowski Module Is Ready\n");
        lebowski_uart_send_info(lebowski_info_p->slot);
        printf("\nHost send NO_CMD : %x", NO_CMD);
        if (lebowski_send_cmd(lebowski_info_p, NO_CMD, 0)) {
        }
    }   
    lebowski_console_disp = TRUE;
    if (lebowski_rcv_cmd_result_packet(lebowski_info_p, NO_CMD)) {
        ret_val = FAILED;
    }
    msleep(wait_time * 2);

    /* Check if the interface is ready. I2C Expander GIPIO-03 : IN_RDY */
    if (io_port_8bit_i2c_read(pca, PCA9555_IN_PORT0_REG, &data, TRUE)) {
        return (FAILED);
    }
    if ((data & BIT3) == 0) {
        printf("\nLebowski Module Is Not Ready\n");
    } else {
        printf("\nLebowski Module Is Ready\n");
    }
    fflush(0);

    prpass(testpass, "Lebowski SM All Test");
    ret_val = lebowski_all_tests(lebowski_info_p);
    lebowski_console_disp = FALSE;

    if (lebowski_cleanup_ge_env() == FAILED) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/*------------------------------------------------------------------------------
 *
 * Function: lebowski_sm_console_switch().
 *
 * This function provides console redirect for lebowski sm
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
lebowski_sm_console_switch (int show_menu)
{
    lebowski_ds_t   *iface;
    int slot;
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;

    iface = lebowski_iface_p;
    assert(iface);
    slot = lebowski_iface_p->slot;

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
			           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); // pause a second for the NOTE:

    if (slot == LEBOWSKI_SLOT1) {
        new_baud = slot1_uart;
    } else if (slot == LEBOWSKI_SLOT2) {
        new_baud = slot2_uart;
    } else {
        new_baud = slot3_uart;
    }

    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d",
             new_baud ? "b9600" : "b115200", lebowski_iface_p->uart);
#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return(PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: lebowski_host_utility().
 *
 * This function implements the lebowski sm utility menu
 *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
lebowski_host_utility (int show_menu)
{
    menu(lebowski_util_submenup, lebowski_util_submenu_table, '\0');

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: configure_i2c_expander().
 *
 * This function init Lebowski I2C Expander.
 *
 * Input:  none.
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */

long configure_i2c_expander (void)
{
    uchar io_port_conf0 = 0;
    n2g_i2c_if_t *pca;
    pca = &pca_i2c[0];

    /* With HW design , Lebowski only using Port0 */
    if (io_port_8bit_i2c_read(pca, PCA9555_CFG_PORT0_REG,
                              &io_port_conf0, TRUE)) {
        return (FAILED);
    }
#ifdef DEBUG
    printf("\n%d, io_port_conf0 = 0x%02x\n", __LINE__, io_port_conf0);
    printf("\n%d, io_port_conf1 = 0x%02x\n", __LINE__, io_port_conf1);
#endif

    /* Set IO 1, 2, 4, 7 to output.  Set IO 3 to input */
    /* Set IO port0 1, 2, 7 to output.  Set IO 3,5 to input */
    io_port_conf0 |= (1 << PCA9555_IN_RDY_REGISTER);
    io_port_conf0 &= ~((1 << PCA9555_BOOT_SE_REGISTER) | 
                       (1 << PCA9555_SYS_RET_REGISTER) | 
                       (1 << PCA9555_MODE_REGISTER) | 
                       (1 << PCA9555_DETECT_REGISTER) );
#ifdef DEBUG
    printf("\n%d, io_port_conf0 = 0x%02x\n", __LINE__, io_port_conf0);
#endif
    if (io_port_8bit_i2c_write(pca, PCA9555_CFG_PORT0_REG,
                               &io_port_conf0)) {
        return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, PCA9555_OUT_PORT0_REG,
                              &io_port_conf0, TRUE)) {
        return (FAILED);
    }

#ifdef DEBUG
    printf("\n%d, io_port_conf0 = 0x%02x\n", __LINE__, io_port_conf0);
#endif
    /* Set IO 2 to 0x1, this will release the Lebowski Yeti3 CPU from reset */
    /* Set IO 1 to 0x0, this will tell the uboot it's diagnostic */
    /* Set IO 7 to 0x0, this will tell the uboot it's NGIO */

    io_port_conf0 = ((1 << PCA9555_BOOT_SE_REGISTER) | 
                     (1 << PCA9555_SYS_RET_REGISTER) |
                     (1 << PCA9555_MODE_REGISTER));
    io_port_conf0 &= ~((1 << PCA9555_BOOT_SE_REGISTER) | 
              (1 << PCA9555_DETECT_REGISTER) );
    if (io_port_8bit_i2c_write(pca, PCA9555_OUT_PORT0_REG,
                               &io_port_conf0)) {
        return (FAILED);
    }

    return (PASSED);
}
/*------------------------------------------------------------------------------
 *
 * Function: lebowski_sm_init().
 *
 * This function init Lebowski SM except pse2 fpga.
 *
 * Input:  none.
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
lebowski_sm_init (void)
{
    lebowski_ds_t     *lebowski_info_p;
    lebowski_info_p       = &lebowski_info;
    int ge_port0,ge_port1;
    int real_slot = lebowski_info_p->slot;

    /* Initial the GPIO */
    if (configure_i2c_expander()) {
        cterr('f', 0, "unable to configure i2c_expander");
        return (FAILED);
    }

    /* setup the backplane loopback here. We will do the loopback test
       from Yeti CPU */
    ge_port0 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 0);//Serdes Mux
    //set_gesw_line_loopback(ge_port0, 1);
    ge_port1 = ovld_get_ge_sw_port_num(real_slot, TGT_DEV_NGSM, 1);//BP PHY
    set_gesw_line_loopback(ge_port1, 1);

    if (DIAGFLAG & D_VERBOSE) {
        printf("\nSM slot %d, GE0 : GESW port %d Line Loopback = %s",real_slot, 
               ge_port0 ,get_gesw_line_loopback(ge_port0) ? "ENABLE" : "DISABLE");
        printf("\nSM slot %d, GE1 : GESW port %d Line Loopback = %s\n",real_slot, 
               ge_port1 ,get_gesw_line_loopback(ge_port1) ? "ENABLE" : "DISABLE");
        fflush(0);
    }

    return (PASSED);
}
/*************************************************************************
 * Function: lebowski_i2c_ioe_reg_test
 *
 * This function toggles the i2c expander Polarity register and verifies it.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int lebowski_i2c_ioe_reg_test (void)
{
    int i;
    uchar sav, wval, rval;
    n2g_i2c_if_t *pca= &pca_i2c[0];
    int32_t reg = POLARITY_INV_REG;
    
    char *name = "Polarity";

    prpass(testpass, "I2C IO Expander Register Test : ");

    /* Save register under test */
    if (io_port_8bit_i2c_read(pca, reg, &sav, FALSE)) {
        return (FAILED);
    }

    /* ripple 1 test */
    for (i = 0; i < 8; i++) {
        wval = 1 << i;
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }
        printf("%#x ", rval);
        if (rval != wval) {
            cterr ('f', 0, "Ripple one test failed when accessing %s "
                           "register. Expect: %#x, Read: %#x.",
                           name, wval, rval);
            return FAILED;
        }
    } 

    /* ripple 0 test */
    for (i = 0; i < 8; i++) {
        wval = ~(1 << i);
        if (io_port_8bit_i2c_write(pca, reg, &wval)) {
            return FAILED;
        }
        if (io_port_8bit_i2c_read(pca, reg, &rval, FALSE)) {
            return FAILED;
        }
        printf("%#x ", rval);
        if (rval != wval) {
            cterr ('f', 0, "Ripple zero test failed when accessing %s "
                           "register. Expect: %#x, Read: %#x.",
                           name, wval, rval);
            return FAILED;
        }
    } 

    /* Restore register under test */
    if (io_port_8bit_i2c_write(pca, reg, &sav)) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 * Function: lebowski_sm_test()
 *
 * Description: This function is the main program of the Lebowski test.
 *         This function will be called from platform_sm.h.
 *              Upon entering this function, the slot number will be
 *              checked to see if the user chose to execute all tests
 *              or entering the submenu and executes accordingly.
 *
 * Input:  The slot number of the Lebowski SM
 *
 * Output: PASSED if all the tests PASSED
 *         FAILED if at least one of them FAILED
 *
 **********************************************************************
 */

int lebowski_sm_test(void *sm)
{
    int real_slot, ret_val = 0;
    struct ngio_intf_t *lebowski_sm_iface = (struct ngio_intf_t *)sm;
    lebowski_ds_t     *lebowski_info_p;
    ushort cookie_id = 0;
    lebowski_info_p       = &lebowski_info;

    real_slot = lebowski_sm_iface->slot;
    cookie_id = lebowski_sm_iface->id;
    printf ("\n LEBOWSKI is in slot %d, Cookie Id is %x \n", real_slot, cookie_id);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = lebowski_sm_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    oir = (n2g_i2c_if_t *)lebowski_sm_iface->oir;

    /*
     * Initialize an instance of Lebowski data structure
     */
    lebowski_iface_p = (lebowski_ds_t *) &lebowski_iface[real_slot];
    lebowski_iface_p->cookie_id            = cookie_id;
    lebowski_iface_p->slot                = real_slot;
    lebowski_iface_p->uart                = lebowski_sm_iface->uart_ctrl;
    lebowski_iface_p->lebowski_sm_iface = (struct ngio_intf_t *)sm;
    lebowski_info_p->slot = real_slot;

   /*
     * Release Lebowski SM out of reset.
     */
    /* uart/i2c unreset should be done via function pointer passed into the
       entry point */
    lebowski_sm_iface->i2c_unreset(lebowski_sm_iface);
    lebowski_sm_iface->uart_on(lebowski_sm_iface);
    lebowski_sm_iface->unreset(lebowski_sm_iface);
    msleep(1000);
    assert(sm);
    setup_uart();

    switch (cookie_id) {
    case LEBOWSKI_X_ES3D_24_P_ID:
        sprintf(lebowski_iface_p->testname,"Slot%d 24-port L3G ILP Lebowski SW",
                        real_slot);
        break;
    case LEBOWSKI_X_ES3D_16_P_ID:
        sprintf(lebowski_iface_p->testname,"Slot%d 16-port L3G ILP Lebowski SW",
                        real_slot);
        break;
    case LEBOWSKI_X_ES3D_48_P_ID:
        lebowski_info_p->num_asic++;
        sprintf(lebowski_iface_p->testname,"Slot%d 48-port L3G ILP Lebowski DW",
                        real_slot);
        break;
    default:
        cterr('f', 0, "Invalid Lebowski cookie id %#.4x in slot %d", 
                                          cookie_id, real_slot);
        return (FAILED);
    }
    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER) == FAILED) {
        return (FAILED);
    };

    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY) == FAILED) {
        lebowski_sm_cleanup();
        return (FAILED);
    };
    if (lebowski_sm_init()) {
        lebowski_sm_cleanup();
        return(FAILED);
    }
    build_primary_submenu(lebowski_submenu_table, 
                          LEBOWSKI_SUBMENU_TABLE_SIZE,
                          (char *)lebowski_sm_iface->name,
                          (menuinfo_t **)&lebowski_submenup);
    build_secondary_submenu(lebowski_submenu_table,
                            LEBOWSKI_SUBMENU_TABLE_SIZE,
                            lebowski_tests_secondary_items);

    /*
    * pm_subtest_menu now built.  Display and interact with user until
    * <ESC><RET> back to main menu.
    *
    * To prevent freeing up allocated memory prematurely,
    * save the pre_diag_exec function and set it to NULL.
    * This will prevent menu() marking the needed memory freed.
    */
    lebowski_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    if (lebowski_sm_iface->menu_display == TRUE) {
        menu(lebowski_submenup, lebowski_tests_secondary_items, '\0' );
    } else {
        if (lebowski_sm_iface->test_type == IFACE_TEST) {
            ret_val |= oir_ltc4215_register_test(oir);
            ret_val |= lebowski_i2c_ioe_reg_test();        
        } else {  /* FULL_TEST */    
            do_all_menu_items(lebowski_submenup);
        }
    }

    ret_val |= util_oir_ltc4215_led(oir, OIR_LED_OFF);

    lebowski_sm_cleanup();

    return (ret_val);
}

/*
 *------------------------------------------------------------------
 * $Log: lebowski_host.c,v $
 * Revision 1.16  2018/05/22 02:31:10  alpeng
 * fixed compiler warning, CSCvj57934
 *
 * Revision 1.15  2018/05/18 09:24:48  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.14  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.13.48.4  2018/05/17 10:50:19  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.13.48.3  2017/01/18 08:20:29  alpeng
 * fix uart extend to slot3
 *
 * Revision 1.13.48.2  2017/01/18 08:10:09  alpeng
 * remove assert for supporting sm3
 *
 * Revision 1.13.48.1  2016/12/05 06:36:59  alpeng
 * fixed the uart ctrl num for ngio; change is approved on prrq
 *
 * Revision 1.14  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.13  2014/02/19 09:28:07  iachang
 * CSCum79984 : Fixed Command response failed with O2 image o2x86_lnx.V9.0.0
 *
 * Revision 1.12  2014/01/27 11:47:25  iachang
 * CSCum78285 : Fixed Timeout on Receiving Frame without reporting error
 *
 * Revision 1.11  2013/12/18 06:32:46  hroni
 * use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64
 *
 * Revision 1.10  2013/11/26 08:40:33  hroni
 * fix compiler warning
 *
 * Revision 1.9  2013/11/11 21:18:39  mcharon
 * pass string instead of number in first argum of host_send_packet ; add xaui support
 *
 * Revision 1.8  2013/10/09 12:25:09  iachang
 * CSCuj21962 : Change UART BAUD to 9600
 *
 * Revision 1.7  2013/06/17 15:18:23  jamlin
 * CSCuh00719, Fix Lebowski power off/on utils problems.
 *
 * Revision 1.6  2013/05/09 19:25:17  mcharon
 * remove unused header files. fixed dependancy compile problem
 *
 * Revision 1.5  2013/04/22 07:12:06  iachang
 * CSCug47593 : Hiding image boot up "***" prompt to avoid EDVT detect error.
 *
 * Revision 1.4  2013/04/15 02:52:31  iachang
 * Remove Un-used Cookie ID
 * CSCug33279 : Support UART Baud Rate Chang Utility
 *
 * Revision 1.3  2013/04/03 11:53:51  iachang
 * CSCuf98822 : Rolling SM test string at the same line
 *
 * Revision 1.2  2013/03/31 05:11:58  iachang
 * Support SM Lebowski on Overlord
 *
 * Revision 1.1.6.8  2013/03/31 03:57:22  iachang
 * Display backplan GESW line loopback setting message.
 *
 * Revision 1.1.6.7  2013/03/29 13:24:42  iachang
 * On the Host main test, fix SM test string with the same line.
 *
 * Revision 1.1.6.6  2013/03/18 09:19:19  iachang
 * Code clean up.
 *
 * Revision 1.1.6.5  2013/02/25 11:58:37  iachang
 * CSCue78426 : Modify the Uart related code.
 * Add spining wheel when waiting time.
 * Check IN_RDY pin to determine SM communication is ready.
 *
 * Revision 1.1.6.4  2013/01/14 07:32:20  iachang
 * CSCue03760 : Fixed Lebowski Config/Passwd reset issue
 *
 * Revision 1.1.6.3  2012/12/20 07:59:00  iachang
 * CSCud60670:Assign the unique MAC address locally on Lebowski
 * CSCud74624:Get SM error message
 * Bump up revision number
 *
 * Revision 1.1.6.2  2012/12/17 08:54:35  iachang
 * Sync with main trunk
 *
 * Revision 1.1.4.13  2012/12/10 02:12:24  iachang
 * CSCud60670:Assign the unique MAC address locally on Lebowski
 *
 * Revision 1.1.4.12  2012/11/01 02:22:00  iachang
 * Verify SM response ethernet type.
 *
 * Revision 1.1.4.11  2012/10/29 11:56:18  iachang
 * Fix compile error
 *
 * Revision 1.1.4.10  2012/10/29 11:49:45  iachang
 * Indicates module ready to communicate over primary bus
 *
 * Revision 1.1.4.9  2012/10/29 03:17:51  iachang
 * Avoid the interference from Broadcast packet
 *
 * Revision 1.1.4.8  2012/10/12 07:05:34  iachang
 * Check Primary Interface Ready (I2C Expander GPIO3)
 *
 * Revision 1.1.4.7  2012/10/12 06:01:39  iachang
 * Get Host MAC Address
 *
 * Revision 1.1.4.6  2012/10/12 05:55:24  iachang
 * Support the 48P 0x0B4B Cookie ID
 *
 * Revision 1.1.4.5  2012/10/03 07:04:46  iachang
 * Support IO interface test.
 *
 * Revision 1.1.4.4  2012/10/03 06:48:25  iachang
 * Check O2 platform ILP power supply
 *
 * Revision 1.1.4.3  2012/09/27 08:19:00  iachang
 * Support the Host-SM command response.
 * Correct the Mode Switch GPIO setting.
 *
 * Revision 1.1.4.2  2012/09/24 08:16:20  iachang
 * Sync. with main trunk
 *
 * Revision 1.1.2.7  2012/08/22 15:07:10  iachang
 * Fixed the GPIO initial issue.
 *
 * Revision 1.1.2.6  2012/08/22 12:06:00  iachang
 * Modify the Switch Overlord GESW utility
 *
 * Revision 1.1.2.5  2012/08/14 05:43:21  iachang
 * Display the GESW line loopback status.
 * For bring up, un-reset Yeti3 at initial.
 *
 * Revision 1.1.2.4  2012/08/09 03:26:32  iachang
 * Only setup E0 GESW loopabck at board initial.
 *
 * Revision 1.1.2.3  2012/08/08 07:09:37  iachang
 * Support the Overlord GE Switch utility
 *
 * Revision 1.1.2.2  2012/07/24 09:56:00  iachang
 * Fixed the initial Segmentation fault issue
 *
 * Revision 1.1.2.1  2012/07/23 09:53:19  iachang
 * Initial check in for Lebowski SM on O2
 *
 * Revision 1.1.2.5  2012/07/04 01:56:01  iachang
 * Porting new NGIO driver.
 *
 * Revision 1.1.2.4  2012/06/21 12:05:52  iachang
 * Support Controller Type 0xB4A with 24P.
 *
 * Revision 1.1.2.3  2012/03/21 00:02:42  iachang
 * Support Lebowski LEBOWSKI_ES3X_24_P board ID
 *
 * Revision 1.1.2.2  2012/03/15 17:44:39  iachang
 * Support the NGIO driver.
 *
 * Revision 1.1.2.1  2012/02/14 11:48:13  iachang
 * Initial check in for Lebowski SM
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
