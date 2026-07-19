/* $Id: diag.c,v 1.2 2019/06/14 05:24:48 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Informers diagmon main menu and supporting wrappers.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2009-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "pcmap.h"
#include "monitor.h"
#include "mon_plat_defs.h"
#include "nvmonvars.h"
#include "menu.h"
#include "signals.h"
#include "uio_utils.h"
#include "pci.h"
#include "queryflags.h"
#include "error.h"
#include "cross_platform.h"
#include "proto.h"
#include "strings.h"
#include "platform_fpga.h"
#include "cookie_4.h"
#include "linux_usb_test.h"
#include "platform_poe_psu.h"
#include "platform_eeprom_access.h"
#include "platform_pci.h"
#include "plat_defs.h"   
#include "platform_poe.h"
#include "mb_tests.h"

extern struct menuinfo *led_menup;
extern struct menuinfo *eth_menup;
extern struct menuinfo *intr_menup;
extern struct menuinfo *fan_utilmenup;
/* Function prototype */
extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem();
extern int  build_i2c_menu(void);
extern int  katar_alter_mb_cookie(void);
extern int  katar_read_sfp_cookie(int);
extern int katar_smartchip(int submenu_flag);
extern void build_fan_menu(void);
extern int display_fpga_regs(int);
extern int rtc_utility_main(int);
/* MDIO utilities */
extern int mb_tests(int flag);

/* FPGA update related */
extern int katar_program_fpga_spi_prom (int header);
extern int katar_program_fpga_header (int dummy);
extern int katar_display_prom_sector (int dummy);

extern int platform_intr_test();
extern int platform_intr_mask (int dummy);
extern int program_reggio_spi_prom(int);
extern int program_aikido_dev_key(void);

static int display_brd_info(int);
static int reset_dev(int);
static int katar_usb_manual_ctrl(int);
static int common_pci_read(void);
static int common_pci_write(void);
int reset_sys_by_watchdog(void);
int dump_AQC_SFP_eeprom_data(int port_num);

/* display system info */
extern int katar_show_cpuinfo (void);
extern int show_meminfo (void);
extern uint32 show_temperature_all(void);
extern void print_eth_fw_ver(void);
extern void show_fan_sts (void);

extern char *strcasestr(char* , char*);
extern int katar_dash_rd_wr_test(int);
static int diag_sys_info_util(int);
extern uint32_t diag_pci_get_device_bus(ushort vendor, ushort device, uint32_t *bus, uint32_t device_num);
extern int katar_is_sfp_sku(void);

extern int platform_intr_manual_tests(int test_item);

static struct mitem reggio_fpga_items[] = {

    {"Program logic FPGA SPI PROM image without header",  0, 0,
     (type_t(*)())katar_program_fpga_spi_prom,  &zero, 0, (type_t(*)())0, 0},
    {"Program logic FPGA SPI PROM image with header",  0, 0,
     (type_t(*)())katar_program_fpga_spi_prom,   &one, 0, (type_t(*)())0, 0},
    {"Erase/Program logic FPGA Image Upgrade Header",  0, 0,
     (type_t(*)())katar_program_fpga_header,   &one, 0, (type_t(*)())0, 0},
    {"Display a logic FPGA sector", 0, 0, 
     (type_t(*)())katar_display_prom_sector,   &zero, 0, (type_t(*)())0, 0},
    {"Program Aikido FPGA SPI PROM image",  0, 0,
     (type_t(*)())program_reggio_spi_prom,  &zero, 0, (type_t(*)())0, 0},
    {"Program Aikido FPGA DEV keys",  0, 0,
     (type_t(*)())program_aikido_dev_key,  &zero, 0, (type_t(*)())0, 0},
    {"Display FPGA registers",  0, 0,
     (type_t(*)())display_fpga_regs,   &one, 0, (type_t(*)())0, 0},
    {"Show Board type/FPGA Version",           0, 0,
     (type_t(*)())display_brd_info, &one, 0, (type_t(*)())0, 0},
    {"Reset devices",           0, 0,
     (type_t(*)())reset_dev, &one, 0, (type_t(*)())0, 0},
    {"Unreset devices",           0, 0,
     (type_t(*)())reset_dev, &zero, 0, (type_t(*)())0, 0},
    {"interrupt debug tool", 0, 0,
     (type_t(*)())platform_intr_test,   &one, 0, (type_t(*)())0, 0},
    {"interrupt mask setting", 0, 0,
     (type_t(*)())platform_intr_mask,   &one, 0, (type_t(*)())0, 0},
    {"Change console switch setting",           0, 0,
     (type_t(*)())katar_usb_manual_ctrl, &one, 0, (type_t(*)())0, 0},
    {"FPGA read", 0, 0, 
     (type_t(*)())read_fpga_reg,   &one, 0, (type_t(*)())0, 0},
    {"FPGA alter", 0, 0, 
     (type_t(*)())write_fpga_reg,   &one, 0, (type_t(*)())0, 0},
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
 * Memory debug utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory",                0,          0,
     (PFT)alt_mem,   &one,      0, (type_t(*)())0, 0},
    {"compare memory block",        0,          0,
     (PFT)cmp_mem,                          &one,       0, (type_t(*)())0, 0},
    {"display memory",              0,          0,
     (PFT)dis_mem,                          &one,       0, (type_t(*)())0, 0},
    {"move memory block",           0,          0,
     (PFT)mov_mem,                          &one,       0, (type_t(*)())0, 0},
    {"fill memory",                 0,          0,
     (PFT)fil_mem,                          &one,       0, (type_t(*)())0, 0},
    {"find memory",                 0,          0,
     (PFT)memtest,                          &one,       0, (type_t(*)())0, 0},
    {"memory read or write loop",   0,          0,
     (PFT)memloop,                          &one,       0, (type_t(*)())0, 0},
    {"memory debug loop",           0,          0,
     (PFT)memdebug,                         &one,       0, (type_t(*)())0, 0},
    {"address loop",                0,          0,
     (PFT)addrloop,                         &one,       0, (type_t(*)())0, 0},
};

static struct menuinfo mem_debug_menu = {
    "Memory debug utility Menu",
    0,
    0,
    0,
    sizeof(mem_debug_items)/sizeof(struct mitem),
    mem_debug_items,
};
static struct menuinfo *mem_debug_menup = &mem_debug_menu;

/*
 * Cookie menu utility
 */
static struct mitem cookie_items[] = {
    {"alter MB CPU cookie on Aikido FPGA",0,    0,
     (type_t(*)())katar_alter_mb_cookie,  &one, 0, (type_t(*)())0, 0},
};

static struct menuinfo cookie_menu = {
    "Cookie utility Menu",
    0,
    0,
    0,
    sizeof(cookie_items)/sizeof(struct mitem),
    cookie_items,
};
static struct menuinfo *cookie_menup = &cookie_menu;

/*
 * pcie debug utility
 */
static struct mitem pcie_debug_items[] = {
    {"PCIE read utility", 0, 0,
     (PFT)common_pci_read, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"PCIE write utility", 0, 0,
     (PFT)common_pci_write, (type_t *)&one, 0, (type_t(*)())0, 0},
};

static struct menuinfo pcie_debug_menu = {
    "PCIE debug utility Menu",
    0,
    0,
    0,
    sizeof(pcie_debug_items)/sizeof(struct mitem),
    pcie_debug_items,
};
static struct menuinfo *pcie_debug_menup = &pcie_debug_menu;

/*
 * SFP utility
 */
static struct mitem sfp_items[] = {
    {"SFP port0 SPD dump", 0, 0,
     (PFT)dump_AQC_SFP_eeprom_data, (type_t *)&zero, 0, (type_t(*)())0, 0},
	{"SFP port1 SPD dump", 0, 0,
     (PFT)dump_AQC_SFP_eeprom_data, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"SFP port0 present interrupt test", 0, 0,
     (PFT)platform_intr_manual_tests, (type_t *)&two, 0, (type_t(*)())0, 0},
    {"SFP port1 present interrupt test", 0, 0,
     (PFT)platform_intr_manual_tests, (type_t *)&one, 0, (type_t(*)())0, 0},
};

static struct menuinfo sfp_menu = {
    "SFP utility Menu",
    0,
    0,
    0,
    sizeof(sfp_items)/sizeof(struct mitem),
    sfp_items,
};
static struct menuinfo *sfp_menup = &sfp_menu;

/* 
 * Basic utilities
 */
static struct mitem utilmenuitems[] = {

    {"System Information", 0, 0,
     (PFT)diag_sys_info_util, (type_t *) &one, 0,
     (type_t(*)())0, 0}, 
    {"PCIE debug utilities",        0, 0,
     (PFT)menu, (type_t *)&pcie_debug_menup, 0,
     (type_t(*)())0,0},
    {"Memory debug utilities",        0, 0,
     (PFT)menu, (type_t *)&mem_debug_menup, 0,
     (type_t(*)())0,0},
    {"FPGA utilities",          0, 0,
     (PFT)menu, (type_t *)&reggio_fpga_menup, 0,
     (type_t(*)())0,0},
    {"I2C utilities",
     0,                 0,
     (PFT)build_i2c_menu,       (type_t *)&one,         0,
     (type_t(*)())0,            0},
    {"Cookie utility",
     0,                 0,
     (PFT)menu,         (type_t *)&cookie_menup,   0,
     (type_t(*)())0,            0},
    {"USB utility",
     0,                 0,
     (PFT)usb_utils_v2, (type_t *)&one,           0,
     (type_t(*)())0,            0},
    {"RTC utilities",
     0,                 0,
     (PFT)rtc_utility_main,     (type_t *)&zero,                0,
     (type_t(*)())0,            0},
    {"FAN utilities",
     0,                 0,
     (PFT)build_fan_menu, (type_t *)&zero,                0,
     (type_t(*)())0,            0},
    {"Mother LED utility",
     0,                      0,
     (PFT)menu,              (type_t *)&led_menup,      0,
     (type_t(*)())0,         0},
    {"Ethernet utility",
     0,                      0,
     (PFT)menu,              (type_t *)&eth_menup,      0,
     (type_t(*)())0,         0},
	{"Reset button utility",
     0,                      0,
     (PFT)platform_intr_manual_tests,              (type_t *)&zero,      0,
     (type_t(*)())0,         0},
    {"SFP utility",
     0,                      0,
     (PFT)menu,              (type_t *)&sfp_menup,      0,
     (type_t(*)())0,         0},
    {"Reset system by watchdog", 0, 0, 
     (PFT)reset_sys_by_watchdog, (type_t *)&one, 0, (type_t(*)())0, 0},
};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems)/sizeof(struct mitem),
    utilmenuitems,
};
struct menuinfo *utilmenup = &utilmenu;

/*
 * Main menu test flag defines
 */

#define MM_1    (MF_CONTINUOUS)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)

/*=========================================
 * Main menu items
 *=========================================
 */
submenu_xtable_t main_menu_table[] = {
    {"ACT-2 utilities and programming",
     (PFT)katar_smartchip,      FALSE,          MM_1,
     (type_t(*)())0, 0,         (PFT)katar_smartchip, TRUE},
    {"motherboard tests",
     (PFT)mb_tests,             TRUE,           MM_2,
     (type_t(*)())0, 0,         (PFT)mb_tests,  FALSE},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",                  /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;


/*************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *************************************************************
 */

void
diag_menu(int argc, char *argv[]) 
{
    char arg;

    if (argc > 1) {
        arg = *argv[1];
    } else {
        arg = 0;
    }
    (NVRAM)->pollcon = 1;               /* poll the console */
    /*envflag = INDIAG;*/                       /* set the environment flag */
    //    dobro_debug_flag = 0;
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    /* Add the mother board skipped plugin list after the menu title
     */
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
    unsigned int fpga_ver, cpld_ver, fpga_brd, cpld_brd;

    katar_show_cpuinfo();

    show_meminfo();

    show_temperature_all();

#ifdef ENABLE_POE_MODULE
	poe_status_util(TRUE);
#endif

    printf("Ethernet port firmware-version\n");
    print_eth_fw_ver();

	printf("\n");
	show_fan_sts();
	printf("\n");
    katar_get_platform_ver(1, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    printf("\nDiagnostic Kernel for Katar\n");
    system("uname|grep version");

    printf("%s", banner_string);

    return (PASSED);
} 

/**********************************************************************
 *
 * Function: cli_main_menu_table_size()
 *
 * This routine is used for cli command to return the menu size
 *
 * Input : none
 *
 * Output: MAIN_MENU_TABLE_SIZE
 *
 **********************************************************************
 */
int
cli_main_menu_table_size(void)
{
    return (MAIN_MENU_TABLE_SIZE);
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
static int
display_brd_info(int val)
{
    unsigned int fpga_ver, cpld_ver, fpga_brd, cpld_brd;

        katar_get_platform_ver(1, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);
    return (PASSED);
}

static int katar_usb_manual_ctrl (int dummy)
{
	int usb_cur;

	usb_cur = katar_get_usb_com_manual();

    printf("Current console switch setting is %s \n", usb_cur?"manual":"auto");
    if (getc_answer("Change setting?", "yn", 'n') == 'y') {
		katar_set_usb_com_manual(usb_cur^1);
		usb_cur = katar_get_usb_com_manual();
		printf("console switch is set to %s \n", usb_cur?"manual":"auto");
    }

    return (PASSED);
}

static int
reset_dev(int bReset)
{
    unsigned int select;

        if(bReset)
                printf("Select reset device : \n");
        else
                printf("Select unreset device : \n");
    printf("0)  quit\n");
    printf("1)  USB 3.0\n");
    printf("2)  USB HUB\n");
    printf("3)  eMMC\n");
    printf("4)  GE B PHY\n");
    printf("5)  GE A PHY\n");
    printf("6)  2.5G PHY\n");
	printf("7)  POE\n");
	printf("8)  10G mGIG PHY A\n");
    printf("9)  10G mGIG PHY B\n");

        select = getdec_answer("Enter bit value: ", 0, 0, 0xF);

        switch(select)
        {
                case 1:
                        katar_reset_device(RSTDEV_USB30,bReset);
                        break;
                case 2:
                        katar_reset_device(RSTDEV_USB_HUB,bReset);
                        break;
                case 3:
                        katar_reset_device(RSTDEV_EMMC,bReset);
                        break;
                case 4:
                        katar_reset_device(RSTDEV_GE_PHY_0,bReset);
                        break;
                case 5:
                        katar_reset_device(RSTDEV_GE_PHY_1,bReset);
                        break;
				case 6:
                        katar_reset_device(RSTDEV_25G_PHY,bReset);
						break;
				case 7:
                        katar_reset_device(RSTDEV_POE,bReset);
                        break;
				case 8:
                        katar_reset_device(RSTDEV_10G_PHY_A,bReset);
                        break;
                case 9:
                        katar_reset_device(RSTDEV_10G_PHY_B,bReset);
                        break;	
                default:
                        break;
        }
    return PASSED;
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
    int fd_wd;

    /* Reset system by watchdog */
    fd_wd = open("/dev/watchdog", O_RDWR);
    if (fd_wd == -1) {
        cterr('f',0,"open /dev/watchdog failed. \n");
        return (FAILED);
    } else {
        printf("System will be reboot by watchdog after 1 mins.\n");
    }
    return (PASSED);
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

   int bus, dev, func, reg, val; 
    do {
        bus = gethex_answer("BUS", 0, 0, 0xFF); 
        dev = gethex_answer("DEV", 0, 0, 0xFF); 
        func = gethex_answer("FUN", 0, 0, 0xFF); 
        reg = gethex_answer("REG", 0, 0, 0xFFF); 

        val = pci_config_read(bus, dev, func, reg);

                printf("%x:%x:%x reg(%x) result === 0x%x \n",bus, dev, func, reg, val);

    } while(getc_answer("Continue?", "yn", 'y') == 'y');

   return 0;

}

int common_pci_write (void) {

   int bus, dev, func, reg, val, read;
    do {
        bus = gethex_answer("BUS", 0, 0, 0xFF);
        dev = gethex_answer("DEV", 0, 0, 0xFF);
        func = gethex_answer("FUN", 0, 0, 0xFF);
        reg = gethex_answer("REG", 0, 0, 0xFFF);
                val = gethex_answer("VAL", 0, 0, 0xFFFFFFFF);

                pci_config_write(bus, dev, func, reg, val);

        read = pci_config_read(bus, dev, func, reg);

        printf("%x:%x:%x reg(%x) result === 0x%x \n",bus, dev, func, reg, read);

    } while(getc_answer("Continue?", "yn", 'y') == 'y');

   return 0;

}

int get_AQC_pci_num(int unit_num)
{
	uint32_t bus[2];
	uint32_t dev_num;
	int pci_num = -1;

	switch(unit_num)
	{
		case 0:
			unit_num = 1;
			break;
		case 1:
			unit_num = 0;
            break;
		default:
			printf("Wrong unit number\n");
			return pci_num;
			break;
	}

	switch(katar_get_plat_sku())
	{
		case KATAR_RJ45_SKU:
			dev_num = diag_pci_get_device_bus(0x1d6a,0x07b1,bus,2);
			break;
		case KATAR_SFP_SKU:
			dev_num = diag_pci_get_device_bus(0x1d6a,0x0001,bus,2);
			break;
		case KATAR_SFP1_SKU:
            dev_num = diag_pci_get_device_bus(0x1d6a,0xD100,bus,2);
            break;
	}
	if(dev_num !=2)
    {
        printf("only found %d AQC dev \n",dev_num);
    }else
		pci_num = bus[unit_num];

	return pci_num;	
}

int is_SFP_plus_module(int port_num)
{
	char cmd[1024];
    char buf[1024] = "NULL";
    int pci_num = 0;
    FILE *fp;
	int bit_rate = 0;

	if(!katar_is_sfp_sku())
        return FALSE;

    if( access( "/diag_utils/aqdiag/atltool/readstat", F_OK ) == -1 )
        return FALSE;

	pci_num = get_AQC_pci_num(port_num);
    if(pci_num == -1)
        return FALSE;

    sprintf(cmd,"/diag_utils/aqdiag/atltool/readstat -d 0000-%02x:00.0", pci_num);

    fp = popen(cmd,"r");
    while ((fgets(buf, sizeof(buf), fp))!=NULL)
	{
		char *ptr;

		ptr = strstr(buf, "bit rate = ");
		if(ptr)
		{
			sscanf(ptr+11,"%d", &bit_rate);
		}
	}
	pclose(fp);
	if(bit_rate>=10000)
		return TRUE;
	else
		return FALSE;
}

int dump_AQC_SFP_eeprom_data(int port_num)
{
	char cmd[1024];
	char buf[1024] = "NULL";
	int pci_num = 0;
	int bPrintCount = 0xFF;
	FILE *fp;
	int rc = FAILED;

	if(!katar_is_sfp_sku())
	{
		printf("------ Not SFP sku ------\n");
		return rc;
	}

	if( access( "/diag_utils/aqdiag/atltool/readstat", F_OK ) == -1 )
	{
		printf("readstat not exist ,please use correct kernel image\n");
		return rc;
	}

	pci_num = get_AQC_pci_num(port_num);
	if(pci_num == -1)
		return rc;

	sprintf(cmd,"/diag_utils/aqdiag/atltool/readstat -d 0000-%02x:00.0", pci_num);

	fp = popen(cmd,"r");
	fflush(stdout);
	rc = PASSED;
	while ((fgets(buf, sizeof(buf), fp))!=NULL) 
	{
		if(strstr(buf, "PHY in SMBUS"))
			bPrintCount = 0;
		if(bPrintCount<5)
		{
			printf("%s",buf);
			fflush(stdout);
			bPrintCount ++;
		}
		if(strstr(buf, "Failed"))
		{
			rc = FAILED;
			bPrintCount = 0xFF;
		}
	}
	pclose(fp);

	if (rc != PASSED)
        printf("Dump port %d eeprom failed\n",port_num);

	return rc;
}

/*
 *------------------------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.2  2019/06/14 05:24:48  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.22  2019/06/10 02:26:44  mikech2
 * Remove skip test function base on PRRQ#4685780 Comment#4
 *
 * Revision 1.1.2.21  2019/04/30 06:06:58  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.20  2019/03/26 06:28:11  mikech2
 * Add SFP/SFP+ module check in AQC100 cross-port test
 *
 * Revision 1.1.2.19  2019/03/08 07:44:37  mikech2
 * Add SFP sku check for SFP utility
 *
 * Revision 1.1.2.18  2019/03/07 02:51:08  mikech2
 * Move reset button/SFP present test to utility
 *
 * Revision 1.1.2.17  2019/03/06 01:56:23  mikech2
 * Add dump SFP eeprom in SFP intr test
 *
 * Revision 1.1.2.16  2019/03/05 03:33:35  mikech2
 * Use Aquantia tool dump SFP eeprom
 *
 * Revision 1.1.2.15  2019/02/12 08:06:28  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.14  2019/02/12 01:32:24  mikech2
 * Add program Aikido FPGA DEV keys function
 *
 * Revision 1.1.2.13  2019/01/29 08:02:05  mikech2
 * remove POE test for katar P2 build
 *
 * Revision 1.1.2.12  2019/01/17 07:14:19  mikech2
 * Modify according to Kwok's review comments
 *
 * Revision 1.1.2.11  2018/12/27 00:42:24  peteteng
 * Support Aikido thru UserLogic FPGA I2C
 *
 * Revision 1.1.2.10  2018/12/20 09:10:56  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.9  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.8  2018/11/22 06:55:08  mikech2
 * Add security FPGA version info
 *
 * Revision 1.1.2.7  2018/11/13 02:53:07  mikech2
 * Add PoE operating mode info
 *
 * Revision 1.1.2.6  2018/11/08 06:00:15  mikech2
 * Add fan low and interrupt test in mb test and remove intr utility
 *
 * Revision 1.1.2.5  2018/11/05 07:37:59  mikech2
 * Add interrupt utility
 *
 * Revision 1.1.2.4  2018/11/01 07:24:53  mikech2
 * Add fan spped in system info
 *
 * Revision 1.1.2.3  2018/10/30 06:32:01  peteteng
 * Change cookie util order; remove i2c-1 inspection; add ACT2 programming case Qq
 *
 * Revision 1.1.2.2  2018/10/26 02:39:34  mikech2
 * Fix typo
 *
 * Revision 1.1.2.1  2018/10/22 08:02:20  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.22  2018/10/22 03:04:32  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.21  2018/10/08 08:05:26  peteteng
 * Update PCAMAP-v3 default array
 *
 * Revision 1.1.2.20  2018/10/08 03:36:17  mikech2
 * Modify pcie scan for different AQC100 FW
 *
 * Revision 1.1.2.19  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.18  2018/09/21 08:52:12  mikech2
 * Add cross-port & internal lpbk test util
 *
 * Revision 1.1.2.17  2018/09/20 06:44:59  peteteng
 * Add load/read 256-byte cookie on EEPROM in cookie utility
 *
 * Revision 1.1.2.15  2018/09/12 08:32:48  mikech2
 * Fix userlogic FPGA update & system info version issue
 *
 * Revision 1.1.2.14  2018/09/07 03:14:20  peteteng
 * Add system info utility
 *
 * Revision 1.1.2.13  2018/09/07 02:16:52  mikech2
 * Fix FPGA util issue
 *
 * Revision 1.1.2.12  2018/07/24 09:54:12  peteteng
 * Add SFP cookie - read
 *
 * Revision 1.1.2.11  2018/07/19 06:32:03  mikech2
 * modify logic FPGA upgrade flow
 *
 * Revision 1.1.2.10  2018/07/10 07:50:30  mikech2
 * Add security FPGA FW update util
 *
 * Revision 1.1.2.9  2018/07/02 09:12:50  mikech2
 * Add Ethernet utility Menu
 *
 * Revision 1.1.2.8  2018/06/29 07:17:31  mikech2
 * Remove compile warning and unused files
 *
 * Revision 1.1.2.7  2018/06/29 03:40:01  peteteng
 * Add ACT2 utility Menu
 *
 * Revision 1.1.2.6  2018/06/28 03:32:56  mikech2
 * Add interrupt mask control menu
 *
 * Revision 1.1.2.5  2018/06/27 01:26:29  mikech2
 * Add reset/unreset device menu
 *
 * Revision 1.1.2.4  2018/06/26 06:30:09  peteteng
 * Add Aikido Cookie menu
 *
 * Revision 1.1.2.3  2018/06/25 08:24:53  mikech2
 * Add interupt test menu
 *
 * Revision 1.1.2.2  2018/06/21 08:24:09  mikech2
 * remove unused menu, add scratchpad reg test
 *
 * Revision 1.1.2.1  2018/06/07 01:19:22  peteteng
 * add project katar - based on neptune
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

