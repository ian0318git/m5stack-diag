/* $Id: reva_scc.c,v 1.5 2019/09/23 10:04:26 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/reva_scc.c,v $
 *------------------------------------------------------------------
 *
 * reva_scc.c - Reva Serial Channel Controller main function/menu.
 *
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
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
#include "prince_reg.h"
#include "reva_def.h"
#include "reva_reg.h"
#include "prince_scc_struct.h"
#include "module_fru.h"

/*
 * External Functions
 */
extern int prince_scc_reg_test();
extern int prince_scc_intr_test();
extern int reva_check_scc_reg_offset (ushort);
extern ulong get_reva_scc_reg_size (ushort);
extern int reva_scc_reg_dp();
extern int reva_scc_intr_test(int);

/*
 * Global variables
 */
int as_num_chan = 0;

/*
 * Global Functions
 */
int get_as_scc_channel_num();

/*
 * Static definition
 */
static mitem_t      as_scc_menu_items[MAX_SUBTEST_ITEMS];
static title_buf_t  as_scc_menu_title[MAX_SUBTEST_ITEMS];
static int          as_scc_menu_index[MAX_SUBTEST_ITEMS];
static title_buf_t  as_scc_menu_header;

static int reva_all_reg_test(int);
static int reva_scc_reg_test(int);

/* Async SCC Register Tables */
static reg_info_t reva_scc_ctrl_reg_table[] =
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

static reg_info_t reva_scc_rx_dma_ctrl_reg_table[] =
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

static reg_info_t reva_scc_tx_dma_ctrl_reg_table[] =
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

static reg_info_t reva_scc_bisync_tx_ctrl_reg_table[] =
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

static reg_info_t reva_scc_bisync_rx_ctrl_reg_table[] =
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

static reg_info_t reva_scc_iface_ctrl_reg_table[] =
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

static reg_info_t reva_scc_ppp_tx_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"PPP Transmit Ch 0 Async Control Character Map",
    0x00,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Transmit Ch 0 Special Mapped Character",
    0x04,       READ_WRITE, {4},    0xffff0fff, 0x0},
    {"PPP Transmit Ch 1 Async Control Character Map",
    0x10,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Transmit Ch 1 Special Mapped Character",
    0x14,       READ_WRITE, {4},    0xffff0fff, 0x0},
    {"PPP Transmit Ch 2 Async Control Character Map",
    0x20,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Transmit Ch 2 Special Mapped Character",
    0x24,       READ_WRITE, {4},    0xffff0fff, 0x0},
    {"PPP Transmit Ch 3 Async Control Character Map",
    0x30,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Transmit Ch 3 Special Mapped Character",
    0x34,       READ_WRITE, {4},    0xffff0fff, 0x0},
    
    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

static reg_info_t reva_scc_ppp_rx_ctrl_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"PPP Receive Ch 0 Async Control Character Map",
    0x00,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Receive Ch 0 Special Mapped Character",
    0x04,       READ_WRITE, {4},    0xffff00ff, 0x0},
    {"PPP Receive Ch 1 Async Control Character Map",
    0x10,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Receive Ch 1 Special Mapped Character",
    0x14,       READ_WRITE, {4},    0xffff00ff, 0x0},
    {"PPP Receive Ch 2 Async Control Character Map",
    0x20,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Receive Ch 2 Special Mapped Character",
    0x24,       READ_WRITE, {4},    0xffff00ff, 0x0},
    {"PPP Receive Ch 3 Async Control Character Map",
    0x30,       READ_WRITE, {4},    0xffffffff, 0x0},
    {"PPP Receive Ch 3 Special Mapped Character",
    0x34,       READ_WRITE, {4},    0xffff00ff, 0x0},
 
    {"END",     0x000,  0,  {0},    0x0,        0x0},
};

/* =========================================
*  Async Serial Channel Controller menu
* ========================================= */

static struct menuinfo as_scc_menu = {
    "Async Serial Channel %s ",       /* title */
    (int)0 ,                    /* title param */
    (PFT)menu_show_dflags,      /* show diag flags */
    0,                          /* generic prompt */
    0,                          /* size of menu */
    as_scc_menu_items,
};

static struct menuinfo *as_scc_menup = &as_scc_menu;

/* common item menu */
static mitem_t as_scc_common_menu_items[] = {
    {"Register test", 0, 0, 
    (PFT)reva_all_reg_test, (type_t *)0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT, (PFT)reva_all_reg_test, 0},
};
#define AS_NUM_COMMON_TESTS sizeof(as_scc_common_menu_items)/sizeof(mitem_t)
static title_buf_t  as_common_menu_title[AS_NUM_COMMON_TESTS];

/**********************************************************************
 * Function: reva_scc_ctrl_reg_test
 *
 * Description: SCC control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_ctrl_reg_test(channel)
{
    ulong scc_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) +
                          ZYNC_SCC_CTRL_OFFSET;
    prpass(testpass, "SCC control registers test.");
    return (register_tests(scc_ctrl_base, 
                          &reva_scc_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_rx_dma_ctrl_reg_test
 *
 * Description: SCC RX DMA control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_rx_dma_ctrl_reg_test(channel)
{
    ulong scc_rx_dma_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                 ZYNC_SCC_RX_DMA_CTRL_OFFSET;
    prpass(testpass, "SCC RX DMA control registers test.");
    return (register_tests(scc_rx_dma_ctrl_base, 
                          &reva_scc_rx_dma_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_tx_dma_ctrl_reg_test
 *
 * Description: SCC TX DMA control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_tx_dma_ctrl_reg_test(channel)
{
    ulong scc_tx_dma_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                 ZYNC_SCC_TX_DMA_CTRL_OFFSET;
    prpass(testpass, "SCC TX DMA control registers test.");
    return (register_tests(scc_tx_dma_ctrl_base, 
                          &reva_scc_tx_dma_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_bisync_tx_ctrl_reg_test
 *
 * Description: SCC Bisync Tx control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_bisync_tx_ctrl_reg_test(channel)
{
    ulong scc_bisync_tx_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                    ZYNC_SCC_BISYNC_TX_CTRL_OFFSET;
    prpass(testpass, "SCC Bisync Tx control registers test.");
    return (register_tests(scc_bisync_tx_ctrl_base, 
                          &reva_scc_bisync_tx_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_bisync_rx_ctrl_reg_test
 *
 * Description: SCC Bisync Rx control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_bisync_rx_ctrl_reg_test(channel)
{
    ulong scc_bisync_rx_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                    ZYNC_SCC_BISYNC_RX_CTRL_OFFSET;
    prpass(testpass, "SCC Bisync Rx control registers test.");
    return (register_tests(scc_bisync_rx_ctrl_base, 
                          &reva_scc_bisync_rx_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_iface_ctrl_reg_test
 *
 * Description: SCC Interface control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_iface_ctrl_reg_test(channel)
{
    ulong scc_iface_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                ZYNC_SCC_IFACE_CTRL_OFFSET;
    prpass(testpass, "SCC Interface control registers test.");
    return (register_tests(scc_iface_ctrl_base, 
                          &reva_scc_iface_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_ppp_tx_ctrl_reg_test
 *
 * Description: Async SCC PPP Tx control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_ppp_tx_ctrl_reg_test(channel)
{
    ulong as_scc_ppp_tx_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                    ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET;
    prpass(testpass, "Async SCC PPP Tx control registers test.");
    return (register_tests(as_scc_ppp_tx_ctrl_base, 
                          &reva_scc_ppp_tx_ctrl_reg_table[0]));
}

/**********************************************************************
 * Function: reva_scc_ppp_rx_ctrl_reg_test
 *
 * Description: Async SCC PPP Rx control registers test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_scc_ppp_rx_ctrl_reg_test(channel)
{
    ulong as_scc_ppp_rx_ctrl_base = get_scc_base() + ((channel / 4) * 0x4000) + 
                                    ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET;
    prpass(testpass, "Async SCC PPP Rx control registers test.");
    return (register_tests(as_scc_ppp_rx_ctrl_base, 
                          &reva_scc_ppp_rx_ctrl_reg_table[0]));
}


/**********************************************************************
 * Function: reva_all_reg_test
 *
 * Description: Serial Channel Register Test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int reva_all_reg_test(int dummy)
{
    dummy;
    int  i;

    /* Run rev tests */
    for ( i = 0; i < as_num_chan; i++ ) {
        prpass(testpass, "test channel %d,", i);
        if (reva_scc_reg_test(i)) {
            return (FAILED);
        }
    }
    return (PASSED);
    
}

/**********************************************************************
 * Function: reva_scc_reg_test
 *
 * Description: Serial Channel Register Test for Reva module.
 *
 * Inputs:  Number of Channels
 * Outputs: PASSED or FAILED
 **********************************************************************/
static int reva_scc_reg_test(int channel)
{
    testname("Async SCC Register");

    if ((reva_scc_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_rx_dma_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_tx_dma_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_bisync_rx_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_bisync_tx_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_iface_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_ppp_tx_ctrl_reg_test(channel) != PASSED) ||
        (reva_scc_ppp_rx_ctrl_reg_test(channel) != PASSED)) {
            cterr('f', 0, "Async SCC register test failed.");
            return (FAILED);
    }

    prpass(testpass, "Async SCC Register test passed");
    return (PASSED);
}

/*****************************************************************************
 * Function: check_scc_reg_offset
 *
 * Description: validate register is in range by user specify offset
 *
 * Input:  offset
 * Output: PASSED / FAILED
 *****************************************************************************/
int check_scc_reg_offset (ushort offset)
{
    if (offset >= ZYNC_SCC_IFACE_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_IFACE_CTRL_OFFSET, 
                             reva_scc_iface_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_RX_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_BISYNC_RX_CTRL_OFFSET, 
                             reva_scc_bisync_rx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_TX_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_BISYNC_TX_CTRL_OFFSET, 
                             reva_scc_bisync_tx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_TX_DMA_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_TX_DMA_CTRL_OFFSET, 
                             reva_scc_tx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_RX_DMA_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_RX_DMA_CTRL_OFFSET, 
                             reva_scc_rx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_CTRL_OFFSET, 
                             reva_scc_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET, 
                             reva_scc_ppp_rx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET) {
        return (check_offset(offset - ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET, 
                             reva_scc_ppp_tx_ctrl_reg_table));
    }
    return (FAILED);
}

/*****************************************************************************
 * Function: get_scc_reg_size
 *
 * Description: get register size by user specify offset
 *
 * Input:  offset
 * Output: size of register
 *****************************************************************************/
int get_scc_reg_size (ushort offset)
{
    if (offset >= ZYNC_SCC_IFACE_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_IFACE_CTRL_OFFSET, 
                             reva_scc_iface_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_RX_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_BISYNC_RX_CTRL_OFFSET, 
                             reva_scc_bisync_rx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_BISYNC_TX_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_BISYNC_TX_CTRL_OFFSET, 
                             reva_scc_bisync_tx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_TX_DMA_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_TX_DMA_CTRL_OFFSET, 
                             reva_scc_tx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_RX_DMA_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_RX_DMA_CTRL_OFFSET, 
                             reva_scc_rx_dma_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_CTRL_OFFSET, 
                             reva_scc_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET, 
                             reva_scc_ppp_rx_ctrl_reg_table));
    } else if (offset >= ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET) {
        return (get_reg_size(offset - ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET, 
                             reva_scc_ppp_rx_ctrl_reg_table));
    }

    return (FAILED);
}

/*****************************************************************************
 * Function: scc_reg_rd
 *
 * Description: specify read register value.
 *
 * Input:  None
 * Output: PASSED/FAILED
 *****************************************************************************/
int scc_reg_rd ()
{
    ulong base_addr;
    uint offset, valid_offset;

    base_addr = get_scc_base();

    offset = gethex_answer("\nEnter register offset[0x1000 to 0x17fff]:",
               0x1000, 0x1000, 0x17fff);

    valid_offset = offset % 0x4000;
    if (check_scc_reg_offset(valid_offset)) {
        printf("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        ulong size = get_scc_reg_size(valid_offset);
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

/*****************************************************************************
 * Function: scc_reg_wr
 *
 * Description: specify change register value.
 *
 * Input:  None
 * Output: PASSED/FAILED
 *****************************************************************************/
int scc_reg_wr ()
{
    ulong base_addr;
    uint offset, valid_offset;

    base_addr = get_scc_base();

    offset = gethex_answer("\nEnter register offset[0x1000 to 0x17fff]:",
               0x1000, 0x1000, 0x17fff);

    valid_offset = offset % 0x4000;
    if (check_scc_reg_offset(valid_offset)) {
        printf("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        ulong size = get_scc_reg_size(valid_offset);
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

/*****************************************************************************
 * Function: reva_reg_dump
 *
 * Description: This function will dump scc registers.
 *
 * Input:  base address of SCC / point of reg_info / counter
 * Output: PASSED/FAILED
 *****************************************************************************/
void reva_reg_dump(ulong base_addr, reg_info_t* reg_table_p, uint reg_run)
{
    uchar *reg_p;
    int ch_num;
    char str1[10], str2[10], str3[10],context[128], all_str[128];
    char *found_str;

    for (; reg_table_p->size.size != 0; reg_table_p++) {
        reg_p = (uchar *)(base_addr + reg_table_p->offset);

        switch (reg_table_p->size.size) {
        case 1:
            /* Search the "Channel xx" and rename the channel number */
            if(found_str = strstr(reg_table_p->name, "Channel ")) {
                sscanf(found_str, "%s %d %[^\\0]", str1, &ch_num, context);
                sprintf(all_str, "%s %d %s" ,str1, (reg_run * 4 + ch_num), context);
                /* Recognize the string of "Serial Interface Channel 0 Control" */
                if(found_str - (reg_table_p->name) != 0) {
                    memset(context, 0, sizeof(context));
                    strncpy(context, reg_table_p->name, found_str - (reg_table_p->name));
                    strcat(context, all_str);                    
                    printf("\n %s @%#x = %#x ", context, 
                        (base_addr + reg_table_p->offset), *reg_p);
                } else {
                    printf("\n %s @%#x = %#x ", all_str, 
                        (base_addr + reg_table_p->offset), *reg_p);
                }
            } 
            else if(strstr(reg_table_p->name, "Ch ")) {
                sscanf(reg_table_p->name, "%s %s %s %d %[^\\0]", str1, str2, str3, &ch_num, context);
                sprintf(all_str, "%s %s %s %d %s" , str1, str2, str3, (reg_run * 4 + ch_num), context);                
                printf("\n %s @%#x = %#x ", all_str, 
                    (base_addr + reg_table_p->offset), *reg_p);
            }
            else {             
                printf("\n %s @%#x = %#x ", reg_table_p->name, 
                    (base_addr + reg_table_p->offset), *reg_p);
            }            
        break;
        case 2:
            /* Search the "Channel xx" and rename the channel number */            
            if(found_str = strstr(reg_table_p->name, "Channel ")) {
                sscanf(found_str, "%s %d %[^\\0]", str1, &ch_num, context);
                sprintf(all_str, "%s %d %s" ,str1, (reg_run * 4 + ch_num), context);
                /* Recognize the string of "Serial Interface Channel 0 Control" */
                if(found_str - (reg_table_p->name) != 0) {
                    memset(context, 0, sizeof(context));
                    strncpy(context, reg_table_p->name, found_str - (reg_table_p->name));
                    strcat(context, all_str);                    
                    printf("\n %s @%#x = %#x ", context, 
                        (base_addr + reg_table_p->offset), *(ushort *)reg_p);
                } else {
                    printf("\n %s @%#x = %#x ", all_str, 
                        (base_addr + reg_table_p->offset), *(ushort *)reg_p);
                }
            } 
            else if(strstr(reg_table_p->name, "Ch ")) {
                sscanf(reg_table_p->name, "%s %s %s %d %[^\\0]", str1, str2, str3, &ch_num, context);
                sprintf(all_str, "%s %s %s %d %s" , str1, str2, str3, (reg_run * 4 + ch_num), context);                
                printf("\n %s @%#x = %#x ", all_str, 
                    (base_addr + reg_table_p->offset), *(ushort *)reg_p);
            }            
            else {             
                printf("\n %s @%#x = %#x ", reg_table_p->name, 
                    (base_addr + reg_table_p->offset), *(ushort *)reg_p);
            }            
        break;
        case 4:
            /* Search the "Channel xx" and rename the channel number */            
            if(found_str = strstr(reg_table_p->name, "Channel ")) {
                sscanf(found_str, "%s %d %[^\\0]", str1, &ch_num, context);
                sprintf(all_str, "%s %d %s" ,str1, (reg_run * 4 + ch_num), context);
                /* Recognize the string of "Serial Interface Channel 0 Control" */
                if(found_str - (reg_table_p->name) != 0) {
                    memset(context, 0, sizeof(context));
                    strncpy(context, reg_table_p->name, found_str - (reg_table_p->name));
                    strcat(context, all_str);                    
                    printf("\n %s @%#x = %#x ", context, 
                        (base_addr + reg_table_p->offset), *(ulong *)reg_p);
                } else {
                    printf("\n %s @%#x = %#x ", all_str, 
                        (base_addr + reg_table_p->offset), *(ulong *)reg_p);
                }
            } 
            else if(strstr(reg_table_p->name, "Ch ")) {
                sscanf(reg_table_p->name, "%s %s %s %d %[^\\0]", str1, str2, str3, &ch_num, context);
                sprintf(all_str, "%s %s %s %d %s" , str1, str2, str3, (reg_run * 4 + ch_num), context);                
                printf("\n %s @%#x = %#x ", all_str, 
                    (base_addr + reg_table_p->offset), *(ulong *)reg_p);
            }           
            else {             
                printf("\n %s @%#x = %#x ", reg_table_p->name, 
                    (base_addr + reg_table_p->offset), *(ulong *)reg_p);
            }        
        break;
        }
    }
}

/*****************************************************************************
 * Function: scc_reg_dp
 *
 * Description: This function will dump scc registers.
 *
 * Input:  None
 * Output: PASSED/FAILED
 *****************************************************************************/
int scc_reg_dp()
{
    ulong base_addr;
    reg_info_t *reg_table_p;
    printf("ASYNC SCC Register Dump");
    int i=0, ch_num;
    char str1[10], str2[10], str3[10], context[128], all_str[128];

    for (i = 0; i < 6; i++) {
        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_CTRL_OFFSET;
        reg_table_p = &reva_scc_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);

        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_RX_DMA_CTRL_OFFSET;
        reg_table_p = &reva_scc_rx_dma_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);

        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_TX_DMA_CTRL_OFFSET;
        reg_table_p = &reva_scc_tx_dma_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);

        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_BISYNC_TX_CTRL_OFFSET;
        reg_table_p = &reva_scc_bisync_tx_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);
 
        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_AS_PPP_TX_CTRL_OFFSET;
        reg_table_p = &reva_scc_ppp_tx_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);

        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_AS_PPP_RX_CTRL_OFFSET;
        reg_table_p = &reva_scc_ppp_rx_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);

        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_BISYNC_RX_CTRL_OFFSET;
        reg_table_p = &reva_scc_bisync_rx_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);

        base_addr = get_scc_base() + (i * 0x4000) + ZYNC_SCC_IFACE_CTRL_OFFSET;
        reg_table_p = &reva_scc_iface_ctrl_reg_table[0];
        reva_reg_dump(base_addr, reg_table_p, i);
    }
    return PASSED;
}

/*****************************************************************************
 * Function: mgmt_intr_test_one
 *
 * Description: This function will test modem interrupt for one setting.
 *
 * Input:  chan - channel number
 *         type - type of interrupt
 * Output: PASSED/FAILED
 *****************************************************************************/
static int mgmt_intr_test_one (int chan, ushort type)
{
    prince_scc_regs_t * base_addr;
    prince_serial_itf_t *serial_reg; /*flow_cntrl  modem_intr_status */
    ushort assert = 0;
    ushort data;
    int rc = PASSED;
    int i;

    prpass(testpass, "mgmt intr chan[%d] type[%x]", chan, type);
    base_addr = (prince_scc_regs_t *)(get_scc_base() + ((chan / 4) * 0x4000));
    serial_reg = &base_addr->serial_itf[chan % 4];

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

/*****************************************************************************
 * Function: net_intr_test_one
 *
 * Description: This function will test DMA network interrupt for one setting.
 *
 * Input:  chan - channel number
 *         type - type of interrupt
 * Output: PASSED FAILED
 **********************************************************************/
 
/* Data Structure for channel loopback test */
prince_serial_ds_t prince_serial_ds[REVA_MAX_CH_NUM];

static int net_intr_test_one (int chan, ushort type, int priority)
{
    prince_scc_regs_t * base_addr;
    prince_proto_regs_t *proto_reg;
    prince_proto_regs_lp_t *proto_reg_lp;
    prince_serial_ds_t *s_ds;
    ushort assert = 0x0;
    ushort data;
    ushort chan_mask = 0;
    ushort int1_status_save, int2_status_save;
    int rc = PASSED;
    int i;

    base_addr = (prince_scc_regs_t *)(get_scc_base() + ((chan / 4) * 0x4000));
    proto_reg = &base_addr->proto_regs[chan % 4];
    proto_reg_lp = &base_addr->proto_regs_lp[chan % 4];

    /* test net irq2 */
    prpass(testpass, "net irq2 intr chan[%d] type[%x]", chan, type);

    if (priority == PRINCE_SCC_HIGH_PRIORITY ||
        type == PRINCE_RX_BUF_TYPE ||
        type == PRINCE_RX_FRAME_TYPE) {
        chan_mask = 1 << (chan % 4);
    } else {
        chan_mask = 1 << (chan % 4 + 4);
    }
    printf("chan_mask = %#x \t", chan_mask);

    base_addr->cntrl_1_regs.int_2_intr_mask = chan_mask;
    if (diagflag_xram & D_TRACE) {
        printf("Enable channel %d interrupt: "
            "cntrl_1_regs.int_2_intr_mask @%#x = %#x\n", chan,
            &base_addr->cntrl_1_regs.int_2_intr_mask, 
            base_addr->cntrl_1_regs.int_2_intr_mask);
    }

    s_ds = &prince_serial_ds[chan % 4];
    prince_init_serial_ds(s_ds);
    s_ds->port_num = chan % 4;
    s_ds->org_port_num = chan;

    /* enable interrupt at reva level and run loopback */
    switch (type) {
    case PRINCE_TX_BUF_TYPE:
        if (priority == PRINCE_SCC_HIGH_PRIORITY) {
            proto_reg->intr_mask |= PRINCE_DMA_TX_BUF_ENB ;
        } else {
            proto_reg_lp->intr_mask |= PRINCE_DMA_TX_BUF_ENB ;
        }
        assert = PRINCE_DMA_TX_BUF_STATUS;
        if (reva_chan_lpbk_test(PRINCE_INT_MODE, (ulong)base_addr,
            PRINCE_AS_BPS_128K, PRINCE_CLK_SRC_ASYNC, 
            PRINCE_SCC_INT_LOOPBACK, PRINCE_UART_ASYNC, 
            s_ds, priority))
            rc = FAILED;    
        break;

    case PRINCE_RX_BUF_TYPE:
        proto_reg->intr_mask |= PRINCE_DMA_RX_BUF_ENB ;
        assert = PRINCE_DMA_RX_BUF_STATUS;
        if (reva_chan_lpbk_test(PRINCE_INT_MODE, (ulong)base_addr,
            PRINCE_AS_BPS_128K, PRINCE_CLK_SRC_ASYNC, 
            PRINCE_SCC_EXT_LOOPBACK, PRINCE_UART_ASYNC, 
            s_ds, priority))
            rc = FAILED;
        break;

    case PRINCE_TX_FRAME_TYPE:
        if (priority == PRINCE_SCC_HIGH_PRIORITY) {
            proto_reg->intr_mask |= PRINCE_DMA_TX_FRAME_ENB ;
        } else {
            proto_reg_lp->intr_mask |= PRINCE_DMA_TX_FRAME_ENB ;
        }
        assert = PRINCE_DMA_TX_FRAME_STATUS;
        if (reva_chan_lpbk_test(PRINCE_FRAME_INT_MODE, (ulong)base_addr,
            PRINCE_AS_BPS_128K, PRINCE_CLK_SRC_ASYNC, 
            PRINCE_SCC_INT_LOOPBACK, PRINCE_UART_ASYNC, 
            s_ds, priority))
            rc = FAILED;
        break;

    case PRINCE_RX_FRAME_TYPE:
        proto_reg->intr_mask |= PRINCE_DMA_RX_FRAME_ENB ;
        assert = PRINCE_DMA_RX_FRAME_STATUS;
        if (reva_chan_lpbk_test(PRINCE_FRAME_INT_MODE, (ulong)base_addr,
            PRINCE_AS_BPS_128K, PRINCE_CLK_SRC_ASYNC, 
            PRINCE_SCC_EXT_LOOPBACK, PRINCE_UART_ASYNC, 
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
    }

    base_addr->cntrl_1_regs.int_1_intr_mask &= ~(PRINCE_DMA_ENB);
    base_addr->cntrl_1_regs.int_2_intr_mask &= ~chan_mask;
    proto_reg->intr_mask &= ~(PRINCE_DMA_TX_BUF_ENB | PRINCE_DMA_RX_BUF_ENB |
                            PRINCE_DMA_TX_FRAME_ENB |
                            PRINCE_DMA_RX_FRAME_ENB);
    proto_reg_lp->intr_mask &= ~(PRINCE_DMA_TX_BUF_ENB |
                            PRINCE_DMA_TX_FRAME_ENB);

    /* disable dma */
    prince_cleanup_mode_bits(s_ds);

    return (rc);
}

/**********************************************************************
 * Function: reva_scc_intr_test
 *
 * Description: Serial Channel Interrupt Test for Reva module.
 *
 * Inputs:  None
 * Outputs: PASSED / FAILED
 **********************************************************************/
int reva_scc_intr_test(int dummy)
{
    int chan;
    int board_id;
    dummy;

    testname("Async SCC Interrupt");

    board_id = get_board_id();
    /*
     * based on pid, build the test, menu string
     */
    switch(board_id) {
    case BOARD_ID_NIM_16A:
        as_num_chan = 16;
        sprintf(as_scc_menu_header.title, "NIM-16A-2G");
        break;
    case BOARD_ID_NIM_24A:
        as_num_chan = 24;
        sprintf(as_scc_menu_header.title, "NIM-24A-2G");
        break;
    case BOARD_ID_NIM_16A_4G:
        as_num_chan = 16;
        sprintf(as_scc_menu_header.title, "NIM-16A-4G");
        break;
    case BOARD_ID_NIM_24A_4G:
        as_num_chan = 24;
        sprintf(as_scc_menu_header.title, "NIM-24A-4G");
        break;

    default:
        cterr('f', 0, "Invalid reva ID 0x%X", board_id);
        return (FAILED);
    }

    /* test modem management IRQ */
    for (chan = 0; chan < as_num_chan; chan++) {
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
    for (chan = 0; chan < as_num_chan; chan++) {
        /*for multi*/
        msleep(10);

        if (net_intr_test_one(chan, PRINCE_TX_FRAME_TYPE, PRINCE_SCC_HIGH_PRIORITY))
            return (FAILED);
        if (net_intr_test_one(chan, PRINCE_TX_BUF_TYPE, PRINCE_SCC_HIGH_PRIORITY))
            return (FAILED);
        if (net_intr_test_one(chan, PRINCE_TX_FRAME_TYPE, PRINCE_SCC_LOW_PRIORITY))
            return (FAILED);
        if (net_intr_test_one(chan, PRINCE_TX_BUF_TYPE, PRINCE_SCC_LOW_PRIORITY))
            return (FAILED);

        if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
            if (net_intr_test_one(chan, PRINCE_RX_FRAME_TYPE, PRINCE_SCC_HIGH_PRIORITY))
                return (FAILED);
            if (net_intr_test_one(chan, PRINCE_RX_BUF_TYPE, PRINCE_SCC_HIGH_PRIORITY))
                return (FAILED);
            if (net_intr_test_one(chan, PRINCE_RX_FRAME_TYPE, PRINCE_SCC_LOW_PRIORITY))
                return (FAILED);
            if (net_intr_test_one(chan, PRINCE_RX_BUF_TYPE, PRINCE_SCC_LOW_PRIORITY))
                return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 * Function: build_asyn_scc_sub_menu
 *
 * Description: Build Async Serial Channel Test menu.
 *
 * Input :  Number of Channels
 * Outputs: None.
 ***********************************************************************/
static void build_async_scc_sub_menu ()
{
    int i;

    /* Init base menu */
    init_base_submenu(&as_scc_menup, (int)as_scc_menu_header.title);

    /* Add common menu items */
    for ( i = 0; i < AS_NUM_COMMON_TESTS; i++ ) {
        sprintf(as_common_menu_title[i].title, "%s",
                as_scc_common_menu_items[i].mline);
        add_menu_item(&as_scc_menu, as_common_menu_title[i].title,
                        as_scc_common_menu_items[i].mfunc,
                        /*scc_common_menu_items[i].mfparam*/(type_t *)&i, 
                        as_scc_common_menu_items[i].mflag);
    }

    /* Add channel test menu items */ 
    for ( i = 0; i < as_num_chan; i++ ) {
        sprintf(as_scc_menu_title[i].title, "%s %d",
            "test channel", i);
        as_scc_menu_index[i] = i;
        add_menu_item(&as_scc_menu, as_scc_menu_title[i].title,
            (PFT)ngwic_reva_chan_lpbk_test, (type_t *)&as_scc_menu_index[i],
            MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT | MF_MULTI);
    }
}

/**********************************************************************
 * Function: async_scc_sub_test
 *
 * This function invokes the sub diagnostics tests for SCC
 *
 * Input : Number of Channels
 * Output: PASSED or FAILED
 ***********************************************************************/
static int async_scc_sub_test()
{
    int  i;

    /* Run common tests - skip util from table */
    for (i = 0; i < AS_NUM_COMMON_TESTS; i++) {
        if (as_scc_common_menu_items[i].mflag & MF_DOALL) {
            prpass(testpass, "%s,", as_scc_menu_title[i].title);
            if ((as_scc_common_menu_items[i].mfunc)(as_scc_common_menu_items[i].mfparam)) {
                return (FAILED);
            }
        }
    }

    /* Run channel tests */
    for ( i = 0; i < as_num_chan; i++ ) {
        prpass(testpass, "test channel %d,", i);
        if (ngwic_reva_chan_lpbk_test(i)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 * Function: async_serial_channel_test
 *
 * Description: Async Serial Channel Test.
 *
 * Inputs:  show_menu - FALSE for tests. TRUE for submenu.
 * Outputs: PASSED or FAILED
 ***********************************************************************/
int async_serial_channel_test(int show_menu)
{
    int rc;
    int board_id;

    testname("Async SCC");

    board_id = get_board_id();
    /* 
     * based on pid, build the test, menu string
     */
    switch(board_id) {
    case BOARD_ID_NIM_16A:
        as_num_chan = 16;
        sprintf(as_scc_menu_header.title, "NIM-16A-2G");
        break;
    case BOARD_ID_NIM_24A:
        as_num_chan = 24;
        sprintf(as_scc_menu_header.title, "NIM-24A-2G");
        break;
    case BOARD_ID_NIM_16A_4G:
        as_num_chan = 16;
        sprintf(as_scc_menu_header.title, "NIM-16A-4G");
        break;
    case BOARD_ID_NIM_24A_4G:
        as_num_chan = 24;
        sprintf(as_scc_menu_header.title, "NIM-24A-4G");
        break;

    default:
        cterr('f', 0, "Invalid reva ID 0x%X", board_id);
        return (FAILED);
    }
    
    build_async_scc_sub_menu();

    if (show_menu) {
        /* Entered submenu */
        menu(&as_scc_menu, (mitem_t *)0, 0);
        rc = PASSED;
    } else {
        /* Invoked the test */
        rc = async_scc_sub_test();
    }

    return rc;
}

/**********************************************************************
 * Function: get_as_scc_channel_num
 *
 * Description: Get async scc Channel number
 *
 * Inputs:  None.
 * Outputs: as_num_chan
 ***********************************************************************/
int get_as_scc_channel_num()
{
    return as_num_chan;
}

/******** History ******** 
$Log: reva_scc.c,v $
Revision 1.5  2019/09/23 10:04:26  alpeng
 CSCvq77997 - resolve reva duplicate problem

Revision 1.4  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3  2018/07/23 06:45:00  easochen
Reva: Add new board_id and firmware for 4G DDR

Revision 1.2  2016/05/06 03:43:53  umlin
Reva: Commit Reva module side diag codes to main trunk


$Endlog$
*/
