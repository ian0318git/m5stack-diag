/* $Id: plugser_scc_diag.c,v 1.4 2018/08/02 09:35:01 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/plugser_scc_diag.c,v $
 *------------------------------------------------------------------
 *
 * plugser_scc_diag.c - Pluggable Serial Channel Controller diag function.
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "types.h"
#include "strings.h"
#include "error.h"
#include "cpu.h"
#include "common.h"
#include "nvmonvars.h"
#include "free.h"
#include "common_utils.h"
#include "prince_reg.h"
#include "reva_def.h"
#include "prince_scc_struct.h"
#include "plugser_new_fpga_ver.h"

#define LOGICAL_ANALYZER_TRIGGER    (0x8000)  /* logical analyzer trigger bit */
#define FREQ_CNT_PORT_SELECT        (0xa)     /* Frequency Counter Select Register */

/***********************************************************************
 *                    Functions Declaration
 ************************************************************************/
int plugser_sync_chan_lpbk_test(int);
void plugser_config_serial_ds(uchar, ulong, ushort, uchar, uchar, uchar,
                              prince_serial_ds_t *, int, ulong *, ushort *);
int plugser_chan_lpbk_test(uchar, ulong, ushort, uchar, uchar, uchar,
                           prince_serial_ds_t *, int);
ushort plugser_start_dma_tx(prince_serial_ds_t *, int);
ushort plugser_check_dma_received_data(prince_serial_ds_t *);
void plugser_cleanup_serial(prince_serial_ds_t *, int);
void plugser_trap(prince_serial_ds_t *, int);
void plugser_init_rx_dma_buffers(prince_serial_ds_t *);
void plugser_setup_dma_reg(prince_serial_ds_t *);
void plugser_init_tx_dma_buffers(prince_serial_ds_t *, uchar, uchar);
void plugser_set_async_timer(prince_serial_ds_t *, ushort);
void plugser_init_serial_ds(prince_serial_ds_t *);
void plugser_cleanup_dma_ring(ulong , prince_serial_ds_t *);
void plugser_init_tx_dma_ring(uchar, ulong, prince_serial_ds_t *);
void plugser_init_rx_dma_ring(uchar, ulong, prince_serial_ds_t *);
static ushort tx_status, rx_status;

/*===================================================================*
 *                    extern functions                               *
 *===================================================================*/
extern int flush_io_wb(void);
extern void msleep(int msecs);
extern char getchar();


/*===================================================================*
 *                             Globals                               *
 *===================================================================*/
/* Data Structure for channel loopback test */
prince_serial_ds_t plugser_serial_ds[MAX_CH_NUM];

/*
 * Sync Clock Baud rate tables 
 */
prince_baud_t plugser_sync_baud[] = {
    { PRINCE_SYNC_BPS_NONE,      300,      300,    840, PRINCE_MODEM_BRG_252K },
    { PRINCE_SYNC_BPS_300,       300,      300,    840, PRINCE_MODEM_BRG_252K },
    { PRINCE_SYNC_BPS_600,       600,      600,    420, PRINCE_MODEM_BRG_252K },
    { PRINCE_SYNC_BPS_1200,     1200,     1200,    210, PRINCE_MODEM_BRG_252K },
    { PRINCE_SYNC_BPS_2400,     2400,     2400,    840, PRINCE_MODEM_BRG_2M },
    { PRINCE_SYNC_BPS_4800,     4800,     4755,    420, PRINCE_MODEM_BRG_2M },
    { PRINCE_SYNC_BPS_9600,     9600,     9600,    210, PRINCE_MODEM_BRG_2M },
    { PRINCE_SYNC_BPS_14400,   14400,    14400,    140, PRINCE_MODEM_BRG_2M },
    { PRINCE_SYNC_BPS_19200,   19200,    19200,    105, PRINCE_MODEM_BRG_2M },
    { PRINCE_SYNC_BPS_28800,   28800,    28800,     70, PRINCE_MODEM_BRG_2M },
    { PRINCE_SYNC_BPS_32K,     32000,    32000,   1008, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_38400,   38400,    38400,    840, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_48K,     48000,    48000,    672, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_56K,     56000,    56000,    576, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_57600,   57600,    57600,    560, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_64K,     64000,    64000,    504, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_72K,     72000,    72000,    448, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_115200, 115200,   115200,    280, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_128K,   128000,   128000,    252, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_230400, 230400,   230400,    140, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_256K,   256000,   252000,    126, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_512K,   512000,   504000,     63, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_1024K, 1024000,  1008000,     32, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_2016K, 2016000,  2016000,     16, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_2048K, 2048000,  2016000,     16, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_4032K, 4032000,  4032000,      8, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_8064K, 8064000,  8064000,      4, PRINCE_MODEM_BRG_32M },
    { PRINCE_SYNC_BPS_10M,  10000000, 10000000,      1, PRINCE_MODEM_BRG_10M }
};

/*
 *  ASync Clock Baud rate tables, 8x baud clock output 
 */
prince_baud_t plugser_async_baud[] = {
    { REVA_AS_BPS_NONE,     300,    2400,      651, PRINCE_MODEM_BRG_2M },
    { REVA_AS_BPS_300,      300,    2400,      840, PRINCE_MODEM_BRG_2M },
    { REVA_AS_BPS_600,      600,    4800,      420, PRINCE_MODEM_BRG_2M },
    { REVA_AS_BPS_1200,    1200,    9600,      210, PRINCE_MODEM_BRG_2M },
    { REVA_AS_BPS_2400,    2400,   19200,      105, PRINCE_MODEM_BRG_2M },
    { REVA_AS_BPS_4800,    4800,   38400,      840, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_9600,    9600,   76800,      420, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_14400,  14400,  115200,      280, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_19200,  19200,  153600,      210, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_32K,    32000,  256000,      126, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_38400,  38400,  307200,      105, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_48K,    48000,  384000,       84, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_56K,    56000,  448000,       72, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_57600,  57600,  460800,       70, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_64K,    64000,  512000,       63, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_72K,    72000,  576000,       56, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_115200,115200,  921600,       35, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_128K,  128000, 1008000,       32, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_230400,230400, 1792000,       18, PRINCE_MODEM_BRG_32M },
    { REVA_AS_BPS_256K,  256000, 2016000,       16, PRINCE_MODEM_BRG_32M },
};

/******************************************************************************
 * Function: plugser_display_cntrl_1_regs 
 *
 * Description: This function will display control 1 registers. 
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 * Output: None
 ******************************************************************************/
void plugser_display_cntrl_1_regs (ulong scc_base_addr)
{
    prince_cntrl_1_regs_t *cntrl_1_regs; 
    prince_scc_regs_t * base_addr;

    base_addr = (prince_scc_regs_t *) scc_base_addr; 
    cntrl_1_regs = &base_addr-> cntrl_1_regs;
    printf("\nPluggable Serial CONTROL-1 REGISTERS:................\n");
    printf("cheerio revision      @%#x = %#x\n", &cntrl_1_regs->
           cheerios_revision,
           cntrl_1_regs->cheerios_revision);
    printf("cheerio revision1     @%#x = %#x\n", &cntrl_1_regs->
           cheerios_revision1,
           cntrl_1_regs->cheerios_revision1);
    printf("cheerio control       @%#x = %#x\n", 
           &cntrl_1_regs->cheerios_cntrl,
           cntrl_1_regs->cheerios_cntrl);
    printf("INT 1 status          @%#x = %#x\n", &cntrl_1_regs->
           int_1_intr_status,
           cntrl_1_regs->int_1_intr_status);
    printf("INT 2 status          @%#x = %#x\n", &cntrl_1_regs->
           int_2_intr_status,
           cntrl_1_regs->int_2_intr_status);
    printf("timer 1-16 status     @%#x = %#x\n", &cntrl_1_regs->
           timer_1_16_intr_status,
           cntrl_1_regs->timer_1_16_intr_status);
    printf("timer 17-20 status    @%#x = %#x\n", &cntrl_1_regs->
           timer_17_20_intr_status,
           cntrl_1_regs->timer_17_20_intr_status);
    printf("INT 1 mask            @%#x = %#x\n", 
           &cntrl_1_regs->int_1_intr_mask,
           cntrl_1_regs->int_1_intr_mask);
    printf("INT 2 mask            @%#x = %#x\n", 
           &cntrl_1_regs->int_2_intr_mask,
           cntrl_1_regs->int_2_intr_mask);
    printf("timer 1-16 mask       @%#x = %#x\n", &cntrl_1_regs->
           timer_1_16_intr_mask,
           cntrl_1_regs->timer_1_16_intr_mask);
    printf("timer 17-20 mask      @%#x = %#x\n", &cntrl_1_regs->
           timer_17_20_intr_mask,
           cntrl_1_regs->timer_17_20_intr_mask);
    printf("timer 1-2 program     @%#x = %#x\n", 
           &cntrl_1_regs->timer_1_2_pgm,
           cntrl_1_regs->timer_1_2_pgm);
    printf("timer 3-4 program     @%#x = %#x\n", 
           &cntrl_1_regs->timer_3_4_pgm,
           cntrl_1_regs->timer_3_4_pgm);
    printf("timer 5-6 program     @%#x = %#x\n",
            &cntrl_1_regs->timer_5_6_pgm,
           cntrl_1_regs->timer_5_6_pgm);
    printf("timer 7-8 program     @%#x = %#x\n", 
           &cntrl_1_regs->timer_7_8_pgm,
           cntrl_1_regs->timer_7_8_pgm);
    printf("timer 9-10 program    @%#x = %#x\n", 
           &cntrl_1_regs->timer_9_10_pgm,
           cntrl_1_regs->timer_9_10_pgm);
    printf("timer 11-12 program   @%#x = %#x\n", 
           &cntrl_1_regs->timer_11_12_pgm,
           cntrl_1_regs->timer_11_12_pgm);
    printf("timer 13-14 program   @%#x = %#x\n", 
           &cntrl_1_regs->timer_13_14_pgm,
           cntrl_1_regs->timer_13_14_pgm);
    printf("timer 15-16 program   @%#x = %#x\n", 
           &cntrl_1_regs->timer_15_16_pgm,
           cntrl_1_regs->timer_15_16_pgm);
    printf("timer 17 program      @%#x = %#x\n", 
           &cntrl_1_regs->timer_17_pgm,
           cntrl_1_regs->timer_17_pgm);
    printf("timer 18 program      @%#x = %#x\n", 
           &cntrl_1_regs->timer_18_pgm,
           cntrl_1_regs->timer_18_pgm);
    printf("timer 19 program      @%#x = %#x\n", 
           &cntrl_1_regs->timer_19_pgm,
           cntrl_1_regs->timer_19_pgm);
    printf("timer 20 program      @%#x = %#x\n", 
           &cntrl_1_regs->timer_20_pgm,
           cntrl_1_regs->timer_20_pgm);
}

/******************************************************************************
 * Function: plugser_display_proto_regs 
 *
 * Description: This function will display protocol registers. 
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 *         disp_opt - Display mode 
 *         chan - Channel number 
 * Output: None 
 ******************************************************************************/
void plugser_display_proto_regs (ulong scc_base_addr, uchar disp_opt,
                         uchar chan)
{
    prince_proto_regs_t *proto_regs; 
    prince_scc_regs_t * base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (prince_scc_regs_t *) scc_base_addr; 
    num_chan = get_scc_channel_num();

    if (disp_opt == PRINCE_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }
    for (ix = start_chan; ix < end_chan; ix++) {  
        proto_regs = &base_addr-> proto_regs_hp1[ix];
        printf("\nPRINCE PROTOCOL REGISTERS CHAN %d:................\n",ix);
        printf("mode register          @%#x = %#x\n", 
               &proto_regs->mode_reg,
               proto_regs->mode_reg);
        printf("flag config            @%#x = %#x\n", 
               &proto_regs->flag_config,
               proto_regs->flag_config);
        printf("flow control config    @%#x = %#x\n", 
               &proto_regs->flow_cntrl_config,
               proto_regs->flow_cntrl_config);
        printf("interrupt status       @%#x = %#x\n", 
               &proto_regs->intr_status,
               proto_regs->intr_status);
        printf("interrupt mask         @%#x = %#x\n", 
               &proto_regs->intr_mask,
               proto_regs->intr_mask);
        printf("command register       @%#x = %#x\n", 
               &proto_regs->command_reg,
               proto_regs->command_reg);
    }
}

/******************************************************************************
 * Function: plugser_display_dma_regs 
 *
 * Description: This function will display DMA receive/transmit registers. 
 *
 * Input: scc_base_addr - FPGA SCC register base addr
 *        option - PRINCE_DMA_TX / PRINCE_DMA_RX
 *        disp_opt - Display mode
 *        priority - High / Low priority
 * Output: None
 ******************************************************************************/
void plugser_display_dma_regs (ulong scc_base_addr, uchar option, uchar 
                       disp_opt, uchar chan, int priority)
{
    prince_dma_regs_t *dma_regs; 
    prince_scc_regs_t * base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (prince_scc_regs_t *) scc_base_addr; 
    num_chan = get_scc_channel_num();

    if (disp_opt == PRINCE_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }

    for (ix = start_chan; ix < end_chan; ix++) {  
        if (option == PRINCE_DMA_TX) {
            switch (priority) {
            case PRINCE_SCC_HIGH_PRIORITY_1:
                dma_regs = &base_addr-> dma_tx_regs_hp1[ix];
                printf("\nPluggable Serial DMA-TX REGISTERS CHAN %d HP1 @%#x:................\n", dma_regs, ix);
                break;
            case PRINCE_SCC_HIGH_PRIORITY_2:
                dma_regs = &base_addr-> dma_tx_regs_hp2[ix];
                printf("\nPluggable Serial DMA-TX REGISTERS CHAN %d HP2 @%#x:................\n", dma_regs, ix);
                break;
            case PRINCE_SCC_LOW_PRIORITY:
                dma_regs = &base_addr-> dma_tx_regs_lp[ix];
                printf("\nPluggable Serial DMA-TX REGISTERS CHAN %d LP @%#x:................\n", dma_regs, ix);
                break;
            default:
                printf("\nNot supported this priority");
                break;
            }
        } else {
            dma_regs = &base_addr-> dma_rx_regs[ix];
            printf("\nPluggable Serial DMA-RX REGISTERS CHAN %d:................\n",ix);
        }
        printf("ring start          @%#x = %#x\n", &dma_regs->ring_start,
               dma_regs->ring_start);
        printf("ring size mask & index    @%#x = %#x\n", 
               &dma_regs->ring_size_index,
               dma_regs->ring_size_index);
        printf("curr buf stat high  @%#x = %#x\n", &dma_regs->reserved[0],
               dma_regs->reserved[0]);
        printf("curr buf stat low   @%#x = %#x\n", &dma_regs->reserved[1],
               dma_regs->reserved[1]);
        printf("curr buf addr high  @%#x = %#x\n", &dma_regs->reserved[2],
               dma_regs->reserved[2]);
        printf("curr buf addr low   @%#x = %#x\n", &dma_regs->reserved[3],
               dma_regs->reserved[3]);
        printf("rx dma context high @%#x = %#x\n", &dma_regs->reserved[4],
               dma_regs->reserved[4]);
        printf("rx dma context low  @%#x = %#x\n", &dma_regs->reserved[5],
               dma_regs->reserved[5]);
        printf("partial bytes high  @%#x = %#x\n", &dma_regs->reserved[6],
               dma_regs->reserved[6]);
        printf("partial bytes low   @%#x = %#x\n", &dma_regs->reserved[7],
               dma_regs->reserved[7]);
    }
}

/******************************************************************************
 * Function: plugser_display_serial_itf_regs 
 *
 * Description: This function will display serial interface registers. 
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 *         disp_opt - Display mode
 *         chan - Channel number
 * Output: None
 ******************************************************************************/
void plugser_display_serial_itf_regs (ulong scc_base_addr, 
                              uchar disp_opt, uchar chan)
{
    prince_serial_itf_t *serial_regs; 
    prince_scc_regs_t * base_addr;
    int ix, num_chan, start_chan, end_chan;

    base_addr = (prince_scc_regs_t *) scc_base_addr; 
    num_chan = get_scc_channel_num();

    if (disp_opt == PRINCE_REG_ALL) {
        start_chan = 0;
        end_chan = num_chan;
    } else {
        start_chan = chan;
        end_chan = chan + 1;
    }

    for (ix = start_chan; ix < end_chan; ix++) {  
        serial_regs = &base_addr-> serial_itf[ix];
        printf("\nPRINCE SERIAL REGISTERS CHAN %d:................\n",ix);
        printf("serial interface cntrl    @%#x = %#x\n", 
               &serial_regs->serial_itf_cntrl,
               serial_regs->serial_itf_cntrl);
        printf("modem control             @%#x = %#x\n", 
               &serial_regs->modem_cntrl,
               serial_regs->modem_cntrl);
        printf("flow control              @%#x = %#x\n", 
               &serial_regs->flow_cntrl,
               serial_regs->flow_cntrl);
        printf("brg divider               @%#x = %#x\n", 
               &serial_regs->brg_divider,
               serial_regs->brg_divider);
        printf("modem intr status         @%#x = %#x\n", 
               &serial_regs->modem_intr_status,
               serial_regs->modem_intr_status);
    }
}

/******************************************************************************
 * Function: plugser_display_cntrl_2_regs 
 *
 * Description: This function will display control 2 registers. 
 *
 * Input:  scc_base_addr - FPGA SCC register base addr
 * Output: None
 ******************************************************************************/
void plugser_display_cntrl_2_regs (ulong scc_base_addr)
{
    prince_cntrl_2_regs_t *cntrl_2_regs; 
    prince_scc_regs_t * base_addr;

    base_addr = (prince_scc_regs_t *) scc_base_addr; 
    
    cntrl_2_regs = &base_addr-> cntrl_2_regs;
    printf("\nPluggable Serial CONTROL-2 REGISTERS:................\n");
    printf("TDM control           @%#x = %#x\n", &cntrl_2_regs->tdm_cntrl,
           cntrl_2_regs->tdm_cntrl);
    printf("TDMA 8K divider       @%#x = %#x\n", 
           &cntrl_2_regs->tdma_8k_divider,
           cntrl_2_regs->tdma_8k_divider);
    printf("TDMB 8K divider       @%#x = %#x\n", 
           &cntrl_2_regs->tdmb_8k_divider,
           cntrl_2_regs->tdmb_8k_divider);
    printf("serial intr status    @%#x = %#x\n", &cntrl_2_regs->
           serial_intr_status,
           cntrl_2_regs->serial_intr_status);
    printf("freq counter port sel @%#x = %#x\n", &cntrl_2_regs->
           freq_cnt_port_select,
           cntrl_2_regs->freq_cnt_port_select);
    printf("freq counter high     @%#x = %#x\n", &cntrl_2_regs->freq_cnt_hi,
           cntrl_2_regs->freq_cnt_hi);
    printf("freq counter low      @%#x = %#x\n", &cntrl_2_regs->freq_cnt_lo,
           cntrl_2_regs->freq_cnt_lo);
    printf("PLL 1 control         @%#x = %#x\n", &cntrl_2_regs->pll1_cntrl,
           cntrl_2_regs->pll1_cntrl);
    printf("PLL 1 8K divider      @%#x = %#x\n", 
           &cntrl_2_regs->pll1_8k_divider,
           cntrl_2_regs->pll1_8k_divider);
    printf("PLL 1 pre divider     @%#x = %#x\n", &cntrl_2_regs->
           pll1_pre_divider,
           cntrl_2_regs->pll1_pre_divider);
    printf("PLL 1 post divider    @%#x = %#x\n", &cntrl_2_regs->
           pll1_post_divider,
           cntrl_2_regs->pll1_post_divider);
    printf("PLL 2 control         @%#x = %#x\n", &cntrl_2_regs->pll2_cntrl,
           cntrl_2_regs->pll2_cntrl);
    printf("PLL 2 8K divider      @%#x = %#x\n", 
           &cntrl_2_regs->pll2_8k_divider,
           cntrl_2_regs->pll2_8k_divider);
    printf("PLL 2 pre divider     @%#x = %#x\n", &cntrl_2_regs->
           pll2_pre_divider,
           cntrl_2_regs->pll2_pre_divider);
    printf("PLL 2 post divider    @%#x = %#x\n", &cntrl_2_regs->
           pll2_post_divider,
           cntrl_2_regs->pll2_post_divider);
}
/******************************************************************************
 * Function: pluggable_sync_chan_lpbk_test 
 *
 * Description: This function will run datapath channel loopback test 
 *
 * Input: channel - Channel number
 * Output: PASSED/FAILED
 ******************************************************************************/
int pluggable_sync_chan_lpbk_test (int channel)
{
    ulong scc_base_addr;
    prince_serial_ds_t *s_ds;
    int port_num;

    testname("SCC Loopback test");

    scc_base_addr = get_scc_base();

    /* get the port number, considering multi channel testing */
    port_num = channel;

    /* Get serial data structure and init it */
    s_ds = &plugser_serial_ds[port_num];
    plugser_init_serial_ds(s_ds);

    s_ds->port_num = port_num;
    s_ds->org_port_num = port_num;

    if (plugser_lpbk_test(scc_base_addr, s_ds)) {
        return (FAILED);
    }

    return (PASSED);
}
/******************************************************************************
 * Function: pluggable_async_chan_lpbk_test 
 *
 * Description: This function will run datapath channel loopback test 
 *
 * Input: channel - Channel number
 * Output: PASSED/FAILED
 ******************************************************************************/
int pluggable_async_chan_lpbk_test (int channel)
{
    ulong scc_base_addr;
    prince_serial_ds_t *s_ds;
    int port_num;

    testname("Async SCC Loopback test");

    scc_base_addr = get_scc_base() + (channel / 4) * 0x4000;

    /* get the port number, considering multi channel testing */
    port_num = channel % 4;

    /* Get serial data structure and init it */
    s_ds = &plugser_serial_ds[port_num];

    plugser_init_serial_ds(s_ds);

    s_ds->port_num = port_num;
    s_ds->org_port_num = channel;

    if (plugser_async_lpbk_test(scc_base_addr, s_ds)) {
        return (FAILED);
    }

    return (PASSED);
}
/*******************************************************************************
 * Function: plugser_lpbk_test 
 *
 * This function will run loopkback test
 *
 * Input: scc_base_addr
 *        s_ds - Pointer to the serial data structure.
 *
 * Output: PASSED FAILED
 ******************************************************************************/
int plugser_lpbk_test (ulong scc_base_addr, prince_serial_ds_t *s_ds)
{
    ushort index, clk_src, lpbk_mode, protocol, run_mode = PRINCE_POLL_MODE;

    /*
     * HDLC internal loopback test
     */
    lpbk_mode = PRINCE_SCC_INT_LOOPBACK;
    protocol = PRINCE_HDLC;
    clk_src = PRINCE_CLK_SRC_SYNC;

    /* CSCvc04300 : Fixed loopback test max data rate at 8Mbps
        The Prince System Functional Spec shows 8 Mbits/s as highest rate. */
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_SYNC_BPS_300; index <= PRINCE_SYNC_BPS_8064K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                      lpbk_mode, protocol, s_ds, 
                                      PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                          lpbk_mode, protocol, s_ds, 
                                          PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                      lpbk_mode, protocol,s_ds, 
                                      PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * Bisync internal loopback test
     */
    protocol = PRINCE_BISYNC;
    lpbk_mode = PRINCE_SCC_INT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_SYNC;
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_LOW_SPEED_INDEX; index <= PRINCE_SYNC_BPS_8064K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds, 
                                    PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                        lpbk_mode, protocol,s_ds, 
                                        PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds, 
                                    PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * sync transparent internal loopback test
     */
    lpbk_mode = PRINCE_SCC_INT_LOOPBACK;
    protocol = PRINCE_TRANSPARENT;
    clk_src = PRINCE_CLK_SRC_SYNC;
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_LOW_SPEED_INDEX; index <= PRINCE_SYNC_BPS_8064K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds, 
                                    PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                        lpbk_mode, protocol,s_ds, 
                                        PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds, 
                                    PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * HDLC external loopback test
     */
    protocol = PRINCE_HDLC;
    lpbk_mode = PRINCE_SCC_EXT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_SYNC;

    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_SYNC_BPS_300; index <= PRINCE_SYNC_BPS_8064K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol,s_ds,
                                PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol,s_ds,
                                PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * Bisync external loopback test
     */
    protocol = PRINCE_BISYNC;
    lpbk_mode = PRINCE_SCC_EXT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_SYNC;
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_SYNC_BPS_300; index <= /*PRINCE_SYNC_BPS_256K*/PRINCE_SYNC_BPS_8064K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol,s_ds,
                                PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol,s_ds,
                                PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * sync transparent external loopback test
     */
    protocol = PRINCE_TRANSPARENT;
    lpbk_mode = PRINCE_SCC_EXT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_SYNC;

    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_SYNC_BPS_300; index <= PRINCE_SYNC_BPS_8064K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol,s_ds,
                                PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
            if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol,s_ds,
                                PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    return (PASSED);
}

/******************************************************************************
 * Function: plugser_async_lpbk_test 
 *
 * Description: This function will run loopkback test
 *
 * Input: scc_base_addr - FPGA SCC register base addr
 *        s_ds - Pointer to the serial data structure.
 * Output: PASSED/FAILED
 ******************************************************************************/

int plugser_async_lpbk_test (ulong scc_base_addr, prince_serial_ds_t *s_ds)
{
    ushort index, clk_src, lpbk_mode, protocol, run_mode = PRINCE_POLL_MODE;

    /* Pluggable serial FPGA did not support DCD <-> DTR ASYNC loopback test */

    /*
     * async internal loopback test
     */
    protocol = PRINCE_UART_ASYNC;
    lpbk_mode = PRINCE_SCC_INT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_ASYNC;
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = REVA_AS_BPS_300; index <= REVA_AS_BPS_256K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol, s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
			if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol, s_ds,
                                    PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * PPP internal loopback test
     */
    lpbk_mode = PRINCE_SCC_INT_LOOPBACK;
    protocol = PRINCE_UART_PPP;
    clk_src = PRINCE_CLK_SRC_ASYNC;
    if (((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = REVA_AS_BPS_300; index <= REVA_AS_BPS_256K;
             index = index + PRINCE_SPEED_INDEX_INC ) {

            if (index != REVA_AS_BPS_300 && index != REVA_AS_BPS_19200 &&
                index != REVA_AS_BPS_128K && index != REVA_AS_BPS_230400 &&
                index != REVA_AS_BPS_256K) {
                continue;
            }

            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol, s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
			if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol, s_ds,
                                    PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * async external loopback test
     */
    protocol = PRINCE_UART_ASYNC;
    lpbk_mode = PRINCE_SCC_EXT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_ASYNC;
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_AS_BPS_300; index <= REVA_AS_BPS_256K;
             index = index + PRINCE_SPEED_INDEX_INC ) {
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol, s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
			if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol, s_ds,
                                PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    /*
     * PPP external loopback test
     */
    protocol = PRINCE_UART_PPP;
    lpbk_mode = PRINCE_SCC_EXT_LOOPBACK;
    clk_src = PRINCE_CLK_SRC_ASYNC;
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        for (index = PRINCE_AS_BPS_300; index <= REVA_AS_BPS_256K;
             index = index + PRINCE_SPEED_INDEX_INC ) {

            if (index != REVA_AS_BPS_300 && index != REVA_AS_BPS_19200 &&
                index != REVA_AS_BPS_128K && index != REVA_AS_BPS_230400 &&
                index != REVA_AS_BPS_256K) {
                continue;
            }

            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol, s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_1)) {
                return (FAILED);
            }
			if (fpga_ver >= NEW_FPGA_VERSION) {
                if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                    lpbk_mode, protocol,s_ds,
                                    PRINCE_SCC_HIGH_PRIORITY_2)) {
                    return (FAILED);
                }
            }
            if (plugser_chan_lpbk_test(run_mode, scc_base_addr, index, clk_src,
                                lpbk_mode, protocol, s_ds,
                                PRINCE_SCC_LOW_PRIORITY)) {
                return (FAILED);
            }
        }
    }

    return (PASSED);
}

/******************************************************************************
 * Function: plugser_chan_lpbk_test 
 *
 * Description: This function will run one-shot loopback test for a single channel 
 *
 * Input: run_mode  - Interrupt or poll. 
 *      : scc_base_addr - base address for SCC.
 *      : speed_idx - index to the baud rate table.
 *      : clk_src - sync/async or plls.
 *      : lpbk_mode - internal/external.
 *      : protocol - HDLC/Bisync/TransparenUART ASYNC / UART PPP.
 *      : s_ds - Pointer to the serial data structure.
 * Output: PASSED FAILED
 ******************************************************************************/
int plugser_chan_lpbk_test (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                     uchar clk_src, uchar lpbk_mode, uchar protocol,
                     prince_serial_ds_t *s_ds, int priority)
{
    char tmp_name_buf[30];
    prince_scc_regs_t * base_addr;
    ulong des_baud = 0;
    ushort clk_src_bits = 0;
    ulong dummy;
    int ix;
 
    /* The old FPGA LP map to new FPGA HP2 */
    if (fpga_ver < NEW_FPGA_VERSION) {
        switch (priority) {
        case PRINCE_SCC_LOW_PRIORITY:
            priority = PRINCE_SCC_HIGH_PRIORITY_2;
            break;
        default:
            printf("\nNot supported this priority");
            break;
        }
    }

    plugser_config_serial_ds(run_mode, scc_base_addr, speed_idx, clk_src,
                            lpbk_mode, protocol, s_ds, priority,
                            &des_baud, &clk_src_bits);
    
	plugser_form_name_buf(tmp_name_buf, s_ds, priority);

    prpass(testpass,"%schan %d @speed %d ", tmp_name_buf, s_ds->org_port_num,
           des_baud);

    /* 
     *  Stop DMA
     */
    switch (priority) {
    case PRINCE_SCC_HIGH_PRIORITY_1:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_HP1_STOP_TX_RX;
        break;
    case PRINCE_SCC_HIGH_PRIORITY_2:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_HP2_STOP_TX_RX;
        break;
    case PRINCE_SCC_LOW_PRIORITY:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_LP_STOP_TX_RX;
        break;
    default:
        break;
    }

    if (diagflag_xram & D_TRACE) {
        printf("\n Step 1: Stop TX & RX\n");
        printf("\ts_ds->proto_regs_addr->command_reg @%#x = %#x\n", 
            &s_ds->proto_regs_addr->command_reg, 
            s_ds->proto_regs_addr->command_reg);
    }

    /* 
     *  Disable mode
     */
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_MASK;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 2: Disable the channel\n");
        printf("\ts_ds->proto_regs_addr->mode_reg @%#x = %#x\n", 
            &s_ds->proto_regs_addr->mode_reg, 
            s_ds->proto_regs_addr->mode_reg);
    }

    /* 
     *  reset Tx and Rx paths
     */
    s_ds->proto_regs_addr->command_reg = PRINCE_CMD_RESET_TX_RX;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 3: reset Tx and Rx paths\n");
        printf("\ts_ds->proto_regs_addr->command_reg @%#x = %#x\n", 
            &s_ds->proto_regs_addr->command_reg, 
            s_ds->proto_regs_addr->command_reg);
    }

    /* 
     *  setup DMA register 
     */
	plugser_setup_dma_reg(s_ds);
    if (diagflag_xram & D_TRACE) {
        printf(" Step 4 & 5: setup DMA register\n");
    }

    /*
     *   assert reset interface circuit 
     */
    s_ds->serial_itf_addr->serial_itf_cntrl |= PRINCE_RESET_ITF_CKT;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 6: reset channel interface\n");
        printf("\ts_ds->serial_itf_addr->serial_itf_cntrl @%#x = %#x\n", 
            &s_ds->serial_itf_addr->serial_itf_cntrl, s_ds->serial_itf_addr->serial_itf_cntrl);
    }

    /* 
     *  Set baud rate divider 
     */
    s_ds->serial_itf_addr->brg_divider = s_ds->divider;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 7: Set baud rate divider\n");
        printf("\ts_ds->serial_itf_addr->brg_divider @%#x = %#x\n", 
            &s_ds->serial_itf_addr->brg_divider, 
            s_ds->serial_itf_addr->brg_divider);
    }

    /* 
     *  Set modem control 
     */
    s_ds->serial_itf_addr->modem_cntrl &= ~PRINCE_MODEM_BRG_MASK;

    if (clk_src == PRINCE_CLK_SRC_SYNC) {
        if (clk_src_bits == PRINCE_MODEM_BRG_10M)
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_10M;
        else if (clk_src_bits == PRINCE_MODEM_BRG_252K)
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_252K;
        else if (clk_src_bits == PRINCE_MODEM_BRG_2M)
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_2M;
        else
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_32M;
    } else if (clk_src == PRINCE_CLK_SRC_ASYNC) {
        if (clk_src_bits == PRINCE_MODEM_BRG_1M)
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_1M;
		else if (clk_src_bits == PRINCE_MODEM_BRG_2M)
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_2M;
        else
            s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_BRG_32M;
    }

    if (diagflag_xram & D_TRACE) {
        printf(" Step 8: Setup modem control\n");
        printf("\ts_ds->serial_itf_addr->modem_cntrl @%#x = %#x\n", 
            &s_ds->serial_itf_addr->modem_cntrl, 
            s_ds->serial_itf_addr->modem_cntrl);
    }

    s_ds->serial_itf_addr->flow_cntrl = 0;

    if (diagflag_xram & D_TRACE) {
        printf(" Step 9: intfc flow control\n");
        printf("\ts_ds->serial_itf_addr->flow_cntrl @%#x = %#x\n", 
            &s_ds->serial_itf_addr->flow_cntrl, 
            s_ds->serial_itf_addr->flow_cntrl);
    }

    /* 
     *  Set Channel Flag 
     */
    switch (protocol) {
    case PRINCE_HDLC:
        s_ds->proto_regs_addr->flag_config = 0x007e;
        break;

    case PRINCE_BISYNC:
        s_ds->proto_regs_addr->flag_config = /*0x7e7e*/0x3232;
        break;

    case PRINCE_TRANSPARENT:
        s_ds->proto_regs_addr->flag_config = 0x7e7e;
        break;
        
    case PRINCE_UART_ASYNC:
        s_ds->proto_regs_addr->flag_config = 0xffee;
        /* Flow Control, Pluggable serial support RTS/CTS flow control */
        s_ds->proto_regs_addr->flow_cntrl_config = 0x2200;
        break;

    case PRINCE_UART_PPP:
        s_ds->proto_regs_addr->flag_config = 0xffee;
        /* Flow Control, Pluggable serial support RTS/CTS flow control */
        s_ds->proto_regs_addr->flow_cntrl_config = 0x2200;
        break;

    default:
        break;
    }
    if (diagflag_xram & D_TRACE) {
        printf(" Step 10: Setup Channel Flag \n");
        printf("\ts_ds->proto_regs_addr->flag_config @%#x = %#x\n", 
               &s_ds->proto_regs_addr->flag_config, 
               s_ds->proto_regs_addr->flag_config);
        printf("\ts_ds->proto_regs_addr->flow_cntrl_config @%#x = %#x\n",
               &s_ds->proto_regs_addr->flow_cntrl_config,
               s_ds->proto_regs_addr->flow_cntrl_config);
    }

    /* 
     *  De-assert reset to channel interface 
     */
    s_ds->serial_itf_addr->serial_itf_cntrl &= ~PRINCE_RESET_ITF_CKT;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 11: De-assert reset to channel interface\n");
        printf("\ts_ds->serial_itf_addr->serial_itf_cntrl @%#x = %#x\n", 
            &s_ds->serial_itf_addr->serial_itf_cntrl, 
            s_ds->serial_itf_addr->serial_itf_cntrl);
    }

    /* 
     *  reset Tx and Rx paths again
     */
    s_ds->proto_regs_addr->command_reg = PRINCE_CMD_RESET_TX_RX;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 12: reset Tx and Rx paths again\n");
        printf("\ts_ds->proto_regs_addr->command_reg @%#x = %#x\n", 
            &s_ds->proto_regs_addr->command_reg, 
            s_ds->proto_regs_addr->command_reg);
    }

    /* 
     *  clear FCS and initialize mode register 
     */
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_FCS_MASK;
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_NRZ_MASK;
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_FLAG_CHK_MASK;
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_EBCDIC;
    s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_CRC_16_MASK;   /* default */
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_IDLE_MARK;

    /* 
     *  Set Protocol Mode register 
     */
    switch (protocol) {
    case PRINCE_HDLC:
        s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_NRZ_MASK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_FLAG1_CHK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_HDLC;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_CRC_16_MASK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_IDLE_FLAG;
        break;

    case PRINCE_BISYNC:
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_EBCDIC; 
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_FLAG_BOTH_CHK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_BISYNC;
        break;

    case PRINCE_TRANSPARENT:
        s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_NRZ_MASK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_FLAG1_CHK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_CRC_16_MASK;
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_IDLE_MARK; 
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_TRANSPARENT;
        break;       
        
    case PRINCE_UART_ASYNC:
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_UART_ASYNC;
		/* turn off CRC if requested */
        if (!s_ds->crc_enb) {
            s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_CRC;
        }
        plugser_set_async_timer(s_ds, PRINCE_ASYNC_TIMER_COUNT);
        break;

    case PRINCE_UART_PPP:
        s_ds->proto_regs_addr->mode_reg |= PRINCE_MODE_UART_PPP;
        /* turn off CRC if requested */
        if (!s_ds->crc_enb) {
            s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_CRC;
        }
        plugser_set_async_timer(s_ds, PRINCE_ASYNC_TIMER_COUNT);
        break;

    default:
        break;
    }
    if (diagflag_xram & D_TRACE) {
        printf(" Step 13: Setup the channel mode\n");
        printf("\ts_ds->proto_regs_addr->mode_reg @%#x = %#x\n", 
            &s_ds->proto_regs_addr->mode_reg, 
            s_ds->proto_regs_addr->mode_reg);
    }

    /* Set Bisync TX & RX PAD Characters */
    if (s_ds->protocol == PRINCE_BISYNC) {
        /* EBCIDIC mode */
        s_ds->bcc_tx_regs_addr->context = (0x55 << 24);
        s_ds->bcc_rx_regs_addr->context = (0xFF << 24);
        for (ix = 0; ix < 3; ix++) {
            s_ds->bcc_tx_regs_addr->unused[ix] = 0;
            s_ds->bcc_rx_regs_addr->unused[ix] = 0;
        }
    }
    if (diagflag_xram & D_TRACE) {
        printf(" Set Bisync TX & RX PAD Characters\n");
        printf("\ts_ds->bcc_tx_regs_addr->context @%#x = %#x"
               "\ts_ds->bcc_rx_regs_addr->context @%#x = %#x\n", 
               &s_ds->bcc_tx_regs_addr->context,
               s_ds->bcc_tx_regs_addr->context,
               &s_ds->bcc_rx_regs_addr->context,
               s_ds->bcc_rx_regs_addr->context);
    }

    /* 
     *  dummy read of modem interrupt status 
     */
    dummy = s_ds->serial_itf_addr->modem_intr_status;
    if (diagflag_xram & D_TRACE) {
        printf(" Step 14: Clear pending interrupts\n");
        printf("\ts_ds->serial_itf_addr->modem_intr_status @%#x = %#x\n", 
            &s_ds->serial_itf_addr->modem_intr_status, 
            s_ds->serial_itf_addr->modem_intr_status);
    }

    /* 
     *  Enable DTE external loopback if needed.
     */
    if (s_ds->serial_itf_addr->serial_itf_cntrl & PRINCE_CABLE_ID_MODE_BIT) {
        s_ds->serial_itf_addr->serial_itf_cntrl &= ~PRINCE_DTE_EXT_EN;
    } else {
        s_ds->serial_itf_addr->serial_itf_cntrl |= PRINCE_DTE_EXT_EN;
        /* Skip the internal loopback test in DTE mode */
        if (lpbk_mode == PRINCE_SCC_INT_LOOPBACK) {
            return (PASSED);
        }
    }

    /* 
     *  Set loopback mode 
     */
    if (lpbk_mode == PRINCE_SCC_INT_LOOPBACK) {
        s_ds->serial_itf_addr->modem_cntrl |= PRINCE_MODEM_LOOPBACK;
    } else {
        s_ds->serial_itf_addr->modem_cntrl &= ~PRINCE_MODEM_LOOPBACK;
    } 

    /*
     * Init the Tx and Rx Rings and the buffers
     */
    plugser_init_tx_dma_ring(run_mode, scc_base_addr, s_ds);
    plugser_init_rx_dma_ring(run_mode, scc_base_addr, s_ds);

    /*
     * Fill the Tx Buffers with Data.
     * Fill the Rx Buffers with Zeros
     */
    if (protocol == PRINCE_BISYNC) {
        plugser_init_tx_dma_buffers(s_ds,  0x12, PRINCE_DATA_PAT);
    } else if (protocol == PRINCE_TRANSPARENT) {
        plugser_init_tx_dma_buffers(s_ds, 0, PRINCE_INC_PAT);
    } else {
        plugser_init_tx_dma_buffers(s_ds, 1, PRINCE_INC_PAT);
    }
    plugser_init_rx_dma_buffers(s_ds);

    /*
     * Start the Tx.
     */
    if (run_mode != PRINCE_NO_CHECK_MODE) {
        if (plugser_start_dma_tx(s_ds, priority)) {
            plugser_cleanup_serial(s_ds, priority);
            plugser_cleanup_dma_ring(scc_base_addr, s_ds); 
            return (FAILED);
        }
        /*
         * Check that we received the Data OK.
         */
        if (plugser_check_dma_received_data(s_ds)) {
            plugser_trap(s_ds, priority);
            plugser_cleanup_serial(s_ds, priority); 
            plugser_cleanup_dma_ring(scc_base_addr, s_ds); 
            return (FAILED);
        }
    } else {
        /* PLL test needs the setup and not the data transfer */
        return (PASSED);

        /* cleanup will be done in PLL */ 
    }

    /* 
     *  clean up buf 
     */
    plugser_cleanup_serial(s_ds, priority);
    plugser_cleanup_dma_ring(scc_base_addr, s_ds);

    return (PASSED);
}



/******************************************************************************
 * Function: plugser_config_serial_ds
 *
 * Description: This function will config the serial port
 *
 * Input: run_mode  - Interrupt or poll.
 *      : scc_base_addr - base address for SCC.
 *      : speed_idx - index to the baud rate table.
 *      : clk_src - sync/async or plls.
 *      : lpbk_mode - internal/external.
 *      : protocol - UART ASYNC / UART PPP.
 *      : s_ds - Pointer to the serial data structure.
 *      : des_baud - divider.
 *      : clk_src_bits - clock selection.
 * Output: PASSED / FAILED
 ******************************************************************************/
void plugser_config_serial_ds (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                              uchar clk_src, uchar lpbk_mode, uchar protocol,
                              prince_serial_ds_t *s_ds, int priority,
                              ulong *des_baud, ushort *clk_src_bits)
{
    ulong baudrate = 0;
    ulong divider = 1;
    prince_scc_regs_t * base_addr;

    *clk_src_bits = 0;
    *des_baud = 0;
    base_addr = (prince_scc_regs_t *) scc_base_addr; 

    /* 
     *  divider and clk_src_bits (clock selection) are defined in FPGA Table 3
     *  USRT/SYNC baud rate divisor table and Table 4 UART/ASYNC baud rate
     *  divisor table.
     */
    switch (clk_src) {
    case PRINCE_CLK_SRC_SYNC:
        divider = plugser_sync_baud[speed_idx].divider;
        baudrate = plugser_sync_baud[speed_idx].baudrate;
        *des_baud = plugser_sync_baud[speed_idx].desired_baud;
        *clk_src_bits = plugser_sync_baud[speed_idx].clk_src;
        break;

    case PRINCE_CLK_SRC_ASYNC:
        divider = plugser_async_baud[speed_idx].divider;
        baudrate = plugser_async_baud[speed_idx].baudrate;
        *des_baud = plugser_async_baud[speed_idx].desired_baud;
        *clk_src_bits = plugser_async_baud[speed_idx].clk_src;
        break;

    default:
        break;
    }

    /* 
     *  PPP/HDLC/Bisync needs to reserve extra CRC bytes
     *  in the receive buffer.
     */
    if (protocol != PRINCE_UART_ASYNC) {
        s_ds->crc_rx_buf_opt = CRC_RX_BUF_OPT_ON;
    } else {
        s_ds->crc_rx_buf_opt = 0;
    }

    /* 
     *  For lower speed, use smaller buffer size to save test time.
     */
    if (speed_idx < PRINCE_LOW_SPEED_INDEX) {
        s_ds->buff_size = MAX_PRINCE_SMALL_BUFFER_SIZE;
    } else {
        s_ds->buff_size = MAX_PRINCE_BUFFER_SIZE;
    }

    /* 
     *  Fill up serial data struct information
     */
    s_ds->baudrate = baudrate; 
    s_ds->divider = divider;
    s_ds->protocol = protocol;
    s_ds->lpbk_mode = lpbk_mode;
    s_ds->clk_src = clk_src;
    s_ds->speed_idx = speed_idx; 
    s_ds->run_mode = run_mode; 
    s_ds->base_addr = scc_base_addr;
    s_ds->num_buff = MAX_PRINCE_BUFFERS;
    s_ds->dma_rx_regs_addr = (prince_dma_regs_t *) (&( base_addr->
                             dma_rx_regs[s_ds->port_num]));
    s_ds->proto_regs_addr = (prince_proto_regs_t *) (&( base_addr->
                            proto_regs_hp1[s_ds->port_num]));
    s_ds->ppp_tx_regs_addr = (prince_ppp_regs_t *) (&( base_addr->
                             ppp_tx_regs[s_ds->port_num]));
    s_ds->ppp_rx_regs_addr = (prince_ppp_regs_t *) (&( base_addr->
                             ppp_rx_regs[s_ds->port_num]));
    s_ds->bcc_tx_regs_addr = (prince_bcc_regs_t *) (&( base_addr->
                             bcc_regs[s_ds->port_num]));
    s_ds->bcc_rx_regs_addr = (prince_bcc_regs_t *) (&( base_addr->
                             bcc_regs_rx[s_ds->port_num]));
    s_ds->serial_itf_addr = (prince_serial_itf_t *) (&( base_addr->
                            serial_itf[s_ds->port_num]));
    s_ds->cntrl_1_regs_addr = (prince_cntrl_1_regs_t *) (&( base_addr->
                              cntrl_1_regs));
    s_ds->cntrl_2_regs_addr = (prince_cntrl_2_regs_t *) (&( base_addr->
                              cntrl_2_regs));

    switch (priority) {
    case PRINCE_SCC_HIGH_PRIORITY_1:
        s_ds->dma_tx_regs_addr = (prince_dma_regs_t *) (&( base_addr->
                            dma_tx_regs_hp1[s_ds->port_num]));
        break;
    case PRINCE_SCC_HIGH_PRIORITY_2:
        s_ds->dma_tx_regs_addr = (prince_dma_regs_t *) (&( base_addr->
                            dma_tx_regs_hp2[s_ds->port_num]));
        break;
    case PRINCE_SCC_LOW_PRIORITY:
        s_ds->dma_tx_regs_addr = (prince_dma_regs_t *) (&( base_addr->
                            dma_tx_regs_lp[s_ds->port_num]));
        break;
    default:
        break;
    }
    s_ds->tx_desc_addr_virt = get_scc_dma_txbd_virt();
    s_ds->tx_desc_addr_phys = get_scc_dma_txbd_phys();
    s_ds->tx_buf_addr_virt = get_scc_dma_tx_virt();
    s_ds->tx_buf_addr_phys = get_scc_dma_tx_phys();

    s_ds->rx_desc_addr_virt = get_scc_dma_rxbd_virt();
    s_ds->rx_desc_addr_phys = get_scc_dma_rxbd_phys();
    s_ds->rx_buf_addr_virt = get_scc_dma_rx_virt();
    s_ds->rx_buf_addr_phys = get_scc_dma_rx_phys();
}

/******************************************************************************
 *
 * Function: plugser_setup_dma_reg
 *
 * Description: This function set up dma register for FPGA to pick the ring buffer
 *
 * Input: s_ds - Pointer to the serial data structure.
 * Output: None
 ******************************************************************************/
void plugser_setup_dma_reg (prince_serial_ds_t *s_ds)
{
    prince_dma_bd_t *tx_desc;
    prince_dma_bd_t *rx_desc;
    ushort ix;
    prince_dma_regs_t * tx_dma_regs;
    prince_dma_regs_t * rx_dma_regs;

    tx_desc = (prince_dma_bd_t *) s_ds->tx_desc_addr_virt;
    rx_desc = (prince_dma_bd_t *) s_ds->rx_desc_addr_virt;

    tx_dma_regs = (prince_dma_regs_t *) s_ds->dma_tx_regs_addr;
    rx_dma_regs = (prince_dma_regs_t *) s_ds->dma_rx_regs_addr;

    tx_dma_regs->ring_start = s_ds->tx_desc_addr_phys;
    rx_dma_regs->ring_start = s_ds->rx_desc_addr_phys;

    /* Bit 0~15     -- Ring index
       Bit 16~18    -- alignment, should always be zero 
       Bit 19~31    -- Ring size mask */
    tx_dma_regs->ring_size_index = (MAX_PRINCE_BUFFERS-1) << 19;
    rx_dma_regs->ring_size_index = (MAX_PRINCE_BUFFERS-1) << 19;

    /* clean up reserved array, which used by FPGA internal */
    for (ix = 0; ix < 6; ix++) {
        rx_dma_regs->reserved[ix] = 0;
        tx_dma_regs->reserved[ix] = 0;
    }

    if (diagflag_xram & D_TRACE) {
        printf("\ttx_dma_regs->ring_start = %#x tx_dma_regs->ring_size_index = %#x\n",
            tx_dma_regs->ring_start, tx_dma_regs->ring_size_index);
        printf("\trx_dma_regs->ring_start = %#x rx_dma_regs->ring_size_index = %#x\n",
            rx_dma_regs->ring_start, rx_dma_regs->ring_size_index);
    }
}

/******************************************************************************
 * Function: prince_start_dma_tx
 *
 * Description: This function sets up the DMA engine for transmission of data.
 *              It checks to make sure that the data has been Tx'ed, and Rx'ed OK.
 *
 * Input: s_ds      - pointer to the serial data structure.
 *        priority  - TX priority
 * Output: PASSED or FAILED.
 ******************************************************************************/
ushort plugser_start_dma_tx (prince_serial_ds_t *s_ds, int priority)
{
    prince_dma_bd_t *tx_desc;
    prince_dma_bd_t *rx_desc;
    prince_dma_bd_t *last_rx_desc;
    ushort ix, jx, kx, rx_err_mask, wait_level;
    prince_dma_regs_t * tx_dma_regs;
    prince_dma_regs_t * rx_dma_regs;

    tx_desc = (prince_dma_bd_t *) s_ds->tx_desc_addr_virt;
    rx_desc = (prince_dma_bd_t *) s_ds->rx_desc_addr_virt;
    last_rx_desc = (prince_dma_bd_t *) s_ds->last_rx_desc_addr;

    tx_dma_regs = (prince_dma_regs_t *) s_ds->dma_tx_regs_addr;
    rx_dma_regs = (prince_dma_regs_t *) s_ds->dma_rx_regs_addr;

    tx_status = rx_status = 0;

    /* 
     * Start TX
     */
    switch (priority) {
    case PRINCE_SCC_HIGH_PRIORITY_1:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_HP1_START_TX;
        break;
    case PRINCE_SCC_HIGH_PRIORITY_2:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_HP2_START_TX;
        break;
    case PRINCE_SCC_LOW_PRIORITY:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_LP_START_TX;
        break;
    default:
        break;
    }

    if (diagflag_xram & D_TRACE) {
        printf("\ns_ds->speed_idx = %d PRINCE_LOW_SPEED_INDEX = %d\n", 
            s_ds->speed_idx, PRINCE_LOW_SPEED_INDEX);
    }

    /* 
     *  Sleep until the ownership bit has been unset by FPGA
     */
    for (jx = 0; jx < PRINCE_MAX_DMA_PASS; jx++) {
        if (s_ds->speed_idx < PRINCE_LOW_SPEED_INDEX) {
            msleep(PRINCE_LOW_SPEED_WAIT);
        } else {
            msleep(PRINCE_HIGH_SPEED_WAIT); 
        }
        if (!(last_rx_desc->command_status & DMA_CMD_STAT_OWNER)) {
            break;
        }
    }

    wait_level = 0;

    if (jx >= PRINCE_MAX_DMA_PASS) {
        for (wait_level = 1; wait_level <= 3; wait_level++) {
            if (diagflag_xram & D_TRACE) {
                printf("\nDMA in wait level %d serial_itf @%#x = %#x --\n", 
                    wait_level,&s_ds->serial_itf_addr->serial_itf_cntrl,
                    s_ds->serial_itf_addr->serial_itf_cntrl);
            }

            for (kx = 0; kx < PRINCE_DMA_RETRY_PASS; kx++) {
                if (s_ds->speed_idx < PRINCE_LOW_SPEED_INDEX) {
                    msleep(PRINCE_LOW_SPEED_WAIT);
                } else {
                    msleep(PRINCE_HIGH_SPEED_WAIT); 
                }
                if (!(last_rx_desc->command_status & DMA_CMD_STAT_OWNER)) {
                    break;
                }
            }
            if (kx < PRINCE_DMA_RETRY_PASS) {
                break;
            }
        }
    }

    /*
     *   For Bisync, the user monitors CRC error.
     *   We don't want to look at CRC error. It has no meaning. 
     */ 
    if (s_ds->protocol == PRINCE_BISYNC) {
        rx_err_mask = DMA_CMD_STAT_ERR_MASK_BISYNC;
    } else {
        rx_err_mask = DMA_CMD_STAT_ERR_MASK;
    }

    for (ix = 0; ix < s_ds->num_buff; ix++) {
        /*
         * Check to make sure that the Data is been received.
         */
        if (rx_desc->command_status & DMA_CMD_STAT_OWNER) {
            /*
             * Data was not received. Check the transmit side also.
             */
            if (tx_desc->command_status & DMA_CMD_STAT_OWNER) {
                plugser_trap(s_ds, priority);
                printf("\n -Tx Descr %d at 0x%08x --\n",
                       ix, tx_desc);
                cterr('f',0," Tx of Data did not occur, speed_idx[%d] "
                      "proto[%d], wait_level[%d]",
                      s_ds->speed_idx, s_ds->protocol, wait_level);
                return (FAILED);
            } else {
                plugser_trap(s_ds, priority);
                printf("\n -Rx Descr %d at 0x%08x --\n",
                       ix, rx_desc);
                cterr('f',0," Rx of Data did not occur, speed_idx[%d] "
                      "proto[%d], wait_level[%d]",
                      s_ds->speed_idx, s_ds->protocol, wait_level);
                return (FAILED);
            }
        }
        /*
         *  Check the error status set by FPGA. 
         *  Save the error and report it later if data compare mismatch.
         */
        if (tx_desc->command_status & DMA_CMD_STAT_ERR_MASK) {
            tx_status = tx_desc->command_status;
        } else if (rx_desc->command_status & rx_err_mask) {
            rx_status = rx_desc->command_status;
        }

        rx_desc++;
        tx_desc++;
    }

    return (PASSED);
}

/******************************************************************************
 * Function: plugser_check_dma_received_data
 *
 * Description: This function compares the received data to the data that was 
 *              transmitted.
 *
 * Input: s_ds - pointer to serial data structure 
 * Output: PASSED or FAILED.
 ******************************************************************************/
ushort plugser_check_dma_received_data (prince_serial_ds_t *s_ds)
{
    ushort size, ix;
    uchar *tx_data, *rx_data;

    tx_data = (uchar *) s_ds->tx_buf_addr_virt;
    rx_data = (uchar *) s_ds->rx_buf_addr_virt;

    /* CRC two bytes need not be checked */
    if (s_ds->crc_rx_buf_opt) {
        size = ( s_ds->num_buff * s_ds->buff_size - PRINCE_CRC_RESERVED );
    } else  {
        size = ( s_ds->num_buff * s_ds->buff_size  );
    }
    /* sync transparent, shift right one byte */
    if (s_ds->protocol == PRINCE_TRANSPARENT) {
        rx_data++;
    }
     
    for (ix = 0; ix < size; ix++) {
        if (*rx_data != *tx_data) {
            cterr('f',0," Byte %x Data Mismatch. tx = 0x%x rx = 0x%x "
                  "tx_status = 0x%x rx_status = 0x%x", ix,
                  *tx_data, *rx_data, tx_status, rx_status);
            return (FAILED);
        }
        *tx_data++;
        *rx_data++;
    }

    return (PASSED);
}

/******************************************************************************
 * Function: plugser_init_tx_dma_buffers
 *
 * Description: This function will initializes the tx buffers of the channel.
 *              It will fill buffer with Test data.
 *
 * Input: s_ds - Pointer to the prince data structure.
 *        data - Data to be used.
 *        pattern - Which data pattern to use for test.
 * Output: None
 ******************************************************************************/
void plugser_init_tx_dma_buffers (prince_serial_ds_t *s_ds, uchar data, uchar pattern) 
{
    ushort ix, size;
    uchar *data_ptr;

    data_ptr = (uchar *) s_ds->tx_buf_addr_virt;
    size = ( s_ds->num_buff * s_ds->buff_size );

    switch (pattern) {
    case PRINCE_INC_PAT:
        for (ix = 0; ix < size; ix++)
            *data_ptr++ = data++;
        break;

    case PRINCE_DEC_PAT:
        for (ix = 0; ix < size; ix++)
            *data_ptr++ = data--;
        break;

    case PRINCE_DATA_PAT:
        for (ix = 0; ix < size; ix++)
            *data_ptr++ = data;
        break;

    default:
        break;
    }
   
    if (s_ds->protocol == PRINCE_BISYNC) {
        /* EBCDIC */
        data_ptr = (uchar *) s_ds->tx_buf_addr_virt;
        *data_ptr = PRINCE_STX;
        *(data_ptr + size - /*PRINCE_CRC_RESERVED*/2 - 1) = PRINCE_ETX;  
    }
    if (s_ds->protocol == PRINCE_TRANSPARENT) {
        data_ptr = (uchar *) s_ds->tx_buf_addr_virt;
        *data_ptr = 0x7e;
    }
}

/******************************************************************************
 * Function: plugser_init_rx_dma_buffers
 *
 * Description: This function will initializes the rx buffers of the channel with
 *              zeroes.
 *
 * Input: s_ds - Pointer to the serial data structure.
 * Output: None
 ******************************************************************************/
void plugser_init_rx_dma_buffers (prince_serial_ds_t *s_ds) 
{
    ushort ix, size;
    uchar *data_ptr;

    data_ptr = (uchar *) s_ds->rx_buf_addr_virt;
    size = ( s_ds->num_buff * s_ds->buff_size );

    for (ix = 0; ix < size; ix++) {
        *data_ptr++ = 0x00;
    }
}

/******************************************************************************
 * Function: plugser_init_tx_dma_ring 
 *
 * Description: This function will initializes the tx DMA ring
 *
 * Input: s_ds - Pointer to the serial data structure.
 *      : scc_base_addr - FPGA SCC register base addr
 *      : run_mode - poll or interrupt
 * Output: PASSED / FAILED 
 ******************************************************************************/
void plugser_init_tx_dma_ring (uchar run_mode, ulong scc_base_addr,
                       prince_serial_ds_t *s_ds)
{
    ushort ix;
    prince_dma_bd_t *tx_desc;
    ulong tx_desc_size = sizeof(prince_dma_bd_t);
    ulong current_desc_addr, current_buff_addr;

    tx_desc = (prince_dma_bd_t *) s_ds->tx_desc_addr_virt;
    current_desc_addr = s_ds->tx_desc_addr_virt;
    current_buff_addr = s_ds->tx_buf_addr_phys;

    for (ix = 0; ix < s_ds->num_buff; ix++) {
        /* 
         *  Mark the starting buffer
         */
        tx_desc->command_status = 0;
        if (ix == 0) {
            tx_desc->command_status |= DMA_CMD_STAT_SEQ_START;
        }

        /*
         *  Set buffer pointer
         */
        tx_desc->buff_ptr = current_buff_addr;

        /*
         *  Mark the ending buffer
         */
        if (ix == (s_ds->num_buff -1)) {
            tx_desc->command_status |= DMA_CMD_STAT_SEQ_END;
            if (s_ds->buff_size < MAX_PRINCE_BUFFER_SIZE) {
                tx_desc->command_status |= DMA_EARLY_TRANSMIT;
            }
        }

        /*
         *  last buffer - reserved 2 CRC bytes. For HDLC, fpga
         *  added 2 more bytes as CRC is generated by hardware.
         *  For bisync, fpga just pass the same 2 bytes CRC
         *  without changing it as CRC is generated by software.
         *  So HDLC receives 2 more dummy bytes plus 2 CRC bytes.
         *  Bisync receives just 2 CRC bytes.
         */
        if (ix == (s_ds->num_buff - 1) && s_ds->crc_rx_buf_opt) {
            tx_desc->byte_count = s_ds->buff_size - PRINCE_CRC_RESERVED +2;
        } else {
            tx_desc->byte_count = s_ds->buff_size ;
        }

        /*
         *  Set the ownership bit to FPGA 
         */
        tx_desc->command_status |= DMA_CMD_STAT_OWNER;

        /* 
         *  Set expecting interrupt
         */
        if (run_mode == PRINCE_INT_MODE) {
            tx_desc->command_status |= DMA_CMD_STAT_TX_BUF_INT;
        } else if (run_mode == PRINCE_FRAME_INT_MODE) {
            tx_desc->command_status |= DMA_CMD_STAT_TX_FRAME_INT;
        }

        if (diagflag_xram & D_TRACE) {
            printf("\n -Tx Descriptor number%d at 0x%08x --", ix, tx_desc );
            printf("\n  command= 0x%04x len= 0x%04x ptr= 0x%08x ",
                   tx_desc->command_status,
                   tx_desc->byte_count,
                   tx_desc->buff_ptr);
        }

        tx_desc++;
        current_desc_addr = ( current_desc_addr + tx_desc_size );
        current_buff_addr = ( current_buff_addr + s_ds->buff_size);
    }

    /* this is required by FPGA */
    tx_desc->command_status = 0;
    tx_desc->byte_count = 0;
    tx_desc->buff_ptr = 0;

    return;
}

/******************************************************************************
 * Function: plugser_init_rx_dma_ring 
 *
 * Description: This function will initializes the rx DMA ring
 *
 * Input: s_ds - Pointer to the serial data structure.
 *      : scc_base_addr
 *      : run_mode - poll or interrupt
 * Output: PASSED / FAILED 
 ******************************************************************************/
void plugser_init_rx_dma_ring (uchar run_mode, ulong scc_base_addr,
                             prince_serial_ds_t *s_ds)
{
    ushort ix;
    prince_dma_bd_t *rx_desc;
    ulong rx_desc_size = sizeof(prince_dma_bd_t);
    ulong current_desc_addr, current_buff_addr;

    rx_desc = (prince_dma_bd_t *) s_ds->rx_desc_addr_virt;
    current_desc_addr = s_ds->rx_desc_addr_virt;
    current_buff_addr = s_ds->rx_buf_addr_phys;

    for (ix = 0; ix < s_ds->num_buff; ix++) {

        rx_desc->command_status = 0; /* zero it out */
        rx_desc->command_status |= DMA_CMD_STAT_OWNER;  /* Cheerio owns */

        if (run_mode == PRINCE_INT_MODE) {
            rx_desc->command_status |= DMA_CMD_STAT_RX_BUF_INT;  
        } else if (run_mode == PRINCE_FRAME_INT_MODE) {
            rx_desc->command_status |= DMA_CMD_STAT_RX_FRAME_INT;
        }

        rx_desc->byte_count = s_ds->buff_size;
        rx_desc->buff_ptr = current_buff_addr;

        /* 
         *  save the address of last Rx descriptor 
         *  so we can check the completion.
         */
        if (ix == (s_ds->num_buff -1) ) {
            s_ds->last_rx_desc_addr = (ulong) rx_desc;
        }

        if (diagflag_xram & D_TRACE) {
            printf("\n -Rx Descriptor number%d at 0x%08x --", ix, rx_desc );
            printf("\n  command= 0x%04x len= 0x%04x ptr= 0x%08x ", 
                rx_desc->command_status,
                rx_desc->byte_count,
                rx_desc->buff_ptr);
        }

        rx_desc++;
        current_desc_addr = ( current_desc_addr + rx_desc_size );
        current_buff_addr = ( current_buff_addr + s_ds->buff_size );
    }

    /* this is required by FPGA */
    rx_desc->command_status |= DMA_CMD_STAT_SEQ_END;
    rx_desc->byte_count = 0;
    rx_desc->buff_ptr = 0;

    /* 
     *  Enable RX DMA
     */
    s_ds->proto_regs_addr->command_reg = PRINCE_CMD_START_RX;

    return;
}

/******************************************************************************
 * Function: plugser_cleanup_dma_ring 
 *
 * Description: This function will free up memory allocated for DMA ring 
 *
 * Input: s_ds - pointer to serial data structure
 *        scc_base_addr - FPGA SCC register base addr
 * Output: None
 ******************************************************************************/
void plugser_cleanup_dma_ring (ulong scc_base_addr, prince_serial_ds_t *s_ds)
{
    s_ds->tx_buf_addr_virt = NULL;
    s_ds->tx_buf_addr_phys = NULL;
    s_ds->rx_buf_addr_virt = NULL;
    s_ds->rx_buf_addr_phys = NULL;
    s_ds->tx_desc_addr_virt = NULL;
    s_ds->tx_desc_addr_phys = NULL;
    s_ds->rx_desc_addr_virt = NULL;
    s_ds->rx_desc_addr_phys = NULL;
}

/******************************************************************************
 * Function: prince_cleanup_serial 
 *
 * Description: This function will clean up the setup for the loopback test. 
 *
 * Input:  - s_ds       - pointer to serial data structure 
 *           priority   - TX priority
 * Output: None
 ******************************************************************************/
void plugser_cleanup_serial (prince_serial_ds_t *s_ds, int priority)
{
    int ix;

    switch (priority) {
    case PRINCE_SCC_HIGH_PRIORITY_1:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_HP1_STOP_TX_RX;
        break;
    case PRINCE_SCC_HIGH_PRIORITY_2:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_HP2_STOP_TX_RX;
        break;
    case PRINCE_SCC_LOW_PRIORITY:
        s_ds->proto_regs_addr->command_reg = PRINCE_CMD_LP_STOP_TX_RX;
        break;
    default:
        break;
    }

    /* cleanup async timer */
    s_ds->cntrl_1_regs_addr->timer_1_2_pgm = 0;
    s_ds->cntrl_1_regs_addr->timer_3_4_pgm = 0;
    s_ds->cntrl_1_regs_addr->timer_5_6_pgm = 0;
    s_ds->cntrl_1_regs_addr->timer_7_8_pgm = 0;

    /* cleanup ppp context area */
    s_ds->ppp_tx_regs_addr->context = 0;
    s_ds->ppp_rx_regs_addr->context = 0;

    /* cleanup bisync context area */
    if (s_ds->protocol == PRINCE_BISYNC || 
        s_ds->protocol == PRINCE_TRANSPARENT) {
        s_ds->bcc_tx_regs_addr->context = 0;
        s_ds->bcc_rx_regs_addr->context = 0;
        for (ix = 0; ix < 3; ix++) {
            s_ds->bcc_tx_regs_addr->unused[ix] = 0;
            s_ds->bcc_rx_regs_addr->unused[ix] = 0;
        }
    }

    /* cleanup mode for poll mode only */
    if (s_ds->run_mode == PRINCE_POLL_MODE)
        prince_cleanup_mode_bits(s_ds);

    /* cleanup flow control */
    s_ds->proto_regs_addr->flow_cntrl_config = 0x0;

    /* RTS / DTR swap mux disable */
    s_ds->serial_itf_addr->serial_itf_cntrl &= ~(0x1000);
}

/******************************************************************************
 * Function: prince_cleanup_mode_bits
 *
 * Description: This function cleans up mode in DMA mode register. 
 *
 * Input: s_ds - pointer to serial data structure
 * Output: None 
 ******************************************************************************/
void prince_cleanup_mode_bits (prince_serial_ds_t *s_ds)
{
    s_ds->proto_regs_addr->mode_reg &= ~PRINCE_MODE_MASK;
}

/******************************************************************************
 * Function: plugser_init_serial_ds
 *
 * Description: This function initialized the serial data struct. 
 *
 * Input: s_ds - pointer to serial data structure
 * Output: None
 ******************************************************************************/
void plugser_init_serial_ds (prince_serial_ds_t *s_ds)
{
    s_ds->proto_regs_addr = 0;
    s_ds->dma_tx_regs_addr = 0;
    s_ds->dma_rx_regs_addr = 0;
    s_ds->ppp_tx_regs_addr = 0;
    s_ds->ppp_rx_regs_addr = 0;
    s_ds->bcc_tx_regs_addr = 0;
    s_ds->bcc_rx_regs_addr = 0;
    s_ds->serial_itf_addr = 0;
    s_ds->cntrl_1_regs_addr = 0;
    s_ds->cntrl_2_regs_addr = 0;
    s_ds->base_addr = 0;
    s_ds->speed_idx = 0;
    s_ds->run_mode = 0;
    s_ds->port_num = s_ds->org_port_num = s_ds->brg_num = s_ds->divider = 0;
    s_ds->clk_src = s_ds->lpbk_mode = s_ds->protocol = s_ds->ctrl_id = 0;
    s_ds->num_buff = s_ds->buff_size = s_ds->tx_desc_addr_virt = 0;
    s_ds->rx_desc_addr_virt = 0;
    s_ds->tx_buf_addr_virt = s_ds->tx_buf_addr_virt = 0;
    s_ds->last_rx_desc_addr = 0;
    s_ds->baudrate = s_ds->crc_rx_buf_opt = 0;
    s_ds->crc_enb = PRINCE_MODE_CRC;

}

/*******************************************************************************
 * Function: plugser_trap
 *
 * Description: This function trap some helpful information for users and 
 *              trigger logical analyzer. 
 *
 * Input: s_ds - pointer to serial data structure
 *
 * Output: None 
 ******************************************************************************/
void plugser_trap (prince_serial_ds_t *s_ds, int priority) 
{
    volatile ushort *s_ptr;
    ulong scc_base_addr;
    char tmp_name_buf[30];
    uint des_baud=0;
    prince_dma_bd_t *tx_desc;
    prince_dma_bd_t *rx_desc;
    int ix;

    tx_desc = (prince_dma_bd_t *) s_ds->tx_desc_addr_virt;
    rx_desc = (prince_dma_bd_t *) s_ds->rx_desc_addr_virt;

    s_ptr = (volatile ushort *)((char *)s_ds->cntrl_2_regs_addr + FREQ_CNT_PORT_SELECT);
    *s_ptr = LOGICAL_ANALYZER_TRIGGER;   /* logical analyzer trigger bit */
    *s_ptr = 0;
    scc_base_addr = s_ds->base_addr;
    plugser_display_dma_regs(scc_base_addr, PRINCE_DMA_TX, 2,
                          s_ds->port_num, priority);
    plugser_display_dma_regs(scc_base_addr, PRINCE_DMA_RX, 2,
                          s_ds->port_num, priority);
    plugser_display_cntrl_1_regs(scc_base_addr);
    plugser_display_cntrl_2_regs(scc_base_addr);
    plugser_display_proto_regs(scc_base_addr,  2,
                            s_ds->port_num);
    plugser_display_serial_itf_regs(scc_base_addr, 2, s_ds->port_num);

    plugser_form_name_buf(tmp_name_buf, s_ds, priority);
    if (s_ds->clk_src == PRINCE_CLK_SRC_SYNC ) {
        des_baud = plugser_sync_baud[s_ds->speed_idx].desired_baud;
    } else if (s_ds->clk_src == PRINCE_CLK_SRC_ASYNC) {
        des_baud = plugser_async_baud[s_ds->speed_idx].desired_baud;
    }
    printf("\n -%schan %d @speed %d --\n", tmp_name_buf, s_ds->org_port_num,
           des_baud);

    for (ix = 0; ix < s_ds->num_buff; ix++) {
        printf("\n tx command_stat[0x%04x] buf_len[0x%04x] buf_ptr[0x%08x]",
               tx_desc->command_status, tx_desc->byte_count, tx_desc->buff_ptr);
               tx_desc++;
    }
    for (ix = 0; ix < s_ds->num_buff; ix++) {
        printf("\n rx command_stat[0x%04x] buf_len[0x%04x] buf_ptr[0x%08x]",
               rx_desc->command_status, rx_desc->byte_count, rx_desc->buff_ptr);
               rx_desc++;
    }
}

/******************************************************************************
 * Function: plugser_form_name_buf
 *
 * Description: This function form a name buffer to print 
 *
 * Input: pointer to name buffer 
 *        s_ds      - pointer to serial data structure 
 *        priority  - TX priority
 * Output: PASSED / FAILED
 ******************************************************************************/
int plugser_form_name_buf (char *tmp_name_buf, prince_serial_ds_t *s_ds, int priority)
{
    if (s_ds->lpbk_mode == PRINCE_SCC_INT_LOOPBACK) {
        strcpy (tmp_name_buf, "Int ");
    } else {
        strcpy (tmp_name_buf, "Ext ");
    }

    switch (s_ds->protocol) {
    case PRINCE_HDLC:
        strcat (tmp_name_buf, "HDLC ");
        break;
    case PRINCE_BISYNC:
        strcat (tmp_name_buf, "Bisync ");
        break;
    case PRINCE_UART_ASYNC:
        strcat (tmp_name_buf, "Uart Async ");
        break;
    case PRINCE_UART_PPP:
        strcat (tmp_name_buf, "Uart PPP ");
        break;
    case PRINCE_TRANSPARENT:
        strcat (tmp_name_buf, "Transp ");
        break;
    default:
        break;
    }
    switch (priority) {
    case PRINCE_SCC_HIGH_PRIORITY_1:
        strcat (tmp_name_buf, "HP ");
        break;
    case PRINCE_SCC_HIGH_PRIORITY_2:
        strcat (tmp_name_buf, "HP2 ");
        break;
    case PRINCE_SCC_LOW_PRIORITY:
        strcat (tmp_name_buf, "LP ");
        break;
    default:
        break;
    }
    return (PASSED);
}

/******************************************************************************
 * Function: plugser_set_async_timer
 *
 * Description: This function set up async timer for async/PPP engine
 *
 * Input: s_ds - pointer to serial data structure
 *        value - timer value
 * Output: None
 ******************************************************************************/
void plugser_set_async_timer (prince_serial_ds_t *s_ds, ushort value)
{
    volatile ushort * timer_ptr;
    ushort offset;

    /*
     *  program async timer, for diag loopback, we
     *  need a big enough value to complete the transfer.
     */
    offset = s_ds->port_num / 2;
    timer_ptr = (volatile ushort *) (&s_ds->cntrl_1_regs_addr->timer_1_2_pgm
                + offset);
    if (s_ds->port_num %2) {
        *timer_ptr = value << 8;
    } else {
        *timer_ptr = value;
    }
}


/******** History ******** 
$Log: plugser_scc_diag.c,v $
Revision 1.4  2018/08/02 09:35:01  iachang
Merge Pluggable Serial from branch star-branch-c9xx to main trunk

Revision 1.3  2018/02/09 09:17:38  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:55:09  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:59:01  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.6  2017/12/22 00:38:13  lucywang
Removed unused code

Revision 1.1.4.5  2017/09/14 07:20:45  lucywang
only execute external loopback when Ext. loopback flag is on

Revision 1.1.4.4  2017/08/25 10:27:18  lucywang
modified for ASYNC test and FPGA update

Revision 1.1.4.3  2017/08/22 04:13:19  lucywang
add firmware upgrade for 7007 FPGA

Revision 1.1.4.2  2017/08/08 07:43:38  hondwang
add pluggable serial for star-branch-c9xx

Revision 1.1.2.1  2017/07/31 10:49:58  lucywang
add pluggable serial code of host and module



$Endlog$
*/
