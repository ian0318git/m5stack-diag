 /* $Id: diag_fpga.h,v 1.5 2018/11/09 07:33:24 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_fpga.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_fpga.h
 * Description: Header file of VIPER FPGA Diag.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_H__
#define __DIAG_FPGA_H__

#include "defs.h"
#include "common_utils.h"

/* Common */


#define PLAT_FPGA_REG_WIDTH     4
#define FPGA_REG_WIDTH_IN_BIT   (PLAT_FPGA_REG_WIDTH * 8)
#define WAITTIME_5_MS           5
#define WAITTIME_20_MS          20
#define WAITTIME_150_MS         150
#define WAITTIME_250_MS         250
#define WAITTIME_1000_MS        1000
#define WAITTIME_1500_MS        1500
#define WAITTIME_2000_MS        2000
#define WAITTIME_3500_MS        3500
#define WAITTIME_5000_MS        5000
#define DELAY_FOR_LED_TEST      1500 
#define WAITTIME_59000_MS       59000
#define TIMEVALUE_FOR_WATCHDOG  60
#define TIME_299000MS           0x48ff8


#define FPGA_RONLY    (READ_ONLY | REG_ACCESS)
#define FPGA_RW       (READ_WRITE | REG_ACCESS)

typedef struct viper_reg_bit {
    char           *name;
    unsigned int   offset;
} viper_reg_bit_t;

/* FPGA Watch Dog pattern */
#define FOXCONN_WDT_ENABLE_PATTERN    0xABCD0001
#define PEGATRON_WDT_ENABLE_PATTERN    0xABCD0001
#define PEGATRON_WDT_DISABLE_PATTERN    0x00000000
#define WDT_DISABLE_PATTERN           0xBD000000
#define WDT_DISABLE_TEST_PATTERN      0xAA000000

/* SKU IDs */
/* xDSL SKUs */
/* TBD */
#define C921J_E2E                                    0x1

#define DSL_SKU_ANNEX_B                              0x0
#define DSL_SKU_ANNEX_M                              0x1
#define DSL_SKU_ANNEX_A                              0x2
#define DSL_SKU_MASK                                 0x3

/* Register Definition */
/* Reg Offset */
#define FPGA_SPI_CONTROL_REG         0x058
#define FPGA_IOS_WATCHDOG_TIMER_REG  0x084
#define FPGA_EXTER_DEV_RST_REG       0x804
#define FPGA_INT_DEV_RST_REG         0x808
#define FPGA_RESET_REASON_REG        0x810
#define FPGA_WATCHDOG_BOOT_TIMER     0x064
#define FPGA_BOARD_TYPE_REG          0x8C0
#define FPGA_MASTER_REV_REG          0x884
#define FPGA_REV_REG                 0x88C
#define FPGA_CPUMUX_AND_USBPWR_REG   0x904
#define FPGA_STAT_AND_CTRL_REG       0x90C
#define FPGA_PWR_STAT_REG            0x910
#define FPGA_BT_STR_CHK_REG          0x914
#define FPGA_LED_REG                 0x91C
#define FPGA_LTE_RSSI_LED            0x920
#define FPGA_WATCHDOG_REG            0x924
#define FPGA_LTE_CTL_REG             0x938
#define FPGA_SIM_STATUS_CTL_REG      0x93C
#define FPGA_DSL_STATUS_CTL_REG      0x940

#define FPGA_I2C_CTL_REG             0xA00
#define FPGA_I2C_SCRATCH_PAD         0xA04
#define FPGA_I2C_STAT_REG            0xA08
#define FPGA_I2C_STAT_MASK_REG       0xA0C
#define FPGA_I2C_SLA_ADDR_REG        0xA10
#define FPGA_I2C_SLA_SUBADDR_REG     0xA14
#define FPGA_I2C_BIT_BANG_REG        0xA18
#define FPGA_I2C_BYTE_COUNT_REG      0xA1C
#define FPGA_I2C_DATA_FIFO_REG       0xA40
#define FPGA_I2C_DATA_RW_PTR_REG     0xA44

#define FPGA_SPI_CTRL_REG            0xB00
#define FPGA_SPI_STAT_REG            0xB04
#define FPGA_SPI_RD_SIZE_REG         0xB08
#define FPGA_SPI_RW_DATA_REG         0xB0C
#define FPGA_SPI_OP_ADDR_REG         0xB10

#define FPGA_RECONFIG_CTRL_REG       0xC00

#define FPGA_FIRMWARE_STATUS_REG     0xF00
#define FPGA_SOFT_SECURE_BOOT_STATUS_REG    0xF04
#define FPGA_SCRATCHPAD_REG_1        0xF08
#define FPGA_SCRATCHPAD_REG_2        0xF0C
#define FPGA_SCRATCHPAD_REG_3        0xF10
#define FPGA_SCRATCHPAD_REG_4        0xF14
#define FPGA_SCRATCHPAD_REG_5        0xF18
#define FPGA_SCRATCHPAD_REG_6        0xF1C

#define FPGA_MANUFAC_TEST_MODE_REG   0xD00
#define FPGA_LTE_SIM_LED             0xD10
#define FPGA_WDT_ENABLE_REG          0xD10
#define FPGA_ACCESS_TEST_REG         0xD20

/* Reg Content */
/* LED Control Reg(0x0014) */
#define PWR_OK_LED                   (1 << 2)
#define PWR_OK_LED_OFF               (0 << 2)
#define STAT_LED_OFF                 0
#define STAT_LED_YB                  0x1
#define STAT_LED_Y                   0x2
#define STAT_LED_G                   0x3


/* LPC Reset Button Reg(0x00C4) */
#define RST_BUTTON_MSK_OFFSET       1
#define RST_BUTTON_MSK              (1 << RST_BUTTON_MSK_OFFSET)
#define RST_BUTTON_IS_MASKED        (1 << RST_BUTTON_MSK_OFFSET)
#define RST_BUTTON_STAT             (1 << 0)
#define RST_BUTTON_PRESSED          0x1

/* (0x001C) */
#define FPGA_ACT2_RST_L             (1 << 2)

/* FPGA External Device Reset Reg(0x804) */
#define FPGA_1512_RESET             (1 << 24)
#define FPGA_EMMC_RESET             (1 << 23)
#define FPGA_GEWAN1_RESET           (1 << 22)
#define FPGA_GEWAN0_RESET           (1 << 21)
#define EXT_DSL_CHIP_RESET          (1 << 20)
#define EXT_PRI_LTE_RESET           (1 << 6) 
#define EXT_CPU_SYS_RESET           (1 << 3)
#define ACT2_RESET                  (1 << 2)
#define EXT_ESW_RESET               (1 << 1)

/* FPGA External Device Reset Reg(0x804) */
#define I2C_CONTROLLER_RESET        (1 << 0)
#define I2C_CONTROLLER_OUT_RESET    (0 << 0)

/* FPGA Internal Device Reset Reg(0x808) */
#define INT_I2C_RESET               (1 << 0)

/* FPGA CPU Reset Reason Reg(0x810) */
#define REASON_BOOT_TIMER_TIMEOUT     (1 << 16)
#define REASON_INTEL_POWER_CYCLE_REQ_HARD_RESET     (1 << 13)
#define REASON_INTEL_RESET_REQ_SOFT_RESET     (1 << 12)
#define REASON_IOS_WATCHDOG_TIMEOUT   (1 << 6)
#define REASON_BOOT_FAIL              (1 << 5)
#define REASON_CPU_THERMAL_RESET      (1 << 4)
#define REASON_SOFTWARE_REQUEST_RESET (1 << 1)
#define REASON_POWER_ON_RESET         (1 << 0)


/* Master FPGA Revision Register(0x884) */
#define MASTER_FPGA_DEBUG           (1 << 23)
#define MASTER_FPGA_DEBUG_MASK      (1 << 23)
#define MASTER_FPGA_MAJOR_REV       (1 << 16)
#define MASTER_FPGA_MAJOR_REV_MASK  (0x7F << 16)
#define MASTER_FPGA_MAJOR_REV_SHIFT (16)
#define MASTER_FPGA_MINOR_REV       (1 << 8)
#define MASTER_FPGA_MINOR_REV_SHIFT (8)
#define MASTER_FPGA_MINOR_REV_MASK  (0xFF << 8)
#define MASTER_FPGA_DEBUG_REV       (1 << 0)
#define MASTER_FPGA_DEBUG_REV_MASK  (0xF << 0)

/* FPGA Revision Register(0x88C) */
#define FPGA_MAJOR_REV_MASK         (0xF << 4)
#define FPGA_MAJOR_REV_SHIFT        4
#define FPGA_MINOR_REV_MASK         (0xF << 0)

/* CPU MUX and USB Power Register(0x904) */
#define CPU_SPI_MUX_SEL             (1 << 5)
#define SPI_MUX_RPOM_SEL            (1 << 4)
#define FPGA_USB_PWR                (1 << 0)

/* FPGA LED Reg(0x91C) */
#define LED_TEST_MODE               (1 << 0)
#define LED_NORMAL_MODE             (0 << 0)
#define VPN_OK_LED_GREEN            (1 << 5)
#define LTE_SIM_ACT_LED             (1 << 4)
#define SYS_OK_LED_AMBER            (0x3 << 0)
#define SYS_OK_LED_GREEN            (1 << 0)
#define LED_OFF                     (0 << 0)
#define SYS_OK_LED_AMBER_J          (0x3 << 0)
#define SYS_OK_LED_GREEN_J          (0x1 << 0)
#define LED_OFF_J                   (0x0 << 1)

/* FPGA LTE RSSI LED Reg(0x920) */
#define LTE_MOD_SERV_TYPE_MSK       (0x3 << 6)   /* bit[7:6] LTE modem Service Type */
#define LTE_MOD_NO_SERV             (0 << 6)
#define LTE_MOD_2G3G_SIGNAL_AMBER   (1 << 6)
#define LTE_MOD_LTE_SIGNAL_GREEN    (2 << 6)
#define LTE_MOD_NOT_EXIST           (3 << 6)
#define LTE_MOD_RSSI_MSK            (0xF << 2)   /* bit[5:2] LTE modem RSSI */
#define LTE_MOD_NO_RSSI             (0 << 2)
#define LTE_MOD_RSSI                (1 << 2)
#define LTE_MOD_LOW_RSSI            (3 << 2)
#define LTE_MOD_MEDIUM_RSSI         (7 << 2)
#define LTE_MOD_HIGH_RSSI           (0xF << 2)

/* FPGA External Interrupt Pending Reg(0x1128) */
#define WIFI_THERM_INTERRUPT_PENDING  (1 << 19)
#define MB_THERM_INTERRUPT_PENDING  (1 << 13)
#define XDSL_INTERRUPT_PENDING      (1 << 4)
#define POE_FPGA_INTR_PENDING       (1 << 3)

/* Force External Interrupt Reg(0x1130) */
#define RESET_BTN_LONG_F_INTR       (1 << 23)
#define RESET_BTN_SHORT_F_INTR      (1 << 22)
#define USB_SW_FPGA_OC_F_INTR       (1 << 16)
#define SMI0_DETECT_F_INTR          (1 << 8)
#define XDSL_F_INTR                 (1 << 4)
#define UART0_F_INTR                (1 << 1)
#define XDSL_DYING_GASP_F_INTR      (1 << 0)

/* LTE control Reg(0x938) */
#define WWAN_A_FPGA_LED_L           (1 << 10)
#define LTE_SAFE_POWER_RM_SIG       (1 << 9)
#define LTE_MODEM_POWER_CONTROL     (1 << 6)
#define EXT_PRI_LTE_WDIS_1_RESET    (1 << 4)
#define LTE_USB_MUX_SEL_CTL         (1 << 3)
#define LTE_USB_MUX_DISABLE         (1 << 2)
#define LTE_PRI_MODEM_EN_CTL        (1 << 0)

/* SIM status and control Reg(0x93C) */
#define LTE_SIM_0_PRESENT_DECTECT   (1 << 3)
#define LTE_SIM_POWER_EN_PRI        (1 << 0)

/* xDSL status and control Reg(0x940) */
#define DSL_FPGA_EXP_PRI_RDY        (1 << 0)


/* FPGA JDM Debug Reg(0xD00) - Viper J */
#define DBG_TEST_MODE_OFF 0x0
#define DBG_TEST_MODE_ON 0x1

/* FPGA JDM Debug Reg(0xD00) - Viper - Intel */
#define POWER_MARGIN_CTL_REG         0xD18
#define VCC_MARG_HI_VALUE            0xA8
#define VCC_MARG_LO_VALUE            0x54
#define POWER_MARGIN_HI              2
#define POWER_MARGIN_LO              1
#define POWER_MARGIN_N               0

/* FPGA JDM Debug Reg(0xD10) - Viper - Intel*/
#define LTE_SIM_LED_ON                   0x1ED00000
#define LTE_SIM_LED_OFF                  0x0


#define INTERRUPT_MASK_ALL           0xFFFFFFFF
#define INTERRUPT_PENDING_ALL        0xFFFFFFFF

#define FPGA_BTYPE_VIPER_J           0x0
#define FPGA_BTYPE_PRODUCT_SKU_BIT   0x20 
#define FPGA_BTYPE_PRODUCT_SKU_BIT_SHIFT    0x5 

#define FPGA_BTYPE_DSL_SKU_MASK      0x18
#define FPGA_BTYPE_DSL_SKU_BIT_SHIFT 0x03

#define FPGA_BTYPE_LTE_SKU_BIT           0x1
#define FPGA_BTYPE_DSL_SKU_BIT           0x4
#define FPGA_BTYPE_DSL_BIT_SHIFT         0x2
#define FPGA_BTYPE_DSL                   0x0

#define FPGA_UPGRADE_HEADER_NOTE_READ 0x0
#define FPGA_UPGRADE_RECONF_FSM_RST 0x2


#define FPGA_IOS_WDT_COUNT_MASK 0xFFFFF
#define FPGA_IOS_WDT_ENABLE_KEY 0xAC000000
#define FPGA_IOS_WDT_DISABLE_KEY 0xCA000000

#define FPGA_IOS_WDT_DEFAULT_VALUE 0xEA60
#define FPGA_IOS_WDT_STROBE_VALUE 0xAA

#define FPGA_IOS_WDT_TEST_TIMES 2

#define FPGA_PLATFORM_PWR_STATUS_BIT    0x2
#define FPGA_LTE_3V7_BIT    0x1



typedef struct sys_lvl_t_ {
    volatile unsigned int pad1;
    volatile unsigned int in_rst  /*0x04*/;
    volatile unsigned int ext_rst /*0x08*/;
    volatile unsigned char pad3[0x74];  
    volatile unsigned int brd /*0x80*/;
    volatile unsigned int mas_ver /*0x84*/;
    volatile unsigned int pad2;
    volatile unsigned int ver /*0x8C*/;
} sys_lvl_t;


/*
 * Externs
 */
extern unsigned long dash_fpga;
extern int fpga_reset_api(uint, uint, uint, uint);
extern int fpga_read_reg(uint, uint *);
extern int fpga_write_reg(uint, uint);
extern int fpga_reg_rd_util(int);
extern int fpga_reg_wr_util(int);
extern int viper_fpga_utils(int );
extern int this_is_viper_j(void);
extern int this_is_viper_foxconn(void);
extern int get_dsl_annex_sku_id(void);
extern int has_lte_sku(void);
extern int has_dsl_sku(void);
extern int has_ge1_sku(void);
extern int fpga_vol_margin(uint8_t);

#endif   /* __PLATFORM_FPGA_H__ */

/*-------------------------------------------------
 * $Log: diag_fpga.h,v $
 * Revision 1.5  2018/11/09 07:33:24  yungchen
 * Merge viper branch4 to the main trunk (CSCvn11857)
 *
 * Revision 1.4  2018/10/11 06:02:59  harrchan
 * Add FPGA function test (CSCvm72986)
 *
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.20  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.19  2018/06/11 10:57:53  lucywang
 * Added utility to enable FPGA IOS Watchdog
 *
 * Revision 1.1.2.18  2018/05/29 07:24:45  harrchan
 * Add LTE SIM card LED on/off utility
 *
 * Revision 1.1.2.17  2018/05/25 08:08:13  olin2
 * Support system reset and display reset reason util
 *
 * Revision 1.1.2.16  2018/05/10 05:51:21  olin2
 * Support voltage margin util for Viper-Intel
 *
 * Revision 1.1.2.15  2018/05/09 07:11:25  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.14  2018/04/20 10:07:26  harrchan
 * Modify FPGA register according register map
 *
 * Revision 1.1.2.13  2018/04/20 03:05:49  lucywang
 * Based on FPGA Board Type Register to show LTE/DLS test item
 *
 * Revision 1.1.2.12  2018/04/16 08:41:43  olin2
 * Support DSL test
 *
 * Revision 1.1.2.11  2018/04/13 11:19:12  lucywang
 * Modified to use Cisco FPGA : 1) Upgrade 2) LED 3) FPGA register 4) FPGA I2C reset
 *
 * Revision 1.1.2.10  2018/04/13 03:29:07  harrchan
 * Set FPGA register to out of reset component
 *
 * Revision 1.1.2.9  2018/04/10 06:17:15  harrchan
 * Modify FPGA register address
 *
 * Revision 1.1.2.8  2018/03/29 12:56:06  lucywang
 * Added LED utilities to turn on/off all green/amber LEDs
 *
 * Revision 1.1.2.7  2018/03/28 10:49:45  lucywang
 * Fixed bug : used bit 5 of FPGA_BOARD_TYPE to distinguish ViperJ
 *
 * Revision 1.1.2.6  2018/03/28 07:03:51  lucywang
 * Added API to check SKU ViperJ and changed interface name for ViperJ
 *
 * Revision 1.1.2.5  2018/03/26 09:21:03  harrchan
 * Add led utility
 *
 * Revision 1.1.2.4  2018/03/16 06:51:04  harrchan
 * Update FPGA register table
 *
 * Revision 1.1.2.3  2018/03/16 02:12:29  harrchan
 * Change FPGA register test offset to watchdog boot timer(0x864)
 *
 * Revision 1.1.2.2  2018/03/15 08:26:16  harrchan
 * Change I/O access to memory map
 *
 * Revision 1.1.2.1  2018/02/27 08:06:41  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
