/* $Id: diag.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Fugazi diagmon main menu and supporting wrappers.
 *
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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
#include <assert.h>
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
#include "platform_tam_cookie.h"
#include "platform_led.h"
#include "platform_i2c.h"
#include "mb_tests.h"
#include "i2c_api.h"
#include "platform_prom.h"
#include "dash_fpga.h"
#include "uio_utils.h"
#include "queryflags.h"
#include "linux_api.h"
#include "linux_usb_test.h"
#include "diag_bcm82757_test.h"
#include "diag_bcm57412_test.h"
#include "cookie_4.h"
#include "platform_eth.h"
#include "diag_bcm_lib.h"
#include "tam_aikido_upgrade.h"
#include "m2_testcard.h"
#include "m2_testcard_host_impl.h"

/*
 * Declare external function
 */
extern char *banner_string;

/*
 * Declare local function
 */

static int diag_sys_info_util(int);
static int display_brd_info(int);
static int reset_dev(int);
static int common_pci_read(void);
static int platform_shell(void);
static int shell_command(void);
static int program_eth_mac(void);
static int program_i211_mac(void);
static int program_bcm57412_mac(void);
static int verify_mac_program_result(void);

/*
 *  Globals  
 */
int netflashbooted = 0; /* menu.c need this */
int reset_sys_by_watchdog(void);

/* Function prototype */
extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem();
extern int  alter_mb_cookie(void);
extern int act1_prog(unsigned int, unsigned char *choice);
extern int smartchip(int submenu_flag);
extern int program_reggio_spi_prom(void);
extern void tam_aikido_reset_utilty(void); 
extern void build_fan_menu(void);
extern int rtc_utility_main(int);
extern int  build_margin_menu(void);
extern int platform_ser_irq_intr_test(int dummy);
extern char *strcasestr(char* , char*);
extern  void platform_env_status(void);
extern int program_spi_update_version(void);
extern void bcm54194_reset(int);




static struct mitem reggio_fpga_items[] = {
    {"Platfrom FPGA Program SPI PROM image without header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,  &zero, 0, (type_t(*)())0, 0},
    {"Platform FPGA Program SPI PROM image with header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,   &one, 0, (type_t(*)())0, 0},
    {"Aikido Program FPGA SPI PROM image", 0, 0, 
     (type_t(*)())program_reggio_spi_prom,      &zero, 0, (type_t(*)())0, 0},
    {"Erase/Program Image Upgrade Header",  0, 0,
     (type_t(*)())program_image_upgrade_header,   &one, 0, (type_t(*)())0, 0},
    {"Set FPGA update flag",  0, 0,
     (type_t(*)())program_image_update_type,   &one, 0, (type_t(*)())0, 0},
    {"Set FPGA revision and date",  0, 0,
     (type_t(*)())set_date_revision,   &one, 0, (type_t(*)())0, 0},
    {"Display FPGA MULTI BOOT registers",  0, 0,
     (type_t(*)())display_multiboot,   &one, 0, (type_t(*)())0, 0},
    {"Display a sector", 0, 0, 
     (type_t(*)())display_prom_sector,   &zero, 0, (type_t(*)())0, 0},
    {"Test NIOS SPI",  0, 0,
     (type_t(*)())nios_test_spi_prom,  &three, 0, (type_t(*)())0, 0},
    {"Show Board type/FPGA Version",           0, 0,
     (type_t(*)())display_brd_info, &one, 0, (type_t(*)())0, 0},
    {"Erase Config header Sector",           0, 0,
     (type_t(*)())erase_config_header, &one, 0, (type_t(*)())0, 0},
    {"Program Boot Upgrade Flags",           0, 0,
     (type_t(*)())program_boot_upgrade_flag, &one, 0, (type_t(*)())0, 0},
    {"Reset internal devices",           0, 0,
     (type_t(*)())reset_dev, &zero, 0, (type_t(*)())0, 0},
    {"Reset external devices",           0, 0,
     (type_t(*)())reset_dev, &one, 0, (type_t(*)())0, 0},
    {"rd/wr test",           0, 0,
     (type_t(*)())dash_rd_wr_test, &one, 0, (type_t(*)())0, 0},
    {"FPGA intr test", 0, 0,
     (type_t(*)())platform_intr_test,   &one, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0, 0},
    {"serial IRQ intr test", 0, 0,
     (type_t(*)())platform_ser_irq_intr_test,   &one, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0, 0},
    {"Toggle to CPLD/FPGA (default FPGA)", 0, 0, 
     (type_t(*)())dash_set_map,   &one, 0, (type_t(*)())0, 0},
    {"FPGA read", 0, 0, 
     (type_t(*)())dash_dis_mem,   &one, 0, (type_t(*)())0, 0},
    {"FPGA fill", 0, 0, 
     (type_t(*)())dash_fil_mem,   &one, 0, (type_t(*)())0, 0},
    {"FPGA alter", 0, 0, 
     (type_t(*)())dash_alt_mem,   &one, 0, (type_t(*)())0, 0},
    {"Modify Aikido SPI Directory Table", 0, 0, 
     (type_t(*)())program_spi_update_version,      &zero, 0, (type_t(*)())0, 0},
    {"AIKIDO SPI read", 0, 0, 
     (type_t(*)())aikido_spi_read_util, &zero, 0, (type_t(*)())0, 0},
    {"AIKIDO SPI write", 0, 0, 
     (type_t(*)())aikido_spi_write_util, &zero, 0, (type_t(*)())0, 0},
    {"AIKIDO reset and unreset", 0, 0, 
     (type_t(*)())tam_aikido_reset_utilty, &zero, 0, (type_t(*)())0, 0},
    {"toggle flag : aikido_mailbox_flag", 0, 0, 
     (type_t(*)())aikido_flag_mailbox, &zero, 0, (type_t(*)())0, 0},
    {"toggle flag: aikido_act2_flag", 0, 0, 
     (type_t(*)())aikido_flag_act2, &zero, 0, (type_t(*)())0, 0},
    {"CPLD reset(CPU self-reset)", 0, 0, 
     (type_t(*)())cpld_reset, &zero, 0, (type_t(*)())0, 0},
#ifdef AIKIDO_DEV_KEY
    {"Program Aikido FPGA DEV keys (Development phase)", 0, 0,
    (type_t(*)())program_aikido_dev_key, &zero, 0, (type_t(*)())0, 0},
#endif
};

static struct menuinfo reggio_fpga_menu = {
    "  FPGA utility Menu",
    0,
    0,
    0,
    sizeof(reggio_fpga_items)/sizeof(struct mitem),
    reggio_fpga_items,
};
static struct menuinfo *reggio_fpga_menup = &reggio_fpga_menu;

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

/* 
 * Cookie menu utility
 */

static struct mitem cookie_items[] = {
    {"alter MB CPU cookie", 0, 0,
     (type_t(*)()) alter_mb_cookie, &one, 0, (type_t(*)())0, 0},
    {"program I211/1G/10G port MAC",     0,	0,
     (type_t(*)())program_eth_mac,	  &zero,	0, (type_t(*)())0, 0},
    {"Check MAC programming result",     0,	0,
     (type_t(*)())verify_mac_program_result,	  &zero,	0, (type_t(*)())0, 0},
    {"program I211 MAC (option)",     0,	0,
     (type_t(*)())program_i211_mac,	  &zero,	0, (type_t(*)())0, 0},
    {"program BCM57412 MAC (option)",     0,	0,
     (type_t(*)())program_bcm57412_mac,	  &one,	0, (type_t(*)())0, 0},
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

    {"PCIE read utility", 0, 0,
     (PFT)common_pci_read, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"System Information", 0, 0,
     (PFT) diag_sys_info_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},
    {"Memory debug utilities", 0, 0,
     (PFT) menu, (type_t *) &mem_debug_menup, 0,
     (type_t(*)())0, 0},
    {"FPGA utilities",          0, 0,
     (PFT)menu, (type_t *)&reggio_fpga_menup, 0,
     (type_t(*)())0,0},
    {"I2C utility", 0, 0,
     (PFT) build_i2c_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},
    {"Margin utilities",      0,			0,
     (PFT)build_margin_menu,	(type_t *)&zero,		0,
     (type_t(*)())0,		0},
     {"Cookie utility", 0, 0,
     (PFT) menu, (type_t *) &cookie_menup, 0,
     (type_t(*)())0, 0},
    {"USB/Compact Flash utility",
     0,			0,
     (PFT)usb_utils_v2,	(type_t *)&one,           0,
     (type_t(*)())0,		0},
    {"RTC utilities",
     0,			0,
     (PFT)rtc_utility_main,	(type_t *)&zero,		0,
     (type_t(*)())0,		0},
    {"FAN utilities",
     0,                 0,
     (PFT)build_fan_menu, (type_t *)&zero,                0,
     (type_t(*)())0,            0},
    {"Mother LED utility",
     0,                      0,
     (PFT)menu,              (type_t *)&led_menup,      0,
     (type_t(*)())0,         0},
    {"Reset system by watchdog", 0, 0, 
     (PFT)reset_sys_by_watchdog, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"Escape to Shell (debugging only)",  0, 0,   (PFT)platform_shell,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    {"Execute a Shell command (debugging only)",   0, 0,  (PFT)shell_command,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
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

    /* M.2 Test Card: if present, show this test; not present, not show*/
    {"M.2 testcard test",
    (PFT)m2_testcard_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (PFT)is_m2_testcard_in, 0, (PFT)m2_testcard_test,  FALSE},
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

static int diag_platform_init(int argc, char *argv[])
{
    if (fugazi_diag_init(argc, argv) < 0) {
        return (FAILED);
    }

    /* Diag needs to unreset 1G PHY at Diag bootup, since From Fugazi FPGA
       v2.0 and up, will put 1G PHY in reset by default when power on. */
    printf("Reset and init 1GE PHY...\n");
    bcm54194_reset(0);

    return (PASSED);
}

static void diag_platform_exit(void)
{
    fugazi_diag_exit();
}


void diag_menu (int argc, char *argv[])
{
    char arg = 0;
    if (argc > 1) {
        arg = *argv[1];
    } else {
        arg = 0;
    }
    (NVRAM)->pollcon = 1;       /* poll the console */
    if (diag_platform_init(argc, argv)) {
        cterr('f', 0, "diag platform init failed");
    }

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    menu(&maindiag, main_menu_secondary_items, arg);
    diag_platform_exit();
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
    
    platform_env_status();

    /* Show bcm82757 firmware version */
    if ((fugazi_bcm82757_init(fugazi_struct))) {
        cterr('f', 0, "fugazi_bcm82757_init failed");
    }
    bcm82757_show_fw_version();

    system("/printver_gen.sh");
    printf("%s", banner_string);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: reset_sys_by_watchdog
 *
 * Description: reboot system by enableing watchdog timer.
 *              this test will reboot system!!!
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int reset_sys_by_watchdog (void)
{
    int ix;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    printf("System will be reboot by watchdog after 5 sec.\n");
    cpld->wdog = WATCHDOG_TIMEOUT;
    for (ix = 0; ix < WATCHDOG_COUNT; ix++) {
        printf("\n%d", WATCHDOG_COUNT - ix);
        sleep(1);
    }
    return (PASSED);

}

/**********************************************************************
 *
 * Function: display_brd_info
 *
 * Description: display board info, ie version number, revision number,
 *              etc...
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int display_brd_info(int val)
{
    unsigned int fpga_ver, cpld_ver, fpga_brd, cpld_brd;
    get_platform_ver(1, &fpga_ver, &cpld_ver, &fpga_brd, &cpld_brd);
    return (PASSED);
}

static int reset_dev(int val)
{
    unsigned int c;
    c = getdec_answer("enter '1' to reset; enter '0' to unreset", 1, 0, 1);

    if (val == 1) {
        dash_reset_ext(c);
    }
    if (val == 0) {
        dash_reset_int(c);
    }
    return PASSED;
}

/**********************************************************************
 *
 * Function: common_pci_read
 *
 * Description: PCI read util for configuration space read
 *
 * Input : None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int common_pci_read (void) {

   int bus, dev, func, reg;
    do {
        bus = gethex_answer("BUS", 0, 0, 0xFF);
        dev = gethex_answer("DEV", 0, 0, 0xFF);
        func = gethex_answer("FUN", 0, 0, 0xFF);
        reg = gethex_answer("REG", 0, 0, 0xFF);


        reg = pci_config_read(bus, dev, func, reg);

        printf("reg === 0x%x \n", reg);


    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return (PASSED);
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

/**********************************************************************
 *
 * Function: program_eth_mac
 *
 * Description: entry point to execute diag_util function script 
 *
 * Input : NONE 
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static int program_eth_mac(void)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    unsigned int  size = 0;
    unsigned int port = 0;
    char cmd[128];

    /* Fugazi:
     * The managemenet port is implemented with i211
     * The other 4 10G ports and 8 1G ports.
     * In the kernel, these ports are eth0-3 and eth4-11, eth12 for i211 as shown below.
     * offset	description	count
     * 0	    gig0/0/0	8        
     * 8	    te0/1/0	    4            
     * 12	    mgmte	    1            
     * 13	    spare	    14           
     * 27	    VM	        32               
     * 59	    MST	        1                
     * 60	    overlay	    1            
     * 61	    appliance	2        
     * 63	    BDI	        1                
     * 64	    portchannel	64       
     * 		                128                 
     *
     * Chasis mac base and blocksize are values stored in MB cookie.
     *
     * The MAC address progarmming of this ports uses the Intel
     * eepudate utility and Boradcom tool. The utility is put under
     * /diag_utils/fugazi/scripts/function.sh in the kernel rootfs.
     */
    /* Fugazi size is 128 */
    size = get_mac_blk_size();
    printf("size is %d\n", size);
    if (size < 10) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }
    
    /* Fugazi port offset start from 0 */
    port = 0;
    get_mac_from_block(port, buf);
    if (!((*buf) || *(buf + 1) || *(buf + 2) || *(buf + 3) || *(buf + 4) ||
          (*buf + 5))) {
        printf("error getting mac base addr; did u run cookie util yet?\n");
        printf("Please run 'alter mb cpu cookie' and 'display cookie'"
	       " content at least once.\n");
        return(FAILED);
    }

    printf("eth%d: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
           port, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    sprintf(cmd, "%s;%s %.2x%.2x%.2x%.2x%.2x%.2x %d", SOURCE_SCRIPT, 
            FUGAZI_UPDATE_MAC, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], 
            size);
    printf("%s\n", cmd);
    system(cmd);
    msleep(10);
    printf("\nMAC program successfully! Please power cycle the system\n");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: program_i211_mac
 *
 * Description: entry point to execute diag_util function script 
 *
 * Input : NONE 
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static int program_i211_mac(void)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    unsigned int  size = 0;
    unsigned int port = 0;
    char cmd[128];

    /* Fugazi:
     * The managemenet port is implemented with i211
     * The other 4 10G ports and 8 1G ports.
     * In the kernel, these ports are eth0-3 and eth4-11, eth12 for i211 as shown below.
     * offset	description	count
     * 0	    gig0/0/0	8        
     * 8	    te0/1/0	    4            
     * 12	    mgmte	    1            
     * 13	    spare	    14           
     * 27	    VM	        32               
     * 59	    MST	        1                
     * 60	    overlay	    1            
     * 61	    appliance	2        
     * 63	    BDI	        1                
     * 64	    portchannel	64       
     * 		                128                 
     *
     * Chasis mac base and blocksize are values stored in MB cookie.
     *
     * The MAC address progarmming of this ports uses the Intel
     * eepudate utility and Boradcom tool. The utility is put under
     * /diag_utils/fugazi/scripts/function.sh in the kernel rootfs.
     */
    /* Fugazi size is 128 */
    size = get_mac_blk_size();
    printf("size is %d\n", size);
    if (size < 10) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }
    
    /* Fugazi port offset start from 0 */
    port = 0;
    get_mac_from_block(port, buf);
    if (!((*buf) || *(buf + 1) || *(buf + 2) || *(buf + 3) || *(buf + 4) ||
          (*buf + 5))) {
        printf("error getting mac base addr; did u run cookie util yet?\n");
        printf("Please run 'alter mb cpu cookie' and 'display cookie'"
	       " content at least once.\n");
        return(FAILED);
    }

    printf("eth%d: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
           port, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    sprintf(cmd, "%s;%s %.2x%.2x%.2x%.2x%.2x%.2x %d", SOURCE_SCRIPT, 
            FUGAZI_UPDATE_I211_MAC , buf[0], buf[1], buf[2], buf[3], buf[4], 
            buf[5], size);
    printf("%s\n", cmd);
    system(cmd);
    msleep(10);
    printf("\nMAC program successfully! Please power cycle the system\n");
    return (PASSED);
}
/**********************************************************************
 *
 * Function: program_bcm57412_mac
 *
 * Description: entry point to execute diag_util function script 
 *
 * Input : NONE 
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static int program_bcm57412_mac(void)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    unsigned int  size = 0;
    unsigned int port = 0;
    char cmd[128];

    /* Fugazi:
     * The managemenet port is implemented with i211
     * The other 4 10G ports and 8 1G ports.
     * In the kernel, these ports are eth0-3 and eth4-11, eth12 for i211 as shown below.
     * offset	description	count
     * 0	    gig0/0/0	8        
     * 8	    te0/1/0	    4            
     * 12	    mgmte	    1            
     * 13	    spare	    14           
     * 27	    VM	        32               
     * 59	    MST	        1                
     * 60	    overlay	    1            
     * 61	    appliance	2        
     * 63	    BDI	        1                
     * 64	    portchannel	64       
     * 		                128                 
     *
     * Chasis mac base and blocksize are values stored in MB cookie.
     *
     * The MAC address progarmming of this ports uses the Intel
     * eepudate utility and Boradcom tool. The utility is put under
     * /diag_utils/fugazi/scripts/function.sh in the kernel rootfs.
     */
    /* Fugazi size is 128 */
    size = get_mac_blk_size();
    printf("size is %d\n", size);
    if (size < 10) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }
    
    /* Fugazi port offset start from 0 */
    port = 0;
    get_mac_from_block(port, buf);
    if (!((*buf) || *(buf + 1) || *(buf + 2) || *(buf + 3) || *(buf + 4) ||
          (*buf + 5))) {
        printf("error getting mac base addr; did u run cookie util yet?\n");
        printf("Please run 'alter mb cpu cookie' and 'display cookie'"
	       " content at least once.\n");
        return(FAILED);
    }

    printf("eth%d: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
           port, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    sprintf(cmd, "%s;%s %.2x%.2x%.2x%.2x%.2x%.2x %d ", SOURCE_SCRIPT, 
            FUGAZI_UPDATE_BCM57412_MAC, buf[0], buf[1], buf[2], buf[3], buf[4], 
            buf[5], size);
    printf("%s\n", cmd);
    system(cmd);
    msleep(10);
    printf("\nMAC program successfully! Please power cycle the system\n");
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
    uchar mac_map[13] = { 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 12}; /* Fugazi port mapping */
    uchar buf[6] = {0, 0, 0, 0, 0, 0}; /* MAC address at least 6 */
    int32_t size = 0;
    unsigned int port = 0;
    char cmd[128];
    FILE *fp;
    char rd_mac[128];
    int rc = FAILED;
    char cookie_mac[128];

    size = get_mac_blk_size();
    if (size < 10 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return (FAILED);
    }
    get_mac_from_block(0, buf);
	if (!((*buf) || *(buf + 1) || *(buf + 2) || *(buf + 3) || *(buf + 4) ||
	      (*buf + 5))) {
	    printf("error getting mac base addr; did u run cookie util yet?\n");
	    printf("Please run 'alter mb cpu cookie' and 'display cookie'"
		       " content at least once.\n");
	    return (rc);
	}
    for (port = 0; port <= 12; port++) {
   	    sprintf(cookie_mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
    		    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5] + mac_map[port]);
        sprintf(cmd, "ifconfig eth%d | grep HWaddr | sed -n 's/.*'HWaddr'//p' "
                     "| sed s/[[:space:]]//g > %s", port, CHECK_MAC_FILE);
    	system(cmd);

        fp = fopen(CHECK_MAC_FILE, "r");
        if (fp == NULL) {
            printf("Unable to open file - %s\n", CHECK_MAC_FILE);
            return (rc);
        }

        while (!feof(fp)) {
            fgets(rd_mac, sizeof(rd_mac), fp);
            if (strcasestr(rd_mac, cookie_mac) != NULL) {
                rc = PASSED;
            } 
        }
    	sprintf(cmd, "rm -f %s", CHECK_MAC_FILE);
    	system(cmd);
        fclose(fp);

        if (rc == PASSED) {
            printf("eth%d MAC program successfully! expected - %s, got - %s\n", 
                    port, cookie_mac, rd_mac);
        } else {
            printf("Error - eth%d MAC didn't program correctly! expected - %s, "
                   "got - %s\n", port, cookie_mac, rd_mac);
            return (rc);
        }
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2021/04/26 08:15:25  iachang
 * CSCvy10910:Fugazi Diag supportted M.2 test card
 *
 * Revision 1.1.8.3  2020/09/02 17:46:15  pdoong
 * Add reset, unrest, init 1G PHY at Diag bootup since from Fugazi FPGA v2.0 and up, will put 1G PHY in reset by default at power on.
 *
 * Revision 1.1.8.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.15  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.14  2020/06/09 03:12:56  iachang
 * Changed program MAC item notes from debug to option
 *
 * Revision 1.1.6.13  2020/06/08 08:23:03  iachang
 * Move "Program Aikido FPGA DEV keys items" in the end.
 *
 * Revision 1.1.6.12  2020/06/08 06:54:39  iachang
 * Program Aikido FPGA DEV key utility
 * Program I211 and BCM57412 MAC address utility.
 *
 * Revision 1.1.6.11  2020/05/12 02:38:39  iachang
 * Fixed execute System Information Segmentation fault issue.
 *
 * Revision 1.1.6.10  2020/01/23 04:06:00  iachang
 * Add utility : CPLD reset(CPU self-reset)
 *
 * Revision 1.1.6.9  2019/05/14 02:00:40  pdoong
 * Added to sysyem info to display SyncE/bam82757 firmware version
 *
 * Revision 1.1.6.8  2019/05/02 02:53:00  iachang
 * Add Watchdog reset system utility
 *
 * Revision 1.1.6.7  2019/04/25 01:19:40  letsai
 * 1. Remove UART utility
 * 2. Modify SyncE PLL Interrupt test
 * 3. Modify Margin utility
 *
 * Revision 1.1.6.6  2019/04/10 02:11:06  iachang
 * Support Aikido firmware upgrade
 *
 * Revision 1.1.6.5  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.4  2019/04/01 21:02:04  iachang
 * Support MAC address program and verify utility.
 *
 * Revision 1.1.6.3  2019/03/18 19:44:51  iachang
 * Bring up PCIE read utility
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */

