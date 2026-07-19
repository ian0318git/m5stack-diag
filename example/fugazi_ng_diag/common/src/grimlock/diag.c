/* $Id: diag.c,v 1.2 2020/03/13 12:06:53 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/diag.c,v $
 *------------------------------------------------------------------
 * diag.c
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
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
#include "grimlock.h"
#include "module_fru.h"


/* Function prototype */

extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem(), memdebug();
extern int  linux_memory_tester(int);
extern int  grimlock_led_test();
extern int  fpga_reg_test();
extern int  fpga_mem_test();
extern int  fpga_intr_test();
extern void fpga_reset_framer();
extern void fpga_unreset_framer();
extern void fpga_reset_tdm_pll();
extern void fpga_unreset_tdm_pll();
extern int  fpga_reset_tdmsw();
extern int  fpga_unreset_tdmsw();
extern void fpga_get_rev();
extern int  grimlock_lpbk_tests();
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
extern int  grimlock_fpga_upgrade_secondary();
extern int  grimlock_fpga_upgrade_golden();
extern int  fpga_set_nor_flash_a23();
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
static int padding_func(void);

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
    {"FPGA utilities",                0, 0,
     (PFT)menu, (type_t *)&tdm_menup,       0, (type_t(*)())0,0},
    {"Show FPGA Revision",            0, 0,
     (PFT)fpga_get_rev,          &one,	    0, (type_t(*)())0,0},
    {"Backplane SGMII loopback test", 0, 0, 
     (PFT)bp_sgmii_lpbk_test,    &one,      0, (type_t(*)())0,0}, 
    {"NPU release driver",            0, 0,
     (PFT)npu_release_driver,    &one,	    0, (type_t(*)())0,0},
    {"Voltage Margining utilities",   0, 0,
     (PFT)menu, (type_t *)&voltage_margin_menup, 0, (type_t(*)())0,0},
    {"Golden FPGA image upgrade",     0, 0,
     (PFT)grimlock_fpga_upgrade_golden,    &one, 0, (PFT)is_hw_rev_new,0},
    {"Secondary FPGA image upgrade",  0, 0,
     (PFT)grimlock_fpga_upgrade_secondary, &one, 0, (PFT)is_hw_rev_new,0},
    {"Set NOR flash address 23",      0, 0,
     (PFT)fpga_set_nor_flash_a23, &one,	    0, (PFT)is_hw_rev_new,0},
    {"Set GE1 line loopback",         0, 0,
     (PFT)set_ge1_line_lpbk,      &one,	    0, (type_t(*)())0,0},    
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
    {"Padding",
     (PFT)0,   		                 0,		MM_2,
     (type_t(*)())padding_func, 0,		(type_t(*)())0,                 0},
    {"TDMSW64 force byte test",
     (PFT)tdmsw_force_byte_test,      	0,		MM_2,
     (type_t(*)())0, 0,		(type_t(*)())0,                 0},
    {"Grimlock lpbk Tests",
     (PFT)grimlock_lpbk_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)grimlock_lpbk_tests,	FALSE},
    {"LED Tests",
     (PFT)grimlock_led_test,   	0,		MM_2,
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
void diag_menu(int argc, char *argv[]) 
{
    char arg;

    if(argc > 1) {
	arg = *argv[1];
    }
    else{ 
	arg = 0;
    }

    testname("Grimlock NIM");

    (NVRAM)->pollcon = 1;		/* poll the console */

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

/*********************************************************************
 * Function: linux_memory_test
 * Description: Linux Memory Test
 * Inputs: option
 * Outputs: ret
 *********************************************************************
 */
static int linux_memory_test(int option)
{
    int ret;

    ret = linux_memory_tester(option);

    testname("Grimlock NIM");

    return (ret);
}

/*********************************************************************
 * Function: config_ngvm_db
 * Description: Config ngvm
 * Inputs: None
 * Outputs: ret
 *********************************************************************
 */
static int config_ngvm_db ()
{
    int ret;

    ret = config_ngvm_enet();
    ret |= config_ngvm_tdm_lpbk();
    return (ret);
}

/*********************************************************************
 * Function: voltage_margin_hign
 * Description: Set margin high
 * Inputs: None
 * Outputs: None
 *********************************************************************
 */
static void voltage_margin_hign ()
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

/*********************************************************************
 * Function: voltage_margin_low
 * Description: Set margin low
 * Inputs: None
 * Outputs: None
 *********************************************************************
 */
static void voltage_margin_low ()
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

/*********************************************************************
 * Function: voltage_margin_normal
 * Description: Set margin normal
 * Inputs: None
 * Outputs: None
 *********************************************************************
 */
static void voltage_margin_normal ()
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

/*********************************************************************
 * Function: voltage_margin_specific
 * Description: Set margin by user
 * Inputs: None
 * Outputs: None
 *********************************************************************
 */
static void voltage_margin_specific ()
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
 * This is not test item for Grimlock side.
 *
 * Input : NONE
 * Output: PASSED
 */
static int for_uart_msg_exh_test (void) 
{

    /* using 'uname -a' to dispay system info as a string.
     * x86 side will compare string for uart test 
     */ 
    system("uname -a");
    return PASSED;
}

static int set_ge1_line_lpbk (void) 
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

static int padding_func (void)
{
    return (FALSE);
}

/******** History ********
$Log: diag.c,v $
Revision 1.2  2020/03/13 12:06:53  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.3  2020/01/15 09:28:49  wilbhuan
Removed T1/E1 framer function.

Revision 1.1.4.2  2020/01/15 03:30:10  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/
