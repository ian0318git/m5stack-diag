/* $Id: diag.c,v 1.7 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Pluggable Serial diagmon main menu and supporting wrappers.
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
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
#include "common_utils.h"
#include "pcmap.h"
#include "reva_def.h"
#include "prince_reg.h"
#include "prince_ecc.h"
#include "reva_reg.h"
#include "i2c_util.h"
#include "zynq_qspi.h"

#define DIAG_RTN_STS_OUT_PORT_BASE               (2016)
#define BP_GE_IP_ADDR                            "192.168.2.100"
#define BP_PASS_STR                              "PASS"
#define BP_FAIL_STR                              "FAIL"

#define UPGRADE_FOLDER        "/firmware/"
#define ZYNQ_UPGRADE_IMAGE    "plugser_sb_upgrade.bin"
#define TFTP_SERVERIP         "192.168.2.100"
#define SYSFS_GPIO_DIR        "/sys/class/gpio"

#define UPGRADE_IMAGE_START_ADDR 0x0
#define GOLDEN_IMAGE_START_ADDR  0x680000
#define MAX_BUF     (64)

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern int alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int memtest(), memloop(), addrloop(), find_mem(), memdebug();

extern int linux_memory_tester(int);
extern int zynq_qspi_regtest(void);
extern int zynq_qspi_reset(void);
extern int zynq_qspi_showinfo(void);
extern int zynq_qspi_wrtest(void);
extern int zynq_qspi_rdtest(void);
extern int zynq_qspi_erstest(void);
extern int zynq_qspi_rdcfg(void);
extern int zynq_qspi_lock(void);
extern int zynq_clear_sr(void);

extern int prince_firmware_upgrade(void);
extern int prince_img_lock(void);
extern int prince_golden_lock_check(void);
extern int prince_golden_lock_check_all(void);
extern int prince_scc_intr_test(int);
extern void display_sys_info(int);
extern int SERDES_TYPE_0;
extern int SERDES_TYPE_1;

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

static int fpga_sys_reg_rd(void);
static int fpga_sys_reg_wr(void);
static int fpga_sys_reg_dp(void);

static int linux_memory_test(int);
static int led_test(void);
static int intr_test(void);
static boolean has_hidden_item(int);
static boolean is_ecc_enabled(void);
static int get_ecc_status(void);
static int plugser_uart_msg_exh_test(void);
static int loopback_led_util(int);
static int activity_led_util(int);
void plugser_firmware_upgrade(int);
int fpga_version(void);
int plugser_7015_tftp_firmware_upgrade(void);
int serdes_type_gpio_test(void);
int serdes_gpio_test(void);
int gpio_get_value(unsigned int, unsigned int *);

/*
 * Global variables
 */
static boolean ecc_occur = 0;

/* This Fru table will be used by linux_error.c in common/src */
fru_table_t platform_fru_table[];

/* FRU PID and Location Strings */
uchar io_pid[] = "IO-PID";
uchar dimm_pid[] = "DIMM-PID";

uchar io_loc[] = "IO";
uchar dimm0_loc[] = "IO/DIMM0";

fru_table_t platform_fru_table[] = {
    { io_pid,        io_loc },
    { dimm_pid,      dimm0_loc },
};

static reg_info_t fpga_sys_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"FPGA Revision",                
     0x00,      READ_ONLY,  {4}, 0x01ffffff, 0x0},
    {"FPGA Timestamp",       
     0x04,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"LED and Status",       
     0x08,      READ_ONLY,  {4}, 0x00001313, 0x0},
    {"MAC Control",     
     0x0c,      READ_WRITE, {4}, 0xffffff1f, 0x28000c1d}, 
    {"MAC Pause Request",     
     0x10,      READ_WRITE, {4}, 0x0000ffff, 0x0000ffff}, 
    {"RGMII Inband Status",   
     0x14,      READ_ONLY,  {4}, 0xffffffff, 0x0}, 
    {"Scratchpad Register",   
     0x18,      READ_WRITE, {4}, 0xffffffff, 0x0}, 
    {"END",  0x000,  0,     {0}, 0x0,        0x0},
};

/* =========================================
 *  DDR3 Memory Access Utilities
 * ========================================= */
static struct mitem ddr3_mem_items[] = {
    {"alter memory",            0,      0,
     (PFT)alt_mem,              &one,   0, (type_t(*)())0, 0},
    {"compare memory block",    0,      0,
     (PFT)cmp_mem,              &one,   0, (type_t(*)())0, 0},
    {"display memory",          0,      0,
     (PFT)dis_mem,              &one,   0, (type_t(*)())0, 0},
    {"move memory block",       0,      0,
     (PFT)mov_mem,              &one,   0, (type_t(*)())0, 0},
    {"fill memory",             0,      0,
    (PFT)fil_mem,               &one,   0, (type_t(*)())0, 0},
    {"memory test",             0,      0,
     (PFT)memtest,              &one,   0, (type_t(*)())0, 0},
    {"memory read or write loop",   0,      0,
     (PFT)memloop,              &one,   0, (type_t(*)())0, 0},
    {"memory debug loop",       0,      0,
     (PFT)memdebug,             &one,   0, (type_t(*)())0, 0},
    {"address loop",            0,      0,
     (PFT)addrloop,             &one,   0, (type_t(*)())0, 0},
};

static struct menuinfo ddr3_mem_menu = {
    "DDR3 Memory Access Utilities Menu",
    0,
    0,
    0,
    sizeof(ddr3_mem_items)/sizeof(struct mitem),
    ddr3_mem_items,
};
static struct menuinfo *ddr3_mem_menup = &ddr3_mem_menu;

/* =========================================
 *  FPGA Utilities
 * ========================================= */
static struct mitem fpga_util_items[] = {
    {"Show Board type/FPGA version",    0,      0,
     (PFT)fpga_version,         &one,   0, (type_t(*)())0, 0},
    {"FPGA system register read",       0,      0, 
     (PFT)fpga_sys_reg_rd,      &one,   0, (type_t(*)())0, 0},
    {"FPGA system register write",      0,      0, 
     (PFT)fpga_sys_reg_wr,      &one,   0, (type_t(*)())0, 0},
    {"FPGA system registers dump",      0,      0, 
     (PFT)fpga_sys_reg_dp,      &one,   0, (type_t(*)())0, 0},
    {"GE DMA register read",            0,      0, 
     (PFT)ge_dma_reg_rd,   &one,        0, (type_t(*)())0, 0},
    {"GE DMA register write",           0,      0, 
     (PFT)ge_dma_reg_wr,   &one,        0, (type_t(*)())0, 0},
    {"GE DMA registers dump",           0,      0, 
     (PFT)ge_dma_reg_dp,   &one,        0, (type_t(*)())0, 0},
    {"GE MAC register read",            0,      0, 
     (PFT)ge_mac_reg_rd,   &one,        0, (type_t(*)())0, 0},
    {"GE MAC register write",           0,      0, 
     (PFT)ge_mac_reg_wr,   &one,        0, (type_t(*)())0, 0},
    {"GE MAC registers dump",           0,      0, 
     (PFT)ge_mac_reg_dp,   &one,        0, (type_t(*)())0, 0},
    {"SCC register read",               0,      0, 
     (PFT)scc_reg_rd,   &one,           0, (type_t(*)())0, 0},
    {"SCC register write",              0,      0, 
     (PFT)scc_reg_wr,   &one,           0, (type_t(*)())0, 0},
    {"SCC registers dump",              0,      0, 
     (PFT)scc_reg_dp,   &one,           0, (type_t(*)())0, 0},
 };

static struct menuinfo fpga_util_menu = {
    "FPGA Utilities Menu",
    0,
    0,
    0,
    sizeof(fpga_util_items)/sizeof(struct mitem),
    fpga_util_items,
};
static struct menuinfo *fpga_util_menup = &fpga_util_menu;

/* =========================================
 *  PHY Utilities
 * ========================================= */
static struct mitem phy_util_items[] = {
    {"PHY register read",               0,  0,
     (PFT)phy_reg_rd,           &one,   0, (type_t(*)())0, 0},
    {"PHY register write",              0,  0,
     (PFT)phy_reg_wr,           &one,   0, (type_t(*)())0, 0},
    {"PHY register dump",               0,  0,
     (PFT)phy_reg_dp,           &one,   0, (type_t(*)())0, 0},
    {"SMI register dump",               0,  0,
     (PFT)smi_reg_dp,           &one,   0, (type_t(*)())0, 0},
};

static struct menuinfo phy_util_menu = {
    "PHY Utilities Menu",
    0,
    0,
    0,
    sizeof(phy_util_items)/sizeof(struct mitem),
    phy_util_items,
};
static struct menuinfo *phy_util_menup = &phy_util_menu;

/* =========================================
 *  QSPI Flash Utilities
 * ========================================= */
static struct mitem qspi_flash_util_items[] = {
    {"QSPI flash register test", 0,0, (PFT)zynq_qspi_regtest,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash reset",         0,0, (PFT)zynq_qspi_reset,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash information",   0,0, (PFT)zynq_qspi_showinfo,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash read test",     0,0, (PFT)zynq_qspi_rdtest,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash write test",    0,0, (PFT)zynq_qspi_wrtest,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash sector erase",  0,0, (PFT)zynq_qspi_erstest,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash read config reg",  0,0, (PFT)zynq_qspi_rdcfg,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash sector lock",   0,0, (PFT)zynq_qspi_lock,
                                 &one,  0, (type_t(*)())0, 0},
    {"QSPI flash clear status reg", 0,0, (PFT)zynq_clear_sr,
                                 &one,  0, (type_t(*)())0, 0},
};

static struct menuinfo qspi_flash_util_menu = {
    "QSPI Flash Utilities Menu",
    0,
    0,
    0,
    sizeof(qspi_flash_util_items)/sizeof(struct mitem),
    qspi_flash_util_items,
};
static struct menuinfo *qspi_flash_util_menup = &qspi_flash_util_menu;

/* =========================================
 *   Basic utilities
 * ========================================= */
static struct mitem utilmenuitems[] = {
    {"System Information", 0, 0,
     (PFT) display_sys_info, (type_t *) & one,      0, (type_t(*)())0, 0},
    {"DDR3 Memory Access",      0,      0,
     (PFT)menu,     (type_t *)&ddr3_mem_menup,      0, (type_t(*)())0,0},
    {"FPGA Utilities",          0,      0,
     (PFT)menu,     (type_t *)&fpga_util_menup,     0, (type_t(*)())0,0},
    {"PHY Utilities",          0,      0,
     (PFT)menu,     (type_t *)&phy_util_menup,     0, (type_t(*)())0,0},
    {"QSPI Flash Utilities",     0,      0,
     (PFT)menu,     (type_t *)&qspi_flash_util_menup,    0, (type_t(*)())0,0},
    {"QSPI boot code/firmware upgrade",  0, 0,
     (PFT)plugser_firmware_upgrade, &one, 0, (type_t(*)())0,0},
    {"QSPI firmware and golden image protection",     0,      0,
     (PFT)prince_img_lock,      &one,    0, (type_t(*)())0,0},
    {"QSPI lock test",      0,      0,
     (PFT)prince_golden_lock_check,   &one,    0, (type_t(*)())0,0},
    {"QSPI lock test all",      0,      0,
     (PFT)prince_golden_lock_check_all,   &one,    0, (type_t(*)())0,0},
    {"Backplane loopback test",  0, 0,
     (PFT)phy_ext_lpbk_test_raw_skt,    &one,    0, (type_t(*)())0,0},
    {"Loopback LED Utility",  0, 0,
     (PFT)loopback_led_util,    &one,    0, (type_t(*)())0,0},
    {"Activity LED Utility",  0, 0,
     (PFT)activity_led_util,    &one,    0, (type_t(*)())0,0},
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

/* =========================================
 *  Main menu items
 * ========================================= */
static submenu_xtable_t main_menu_table[] = {
    {"Linux memory test",
     (PFT)linux_memory_test,        FALSE,      
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)linux_memory_test,     TRUE},
    {"MAC register test",
     (PFT)ge_mac_reg_test,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)ge_mac_reg_test,       TRUE},
    {"MAC loopback test",
     (PFT)ge_mac_lpbk_test_raw_skt,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)ge_mac_lpbk_test_raw_skt, TRUE},
    {"GE DMA register test",
     (PFT)ge_dma_reg_test,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)ge_dma_reg_test,       TRUE},
    {"PHY register test",
     (PFT)phy_reg_test,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)phy_reg_test,          TRUE},
    {"PHY internal loopback test",
     (PFT)phy_int_lpbk_test_raw_skt,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)phy_int_lpbk_test_raw_skt, TRUE},
    {"Serial channel test for SYNC/ASYNC",
     (PFT)serial_channel_test, FALSE,       
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)serial_channel_test,   TRUE},
    {"Interrupt test",
     (PFT)intr_test,         FALSE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)0,                     0},
    {"LED Test",
     (PFT)led_test,         0,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)0,                     0},
    {"Dummy item to send string to UART",
     (PFT)plugser_uart_msg_exh_test,         0,          0,
     (type_t(*)())0, 0,     (PFT)0,                     0},
    {"SerDes Type GPIO Test(Execute from host side)",
     (PFT)serdes_type_gpio_test,            0,          0,
     (type_t(*)())0, 0,     (PFT)0,                     0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Pluggable Serial Main %s", /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*********************************************************************
 * Function: ecc_check_thread
 *
 * Description: Create a thread to monitor ECC status reg.
 *
 * Inputs: arg
 * Outputs: None
 *********************************************************************/
void *ecc_check_thread (void *arg)
{
    ulong* reg_p;
    ulong base_addr = get_ps_ddr_ctrl_base();
    reg_p = (ulong* )(base_addr + ZYNQ_DDRC_ECC_STAT_REG_OFFSET);

    /* Get the ECC status when it's enabled */
    if (is_ecc_enabled()) {
        while(1) {
            msleep(200);
            if( 0 != ((*reg_p) >> 8) ) {
                get_ecc_status();
                ecc_occur = 1;
                cterr('f', 0, "ECC Error.");
            }
        }
    }
    return ((void *)0);
}

/*********************************************************************
 * Function: diag_menu
 * Description: This is the main entry to diag menu interface.
 * Inputs: argc
 *         argv
 * Outputs: None
 *********************************************************************
 */
void diag_menu (int argc, char *argv[]) 
{
    char arg;

    if(argc > 1) {
        arg = *argv[1]; 
    } else { 
        arg = 0;
    }
    testname("Pluggable Serial ");
    (NVRAM)->pollcon = 1;       /* poll the console */

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
        &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
        main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

/******************************************************************************
 * Function: is_ecc_enabled
 *
 * Description: Check the ECC mode is enabled or not?
 *
 * Input : None
 * Output: TRUE - ECC mode is enabled
 *         FALSE - ECC mode is NOT enabled
 ******************************************************************************/
static boolean is_ecc_enabled (void)
{
    ulong base_addr = get_ps_ddr_ctrl_base();
    ulong* ddr_ctrl_reg_p;
    ulong* ecc_scrub_reg_p;
    ulong width;
    ulong ecc_mode;

    /* Read DDR bus width */
    ddr_ctrl_reg_p = (ulong *)(base_addr + ZYNQ_DDRC_CONTROL_REG_OFFSET);
    width = (*ddr_ctrl_reg_p & ZYNQ_DDRC_CTRLREG_BUSWIDTH_MASK) >>
            ZYNQ_DDRC_CTRLREG_BUSWIDTH_SHIFT;

    /* Read ECC mode */
    ecc_scrub_reg_p = (ulong *)(base_addr + ZYNQ_DDRC_ECC_SCRUB_REG_OFFSET);
    ecc_mode = *ecc_scrub_reg_p & ZYNQ_DDRC_ECC_SCRUBREG_ECC_MODE_MASK;

    if ((width == ZYNQ_DDRCTL_WDTH_16) && 
        (ecc_mode & ZYNQ_DDRC_ECC_SCRUBREG_ECCMODE_SECDED)) {
        printf("ECC is enabled.\n");
        ulong* reg_p = (ulong *)(base_addr + ZYNQ_DDRC_ECC_CONTROL_REG_OFFSET);
        *reg_p = 0;
        return (TRUE);
    } else {
        printf("ECC is NOT enabled.\n");
        return (FALSE);
    }
}

/******************************************************************************
 * Function: get_ecc_status
 *
 * Description: Print Plug_Serial DDR ECC ststus register, CE/UE ststus register
 *              and ECC error count
 *
 * Input : None
 * Output: PASSED/FAILED
 ******************************************************************************/
static int get_ecc_status (void)
{
    ulong base_addr = get_ps_ddr_ctrl_base();
    struct zynq_ecc_status_reg_info ecc_regs;
    struct zynq_ecc_status err_status;
    ulong* reg_p;
    ulong clearval;

    printf("\nPluggable Serial DDR ECC STATUS REGISTERS:\n");

    reg_p = (ulong* )(base_addr + ZYNQ_DDRC_ECC_CE_LOG_REG_OFFSET);
    printf("ECC error status                    @%#x = %#x\n", reg_p, *reg_p);
    ecc_regs.ecc_ce_log = *reg_p;

    reg_p = (ulong* )(base_addr + ZYNQ_DDRC_ECC_CE_ADDR_REG_OFFSET);
    printf("ECC error address                   @%#x = %#x\n", reg_p, *reg_p);
    ecc_regs.ecc_ce_addr = *reg_p;

    reg_p = (ulong* )(base_addr + ZYNQ_DDRC_ECC_UE_LOG_REG_OFFSET);
    printf("ECC unrecoverable error status      @%#x = %#x\n", reg_p, *reg_p);
    ecc_regs.ecc_ue_log = *reg_p;

    reg_p = (ulong* )(base_addr + ZYNQ_DDRC_ECC_UE_ADDR_REG_OFFSET);
    printf("ECC unrecoverable error address     @%#x = %#x\n", reg_p, *reg_p);
    ecc_regs.ecc_ue_addr = *reg_p;

    reg_p = (ulong* )(base_addr + ZYNQ_DDRC_ECC_STAT_REG_OFFSET);
    printf("ECC error count                     @%#x = %#x\n", reg_p, *reg_p);
    ecc_regs.ecc_stat = *reg_p;

    ecc_regs.ecc_stat = ecc_regs.ecc_stat &
            (ZYNQ_DDRC_ECC_STATREG_UECOUNT_MASK |
             ZYNQ_DDRC_ECC_STATREG_CECOUNT_MASK);

    if (ecc_regs.ecc_stat == 0) {
        return (PASSED);
    }

    memset(&err_status, 0, sizeof(struct zynq_ecc_status));

    err_status.ce_count = (ecc_regs.ecc_stat & ZYNQ_DDRC_ECC_STATREG_CECOUNT_MASK) >>
                           ZYNQ_DDRC_ECC_STATREG_CECOUNT_SHIFT;
    err_status.ue_count = (ecc_regs.ecc_stat & ZYNQ_DDRC_ECC_STATREG_UECOUNT_MASK);

    if (err_status.ce_count) {
        if (ecc_regs.ecc_ce_log & ZYNQ_DDRC_ECC_CE_LOGREG_VALID) {
            err_status.ceinfo.bitpos = (ecc_regs.ecc_ce_log &
                ZYNQ_DDRC_ECC_CE_LOGREG_BITPOS_MASK) >>
                ZYNQ_DDRC_ECC_CE_LOGREG_BITPOS_SHIFT;
            err_status.ceinfo.row = (ecc_regs.ecc_ce_addr &
                ZYNQ_DDRC_ECC_ADDRREG_ROW_MASK) >>
                ZYNQ_DDRC_ECC_ADDRREG_ROW_SHIFT;
            err_status.ceinfo.col = (ecc_regs.ecc_ce_addr &
                ZYNQ_DDRC_ECC_ADDRREG_COL_MASK);
            err_status.ceinfo.bank = (ecc_regs.ecc_ce_addr &
                ZYNQ_DDRC_ECC_ADDRREG_BANK_MASK) >>
                ZYNQ_DDRC_ECC_ADDRREG_BANK_SHIFT;
            err_status.ceinfo.data = *(ulong *)(base_addr +
                ZYNQ_DDRC_ECC_CE_DATA_31_0_REG_OFFSET);
            printf("DDR ECC CE : Row %d Bank %d Col %d"
                " bitposition: %d data: %#x\n",
                err_status.ceinfo.row, 
                err_status.ceinfo.bank, 
                err_status.ceinfo.col,
                err_status.ceinfo.bitpos,
                err_status.ceinfo.data);
        }
        clearval = ZYNQ_DDRC_ECCCTRL_CLR_CE_ERR;
    }

    if (err_status.ue_count) {
        if (ecc_regs.ecc_ue_log & ZYNQ_DDRC_ECC_CE_LOGREG_VALID) {
            err_status.ueinfo.row = (ecc_regs.ecc_ue_addr &
                ZYNQ_DDRC_ECC_ADDRREG_ROW_MASK) >>
                ZYNQ_DDRC_ECC_ADDRREG_ROW_SHIFT;
            err_status.ueinfo.col = (ecc_regs.ecc_ue_addr &
                ZYNQ_DDRC_ECC_ADDRREG_COL_MASK);
            err_status.ueinfo.bank = (ecc_regs.ecc_ue_addr &
                ZYNQ_DDRC_ECC_ADDRREG_BANK_MASK) >>
                ZYNQ_DDRC_ECC_ADDRREG_BANK_SHIFT;
            err_status.ueinfo.data = *(ulong *)(base_addr +
                ZYNQ_DDRC_ECC_UE_DATA_31_0_REG_OFFSET);
            printf("DDR ECC UE : Row %d Bank %d Col %d data %#x\n",
                err_status.ueinfo.row,
                err_status.ueinfo.bank,
                err_status.ueinfo.col,
                err_status.ueinfo.data);
            }
        clearval |= ZYNQ_DDRC_ECCCTRL_CLR_UE_ERR;
    }

    reg_p = (ulong *)(base_addr + ZYNQ_DDRC_ECC_CONTROL_REG_OFFSET);
    *reg_p = clearval;
    *reg_p = 0;

    return (FAILED);
}

/******************************************************************************
 * Function: linux_memory_test
 *
 * Description: Linux memory test
 *
 * Input : option - option of memory test algorithm
 * Output: PASSED/FAILED
 ******************************************************************************/
static int linux_memory_test (int option)
{
    int rc = 0;

    testname("Memory");

    rc = linux_memory_tester(option);

    /* Get the ECC status when it's enabled */
    if (is_ecc_enabled()) {
        if (get_ecc_status()) {
            cterr('f', 0, "ECC Error.");
            return (FAILED);
        }
    }

    return (rc);
}

/******************************************************************************
 * Function: led_test
 *
 * Description: LED test, Turn LED off -> Turn on LED(amber) for 3 second
 *               -> Turn on LED(green) for 3 second
 *
 * Input : None
 * Output: PASSED/FAILED
 ******************************************************************************/
static int led_test (void)
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;

    testname("LED");

    /* Turn LED off */
    sys_csr->leds_sts &= ~LED_LPBK;
    sys_csr->leds_sts &= ~LED_ACTIV;
    sleep(1);

    /* Turn on Loopback LED(amber) for 1 second */
    sys_csr->leds_sts |= LED_LPBK;
    printf("LED: AMBER\n");
    sleep(1);
    sys_csr->leds_sts &= ~LED_LPBK;

    /* Turn on Activity LED(green) */
    sys_csr->leds_sts |= LED_ACTIV;
    printf("LED: GREEN\n");
    sleep(1);
    sys_csr->leds_sts &= ~LED_ACTIV;

    return (PASSED);
}

/******************************************************************************
 * Function: intr_test
 *
 * Description: Interrupt test
 *
 * Input : None
 * Output: PASSED/FAILED
 ******************************************************************************/
static int intr_test (void)
{
    int rc;

    testname("INTERRUPT");

    prpass(testpass, "MAC interrupt test.");
    rc = ge_mac_intr_test(0);
    if (rc == FAILED) {
        return (rc);
    }

    prpass(testpass, "PHY interrupt test.");
    rc = phy_intr_test(0);
    if (rc == FAILED) {
        return (rc);
    }

    prpass(testpass, "SCC interrupt test.");
    rc = prince_scc_intr_test(0);
    if (rc == FAILED) {
        return (rc);
    }

    return (rc);
}

/******************************************************************************
 * Function: fpga_version
 *
 * Description: Print Board type/FPGA version/Release date from FPGA reg
 *
 * Input : None
 * Output: PASSED/FAILED
 ******************************************************************************/
int fpga_version (void)
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;
    ulong revision = sys_csr->fpga_revision;
    ulong timestamp = sys_csr->fpga_timestamp;
    int board_id;
    uchar *board_type[16];
    
    sprintf(board_type, "P-1T");

    printf("Board type:         %s\n", board_type);
    printf("FPGA revision:      %x.%x\n", (revision >> 16) & 0xff, (revision >> 8) & 0xff);
    printf("Official release:   %s\n", (revision & (1 << 24)) ? "No" : "Yes");
    printf("Release date:       %x/%x/%x\n", (timestamp >> 24) & 0xff, 
          (timestamp >> 16) & 0xff, (timestamp >> 8) & 0xff);

    return (PASSED);
}

/******************************************************************************
 * Function: check_offset
 *
 * Description: Check whether the register offset is valid
 *
 * Input : offset - register offset
 *         reg_table_p - Point to which type of reg table
 * Output: None
 ******************************************************************************/
int check_offset (ushort offset, reg_info_t* reg_table_p)
{
    for (; reg_table_p->size.size != 0; reg_table_p++) {
        if (reg_table_p->offset == offset) {
            return (PASSED);
        }
    }
    return (FAILED);
}

/******************************************************************************
 * Function: get_reg_size
 *
 * Description: Get the register size
 *
 * Input : offset - register offset
 *         reg_table_p - Point to which type of reg table
 * Output: the register size, 0 if invalid offset
 ******************************************************************************/
ulong get_reg_size (ushort offset, reg_info_t* reg_table_p)
{
    for (; reg_table_p->size.size != 0; reg_table_p++) {
        if (reg_table_p->offset == offset) {
            return (reg_table_p->size.size);
        }
    }
    return (0);
}

/******************************************************************************
 * Function: fpga_sys_reg_rd
 *
 * Description: Read FPGA system register
 *
 * Input : None
 * Output: PASSED/FAILED
 ******************************************************************************/
static int fpga_sys_reg_rd (void)
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    ulong *reg_p;

    base_addr = get_fpga_base();

    offset = gethex_answer("\nEnter register offset[0x00 to 0x18]:",
               0, 0, 0x18);

    /* all the FPGA CSR registers are 4 bytes aligned */
    offset &= 0xfc;

    if (check_offset(offset, fpga_sys_reg_table)) {
        printf("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        reg_p = (ulong *)(base_addr + offset);
        reg_data = *reg_p;
        printf("\n register value @%#x = %#x ", (base_addr + offset), reg_data);
        return (PASSED);
    }
}

/******************************************************************************
 * Function: fpga_sys_reg_wr
 *
 * Description: Write FPGA system register
 *
 * Input : None
 * Output: PASSED/FAILED
 ******************************************************************************/
static int fpga_sys_reg_wr (void)
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    ulong *reg_p;

    base_addr = get_fpga_base();

    offset = gethex_answer("\nEnter register offset[0x00 to 0x18]:",
               0, 0, 0x18);
    /* all the FPGA CSR registers are 4 bytes aligned */
    offset &= 0xfc;

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
                0, 0, 0xffffffff);
   
    if (check_offset(offset, fpga_sys_reg_table)) {
        printf("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        reg_p = (ulong *)(base_addr + offset);
        *reg_p = reg_data;
        printf("\n register value @%#x = %#x ", (base_addr + offset), *reg_p);
        return (PASSED);
    }
}

/******************************************************************************
 * Function: reg_dump
 *
 * Description: Dump FPGA register
 *
 * Input : base_addr - FPGA base addr
 *         reg_table_p - Point to which type of reg table
 * Output: None
 ******************************************************************************/
void reg_dump (ulong base_addr, reg_info_t* reg_table_p)
{
    uchar *reg_p;

    for (; reg_table_p->size.size != 0; reg_table_p++) {
        reg_p = (uchar *)(base_addr + reg_table_p->offset);

        switch (reg_table_p->size.size) {
        case 1:
            printf("\n %s @%#x = %#x ", reg_table_p->name, 
                (base_addr + reg_table_p->offset), *reg_p);
        break;
        case 2:
            printf("\n %s @%#x = %#x ", reg_table_p->name, 
                (base_addr + reg_table_p->offset), *(ushort *)reg_p);
        break;
        case 4:
            printf("\n %s @%#x = %#x ", reg_table_p->name, 
                (base_addr + reg_table_p->offset), *(ulong *)reg_p);
        break;
        }
    }
}

/******************************************************************************
 * Function: fpga_sys_reg_dp
 *
 * Description: Dump FPGA system register
 *
 * Input : None
 * Output: PASSED
 ******************************************************************************/
static int fpga_sys_reg_dp (void)
{
    ulong base_addr = get_fpga_base();
    reg_info_t *reg_table_p = &fpga_sys_reg_table[0];

    reg_dump(base_addr, reg_table_p);

    return (PASSED);
}

/******************************************************************************
 * Function: plugser_uart_msg_exh_test
 *
 * Description: 'uname' to generate strings for the host to catch via UART.
 *               This is to respond to overlord side UART test
 *
 * Input : None
 * Output: PASSED
 ******************************************************************************/
static int plugser_uart_msg_exh_test (void)
{
    /* using 'uname' to dispay system info as a string.
     * x86 side will compare string for uart test 
     */ 
    system("uname");
    return (PASSED);
}

/******************************************************************************
 * Function: plugser_check_diag_flags 
 *
 * Description: This function checks the diag flags passed from Host and 
 *              modifys module diag flags accordingly.
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *****************************************************************************/
static int plugser_check_diag_flags (void)
{
    FILE *fp;
    char flag_file[32];
    char buf[256];
    char cmd[32];

    sprintf(flag_file, "/tmp/host_flags");


    fp = fopen(flag_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, flag_file);
        return (FAILED);
    }

    if (fgets(buf, sizeof(buf), fp) != NULL) {
        sprintf(cmd, "cat %s", flag_file);
        system(cmd);
        sscanf(buf, "diagflag=%x\tdiagflag_xram=%x",
            &((NVRAM)->diagflag), &diagflag_xram);
        printf("(NVRAM)->diagflag = %#x, diagflag_xram = %#x\n", 
            (NVRAM)->diagflag, diagflag_xram);
    } else {
        printf("Warning: flag file is empty.\n");
        fclose(fp);
        return (FAILED);
    }

    fclose(fp);

    return (PASSED);
}

/**********************************************************************
 * Function: diag_report_status_host
 *
 * This function reports the pass/fail status to host through nc
 *
 * Input : str - status string
 * Output: None
 **********************************************************************/
static void diag_report_status_host (char *str)
{
    char cmd[128];

    /* Sanity check */
    if (str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return;
    }

    if(ecc_occur == 0) {
        sprintf(cmd, "echo %s > result", str);
    } else {
        sprintf(cmd, "echo %s,ECC > result", str);
    }
    printf("%s\n", cmd);
    system(cmd);
    sleep(1);
    sprintf(cmd, "nc %s %d < result", BP_GE_IP_ADDR, DIAG_RTN_STS_OUT_PORT_BASE);
    printf("Pluggable Serial nc command: %s\n", cmd);
    system(cmd);
    sleep(1);
}


/******************************************************************************
 * Function: doall_print_head 
 *
 * Description: This function prints out testname at the beginning of test 
 *
 * Inputs      : teststr - Test String 
 * Outputs     : None
 *****************************************************************************/
static void doall_print_head (char *teststr)
{
    printf("\n--- Running %s Test ---\n", teststr);
}


/******************************************************************************
 * Function: doall_print_tail
 *
 * Description: This function prints out testname at the end of test 
 *
 * Inputs      : teststr - Test String 
 * Outputs     : None
 *****************************************************************************/
static void doall_print_tail (char *teststr)
{
    printf("\n--- %s Test PASS ---\n", teststr);
}

/******************************************************************************
 * Function: diag_do_all
 *
 * Description: This function performs all tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *****************************************************************************/
int diag_do_all (void)
{
    char *ext_lpbk_flag;

    pthread_t tid;
    pthread_create(&tid, NULL, ecc_check_thread, NULL);
    msleep(200);

    /*
     * Check Host Flags
     */
    plugser_check_diag_flags();

    doall_print_head("Memory");
    if (linux_memory_test(0) == FAILED) {
        diag_report_status_host("Memory");
        cterr('f', 0, "Main Memory Test Fails");
        return (FAILED);
    }
    doall_print_tail("Memory");

    doall_print_head("MAC");
    if (ge_mac_reg_test(0) == FAILED) {
        diag_report_status_host("MAC_reg");
        cterr('f', 0, "MAC Register Test Fails");
        return (FAILED);
    }
    if (ge_mac_intr_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "MAC Interrupt Test Fails");
        return (FAILED);
    }
    if (ge_mac_lpbk_test_raw_skt() == FAILED) {
        diag_report_status_host("MAC_lpbk");
        cterr('f', 0, "MAC Loopback Test Fails");
        return (FAILED);
    }
    doall_print_tail("MAC");

    doall_print_head("DMA");
    if (ge_dma_reg_test(0) == FAILED) {
        diag_report_status_host("DMA_reg");
        cterr('f', 0, "DMA Register Test Fails");
        return (FAILED);
    }
    doall_print_tail("DMA");

    doall_print_head("PHY");
    if (phy_reg_test(0) == FAILED) {
        diag_report_status_host("PHY_reg");
        cterr('f', 0, "PHY Register Test Fails");
        return (FAILED);
    }
    if (phy_int_lpbk_test_raw_skt() == FAILED) {
        diag_report_status_host("PHY_int_lpbk");
        cterr('f', 0, "PHY Loopback Test Fails");
        return (FAILED);
    }
    doall_print_tail("PHY");

    doall_print_head("SCC");
    if (serial_channel_test(0) == FAILED) {
        diag_report_status_host("SCC");
        cterr('f', 0, "SCC Test Fails");
        return (FAILED);
    }
    doall_print_tail("SCC");

    doall_print_head("LED");
    if (led_test() == FAILED) {
        diag_report_status_host("LED");
        cterr('f', 0, "LED Test Fails");
        return (FAILED);
    }
    doall_print_tail("LED");

    doall_print_head("INTERRUPT");
    if (intr_test() == FAILED) {
        diag_report_status_host("INTERRUPT");
        cterr('f', 0, "INTERRUPT Test Fails");
        return (FAILED);
    }
    doall_print_tail("INTERRUPT");

    if (ecc_occur == 1)
    {
        diag_report_status_host("ECC");
        return (FAILED);
    }
    diag_report_status_host(BP_PASS_STR);

    return (PASSED);
}

/*********************************************************************
 * Function: has_hidden_item
 *
 * Description: Use this function to hidden menu item
 *
 * Inputs: dummy
 * Outputs: Return FALSE if item is hidden
 *********************************************************************/
boolean has_hidden_item (int dummy)
{
    return (FALSE);
}

/*********************************************************************
 * Function: plugser_firmware_upgrade
 *
 * Description: QSPI flash and SPI flash util.
 *
 * Inputs: dummy
 * Outputs: void
 *********************************************************************/
void plugser_firmware_upgrade (int dummy)
{
    uchar rdcmd;
    int rc = 0;

    printf("####################################\n");
    printf("# Current version #\n");
    fpga_version();

    printf("####################################\n");
    printf("# Upgrade version of option (a) below #\n");
    printf("####################################\n");
    rdcmd = (uchar)getc_answer("\n (a) QSPI flash for 7z007"
							   "\n (b) QSPI flash for 7z007 by using the image in the firmware directory"
                               "\n (e) Exit\n", "AaBbEe", 'e');

    switch (rdcmd) {
    case 'A':
    case 'a':
        printf("PLEASE DO NOT POWER DOWN DURING THE UPGRADE !!!\n");
        rdcmd = (uchar)getc_answer("Do you want to continue? (Y/N)", "YyNn", 'N');
        if(rdcmd == 'Y' || rdcmd == 'y') {
            prince_firmware_upgrade();
            printf("Image already upgraded to 7007 flash. Please reload Pluggable Serial module.\n");
        }
        break;
    case 'B':
    case 'b':
        printf("PLEASE DO NOT POWER DOWN DURING THE UPGRADE !!!\n");
        rdcmd = (uchar)getc_answer("Do you want to continue? (Y/N)", "YyNn", 'N');
        if(rdcmd == 'Y' || rdcmd == 'y') {
            rc = plugser_7015_tftp_firmware_upgrade();
            if (rc == PASSED) {
                printf("Image already upgraded to 7007 flash. Please reload Pluggable Serial module.\n");
            } else {
                printf("Upgrade failed!\n");
            }
        }
        break;
    case 'E':
    case 'e':
        break;
    default:
        printf("-- Wrong input --\n");
        break;
    }
}

/******************************************************************************
 * Function: plugser_7015_tftp_firmware_upgrade
 * Description: TFTP 7015 upgrade image ZYNQ_UPGRADE_IMAGE from the platform side.
 *
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int plugser_7015_tftp_firmware_upgrade (void)
{
    char cmd[256];
    FILE *fp;
    unsigned char *fw_data;
    int fw_size, i, fw_addr;

    rmdir(UPGRADE_FOLDER);
    mkdir(UPGRADE_FOLDER, 0777);

    sprintf(cmd, "tftp -g -r %s -l %s%s %s", ZYNQ_UPGRADE_IMAGE, UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE, TFTP_SERVERIP);
    system(cmd);

    fp = fopen(UPGRADE_FOLDER ZYNQ_UPGRADE_IMAGE,"rb");
    if (fp == NULL) {
        printf("Upgrade image isn't exist!\n");
        printf("Please put the upgrade image into platform side as %s%s manually\n", UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE);
        return (FAILED);
    }

    /* seek to end of file */
    fseek(fp, 0, SEEK_END);
    /* get current file pointer */
    fw_size = ftell(fp);
    /* seek back to beginning of file */
    fseek(fp, 0, SEEK_SET);

    if(fw_size <= 0) {
        printf("Upgrade image isn't exist!\n");
        printf("Please put the upgrade image into platform side as %s%s manually\n", UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE);
        fclose(fp);
        return (FAILED);
    }

    fw_data = (unsigned char *)malloc(fw_size * sizeof(char));
    fread(fw_data, fw_size, 1, fp);
    fclose(fp);

    i = (fw_size - 1) / SECTOR_SIZE + 1;
    if (i < 0) {
        i = 0;
    }
    fw_addr = UPGRADE_IMAGE_START_ADDR;

    printf("Erasing flash...\n");
    if (qspi_erase(fw_addr & 0xff0000, i)) {
        cterr('f', 0, "Failed to erase sector.\n");

        return (FAILED);
    }

    printf("Writing flash...\n");
    if (qspi_write(fw_addr, fw_size, fw_data, WRITE_CMD)) {
        cterr('f', 0, "Failed to program image.\n");

        return (FAILED);
    }
    printf("sector erase. i = %#x, fw_addr = %#x\n", i, fw_addr);
    printf("size  %d\n", fw_size);
    printf("\nFinish standard firmware image upgrade!\n");

    return (PASSED);
}

/******************************************************************************
 * Description:  wrapper for wlan_bootup test 
 * CSCvm45577 : Pluggable Serial - SerDes Type test Failed	        
 * Input : None
 * Output: PASSED
 ******************************************************************************/
int serdes_type_gpio_test (void)
{
    char cmd[128];

    doall_print_head("SERDES TYPE");
    if (serdes_gpio_test() == FAILED) {
        cterr('f', 0, "SERDES TYPE Test Fails");
        return (FAILED);
    }
    return (PASSED);
}
/******************************************************************************
 * Function: serdes_type_gpio_test
 *
 * Description: Read Serdes type GPIO pin value.
 *
 * Input : None
 * Output: PASSED
 ******************************************************************************/
int serdes_gpio_test (void)
{
    char cmd[64];
    int *serdes_type0, *serdes_type1;
    FILE *fp;
    char serdes_val[32];
    char buf[256];

    /* Step1 : Get SERDES_TYPE_0 and SERDES_TYPE_1 GPIO pin value */
    if (gpio_get_value(SERDES_TYPE_0, &serdes_type0)) {
        cterr('f', 0, "Failed to get SERDES_TYPE_0 GPIO %d value.\n", SERDES_TYPE_0);
        return (FAILED);
    }
    if (gpio_get_value(SERDES_TYPE_1, &serdes_type1)) {
        cterr('f', 0, "Failed to get SERDES_TYPE_1 GPIO %d value.\n", SERDES_TYPE_1);
        return (FAILED);
    }
    /* Step2 : Display SERDES_TYPE pin value */
    printf("\nModule: SERDES_TYPE_0: %d ; SERDES_TYPE_1: %d\n",serdes_type0, serdes_type1);

    return (PASSED);
}
/*******************************************************************************
 * Function: gpio_get_value
 * Description: Get the GPIO Pin value.
 *
 * Input : gpio - GPIO pin no.
 *         value - GPIO pin value
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/

int gpio_get_value (unsigned int gpio, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
        perror("gpio/get-value");
        return (FAILED);
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
        *value = 1;
	} else {
        *value = 0;
	}
 
	close(fd);
    return (PASSED);
}
/*******************************************************************************
 * Function   : loopback_led_util
 * Description: Loopback LED Utility for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int loopback_led_util (int input)
{
    int opt;
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;

    printf("Loopback LED Utility\n");
    opt = getdec_answer("LED action? (0-OFF, 1-ON):", 0, 0, 1);

    if (opt == 0) {
        /* Turn LED off */
        sys_csr->leds_sts &= ~LED_LPBK;
        printf("Loopback LED Off\n");
    } else {
        /* Turn on Loopback LED(amber) */
        sys_csr->leds_sts |= LED_LPBK;
        printf("Loopback LED On\n");
    }
}
/*******************************************************************************
 * Function   : activity_led_util
 * Description: Activity LED Utility for Pluggable Serial
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int activity_led_util (int input)
{
    int opt;
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;

    printf("Activity LED Utility\n");
    opt = getdec_answer("LED action? (0-OFF, 1-ON):", 0, 0, 1);

    if (opt == 0) {
        /* Turn LED off */
        sys_csr->leds_sts &= ~LED_ACTIV;
        printf("Activity LED Off\n");
    } else {
        /* Turn on Loopback LED(amber) */
        sys_csr->leds_sts |= LED_ACTIV;
        printf("Activity LED On\n");
    }
}

/******** History ******** 
$Log: diag.c,v $
Revision 1.7  2018/11/23 09:28:46  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.6  2018/09/21 03:01:16  iachang
CSCvm45577: Fixed SerDes Type GPIO test issue

Revision 1.5.10.1  2018/11/21 09:37:22  iachang
Sync up with main trunk.

Revision 1.5  2018/08/02 09:35:01  iachang
Merge Pluggable Serial from branch star-branch-c9xx to main trunk

Revision 1.4  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.3.2.1  2018/03/02 03:13:35  iachang
Support QSPI firmware and golden image protection
Support QSPI lock test

Revision 1.3  2018/02/09 09:17:33  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:54:52  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:58:56  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.7  2017/10/24 11:16:05  iachang
Supported SerDes Type GPIO Test.

Revision 1.1.4.6  2017/09/25 15:16:01  iachang
Fixed QSPI boot code/firmware upgrade TFTP download issue.

Revision 1.1.4.5  2017/09/13 16:54:29  iachang
Support Pluggable Serial test via NC command

Revision 1.1.4.4  2017/08/25 10:27:16  lucywang
modified for ASYNC test and FPGA update

Revision 1.1.4.3  2017/08/22 04:13:17  lucywang
add firmware upgrade for 7007 FPGA

Revision 1.1.4.2  2017/08/08 07:43:37  hondwang
add pluggable serial for star-branch-c9xx

Revision 1.1.2.1  2017/07/31 10:49:58  lucywang
add pluggable serial code of host and module


$Endlog$
*/
