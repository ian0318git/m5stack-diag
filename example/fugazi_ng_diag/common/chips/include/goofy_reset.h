/* $Id: goofy_reset.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/goofy_reset.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's reset registers
 *
 * May 2006, Bao Buu
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef GOOFY_RESET_H

/*****************************************
 March 07, 2005 09:03:54 AM
 constants
*****************************************/
#define GOOFY_RESET_H

/*
 * Reset controller memory map
 */
#define A_OFST_GFY_DEVICE_NODE_RST   0x00000000
#define A_OFST_GFY_FUNC_BLOCK_RST    0x00000004
#define A_OFST_GFY_HOST_CPU_RST      0x00000008
#define A_OFST_GFY_GOOFY_WARM_RST    0x0000000c
#define A_OFST_GFY_HOST_SYSTEM_RST   0x00000010
#define A_OFST_GFY_WD_TMR0_LIMIT     0x00000014
#define A_OFST_GFY_WD_TMR0_SERVICE   0x00000018
#define A_OFST_GFY_WD_TMR0_ENABLE    0x0000001c
#define A_OFST_GFY_WD_TMR0_TRIG_LVL  0x00000020
#define A_OFST_GFY_WD_TMR1_LIMIT     0x00000024
#define A_OFST_GFY_WD_TMR1_SERVICE   0x00000028
#define A_OFST_GFY_WD_TMR1_ENABLE    0x0000002c
#define A_OFST_GFY_WD_TMR1_TRIG_LVL  0x00000030
#define A_OFST_GFY_RST_STATUS        0x00000034

/*
 * Number of clock ticks for 1 milli sec at 5MHz.
 * Used in programming the goofy watchdog time limit reg.
 */
#define ONE_MS_AT_5MHZ              5000

#define GOOFY_WD_TIMER_LIMIT_DEFAULT        0x00ffffff
#define GOOFY_RESET_DEASSERT_TIMER  31

#define RESET_CMD_50                0x50
#define RESET_CMD_00                0x00
#define RESET_CMD_CPU_HARD          0xA2
#define RESET_CMD_CPU1_SOFT         0xA4
#define RESET_CMD_CPU2_SOFT         0xA8
#define RESET_CMD_GOOFY_WARM        0xC1
#define RESET_CMD_POWER_ON          0xA1

#define RESET_CMD_WD_C              0xC
#define RESET_CMD_WD_EN_TIMER       0x2
#define RESET_CMD_WD_DIS_TIMER      0x0
#define RESET_CMD_WD_IDLE           0x4

/*****************************************
 March 07, 2005 09:04:04 AM
 typedefs
*****************************************/
/*
 * Goofy system reset type via reset registers
 */
typedef enum goofy_reset_class_
{
    GOOFY_WARM_RESET = 0,
    HOST_CPU_HARD_RESET,
    HOST_CPU1_SOFT_RESET,
    HOST_CPU2_SOFT_RESET,
    HOST_SYS_RESET,
    GOOFY_RESET_CLASS_MAX,
} goofy_reset_class_t;

/*
 * Goofy watchdog timer timeout trigger levels
 */
typedef enum goofy_wdog_trig_lvl_
{
    GOOFY_WDOG_TIMER_INTR_TRIG = 0,
    GOOFY_WDOG_CPU_SOFT_RESET_TRIG,
    GOOFY_WDOG_CPU_HARD_RESET_TRIG,
    GOOFY_WDOG_HOST_RESET_TRIG,
} goofy_wdog_trig_lvl_t;

typedef struct goofy_wd_
{
    volatile uint32_t watch_dog_timer_limit;
    volatile uint32_t watch_dog_timer_serv;
    volatile uint32_t watch_dog_timer_en;
    volatile uint32_t watch_dog_timer_trig;
} goofy_wd_t; /* struct goofy_wd_t */

typedef struct goofy_reset_
{
    volatile uint32_t device_node_reset;                /* 0x0  */
    volatile uint32_t functional_block_reset;
    volatile uint32_t host_cpu_reset;
    volatile uint32_t goofy_warm_reset;
    volatile uint32_t host_system_reset;                 /* 0x10 */
    goofy_wd_t     wd_regs[2];
    volatile uint32_t reset_status;
    uint32_t reserved[0x12];                    /* 0x38-0x7F */
} goofy_reset_t; /* struct goofy_reset_ */

/*****************************************
 March 07, 2005 09:04:24 AM
 global variables
*****************************************/
#ifndef GOOFY_REGS_C
    extern goofy_reset_t *goofy_reset_regs;
    extern goofy_reset_t goofy_reset_tmp_regs;
    extern goofy_reset_t *goofy_reset_dbg_regs;
#else
    goofy_reset_t *goofy_reset_regs;
    goofy_reset_t goofy_reset_tmp_regs;
    goofy_reset_t *goofy_reset_dbg_regs;
#endif

/*****************************************
 March 07, 2005 12:35:17 PM
 macros
*****************************************/

/*******************************************************************************
START OF GENERATED CODE brichang - DO NOT TYPE BELOW THIS LINE
*******************************************************************************/
  /******************************************
  * device_node_reset
  *  0: 0 pcie_us_warm
  *  1: 1 pcie_us_hot
  *  2: 2 pcie_ds0_warm
  *  3: 3 pcie_ds0_hot
  *  4: 4 pcie_ds1_warm
  *  5: 5 pcie_ds1_hot
  *  6: 6 pcie_ds2_warm
  *  7: 7 pcie_ds2_hot
  *  8: 8 pcie_ds3_warm
  *  9: 9 pcie_ds3_hot
  * 10:10 glbl_warm
  * 11:11 glbl_hot
  * 12:12 quad_hwic_warm
  * 13:13 quad_hwic_hot
  * 14:14 wan_warm
  * 15:15 wan_hot
  * 31:16 reserved1
  ******************************************/
/**** field offsets ********************/
#define OFFSET_DN_RESET_HT_COLD                    0
#define OFFSET_DN_RESET_HT_WARM                    1
#define OFFSET_DN_RESET_PCI0_COLD                  2
#define OFFSET_DN_RESET_PCI0_WARM                  3
#define OFFSET_DN_RESET_PCI1_COLD                  4
#define OFFSET_DN_RESET_PCI1_WARM                  5
#define OFFSET_DN_RESET_WAN_COLD                   6
#define OFFSET_DN_RESET_WAN_WARM                   7
#define OFFSET_DN_RESET_HWIC_COLD                  8
#define OFFSET_DN_RESET_HWIC_WARM                  9
#define OFFSET_DN_RESET_GLBL_COLD                  10
#define OFFSET_DN_RESET_GLBL_WARM                  11
/**** field masks **********************/
#define MASK_DN_RESET_PCIE_US_WARM                 0x00000001
#define MASK_DN_RESET_PCIE_US_HOT                  0x00000002
#define MASK_DN_RESET_PCIE_DS0_WARM                0x00000004
#define MASK_DN_RESET_PCIE_DS0_HOT                 0x00000008
#define MASK_DN_RESET_PCIE_DS1_WARM                0x00000010
#define MASK_DN_RESET_PCIE_DS1_HOT                 0x00000020
#define MASK_DN_RESET_PCIE_DS2_WARM                0x00000040
#define MASK_DN_RESET_PCIE_DS2_HOT                 0x00000080
#define MASK_DN_RESET_PCIE_DS3_WARM                0x00000100
#define MASK_DN_RESET_PCIE_DS3_HOT                 0x00000200
#define MASK_DN_RESET_GLBL_WARM                    0x00000400
#define MASK_DN_RESET_GLBL_HOT                     0x00000800
#define MASK_DN_RESET_QHWIC_WARM                   0x00001000
#define MASK_DN_RESET_QHWIC_HOT                    0x00002000
#define MASK_DN_RESET_WAN_WARM                     0x00004000
#define MASK_DN_RESET_WAN_HOT                      0x00008000

  /******************************************
  * functional_block_reset
  *  0: 0 pcie_us_warm
  *  1: 1 reserved
  *  2: 2 pcie_ds0_warm
  *  3: 3 pcie_ds0_hot
  *  4: 4 pcie_ds1_warm
  *  5: 5 pcie_ds1_hot
  *  6: 6 pcie_ds2_warm
  *  7: 7 pcie_ds2_hot
  *  8: 8 pcie_ds3_warm
  *  9: 9 pcie_ds3_hot
  * 10:10 tdm
  * 11:11 interrupt
  * 12:12 gpio
  * 13:13 global_regs
  * 14:14 hwic0
  * 15:15 hwic1
  * 16:16 hwic2
  * 17:17 hwic3
  * 18:18 scc
  * 19:19 hdlc0
  * 20:20 hdlc1
  * 21:21 hdlc2
  * 22:22 hdlc3
  * 23:23 hdlc4
  * 24:24 hdlc5
  * 25:25 hdlc6
  * 26:26 hdlc7
  * 27:27 packet_pump
  * 28:28 hsib
  * 31:23 reserved1
  ******************************************/
#define MASK_FUNC_BLK_RESET_PCIE_US0_WRM            0x00000001
#define MASK_FUNC_BLK_RESET_PCIE_DS1_WRM            0x00000004
#define MASK_FUNC_BLK_RESET_PCIE_DS1_HOT            0x00000008
#define MASK_FUNC_BLK_RESET_PCIE_DS2_WRM            0x00000010
#define MASK_FUNC_BLK_RESET_PCIE_DS2_HOT            0x00000020
#define MASK_FUNC_BLK_RESET_PCIE_DS3_WRM            0x00000040
#define MASK_FUNC_BLK_RESET_PCIE_DS3_HOT            0x00000080
#define MASK_FUNC_BLK_RESET_PCIE_DS4_WRM            0x00000100
#define MASK_FUNC_BLK_RESET_PCIE_DS4_HOT            0x00000200
#define MASK_FUNC_BLK_RESET_TDM                     0x00000400
#define MASK_FUNC_BLK_RESET_INTR                    0x00000800
#define MASK_FUNC_BLK_RESET_GPIO                    0x00001000
#define MASK_FUNC_BLK_RESET_GLOBAL_REGS             0x00002000
#define MASK_FUNC_BLK_RESET_HWIC0                   0x00004000
#define MASK_FUNC_BLK_RESET_HWIC1                   0x00008000
#define MASK_FUNC_BLK_RESET_HWIC2                   0x00010000
#define MASK_FUNC_BLK_RESET_HWIC3                   0x00020000
#define MASK_FUNC_BLK_RESET_SCC                     0x00040000
#define MASK_FUNC_BLK_RESET_HDLC0                   0x00080000
#define MASK_FUNC_BLK_RESET_HDLC1                   0x00100000
#define MASK_FUNC_BLK_RESET_HDLC2                   0x00200000
#define MASK_FUNC_BLK_RESET_HDLC3                   0x00400000
#define MASK_FUNC_BLK_RESET_HDLC4                   0x00800000
#define MASK_FUNC_BLK_RESET_HDLC5                   0x01000000
#define MASK_FUNC_BLK_RESET_HDLC6                   0x02000000
#define MASK_FUNC_BLK_RESET_HDLC7                   0x04000000
#define MASK_FUNC_BLK_RESET_PACKET_PUMP             0x08000000
#define MASK_FUNC_BLK_RESET_HSIB                    0x10000000

/*
 * Mask bits for reset status register
 */
#define MASK_RST_STAUS_REG                    0x000003ff
#define MASK_RST_STAUS_VCCOK_RST              0x00000001
#define MASK_RST_STAUS_CPU_HARD_RST           0x00000002
#define MASK_RST_STAUS_WD_TIMER_HARD_RST      0x00000004
#define MASK_RST_STAUS_CPU1_SOFT_RST          0x00000008
#define MASK_RST_STAUS_CPU2_SPFT_RST          0x00000010
#define MASK_RST_STAUS_WD_TIMER0_SOFT_RST     0x00000020
#define MASK_RST_STAUS_WD_TIMER1_SOFT_RST     0x00000040
#define MASK_RST_STAUS_PCIE_USTREAM_HOT_RST   0x00000080
#define MASK_RST_STAUS_PCIE_USTREAM_WARM_RST  0x00000100
#define MASK_RST_STAUS_GOOFY_WARM_RST         0x00000200

extern void goofy_reset_reg_display (dev_object_t *dev);
extern int goofy_reset_reg_test (dev_object_t *dev);
extern void gfy_reset_attach (dev_object_t *dev);

#endif /* GOOFY_RESET_H */

/******** History ******** 
$Log: goofy_reset.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
