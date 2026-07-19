/* $Id: prince_scc.c,v 1.4 2017/07/18 08:48:41 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_scc.c,v $
 *------------------------------------------------------------------
 *
 * prince_scc.c - Prince Serial Channel Controller main function/menu.
 *
 * Xiaoying Zhang -- Oct. 2012
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "defs.h"
#include "common_utils.h"
#include "menu.h"
#include "slot.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "pcmap.h"
#include "prince_reg.h"
#include "prince_def.h"
#include "prince_scc_struct.h"

extern int get_board_id();
extern int check_offset (ushort, reg_info_t*);
extern void reg_dump(ulong, reg_info_t*);
extern int ngwic_prince_chan_lpbk_test();

static mitem_t      scc_menu_items[MAX_SUBTEST_ITEMS];
static title_buf_t  scc_menu_title[MAX_SUBTEST_ITEMS];
static int          scc_menu_index[MAX_SUBTEST_ITEMS];
static title_buf_t  scc_menu_header;

static int scc_reg_test();
static int scc_intr_test();

static int num_chan = 0;

/* SCC Register Tables */
static reg_info_t scc_ctrl_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"SCC Revision",                
     0x00,      READ_ONLY,  {4}, 0xffffffff, 0x4b570000},
    {"MGMT Interrupt Status",       
     0x10,      READ_ONLY,  {2}, 0x00ff,    0x0},
    {"NetIO Interrupt Status",     
     0x12,      READ_ONLY,  {2}, 0x00ff,    0x0}, 
    {"Timer0-3 Interrupt Status",   
     0x14,      READ_ONLY,  {2}, 0x000f,    0x0}, 
    {"Timer16-19 Interrupt Status", 
     0x16,      READ_ONLY,  {2}, 0x000f,    0x0},
    {"RAM Parity Interrupt Status", 
     0x18,      READ_ONLY,  {2}, 0x007f,    0x0},
    {"MGMT Interrupt Enable",
     0x20,      READ_ONLY,  {2}, 0x00fb,    0x0},
    {"NetIO Interrupt Enable ",     
     0x22,      READ_ONLY,  {2}, 0x00ff,    0x0},
    {"Timer0-3 Interrupt Enable",   
     0x24,      READ_ONLY,  {2}, 0x000f,    0x0},
    {"Timer16-19 Interrupt Enable", 
     0x26,      READ_ONLY,  {2}, 0x000f,    0x0},
    {"RAM Parity Interrupt Enable", 
     0x28,      READ_ONLY,  {2}, 0x007f,    0x0},
    {"RAM Parity Error Insertion",  
     0x2a,      READ_WRITE, {2}, 0xc07f,    0x0},
    {"Timer 0-1 Control",           
     0x30,      READ_WRITE, {2}, 0x7f7f,    0x0},
    {"Timer 2-3 Control",           
     0x32,      READ_WRITE, {2}, 0x7f7f,    0x0},
    {"Timer 16 Control",            
     0x40,      READ_WRITE, {2}, 0xffff,    0x0},
    {"Timer 17 Control",            
     0x42,      READ_WRITE, {2}, 0xffff,    0x0},
    {"Timer 18 Control",           
     0x44,      READ_WRITE, {2}, 0xffff,    0x0},
    {"Timer 19 Control",            
     0x46,      READ_WRITE, {2}, 0xffff,    0x0},
    {"Bisync Text Timeout Upper",   
     0x48,      READ_WRITE, {2}, 0x0fff,    0x0},
    {"Bisync Text Timeout Lower",   
     0x4a,      READ_WRITE, {2}, 0xffff,    0x0},
    {"Bisync Long Timeout Upper",   
     0x4c,      READ_WRITE, {2}, 0x0fff,    0x0},
    {"Bisync Long Timeout Lower",   
     0x4e,      READ_WRITE, {2}, 0xffff,    0x0},
    {"Bisync Timeout Enable",       
     0x50,      READ_WRITE, {2}, 0x0007,    0x0},
    
    {"Channel 0 Mode",             
     0x100,     READ_WRITE, {4}, 0x1ffdff33, 0x10019030},
    {"Channel 0 Flag Config",      
     0x104,     READ_ONLY,  {2}, 0x7fff,    0x0},
    {"Channel 0 Flag Control",     
     0x108,     READ_ONLY,  {2}, 0x7f00,    0x0},
    {"Channel 0 Interrupt Status", 
     0x10a,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 0 Interrupt Enable", 
     0x10c,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 0 Interrupt Command/Status", 
     0x10e,     READ_ONLY,  {2}, 0x80ff,    0x0},
    
    {"Channel 1 Mode",             
     0x110,     READ_WRITE, {4}, 0x1ffdff33, 0x10019030},
    {"Channel 1 Flag Config",      
     0x114,     READ_ONLY,  {2}, 0x7fff,    0x0},
    {"Channel 1 Flag Control",     
     0x118,     READ_ONLY,  {2}, 0x7f00,    0x0},
    {"Channel 1 Interrupt Status", 
     0x11a,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 1 Interrupt Enable", 
     0x11c,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 1 Interrupt Command/Status", 
     0x11e,     READ_ONLY,  {2}, 0x80ff,    0x0},

    {"Channel 2 Mode",             
     0x120,     READ_WRITE, {4}, 0x1ffdff33, 0x10019030},
    {"Channel 2 Flag Config",      
     0x124,     READ_ONLY,  {2}, 0x7fff,    0x0},
    {"Channel 2 Flag Control",     
     0x128,     READ_ONLY,  {2}, 0x7f00,    0x0},
    {"Channel 2 Interrupt Status", 
     0x12a,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 2 Interrupt Enable", 
     0x12c,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 2 Interrupt Command/Status", 
     0x12e,     READ_ONLY,  {2}, 0x80ff,    0x0},

    {"Channel 3 Mode",             
     0x130,     READ_WRITE, {4}, 0x1ffdff33, 0x10019030},
    {"Channel 3 Flag Config",      
     0x134,     READ_ONLY,  {2}, 0x7fff,    0x0},
    {"Channel 3 Flag Control",     
     0x138,     READ_ONLY,  {2}, 0x7f00,    0x0},
    {"Channel 3 Interrupt Status", 
     0x13a,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 3 Interrupt Enable", 
     0x13c,     READ_ONLY,  {2}, 0xefff,    0x0},
    {"Channel 3 Interrupt Command/Status", 
     0x13e,     READ_ONLY,  {2}, 0x80ff,    0x0},

    {"END",  0x000,  0,     {0}, 0x0,       0x0},
};

static reg_info_t scc_rx_dma_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Channel 0 Rx DMA Ring Start Pointer",
     0x00,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 0 Rx DMA Ring Size Mask & Ring Index",
     0x04,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 1 Rx DMA Ring Start Pointer", 
     0x20,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 1 Rx DMA Ring Size Mask & Ring Index",
     0x24,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 2 Rx DMA Ring Start Pointer",
     0x40,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 2 Rx DMA Ring Size Mask & Ring Index",
     0x44,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 3 Rx DMA Ring Start Pointer",
     0x60,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 3 Rx DMA Ring Size Mask & Ring Index",
     0x64,      READ_WRITE, {4},    0xffffffff, 0x0},

    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

static reg_info_t scc_tx_dma_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,     Size,     Mask,       Reset Value */
    {"Channel 0 Tx DMA Ring Start Pointer - HP", 
     0x00,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 0 Tx DMA Ring Size Mask & Ring Index - HP",
     0x04,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 1 Tx DMA Ring Start Pointer - HP",
     0x20,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 1 Tx DMA Ring Size Mask & Ring Index - HP",
     0x24,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 2 Tx DMA Ring Start Pointer - HP",
     0x40,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 2 Tx DMA Ring Size Mask & Ring Index - HP",
     0x44,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 3 Tx DMA Ring Start Pointer - HP",
     0x60,      READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 3 Tx DMA Ring Size Mask & Ring Index - HP",
     0x64,      READ_WRITE, {4},    0xffffffff, 0x0},

    {"Channel 0 Tx DMA Ring Start Pointer - LP",
     0x100,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 0 Tx DMA Ring Size Mask & Ring Index - LP",
     0x104,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 1 Tx DMA Ring Start Pointer - LP",
     0x120,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 1 Tx DMA Ring Size Mask & Ring Index - LP",
     0x124,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 2 Tx DMA Ring Start Pointer - LP",
     0x140,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 2 Tx DMA Ring Size Mask & Ring Index - LP",
     0x144,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 3 Tx DMA Ring Start Pointer - LP",
     0x160,     READ_WRITE, {4},    0xffffffff, 0x0},
    {"Channel 3 Tx DMA Ring Size Mask & Ring Index - LP",
     0x164,     READ_WRITE, {4},    0xffffffff, 0x0},

    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

static reg_info_t scc_bisync_tx_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Channel 0 TX Pad char and Context Storage",
    0x00,       READ_WRITE, {4},    0xffff0000, 0x0},
    {"Channel 1 TX Pad char and Context Storage",
    0x10,       READ_WRITE, {4},    0xffff0000, 0x0},
    {"Channel 2 TX Pad char and Context Storage", 
    0x20,       READ_WRITE, {4},    0xffff0000, 0x0},
    {"Channel 3 TX Pad char and Context Storage",
    0x30,       READ_WRITE, {4},    0xffff0000, 0x0},
 
    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

static reg_info_t scc_bisync_rx_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Channel 0 Rx Pad char and Context Storage",
    0x00,       READ_WRITE, {4},    0xffff0000, 0x0},
    {"Channel 1 RX Pad char and Context Storage",
    0x10,       READ_WRITE, {4},    0xffff0000, 0x0},
    {"Channel 2 RX Pad char and Context Storage", 
    0x20,       READ_WRITE, {4},    0xffff0000, 0x0},
    {"Channel 3 RX Pad char and Context Storage",
    0x30,       READ_WRITE, {4},    0xffff0000, 0x0},
 
    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

static reg_info_t scc_iface_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Serial Interface Channel 0 Control",
    0x00,       READ_WRITE, {2},    0x3e00,     0x0204},
    {"Channel 0 Modem Control",
    0x02,       READ_WRITE, {2},    0x01ea,     0xe010},
    {"Channel 0 Serial Interface Flow Control", 
    0x04,       READ_WRITE, {2},    0xf70f,     0x0},
    {"Channel 0 Serial Port Baud Rate Generator Divide",
    0x06,       READ_WRITE, {2},    0x03ff,     0x0},
    {"Channel 0 Serial Interface Modem Interrupt Status",
    0x08,       READ_WRITE, {2},    0x00f0,     0x0},
    
    {"Serial Interface Channel 1 Control",
    0x10,       READ_WRITE, {2},    0x3e00,     0x0204},
    {"Channel 1 Modem Control",
    0x12,       READ_WRITE, {2},    0x01ea,     0xe010},
    {"Channel 1 Serial Interface Flow Control", 
    0x14,       READ_WRITE, {2},    0xf70f,     0x0},
    {"Channel 1 Serial Port Baud Rate Generator Divide",
    0x16,       READ_WRITE, {2},    0x03ff,     0x0},
    {"Channel 1 Serial Interface Modem Interrupt Status",
    0x18,       READ_WRITE, {2},    0x00f0,     0x0},

    {"Serial Interface Channel 2 Control",
    0x20,       READ_WRITE, {2},    0x3e00,     0x0204},
    {"Channel 2 Modem Control",
    0x22,       READ_WRITE, {2},    0x01ea,     0xe010},
    {"Channel 2 Serial Interface Flow Control", 
    0x24,       READ_WRITE, {2},    0xf70f,     0x0},
    {"Channel 2 Serial Port Baud Rate Generator Divide",
    0x26,       READ_WRITE, {2},    0x03ff,     0x0},
    {"Channel 2 Serial Interface Modem Interrupt Status",
    0x28,       READ_WRITE, {2},    0x00f0,     0x0},
    
    {"Serial Interface Channel 3 Control",
    0x30,       READ_WRITE, {2},    0x3e00,     0x0204},
    {"Channel 3 Modem Control",
    0x32,       READ_WRITE, {2},    0x01ea,     0xe010},
    {"Channel 3 Serial Interface Flow Control", 
    0x34,       READ_WRITE, {2},    0xf70f,     0x0},
    {"Channel 3 Serial Port Baud Rate Generator Divide",
    0x36,       READ_WRITE, {2},    0x03ff,     0x0},
    {"Channel 3 Serial Interface Modem Interrupt Status",
    0x38,       READ_WRITE, {2},    0x00f0,     0x0},

    {"Serial Interface IRQ Status ",
    0x106,      READ_ONLY,  {2},    0x00ff,     0x0},
    {"Frequency Counter Select", 
    0x10a,      READ_WRITE, {2},    0x000f,     0x0},
    {"Frequency Counter",
    0x10c,      READ_ONLY,  {4},    0x00ffffff, 0x0},
 
    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

 /* =========================================
  *  Serial Channel Controller menu
  * ========================================= */

static struct menuinfo scc_menu = {
    "Serial Channel %s ",       /* title */
    (int)0 ,                    /* title param */
    (PFT)menu_show_dflags,      /* show diag flags */
    0,                          /* generic prompt */
    0,                          /* size of menu */
    scc_menu_items,
};

static struct menuinfo *scc_menup = &scc_menu;

/* common item menu */
static mitem_t scc_common_menu_items[] = {
    {"Register test", 0, 0, 
    (PFT)scc_reg_test, (type_t *)0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT, (PFT)scc_reg_test, 0},
    {"Interrupt test", 0, 0, 
    (PFT)scc_intr_test, (type_t *)0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT, (PFT)scc_intr_test, 0},
};
#define NUM_COMMON_TESTS sizeof(scc_common_menu_items)/sizeof(mitem_t)
static title_buf_t  common_menu_title[NUM_COMMON_TESTS];

static int scc_ctrl_reg_test()
{
    ulong scc_ctrl_base = get_scc_base() + 
                          ZYNC_SCC_CTRL_OFFSET;
    prpass(testpass, "SCC control registers test.");
    return (register_tests(scc_ctrl_base, 
                          &scc_ctrl_reg_table[0]));
}

static int scc_rx_dma_ctrl_reg_test()
{
    ulong scc_rx_dma_ctrl_base = get_scc_base() + 
                                 ZYNC_SCC_RX_DMA_CTRL_OFFSET;
    prpass(testpass, "SCC RX DMA control registers test.");
    return (register_tests(scc_rx_dma_ctrl_base, 
                          &scc_rx_dma_ctrl_reg_table[0]));
}

static int scc_tx_dma_ctrl_reg_test()
{
    ulong scc_tx_dma_ctrl_base = get_scc_base() + 
                                 ZYNC_SCC_TX_DMA_CTRL_OFFSET;
    prpass(testpass, "SCC TX DMA control registers test.");
    return (register_tests(scc_tx_dma_ctrl_base, 
                          &scc_tx_dma_ctrl_reg_table[0]));
}

static int scc_bisync_tx_ctrl_reg_test()
{
    ulong scc_bisync_tx_ctrl_base = get_scc_base() + 
                                    ZYNC_SCC_BISYNC_TX_CTRL_OFFSET;
    prpass(testpass, "SCC Bisync Tx control registers test.");
    return (register_tests(scc_bisync_tx_ctrl_base, 
                          &scc_bisync_tx_ctrl_reg_table[0]));
}

static int scc_bisync_rx_ctrl_reg_test()
{
    ulong scc_bisync_rx_ctrl_base = get_scc_base() + 
                                    ZYNC_SCC_BISYNC_RX_CTRL_OFFSET;
    prpass(testpass, "SCC Bisync Rx control registers test.");
    return (register_tests(scc_bisync_rx_ctrl_base, 
                          &scc_bisync_rx_ctrl_reg_table[0]));
}

static int scc_iface_ctrl_reg_test()
{
    ulong scc_iface_ctrl_base = get_scc_base() + 
                                ZYNC_SCC_IFACE_CTRL_OFFSET;
    prpass(testpass, "SCC Interface control registers test.");
    return (register_tests(scc_iface_ctrl_base, 
                          &scc_iface_ctrl_reg_table[0]));
}

static int scc_reg_test(int dummy)
{
    dummy;

    testname("SCC Register");

    if ((scc_ctrl_reg_test() != PASSED) ||
        (scc_rx_dma_ctrl_reg_test() != PASSED) ||
        (scc_tx_dma_ctrl_reg_test() != PASSED) ||
        (scc_bisync_rx_ctrl_reg_test() != PASSED) ||
        (scc_bisync_tx_ctrl_reg_test() != PASSED) ||
        (scc_iface_ctrl_reg_test() != PASSED)) {
        cterr('f', 0, "SCC register test failed.");
        return (FAILED);
    }

    prpass(testpass, "SCC Register test passed");
    return (PASSED);
}

int check_scc_reg_offset (ushort offset)
{
    if (offset >= ZYNC_SCC_IFACE_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_IFACE_CTRL_OFFSET, 
                             scc_iface_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_RX_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_BISYNC_RX_CTRL_OFFSET, 
                             scc_bisync_rx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_TX_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_BISYNC_TX_CTRL_OFFSET, 
                             scc_bisync_tx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_TX_DMA_CTRL_OFFSET) {
       return (check_offset(offset - ZYNC_SCC_TX_DMA_CTRL_OFFSET, 
                             scc_tx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_RX_DMA_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_RX_DMA_CTRL_OFFSET, 
                             scc_rx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_CTRL_OFFSET, 
                             scc_ctrl_reg_table));
    }

    return (FAILED);
}

int get_scc_reg_size (ushort offset)
{
    if (offset >= ZYNC_SCC_IFACE_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_IFACE_CTRL_OFFSET, 
                             scc_iface_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_RX_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_BISYNC_RX_CTRL_OFFSET, 
                             scc_bisync_rx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_TX_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_BISYNC_TX_CTRL_OFFSET, 
                             scc_bisync_tx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_TX_DMA_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_TX_DMA_CTRL_OFFSET, 
                             scc_tx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_RX_DMA_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_RX_DMA_CTRL_OFFSET, 
                             scc_rx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_CTRL_OFFSET, 
                             scc_ctrl_reg_table));
    }

    return (FAILED);
}

int scc_reg_rd ()
{
    ulong base_addr;
    ushort offset;

    base_addr = get_scc_base();

    offset = gethex_answer("\nEnter register offset[0x1000 to 0x310c]:",
               0x1000, 0x1000, 0x310c);

    if (check_scc_reg_offset(offset)) {
        printf("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        ulong size = get_scc_reg_size(offset);
        ulong *reg32_p;
        ushort *reg16_p;

        switch (size) {
        case 2:
            reg16_p = (ushort *)(base_addr + offset);
            printf("\n reg16_p is %#x.\n ", reg16_p);
            printf("\n register value @%#x = %#x ", (base_addr + offset), *reg16_p);
            return (PASSED);
        case 4:
            reg32_p = (ulong *)(base_addr + offset);
            printf("\n reg32_p is %#x.\n ", reg32_p);
            printf("\n register value @%#x = %#x ", (base_addr + offset), *reg32_p);
            return (PASSED);
        default:
            printf("Register size not supported.\n");
            return (FAILED);
        }
    }
}

int scc_reg_wr ()
{
    ulong base_addr;
    ushort offset;

    base_addr = get_scc_base();

    offset = gethex_answer("\nEnter register offset[0x1000 to 0x310c]:",
               0x1000, 0x1000, 0x310c);

    if (check_scc_reg_offset(offset)) {
        printf("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        ulong size = get_scc_reg_size(offset);
        ulong reg_data_32;
        ulong *reg32_p;
        ushort reg_data_16;
        ushort *reg16_p;

        switch (size) {
        case 2:
            reg_data_16 = gethex_answer("\nEnter write value[0x0 to 0xFFFF]:", 
                        0, 0, 0xffff);
            reg16_p = (ushort *)(base_addr + offset);
            *reg16_p = reg_data_16;
            printf("\n register value @%#x = %#x ", (base_addr + offset), *reg16_p);
            return (PASSED);
        case 4:
            reg_data_32 = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
                        0, 0, 0xffffffff);
            reg32_p = (ulong *)(base_addr + offset);
            *reg32_p = reg_data_32;
            printf("\n register value @%#x = %#x ", (base_addr + offset), *reg32_p);
            return (PASSED);
        default:
            printf("Register size not supported.\n");
            return (FAILED);
        }
    }
}

int scc_reg_dp()
{
    ulong base_addr = get_scc_base() + ZYNC_SCC_CTRL_OFFSET;
    reg_info_t *reg_table_p = &scc_ctrl_reg_table[0];

    printf("SCC Register Dump");

    reg_dump(base_addr, reg_table_p);

    base_addr = get_scc_base() + ZYNC_SCC_RX_DMA_CTRL_OFFSET;
    reg_table_p = &scc_rx_dma_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    base_addr = get_scc_base() + ZYNC_SCC_TX_DMA_CTRL_OFFSET;
    reg_table_p = &scc_tx_dma_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    base_addr = get_scc_base() + ZYNC_SCC_BISYNC_TX_CTRL_OFFSET;
    reg_table_p = &scc_bisync_tx_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    base_addr = get_scc_base() + ZYNC_SCC_BISYNC_RX_CTRL_OFFSET;
    reg_table_p = &scc_bisync_rx_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    base_addr = get_scc_base() + ZYNC_SCC_IFACE_CTRL_OFFSET;
    reg_table_p = &scc_iface_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    return PASSED;
}

/*-----------------------------------------------------------------------------
 *
 * Function: mgmt_intr_test_one
 *
 * This function will test modem interrupt for one setting.
 *
 * Input:  chan - channel number
 *         type - type of interrupt
 *
 * Output: PASSED FAILED
 */
static int 
mgmt_intr_test_one (int chan, ushort type)
{
    prince_scc_regs_t * base_addr;
    prince_serial_itf_t *serial_reg; /*flow_cntrl  modem_intr_status */
    ushort assert = 0;
    ushort data;
    int rc = PASSED;
    int i;

    prpass(testpass, "mgmt intr chan[%d] type[%x]", chan, type);

    base_addr = (prince_scc_regs_t *)get_scc_base();
    serial_reg = &base_addr->serial_itf[chan];

    /* do a dummy read, clear pending interrupt */
    data = serial_reg->modem_intr_status;

    /* enable mgmt interrupt */
    base_addr->cntrl_1_regs.int_1_intr_mask |= PRINCE_MODEM_ENB;
    if (diagflag_xram & D_TRACE) {
        printf("\nEnable mgmt interrupt: "
            "cntrl_1_regs.int_1_intr_mask @%#x = %#x\n",
            &base_addr->cntrl_1_regs.int_1_intr_mask, 
            base_addr->cntrl_1_regs.int_1_intr_mask);
    }

    /* enable interrupt*/
    switch (type) {
    case PRINCE_DCD_FORCE_IRQ:
        serial_reg->flow_cntrl |= PRINCE_DCD_ENABLE;
        assert = PRINCE_DCD_STATUS;
        break;
    case PRINCE_DSR_FORCE_IRQ:
        serial_reg->flow_cntrl |= PRINCE_DSR_ENABLE;
        assert = PRINCE_DSR_STATUS;
        break;
    case PRINCE_CTS_FORCE_IRQ:
        serial_reg->flow_cntrl |= PRINCE_CTS_ENABLE;
        assert = PRINCE_CTS_STATUS;
        break;
    case PRINCE_CABLEID_FORCE_IRQ:
        serial_reg->flow_cntrl |= PRINCE_CABLEID_ENABLE;
        assert = PRINCE_CABLEID_STATUS;
        break;
    default:
        cterr('f', 0, "Incorrect force type");
        return (FAILED);
    }

    if (diagflag_xram & D_TRACE) {
        printf("Enable interrupt: serial_reg->flow_cntrl @%#x = %#x\n", 
            &serial_reg->flow_cntrl, serial_reg-> flow_cntrl);
    }

    serial_reg->modem_intr_status |= type;   /* fire up force type */

    /* Wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
        if ((base_addr->cntrl_1_regs.int_1_intr_status & PRINCE_MODEM_STATUS) == 0) {
            break;
        } else {
            usleep(100);
        }
    }

    if (i == 5000) {
        cterr('f', 0, 
              "Timeout waiting for interrupt to be cleared. "
              "intr status = %#x\n", 
              serial_reg->modem_intr_status);
        rc = FAILED;
    } else {
        rc = PASSED;
    }

    serial_reg->modem_intr_status &= ~type;
    serial_reg->flow_cntrl &= ~(PRINCE_DCD_ENABLE | PRINCE_DSR_ENABLE |
                              PRINCE_CTS_ENABLE | PRINCE_CABLEID_ENABLE);
    base_addr->cntrl_1_regs.int_1_intr_mask &= ~(PRINCE_MODEM_ENB);

    return (rc);
}

/*-----------------------------------------------------------------------------
 *
 * Function: net_intr_test_one
 *
 * This function will test DMA network interrupt for one setting.
 *
 * Input:  chan - channel number
 *         type - type of interrupt
 *
 * Output: PASSED FAILED
 */
/* Data Structure for channel loopback test */
prince_serial_ds_t prince_serial_ds[MAX_CH_NUM];

static int 
net_intr_test_one (int chan, ushort type, int priority)
{
    prince_scc_regs_t * base_addr;
    prince_proto_regs_t *proto_reg_hp1;
    prince_proto_regs_hp2_t *proto_reg_hp2;
    prince_proto_regs_lp_t *proto_reg_lp;
    prince_serial_ds_t *s_ds;
    ushort assert = 0x0;
    ushort data;
    ushort chan_mask = 0;
    ushort int1_status_save, int2_status_save;
    int rc = PASSED;
    int i;
    int priority_tmp = priority;

    base_addr = (prince_scc_regs_t *)get_scc_base();
    proto_reg_hp1 = &base_addr->proto_regs_hp1[chan];
    proto_reg_hp2 = &base_addr->proto_regs_hp2[chan];
    proto_reg_lp = &base_addr->proto_regs_lp[chan];

    /* The old FPGA LP mapping to new FPGA HP2 register */
    if (fpga_ver < NEW_FPGA_VERSION) {
        switch (priority_tmp) {
        case PRINCE_SCC_LOW_PRIORITY:
            priority_tmp = PRINCE_SCC_HIGH_PRIORITY_2;
            break;
        default:
            break;
        }
    }

    /* test net irq2 */
    prpass(testpass, "net irq2 intr chan[%d] type[%x]", chan, type);
    if (priority_tmp == PRINCE_SCC_HIGH_PRIORITY_1 || 
        type == PRINCE_RX_BUF_TYPE ||
        type == PRINCE_RX_FRAME_TYPE) {
        chan_mask = 1 << chan;
    } else if (priority_tmp == PRINCE_SCC_HIGH_PRIORITY_2) {
        chan_mask = 1 << (chan + 4);
    } else { 
        chan_mask = 1 << (chan + 8);
    }

    printf("chan_mask = %#x \t", chan_mask);

    base_addr->cntrl_1_regs.int_2_intr_mask = chan_mask;
    if (diagflag_xram & D_TRACE) {
        printf("Enable channel %d interrupt: "
            "cntrl_1_regs.int_2_intr_mask @%#x = %#x\n", chan,
            &base_addr->cntrl_1_regs.int_2_intr_mask, base_addr->cntrl_1_regs.int_2_intr_mask);
    }

    s_ds = &prince_serial_ds[chan];
    prince_init_serial_ds(s_ds);
    s_ds->port_num = chan;
    s_ds->org_port_num = chan;

    /* enable interrupt at prince level and run loopback */
    switch (type) {
    case PRINCE_TX_BUF_TYPE:
        switch (priority_tmp) {
        case PRINCE_SCC_HIGH_PRIORITY_1:
            proto_reg_hp1->intr_mask |= PRINCE_DMA_TX_BUF_ENB ;
            break;
        case PRINCE_SCC_HIGH_PRIORITY_2:
            proto_reg_hp2->intr_mask |= PRINCE_DMA_TX_BUF_ENB ;
            break;
        case PRINCE_SCC_LOW_PRIORITY:
            proto_reg_lp->intr_mask |= PRINCE_DMA_TX_BUF_ENB ;
            break;
        default:
            break;
        }
        assert = PRINCE_DMA_TX_BUF_STATUS;
        if (prince_chan_lpbk_test(PRINCE_INT_MODE, (ulong)base_addr,
            PRINCE_SYNC_BPS_128K, PRINCE_CLK_SRC_SYNC, 
            PRINCE_SCC_INT_LOOPBACK, PRINCE_HDLC, 
            s_ds, priority))
            rc = FAILED;
        break;

    case PRINCE_RX_BUF_TYPE:
        proto_reg_hp1->intr_mask |= PRINCE_DMA_RX_BUF_ENB ;
        assert = PRINCE_DMA_RX_BUF_STATUS;
        if (prince_chan_lpbk_test(PRINCE_INT_MODE, (ulong)base_addr,
            PRINCE_SYNC_BPS_128K, PRINCE_CLK_SRC_SYNC, 
            PRINCE_SCC_EXT_LOOPBACK, PRINCE_HDLC,
            s_ds, priority))
            rc = FAILED;
        break;

    case PRINCE_TX_FRAME_TYPE:
        switch (priority) {
        case PRINCE_SCC_HIGH_PRIORITY_1:
            proto_reg_hp1->intr_mask |= PRINCE_DMA_TX_FRAME_ENB ;
            break;
        case PRINCE_SCC_HIGH_PRIORITY_2:
            proto_reg_hp2->intr_mask |= PRINCE_DMA_TX_FRAME_ENB ;
            break;
        case PRINCE_SCC_LOW_PRIORITY:
            proto_reg_lp->intr_mask |= PRINCE_DMA_TX_FRAME_ENB ;
            break;
        default:
            break;
        }
        assert = PRINCE_DMA_TX_FRAME_STATUS;
        if (prince_chan_lpbk_test(PRINCE_FRAME_INT_MODE, (ulong)base_addr,
            PRINCE_SYNC_BPS_128K, PRINCE_CLK_SRC_SYNC, 
            PRINCE_SCC_INT_LOOPBACK, PRINCE_HDLC, 
            s_ds, priority))
            rc = FAILED;
        break;
    case PRINCE_RX_FRAME_TYPE:
        proto_reg_hp1->intr_mask |= PRINCE_DMA_RX_FRAME_ENB ;
        assert = PRINCE_DMA_RX_FRAME_STATUS;
        if (prince_chan_lpbk_test(PRINCE_FRAME_INT_MODE, (ulong)base_addr,
            PRINCE_SYNC_BPS_128K, PRINCE_CLK_SRC_SYNC, 
            PRINCE_SCC_EXT_LOOPBACK, PRINCE_HDLC,
            s_ds, priority))
            rc = FAILED;
        break;
    default:
        break;
    }

    /* Wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
        if ((base_addr->cntrl_1_regs.int_2_intr_status && chan_mask) == 0) {
            break;
        } else {
            usleep(100);
        }
    }

    if (i == 5000) {
        cterr('f', 0, 
              "Timeout waiting for interrupt to be cleared. "
              "intr status = %#x\n", 
              base_addr->cntrl_1_regs.int_2_intr_status);
        get_gic_spi_status1();
        rc = FAILED;
    }/* else {
        rc = PASSED;
    }*/

    base_addr->cntrl_1_regs.int_1_intr_mask &= ~(PRINCE_DMA_ENB);
    base_addr->cntrl_1_regs.int_2_intr_mask &= ~chan_mask;
    proto_reg_hp1->intr_mask &= ~(PRINCE_DMA_TX_BUF_ENB | PRINCE_DMA_RX_BUF_ENB |
                            PRINCE_DMA_TX_FRAME_ENB |
                            PRINCE_DMA_RX_FRAME_ENB);
    proto_reg_hp2->intr_mask &= ~(PRINCE_DMA_TX_BUF_ENB |
                            PRINCE_DMA_TX_FRAME_ENB);
    proto_reg_lp->intr_mask &= ~(PRINCE_DMA_TX_BUF_ENB |
                            PRINCE_DMA_TX_FRAME_ENB);

    /* disable dma */
    prince_cleanup_mode_bits(s_ds);

    return (rc);
}

static int scc_intr_test(int dummy)
{
    int chan;
    dummy;

    testname("SCC Interrupt");

    /* test modem management IRQ */
    for (chan = 0; chan < num_chan; chan++) {
        /*for multi*/
        msleep(10);

        if (mgmt_intr_test_one(chan, PRINCE_CTS_FORCE_IRQ))
            return (FAILED);
        if (mgmt_intr_test_one(chan, PRINCE_DCD_FORCE_IRQ))
            return (FAILED);
        if (mgmt_intr_test_one(chan, PRINCE_DSR_FORCE_IRQ))
            return (FAILED);
    }

    /* test network IRQ */
    for (chan = 0; chan < num_chan; chan++) {
        /*for multi*/
        msleep(10);

        if (net_intr_test_one(chan, PRINCE_TX_FRAME_TYPE, PRINCE_SCC_HIGH_PRIORITY_1))
            return (FAILED);
        if (net_intr_test_one(chan, PRINCE_TX_BUF_TYPE, PRINCE_SCC_HIGH_PRIORITY_1))
            return (FAILED);
        if (fpga_ver >= NEW_FPGA_VERSION) {
            if (net_intr_test_one(chan, PRINCE_TX_FRAME_TYPE, PRINCE_SCC_HIGH_PRIORITY_2))
                return (FAILED);
            if (net_intr_test_one(chan, PRINCE_TX_BUF_TYPE, PRINCE_SCC_HIGH_PRIORITY_2))
                return (FAILED);
        }
        if (net_intr_test_one(chan, PRINCE_TX_FRAME_TYPE, PRINCE_SCC_LOW_PRIORITY))
            return (FAILED);
        if (net_intr_test_one(chan, PRINCE_TX_BUF_TYPE, PRINCE_SCC_LOW_PRIORITY))
            return (FAILED);

        if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
            if (net_intr_test_one(chan, PRINCE_RX_FRAME_TYPE, PRINCE_SCC_HIGH_PRIORITY_1))
                return (FAILED);
            if (net_intr_test_one(chan, PRINCE_RX_BUF_TYPE, PRINCE_SCC_HIGH_PRIORITY_1))
                return (FAILED);
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (net_intr_test_one(chan, PRINCE_RX_FRAME_TYPE, PRINCE_SCC_HIGH_PRIORITY_2))
                    return (FAILED);
                if (net_intr_test_one(chan, PRINCE_RX_BUF_TYPE, PRINCE_SCC_HIGH_PRIORITY_2))
                    return (FAILED);
            }
            if (net_intr_test_one(chan, PRINCE_RX_FRAME_TYPE, PRINCE_SCC_LOW_PRIORITY))
                return (FAILED);
            if (net_intr_test_one(chan, PRINCE_RX_BUF_TYPE, PRINCE_SCC_LOW_PRIORITY))
                return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: build_scc_sub_menu
 *
 * Description: Build Serial Channel Test menu.
 *
 * Input :  Number of Channels
 *
 * Outputs: None.
 *
 **********************************************************************
 */
static void
build_scc_sub_menu ()
{
    int i;

    /* Init base menu */
    init_base_submenu(&scc_menup, (int)scc_menu_header.title);

    /* Add common menu items */
    for ( i = 0; i < NUM_COMMON_TESTS; i++ ) {
        sprintf(common_menu_title[i].title, "%s",
                scc_common_menu_items[i].mline);
        add_menu_item(&scc_menu, common_menu_title[i].title,
                        scc_common_menu_items[i].mfunc,
                        /*scc_common_menu_items[i].mfparam*/(type_t *)&i, 
                        scc_common_menu_items[i].mflag);
    }

    /* Add channel test menu items */ 
    for ( i = 0; i < num_chan; i++ ) {
        sprintf(scc_menu_title[i].title, "%s %d",
            "test channel", i);
        scc_menu_index[i] = i;
        add_menu_item(&scc_menu, scc_menu_title[i].title,
            (PFT)ngwic_prince_chan_lpbk_test, (type_t *)&scc_menu_index[i],
            MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT | MF_MULTI);
    }
}

/**********************************************************************
 *
 * Function: scc_sub_test
 *
 * This function invokes the sub diagnostics tests for SCC
 *
 * Input : Number of Channels
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
static int 
scc_sub_test()
{
    int  i;

    /* Run common tests - skip util from table */
    for (i = 0; i < NUM_COMMON_TESTS; i++) {
        if (scc_common_menu_items[i].mflag & MF_DOALL) {
            prpass(testpass, "%s,", scc_menu_title[i].title);
            if ((scc_common_menu_items[i].mfunc)(scc_common_menu_items[i].mfparam)) {
                return (FAILED);
            }
        }
    }

    /* Run channel tests */
    for ( i = 0; i < num_chan; i++ ) {
        prpass(testpass, "test channel %d,", i);
        if (ngwic_prince_chan_lpbk_test(i)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: serial_channel_test
 *
 * Description: Serial Channel Test.
 *
 * Inputs:  show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED or FAILED
 *
 **********************************************************************
 */
int serial_channel_test(int show_menu)
{
    int rc;
    int board_id;

    testname("SCC");

    board_id = get_board_id();
    /* 
     * based on pid, build the test, menu string
     */
    switch(board_id) {
    case BOARD_ID_NIM_1T:
        num_chan = 1;
        sprintf(scc_menu_header.title, "NIM-1T");
        break;
    case BOARD_ID_NIM_2T:
        num_chan = 2;
        sprintf(scc_menu_header.title, "NIM-2T");
        break;
    case BOARD_ID_NIM_4T:
        num_chan = 4;
        sprintf(scc_menu_header.title, "NIM-4T");
        break;
    default:
        cterr('f', 0, "Invalid prince ID 0x%X", board_id);
        return (FAILED);
    }

    build_scc_sub_menu();

    if (show_menu) {
        /* Entered submenu */
        menu(&scc_menu, (mitem_t *)0, 0);
        rc = PASSED;
    } else {
        /* Invoked the test */
        rc = scc_sub_test();
    }

    return rc;
}

int get_scc_channel_num()
{
    return num_chan;
}

/******** History ******** 
$Log: prince_scc.c,v $
Revision 1.4  2017/07/18 08:48:41  iachang
Prince FPGA Enhanced Feature, Support HP1, HP2, and LP.

Revision 1.3  2013/07/30 10:08:24  xiaoyizh
Modify some printf to prpass.

Revision 1.2  2013/06/25 08:15:41  xiaoyizh
Add RX frame/buffer interrupt tests for low-priority drings.

Revision 1.1  2013/04/19 07:17:52  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/
