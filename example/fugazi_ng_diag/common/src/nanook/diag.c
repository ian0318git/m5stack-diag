 /* $Id: diag.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Nanook diagmon main menu and supporting wrappers.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/mman.h>
#include "common.h"
#include "error.h"
#include "proto.h"
#include "types.h"
#include "setjmps.h"
#include "monitor.h"
#include "nvmonvars.h"
#include "menu.h"
#include "error.h"
#include "plat_defs.h"
#include "platform_cookie.h"
#include "mb_tests.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "diag_usb_util.h"
#include "diag_cpu_util.h"
#include "tam_act2_api_drv_support.h"
#include "dash_fpga.h"
#include "diag_rtc_util.h"
#include "diag_led_test.h"
#include "diag_spi_flash_util.h"
#include "diag_emmc_util.h"
#include "diag_fpga_util.h"
#include "slot.h"
#include "ngio.h"
#include "diag_temp_snsr_test.h"
#include "diag_m2_test.h"
#include "diag_rtc_test.h"
#include "uio_utils.h"
#include "common_utils.h"
#include "dnv_eth_lib.h"

#define INTEL_LAN_CONTROL_PORTS         0

#define ETH0 0    /* 88E1543,   NIC3 */
#define ETH1 1    /* 88E1543,   NIC4 */
#define ETH2 2    /* NIM,       NIC1 */
#define ETH3 3    /* AlleyCat3, NIC2 */

#define NIC1 1 
#define NIC2 2 
#define NIC3 3 
#define NIC4 4 
#define NAL_CMD            "nal"
#define MAC_SET_CMD        "eeupdate64e /NIC=%d /MAC=%.2x%.2x%.2x%.2x%.2x%.2x"
#define MAC_GET_CMD        "ifconfig %s | grep HWaddr | sed -n 's/.*'HWaddr'//p' | sed s/[[:space:]]//g | tr '[:upper:]' '[:lower:]'> %s"

/*
 * Declare external function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int show_emmc_info(void);
extern int emmc_pslc_fully_enable(int);
extern int emmc_full_test(int);
//extern int nanook_show_fpga_ver(int);
extern char *banner_string;
extern int nanook_cpu_ondie_temp (int opt);
extern int diag_temp_sensor_show_temp(void);
extern int diag_full_load_util(void);
extern void build_fan_menu(void);
extern boolean menu_display(void);
extern int crocus_show_fpga_ver (int opt);
extern int fpga_vol_margin (int);
extern int get_pwr_seq_fw_rev (int option);
extern int get_mac_from_block(uint32_t, uchar *);
extern uint32_t get_mac_blk_size();

/*
 * Declare local function
 */

static int diag_sys_info_util(int);
static int diag_ex_feature_util(int);

static int diag_vol_margin_util(int);
static int diag_mb_temper_util(int);
//static int io_interface_tests(void);

static int enable_uart(int);
static int display_regs(int);
static int set_uart_lpbk (int);

static int program_eth_mac(int);
static int verify_mac_program_result(void);

/*
 *  Globals  
 */
int netflashbooted = 0; /* menu.c need this */

/*
 * Main menu -> Basic utility -> Memory Utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory", 0, 0,
     (PFT) alt_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"compare memory block", 0, 0,
     (PFT) cmp_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"display memory", 0, 0,
     (PFT) dis_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"move memory block", 0, 0,
     (PFT) mov_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"fill memory", 0, 0,
     (PFT) fil_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"find memory", 0, 0,
     (PFT) memtest, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"memory read or write loop", 0, 0,
     (PFT) memloop, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"memory debug loop", 0, 0,
     (PFT) memdebug, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"address loop", 0, 0,
     (PFT) addrloop, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
};

static struct menuinfo mem_debug_menu = {
    "Memory Utility Menu",
    0,
    0,
    0,
    sizeof(mem_debug_items) / sizeof(struct mitem),
    mem_debug_items,
};

static struct menuinfo *mem_debug_menup = &mem_debug_menu;

static struct mitem uart_items[] = {
    {"AUX: Uart loopback",         0, 0,
     (type_t(*)())set_uart_lpbk, &one, 0, (type_t(*)())0, 0},
    {"AUX: write to Uart ",         0, 0,
     (type_t(*)())set_uart_lpbk, &zero, 0, (type_t(*)())0, 0},
    {"display UART regs",         0, 0,
     (type_t(*)())display_regs, &zero, 0, (type_t(*)())0, 0},
    {"enable UART intr",         0, 0,
     (type_t(*)())enable_uart, &zero, 0, (type_t(*)())0, 0},

};

static struct menuinfo uart_menu = {
    "  UART  utility Menu",
    0,
    0,
    0,
    sizeof(uart_items)/sizeof(struct mitem),
    uart_items,
};
static struct menuinfo *uart_menup = &uart_menu;

/* 
 * Cookie menu utility
 */

static struct mitem cookie_items[] = {
    {"alter MB CPU cookie", 0, 0,
     (type_t(*)()) alter_mb_cookie,  &one,  0,  (type_t(*)())0,  0},
    {"program Intel LAN control MAC",  0,  0,
     (type_t(*)())program_eth_mac,  &zero,  0,  (type_t(*)())0, 0},
    {"Check MAC programming result",  0,  0,
     (type_t(*)())verify_mac_program_result,  &zero,  0,  (type_t(*)())0,  0},
    {"alter NIM Slot 2 cookie", 0, 0,
     (type_t(*)()) alter_nim_cookie, &one, 0, (type_t(*)())is_nanook, 0},
    {"alter NIM Slot 3 cookie", 0, 0,
     (type_t(*)()) alter_nim_cookie, &two, 0, (type_t(*)())is_nanook_plus, 0},
};

static struct menuinfo cookie_menu = {
    "Cookie utility Menu",
    0,
    0,
    0,
    sizeof(cookie_items) / sizeof(struct mitem),
    cookie_items,
};

static struct menuinfo *cookie_menup = &cookie_menu;


/*
 * Main menu -> Basic utilities
 */

static struct mitem utilmenuitems[] = {
    {"System Information", 0, 0,
     (PFT) diag_sys_info_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Memory debug utilities", 0, 0,
     (PFT) menu, (type_t *) &mem_debug_menup, 0,
     (type_t(*)())0, 0},

    {"I2C utility", 0, 0,
     (PFT) build_i2c_util_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Cookie utility", 0, 0,
     (PFT) menu, (type_t *) &cookie_menup, 0,
     (type_t(*)())0, 0},

    {"USB Utilies", 0, 0,
     (PFT) nanook_usb_utils, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Extended Feature", 0, 0,
     (PFT) diag_ex_feature_util, (type_t *) &one, MF_HIDDEN_EXE,
     (type_t(*)())menu_display, 0},

    {"LED Utilities", 0, 0,
     (PFT) build_led_util_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"FPGA utilities",          0, 0,
     (PFT)menu, (type_t *)&reggio_fpga_menup, 0,
     (type_t(*)())0,0},

    {"UART utility",          0, 0,
     (PFT)menu, (type_t *)&uart_menup, 0,
     (type_t(*)())0,0},

    {"M/B Temperature Utilites", 0, 0,
     (PFT) diag_mb_temper_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Full Load Utility", 0, 0,
     (PFT) diag_full_load_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Intel Denverton Interface Utility", 0, 0,
     (PFT) build_dnv_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Enable eMMC pSLC mode", 0, 0,
     (PFT) emmc_pslc_fully_enable, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"Show eMMC info", 0, 0,
     (PFT) show_emmc_info, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"eMMC full test", 0, 0,
     (PFT) emmc_full_test, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"RTC Utility", 0, 0,
     (PFT) build_rtc_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},
     
    {"SPI Flash Utility",
    0, 0,
    (PFT)nanook_spi_flash_utils, (type_t *)&zero, 0,
    (type_t(*)())0, 0},

    {"FAN utilities",
    0,                 0,
    (PFT)build_fan_menu, (type_t *)&zero,                0,
    (type_t(*)())0,            0},

    {"Check M2 Device utilities",
    0,                 0,
    (PFT)check_m2_device_utility, (type_t *)&zero,                0,
    (type_t(*)())0,            0},
    
    {"Voltage Margin", 0, 0,
     (PFT) diag_vol_margin_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

#if 0
    {"AIKIDO SPI read", 0, 0,
     (type_t(*)())aikido_spi_read_util, &zero, 0, (type_t(*)())0, 0},

    {"AIKIDO SPI write", 0, 0,
     (type_t(*)())aikido_spi_write_util, &zero, 0, (type_t(*)())0, 0},

    {"toggle flag : aikido_mailbox_flag", 0, 0,
     (type_t(*)())aikido_flag_mailbox, &zero, 0, (type_t(*)())0, 0},

    {"toggle flag: aikido_act2_flag", 0, 0,
     (type_t(*)())aikido_flag_act2, &zero, 0, (type_t(*)())0, 0},
#endif

};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems) / sizeof(struct mitem),
    utilmenuitems,
};

struct menuinfo *utilmenup = &utilmenu;

/*
 * Main menu
 */

submenu_xtable_t main_menu_table[] = {
    {"ACT-2 utilities and programming",
     (PFT) smartchip, FALSE,
     MF_CONTINUOUS,
     (type_t(*)())0, 0, (PFT) smartchip, TRUE},

    {"Motherboard tests",
    (PFT) mb_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) mb_tests, FALSE},

    {"test NIM Slot 2",
    (PFT)wic_test,             FIRST_SLOT + 1,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())is_nanook, 0,
    (PFT)wic_test,  FIRST_SLOT + 1 + MAX_WIC},

    {"test NIM Slot 3",
    (PFT)wic_test,             FIRST_SLOT + 2,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())is_nanook_plus, 0,
    (PFT)wic_test,  FIRST_SLOT + 2 + MAX_WIC},

};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Main menu primary & secondary submenu items
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",                  /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};

static struct menuinfo *maindiagp = &maindiag;

void diag_menu (int argc, char *argv[])
{
    char arg = 0;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    menu(&maindiag, main_menu_secondary_items, arg);
}


/**********************************************************************
 *
 * Function: diag_sys_info_util
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_sys_info_util (int dummy)
{
    nanook_show_cpuinfo();
    nanook_show_meminfo();
    crocus_show_fpga_ver(0);
    nanook_show_glory_fpga_ver(0);
    get_pwr_seq_fw_rev(1);

    system("/printver_gen.sh");
    printf("%s", banner_string);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: display_nanook_sku_info
 *
 * Description: display Nanook SKU info
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int display_nanook_sku_info (void)
{   
    uchar mb_get_pid[64] = {0};
    ushort controller_type = 0x0;


    /* Get controller type */
    controller_type = get_mb_id();

    /* Get PID */
    platform_get_pid((char *)mb_get_pid);

    printf("\n");
    printf("PID             : %s\n", mb_get_pid);
    printf("Controller Type : [0x%x]\n", controller_type);


    return (PASSED);

}


/**********************************************************************
 *
 * Function: diag_ex_feature_util
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_ex_feature_util (int dummy)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_vol_margin_util
 *
 * Description: Utility to modify voltage margin
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_vol_margin_util (int dummy)
{
    uint8_t v_status = 0;
    unsigned int val;

    dash_fpga_reg_read(FPGA_VTG_MRG_CTRL, &val);
    printf("Current voltage margin level is %x\n", val);

    v_status = (uint8_t)gethex_answer("Enter Voltage Margin mode"
                                      "(0-Normal, 1-Low, 2-High): ",
                                      0, 0, 2);
    if (val != v_status) {
        fpga_vol_margin(v_status);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_mb_temper_util
 *
 * Description: display mother board temperature info
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_mb_temper_util (int dummy)
{
    nanook_cpu_ondie_temp(0);
    printf("Thermal Sensor\n ");
    show_temperature_all();	
    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_uart_lpbk
 *
 * Description: entry point to put FPGA uart in lpbk
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
set_uart_lpbk (int val)
{
    uart_lpbk(val);
    return PASSED;
}

/**********************************************************************
 *
 * Function: display_regs
 *
 * Description: display uart reg
 *
 * Input : NONE
 *
 * Output: PASSED
 *
 ***********************************************************************
 */
static
int display_regs(int d)
{
    display_uart_regs(d);
    return PASSED;
}

/**********************************************************************
 *
 * Function: enable_uart
 *
 * Description: enable uart
 *
 * Input : d -- not used
 *
 * Output: PASSED
 *
 ***********************************************************************
 */
static
int enable_uart(int d)
{
    enable_platform_uart_intr(0x1FF);
    uio_enable_intr();

    return (PASSED);
}

#if 0
/*-------------------------------------------------------------------
 *
 * Function: io_tests()
 *
 * Description : io interface tests.
 *
 * Inputs: N/A
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
static int io_interface_tests (void)
{
    int rc = FAILED;
    char *tname = "I/O interface";
    
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (plug_intf_test((PLUG_SLOT_1)) == PASSED) {
        rc = PASSED;
        prpass(testpass, "%s PLUG %d detected.", tname, PLUG_SLOT_1);
    }

    prcomplete(testpass, errcount, (char *)0);
    
    return (rc); 
}
#endif

/**********************************************************************
 *
 * Function   : has_daughter_card
 * Description: Function to detect if this board has daughter card.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_daughter_card (int opt)
{
    uint reg_addr;
    uint32_t  reg_value;
    
    reg_addr = FPGA_DB_PRESENT_REG;
    dash_fpga_reg_read(reg_addr, &reg_value);
	
    if(reg_value & FPGA_DB_PRESENT_BIT) {
        return (FALSE);
    }
	
    return (TRUE); /* tbd : Glory LPC 0xc4 bit 0, 0 : inserted */
}

/**********************************************************************
 *
 * Function: program_eth_mac
 *
 * Description: entry point to execute eepudate64 and nal
 *
 * Input : port_type - 0 : Marvell 88E1543 ports 
 * Input : port_type - 1 : Xilinx NIM port
 * Input : port_type - 2 : Marvell AlleyCat3 port
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static int program_eth_mac(int port_type)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0, max_eth_num;
    char cmd[128] = {0};
    int nic_num = 0;
    
    size = get_mac_blk_size() - 1;
    if (size < 4) {
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }
    
    /* This for-loop is just for code sharing purpose. 
     * Port means:
     * ETH0: enp38s0f0 0 is for the 88E1543 Port 0.
     * ETH1: enp38s0f1 1 is for the 88E1543 Port 1.
     * ETH2: enp36s0f0 2 is for the NIM Slot.
     * ETH3: enp36s0f1 3 is for the AlleyCat3.
     */

    if (port_type == INTEL_LAN_CONTROL_PORTS) {
        if (is_nanook_plus())
            max_eth_num = ETH3;
        else
            max_eth_num = ETH2;
    } else {
        printf("port_type is wrong.\n");
        return(FAILED);
    }

    for (port = ETH0; port <= max_eth_num; port++) {
        switch (port) {
        case ETH0:
            nic_num = NIC3;
            break;
        case ETH1:
            nic_num = NIC4;
            break;
        case ETH2:
            nic_num = NIC1;
            break;
        case ETH3:
            nic_num = NIC2;
            break;
        }
        
        get_mac_from_block(port, buf);
        if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
              (*buf+5))) {
            printf("error getting mac base addr; did u run cookie util yet?\n");
            printf("Please run 'alter mb cpu cookie' and 'display cookie'"
               " content at least once.\n");
            return(FAILED);
        }
               
        system(NAL_CMD);
        
        sprintf(cmd, MAC_SET_CMD, nic_num, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        printf("%s\n", cmd);
        system(cmd);
        
        system(NAL_CMD);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: verify_mac_program_result 
 *
 * Description: Check MAC programming result, read out MAC from Linux user 
 *              space and check against the cookie MAC contents
 *
 * Input : NONE 
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int verify_mac_program_result (void)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0;
    char cmd[128] = {0};
    FILE *fp;
    char *check_mac_file = "/nanook-diag/check_mac.txt";
    char rd_mac[128] = {0};
    int rc;
    char cookie_mac[128] = {0};
    char *eth_str = NULL;
    
    size = get_mac_blk_size() - 1;
    if (size < 4) {
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }

    /* This for-loop is just for code sharing purpose. 
     * Port means:
     * ETH0: enp38s0f0 0 is for the 88E1543 Port 0.
     * ETH1: enp38s0f1 1 is for the 88E1543 Port 1.
     * ETH2: enp36s0f0 2 is for the NIM Slot.
     * ETH3: enp36s0f1 3 is for the AlleyCat3.
     */
    for (port = ETH0; (port <= ETH2) || (port == ETH3 && is_nanook_plus()); port++) {
        rc = FAILED;
        
        get_mac_from_block(port, buf);

        if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
              (*buf+5))) {
            printf("error getting mac base addr; did u run cookie util yet?\n");
            printf("Please run 'alter mb cpu cookie' and 'display cookie'"
               " content at least once.\n");
            return (rc);
        }

        sprintf(cookie_mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

        switch (port) {
        case ETH0:
            eth_str = inface_lan1p0;
            break;
        case ETH1:
            eth_str = inface_lan1p1;
            break;
        case ETH2:
            eth_str = inface_lan0p0;
            break;
        case ETH3:
            eth_str = inface_lan0p1;
            break;
        }
        
        if (!eth_str || !strlen(eth_str)) {
            printf("Unable to get infacename\n");
            return (rc);
        }
        
        sprintf(cmd, MAC_GET_CMD, eth_str, check_mac_file);
        system(cmd);

        fp = fopen(check_mac_file, "r");
        if (fp == NULL) {
            printf("Unable to open file - %s\n", check_mac_file);
            return (rc);
        }

        while (!feof(fp)) {
            fgets(rd_mac, sizeof(rd_mac), fp);
            if (strstr(rd_mac, cookie_mac) != NULL) {
                rc = PASSED;
            } 
        }

        sprintf(cmd, "rm -f %s", check_mac_file);
        system(cmd);
        fclose(fp);

        if (rc == PASSED) {
            printf("%s MAC program successfully! expected - %s, got - %s\n", eth_str, cookie_mac, rd_mac);
        } else {
            printf("Error - %s MAC didn't program correctly! expected - %s, got - %s\n", eth_str, cookie_mac, rd_mac);
            return (rc);
        }
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.3  2020/04/20 02:28:24  lucywang
 *
 * 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
 * 2. Added to support NIM Prince
 * 3. (CSCvn43011) add retry workaround for Deverton issue
 * 4. add debug message and set default value to seneors
 * 5. Reverted Register value of temp/press snsr after test
 * 6. Bumped up version to 1.0.2
 *
 * Revision 1.2  2019/12/11 10:10:27  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
