/* $Id: diag_moka_fpga_lib.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_moka_fpga_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_moka_fpga_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_FPGA_H__
#define __PLATFORM_FPGA_H__

#include "defs.h"
#include "common_utils.h"

/* For External Interrupt Pending Register */
#define PENDING_BIT_GE1           1<<7 /* 0:no pending, 1:pending */
#define PENDING_BIT_ESW           1<<6 /* 0:pending, 1:no pending */
#define PENDING_BIT_GE0           1<<5 /* 0:no pending, 1:pending */
#define PENDING_BIT_POE           1<<3 /* 0:no pending, 1:pending */

/* For VDSL Port */
#define GIGA_PORT_3_MAC_CTL_REG2   0xf2133e08
#define GIGA_PORT_3_ADDR           0xf2133e0c
#define RESET_VAL                  0x0000
#define DIS_AN_EN_FORCE_1G_SPD     0x8042
#define DIS_AN_1G_SPD              0x9040
#define CLEAR_RESERVED_BITS        0xFC7F

#define FPGA_MAX_REG_ADDR       (0x1FFFF)
#define PLAT_FPGA_REG_WIDTH    4

/* Common */
#define PLAT_M_P1A_FPGA_VER      0x16071901

#define PLAT_FPGA_REG_WIDTH     4
#define FPGA_REG_WIDTH_IN_BIT   (PLAT_FPGA_REG_WIDTH * 8)
#define WAITTIME_5_MS           5
#define WAITTIME_20_MS          20
#define WAITTIME_150_MS         150
#define WAITTIME_1000_MS        1000
#define WAITTIME_3500_MS        3500
#define WAITTIME_5000_MS        5000

/* Check with HW 1ms enough */
#define MOKA_INT_ACT_WAIT                            (1)

#define FPGA_RONLY    (READ_ONLY | REG_ACCESS)
#define FPGA_RW       (READ_WRITE | REG_ACCESS)

typedef struct fpga_platform_sku_info_t {
    char           *platform_name;
    uint16_t       cook_contype;
    boolean           haslte;
    boolean           haswifi;
    uint16_t       platform;
} fpga_platform_sku_info;

typedef struct plat_reg_bit {
    char           *name;
    unsigned int   offset;
} plat_reg_bit_t;

/* SKU IDs */
#define SKU_ISR961_K9                               0x00
#define SKU_DEBUG                                   0x01
#define SKU_ISR961_LTE_EA_K9                        0x02
#define SKU_ISR961_LTE_LA_K9                        0x03
#define SKU_ISR961_2LTE_EA_K9                       0x04
#define SKU_ISR961_2LTE_LA_K9                       0x05
#define SKU_ISR961_2LTE_EA_DEBUG_K9                 0x06
#define SKU_ISR961_2LTE_LA_DEBUG_K9                 0x07
/* xDSL SKUs */
#define DSL_SKU_ISR967_K9                           0x4
#define DSL_SKU_ISR967B_K9                          0x5
#define DSL_SKU_ISR967_LTE_EA_K9                    0x6

#define DSL_SKU_ANNEX_B                              0x4
#define DSL_SKU_ANNEX_M                              0x5
#define DSL_SKU_ANNEX_A                              0x6

/* BCM963138 DSL SKUs */
#define DSL138_SKU_GFAST                            0x1
#define DSL138_SKU_ANNEX_M                          0x2
#define DSL138_SKU_ANNEX_A                          0x4
#define DSL138_SKU_ANNEX_B                          0x6

#define SUPPORT_DSL_SKU                             (0)
#define BOARD_TYPE_ANNEX_SHIFT                      (3)
#define BOARD_TYPE_DSL_SHIFT                        (2)
#define BOARD_TYPE_ANNEX_A                          (0x1)
#define BOARD_TYPE_ANNEX_M                          (0x2)
#define BOARD_TYPE_ANNEX_BJ                         (0x0)

/* GFast CID */
#define GF_Annex_B                      0xd29
#define GF_Annex_B_LTE                  0xd2a
#define GF_Annex_B_WLAN                 0x1017
#define GF_Annex_B_LTE_WLAN             0x1018
#define GF_Annex_M                      0xd2c
#define GF_Annex_M_LTE                  0xd2e
#define GF_Annex_M_WLAN                 0xd30
#define GF_Annex_A                      0xd2b
#define GF_Annex_A_LTE                  0xd2d
#define GF_Annex_A_WLAN                 0xd2f
#define GF_Annex_A_LTE_WLAN             0xd4a

/* GFast cid struct */
typedef struct GFAST_CID {
    int     val;
    int     cid;
}GFAST_CID;

/* Register Definition */
/* Reg Offset */
#define FPGA_LPC_SCRATCHPAD_REG      0x0008
#define FPGA_LPC_STAT_LED_CTRL_REG   0x0014
#define FPGA_LPC_EXT_DEV_RST_REG     0x001C
#define FPGA_IRQ_TEST_REG            0x0054
#define FPGA_BOARD_PWR_CYCLE_REG     0x0060
#define FPGA_LPC_BOARDTYPE_REG       0x0080
#define FPGA_LPC_SKUFEATURE_REG      0x00C0
#define FPGA_LPC_RESET_BUTTON_REG    0x00C4
#define FPGA_EXTER_DEV_RST_REG       0x1004
#define FPGA_INT_DEV_RST_REG         0x1008
#define FPGA_BOARD_TYPE_REG          0x1080
#define FPGA_MASTER_REV_REG          0x1084
#define FPGA_REV_REG                 0x108C
#define FPGA_DBG_LED_REG             0x10A0
#define FPGA_CPUMUX_AND_USBPWR_REG   0x1104
#define FPGA_STAT_AND_CTRL_REG       0x110C
#define FPGA_PWR_STAT_REG            0x1110
#define FPGA_CARD_AND_PWR_REG        0x1118
#define FPGA_LED_REG                 0x111C
#define FPGA_LTE_RSSI_LED_REG        0x1120
#define FPGA_WATCHDOG_REG            0x1124
#define FPGA_EXTER_INT_PENDING_REG   0x1128
#define FPGA_EXT_INTR_MASK_REG       0x112C
#define FPGA_FORCE_EXT_INTR_REG      0x1130
#define FPGA_SFP_AND_CTRL_REG        0x1134
#define FPGA_LTE_CTL_REG             0x1138
#define FPGA_SIM_STATUS_CTL_REG      0x113C
#define FPGA_DSL_STATUS_CTL_REG      0x1140
#define FPGA_I2C_CTL_REG             0x1200
#define FPGA_I2C_STAT_REG            0x1208
#define FPGA_I2C_STAT_MASK_REG       0x120C
#define FPGA_I2C_SLA_ADDR_REG        0x1210
#define FPGA_I2C_SLA_SUBADDR_REG     0x1214
#define FPGA_I2C_BIT_BANG_REG        0x1218
#define FPGA_I2C_BYTE_COUNT_REG      0x121C
#define FPGA_I2C_DATA_FIFO_REG       0x1240
#define FPGA_I2C_DATA_RW_PTR_REG     0x1244
#define FPGA_SPI_CTRL_REG            0x1300
#define FPGA_SPI_STAT_REG            0x1304
#define FPGA_SPI_RD_SIZE_REG         0x1308
#define FPGA_SPI_RW_DATA_REG         0x130C
#define FPGA_SPI_OP_ADDR_REG         0x1310

/* Reg Content */
/* LED Control Reg(0x0014) */
#define PWR_OK_LED                   (1 << 2)
#define PWR_OK_LED_OFF               (0 << 2)
#define STAT_LED_OFF                 0
#define STAT_LED_YB                  0x1
#define STAT_LED_Y                   0x2
#define STAT_LED_G                   0x3

/* LPC Board Type Reg(0x0080) */
#define PLAT_W_LTE                   (1 << 0)
#define PLAT_W_WIFI                  (1 << 1)
#define PLAT_W_GE1                   (1 << 2)

/* LPC Reset Button Reg(0x00C4) */
#define RST_BUTTON_MSK_OFFSET       1
#define RST_BUTTON_MSK              (1 << RST_BUTTON_MSK_OFFSET)
#define RST_BUTTON_IS_MASKED        (1 << RST_BUTTON_MSK_OFFSET)
#define RST_BUTTON_STAT             (1 << 0)
#define RST_BUTTON_PRESSED          0x1

/* (0x001C) */
#define FPGA_ACT2_RST_L             (1 << 2)

/* FPGA External Device Reset Reg(0x1004) */
#define FPGA_USB_HUB_RESET_BIT      (27)
#define FPGA_USB_HUB_RESET          (1 << 27)
#define FPGA_EMMC_RESET             (1 << 23)
#define FPGA_GEWAN1_RESET           (1 << 22)
#define FPGA_GEWAN0_RESET           (1 << 21)
#define EXT_DSL_CHIP_RESET          (1 << 20)
#define EXT_ROMMON_FLASH_RESET      (1 << 19)
#define EXT_PRI_POE_DC_RESET        (1 << 11)
#define EXT_PRI_LTE_RESET           (1 << 5)
#define EXT_WLAN_RESET              (1 << 4)
#define EXT_CPU_SYS_RESET           (1 << 3)
#define EXT_ESW_RESET               (1 << 1)

/* CPU MUX and USB Power Register(0x1104) */
#define FPGA_USB_PWR                (1 << 0)

/* FPGA Status and Control Regsiter(0x110C)*/
#define FPGA_USB_AND_RJ45_CON_MUX   (1 << 10)

/* FPGA Card and Power Present Reg(0x1118) */
#define FPGA_CPP_LTE0_PRESENT       (1 << 5)
#define FPGA_CPP_POE_PRESENT        (1 << 4)
#define FPGA_CPP_WLAN_PRESENT       (1 << 1)
#define FPGA_CPP_WLAN_READY         (1 << 0)

/* FPGA LED Reg(0x111C) */
#define LTE_WWAN_LED                (1 << 24)
#define LTE_SIM1_STAT_LED_OFF       (0 << 22)
#define LTE_SIM1_STAT_LED_Y         (1 << 22)
#define LTE_SIM1_STAT_LED_G         (2 << 22)
#define LTE_SIM1_STAT_LED_YG        (3 << 22)
#define LTE_SIM0_STAT_LED_OFF       (0 << 20)
#define LTE_SIM0_STAT_LED_Y         (1 << 20)
#define LTE_SIM0_STAT_LED_G         (2 << 20)
#define LTE_SIM0_STAT_LED_YG        (3 << 20)
#define LTE_GPS_STAT_LED_OFF        (0 << 18)
#define LTE_GPS_STAT_LED_Y          (1 << 18)
#define LTE_GPS_STAT_LED_G          (2 << 18)
#define POE_PRESENT_LED_OFF         (0 << 14)
#define POE_PRESENT_LED_G           (1 << 14)
#define POE_PRESENT_LED_Y           (2 << 14)
#define POE_P0_LED                  (1 << 13)
#define POE_P0_LED_OFF              (0 << 13)
#define POE_P1_LED                  (1 << 12)
#define POE_P1_LED_OFF              (0 << 12)
#define POE_P2_LED                  (1 << 11)
#define POE_P2_LED_OFF              (0 << 11)
#define POE_P3_LED                  (1 << 10)
#define POE_P3_LED_OFF              (0 << 10)
#define POE_STAT_LED                (1 << 9)
#define POE_STAT_LED_OFF            (0 << 9)
#define AUX_LED                     (1 << 5)
#define AUX_LED_OFF                 (0 << 5)
#define MICRO_USB_LED               (1 << 4)
#define MICRO_USB_LED_OFF           (0 << 4)
#define USB_LED                     (1 << 3)
#define USB_LED_OFF                 (0 << 3)
#define CONSOLE_LED                 (1 << 2)
#define CONSOLE_LED_OFF             (0 << 2)
#define VPN_OK_LED                  (1 << 1)
#define VPN_OK_LED_OFF              (0 << 1)

/* FPGA LTE RSSI LED Reg(0x1120) */
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
#define EIPR_MB_THERM_INTR            (13)
#define MB_THERM_INTERRUPT_PENDING    (1 << EIPR_MB_THERM_INTR)
#define XDSL_INTERRUPT_PENDING        (1 << 4)
#define POE_FPGA_INTR_PENDING         (1 << 3)

/* Force External Interrupt Reg(0x1130) */
#define USB_SW_FPGA_OC_F_INTR       (1 << 16)
#define I2C_CTRL_F_INTR             (1 << 15)
#define SPI_CTRL_F_INTR             (1 << 14)
#define THEM_FPGA_F_INTR            (1 << 13)
#define SFP_TX_FAULT_F_INTR         (1 << 12)
#define SFP_LOSS_SIGNAL_F_INTR      (1 << 11)
#define SMI0_DETECT_F_INTR          (1 << 9)
#define SMI1_DETECT_F_INTR          (1 << 8)
#define WAN_GE0_PHY_F_INTR          (1 << 7)
#define GE_SWITCH_PHY_F_INTR        (1 << 6)
#define WAN_GE1_PHY_F_INTR          (1 << 5)
#define XDSL_F_INTR                 (1 << 4)
#define POE_F_INTR                  (1 << 3)
#define MGMT_PHY_F_INTR             (1 << 2)
#define UART0_F_INTR                (1 << 1)
#define XDSL_DYING_GASP_F_INTR      (1 << 0)

/* SFP status and control Reg(0x1134) */
#define SFP_SC_MODULE_DEF           (1 << 1)
#define SFP_SC_TX_DIS               (1 << 0)

/* LTE control Reg(0x1138) */
#define GPS_DR_SYNC_STATUS          (1 << 8)
#define LTE_PRI_POWER_EN_CTL        (1 << 6)
#define EXT_PRI_LTE_WDIS_2_RESET    (1 << 5)
#define EXT_PRI_LTE_WDIS_1_RESET    (1 << 4)
#define LTE_USB_MUX_SEL_CTL         (1 << 3)
#define LTE_USB_MUX_DISABLE         (1 << 2)
#define LTE_PRI_MODEM_EN_CTL        (1 << 0)

/* SIM status and control Reg(0x113C) */
#define LTE_CONNECTOR_SIM1_PRESENT_AFTER_A9         (1 << 9)
#define LTE_CONNECTOR_SIM0_PRESENT_AFTER_A9         (1 << 8)
#define LTE_CONNECTOR_1_SIM_DETECT                  (1 << 8)
#define LTE_SIM_POWER_EN_PRI        (1 << 5)
#define LTE_SIM_1_PRESENT_DECTECT   (1 << 4)
#define LTE_SIM_0_PRESENT_DECTECT   (1 << 3)
#define LTE_SIM_SOCKET_EN           (1 << 2)
#define LTE_SIM_SOCKET_SEL          (1 << 1)

/* xDSL status and control Reg(0x1140) */
#define DSL_FPGA_EXP_PRI_RDY        (1 << 0)

/* WiFi */
#define EXT_WLAN_RESET              (1 << 4)  /* FPGA pin_L15 "WLAN_RST_L" */
#define EXT_WLAN_MODE               (1 << 7)
#define WLAN_MODULE_STATUS          (1 << 0) /* FPGA pin_E6 "WLAN_READY" */
#define WLAN_MODULE_PRESENT         (1 << 7) /* FPGA pin_B16 "WLAN_PRSNT_L" */
#define WLAN_MODULE_PRESENT_AFTER_A9     (1 << 7) /* FPGA pin_B16 "WLAN_PRSNT_L" */
/* Card and Power Present Register - 0x1118 */
#define FPGA_CARD_PWR_PRE_REG                       0x1118

/* AIKIDO Register - 0x2004 */
#define FPGA_AIKIDO_REG                       0x2004
#define FPGA_AIKIDO_REG_PATTERN               0x1
#define FPGA_AIKIDO_REG_TEST_ROUND            2

/* AIKIDO Mail Box Register - 0xC000 */
#define FPGA_AIKIDO_MBX_REG                   0xc000
#define FPGA_AIKIDO_MBX_INTCRTL_REG           0xc004
#define FPGA_AIKIDO_MBX_INTSTAT_REG           0xc010
#define FPGA_AIKIDO_MBX_H2M_FLAGS_REG         0xc020
#define FPGA_AIKIDO_MBX_DPRAM_REG             0xE000

#define MBX_DPRAM_OFFSET_FOUR                 0x4
#define MBX_EN_FLAGS_OR                       0x8
#define MBX_FLAGS_OR                          0x8
#define MBX_H2M_FLAGS                         0xFF
#define MBX_M2H_FLAGS                         0xFF
#define MBX_SCC_ID_1                          0x80040011
#define MBX_SCC_ID_2                          0x6A008080
#define MBX_GET_SCC_ID_CMD                    0x9e000061

/*
 * Externs
 */
extern reg_info_t_ext plat_fpga_reg_ext;
extern int            diag_moka_fpga_util(int);
extern int            fpga_reset_32_api(uint, uint, uint, uint);
extern int            fpga_read_32_reg(uint, uint *);
extern int            fpga_write_32_reg(uint, uint);
extern int            is_sfp_present(void);
extern int            sfp_tx_enable_switch(int);
extern int            plat_get_boardtype(uint *);
extern boolean        plat_fpga_check_dev_present(uint);
extern int  show_usb_console_to_uart_connectivity(int);
extern int  getdec_answer(char *, uint, uint, uint);
extern int diag_led_ctrl_util(int);

extern int diag_check_ext_intr_no_pending(int);
extern int diag_check_ext_intr_pending(int);

extern int diag_check_esw_ext_no_intr_pending(void);
extern int diag_check_esw_ext_intr_pending(void);

#endif   /* __PLATFORM_FPGA_H__ */

/*-------------------------------------------------
 * $Log: diag_moka_fpga_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
