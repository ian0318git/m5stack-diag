/* $Id: platform_plug_serial_test.c,v 1.8 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/platform_plug_serial_test.c,v $
 *------------------------------------------------------------------
 *
 * plug_serial_test.c - PLUGGABLE Serial Main Functions
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "endians.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "dev_print.h"
#include "dev_object.h"
#include "byteswap.h"
#include "common.h"
#include "types.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "platform_cookie.h"
#include "pca.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "i2c_api.h"
#include "cross_platform.h"
#include "plug_slot.h"
#include "plug_gpio_exp_test.h"
#include "plug_gpio_exp_lib.h"
#include "plug_temp_sensor_test.h"
#include "plug_temp_sensor_lib.h"
#include "plug_host_fpga_lib.h"
#include "platform_plug_serial_test.h"
#include "platform_plug_serial_util.h"
#include "platform_plug_serial_host.h"
#include "plug_testcard_host_impl.h"
#include "plug_common_lib.h"

int plug_serial_gpio_exp_out_init (void);
int plug_serial_gpio_exp_dir_init (void);
int plug_serial_enable_g_led (int led_on_off);
int plug_serial_enable_y_led (int led_on_off);
int plug_serial_main(void *);

static int plug_serial_ts_test(int);
static int plug_serial_gpio_exp_test(void);
static int plug_serial_pin_test(int);
static int pluggable_serial_card_test(void);
static int nc_cmd_run_pluggable_serial_diag (int port);
static int pluggable_serial_check_test_status (void);
static int pluggable_serial_init_status_file (void);
static void pluggable_serial_kill_nc (void);
static void pluggable_serial_get_host_flag(void);
static int pluggable_serial_send_diag_flag(void);
static int plug_serial_insmod(int);
static int plug_serial_serdes_type_test(void);
int plug_serial_uart_setup(char *);
int pluggable_serial_uart_test(void);
int plug_serial_rx_polling_uart(char *, char *, int);
int plug_serial_tx_uart(char *, char *);
int pluggable_serial_iface_test();
char tftp_server_ip[64];
char eth_num[64];
char uart_device_name[64];
char uart_driver_path[64];

static void (*pluggable_serial_saved_diag_exec)(void) = NULL;

struct plug_intf_t *plug_serial_iface;
static int default_printk_level = -1;

static int serdes_type[] = { 0, 0, 
                             0, 1,
                             1, 0,
                             1, 1 };
extern int do_all_menu_items(struct menuinfo *);

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Pluggable Serial Utilities",  (PFT)pluggable_serial_utils,       0,   0,
     (type_t(*)())0, 0,     (type_t(*)())pluggable_serial_utils, 0},
    {"Thermal Sensor Test",         (type_t(*)())plug_serial_ts_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"GPIO Expander (0x4E) Test",   (type_t(*)())plug_serial_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Pluggable Serial Reset pin Test", (type_t(*)())plug_serial_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Pluggable Serial Card test",  (PFT)pluggable_serial_card_test, 0, 
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (type_t(*)())0,          0},
    {"UART Test",                   (PFT)pluggable_serial_uart_test, 0,  
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,	    (type_t(*)())0,          0},
    {"SerDes Type Test",            (PFT)plug_serial_serdes_type_test, 0,  
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,	    (type_t(*)())0,          0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Pluggable Serial Main Menu",       /* title */
    0,                                  /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,              /* shows major flags */
    0,                                  /* generic prompt */
    0,                                  /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;


/* GPIO Expander Bit Map */
static serial_gpio_exp serial_gpio_exp_table[] = {
    {MANDATORY , OUTPUT , PORT0 , 0, ENABLE_LED_GREEN, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 1, ENABLE_LED_YELLOW, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 2, HOST_SERDES_TYPE_0, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 3, HOST_SERDES_TYPE_1, HIGH},
    {MANDATORY , INPUT  , PORT0 , 4, PRIMARY_INTERFACE_READY, LOW},
    {MANDATORY , INPUT  , PORT1 , 0, DYING_GASP_OK, HIGH},
    {MANDATORY , OUTPUT , PORT1 , 1, USB_DEBUG_ENABLE, LOW},
    {MANDATORY , OUTPUT , PORT1 , 2, WDISABLE_1, LOW},
    {MANDATORY , OUTPUT , PORT1 , 3, WDISABLE_2, LOW},
    {MANDATORY , OUTPUT , PORT1 , 6, RESET, LOW},
    {MANDATORY , OUTPUT , PORT1 , 7, MODEM_POWER_OFF, LOW},
};

int serial_gpio_exp_siz = sizeof(serial_gpio_exp_table)/sizeof(serial_gpio_exp);

/********************************************************************
 * Function   : plug_serial_gpio_exp_out_init
 * Description: Function to initialize the output value of each port 
 *              if this port is configured as output
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_serial_gpio_exp_out_init (void)
{
    int ix;

    for (ix = 0; ix < serial_gpio_exp_siz; ix++) {
        if (serial_gpio_exp_table[ix].dir == OUTPUT) {
            plug_gpio_exp_drive_port(serial_gpio_exp_table[ix].dev, 
                                     serial_gpio_exp_table[ix].port,
                                     serial_gpio_exp_table[ix].bit,
                                     serial_gpio_exp_table[ix].def_val);
        }
    }
    return (PASSED);
}

/********************************************************************
 * Function   : plug_serial_gpio_exp_dir_init
 * Description: Function to initialize port direction to Output/Input
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_serial_gpio_exp_dir_init (void)
{
    int ix;

    for (ix = 0; ix < serial_gpio_exp_siz; ix++) {
        plug_gpio_exp_config_port(serial_gpio_exp_table[ix].dev, 
                                  serial_gpio_exp_table[ix].port,
                                  serial_gpio_exp_table[ix].bit,
                                  serial_gpio_exp_table[ix].dir);
    }
    return (PASSED);
}

/********************************************************************
 *
 * Function   : plug_serial_enable_green_led 
 * Description: Function to toggle port dirve to High/Low
 * Inputs     : which_led - EN GREEN LED 
 * Outputs    : none 
 *
 ********************************************************************/
int plug_serial_enable_g_led (int led_on_off)
{
    plug_gpio_exp_drive_port(MANDATORY,
                             serial_gpio_exp_table[ENABLE_LED_GREEN].port,
                             serial_gpio_exp_table[ENABLE_LED_GREEN].bit,
                             led_on_off);
    return (PASSED);
}

/********************************************************************
 *
 * Function   : plug_serial_enable_y_led 
 * Description: Function to toggle port dirve to High/Low
 * Inputs     : which_led - EN YELLOW LED 
 * Outputs    : none 
 *
 ********************************************************************/
int plug_serial_enable_y_led (int led_on_off)
{
    /* Drive GPIO to High/Low */
    plug_gpio_exp_drive_port(MANDATORY,
                             serial_gpio_exp_table[ENABLE_LED_YELLOW].port,
                             serial_gpio_exp_table[ENABLE_LED_YELLOW].bit,
                             led_on_off);

    return (PASSED);
}

/********************************************************************
 *
 * Function   : plug_serial_enable_led 
 * Description: Function to toggle port dirve to High/Low
 * Inputs     : which_led - EN LED 
 * Outputs    : none 
 *
 ********************************************************************/
int plug_serial_enable_led (int led_action)
{

    switch(led_action) {
    case LED_GREEN:
        plug_serial_enable_g_led(HIGH);
        plug_serial_enable_y_led(LOW);
        break;
    case LED_AMBER:
        plug_serial_enable_g_led(LOW);
        plug_serial_enable_y_led(HIGH);
        break;
    case LED_OFF:
    default:
        plug_serial_enable_g_led(LOW);
        plug_serial_enable_y_led(LOW);
    }
    return (PASSED);
}

/*
 ***************************************************************************************
 *
 *  Function: pluggable_serial_card_test
 *
 *  Description: run Pluggable Serial card test automatically by sending a nc client request to the 
 *               nc server listening on Pluggable Serial side
 *
 *  Input: None 
 *
 *  Returns: PASSED/FAILED
 *
 ****************************************************************************************
 */
static int pluggable_serial_card_test(void)
{
    assert(plug_serial_iface);

    printf("\nStarting Pluggable Serial diag test with nc...\n");
    testname("Pluggable Serial Card");
    prpass(testpass, "Pluggable Serial diag test with nc");

    if (nc_cmd_run_pluggable_serial_diag(PLUG_SERIAL_REQUEST_PORT)) {
        cterr('f', 0, "HOST: NC command failed in run Pluggable Serial test\n");
        pluggable_serial_kill_nc();
        return (FAILED);
    }

    pluggable_serial_kill_nc();
    return (PASSED);
}
/*
 ***************************************************************************************
 *
 *  Function: pluggable_serial_bootup_image
 *
 *  Description: Pluggable Serial automatically boot up by sending U-boot command via UART 
 *               Didn't impact the IOS U-boot parameter.
 *
 *  Input: None 
 *
 *  Returns: PASSED/FAILED
 *
 ****************************************************************************************
 */
int pluggable_serial_bootup_image(void)
{
    int boot_timeout;
    char tty_dev[32];
    int ix, jx, val;
    char diag_bootp[128];
    int result = PASSED;
    struct plug_intf_t *plug;
    int boot_result = FAILED;

    testname("Boot Up Diag Image ");
    prpass(testpass, "U-boot ");

    assert(plug_serial_iface);

    /* Step 1 : Power cycle */
    plug = (struct plug_intf_t *)plug_serial_iface;
   
    if (plug_slot_reset(plug)) {
        cterr('f', 0, "Reset PluggSer %d fail", plug_serial_iface->slot);
        return (FAILED);
    }
    msleep(PLUG_SERIAL_UNRESET_WAIT);
    if (plug_slot_unreset(plug)) {
        cterr('f', 0, "Unreset testcard slot %d fail", plug->slot);
        return (FAILED);
    }

    sprintf(tty_dev, "/dev/%s", uart_device_name);
    sprintf(diag_bootp, "%s%s%s", PLUG_SERIAL_BOOT_CMD, tftp_server_ip, PLUG_SERIAL_BOOT_CMD_TAIL);

    /* Step 2 : Looking for auto boot prompt */
    /* CSCvm33713 : Fixed Serial_1T_bootup_faild by 60C in EEDVT  */
    printf("\nLooking for auto boot star prompt ...");
    fflush(stdout);
    boot_timeout = BOOT_TIMEOUT;
    do{
        result = plug_serial_rx_polling_uart(tty_dev, 
                                             PLUG_SERIAL_AUTOBOOT_PROMPT,
                                             PLUG_SERIAL_UBOOT_TIME);
        if (result != FALSE) {
            printf("Found \n");
            fflush(stdout);
            break;
        }
        msleep(PLUG_SERIAL_TIMEOUT); /* Wait for console printing completed */
    } while (boot_timeout--);
    if (boot_timeout <= 0) {
        printf("FAIL\n");
        printf("Failed to get '%s' bootloader prompt\n",
                PLUG_SERIAL_AUTOBOOT_PROMPT);
        fflush(stdout);
        goto exit_uboot_parms_set_failed;
    }  

    printf("\nLooking for bootloader prompt ...");
    fflush(stdout);


    /* Step 3 : Issue ctrl+C to stop autoboot of IOS */
    boot_timeout = BOOT_TIMEOUT;
    do{
        plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_C_STRING); /* ctrl+C to stop autoboot of IOS */
        plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_C_STRING); /* ctrl+C to stop autoboot of IOS */
        result = plug_serial_rx_polling_uart(tty_dev, PLUG_SERIAL_UBOOT_STRING,
                                             PLUG_SERIAL_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", PLUG_SERIAL_UBOOT_STRING);
            fflush(stdout);
            break;
        }
        msleep(PLUG_SERIAL_UNRESET_WAIT); /* Wait for U-boot receive command completed */
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        printf("Failed to get '%s' bootloader prompt\n",PLUG_SERIAL_UBOOT_STRING);
        goto exit_uboot_parms_set_failed;
    }    
    msleep(PLUG_SERIAL_CMD_WAIT_TIME); /* Wait for console printing completed */

    /* Step 4 : Boot Diag image */
    /* HW request add boot up command retry 
       CSCvm33713 : Fixed Serial_1T_bootup_faild by 60C in EEDVT  */
    for (jx = 0; jx < CHECK_IMAGE_BOOTUP; jx++) {
        plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_STRING);
        msleep(PLUG_SERIAL_TIMEOUT); /* Wait for console printing completed */
        plug_serial_tx_uart(tty_dev, diag_bootp);
        plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_STRING);
    
        printf("Booting pluggable serial image ...");
        fflush(stdout);

        /* Step 5 : Check Primary Interface Ready pin */
        /* poll for Primary Interface Ready pin (GPIO pin 4) which is set 
           by Pluggable Serial module side when the diag menu is up. */
        for (ix = 0; ix < PLUG_SERIAL_BOOT_TIME; ix++) {
            if (plug_gpio_exp_read_port(MANDATORY, PORT0, PRIMARY_READY, &val) == 
                                        FAILED) {
                plug_serial_enable_led(LED_AMBER);
                cterr('f', 0, "Unable to read PCA9555 register\n");
                goto exit_uboot_parms_set_failed;
            }
            if (val == HIGH) {
                boot_result = PASSED;
                break;
            }
            msleep(PLUG_SERIAL_WAIT_TIME);
        }
        if (boot_result == PASSED) {
            break;
        }    
        printf("Timeout waiting for primary interface ready pin asserted..."
               "retry\n");
    }

    if (jx == CHECK_IMAGE_BOOTUP) {
        plug_serial_enable_led(LED_AMBER);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        goto exit_uboot_parms_set_failed;
    }
    if (jx > 0) {
        printf("\nBoot Up Command Retry %d time \n", jx); /* print how many retry happened */
    }

    printf("Done\npluggable serial image is up!\n");
    fflush(stdout);
    return (PASSED);

exit_uboot_parms_set_failed:
    return (FAILED);
}
/*************************************************************************************************
 * Function: nc_cmd_run_pluggable_serial_diag
 * Description: Start the local nc server for receiving test status and initial the status file
 *              Send a nc client request to the module side nc server.
 *              Check the test status.
 *
 * Input:    port - ruuning diag request port number
 *
 * Return: PASSED / FAILED
 **************************************************************************************************
 */
static int nc_cmd_run_pluggable_serial_diag (int port)
{
    char cmdbuf[128];
    int check_flag;
    char my_nc_buf[NC_LENGTH]={0};
    char cmd[NC_LENGTH];
    char *search_str;

    assert(plug_serial_iface);
    
    printf("\nWait for Pluggable Serial module side to boot up diag menu.\n");

    if(pluggable_serial_bootup_image()) {
        cterr('f', 0, "Fail to boot up pluggable serial image\n");
        return (FAILED);
    }
    /* CSCvh67800: Fixed NC command failed intermittent issue */
    /* Check network connect, remove the NC retry */
    sprintf(cmd, "%s%s%s", ARP_PING_CMD, eth_num, ARP_PING_CMD_TAIL);
    if(ExecuteCmdbyPopen(cmd, my_nc_buf, sizeof(my_nc_buf)) == 0 ) {
        cterr('f', 0, "Network didn't not connect: command[%s]:[%s]\n", 
              cmd, my_nc_buf);
        fflush(stdout);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("arping command [%s]:[%s]\n", cmd, my_nc_buf);
        fflush(stdout);
    }
    search_str = strstr(my_nc_buf, ARP_PING_CHK_STRING);
    if (search_str == NULL) { 
        printf("Return Status is %s\n", my_nc_buf);
        fflush(stdout);
        return (FAILED);
    }
    printf("Network connect\n");

    pluggable_serial_get_host_flag();
    pluggable_serial_send_diag_flag();

    sprintf(cmdbuf, "nc %s %d\n", PLUG_SERIAL_LOCAL_IP_ADDR, port);
    printf("HOST: nc command: %s\n", cmdbuf);

    if (pluggable_serial_init_status_file()) {
        printf("Initial status file error.\n");
        return (FAILED);
    }

    if (system(cmdbuf)) {
        printf("Unable to request nc server.(%s)\n",cmdbuf);
        fflush(stdout);
        return (FAILED);
    }
    check_flag = pluggable_serial_check_test_status();

    if (check_flag == FAILED) {
        cterr('f', 0, "PLUGGABLE_SERIAL-%d test fails\n", plug_serial_iface->slot);
    } else if (check_flag == PASSED) {
        prpass(testpass, "PLUGGABLE_SERIAL-%d test passes\n", plug_serial_iface->slot);
    } else {
        cterr('f', 0, "NC Connection Error.\n");
        return (FAILED);
    }

    return (PASSED);
}

/********************************************************************************
 * Function: pluggable_serial_init_status_file
 * Description: This function create the status file if it doesn't exist
 *              and listen to the status port
 *
 * Input:  None
 * Output: PASSED/FAILED
 *
 ********************************************************************************
 */
static int pluggable_serial_init_status_file (void)
{
    char cmd1[84];
    char status_file[32];

    assert(plug_serial_iface);

    sprintf(status_file, "/tmp/pluggable_serial_%d.status", plug_serial_iface->slot);
    /* create or clear the status file */
    /*sprintf(cmd1, "rm -rf %s", status_file);*/
    sprintf(cmd1, "echo ' ' > %s", status_file);
    system(cmd1);

    /* Listen to the command status */
    sprintf(cmd1, "nc -l -p %d > %s &", PLUG_SERIAL_STATUS_PORT1, status_file);
    /* sprintf(cmd, "nc -l -l -p %d  > /dev/console &", PLUGGABLE_SERIAL_STATUS_PORT);*/
    printf("HOST: nc command: %s\n", cmd1);
    if (system(cmd1)) {
        return (FAILED);
    }
    return (PASSED);
}

/*****************************************************************
 *
 * Function: pluggable_serial_check_test_status
 *
 * Description: This function checks the content of status file and
 *              determine whether the test passes or fails.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************
 */
static int pluggable_serial_check_test_status (void)
{
    FILE *fp;
    char status_file[64];
    char buf[DIAG_RTN_STR_LEN + 1];
    char cmd[64];

    sprintf(status_file, "/tmp/pluggable_serial_%d.status", plug_serial_iface->slot);

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    if (fgets(buf, (DIAG_RTN_STR_LEN + 1), fp) != NULL) {
        sprintf(cmd, "cat %s", status_file);
        system(cmd);
        if (strcmp(buf, DIAG_RTN_PASS_STR)) {
            printf("Fail! Return Status is %s\n", buf);
            fflush(stdout);
            fclose(fp);
            return (FAILED);
         } else {
            fclose(fp);
            return (PASSED);
         }
    }

    printf("Warning: status file is empty.\n");
    fclose(fp);

    return (EMPTY);
}

/***************************************************************************
 *
 * Function: pluggable_serial_kill_nc
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
static void pluggable_serial_kill_nc (void)
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

    sprintf(cmd, "ps | grep 'nc %s' > %s", HOST_IP, DIAG_KILL_NC_TMP_FILE);
    system(cmd);
    printf("\nkill cmd: %s\n", cmd);

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        /* separate string of one line, get 1st substring pointer */
        token = strtok(buf, " ");
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

/**********************************************************************
 * Function: pluggable_serial_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void pluggable_serial_cleanup (void)
{
    assert(plug_serial_iface);

#ifdef PLUGGABLE_SERIAL_BACKEND_LOOPBACK
	disable_bp_ge_lpbk();
#endif

    if (pluggable_serial_saved_diag_exec) {
        pre_diag_exec = pluggable_serial_saved_diag_exec;
        pluggable_serial_saved_diag_exec = NULL;
    }
}

/*************************************************************************
 * Function: pluggable_serial_iface_test
 *
 * Test entry for Pluggable Serial interface test.
 *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int pluggable_serial_iface_test (void)
{

    /* Test pluggable serial reset pin */
    if (plug_serial_pin_test(TRUE) == FAILED) {
        return (FAILED);
    }
    /* Thermal Sensor Test */
    if (plug_serial_ts_test(TRUE) == FAILED) {
        return (FAILED);
    }
    /* PIO Expander Test */
    if (plug_serial_gpio_exp_test() == FAILED) {
        return (FAILED);
    }

    printf("\nWait for Pluggable Serial module side to boot up diag menu.\n");
    if(pluggable_serial_bootup_image()) {
        printf("\nFail to boot up pluggable serial image\n");
        return (FAILED);
    }

    /* Testing UART and GE0 interfaces */
    if (pluggable_serial_uart_test()) {
        plug_serial_enable_led(LED_AMBER);
        return (FAILED);
    }

    /* Testing Serdes type test */
    if (plug_serial_serdes_type_test()) {
        plug_serial_enable_led(LED_AMBER);
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}


/********************************************************************
 * Function   : plug_serial_reset_init
 * Description: Function to initialize Pluggable Serial by taking out reset 
 *              signal
 * Inputs     : None
 * Outputs    : PASSED OR FAILED 
 *
 ********************************************************************/
int plug_serial_reset_init (void)
{
    printf("Pluggable Serial Reset Initialization\n");
	
    /* Assert/Deassert Reset signal to Pluggable Serial */
    plug_gpio_exp_drive_port(MANDATORY,
                             serial_gpio_exp_table[RESET].port,
                             serial_gpio_exp_table[RESET].bit,
                             HIGH);
    msleep(PLUG_SERIAL_RESET_WAIT_IN_MS);
    plug_gpio_exp_drive_port(MANDATORY,
                             serial_gpio_exp_table[RESET].port,
                             serial_gpio_exp_table[RESET].bit,
                             LOW);

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: plug_serial_main().
 *
 * Description: This function is the entry point for Pluggable Serial test .
 *
 * Input:  in - pointer to plug_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int plug_serial_main (void *in)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    char tty_dev[32];
    FILE *file;
    char cmd[32];

    assert(in);

    plug_serial_iface = (struct plug_intf_t *)in;

    slot = plug_serial_iface->slot;
    board_id = plug_serial_iface->id;

    plug_serial_iface->uart_on(in);

    printf("\nPlug Serial test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Plugable Serial ", slot);

    /* Set Platform PHY with 1000Base-X mode. */
    if(plug_ser_host_set_1000basex_mode(slot) == FAILED){
        cterr('f', 0, "Set PHY 1000Base-X Mode Failed");
        return (FAILED);
    }

    /* Initialize GPIO Expander Output Value */
    if (plug_serial_gpio_exp_out_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Direction Failed");
        return (FAILED);
    }

    /* Initialize GPIO Expander Direction (Input/Output) */
    if (plug_serial_gpio_exp_dir_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Direction Failed");
        return (FAILED);
    }

    /* Pluggable Serial Reset Initialization */
    if (plug_serial_reset_init() == FAILED) {
        cterr('f', 0, "Pluggable Serial Reset Initialization Failed");
        return (FAILED);
    }
	
    if (tftp_get(0, (unsigned char *)PLUG_SERIAL_DEST_IMG, 
                 0, (unsigned char *)PLUG_SERIAL_SRC_IMG, 1) < 0) {
        cterr('f', 0, "Failed to tftp download firmware to local host");
        return(FAILED);
    }

    plug_serial_iface->unreset(in);
    msleep(PLUG_SERIAL_UNRESET_WAIT);
    /* turn on the green light */
	plug_serial_enable_led(LED_GREEN);

    /* Get TFTP Server IP */
    plug_ser_host_get_server_ip_ethnum(slot, eth_num, tftp_server_ip);
    /* Get UART device infomation */
    plug_ser_host_get_uart_info(slot, uart_device_name, uart_driver_path);

    /* Suppress printk to get UART driver message  */
    if (default_printk_level == -1) {
        file = fopen(SYS_PROC_PRINTK_FILE, "rb");
        if (file == NULL) {
            printf("%s; Warning! %s is not found\n", __func__, SYS_PROC_PRINTK_FILE);
            fflush(stdout);
        } else {
            fscanf(file, "%d", &default_printk_level);
            fclose(file);
        }
    }

    if (default_printk_level != -1) {
        sprintf(cmd, "%s %d", SYS_CHANGE_PRINTK_LEVEL, SYS_SUPPRESS_PRINTK_LEVEL);
        system(cmd);
    }

    plug_serial_insmod(TRUE);

    /* Set UART Device configuration */
    sprintf(tty_dev, "/dev/%s", uart_device_name);
    if(plug_serial_uart_setup(tty_dev) == FAILED) {
        printf("Failed to setup UART\n");
    }

    /* Check Pluggable Power OK */
    if (plug_is_pwr_ok(plug_serial_iface) == FALSE) {
        cterr('f', 0, "Pluggable Serial POWER_OK Status Is Not OK");
        return (FAILED);
    }

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    pluggable_serial_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    if (plug_serial_iface->test_type == IFACE_TEST) {
        ret_val = pluggable_serial_iface_test();
    } else {
        if (plug_serial_iface->menu_display == TRUE) {
            menu(maindiagp, main_menu_secondary_items, '\0');
        } else {
            do_all_menu_items(maindiagp);
        }
    }

    /* Restore printk */
    if (default_printk_level != -1) {
        sprintf(cmd, "%s %d", SYS_CHANGE_PRINTK_LEVEL, default_printk_level);
        system(cmd);
    }

    pluggable_serial_cleanup();
    plug_serial_insmod(FALSE);

    return (ret_val);
}

/*******************************************************************************
 * Function   : plug_serial_ts_test
 * Description: Thermal Sensor Test for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_serial_ts_test (int input)
{
    testname("Thermal Sensor");
    prpass(testpass, "Thermal Sensor");
    return (plug_temp_sensor_reg_test());
}


/*******************************************************************************
 * Function   : plug_serial_gpio_exp_test 
 * Description: GPIO Expander Test for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_serial_gpio_exp_test (void)
{
    printf("GPIO Expander (0x4E) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");
    return (plug_gpio_exp_reg_test(MANDATORY));
}

/*******************************************************************************
 * Function   : plug_serial_pin_test
 * Description: Test pluggable serial module i2c reset pin
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_serial_pin_test (int input)
{

    struct plug_intf_t *plug;
    int i2c_addr = PLUG_I2C_CTRL_OFFSET; 
    uchar data_buf[32];

    int ret = FAILED;

    testname("I2C Reset Pin");
    prpass(testpass, "Pin Test ");
    plug = (struct plug_intf_t *)plug_serial_iface;
    if (plug->slot == PLUG_SLOT_2) {
        i2c_addr = i2c_addr + PLUG_FPGA_I2C_OFFSET;
    }
    
    if (plug_serial_iface->i2c_reset(plug_serial_iface)) {
        cterr('f', 0, "Reset testcard slot %d fail", plug->slot);
        return (FAILED);
    }
    
    ret = plug_common_fpga_i2c_ack_check(i2c_addr, 0,PLUG_I2C_ADDR_ACT2, 0, 1, 1, 
                                  data_buf);
    
    /* ACK testing shoud not PASS when module reset */ 
    if (ret == PASSED) {
        ret = FAILED;
    } else {
        ret = PASSED;
    }

    if (plug_serial_iface->i2c_unreset(plug_serial_iface)) {
        cterr('f', 0, "Unreset testcard slot %d fail", plug->slot);
        return (FAILED);
    }
    msleep(PLUG_SERIAL_UNRESET_WAIT);
    
    if (ret == FAILED) {
        cterr('f', 0, "module reset ping test slot %d fail", plug->slot);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
*
* Function: pluggable_serial_get_host_flag
*
* Get current Host diag flags
*
* Input : none
*
* Output: none
*
**********************************************************************
*/
static void pluggable_serial_get_host_flag (void)
{
    char flag_file[32];
    char flags[256];
    char cmd[256];

    /* Write flags to local file */
    sprintf(flag_file, "%s", HOST_FLAG);
    sprintf(flags, "diagflag=%x\tdiagflag_xram=%x",
           (unsigned int)(NVRAM)->diagflag, (unsigned int)diagflag_xram);
    sprintf(cmd, "echo %s > %s", flags, flag_file);
    system(cmd);

    sprintf(cmd, "more  %s", flag_file);
    system(cmd);
}


/**********************************************************************
*
* Function: pluggable_serial_pass_diag_flag
*
* Send the Host diag flags to module side via nc command
*
* Input : none
*
* Output: none
*
**********************************************************************
*/
static int pluggable_serial_send_diag_flag (void)
{
    char flag_file[32];
    char nc_cmd[84];

    assert(plug_serial_iface);

    sprintf(flag_file, "%s", HOST_FLAG);
    /* HOST: send the flag */
    sprintf(nc_cmd, "nc %s %d < %s\n", PLUG_SERIAL_LOCAL_IP_ADDR, 
            PLUG_SERIAL_HOST_FLAG_PORT, flag_file);
    printf("HOST: nc command: %s\n", nc_cmd);

    if (system(nc_cmd)) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}
/*******************************************************************************
 * Function   : plug_serial_insmod
 * Description: To insert Sirius FPGA UART driver for the test
 * Inputs     : input - TRUE or FALSE
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_serial_insmod (int input)
{
    char cmd[256];
    if (input == TRUE) {
        sprintf(cmd, "insmod %s", uart_driver_path);
    } else {
        sprintf(cmd, "rmmod %s", uart_driver_path);
    }
    system(cmd);

    return (PASSED);
}
/*****************************************************************
 *
 * Function: plug_serial_uart_setup
 *
 * Description: This function setups UART parameter
 *
 * Input:  tty_dev: device string, (ie /dev/ttySIRIUS0)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */

int plug_serial_uart_setup (char *tty_dev)
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
/*****************************************************************
 *
 * Function: plug_serial_tx_uart
 *
 * Description: This function transmits strings into tty
 *
 * Input:  tty_dev: device string, (ie /dev/ttySIRIUS0)
 *         out_str: compared string
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */

int plug_serial_tx_uart (char *tty_dev, char *out_str)
{
    int uart_fd, cnt;
    int rc = PASSED;

    /* Sanity check */
    if (tty_dev == NULL || out_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR | O_NOCTTY);

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
 * Function: plug_serial_rx_polling_uart
 *
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 *
 * Input:  tty_dev: device string, (ie /dev/ttySIRIUS0)
 *         comp_str: compared string
 *         timeout: timeout value (ms)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int plug_serial_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
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
        return (FALSE);
    }

    uart_fd = open(tty_dev, O_RDWR | O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FALSE);
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
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("\nDBG:[%s]",buf);
                fflush(stdout);
            }

            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                close(uart_fd);
                return (TRUE);
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
    return (FALSE);
}
/*
 ***************************************************************************************
 *
 *  Function: plug_serial_serdes_type_test
 *
 *  Description: run Pluggable Serial Serdes type test automatically by sending a test  
 *               command and receive result from Pluggable Serial module side via UART
 *  CSCvm45577 : Pluggable Serial - SerDes Type test Failed	        
 *  Input: None 
 *
 *  Returns: PASSED/FAILED
 *
 ****************************************************************************************
 */
static int plug_serial_serdes_type_test (void)
{

    int val, ix, jx;
    char tty_dev[32];
    char exp_buf[64];
    int result = PASSED;
    assert(plug_serial_iface);

    testname("Pluggable Serial SerDes Type");
    prpass(testpass, "SerDes Type Test - ");

    sprintf(tty_dev, "/dev/ttySIRIUS%d", plug_serial_iface->slot - 1);
    if(plug_serial_uart_setup(tty_dev) == FAILED) {
        printf("Failed to setup UART\n");
    }
    /* Check Primary Interface Ready pin */
    if (plug_gpio_exp_read_port(MANDATORY, PORT0, PRIMARY_READY, &val) == 
                                FAILED) {
        plug_serial_enable_led(LED_AMBER);
        cterr('f', 0, "Unable to read PCA9555 register\n");
        goto exit_serdes_type_failed;
    }
    if (val != HIGH) {
        plug_serial_enable_led(LED_AMBER);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        goto exit_serdes_type_failed;
    }
    printf("\nPluggable serial is up!\n");
    printf("\nStarting Pluggable Serial SerDes Type test...\n");
    fflush(stdout);
    msleep(PLUG_SERIAL_CMD_WAIT_TIME); /* Wait for console printing completed */
    /* There are 4 combinations with Serdes type */
    for (ix = 0; ix < FOUR_SERDES_TYPE; ix++) {
        /* Setup Host SerDes Type GPIO */
        plug_gpio_exp_drive_port(MANDATORY, 
                                 serial_gpio_exp_table[HOST_SERDES_TYPE_0].port,
                                 serial_gpio_exp_table[HOST_SERDES_TYPE_0].bit,
                                 serdes_type[ix * 2]);    
        plug_gpio_exp_drive_port(MANDATORY,
                                 serial_gpio_exp_table[HOST_SERDES_TYPE_1].port,
                                 serial_gpio_exp_table[HOST_SERDES_TYPE_1].bit,
                                 serdes_type[(ix * 2) + 1]);    
        /* Get Module SerDes Type GPIO value via UART */
        for (jx = 0; jx < CHECK_EXP_DATA; jx++) {
            sprintf(exp_buf, "SERDES_TYPE_0: %d ; SERDES_TYPE_1: %d", 
                    serdes_type[ix * 2], serdes_type[(ix * 2) + 1]);
            plug_serial_tx_uart(tty_dev, PLUG_SERIAL_SERDES_TYPE_TEST); 
            result = plug_serial_rx_polling_uart(tty_dev, exp_buf, PLUG_SERIAL_TIMEOUT * 2);
            if (result == TRUE) {
                break;
            }
            msleep(PLUG_SERIAL_WAIT_ONE_SEC);
        }
        if (jx == CHECK_EXP_DATA) {
            cterr('f', 0, "%s: Failed to receive expected data.\n", __FUNCTION__);
            return FAILED;
        }
        msleep(PLUG_SERIAL_WAIT_ONE_SEC);
    }
    return (PASSED);
exit_serdes_type_failed:
    return (FAILED);
}

/**********************************************************************
 * Function: pluggable_serial_uart_test
 *
 * Description: This function performs the uart interface test for the 
                Pluggable Serial
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
int pluggable_serial_uart_test (void)
{

    int ix, val;
    char *sd_pattern = "n\n"; /* n: Dummy item to send string to UART */
    char *exp_pattern = "Linux";
    char tty_dev[32];
    int result = PASSED;

    assert(plug_serial_iface);
    testname("Pluggable Serial Uart");
    prpass(testpass, "Uart Test - ");

    printf("\nWaiting for Pluggable Serial module boot up diag menu.\n");
    /* poll for Primary Interface Ready pin (GPIO pin 4) which is set 
       by Pluggable Serial module side when the diag menu is up. */
    for (ix = 0; ix < PLUG_SERIAL_TIMEOUT; ix++) {
        if (plug_gpio_exp_read_port(MANDATORY, PORT0, PRIMARY_READY, &val) == 
                                    FAILED) {
	        plug_serial_enable_led(LED_AMBER);
            cterr('f', 0, "Unable to read PCA9555 register\n");
            return (FAILED);
        }

        if (val == HIGH) {
            break;
        }
        print_spining_wheel(ix);
        msleep(PLUG_SERIAL_WAIT_TIME);
    }
    if (ix == PLUG_SERIAL_TIMEOUT) {
        plug_serial_enable_led(LED_AMBER);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }
    msleep(PLUG_SERIAL_CMD_WAIT_TIME);

    /* 'n\n' for trigger Module side diag main menu item,
     * which will invoke 'uname'.
     */

    sprintf(tty_dev, "/dev/%s", uart_device_name);
    /* Set UART Device configuration */
    if(plug_serial_uart_setup(tty_dev) == FAILED) {
        printf("Failed to setup UART\n");
    }

    plug_serial_tx_uart(tty_dev, sd_pattern); 
    result = plug_serial_rx_polling_uart(tty_dev, exp_pattern, PLUG_SERIAL_TIMEOUT);
    if (result == FALSE) {
        cterr('f', 0, "%s: Failed to receive expected data.\n", __FUNCTION__);
        return FAILED;
    }
    printf("Found : %s\n", exp_pattern);
    fflush(stdout);
    return (PASSED);
}

/******** History ********
$Log: platform_plug_serial_test.c,v $
Revision 1.8  2018/11/23 09:28:46  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.7  2018/09/25 08:28:55  iachang
CSCvm33713: Fixed Serial_1T_bootup_faild by 60C in EEDVT

Revision 1.6  2018/09/21 03:01:16  iachang
CSCvm45577: Fixed SerDes Type GPIO test issue

Revision 1.5.10.2  2018/11/21 09:37:22  iachang
Sync up with main trunk.

Revision 1.5.10.1  2018/10/15 06:51:13  hondwang
pluggable common code re-instruct modify code

Revision 1.5  2018/08/06 11:17:15  iachang
Bump up version to v4.0.3

Revision 1.4  2018/08/02 09:35:01  iachang
Merge Pluggable Serial from branch star-branch-c9xx to main trunk

Revision 1.3  2018/02/09 09:17:33  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:54:53  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:58:56  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.25  2017/12/13 11:45:44  iachang
Add Pluggable Serial Bootup utility.

Revision 1.1.4.24  2017/12/08 02:16:30  iachang
LED GPIO pin is active high, different with Prince.

Revision 1.1.4.23  2017/11/09 09:37:18  iachang
Suppress printk to get UART driver message
Restore IOS U-BOOT parameters utility

Revision 1.1.4.22  2017/11/02 07:34:09  iachang
Supported Enhance Error Message

Revision 1.1.4.21  2017/11/02 06:41:53  iachang
Add SerDes Type test in the I/O interface test

Revision 1.1.4.20  2017/10/26 14:53:43  iachang
Modify I/O interface test.
Fixed UART test issue.

Revision 1.1.4.19  2017/10/24 11:16:05  iachang
Supported SerDes Type GPIO Test.

Revision 1.1.4.18  2017/10/18 22:45:06  iachang
Remove dummy code and fix compile error.

Revision 1.1.4.17  2017/10/18 09:42:45  iachang
Check Power OK pin.

Revision 1.1.4.16  2017/10/13 02:53:17  iachang
Sent boot up command to module via UART, dind't impact IOS U-boot parameter.

Revision 1.1.4.15  2017/10/11 07:10:31  iachang
Changed firmware name from nim_serial_fw.img to p_1t_fw.img

Revision 1.1.4.14  2017/09/27 00:06:28  iachang
Moved Listen Host diag flags NC command to module kernel initial script
Added insert Sirius FPGA UART driver

Revision 1.1.4.13  2017/09/26 03:28:27  iachang
Changed Reset Pin test from Module reset to I2C reset Pin

Revision 1.1.4.12  2017/09/25 15:26:43  iachang
Fixed Reset Pin test.

Revision 1.1.4.11  2017/09/22 16:57:45  iachang
Add I/O interface test into pluggable sub-menu for bring up

Revision 1.1.4.10  2017/09/20 07:09:10  lucywang
set GE1 to 1000Base-X for pluggable serial module

Revision 1.1.4.9  2017/09/13 16:54:29  iachang
Support Pluggable Serial test via NC command

Revision 1.1.4.8  2017/09/13 14:07:45  iachang
Fixed Primary Interface Ready pin.

Revision 1.1.4.7  2017/09/12 07:46:32  iachang
Skip gephy_set_1000basex_mode function.

Revision 1.1.4.6  2017/09/08 09:54:13  iachang
Add UART Test in the default test item.

Revision 1.1.4.5  2017/08/24 06:46:17  lucywang
add reset test for pluggable serial module

Revision 1.1.4.4  2017/08/23 05:46:33  lucywang
enable/disable Receiver to Tansmitter in local PHY for pluggable serial module

Revision 1.1.4.3  2017/08/22 03:29:58  lucywang
set 1000Base-X for pluggable serial and set sgmii for pluggable test card


$Endlog$
*/



