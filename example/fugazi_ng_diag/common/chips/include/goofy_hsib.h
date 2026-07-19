/* $Id: goofy_hsib.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/goofy_hsib.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's hsib registers
 *
 * May 2006, Bao Buu
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef GOOFY_HSIB_H
#define GOOFY_HSIB_H

#define GOOFY_HSIB_DN_SIZE      0x1000         /* 4K */
#define GOOFY_MAX_LUTS_PER_DN   4

/*
 * Goofy asic device node register set data structure.
 * Each device node has 4K address space in the HSIB config address
 * space.
 */

typedef struct goofy_hsib_dn_
{
    volatile uint32_t dn_control;                     /* 0x00 */
    volatile uint32_t dn_initiator_thres;
    volatile uint32_t dn_target_thres;
    volatile uint32_t dn_status;
    volatile uint32_t dn_initiator_err;               /* 0x10 */
    volatile uint32_t dn_target_err;
    volatile uint32_t dn_initiator_err_mask;
    volatile uint32_t dn_target_err_mask;             
    volatile uint32_t dn_id;                          /* 0x20 */
    volatile uint32_t dn_target_online;
    volatile uint32_t dn_1st_err_hsib_header;
    volatile uint32_t dn_1st_err_addr;
    volatile uint32_t dn_2nd_err_hsib_header;         /* 0x30 */
    volatile uint32_t dn_2nd_err_addr;                /* 0x34 */
    uint32_t reserved[0x6];                           /* 0x38-0x4f */
    volatile uint32_t dn_aux_ctrl;                    /* 0x50 */
    volatile uint32_t dn_timeout_ctrl;
    volatile uint32_t dn_timer;
    volatile uint32_t hsib_no_lut_priority;
    volatile uint32_t lut_ctrl_0;                     /* 0x60 */
    volatile uint32_t lut_base_limit_0;
    volatile uint32_t lut_ctrl_1;
    volatile uint32_t lut_base_limit_1;
    volatile uint32_t lut_ctrl_2;                     /* 0x70 */
    volatile uint32_t lut_base_limit_2;
    volatile uint32_t lut_ctrl_3;
    volatile uint32_t lut_base_limit_3;
    volatile uint32_t pcie_ds1_mem_limit_shad;        /* 0x80 */
    volatile uint32_t pcie_ds2_mem_limit_shad;
    volatile uint32_t pcie_ds3_mem_limit_shad;
    volatile uint32_t pcie_ds4_mem_limit_shad;
    volatile uint32_t pcie_ds1_pf_mem_limit_shad;     /* 0x90 */
    volatile uint32_t pcie_ds2_pf_mem_limit_shad;
    volatile uint32_t pcie_ds3_pf_mem_limit_shad;
    volatile uint32_t pcie_ds4_pf_mem_limit_shad;
    volatile uint32_t pcie_ds1_bus_num_shad;          /* 0xa0 */
    volatile uint32_t pcie_ds2_bus_num_shad;
    volatile uint32_t pcie_ds3_bus_num_shad;
    volatile uint32_t pcie_ds4_bus_num_shad;
    volatile uint32_t pcie_ds1_priority;              /* 0xb0 */
    volatile uint32_t pcie_ds2_priority;
    volatile uint32_t pcie_ds3_priority;
    volatile uint32_t pcie_ds4_priority;
    volatile uint32_t pcie_us0_bus_num_shad;          /* 0xc0 */
    volatile uint32_t dn_initiator_err_inject;
    volatile uint32_t dn_target_err_inject;
    uint32_t reserved2[0x1cd];                        /* 0x0cc-0x7ff */
    uint32_t reserved3[0x200];                        /* 0x800-0xfff */
} goofy_hsib_dn_t;

/*
 * Goofy HSIB Error monitor interrupt test uses this
 * data structure to keep the interrupt information
 */
typedef struct goofy_hsib_err_intr_result {
    uint8_t intr_count;
    uint8_t dn;
    uint8_t hsib_err_intr_num;
    uint32_t hsib_err_event_reg;
} goofy_hsib_err_intr_result_t;

/*****************************************
 March 07, 2005 08:57:58 AM
 prototypes
*****************************************/
int goofy_init_wan(void);
int goofy_init_hwic(void);
int goofy_init_hsib(void);

/*
 * HSIB device node mandatory config space register address offset
 */
#define DN_CONTROL                0x000
#define DN_INITIATOR_THRES        0x004
#define DN_TARGET_THRES           0x008
#define DN_STATUS                 0x00c
#define DN_INITIATOR_ERR          0x010
#define DN_TARGET_ERR             0x014
#define DN_INITIATOR_ERR_MASK     0x018
#define DN_TARGET_ERR_MASK        0x01c
#define DN_ID                     0x020
#define DN_TARGET_ONLINE          0x024
#define DN_1ST_ERR_HEADER         0x028
#define DN_1ST_ERR_ADDR           0x02c
#define DN_2ND_ERR_HEADER         0x030
#define DN_2ND_ERR_ADDR           0x034

/*
 * HSIB device node Goofy specific config space register address offset
 */
#define DN_AUX_CONTROL                           0x050
#define DN_TIMEOUT_CONTROL                       0x054
#define DN_TIMER                                 0x058
#define HSIB_NO_LUT_PRIORITY                     0x05c
#define LUT_CONTROL_0                            0x060
#define LUT_BASE_LIMIT_0                         0x064
#define LUT_CONTROL_1                            0x068
#define LUT_BASE_LIMIT_1                         0x06c
#define LUT_CONTROL_2                            0x070
#define LUT_BASE_LIMIT_2                         0x074
#define LUT_CONTROL_3                            0x078
#define LUT_BASE_LIMIT_3                         0x07c
#define PCIE_DS1_MEM_BASE_LIMIT_SHADOW        0x080
#define PCIE_DS2_MEM_BASE_LIMIT_SHADOW        0x084
#define PCIE_DS3_MEM_BASE_LIMIT_SHADOW        0x088
#define PCIE_DS4_MEM_BASE_LIMIT_SHADOW        0x08c
#define PCIE_DS1_PF_MEM_BASE_LIMIT_SHADOW     0x090
#define PCIE_DS2_PF_MEM_BASE_LIMIT_SHADOW     0x094
#define PCIE_DS3_PF_MEM_BASE_LIMIT_SHADOW     0x098
#define PCIE_DS4_PF_MEM_BASE_LIMIT_SHADOW     0x09c
#define PCIE_DS1_BUS_NUM_SHADOW              0x0a0
#define PCIE_DS2_BUS_NUM_SHADOW              0x0a4
#define PCIE_DS3_BUS_NUM_SHADOW              0x0a8
#define PCIE_DS4_BUS_NUM_SHADOW              0x0ac
#define PCIE_DS1_PRIORITY                        0x0b0
#define PCIE_DS2_PRIORITY                        0x0b4
#define PCIE_DS3_PRIORITY                        0x0b8
#define PCIE_DS4_PRIORITY                        0x0bc
#define PCIE_US0_BUS_NUM_SHADOW              0x0c0
#define DN_INITIATOR_ERR_INJECT                0x0c4
#define DN_TARGET_ERR_INJECT                   0x0c8

/*
 * HSIB config space address bit defines
 */
#define MASK_HSIB_CONFIG_ADDR_DN_REG_OFST       0x00000fff
#define MASK_HSIB_CONFIG_ADDR_DN_REG_OFST_SHFT  0
#define MASK_HSIB_CONFIG_ADDR_DN_NUM            0x00007000
#define MASK_HSIB_CONFIG_ADDR_DN_NUM_SHFT       12

/*
 * DN_CONTROL bit defines
 */
#define MASK_DN_CONTROL_EN_INITIATOR                0x00000001
#define MASK_DN_CONTROL_EN_TARGET                   0x00000002

/*
 * DN_INITIATOR_THRESHOLDS bit defines
 */
#define MASK_DN_INI_THRES_POSTED_MAXCOUNT     0x0000000f
#define MASK_DN_INI_THRES_NONPOSTED_MAXCOUNT  0x00000f00
#define MASK_DN_INI_THRES_RESP_MAXCOUNT       0x000f0000

/*
 * DN_TARGET_THRESHOLDS bit defines
 */
#define MASK_DN_TGT_THRES_POSTED_MAXCOUNT     0x0000000f
#define MASK_DN_TGT_THRES_NONPOSTED_MAXCOUNT  0x00000f00
#define MASK_DN_TGT_THRES_RESP_MAXCOUNT       0x000f0000

/*
 * DN_INITIATOR_ERROR bit defines
 * DN_INITIATOR_ERROR_MASK use the same bit defines
 * Note:
 * HSIBErrorInterruptEnableRegister_dn_module_High and
 * HSIBErrorEventRegister_dn_module_High in the interrupt controller
 * use the same bit defines
 */
#define OFFSET_DN_INI_ERR_HIST_OVERFLOW            0
#define OFFSET_DN_INI_ERR_LUT_MULT_MATCH           1
#define OFFSET_DN_INI_ERR_LUT_NO_MATCH             2
#define OFFSET_DN_INI_ERR_MALFORM_CFG_REQ          3
#define OFFSET_DN_INI_ERR_MALFORM_REQ              4
#define OFFSET_DN_INI_ERR_ARB_REQ_PAR              5
#define OFFSET_DN_INI_ERR_UNEXP_INI_RESP           6
#define OFFSET_DN_INI_ERR_HSIB_TIMEOUT             7
#define OFFSET_DN_INI_ERR_DEST_OFFLINE             8
#define OFFSET_DN_INI_ERR_LUT_MULT_TGT             9
#define OFFSET_DN_INI_ERR_LUT_RD_PROTECT           10
#define OFFSET_DN_INI_ERR_LUT_POSTED_WR_PROTECT    11
#define OFFSET_DN_INI_ERR_LUT_NONPOSTED_WR_PROTECT 12
#define OFFSET_DN_INI_ERR_REQ_DATA_PAR             13
#define OFFSET_DN_INI_ERR_REQ_DATA_BUF_FULL        14
#define OFFSET_DN_INI_ERR_REQ_CMD_BUF_FULL         15
#define OFFSET_DN_INI_ERR_REQ_DATA_SIZE            16
#define OFFSET_DN_INI_ERR_RESP_DATA_PAR            17
#define OFFSET_DN_INI_ERR_RESP_DATA_FULL           18
#define OFFSET_DN_INI_ERR_REQ_SRCTAG_FULL          19
#define OFFSET_DN_INI_ERR_REQ_POISON_TLP           20
#define OFFSET_DN_INI_ERR_REQ_RX_ERR_HDR           21
#define OFFSET_DN_INI_ERR_REQ_RX_ERR_DATA          22
#define OFFSET_DN_INI_ERR_RESP_UNCORRECTABLE       23

#define MASK_DN_INI_ERR_HIST_OVERFLOW      0x00000001
#define MASK_DN_INI_ERR_LUT_MULT_MATCH     0x00000002
#define MASK_DN_INI_ERR_LUT_NO_MATCH       0x00000004
#define MASK_DN_INI_ERR_MALFORM_CFG_REQ    0x00000008
#define MASK_DN_INI_ERR_MALFORM_REQ        0x00000010
#define MASK_DN_INI_ERR_ARB_REQ_PAR        0x00000020
#define MASK_DN_INI_ERR_UNEXP_INI_RESP     0x00000040
#define MASK_DN_INI_ERR_HSIB_TIMEOUT       0x00000080
#define MASK_DN_INI_ERR_DEST_OFFLINE       0x00000100
#define MASK_DN_INI_ERR_LUT_MULT_TGT       0x00000200
#define MASK_DN_INI_ERR_LUT_RD_PROTECT           0x00000400
#define MASK_DN_INI_ERR_LUT_POSTED_WR_PROTECT    0x00000800
#define MASK_DN_INI_ERR_LUT_NONPOSTED_WR_PROTECT 0x00001000
#define MASK_DN_INI_ERR_REQ_DATA_PAR             0x00002000
#define MASK_DN_INI_ERR_REQ_DATA_BUF_FULL        0x00004000
#define MASK_DN_INI_ERR_REQ_CMD_BUF_FULL         0x00008000
#define MASK_DN_INI_ERR_REQ_DATA_SIZE            0x00010000
#define MASK_DN_INI_ERR_RESP_DATA_PAR            0x00020000
#define MASK_DN_INI_ERR_RESP_DATA_FULL           0x00040000
#define MASK_DN_INI_ERR_REQ_SRCTAG_FULL          0x00080000
#define MASK_DN_INI_ERR_REQ_POISON_TLP           0x00100000
#define MASK_DN_INI_ERR_REQ_RX_ERR_HDR           0x00200000
#define MASK_DN_INI_ERR_REQ_RX_ERR_DATA          0x00400000
#define MASK_DN_INI_ERR_RESP_UNCORRECTABLE       0x00800000
#define MASK_DN_INI_ERR_ALL                      0x00ffffff

/*
 * DN_TARGET_ERROR bit defines
 * DN_TARGET_ERROR_MASK use the same bit defines
 * Note:
 * HSIBErrorInterruptEnableRegister_dn_module_Low and
 * HSIBErrorEventRegister_dn_module_Low in the interrupt controller
 * use the same bit defines
 */
#define OFFSET_DN_TGT_ERR_UNEXP_TGT_RESP             1
#define OFFSET_DN_TGT_ERR_HSIB_PAR                   2
#define OFFSET_DN_TGT_ERR_ARB_REQ_PAR                3
#define OFFSET_DN_TGT_ERR_IFACE_TIMEOUT              4
#define OFFSET_DN_TGT_ERR_DN_QUIESCED                5
#define OFFSET_DN_TGT_ERR_REQ_DATA_PAR               6
#define OFFSET_DN_TGT_ERR_REQ_DATA_BUF_FULL          7
#define OFFSET_DN_TGT_ERR_REQ_CMD_BUF_FULL           8
#define OFFSET_DN_TGT_ERR_RESP_CMD_BUF_FULL          9
#define OFFSET_DN_TGT_ERR_RESP_DATA_BUF_FULL         10
#define OFFSET_DN_TGT_ERR_RESP_DATA_PAR              11
#define OFFSET_DN_TGT_ERR_REQ_BAD_OPCODE             12
#define OFFSET_DN_TGT_ERR_REQ_SRCTAG_FULL            13
#define OFFSET_DN_TGT_ERR_RESP_POISON_TLP            14
#define OFFSET_DN_TGT_ERR_RESP_RX_ERR_HDR            15
#define OFFSET_DN_TGT_ERR_RESP_RX_ERR_DATA           16
#define OFFSET_DN_TGT_ERR_REQ_UNCORRECTABLE          17
#define OFFSET_DN_TGT_ERR_HSIB_HDR_PAR_ERR           18
#define OFFSET_DN_TGT_ERR_HSIB_CONG_WR_DATA_UNCORR   19
#define OFFSET_DN_TGT_ERR_HSIB_CONG_WR_DATA_PAR_ERR  20

#define MASK_DN_TGT_ERR_UNEXP_TGT_RESP                0x00000002
#define MASK_DN_TGT_ERR_HSIB_PAR                      0x00000004
#define MASK_DN_TGT_ERR_ARB_REQ_PAR                   0x00000008
#define MASK_DN_TGT_ERR_IFACE_TIMEOUT                 0x00000010
#define MASK_DN_TGT_ERR_DN_QUIESCED                   0x00000020
#define MASK_DN_TGT_ERR_REQ_DATA_PAR                  0x00000040
#define MASK_DN_TGT_ERR_REQ_DATA_BUF_FULL             0x00000080
#define MASK_DN_TGT_ERR_REQ_CMD_BUF_FULL              0x00000100
#define MASK_DN_TGT_ERR_RESP_CMD_BUF_FULL             0x00000200
#define MASK_DN_TGT_ERR_RESP_DATA_BUF_FULL            0x00000400
#define MASK_DN_TGT_ERR_RESP_DATA_PAR                 0x00000800
#define MASK_DN_TGT_ERR_REQ_BAD_OPCODE                0x00001000
#define MASK_DN_TGT_ERR_REQ_SRCTAG_FULL               0x00002000
#define MASK_DN_TGT_ERR_RESP_POISON_TLP               0x00004000
#define MASK_DN_TGT_ERR_RESP_RX_ERR_HDR               0x00008000
#define MASK_DN_TGT_ERR_RESP_RX_ERR_DATA              0x00010000
#define MASK_DN_TGT_ERR_REQ_UNCORRECTABLE             0x00020000
#define MASK_DN_TGT_ERR_HSIB_HDR_PAR_ERR              0x00040000
#define MASK_DN_TGT_ERR_HSIB_CONG_WR_DATA_UNCORR      0x00080000
#define MASK_DN_TGT_ERR_HSIB_CONG_WR_DATA_PAR_ERR     0x00100000
#define MASK_DN_TGT_ERR_ALL                           0x001FFFFF

/*
 * DEVICE_NODE_ID bit defines
 */
#define MASK_DN_ID             0x0000000f
#define MASK_DN_ID_BRIDGE      0x00000010

/*
 * DN_TARGET_ONLINE bit defines
 */
#define MASK_DN_TGT_ONLINE            0x0000ffff
#define MASK_DN_TGT_ONLINE_PCIE_US    0x00000001
#define MASK_DN_TGT_ONLINE_PCIE_DS1   0x00000002
#define MASK_DN_TGT_ONLINE_PCIE_DS2   0x00000004
#define MASK_DN_TGT_ONLINE_PCIE_DS3   0x00000008
#define MASK_DN_TGT_ONLINE_PCIE_DS4   0x00000010
#define MASK_DN_TGT_ONLINE_GLBL       0x00000020
#define MASK_DN_TGT_ONLINE_HWIC       0x00000040
#define MASK_DN_TGT_ONLINE_WAN        0x00000080

/*
 * HSIB_NO_LUT_PRIORITY bit defines
 */
#define MASK_HSIB_NO_LUT_PRIORITY_SWAPB             0x00000100

/*
 * LUT control bit defines
 */
#define MASK_LUT_CTRL_DN_US                         0x00000001
#define MASK_LUT_CTRL_DN_DS1                        0x00000002
#define MASK_LUT_CTRL_DN_DS2                        0x00000004
#define MASK_LUT_CTRL_DN_DS3                        0x00000008
#define MASK_LUT_CTRL_DN_DS4                        0x00000010
#define MASK_LUT_CTRL_DN_GLBL                       0x00000020
#define MASK_LUT_CTRL_DN_HWIC                       0x00000040
#define MASK_LUT_CTRL_DN_WAN                        0x00000080
#define MASK_LUT_CTRL_PRIORITY                      0x000f0000
#define MASK_LUT_CTRL_PRIORITY_SHFT                 16
#define MASK_LUT_CTRL_NPWP                          0x00100000
#define MASK_LUT_CTRL_PWP                           0x00200000
#define MASK_LUT_CTRL_RP                            0x00400000
#define MASK_LUT_CTRL_SWAPB                         0x08000000
#define MASK_LUT_CTRL_VALID                         0x80000000

/*
 * LUT base limit bit defines
 */
#define MASK_LUT_BASE_LMT_BASE                      0x0000ffff
#define MASK_LUT_BASE_LMT_LMT                       0xffff0000
#define LUT_BASE_LMT_SHFT                           16

/*
 * DN_ERR_INITIATOR_INJECT bit defines
 * DN_ERR_TARGET_INJECT bit defines
 */
#define MASK_DN_ERR_INJECT_ARB_REQ_PAR        0x00000001
#define MASK_DN_ERR_INJECT_REQ_DATA_LOW_PAR   0x00000002
#define MASK_DN_ERR_INJECT_REQ_DATA_HI_PAR    0x00000004
#define MASK_DN_ERR_INJECT_RESP_DATA_LOW_PAR  0x00000008
#define MASK_DN_ERR_INJECT_RESP_DATA_HI_PAR   0x00000010

extern int goofy_hsib_reg_test (dev_object_t *dev);
extern void goofy_hsib_display_err_stat (dev_object_t *dev, int, boolean);
extern void goofy_hsib_reg_display (dev_object_t *dev, uint32_t dn);
extern void gfy_hsib_attach (dev_object_t *dev);

#endif /* GOOFY_HSIB_H */


/******** History ******** 
$Log: goofy_hsib.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
