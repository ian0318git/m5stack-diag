/* $Id: diag_fpga.h,v 1.7 2020/02/04 08:49:42 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_fpga.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_fpga.h
 * Description: Header file of NUTELLA FPGA Diag.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#define DELAY_FOR_OPERATION     1500 
#define DELAY_FOR_LED_TEST      1500 
#define WAITTIME_59000_MS       59000
#define TIMEVALUE_FOR_WATCHDOG  60
#define TIME_299000MS           0x48ff8
#define CEDGE_FPGA_MAJ_VER_2      0x2
#define CEDGE_FPGA_MAJ_VER_3      0x3


#define FPGA_RONLY    (READ_ONLY | REG_ACCESS)
#define FPGA_RW       (READ_WRITE | REG_ACCESS)

typedef struct nutella_reg_bit {
    char           *name;
    unsigned int   offset;
} nutella_reg_bit_t;

/* FPGA Watch Dog pattern */
#define FOXCONN_WDT_ENABLE_PATTERN    0xABCD0001
#define WDT_DISABLE_PATTERN           0xBD000000
#define WDT_DISABLE_TEST_PATTERN      0xAA000000

/* SKU IDs */

/* Register Definition */
/* Reg Offset */
#define CEDGE_FPGA_RESET_REASON_REG        0x004
#define CEDGE_LPC_SCRATCHPAD_REG           0x008
#define CEDGE_LPC_STATUS_LED_REG           0x014
#define CEDGE_LPC_DEV_RST_CONTROL_REG      0x01C
#define FPGA_SPI_CONTROL_REG                0x058
#define CEDGE_FPGA_IOS_WATCHDOG_TIMER_REG  0x07C
#define FPGA_IOS_WATCHDOG_TIMER_REG        0x084
#define CEDGE_FPGA_REV_REG                 0x084
#define CEDGE_FPGA_MASTER_REV_REG          0x088
#define CEDGE_SOFT_SECURE_BOOT_STATS       0x704
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

#define FPGA_AIKIDO_SPI_MASTER_OFFSET       0xE00
#define FPGA_AIKIDO_SPI_CTRL_REG            0xE00
#define FPGA_AIKIDO_SPI_STAT_REG            0xE04
#define FPGA_AIKIDO_SPI_RD_SIZE_REG         0xE08
#define FPGA_AIKIDO_SPI_RW_DATA_REG         0xE0C
#define FPGA_AIKIDO_SPI_OP_ADDR_REG         0xE10

#define FPGA_FIRMWARE_STATUS_REG            0xF00
#define FPGA_SOFT_SECURE_BOOT_STATUS_REG    0xF04
#define FPGA_SCRATCHPAD_REG_1               0xF08
#define FPGA_SCRATCHPAD_REG_2               0xF0C
#define FPGA_SCRATCHPAD_REG_3               0xF10
#define FPGA_SCRATCHPAD_REG_4               0xF14
#define FPGA_SCRATCHPAD_REG_5               0xF18
#define FPGA_SCRATCHPAD_REG_6               0xF1C

#define CEDGE_FPGA_FIRMWARE_STATUS_REG            0x700
#define CEDGE_FPGA_SOFT_SECURE_BOOT_STATUS_REG    0x704
#define CEDGE_FPGA_SCRATCHPAD_REG_1               0x708
#define CEDGE_FPGA_SCRATCHPAD_REG_2               0x70C
#define CEDGE_FPGA_SCRATCHPAD_REG_3               0x710
#define CEDGE_FPGA_SCRATCHPAD_REG_4               0x714
#define CEDGE_FPGA_SCRATCHPAD_REG_5               0x718
#define CEDGE_FPGA_SCRATCHPAD_REG_6               0x71C

#define FPGA_MANUFAC_TEST_MODE_REG   0xD00
#define FPGA_LTE_SIM_LED             0xD10
#define FPGA_WDT_ENABLE_REG          0xD10
#define FPGA_ACCESS_TEST_REG         0xD20

#define FPGA_LPC_BAD_ADDR_ACCESS_REG  0x688

/* Phase 2 FPGA Registers*/
#define PHASE2_FPGA_LPC_BOARD_TYPE_REG            0x080
#define PHASE2_FPGA_LPC_NIOS_VER_REG              0x090
#define PHASE2_FPGA_EXT_PIN_CTL_REG               0x810
#define PHASE2_FPGA_BOARD_TYPE_REG                0x880
#define PHASE2_FPGA_SLAVE_REV_REG                 0x888
#define PHASE2_FPGA_NIOS_VER_REG                  0x890
#define PHASE2_FPGA_SCRATCHPAD_REG                0x800
#define PHASE2_FPGA_LPC_CHASSIS_TEST_REG          0x054
#define PHASE2_FPGA_INTR_IRQ0_STAT_REG            0x024
#define PHASE2_FPGA_INTR_IRQ0_MASK_REG            0x028
#define PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG        0x078
#define PHASE2_FPGA_INTR_IRQ6_STAT_REG            0x034
#define PHASE2_FPGA_INTR_IRQ6_MASK_REG            0x038
#define PHASE2_FPGA_CCCP_RST_CTL_REG              0x404
#define PHASE2_FPGA_FPCP_RST_CTL_REG              0x604
#define PHASE2_FPGA_FP_RST_CTL_REG                0x610
#define PHASE2_FPGA_LPC_STAT_REG                  0x00C
#define PHASE2_FPGA_LPC_RESET_CONTROL_REG         0x010
#define PHASE2_FPGA_LPC_DEBUG_CTL_STAT_REG        0x018
#define PHASE2_FPGA_LPC_FLASH_LED_CTL_REG         0x020
#define PHASE2_FPGA_LPC_CHASSIS_MANAGE_CTL_REG    0x044
#define PHASE2_FPGA_LPC_ALARM_MANAGE_CTL_REG      0x048
#define PHASE2_FPGA_LPC_CTL_REG_0                 0x058
#define PHASE2_FPGA_LPC_CTL_REG_1                 0x05C
#define PHASE2_FPGA_LPC_PWR_CYCLE_REG             0x060
#define PHASE2_SECURE_JTAG_STAT_REG               0x8A8

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
#define RST_BUTTON_LOOSEN           0x0

/* LPC device reset control register (0x01C) */
#define CEDGE_FPGA_ACT2_RST_L             (1 << 2)
#define CEDGE_FPGA_EMMC_RESET             (1 << 1)

/* FPGA External Device Reset Reg(0x804) */
#define FPGA_EMMC_RESET             (1 << 23)
#define FPGA_GEWAN0_RESET           (1 << 21)
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
#define REASON_LONG_PRESS                           (1 << 18)
#define REASON_SHORT_PRESS                          (1 << 17)
#define REASON_BOOT_TIMER_TIMEOUT                   (1 << 16)
#define REASON_INTEL_POWER_CYCLE_REQ_HARD_RESET     (1 << 13)
#define REASON_INTEL_RESET_REQ_SOFT_RESET           (1 << 12)
#define REASON_CEDGE_CATASTROPHIC_ERROR_RESET       (1 << 11)
#define REASON_CEDGE_CPU_THERMAL_RESET              (1 << 10)
#define REASON_IOS_WATCHDOG_TIMEOUT                 (1 << 6)
#define REASON_BOOT_FAIL                            (1 << 5)
#define REASON_CPU_THERMAL_RESET                    (1 << 4)
#define REASON_CEDGE_BOARD_RESET                    (1 << 3)
#define REASON_SOFTWARE_REQUEST_RESET               (1 << 1)
#define REASON_POWER_ON_RESET                       (1 << 0)


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

/* LPC Status LED Reg(0x014) */
/* FPGA LED Reg(0x91C) */
#define LED_TEST_MODE               (1 << 0)
#define LED_NORMAL_MODE             (0 << 0)
#define VPN_OK_LED_YELLOW           (1 << 6)
#define VPN_OK_LED_GREEN            (1 << 5)
#define LTE_SIM_ACT_LED             (1 << 4)
#define SYS_OK_LED_AMBER            (3 << 0)
#define SYS_OK_LED_GREEN            (1 << 0)
#define LED_OFF                     (0 << 0)
#define CEDGE_SYS_OK_LED_AMBER      (1 << 1)
#define CEDGE_SYS_OK_LED_AMBER_BLINK (1 << 0)
#define CEDGE_SYS_OK_LED_GREEN      (3 << 0)
#define VPN_SYS_LED_GREEN            0x21
#define VPN_SYS_LED_YELLOW           0x43
#define CEDGE_VPN_SYS_LED_GREEN      0x23
#define CEDGE_VPN_SYS_LED_YELLOW     0x42

#define DEFAULT_TO_ZERO             (0 << 0)

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
#define POE_FPGA_INTR_PENDING       (1 << 3)

/* Force External Interrupt Reg(0x1130) */
#define RESET_BTN_LONG_F_INTR       (1 << 23)
#define RESET_BTN_SHORT_F_INTR      (1 << 22)
#define USB_SW_FPGA_OC_F_INTR       (1 << 16)
#define SMI0_DETECT_F_INTR          (1 << 8)
#define UART0_F_INTR                (1 << 1)

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

/* FPGA JDM Debug Reg(0xD00) - Nutella - Intel */
#define POWER_MARGIN_CTL_REG         0xD18
#define VCC_MARG_HI_VALUE            0xA8
#define VCC_MARG_LO_VALUE            0x54
#define POWER_MARGIN_HI              2
#define POWER_MARGIN_LO              1
#define POWER_MARGIN_N               0

/* FPGA JDM Debug Reg(0xD10) - Nutella - Intel*/
#define LTE_SIM_LED_ON                   0x1ED00000
#define LTE_SIM_LED_OFF                  0x0


#define INTERRUPT_MASK_ALL           0xFFFFFFFF
#define INTERRUPT_PENDING_ALL        0xFFFFFFFF


#define FPGA_BTYPE_NUTELLA                  0x0 
#define FPGA_BTYPE_PRODUCT_SKU_101B         0x24 
#define FPGA_BTYPE_PRODUCT_SKU_101M         0x25 
#define FPGA_BTYPE_PRODUCT_SKU_1001         0x26 
#define FPGA_BTYPE_PRODUCT_SKU_BIT_SHIFT    0x5 

#define FPGA_BTYPE_LTE_SKU_BIT           0x1
#define FPGA_BTYPE_SFP_SKU_BIT           0x2

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

#define FPGA_LPC_BAD_ADDR_RANDOM_VALUE    0xAA55BBCC

/* Phase 2 Check default value */
#define DEFAULT_BOARD_TYPE_REG            0x0500001C
#define DEFAULT_VALUE_IS_ZERO             0x00000000
#define DEFAULT_FPGA_EXT_PIN_CTL_REG      0x00100000
#define DEFAULT_IRQ0_MASK_REG             0x00002200
#define DEFAULT_LPC_CPU_ERROR_STAT_REG    0x57C00000
#define DEFAULT_IRQ6_STAT_REG             0x00001400
#define DEFAULT_IRQ6_MASK_REG             0x00000006
#define DEFAULT_LPC_RESET_CTL_REG         0x00000058
#define DEFAULT_LPC_ALARM_MANAGE_CTL_REG  0x00000100

/* Phase 2 FPGA LPC IRQ Force Test */
#define TRIGGER_IRQ0_INTERRUPT            0xCA040000
#define TRIGGER_IRQ6_INTERRUPT            0xCA100000
#define IRQ0_INTERRUPT_MAGIC_VAL          0xABCDEF00
#define IRQ6_INTERRUPT_MAGIC_VAL          0xABCDEF01
#define CLEAR_LPC_CHASSIS_INTR            0xCA000000

/* Phase 2 FPGA Scratchpad Register (0x800)*/
#define SCRATCHPAD_REG_TEST_PATTERN_1    0x12345678
#define SCRATCHPAD_REG_TEST_PATTERN_2    0x5A5A5A5A
#define SCRATCHPAD_REG_TEST_PATTERN_3    0xCACACACA
#define SCRATCHPAD_REG_TEST_PATTERN_4    0x87654321
#define SCRATCHPAD_REG_TEST_PATTERN_5    0x96969696

/* Phase 2 FPGA LPC CPU ERROR Status & Respone Register (0x078)*/
#define LPC_CPU_ERROR_STAT_REG_TEST_PATTERN_1    0x12340000
#define LPC_CPU_ERROR_STAT_REG_TEST_PATTERN_2    0xEDCB0000

/* IRQ6 Status Register(0x34) */
#define PKT_REAL_TIME_STATUS         (1<<26)
#define CC_REAL_TIME_STATUS          (1<<20)
#define FP_REAL_TIME_STATUS          (1<<18)
#define PACKET_READY_CHANGE_DETECT    (1<<2)
#define EOBC_READY_CHANGE_DETECT       (1<<1)

/* IRQ6 Mask Register(0x38) */
#define PACKET_READY_CHANGE_ON       (1<<2)
#define EOBC_READY_CHANGE_ON         (1<<1)

/* CC/FP CP Reset Conterol Register(0x404/0x604) */
#define CP_READY_OUTPUT_CTL      (1<<0)

/* FP Reset Conterol Register(0x610) */
#define PKT_READY_OUTPUT_CTL     (1<<0)

/* Bit to check */
#define FPGA_MCERR_BIT3      3
#define FPGA_MCERR_BIT9      9
#define FPGA_PROCHOT_BIT13   13
#define CHECK_TWICE          1

/* Read back value */
#define READ_BACK_VALUE_1    0x00000600
#define READ_BACK_VALUE_2    0x00000202
#define READ_BACK_VALUE_3    0x00000202
#define READ_BACK_VALUE_4    0x00000301
#define READ_BACK_VALUE_5    0x00000606

#define CHASSIS_MANAGE_READ_BACK_VALUE_1    0x12345478
#define CHASSIS_MANAGE_READ_BACK_VALUE_2    0x5A5A5858
#define CHASSIS_MANAGE_READ_BACK_VALUE_3    0xCACA48C8
#define CHASSIS_MANAGE_READ_BACK_VALUE_4    0x87654121
#define CHASSIS_MANAGE_READ_BACK_VALUE_5    0x96961494

#define FPGA_INS_IRQ_MODULE    "insmod /lib/modules/4.14.3/fpga_isr.ko" 
#define FPGA_RM_IRQ_MODULE    "rmmod fpga_isr.ko" 

enum 
{
    LPC_IRQ0 = 0,
    LPC_IRQ6,
};

enum 
{
    CCCP_RESET = 0,
    FPCP_RESET,
    FP_RESET,
};

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
extern unsigned long fpga_ptr;
extern int fpga_reset_api(uint, uint, uint, uint);
extern int fpga_read_reg(uint, uint *);
extern int fpga_write_reg(uint, uint);
extern int fpga_reg_rd_util(int);
extern int fpga_reg_wr_util(int);
extern int nutella_fpga_utils(int );
extern int has_lte_sku(void);
extern int has_sfp_sku(void);
extern int fpga_vol_margin(uint8_t);
extern int lpc_irq_force_test(int);

#endif   /* __PLATFORM_FPGA_H__ */

/*-------------------------------------------------
$Log: diag_fpga.h,v $
Revision 1.7  2020/02/04 08:49:42  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.6  2019/10/16 23:50:47  alicehua
CSCvr68092: Add LED utility (turn on/off all LED).

Revision 1.5  2019/07/12 09:13:42  alicehua
Modified codes based on code PRRQs.

Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
