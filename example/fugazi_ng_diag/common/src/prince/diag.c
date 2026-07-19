/* $Id: diag.c,v 1.12 2014/08/04 05:20:12 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Prince diagmon main menu and supporting wrappers.
 *
 * Xiaoying Zhang -- Oct. 2012
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
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
#include "prince_def.h"
#include "prince_reg.h"
#include "prince_ecc.h"
#include "module_fru.h"

#define DIAG_RTN_STS_TMP_FILE                    "/tmp/prince.status"
#define DIAG_RTN_STS_OUT_PORT_BASE               (2016)
#define DIAG_IP_ADDR                             "192.123.123.200"
#define BP_GE_IP_ADDR                            "192.123.123.1"
#define BP_PASS_STR                              "PASS"
#define BP_FAIL_STR                              "FAIL"

/* Function prototype */
extern int alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int memtest(), memloop(), addrloop(), find_mem(), memdebug();

extern int linux_memory_tester(int);

extern int zynq_i2c_reset(void);
extern int zynq_i2c_regtest(void);

extern int voltage_margin_specific(void);
extern void voltage_margin_diaplay(void);
extern int voltage_margin_low(void);
extern int voltage_margin_normal(void);
extern int voltage_margin_high(void);
extern int voltage_no_margin(void);
extern int margin_test(void);

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

static int fpga_version();
static int fpga_sys_reg_rd();
static int fpga_sys_reg_wr();
static int fpga_sys_reg_dp();


static int linux_memory_test(int);
static int led_test();
static int prince_uart_msg_exh_test(void);

/*
 * Global variables
 */
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
 *  Voltage Margining Utilities
 * ========================================= */
static struct mitem margin_util_items[] = {
    {"Margin all voltages to high", 0,0, (PFT)voltage_margin_high,
                                    &one,  0, (type_t(*)())0, 0},
    {"Margin all voltages to low",  0,0, (PFT)voltage_margin_low,
                                    &one,  0, (type_t(*)())0, 0},
    {"Set all voltages to normal",  0,0, (PFT)voltage_margin_normal,
                                    &one,  0, (type_t(*)())0, 0},
    {"Set all voltages to no margin", 0,0, (PFT)voltage_no_margin,
                                    &one,  0, (type_t(*)())0, 0},
    {"Margin a specific voltage",   0,0, (PFT)voltage_margin_specific,
                                    &one,  0, (type_t(*)())0, 0},
    {"Display current margins", 0,0, (PFT)voltage_margin_diaplay,
                            &one,  0, (type_t(*)())0, 0},
    {"I2c reset", 0,0, (PFT)zynq_i2c_reset,
                            &one,  0, (type_t(*)())0, 0},
    {"I2c register test", 0,0, (PFT)zynq_i2c_regtest,
                            &one,  0, (type_t(*)())0, 0},
};

static struct menuinfo margin_util_menu = {
    "Voltage Margin Utilities Menu",
    0,
    0,
    0,
    sizeof(margin_util_items)/sizeof(struct mitem),
    margin_util_items,
};
static struct menuinfo *margin_util_menup = &margin_util_menu;

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
    {"DDR3 Memory Access",      0,      0,
     (PFT)menu,     (type_t *)&ddr3_mem_menup,      0, (type_t(*)())0,0},
    {"FPGA Utilities",          0,      0,
     (PFT)menu,     (type_t *)&fpga_util_menup,     0, (type_t(*)())0,0},
    {"PHY Utilities",          0,      0,
     (PFT)menu,     (type_t *)&phy_util_menup,     0, (type_t(*)())0,0},
    {"Margin Utilities",        0,      0,
     (PFT)menu,     (type_t *)&margin_util_menup,   0, (type_t(*)())0,0},
    {"QSPI Flash Utilities",     0,      0,
     (PFT)menu,     (type_t *)&qspi_flash_util_menup,    0, (type_t(*)())0,0},
    {"QSPI boot code/firmware upgrade",  0, 0,
     (PFT)prince_firmware_upgrade, &one, 0, (type_t(*)())0,0},
    {"QSPI firmware and golden image protection",     0,      0,
     (PFT)prince_img_lock,      &one,    0, (type_t(*)())0,0},
    {"QSPI lock test",      0,      0,
     (PFT)prince_golden_lock_check,   &one,    0, (type_t(*)())0,0},
    {"QSPI lock test all",      0,      0,
     (PFT)prince_golden_lock_check_all,   &one,    0, (type_t(*)())0,0},
    {"Backplane loopback test",  0, 0,
     (PFT)phy_ext_lpbk_test_raw_skt,    &one,    0, (type_t(*)())0,0},
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
    {"MAC interrupt test",
     (PFT)ge_mac_intr_test,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)ge_mac_intr_test,      TRUE},
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
#if 0
    {"PHY interrupt test",
     (PFT)phy_intr_test,    FALSE,        
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)phy_intr_test,         TRUE},
#endif
    {"Serial channel test",
     (PFT)serial_channel_test, FALSE,       
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)serial_channel_test,   TRUE},
    {"Margin Test",
     (PFT)margin_test,         0,      
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)0,                     0},
    {"LED Test",
     (PFT)led_test,         0,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,     (PFT)0,                     0},
    {"Dummy item to send string to UART",
     (PFT)prince_uart_msg_exh_test,         0,          0,
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
    "Main %s",      /* title */
    0,              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,  /* shows major flags */
    0,              /* generic prompt */
    0,              /* size -- bumped by add_menu_item() */
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
    } else { 
        arg = 0;
    }

    testname("Prince NGWIC");

    (NVRAM)->pollcon = 1;       /* poll the console */

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
        &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
        main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

static boolean is_ecc_enabled(void)
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

static int get_ecc_status(void)
{
    ulong base_addr = get_ps_ddr_ctrl_base();
    struct zynq_ecc_status_reg_info ecc_regs;
    struct zynq_ecc_status err_status;
    ulong* reg_p;
    ulong clearval;

    printf("\nPRINCE DDR ECC STATUS REGISTERS:\n");

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

static int linux_memory_test(int option)
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

static int led_test()
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

static int fpga_version()
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;
    ulong revision = sys_csr->fpga_revision;
    ulong timestamp = sys_csr->fpga_timestamp;
    int board_id;
    uchar *board_type[16];

    board_id = get_board_id();
    switch(board_id) {
    case BOARD_ID_NIM_1T:
        sprintf(board_type, "NIM-1T");
        break;
    case BOARD_ID_NIM_2T:
        sprintf(board_type, "NIM-2T");
        break;
    case BOARD_ID_NIM_4T:
        sprintf(board_type, "NIM-4T");
        break;
    default:
        cterr('f', 0, "Invalid prince ID 0x%X", board_id);
        return (FAILED);
    }

    printf("Board type:         %s\n", board_type);
    printf("FPGA revision:      %x.%x\n", (revision >> 16) & 0xff, (revision >> 8) & 0xff);
    printf("Official release:   %s\n", (revision & (1 << 24)) ? "No" : "Yes");
    printf("Release date:       %x/%x/%x\n", (timestamp >> 24) & 0xff, 
        (timestamp >> 16) & 0xff, (timestamp >> 8) & 0xff);

    return (PASSED);
}

int get_board_id()
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;
    ulong board_id = (sys_csr->leds_sts & BOARD_ID_MASK) >> BOARD_ID_SHIFT;

//    printf("Board ID: %d\n", board_id);

    return (board_id);
}

/* Check whether the register offset is valid */
int check_offset (ushort offset, reg_info_t* reg_table_p)
{
    for (; reg_table_p->size.size != 0; reg_table_p++) {
        if (reg_table_p->offset == offset) {
            return (PASSED);
        }
    }
    return (FAILED);
}

/* Get the register size */
ulong get_reg_size (ushort offset, reg_info_t* reg_table_p)
{
    for (; reg_table_p->size.size != 0; reg_table_p++) {
        if (reg_table_p->offset == offset) {
            return (reg_table_p->size.size);
        }
    }
    return (0);
}

static int fpga_sys_reg_rd()
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

static int fpga_sys_reg_wr()
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

void reg_dump(ulong base_addr, reg_info_t* reg_table_p)
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

static int fpga_sys_reg_dp()
{
    ulong base_addr = get_fpga_base();
    reg_info_t *reg_table_p = &fpga_sys_reg_table[0];

    reg_dump(base_addr, reg_table_p);

    return PASSED;
}

/*
 * Function: prince_uart_msg_exh_test()
 *
 * 'uname' to generate strings for the host to catch via UART.
 *  This is to respond to overlord side UART test
 *
 * Input : NONE
 * Output: PASSED
 */
static int prince_uart_msg_exh_test (void)
{
    /* using 'uname' to dispay system info as a string.
     * x86 side will compare string for uart test 
     */ 
    system("uname");
    return PASSED;
}

/******************************************************************************
 *
 * Function: prince_check_diag_flags 
 *
 * Description: This function checks the diag flags passed from Host and 
                modifys module diag flags accordingly.
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static int prince_check_diag_flags (void)
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
 *
 * Function: diag_report_status_host
 *
 * This function reports the pass/fail status to host through nc
 *
 * Input : str - status string
 *
 * Output: none
 *
 **********************************************************************
 */
static void diag_report_status_host (char *str)
{
    char cmd[128];

    /* Sanity check */
    if (str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return;
    }

    sprintf(cmd, "echo %s > result", str);
    printf("%s\n", cmd);
    system(cmd);
    sleep(1);
    sprintf(cmd, "nc %s %d < result", BP_GE_IP_ADDR,
            DIAG_RTN_STS_OUT_PORT_BASE);
    printf("Prince nc command: %s\n", cmd);
    system(cmd);
    sleep(1);
}


/******************************************************************************
 *
 * Function: doall_print_head 
 *
 * Description: This function prints out testname at the beginning of test 
 *
 * Inputs      : teststr - Test String 
 * Outputs     : None
 *
 *****************************************************************************/
static void doall_print_head (char *teststr)
{
    printf("\n--- Running %s Test ---\n", teststr);
}


/******************************************************************************
 *
 * Function: doall_print_tail
 *
 * Description: This function prints out testname at the end of test 
 *
 * Inputs      : teststr - Test String 
 * Outputs     : None
 *
 *****************************************************************************/
static void doall_print_tail (char *teststr)
{
    printf("\n--- %s Test PASS ---\n", teststr);
}

/******************************************************************************
 *
 * Function: diag_do_all
 *
 * Description: This function performs all tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int diag_do_all (void)
{
    char *ext_lpbk_flag;

    /*
     * Check Host Flags
     */
    prince_check_diag_flags();

    doall_print_head("Memory");
    if (linux_memory_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "Main Memory Test Fails");
        return (FAILED);
    }
    doall_print_tail("Memory");

    doall_print_head("MAC");
    if (ge_mac_reg_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "MAC Register Test Fails");
        return (FAILED);
    }
    if (ge_mac_intr_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "MAC Interrupt Test Fails");
        return (FAILED);
    }
    if (ge_mac_lpbk_test_raw_skt() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "MAC Loopback Test Fails");
        return (FAILED);
    }
    doall_print_tail("MAC");

    doall_print_head("DMA");
    if (ge_dma_reg_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "DMA Register Test Fails");
        return (FAILED);
    }
    doall_print_tail("DMA");

    doall_print_head("PHY");
    if (phy_reg_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "PHY Register Test Fails");
        return (FAILED);
    }
    if (phy_int_lpbk_test_raw_skt() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "PHY Loopback Test Fails");
        return (FAILED);
    }
    doall_print_tail("PHY");

    doall_print_head("SCC");
    if (serial_channel_test(0) == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "SCC Test Fails");
        return (FAILED);
    }
    doall_print_tail("SCC");

    doall_print_head("Margin");
    if (margin_test() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "Margin Test Fails");
        return (FAILED);
    }
    doall_print_tail("Margin");

    doall_print_head("LED");
    if (led_test() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "LED Test Fails");
        return (FAILED);
    }
    doall_print_tail("LED");

    diag_report_status_host(BP_PASS_STR);

    return (PASSED);
}


/******** History ******** 
$Log: diag.c,v $
Revision 1.12  2014/08/04 05:20:12  xiaoyizh
Add routine to check the host diag flags.

Revision 1.11  2014/01/13 03:14:22  xiaoyizh
Add new routine for PHY external loopback test using raw socket.

Revision 1.10  2013/12/20 09:33:03  xiaoyizh
Add detailed info for FPGA revision dump.
Add golden image lock test in the utilities.

Revision 1.9  2013/11/26 08:40:38  hroni
fix compiler warning

Revision 1.8  2013/11/13 05:38:14  xiaoyizh
Remove PHY interrupt test since the irq is used by SW klm.

Revision 1.7  2013/10/08 11:03:50  erwu2
enhanced err msg first check-in

Revision 1.6  2013/09/03 07:15:58  liwwang
add support for QSPI images protection utility

Revision 1.5  2013/08/27 09:28:50  xiaoyizh
Add LED test in do_all routine.

Revision 1.4  2013/08/02 09:59:35  xiaoyizh
Remove DMA interrupt test. Add DDR ECC status monitoring.
Add routines and definitions to support nc communication.

Revision 1.3  2013/07/16 04:45:36  liwwang
update QSPI flash util items

Revision 1.2  2013/06/25 07:42:19  xiaoyizh
Update Margin test. Fix BoardId issue.

Revision 1.1  2013/04/19 07:17:49  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/
