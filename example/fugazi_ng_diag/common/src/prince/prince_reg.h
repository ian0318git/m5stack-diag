/* $Id: prince_reg.h,v 1.4 2018/01/17 11:48:04 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_reg.h,v $
 *------------------------------------------------------------------
 * prince_reg.h 
 *      Prince projects - NIM 1T/2T/4T
 *                        memory map and register structures.
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PRINCE_REG__
#define __PRINCE_REG__

typedef struct sys_csr_reg_t_ {
    ulong fpga_revision;                 /* 0x0000, FPGA Revision Reg */
    ulong fpga_timestamp;                /* 0x0004, FPGA Timestamp Reg */
    ulong leds_sts;                      /* 0x0008, LEDs and Status Reg */
    ulong mac_ctrl;                      /* 0x000c, MAC Control Reg */
    ulong mac_pause_request;             /* 0x0010, MAC Pause Request Reg */
    ulong rgmii_inband_sts;              /* 0x0014, RGMII Inband Status Reg */
    ulong scratchpad;                    /* 0x0018, Scratchpad Reg */
} sys_csr_reg_t;

/* Defines for LED and Status Register */
#define BOARD_ID_MASK       0x00000300
#define PRI_INTF_READY      0x00000010
#define LED_LPBK_BLINK      0x00000008
#define LED_ACTIV_BLINK     0x00000004
#define LED_LPBK            0x00000002
#define LED_ACTIV           0x00000001

#define BOARD_ID_SHIFT      8
#define BOARD_ID_NIM_1T     1
#define BOARD_ID_NIM_2T     2
#define BOARD_ID_NIM_4T     0

#define NEW_FPGA_VERSION    0x121

#define ZYNC_GE_MAC_CSR_OFFSET              0x10000
#define ZYNC_GE_MAC_STAT_COUNTER_OFFSET     0x0200

typedef struct ge_mac_stat_counter_reg_t_ {
    ulong rx_bytes[2];                   /* 0x0000, Received bytes */
    ulong tx_bytes[2];                   /* 0x0008, Transmitted bytes */
    ulong rx_uz_frames[2];               /* 0x0010, RX Undersize frames */
    ulong rx_frag_frames[2];             /* 0x0018, RX Fragment frames */
    ulong rx_64_byte_frames[2];          /* 0x0020, RX 64 byte frames */
    ulong rx_65_127_byte_frames[2];      /* 0x0028, RX 65-127 byte frames */
    ulong rx_128_255_byte_frames[2];     /* 0x0030, RX 128-255 byte frames */
    ulong rx_256_511_byte_frames[2];     /* 0x0038, RX 256-511 byte frames */
    ulong rx_512_1023_byte_frames[2];    /* 0x0040, RX 512-1023 byte frames */
    ulong rx_1024_max_byte_frames[2];    /* 0x0048, RX 1024-max byte frames */
    ulong rx_os_frames[2];               /* 0x0050, RX Oversize frames */
    
    ulong tx_64_byte_frames[2];          /* 0x0058, TX 64 byte frames */
    ulong tx_65_127_byte_frames[2];      /* 0x0060, TX 65-127 byte frames */
    ulong tx_128_255_byte_frames[2];     /* 0x0068, TX 128-255 byte frames */
    ulong tx_256_511_byte_frames[2];     /* 0x0070, TX 256-511 byte frames */
    ulong tx_512_1023_byte_frames[2];    /* 0x0078, TX 512-1023 byte frames */
    ulong tx_1024_max_byte_frames[2];    /* 0x0080, TX 1024-max byte frames */
    ulong tx_os_frames[2];               /* 0x0088, TX Oversize frames */
    
    ulong rx_good_frames[2];             /* 0x0090, RX Good frames */
    ulong rx_fc_seq_errs[2];             /* 0x0098, RX Frame Check Sequence Errors */
    ulong rx_good_bc_frames[2];          /* 0x00a0, RX Good Broadcast frames */
    ulong rx_good_mc_frames[2];          /* 0x00a8, RX Good Multicase frames */
    ulong rx_good_cntl_frames[2];        /* 0x00b0, RX Good Control frames */
    ulong rx_len_type_oor[2];            /* 0x00b8, RX Length/Type Out of Range */
    ulong rx_good_vlan_frames[2];        /* 0x00c0, RX Good VLAN Tagged frames */
    ulong rx_good_pause_frames[2];       /* 0x00c8, RX Good Pause frames */
    ulong rx_bad_opcode[2];              /* 0x00d0, RX Bad Opcode */
    
    ulong tx_good_frames[2];             /* 0x00d8, TX Good frames */
    ulong tx_good_bc_frames[2];          /* 0x00e0, TX Good Broadcast frames */
    ulong tx_good_mc_frames[2];          /* 0x00e8, TX Good Multicase frames */
    ulong tx_good_ur_errs[2];            /* 0x00f0, TX Good Underrun Errors */
    ulong tx_good_cntl_frames[2];        /* 0x00f8, TX Good Control frames */
    ulong tx_good_vlan_frames[2];        /* 0x0100, TX Good VLAN Tagged frames */
    ulong tx_good_pause_frames[2];       /* 0x0108, TX Good Pause frames */
    ulong tx_sc_frames[2];               /* 0x0110, TX Single Collision frames */
    ulong tx_mc_frames[2];               /* 0x0118, TX Multiple Collision frames */
    ulong tx_deferred[2];                /* 0x0120, TX Deferred */
    ulong tx_late_collision[2];          /* 0x0128, TX Late Collisions */
    ulong tx_exce_collision[2];          /* 0x0130, TX Excess Collisions */
    ulong tx_exce_deferral[2];           /* 0x0138, TX Excess Deferral */
    ulong tx_align_err[2];               /* 0x0140, TX Alignment Errors */
} ge_mac_stat_counter_reg_t;

#define ZYNC_GE_MAC_CFG_OFFSET              0x0400

typedef struct ge_mac_cfg_reg_t_ {
    volatile ulong rx_cfg_wd_0;                      /* 0x0000, Receiver Config Word0 */
    volatile ulong rx_cfg_wd_1;                      /* 0x0004, Receiver Config Word1 */
    volatile ulong tx_cfg;                           /* 0x0008, Transmitter Config */
    volatile ulong flow_ctrl;                        /* 0x000c, Flow Control Config */
    volatile ulong speed_cfg;                        /* 0x0010, Speed Config */
    volatile ulong rx_max_fram_cfg;                  /* 0x0014, RX Max Frame Config */
    volatile ulong tx_max_fram_cfg;                  /* 0x0018, TX Max Frame Config */
    volatile ulong reserve[55];                      /* 0x001c - 0x00f4 */
    volatile ulong id;                               /* 0x00f8, ID Register */
    volatile ulong ability;                          /* 0x00fc, Ability Register */
} ge_mac_cfg_reg_t;

#define ZYNC_GE_MDIO_INTERFACE_OFFSET       0x0500

typedef struct ge_mdio_cfg_reg_t_ {
    volatile ulong mdio_cfg_wd_0;                    /* 0x0000, MDIO Config Word0 */
    volatile ulong mdio_cfg_wd_1;                    /* 0x0004, MDIO Config Word1 */
    volatile ulong mdio_tx_data;                     /* 0x0008, MDIO TX Data */
    volatile ulong mdio_rx_data;                     /* 0x000c, MDIO RX Data */
} ge_mdio_cfg_reg_t;

#define ZYNC_GE_INTR_OFFSET                 0x0600

typedef struct ge_intr_ctrl_cfg_reg_t_ {
    volatile ulong intr_sts;                         /* 0x0000, Interrupt Status Reg */
    volatile ulong reserve1[3];
    volatile ulong intr_pending;                     /* 0x0010, Interrupt Pending Reg */
    volatile ulong reserve2[3];
    volatile ulong intr_en;                          /* 0x0020, Interrupt Enable Reg */
    volatile ulong reserve3[3];
    volatile ulong intr_clr;                         /* 0x0030, Interrupt Clear Reg */
} ge_intr_ctrl_cfg_reg_t;

#define ZYNC_GE_FRAME_FILTER_OFFSET         0x0700

typedef struct ge_mac_reg_t_ {
    volatile ulong reserve1[0x80];                   /* 0x0000-0x01fc Reserved */
    ge_mac_stat_counter_reg_t   stat_cnt;            /* 0x0200-0x0344 */
    volatile ulong reserve2[0x2e];                   /* 0x0348-0x03fc Reserved */
    ge_mac_cfg_reg_t            config;              /* 0x0400-0x04fc */
    ge_mdio_cfg_reg_t           mdio_cfg;            /* 0x0500-0x050c */
    volatile ulong reserve3[0x3c];                   /* 0x0510-0x05fc Reserved */
    ge_intr_ctrl_cfg_reg_t      intr_ctrl;           /* 0x0600-0x0630 */
} ge_mac_reg_t;

#define ZYNC_GE_DMA_CSR_OFFSET              0x20000

typedef struct ge_dma_reg_t_ {
    volatile ulong ge_dma_ctrl;                   /* 0x0000, GE DMA Control Reg */
    volatile ulong ge_tx_drings_base_hi;          /* 0x0004, GE Tx Drings Base Hi Reg */
    volatile ulong ge_rx_drings_base_hi;          /* 0x0008, GE Tx Drings Base Hi Reg */
    volatile ulong ge_rx_buffer_size;             /* 0x000c, GE Rx Buffer Size Reg */
    volatile ulong ge_rx_flow_ctrl;               /* 0x0010, GE Rx Flow Control Reg */
    volatile ulong reserve1[3];
    volatile ulong ge_tx_drings_0_base_tail;      /* 0x0020, GE Tx Drings 0 Base/Tail Reg */
    volatile ulong ge_tx_drings_0_head;           /* 0x0024, GE Tx Drings 0 Head Reg */
    volatile ulong ge_tx_drings_0_add;            /* 0x0028, GE Tx Drings 0 Add Reg */
    volatile ulong ge_tx_drings_0_sts;            /* 0x002c, GE Tx Drings 0 Status Reg */
    volatile ulong ge_tx_dma_0_err_en;            /* 0x0030, GE Tx DMA 0 Error Enable Reg */
    volatile ulong reserve2[3];
    volatile ulong ge_tx_drings_1_base_tail;      /* 0x0040, GE Tx Drings 1 Base/Tail Reg */
    volatile ulong ge_tx_drings_1_head;           /* 0x0044, GE Tx Drings 1 Head Reg */
    volatile ulong ge_tx_drings_1_add;            /* 0x0048, GE Tx Drings 1 Add Reg */
    volatile ulong ge_tx_drings_1_sts;            /* 0x004c, GE Tx Drings 1 Status Reg */
    volatile ulong ge_tx_dma_1_err_en;            /* 0x0050, GE Tx DMA 1 Error Enable Reg */
    volatile ulong reserve3[3];
    volatile ulong ge_tx_drings_2_base_tail;      /* 0x0060, GE Tx Drings 2 Base/Tail Reg */
    volatile ulong ge_tx_drings_2_head;           /* 0x0064, GE Tx Drings 2 Head Reg */
    volatile ulong ge_tx_drings_2_add;            /* 0x0068, GE Tx Drings 2 Add Reg */
    volatile ulong ge_tx_drings_2_sts;            /* 0x006c, GE Tx Drings 2 Status Reg */
    volatile ulong ge_tx_dma_2_err_en;            /* 0x0070, GE Tx DMA 2 Error Enable Reg */
    volatile ulong reserve4[43];
    volatile ulong ge_rx_drings_base_head;        /* 0x00120, GE Rx Drings Base/Head Reg */
    volatile ulong ge_rx_drings_tail;             /* 0x00124, GE Rx Drings Tail Reg */
    volatile ulong ge_rx_drings_add;              /* 0x00128, GE Rx Drings Add Reg */
    volatile ulong ge_rx_drings_sts;              /* 0x0012c, GE Rx Drings Status Reg */
    volatile ulong ge_rx_dma_err_en;              /* 0x00130, GE Rx DMA Error Enable Reg */
    volatile ulong ge_rx_dma_overflow_count;      /* 0x00134, GE Rx DMA Overflow Count Reg */
    volatile ulong reserve5[2];
    volatile ulong ge_intr_sts;                   /* 0x00140, GE Interrupt Status Reg */
    volatile ulong ge_rx_intr_en;                 /* 0x00144, GE Rx Interrupt Enable Reg */
    volatile ulong ge_tx_intr_en;                 /* 0x00148, GE Tx Interrupt Enable Reg */
    volatile ulong ge_err_intr_sts;               /* 0x0014c, GE Error Interrupt Status Reg */
    volatile ulong ge_err_intr_en;                /* 0x00150, GE Error Interrupt Enable Reg */
    volatile ulong ge_axi_err_addr_latch;         /* 0x00154, GE AXI Error Address Latch Reg */
} ge_dma_reg_t;

#define ZYNC_SCC_CTRL_OFFSET                0x1000
#define ZYNC_SCC_RX_DMA_CTRL_OFFSET         0x2000
#define ZYNC_SCC_TX_DMA_CTRL_OFFSET         0x2200
#define ZYNC_SCC_BISYNC_TX_CTRL_OFFSET      0x2400
#define ZYNC_SCC_BISYNC_RX_CTRL_OFFSET      0x2a00
#define ZYNC_SCC_IFACE_CTRL_OFFSET          0x3000

/* structure for Prince protocol regs */
typedef struct prince_proto_regs_t_ {
//    volatile ushort mode_reg_lo;              /*0x0000*/
//    volatile ushort mode_reg_hi;              /*0x0002*/
    volatile ulong mode_reg;                  /*0x0000*/
    volatile ushort flag_config;              /*0x0004*/
    volatile ushort pad_config;               /*0x0006*/
    volatile ushort flow_cntrl_config;        /*0x0008*/
    volatile ushort intr_status;              /*0x000A*/
    volatile ushort intr_mask;                /*0x000C*/
    volatile ushort command_reg;              /*0x000E*/
} prince_proto_regs_t;

/* structure for Prince protocol regs -- HP2 priority */
typedef struct prince_proto_regs_hp2_t_ {
    volatile ushort reserved1[5];             /*0x0000-0x0008*/
    volatile ushort intr_status;              /*0x000A*/
    volatile ushort intr_mask;                /*0x000C*/
    volatile ushort reserved2;                /*0x000E*/
} prince_proto_regs_hp2_t;

/* structure for Prince protocol regs -- Low priority */
typedef struct prince_proto_regs_lp_t_ {
    volatile ushort reserved1[5];             /*0x0000-0x0008*/
    volatile ushort intr_status;              /*0x000A*/
    volatile ushort intr_mask;                /*0x000C*/
    volatile ushort reserved2;                /*0x000E*/
} prince_proto_regs_lp_t;

/* structure for Prince dma regs */
typedef struct prince_dma_regs_t_ {
    volatile ulong ring_start;                /*0x0000*/
    volatile ulong ring_size_index;           /*0x0004*/
    volatile ulong reserved[6];               /*0x0008-001f*/
} prince_dma_regs_t;

/* structure for Prince ppp regs */
typedef struct prince_ppp_regs_t_ {
    volatile ulong  cntrl_char_map;          /*0x0000*/
    volatile ulong  tx_special;              /*0x0004*/
    volatile ulong  unused;                  /*0x0008*/
    volatile ulong  context;                 /*0x000c*/
} prince_ppp_regs_t;

/* structure for Prince BCC regs */
typedef struct prince_bcc_regs_t_ {
    volatile ulong context;                  /*0x0000*/
    volatile ulong unused[3];                /*0x0004-0x000c*/
} prince_bcc_regs_t;

/* structure for Prince serial interafce regs */
typedef struct prince_serial_itf_t_ {
    volatile ushort serial_itf_cntrl;        /*0x0000*/
    volatile ushort modem_cntrl;             /*0x0002*/
    volatile ushort flow_cntrl;              /*0x0004*/
    volatile ushort brg_divider;             /*0x0006*/
    volatile ushort modem_intr_status;       /*0x0008*/
    volatile ushort unused1;                 /*0x000A*/
    volatile ushort unused2;                 /*0x000C*/
    volatile ushort unused3;                 /*0x000E*/
} prince_serial_itf_t;

/* structure for prince control_1 regs  */
typedef struct prince_cntrl_1_regs_t_ {
    volatile ushort cheerios_revision;            /*0x00*/
    volatile ushort cheerios_revision1;           /*0x02*/
    volatile ushort cheerios_cntrl;               /*0x04*/
    volatile ushort unused1[5];                   /*0x06-0x0E*/
    volatile ushort int_1_intr_status;            /*0x10*/
    volatile ushort int_2_intr_status;            /*0x12*/
    volatile ushort timer_1_16_intr_status;       /*0x14*/
    volatile ushort timer_17_20_intr_status;      /*0x16*/
    volatile ushort unused2[4];                   /*0x18-0x1E*/
    volatile ushort int_1_intr_mask;              /*0x20*/
    volatile ushort int_2_intr_mask;              /*0x22*/
    volatile ushort timer_1_16_intr_mask;         /*0x24*/
    volatile ushort timer_17_20_intr_mask;        /*0x26*/
    volatile ushort unused3[4];                   /*0x28-0x2E*/
    volatile ushort timer_1_2_pgm;                /*0x30*/
    volatile ushort timer_3_4_pgm;                /*0x32*/
    volatile ushort timer_5_6_pgm;                /*0x34*/
    volatile ushort timer_7_8_pgm;                /*0x36*/
    volatile ushort timer_9_10_pgm;               /*0x38*/
    volatile ushort timer_11_12_pgm;              /*0x3A*/
    volatile ushort timer_13_14_pgm;              /*0x3C*/
    volatile ushort timer_15_16_pgm;              /*0x3E*/
    volatile ushort timer_17_pgm;                 /*0x40*/
    volatile ushort timer_18_pgm;                 /*0x42*/
    volatile ushort timer_19_pgm;                 /*0x44*/
    volatile ushort timer_20_pgm;                 /*0x46*/
    volatile ushort bcc_text_tm_u;                /*0x48*/
    volatile ushort bcc_text_tm_l;                /*0x4a*/
    volatile ushort bcc_long_tm_u;                /*0x4c*/
    volatile ushort bcc_long_tm_l;                /*0x4e*/
    volatile ushort bcc_tm_en;                    /*0x50*/
    volatile ushort unused4[0x57];                /*0x52-0xFF*/
} prince_cntrl_1_regs_t;

/* structure for prince control_2 regs  */
typedef struct prince_cntrl_2_regs_t_ {
    volatile ushort tdm_cntrl;                    /*0x00*/
    volatile ushort tdma_8k_divider;              /*0x02*/
    volatile ushort tdmb_8k_divider;              /*0x04*/
    volatile ushort serial_intr_status;           /*0x06*/
    volatile ushort unused7;                      /*0x08*/
    volatile ushort freq_cnt_port_select;         /*0x0A*/
    volatile ushort freq_cnt_hi;                  /*0x0C*/
    volatile ushort freq_cnt_lo;                  /*0x0E*/
    volatile ushort pll1_cntrl;                   /*0x10*/
    volatile ushort pll1_8k_divider;              /*0x12*/
    volatile ushort pll1_pre_divider;             /*0x14*/
    volatile ushort pll1_post_divider;            /*0x16*/
    volatile ushort unused8[4];                   /*0x18-0x1E*/
    volatile ushort pll2_cntrl;                   /*0x20*/
    volatile ushort pll2_8k_divider;              /*0x22*/
    volatile ushort pll2_pre_divider;             /*0x24*/
    volatile ushort pll2_post_divider;            /*0x26*/
    volatile ushort unused9[4];                   /*0x28-0x2E*/
    volatile ushort unused10[0x68];               /*0x30-0xFF*/
} prince_cntrl_2_regs_t;

/* structure for Prince memory map */
#ifdef REVA
typedef struct prince_scc_regs_t_ {
    ushort reserved1[0x800];                /* 0x0000-0x0FFF */

    /* prince scc control_1 regs 0x1000 */
    prince_cntrl_1_regs_t cntrl_1_regs;     /* 0x1000-0x10FF */

    /* prince scc protocol regs 0x1100*/
    prince_proto_regs_t proto_regs[4];      /* 0x1100-0x113F */
    prince_proto_regs_lp_t proto_regs_lp[4];/* 0x1140-0x117F */

    ushort reserved2[0x40];                 /* 0x1180-0x11FF */

    ushort reserved3[0x700];                /* 0x1200-0x1FFF */

    /* prince scc dma regs  0x2000 */
    prince_dma_regs_t    dma_rx_regs[16];   /* 0x2000-0x21FF */
    prince_dma_regs_t    dma_tx_regs_hp[4]; /* 0x2200-0x227F */
    prince_dma_regs_t    dma_tx_regs_lp[4]; /* 0x2280-0x22FF */

    ushort reserved4[0x80];                 /* 0x2300-0x23FF */

    /* prince scc protocol extra  0x2400*/
    prince_bcc_regs_t   bcc_regs[16];       /* 0x2400-0x24FF */
    ushort reserved5[0x80];                 /* 0x2500-0x25FF */
    prince_ppp_regs_t   ppp_tx_regs[16];    /* 0x2600-0x26FF */
    ushort reserved6[0x80];                 /* 0x2700-0x27FF */
    prince_ppp_regs_t   ppp_rx_regs[16];    /* 0x2800-0x28FF */

    ushort reserved7[0x80];                 /* 0x2900-0x29FF */
    prince_bcc_regs_t   bcc_regs_rx[16];    /* 0x2a00-0x2aFF */
    ushort reserved8[0x280];                /* 0x2b00-0x2FFF */

    /* prince scc serial interface regs 0x3000 */
    prince_serial_itf_t serial_itf[16];     /* 0x3000-0x30FF */
    prince_cntrl_2_regs_t cntrl_2_regs;     /* 0x3100-0x31FF */
} prince_scc_regs_t;
#else
typedef struct prince_scc_regs_t_ {
    ushort reserved1[0x800];                /* 0x0000-0x0FFF */

    /* prince scc control_1 regs 0x1000 */
    prince_cntrl_1_regs_t cntrl_1_regs;     /* 0x1000-0x10FF */

    /* prince scc protocol regs 0x1100*/
    prince_proto_regs_t proto_regs_hp1[4];    /* 0x1100-0x113F */
    prince_proto_regs_hp2_t proto_regs_hp2[4];/* 0x1140-0x117F */
    prince_proto_regs_lp_t proto_regs_lp[4];  /* 0x1180-0x11BF */

    ushort reserved2[0x20];                 /* 0x11C0-0x11FF */

    ushort reserved3[0x700];                /* 0x1200-0x1FFF */

    /* prince scc dma regs  0x2000 */
    prince_dma_regs_t    dma_rx_regs[16];    /* 0x2000-0x21FF */
    prince_dma_regs_t    dma_tx_regs_hp1[4]; /* 0x2200-0x227F */
    prince_dma_regs_t    dma_tx_regs_hp2[4]; /* 0x2280-0x22FF */
    prince_dma_regs_t    dma_tx_regs_lp[4];  /* 0x2300-0x237F */

    ushort reserved4[0x40];                 /* 0x2380-0x23FF */

    /* prince scc protocol extra  0x2400*/
    prince_bcc_regs_t   bcc_regs[16];       /* 0x2400-0x24FF */
    ushort reserved5[0x80];                 /* 0x2500-0x25FF */
    prince_ppp_regs_t   ppp_tx_regs[16];    /* 0x2600-0x26FF */
    ushort reserved6[0x80];                 /* 0x2700-0x27FF */
    prince_ppp_regs_t   ppp_rx_regs[16];    /* 0x2800-0x28FF */

    ushort reserved7[0x80];                 /* 0x2900-0x29FF */
    prince_bcc_regs_t   bcc_regs_rx[16];    /* 0x2a00-0x2aFF */
    ushort reserved8[0x280];                /* 0x2b00-0x2FFF */

    /* prince scc serial interface regs 0x3000 */
    prince_serial_itf_t serial_itf[16];     /* 0x3000-0x30FF */
    prince_cntrl_2_regs_t cntrl_2_regs;     /* 0x3100-0x31FF */
} prince_scc_regs_t;
#endif
typedef struct uart0_ctrl_reg_t_ {
    ulong control;
    ulong mode;
    ulong intrpt_en;
    ulong intrpt_dis;
    ulong intrpt_mask;
    ulong chnl_int_sts;
    ulong baud_rate_gen;
    ulong rcvr_timeout;
    ulong rcvr_fifo_trigger_level;
    ulong modem_ctrl;
    ulong modem_sts;
    ulong channel_sts;
    ulong tx_rx_fifo;
    ulong baud_rate_divider;
    ulong flow_delay;
    ulong tx_fifo_trigger_level;
} uart_ctrl_reg_t;

typedef struct i2c0_ctrl_reg_t_ {
    ulong control;
    ulong status;
    ulong i2c_address;
    ulong i2c_data;
    ulong interrupt_status;
    ulong transfer_size;
    ulong slave_mon_pause;
    ulong time_out;
    ulong intrpt_mask;
    ulong intrpt_enable;
    ulong intrpt_disable;
} i2c0_ctrl_reg_t;

#endif

/******** History ********
$Log: prince_reg.h,v $
Revision 1.4  2018/01/17 11:48:04  iachang
CSCvh19145 : Fixed Reva firmware compiler issue. Reva didn't support new FPGA.

Revision 1.3  2017/07/18 08:48:41  iachang
Prince FPGA Enhanced Feature, Support HP1, HP2, and LP.

Revision 1.2  2013/11/14 03:01:23  xiaoyizh
Add definition for primary interface ready pin.

Revision 1.1  2013/04/19 07:17:52  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/
