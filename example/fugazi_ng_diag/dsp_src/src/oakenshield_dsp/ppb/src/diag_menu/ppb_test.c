/* $Id: ppb_test.c,v 1.6 2021/04/15 00:53:09 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/diag_menu/ppb_test.c,v $
 *------------------------------------------------------------------
 * ppb_test.c
 *      Oakenshield menu 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "menu.h"
#include "diag_ppb.h"
#include "common.h"
#include "error.h"
#include "debug_console.h"
#include "uart.h"
#include "diag_fpga.h"
#include "fxs_test.h"

static int eth_int_lpbk(int);
static int eth_ext_lpbk(int);
static int dss_core0_sanity(int);
static int dss_core1_sanity(int);
static int dss_core2_sanity(int);
static int dss_core3_sanity(int);

extern uint32_t test_mem(void);
extern int ecc_mem_test(void);
extern int silab_fxs_lpbk_test(void);
extern int silab_fxo_lpbk_test(void);
extern int si32261_codec_utilities(void);
extern int si3050_codec_utilities(void);
extern uint32_t ppb_con(void);
extern uint32_t ethernet_test(uint32_t, int, int);
extern uint32_t eth_reg_disp(void);
extern void reload(void);
extern int tdm_lpbk(void);
extern int dss_core_sanity(int);
extern int tdm_int_lpbk(int);
extern int tdm_ext_lpbk(int);
extern int arm11_cpu1_boot_test(void);
extern int get_info(int host_cmd);
extern void fpga_get_rev(void);
extern int fpga_spi_read_utility (void);
extern int fpga_spi_write_utility (void);
extern int load_patch_debug (void);
extern void setup_sep_test_dbx_flag(void);
extern void setup_hw_brd_type_flag(void);

void open_close_debug_flag(void);
static struct menuinfo *maindiagp;

extern int oak_diag_flag;
static int zero  = 0;
static int one  = 1;

/* 
 * Basic utilities
 */
static struct mitem utilmenuitems[] = {
    {"Show FPGA Revision",            0, 0,
     (PFT)fpga_get_rev,          (type_t *)&zero,      0, 0, 0},
    {"reset TDM PLL",            0,0, (PFT)fpga_reset_tdm_pll,
                                 &one,  0, 0, 0},
    {"unreset TDM PLL",          0,0, (PFT)fpga_unreset_tdm_pll,
                                 &one,  0, 0, 0},
    {"reset TDMSW16",            0,0, (PFT)fpga_reset_tdmsw,
                                 &one,  0, 0, 0},
    {"unreset TDMSW16",          0,0, (PFT)fpga_unreset_tdmsw,
                                 &one,  0, 0, 0},
    {"show TDMSW16 registers",   0,0, (PFT)show_tdmsw_regs,
                                 &one,  0, 0, 0},
    {"peek TDMSW16 register",    0,0, (PFT)tdmsw_peek_reg,
                                 &one,  0, 0, 0},    
    {"poke TDMSW16 register",    0,0, (PFT)tdmsw_poke_reg,
                                 &one,  0, 0, 0}, 
    {"peek TDMSW16 connection memory", 0,0, (PFT)tdmsw_peek_conn_mem,
                                 &one,  0, 0, 0},   
    {"poke TDMSW16 connection memory", 0,0, (PFT)tdmsw_poke_conn_mem,
                                 &one,  0, 0, 0},
    {"show FPGA general registers",    0,0, (PFT)show_gen_regs,
                                 &one,  0, 0, 0},
    {"peek FPGA general register",     0,0, (PFT)fpga_peek_reg,
                                 &one,  0, 0, 0},    
    {"poke FPGA general register",     0,0, (PFT)fpga_poke_reg,
                                 &one,  0, 0, 0},
    {"peek DS0 dump memory",           0,0, (PFT)fpga_peek_dump_mem,
                                 &one,  0, 0, 0},    
    {"show Multiboot registers", 0,0, (PFT)show_mb_regs,
                                 &one,  0, 0, 0},
    {"peek Multiboot register",  0,0, (PFT)mb_peek_reg,
                                 &one,  0, 0, 0},    
    {"poke Multiboot register",  0,0, (PFT)mb_poke_reg,
                                 &one,  0, 0, 0}, 
    {"show SPI flash registers", 0,0, (PFT)show_spi_regs,
                                 &one,  0, 0, 0},
    {"peek SPI flash register",  0,0, (PFT)spi_peek_reg,
                                 &one,  0, 0, 0},    
    {"poke SPI flash register",  0,0, (PFT)spi_poke_reg,
                                 &one,  0, 0, 0},
    {"peek SPI flash",           0,0, (PFT)peek_spi_flash,
                                 &one,  0, 0, 0}, 
    {"poke SPI flash",           0,0, (PFT)poke_spi_flash,
                                 &one,  0, 0, 0}, 
    {"Read FPGA direct Register",       0,0, (PFT)fpga_spi_read_utility,
                                 &one,  0, 0, 0}, 
    {"Write FPGA direct Register",       0,0, (PFT)fpga_spi_write_utility,
                                 &one,  0, 0, 0}, 
    {"Read PPB GPIO",            0,0, (PFT)read_ppb_gpio_utility,
                                 &one,  0, 0, 0}, 
    {"Verify FXS/FXO LED",       0,0, (PFT)fxs_fxo_led_utility,
                                 &one,  0, 0, 0}, 
    {"Upgrade Secondary FPGA",   0,0, (PFT)oak_fpga_upgrade_secondary,
                                 &one,  0, 0, 0},
    {"Debug Flag",               0,0, (PFT)open_close_debug_flag,
                                 &one,  0, 0, 0},
    {"Separate MB or DBx test Flag", 0,0, (PFT)setup_sep_test_dbx_flag,
                                 &one,  0, (PFT)is_phoenix, 0},
    {"Force setup Hardware board type Flag", 0,0, (PFT)setup_hw_brd_type_flag,
                                 &one,  0, (PFT)is_phoenix, 0},
    {"Upgrade CPLD firmware",    0,0, (PFT)fpga_upgrade_cpld,
                                 &one,  0, (PFT)is_phoenix, 0},
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


static struct mitem oakenshield_mainmenu[] = {
    {adiagfstr,                   0, 0, (PFI)menu, 
                                             (type_t *)&menu_diagflagp, 0,0, 0},
    {basutilstr,  0, 0, (PFI)menu, (int *)&utilmenup,                 0,  0, 0},
    {doalldgstr,                  0, 0, (PFI)do_menu_all_diags,
                (type_t *)&maindiagp, MF_CONTINUOUS, 0, 0}, {dogrpdgstr,       
                                                   0, 0, (PFI)do_menu_grp_diags, 
                                        (int *)&maindiagp, MF_CONTINUOUS, 0, 0},
    {"PPB Console Utility", 0,0, (PFT)ppb_con,                         0,0,0,0},
    {"FXS Utility", 0,0, (PFT)si32261_codec_utilities,         0,0,0,0},
    {"FXO Utility", 0,0, (PFT)si3050_codec_utilities,         0,0,0,0},
    {"DDR3 Memory test", 0,0, (PFT)test_mem, 0,MF_CONTINUOUS | MF_DOALL | 
                                                          MF_SHOW_ERRCOUNT,0,0},
    {"ARM11 CPU1 Boot test ", 0,0, (PFT)arm11_cpu1_boot_test, 0,MF_CONTINUOUS |  
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core0 Sanity test ", 0,0, (PFT)dss_core0_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core1 Sanity test ", 0,0, (PFT)dss_core1_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core2 Sanity test ", 0,0, (PFT)dss_core2_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core3 Sanity test ", 0,0, (PFT)dss_core3_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"FPGA Register Tests", 0,0,    (PFT)fpga_reg_test    , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"FPGA Memory Tests", 0,0,      (PFT)fpga_mem_test    , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"FPGA Interrupt Tests", 0,0,   (PFT)fpga_intr_test    , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"TDMSW16 force byte test", 0,0,   (PFT)tdmsw_force_byte_test    , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"TDM External Loopback test ", 0,0, (PFT)tdm_ext_lpbk , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"Silab FXS Codec Loopback test ", 0,0, (PFT)silab_fxs_lpbk_test , 0, MF_CONTINUOUS |
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"Silab FXO Codec Loopback test ", 0,0, (PFT)silab_fxo_lpbk_test , 0, MF_CONTINUOUS |
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"ECC Memory test ", 0,0, (PFT)ecc_mem_test , 0,MF_CONTINUOUS | MF_DOALL | 
                                                          MF_SHOW_ERRCOUNT,0,0},
    {"Load patch test ", 0,0, (PFT)load_patch_debug, 0,MF_CONTINUOUS | MF_DOALL | 
                                                          MF_SHOW_ERRCOUNT,0,0},
    {"TDM Codec Reset test ", 0,0, (PFT)tdm_codec_reset_test , 0, MF_CONTINUOUS |
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
};

static struct menuinfo  oakenshieldmainmenu = {
    "LSI SP270x Submenu",                             /* title */
    (int)0,                                           /* title param */
    (PFI)menu_show_dflags,                            /* show diag flags */
    0,                                                /* generic prompt */
    sizeof(oakenshield_mainmenu)/sizeof(struct mitem),   /* size of menu */
    oakenshield_mainmenu,
};


int oak_main_test (void)
{
    uchar board_id = 0xFF;
    int  retval = PASSED;
    board_id = get_oak_id();

    if (retval == PASSED) {
        retval = test_mem();
    }
    if (retval == PASSED) {
        retval = eth_int_lpbk(0);
    }
    if (retval == PASSED) {
        retval = eth_int_lpbk(1);
    }
    if (retval == PASSED) {
        retval = eth_ext_lpbk(0);
    }
    if (retval == PASSED) {
        retval = eth_ext_lpbk(1);
    }
    if (retval == PASSED) {
        retval = dss_core0_sanity(DSS_CORE0);
    }
    if (retval == PASSED) {
        retval = dss_core1_sanity(DSS_CORE1);
    }
    if (retval == PASSED) {
        if (is_vg400()) {
            /* VG400 dosen't have core 2 */
            bsp_debug_printf("\n\rVG400 dosen't have core 2"); 
        } else {
            retval = dss_core2_sanity(DSS_CORE2);
        }
    }
    if (retval == PASSED) {
        if (is_vg400()) {
            /* VG400 dosen't have core 3 */
            bsp_debug_printf("\n\rVG400 dosen't have core 3"); 
        } else {
            retval = dss_core3_sanity(DSS_CORE3);
        }
    }
    if (retval == PASSED) {
        retval = tdm_ext_lpbk(EXTERNAL);
    }
    if (retval == PASSED) {
        retval = ecc_mem_test();
    }
    if (retval == PASSED) {
        retval = silab_fxs_lpbk_test();
    }
    if (retval == PASSED) {
        if ((board_id == VG400_8FXS) || (board_id == PHOENIX_144FXS)) {
            retval = PASSED;
        } else {
            retval = silab_fxo_lpbk_test();
        }
    }
    return (retval);
}

void diag_menu (void)
{
#ifdef MENU_DEBUG
    uart_puts("\r\nIn diag_menu\n");
#endif

    maindiagp = &oakenshieldmainmenu;

    if (menu_display == 1) {
        menu(&oakenshieldmainmenu, 0, 0);
    } else {
        oak_main_test();
        prcomplete(testpass, errcount, 0);
        uart_puts("\r\n Enter <ctrl-a> <ctrl-x> to return to Host menu");
    }
}

static int eth_ext_lpbk (int port)
{
    uart_puts("\r\n Make sure loopback is enabled on host\n");
    uart_puts("\r\n ETH %d external loopback \n");
    return (ethernet_test(0, EXTERNAL, port));
}

static int eth_int_lpbk (int port)
{
    return (ethernet_test(0, INTERNAL, port));
}

int dss_core0_sanity (int core)
{
    return(dss_core_sanity(DSS_CORE0));
}

int dss_core1_sanity (int core)
{
    return(dss_core_sanity(DSS_CORE1));
}

int dss_core2_sanity (int core)
{
    if (is_vg400()) {
        bsp_debug_printf("\n\rVG400 dosen't have core 2");
        return 0;
    } else {
        return(dss_core_sanity(DSS_CORE2));
    }
}

int dss_core3_sanity (int core)
{
    if (is_vg400()) {
        bsp_debug_printf("\n\rVG400 dosen't have core 3");
        return 0;
    } else {
        return(dss_core_sanity(DSS_CORE3));
    }
}

void open_close_debug_flag (void)
{
    int flag = 0;
    bsp_debug_printf("\n\roak_diag_flag : %d",oak_diag_flag);
    flag = gethex_answer("\nEnter 1 to open flag, 0 to close", 0, 0, 1); 
    if (flag == 1) {
        oak_diag_flag = 1;
        bsp_debug_printf("oak_diag_flag on!");
    } else { 
        oak_diag_flag = 0;
        bsp_debug_printf("oak_diag_flag off!");
    }
}
/******** History ********
$Log: ppb_test.c,v $
Revision 1.6  2021/04/15 00:53:09  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.5  2018/08/30 06:39:51  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.4.20.2  2018/04/13 03:18:06  haohsu
Code Change for Vg400 (return calibration value, skip FXO loopback in FXO SKus)

Revision 1.4.20.1  2018/01/26 09:42:09  haohsu
*** empty log message ***

Revision 1.4  2017/08/14 08:28:21  harrchan
Rename Oakenshield DSP image to avoid conflicting with PVDM4.(CSCvf54988)

Revision 1.3  2017/08/09 08:12:25  harrchan
Display TDM bus number when FXS/FXO loopback fail

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:40  harrchan
Initial commit code for Oakenshield

Revision 1.8.84.7  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.8.84.6  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.8.84.5  2017/02/09 06:41:05  olin2
Support voltage margin and fail over port utility

Revision 1.8.84.4  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.8.84.3  2017/01/05 06:06:34  olin2
Support FXS Ring and Calibration

Revision 1.8.84.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.8.84.1  2016/12/14 04:57:39  olin2
Initial commit code for Oakenshield

Revision 1.8  2012/09/10 06:46:04  srane
Add ARM11 CPU1 test to dsp menu.

Revision 1.7  2012/08/15 15:03:00  srane
Add support for EMAC1 loopback test.

Revision 1.6  2012/07/17 20:34:43  srane
cleanup

Revision 1.5  2012/06/28 21:31:37  srane
add support routines for menu display.

Revision 1.4  2012/06/07 22:51:10  srane
TDM external loopback, ECC memory test

Revision 1.3  2012/05/24 23:25:47  srane
Add GPIO code to set ready bit, uart test, support both
uart mode and ethernet mode, other cleanup

Revision 1.2  2012/05/10 22:58:10  srane
Add TDM support.

Revision 1.1  2012/04/18 09:44:12  srane
Initial checkin


$Endlog$
*/

