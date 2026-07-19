 /* $Id: crocus_scc_diag.c,v 1.2 2019/12/11 10:10:27 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/crocus_scc_diag.c,v $
 *------------------------------------------------------------------
 *
 * crocus_scc_diag.c - Serial Channel Controller diag function.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "types.h"
#include "strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include "error.h"
#include "common.h"
#include "nvmonvars.h"
#include "proto.h"
#include "common_utils.h"
#include "diag_async_test.h"
#include "nanook_crocus_def.h"
#include "linux_api.h" /* print_offset_val */

//#define ASYNC_4_BYTE_TX_RX
/*===================================================================*
 *                    extern functions                               *
 *===================================================================*/
extern int get_as_scc_channel_num();
extern void crocus_init_serial_ds(crocus_serial_ds_t *s_ds);
extern int crocus_async_lpbk_test(ulong scc_base_addr, crocus_serial_ds_t *s_ds);
extern int crocus_form_name_buf(char *tmp_name_buf, crocus_serial_ds_t *s_ds);
extern void crocus_config_serial_ds(uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                              uchar lpbk_mode, uchar protocol,
                              crocus_serial_ds_t *s_ds,
#ifdef XDMA
                              char *xdma_path,
#endif
                              ulong *des_baud, ushort *clk_src_bits);
extern void crocus_init_tx_buffers(crocus_serial_ds_t *s_ds);
extern void crocus_init_rx_buffers(crocus_serial_ds_t *s_ds);
extern void crocus_tx(crocus_serial_ds_t *s_ds);
extern void crocus_rx(crocus_serial_ds_t *s_ds);
extern void crocus_tx_rx(crocus_serial_ds_t *s_ds);
extern void crocus_cleanup_serial(crocus_serial_ds_t *s_ds);
extern void crocus_cleanup_buffer(ulong scc_base_addr, crocus_serial_ds_t *s_ds);
extern ushort crocus_check_received_data(crocus_serial_ds_t *s_ds);
extern int crocus_dcd_dtr_test(ulong scc_base_addr, crocus_serial_ds_t *s_ds);
extern int crocus_cts_rts_test(ulong scc_base_addr, crocus_serial_ds_t *s_ds);
extern void crocus_cleanup_mode_bits(crocus_serial_ds_t *s_ds);
extern int get_scc_base_addr(int mb);
extern boolean has_daughter_card(int);

/*
 * Global variables
 */

/*===================================================================*
 *                             Globals                               *
 *===================================================================*/

/* Data Structure for channel loopback test */
crocus_serial_ds_t crocus_serial_ds[NANOOK_ASYNC_MAX_CH_NUM];

/*
 *  ASync Clock Baud rate tables, 8x baud clock output
 */
crocus_baud_t crocus_async_baud[] = {
    { CROCUS_ASYNC_BPS_NONE,     300,    2400,      840, CROCUS_MODEM_BRG_2M },
    { CROCUS_ASYNC_BPS_300,      300,    2400,      840, CROCUS_MODEM_BRG_2M },
    { CROCUS_ASYNC_BPS_600,      600,    4800,      420, CROCUS_MODEM_BRG_2M },
    { CROCUS_ASYNC_BPS_1200,    1200,    9600,      210, CROCUS_MODEM_BRG_2M },
    { CROCUS_ASYNC_BPS_2400,    2400,   19200,      105, CROCUS_MODEM_BRG_2M },
    { CROCUS_ASYNC_BPS_4800,    4800,   38400,      840, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_9600,    9600,   76800,      420, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_14400,  14400,  115200,      280, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_19200,  19200,  153600,      210, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_28800,  28800,  230400,      140, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_32K,    32000,  256000,      126, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_38400,  38400,  307200,      105, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_48K,    48000,  384000,       84, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_56K,    56000,  448000,       72, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_57600,  57600,  460800,       70, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_64K,    64000,  512000,       63, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_72K,    72000,  576000,       56, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_115200,115200,  921600,       35, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_128K,  128000, 1008000,       32, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_230400,230400, 1792000,       18, CROCUS_MODEM_BRG_32M },
    { CROCUS_ASYNC_BPS_256000,256000, 2016000,       16, CROCUS_MODEM_BRG_32M },
};


/* Crocus Global Register Tables */
static reg_info_t crocus_global_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size, Mask,     Reset Value */
    {"Revision ID",
    0x00,       READ_ONLY,  {4},    0xffffffff,     0x0},
    {"Status",
    0x04,       READ_ONLY,  {4},    0x0000000f,     0x0},
    {"LED Control",
    0x08,       READ_WRITE, {4},    0x000000ff,     0x0},
    {"General Purpose Timer 0-3 Interrupt Status",
    0x0c,       READ_ONLY,  {4},    0x0000000f,     0x0},
    {"General Pirpose Timer 0-3 Source Enable",
    0x10,       READ_WRITE, {4},    0x0000000f,     0x0},
    {"General Purpose Timer 0-1 Programmable",
    0x14,       READ_WRITE, {4},    0xffffffff,     0x0},
    {"General Purpose Timer 2-3 Programmable",
    0x18,       READ_WRITE, {4},    0xffffffff,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_global_reg_table_db[] =
{
/*  Register name,
    Offset,     Type,       Size, Mask,     Reset Value */
    {"Revision ID",
    0x00,       READ_ONLY,  {4},    0xffffffff,     0x0},
    {"Status",
    0x04,       READ_ONLY,  {4},    0x0000000f,     0x0},
    {"LED Control",
    0x08,       READ_WRITE, {4},    0x0000000f,     0x0},
    {"General Purpose Timer 0-3 Interrupt Status",
    0x0c,       READ_ONLY,  {4},    0x0000000f,     0x0},
    {"General Pirpose Timer 0-3 Source Enable",
    0x10,       READ_WRITE, {4},    0x0000000f,     0x0},
    {"General Purpose Timer 0-1 Programmable",
    0x14,       READ_WRITE, {4},    0xffffffff,     0x0},
    {"General Purpose Timer 2-3 Programmable",
    0x18,       READ_WRITE, {4},    0xffffffff,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_spi_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"SPI Flash Control",
    0x00,       READ_ONLY,  {4},    0x000087ff,     0x10},
    {"SPI Flash Status",
    0x04,       READ_ONLY,  {4},    0x0000803f,     0xa0},
    {"SPI Flash Read Size",
    0x08,       READ_ONLY,  {4},    0x000000ff,     0x0},
    {"SPI Flash Read/Write Data",
    0x0c,       READ_ONLY,  {4},    0x000000ff,     0x0},
    {"SPI Flash TX Data",
    0x10,       READ_ONLY,  {4},    0x00ffffff,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_ctrl_intr_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Interrupt Status",
    0x00,       READ_ONLY , {4},    0x0000000f,     0x0},
    {"Interrupt Enable",
    0x04,       READ_WRITE, {4},    0x0000000f,     0x0},
    {"Timer Proggrammable",
    0x08,       READ_WRITE, {4},    0x0000007f,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_scc_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Channeln Mode",
    0x00,       READ_WRITE, {4},    0x1ffdff3f,     0x10019030},
    {"Channeln Flag Config",
    0x04,       READ_WRITE, {4},    0x0000ffff,     0x0},
    {"Channeln Flow Control",
    0x08,       READ_WRITE, {4},    0x00007fff,     0x0},
    {"Channeln Interrupt Status",
    0x0c,       READ_ONLY,  {4},    0x0000ffff,     0x0},
    {"Channeln Interrupt Enable",
    0x10,       READ_WRITE, {4},    0x0000ea00,     0x0},
    {"Channeln Command/Status",
    0x14,       READ_ONLY,  {4},    0x00000707,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_ppp_tx_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Transmit Async Control Character Map",
    0x00,       READ_WRITE, {4},    0xffffffff,     0x0},
    {"Transmit Special Mapped Character",
    0x04,       READ_WRITE, {4},    0x0fffffff,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_ppp_rx_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Receive Async Control Character Map",
    0x00,       READ_WRITE, {4},    0xffffffff,     0x0},
    {"Receive Special Mapped Character",
    0x04,       READ_WRITE, {4},    0x00ffffff,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_serial_itf_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"Serial Interface Control",
    0x00,       READ_WRITE, {4},    0x00000e00,     0x200},
    {"Modem Control",
    0x04,       READ_WRITE, {4},    0x000001ef,     0xe010},
    {"Serial Interface Flow Control",
    0x08,       READ_WRITE, {4},    0x0000f70f,     0x0},
    {"Serial Port Baud Rate Generator Divide",
    0x0c,       READ_WRITE, {4},    0x000003ff,     0x0},
    {"Serial Interface Modem Interrupt Status",
    0x10,       READ_WRITE, {4},    0x000000f0,     0x0},
    {"Frequency Counter Select",
    0x14,       READ_WRITE, {4},    0x00000001,     0x0},
    {"Frequency Counter",
    0x18,       READ_ONLY , {4},    0x00ffffff,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

static reg_info_t crocus_ring_buf_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size,   Mask,       Reset Value */
    {"RX Ring Buffer Start Pointer",
    0x00,       READ_WRITE, {4},    0x00000ffc,     0x0},
    {"Rx Ring Buffer End Pointer",
    0x04,       READ_ONLY,  {4},    0x00000fff,     0x0},
    {"TX Ring Buffer Start Pointer",
    0x08,       READ_ONLY,  {4},    0x00000efc,     0x0},
    {"Tx Ring Buffer End Pointer",
    0x0c,       READ_ONLY, {4},    0x00000eff,     0x0},
    {"Ring Buffer Control",
    0x10,       READ_WRITE, {4},    0x00000003,     0x0},
    {"RX Ring Buffer Status",
    0x14,       READ_ONLY,  {4},    0x1fff1fff,     0x1000},
    {"TX Ring Buffer Status",
    0x18,       READ_ONLY,  {4},    0x1fff1fff,     0x800},
    {"Buffer Interrupt Status",
    0x1c,       READ_ONLY,  {4},    0x00000003,     0x0},
    {"Buffer Interrupt Enable",
    0x20,       READ_WRITE, {4},    0x00000003,     0x0},
    {"END",                                
    0x00,       0,          {0},           0x0,     0x0},
};

extern ulong scc_base;
extern ulong mb_scc_base;
extern ulong db_scc_base;
extern char *cur_xdma_path;
extern char *mb_xdma_path;
extern char *db_xdma_path;

/*****************************************************************************
 * Function   : get_scc_base
 *
 * Description: Get the scc_base address
 *
 * Inputs     : None
 * Outputs    : scc_base address
 *****************************************************************************/
ulong get_scc_base ()
{
    return scc_base;
}

/******************************************************************************
 * Function: crocus_display_global_regs
 *
 * Description: This function will display global registers.
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 * Output: None
 ******************************************************************************/
void crocus_display_global_regs (ulong scc_base_addr)
{
    crocus_global_regs_t *global_regs;
    crocus_regs_t * base_addr;

    base_addr = (crocus_regs_t *)scc_base_addr;
    global_regs = &base_addr->global_regs;
    printf("\nCROCUS GLOBAL REGISTERS:................\n");
    printf("Revision ID           @%#x = %#x\n", &global_regs->revision_ID, global_regs->revision_ID);
    printf("Status Register       @%#x = %#x\n", &global_regs->status_reg, global_regs->status_reg);
    printf("LED Register          @%#x = %#x\n", &global_regs->led_reg, global_regs->led_reg);
    printf("Timer 0-3 Status      @%#x = %#x\n", &global_regs->timer_0_3_intr_status, global_regs->timer_0_3_intr_status);
    printf("Timer 0-3 Mask        @%#x = %#x\n", &global_regs->timer_0_3_intr_mask, global_regs->timer_0_3_intr_mask);
    printf("Timer 0-1 program     @%#x = %#x\n", &global_regs->timer_0_1_pgm, global_regs->timer_0_1_pgm);
    printf("timer 2-3 program     @%#x = %#x\n", &global_regs->timer_2_3_pgm, global_regs->timer_2_3_pgm);
}

/******************************************************************************
 * Function: crocus_display_scc_regs
 *
 * Description: This function will display protocol registers.
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 *         disp_opt - Display mode
 *         chan - Channel number
 * Output: None
 ******************************************************************************/
void crocus_display_scc_regs (ulong scc_base_addr, uchar disp_opt, uchar chan)
{
    crocus_scc_regs_t *scc_regs;
    crocus_regs_t *base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (crocus_regs_t *) scc_base_addr;
    num_chan = get_as_scc_channel_num();

    if (disp_opt == CROCUS_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }
    for (ix = start_chan; ix < end_chan; ix++) {
        scc_regs = &base_addr->chan_regs[ix].scc_regs;
        printf("\nCROCUS SCC REGISTERS CHAN %d:................\n",ix);
        printf("mode register          @%#x = %#x\n", &scc_regs->mode_reg, scc_regs->mode_reg);
        printf("flag config            @%#x = %#x\n", &scc_regs->flag_config, scc_regs->flag_config);
        printf("flow control config    @%#x = %#x\n", &scc_regs->flow_cntrl_config, scc_regs->flow_cntrl_config);
        printf("interrupt status       @%#x = %#x\n", &scc_regs->intr_status, scc_regs->intr_status);
        printf("interrupt mask         @%#x = %#x\n", &scc_regs->intr_mask, scc_regs->intr_mask);
        printf("command register       @%#x = %#x\n", &scc_regs->command_reg, scc_regs->command_reg);
    }
}

/******************************************************************************
 *
 * Function: crocus_display_ppp_regs
 *
 * Description: This function will display ppp transmit/receive registers.
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 *         disp_opt - Display mode
 *         chan - Channel number
 * Output: None
 ******************************************************************************/
static void crocus_display_ppp_regs (ulong scc_base_addr, uchar disp_opt, uchar chan)
{
    crocus_ppp_tx_regs_t *ppp_tx_regs;
    crocus_ppp_rx_regs_t *ppp_rx_regs;
    crocus_regs_t *base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (crocus_regs_t *)scc_base_addr;
    num_chan = get_as_scc_channel_num();

    if (disp_opt == CROCUS_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }

    for (ix = start_chan; ix < end_chan; ix++) {
        ppp_tx_regs = &base_addr->chan_regs[ix].ppp_tx_regs;
        printf("\nCROCUS PPP-TX REGISTERS CHAN %d:................\n",ix);
        printf("control char map     @%#x = %#x\n", &ppp_tx_regs->tx_ctrl_char_map, ppp_tx_regs->tx_ctrl_char_map);
        printf("tx special           @%#x = %#x\n", &ppp_tx_regs->tx_special, ppp_tx_regs->tx_special);

        ppp_rx_regs = &base_addr->chan_regs[ix].ppp_rx_regs;
        printf("\nCROCUS PPP-TX REGISTERS CHAN %d:................\n",ix);
        printf("control char map     @%#x = %#x\n", &ppp_rx_regs->rx_ctrl_char_map, ppp_rx_regs->rx_ctrl_char_map);
        printf("rx special           @%#x = %#x\n", &ppp_rx_regs->rx_special, ppp_rx_regs->rx_special);
    }
}

/******************************************************************************
 * Function: crocus_display_serial_itf_regs
 *
 * Description: This function will display serial interface registers.
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 *         disp_opt - Display mode
 *         chan - Channel number
 * Output: None
 ******************************************************************************/
void crocus_display_serial_itf_regs (ulong scc_base_addr, uchar disp_opt, uchar chan)
{
    crocus_serial_itf_t *serial_regs;
    crocus_regs_t * base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (crocus_regs_t *) scc_base_addr;
    num_chan = get_as_scc_channel_num();

    if (disp_opt == CROCUS_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }

    for (ix = start_chan; ix < end_chan; ix++) {
        serial_regs = &base_addr->chan_regs[ix].serial_itf;
        printf("\nCROCUS SERIAL REGISTERS CHAN %d:................\n",ix);
        printf("serial interface cntrl    @%#x = %#x\n", &serial_regs->serial_itf_cntrl, serial_regs->serial_itf_cntrl);
        printf("modem control             @%#x = %#x\n", &serial_regs->modem_cntrl, serial_regs->modem_cntrl);
        printf("flow control              @%#x = %#x\n", &serial_regs->flow_cntrl, serial_regs->flow_cntrl);
        printf("brg divider               @%#x = %#x\n", &serial_regs->brg_divider, serial_regs->brg_divider);
        printf("modem intr status         @%#x = %#x\n", &serial_regs->modem_intr_status, serial_regs->modem_intr_status);
        printf("freq counter port sel     @%#x = %#x\n", &serial_regs->freq_cnt_port_select, serial_regs->freq_cnt_port_select);
        printf("freq counter              @%#x = %#x\n", &serial_regs->freq_cnt, serial_regs->freq_cnt);
    }
}

/******************************************************************************
 * Function: crocus_display_ring_buf_regs_regs
 *
 * Description: This function will display Ring Buffer Control registers.
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 *         disp_opt - Display mode
 *         chan - Channel number
 * Output: None
 ******************************************************************************/
void crocus_display_ring_buf_regs_regs (ulong scc_base_addr, uchar disp_opt, uchar chan)
{
    crocus_ring_buf_regs_t *ring_buf_regs;
    crocus_regs_t * base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (crocus_regs_t *) scc_base_addr;
    num_chan = get_as_scc_channel_num();

    if (disp_opt == CROCUS_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }

    for (ix = start_chan; ix < end_chan; ix++) {
        ring_buf_regs = &base_addr->chan_regs[ix].ring_buf_regs;
        printf("\nCROCUS Ring Buffer Control REGISTERS CHAN %d:................\n",ix);
        printf("RX Ring Buffer Start Pointer Register    @%#x = %#x\n", &ring_buf_regs->rx_start, ring_buf_regs->rx_start);
        printf("RX Ring Buffer End Pointer Register      @%#x = %#x\n", &ring_buf_regs->rx_end, ring_buf_regs->rx_end);
        printf("TX Ring Buffer Start Pointer Register    @%#x = %#x\n", &ring_buf_regs->tx_start, ring_buf_regs->tx_start);
        printf("TX Ring Buffer End Pointer Register      @%#x = %#x\n", &ring_buf_regs->tx_end, ring_buf_regs->tx_end);
        printf("Ring Buffer Control Register             @%#x = %#x\n", &ring_buf_regs->ctrl, ring_buf_regs->ctrl);
        printf("RX Ring Buffer Status Register           @%#x = %#x\n", &ring_buf_regs->rx_status, ring_buf_regs->rx_status);
        printf("TX Ring Buffer Status Register           @%#x = %#x\n", &ring_buf_regs->tx_status, ring_buf_regs->tx_status);
		printf("Buffer Interrupt Status Register         @%#x = %#x\n", &ring_buf_regs->intr_status, ring_buf_regs->intr_status);
		printf("Ring Buffer Interrupt Enable Register    @%#x = %#x\n", &ring_buf_regs->intr_mask, ring_buf_regs->intr_mask);
    }
}

/******************************************************************************
 * Function: crocus_async_chan_lpbk_test
 *
 * Description: This function will run datapath channel loopback test
 *
 * Input: channel - Channel number
 * Output: PASSED/FAILED
 ******************************************************************************/
int crocus_async_chan_lpbk_test (int channel)
{
    ulong scc_base_addr;
    crocus_serial_ds_t *s_ds;

    testname("Async SCC Loopback test");

    scc_base_addr = get_scc_base();

    /* Get serial data structure and init it */
    s_ds = &crocus_serial_ds[channel];

    s_ds->port_num = channel;
    crocus_init_serial_ds(s_ds);

    if (crocus_async_lpbk_test(scc_base_addr, s_ds)) {
        return (FAILED);
    }

    return (PASSED);
}

int retval[NANOOK_ASYNC_MAX_CH_NUM];
void *crocus_async_chan_lpbk_thread(void *data)
{
    ulong scc_base_addr;
    crocus_serial_ds_t *s_ds = (crocus_serial_ds_t *)data;
    
    scc_base_addr = get_scc_base();
    if (crocus_async_lpbk_test(scc_base_addr, s_ds)) {
        retval[s_ds->port_num] = FAILED;
    } else {
        retval[s_ds->port_num] = PASSED;
    }
    pthread_exit(NULL);
}

/******************************************************************************
 * Function: crocus_async_chan_lpbk_test_all
 *
 * Description: This function will run datapath channel loopback test for all channels
 *
 * Output: PASSED/FAILED
 ******************************************************************************/
int crocus_async_chan_lpbk_test_all (void)
{
    crocus_serial_ds_t *s_ds;
    int ix, num_chan, ret = PASSED;
    pthread_t threads[NANOOK_ASYNC_MAX_CH_NUM];

    testname("Async SCC Loopback test");

    num_chan = get_as_scc_channel_num();

    for (ix = 0; ix < num_chan; ix++) {
        /* Get serial data structure and init it */
        s_ds = &crocus_serial_ds[ix];
        s_ds->port_num = ix;
        crocus_init_serial_ds(s_ds);
        if (pthread_create(&threads[ix], NULL, crocus_async_chan_lpbk_thread, (void *)s_ds)) {
            printf("pthread_create %d failed \n", ix);
            return (FAILED);
        }
    }
    
    for (ix = 0; ix < num_chan; ix++) {
        pthread_join(threads[ix], NULL);
        if (retval[ix] != PASSED) {
            cterr('f',0," Channel [%d], ASYNC loopback test fail,", ix);
            ret = FAILED;
        }
    }

    return (ret);
}

/*******************************************************************************
 * Function: crocus_trap
 *
 * Description: This function trap some helpful information for users and
 *              trigger logical analyzer.
 *
 * Input: s_ds - pointer to serial data structure
 * Output: Nonne
 ******************************************************************************/
void crocus_trap (crocus_serial_ds_t *s_ds)
{
    ulong scc_base_addr;
    char tmp_name_buf[30];
    uint des_baud = 0;

    s_ds->serial_itf_addr->freq_cnt_port_select = CROCUS_TRIGGER_LOGIC_ANALYZER;
    s_ds->serial_itf_addr->freq_cnt_port_select = 0;

    scc_base_addr = s_ds->base_addr;
    crocus_display_global_regs(scc_base_addr);
    crocus_display_scc_regs(scc_base_addr, CROCUS_REG_ONE, s_ds->port_num);
    crocus_display_ppp_regs(scc_base_addr, CROCUS_REG_ONE, s_ds->port_num);
    crocus_display_serial_itf_regs(scc_base_addr, CROCUS_REG_ONE, s_ds->port_num);
    crocus_display_ring_buf_regs_regs(scc_base_addr, CROCUS_REG_ONE, s_ds->port_num);

    crocus_form_name_buf(tmp_name_buf, s_ds);
    des_baud = crocus_async_baud[s_ds->speed_idx].desired_baud;

    printf("\n -%schan %d @speed %d --\n", tmp_name_buf, s_ds->port_num, des_baud);
}

/******************************************************************************
 * Function: crocus_chan_lpbk_tx
 *
 * Description: This function will run one-shot loopback tx for a single channel
 *
 * Input: run_mode  - Interrupt or poll.
 *      : scc_base_addr - base address for SCC.
 *      : speed_idx - index to the baud rate table.
 *      : lpbk_mode - internal/external.
 *      : protocol - UART ASYNC / UART PPP.
 *      : s_ds - Pointer to the serial data structure.
 * Output: PASSED FAILED
 ******************************************************************************/
int crocus_chan_lpbk_tx (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                         uchar lpbk_mode, uchar protocol,
#ifdef XDMA
                         char *xdma_path,
#endif
                         crocus_serial_ds_t *s_ds)
{
    char tmp_name_buf[30];
    ulong des_baud = 0;
    ushort clk_src_bits = 0;
    ulong dummy;

    crocus_config_serial_ds(run_mode, scc_base_addr, speed_idx,
                            lpbk_mode, protocol, s_ds,
#ifdef XDMA
                            xdma_path,
#endif
                            &des_baud, &clk_src_bits);

    crocus_form_name_buf(tmp_name_buf, s_ds);
    prpass(testpass,"%schan %d @speed %d ", tmp_name_buf, s_ds->port_num,
           des_baud);

    /*
     *  Disable mode
     */
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_MASK;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 1: Disable the channel\n", s_ds->port_num);
        printf("\ts_ds->scc_regs_addr->mode_reg @%#x = %#x\n",
            &s_ds->scc_regs_addr->mode_reg,
            s_ds->scc_regs_addr->mode_reg);
    }

    /*
     *  reset Tx and Rx paths
     */
    s_ds->scc_regs_addr->command_reg = CROCUS_CMD_RESET_TX_RX;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 2: reset Tx and Rx paths\n", s_ds->port_num);
        printf("\ts_ds->scc_regs_addr->command_reg @%#x = %#x\n",
            &s_ds->scc_regs_addr->command_reg,
            s_ds->scc_regs_addr->command_reg);
    }

    /*
     *   assert reset interface circuit
     */
    s_ds->serial_itf_addr->serial_itf_cntrl |= CROCUS_RESET_ITF_CKT;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 3: reset channel interface\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->serial_itf_cntrl @%#x = %#x\n",
            &s_ds->serial_itf_addr->serial_itf_cntrl, s_ds->serial_itf_addr->serial_itf_cntrl);
    }

    /*
     *  Set baud rate divider
     */
    s_ds->serial_itf_addr->brg_divider = s_ds->divider;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 4: Set baud rate divider\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->brg_divider @%#x = %#x\n",
            &s_ds->serial_itf_addr->brg_divider,
            s_ds->serial_itf_addr->brg_divider);
    }

    /*
     *  Set modem control
     */
    s_ds->serial_itf_addr->modem_cntrl &= ~CROCUS_MODEM_BRG_MASK;

    if (clk_src_bits == CROCUS_MODEM_BRG_2M) {
        s_ds->serial_itf_addr->modem_cntrl |= CROCUS_MODEM_BRG_2M;
    } else {
        s_ds->serial_itf_addr->modem_cntrl |= CROCUS_MODEM_BRG_32M;
    }

    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 5: Setup modem control\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->modem_cntrl @%#x = %#x\n",
            &s_ds->serial_itf_addr->modem_cntrl,
            s_ds->serial_itf_addr->modem_cntrl);
    }

    s_ds->serial_itf_addr->flow_cntrl = 0;

    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 6: intfc flow control\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->flow_cntrl @%#x = %#x\n",
            &s_ds->serial_itf_addr->flow_cntrl,
            s_ds->serial_itf_addr->flow_cntrl);
    }

    /*
     *  Set Channel Flag & Flow Control
     */
    s_ds->scc_regs_addr->flag_config = CROCUS_LOOPBACK_XON_XOFF_FLAG;
    /* Flow Control DTR/CTS &  RTS/DTR swap mux enable */
    s_ds->scc_regs_addr->flow_cntrl_config = CROCUS_LOOPBACK_FLOW_CTRL_FLAG;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 7: Setup Channel Flag & Flow Control \n", s_ds->port_num);
        printf("\ts_ds->scc_regs_addr->flag_config @%#x = %#x\n",
            &s_ds->scc_regs_addr->flag_config,
            s_ds->scc_regs_addr->flag_config);
        printf("\ts_ds->scc_regs_addr->flow_cntrl_config @%#x = %#x\n",
            &s_ds->scc_regs_addr->flow_cntrl_config,
            s_ds->scc_regs_addr->flow_cntrl_config);
    }

    /*
     *  De-assert reset to channel interface
     */
    s_ds->serial_itf_addr->serial_itf_cntrl &= ~CROCUS_RESET_ITF_CKT;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 8: De-assert reset to channel interface\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->serial_itf_cntrl @%#x = %#x\n",
            &s_ds->serial_itf_addr->serial_itf_cntrl,
            s_ds->serial_itf_addr->serial_itf_cntrl);
    }

    /*
     *  reset Tx and Rx paths again
     */
    s_ds->scc_regs_addr->command_reg = CROCUS_CMD_RESET_TX_RX;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 9: reset Tx and Rx paths again\n", s_ds->port_num);
        printf("\ts_ds->scc_regs_addr->command_reg @%#x = %#x\n",
            &s_ds->scc_regs_addr->command_reg,
            s_ds->scc_regs_addr->command_reg);
    }

    /*
     *  clear FCS and initialize mode register
     */
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_FCS_MASK;
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_NRZ_MASK;
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_FLAG_CHK_MASK;
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_EBCDIC;
    s_ds->scc_regs_addr->mode_reg |= CROCUS_MODE_CRC_16_MASK;   /* default */
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_IDLE_MARK;

    /*
     *  Set Protocol Mode register
     */
    s_ds->ring_buf_regs_addr->ctrl &= ~CROCUS_RING_BUFFER_MODE_MASK;
    switch (protocol) {
    case CROCUS_ASYNC_UART:
	s_ds->scc_regs_addr->mode_reg |= CROCUS_MODE_UART_ASYNC;
        s_ds->ring_buf_regs_addr->ctrl |= CROCUS_RING_BUFFER_TTY;
        s_ds->ctrl_intr_regs_addr->timer_pgm = CROCUS_ASYNC_TIMER_COUNT;
        break;

    case CROCUS_ASYNC_PPP:
        s_ds->scc_regs_addr->mode_reg |= CROCUS_MODE_UART_PPP;
        s_ds->ring_buf_regs_addr->ctrl |= CROCUS_RING_BUFFER_PPP;
        /* turn off CRC if requested */
        if (!s_ds->crc_enb) {
            s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_CRC;
        }
        s_ds->ctrl_intr_regs_addr->timer_pgm = CROCUS_ASYNC_TIMER_COUNT;
        break;

    default:
        break;
    }
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 10: Setup the channel mode\n", s_ds->port_num);
        printf("\ts_ds->scc_regs_addr->mode_reg @%#x = %#x\n",
            &s_ds->scc_regs_addr->mode_reg,
            s_ds->scc_regs_addr->mode_reg);
        printf("\ts_ds->ring_buf_regs_addr->ctrl @%#x = %#x\n",
            &s_ds->ring_buf_regs_addr->ctrl,
            s_ds->ring_buf_regs_addr->ctrl);
    }

    /*
     *  dummy read of modem interrupt status
     */
    dummy = s_ds->serial_itf_addr->modem_intr_status;
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 11: Clear pending interrupts\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->modem_intr_status @%#x = %#x\n",
            &s_ds->serial_itf_addr->modem_intr_status,
            s_ds->serial_itf_addr->modem_intr_status);
    }

    /*
     *  Set loopback mode
     */
    if (lpbk_mode == CROCUS_ASYNC_SCC_INT_LOOPBACK) {
        s_ds->serial_itf_addr->modem_cntrl |= CROCUS_MODEM_LOOPBACK;
    } else {
        s_ds->serial_itf_addr->modem_cntrl &= ~CROCUS_MODEM_LOOPBACK;
    }
    if (diagflag_xram & D_TRACE) {
        printf(" ch %d Step 12: Set loopback mode\n", s_ds->port_num);
        printf("\ts_ds->serial_itf_addr->modem_cntrl @%#x = %#x\n",
            &s_ds->serial_itf_addr->modem_cntrl,
            s_ds->serial_itf_addr->modem_cntrl);
    }

    crocus_init_tx_buffers(s_ds);

    crocus_tx(s_ds);

    return (PASSED);
}

/******************************************************************************
 * Function: crocus_chan_lpbk_rx
 *
 * Description: This function will run one-shot loopback rx for a single channel
 *
 * Input: run_mode  - Interrupt or poll.
 *      : scc_base_addr - base address for SCC.
 *      : speed_idx - index to the baud rate table.
 *      : lpbk_mode - internal/external.
 *      : protocol - UART ASYNC / UART PPP.
 *      : s_ds - Pointer to the serial data structure.
 * Output: PASSED FAILED
 ******************************************************************************/
int crocus_chan_lpbk_rx (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                         uchar lpbk_mode, uchar protocol,
#ifdef XDMA
                         char *xdma_path,
#endif
                         crocus_serial_ds_t *s_ds)
{
    crocus_init_rx_buffers(s_ds);

    crocus_rx(s_ds);

    /*
     * Check that we received the Data OK.
     */
    if (crocus_check_received_data(s_ds)) {
        crocus_trap(s_ds);
        crocus_cleanup_serial(s_ds);
        crocus_cleanup_buffer(scc_base_addr, s_ds);
        return (FAILED);
    }

    /*
     *  clean up buf
     */
    crocus_cleanup_serial(s_ds);
    crocus_cleanup_buffer(scc_base_addr, s_ds);

    return (PASSED);
}

/******************************************************************************
 * Function: crocus_chan_lpbk_test
 *
 * Description: This function will run one-shot loopback test for a single channel
 *
 * Input: run_mode  - Interrupt or poll.
 *      : scc_base_addr - base address for SCC.
 *      : speed_idx - index to the baud rate table.
 *      : lpbk_mode - internal/external.
 *      : protocol - UART ASYNC / UART PPP.
 *      : s_ds - Pointer to the serial data structure.
 * Output: PASSED FAILED
 ******************************************************************************/
int crocus_chan_lpbk_test (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                         uchar lpbk_mode, uchar protocol,
#ifdef XDMA
                         char *xdma_path,
#endif
                         crocus_serial_ds_t *s_ds)
{
    if (crocus_chan_lpbk_tx(run_mode, scc_base_addr, speed_idx,
                            lpbk_mode, protocol,
#ifdef XDMA
                            xdma_path,
#endif
							s_ds)) {
        return (FAILED);
    }
    
    if (crocus_chan_lpbk_rx(run_mode, scc_base_addr, speed_idx,
                            lpbk_mode, protocol,
#ifdef XDMA
                            xdma_path,
#endif
							s_ds)) {
        return (FAILED);
    }
    
    return (PASSED);
}

/******************************************************************************
 * Function: crocus_async_lpbk_test
 *
 * Description: This function will run loopkback test
 *
 * Input: scc_base_addr - FPGA SCC register base addr
 *        s_ds - Pointer to the serial data structure.
 * Output: PASSED/FAILED
 ******************************************************************************/

int crocus_async_lpbk_test (ulong scc_base_addr, crocus_serial_ds_t *s_ds)
{
    ushort index, lpbk_mode, protocol, run_mode = CROCUS_POLL_MODE;

    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        /*
         * DCD <-> DTR loopback test
         */
        prpass(testpass, "DCD <-> DTR loopback test");
        if (crocus_dcd_dtr_test(scc_base_addr, s_ds) != PASSED) {
            cterr('f',0," Channel [%d], DCD <-> DTR loopback test fail,", s_ds->port_num);
            return (FAILED);
        }
        /*
         * CTS <-> RTS loopback test
         */
        prpass(testpass, "CTS <-> RTS loopback test");
        if (crocus_cts_rts_test(scc_base_addr, s_ds) != PASSED) {
            cterr('f',0," Channel [%d], CTS <-> RTS loopback test fail,", s_ds->port_num);
            return (FAILED);
        }
    }

    /*
     * async internal loopback test
     */
    protocol = CROCUS_ASYNC_UART;
    lpbk_mode = CROCUS_ASYNC_SCC_INT_LOOPBACK;
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = CROCUS_ASYNC_BPS_300; index <= CROCUS_ASYNC_BPS_256000;
             index = index + CROCUS_SPEED_INDEX_INC ) {
            if (!(diagflag_xram & D_MIN_TEST_TIME)) {
                if ((index != CROCUS_ASYNC_BPS_300) && (index != CROCUS_ASYNC_BPS_256000))
                    continue;
            }
            if (crocus_chan_lpbk_test(run_mode, scc_base_addr, index,
                                    lpbk_mode, protocol,
#ifdef XDMA
                                    cur_xdma_path,
#endif
									s_ds)) {
                return (FAILED);
            }
        }
    }

#ifdef ASYNC_PPP
    /*
     * PPP internal loopback test
     */
    lpbk_mode = CROCUS_ASYNC_SCC_INT_LOOPBACK;
    protocol = CROCUS_ASYNC_PPP;
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = CROCUS_ASYNC_BPS_300; index <= CROCUS_ASYNC_BPS_256000;
             index = index + CROCUS_SPEED_INDEX_INC ) {
            if (!(diagflag_xram & D_MIN_TEST_TIME)) {
                if ((index != CROCUS_ASYNC_BPS_300) && (index != CROCUS_ASYNC_BPS_256000))
                    continue;
            }
            if (crocus_chan_lpbk_test(run_mode, scc_base_addr, index,
                                    lpbk_mode, protocol,
#ifdef XDMA
                                    cur_xdma_path,
#endif
									s_ds)) {
                return (FAILED);
            }
        }
    }
#endif
    /*
     * async external loopback test
     */
    protocol = CROCUS_ASYNC_UART;
    lpbk_mode = CROCUS_ASYNC_SCC_EXT_LOOPBACK;
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = CROCUS_ASYNC_BPS_300; index <= CROCUS_ASYNC_BPS_256000;
             index = index + CROCUS_SPEED_INDEX_INC ) {
            if (!(diagflag_xram & D_MIN_TEST_TIME)) {
                if ((index != CROCUS_ASYNC_BPS_300) && (index != CROCUS_ASYNC_BPS_256000))
                    continue;
            }
            if (crocus_chan_lpbk_test(run_mode, scc_base_addr, index,
                                lpbk_mode, protocol,
#ifdef XDMA
                                    cur_xdma_path,
#endif
								s_ds)) {
                return (FAILED);
            }
        }
    }
#ifdef ASYNC_PPP
    /*
     * PPP external loopback test
     */
    protocol = CROCUS_ASYNC_PPP;
    lpbk_mode = CROCUS_ASYNC_SCC_EXT_LOOPBACK;
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = CROCUS_ASYNC_BPS_300; index <= CROCUS_ASYNC_BPS_256000;
             index = index + CROCUS_SPEED_INDEX_INC ) {
            if (!(diagflag_xram & D_MIN_TEST_TIME)) {
                if ((index != CROCUS_ASYNC_BPS_300) && (index != CROCUS_ASYNC_BPS_256000))
                    continue;
            }
            if (crocus_chan_lpbk_test(run_mode, scc_base_addr, index,
                                lpbk_mode, protocol,
#ifdef XDMA
                                    cur_xdma_path,
#endif
								s_ds)) {
                return (FAILED);
            }
        }
    }
#endif
    return (PASSED);
}

/******************************************************************************
 * Function: crocus_config_serial_ds
 *
 * Description: This function will config the serial port
 *
 * Input: run_mode  - Interrupt or poll.
 *      : scc_base_addr - base address for SCC.
 *      : speed_idx - index to the baud rate table.
 *      : lpbk_mode - internal/external.
 *      : protocol - UART ASYNC / UART PPP.
 *      : s_ds - Pointer to the serial data structure.
 *      : des_baud - divider.
 *      : clk_src_bits - clock selection.
 * Output: PASSED / FAILED
 ******************************************************************************/
void crocus_config_serial_ds (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                              uchar lpbk_mode, uchar protocol,
                              crocus_serial_ds_t *s_ds,
#ifdef XDMA
                              char *xdma_path,
#endif
                              ulong *des_baud, ushort *clk_src_bits)
{
    ulong baudrate = 0;
    ulong divider = 1;
    crocus_regs_t * base_addr;

    *clk_src_bits = 0;
    *des_baud = 0;
    base_addr = (crocus_regs_t *) scc_base_addr;

    divider = crocus_async_baud[speed_idx].divider;
    baudrate = crocus_async_baud[speed_idx].baudrate;
    *des_baud = crocus_async_baud[speed_idx].desired_baud;
    *clk_src_bits = crocus_async_baud[speed_idx].clk_src;

    /*
     *  PPP/HDLC/Bisync needs to reserve extra CRC bytes
     *  in the receive buffer.
     */
    if (protocol != CROCUS_ASYNC_UART) {
        s_ds->crc_rx_buf_opt = CRC_RX_BUF_OPT_ON;
    } else {
        s_ds->crc_rx_buf_opt = 0;
    }

	#if 1
	s_ds->buff_size = MAX_CROCUS_SMALL_BUFFER_SIZE;
	#else
    /*
     *  For lower speed, use smaller buffer size to save test time.
     */
    if (speed_idx < CROCUS_LOW_SPEED_INDEX) {
        s_ds->buff_size = MAX_CROCUS_SMALL_BUFFER_SIZE;
    } else {
        s_ds->buff_size = MAX_CROCUS_BUFFER_SIZE;
    }
	#endif

    /*
     *  Fill up serial data struct information
     */
    s_ds->baudrate = baudrate;
    s_ds->divider = divider;
    s_ds->protocol = protocol;
    s_ds->lpbk_mode = lpbk_mode;
    s_ds->speed_idx = speed_idx;
    s_ds->run_mode = run_mode;
    s_ds->base_addr = scc_base_addr;
    s_ds->num_buff = MAX_CROCUS_BUFFERS;
    s_ds->global_regs_addr = (crocus_global_regs_t *) (&( base_addr->global_regs));
    s_ds->ctrl_intr_regs_addr = (crocus_ctrl_intr_regs_t *) (&( base_addr->chan_regs[s_ds->port_num].ctrl_intr_regs));
    s_ds->scc_regs_addr = (crocus_scc_regs_t *) (&( base_addr->chan_regs[s_ds->port_num].scc_regs));
    s_ds->ppp_tx_regs_addr = (crocus_ppp_tx_regs_t *) (&( base_addr->chan_regs[s_ds->port_num].ppp_tx_regs));
    s_ds->ppp_rx_regs_addr = (crocus_ppp_rx_regs_t *) (&( base_addr->chan_regs[s_ds->port_num].ppp_rx_regs));
    s_ds->serial_itf_addr = (crocus_serial_itf_t *) (&( base_addr->chan_regs[s_ds->port_num].serial_itf));
    s_ds->ring_buf_regs_addr = (crocus_ring_buf_regs_t *) (&( base_addr->chan_regs[s_ds->port_num].ring_buf_regs));
    s_ds->tx_buf_addr = malloc(s_ds->num_buff * s_ds->buff_size + CROCUS_CRC_RESERVED);
	#ifdef XDMA
    posix_memalign((void **)&s_ds->rx_buf_addr, 4096 /*alignment */ , (s_ds->num_buff * s_ds->buff_size + CROCUS_CRC_RESERVED) + 4096);
	s_ds->xdma_path = xdma_path;
    #else
	s_ds->rx_buf_addr = malloc(s_ds->num_buff * s_ds->buff_size + CROCUS_CRC_RESERVED);
	#endif
	s_ds->tx_ring_addr = (uchar *) (scc_base_addr + CROCUS_TX_RING_BUFFER_OFFSET + CROCUS_TX_RING_BUFFER_SIZE_PER_CHAN * s_ds->port_num);
    s_ds->rx_ring_addr = (uchar *) (scc_base_addr + CROCUS_RX_RING_BUFFER_OFFSET + CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN * s_ds->port_num);
}

/******************************************************************************
 * Function: crocus_check_received_data
 *
 * Description: This function compares the received data to the data that was
 *              transmitted.
 *
 * Input: s_ds - pointer to serial data structure
 * Output: PASSED or FAILED.
 ******************************************************************************/
ushort crocus_check_received_data (crocus_serial_ds_t *s_ds)
{
    ushort size, ix;
    uchar *tx_data, *rx_data;

    tx_data = (uchar *) s_ds->tx_buf_addr;
    rx_data = (uchar *) s_ds->rx_buf_addr;

    /* CRC two bytes need not be checked */
    if (s_ds->crc_rx_buf_opt)
        size = ( s_ds->num_buff * s_ds->buff_size - CROCUS_CRC_RESERVED );
    else
        size = ( s_ds->num_buff * s_ds->buff_size  );

    for (ix = 0; ix < size; ix++) {
        if (*rx_data != *tx_data) {
            cterr('f',0," Byte %x Data Mismatch. tx = 0x%x rx = 0x%x ", ix,
                  *tx_data, *rx_data);
			printf("\n Chan %d TX buffer : %d", s_ds->port_num, size);
			for (ix = 0; ix < size; ix ++) {
				if (ix % 16 == 0) {
					printf("\n");
				}
				printf("%02x ", s_ds->tx_buf_addr[ix]);
			}
			printf("\n");

			printf(" Chan %d RX buffer : %d", s_ds->port_num, size);
			for (ix = 0; ix < size; ix ++) {
				if (ix % 16 == 0) {
					printf("\n");
				}
				printf("%02x ", s_ds->rx_buf_addr[ix]);
			}
			printf("\n");
            return(FAILED);
        }
        tx_data++;
        rx_data++;
    }

    return(PASSED);
}

/******************************************************************************
 * Function: crocus_tx
 *
 * Description: This function will move data to tx_start
 *
 * Input: s_ds - Pointer to the crocus data structure.
 * Output: None
 ******************************************************************************/
void crocus_tx (crocus_serial_ds_t *s_ds)
{
    uchar *data_ptr, *ring_addr;
    int ix, timeout, size, length;
    uint *dst_ptr, *src_ptr, ppp_header;

    size = ( s_ds->num_buff * s_ds->buff_size );

    timeout = CROCUS_TIMEOUT_5S;
    while(s_ds->ring_buf_regs_addr->tx_start != s_ds->ring_buf_regs_addr->tx_end) {
        msleep(CROCUS_SLEEP_10MS);
        timeout--;
        if (timeout == 0) {
            printf("chan %d: tx wait empty timeout, tx_start = %x, tx_end = %x\n", s_ds->port_num, s_ds->ring_buf_regs_addr->tx_start, s_ds->ring_buf_regs_addr->tx_end);
            return;
        }
    }

    if (diagflag_xram & D_TRACE) {
        printf("chan %d(%s) before tx : tx_start = %x, tx_end = %x, tx_ring_addr = %x, size = %d\n",
            s_ds->port_num,
            ((s_ds->protocol == CROCUS_ASYNC_PPP) ? "PPP" : "UART"),
            s_ds->ring_buf_regs_addr->tx_start,
            s_ds->ring_buf_regs_addr->tx_end,
            s_ds->tx_ring_addr,
            size);
    }

    data_ptr = s_ds->tx_buf_addr;
    ring_addr = s_ds->tx_ring_addr + s_ds->ring_buf_regs_addr->tx_end;
    if (s_ds->protocol == CROCUS_ASYNC_PPP) {
        while (size > 0) {
            ppp_header = CROCUS_PPP_START_FLAG;
            if (size > CROCUS_PPP_MAX_LENGTH) {
                length = CROCUS_PPP_MAX_LENGTH;
            } else {
                length = size;
                ppp_header |= CROCUS_PPP_STATUS_EOF;
            }
            ppp_header |= (length & CROCUS_PPP_LENGTH_MASK);
            size -= length;
            ppp_header = REVERSE_UINT(ppp_header);
            dst_ptr = (uint*)ring_addr;
            src_ptr = (uint*)data_ptr;
            
            *dst_ptr = ppp_header;
            dst_ptr++;
            for (ix = 0; ix < (length / sizeof(uint)); ix++) {
                if (ring_addr >= (s_ds->tx_ring_addr + CROCUS_TX_RING_BUFFER_SIZE_PER_CHAN)) {
                    if (diagflag_xram & D_TRACE) {
                        printf("chan %d tx ring buffer overflow\n", s_ds->port_num);
                    }
                    dst_ptr = (uint*)s_ds->tx_ring_addr;
                    ring_addr = s_ds->tx_ring_addr;
                }
                *dst_ptr = *src_ptr;
                dst_ptr++;
                src_ptr++;
                ring_addr += sizeof(uint);
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d : dst %x, src %x, tx_start %x, ring_addr %x\n", s_ds->port_num, dst_ptr, src_ptr, s_ds->ring_buf_regs_addr->tx_start, ring_addr);
                }
            }
        }
    } else {
        #ifdef ASYNC_4_BYTE_TX_RX
        dst_ptr = (uint*)ring_addr;
        src_ptr = (uint*)data_ptr;
        for (ix = 0; ix < (size / sizeof(uint)); ix++) {
            if (ring_addr >= (s_ds->tx_ring_addr + CROCUS_TX_RING_BUFFER_SIZE_PER_CHAN)) {
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d tx ring buffer overflow\n", s_ds->port_num);
                }
                dst_ptr = (uint*)s_ds->tx_ring_addr;
                ring_addr = s_ds->tx_ring_addr;
            }
            *dst_ptr = *src_ptr;
            dst_ptr++;
            src_ptr++;
            ring_addr += sizeof(uint);
            if (diagflag_xram & D_TRACE) {
                printf("chan %d : dst %x, src %x, tx_start %x, ring_addr %x\n", s_ds->port_num, dst_ptr, src_ptr, s_ds->ring_buf_regs_addr->tx_start, ring_addr);
            }
        }
        #else
        for (ix = 0; ix < size; ix++) {
            if (ring_addr >= (s_ds->tx_ring_addr + CROCUS_TX_RING_BUFFER_SIZE_PER_CHAN)) {
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d tx ring buffer overflow\n", s_ds->port_num);
                }
                ring_addr = s_ds->tx_ring_addr;
            }
            *ring_addr = *data_ptr;
            ring_addr++;
            data_ptr++;
        }
        #endif
    }
    
    if ((ring_addr - s_ds->tx_ring_addr) >= CROCUS_TX_RING_BUFFER_SIZE_PER_CHAN) {
        if (diagflag_xram & D_TRACE) {
            printf("chan %d tx ring buffer overflow\n", s_ds->port_num);
        }
        s_ds->ring_buf_regs_addr->tx_end = 0;
    } else {
        s_ds->ring_buf_regs_addr->tx_end = (ring_addr - s_ds->tx_ring_addr);
    }
    
    timeout = CROCUS_TIMEOUT_5S;
    while(s_ds->ring_buf_regs_addr->tx_start != s_ds->ring_buf_regs_addr->tx_end) {
        msleep(CROCUS_SLEEP_10MS);
        timeout--;
        if (timeout == 0) {
            printf("ch %d: tx wait complete timeout, tx_start = %x, tx_end = %x\n", s_ds->port_num, s_ds->ring_buf_regs_addr->tx_start, s_ds->ring_buf_regs_addr->tx_end);
            return;
        }
    }


    if (diagflag_xram & D_TRACE) {
        printf("chan %d(%s) after tx : tx_start = %x, tx_end = %x, tx_ring_addr = %x, size = %d\n",
            s_ds->port_num,
            ((s_ds->protocol == CROCUS_ASYNC_PPP) ? "PPP" : "UART"),
            s_ds->ring_buf_regs_addr->tx_start,
            s_ds->ring_buf_regs_addr->tx_end,
            s_ds->tx_ring_addr,
            size);
    }
}

#ifdef XDMA
#define RW_MAX_SIZE	0x7ffff000
#include <fcntl.h>
ssize_t read_to_buffer(char *xdma_path, unsigned char *buffer, ulong size,
			ulong base)
{
	ssize_t rc;
	ulong count = 0;
	unsigned char *buf = buffer;
	off_t offset = base;
	int fd;

	fd = open(xdma_path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
				xdma_path, fd);
		return -1;
    }

	while (count < size) {
		ulong bytes = size - count;

		if (bytes > RW_MAX_SIZE)
			bytes = RW_MAX_SIZE;

		rc = lseek(fd, offset, SEEK_SET);
		if (rc != offset) {
			fprintf(stderr, " seek off 0x%lx != 0x%lx.\n",
				rc, offset);
			close(fd);
			return -1;
		}

		/* read data from file into memory buffer */
		rc = read(fd, buf, bytes);
		if (rc != bytes) {
			fprintf(stderr, " R off 0x%lx, 0x%lx != 0x%lx.\n",
				count, rc, bytes);
				perror("read file");
			close(fd);
			return -1;
		}

		count += bytes;
		buf += bytes;
		offset += bytes;
	}
    close(fd);	

	if (count != size) {
		fprintf(stderr, " Read failed 0x%lx != 0x%lx.\n",
				count, size);
		return -1;
	}
	return count;
}
#endif
/******************************************************************************
 * Function: crocus_rx
 *
 * Description: This function will get data from rx_start.
 *
 * Input: s_ds - Pointer to the crocus data structure.
 * Output: None
 ******************************************************************************/
void crocus_rx (crocus_serial_ds_t *s_ds)
{
#ifdef XDMA
	ulong base = 0;
#else
    uchar *data_ptr, *ring_addr;
    int ix;
    uint *dst_ptr, *src_ptr;
#endif
	int timeout, size, length;
	uint expected_end, ppp_header;

    size = ( s_ds->num_buff * s_ds->buff_size );
    timeout = CROCUS_TIMEOUT_10S;
    expected_end = s_ds->ring_buf_regs_addr->rx_start + size;
    if (s_ds->protocol == CROCUS_ASYNC_PPP) {
        expected_end += sizeof(ppp_header);
    }
    if (expected_end >= CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN) {
        expected_end -= CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN;
    }
    while ((s_ds->ring_buf_regs_addr->rx_start == s_ds->ring_buf_regs_addr->rx_end) ||
           (s_ds->ring_buf_regs_addr->rx_end != expected_end)) {
        msleep(CROCUS_SLEEP_10MS);
        timeout--;
        if (timeout == 0) {
            printf("chan %d: rx wait data timeout, rx_start = %x, rx_end = %x, expected_end = %x\n", s_ds->port_num, s_ds->ring_buf_regs_addr->rx_start,
            s_ds->ring_buf_regs_addr->rx_end, expected_end);
            return;
        }
    }

    if (diagflag_xram & D_TRACE) {
        printf("chan %d(%s) before rx : rx_start = %x, rx_end = %x, rx_ring_addr = %x, size = %d\n",
            s_ds->port_num,
            ((s_ds->protocol == CROCUS_ASYNC_PPP) ? "PPP" : "UART"),
            s_ds->ring_buf_regs_addr->rx_start,
            s_ds->ring_buf_regs_addr->rx_end,
            s_ds->rx_ring_addr,
            size);
    }

#ifdef XDMA
	base = (ulong)(CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN * s_ds->port_num) + (ulong)s_ds->ring_buf_regs_addr->rx_start;
	if (size > (CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN - s_ds->ring_buf_regs_addr->rx_start)) {
		length = CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN - s_ds->ring_buf_regs_addr->rx_start;
		if (diagflag_xram & D_TRACE) {
			printf("ch %d: read from DMA size %d, base 0x%x\n", s_ds->port_num, length, base);
		}
		if (read_to_buffer(s_ds->xdma_path, s_ds->rx_buf_addr, length, base) < 0) {
			printf("ch %d: read from DMA fail, base %x\n", s_ds->port_num, base);
			return;
		}
		length = size - length;
		base = CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN * s_ds->port_num;
		if (diagflag_xram & D_TRACE) {
			printf("ch %d: read from DMA size %d, base 0x%x\n", s_ds->port_num, length, base);
		}
		if (read_to_buffer(s_ds->xdma_path, s_ds->rx_buf_addr + length, length, base) < 0) {
			printf("ch %d: read from DMA fail, base %x\n", s_ds->port_num, base);
			return;
		}
	} else {
		if (diagflag_xram & D_TRACE) {
			printf("ch %d: read from DMA size %d, base 0x%x\n", s_ds->port_num, size, base);
		}
		if (read_to_buffer(s_ds->xdma_path, s_ds->rx_buf_addr, size, base) < 0) {
			printf("ch %d: read from DMA fail, base %x\n", s_ds->port_num, base);
			return;
		}
	}
	
	s_ds->ring_buf_regs_addr->rx_start = expected_end;
#else
    data_ptr = s_ds->rx_buf_addr;
    ring_addr = s_ds->rx_ring_addr + s_ds->ring_buf_regs_addr->rx_start;
    if (s_ds->protocol == CROCUS_ASYNC_PPP) {
        ppp_header = *((uint*)ring_addr);
        ppp_header = REVERSE_UINT(ppp_header);
        ring_addr += sizeof(uint);
        while ((size > 0)) {
            if ((ppp_header & CROCUS_PPP_START_FLAG_MASK) != CROCUS_PPP_START_FLAG) {
                printf("chan %d: incorrect PPP start flag %x for RX\n", s_ds->port_num, ppp_header);
                return;
            }
            length = ppp_header & CROCUS_PPP_LENGTH_MASK;
            dst_ptr = (uint*)data_ptr;
            src_ptr = (uint*)ring_addr;
            for (ix = 0; ix < (length / sizeof(uint)); ix++) {
                if (ring_addr >= (s_ds->rx_ring_addr + CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN)) {
                    if (diagflag_xram & D_TRACE) {
                        printf("chan %d rx ring buffer overflow\n", s_ds->port_num);
                    }
                    ring_addr = s_ds->rx_ring_addr;
                    src_ptr = (uint*)s_ds->rx_ring_addr;
                }
                if ((ring_addr - s_ds->rx_ring_addr) == s_ds->ring_buf_regs_addr->rx_end) {
                    if (diagflag_xram & D_TRACE) {
                        printf("chan %d : rx_end %x\n", s_ds->port_num, s_ds->ring_buf_regs_addr->rx_end);
                    }
                    break;
                }
                *dst_ptr = *src_ptr;
                dst_ptr++;
                src_ptr++;
                ring_addr += sizeof(uint);
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d : dst %x, src %x, rx_end %x, ring_addr %x\n", s_ds->port_num, dst_ptr, src_ptr, s_ds->ring_buf_regs_addr->rx_end, ring_addr);
                }
            }
            size -= length;
            if ((ring_addr - s_ds->rx_ring_addr) == s_ds->ring_buf_regs_addr->rx_end) {
                break;
            }
        }
    } else {
        #ifdef ASYNC_4_BYTE_TX_RX
        dst_ptr = (uint*)data_ptr;
        src_ptr = (uint*)ring_addr;
        for (ix = 0; ix < (size / sizeof(uint)); ix++) {
            if (ring_addr >= (s_ds->rx_ring_addr + CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN)) {
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d rx ring buffer overflow\n", s_ds->port_num);
                }
                ring_addr = s_ds->rx_ring_addr;
                src_ptr = (uint*)s_ds->rx_ring_addr;
            }
            if ((ring_addr - s_ds->rx_ring_addr) == s_ds->ring_buf_regs_addr->rx_end) {
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d : rx_end %x\n", s_ds->port_num, s_ds->ring_buf_regs_addr->rx_end);
                }
                break;
            }
            *dst_ptr = *src_ptr;
            dst_ptr++;
            src_ptr++;
            ring_addr += sizeof(uint);
            if (diagflag_xram & D_TRACE) {
                printf("chan %d : dst %x, src %x, rx_end %x, ring_addr %x\n", s_ds->port_num, dst_ptr, src_ptr, s_ds->ring_buf_regs_addr->rx_end, ring_addr);
            }
        }
        #else
        for (ix = 0; ix < size; ix++) {
            if (ring_addr >= (s_ds->rx_ring_addr + CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN)) {
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d rx ring buffer overflow\n", s_ds->port_num);
                }
                ring_addr = s_ds->rx_ring_addr;
                src_ptr = (uint*)s_ds->rx_ring_addr;
            }
            if ((ring_addr - s_ds->rx_ring_addr) == s_ds->ring_buf_regs_addr->rx_end) {
                if (diagflag_xram & D_TRACE) {
                    printf("chan %d : rx_end %x\n", s_ds->port_num, s_ds->ring_buf_regs_addr->rx_end);
                }
                break;
            }
            *data_ptr = *ring_addr;
            data_ptr++;
            ring_addr++;
        }    
        #endif
    }
    if ((ulong)ring_addr & CROCUS_RING_BUFFER_ADDRESS_ALIGN_MASK) { /* 4 bytes alignment */
        ring_addr = (uchar*)(((ulong)ring_addr & ~CROCUS_RING_BUFFER_ADDRESS_ALIGN_MASK) + CROCUS_RING_BUFFER_ADDRESS_ALIGN_OFFSET);
    }

    if ((ring_addr - s_ds->rx_ring_addr) >= CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN) {
        if (diagflag_xram & D_TRACE) {
            printf("chan %d rx ring buffer overflow\n", s_ds->port_num);
        }
        s_ds->ring_buf_regs_addr->rx_start = 0;
    } else {
        s_ds->ring_buf_regs_addr->rx_start = (ring_addr - s_ds->rx_ring_addr);
    }
#endif
    if (diagflag_xram & D_TRACE) {
        printf("chan %d(%s) after rx : rx_start = %x, rx_end = %x, rx_ring_addr = %x, size = %d\n",
            s_ds->port_num,
            ((s_ds->protocol == CROCUS_ASYNC_PPP) ? "PPP" : "UART"),
            s_ds->ring_buf_regs_addr->rx_start,
            s_ds->ring_buf_regs_addr->rx_end,
            s_ds->rx_ring_addr,
            size);
    }
}

/******************************************************************************
 * Function: crocus_tx_rx
 *
 * Description: This function will move data to tx_start and get data from
 *              rx_start.
 *
 * Input: s_ds - Pointer to the crocus data structure.
 * Output: None
 ******************************************************************************/
void crocus_tx_rx (crocus_serial_ds_t *s_ds)
{
    crocus_tx(s_ds);
    crocus_rx(s_ds);
}

/******************************************************************************
 * Function: crocus_init_tx_buffers
 *
 * Description: This function will initializes the tx buffers.
 *              It will fill buffer with Test data.
 *
 * Input: s_ds - Pointer to the crocus data structure.
 * Output: None
 ******************************************************************************/
void crocus_init_tx_buffers (crocus_serial_ds_t *s_ds)
{
    ushort ix, size;
    uchar *data_ptr;
    static uchar data = CROCUS_TX_DATA_START;
	uchar tmp = data;
	data++;

    data_ptr = (uchar *)(s_ds->tx_buf_addr);
    size = (s_ds->num_buff * s_ds->buff_size);

    for (ix = 0; ix < size; ix++)
        *data_ptr++ = tmp++;
}

/******************************************************************************
 * Function: crocus_init_rx_buffers
 *
 * Description: This function will initializes the rx buffers with zeroes.
 *
 * Input: s_ds - Pointer to the crocus data structure.
 * Output: None
 ******************************************************************************/
void crocus_init_rx_buffers (crocus_serial_ds_t *s_ds)
{
    ushort size;
    uchar *data_ptr;

    data_ptr = (uchar *) s_ds->rx_buf_addr;
    size = (s_ds->num_buff * s_ds->buff_size);

    memset(data_ptr, 0, size);
}

/******************************************************************************
 * Function: crocus_cleanup_buffer
 *
 * Description: This function will free up memory allocated
 *
 * Input: s_ds - pointer to serial data structure
 *        scc_base_addr - FPGA SCC register base addr
 * Output: None
 ******************************************************************************/
void crocus_cleanup_buffer (ulong scc_base_addr, crocus_serial_ds_t *s_ds)
{
    free(s_ds->tx_buf_addr);
    free(s_ds->rx_buf_addr);
    s_ds->tx_buf_addr = NULL;
    s_ds->rx_buf_addr = NULL;
}

/******************************************************************************
 * Function: crocus_cleanup_serial
 *
 * Description: This function will clean up the setup for the loopback test.
 *
 * Input:  - s_ds       - pointer to serial data structure
 *           priority   - TX priority
 * Output: None
 ******************************************************************************/
void crocus_cleanup_serial (crocus_serial_ds_t *s_ds)
{
#if 0	// tbd
    /* cleanup async timer */
    s_ds->global_regs_addr->timer_0_1_pgm = 0;
    s_ds->global_regs_addr->timer_2_3_pgm = 0;
#endif
    /* cleanup ppp context area */
    s_ds->ppp_tx_regs_addr->context = 0;
    s_ds->ppp_rx_regs_addr->context = 0;

    crocus_cleanup_mode_bits(s_ds);

    /* cleanup flow control */
    s_ds->scc_regs_addr->flow_cntrl_config = 0x0;
}

/******************************************************************************
 * Function: crocus_cleanup_mode_bits
 *
 * Description: This function cleans up mode in DMA mode register.
 *
 * Input: s_ds - pointer to serial data structure
 * Output: None
 ******************************************************************************/
void crocus_cleanup_mode_bits (crocus_serial_ds_t *s_ds)
{
    s_ds->scc_regs_addr->mode_reg &= ~CROCUS_MODE_MASK;
}

/******************************************************************************
 * Function: crocus_init_serial_ds
 *
 * Description: This function initialized the serial data struct.
 *
 * Input: s_ds - pointer to serial data structure
 * Output: None
 ******************************************************************************/
void crocus_init_serial_ds (crocus_serial_ds_t *s_ds)
{
    s_ds->scc_regs_addr = 0;
    s_ds->ring_buf_regs_addr = 0;
    s_ds->ppp_tx_regs_addr = 0;
    s_ds->ppp_rx_regs_addr = 0;
    s_ds->serial_itf_addr = 0;
    s_ds->global_regs_addr = 0;
    s_ds->base_addr = 0;
    s_ds->speed_idx = 0;
    s_ds->run_mode = 0;
    s_ds->brg_num = s_ds->divider = 0;
    s_ds->lpbk_mode = s_ds->protocol = s_ds->ctrl_id = 0;
    s_ds->num_buff = s_ds->buff_size = 0;
    s_ds->baudrate = s_ds->crc_rx_buf_opt = 0;
    s_ds->crc_enb = CROCUS_MODE_CRC;
#ifdef XDMA
	s_ds->xdma_path = cur_xdma_path;
#endif
}

/******************************************************************************
 * Function: crocus_form_name_buf
 *
 * Description: This function form a name buffer to print
 *
 * Input: pointer to name buffer
 *        s_ds      - pointer to serial data structure
 * Output: PASSED / FAILED
 ******************************************************************************/
int crocus_form_name_buf (char *tmp_name_buf, crocus_serial_ds_t *s_ds)
{
    if (s_ds->lpbk_mode == CROCUS_ASYNC_SCC_INT_LOOPBACK) {
        strcpy (tmp_name_buf, "Int ");
    } else {
        strcpy (tmp_name_buf, "Ext ");
    }

    switch (s_ds->protocol) {
    case CROCUS_ASYNC_UART:
        strcat (tmp_name_buf, "Uart Async ");
        break;
    case CROCUS_ASYNC_PPP:
        strcat (tmp_name_buf, "Uart PPP ");
        break;
    default:
        break;
    }
    return (PASSED);
}

/**********************************************************************
 * Function: crocus_global_reg_test
 *
 * Description: Crocus Global registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_global_reg_test (void)
{
    ulong ctrl_base = (ulong)get_scc_base() + CROCUS_GLOBAL_OFFSET;
    prpass(testpass, "SCC control registers test.");
    if (scc_base == mb_scc_base) {
        return (register_tests(ctrl_base,
                              &crocus_global_reg_table[0]));
    } else {
        return (register_tests(ctrl_base,
                              &crocus_global_reg_table_db[0]));
    }
}

/**********************************************************************
 * Function: crocus_spi_reg_test
 *
 * Description: SPI Flash registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_spi_reg_test (void)
{
    ulong spi_reg_base = (ulong)get_scc_base() + CROCUS_SPI_FLASH_OFFSET;
    prpass(testpass, "SPI Flash registers test.");
    return (register_tests(spi_reg_base, &crocus_spi_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_ctrl_intr_reg_test
 *
 * Description: Control and Interrupt registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_ctrl_intr_reg_test (channel)
{
    ulong ctrl_intr_reg_base = (ulong)get_scc_base() + CROCUS_CTRL_INTR_OFFSET + CROCUS_PER_CHAN_REG_SIZE * channel;
    prpass(testpass, "Control and Interrupt registers test.");
    return (register_tests(ctrl_intr_reg_base, &crocus_ctrl_intr_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_scc_reg_test
 *
 * Description: Serial Communication Controller registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_scc_reg_test (channel)
{
    ulong scc_reg_base = (ulong)get_scc_base() + CROCUS_SCC_OFFSET + CROCUS_PER_CHAN_REG_SIZE * channel;
    prpass(testpass, "Serial Communication Controller registers test.");
    return (register_tests(scc_reg_base, &crocus_scc_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_ppp_tx_reg_test
 *
 * Description: PPP TX registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_ppp_tx_reg_test (channel)
{
    ulong ppp_tx_reg_base = (ulong)get_scc_base() + CROCUS_PPP_TX_OFFSET + CROCUS_PER_CHAN_REG_SIZE * channel;
    prpass(testpass, "PPP TX registers test.");
    return (register_tests(ppp_tx_reg_base, &crocus_ppp_tx_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_ppp_rx_reg_test
 *
 * Description: PPP RX registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_ppp_rx_reg_test (channel)
{
    ulong ppp_rx_reg_base = (ulong)get_scc_base() + CROCUS_PPP_RX_OFFSET + CROCUS_PER_CHAN_REG_SIZE * channel;
    prpass(testpass, "PPP RX registers test.");
    return (register_tests(ppp_rx_reg_base, &crocus_ppp_rx_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_serial_itf_reg_test
 *
 * Description: Serial Line Interafce registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_serial_itf_reg_test (channel)
{
    ulong serial_itf_reg_base = (ulong)get_scc_base() + CROCUS_SERIAL_INTERFACE_OFFSET + CROCUS_PER_CHAN_REG_SIZE * channel;
    prpass(testpass, "Serial Line Interafce registers test.");
    return (register_tests(serial_itf_reg_base, &crocus_serial_itf_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_ring_buf_reg_test
 *
 * Description: Ring Buffer Control registers test.
 *
 * Inputs:  None
 * Outputs: PASSED or FAILED
 ***********************************************************************/
static int crocus_ring_buf_reg_test (channel)
{
    ulong ring_buf_reg_base = (ulong)get_scc_base() + CROCUS_RING_BUFFER_CTRL_OFFSET + CROCUS_PER_CHAN_REG_SIZE * channel;
    prpass(testpass, "Ring Buffer Control registers test.");
    return (register_tests(ring_buf_reg_base, &crocus_ring_buf_reg_table[0]));
}

/**********************************************************************
 * Function: crocus_async_reg_test
 *
 * Description: Serial Channel Register Test for Crocus.
 *
 * Inputs:  Number of Channels
 * Outputs: PASSED or FAILED
 **********************************************************************/
int crocus_async_reg_test (int channel)
{
    testname("Async Register");

    if ((crocus_global_reg_test() != PASSED) ||
        (crocus_spi_reg_test() != PASSED) ||
        (crocus_ctrl_intr_reg_test(channel) != PASSED) ||
        (crocus_scc_reg_test(channel) != PASSED) ||
        (crocus_ppp_tx_reg_test(channel) != PASSED) ||
        (crocus_ppp_rx_reg_test(channel) != PASSED) ||
        (crocus_serial_itf_reg_test(channel) != PASSED) ||
        (crocus_ring_buf_reg_test(channel) != PASSED)) {
            cterr('f', 0, "Async register test failed.");
            return (FAILED);
    }

    prpass(testpass, "Async Register test passed");
    return (PASSED);
}

/*****************************************************************************
 * Function: modem_intr_test_one
 *
 * Description: This function will test modem interrupt for one setting.
 *
 * Input:  chan - channel number
 *         type - type of interrupt
 * Output: PASSED/FAILED
 *****************************************************************************/
static int modem_intr_test_one (int chan, ushort type)
{
    crocus_regs_t *base_addr;
    crocus_ctrl_intr_regs_t *ctrl_intr_regs;
    crocus_serial_itf_t *serial_regs;
    uint data, modem_intr_status_expected;
    int rc = PASSED;
    int ix;

    prpass(testpass, "modem intr chan[%d] type[%x]", chan, type);
    base_addr = (crocus_regs_t *)get_scc_base();
    ctrl_intr_regs = &base_addr->chan_regs[chan].ctrl_intr_regs;
    serial_regs = &base_addr->chan_regs[chan].serial_itf;

    /* do a dummy read, clear pending interrupt */
    data = ctrl_intr_regs->intr_status;
    data = serial_regs->modem_intr_status;

    /* enable modem interrupt */
    ctrl_intr_regs->intr_mask |= CROCUS_MODEM_ENB;

    if (diagflag_xram & D_TRACE) {
        printf("\nEnable interrupt: "
            "ctrl_intr_regs->intr_mask @%#x = %#x\n",
            &ctrl_intr_regs->intr_mask,
            ctrl_intr_regs->intr_mask);
    }

    data = ctrl_intr_regs->intr_status;
    if (data & CROCUS_MODEM_STATUS) {
        cterr('f', 0,
              "Interrupt bit has been asserted before force IRQ. "
              "intr status = %#x\n",
              ctrl_intr_regs->intr_status);
        return (FAILED);
    }

    /* enable interrupt*/
    switch (type) {
    case CROCUS_DCD_FORCE_IRQ:
        modem_intr_status_expected = CROCUS_DCD_STATUS;
        serial_regs->flow_cntrl |= CROCUS_DCD_ENABLE;
        break;
    case CROCUS_DSR_FORCE_IRQ:
        modem_intr_status_expected = CROCUS_DSR_STATUS;
        serial_regs->flow_cntrl |= CROCUS_DSR_ENABLE;
        break;
    case CROCUS_CTS_FORCE_IRQ:
        modem_intr_status_expected = CROCUS_CTS_STATUS;
        serial_regs->flow_cntrl |= CROCUS_CTS_ENABLE;
        break;
    case CROCUS_CABLEID_FORCE_IRQ:
        modem_intr_status_expected = CROCUS_CABLEID_STATUS;
        serial_regs->flow_cntrl |= CROCUS_CABLEID_ENABLE;
        break;
    default:
        cterr('f', 0, "Incorrect force type");
        return (FAILED);
    }
    
    data = serial_regs->modem_intr_status;
    if (data & modem_intr_status_expected) {
        cterr('f', 0,
              "Interrupt bit has been asserted before force IRQ. "
              "serial_regs->modem_intr_status = %#x\n",
              data);
        return (FAILED);
    }

    if (diagflag_xram & D_TRACE) {
        printf("Enable interrupt: serial_regs->flow_cntrl @%#x = %#x\n",
            &serial_regs->flow_cntrl, serial_regs-> flow_cntrl);
        printf("Before force interrupt : ctrl_intr_regs->intr_status @%#x = %#x\n",
            &ctrl_intr_regs->intr_status, ctrl_intr_regs->intr_status);
    }

    serial_regs->modem_intr_status |= type;   /* fire up force type */

    /* Wait 1 seconds here to let interrupt to be serviced. */
    for (ix = 0; ix < CROCUS_TIMEOUT_1S; ix++) {
        if ((ctrl_intr_regs->intr_status & CROCUS_MODEM_STATUS) && 
            (serial_regs->modem_intr_status & modem_intr_status_expected)) {
                break;
        } else {
            msleep(CROCUS_SLEEP_10MS);
        }
    }

    if (ix == CROCUS_TIMEOUT_1S) {
        cterr('f', 0,
              "Timeout waiting for interrupt. "
              "intr status = %#x, serial_regs->modem_intr_status = %#x\n",
              ctrl_intr_regs->intr_status, serial_regs->modem_intr_status);
        rc = FAILED;
    } else {
        rc = PASSED;
        if (diagflag_xram & D_TRACE) {
            printf("After force interrupt : ctrl_intr_regs->intr_status @%#x = %#x, serial_regs->modem_intr_status @%#x = %#x\n",
                &ctrl_intr_regs->intr_status, ctrl_intr_regs->intr_status,
                &serial_regs->modem_intr_status, serial_regs->modem_intr_status);
        }
    }

    serial_regs->flow_cntrl &= ~(CROCUS_DCD_ENABLE | CROCUS_DSR_ENABLE |
                              CROCUS_CTS_ENABLE | CROCUS_CABLEID_ENABLE);
    serial_regs->modem_intr_status &= ~type;
    ctrl_intr_regs->intr_mask &= ~(CROCUS_MODEM_ENB);

    return (rc);
}

/*****************************************************************************
 * Function: ring_intr_test_one
 *
 * Description: This function will test Ring buffer interrupt for one setting.
 *
 * Input:  chan - channel number
 * Output: PASSED FAILED
 **********************************************************************/
static int ring_intr_test_one (int chan)
{
    crocus_regs_t *base_addr;
    crocus_ctrl_intr_regs_t *ctrl_intr_regs;
    crocus_ring_buf_regs_t *ring_buf_regs;
    crocus_serial_ds_t *s_ds;
    uint data;
    int rc = PASSED;
    int ix;

    prpass(testpass, "ring buffer intr chan[%d] ", chan);
    base_addr = (crocus_regs_t *)get_scc_base();
    ctrl_intr_regs = &base_addr->chan_regs[chan].ctrl_intr_regs;
    ring_buf_regs = &base_addr->chan_regs[chan].ring_buf_regs;

    s_ds = &crocus_serial_ds[chan];
    s_ds->port_num = chan;
	crocus_init_serial_ds(s_ds);

    /* do a dummy read, clear pending interrupt */
    data = ctrl_intr_regs->intr_status;
    data = ring_buf_regs->intr_status;

    /* enable ring interrupt */
    ctrl_intr_regs->intr_mask |= CROCUS_RING_ENB;

    if (diagflag_xram & D_TRACE) {
        printf("\nEnable interrupt: "
            "ctrl_intr_regs->intr_mask @%#x = %#x\n",
            &ctrl_intr_regs->intr_mask,
            ctrl_intr_regs->intr_mask);
    }
    
    if (diagflag_xram & D_TRACE) {
        printf("Before Enable interrupt : ctrl_intr_regs->intr_status @%#x = %#x, ring_buf_regs->intr_status @%#x = %#x\n",
            &ctrl_intr_regs->intr_status, ctrl_intr_regs->intr_status,
            &ring_buf_regs->intr_status, ring_buf_regs->intr_status);
    }
    
    data = ctrl_intr_regs->intr_status;
    if (data & CROCUS_RING_STATUS) {
        cterr('f', 0,
              "Interrupt bit has been asserted before TX. "
              "intr status = %#x\n",
              ctrl_intr_regs->intr_status);
        return (FAILED);
    }

    data = ring_buf_regs->intr_status;
    if (ring_buf_regs->intr_status && (CROCUS_RING_BUFFER_TX_INTR_ENB | CROCUS_RING_BUFFER_RX_INTR_ENB)) {
        cterr('f', 0,
              "Interrupt bit has been asserted before TX. "
              "ring_buf_regs->intr_status = %#x\n",
              data);
        return (FAILED);
    }

    /* enable interrupt */
    ring_buf_regs->intr_mask |= (CROCUS_RING_BUFFER_TX_INTR_ENB | CROCUS_RING_BUFFER_RX_INTR_ENB);

    if (diagflag_xram & D_TRACE) {
        printf("Before TX : ring_buf_regs->intr_status @%#x = %#x\n",
            &ring_buf_regs->intr_status, ring_buf_regs->intr_status);
    }
    
    /* run loopback tx */
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        if (crocus_chan_lpbk_tx(CROCUS_INT_MODE, (ulong)base_addr,
            CROCUS_ASYNC_BPS_128K, CROCUS_ASYNC_SCC_INT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
            cur_xdma_path,
#endif
			s_ds)) {
            cterr('f', 0, "TX interrupt mode failed");
            return (FAILED);
        }
    } else {
        if (crocus_chan_lpbk_tx(CROCUS_INT_MODE, (ulong)base_addr,
            CROCUS_ASYNC_BPS_128K, CROCUS_ASYNC_SCC_EXT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
            cur_xdma_path,
#endif
			s_ds)) {
            cterr('f', 0, "TX interrupt mode failed");
            return (FAILED);
        }
    }

    /* Wait 1 seconds here to let interrupt to be serviced. */
    for (ix = 0; ix < CROCUS_TIMEOUT_1S; ix++) {
        if ((ctrl_intr_regs->intr_status & CROCUS_RING_STATUS) &&
            (ring_buf_regs->intr_status && CROCUS_RING_BUFFER_TX_INTR_ENB)) {
            break;
        } else {
            msleep(CROCUS_SLEEP_10MS);
        }
    }
    
    if (ix == CROCUS_TIMEOUT_1S) {
        cterr('f', 0,
              "Timeout waiting for tx interrupt."
              "intr status = %#x, ring_buf_regs->intr_status = %#x\n",
              ctrl_intr_regs->intr_status, ring_buf_regs->intr_status);
        rc = FAILED;
    } else {
        if (diagflag_xram & D_TRACE) {
            printf("After TX : ctrl_intr_regs->intr_status @%#x = %#x, ring_buf_regs->intr_status @%#x = %#x\n",
                &ctrl_intr_regs->intr_status, ctrl_intr_regs->intr_status,
                &ring_buf_regs->intr_status, ring_buf_regs->intr_status);
        }
    }
    
    /* Wait 1 seconds here to let interrupt to be serviced. */
    for (ix = 0; ix < CROCUS_TIMEOUT_1S; ix++) {
        if (ring_buf_regs->intr_status && CROCUS_RING_BUFFER_RX_INTR_ENB) {
            break;
        } else {
            msleep(CROCUS_SLEEP_10MS);
        }
    }
    
    if (ix == CROCUS_TIMEOUT_1S) {
        cterr('f', 0,
              "Timeout waiting for rx interrupt."
              "intr status = %#x\n",
              ring_buf_regs->intr_status);
        rc = FAILED;
    } else {
        if (diagflag_xram & D_TRACE) {
            printf("Befor RX : ring_buf_regs->intr_status @%#x = %#x\n",
                &ring_buf_regs->intr_status, ring_buf_regs->intr_status);
        }
    }
    
    /* run loopback rx */
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        if (crocus_chan_lpbk_rx(CROCUS_INT_MODE, (ulong)base_addr,
            CROCUS_ASYNC_BPS_128K, CROCUS_ASYNC_SCC_INT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
            cur_xdma_path,
#endif
			s_ds)) {
            cterr('f', 0, "RX interrupt mode failed");
            return (FAILED);
        }
    } else {
        if (crocus_chan_lpbk_rx(CROCUS_INT_MODE, (ulong)base_addr,
            CROCUS_ASYNC_BPS_128K, CROCUS_ASYNC_SCC_EXT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
            cur_xdma_path,
#endif
			s_ds)) {
            cterr('f', 0, "RX interrupt mode failed");
            return (FAILED);
        }
    }

    /* disable interrupt */
    ring_buf_regs->intr_mask &= ~(CROCUS_RING_BUFFER_TX_INTR_ENB | CROCUS_RING_BUFFER_RX_INTR_ENB);
    ctrl_intr_regs->intr_mask &= ~(CROCUS_RING_ENB);

    return (rc);
}

#ifdef PCIE_INTR
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <errno.h>
int
uio_enable_intr1 ()
{
    int err;
    int configfd;
    char config[80];
                  
    unsigned char command_high;

    sprintf(config, "/sys/class/uio/uio1/device/config");
    
    configfd = open(config, O_RDWR);
    if (configfd < 0) {
        printf("can't open %s\n", config);
        perror("config open:");
        return errno;
    }

    /* Read and cache command value */
    err = pread(configfd, &command_high, 1, 5);
    if (err != 1) {
        perror("uio_enable_intr: command config read:");
        return errno;
    }
    command_high &= ~0x4;

    /* Re-henable interrupts. */
    err = pwrite(configfd, &command_high, 1, 5);
    if (err != 1) {
        perror("uio_enable_intr: config write:");
        return errno;
    }

    close(configfd);
    return 0;
}

static void *crocus_rx_intr(void *data)
{
    int fd = -1, rc, count;
    fd_set set;
    struct timeval timeout;
    
    fd = open("/dev/uio1", O_RDONLY);
    if (fd < 0) {
        perror("open /dev/uio1 failed\n");
        goto rx_intr_exit;
    }
    
    FD_ZERO(&set);
    FD_SET(fd, &set);
    
    timeout.tv_sec  = 5;
    timeout.tv_usec = 0;
    
    if (uio_enable_intr1() != 0) {
        goto rx_intr_exit;
    }
    
    rc = select(fd + 1, &set, NULL, NULL, &timeout);
    if (rc == -1) {
        perror("select");
        goto rx_intr_exit;
    } else if (rc == 0) {
        printf("select timeout\n");
        goto rx_intr_exit;
    }
    
    rc = read(fd, &count, sizeof(count));
    if (rc < 0) {
        perror("read");
        goto rx_intr_exit;
    }
    
    printf("read from /dev/uio1: %d\n", count);    
    
rx_intr_exit:
    if (fd >= 0) {
        close(fd);
    }
    pthread_exit(NULL);
}
#endif
/**********************************************************************
 * Function: crocus_async_int_test
 *
 * Description: Serial Channel Interrupt Test for Crocus.
 *
 * Inputs:  Number of Channels
 * Outputs: PASSED or FAILED
 **********************************************************************/
int crocus_async_int_test (int channel)
{
    testname("Async SCC Interrupt");
#ifdef PCIE_INTR
    pthread_t thread;
    if (pthread_create(&thread, NULL, crocus_rx_intr, 0)) {
        printf("pthread_create rx intr failed \n");
        exit(-1);            
        return (FAILED);
    }
#endif
    if ((modem_intr_test_one(channel, CROCUS_DCD_FORCE_IRQ) != PASSED)) {
        cterr('f', 0, "Force DCD Interrupt test failed.");
        return (FAILED);
    }
    if ((modem_intr_test_one(channel, CROCUS_DSR_FORCE_IRQ) != PASSED)) {
        cterr('f', 0, "Force DSR Interrupt test failed.");
        return (FAILED);
    }
    if ((modem_intr_test_one(channel, CROCUS_CTS_FORCE_IRQ) != PASSED)) {
        cterr('f', 0, "Force CTS Interrupt test failed.");
        return (FAILED);
    }
    if ((modem_intr_test_one(channel, CROCUS_CABLEID_FORCE_IRQ) != PASSED)) {
        cterr('f', 0, "Force Cable ID Interrupt test failed.");
        return (FAILED);
    }
    if ((ring_intr_test_one(channel) != PASSED)) {
        cterr('f', 0, "Ring Buffer TX/RX Interrupt test failed.");
    }
#ifdef PCIE_INTR    
    pthread_join(thread, NULL);
#endif
    prpass(testpass, "Async SCC Interrupt test passed");
    return (PASSED);
}

/******************************************************************************
 * Function: crocus_dcd_dtr_test
 *
 * Description: This function is DCD<->DTR path loopback test
 *
 * Input: scc_base_addr - FPGA SCC register base addr
 *        s_ds - pointer to serial data structure
 * Output: PASSED / FAILED
 ******************************************************************************/
int crocus_dcd_dtr_test (ulong scc_base_addr, crocus_serial_ds_t *s_ds)
{
    int rc;        
    crocus_regs_t * base_addr;

    base_addr = (crocus_regs_t *) scc_base_addr;
    base_addr->chan_regs[s_ds->port_num].serial_itf.modem_cntrl |= CROCUS_MODEM_CTRL_DTR;
    msleep(CROCUS_SLEEP_500MS);
    
    if ((base_addr->chan_regs[s_ds->port_num].serial_itf.flow_cntrl & CROCUS_FLOW_CTRL_DCD_STATUS) == 0) {
        rc = FAILED;
    } else {
        rc = PASSED;
    }

    base_addr->chan_regs[s_ds->port_num].serial_itf.modem_cntrl &= ~(CROCUS_MODEM_CTRL_DTR);
    return rc;
}

/******************************************************************************
 * Function: crocus_cts_rts_test
 *
 * Description: This function is CTS<->RTS path loopback test
 *
 * Input: scc_base_addr - FPGA SCC register base addr
 *        s_ds - pointer to serial data structure
 * Output: PASSED / FAILED
 ******************************************************************************/
int crocus_cts_rts_test (ulong scc_base_addr, crocus_serial_ds_t *s_ds)
{
    int rc;
    crocus_regs_t * base_addr;

    base_addr = (crocus_regs_t *) scc_base_addr;
    base_addr->chan_regs[s_ds->port_num].serial_itf.modem_cntrl |= CROCUS_MODEM_CTRL_RTS;
    msleep(CROCUS_SLEEP_500MS);

    if ((base_addr->chan_regs[s_ds->port_num].serial_itf.flow_cntrl & CROCUS_FLOW_CTRL_CTS_STATUS) == 0) {
        rc = FAILED;
    } else {
        rc = PASSED;
    }

    base_addr->chan_regs[s_ds->port_num].serial_itf.modem_cntrl &= ~(CROCUS_MODEM_CTRL_RTS);
    return rc;
}

int crocus_async_chan_lpbk_test2 (int channel, int baud)
{
    ulong scc_base_addr;
    crocus_serial_ds_t *s_ds;
    int baud_ix;

    testname("Async SCC Loopback test");

    scc_base_addr = get_scc_base();

    /* Get serial data structure and init it */
    s_ds = &crocus_serial_ds[channel];

    s_ds->port_num = channel;
    crocus_init_serial_ds(s_ds);

    switch (baud) {
    case 300:
        baud_ix = CROCUS_ASYNC_BPS_300;
        break;
    case 600:
        baud_ix = CROCUS_ASYNC_BPS_600;
        break;
    case 1200:
        baud_ix = CROCUS_ASYNC_BPS_1200;
        break;
    case 2400:
        baud_ix = CROCUS_ASYNC_BPS_2400;
        break;
    case 4800:
        baud_ix = CROCUS_ASYNC_BPS_4800;
        break;
    case 9600:
        baud_ix = CROCUS_ASYNC_BPS_9600;
        break;
    case 14400:
        baud_ix = CROCUS_ASYNC_BPS_14400;
        break;
    case 19200:
        baud_ix = CROCUS_ASYNC_BPS_19200;
        break;
    case 28800:
        baud_ix = CROCUS_ASYNC_BPS_28800;
        break;
    case 32000:
        baud_ix = CROCUS_ASYNC_BPS_32K;
        break;
    case 38400:
        baud_ix = CROCUS_ASYNC_BPS_38400;
        break;
    case 48000:
        baud_ix = CROCUS_ASYNC_BPS_48K;
        break;
    case 56000:
        baud_ix = CROCUS_ASYNC_BPS_56K;
        break;
    case 57600:
        baud_ix = CROCUS_ASYNC_BPS_57600;
        break;
    case 64000:
        baud_ix = CROCUS_ASYNC_BPS_64K;
        break;
    case 72000:
        baud_ix = CROCUS_ASYNC_BPS_72K;
        break;
    case 115200:
        baud_ix = CROCUS_ASYNC_BPS_115200;
        break;
    case 128000:
        baud_ix = CROCUS_ASYNC_BPS_128K;
        break;
    case 230400:
        baud_ix = CROCUS_ASYNC_BPS_230400;
        break;
    case 256000:
        baud_ix = CROCUS_ASYNC_BPS_256000;
        break;
    default:
        printf("invalid baud rate %d\n", baud);
        return (FAILED);
    }
    if (crocus_chan_lpbk_test(CROCUS_POLL_MODE, (ulong)scc_base_addr,
        baud_ix, CROCUS_ASYNC_SCC_EXT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
        cur_xdma_path,
#endif
		s_ds)) {
        return (FAILED);
    }

    return (PASSED);
}

int lpbk_run = 0;
void *crocus_async_chan_lpbk_thread_mb(void *data)
{
    crocus_serial_ds_t *s_ds = (crocus_serial_ds_t *)data;
    
    while (lpbk_run) {
        if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
            if (crocus_chan_lpbk_test(CROCUS_POLL_MODE, (ulong)mb_scc_base,
                CROCUS_ASYNC_BPS_256000, CROCUS_ASYNC_SCC_INT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
                mb_xdma_path,
#endif
				s_ds)) {
                pthread_exit(NULL);
            }
        } else {
            if (crocus_chan_lpbk_test(CROCUS_POLL_MODE, (ulong)mb_scc_base,
                CROCUS_ASYNC_BPS_256000, CROCUS_ASYNC_SCC_EXT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
                mb_xdma_path,
#endif
				s_ds)) {
                pthread_exit(NULL);
            }
        }
    }
    pthread_exit(NULL);
}

void *crocus_async_chan_lpbk_thread_db(void *data)
{
    crocus_serial_ds_t *s_ds = (crocus_serial_ds_t *)data;

    while (lpbk_run) {
        if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
            if (crocus_chan_lpbk_test(CROCUS_POLL_MODE, (ulong)db_scc_base,
                CROCUS_ASYNC_BPS_256000, CROCUS_ASYNC_SCC_INT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
                db_xdma_path,
#endif
				s_ds)) {
                pthread_exit(NULL);
            }
        } else {
            if (crocus_chan_lpbk_test(CROCUS_POLL_MODE, (ulong)db_scc_base,
                CROCUS_ASYNC_BPS_256000, CROCUS_ASYNC_SCC_EXT_LOOPBACK, CROCUS_ASYNC_UART,
#ifdef XDMA
                db_xdma_path,
#endif
				s_ds)) {
                pthread_exit(NULL);
            }
        }
    }
    pthread_exit(NULL);
}

static pid_t pid = 0;

static void signal_handler(int sig){
    if (sig == SIGUSR1) {
        lpbk_run = 0;
    }
}

static void init_signals(void){
    struct sigaction sigact;
    
    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
//    sigaddset(&sigact.sa_mask,SIGTSTP);
    sigact.sa_flags = 0;
    sigaction(SIGUSR1, &sigact, (struct sigaction *)NULL);
    sigaction(SIGTSTP, &sigact, (struct sigaction *)NULL);
}

int crocus_async_chan_lpbk_all_start (int dummy)
{
    int ix;
    crocus_serial_ds_t *s_ds;
    pthread_t mb_threads[MB_ASYNC_CHAN_NUM];
    pthread_t db_threads[DB_ASYNC_CHAN_NUM];
    crocus_serial_ds_t mb_crocus_serial_ds[MB_ASYNC_CHAN_NUM];
    crocus_serial_ds_t db_crocus_serial_ds[DB_ASYNC_CHAN_NUM];


    if (pid) {
        printf("%s() there is an child exist\n");
        return (FAILED);
    }
    
    pid = fork();
    if (pid == -1) {
        printf("%s() fork() failed\n", __func__);
        pid = 0;
        return (FAILED);
    }
    
    if (pid > 0) { /* Parent */
        return (PASSED);
    }

    /* Child */
    init_signals();
    get_scc_base_addr(1);
    lpbk_run = 1;
    
    for (ix = 0; ix < MB_ASYNC_CHAN_NUM; ix++) {
        s_ds = &mb_crocus_serial_ds[ix];
        s_ds->port_num = ix;
        crocus_init_serial_ds(s_ds);
        if (pthread_create(&mb_threads[ix], NULL, crocus_async_chan_lpbk_thread_mb, (void *)s_ds)) {
            printf("pthread_create for mb %d failed \n", ix);
            exit(-1);            
            return (FAILED);
        }
    }
    
    if (has_daughter_card(0)) {
        get_scc_base_addr(0);
        for (ix = 0; ix < DB_ASYNC_CHAN_NUM; ix++) {
            s_ds = &db_crocus_serial_ds[ix];
            s_ds->port_num = ix;
            crocus_init_serial_ds(s_ds);
            if (pthread_create(&db_threads[ix], NULL, crocus_async_chan_lpbk_thread_db, (void *)s_ds)) {
                printf("pthread_create for db %d failed \n", ix);
                exit(-1);
                return (FAILED);
            }
        }
    }
    
    for (ix = 0; ix < MB_ASYNC_CHAN_NUM; ix++) {
        pthread_join(mb_threads[ix], NULL);
        printf("stop mb %d\n", ix);
    }
    
    if (has_daughter_card(0)) {
        for (ix = 0; ix < DB_ASYNC_CHAN_NUM; ix++) {
            pthread_join(db_threads[ix], NULL);
            printf("stop db %d\n", ix);
        }
    }
    exit(0);
}

int crocus_async_chan_lpbk_all_stop (int dummy)
{
    if (pid) {
        kill(pid, SIGUSR1);
    }
    pid = 0;
    
    return (PASSED);
}

/**********************************************************************
 * Function: show_async_fpga_ver
 *
 * Description: show ASYNC FPGA Version
 *
 * Inputs:  None.
 * Outputs: None.
 ***********************************************************************/
int crocus_show_fpga_ver (int opt)
{
    crocus_global_regs_t *global_regs;
    crocus_regs_t * base_addr;

    get_scc_base_addr(1);
    
    base_addr = (crocus_regs_t *)mb_scc_base;
    global_regs = &base_addr->global_regs;
    printf("Main board Crocus version : %08X\n", global_regs->revision_ID);
    
    if (has_daughter_card(0)) {
        get_scc_base_addr(0);
        base_addr = (crocus_regs_t *)db_scc_base;
        global_regs = &base_addr->global_regs;
        printf("Daughter card Crocus version : %08X\n", global_regs->revision_ID);
    }
    
    return (PASSED);
}

/******** History ********
$Log: crocus_scc_diag.c,v $
Revision 1.2  2019/12/11 10:10:27  lucywang
Merged Nanook to main trunk


$Endlog$
*/

