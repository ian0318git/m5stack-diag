/* $Id: dev_pktpump.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/dev_pktpump.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: dev_pktpump.h
 *
 * Common Device Driver for packet pump functional block within Goofy ASIC.
 * Ported from Dobro and Jebbin code.
 *
 * April 2006 - Christine Wen
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_PKTPUMP_H__
#define __DEV_PKTPUMP_H__

#define PP_PAD uint32_t

typedef struct pktpump_regs {
    volatile uint32_t ppsr0;		       	        /* 0000 */
    volatile uint32_t ppsr1;		       	        /* 0004 */
    volatile uint32_t ppcr0; 			        /* 0008 */
    volatile uint32_t ppcr1; 			        /* 000c */
    volatile uint32_t ppcr2; 			        /* 0010 */
    volatile uint32_t ppcr3; 			        /* 0014 */
    volatile uint32_t ppcr4; 			        /* 0018 */
    volatile uint32_t dsp_movement_en;		        /* 001c */
    volatile uint32_t chpi_cr0;			        /* 0020 */
    volatile uint32_t chpi_cr1;			        /* 0024 */
    volatile uint32_t rrcr;				/* 0028 */
    volatile uint32_t rrar;	                	/* 002c */
    volatile uint32_t d_bus_timeout_val;		/* 0030 */
    volatile uint32_t d_mode_reg;     	  	        /* 0034 */
    volatile uint32_t d_wait_state;                	/* 0038 */
    volatile uint32_t d_ing_dq_ctrl;               	/* 003c */
    PP_PAD pad0;                                        /* 0040 */
    volatile uint32_t d_cdm_db_bar1;               	/* 0044 */
    volatile uint32_t d_cdm_db_bar2;               	/* 0048 */
    volatile uint32_t d_cdm_db_bar3;               	/* 004c */
    volatile uint32_t eg_wq_val0;               	/* 0050 */
    volatile uint32_t eg_wq_val1;               	/* 0054 */
    volatile uint32_t eg_wq_val2;               	/* 0058 */
    volatile uint32_t eg_wq_val3;               	/* 005c */
    volatile uint32_t ing_wq_val0;               	/* 0060 */
    volatile uint32_t ing_wq_val1;               	/* 0064 */
    volatile uint32_t ing_wq_val2;               	/* 0068 */
    volatile uint32_t ing_wq_val3;               	/* 006c */
    PP_PAD pad1[36];                                    /* 0x0070 - 0x00ff */
    volatile uint32_t d_irpr;			        /* 0100 */
    volatile uint32_t d_iipr;        	       	        /* 0104 */    
    volatile uint32_t d_c0_q0_erpr[4];        	        /* 0108 - 0114 */
    volatile uint32_t d_c0_q0_eipr[16];		        /* 0118 - 0154 */
    volatile uint32_t d_c0_q1_erpr[4];        	        /* 0158 - 0164 */
    volatile uint32_t d_c0_q1_eipr[16];		        /* 0168 - 01a4 */
    volatile uint32_t d_c0_q2_erpr[4];        	        /* 01a8 - 01b4 */
    volatile uint32_t d_c0_q2_eipr[16];		        /* 01b8 - 01f4 */
    volatile uint32_t d_c1_q0_erpr[4];        	        /* 01f8 - 0204 */
    volatile uint32_t d_c1_q0_eipr[16];		        /* 0208 - 0244 */
    volatile uint32_t d_c1_q1_erpr[4];        	        /* 0248 - 0254*/
    volatile uint32_t d_c1_q1_eipr[16];		        /* 0258 - 0294 */
    volatile uint32_t d_c1_q2_erpr[4];        	        /* 0298 - 02a4 */
    volatile uint32_t d_c1_q2_eipr[16];		        /* 02a8 - 02e4 */
    PP_PAD pad2[6];                                     /* 0x02e8 - 0x02ff */
    volatile uint32_t parity_err_isr;	       	        /* 0300 */
    volatile uint32_t parity_err_ier;	       	        /* 0304 */
    volatile uint32_t parity_err_inj;	       	        /* 0308 */
    PP_PAD pad3[317];                                   /* 0x030c - 0x07ff */
    volatile uint32_t pp_sti_err_stat;	       	        /* 0800 */
    volatile uint32_t pp_sti_err_ctrl;	       	        /* 0804 */
    PP_PAD pad4[510];                                   /* 0x0808 - 0x0fff */
    volatile uint32_t dsp_da[16][16];                   /* 0x1000 - 0x13ff */
    volatile uint32_t pld_cs_access[4][16];             /* 0x1400 - 0x14ff */
} pktpump_regs_t;

/*
 * Bit definitions for packet pump status register (PPSR0)
 * DSP errors are due to cHPI or time-out error and can
 * be reset by removing payload movement enable (ppcr)
 */
#define PPSR0_DSP15_ERROR                0x40000000
#define PPSR0_DSP14_ERROR                0x20000000
#define PPSR0_DSP13_ERROR                0x10000000
#define PPSR0_DSP12_ERROR                0x08000000
#define PPSR0_DSP11_ERROR                0x04000000
#define PPSR0_DSP10_ERROR                0x02000000
#define PPSR0_DSP9_ERROR                 0x01000000
#define PPSR0_DSP8_ERROR                 0x00800000
#define PPSR0_DSP7_ERROR                 0x00400000
#define PPSR0_DSP6_ERROR                 0x00200000
#define PPSR0_DSP5_ERROR                 0x00100000
#define PPSR0_DSP4_ERROR                 0x00080000
#define PPSR0_DSP3_ERROR                 0x00040000
#define PPSR0_DSP2_ERROR                 0x00020000
#define PPSR0_DSP1_ERROR                 0x00010000
#define PPSR0_DSP0_ERROR                 0x00008000
#define PPSR0_DSP_ERROR_MASK             0x7fff8000
#define PPSR0_BUS_TIMEOUT_ERROR_INT      0x00004000
#define PPSR0_CHPI_ERROR_INT             0x00002000
#define PPSR0_ERROR_MASK                 0x7fffe000
#define PPSR0_DSP_EG_BUF_REL_INT         0x00001000
#define PPSR0_DSP_ING_QUE_FULL_INT       0x00000800
#define PPSR0_DSP_ING_PAYLOAD_INT        0x00000400
#define PPSR0_CHPI_ERR_FRM_DSP           0x00000004
#define PPSR0_CHPI_ERR_ING_MESS_LEN      0x00000002
#define PPSR0_CHPI_ERR_WINDOW_STUCK      0x00000001

/*
 * Bit definitions for packet pump status register (PPSR1)
 * Any bit being set here will set PPSR0_DSP_EG_BUF_REL_INT.
 */
#define PPSR1_DSP_EG_C1Q2_BUF_REL_INT0   0x00100000
#define PPSR1_DSP_EG_C1Q1_BUF_REL_INT0   0x00010000
#define PPSR1_DSP_EG_C1Q0_BUF_REL_INT0   0x00001000
#define PPSR1_DSP_EG_C0Q2_BUF_REL_INT0   0x00000100
#define PPSR1_DSP_EG_C0Q1_BUF_REL_INT0   0x00000010
#define PPSR1_DSP_EG_C0Q0_BUF_REL_INT0   0x00000001

/*
 * Bit definitions for packet pump control register0 (PPCR0)
 * supported ingress BD ring size: 512, 1024, 2048, 4096
 * supported egress BD ring size: 128, 256
 * supported ingress particle size: 256B, 512B, 1024B, 2048B
 * Max CHPI ingress message size: 2048B, 4096B, 8192B, 16384B
 */
#define PPCR0_ING_RING_SIZE_512         0x00000000
#define PPCR0_ING_RING_SIZE_1024        0x40000000
#define PPCR0_ING_RING_SIZE_2048        0x80000000
#define PPCR0_ING_RING_SIZE_4096        0xc0000000
#define PPCR0_ING_RING_SIZE_MASK        0xc0000000
#define PPCR0_ING_PART_SIZE_256B        0x00000000
#define PPCR0_ING_PART_SIZE_512B        0x10000000
#define PPCR0_ING_PART_SIZE_1024B	0x20000000
#define PPCR0_ING_PART_SIZE_2048B	0x30000000
#define PPCR0_ING_PART_SIZE_MASK        0x30000000
#define PPCR0_BUS_TIMEOUT_CNTR_EN	0x08000000
#define PPCR0_EG_C0Q0_SIZE_128          0x00000000
#define PPCR0_EG_C0Q0_SIZE_256  	0x04000000
#define PPCR0_EG_C0Q0_SIZE_MASK         0x04000000
#define PPCR0_HPIC_DSP_INT_EN		0x02000000
#define PPCR0_BUS_TIMEOUT_INT_EN	0x01000000
#define PPCR0_CHPI_ING_MSG_SIZE_16384B	0x00C00000
#define PPCR0_CHPI_ING_MSG_SIZE_8192B	0x00800000
#define PPCR0_CHPI_ING_MSG_SIZE_4096B	0x00400000
#define PPCR0_CHPI_ING_MSG_SIZE_2048B   0x00000000
#define PPCR0_CHPI_ING_MSG_SIZE_MASK	0x00C00000
#define PPCR0_CHPI_ERROR_INT_EN		0x00200000
#define PPCR0_DSP_EG_SINGLE		0x00100000
#define PPCR0_DSP_ING_FULL_INT_EN	0x00040000
#define PPCR0_DSP_ING_PYLD_INT_EN	0x00020000
#define PPCR0_DSP_ING_PYLD_MVMT_EN 	0x00010000

/* this definition works for all the cores and queues */
#define PPCR_DSP0_EGR_PYLD_MVMT_EN	0x00000001

/*
 * Bit definitions for packet pump control register1 (PPCR1)
 */
#define PPCR1_DSP_ING_PYLD_BD_INT_EN	   0x20000000
#define PPCR1_EG_C1Q2_SIZE_128             0x00000000
#define PPCR1_EG_C1Q2_SIZE_256             0x10000000
#define PPCR1_EG_C1Q2_SIZE_MASK            0x10000000
#define PPCR1_EG_C1Q1_SIZE_128             0x00000000
#define PPCR1_EG_C1Q1_SIZE_256             0x08000000
#define PPCR1_EG_C1Q1_SIZE_MASK            0x08000000
#define PPCR1_EG_C1Q0_SIZE_128             0x00000000
#define PPCR1_EG_C1Q0_SIZE_256             0x04000000
#define PPCR1_EG_C1Q0_SIZE_MASK            0x04000000
#define PPCR1_EG_C0Q2_SIZE_128             0x00000000
#define PPCR1_EG_C0Q2_SIZE_256             0x02000000
#define PPCR1_EG_C0Q2_SIZE_MASK            0x02000000
#define PPCR1_EG_C0Q1_SIZE_128             0x00000000
#define PPCR1_EG_C0Q1_SIZE_256             0x01000000
#define PPCR1_EG_C0Q1_SIZE_MASK            0x01000000
#define PPCR1_DSP_EG_C1Q2_BUF_REL_INT0_EN  0x00100000
#define PPCR1_DSP_EG_C1Q1_BUF_REL_INT0_EN  0x00010000
#define PPCR1_DSP_EG_C1Q0_BUF_REL_INT0_EN  0x00001000
#define PPCR1_DSP_EG_C0Q2_BUF_REL_INT0_EN  0x00000100
#define PPCR1_DSP_EG_C0Q1_BUF_REL_INT0_EN  0x00000010
#define PPCR1_DSP_EG_C0Q0_BUF_REL_INT0_EN  0x00000001

/* bit dinifition for DSP ingress insert pointer register */
#define DIIPR_MASK                         0x00000fff
/* Bit dinifitions for DSP ingress remove pointer register */
#define DSP_INGRESS_REMOVE_PTR             0xfffffff0

/*
 * Bit definitions for egress remove pointers 0-3
 * Queue 0, Queue1 and Queue2 of core 0 and core 1
 * will share the same bit definitions.
 */
#define DSP_3_EGRESS_REMOVE_PTR		0x7f000000
#define DSP_2_EGRESS_REMOVE_PTR		0x007f0000
#define DSP_1_EGRESS_REMOVE_PTR		0x00007f00
#define DSP_0_EGRESS_REMOVE_PTR		0x0000007f
#define DSP_EGRESS_REMOVE_PTR_SHIFT	8
#define DSP_EG_REM_PTR_PER_REG          4

/* 
 * bit difinitions for DSP egress Insert Pointer register 
 * Queue 0, Queue1 and Queue2 of core 0 and core 1 for all 
 * the DSPs (0-15) will share the same bit definitions.
 */
#define DSP_EGRESS_BD_TABLE_BASE_ADDR   0xfffff800
#define DSP_EGRESS_INSERT_PTR           0x000007f0

/*
 * bit definition for relayed read command register
 */
#define RRCR_LEN_1024B                  0x00008000
#define RRCR_LEN_SHIFT                  5
#define RRCR_DSP_SHIFT                  1
#define RRCR_RELAYED_RD_EN              1

/*
 * bit definition for relayed read address register
 */
#define RRAR_UP_ADDR_SHIFT              20
#define RRAR_UP_ADDR_MASK               0xfff00000
#define RRAR_LO_ADDR_MASK               0x000fffff

/*
 * Bit definitions for CHPI control register (CHPI_CONTROL_REG)
 * both the core 0 and core 1 will share the same definition. 
 */
#define DSP15_CHPI_UP			0x80000000
#define DSP14_CHPI_UP			0x40000000
#define DSP13_CHPI_UP			0x20000000
#define DSP12_CHPI_UP			0x10000000
#define DSP11_CHPI_UP			0x08000000
#define DSP10_CHPI_UP			0x04000000
#define DSP9_CHPI_UP			0x02000000
#define DSP8_CHPI_UP			0x01000000
#define DSP7_CHPI_UP			0x00800000
#define DSP6_CHPI_UP			0x00400000
#define DSP5_CHPI_UP			0x00200000
#define DSP4_CHPI_UP			0x00100000
#define DSP3_CHPI_UP			0x00080000
#define DSP2_CHPI_UP			0x00040000
#define DSP1_CHPI_UP			0x00020000
#define DSP0_CHPI_UP			0x00010000
#define DSP15_CHPI_EN			0x00008000
#define DSP14_CHPI_EN			0x00004000
#define DSP13_CHPI_EN			0x00002000
#define DSP12_CHPI_EN			0x00001000
#define DSP11_CHPI_EN			0x00000800
#define DSP10_CHPI_EN			0x00000400
#define DSP9_CHPI_EN			0x00000200
#define DSP8_CHPI_EN			0x00000100
#define DSP7_CHPI_EN			0x00000080
#define DSP6_CHPI_EN			0x00000040
#define DSP5_CHPI_EN			0x00000020
#define DSP4_CHPI_EN			0x00000010
#define DSP3_CHPI_EN			0x00000008
#define DSP2_CHPI_EN			0x00000004
#define DSP1_CHPI_EN			0x00000002
#define DSP0_CHPI_EN			0x00000001

/*
 * Bit definitions for DSP movement enable register (DSP_MOVEMENT_EN)
 */
#define DSP15_MOVEMENT_EN		0x00008000
#define DSP14_MOVEMENT_EN		0x00004000
#define DSP13_MOVEMENT_EN		0x00002000
#define DSP12_MOVEMENT_EN		0x00001000
#define DSP11_MOVEMENT_EN		0x00000800
#define DSP10_MOVEMENT_EN		0x00000400
#define DSP9_MOVEMENT_EN		0x00000200
#define DSP8_MOVEMENT_EN		0x00000100
#define DSP7_MOVEMENT_EN		0x00000080
#define DSP6_MOVEMENT_EN		0x00000040
#define DSP5_MOVEMENT_EN		0x00000020
#define DSP4_MOVEMENT_EN		0x00000010
#define DSP3_MOVEMENT_EN		0x00000008
#define DSP2_MOVEMENT_EN		0x00000004
#define DSP1_MOVEMENT_EN		0x00000002
#define DSP0_MOVEMENT_EN		0x00000001

/* 
 * Bit definitions for DSP Bus Timeout Value register (0x30)
 */
#define DHTVR_MASK                      0x0000ffff

/*
  must be 0 to cause DSP bus timeout interrupt to happen. With any 
  non-zero value in counter register, the timeout interrupt won't happen. 
*/
#define DHTVR_TEST_VAL                  0   

/*
 * Bit definitions for DSP TI/Freescale mode register
 */
#define DSP_EHPI_MODE                   0x00000000
#define DSP_HDI16_MODE                  0x00000001
#define DSP_C6455_MODE                  0x00000002
#define DSP_V2520_MODE                  0x00000003
#define DSP_MODE_SHIFT                  2

/*
 * Bit definitions for DSP wait state register
 */
#define WAIT_STATE_PVDM_SHIFT          8
#define WAIT_STATE_SIGNAL_SHIFT        2
#define WAIT_STATE_MAX_SIGNALS         4

/*
 * Bit definitions for DSP ingress dual queue support register
 */
#define DSP_1_QUEUE_SUPPORT            0x00000000
#define DSP_2_QUEUE_SUPPORT            0x00000001
#define DSP_DUAL_QUEUE_SUPPORT_SHIFT   1    

/*
 * Bit definitions for parity error interrupt status register
 */
#define ING_MEM_PAR_ERR_INT            0x00000002
#define EG_MEM_PAR_ERR_INT             0x00000001

/*
 * Bit definitions for parity error interrupt enable register
 */
#define ING_MEM_PAR_ERR_INT_EN         0x00000002
#define EG_MEM_PAR_ERR_INT_EN          0x00000001

/*
 * Bit definitions for STI bus error status register
 */
#define ING_RESP_NXA                   0x00000400
#define ING_RESP_ERR                   0x00000200
#define ING_RESP_SIZE_ERR              0x00000100
#define ING_RESP_CMD_ERR               0x00000080
#define EG_RESP_NXA                    0x00000040
#define EG_RESP_ERR                    0x00000020
#define EG_RESP_SIZE_ERR               0x00000010
#define EG_RESP_CMD_ERR                0x00000008
#define TGT_BYTE_EN_ERR                0x00000004
#define TGT_SIZE_ERR                   0x00000002
#define TGT_WR_ERR                     0x00000001

/*
 * Bit definitions for STI bus error control register
 */
#define ING_RESP_NXA_EN                 0x00000400
#define ING_RESP_ERR_EN                 0x00000200
#define ING_RESP_SIZE_ERR_EN            0x00000100
#define ING_RESP_CMD_ERR_EN             0x00000080
#define EG_RESP_NXA_EN                  0x00000040
#define EG_RESP_ERR_EN                  0x00000020
#define EG_RESP_SIZE_ERR_EN             0x00000010
#define EG_RESP_CMD_ERR_EN              0x00000008
#define TGT_BYTE_EN_ERR_EN             0x00000004
#define TGT_SIZE_ERR_EN                0x00000002
#define TGT_WR_ERR_EN                  0x00000001
#define STI_ERR_EN_ALL                 0x000007ff

/*
 * Bit definitions for pld_dsp_reset_reg
 */
#define DSP3_RESET                      0x00000008
#define DSP2_RESET                      0x00000004
#define DSP1_RESET                      0x00000002
#define DSP0_RESET                      0x00000001

/*
 * Bit definitions for pld_dsp_rev_reg
 */
#define REVISION_MASK                  0x0000000f

/*
 * Bit definitions for pld_dsp_int_reg
 */
#define DSP3_INT                       0x00000008
#define DSP2_INT                       0x00000004
#define DSP1_INT                       0x00000002
#define DSP0_INT                       0x00000001

/*
 * Bit definitions for pld_dsp_int_mask_reg
 */
#define DSP3_INT_MASK                  0x00000008
#define DSP2_INT_MASK                  0x00000004
#define DSP1_INT_MASK                  0x00000002
#define DSP0_INT_MASK                  0x00000001

/* Bit definitions for EGRESS buffer descriptor */
#define DSP_EG_RELAYED_WRITE	       	0x0001
#define DSP_EG_SOM			0x0002
#define DSP_EG_EOM			0x0004
#define DSP_EG_HPIC_RESET		0x0020
#define DSP_EG_INT_REQ	        	0x0040
#define DSP_EG_RELAYED_ADDR_SHIFT       16
#define DSP_EG_RELAYED_ADDR_MASK        0x0000ffff    

/* Bit definitions for INGRESS buffer descriptor */
#define DSP_ING_RELAYED_READ		0x8000
#define DSP_ING_EOM      		0x2000
#define DSP_ING_SOM      		0x1000
#define DSP_ING_LEN_MASK		0x0fff
#define DSP_ING_OWNERSHIP		0x8000
#define DSP_ING_DSP_OFFSET 		10
#define DSP_ING_CHANNEL_MASK		0x03ff
#define DSP_ING_CORE_1                  0x0400
#define DSP_ING_MSG_ID_MASK             0x03ff

/* Packet Pump Egress Descriptors */ 
/* egress_buffer_ descriptor contains the pointer to the data buffer in host
  * memory and information about the type and size of the egress message. It
  * can be of normal CHPI/DMA-HDI type message or relay type where an address
  * will be appended to the front of the data payload to point to destination 
  * location inside the device */
typedef struct pp_egress_descriptor_ {
    uint32_t  data_ptr;  /* Pointer to the start of data in the buffer */
    uint16_t  length;    /* Length in bytes of data in the buffer */
    uint16_t  status;    /* intr_req, hpic_reset, pre_len, EOM, SOM, xfer_mode */
    uint32_t  preamble0; /* preamble words 0 */
    uint32_t  preamble1; /* preamble words 1 */
} pp_egress_bd_t;

/* Packet Pump Ingress Descriptors */
typedef struct pp_ingress_descriptor_ {
    uint32_t data_ptr;            /* Pointer to the start of data buffer */
    volatile uint16_t length;     /* xfer_mode, EOM, SOM, length */
    volatile uint16_t dsp;        /* ownership bit, DSP0-3, channel # */
    volatile uint16_t message_id; /* core, Msg ID */
    volatile uint16_t process_id; /* Process ID */
    uint32_t reserved;
} pp_ingress_bd_t;

#define EGRESS_DESC_SIZE           sizeof(struct pp_egress_descriptor_)
#define INGRESS_DESC_SIZE          sizeof(struct pp_ingress_descriptor_)

#define PKTPUMP_LOG_BUF_SIZE       380
#define ILLEGAL_INDEX              0xFFFF
#define RETRY_CNT                  0x20
/* With Shinkansen Goofy WAN FPGA version 0x16, modem sometimes takes longer time
   to send connection response back to host in modem loopback test. 
   This delay is for receiving ingress intr.  */ 
#define PVDM_MAX_WAIT_TIME         120000 /* 120 sec. Jebbin modem needs this long delay. */
#define PKTPUMP_DELAY              1000
/* totally support 16 dsp devices, each dsp can have at max 2 cores. */
#define MAX_PVDM_PER_PKTPUMP   	   4
#define MAX_DSPS_PER_PVDM          4
#define MAX_DEV_PER_PKTPUMP	   MAX_PVDM_PER_PKTPUMP *  MAX_DSPS_PER_PVDM   /* core0: 0 - 15; core1: 16 - 31 */
#define PP_EGRESS_MAX_ENTRY_PER_Q  128
#define PP_INGRESS_MAX_ENTRY_PER_Q 1024
#define DSP_DATA_BUF_LENGTH        2048
#define PP_EGRESS_MAX_QUEUE        3
#define MAX_CORE_PER_DSP           2
#define PKTPUMP_BOOTLOAD_ADDR_SIZE 4

typedef struct pp_egress_bd_q_t_ {
    pp_egress_bd_t bd_array[MAX_CORE_PER_DSP][PP_EGRESS_MAX_QUEUE] 
    [MAX_DEV_PER_PKTPUMP][PP_EGRESS_MAX_ENTRY_PER_Q];
} pp_egress_bd_q_t;

/* all the target devices share the ingress queues */
typedef struct pp_ingress_bd_q_t_ {
    pp_ingress_bd_t bd_array[PP_INGRESS_MAX_ENTRY_PER_Q];
} pp_ingress_bd_q_t;

typedef struct pp_egress_buf_q_t_ {
    char buf_array[MAX_CORE_PER_DSP][PP_EGRESS_MAX_QUEUE] 
    [MAX_DEV_PER_PKTPUMP][PP_EGRESS_MAX_ENTRY_PER_Q][DSP_DATA_BUF_LENGTH];
} pp_egress_buf_q_t;

typedef struct pp_ingress_buf_q_t_ {
    char buf_array[PP_INGRESS_MAX_ENTRY_PER_Q][DSP_DATA_BUF_LENGTH];
} pp_ingress_buf_q_t;

/* define for DSP 0 - 15 direct access register map */
/* define for TI mode */
typedef struct ti_hpi_reg_t {
    volatile uint32_t hpic;
    volatile uint32_t hpida;
    volatile uint32_t hpia;
    volatile uint32_t hpid;
    PP_PAD pad[12];
} ti_hpi_reg;

/* define for Freescale mode */
typedef struct mot_hpi_reg_t {
    volatile uint32_t icr;
    volatile uint32_t cvr;
    volatile uint32_t isr; 
    PP_PAD pad0;
    volatile uint32_t tr_reg3; 
    volatile uint32_t tr_reg2; 
    volatile uint32_t tr_reg1; 
    volatile uint32_t tr_reg0; 
    volatile uint32_t rc_reg0; 
    volatile uint32_t rc_reg1; 
    volatile uint32_t rc_reg2; 
    volatile uint32_t rc_reg3; 
    PP_PAD pad1[4];
} mot_hpi_reg;

/* define for PLD CS 0-3 access register map */
/* define for TI mode */
typedef struct ti_pld_reg_t {
    volatile uint32_t dsp_reset;
    volatile uint32_t dsp_rev;
    volatile uint32_t dsp_intr;
    volatile uint32_t dsp_intr_mask;
    PP_PAD pad[12];
} ti_pld_reg;

/* define for Freescale mode */
typedef struct mot_pld_reg_t {
    volatile uint32_t dsp_reset;
    volatile uint32_t dsp_rev;
    volatile uint32_t dsp_intr;
    volatile uint32_t dsp_intr_mask;
    volatile uint32_t hrrq_mask;
    volatile uint32_t htrq_mask;
    volatile uint32_t hrrq_reg;
    volatile uint32_t htrq_reg;
    volatile uint32_t preset_reg;
    PP_PAD pad[7];
} mot_pld_reg;

typedef enum dsp_core_t_ {
    CORE0,
    CORE1,
} dsp_core_t;

typedef enum pktpump_trans_dir_ {
    PKTPUMP_EGRESS,
    PKTPUMP_INGRESS,
} pktpump_trans_dir;

typedef enum pktpump_queue_t_ {
    QUEUE0,
    QUEUE1,
    QUEUE2,
}pktpump_queue_t;

typedef enum pktpump_mode_t_ {
    DIRECT_MODE,
    RELAYED_WRITE,
    RELAYED_READ,
    DMA_MODE_CHPI,
} pktpump_mode_t;

typedef enum target_dev_type_t_ {
    TARGET_DEV_EHPI,			
    TARGET_DEV_HDI16,			
    TARGET_DEV_C64X_UHPI,
    TARGET_DEV_V2520_UHPI,
} target_dev_type_t;

typedef enum pktpump_wait_state_t_ {
    NO_WAIT_STATE,
    WAIT_STATE_1,
    WAIT_STATE_2,
    WAIT_STATE_3,
} pktpump_wait_state_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct dev_pktpump_callin_fvt_t_ {
    /* ywen: make sure these three addresses are all 64 bits in diagmon64. */
    ulong (*get_pktpump_base_addr)(dev_object_t *);
    ulong (*get_dsp_hpi_addr)(dev_object_t *, int, int);
    ulong (*get_pvdm_pld_addr)(dev_object_t *, int);
    void (*pktpump_set_dev)(dev_object_t *, int, int, target_dev_type_t);
    int (*pktpump_set_mode)(dev_object_t *, int, int, dsp_core_t, 
				pktpump_mode_t, uint32_t);
    int (*pktpump_init_dma)(dev_object_t *, int, int, dsp_core_t);
    void (*pktpump_cleanup)(dev_object_t *, int, int);
    int (*pktpump_dma_rcv)(dev_object_t *, uchar *, uint32_t *);
    int (*pktpump_dma_xmit)(dev_object_t *, int, int, dsp_core_t, 
				pktpump_queue_t, uchar *, uint32_t);
    int (*get_egress_index)(dev_object_t *, int, int, dsp_core_t, 
				pktpump_queue_t);
    int (*get_egress_buf_size)(dev_object_t *, int, int, dsp_core_t, 
				   pktpump_queue_t);
    int (*get_egress_max_msg_size)(dev_object_t *, int, int, dsp_core_t, 
				       pktpump_queue_t);
    void (*set_wait_state)(dev_object_t *, int, int, pktpump_wait_state_t);
    void (*set_wait_state_all)(dev_object_t *, int, pktpump_wait_state_t);
    void (*set_dsp_cdm_bar)(dev_object_t *, int, int, dsp_core_t, uint32_t);
    void (*pktpump_write_ppcr1)(dev_object_t *, int, int);
    /* testing functions */
    int (*register_test)(dev_object_t *);
    int (*timeout_counter_test)(dev_object_t *);
} dev_pktpump_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct dev_pktpump_callout_fvt_t_ {
    int (*platform_get_relative_dsp_num)(int, int);
    int (*set_pktpump_intr)(dev_object_t *, goofy_intr_class_t,
			    uint, uint, uint, boolean);
    PFI (*install_isr_vect)(dev_object_t *, goofy_intr_class_t,
			    uint, uint, PFI);
    void * (*get_plat_goofy_inst)(uint32_t mod_type, uint32_t mod_num);
    ulong (*make_phy_addr)(ulong address);
    ulong (*make_vir_addr)(ulong address);
} dev_pktpump_callout_fvt_t;

/* define the structure for pktpump device specific information */
typedef struct dev_pktpump_dep_t_ {
    /* pointers for egress/ingress buffer descriptor queues and buffer queues */
    void                      *pp_egress_bdq_ptr;
    void                      *pp_ingress_bdq_ptr;
    pp_egress_bd_q_t          *pp_egress_bd_q;
    pp_ingress_bd_q_t         *pp_ingress_bd_q;
    pp_egress_buf_q_t         *pp_egress_buf_q;
    pp_ingress_buf_q_t        *pp_ingress_buf_q;
    /* Buffer pointer for error or information log */
    uchar                     *log_buffer;
    /* pointers for pktpump device function vector table */
    dev_object_fvt_t          *pktpump_fvt;
    dev_pktpump_callin_fvt_t  *pktpump_callin;
    dev_pktpump_callout_fvt_t *pktpump_callout;
    target_dev_type_t         dsp_type[MAX_DEV_PER_PKTPUMP];
    pktpump_mode_t            dsp_mode[MAX_DEV_PER_PKTPUMP][MAX_CORE_PER_DSP];
    uint32_t                  relay_addr[MAX_DEV_PER_PKTPUMP][MAX_CORE_PER_DSP];
    uint32_t                  pktpump_id;
} dev_pktpump_dep_t;

/* define the structure for pktpump device interrupt specific information */
typedef struct dev_pktpump_intr_dep_t_ {
    uint32_t ingress_int;
    uint32_t egress_int[MAX_CORE_PER_DSP][PP_EGRESS_MAX_QUEUE][MAX_PVDM_PER_PKTPUMP];
    uint32_t ppsr0;
    uint32_t ppsr1;
    uint32_t parity_err_isr;
    uint32_t pp_sti_err_stat;
}dev_pktpump_intr_dep_t;

/*
 * Define the packet pump device object structure.
 */
typedef struct dev_pktpump_object_t_ {
    dev_object_t                base;          
    dev_pktpump_callout_fvt_t 	*callout_fvt;
    dev_pktpump_callin_fvt_t  	*callin_fvt;
    dev_pktpump_dep_t           pktpump_dep; 
    dev_pktpump_intr_dep_t      pktpump_intr_dep; 
} dev_pktpump_object_t;

#undef PP_PAD

extern void dev_pktpump_create(dev_object_t *dev, 
			       dev_error_report_t error_report_fn);

#endif /* __DEV_PKTPUMP_H__ */


/******** History ******** 
$Log: dev_pktpump.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
