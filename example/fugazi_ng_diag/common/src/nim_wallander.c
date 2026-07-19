/* $Id: nim_wallander.c,v 1.15 2020/04/20 02:27:59 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nim_wallander.c,v $
 * ------------------------------------------------------------------
 *
 * nim_wallander.c - This file contains functions for NIM Wallander.
 *
 * bowang3 -- Mar. 2014
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
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
#include "linux_api.h"
#include "cross_platform.h"
#include "dash_fpga.h"
#include "cookie_4.h"

#ifdef TACHI
#include "diag_console_util.h"
#include "diag_fpga_lib.h"
#include "cetus_gesw_defs.h"
#elif TABEIL
#include "diag_eth_pkt_txrx.h"
#include "diag_fpga_lib.h"
#elif NANOOK
#include "diag_eth_pkt_txrx.h"
#include "ngio_testcard.h"
#else 
#include "platform_eth_pkt_txrx.h"
#if !defined (UTAH) && !defined (CURIE_1RU) && !defined (CURIE_2RU)
#include "vm_timingcard.h"
#include "vm_timingcard_cpld_lib.h"
#endif
#endif

#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>

#define OIR_RES    0.02
#define OIR_SENSE_RES    0.151

#define WALLANDER_LOCAL_IP_ADDR   "192.123.123.150"
#define WALLANDER_REQUEST_PORT    2015
#define WALLANDER_REQUEST_PORT1   2017
#define WALLANDER_STATUS_PORT     2016
#define WALLANDER_CR_STRING       "\012"
#define DIAG_KILL_NC_TMP_FILE     "/tmp/wallander_nc_tmp.pid"
#define BOOT_TIMEOUT       50
#define FTP_SERVER         "192.123.123.1:69"
#define BOOT_DELAY         500
#define DELAY_HELF_SEC     0.5

extern int netstat_main(char *);
extern int utah_port_is_linkup(int);

#if defined TACHI || defined NANOOK
extern int get_ctrl_plane_sgmii_port(void);
#endif

extern int tftp_get (unsigned char *dir, unsigned char *file, 
     unsigned char *server_ip, unsigned char *dest, unsigned int check);
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);
extern void port_tx_util(void);
extern int ovld_bcm_check_port_init(void);
extern int get_gesw_line_loopback(int port_num);
extern int sgmii_lpbk_util(int, int); 
extern int do_all_menu_items(struct menuinfo *);

static int ltc4215_test(void);
static int ltc4215_register_test(void);
static int wallander_pwr_off (void);
static int wallander_pwr_on (void);
static int wallander_power_off (void);
static int wallander_power_on (void);
static int wallander_pwr_cycle (void);
static int wallander_console_switch(void);
static void disable_bp_ge_lpbk (int);
static void enable_bp_ge_lpbk (int);
static int wallander_bp_ge_test(void);
static int pca9555_test(void);
static int pca9555_reg_write(void);
static int pca9555_reg_read(void);
static int set_ngwic_console(void);
static int wallander_uart_test(void);
static int wallander_reset (void);
static int wallander_utils (void);
static int wallander_nim_test(void);
static int nc_cmd_run_wallander_diag(int);
static int wallander_check_test_status(void);
static int wallander_init_status_file(void);
static void wallander_kill_nc(void);
static int wallander_bootup_image(void);
static long ge_bp_lpbk_test(void);
#if !defined(UTAH) && !defined(TACHI) && !defined (CURIE_1RU) && !defined (CURIE_2RU) && !defined(TABEIL) && !defined (NANOOK)
static long clk_verify(void);
static long timingcard_power_on(int);
#endif

int wallander_tx_uart(char *, char *);
int wallander_rx_polling_uart(char *, char *, int);
int wallander_uart_setup(char *);

int wallander_test_slot = 0;
boolean wallander_init_3036x = FALSE;

static void (*wallander_saved_diag_exec)(void) = NULL;
static void *oir_if;

static n2g_i2c_if_t *pca_i2c;

static struct ngio_intf_t *wallander_wic_iface;

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t wallander_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)wallander_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Wallander NGWIC",      (PFT)wallander_power_on,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off Wallander NGWIC",     (PFT)wallander_power_off,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Wallander NGWIC",   (PFT)wallander_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Wallander Backplane GE Utility",(PFT)wallander_bp_ge_test,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Hotswap Utility",       (PFT)ltc4215_test, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9555 GPIO Expander Utility", (PFT)pca9555_test,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)wallander_uart_test,   0,   MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Reset Wallander NGWIC",         (PFT)wallander_reset,         0,    0,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"GE Backplane Loopback Test",     ge_bp_lpbk_test,    0,   MF_CONTINUOUS | MF_DOALL, 
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
#if !defined(UTAH) && !defined(TACHI) && !defined (CURIE_1RU) && !defined (CURIE_2RU) && !defined(TABEIL) && !defined (NANOOK)
    {"Timing Card Clock",             (PFT)clk_verify,       0,     0,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
#endif
    {"Boop up Wallander image",       (PFT)wallander_bootup_image,  0,  0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
};

#define WALLANDER_UTILS_SUBMENU_TABLE_SZ (sizeof(wallander_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t wallander_utils_primary_items[WALLANDER_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t wallander_utils_secondary_items[WALLANDER_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];

char wallanderutiltitle[50];

menuinfo_t wallander_util_submenu = {
    wallanderutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,            /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    wallander_utils_primary_items,
};

menuinfo_t *wallander_util_submenup = &wallander_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Wallander Utilities",           (PFT)wallander_utils,       0,   0,
     (type_t(*)())0, 0,    (type_t(*)())wallander_utils, 0},
    {"Wallander NIM test",            (PFT)wallander_nim_test,    0,   MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Wallander Main Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 **********************************************************************
 *
 *  Function: wallander_utils
 *
 *  Description: Wallander Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int wallander_utils (void)
{
    assert(wallander_wic_iface);

    sprintf(wallanderutiltitle, "Wallander Slot %d Utilities Menu",
            wallander_wic_iface->slot);
    build_primary_submenu(wallander_utils_submenu_table,
                          WALLANDER_UTILS_SUBMENU_TABLE_SZ,
                          wallanderutiltitle, &wallander_util_submenup);

    build_secondary_submenu(wallander_utils_submenu_table,
                            WALLANDER_UTILS_SUBMENU_TABLE_SZ,
                            wallander_utils_secondary_items);

    menu(wallander_util_submenup, wallander_utils_secondary_items, '\0');

    return (PASSED);
}

/**********************************************************************
 * Function: nc_cmd_run_wallander_diag
 *
 * Description: Initial the status file and nc listen for test status
 *              Send nc client request to module side
 *              Check the test status
 *
 * Input:  port - running diag request port number
 *
 * Return: PASSED / FAILED
 *
 **********************************************************************
 */
static int wallander_nim_test(void)
{
    if(nc_cmd_run_wallander_diag(WALLANDER_REQUEST_PORT)){
        cterr('f', 0, "HOST: NC command failed to run Wallander test\n");
        wallander_kill_nc();
        return (FAILED);
    }
    wallander_kill_nc();
    return (PASSED);
}

static int nc_cmd_run_wallander_diag(int port)
{
    char cmd[32];
    uchar data;

    assert(oir_if);
    
    if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }

    if (data & 0x08)
        printf("Wallander Diags menu is already up!\n");
    else {
        if(wallander_bootup_image()) {
            printf("\nFail to boot up Wallander image\n");
            return (FAILED);
        }
    }

    sleep(1);

    // initial status file and nc listen for NIM test result
    if(wallander_init_status_file()) {
        printf("Initial status file error.\n");
        return (FAILED);
    }

    // send nc request
    if((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("Ext loopback flag is toggled OFF\n");
        sprintf(cmd, "nc %s %d", WALLANDER_LOCAL_IP_ADDR, WALLANDER_REQUEST_PORT1);
        printf("HOST: nc command: %s\n", cmd);
    }
    else {
        printf("Ext loopback flag is ON\n");
        sprintf(cmd, "nc %s %d", WALLANDER_LOCAL_IP_ADDR, WALLANDER_REQUEST_PORT);
        printf("HOST: nc command: %s\n", cmd);
    }
    system(cmd);

    // check status file
    if (wallander_check_test_status()) {
        cterr('f', 0, "Wallander WIC-%d test fails", wallander_test_slot);
        return (FAILED);
    }

    return (PASSED);
}

static int wallander_init_status_file (void)
{
    char cmd[64];
    char status_file[32];

    sprintf(status_file, "/tmp/nim_wallander_%d.status", wallander_test_slot);
    sprintf(cmd, "echo ' ' > %s", status_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -l -p %d > %s &", WALLANDER_STATUS_PORT, status_file);
    printf("HOST nc command: %s\n", cmd);
    
    if (system(cmd)) {
        return (FAILED);
    }
    return (PASSED);
}

static int wallander_check_test_status (void)
{
    FILE *fp;
    char status_file[32];
    char buf[5];  // expect "PASS" or "FAIL"

    sprintf(status_file, "/tmp/nim_wallander_%d.status", wallander_test_slot);

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    if (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strcmp(buf, "PASS")) {
            printf("Fail! Return Status is %s\n", buf);
            fflush(stdout);
            fclose(fp);
            return (FAILED);
         } else {
            fclose(fp);
            return (PASSED);
         }
    }

    printf("Fail to get content of status file\n");
    fflush(stdout);
    fclose(fp);

    return (FAILED);
}

/***************************************************************************
 *
 * Function: wallander_kill_nc
 *
 * Description: This function lists all process and grep nc process,
 *              and dump their pids to a temporary to kill them
 *
 * Input:  None
 *
 * Output: None
 *
 ***************************************************************************
 */
static void wallander_kill_nc (void)
{
    char cmd[128];
    char buf[128];
    char pid_file[32];
    char *token;
    int pid;
    FILE *fp;
    struct stat sts;

    fp = fopen(DIAG_KILL_NC_TMP_FILE, "w+");
    if (fp == NULL) {
        printf("%s: Open '%s' Failed\n", __FUNCTION__, DIAG_KILL_NC_TMP_FILE);
        return;
    }

    sprintf(cmd, "ps | grep 'nc 192.123.123 \\| nc -l -l -p' > %s", DIAG_KILL_NC_TMP_FILE);
    system(cmd);
    printf("\nkill cmd: %s\n", cmd);

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        /* separate string of one line, get 1st substring pointer */
        token = strtok(buf, " ");
#if defined(TABEIL) || defined(NANOOK)
        /* Tabei-L - atoi input should not be NULL, or wallander will crash */
        if (token == NULL) {
            continue;
        }
#endif
        pid = atoi(token);
        /* Check if this process is still alive */
        sprintf(pid_file, "/proc/%d", pid);
        if (stat(pid_file, &sts) == -1) {
            /* Process doesn't exist */
            continue;
        }
        printf("Killing a nc process.\n"); 
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }

    fclose(fp);

}

static int wallander_bootup_image(void)
{
    int boot_timeout, dhcp_time, i;
    char tty_dev[32];
    char load_linux_str[256];
    uchar data;
    uint32_t status;
    int retry = 6;

    assert(wallander_wic_iface);

    if(tftp_get(0, (unsigned char *)"nim_wallander_diag.img", 
                0, (unsigned char *)"/firmware/nim_wallander_diag.img", 1) < 0) {
        cterr('f', 0, "Failed to tftp download firmware to local host");
        return (FAILED);
    }
    else {
        printf("Wallander image is downloaded.\n");
        fflush(stdout);
    }

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(wallander_test_slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (wallander_wic_iface->mod_type == SM_DAUGHTER_CARD) { 
        sprintf(tty_dev, "/dev/ttyS%d", wallander_test_slot-1);
    } else {
        sprintf(tty_dev, "/dev/ttyS%d", wallander_test_slot+1);
    }
#else    
    sprintf(tty_dev, "/dev/ttyDASH%d", wallander_wic_iface->uart_ctrl); 
#endif

    msleep(100);

/*
 *  Per MFG's request, retry 5 times with power cycle
 */
    while(retry > 0)
    {
        retry--;
        if(retry == 5) {
            if(wallander_power_on() == FAILED) {
                printf("Failed to power up Wallander card\n");
                continue;
            }
        } else {
            printf("*******************************\n");
            printf("*Retry No.%d after power cycle*\n", 5-retry);
            printf("*******************************\n");
            //  Wallander_power cycle
            printf("Wallander power cycle");
            if (wallander_pwr_off()) {
                printf("Failed to Power Off the wallander NGWIC\n");
                continue;
            }

            /* msleep for 10 seconds. */
            for (i = 0; i < 10; i++) {
                printf(".");
                msleep(1000);
            }

            if (wallander_pwr_on()) {
                printf("Failed to Power On the wallander NGWIC\n");
                continue;
            }
        }

        printf("\nLooking for bootloader prompt ...");
        fflush(stdout);

        boot_timeout = 300;
        do{
            wallander_tx_uart(tty_dev, "\003");    // issue ctrl+C to stop autoboot of IOS
            wallander_tx_uart(tty_dev, "\003");

            if (wallander_rx_polling_uart(tty_dev, "NIM-", 100) == PASSED) {
                printf("OK\n");
                fflush(stdout);
                break;
            }
#if defined(TACHI) || defined(NANOOK)
            msleep(100);
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("Retry to polling NIM at UART.\n"); 
            }
#endif            
        } while (boot_timeout--);

        if (boot_timeout <= 0) {
            printf("FAIL\n");
            fflush(stdout);
            printf("Failed to get '%s' bootloader prompt\n", "NIM-");
            continue;
        }    
        msleep(1000);

        // DHCP
        printf("DHCP ...");
        fflush(stdout);

        dhcp_time = 300;
        wallander_tx_uart(tty_dev, "dhcp\012");
        do{
            if (wallander_rx_polling_uart(tty_dev, "192.123.123", 100) == PASSED) {
                printf("OK\n");
                fflush(stdout);
                break;
            }
        } while (dhcp_time--);

        if (dhcp_time <=0) {
            printf("FAIL\n");
            fflush(stdout);
            printf("Failed to get correct wallander ip by DHCP\n");
            continue;
        }   
        msleep(500);
#if !defined(NANOOK)
        if (is_goldbeach()) {
#else
        if (is_nanook() || is_nanook_plus()) {
#endif
            /* CSCuz69331 : NIM Card TFTP Firmware Download Time Out Issue*/
            boot_timeout = BOOT_TIMEOUT;
            do{
#if !defined(NANOOK)
                status = utah_port_is_linkup(wallander_wic_iface->slot + 1);
#else
                status = eth_is_linkup(wallander_wic_iface->slot + 1);
#endif
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("\neth%d status  = %x", (wallander_wic_iface->slot) +1, 
                            status);
                }
                if (((netstat_main(FTP_SERVER)) == PASSED) && 
                     (status == PASSED)) {
                   if (diagflag_xram & D_DEBUG_OPTIONS) {
                       printf("\neth%d status  = %x\n", (wallander_wic_iface->slot) +1, 
                       	    status);
                       printf("\n------ Host UDP Port 69 Link Up (%f)s  -----\n",
                       	 (BOOT_TIMEOUT - boot_timeout) * DELAY_HELF_SEC);
                       fflush(stdout);
                   }
                   break;
                }
                msleep(BOOT_DELAY);
            } while (boot_timeout--);
            if (boot_timeout < 0) {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("H\nost UDP Port 69 Still Link Down\n");
                    fflush(stdout);
                }
            }
        }
        // "boot_diag" later
        //sprintf(load_linux_str, "tftpboot 0 /firmware/nim_wallander_diag.img;bootoctlinux 0 console=ttyS0,9600\012");
        sprintf(load_linux_str, "boot_diag\012");
        wallander_tx_uart(tty_dev, load_linux_str);
        wallander_tx_uart(tty_dev, WALLANDER_CR_STRING);

        printf("Booting wallander image ...");
        fflush(stdout);

        // poll for Primary Interface Ready pin (GPIO pin 3) which is set 
        // by Wallander module side when the diag menu is up. 
        for (i = 0; i < 1000; i++) {
            if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
                util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
                printf("Unable to read PCA9557 register @ 0x03\n");
                continue;
            }

            if (data & 0x08)
                break;

            msleep(200);
        }

        if (i == 1000) {
            util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
            printf("Timeout waiting for primary interface ready pin asserted\n");
            continue;
        }
        sleep(1);

        printf("Done\nWallander image is up!\n");
        fflush(stdout);
        return (PASSED);
    }

    cterr('f',0,"\nCan not boot up Wallander image after 5 retry!\n");
    return (FAILED);
}

int
wallander_iface_test()
{
    assert(oir_if);

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }

    if(wallander_bootup_image()) {
        printf("\nFail to boot up Wallander image\n");
        return (FAILED);
    }

    sleep(3);

    /* Testing UART and GE0 interfaces */
    if (wallander_uart_test()) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

int
wallander_test(void *wic)
{
    int slot;
    ushort board_id = 0;
    char tty_dev[32];
    int ret_val = PASSED;

    assert(wic);

    wallander_wic_iface = (struct ngio_intf_t *)wic;

    slot = wallander_wic_iface->slot;
    board_id = wallander_wic_iface->id;

    wallander_test_slot = wallander_wic_iface->slot;

    wallander_wic_iface->uart_on(wic);

    printf("\nwallander_test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Wallander NGWIC ", slot);

    oir_if = (void *)(wallander_wic_iface->oir);

    pca_i2c = wallander_wic_iface->pca;

    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(wallander_test_slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (wallander_wic_iface->mod_type == SM_DAUGHTER_CARD) { 
        sprintf(tty_dev, "/dev/ttyS%d", wallander_test_slot-1);
    } else {
        sprintf(tty_dev, "/dev/ttyS%d", wallander_test_slot+1);
    }
#else    
    sprintf(tty_dev, "/dev/ttyDASH%d", wallander_wic_iface->uart_ctrl); 
#endif


    if(wallander_uart_setup(tty_dev) == FAILED) {
        printf("Failed to setup UART\n");
    }

    wallander_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr, &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, main_menu_secondary_items);

    if (wallander_wic_iface->test_type == IFACE_TEST) {
	ret_val = wallander_iface_test();
    } else {
        if (wallander_wic_iface->menu_display == TRUE) {
            menu(maindiagp, main_menu_secondary_items, '\0');
        } else {
            do_all_menu_items(maindiagp);
        }
    }

    return(ret_val);
}


static int 
set_ngwic_console ()
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x07, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x07\n");
        return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x07, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ 0x07\n");
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x01\n");
        return (FAILED);
    }

    /* set GPIO pin 4 to 0 for NGWIC console redirect */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ 0x01\n");
        return (FAILED);
    }
    return (PASSED);
}

static int
wallander_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(wallander_wic_iface);
    assert(oir_if);

    if (set_ngwic_console()) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
    }

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(wallander_wic_iface->slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2");
#else    
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d", 
             wallander_wic_iface->uart_ctrl);
#endif

    system(cmd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: wallander_pwr_off
 *
 * Description: This function does all necessary configuration to power off.
 *              reset module, power off, i2c reset.
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int
wallander_pwr_off (void)
{
    uint8_t data = 0;
    int slot;

    assert(oir_if);
    assert(wallander_wic_iface);

    slot = wallander_wic_iface->slot;
    printf("\nPower Off the Wallander NGWIC.\n");

    /* disable power interrupt */
    ngiowic_disable_intr (slot, NGIO_FLT_INTR);

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
    wallander_wic_iface->i2c_reset(wallander_wic_iface);
    wallander_wic_iface->off(wallander_wic_iface);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: wallander_power_off
 *
 * Description: This function is called for power-off utility.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
wallander_power_off (void)
{
    uint8_t ans;

    assert(wallander_wic_iface);

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Wallander NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    if (wallander_pwr_off()) {
        return (FAILED);
    }

    return PASSED;
}

/***************************************************************************
 *
 * Function: wallander_pwr_on
 *
 * Description: This function does all necessary configuration to power on.
 *              enable Wallander NGWIC, power it on, take it out of reset.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 ***************************************************************************
 */
static int
wallander_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Wallander NGWIC.\n");

    assert(oir_if);
    assert(wallander_wic_iface);

    /* enable ngwic and take I2C out of reset */
    slot_i2c_unreset(wallander_wic_iface, wallander_wic_iface->slot, "WIC");

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

    printf("Waiting for Wallander NGWIC to Power-Up.\n");
    msleep(2000);

    /* take Wallander NGWIC out of reset */
    wallander_wic_iface->unreset(wallander_wic_iface);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
    wallander_wic_iface->uart_on(wallander_wic_iface);
#ifdef wallander_P1B
    enable_bp_ge_lpbk();
#endif
    return (PASSED);
}

/**********************************************************************
 *
 * Function: wallander_power_on
 *
 * Description: This function is called for power-on utiliy.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
wallander_power_on (void)
{
    assert(wallander_wic_iface);

    if(wallander_pwr_on()) {
        return(FAILED);
    }

    printf("Wallander NGWIC is powered up.\n");

   return (PASSED);
}

/**********************************************************************
 *
 * Function: wallander_pwr_cycle
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
wallander_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the Wallander NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "wallander is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (wallander_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the wallander NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (wallander_pwr_on()) {
        cterr('f', 0, "Failed to Power On the wallander NGWIC");
        return(FAILED);
    }

    return(PASSED);
}

static void
disable_bp_ge_lpbk (int local_port)
{
    int ge_port;

    assert(wallander_wic_iface);

#ifdef TABEIL
    /* tabei-l platform didn't have gesw*/
    printf("\nTabei-l didn't support gesw\n");
    return;
#endif

    if (is_goldbeach()) {
        /* Goldbeach platform didn't have GESW*/
        printf("\nGoldbeach Didn't Support GESW\n");
        return;
    }
    /* When wic card is in sm adapter card
    */
#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else
    if (wallander_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGSM, local_port);
    else
        ge_port = ovld_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGWIC, local_port);
#endif
    set_gesw_line_loopback(ge_port, 0);
}

static void
enable_bp_ge_lpbk (int local_port)
{
    int ge_port;

    assert(wallander_wic_iface);

#ifdef TABEIL
    /* tabei-l platform didn't have gesw*/
    printf("\nTabei-l didn't support gesw\n");
    return;
#endif

    if (is_goldbeach()) {
        /* Goldbeach platform didn't have GESW*/
        printf("\nGoldbeach Didn't Support GESW\n");
        return;
    }
    /* When wic card is in sm adapter card
    */
#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else    
    if (wallander_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGSM, local_port);
    else
        ge_port = ovld_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGWIC, local_port);
#endif
    set_gesw_line_loopback(ge_port, 1);
}

/**********************************************************************
 *
 * Function: wallander_bp_ge_test
 * Description: This function provides tests for Wallander port of Backplane GESW
 *
 **********************************************************************
 */
static int
wallander_bp_ge_test ()
{
    int ge_port;
    uint port;
    uchar type = 'f';
    int stop = 0;
    int state = -1;

    assert(wallander_wic_iface);

#ifdef TABEIL
    /* tabei-l platform didn't have gesw*/
    printf("\ntabei-l didn't support gesw\n");
    return (PASSED);
#endif

    port = gethex_answer("Overlord GESW port E0/E1", 0, 0, 1);
    printf("\nWallander Backplane GE%d Utility\n", port); 
    
    /* When wic card is in sm adapter card
    */
#ifdef TACHI
    ge_port = tachi_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGWIC, 0);
#else    
    if (wallander_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port = ovld_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGSM, port);
    else
        ge_port = ovld_get_ge_sw_port_num(wallander_wic_iface->slot, TGT_DEV_NGWIC, port);
#endif
    while (1) {
        printf("\na: enable motherboard line loopback at Wallander GESW port\n");
        printf("b: disable motherboard line loopback at Wallander GESW port\n");
        printf("c: get Wallander GESW port loopback setting\n");
        printf("d: Wallander GESW port send package\n");
#ifndef TACHI
        printf("e: check GESW port initialized\n");
#endif
        printf("f: exit\n");
        type = getc_answer("Select an option", "abcdef", 'f');
        switch(type) {
        case 'a':
            enable_bp_ge_lpbk(port);
            break;
        case 'b':
            disable_bp_ge_lpbk(port);
            break;
        case 'c':
            state = get_gesw_line_loopback(ge_port);
            if (state) {
                printf("line loopback has been enabled.\n");
            } else {
                printf("line loopback has been disabled.\n");
            }
            break;
        case 'd':
            printf("Wallander GE%d port is %d\n", port, ge_port);
            port_tx_util();
            break;
#ifndef TACHI
        case 'e':
            ovld_bcm_check_port_init();
            break;
#endif
        case 'f':
            stop = 1;
            break;
        default:
            break;
        }
        if (stop) {
            break;
        }
    }
    return (PASSED);
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
    int ret;

    prpass(testpass, "LTC4215 OIR Register test ");

    ret = oir_ltc4215_register_test(oir_if);
    if (ret == FAILED)
	cterr('f',0,"LTC4215 register test failed.");

    return (ret);
}

/**********************************************************************
 *
 * Function: ltc4215_test
 *
 * Description: LTC4215 Register Test/Read/Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_test (void)
{
    uchar type = 'd';
    int stop = 0;
    uint8_t  data = 0;
    float current = 0.0;

    assert(oir_if);
    printf("\nLTC4215 Hotswap Utility\n"); 

    while (1) {
        printf("\na: LTC4215 Register Test\n");
        printf("b: LTC4215 Register Read\n");
        printf("c: LTC4215 Register Write\n");
        printf("d: show 12V current consumption\n");
        printf("e: exit\n");
        type = getc_answer("Select an option", "abcde", 'e');
        switch(type) {
        case 'a':
            prpass(testpass, "LTC4215 OIR Register test ");
            if (oir_ltc4215_register_test(oir_if)) {
	        cterr('f',0,"LTC4215 register test failed.");
                stop = 1;
            }
            break;
        case 'b':
            util_oir_ltc4215_reg_read(oir_if);
            break;
        case 'c':
            util_oir_ltc4215_reg_write(oir_if);
            break;
        case 'd':
            oir_ltc4215_reg_read(oir_if, LTC4215_SENSE_REG, &data);
            current = OIR_SENSE_RES * data / OIR_RES;
            printf("Sense Voltage Register divided by R28: %f mA\n", current);
            break;
        case 'e':
            stop = 1;
            break;
        default:
            break;
        }
        if (stop) {
            break;
        }
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: pca9555_reg_read
 *
 * Description: PCA9555 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9555_reg_read (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x7);

    if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ %#x\n", offset);
	return (FAILED);
    }
    printf("\nRegister @ %#x = %#x\n", offset, data);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: pca9555_reg_write
 *
 * Description: PCA9555 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9555_reg_write (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x7);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ %#x\n", offset);
	return (FAILED);
    }
    return (PASSED);
}

static int
pca9555_test (void)
{
    uint8_t ans;

    assert(wallander_wic_iface); 

    printf("\nRead or Write pca9555 register? (r/w): ");
    ans = getchar();
    getchar();
    if (ans == 'r' || ans == 'R') {
        if (pca9555_reg_read()){
            return (FAILED);
        }
    } else if (ans == 'w' || ans == 'W') {
         if (pca9555_reg_write()){
            return (FAILED);
         }
    } else {
        printf("ABORT!\n");
    }

    return (PASSED);
}

/*************************************************************************
 * Function: wallander_uart_test
 *
 * Description: Test the UART connection from the host to Wallander by sending 
 *              string to uart and recieve expect msg from uart interface
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static int wallander_uart_test (void)
{
    int port;
    char *ptr;
    char *tx_str = "i\n";
    char *exp_pattern = "Linux";
    char rx_str[256];
    int tx_len = strlen(tx_str);
    int rx_sz;
    int i;
#ifdef TACHI
    const int maxlen = 28;
    char test_if[maxlen];
    int ret_val = PASSED;
#endif
    prpass(testpass, "Uart Test - ");
    // When Wallander is in Thule adapter card

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(wallander_wic_iface->slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    snprintf(test_if, maxlen-1, UART_TTYS2_DEV);

    ret_val = uart_msg_exh_test(test_if, "diag\n", "Menuitem>",TRIG_DIAG_M);
    if (ret_val == FAILED) {
        cterr('f',0,"Wallander UART test failed\n");
    }
    sleep(1);
    return (ret_val);
#else    
    port = wallander_wic_iface->uart_ctrl; 
#endif

    printf("UART port %d\n", port);
    memset(rx_str, 0, sizeof(rx_str));
    rx_sz = 0;

    dash_uart_reset(port);
    dash_uart_tx(port, 9600, tx_str, tx_len, 0);
    sleep(1);
    dash_uart_rx(port, &rx_sz, rx_str);
    dash_uart_reset(port);

    ptr = memchr(rx_str, exp_pattern[0], strlen(rx_str));
    printf("rx_str = %s\n", rx_str);
    if (ptr == NULL) {
        cterr('f', 0, "tx/rx strings do not match: expected %s, got %s",
                exp_pattern, rx_str);
        return (FAILED);
    }
    for (i = 0; i < strlen(exp_pattern); i++) {
        if (ptr[i] != exp_pattern[i]) {
            cterr('f', 0, ".tx/rx strings do not match: expected %s, got %s",
                     exp_pattern, ptr);
            return (FAILED);
        }
    }

    printf("Wallander UART test passed. \n");
    return (PASSED);
}

#if !defined(UTAH) && !defined(TACHI) && !defined (CURIE_1RU) && !defined (CURIE_2RU) && !defined(TABEIL) && !defined (NANOOK)
/**********************************************************************
 *
 * Function: clk_verify
 *
 * Description: To test whether the host side provides clock successfully.
 * 
 * Input : none
 * 
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
long clk_verify (void)
{
    int clk_slot;

    if (wallander_init_3036x == FALSE) {
        if (timingcard_power_on(FIRST_SLOT) == FAILED) {
            printf("*** Power on timing card failed\n");
            return (FAILED);
        }

        /* Clear the timing card initialized flag */
        clear_timingcard_init_flag();

        /* Set i2c address of timing card*/
        set_timingcard_i2c_addr();
        
        /* Initialize the timing card */
        if (timingcard_init_seq() == FAILED) {
            cterr('f', 0, "Initialize the timing card fail");
            return (FAILED);
        }
        wallander_init_3036x = TRUE;
    }

    /* Set up the clock path - Overlord -> Timing card -> Wallander */
    /* WIC slot 1 -> 4, WIC slot 2 -> 5, WIC slot 3 -> 6 */
    if (wallander_wic_iface->mod_type == SM_DAUGHTER_CARD) 
        clk_slot = wallander_test_slot;
    else
        clk_slot = wallander_test_slot + 3;

    if (clock_verification_path_lib(clk_slot) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}
#endif

/**********************************************************************
 *
 * Function: wallander_reset
 *
 * Description: This function query for reset or unreset Wallander NGWIC module.
 *              It doesn't reset or unreset i2c.
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
wallander_reset (void)
{
    uint8_t ans;

    assert(wallander_wic_iface);

    printf("\nReset or Unreset Wallander module? (r/u): ");
    ans = getchar();
    putchar(ans);
    printf("\n");
    if (ans == 'r' || ans == 'R') {
        if (wallander_wic_iface->reset(wallander_wic_iface)){
            printf("Unable to reset Wallander module\n");
            return (FAILED);
        }
        msleep(1000);
    } else if (ans == 'u' || ans == 'U') {
         if (wallander_wic_iface->unreset(wallander_wic_iface)){
            printf("Unable to unreset Wallander module\n");
            return (FAILED);
         }
        msleep(1000);
    } else {
        printf("ABORT!\n");
    }
    return (PASSED);
}

/**********************************************************************
*
* Function: ge_bp_lpbk_test
*
* Perform GE loopback test to verify GE0 backplane connectivity between
* host and NIM card
*
* Input : none
*
* Output: PASSED/FAILED
*
**********************************************************************
*/
static long ge_bp_lpbk_test (void)
{
    int rc = PASSED;
    int packet_no = 10;
    int ctrl_plane_sgmii_port;

    prpass(testpass, "Running loopback test now");

#ifdef NANOOK
    ctrl_plane_sgmii_port = get_sgmii_port_num(0, 0);
#else
    ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port();
#endif

    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, packet_no) != PASSED) {
        cterr('f', 0, "GE loopback from Host side %d fails.", ctrl_plane_sgmii_port);
        rc = (FAILED);
    }

    return (rc);
}

/*****************************************************************
 *
 * Function: wallander_tx_uart
 *
 * Description: This function transmits strings into tty
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         out_str: compared string
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int wallander_tx_uart (char *tty_dev, char *out_str)
{
    int uart_fd, cnt;
    int rc = PASSED;

    /* Sanity check */
    if (tty_dev == NULL || out_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_WRONLY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    cnt = write(uart_fd, out_str, strlen(out_str));

    if (cnt < 0) {
        perror("tx_uart: write failed\n");
        rc = FAILED;
    }

    close(uart_fd);
    return (rc);
}

/*****************************************************************
 *
 * Function: wallander_rx_polling_uart
 *
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         comp_str: compared string
 *         timeout: timeout value (ms)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int wallander_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
{
    int uart_fd, cnt;
    struct timeval read_timeout;
    fd_set set;
    char buf[1024];
    char *search_str;
    int rc;
    struct timeval start_time, curr_time;
    int elapsed_time_in_ms;

    /* Sanity check */
    if (tty_dev == NULL || comp_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    gettimeofday(&start_time, NULL);

    do {
        /* Set timeout on file descriptor */
        FD_ZERO(&set);
        FD_SET(uart_fd, &set);

        read_timeout.tv_sec  = 1;
        read_timeout.tv_usec = 0;

        rc = select(uart_fd + 1, &set, NULL, NULL, &read_timeout);

        if (rc == -1) { /* Error occured */
            perror("select");
            fflush(stdout);
            goto exit_poll_uart;
        }

        if (FD_ISSET(uart_fd, &set)) {
            /* Now, we read the buffer */
            cnt = read(uart_fd, buf, 255);
            if (cnt < 0) {
                perror("Read error");
                fflush(stdout);
                goto exit_poll_uart;
            }

            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                close(uart_fd);
                return (PASSED);
            }
        }

        /* Now check if elapsed time exceeds timeout value */
        gettimeofday(&curr_time, NULL);

        elapsed_time_in_ms = (curr_time.tv_sec - start_time.tv_sec) * 1000;
        elapsed_time_in_ms += (curr_time.tv_usec - start_time.tv_usec) / 1000;

        if (elapsed_time_in_ms > timeout) {
            goto exit_poll_uart;
        }

        msleep(1);
    } while (1);

exit_poll_uart:
    close(uart_fd);

    return (FAILED);
}

/*****************************************************************
 *
 * Function: wallander_uart_setup
 *
 * Description: This function setups UART parameter
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */

int wallander_uart_setup (char *tty_dev)
{
    int uart_fd;
    struct termios tio_setting;

    /* Sanity check */
    if (tty_dev == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    tcgetattr(uart_fd, &tio_setting);

    tio_setting.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    tio_setting.c_iflag = IGNPAR | ICRNL;
    tio_setting.c_oflag = 0;
    tio_setting.c_lflag = ICANON;

    tcflush(uart_fd, TCIFLUSH);
    tcflush(uart_fd, TCOFLUSH);
    tcsetattr(uart_fd, TCSANOW, &tio_setting);
    close(uart_fd);

    return (PASSED);
}

#if !defined(UTAH) && !defined(TACHI)  && !defined (CURIE_1RU) && !defined (CURIE_2RU) && !defined(TABEIL) && !defined (NANOOK)
/**********************************************************************
 *
 * Function: timingcard_power_on
 *
 * This function power-on timing card
 *
 * Input : test_slot - slot number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_power_on (int test_slot)
{
    struct ngio_intf_t *vm_ptr;
    char err[80];
    int retry;
    int status = PASSED;
    
    vm_ptr = slot_get_ngiovm(test_slot);
    if (vm_ptr == NULL) {
        printf("%s: Null Pointer!\n", __FUNCTION__);
        return (FAILED);
    }

    /* Check if SM is present or not */
    if (!vm_ptr->is_present((void *)vm_ptr)) {
        printf("VM Slot-%d is Vacant! Exiting...\n", test_slot);
        return (FAILED);
    }

    /* NGIO Power On and read ID */
    for (retry = 0, *err = '\0'; retry < 3; retry++) {
        if (slot_i2c_unreset(vm_ptr, vm_ptr->slot, "VM") == FAILED) {
            return (FAILED);
        }

        /* Read Controller Type */
        if ((status = vm_ptr->get_id((void *)vm_ptr, err)) == FAILED) {
            if (vm_ptr->off) {
                vm_ptr->off(vm_ptr);
            }
            sleep(2);
            continue;
        }
        break;
    }

    /* Make sure the VM card is Timing Card */
    if (vm_ptr->id == TIMINGCARD_VM) {
        printf("Timing Card is detected!\n");
    } else {
        printf("VM Card Controller Type: 0x%x\n", vm_ptr->id);
        printf("Timing Card is required for the test! Exiting...\n");
        return (FAILED);
    }

    /* Power up the VM card */
    printf("Powering up VM-%d now...\n", vm_ptr->slot);
    fflush(stdout);

    return (PASSED);
}
#endif

/******** History ********
$Log: nim_wallander.c,v $
Revision 1.15  2020/04/20 02:27:59  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.14  2020/01/09 01:01:52  jiajliu
Merge Curie 2RU to main trunk

Revision 1.13  2019/11/25 08:55:49  kehuang2
Collapse Tabei-L into main trunk

Revision 1.12  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.11  2018/12/18 13:29:11  hondwang
Fix CDETs CSCvn58971 with Goldschlager and Wallander on Tachi-L

Revision 1.10  2018/05/22 02:31:11  alpeng
fixed compiler warning, CSCvj57934

Revision 1.9  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.8  2017/08/10 10:29:25  iachang
Fixed compile issue

Revision 1.7  2017/08/10 10:10:34  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.6  2017/03/30 08:21:06  hondwang
Tachi-L brach merge

Revision 1.5.2.3  2017/03/07 03:28:18  hondwang
Fix dreamliner and wallander IO interface test

Revision 1.5.2.2  2017/01/10 08:49:58  hondwang
fix wallander NIM bootloader prompt

Revision 1.5.2.1  2017/01/09 12:15:57  hondwang
Add Wallander support

Revision 1.5  2016/10/16 12:28:13  iachang
Supported Goldbeach Platform.

Revision 1.4.20.5  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.4.20.4  2017/06/14 03:11:57  leschen
Fix Wallander UART kernel panic problem on Neptune system.

Revision 1.4.20.3  2017/04/05 06:40:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.4.20.2  2016/12/15 05:10:35  alpeng
update wallander uart test for supporting get uart_ctrl via api

Revision 1.4.20.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.8  2017/08/10 10:29:25  iachang
Fixed compile issue

Revision 1.7  2017/08/10 10:10:34  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.6  2017/03/30 08:21:06  hondwang
Tachi-L brach merge

Revision 1.5.2.3  2017/03/07 03:28:18  hondwang
Fix dreamliner and wallander IO interface test

Revision 1.5.2.2  2017/01/10 08:49:58  hondwang
fix wallander NIM bootloader prompt

Revision 1.5.2.1  2017/01/09 12:15:57  hondwang
Add Wallander support

Revision 1.5  2016/10/16 12:28:13  iachang
Supported Goldbeach Platform.

Revision 1.4  2015/04/20 06:16:49  bowang3
Correct retry logic of booting up image

Revision 1.3  2015/02/20 08:17:28  kwochan
TimingCard codes commit broke utah_lnx daily-built, but O2 x86_lnx daily-built is
ok. TimingCard is an uncommitted ISC project, which is used to verify project
that needs to test 1588, eg. Woodlawn SM and Wallander NIM.
It is believe that Woodlawn SM and Wallander NIM verify 1588 on O2 only, so
#ifndef UTAH to commit the codes in nim_wallander.c so that other projects
can build the utah-lnx successful for them to release the image.
Bo needs to confirm if the codes change of this nim_wallander.c are done
correctly ...

Revision 1.2  2015/02/18 06:08:25  bowang3
Support Wallander NIM 1588 test with timing card


$Endlog$
*/

