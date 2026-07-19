/* $Id: platform_plug_serial_util.c,v 1.5 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/platform_plug_serial_util.c,v $
 *------------------------------------------------------------------
 *
 * plug_serial_util.c - PLUGGABLE Serial Utility
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "plug_slot.h"
#include "platform_plug_serial_util.h"
#include "platform_plug_serial_test.h"
#include "plug_gpio_exp_test.h"
#include "plug_temp_sensor_test.h"
#include "plug_temp_sensor_lib.h"
#include "proto.h"
#include "pca.h"
#include "common_utils.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "mem_mgr.h"
#include "linux_api.h"
#include "plug_gpio_exp_lib.h"
#include "platform_plug_serial_host.h"

int pluggable_serial_pwr_off(void);
int pluggable_serial_pwr_on(void);
void disable_bp_ge_lpbk(void);
void enable_bp_ge_lpbk(void);
static int pluggable_serial_power_off(void);
static int pluggable_serial_power_on(void);
static int pluggable_serial_pwr_cycle(void);
static int pluggable_serial_console_switch(void);
static int pluggable_serial_bp_ge_test(void);
static int pluggable_serial_reset(void);
static int pluggable_serial_gpio_exp_util(int);
static int pluggable_serial_show_temp(int);
static int pluggable_serial_ts_util(int);
static int pluggable_serial_led_util(int input);
static int platform_shell(void);
static int shell_command(void);
static int restore_ios_parameter_util(int);

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t pluggable_serial_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)pluggable_serial_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Pluggable Serial",     (PFT)pluggable_serial_power_on,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off Pluggable Serial",      (PFT)pluggable_serial_power_off,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Pluggable Serial",    (PFT)pluggable_serial_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Backplane GE Utility",            (PFT)pluggable_serial_bp_ge_test,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Reset Pluggable Serial",            (PFT)pluggable_serial_reset,         0,    0,
     (type_t(*)())0, 0,    (type_t(*)())0,           0},
    {"GPIO Expander Register Read/Write Utility", (type_t(*)())pluggable_serial_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Display Utility", (type_t(*)())pluggable_serial_show_temp, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Sensor Register Read/Write Utility", (type_t(*)())pluggable_serial_ts_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LED Utility", (type_t(*)())pluggable_serial_led_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Restore IOS U-BOOT Parameters", (type_t(*)())restore_ios_parameter_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Pluggable Serial Bootup", (type_t(*)())pluggable_serial_bootup_image, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Escape to Shell (debugging only)", (type_t(*)())platform_shell, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Execute a Shell command (debugging only)", (type_t(*)())shell_command, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUGGABLE_SERIAL_UTILS_SUBMENU_TABLE_SZ (sizeof(pluggable_serial_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t pluggable_serial_utils_primary_items[PLUGGABLE_SERIAL_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t pluggable_serial_utils_secondary_items[PLUGGABLE_SERIAL_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];

char pluggable_serial_utiltitle[50];

menuinfo_t pluggable_serial_util_submenu = {
    pluggable_serial_utiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,            /* notes missing Plugs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    pluggable_serial_utils_primary_items,
};

menuinfo_t *pluggable_serial_util_submenup = &pluggable_serial_util_submenu;


/* U-Boot Environment Tables */
static uboot_info_t uboot_table[] =
{
/*  Uboot name,        value  */
    {"baudrate"     , "9600"},
    {"bootargs"     , "console=ttyPS0,9600 root=/dev/ram rw ip=none earlyprintk"},
    {"bootcmd"      , "run modeboot"},
    {"bootdelay"    , "3"},
    {"copybootbin"  , "echo BOOTP BOOT.BIN to RAM...;bootp 0x00400000 firmware/BOOT.BIN"},
    {"diag_bootm"   , "bootm 0x00400000"},
    {"diag_bootp"   , "echo BOOTP Diag to RAM and boot...;bootp 0x00400000 firmware/p_1t_fw.img"},
    {"diagboot"     , "run diag_bootp diag_bootm"},
    {"ethact"       , "cisco_psge_emac"},
    {"ethaddr"      , "30:f7:0d:54:e9:64"},
    {"fdt_high"     , "0x07700000"},
    {"initrd_high"  , "0x07700000"},
    {"ipaddr"       , "10.100.0.3"},
    {"jtagboot"     , "echo TFTPing Linux to RAM...;tftp 0x10000 ${multi_image};bootm 0x10000"},
    {"loadaddr"     , "0x400000"},
    {"modeboot"     , "run qspiboot"},
    {"multi_image"  , "multi.img"},
    {"nandboot"     , "echo Copying Linux from NAND flash to RAM...;nand read 0x3000000 0x100000 ${kernel_size};nand read 0x2A00000 0x600000 ${devicetree_size};echo Copying ramdisk...;nand read 0x2000000 0x620000 ${ramdisk_size};bootm 0x3000000 0x2000000 0x2A00000"},
    {"netmask"      , "255.255.255.0"},
    {"norboot"      , "echo Copying Linux from NOR flash to RAM...;cp 0xE2100000 0x3000000 ${kernel_size};cp 0xE2600000 0x2A00000 ${devicetree_size};echo Copying ramdisk...;cp 0xE2620000 0x2000000 ${ramdisk_size};bootm 0x3000000 0x2000000 0x2A00000"},
    {"prince_bootm" , "bootm 0x00400000"},
    {"prince_bootp" , "echo BOOTP Linux to RAM and boot...;bootp 0x00400000 firmware/p_1t_fw.img"},
    {"qspiboot"     , "run prince_bootp prince_bootm"},
    {"sdboot"       , "echo Copying Linux from SD to RAM...;mmcinfo;fatload mmc 0 0x06000000 multi.img;bootm 0x06000000"},
    {"serverip"     , "10.100.0.1"},
    {"stderr"       , "serial"},
    {"stdin"        , "serial"},
    {"stdout"       , "serial"},
};
#define UBOOT_TABLE_SZ (sizeof(uboot_table) / sizeof(uboot_info_t))

/*
 **********************************************************************
 *
 *  Function: pluggable_serial_utils
 *
 *  Description: Pluggable Serial Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
int pluggable_serial_utils (void)
{
    assert(plug_serial_iface);

    sprintf(pluggable_serial_utiltitle, "Pluggable Serial Slot %d Utilities Menu",
            plug_serial_iface->slot);
    build_primary_submenu(pluggable_serial_utils_submenu_table,
                          PLUGGABLE_SERIAL_UTILS_SUBMENU_TABLE_SZ,
                          pluggable_serial_utiltitle, &pluggable_serial_util_submenup);

    build_secondary_submenu(pluggable_serial_utils_submenu_table,
                            PLUGGABLE_SERIAL_UTILS_SUBMENU_TABLE_SZ,
                            pluggable_serial_utils_secondary_items);

    menu(pluggable_serial_util_submenup, pluggable_serial_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: pluggable_serial_console_switch
 *
 *  Description: Pluggable Serial console switch utility
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int pluggable_serial_console_switch (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    char cmd[maxlen];
    snprintf(tty, maxlen-1, "/dev/%s", uart_device_name);

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
                          "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(WAIT_SCREEN_PRINT); /* pause a second for the NOTE: */

    snprintf(cmd, maxlen - 1, "picocom -b%d -d8 -p1 -fn %s",
             BAUD9600, tty); 

    system(cmd);

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: disable_bp_ge_lpbk
 *
 *  Description: Disable Platform GE loopbcak register.
 *
 *  Input: None 
 *
 *  Returns: None
 *
 **********************************************************************
 */
void disable_bp_ge_lpbk (void)
{
    /* clear bit[15] of Digitable Loopback Enable Register */
	plug_ser_host_set_loopback_mode(plug_serial_iface->slot, 0);	
}

/*
 **********************************************************************
 *
 *  Function: enable_bp_ge_lpbk
 *
 *  Description: Enable Platform GE loopbcak register.
 *
 *  Input: None 
 *
 *  Returns: None
 *
 **********************************************************************
 */
void enable_bp_ge_lpbk ()
{
	/* enable bit[15] of Digitable Loopback Enable Register, pluggable interface at Lane1/SGMII2, offset comphy num1: 0xF212188C */
	plug_ser_host_set_loopback_mode(plug_serial_iface->slot, 1);
}

/*
 **********************************************************************
 *
 *  Function: get_bp_ge_loopback
 *
 *  Description: Get the Platform GE loopbcak status.
 *
 *  Input: None 
 *
 *  Returns: None
 *
 **********************************************************************
 */
static int get_bp_ge_loopback()
{
	return plug_ser_host_get_loopback_mode(plug_serial_iface->slot);
}

/**********************************************************************
 *
 * Function: pluggable_serial_bp_ge_test
 * Description: This function provides tests for Pluggable Serial port of Backplane GESW
 *
 *  Input: None 
 *
 *  Returns: PASSED
 **********************************************************************
 */
static int pluggable_serial_bp_ge_test (void)
{
    uchar type = 'e';
    int stop = 0;
    int state = -1;

    printf("\nBackplane GE Utility\n"); 

    while (1) {
        printf("\na: enable motherboard line loopback at Pluggable Serial GE port\n");
        printf("b: disable motherboard line loopback at Pluggable Serial GE port\n");
        printf("c: get Pluggable Serial GE port loopback setting\n");
        printf("e: exit\n");
        type = getc_answer("Select an option", "abce", 'e');
        switch(type) {
        case 'a':
            enable_bp_ge_lpbk();
            break;
        case 'b':
            disable_bp_ge_lpbk();
            break;
        case 'c':
            state = get_bp_ge_loopback();
            if (state) {
                printf("line loopback has been enabled.\n");
            } else {
                printf("line loopback has been disabled.\n");
            }
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
 * Function: pluggable_serial_power_on
 *
 * Description: This function is called for power-on utiliy.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pluggable_serial_power_on (void)
{
    assert(plug_serial_iface);

    if(pluggable_serial_pwr_on()) {
        return (FAILED);
    }

    printf("Pluggable Serial Card is powered up.\n");

   return (PASSED);
}

/**********************************************************************
 *
 * Function: pluggable_serial_power_off
 *
 * Description: This function is called for power-off utility.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pluggable_serial_power_off (void)
{
    uint8_t ans;

    assert(plug_serial_iface);

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Pluggable Serial Card Still Power On.\n\n");
        return (PASSED);
    }

    if (pluggable_serial_pwr_off()) {
        return (FAILED);
    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: pluggable_serial_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pluggable_serial_pwr_cycle (void)
{
    uint8_t ix, ans;

    printf("\n");
    printf("Power Cycle the Pluggable Serial Card");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Pluggable Serial is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (pluggable_serial_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Pluggable Serial Card");
        return (FAILED);
    }

    /* msleep for 10 seconds. */
    for (ix = 0; ix < 10; ix++) {
        printf(".");
        msleep(1000);
    }

    if (pluggable_serial_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Pluggable Serial Card");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: pluggable_serial_pwr_off
 *
 * Description: This function does all necessary configuration to power off.
 *              reset module, power off, i2c reset.
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int pluggable_serial_pwr_off (void)
{
    int slot;

    assert(plug_serial_iface);

    slot = plug_serial_iface->slot;
    printf("\nPower Off the Pluggable Serial Card.\n");

    plug_serial_enable_led(LED_OFF);

	/* power off */
    plug_serial_iface->i2c_reset(plug_serial_iface);
    plug_serial_iface->off(plug_serial_iface);
	
    return (PASSED);
}

/***************************************************************************
 *
 * Function: pluggable_serial_pwr_on
 *
 * Description: This function does all necessary configuration to power on.
 *              enable Pluggable Serial Card, power it on, take it out of reset.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 ***************************************************************************
 */
int pluggable_serial_pwr_on (void)
{
    printf("\nPower On the Pluggable Serial Card.\n");

    assert(plug_serial_iface);

    /* enable pluggabl-serial and take I2C out of reset */
    plug_slot_i2c_poweron_unreset(plug_serial_iface, plug_serial_iface->slot, "PLUG");

	plug_serial_enable_led(LED_AMBER);

	/* power on */
	plug_serial_iface->on(plug_serial_iface);
    msleep(200);

    /* make sure the power is output good */
	/* check status */
	if(!plug_is_pwr_ok(plug_serial_iface)){
		printf("PLUG SERIAL CANNOT be Turned On.\n");
        	return (FAILED);
	}
	
    printf("Waiting for Pluggable Serial Card to Power-Up.\n");
    msleep(2000);

    /* take Pluggable Serial Card out of reset */
    plug_serial_iface->unreset(plug_serial_iface);

    /* turn on the green light */
	plug_serial_enable_led(LED_GREEN);
    plug_serial_iface->uart_on(plug_serial_iface);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: pluggable_serial_reset
 *
 * Description: This function query for reset or unreset  Pluggable Serial Card module.
 *              It doesn't reset or unreset i2c.
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int pluggable_serial_reset (void)
{
    uint8_t ans;

    assert(plug_serial_iface);

    printf("\nReset or Unreset Pluggable Serial module? (r/u): ");
    ans = getchar();
    putchar(ans);
    printf("\n");
    if (ans == 'r' || ans == 'R') {
        if (plug_serial_iface->reset(plug_serial_iface)){
            printf("Unable to reset Pluggable Serial Module\n");
            return (FAILED);
        }
        msleep(1000);
    } else if (ans == 'u' || ans == 'U') {
         if (plug_serial_iface->unreset(plug_serial_iface)){
            printf("Unable to unreset Pluggable Serial Module\n");
            return (FAILED);
         }
        msleep(1000);
    } else {
        printf("ABORT!\n");
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : pluggable_serial_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int pluggable_serial_gpio_exp_util (int input)
{
    int opt;

    printf("GPIO Expander Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ, 
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        return (plug_gpio_exp_show_reg(MANDATORY));
    } else {
        return (plug_gpio_exp_alter_reg(MANDATORY));
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : pluggable_serial_show_temp
 * Description: This function display temperature detected by temperature sensor
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int pluggable_serial_show_temp (int input)
{
    return (plug_ts_show_temp());
}

/*******************************************************************************
 * Function   : pluggable_serial_ts_util
 * Description: Thermal Sensor Utility for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int pluggable_serial_ts_util (int input)
{
    int opt;

    printf("Temperature Sensor Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ, 
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        return (plug_temp_sensor_show_reg());
    } else {
        return (plug_temp_sensor_alter_reg());
    }
}

/*******************************************************************************
 * Function   : pluggable_serial_led_util
 * Description: LED Utility for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int pluggable_serial_led_util (int input)
{
    int opt;

    printf("LED Utility\n");
    opt = getdec_answer("LED action? (0-OFF, 1-AMBER, 2-GREEN):", LED_OFF, 
                         LED_OFF, LED_GREEN);

    if (opt == LED_AMBER) {
        return (plug_serial_enable_led(LED_AMBER));
    }else  if (opt == LED_GREEN){
        return (plug_serial_enable_led(LED_GREEN));
    }else{
    	 return (plug_serial_enable_led(LED_OFF));
    }
}
/**********************************************************************
 *
 * Function: platform_shell
 *
 * This function to escaping to shell bash.
 *
 * Input : None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int platform_shell (void)
{
    printf("\nEscaping to Shell from Main Menu,\n");
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return (PASSED);
}


/**********************************************************************
 *
 * Function: shell_command
 *
 * This function enter shell command
 *
 * Input : void
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int shell_command (void)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return (PASSED);
}
/*******************************************************************************
 * Function   : restore_ios_parameter_util
 * Description: Restore IOS U-boot parameter
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int restore_ios_parameter_util (int input)
{
    int ix;
    char tty_dev[32];
    char cmdbuf[512];
    int result = PASSED;
    int boot_timeout;

    assert(plug_serial_iface);

    sprintf(tty_dev, "/dev/%s", uart_device_name);

    /* Step 1 : Reset Module CPU */
    if (plug_slot_reset(plug_serial_iface)) {
        cterr('f', 0, "Reset PluggSer %d fail", plug_serial_iface->slot);
        return (FAILED);
    }
    msleep(PLUG_SERIAL_UNRESET_WAIT);
    if (plug_slot_unreset(plug_serial_iface)) {
        cterr('f', 0, "Unreset testcard slot %d fail", plug_serial_iface->slot);
        return (FAILED);
    }

    /* Step 2 : Issue ctrl+C to stop autoboot of IOS */
    printf("\nLooking for bootloader prompt ...");
    fflush(stdout);
    boot_timeout = BOOT_TIMEOUT;
    do{
        plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_C_STRING); /* ctrl+C to stop autoboot of IOS */
        result = plug_serial_rx_polling_uart(tty_dev, PLUG_SERIAL_UBOOT_STRING,
        		                             PLUG_SERIAL_TIMEOUT);
        if (result != FALSE) {
            printf("\nFound : %s", PLUG_SERIAL_UBOOT_STRING);
            fflush(stdout);
            break;
        }
    } while (boot_timeout--);

    if (boot_timeout <= 0) {
        printf("FAIL\n");
        fflush(stdout);
        printf("Failed to get '%s' bootloader prompt\n",PLUG_SERIAL_UBOOT_STRING);
        goto exit_uboot_parms_set_failed;
    }    
    msleep(PLUG_SERIAL_UNRESET_WAIT);

    /* Step 3 : Program IOS parameter for U-Boot */
    printf("\nProgram IOS parameter ...");
    fflush(stdout);
    for (ix = 0; ix < UBOOT_TABLE_SZ; ix++) {
        sprintf(cmdbuf, "setenv %s '%s'\n", uboot_table[ix].name, uboot_table[ix].value);
        if (plug_serial_tx_uart(tty_dev, cmdbuf) == FAILED) {
            goto exit_uboot_parms_set_failed;
        }    
        if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_STRING) == FAILED) {
            goto exit_uboot_parms_set_failed;
        } 
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send [%s]\n", cmdbuf);
            fflush(stdout);
        }
        msleep(PLUG_SERIAL_WAIT_TIME);
    }
    /* Step 4 : Save Environment Variables */
    printf("\nSave Environment In U-Boot ...");
    fflush(stdout);
    if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_UBOOT_SAVE) == FAILED) {
        goto exit_uboot_parms_set_failed;
    } 
    if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_STRING) == FAILED) {
        goto exit_uboot_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("U-boot command [%s]\n", PLUG_SERIAL_UBOOT_SAVE);
        fflush(stdout);
    }
    msleep(PLUG_SERIAL_CMD_WAIT_TIME);

    /* Step 5 : Verify Environment Variables */
    printf("\nVerify Environment Variables ...");
    fflush(stdout);
    if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_DISPLAY_UBOOT) == FAILED) {
        goto exit_uboot_parms_set_failed;
    } 
    if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_STRING) == FAILED) {
        goto exit_uboot_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("U-boot command [%s]\n", PLUG_SERIAL_DISPLAY_UBOOT);
        fflush(stdout);
    }
    for (ix = 0; ix < UBOOT_TABLE_SZ; ix++) {
        sprintf(cmdbuf, "%s=%s\n", uboot_table[ix].name, uboot_table[ix].value);
        result = plug_serial_rx_polling_uart(tty_dev, cmdbuf,
        		                     PLUG_SERIAL_TIMEOUT);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (result == TRUE) {
                printf("Found : %s\n", cmdbuf);
                fflush(stdout);
            }
        }
        if (result == FALSE) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                if (result == TRUE) {
                    printf("Check 2'nd time : %s\n", cmdbuf);
                    fflush(stdout);
                }
            }
            if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_DISPLAY_UBOOT) == FAILED) {
                goto exit_uboot_parms_set_failed;
            } 
            if (plug_serial_tx_uart(tty_dev, PLUG_SERIAL_CR_STRING) == FAILED) {
                goto exit_uboot_parms_set_failed;
            } 
            msleep(PLUG_SERIAL_CMD_WAIT_TIME);
            result = plug_serial_rx_polling_uart(tty_dev, cmdbuf,
            		                     PLUG_SERIAL_TIMEOUT);
            if (result == FALSE) {
                printf("Not Found : %s\n", cmdbuf);
                fflush(stdout);
                break;
            }
        }
    }
    printf("\nRestore IOS U-Boot variables Completed ...");
    fflush(stdout);
    return (PASSED);
exit_uboot_parms_set_failed:
    return (FAILED);
}

/******** History ********
$Log: platform_plug_serial_util.c,v $
Revision 1.5  2018/11/23 09:28:46  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.4.10.1  2018/10/15 06:51:13  hondwang
pluggable common code re-instruct modify code

Revision 1.4  2018/08/02 09:35:01  iachang
Merge Pluggable Serial from branch star-branch-c9xx to main trunk

Revision 1.3  2018/02/09 09:17:34  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:54:53  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:58:56  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.15  2017/12/13 11:45:44  iachang
Add Pluggable Serial Bootup utility.

Revision 1.1.4.14  2017/11/17 02:53:44  iachang
Update U-BOOT parameters

Revision 1.1.4.13  2017/11/09 09:37:18  iachang
Suppress printk to get UART driver message
Restore IOS U-BOOT parameters utility

Revision 1.1.4.12  2017/10/26 14:53:43  iachang
Modify I/O interface test.
Fixed UART test issue.

Revision 1.1.4.11  2017/10/13 02:51:15  iachang
Modify the UART Test.

Revision 1.1.4.10  2017/10/02 03:39:03  iachang
Change UART read one byte to read all bytes.

Revision 1.1.4.9  2017/09/22 15:59:22  iachang
Support UART test.

Revision 1.1.4.8  2017/09/15 04:30:37  lucywang
added utility to enable/diable motherboard line loopback for Pluggable Serial GE port, not work yet

Revision 1.1.4.7  2017/09/14 07:36:34  iachang
Add Shell item for debugging

Revision 1.1.4.6  2017/09/12 08:23:46  iachang
Fixed console redirect function.

Revision 1.1.4.5  2017/09/12 07:46:53  iachang
Fixed pluggable_serial_gpio_exp_util utility.

Revision 1.1.4.4  2017/08/23 05:46:33  lucywang
enable/disable Receiver to Tansmitter in local PHY for pluggable serial module

Revision 1.1.4.3  2017/08/22 03:29:58  lucywang
set 1000Base-X for pluggable serial and set sgmii for pluggable test card


$Endlog$
*/
