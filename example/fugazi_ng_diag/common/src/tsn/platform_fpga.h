/* $Id: platform_fpga.h,v 1.13 2019/03/07 09:51:32 lucywang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_fpga.h,v $ 
 *------------------------------------------------------------------
 * 
 * Filename   : platform_fpga.h
 * Description: Header file of TSN FPGA Diag.
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __PLATFORM_FPGA_H__
#define __PLATFORM_FPGA_H__

#include "defs.h"
#include "common_utils.h"

#define FPGA_MAX_REG_ADDR       (0x1FFFF)

/* Common */
#define TSN_M_P1A_FPGA_VER      0x16071901

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

/* Platform Cookie controller type */
#define C1101_4P_CONTROL_TYPE                        0x1032
#define C1101_4PL_CONTROL_TYPE                       0x1030
#define C1101_4PLW_CONTROL_TYPE                      0x102F
#define C1109_2PGB_CONTROL_TYPE                      0x1033  
#define C1109_2PNA_CONTROL_TYPE                      0x1034  
#define C1109_2PVZ_CONTROL_TYPE                      0x1035  
#define C1109_2PJN_CONTROL_TYPE                      0x1036  
#define C1109_2PAU_CONTROL_TYPE                      0x1037  
#define C1109_2PIN_CONTROL_TYPE                      0x1038  
#define C1109_4PL_CONTROL_TYPE                       0x1054
#define C1109_4PLW_CONTROL_TYPE                      0x1052
#define C1111X_8P_CONTROL_TYPE                       0x104F
#define C1118_8P_CONTROL_TYPE                        0x0d5a
#define C951_4P_CONTROL_TYPE                         0x1096
#define C959_2PUS_CONTROL_TYPE                       0x1098
#define C959_2PGB_CONTROL_TYPE                       0x1099
#define C959_2PVZ_CONTROL_TYPE                       0x109b
#define C959_2PIN_CONTROL_TYPE                       0x10C1

typedef struct tsn_reg_bit {
    char           *name;
    unsigned int   offset;
} tsn_reg_bit_t;

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
#define TSN_W_LTE                   (1 << 0)
#define TSN_W_WIFI                  (1 << 1)
#define TSN_W_GE1                   (1 << 2)

/* FPGA_LPC_SKUFEATURE_REG Reg(0x00C0)*/
#define STAR_SKUFEATURE_MASK        0xC0 
#define STAR_SKUFEATURE_C1101P      0x00          
#define STAR_SKUFEATURE_C1101E2E    0x40            
#define STAR_SKUFEATURE_C1109_4P    0x80          
#define STAR_SKUFEATURE_C1109_2P    0xC0          
#define SUPERNOVA_SKUFEATURE_C959_2P    0xC0
#define SUPERNOVA_SKUFEATURE_C951_4P    0x40
#define FPGA_SKUID_TSN_GSHDSL       (0x7 << 3)

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

/* Board Type Register(0x1080) */
#define FPGA_BTYPE_TSN              0x1D
#define FPGA_BTYPE_STAR             0x3D 
#define FPGA_BTYPE_SUPERNOVA        0x4B 
#define FPGA_BTYPE_SUB_HIGH_SHIFT   0x14  /* Shift 20 */
#define FPGA_BTYPE_SUB_HIGH_MASK    (0xFF << 24)
#define FPGA_BTYPE_SUB_LOW_MASK     (0xF)


#define FPGA_SKUID_TSN_H            (1 << 5)
#define FPGA_SKUID_ANNEX_M          (1 << 4)
#define FPGA_SKUID_ANNEX_A          (1 << 3)
#define FPGA_SKUID_GE0              (1 << 2)
#define FGPA_SKUID_WIFI             (1 << 1)
#define FPGA_SKUID_LTE              (1 << 0)

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
#define MB_THERM_INTERRUPT_PENDING  (1 << 13)
#define XDSL_INTERRUPT_PENDING      (1 << 4)
#define POE_FPGA_INTR_PENDING       (1 << 3)

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
#define LTE_SAFE_PWR_REMOVE         (1 << 9)
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

/* For External Interrupt Pending Register */
#define PENDING_BIT_GE1           1<<7 /* 0:no pending, 1:pending */
#define PENDING_BIT_ESW           1<<6 /* 0:pending, 1:no pending */
#define PENDING_BIT_GE0           1<<5 /* 0:no pending, 1:pending */
#define PENDING_BIT_POE           1<<3 /* 0:no pending, 1:pending */

/* Interrupt REG*/
#define ESW_GLOBAL1_REG                0x1B
#define ESW_GLOBAL2_REG                0x1C
#define ESW_GLOBAL_CONTROL_REG         0x04
#define ESW_INT_MASK_REG               0x01
#define ENABLE_ESW_GLOBAL_CONTROL_REG  0x41FF
#define ENABLE_ESW_INT_MASK_REG        0xF81F
#define GLOBAL_CTRL_REG                0x4 /* belong to global 1 */
#define EEINT_ENABLE                   0x0 /* belong to GLOBAL_CTRL_REG */
#define EEINT_DISABLE                  0x1 /* belong to GLOBAL_CTRL_REG */

/*
 * Externs
 */
extern int has_plug_slot(int);
extern reg_info_t_ext tsn_fpga_reg_ext;
extern int            tsn_all_green_leds_on(int);
extern int            tsn_all_yellow_leds_on(int);
extern int            tsn_all_leds_off(int);
extern int            tsn_fpga_utils(int);
extern int            fpga_reset_32_api(uint, uint, uint, uint);
extern int            fpga_read_32_reg(uint, uint *);
extern int            fpga_write_32_reg(uint, uint);
extern int            is_sfp_present(int *);
extern int            sfp_tx_enable_switch(int);
extern int            fpga_reg_rd_util(int);
extern int            fpga_reg_wr_util(int);
extern int            tsn_get_boardtype(uint *);
extern int            this_is_tsn_h_sku(void);
extern boolean        tsn_fpga_check_dev_present(uint);
extern int            this_is_tsn_dsl_annex_sku(void);
extern int  this_is_tsn(void);
extern int  this_is_tsn_gshdsl_sku(void);
extern int  this_is_star(void);
extern int  this_is_supernova(void);
extern int  this_is_not_star(void);
extern int  this_is_not_supernova(void);
extern int  this_is_star_c1101p(void);
extern int  this_is_star_c1101e2e(void);
extern int  this_is_supernova_c951_4p(void);
extern int  this_is_star_c1109_4p(void);
extern int  this_is_star_c1109_2p(void);
extern int  this_is_supernova_c959_2p(void);
extern int  usb_console_to_uart(int);
extern int  tsn_show_fpga_ver(int);
extern int  check_fpga_sku_info(void);
extern int  star_io_interface_show(void);
extern int  this_is_star_with_sirius_fpga(void);
extern int  usb_console_to_uart(int);
extern int  getdec_answer(char *, uint, uint, uint);
extern int  diag_check_ge_ext_intr_no_pending(int);
extern int  diag_check_ge_ext_intr_pending(int);
extern int  diag_check_esw_ext_intr_pending(void);
extern int  diag_check_esw_ext_no_intr_pending(void);

#endif   /* __PLATFORM_FPGA_H__ */

/*-------------------------------------------------
 * $Log: platform_fpga.h,v $
 * Revision 1.13  2019/03/07 09:51:32  lucywang
 * [Supernova] PID changed : C1101L-4P --> C951-4P, C1109L-2P --> C959-2P
 *
 * Revision 1.12  2019/01/24 01:07:22  letsai
 * Add Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.11  2019/01/18 05:54:47  yungchen
 * Merge Supernova branch to the main trunk (CSCvn79871)
 *
 * Revision 1.10  2018/11/23 08:49:51  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.9.36.1  2018/10/15 06:53:07  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.9  2018/05/15 09:37:32  steja
 * CSCvj38863: Enhanced LED single test utility
 *
 * Revision 1.8  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 * Revision 1.7  2018/04/15 22:03:30  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.6.2.1  2018/04/02 09:14:26  palin2
 * Added Vulcan controller type and SKU info to platform SKU table.
 *
 * Revision 1.6  2018/03/27 12:46:38  hondwang
 * Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER
 *
 * Revision 1.5.2.1  2018/02/24 06:22:39  iachang
 * Support Aikido Mailbox Test
 *
 * Revision 1.5  2018/02/09 09:56:55  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.4  2018/01/23 11:38:19  steja
 * Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)
 *
 * Revision 1.3.6.1  2018/01/20 06:27:24  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.3  2017/12/01 13:45:20  palin2
 *
 * Added support RESET button test (CSCvg96921).
 * Revision 1.2.12.2  2017/12/19 06:48:56  shjung
 * Updated the SKUs for new PID
 *
 * Revision 1.2.4.10  2017/12/08 11:20:27  hondwang
 * Add console link with USB or RJ45 utility
 *
 * Revision 1.2.4.9  2017/12/05 02:47:09  lucywang
 * Sync from TSN trunk : Added support RESET button test (CSCvg96921).
 *
 * Revision 1.2.4.8  2017/11/22 09:45:46  hondwang
 * Fix demo SKU and menu show
 *
 * Revision 1.2.4.7  2017/11/20 07:54:32  lucywang
 * Changed PID to C1101/C1109-2P/C1109-4P
 *
 * Revision 1.2.4.6  2017/11/06 06:28:16  lucywang
 * Added GPS pin test for on-board WP module
 *
 * Revision 1.2.4.5  2017/10/07 02:12:40  hondwang
 * Add FPGA SKU check function to double confirm FPGA info
 *
 * Revision 1.2.4.4  2017/09/29 23:01:30  hondwang
 * Show FPGA version with sys_info function
 *
 * Revision 1.2.4.3  2017/09/28 21:46:11  hondwang
 * Add Moka and Sirius FPGA interrupt 1ms(HW suggest) wait
 *
 * Revision 1.2.4.2  2017/09/09 00:47:48  hondwang
 * Add C949-4P support with MB,Wifi,LTE EM
 *
 * Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:48  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.5  2017/07/25 08:31:55  steja
 * 1. Remove unused code.
 * 2. Verified before check-in
 *
 * Revision 1.1.6.4  2017/07/24 14:14:10  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.3  2017/07/21 10:46:03  steja
 * Update based on code review comment
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.11.2.6  2017/07/08 07:27:26  steja
 * Code Clean up
 *
 * Revision 1.1.4.11.2.5  2017/05/17 01:17:53  palin2
 * Updated GE WAN mapping number with team's decision.
 * (GE0: GE WAN with SFP; GE1: 2nd GE WAN)
 *
  * Revision 1.1.4.11.2.4.2.6  2017/08/04 03:24:54  hondwang
 * Add delay 1 second delaytime with SW un-Resetn to fix SW init issue
 *
 * Revision 1.1.4.11.2.4.2.5  2017/07/28 01:23:17  hondwang
 * Modify SKU feature follow FPGA change
 *
 * Revision 1.1.4.11.2.4.2.4  2017/07/26 14:04:53  hondwang
 * Add USB console check function
 *
 * Revision 1.1.4.11.2.4.2.3  2017/07/03 13:16:39  hondwang
 * fix E2E LED, I2C and GE phy testing fail
 *
 * Revision 1.1.4.11.2.4.2.2  2017/06/30 13:37:55  hondwang
 * Fix Star platform I2c scan issue and add this_is_star function
 *
 * Revision 1.1.4.11.2.4.2.1  2017/06/16 06:52:39  tirawan
 * Foxconn Pluggable FPGA I2C Read/Write function correction during the bring up
 *
 * Revision 1.1.4.11.2.4  2017/05/08 11:52:43  steja
 * Fix Enumerate LTE USB (Mini USB) under Rommong (CSCve33718)
 *
 * Revision 1.1.4.11.2.3  2017/04/10 10:53:47  palin2
 * Added RSSI related LEDs into LED test.
 *
 * Revision 1.1.4.11.2.2  2017/02/23 11:03:16  palin2
 * Updated code based on FPGA changes. These updates are verified on P2A TSN.
 *
 * Revision 1.1.4.11.2.1  2017/02/17 07:09:47  steja
 * Update LTE code based on latest FPGA 170215
 *
 * Revision 1.1.4.11  2016/11/15 01:20:02  palin2
 * Added PoE PSU to FPGA interrupt test.
 *
 * Revision 1.1.4.10  2016/10/07 13:07:56  steja
 * 1. Add Check xDSL sku type
 * 2. Support Annex B
 *
 * Revision 1.1.4.9  2016/10/04 06:39:08  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.8  2016/09/28 04:36:15  palin2
 * Added CPU to ESW PHY MAC loopback test.
 *
 * Revision 1.1.4.7  2016/09/07 15:12:52  steja
 * Add wifi temperature interrupt test
 *
 * Revision 1.1.4.6  2016/08/23 08:14:17  steja
 * Add MB Temperature interrupt test
 *
 * Revision 1.1.4.5  2016/08/15 13:02:26  steja
 * Add utility for LTE switch to USB external port
 *
 * Revision 1.1.4.4  2016/07/22 13:04:37  palin2
 * Added function to check DC present.
 *
 * Revision 1.1.4.3  2016/07/17 11:15:16  palin2
 * Added function to distinguish bwteen TSN-H and TSN-M.
 *
 * Revision 1.1.4.2  2016/06/30 06:22:50  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.10  2016/06/21 12:58:52  steja
 * Add OTA out of reset utility
 *
 * Revision 1.1.2.9  2016/05/24 01:19:01  palin2
 * Added Card and Power Present Reg.(0x1118) definition.
 *
 * Revision 1.1.2.8  2016/05/20 03:03:06  leschen
 * Support wifi macro
 *
 * Revision 1.1.2.7  2016/05/18 09:03:03  steja
 * Add Platform Init
 *
 * Revision 1.1.2.6  2016/05/16 06:44:55  palin2
 * Add function to get TSN board type, and config Diag test items in menu for
 * different SKUs based on its board type info.
 *
 * Revision 1.1.2.5  2016/04/29 10:14:56  palin2
 * Updated code and added support ext. loopback test after bring up Switch.
 *
 * Revision 1.1.2.4  2016/04/26 20:48:49  palin2
 * Updated code after bring up SFP external loopback test.
 *
 * Revision 1.1.2.3  2016/04/24 12:42:56  palin2
 * 1. Updated FPGA registers map.
 * 2. Fixed FPGA force interrupt test.
 * 3. Added FPGA registers dump utility.
 *
 * Revision 1.1.2.2  2016/04/14 06:12:17  palin2
 * Updated FPGA register read/write function and register map after bring up.
 *
 * Revision 1.1.2.1  2016/03/23 03:31:11  palin2
 * Added FPGA Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */

