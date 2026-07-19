/* $Id: dev_tdm.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/dev_tdm.h,v $
 *------------------------------------------------------------------
 *
 * dev_tdm.h : Common Device Driver header of TDM Switch 128 chip.
 *
 * June 2006 - Alan Hsu
 *
 * Copyright (c) 2010-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DEV_TDM_H__
#define __DEV_TDM_H__

/*
 * TDM Registers
 */
typedef struct sw128_regs {
    volatile unsigned int tdm_conn_mem[0x4000];	/* 00000 */
    volatile unsigned int tdm_enbl_7f_60;	/* 10000 */
    volatile unsigned int tdm_enbl_5f_40;	/* 10004 */
    volatile unsigned int tdm_enbl_3f_20;	/* 10008 */
    volatile unsigned int tdm_enbl_1f_00;	/* 1000c */
    volatile unsigned int tdm_rate_7f_70;       /* 10010 */
    volatile unsigned int tdm_rate_6f_60;       /* 10014 */
    volatile unsigned int tdm_rate_5f_50;       /* 10018 */
    volatile unsigned int tdm_rate_4f_40;       /* 1001C */
    volatile unsigned int tdm_rate_3f_30;       /* 10020 */
    volatile unsigned int tdm_rate_2f_20;       /* 10024 */
    volatile unsigned int tdm_rate_1f_10;       /* 10028 */
    volatile unsigned int tdm_rate_0f_00;       /* 1002C */
    volatile unsigned int tdm_lpbk_7f_60;	/* 10030 */
    volatile unsigned int tdm_lpbk_5f_40;	/* 10034 */
    volatile unsigned int tdm_lpbk_3f_20;	/* 10038 */
    volatile unsigned int tdm_lpbk_1f_00;	/* 1003C */
    volatile unsigned int tdm_ctl;		/* 10040 */
    volatile unsigned int conn_mem_err_adr;	/* 10044 */
    volatile unsigned int stretched_32m;	/* 10048 */
    unsigned int pad0[1];
    volatile unsigned int tdm_mgmt_event;	/* 10050 */
    volatile unsigned int tdm_mgmt_enbl;	/* 10054 */
    volatile unsigned int tdm_err_event;	/* 10058 */
    volatile unsigned int tdm_err_enbl;		/* 1005C */
    volatile unsigned int vmcr0;		/* 10060 */
    volatile unsigned int vmcr1;		/* 10064 */
    volatile unsigned int vmcr2;		/* 10068 */
    volatile unsigned int vmcr3;		/* 1006C */
    volatile unsigned int tdm_cr;		/* 10070 */
    volatile unsigned int pvdm_tdm_cr;		/* 10074 */
    volatile unsigned int ntr_cr0;		/* 10078 */
    volatile unsigned int ntr_cr0_aux;		/* 1007C */
    volatile unsigned int ntr_cr1;		/* 10080 */
    volatile unsigned int ntr_cr1_aux;		/* 10084 */
    volatile unsigned int tpllr_cr;		/* 10088 */
    unsigned int pad1[1];
    volatile unsigned int ds0_dump_beg_adrs;	/* 10090 */
    volatile unsigned int ds0_dump_end_adrs;	/* 10094 */
    volatile unsigned int ds0_dump_ctl;		/* 10098 */
    unsigned int pad2[1];
    volatile unsigned int tdm_sync_ctrl_intf;	/* 100a0 */
    volatile unsigned int tdm_sync_stat_intf;	/* 100a4 */
} sw128_regs_t;


/*
 * register test table
 * rd_mask masks off all but valid register read bits.
 * wr_mask holds write data used for register uniqueness test.
 * rw_mask holds r/w mask for r/w registers but for ro registers
 * rw_mask holds mask used when checking reset_val (allows us to
 * mask off changable bits, .e.g., CP_L bit in status register
 * is dependent on whether a hwic is inserted or not).
 * NOTE:
 * register values used for the interrupt enable/interrupt event
 * registers during the register uniqueness test (in wr_mask)
 * are chosen so that no interrupts are generated.
 */
typedef struct sw128_test_regs_t_ {
    char          *pname;             /* register name */
    unsigned int  offset;             /* offset from base addr */
    unsigned int  rd_mask;            /* used when checking valid bits */
    unsigned int  wr_mask;            /* used for addr test */
    boolean       rdwrtest;           /* 0: don't perform R/W test */
    unsigned int  rw_mask;            /* used for R/W test, reset_val check */
    unsigned int  reset_val;          /* value at reset (default value) */
} sw128_test_regs_t;


/*
 * Defines for high speed card/stream access
 */
#define HIGH_SPEED_ENABLE         0x08000000  /* Bit 27    */
#define HS_CARD_ID_MASK           0x07000000  /* Bit 24-26 */
#define HS_TDM_LINE_3_MASK        0x00ff0000
#define HS_TDM_LINE_2_MASK        0x0000ff00
#define HS_TDM_LINE_1_MASK        0x000000ff
#define HS_TDM_LINE_2_SHIFT       8
#define HS_TDM_LINE_3_SHIFT       16





#define NM0_HS_ID                 0
#define NM1_HS_ID                 1
#define NM2_HS_ID                 2
#define NM3_HS_ID                 3
#define HWIC0_HS_ID               4
#define HWIC2_HS_ID               5
#define HS_TDM_LINE_1             1       
#define HS_TDM_LINE_2             2       
#define HS_TDM_LINE_3             3       
#define HS_TDM_LINE_ALL           4
#define UNDEFINED_STREAM          0xff
       
/*
 * Bit definitions for connection memory
 */
#define CONN_MEM_MASK			0xfff1ffff
#define TS_PASSWORD			0xCAC00000
#define TS_FORCE_BYTE			0x00010000
#define TS_DRIVE                        0x00008000
#define TS_FORCELSB			0x00004000
#define TS_FORCE_BYTE_MASK		0x000000ff
#define INPUT_CID_MASK			0x00003fff
#define NUM_2M_TIMESLOTS_MASK		0x0000001f
#define NUM_8M_TIMESLOTS_MASK		0x0000007f
#define NUM_16M_TIMESLOTS_MASK		0x000000ff
#define NUM_32M_TIMESLOTS_MASK		0x000001ff
#define NUM_HDLC_STREAMS		2
#define INPUT_STREAM_SHIFT              8
#define OUTPUT_STREAM_SHIFT             7
#define NUM_2M_TIMESLOTS		32
#define NUM_8M_TIMESLOTS		128
#define NUM_16M_TIMESLOTS		256
#define NUM_32M_TIMESLOTS		512
#define CID_TO_CM_ADDR_MULTIPLIER	4
#define NUM_TDM_STREAMS                 124  /* (128 - 4) */

/*
 * Bit definitions for TDM Stream enable register
 * bits 31 - 0 apply to streams 31 - 00 respectively for ENBL_31_00,
 * bits 31 - 0 apply to streams 63 - 32 respectively for ENBL_63_32,
 */
#define TDMSW_STREAM_DISABLE_ALL	0x00000000
#define TDMSW_STREAM_ENABLE_ALL		0xFFFFFFFF
#define TDMSW_STREAM_ENABLE		1
#define TDMSW_STREAM_DISABLE		0
#define NUM_STREAMS_PER_ENBL_REG	32	/* 32 streams/enbl register */
#define NUM_ENBL_REGISTERS		4	/* 4 enbl registers */

/*
 * Bit definitions for TDMSW128 RATE registers
 * There are 4 rate registers, each able to set the rate for 16 streams
 * the number of streams required to support the specified rate is:
 *  2 Mbps = 1 stream
 *  8 Mbps = 1 stream
 *  16 Mbps = 2 streams, start stream and companion stream
 *  32 Mbps = 4 streams, start stream and 3 companion streams
 */
#define TDMSW_RATE_8MBPS_ALL		0x55555555
#define TDMSW_RATE_2MBPS_ALL		0x00000000
#define TDMSW_STREAM_2MB		0
#define TDMSW_STREAM_8MB		1
#define TDMSW_STREAM_16MB		2
#define TDMSW_STREAM_32MB		3
#define TDMSW_RATE_MASK			3
#define TDMSW_RATE_SHIFT		2
#define NUM_STREAMS_PER_RATE_REG	16	/* 16 streams/rate register */
#define NUM_RATE_REGISTERS		8	/* 4 rate registers */
#define NUM_RATE_PROFILE_MASK           0x00001f00
#define NUM_TDM_STREAMS_FOR_16MB	2	/* 2 consecutive streams */
#define NUM_TDM_STREAMS_FOR_32MB	4	/* 4 consecutive streams */
#define HWIC_TDM_STREAM_OFFSET 		4	/* 4 consecutive streams */
#define PVDM_TDM_STREAM_OFFSET		2	/* 2 consecutive streams */
/* Maximum of 2 cards can run high speed */
#define NUM_HIGH_SPEED_CARD             2 
/*
 * Bit definitions for TDM Stream loopback register
 * bits 31 - 0 apply to streams 0x1f - 0x00 respectively for LPBK_1f_00,
 * bits 31 - 0 apply to streams 0x3f - 0x20 respectively for LPBK_3f_20,
 * loops tdm_out to tdm_in
 */
#define TDMSW_LOOPBACK_DISABLE_ALL	0x00000000
#define TDMSW_LOOPBACK_ENABLE_ALL	0xFFFFFFFF
#define TDMSW_LOOPBACK_DISABLE		0x00000000
#define TDMSW_LOOPBACK_ENABLE		0x00000001
#define NUM_STREAMS_PER_LPBK_REG	32	/* 32 streams/lpbk register */
#define NUM_LPBK_REGISTERS		4	/* 4 lpbk registers */

/*
 * Bit definitions for TDMSW128 CONTROL registers
 */
#define TDMSW_C32_D32_50_SAMPLE_CTL	    0x00010000
#define TDMSW_FRC_CONNMEM_PERR              0x00000100
#define TDMSW_RST			    0x00000010
#define TDMSW_SLAVE_SYNC_DSP_FSYNC	    0x00000000
#define TDMSW_SLAVE_SYNC_D32C32_NO_SAMPLE   0x00000008
#define TDMSW_SLAVE_SYNC_D32C32_SAMPLE      0x00000004
#define TDMSW_SLAVE_SYNC_STBUS		    0x00000000
#define TDMSW_SLAVE_SYNC_TYPE_MASK	    0x0000000C
#define TDMSW_SYNC_SLAVE		    0x00000002

/*
 * Bit definitions for TDMSW MGMT EVENT registers
 */
#define FSYNC_OCCURRED			0x00000002
#define DS0_DUMP_LIMIT_REACHED		0x00000001

/*
 * Bit definitions for TDMSW MGMT ENABLE registers
 */
#define FSYNC_OCCURRED_RUPT_ENBL	0x00000002
#define DS0_DUMP_LIMIT_REACHED_EN	0x00000001

/*
 * Bit definitions for TDMSW ERR EVENT registers
 */
#define DATAMEM_PERR                    0x00000200
#define CONNMEM_PERR                    0x00000100
#define CONNMEM_WR_BAD_PASSWORD         0x00000080
#define CONNMEM_WR_NOT_4BYTES           0x00000040
#define DS0_DUMP_FIFO_OVERFLOW          0x00000020
#define STIT_WRSIZE_ERR_PULSE		0x00000010
#define STIT_RDSIZE_ERR_PULSE		0x00000008
#define STIT_BAD_CMD_ERR_PULSE		0x00000004
#define TDMSW128_FSYNC_UNEXP_ERR	0x00000002
#define TDMSW128_FSYNC_MISSING_ERR	0x00000001

/*
 * Bit definitions for TDMSW ERR ENABLE registers
 */
#define DATAMEM_PERR_RUPT_ENBL                  0x00000200
#define CONNMEM_PERR_RUPT_ENBL                  0x00000100
#define CONNMEM_WR_BAD_PASSWORD_RUPT_ENBL       0x00000080
#define CONNMEM_WR_NOT_4BYTES_RUPT_ENBL         0x00000040
#define DS0_DUMP_FIFO_OVERFLOW_RUPT_ENBL        0x00000020
#define STIT_WRSIZE_ERR_PULSE_RUPT_ENBL		0x00000010
#define STIT_RDSIZE_ERR_PULSE_RUPT_ENBL		0x00000008
#define STIT_BAD_CMD_ERR_PULSE_RUPT_ENBL	0x00000004
#define TDMSW128_FSYNC_UNEXP_ERR_RUPT_ENBL	0x00000002
#define TDMSW128_FSYNC_MISSING_ERR_RUPT_ENBL	0x00000001

/*
 * Bit definitions for (VMCR) VWIC mode control registers
 */
#define QUAD_32M_LT			0xC0000000
#define TRI_32M_LT			0x80000000
#define HWIC_32MBOS_PROFILE		0x04000000
#define ENBL_L1RCLKB			0x02000000	/* L1RCLKB on pin 62 */
#define ENBL_L1RCLKA			0x01000000	/* L1RCLKA on pin 65 */
#define TRI_STATE_ENABLE		0x00800000
#define NMSI_DSL_COMPAT			0x00400000
#ifdef EXTRA
#define WIC_BRI_U_COMPAT		0x00200000
#endif
#define CTS_TO_SCC			0x00100000
#define DCD2_TO_SCC			0x00080000
#define DCD1_TO_SCC			0x00040000
#define NMSI2_ASYNC			0x00000000
#define NMSI2_SYNC			0x00020000
#define NMSI1_ASYNC			0x00000000
#define NMSI1_SYNC			0x00010000

#define NMSI2_DTE			0x00000000
#define NMSI2_DCE			0x00008000
#define NMSI2_DISABLE			0x00000000
#define NMSI2_ENABLE			0x00004000
#define TDMB_BUS_FORMAT			0x00002000
#define TDM_MUX_TDMB_DATA		0x00000000
#define TDM_MUX_TDMA_SIGNALING		0x00001000
#define TDMB_BUS_FORMAT_MASK		0x00002c00
#define TDMB_E32M			0x00002800	/* 32 Mbps */
#define TDMB_E16M			0x00002400	/* 16 Mbps */
#define TDMB_E8M			0x00002000	/* 8 Mbps */
#define TDMB_ST_BUS			0x00000c00	/* 8 Mbps */
#define TDMB_H100			0x00000800	/* 8 Mbps */
#define TDMB_IOM2			0x00000400	/* 2 Mbps */
#define TDMB_SLAVE			0x00000000	/* LT mode */
#define TDMB_MASTER			0x00000200	/* TE mode */
#define TDMB_ENABLE			0x00000100

#define NMSI1_DTE			0x00000000
#define NMSI1_DCE			0x00000080
#define NMSI1_DISABLE			0x00000000
#define NMSI1_ENABLE			0x00000040
#define TDMA_BUS_FORMAT			0x00000020
#define TDM_MUX_TDMA_DATA		0x00000000
#define TDM_MUX_TDMB_SIGNALING		0x00000010
#define TDMA_BUS_FORMAT_MASK		0x0000002c
#define TDMA_E32M                       0x00000028      /* 32 Mbps */
#define TDMA_E16M			0x00000024	/* 16 Mbps */
#define TDMA_E8M			0x00000020      /* 8 Mbps */
#define TDMA_ST_BUS			0x0000000c	/* 8 Mbps */
#define TDMA_H100			0x00000008	/* 8 Mbps */
#define TDMA_IOM2                       0x00000004      /* 2 Mbps */
#define TDMA_SLAVE			0x00000000	/* LT mode */
#define TDMA_MASTER			0x00000002	/* TE mode */
#define TDMA_ENABLE			0x00000001
#define TDMB_SHIFT			8
#define TDMA_SHIFT			0

/*
 * Bit definitions for TDM Config (CR) Register 
 */
#ifdef EXTRA
#define NM3_32MBPS_PROFILE              0x00800000
#define NM3_TDM_FORMAT_MASK		0x00700000
#define NM2_32MBPS_PROFILE              0x00080000
#define NM2_TDM_FORMAT_MASK		0x00070000
#define NM1_32MBPS_PROFILE              0x00008000
#define NM1_TDM_FORMAT_MASK		0x00007000
#define NM0_32MBPS_PROFILE              0x00000800
#define NM0_TDM_FORMAT_MASK		0x00000700
#define AIM1_TDM_FORMAT_MASK		0x00000070
#define AIM0_TDM_FORMAT_MASK		0x00000007
#endif


/* H100 is D8C8 what went wrong, 2 and 4 are the same */
#define TDM_FORMAT_D32C32		0x6
#define TDM_FORMAT_D16C16		0x5
#define TDM_FORMAT_D8C8 		0x4
#define TDM_FORMAT_STBUS		0x3
#define TDM_FORMAT_H100			0x2
#define TDM_FORMAT_IOM2			0x1
#define TDM_FORMAT_STRM_DISABLE		0x0
#define TDM_FORMAT_SHIFT		8
#define TDM_FORMAT_MASK                 0x7

/*
 * Bit definitions for TDM PVDM Config (CR) Register 
 */
#ifdef EXTRA
#define PVDM_TDM_FORMAT_E16M		0x00000001
#define PVDM_TDM_FORMAT_E8M		0x00000000
#endif

/*
 * Bit definitions for NTR CR0, NTR CR0 AUX register
 */
#define WIC3_TDMB_MASK			0xF0000000
#define WIC3_TDMA_MASK			0x0F000000
#define WIC2_TDMB_MASK			0x00F00000
#define WIC2_TDMA_MASK			0x000F0000
#define WIC1_TDMB_MASK			0x0000F000
#define WIC1_TDMA_MASK			0x00000F00
#define WIC0_TDMB_MASK			0x000000F0
#define WIC0_TDMA_MASK			0x0000000F
#define WIC3_TDMB_SHIFT			28
#define WIC3_TDMA_SHIFT			24
#define WIC2_TDMB_SHIFT			20
#define WIC2_TDMA_SHIFT			16
#define WIC1_TDMB_SHIFT			12
#define WIC1_TDMA_SHIFT			8
#define WIC0_TDMB_SHIFT			4
#define WIC_TDM_SHIFT			4

/*
 * Bit definitions for NRT CR1, NTR CR1 AUX register 
 */
#define SPARE_NTR_MASK			0x0f000000
#define SPARE_NTR_SRC			0x0f000000
#define NM5_NTR_MASK			0x00f00000
#define NM5_NTR_SRC			0x00f00000
#define NM4_NTR_MASK			0x000f0000
#define NM4_NTR_SRC			0x000f0000
#define NM3_NTR_MASK			0x0000f000
#define NM3_NTR_SRC			0x0000f000
#define NM2_NTR_MASK			0x00000f00
#define NM2_NTR_SRC			0x00000f00
#define NM1_NTR_MASK			0x000000f0
#define NM1_NTR_SRC			0x000000f0
#define NM0_NTR_MASK			0x0000000f
#define NM0_NTR_SRC			0x0000000f
#ifdef EXTRA
#define NM0_NTR_TDM_PLL			0x0000000e
#define NM0_NTR_LOCAL_OSC		0x0000000d
#endif
#define SPARE_NTR_SHIFT			24
#define NM5_NTR_SHIFT			20
#define NM4_NTR_SHIFT			16
#define NM3_NTR_SHIFT			12
#define NM2_NTR_SHIFT			8
#define NM1_NTR_SHIFT			4

/*
 * Bit definitions for AUX NTR Reference Control in
 * TDM PLL Reference Control register
 */
#define NUM_SEC_REF_OPTIONS             0xb
#define SPARE_SEC_8K_FRAME              0xb
#define SPARE_PRI_8K_FRAME              0xa
#define SCC_SEC_8K_FRAME                0x9
#define SCC_PRI_8K_FRAME                0x8
#define NM5_SEC_8K_FRAME                0x7
#define NM5_PRI_8K_FRAME                0x6
#define NM4_SEC_8K_FRAME                0x5
#define NM4_PRI_8K_FRAME                0x4
#define NM3_SEC_8K_FRAME                0x3
#define NM3_PRI_8K_FRAME                0x2
#define NM2_SEC_8K_FRAME                0x1
#define NM2_PRI_8K_FRAME                0x0

/*
 * Bit definitions for WIC/NM NTR Reference Control table and
 * TDM PLL Reference Control register
 */
#define NUM_PRI_REF_OPTIONS             0xf
#define AUX_NM_NTR                      0xf
#define TDM_PLL_OUTPUT                  0xe
#define TDM_PLL_LOCAL_20MHZ_OSC         0xd
#define NMI_SEC_8K_FRAME                0xc
#define NMI_PRI_8K_FRAME                0xb
#define VWIC3_TDMB                      0xa
#define VWIC3_TDMA_DSL                  0x9
#define VWIC2_TDMB                      0x8
#define VWIC2_TDMA_DSL                  0x7
#define VWIC1_TDMB                      0x6
#define VWIC1_TDMA_DSL                  0x5
#define VWIC0_TDMB                      0x4
#define VWIC0_TDMA_DSL                  0x3
#define NM_SEC_8K_FRAME                 0x2
#define NM_PRI_8K_FRAME                 0x1
#define NTR_REFERENCE_DISABLE           0x0
#define SEC_REF_AUX_NM_SHIFT            12
#define PRI_REF_AUX_NM_SHIFT            8
#define SECONDARY_REF_SHIFT             4
#define PRIMARY_REF_SHIFT               0
#define PLL_REF_MASK                    0xf

/*
 * Bit definitions for TDM PLL Register 
 */
#define TDM_PLL_REF_SRC_AUX_MASK	0x0000ff00
#define TDM_PLL_SEC_REF_SRC_AUX_MASK	0x0000f000
#define TDM_PLL_PRI_REF_SRC_AUX_MASK	0x00000f00

#define TDM_PLL_REF_SRC_MASK		0x000000ff
#define TDM_PLL_SEC_REF_SRC_MASK	0x000000f0
#define TDM_PLL_PRI_REF_SRC_MASK	0x0000000f

#define TDM_PLL_AUX_REF_SRCID		0x0000000f
#define TDM_PLL_PRI_REF_SRC_LOCAL_OSC	0x00000000

/*
 * Defines for DS0_DUMP facility
 */
#ifdef AHSU
#define DUMP_STREAM           		0x007d
#endif
#define GOOFY_DUMP_PORT           	0x007d
#define DONALD_DUMP_PORT           	0x003d
#define SCROOGE_DUMP_PORT           	0x001d
#define DS0_DUMP_ADDR_MASK    		0xffffffc0
#define DUMP_GO               		0x00000001
#define DUMP_CIR_FLAG         		0x00000002
#define DS0_SIZE              		1   /* 1 byte */

/*
 * Defines for TDM SYNCHRONIZER INTERFACE Reg
 */
#define TDMSYNC_SLIP_COUNTER_IN_MASK         	0xffff0000
#define TDMSYNC_F_DELTA_IN_MASK			0x0000ff00
#define TDMSYNC_DETECTED_IN			0x00000080
#define TDMSYNC_8K_VALID_IN			0x00000040
#define TDMSYNC_SYNC_DETECT_PRI_IN              0x00004000
#define TDMSYNC_IN8K_VALID_IN                   0x00001000
#define TDMSYNC_INTERNAL_SYNCHRONIZER           0x00000010
#define TDMSYNC_EXTERNAL_PLL			0x00000000
#define TDMSYNC_WAN_CLK_SEL_INTERNAL_OUT	0x00000008
#define TDMSYNC_WAN_CLK_SEL_EXTERNAL_OUT	0x00000000

#define TDMSYNC_CLASSIC_MODE_OUT		0x00000004
#define TDMSYNC_NORMAL_MODE_OUT			0x00000000

#define TDMSYNC_SLIP_ASSERT_RESET_OUT		0x00000002
#define TDMSYNC_SLIP_DEASSERT_RESET_OUT		0x00000000

#define TDMSYNC_8K_RSEL_SEC_OUT			0x00000001
#define TDMSYNC_8K_RSEL_PRI_OUT			0x00000000


/*
 * Defines for TDM DEBUG Reg
 */
#define TDM_DBG_ST1_STATE                       0x258000
#define TDM_DBG_ERR_EVENT_STATE                 0x258004


#define GOOFY_TDM_SM0_TDM0_STREAM             0x00
#define GOOFY_TDM_SM0_TDM1_STREAM             0x04
#define GOOFY_TDM_SM0_TDM2_STREAM             0x08
#define GOOFY_TDM_SM0_TDM3_STREAM             0x0C
#define GOOFY_TDM_SM2_TDM0_STREAM             0x0C
#define GOOFY_TDM_SM3_TDM3_STREAM             0x0C     /* double wide SM for Megatron */
#define GOOFY_TDM_SM1_TDM0_STREAM             0x10
#define GOOFY_TDM_SM1_TDM1_STREAM             0x14
#define GOOFY_TDM_SM1_TDM2_STREAM             0x18
#define GOOFY_TDM_SM1_TDM3_STREAM             0x1C
#define GOOFY_TDM_SM2_TDM1_STREAM             0x1C
#define GOOFY_TDM_SM3_TDM4_STREAM             0x1C     /* double wide SM for Megatron */
#define GOOFY_TDM_HWIC1_TDMA_STREAM           0x20
#define GOOFY_TDM_HWIC1_TDMB_STREAM           0x24
#define GOOFY_TDM_HWIC1_TDMC_STREAM           0x28
#define GOOFY_TDM_HWIC1_TDMD_STREAM           0x2C
#define GOOFY_TDM_HWIC0_TDMA_STREAM           0x2C
#define GOOFY_TDM_HWIC0_TDMB_STREAM           0x30
#define GOOFY_TDM_HWIC0_TDMC_STREAM           0x34
#define GOOFY_TDM_HWIC3_TDMA_STREAM           0x38
#define GOOFY_TDM_HWIC3_TDMB_STREAM           0x3C
#define GOOFY_TDM_HWIC3_TDMC_STREAM           0x40
#define GOOFY_TDM_HWIC3_TDMD_STREAM           0x44
#define GOOFY_TDM_HWIC2_TDMA_STREAM           0x44
#define GOOFY_TDM_HWIC2_TDMB_STREAM           0x48
#define GOOFY_TDM_HWIC2_TDMC_STREAM           0x4C
#define GOOFY_TDM_PVDM0_STRM0_STREAM          0x50
#define GOOFY_TDM_PVDM0_STRM1_STREAM          0x52
#define GOOFY_TDM_PVDM0_STRM2_STREAM          0x54
#define GOOFY_TDM_PVDM0_STRM3_STREAM          0x56
#define GOOFY_TDM_PVDM1_STRM0_STREAM          0x58
#define GOOFY_TDM_PVDM1_STRM1_STREAM          0x5A
#define GOOFY_TDM_PVDM1_STRM2_STREAM          0x5C
#define GOOFY_TDM_PVDM1_STRM3_STREAM          0x5E
#define GOOFY_TDM_PVDM2_STRM0_STREAM          0x60
#define GOOFY_TDM_PVDM2_STRM1_STREAM          0x62
#define GOOFY_TDM_PVDM2_STRM2_STREAM          0x64
#define GOOFY_TDM_PVDM2_STRM3_STREAM          0x66
#define GOOFY_TDM_PVDM3_STRM0_STREAM          0x68
#define GOOFY_TDM_PVDM3_STRM1_STREAM          0x6A
#define GOOFY_TDM_PVDM3_STRM2_STREAM          0x6C
#define GOOFY_TDM_PVDM3_STRM3_STREAM          0x6E
#define GOOFY_TDM_EXPANSION0_STREAM           0x70
#define GOOFY_TDM_SM3_TDM0_STREAM             0x70
#define GOOFY_TDM_EXPANSION1_STREAM           0x74
#define GOOFY_TDM_SM3_TDM1_STREAM             0x74
#define GOOFY_TDM_EXPANSION2_STREAM           0x78
#define GOOFY_TDM_SM3_TDM2_STREAM             0x78
#define GOOFY_TDM_EXPANSION3_STREAM           0x7C
#define GOOFY_TDM_SM2_TDM2_STREAM             0x7C
#define GOOFY_TDM_SM3_TDM5_STREAM             0x7C     /* double wide SM for Megatron */
#define GOOFY_TDM_DS0_DUMP_STREAM             0x7D
#define GOOFY_TDM_TPSM0_STREAM                0x7E
#define GOOFY_TDM_TPSM1_STREAM                0x7F

/*
#define DONALD_TDM_HWIC0_TDMA_STREAM          0x00
#define DONALD_TDM_HWIC0_TDMB_STREAM          0x04
#define DONALD_TDM_HWIC0_TDMC_STREAM          0x08
#define DONALD_TDM_HWIC0_TDMD_STREAM          0x0C
#define DONALD_TDM_HWIC1_TDMA_STREAM          0x0C
#define DONALD_TDM_HWIC1_TDMB_STREAM          0x10
#define DONALD_TDM_HWIC1_TDMC_STREAM          0x14
#define DONALD_TDM_HWIC2_TDMA_STREAM          0x24
#define DONALD_TDM_HWIC2_TDMB_STREAM          0x28
#define DONALD_TDM_HWIC2_TDMC_STREAM          0x2C
#define DONALD_TDM_PVDM0_STRM0_STREAM         0x30
#define DONALD_TDM_PVDM0_STRM1_STREAM         0x32
#define DONALD_TDM_PVDM0_STRM2_STREAM         0x34
#define DONALD_TDM_PVDM0_STRM3_STREAM         0x36
#define DONALD_TDM_DS0_DUMP_STREAM            0x3D
#define DONALD_TDM_TPSM0_STREAM               0x3E
#define DONALD_TDM_TPSM1_STREAM               0x3F
*/

#define SCROOGE_TDM_HWIC1_TDMA_STREAM         0x00
#define SCROOGE_TDM_HWIC1_TDMB_STREAM         0x04
#define SCROOGE_TDM_HWIC1_TDMC_STREAM         0x08
#define SCROOGE_TDM_HWIC0_TDMA_STREAM         0x0C
#define SCROOGE_TDM_HWIC0_TDMB_STREAM         0x10
#define SCROOGE_TDM_HWIC0_TDMC_STREAM         0x14
#define SCROOGE_TDM_PVDM0_STRM0_STREAM        0x18
#define SCROOGE_TDM_PVDM0_STRM1_STREAM        0x19
#define SCROOGE_TDM_PVDM0_STRM2_STREAM        0x1a
#define SCROOGE_TDM_PVDM0_STRM3_STREAM        0x1b
#define SCROOGE_TDM_DS0_DUMP_STREAM           0x1D
#define SCROOGE_TDM_TPSM0_STREAM              0x1E
#define SCROOGE_TDM_TPSM1_STREAM              0x1F


/*
 * General Bit Definitions 
 */
#define MEM_INCREMENT_1			0x00000001
#define MEM_DECREMENT_1			0xffffffff

/*
 * the following 4 defines are bit specific and should not be changed
 * bit 0 = 0 indicates DATA, bit 0 = 1 indicates SIGNALING
 * bit 1 = 0 indicates TDMA, bit 1 = 1 indicates TDMB
 */
#define TDMA_DATA			0	/* b0000 */
#define TDMA_SIGNALING			1	/* b0001 */
#define TDMB_DATA			2	/* b0010 */
#define TDMB_SIGNALING			3	/* b0011 */

#define TDM_MUX_SIGNALING		1	/* b0001 */
#define TDM_MUX_B			2	/* b0010 */
#define TDM_MUX_A			0	/* b0000 */

#define TDM_IOM2			1
#define TDM_H100			2
#define TDM_STBUS			3

#define TDM_SLAVE			0
#define TDM_MASTER			1

#define PASSTHRU_MODE			0
#define LOOPBACK_MODE			1

#define GOOFY_MAX_SW128                 1      /* temp deal with one chip */
#define GOOFY_MAX_TDM_STREAMS           0x7C   /* 124 */
#define DONALD_MAX_TDM_STREAMS          0x3C   /* 60  */
#define SCROOGE_MAX_TDM_STREAMS         0x1C   /* 28  */

#define SIZE_OF_CONN_MEM                0x4000 /* in 4 bytes unit */

enum {
    HWIC_STREAM,
    PVDM_STREAM,
    SM_STREAM,
    EXPANSION_STREAM,
    DSP0_DUMP_STREAM,
    HDLC_STREAM,
};

#ifndef LINUX_KLM
/*
 * device callin function - service provided and defined by the device
 */
typedef struct sw128_callin_fvt_t_ {
  int     (*init)(dev_object_t *);
  int     (*sw128_2_sw128_test)(dev_object_t *, dev_object_t *);
  void    (*lpbk_control)(dev_object_t *, int, int);
  void    (*output_control)(dev_object_t *, int, int);
  boolean (*stream_connect)(dev_object_t *, uint, uint, uint, uint, uint);
  void    (*stream_disconnect)(dev_object_t *, uint, uint);
  void    (*speed_select)(dev_object_t *, int, int);
  void    (*reset_tdm_module)(dev_object_t *, int);
  int     (*get_tdm_strm)(dev_object_t *, int, int, int);
  uint    (*get_tdm_base_addr)(dev_object_t *);
  boolean (*message_mode_write)(dev_object_t *, uint, uint, uint);
  boolean (*data_mem_dump)(dev_object_t *, uint, uchar *, uint, int);
  int     (*vmcr_control)(dev_object_t *, uint, uint, uint);
}sw128_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct sw128_callout_fvt_t_ {
  /*
   * Vectors set by the upper level (eg., platform).
   */
   void   (*reset)(dev_object_t *, goofy_dev_t, int, boolean);
   int    (*set_intr)(dev_object_t *, goofy_intr_class_t,
		      uint, uint, uint, boolean);
   PFI    (*install_intr_vect)(dev_object_t *, goofy_intr_class_t, 
			       uint, uint, PFI);
   void * (*get_plat_goofy_inst)(uint32_t, uint32_t);
   void   (*platform_set_tdm_pll_clock)(uint32_t);
   int    (*is_using_int_sync)(void);
} sw128_callout_fvt_t;


/*
** Define the dev goofy device object structure.
*/
typedef struct sw128_object_t_ {
    dev_object_t         base;
    sw128_callin_fvt_t   *callin_fvt;
    sw128_callout_fvt_t  *callout_fvt;
    int tdm_no;          /* tdm number */
#ifdef AHSU
    asic_type_enum 	 asic_type;
    int dump_port;       /* Donald 0x3D, Goofy 0x7D */
    int max_tdm_streams; /* Donald 0x3C, Goofy 0x7C */
#endif
} sw128_object_t;


extern sw128_test_regs_t goofy_sw128_tdm_test_table[];
extern sw128_test_regs_t donald_sw128_tdm_test_table[];
extern sw128_test_regs_t scrooge_sw128_tdm_test_table[];
extern void sw128_tdm_create(dev_object_t *dev, 
                             dev_error_report_t error_report_fn);
extern void sw128_dev_destroy(dev_object_t **dev);
extern void sw128_output_control (dev_object_t *dev, int stream, int flag);
extern void sw128_lpbk_control (dev_object_t *dev,
                                int stream, int flag);
extern boolean sw128_stream_connect (dev_object_t *dev,
                                     uint istrm, uint itslot,
                                     uint ostrm, uint otslot,
                                     uint connect_sel);
extern void sw128_stream_disconnect (dev_object_t *dev, uint ostrm, uint otslot);
extern boolean sw128_data_mem_dump(dev_object_t *dev, uint stream,
                            uchar *buff_p, uint frame_num, int total_tslot);
extern void sw128_cleanup (dev_object_t *dev);
extern boolean sw128_message_mode_write (dev_object_t *dev, uint stream,
                                     uint tslot, uint test_pat);
extern void sw128_speed_select (dev_object_t *dev, int stream, int speed);
extern int  sw128_vmcr_control (dev_object_t *dev, uint hwic_num, uint mask,
				uint value);
extern boolean sw128_disconnect (dev_object_t *, uint , uint , uint , uint );
extern int sw128_get_rate (sw128_regs_t *, uint );
extern void sw128_set_clock (dev_object_t *);
extern int rvw_word (int , uint , uint , uint , int , uint *, volatile uint *);
extern void tdm_sw128_ctl (sw128_regs_t *, int , boolean );
extern int sw128_get_cm_addr (uint , uint , int );
extern int iripple_test (volatile uint *, uint );
extern int tdm_check_fsync_intr(sw128_regs_t *);

#endif /* linux_klm */

#endif /* __DEV_TDM_H__ */

/******** History ********
$Log: dev_tdm.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
