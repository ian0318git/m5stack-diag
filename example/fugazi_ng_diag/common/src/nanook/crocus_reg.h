/* $Id: crocus_reg.h,v 1.2 2019/12/11 10:10:27 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/crocus_reg.h,v $
 *------------------------------------------------------------------
 * crocus_reg.h
 *      Nanook projects - memory map and register structures.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __CROCUS_REG__
#define __CROCUS_REG__

#define CROCUS_GLOBAL_OFFSET                  0x0000
#define CROCUS_SPI_FLASH_OFFSET               0x0100
#define CROCUS_CTRL_INTR_OFFSET               0x8000
#define CROCUS_SCC_OFFSET                     0x8080
#define CROCUS_PPP_TX_OFFSET                  0x8100
#define CROCUS_PPP_RX_OFFSET                  0x8180
#define CROCUS_SERIAL_INTERFACE_OFFSET        0x8200
#define CROCUS_RING_BUFFER_CTRL_OFFSET        0x8300

#define CROCUS_PER_CHAN_REG_SIZE              0x400

#define CROCUS_TX_RING_BUFFER_OFFSET          0x10000
#define CROCUS_RX_RING_BUFFER_OFFSET          0x20000
#define CROCUS_TX_RING_BUFFER_SIZE_PER_CHAN   0x800
#define CROCUS_RX_RING_BUFFER_SIZE_PER_CHAN   0x1000


/* structure for Global regs 0x0000_0000 */
typedef struct crocus_global_regs_t_ {
    volatile uint   revision_ID;              /*0x0000*/
	volatile uint   status_reg;               /*0x0004*/
	volatile uint   led_reg;                  /*0x0008*/
    volatile uint   timer_0_3_intr_status;    /*0x000c*/
    volatile uint   timer_0_3_intr_mask;      /*0x0010*/
    volatile uint   timer_0_1_pgm;            /*0x0014*/
    volatile uint   timer_2_3_pgm;            /*0x0018*/
    volatile uint   reserver1[8185];          /*0x001c-0x7fff*/
} crocus_global_regs_t;

/* structure for Control and Interrupt regs (per channel) 0x0000_8000 */
typedef struct crocus_ctrl_intr_regs_t_ {
    volatile uint   intr_status;              /*0x0000*/
    volatile uint   intr_mask;                /*0x0004*/
    volatile uint   timer_pgm;                /*0x0008*/
    volatile uint   reserve1[29];             /*0x000c-0x007f*/
} crocus_ctrl_intr_regs_t;

/* structure for Serial Communication Controller regs (per channel) 0x0000_8080 */
typedef struct crocus_scc_regs_t_ {
    volatile uint   mode_reg;                 /*0x0000*/
    volatile uint   flag_config;              /*0x0004*/
    volatile uint   flow_cntrl_config;        /*0x0008*/
    volatile uint   intr_status;              /*0x000c*/
    volatile uint   intr_mask;                /*0x0010*/
    volatile uint   command_reg;              /*0x0014*/
    volatile uint   reserve1[26];             /*0x0018-0x007f*/
} crocus_scc_regs_t;

/* structure for PPP TX regs(per channel) 0x0000_8100 */
typedef struct crocus_ppp_tx_regs_t_ {
    volatile uint   tx_ctrl_char_map;         /*0x0000*/
    volatile uint   tx_special;               /*0x0004*/
    volatile uint   context;                  /*0x0008*/
    volatile uint   reserve1[29];             /*0x000c-0x007f*/
} crocus_ppp_tx_regs_t;

/* structure for PPP RX regs(per channel) 0x0000_8180 */
typedef struct crocus_ppp_rx_regs_t_ {
    volatile uint   rx_ctrl_char_map;         /*0x0004*/
    volatile uint   rx_special;               /*0x0008*/
    volatile uint   context;                  /*0x0008*/
    volatile uint   reserve1[29];             /*0x000c-0x007f*/
} crocus_ppp_rx_regs_t;

/* structure for Serial Line Interafce regs(per channel) 0x0000_8200 */
typedef struct crocus_serial_itf_t_ {
    volatile uint   serial_itf_cntrl;        /*0x0000*/
    volatile uint   modem_cntrl;             /*0x0004*/
    volatile uint   flow_cntrl;              /*0x0008*/
    volatile uint   brg_divider;             /*0x000c*/
    volatile uint   modem_intr_status;       /*0x0010*/
    volatile uint   freq_cnt_port_select;    /*0x0014*/
    volatile uint   freq_cnt;                /*0x0018*/
    volatile uint   reserver1[57];           /*0x001c-0x00ff*/
} crocus_serial_itf_t;

/* structure for Ring Buffer Control regs(per channel) 0x0000_8300 */
typedef struct crocus_ring_buf_regs_t_{
    volatile uint   rx_start;                /*0x0000*/
    volatile uint   rx_end;                  /*0x0004*/
    volatile uint   tx_start;                /*0x0008*/
    volatile uint   tx_end;                  /*0x000c*/
    volatile uint   ctrl;                    /*0x0010*/
	volatile uint   rx_status;               /*0x0014*/
	volatile uint   tx_status;               /*0x0018*/
    volatile uint   intr_status;             /*0x001c*/
    volatile uint   intr_mask;               /*0x0020*/
    volatile uint   reserve1[55];            /*0x0024-0x00ff*/
} crocus_ring_buf_regs_t;

typedef struct crocus_chan_ctrl_status_regs_t_ {
    crocus_ctrl_intr_regs_t        ctrl_intr_regs;
    crocus_scc_regs_t              scc_regs;
    crocus_ppp_tx_regs_t           ppp_tx_regs;
    crocus_ppp_rx_regs_t           ppp_rx_regs;
    crocus_serial_itf_t            serial_itf;
    crocus_ring_buf_regs_t         ring_buf_regs;
} crocus_chan_ctrl_status_regs_t;

/* structure for Crocus memory map */
typedef struct crocus_regs_t_ {
    crocus_global_regs_t           global_regs;       /* 0x0000_0000-0x0000_8000 */
    crocus_chan_ctrl_status_regs_t chan_regs[32];     /* 0x0000_8000-0x0001_0000 */
} crocus_regs_t;

#endif

/******** History ********
$Log: crocus_reg.h,v $
Revision 1.2  2019/12/11 10:10:27  lucywang
Merged Nanook to main trunk


$Endlog$
*/
