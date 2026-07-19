/* $Id: goofy_intr.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/goofy_intr.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's interrupt registers
 *
 * May 2006, Bao Buu
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef GOOFY_INTR_H

/*****************************************
 constants
*****************************************/
#define GOOFY_INTR_H
#define GOOFY_MAX_INTR_PREFETCH             12
#define GOOFY_MAX_INTR_DELAY                18
#define GOOFY_INTR_DISABLE_ALL_PREFETCH     0xffffffff

#ifdef LINUX_KLM
#define PFI long
#define ASIC_MAX_LEVEL_INTR  32
#endif

/*
 * Enum
 */
typedef enum
{
    WIC_INTR = 0,
    HDLC_INTR,
    PKTPUMP_INTR,
    SCC_INTR,
    TDM_INTR,
    WDOG_INTR,
    GPIO_INTR,
    GBL_REG_INTR,
    I2C_INTR,
    HSIB_ERR_MON_INTR,
    HSIB_BUS_MON_TRIG_INTR,
    PCIE_ERR_INTR,
    SGPIO_INTR,
    MAX_CLASS_INTR,            /* use to disable all intr */
} goofy_intr_class_t;

/*
 * Goofy top level interrupt type.
 * Do not use for GPIO and SGPIO interrupt setting.
 * They have their own enum in goofy_gpio.h
 */
typedef enum {
    NET_INTR = 0,
    MAN_INTR,
    ERR_INTR,
    MAX_INTR_TYPE,
} goofy_intr_type_enum;
#define GPIO_INTR_TYPE_DISABLE   0x3

typedef enum {
    MSI_1_MSG = 0,
    MSI_2_MSG,
    MSI_4_MSG,
    MSI_8_MSG,
    MSI_16_MSG,
    MSI_32_MSG,
} msi_multi_msg_enum;

typedef enum {
    GOOFY_CPU_1 =0,
    GOOFY_CPU_2,
    GOOFY_REDIR,
    MAX_GOOFY_CPU,
} goofy_cpu_intr_enum;

typedef struct intr_data_ {
    PFI    isr;               /* interrupt service routines */
    int   isr_index;          /* ..used in  by klm */
    int   time_out;           /* ..used by klm */
    ulong address;
    ulong param1;
    ulong param2;
    ulong param3;
    uint32_t intr_count;
} intr_data_t;

typedef struct goofy_cpu_intr_en
{
    volatile uint32_t net_intr;
    uint32_t reserved;
    volatile uint32_t man_intr;
    volatile uint32_t err_intr;
} goofy_cpu_intr_en_t;

/*
 * PCIe MSI capability registers
 */
typedef struct pcie_msi_cap
{
    volatile uint32_t pcie_msi_cap0;
    volatile uint32_t pcie_msi_cap1;
    volatile uint32_t pcie_msi_cap2;
    volatile uint32_t pcie_msi_cap3;
    volatile uint32_t pcie_msi_cap4;
} pcie_msi_cap_t;

typedef struct goofy_msi_cap
{
    pcie_msi_cap_t intr_type[3];
} goofy_msi_cap_t;

typedef struct goofy_intr_
{
    goofy_cpu_intr_en_t intr_en[3];
    volatile uint32_t net_intr_stat;
    uint32_t reserved1;
    volatile uint32_t man_intr_stat;
    volatile uint32_t err_intr_stat;
    volatile uint32_t msi_intr_relax_or_en;
    uint32_t reserved4;
    volatile uint32_t hsib_err_intr_en;
    volatile uint32_t hsib_err_intr_stat;
    volatile uint32_t hsib_err_intr_en_pcie_0_l;
    volatile uint32_t hsib_err_intr_en_pcie_0_h;
    volatile uint32_t hsib_err_intr_en_pcie_1_l;
    volatile uint32_t hsib_err_intr_en_pcie_1_h;
    volatile uint32_t hsib_err_intr_en_pcie_2_l;
    volatile uint32_t hsib_err_intr_en_pcie_2_h;
    volatile uint32_t hsib_err_intr_en_pcie_3_l;
    volatile uint32_t hsib_err_intr_en_pcie_3_h;
    volatile uint32_t hsib_err_intr_en_pcie_4_l;
    volatile uint32_t hsib_err_intr_en_pcie_4_h;
    volatile uint32_t hsib_err_intr_en_glb_l;
    volatile uint32_t hsib_err_intr_en_glb_h;
    volatile uint32_t hsib_err_intr_en_wic_l;
    volatile uint32_t hsib_err_intr_en_wic_h;
    volatile uint32_t hsib_err_intr_en_wan_l;
    volatile uint32_t hsib_err_intr_en_wan_h;
    volatile uint32_t hsib_err_event_pcie_0_l;
    volatile uint32_t hsib_err_event_pcie_0_h;
    volatile uint32_t hsib_err_event_pcie_1_l;
    volatile uint32_t hsib_err_event_pcie_1_h;
    volatile uint32_t hsib_err_event_pcie_2_l;
    volatile uint32_t hsib_err_event_pcie_2_h;
    volatile uint32_t hsib_err_event_pcie_3_l;
    volatile uint32_t hsib_err_event_pcie_3_h;
    volatile uint32_t hsib_err_event_pcie_4_l;
    volatile uint32_t hsib_err_event_pcie_4_h;
    volatile uint32_t hsib_err_event_glb_l;
    volatile uint32_t hsib_err_event_glb_h;
    volatile uint32_t hsib_err_event_wic_l;
    volatile uint32_t hsib_err_event_wic_h;
    volatile uint32_t hsib_err_event_wan_l;
    volatile uint32_t hsib_err_event_wan_h;
    volatile uint32_t i2c_master_intr_en;
    volatile uint32_t i2c_master_intr_stat;
    volatile uint32_t pcie_err_intr_en;
    volatile uint32_t pcie_err_intr_event;
    volatile uint32_t intr_delay_0;
    volatile uint32_t intr_delay_1;
    volatile uint32_t intr_delay_2;
    volatile uint32_t intr_delay_3;
    volatile uint32_t intr_delay_4;
    volatile uint32_t intr_delay_5;
    volatile uint32_t intr_delay_6;
    volatile uint32_t intr_delay_7;
    volatile uint32_t intr_delay_8;
    volatile uint32_t intr_delay_9;
    volatile uint32_t intr_delay_10;
    volatile uint32_t intr_delay_11;
    volatile uint32_t intr_delay_12;
    volatile uint32_t intr_delay_13;
    volatile uint32_t intr_delay_14;
    volatile uint32_t intr_delay_15;
    volatile uint32_t intr_delay_16;
    volatile uint32_t intr_delay_17;
    uint32_t reserved5[0x6];
    goofy_msi_cap_t cpu_msi_cap[3];
    uint32_t reserved6[3];
} goofy_intr_t;

/*
 * Bit positions of each interrupt in the network interrupt enable
 * register
 */
#define OFFSET_NET_INTR_EN_HDLC0                    0
#define OFFSET_NET_INTR_EN_HDLC1                    1
#define OFFSET_NET_INTR_EN_HDLC2                    2
#define OFFSET_NET_INTR_EN_HDLC3                    3
#define OFFSET_NET_INTR_EN_HDLC4                    4
#define OFFSET_NET_INTR_EN_HDLC5                    5
#define OFFSET_NET_INTR_EN_HDLC6                    6
#define OFFSET_NET_INTR_EN_HDLC7                    7
#define OFFSET_NET_INTR_EN_HWIC0                    8
#define OFFSET_NET_INTR_EN_HWIC1                    9
#define OFFSET_NET_INTR_EN_HWIC2                    10
#define OFFSET_NET_INTR_EN_HWIC3                    11
#define OFFSET_NET_INTR_EN_GPIO0                    12
#define OFFSET_NET_INTR_EN_GPIO1                    13
#define OFFSET_NET_INTR_EN_GPIO2                    14
#define OFFSET_NET_INTR_EN_GPIO3                    15
#define OFFSET_NET_INTR_EN_GPIO4                    16
#define OFFSET_NET_INTR_EN_GPIO5                    17
#define OFFSET_NET_INTR_EN_GPIO6                    18
#define OFFSET_NET_INTR_EN_GPIO7                    19
#define OFFSET_NET_INTR_EN_GPIO8                    20
#define OFFSET_NET_INTR_EN_GPIO9                    21
#define OFFSET_NET_INTR_EN_GPIO10                   22
#define OFFSET_NET_INTR_EN_GPIO11                   23
#define OFFSET_NET_INTR_EN_PKTPUMP                  26
#define OFFSET_NET_INTR_EN_I2C                      27
#define OFFSET_NET_INTR_EN_SCC                      28
#define OFFSET_NET_INTR_EN_GPIO                     30
#define OFFSET_NET_INTR_EN_SGPIO                    31

/*
 * Bit mask of each interrupt in the network interrupt enable
 * register
 * Network interrupt status register shares the same bit defines
 */
#define MASK_NET_INTR_HDLC0                      0x00000001
#define MASK_NET_INTR_HDLC1                      0x00000002
#define MASK_NET_INTR_HDLC2                      0x00000004
#define MASK_NET_INTR_HDLC3                      0x00000008
#define MASK_NET_INTR_HDLC4                      0x00000010
#define MASK_NET_INTR_HDLC5                      0x00000020
#define MASK_NET_INTR_HDLC6                      0x00000040
#define MASK_NET_INTR_HDLC7                      0x00000080
#define MASK_NET_INTR_HWIC0                      0x00000100
#define MASK_NET_INTR_HWIC1                      0x00000200
#define MASK_NET_INTR_HWIC2                      0x00000400
#define MASK_NET_INTR_HWIC3                      0x00000800
#define MASK_NET_INTR_GPIO0                      0x00001000
#define MASK_NET_INTR_GPIO1                      0x00002000
#define MASK_NET_INTR_GPIO2                      0x00004000
#define MASK_NET_INTR_GPIO3                      0x00008000
#define MASK_NET_INTR_GPIO4                      0x00010000
#define MASK_NET_INTR_GPIO5                      0x00020000
#define MASK_NET_INTR_GPIO6                      0x00040000
#define MASK_NET_INTR_GPIO7                      0x00080000
#define MASK_NET_INTR_GPIO8                      0x00100000
#define MASK_NET_INTR_GPIO9                      0x00200000
#define MASK_NET_INTR_GPIO10                     0x00400000
#define MASK_NET_INTR_GPIO11                     0x00800000
#define MASK_NET_INTR_GPIO0_11                   0x00fff000
#define MASK_NET_INTR_PKTPUMP                    0x04000000
#define MASK_NET_INTR_I2C                        0x08000000
#define MASK_NET_INTR_SCC                        0x10000000
#define MASK_NET_INTR_GPIO                       0x40000000
#define MASK_NET_INTR_SGPIO                      0x80000000

/*
 * Bit positions of each interrupt in the management interrupt enable
 * register
 */
#define OFFSET_MAN_INTR_EN_HWIC0                       8
#define OFFSET_MAN_INTR_EN_HWIC1                       9
#define OFFSET_MAN_INTR_EN_HWIC2                       10
#define OFFSET_MAN_INTR_EN_HWIC3                       11
#define OFFSET_MAN_INTR_EN_GPIO12                      12
#define OFFSET_MAN_INTR_EN_GPIO13                      13
#define OFFSET_MAN_INTR_EN_GPIO14                      16
#define OFFSET_MAN_INTR_EN_GPIO15                      17
#define OFFSET_MAN_INTR_EN_HSIB_BUS_MON_TGR_DONE       24
#define OFFSET_MAN_INTR_EN_SCC                         28
#define OFFSET_MAN_INTR_EN_TDM                         29
#define OFFSET_MAN_INTR_EN_GPIO                        30
#define OFFSET_MAN_INTR_EN_SGPIO                       31

/*
 * Bit mask of each interrupt in the management interrupt enable
 * register
 * Management interrupt status register shares the same bit defines
 */
#define MASK_MAN_INTR_HWIC0                         0x00000100
#define MASK_MAN_INTR_HWIC1                         0x00000200
#define MASK_MAN_INTR_HWIC2                         0x00000400
#define MASK_MAN_INTR_HWIC3                         0x00000800
#define MASK_MAN_INTR_GPIO12                        0x00001000
#define MASK_MAN_INTR_GPIO13                        0x00002000
#define MASK_MAN_INTR_GPIO14                        0x00010000
#define MASK_MAN_INTR_GPIO15                        0x00020000
#define MASK_MAN_INTR_GPIO12_15                     0x00033000
#define MASK_MAN_INTR_HSIB_BUS_MON_TGR_DONE         0x01000000
#define MASK_MAN_INTR_SCC                           0x10000000
#define MASK_MAN_INTR_TDM                           0x20000000
#define MASK_MAN_INTR_GPIO                          0x40000000
#define MASK_MAN_INTR_SGPIO                         0x80000000

/*
 * Bit positions of each interrupt in the error interrupt enable
 * register
 */
#define OFFSET_ERR_INTR_EN_HDLC0                        0
#define OFFSET_ERR_INTR_EN_HDLC1                        1
#define OFFSET_ERR_INTR_EN_HDLC2                        2
#define OFFSET_ERR_INTR_EN_HDLC3                        3
#define OFFSET_ERR_INTR_EN_HDLC4                        4
#define OFFSET_ERR_INTR_EN_HDLC5                        5
#define OFFSET_ERR_INTR_EN_HDLC6                        6
#define OFFSET_ERR_INTR_EN_HDLC7                        7
#define OFFSET_ERR_INTR_EN_HWIC0_ERR                    8
#define OFFSET_ERR_INTR_EN_HWIC1_ERR                    9
#define OFFSET_ERR_INTR_EN_HWIC2_ERR                    10
#define OFFSET_ERR_INTR_EN_HWIC3_ERR                    11
#define OFFSET_ERR_INTR_EN_GPIO16                       12
#define OFFSET_ERR_INTR_EN_GPIO17                       13
#define OFFSET_ERR_INTR_EN_GPIO18                       14
#define OFFSET_ERR_INTR_EN_GPIO19                       15
#define OFFSET_ERR_INTR_EN_HWIC0_MAN                    16
#define OFFSET_ERR_INTR_EN_HWIC1_MAN                    17
#define OFFSET_ERR_INTR_EN_HWIC2_MAN                    18
#define OFFSET_ERR_INTR_EN_HWIC3_MAN                    19
#define OFFSET_ERR_INTR_EN_HSIB_ERR_MON                 20
#define OFFSET_ERR_INTR_EN_PCIE                         21
#define OFFSET_ERR_INTR_EN_WDOG_TIMER0                  22
#define OFFSET_ERR_INTR_EN_WDOG_TIMER1                  23
#define OFFSET_ERR_INTR_EN_HSIB_BUS_MON_TGR_DONE        24
#define OFFSET_ERR_INTR_EN_GBL_REG                      25
#define OFFSET_ERR_INTR_EN_PKTPUMP                      26
#define OFFSET_ERR_INTR_EN_TDM_MAN                      27
#define OFFSET_ERR_INTR_EN_SCC_MAN                      28
#define OFFSET_ERR_INTR_EN_TDM_ERR                      29
#define OFFSET_ERR_INTR_EN_GPIO                         30
#define OFFSET_ERR_INTR_EN_SGPIO                        31

/*
 * Bit mask of each interrupt in the error interrupt enable
 * register
 * Error interrupt status register shares the same bit defines
 */
#define MASK_ERR_INTR_HDLC0                          0x00000001
#define MASK_ERR_INTR_HDLC1                          0x00000002
#define MASK_ERR_INTR_HDLC2                          0x00000004
#define MASK_ERR_INTR_HDLC3                          0x00000008
#define MASK_ERR_INTR_HDLC4                          0x00000010
#define MASK_ERR_INTR_HDLC5                          0x00000020
#define MASK_ERR_INTR_HDLC6                          0x00000040
#define MASK_ERR_INTR_HDLC7                          0x00000080
#define MASK_ERR_INTR_HWIC0_ERR                      0x00000100
#define MASK_ERR_INTR_HWIC1_ERR                      0x00000200
#define MASK_ERR_INTR_HWIC2_ERR                      0x00000400
#define MASK_ERR_INTR_HWIC3_ERR                      0x00000800
#define MASK_ERR_INTR_GPIO16                         0x00001000
#define MASK_ERR_INTR_GPIO17                         0x00002000
#define MASK_ERR_INTR_GPIO18                         0x00004000
#define MASK_ERR_INTR_GPIO19                         0x00008000
#define MASK_ERR_INTR_GPIO16_19                      0x0000f000
#define MASK_ERR_INTR_HWIC0_MAN                      0x00010000
#define MASK_ERR_INTR_HWIC1_MAN                      0x00020000
#define MASK_ERR_INTR_HWIC2_MAN                      0x00040000
#define MASK_ERR_INTR_HWIC3_MAN                      0x00080000
#define MASK_ERR_INTR_HSIB_ERR_MON                   0x00100000
#define MASK_ERR_INTR_PCIE                           0x00200000
#define MASK_ERR_INTR_WDOG_TIMER0                    0x00400000
#define MASK_ERR_INTR_WDOG_TIMER1                    0x00800000
#define MASK_ERR_INTR_HSIB_BUS_MON_TGR_DONE          0x01000000
#define MASK_ERR_INTR_GLOBAL                         0x02000000
#define MASK_ERR_INTR_PKTPUMP                        0x04000000
#define MASK_ERR_INTR_TDM_MAN                        0x08000000
#define MASK_ERR_INTR_SCC_MAN                        0x10000000
#define MASK_ERR_INTR_TDM_ERR                        0x20000000
#define MASK_ERR_INTR_GPIO                           0x40000000
#define MASK_ERR_INTR_SGPIO                          0x80000000

/*
 * Bit offset and bit mask for hsib error interrupt enable register
 * HSIB error inerrupt status regiater share the bit defines
 * Note:
 * The _H and _L bits corresponds to HSIB initiator and target
 * errors respectively.
 */
#define OFFSET_HSIB_ERR_INTR_EN_WAN_DN_H          15
#define OFFSET_HSIB_ERR_INTR_EN_WAN_DN_L          14
#define OFFSET_HSIB_ERR_INTR_EN_HWIC_DN_H         13
#define OFFSET_HSIB_ERR_INTR_EN_HWIC_DN_L         12
#define OFFSET_HSIB_ERR_INTR_EN_GLB_REG_DN_H      11
#define OFFSET_HSIB_ERR_INTR_EN_GLB_REG_DN_L      10
#define OFFSET_HSIB_ERR_INTR_EN_PCIE4_DN_H         9
#define OFFSET_HSIB_ERR_INTR_EN_PCIE4_DN_L         8
#define OFFSET_HSIB_ERR_INTR_EN_PCIE3_DN_H         7
#define OFFSET_HSIB_ERR_INTR_EN_PCIE3_DN_L         6
#define OFFSET_HSIB_ERR_INTR_EN_PCIE2_DN_H         5
#define OFFSET_HSIB_ERR_INTR_EN_PCIE2_DN_L         4
#define OFFSET_HSIB_ERR_INTR_EN_PCIE1_DN_H         3
#define OFFSET_HSIB_ERR_INTR_EN_PCIE1_DN_L         2
#define OFFSET_HSIB_ERR_INTR_EN_PCIE0_DN_H         1
#define OFFSET_HSIB_ERR_INTR_EN_PCIE0_DN_L         0

#define MASK_HSIB_ERR_INTR_EN_WAN_DN_H          0x00008000
#define MASK_HSIB_ERR_INTR_EN_WAN_DN_L          0x00004000
#define MASK_HSIB_ERR_INTR_EN_HWIC_DN_H         0x00002000
#define MASK_HSIB_ERR_INTR_EN_HWIC_DN_L         0x00001000
#define MASK_HSIB_ERR_INTR_EN_GLB_REG_DN_H      0x00000800
#define MASK_HSIB_ERR_INTR_EN_GLB_REG_DN_L      0x00000400
#define MASK_HSIB_ERR_INTR_EN_PCIE4_DN_H        0x00000200
#define MASK_HSIB_ERR_INTR_EN_PCIE4_DN_L        0x00000100
#define MASK_HSIB_ERR_INTR_EN_PCIE3_DN_H        0x00000080
#define MASK_HSIB_ERR_INTR_EN_PCIE3_DN_L        0x00000040
#define MASK_HSIB_ERR_INTR_EN_PCIE2_DN_H        0x00000020
#define MASK_HSIB_ERR_INTR_EN_PCIE2_DN_L        0x00000010
#define MASK_HSIB_ERR_INTR_EN_PCIE1_DN_H        0x00000008
#define MASK_HSIB_ERR_INTR_EN_PCIE1_DN_L        0x00000004
#define MASK_HSIB_ERR_INTR_EN_PCIE0_DN_H        0x00000002
#define MASK_HSIB_ERR_INTR_EN_PCIE0_DN_L        0x00000001
#define MASK_HSIB_ERR_INTR_EN_ALL               0x0000ffff

/*
 * Notes:
 * HSIBErrorInterruptEnableRegister_dn_module_High and
 * HSIBErrorEventRegister_dn_module_High in the interrupt controller
 * use the same bit defines as the DN_INITIATOR_ERROR register in
 * the HSIB error monitor module.
 * HSIBErrorInterruptEnableRegister_dn_module_Low and
 * HSIBErrorEventRegister_dn_module_Low in the interrupt controller
 * use the same bit defines as the DN_TARGET_ERROR register in
 * the HSIB error monitor module.
 */

/*
 * PCIe MSI Capability Reg bit defines
 */
#define OFF_PCIE_MSI_CAP_ID                   0
#define OFF_PCIE_MSI_CAP_NEXT_PTR NEXT        8
#define OFF_PCIE_MSI_CAP_MSI_EN              16
#define OFF_PCIE_MSI_CAP_MULTI_MSG_CAP       17
#define OFF_PCIE_MSI_CAP_MULTI_MSG_EN        20

#define MASK_PCIE_MSI_CAP_ID             0x000000ff
#define MASK_PCIE_MSI_CAP_NEXT_PTR       0x0000ff00
#define MASK_PCIE_MSI_CAP_MSI_EN         0x00010000
#define MASK_PCIE_MSI_CAP_MULTI_MSG_CAP  0x000e0000
#define MASK_PCIE_MSI_CAP_MULTI_MSG_EN   0x00700000
#define MASK_PCIE_MSI_CAP_MSG_ADDR       0xfffffffc
#define MASK_PCIE_MSI_CAP_MSG_DATA       0x0000ffff
#define MASK_PCIE_MSI_CAP_INTR_MASK      0xffffffff

#ifndef LINUX_KLM
extern int goofy_init_interrupt (dev_object_t *, ulong,
                                             ulong, int);
extern PFI goofy_install_isr_vect (dev_object_t *, goofy_intr_class_t,
                                                        uint, int, PFI);
extern int goofy_set_interrupt (dev_object_t *, goofy_intr_class_t,
                                                uint, int, int, int);
extern void goofy_intr_handler (dev_object_t *, int, int, ulong);
extern void goofy_intr_reg_display (dev_object_t *dev);
extern void gfy_intr_attach (dev_object_t *dev);
#endif /* LINUX_KLM */
#endif /* GOOFY_INTR_H */

/******** History ******** 
$Log: goofy_intr.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
