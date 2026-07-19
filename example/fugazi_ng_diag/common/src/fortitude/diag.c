/* $Id: diag.c,v 1.21 2014/08/18 06:38:48 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Fortitude diagmon main menu and supporting wrappers.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2014 by Cisco Systems, Inc.
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
#include "fortitude.h"
#include "module_fru.h"


/* Function prototype */

extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem(), memdebug();
extern int  linux_memory_tester(int);
extern int  fortitude_led_test();
extern int  fpga_reg_test();
extern int  fpga_mem_test();
extern int  framer_reg_test();
extern int  fpga_intr_test();
extern void fpga_reset_framer();
extern void fpga_unreset_framer();
extern void fpga_reset_tdm_pll();
extern void fpga_unreset_tdm_pll();
extern int  fpga_reset_tdmsw();
extern int  fpga_unreset_tdmsw();
extern void fpga_get_rev();
extern int  fortitude_lpbk_tests();
extern int  fortitude_aloop_util();
extern int  fortitude_lpbk_util();
extern int  framer_ycable_util();
extern int  framer_display_regs();
extern int  framer_dump_rlps_ram();
extern int  framer_dump_indirect();
extern int  framer_rd_indirect();
extern int  framer_wr_indirect();
extern int  framer_rd_pw(); 
extern int  framer_peek_reg();
extern int  framer_poke_reg();
extern int  show_tdmsw_regs();
extern int  tdmsw_peek_reg();
extern int  tdmsw_poke_reg();
extern int  tdmsw_peek_conn_mem();
extern int  tdmsw_poke_conn_mem();
extern int  fpga_peek_reg();
extern int  fpga_poke_reg();
extern int  fpga_peek_dump_mem();
extern int  tdmsw_force_byte_test();
extern void npu_release_driver();
extern int  bp_sgmii_lpbk_test();
extern int  config_ngvm_tdm_lpbk(void);
extern int  config_ngvm_enet();

extern int  show_mb_regs();
extern int  mb_peek_reg();
extern int  mb_poke_reg();
extern int  spi_peek_reg();
extern int  spi_poke_reg();
extern int  peek_spi_flash();
extern int  fortitude_fpga_upgrade_secondary();
extern int  fortitude_fpga_upgrade_golden();
extern int  fpga_set_nor_flash_a23();
extern int  set_bp_reference_clock();
extern int bootflash_test(void);
extern int boot_flash_util(void);

static int linux_memory_test(int);
static int config_ngvm_db(void);
static void voltage_margin_hign();
static void voltage_margin_low();
static void voltage_margin_specific();
static void voltage_margin_normal();
static int for_uart_msg_exh_test(void);
static int set_ge1_line_lpbk(void);

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/* FRU PID and Location Strings */
uchar io_pid[] = "IO-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar pvdm_pid[] = "PVDM-PID";

uchar io_loc[] = "IO";
uchar dimm0_loc[] = "IO/DIMM0";
uchar pvdm0_loc[] = "IO/PVDM0";

fru_table_t platform_fru_table[] = {
    { io_pid,        io_loc },
    { dimm_pid,      dimm0_loc },
    { pvdm_pid,      pvdm0_loc },
};

/*
 * Memory debug utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory",    	    0,		0,
     (PFT)alt_mem,                          &one,	0, (type_t(*)())0, 0},
    {"compare memory block",   	    0,		0,
     (PFT)cmp_mem,		            &one,	0, (type_t(*)())0, 0},
    {"display memory",    	    0,		0,
     (PFT)dis_mem,		            &one,	0, (type_t(*)())0, 0},
    {"move memory block",    	    0,		0,
     (PFT)mov_mem,		            &one,	0, (type_t(*)())0, 0},
    {"fill memory",    	            0,		0,
    (PFT)fil_mem,		            &one,	0, (type_t(*)())0, 0},
    {"find memory",    	            0,		0,
     (PFT)memtest,		            &one,	0, (type_t(*)())0, 0},
    {"memory read or write loop",   0,		0,
     (PFT)memloop,		            &one,	0, (type_t(*)())0, 0},
    {"memory debug loop",    	    0,		0,
     (PFT)memdebug,		            &one,	0, (type_t(*)())0, 0},
    {"address loop",    	    0,		0,
     (PFT)addrloop,		            &one,	0, (type_t(*)())0, 0},
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
 * Framer menu utility
 */
static struct mitem framer_items[] = {
    {"reset framer",             0,0, (PFT)fpga_reset_framer,
                                 &one,  0, (type_t(*)())0, 0},
    {"unreset framer",           0,0, (PFT)fpga_unreset_framer,
                                 &one,  0, (type_t(*)())0, 0},
    {"peek Framer reg",          0,0, (PFT)framer_peek_reg,
                                 &one,  0, (type_t(*)())0, 0},
    {"poke Framer reg",          0,0, (PFT)framer_poke_reg,
                                 &one,  0, (type_t(*)())0, 0},
    {"set Y-cable mode",         0,0, (PFT)framer_ycable_util,
                                 &one,  0, (type_t(*)())0, 0},
    {"display framer regs",      0,0, (PFT)framer_display_regs,
                                 &one,  0, (type_t(*)())0, 0},
    {"dump framer rlps ram",     0,0, (PFT)framer_dump_rlps_ram,
                                 &one,  0, (type_t(*)())0, 0},
    {"dump framer indirect reg", 0,0, (PFT)framer_dump_indirect,
                                 &one,  0, (type_t(*)())0, 0},
    {"read framer indirect reg", 0,0, (PFT)framer_rd_indirect,
                                 &one,  0, (type_t(*)())0, 0},
    {"write framer indirect reg",0,0, (PFT)framer_wr_indirect,
                                 &one,  0, (type_t(*)())0, 0},
    {"read framer pulse waveform", 0,0, (PFT)framer_rd_pw,
                                 &one,  0, (type_t(*)())0, 0},
};

static struct menuinfo framer_menu = {
    "Framer utility Menu",
    0,
    0,
    0,
    sizeof(framer_items)/sizeof(struct mitem),
    framer_items,
};
static struct menuinfo *framer_menup = &framer_menu;

/*
 * TDM menu utility
 */
static struct mitem tdm_items[] = {
    {"reset TDM PLL",            0,0, (PFT)fpga_reset_tdm_pll,
                                 &one,  0, (type_t(*)())0, 0},
    {"unreset TDM PLL",          0,0, (PFT)fpga_unreset_tdm_pll,
                                 &one,  0, (type_t(*)())0, 0},
    {"reset TDMSW64",            0,0, (PFT)fpga_reset_tdmsw,
                                 &one,  0, (type_t(*)())0, 0},
    {"unreset TDMSW64",          0,0, (PFT)fpga_unreset_tdmsw,
                                 &one,  0, (type_t(*)())0, 0},
    {"show TDMSW64 registers",   0,0, (PFT)show_tdmsw_regs,
                                 &one,  0, (type_t(*)())0, 0},
    {"peek TDMSW64 register",    0,0, (PFT)tdmsw_peek_reg,
                                 &one,  0, (type_t(*)())0, 0},    
    {"poke TDMSW64 register",    0,0, (PFT)tdmsw_poke_reg,
                                 &one,  0, (type_t(*)())0, 0}, 
    {"peek TDMSW64 connection memory", 0,0, (PFT)tdmsw_peek_conn_mem,
                                 &one,  0, (type_t(*)())0, 0},   
    {"poke TDMSW64 connection memory", 0,0, (PFT)tdmsw_poke_conn_mem,
                                 &one,  0, (type_t(*)())0, 0},
    {"peek FPGA general register",     0,0, (PFT)fpga_peek_reg,
                                 &one,  0, (type_t(*)())0, 0},    
    {"poke FPGA general register",     0,0, (PFT)fpga_poke_reg,
                                 &one,  0, (type_t(*)())0, 0},
    {"peek DS0 dump memory",           0,0, (PFT)fpga_peek_dump_mem,
                                 &one,  0, (type_t(*)())0, 0},    
    {"show Multiboot registers", 0,0, (PFT)show_mb_regs,
                                 &one,  0, (PFT)is_hw_rev_new, 0},
    {"peek Multiboot register",  0,0, (PFT)mb_peek_reg,
                                 &one,  0, (PFT)is_hw_rev_new, 0},    
    {"poke Multiboot register",  0,0, (PFT)mb_poke_reg,
                                 &one,  0, (PFT)is_hw_rev_new, 0}, 
    {"peek SPI flash register",  0,0, (PFT)spi_peek_reg,
                                 &one,  0, (PFT)is_hw_rev_new, 0},    
    {"poke SPI flash register",  0,0, (PFT)spi_poke_reg,
                                 &one,  0, (PFT)is_hw_rev_new, 0}, 
    {"peek SPI flash",           0,0, (PFT)peek_spi_flash,
                                 &one,  0, (PFT)is_hw_rev_new, 0}, 
#ifdef DEBUG
    {"poke SPI flash",           0,0, (PFT)poke_spi_flash,
                                 &one,  0, (PFT)is_hw_rev_new, 0}, 
#endif
};

static struct menuinfo tdm_menu = {
    "FPGA utility Menu",
    0,
    0,
    0,
    sizeof(tdm_items)/sizeof(struct mitem),
    tdm_items,
};
static struct menuinfo *tdm_menup = &tdm_menu;

/*
 * Voltage Margining utility
 */
static struct mitem voltage_margin_items[] = {
    {"Margin all voltages to high", 0,0, (PFT)voltage_margin_hign,
                                    &one,  0, (type_t(*)())0, 0},
    {"Margin all voltages to low",  0,0, (PFT)voltage_margin_low,
                                    &one,  0, (type_t(*)())0, 0},
    {"Set all voltages to normal",  0,0, (PFT)voltage_margin_normal,
                                    &one,  0, (type_t(*)())0, 0},
    {"Margin a specific voltage",   0,0, (PFT)voltage_margin_specific,
                                    &one,  0, (type_t(*)())0, 0},
};

static struct menuinfo voltage_margin_menu = {
    "Voltage Margining Menu",
    0,
    0,
    0,
    sizeof(voltage_margin_items)/sizeof(struct mitem),
    voltage_margin_items,
};
static struct menuinfo *voltage_margin_menup = &voltage_margin_menu;

/* 
 * Basic utilities
 */
static struct mitem utilmenuitems[] = {
    {"Memory debug utilities",        0, 0,
     (PFT)menu, (type_t *)&mem_debug_menup, 0, (type_t(*)())0,0},
    {"Framer utilities",              0, 0,
     (PFT)menu, (type_t *)&framer_menup,    0, (type_t(*)())0,0},
    {"FPGA utilities",                0, 0,
     (PFT)menu, (type_t *)&tdm_menup,       0, (type_t(*)())0,0},
    {"Show FPGA Revision",            0, 0,
     (PFT)fpga_get_rev,          &one,	    0, (type_t(*)())0,0},
    {"Fortitude Loopback utility",    0, 0,
     (PFT)fortitude_lpbk_util,   &one,	    0, (type_t(*)())0,0},
    {"Framer Y-cable check",          0, 0,
     (PFT)fortitude_aloop_util,  &one,	    0, (type_t(*)())0,0},
    {"Backplane SGMII loopback test", 0, 0, 
     (PFT)bp_sgmii_lpbk_test,    &one,      0, (type_t(*)())0,0}, 
    {"NPU release driver",            0, 0,
     (PFT)npu_release_driver,    &one,	    0, (type_t(*)())0,0},
    {"Voltage Margining utilities",   0, 0,
     (PFT)menu, (type_t *)&voltage_margin_menup, 0, (type_t(*)())0,0},
    {"Golden FPGA image upgrade",     0, 0,
     (PFT)fortitude_fpga_upgrade_golden,    &one, 0, (PFT)is_hw_rev_new,0},
    {"Secondary FPGA image upgrade",  0, 0,
     (PFT)fortitude_fpga_upgrade_secondary, &one, 0, (PFT)is_hw_rev_new,0},
    {"Set NOR flash address 23",      0, 0,
     (PFT)fpga_set_nor_flash_a23, &one,	    0, (PFT)is_hw_rev_new,0},
    {"Set GE1 line loopback",         0, 0,
     (PFT)set_ge1_line_lpbk,      &one,	    0, (type_t(*)())0,0},    
    {"Set backplane reference clock", 0, 0,
     (PFT)set_bp_reference_clock, &one,	    0, (type_t(*)())0,0},    
    {"Boot Flash Test", 0, 0,
     (PFT)bootflash_test, &one,	            0, (type_t(*)())0,0},    
    {"Boot Flash utility", 0, 0,
     (PFT)boot_flash_util, &one,	    0, (type_t(*)())0,0},    
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

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"linux Memory Tester",
     (PFT)linux_memory_test,		FALSE,		MM_2,
     (type_t(*)())0, 0,		(PFT)linux_memory_test,         TRUE},
    {"FPGA Register Tests",
     (PFT)fpga_reg_test,   		0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"FPGA Memory Tests",
     (PFT)fpga_mem_test,   		0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"FPGA Interrupt Tests",
     (PFT)fpga_intr_test,   		0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"Framer Register Tests",
     (PFT)framer_reg_test,   		0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"TDMSW64 force byte test",
     (PFT)tdmsw_force_byte_test,      	0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"Fortitude lpbk Tests",
     (PFT)fortitude_lpbk_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)fortitude_lpbk_tests,	FALSE},
    {"LED Tests",
     (PFT)fortitude_led_test,   	0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"configure NPU for NGVM",
     (PFT)config_ngvm_db,     	        0,		MM_1,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"Dummy item to send string to UART", 
     (PFT)for_uart_msg_exh_test,        0,              0, 
     (type_t(*)())0, 0,         (type_t(*)())0,                 0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",			/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*********************************************************************
 * Function: diag_menu
 * Description: This is the main entry to diag menu interface.
 * Inputs: argc
 *         argv
 * Outputs: None
 *********************************************************************
 */
void
diag_menu(int argc, char *argv[]) 
{
    char arg;

    if(argc > 1) 
	arg = *argv[1];
    else 
	arg = 0;

    testname("Fortitude NGWIC");

    (NVRAM)->pollcon = 1;		/* poll the console */

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

static int 
linux_memory_test(int option)
{
    int ret;

    ret = linux_memory_tester(option);

    testname("Fortitude NGWIC");

    return (ret);
}

static int     
config_ngvm_db ()
{
    int ret;

    ret = config_ngvm_enet();
    ret |= config_ngvm_tdm_lpbk();
    return (ret);
}

static void
voltage_margin_hign ()
{
    system("echo 67 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out1");
    system("echo 67 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out2");
    system("echo 67 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out3");
    system("echo 67 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out4");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out1");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out2");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out3");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out4");
}

static void
voltage_margin_low ()
{
    system("echo e7 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out1");
    system("echo e7 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out2");
    system("echo e7 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out3");
    system("echo e7 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out4");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out1");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out2");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out3");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out4");
}

static void
voltage_margin_normal ()
{
    system("echo 0 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out1");
    system("echo 0 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out2");
    system("echo 0 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out3");
    system("echo 0 > /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out4");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out1");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out2");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out3");
    system("cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out4");
}

static void
voltage_margin_specific ()
{
    char cmd[30], cat_cmd[30];
    int voltage_device, margin;
    char path[] = "/sys/devices/platform/i2c-adapter/i2c-0/0-0030";

    printf("enter voltage device to margin\n");
    printf("enter '1' for 1 volt device\n");
    printf("enter '2' for 1.2 volt device\n");
    printf("enter '3' for 1.8 volt device\n");
    printf("enter '4' for 3.3 volt device\n");
    voltage_device = getdec_answer("enter 1, 2, 3 or 4", 1, 1, 4);

    margin = getdec_answer("enter 1 to set margin high, 2 to set margin low, "
			   "3 to set normal", 1, 1, 3);

    if (margin == 1)
	sprintf(cmd, "echo 67 > %s/reg_out%d", path, voltage_device);
    else if (margin == 2)
	sprintf(cmd, "echo e7 > %s/reg_out%d", path, voltage_device);
    else 
	sprintf(cmd, "echo 0 > %s/reg_out%d", path, voltage_device);
    printf("\ncmd = %s\n", cmd);

    system(cmd);

    sleep(3);

    sprintf(cat_cmd, "cat /sys/devices/platform/i2c-adapter/i2c-0/0-0030/reg_out%d", voltage_device);

    system(cat_cmd);    
}

/*
 * Function: for_uart_msg_exh_test()
 *
 * 'uname -a' to generate strings for the host to catch via
 * UART.
 * This is not test item for Fortitude side.
 *
 * Input : NONE
 * Output: PASSED
 */
static int
for_uart_msg_exh_test (void) 
{

    /* using 'uname -a' to dispay system info as a string.
     * x86 side will compare string for uart test 
     */ 
    system("uname -a");
    return PASSED;
}

static int 
set_ge1_line_lpbk (void) 
{
    volatile unsigned int *serdes_ctrl_reg;
    int enable;

    serdes_ctrl_reg = (int *)(get_npu_rif_base() + NPU_SERDES_CTRL_OFFSET);

    enable = gethex_answer("\nPlease select 0: disable \n"
                           "              1: enable",
			   0, 0, 1);

    if (enable == 1) 
	*serdes_ctrl_reg |= LINE_LPBK_EN;
    else
	*serdes_ctrl_reg &= (~LINE_LPBK_EN);

    return (PASSED);
}

/******** History ********
$Log: diag.c,v $
Revision 1.21  2014/08/18 06:38:48  iachang

Supported boot flash R/W utility.

Revision 1.20  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.19  2013/10/08 11:03:48  erwu2
enhanced err msg first check-in

Revision 1.18  2013/03/19 18:23:49  ywen
Add utility to set backplane reference clock for Nightster testing.

Revision 1.17  2013/03/04 17:15:33  ywen
Add check for voltage margining circuit per MFG request.

Revision 1.16  2012/12/18 17:39:27  ywen
mask off the utility to poke the SPI flash for safety purpose.

Revision 1.15  2012/11/29 17:41:08  ywen
Add support for Nightster platform.

Revision 1.14  2012/10/31 21:28:01  ywen
Add utility to upgrade golden FPGA image.

Revision 1.13  2012/10/04 21:38:29  ywen
Add code to support different HW revision.

Revision 1.12  2012/10/02 22:42:16  ywen
- Add support for host UART test.
- Add utility to set NOR flash address 23 for secure boot.

Revision 1.11  2012/09/25 22:36:43  ywen
- Add SPI flash peek/poke utility
- Update SPI registers access based on the latest FPGA design.

Revision 1.10  2012/09/10 06:02:27  srane
return failure for config_ngvm_tdm_lpbk().

Revision 1.9  2012/08/29 20:07:25  ywen
- Add utility to upgrade FPGA image for P1C and later builds.
- Add peek/poke utilities for FPGA multiboot registers.

Revision 1.8  2012/08/22 18:10:01  ywen
Add utility to display pulse waveform for debug.

Revision 1.7  2012/08/14 22:28:34  ywen
Add voltage marginning utilities.

Revision 1.6  2012/07/23 06:50:38  srane
Set NGVM TDM stream connection memory in loopback mode as part of
setup for running dc tests.

Revision 1.5  2012/05/09 17:43:25  ywen
Add support to do daughter card DSP FW download through Fortitude NGWIC.

Revision 1.4  2012/04/12 23:11:10  ywen
Add util to release NPU driver.
Move backplane SGMII loopback test to utility menu.

Revision 1.3  2012/04/02 17:43:45  ywen
Fix test name issue caused by linux_memory_tester().

Revision 1.2  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
