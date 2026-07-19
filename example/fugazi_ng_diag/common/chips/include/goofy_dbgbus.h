/* $Id: goofy_dbgbus.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/goofy_dbgbus.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's debug bus registers
 *
 * July 2006, Bao Buu
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef GOOFY_DBGBUS_H
#define GOOFY_DBGBUS_H

#define GOOFY_DEBUG_ID  0x44425547

#define DEV0_I2C_ADDR   0xB0
#define DEV1_I2C_ADDR   0xB2
#define DEV2_I2C_ADDR   0xB4
#define DEV3_I2C_ADDR   0xB6
#define DEV4_I2C_ADDR   0xB8
#define DEV5_I2C_ADDR   0xBA
#define DEV6_I2C_ADDR   0xBC
#define DEV7_I2C_ADDR   0xBE

/*
 * Debug bus module internal register address
 */
#define OFFSET_DBGBUS_ID        0x00000000
#define OFFSET_DBGBUS_SCRATCH   0x00000004
#define OFFSET_DBGBUS_STATUS    0x00000008
#define OFFSET_DBGBUS_TMO_VAL   0x0000000c
#define OFFSET_DBGBUS_PLL_STAT  0x00000014

/*
 * Goofy internal module address offset in
 * the debug bus address space
 */
#define OFFSET_DBGBUS_REGS        0x00000000
#define OFFSET_DBGBUS_QUADWIC     0x00080000
#define OFFSET_DBGBUS_PCIE        0x00100000
#define OFFSET_DBGBUS_HDLC_0      0x00200000
#define OFFSET_DBGBUS_HDLC_1      0x00208000
#define OFFSET_DBGBUS_HDLC_2      0x00210000
#define OFFSET_DBGBUS_HDLC_3      0x00218000
#define OFFSET_DBGBUS_HDLC_4      0x00220000
#define OFFSET_DBGBUS_HDLC_5      0x00228000
#define OFFSET_DBGBUS_HDLC_6      0x00230000
#define OFFSET_DBGBUS_HDLC_7      0x00238000
#define OFFSET_DBGBUS_PKTPUMP     0x00240000
#define OFFSET_DBGBUS_SCC         0x00248000
#define OFFSET_DBGBUS_TDM         0x00258000
#define OFFSET_DBGBUS_INTR        0x00280000
#define OFFSET_DBGBUS_GPIO        0x00280200
#define OFFSET_DBGBUS_GBL_REG     0x00280400
#define OFFSET_DBGBUS_RESET       0x00280480
#define OFFSET_DBGBUS_I2C0        0x00290000
#define OFFSET_DBGBUS_I2C1        0x002a0000
#define OFFSET_DBGBUS_I2C2        0x002b0000
#define OFFSET_DBGBUS_I2C3        0x002c0000
#define OFFSET_DBGBUS_I2C4        0x002d0000
#define OFFSET_DBGBUS_BUSMON      0x00300000
#define OFFSET_DBGBUS_DN0         0x00380000
#define OFFSET_DBGBUS_DN1         0x00400000
#define OFFSET_DBGBUS_DN2         0x00480000
#define OFFSET_DBGBUS_DN3         0x00500000
#define OFFSET_DBGBUS_DN4         0x00580000
#define OFFSET_DBGBUS_DN5         0x00600000
#define OFFSET_DBGBUS_DN6         0x00680000
#define OFFSET_DBGBUS_DN7         0x00700000
#define OFFSET_DBGBUS_I2C_1       0x00790000
#define OFFSET_DBGBUS_I2C_2       0x007A0000
#define OFFSET_DBGBUS_I2C_3       0x007B0000
#define OFFSET_DBGBUS_I2C_4       0x007C0000
#define OFFSET_DBGBUS_I2C_5       0x007D0000

#define DBGBUS_HDLC_SIZE          0x8000
#define DBGBUS_DN_SIZE            0x80000
#define DBGBUS_I2C_SIZE           0x10000

typedef struct goofy_dbgbus_pcie_ {
    volatile uint32_t pcie_us_test_in;                      /* 0x0 */
    uint32_t    reserved[3];
    volatile uint32_t pcie_ds1_test_in;                     /* 0x10 */
    uint32_t    reserved1[3];
    volatile uint32_t pcie_ds2_test_in;                     /* 0x20 */
    uint32_t    reserved2[3];
    volatile uint32_t pcie_ds3_test_in;                     /* 0x30 */
    uint32_t    reserved3[3];
    volatile uint32_t pcie_ds4_test_in;                     /* 0x40 */
    uint32_t    reserved4[3];
    uint32_t    reserved5[0x43];      /* 0x50 - 0x158 */
} goofy_dbgbus_pcie;

typedef struct goofy_dbgbus_regs_ {
    volatile uint32_t id;                      /* 0x0 */
    volatile uint32_t scratch;
    volatile uint32_t status;
    volatile uint32_t timeout;
    volatile uint32_t reserved;
    volatile uint32_t pll_status;
//    volatile uint32_t reserved1[0x1fffa];      /* 0x18 - 0x80000 */
} goofy_dbgbus_regs;

/*
 * DEBUG BUS Registers (0x00000000)
 */
#define MASK_DBGBUS_STATUS_BACKEND_ADRS_ERR                 0x00000001
#define MASK_DBGBUS_STATUS_BACKEND_TMO_ERR                  0x00000002
#define MASK_DBGBUS_STATUS_BACKEND_VLD_ERR                  0x00000004
#define MASK_DBGBUS_STATUS_WRITE_DETECTED                   0x00000008
#define MASK_DBGBUS_STATUS_ALIGNMENT_ERR                    0x00000010
#define MASK_DBGBUS_STATUS_I2C_CLK_SEL                      0x80000000
#define MASK_DBGBUS_TIMEOUT_CYCLES                          0x000003ff
#define MASK_DBGBUS_PLL_STATUS_UNLOCKED                     0x00000001
#define MASK_DBGBUS_PLL_STATUS_UNLOCKED_HIST                0x00000002
#define MASK_DBGBUS_PLL_STATUS_I2C_REF_CLOCK                0x00000004

typedef struct goofy_dbgbus_hwic_ {
    volatile uint32_t sti_avails;          /* 0x0 */
    volatile uint32_t sti_states;
    volatile uint32_t stii_pc_cnt;
    volatile uint32_t stii_nc_cnt;
    volatile uint32_t stit_pc_cnt;
    volatile uint32_t stit_nc_cnt;
    volatile uint32_t hwic0_err;
    volatile uint32_t hwic1_err;
    volatile uint32_t hwic2_err;            /* 0x20 */
    volatile uint32_t hwic3_err;
    volatile uint32_t hwic_bdg_id;
//    uint32_t reserved[0x1fff5];             /* 0x2C - 0x80000 */
}goofy_dbgbus_hwic;

/*
 * DEBUG BUS HWIC Registers (0x00080000)
 */
#define MASK_DBGBUS_HWIC_AVAIL_STIT_RD_AVAIL_IN               0x00000004
#define MASK_DBGBUS_HWIC_AVAIL_STIT_RC_AVAIL_IN               0x00000008
#define MASK_DBGBUS_HWIC_AVAIL_STIT_ND_AVAIL_OUT              0x00000010
#define MASK_DBGBUS_HWIC_AVAIL_STIT_NC_AVAIL_OUT              0x00000020
#define MASK_DBGBUS_HWIC_AVAIL_STIT_PD_AVAIL_OUT              0x00000040
#define MASK_DBGBUS_HWIC_AVAIL_STIT_PC_AVAIL_OUT              0x00000080
#define MASK_DBGBUS_HWIC_AVAIL_STII_RD_AVAIL_OUT              0x00040000
#define MASK_DBGBUS_HWIC_AVAIL_STII_RC_AVAIL_OUT              0x00080000
#define MASK_DBGBUS_HWIC_AVAIL_STII_ND_AVAIL_IN               0x00100000
#define MASK_DBGBUS_HWIC_AVAIL_STII_NC_AVAIL_IN               0x00200000
#define MASK_DBGBUS_HWIC_AVAIL_STII_PD_AVAIL_IN               0x00400000
#define MASK_DBGBUS_HWIC_AVAIL_STII_PC_AVAIL_IN               0x00800000

#define MASK_DBGBUS_HWIC_STATES_HW0_STII_REQ_STATE            0x0000000f
#define MASK_DBGBUS_HWIC_STATES_HW1_STII_REQ_STATE            0x000000f0
#define MASK_DBGBUS_HWIC_STATES_HW2_STII_REQ_STATE            0x00000f00
#define MASK_DBGBUS_HWIC_STATES_HW3_STII_REQ_STATE            0x0000f000
#define MASK_DBGBUS_HWIC_STATES_HW0_STII_PC_STATE             0x00030000
#define MASK_DBGBUS_HWIC_STATES_HW1_STII_PC_STATE             0x000c0000
#define MASK_DBGBUS_HWIC_STATES_HW2_STII_PC_STATE             0x00300000
#define MASK_DBGBUS_HWIC_STATES_HW3_STII_PC_STATE             0x00c00000
#define MASK_DBGBUS_HWIC_STATES_HW0_STII_NPC_STATE            0x03000000
#define MASK_DBGBUS_HWIC_STATES_HW1_STII_NPC_STATE            0x0c000000
#define MASK_DBGBUS_HWIC_STATES_HW2_STII_NPC_STATE            0x30000000
#define MASK_DBGBUS_HWIC_STATES_HW3_STII_NPC_STATE            0xc0000000

typedef struct goofy_dbgbus_hdlc_ {
    volatile uint32_t hdlc_states;         /* 0x0 */
    volatile uint32_t sti_states;
    volatile uint32_t stii_pc_count;
    volatile uint32_t stii_nc_count;
    volatile uint32_t stit_pc_count;
    volatile uint32_t stit_nc_count;
    volatile uint32_t hdlc_err;            /* 0x18 */
//    volatile uint32_t reserved[0x1ff9];    /* 0x1C - 0x8000 */
}goofy_dbgbus_hdlc;

/*
 * DEBUG BUS HDLC Registers (0x00200000)
 */
#define MASK_HDLC_I_HDLC_STATES_INIT_ARB_STATE              0x00000001
#define MASK_HDLC_I_HDLC_STATES_EG_Q_CNTL_STATE             0x0000001e
#define MASK_HDLC_I_HDLC_STATES_EG_CORE_CNTL_STATE          0x000001e0
#define MASK_HDLC_I_HDLC_STATES_TX_STATE                    0x00007e00
#define MASK_HDLC_I_HDLC_STATES_ING_Q_CNTL_STATE            0x000f8000
#define MASK_HDLC_I_HDLC_STATES_ING_CORE_CNTL_STATE         0x00f00000
#define MASK_HDLC_I_HDLC_STATES_RX_STATE                    0x0f000000
#define MASK_HDLC_I_STI_STATES_STIT_REG_STATE               0x0000000f
#define MASK_HDLC_I_STI_STATES_STIT_PC_STATE                0x00000030
#define MASK_HDLC_I_STI_STATES_STIT_NPC_STATE               0x00000040
#define MASK_HDLC_I_STI_STATES_STII_STATE                   0x00000780
#define MASK_HDLC_I_STII_PC_COUNT_VALUE                     0x000fffff
#define MASK_HDLC_I_STII_NC_COUNT_VALUE                     0x000fffff
#define MASK_HDLC_I_STIT_PC_COUNT_VALUE                     0x000fffff
#define MASK_HDLC_I_STIT_NC_COUNT_VALUE                     0x000fffff
#define MASK_HDLC_I_HDLC_ERR_STII_RESP_CMD_ERR              0x00000001
#define MASK_HDLC_I_HDLC_ERR_STII_RESP_SIZE_ERR             0x00000002
#define MASK_HDLC_I_HDLC_ERR_STIT_SIZE_ERR                  0x00000004
#define MASK_HDLC_I_HDLC_ERR_STIT_NPW_ERR                   0x00000008
#define MASK_HDLC_I_HDLC_ERR_STIT_BE_ERR                    0x00000010
#define MASK_HDLC_I_HDLC_ERR_EG_UNDERFLOW                   0x00000020
#define MASK_HDLC_I_HDLC_ERR_ING_OVERFLOW                   0x00000040
#define MASK_HDLC_I_HDLC_ERR_ING_FRAME_SIZE_ERR             0x00000080

typedef struct goofy_dbgbus_pktpump_ {
    volatile uint32_t pp_states;           /* 0x0 */
    volatile uint32_t sti_states;
    volatile uint32_t eg_stii_pc_count;
    volatile uint32_t eg_stii_nc_count;
    volatile uint32_t ing_stii_pc_count;
    volatile uint32_t ing_stii_nc_count;
    volatile uint32_t stit_pc_count;
    volatile uint32_t stit_nc_count;
    volatile uint32_t pp_err;              /* 0x20 */
//    volatile uint32_t reserved[0x1ff7];    /* 0x24 - 0x8000 */
}goofy_dbgbus_pktpump;

/*
 * DEBUG BUS PacketPump Registers (0x00200000)
 */
#define MASK_DBGBUS_PP_PP_STATE_HOST_ING_DMA                0x00000007
#define MASK_DBGBUS_PP_PP_STATE_HOST_EG_DMA                 0x00000038
#define MASK_DBGBUS_PP_PP_STATE_HPI_TRNSFR                  0x00000fc0
#define MASK_DBGBUS_PP_PP_STATE_HPI_PUMP                    0x0001f000
#define MASK_DBGBUS_PP_STI_STATE_STIT_REG                   0x0000000f
#define MASK_DBGBUS_PP_STI_STATE_STIT_PC                    0x00000030
#define MASK_DBGBUS_PP_STI_STATE_STIT_NPC                   0x00000040
#define MASK_DBGBUS_PP_STI_STATE_EG_STIT                    0x00000380
#define MASK_DBGBUS_PP_STI_STATE_ING_STIT                   0x00003c00
#define MASK_DBGBUS_PP_EG_STIT_PC_COUNT                     0x000fffff
#define MASK_DBGBUS_PP_EG_STIT_NC_COUNT                     0x000fffff
#define MASK_DBGBUS_PP_ING_STIT_PC_COUNT                    0x000fffff
#define MASK_DBGBUS_PP_ING_STIT_NC_COUNT                    0x000fffff
#define MASK_DBGBUS_PP_STIT_PC_COUNT                        0x000fffff
#define MASK_DBGBUS_PP_STIT_NC_COUNT                        0x000fffff
#define MASK_DBGBUS_PP_ERR_EG_STIT_CMD_ERR                  0x00000001
#define MASK_DBGBUS_PP_ERR_EG_STIT_RESP_SIZE_ERR            0x00000002
#define MASK_DBGBUS_PP_ERR_ING_STIT_CMD_ERR                 0x00000004
#define MASK_DBGBUS_PP_ERR_ING_STIT_RESP_SIZE_ERR           0x00000008
#define MASK_DBGBUS_PP_ERR_STIT_SIZE_ERR                    0x00000010
#define MASK_DBGBUS_PP_ERR_STIT_NPW_ERR                     0x00000020
#define MASK_DBGBUS_PP_ERR_STIT_BE_ERR                      0x00000040
#define MASK_DBGBUS_PP_ERR_ING_Q0_ERR_FULL                  0x00000080
#define MASK_DBGBUS_PP_ERR_ING_Q1_ERR_FULL                  0x00000100
#define MASK_DBGBUS_PP_ERR_CHPI_ERR                         0x00000200
#define MASK_DBGBUS_PP_ERR_DSP_BUS_TIMEOUT                  0x00000400
#define MASK_DBGBUS_PP_ERR_BAD_DSP0                         0x00000800

typedef struct goofy_dbgbus_scc_ {
    volatile uint32_t sti_state;
    volatile uint32_t scc_err;
//    uint32_t reserved[0x1ffe];
}goofy_dbgbus_scc;

/*
 * DEBUG BUS SCC Registers (0x00200000)
 */
#define MASK_DBGBUS_SCC_STI_STATE_STIT_REG                  0x0000000f
#define MASK_DBGBUS_SCC_STI_STATE_STIT_NPC                  0x00000010
#define MASK_DBGBUS_SCC_STI_STATE_STIT_PC                   0x00000060

typedef struct goofy_dbgbus_tdm_ {
    volatile uint32_t sti_state;
    volatile uint32_t tdm_err;
//    uint32_t reserved[0x1ffe];
}goofy_dbgbus_tdm;

/*
 * DEBUG BUS TDM Registers (0x00258000)
 */
#define MASK_DBGBUS_TDM_STI_STATE_STIT_REG_STATE            0x0000000f
#define MASK_DBGBUS_TDM_STI_STATE_STIT_PC_STATE             0x00000030
#define MASK_DBGBUS_TDM_STI_STATE_STIT_NC_STATE             0x00000040
#define MASK_DBGBUS_TDM_STI_STATE_STII_PD_AVAIL_IN          0x00000100
#define MASK_DBGBUS_TDM_STI_STATE_STII_PC_AVAIL_IN          0x00000200
#define MASK_DBGBUS_TDM_STI_STATE_STIT_RD_AVAIL_IN          0x00000400
#define MASK_DBGBUS_TDM_STI_STATE_STIT_RC_AVAIL_IN          0x00000800
#define MASK_DBGBUS_TDM_STI_STATE_STIT_NC_AVAIL_OUT         0x00001000
#define MASK_DBGBUS_TDM_STI_STATE_STIT_PC_AVAIL_OUT         0x00002000
#define MASK_DBGBUS_TDM_TDM_ERR_REGS                        0x000003ff

typedef struct goofy_dbgbus_busmon_ {
    volatile uint32_t bus_monitor_control;
    volatile uint32_t hsib_error_mask;
    volatile uint32_t address;
    volatile uint32_t address_mask;
    volatile uint32_t low_data;
    volatile uint32_t high_data;
    volatile uint32_t low_data_mask;
    volatile uint32_t high_data_mask;
    volatile uint32_t dn_ireq;
    volatile uint32_t dn_ireq_mask;
    volatile uint32_t dn_tsel;
    volatile uint32_t dn_tsel_mask;
    volatile uint32_t arbiter_grant;
    volatile uint32_t arbiter_grant_mask;
    volatile uint32_t dn_bus_cycle;
    volatile uint32_t dn_bus_cycle_mask;
    volatile uint32_t dn_opcode;
    volatile uint32_t dn_opcode_mask;
    volatile uint32_t dn_priority;
    volatile uint32_t dn_priority_mask;
    uint32_t reserved[0x1ffec];
}goofy_dbgbus_busmon;

/*
 * DEBUG BUS HSIB Bus Monitor Registers (0x00300000)
 */

typedef struct goofy_dbgbus_ {
    goofy_dbgbus_regs dbgbus;        /* 0x00000000 - 0x0007FFFC */
    goofy_dbgbus_hwic hwic;          /* 0x00080000 - 0x000FFFFC */
    goofy_dbgbus_pcie pcie[2];       /* 0x00100000 - 0x001FFFFC */
    goofy_dbgbus_hdlc hdlc[8];       /* 0x00200000 - 0x0023FFFC */
    goofy_dbgbus_pktpump pktpump;    /* 0x00240000 - 0x00247FFC */
    goofy_dbgbus_scc scc;            /* 0x00248000 - 0x0024FFFC */
    uint32_t reserved[0x2000];          /* 0x00250000 - 0x00257FFC */
    goofy_dbgbus_tdm tdm;            /* 0x00258000 - 0x0025FFFC */
    uint32_t reserved2[0x8000];         /* 0x00260000 - 0x0027FFFC */
    volatile uint32_t goofy_dbgbus_gbl_reg[0x20000]; /* 0x00280000 - 0x002FFFFC */
    goofy_dbgbus_busmon busmon;      /* 0x00300000 - 0x0037FFFC */
} goofy_dbgbus;

  /*
  * busmon.bus_monitor_control
  *  0: 0 bus_mon_enable
  *  2: 1 bus_mon_mode
  * 10: 3 cap_length
  * 18:11 addr_of_trigger
  * 30:19 reserved1
  * 31:31 bus_mon_trigger_done
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_BUS_MONITOR_CONTROL_BUS_MON_ENABLE    0
      #define OFFSET_BUSMON_BUS_MONITOR_CONTROL_BUS_MON_MODE      1
      #define OFFSET_BUSMON_BUS_MONITOR_CONTROL_CAP_LENGTH        3
      #define OFFSET_BUSMON_BUS_MONITOR_CONTROL_ADDR_OF_TRIGGER   11
      #define OFFSET_BUSMON_BUS_MONITOR_CONTROL_BUS_MON_TRIGGER_DONE  31
    /**** field masks **********************/
      #define MASK_BUSMON_BUS_MONITOR_CONTROL_BUS_MON_ENABLE      0x00000001
      #define MASK_BUSMON_BUS_MONITOR_CONTROL_BUS_MON_MODE        0x00000006
      #define MASK_BUSMON_BUS_MONITOR_CONTROL_CAP_LENGTH          0x000007f8
      #define MASK_BUSMON_BUS_MONITOR_CONTROL_ADDR_OF_TRIGGER     0x0007f800
      #define MASK_BUSMON_BUS_MONITOR_CONTROL_BUS_MON_TRIGGER_DONE  0x80000000
  /******************************************
  * busmon.hsib_error_mask
  *  0: 0 mask
  * 31: 1 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_HSIB_ERROR_MASK_MASK                  0
    /**** field masks **********************/
      #define MASK_BUSMON_HSIB_ERROR_MASK_MASK                    0x00000001
  /******************************************
  * busmon.address
  * 31: 0 regs
  ******************************************/
  /******************************************
  * busmon.address_mask
  * 31: 0 regs
  ******************************************/
  /******************************************
  * busmon.low_data
  * 31: 0 regs
  ******************************************/
  /******************************************
  * busmon.high_data
  * 31: 0 regs
  ******************************************/
  /******************************************
  * busmon.low_data_mask
  * 31: 0 regs
  ******************************************/
  /******************************************
  * busmon.high_data_mask
  * 31: 0 regs
  ******************************************/
  /******************************************
  * busmon.dn_ireq
  *  0: 0 ht
  *  1: 1 pci0
  *  2: 2 pci1
  *  3: 3 wan
  *  4: 4 hwic
  *  5: 5 glbl
  * 31: 6 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_IREQ_HT                            0
      #define OFFSET_BUSMON_DN_IREQ_PCI0                          1
      #define OFFSET_BUSMON_DN_IREQ_PCI1                          2
      #define OFFSET_BUSMON_DN_IREQ_WAN                           3
      #define OFFSET_BUSMON_DN_IREQ_HWIC                          4
      #define OFFSET_BUSMON_DN_IREQ_GLBL                          5
    /**** field masks **********************/
      #define MASK_BUSMON_DN_IREQ_HT                              0x00000001
      #define MASK_BUSMON_DN_IREQ_PCI0                            0x00000002
      #define MASK_BUSMON_DN_IREQ_PCI1                            0x00000004
      #define MASK_BUSMON_DN_IREQ_WAN                             0x00000008
      #define MASK_BUSMON_DN_IREQ_HWIC                            0x00000010
      #define MASK_BUSMON_DN_IREQ_GLBL                            0x00000020
  /******************************************
  * busmon.dn_ireq_mask
  *  0: 0 ht
  *  1: 1 pci0
  *  2: 2 pci1
  *  3: 3 wan
  *  4: 4 hwic
  *  5: 5 glbl
  * 31: 6 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_IREQ_MASK_HT                       0
      #define OFFSET_BUSMON_DN_IREQ_MASK_PCI0                     1
      #define OFFSET_BUSMON_DN_IREQ_MASK_PCI1                     2
      #define OFFSET_BUSMON_DN_IREQ_MASK_WAN                      3
      #define OFFSET_BUSMON_DN_IREQ_MASK_HWIC                     4
      #define OFFSET_BUSMON_DN_IREQ_MASK_GLBL                     5
    /**** field masks **********************/
      #define MASK_BUSMON_DN_IREQ_MASK_HT                         0x00000001
      #define MASK_BUSMON_DN_IREQ_MASK_PCI0                       0x00000002
      #define MASK_BUSMON_DN_IREQ_MASK_PCI1                       0x00000004
      #define MASK_BUSMON_DN_IREQ_MASK_WAN                        0x00000008
      #define MASK_BUSMON_DN_IREQ_MASK_HWIC                       0x00000010
      #define MASK_BUSMON_DN_IREQ_MASK_GLBL                       0x00000020
  /******************************************
  * busmon.dn_tsel
  *  0: 0 ht
  *  1: 1 pci0
  *  2: 2 pci1
  *  3: 3 wan
  *  4: 4 hwic
  *  5: 5 glbl
  * 31: 6 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_TSEL_HT                            0
      #define OFFSET_BUSMON_DN_TSEL_PCI0                          1
      #define OFFSET_BUSMON_DN_TSEL_PCI1                          2
      #define OFFSET_BUSMON_DN_TSEL_WAN                           3
      #define OFFSET_BUSMON_DN_TSEL_HWIC                          4
      #define OFFSET_BUSMON_DN_TSEL_GLBL                          5
    /**** field masks **********************/
      #define MASK_BUSMON_DN_TSEL_HT                              0x00000001
      #define MASK_BUSMON_DN_TSEL_PCI0                            0x00000002
      #define MASK_BUSMON_DN_TSEL_PCI1                            0x00000004
      #define MASK_BUSMON_DN_TSEL_WAN                             0x00000008
      #define MASK_BUSMON_DN_TSEL_HWIC                            0x00000010
      #define MASK_BUSMON_DN_TSEL_GLBL                            0x00000020
  /******************************************
  * busmon.dn_tsel_mask
  *  0: 0 ht
  *  1: 1 pci0
  *  2: 2 pci1
  *  3: 3 wan
  *  4: 4 hwic
  *  5: 5 glbl
  * 31: 6 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_TSEL_MASK_HT                       0
      #define OFFSET_BUSMON_DN_TSEL_MASK_PCI0                     1
      #define OFFSET_BUSMON_DN_TSEL_MASK_PCI1                     2
      #define OFFSET_BUSMON_DN_TSEL_MASK_WAN                      3
      #define OFFSET_BUSMON_DN_TSEL_MASK_HWIC                     4
      #define OFFSET_BUSMON_DN_TSEL_MASK_GLBL                     5
    /**** field masks **********************/
      #define MASK_BUSMON_DN_TSEL_MASK_HT                         0x00000001
      #define MASK_BUSMON_DN_TSEL_MASK_PCI0                       0x00000002
      #define MASK_BUSMON_DN_TSEL_MASK_PCI1                       0x00000004
      #define MASK_BUSMON_DN_TSEL_MASK_WAN                        0x00000008
      #define MASK_BUSMON_DN_TSEL_MASK_HWIC                       0x00000010
      #define MASK_BUSMON_DN_TSEL_MASK_GLBL                       0x00000020
  /******************************************
  * busmon.arbiter_grant
  *  0: 0 ht
  *  1: 1 pci0
  *  2: 2 pci1
  *  3: 3 wan
  *  4: 4 hwic
  *  5: 5 glbl
  *  6: 6 mux_farm
  * 31: 7 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_ARBITER_GRANT_HT                      0
      #define OFFSET_BUSMON_ARBITER_GRANT_PCI0                    1
      #define OFFSET_BUSMON_ARBITER_GRANT_PCI1                    2
      #define OFFSET_BUSMON_ARBITER_GRANT_WAN                     3
      #define OFFSET_BUSMON_ARBITER_GRANT_HWIC                    4
      #define OFFSET_BUSMON_ARBITER_GRANT_GLBL                    5
      #define OFFSET_BUSMON_ARBITER_GRANT_MUX_FARM                6
    /**** field masks **********************/
      #define MASK_BUSMON_ARBITER_GRANT_HT                        0x00000001
      #define MASK_BUSMON_ARBITER_GRANT_PCI0                      0x00000002
      #define MASK_BUSMON_ARBITER_GRANT_PCI1                      0x00000004
      #define MASK_BUSMON_ARBITER_GRANT_WAN                       0x00000008
      #define MASK_BUSMON_ARBITER_GRANT_HWIC                      0x00000010
      #define MASK_BUSMON_ARBITER_GRANT_GLBL                      0x00000020
      #define MASK_BUSMON_ARBITER_GRANT_MUX_FARM                  0x00000040
  /******************************************
  * busmon.arbiter_grant_mask
  *  0: 0 ht
  *  1: 1 pci0
  *  2: 2 pci1
  *  3: 3 wan
  *  4: 4 hwic
  *  5: 5 glbl
  *  6: 6 mux_farm
  * 31: 7 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_HT                 0
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_PCI0               1
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_PCI1               2
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_WAN                3
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_HWIC               4
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_GLBL               5
      #define OFFSET_BUSMON_ARBITER_GRANT_MASK_MUX_FARM           6
    /**** field masks **********************/
      #define MASK_BUSMON_ARBITER_GRANT_MASK_HT                   0x00000001
      #define MASK_BUSMON_ARBITER_GRANT_MASK_PCI0                 0x00000002
      #define MASK_BUSMON_ARBITER_GRANT_MASK_PCI1                 0x00000004
      #define MASK_BUSMON_ARBITER_GRANT_MASK_WAN                  0x00000008
      #define MASK_BUSMON_ARBITER_GRANT_MASK_HWIC                 0x00000010
      #define MASK_BUSMON_ARBITER_GRANT_MASK_GLBL                 0x00000020
      #define MASK_BUSMON_ARBITER_GRANT_MASK_MUX_FARM             0x00000040
  /******************************************
  * busmon.dn_bus_cycle
  *  3: 0 regs
  * 31: 4 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_BUS_CYCLE_REGS                     0
    /**** field masks **********************/
      #define MASK_BUSMON_DN_BUS_CYCLE_REGS                       0x0000000f
  /******************************************
  * busmon.dn_bus_cycle_mask
  *  3: 0 regs
  * 31: 4 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_BUS_CYCLE_MASK_REGS                0
    /**** field masks **********************/
      #define MASK_BUSMON_DN_BUS_CYCLE_MASK_REGS                  0x0000000f
  /******************************************
  * busmon.dn_opcode
  *  3: 0 regs
  * 31: 4 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_OPCODE_REGS                        0
    /**** field masks **********************/
      #define MASK_BUSMON_DN_OPCODE_REGS                          0x0000000f
  /******************************************
  * busmon.dn_opcode_mask
  *  3: 0 regs
  * 31: 4 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_OPCODE_MASK_REGS                   0
    /**** field masks **********************/
      #define MASK_BUSMON_DN_OPCODE_MASK_REGS                     0x0000000f
  /******************************************
  * busmon.dn_priority
  *  3: 0 regs
  * 31: 4 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_PRIORITY_REGS                      0
    /**** field masks **********************/
      #define MASK_BUSMON_DN_PRIORITY_REGS                        0x0000000f
  /******************************************
  * busmon.dn_priority_mask
  *  3: 0 regs
  * 31: 4 reserved1
  ******************************************/
    /**** field offsets ********************/
      #define OFFSET_BUSMON_DN_PRIORITY_MASK_REGS                 0
    /**** field masks **********************/
      #define MASK_BUSMON_DN_PRIORITY_MASK_REGS                   0x0000000f

#endif /* GOOFY_DBGBUS_H */

/******** History ******** 
$Log: goofy_dbgbus.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
