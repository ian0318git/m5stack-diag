 /* $Id: diag_fpga.h,v 1.6 2020/10/07 08:20:48 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_fpga.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_fpga.h
 * Description: Header file of FPGA Diag.
 *
 * Copyright (c) 2018-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_H__
#define __DIAG_FPGA_H__

#include "defs.h"
#include "types.h"
#include "common_utils.h"

/* Common */


#define PLAT_FPGA_REG_WIDTH     4
#define FPGA_REG_WIDTH_IN_BIT   (PLAT_FPGA_REG_WIDTH * 8)
#define WAITTIME_5_MS           5
#define WAITTIME_20_MS          20
#define WAITTIME_100_MS         100
#define WAITTIME_150_MS         150
#define WAITTIME_1000_MS        1000
#define WAITTIME_3500_MS        3500
#define WAITTIME_5000_MS        5000
#define DELAY_FOR_LED_TEST      1500 
#define DELAY_FOR_OPERATION     1500
#define TIMEVALUE_FOR_WATCHDOG  60


#define FPGA_RONLY    (READ_ONLY | REG_ACCESS)
#define FPGA_RW       (READ_WRITE | REG_ACCESS)

typedef struct ntclk_ {
    volatile uint32_t status;               /* 0x00*/
    volatile uint32_t pll_intr;             /* 0x04 */
    volatile uint32_t pll_ref;              /* 0x08 */
    volatile uint32_t mult5;                /* 0x0C */

#define SYNC_TRIG_OUTPUT_ENABLE             0x1000000
#define ENABLE_PRE_SCALER_DIV_5             0x400 
#define ENABLE_PRE_SCALER_DIV_3125          0x200 
#define SYNC_OUT_ENABLE                     0x100 


#define DASH_PAD(from_adr, to_adr, name) \
unsigned int name[(to_adr - from_adr)/sizeof(unsigned int)]

    volatile uint32_t sync_sm[2];           /* 0x10-0X14 */
    volatile uint32_t sync_pad[2];          /* 0x18-0x1c */
    volatile uint32_t sync_wic[3];          /* 0x20-0X28 */
    volatile uint32_t sync_pad1[2];         /* 0x2C */
    volatile uint32_t sync_vm;              /* 0x34 */
    volatile uint32_t pad2;                 /* 0x38 */
    volatile uint32_t qdphy_sync;           /* 0x3C */
    volatile uint32_t qdphy_ptp;            /* 0x40 */
    volatile uint32_t fp_sync;              /* 0x44 */
    volatile uint32_t cp_sync;              /* 0x48 */
    volatile uint32_t ptp;                  /* 0x4c */
    DASH_PAD(0x50, 0x80, pad);
    volatile uint32_t ptp_ctlsts;           /* 0x80 */
    volatile uint32_t ptp_intr_en;          /* 0x84 */
    volatile uint32_t ptp_ctr_h;            /* 0x88 */
    volatile uint32_t ptp_ctr_l;            /* 0x8C */
    volatile uint32_t ptp_h_cycle;          /* 0x90 */
    volatile uint32_t ptp_l_cycle;          /* 0x94 */
    volatile uint32_t ptp_ctr_init_h;       /* 0x98 */
    volatile uint32_t ptp_ctr_init_l;       /* 0x9c */
    volatile uint32_t ptp_rise_trigger_h;   /* 0xA0 */
    volatile uint32_t ptp_rise_trigger_l;   /* 0xA4 */
    volatile uint32_t ptp_fall_trigger_h;   /* 0xA8 */
    volatile uint32_t ptp_rall_trigger_l;   /* 0xAC */

    volatile uint32_t ptp_rise_toggle_h;    /* 0xB0 */
    volatile uint32_t ptp_rise_toggle_l;    /* 0xB4 */
    volatile uint32_t ptp_fall_toggle_h;    /* 0xB8 */
    volatile uint32_t ptp_rall_toggle_l;    /* 0xBc */

    /* Definition of SYNC_TRIG_IN/SYNC_IN Debug Reg.(0xC0) */
#define NGSM3_SYNC_TRIG_IN_STAT    0x80000
#define NGSM3_SYNC_IN_STAT         0x40000
#define NGSM1_SYNC_TRIG_IN_STAT    0x20000
#define NGSM1_SYNC_IN_STAT         0x10000
#define NGSM2_SYNC_TRIG_IN_STAT    0x08000
#define NGSM2_SYNC_IN_STAT         0x04000
#define NGWIC1_SYNC_TRIG_IN_STAT   0x02000
#define NGWIC1_SYNC_IN_STAT        0x01000
#define NGWIC2_SYNC_TRIG_IN_STAT   0x00800
#define NGWIC2_SYNC_IN_STAT        0x00400
#define NGWIC3_SYNC_TRIG_IN_STAT   0x00200
#define NGWIC3_SYNC_IN_STAT        0x00100
#define NGVM_SYNC_TRIG_IN_STAT     0x00080
#define NGVM_SYNC_IN_STAT          0x00040
#define CCPU_SYNC_IN_STAT          0x00010
#define PCH_SYNC_TRIG_IN_STAT      0x00008
#define QPCH_SYNC_IN_STAT          0x00004
#define QPCH_SYNC_TRIG_IN_STAT     0x00002
#define PTP_CTR_STAT               0x00001

    volatile uint32_t sync_dbg;             /* 0xC0 */


} ntclk_t;



/*
#define UART_RX        0
#define UART_IER    0x04
#define UART_IIR    0x08
#define UART_LCR    0x0c
#define UART_MCR    0x10
#define UART_LSR    0x14
#define UART_MSR    0x18
#define UART_SCR    0x1c

#define UART_TX     UART_RX
#define UART_FCR    UART_IIR
*/

typedef struct uart_t_ {
    volatile unsigned int dll;  /* 0 */
    volatile unsigned int dlm;  /* 4 ier*/
    volatile unsigned int fcr; /* 8 */
    volatile unsigned int lcr; /* c */
    volatile unsigned int mcr;/* 10 */
    volatile unsigned int lsr; /* 14 */
    volatile unsigned int msr; /* 18 */
    volatile unsigned int scr;
} uart_t;




/* FPGA Internal Device Reset Register (+0x08) */
#define FPGA_RP_OVLD                        0x8
#define FPGA_IN_NIOS_RST_TAKEN        0x2000000 
#define FPGA_IN_NIOS_RST              0x1000000 
#define FPGA_IN_I2C_15_RST            0x8000 
#define FPGA_IN_I2C_14_RST            0x4000 
#define FPGA_IN_I2C_13_RST            0x2000
#define FPGA_IN_I2C_12_RST            0x1000
#define FPGA_IN_I2C_11_RST            0x0800
#define FPGA_IN_I2C_10_RST            0x0400
#define FPGA_IN_I2C_9_RST             0x0200
#define FPGA_IN_I2C_8_RST             0x0100
#define FPGA_IN_I2C_7_RST             0x0080
#define FPGA_IN_I2C_6_RST             0x0040
#define FPGA_IN_I2C_5_RST             0x0020
#define FPGA_IN_I2C_4_RST             0x0010
#define FPGA_IN_I2C_3_RST             0x0008
#define FPGA_IN_I2C_2_RST             0x0004
#define FPGA_IN_I2C_1_RST             0x0002
#define FPGA_IN_I2C_0_RST             0x0001


#define FPGA_I2C_BASE                 0x30000
#define FPGA_I2C_OFFSET               0x100


#define FPGA_UART_BASE                0x20000
#define FPGA_UART_OFFSET              0x100





/****** TBD Should be removed ********/
typedef struct tabei_reg_bit {
    char           *name;
    unsigned int   offset;
} tabei_reg_bit_t;


/* Register Definition */
/* Reg Offset */
#define FPGA_SCRATCHPAD_REG          0x0

/* FPGA DEVICE RESET REG */
#define FPGA_EXT_DEVICE_RESET_REG    0x4
#define FPGA_EXT_DEVICE_RESET_10G    (1 << 11)
#define FPGA_EXT_DEVICE_RESET_GE     (1 << 0)


#define FPGA_SPI_CONTROL_REG         0x31800
#define FPGA_EXTER_DEV_RST_REG       0x00004
#define FPGA_INT_DEV_RST_REG         0x00008
#define FPGA_BOARD_TYPE_REG          0x00080
#define FPGA_MASTER_REV_REG          0x00084
#define FPGA_SLAVE_REV_REG           0x00088
#define FPGA_REVISION_REG            0x0008C
#define FPGA_PWR_SUPPLY_LED          0x00408
#define FPGA_ENV_LED                 0x00418
#define FPGA_I350_RJ45_LED           0x0042C
#define FPGA_I350_SFP_LED            0x00430
#define FPGA_HDD_LED                 0x00438
#define FPGA_WATCHDOG_REG            0x80800
#define FPGA_IOS_WATCHDOG_TIMER_REG  0x80804


#define FPGA_I2C_CTL_REG             0x30000
#define FPGA_I2C_SCRATCH_PAD         0x30004
#define FPGA_I2C_STAT_REG            0x30008
#define FPGA_I2C_STAT_MASK_REG       0x3000C
#define FPGA_I2C_SLA_ADDR_REG        0x30010
#define FPGA_I2C_SLA_SUBADDR_REG     0x30014
#define FPGA_I2C_BIT_BANG_REG        0x30018
#define FPGA_I2C_BYTE_COUNT_REG      0x3001C
#define FPGA_I2C_DATA_FIFO_REG       0x30040
#define FPGA_I2C_DATA_RW_PTR_REG     0x30044

#define FPGA_SPI_CTRL_REG            0x31A00
#define FPGA_SPI_STAT_REG            0x31A04
#define FPGA_SPI_RD_SIZE_REG         0x31A08
#define FPGA_SPI_RW_DATA_REG         0x31A0C
#define FPGA_SPI_OP_ADDR_REG         0x31A10

#define FPGA_POE_STATE_REG           0x32120

#define FPGA_RECONFIG_CTRL_REG       0xC00

#define POE_PRESENT_MASK             0x4
#define POE_POWER_SUPPLY_PRESENT     0x4

/* (0x001C) */
#define FPGA_ACT2_RST_L             (1 << 2)

/* FPGA External Device Reset Reg(0x804) */
#define I2C_CONTROLLER_RESET        (1 << 0)
#define I2C_CONTROLLER_OUT_RESET    (0 << 0)

/* FPGA Internal Device Reset Reg(0x808) */
#define INT_I2C_RESET               (1 << 0)

/* FPGA LED Reg */
#define TABEI_LED_OFF               (0 << 0)
#define TABEI_HDD_LED_MASK           0x4F
#define ENV_LED_YELLOW               0x4000
#define ENV_LED_GREEN                0x8000
#define PWR_SUP_LED_YELLOW           0x5
#define PWR_SUP_LED_GREEN            0xA
#define RJ45_LED_ON                  0x0099
#define RJ45_PORT0_SPEED_LED         0x0001
#define RJ45_PORT0_LINK_LED          0x0008
#define RJ45_PORT1_SPEED_LED         0x0010
#define RJ45_PORT1_LINK_LED          0x0080
#define SFP_LED_GREEN                0x0099
#define SFP_LED_YELLOW               0x0044
#define SFP_LED_PORT0_SPD_GREEN      0x0001
#define SFP_LED_PORT0_EN_GREEN       0x0008
#define SFP_LED_PORT1_SPD_GREEN      0x0010
#define SFP_LED_PORT1_EN_GREEN       0x0080
#define SFP_LED_PORT0_EN_YELLOW      0x0004
#define SFP_LED_PORT1_EN_YELLOW      0x0040
#define HDD_LED_GREEN                0xA0
#define HDD_LED_YELLOW               0xB0
#define TABEI_LED_MASK              (0 << 0)

#define FPGA_BTYPE_PROMETHIUM        0x6
#define FPGA_BTYPE_PROMETHIUM_L      0x5
#define FPGA_BTYPE_TABEI_L           0x7
#define FPGA_BTYPE_FORTNITE          0x3
#define FPGA_BTYPE_SKU_BIT           0x70000000
#define FPGA_BTYPE_SKU_BIT_SHIFT     0x1C

#define FPGA_UPGRADE_HEADER_NOTE_READ 0x0
#define FPGA_UPGRADE_RECONF_FSM_RST   0x2


#define FPGA_IOS_WDT_COUNT_MASKi     0xFFFFF
#define FPGA_IOS_WDT_ENABLE_KEY      0xAC000000
#define FPGA_IOS_WDT_DISABLE_KEY     0xCA000000




/****** TBD ********/
/* Sync Registers */
#define NET_CLK_PTP_CONF_REG_OFF        (0x10100)
#define FPGA_NIM1_STNC_TRIG_CTRL        (NET_CLK_PTP_CONF_REG_OFF + 0x20)

/* UART Controller Register */
#define PLUG_UART_CTRL0_OFFSET          (0x20000)
#define PLUG_UART_CTRL2_OFFSET          (PLUG_UART_CTRL0_OFFSET + 0x200)
#define PLUG_UART_CTRL3_OFFSET          (PLUG_UART_CTRL0_OFFSET + 0x300)
#define PLUG_UART_CTRL6_OFFSET          (PLUG_UART_CTRL0_OFFSET + 0x600)
#define PLUG_UART_CTRL8_OFFSET          (PLUG_UART_CTRL0_OFFSET + 0x800)

#define PLUG_UART_RBR_THR_DLL_OFFSET    (0x00)
#define PLUG_UART_IER_DLM_OFFSET        (0x04)
#define PLUG_UART_IIR_FCR_OFFSET        (0x08)
#define PLUG_UART_LCR_OFFSET            (0x0C)
#define PLUG_UART_MCR_OFFSET            (0x10)
#define PLUG_UART_LSR_OFFSET            (0x14)
#define PLUG_UART_MSR_OFFSET            (0x18)
#define PLUG_UART_SCR_OFFSET            (0x1C)

/* UART LCR Register */
#define PLUG_UART_LCR_LAB_BIT           (7)
#define PLUG_UART_LCR_BC_BIT            (6)
#define PLUG_UART_LCR_SP_BIT            (5)
#define PLUG_UART_LCR_EPSEL_BIT         (4)
#define PLUG_UART_LCR_PE_BIT            (3)
#define PLUG_UART_LCR_SB_BIT            (2)

/* PLUG I2C Controller Offset */
#define PLUG_I2C_CTRL0_OFFSET            (0x30000)
#define PLUG_I2C_CTRL1_OFFSET            (PLUG_I2C_CTRL0_OFFSET + 0x100)
#define PLUG_I2C_CTRL2_OFFSET            (PLUG_I2C_CTRL0_OFFSET + 0x200)
#define PLUG_I2C_CTRL4_OFFSET            (PLUG_I2C_CTRL0_OFFSET + 0x500)
#define PLUG_I2C_CTRL5_OFFSET            (PLUG_I2C_CTRL0_OFFSET + 0x400)
#define PLUG_I2C_CTRL10_OFFSET           (PLUG_I2C_CTRL0_OFFSET + 0xA00)
#define PLUG_I2C_CTRL12_OFFSET           (PLUG_I2C_CTRL0_OFFSET + 0xC00)
#define PLUG_I2C_CTRL13_OFFSET           (PLUG_I2C_CTRL0_OFFSET + 0xD00)
#define PLUG_I2C_CTRL16_OFFSET           (PLUG_I2C_CTRL0_OFFSET + 0x1000)
#define PLUG_I2C_CTRL20_OFFSET           (PLUG_I2C_CTRL0_OFFSET + 0x1400)

/* I2C Controller Register */
#define PLUG_I2C_MSTR_CTRL_OFFSET       (0x00)
#define PLUG_I2C_SCEACH_PAD_OFFSET      (0x04)
#define PLUG_I2C_MSTR_STS_OFFSET        (0x08)
#define PLUG_I2C_MSTR_STS_MASK_OFFSET   (0x0C)
#define PLUG_I2C_MSTR_SLAVE_ADDR_OFFSET (0x10)
#define PLUG_I2C_MSTR_SUBSL_ADDR_OFFSET (0x14)
#define PLUG_I2C_BITBANG_OFFSET         (0x18)
#define PLUG_I2C_BYTE_CNT_OFFSET        (0x1C)
#define PLUG_I2C_DATA_FIFO_OFFSET       (0x40)
#define PLUG_I2C_DATA_FIFO_RWPTR_OFFSET (0x44)

#define PLUG_I2C_CTRL_CLK_25             0x00000000
#define PLUG_I2C_CTRL_CLK_50             0x00000004
#define PLUG_I2C_CTRL_SLV_ADDR_7         0x00000000
#define PLUG_I2C_CTRL_SLV_ADDR_10        0x00000020
#define PLUG_I2C_CTRL_SPEED_NORMAL_100   0x00000000
#define PLUG_I2C_CTRL_SPEED_NORMAL_400   0x00000040
#define PLUG_I2C_CTRL_SPEED_DMA_400      0x00000000
#define PLUG_I2C_CTRL_SPEED_DMA_HI       0x00000040
#define PLUG_I2C_CTRL_WR_MODE            0x00000000
#define PLUG_I2C_CTRL_RD_MODE            0x00000080
#define PLUG_I2C_CTRL_SUB_ADDR_DIS       0x00000000
#define PLUG_I2C_CTRL_SUB_ADDR_1BYTE     0x01000000
#define PLUG_I2C_CTRL_SUB_ADDR_2BYTE     0x02000000
#define PLUG_I2C_CTRL_SUB_ADDR_3BYTE     0x03000000
#define PLUG_I2C_CTRL_SOFT_RESET         0x04000000
#define PLUG_I2C_CTRL_CHK_SLV_ACK        0x00000000
#define PLUG_I2C_CTRL_IGNOR_SLV_ACK      0x08000000


#define FPGA_ENV_FAN_OFFSET     0x32200
extern unsigned long get_platform_env_fan_base();
extern int get_fan_status(void);
extern void enable_fan_ctrl(uint);
extern void disable_fan_ctrl(uint);
extern uint fan_pwm_slope_read(void);
extern void fan_pwm_slope_write(int);
extern uint tachometer_rps_read(int);
extern uint fan_speed_rd(int);
extern void fan_speed_wr(int, uint);

typedef struct env_fan_t_ {
    volatile unsigned int status;        /* 0x00: Environmental Fan Status 1 */
#define AGGRE_ALERT              0x4000
#define FAN4_ROTATION            0x800
#define FAN3_ROTATION            0x400
#define FAN2_ROTATION            0x200
#define FAN1_ROTATION            0x100
#define NEBS_FAN_TRAY_PRESENT      0x2
#define FAN_TRAY_PRESENT           0x1
    volatile unsigned int ctrl;          /* 0x04: Environmental Fan Control */

#define FAN4_OPTION     0x800
#define FAN3_OPTION     0x400
#define FAN2_OPTION     0x200
#define FAN1_OPTION     0x100

    volatile unsigned int pwm_slope;     /* 0x08: Fan PWM Slope */
    volatile unsigned int pad0;          /* 0x0C */
    volatile unsigned int tach_rps1;     /* 0x10: Fan1 Tachometer RPS RD only */
    volatile unsigned int tach_rps2;     /* 0x14: Fan2 Tachometer RPS RD only */
    volatile unsigned int tach_rps3;     /* 0x18: Fan3 Tachometer RPS RD only */
    volatile unsigned int tach_rps4;     /* 0x1C: Fan4 Tachometer RPS RD only */
    volatile unsigned int speed1;        /* 0x20: Fan1 Speed */
    volatile unsigned int speed2;        /* 0x24: Fan2 Speed */
    volatile unsigned int speed3;        /* 0x28: Fan3 Speed */
    volatile unsigned int speed4;        /* 0x2C: Fan4 Speed */
#define FAN_SPD_0PER_PWM                0x0
#define FAN_SPD_35PER_PWM               0x2BC /* 35% PWM duty cycle */
#define FAN_SPD_100PER_PWM              0x7D0 /* 100% PWM duty cycle */
} env_fan_t ;
typedef enum {
     FAN_NO_1 = 1,
     FAN_NO_2,
     FAN_NO_3,
     FAN_NO_4,
} fan_num_env_fan;

/* PLUG Module Register */
#define PLUG_MODULE_OFFSET              (0x32800)
#define FPGA_PLUG1_STSCTL_REG           (PLUG_MODULE_OFFSET + 0x10)
#define FPGA_PLUG1_INTEN_REG            (PLUG_MODULE_OFFSET + 0x14)
#define FPGA_PLUG1_DEB_REG              (PLUG_MODULE_OFFSET + 0x18)
#define FPGA_PLUG2_STSCTL_REG           (PLUG_MODULE_OFFSET + 0x60)
#define FPGA_PLUG2_INTEN_REG            (PLUG_MODULE_OFFSET + 0x64)
#define FPGA_PLUG2_DEB_REG              (PLUG_MODULE_OFFSET + 0x68)
#define PLUG_MISCELLANEOUS_REG          (PLUG_MODULE_OFFSET + 0xF0)

/* PLUG Status / Control Register */
#define PLUG_PWR_OK                     (0x10000)
#define PLUG_FLT_INTR                   (0x400)
#define PLUG_INS_INTR                   (0x200)
#define PLUG_RMV_INTR                   (0x100)
#define PLUG_PRSNT                      (0x80)
#define PLUG_I2C_OK                     (0x40)
#define PLUG_UART_TX                    (0x20)
#define PLUG_PWR_EN                     (0x10)
#define PLUG_RESET                      (2)
#define PLUG_I2C_RESET                  (1)

#define PLUG_I2C_RESET_BIT              (0)
#define PLUG_RESET_BIT                  (1)
#define PLUG_PWR_EN_BIT                 (4)
#define PLUG_UART_TX_EN_BIT             (5)
#define PLUG_PWR_OK_FLT_INTR_BIT        (10)

#define PLUG_DBG_LED_ON                  (0xFF)
#define PLUG_DBG_LED_OFF                 (0)
/* NGIO Module Register */
#define NGIO_MODULE_OFFSET              (0x32000)
#define FPGA_MODULE_DEB_CTL_REG         (NGIO_MODULE_OFFSET + 0x00)
#define FPGA_NIM1_STSCTL_REG            (NGIO_MODULE_OFFSET + 0x50)
#define FPGA_NIM1_INTEN_REG             (NGIO_MODULE_OFFSET + 0x54)
#define FPGA_NIM1_DEB_REG               (NGIO_MODULE_OFFSET + 0x58)
#define FPGA_NIM2_STSCTL_REG            (NGIO_MODULE_OFFSET + 0x60)
#define FPGA_NIM2_INTEN_REG             (NGIO_MODULE_OFFSET + 0x64)
#define FPGA_NIM2_DEB_REG               (NGIO_MODULE_OFFSET + 0x68)
#define FPGA_NIM3_STSCTL_REG            (NGIO_MODULE_OFFSET + 0x70)
#define FPGA_NIM3_INTEN_REG             (NGIO_MODULE_OFFSET + 0x74)
#define FPGA_NIM3_DEB_REG               (NGIO_MODULE_OFFSET + 0x78)
#define FPGA_PCIE_STS_CTL_REG           (NGIO_MODULE_OFFSET + 0x90)
#define FPGA_M2_CTLSTS_REG              (NGIO_MODULE_OFFSET + 0xA0)
#define FPGA_M2_INT_EN_REG              (NGIO_MODULE_OFFSET + 0xA4)
#define FPGA_M2_DEB_REG                 (NGIO_MODULE_OFFSET + 0xA8)

/* FPGA_M2_CTLSTS_REG register */
#define FPGA_M2_MODULE_PRESENT          (0x10)
#define FPGA_M2_REMOVAL_INTR            (0x100)
#define FPGA_M2_INSTERT_INTR            (0x200)
#define FPGA_M2_DEVICE_PRESENT_MASK     (0x30000)
#define FPGA_M2_SATA_PRESENT            (0x00000)
#define FPGA_M2_PCIE_PRESENT            (0x10000)
#define FPGA_M2_USB_PRESENT             (0x20000)
#define FGPA_M2_NO_DEVICE               (0xA5A5)  /* Magic number for no M.2 device */

/* PLUG Module Register */
#define PLUG_MODULE_OFFSET              (0x32800)
#define FPGA_PLUG1_STSCTL_REG           (PLUG_MODULE_OFFSET + 0x10)
#define FPGA_PLUG1_INTEN_REG            (PLUG_MODULE_OFFSET + 0x14)
#define FPGA_PLUG1_DEB_REG              (PLUG_MODULE_OFFSET + 0x18)
#define FPGA_PLUG2_STSCTL_REG           (PLUG_MODULE_OFFSET + 0x60)
#define FPGA_PLUG2_INTEN_REG            (PLUG_MODULE_OFFSET + 0x64)
#define FPGA_PLUG2_DEB_REG              (PLUG_MODULE_OFFSET + 0x68)
#define PLUG_MISCELLANEOUS_REG          (PLUG_MODULE_OFFSET + 0xF0)

/* PLUG Status / Control Register */
#define PLUG_PWR_OK                     (0x10000)
#define PLUG_FLT_INTR                   (0x400)
#define PLUG_INS_INTR                   (0x200)
#define PLUG_RMV_INTR                   (0x100)
#define PLUG_PRSNT                      (0x80)
#define PLUG_I2C_OK                     (0x40)
#define PLUG_UART_TX                    (0x20)
#define PLUG_PWR_EN                     (0x10)
#define PLUG_RESET                      (2)
#define PLUG_I2C_RESET                  (1)

#define PLUG_I2C_RESET_BIT              (0)
#define PLUG_RESET_BIT                  (1)
#define PLUG_PWR_EN_BIT                 (4)
#define PLUG_UART_TX_EN_BIT             (5)
#define PLUG_PWR_OK_FLT_INTR_BIT        (10)

#define PLUG_DBG_LED_ON                  (0xFF)
#define PLUG_DBG_LED_OFF                 (0)

/*
 * Control register bit position left shift value
 */
#define L_SHFT_PLUG_I2C_CTRL_EN                 0
#define L_SHFT_PLUG_I2C_CTRL_CLK_SEL            2
#define L_SHFT_PLUG_I2C_CTRL_SLV_EXT_ADDR_MODE  5
#define L_SHFT_PLUG_I2C_CTRL_SPEED              6
#define L_SHFT_PLUG_I2C_CTRL_RW                 7
#define L_SHFT_PLUG_I2C_CTRL_BYTE_LEN           8
#define L_SHFT_PLUG_I2C_CTRL_SUB_ADDR_EN        24  //16
#define L_SHFT_PLUG_I2C_CTRL_SOFT_RESET         26  //18
#define L_SHFT_PLUG_I2C_CTRL_SLV_ACK_MSK        27  //19
#define L_SHFT_PLUG_I2C_CTRL_MUX                29

/*
 * Contrl register bit field values
 */
#define PLUG_I2C_CTRL_DISABLE            0
#define PLUG_I2C_CTRL_NORMAL             1
#define PLUG_I2C_CTRL_DMA                2
#define PLUG_I2C_CTRL_BITBANG            3

/*
 * Status register bit mask
 */
#define MSK_PLUG_I2C_STAT_NOT_ACTIVE      0x00000001
#define MSK_PLUG_I2C_STAT_BUS_ERR         0x00000002
#define MSK_PLUG_I2C_STAT_NO_SLV          0x00000004
#define MSK_PLUG_I2C_STAT_SUB_ADDR_NACK   0x00000008
#define MSK_PLUG_I2C_STAT_STD_DONE        0x00000010
#define MSK_PLUG_I2C_STAT_DATA_NACK       0x00000020
#define MSK_PLUG_I2C_STAT_FIFO_UNDER      0x00000040
#define MSK_PLUG_I2C_STAT_FIFO_OVER       0x00000080

#define PLUG_I2C_BITBANG_SCL_DRIVER       0x00000001
#define PLUG_I2C_BITBANG_SDA_DRIVER       0x00000002
#define PLUG_I2C_BITBANG_SCL_IN           0x00000004
#define PLUG_I2C_BITBANG_SDA_IN           0x00000008

/* Sync Registers */
#define NET_CLK_PTP_CONF_REG_OFF        (0x10100)
#define FPGA_NIM1_STNC_TRIG_CTRL        (NET_CLK_PTP_CONF_REG_OFF + 0x20)
#define FPGA_SYNC_DEBUG_REG_OFF         (NET_CLK_PTP_CONF_REG_OFF + 0xC0)


/* Definition of SYNC_TRIG_IN/SYNC_IN Debug Reg.(0xC0) */
#define NGSM1_SYNC_TRIG_IN_STAT    0x20000
#define NGSM1_SYNC_IN_STAT         0x10000
#define NGSM2_SYNC_TRIG_IN_STAT    0x08000
#define NGSM2_SYNC_IN_STAT         0x04000
#define NGWIC1_SYNC_TRIG_IN_STAT   0x02000
#define NGWIC1_SYNC_IN_STAT        0x01000
#define NGWIC2_SYNC_TRIG_IN_STAT   0x00800
#define NGWIC2_SYNC_IN_STAT        0x00400
#define NGWIC3_SYNC_TRIG_IN_STAT   0x00200
#define NGWIC3_SYNC_IN_STAT        0x00100
#define NGVM_SYNC_TRIG_IN_STAT     0x00080
#define NGVM_SYNC_IN_STAT          0x00040
#define CCPU_SYNC_IN_STAT          0x00010
#define PCH_SYNC_TRIG_IN_STAT      0x00008
#define QPCH_SYNC_IN_STAT          0x00004
#define QPCH_SYNC_TRIG_IN_STAT     0x00002
#define PTP_CTR_STAT               0x00001

#define SYNC_DEBUG_REG_OFF         0x101C0
#define FPGA_NIM3_STNC_TRIG_CTRL        (NET_CLK_PTP_CONF_REG_OFF + 0x28)



#define NIOS_MODE_REG       0x34010
#define NIOS_STATUS_REG     0x34000
#define NIOS_VERSION_REG    0x34002
#define NIOS_NORMAL_MODE    0x0
#define NIOS_DISABLE_MODE   0x1
#define NIOS_DIAG_MODE      0x3
#define NIOS_MIN_VERSION    0x117  /* for compatibility check purpose */
#define NIOS_NORMAL_CHECK   0x4E49  /* check value for the normal mode */
#define NIOS_CPU_TEMP_OFF   0xD90   /* main cpu temperature register */

#define NIOS_MAX_RETRY      (10)    /* per hw suggest, 10* 300000us for each polling */
#define NIOS_POLLING_DELAY  (300000)
#define FPGA_AIKIDO_SPI_MASTER_OFFSET   0x31A00


/* Voltage margin Control Register (0xC0) */
#define VOLTAGE_MARGIN_REG              0xC0
#define MARGIN_CONTROL_BASE             0xC5C00000
#define NO_MARGIN                       0x0
#define MARGIN_LOW                      0x1
#define MARGIN_HIGH                     0x2

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


/* RST_CPLD (LPC) Register Map */
#define LPC_STATUS_REG       0x0C
#define LPC_CONTROL_REG      0x58

typedef struct rst_cpld_t_ {
    volatile unsigned int rsv  /*0x00*/;
#define FPGA_RST_REASON               0x4
    volatile unsigned int rst_reason /*0x04*/;
#define FPGA_MAGIC_COOKIE             0x8
    volatile unsigned int magic_cookie  /*0x08*/;
#define FPGA_STS                      0xC
#define FPGA_BOOT_DEV1_MASK           0x20
#define FPGA_SEL_DEV1_MASK            0x10
    volatile unsigned int sts /*0x0c*/;
#define FPGA_RST_CTRL                 0x10
    volatile unsigned int rst_ctrl /*0x10*/;
#define FPGA_SYS_LED                  0x14
    volatile unsigned int led /*0x14*/;
#define CPLD_SYS_STATUS_LED_GREEN    (0x03)
#define CPLD_SYS_STATUS_LED_YELLOW   (0x02)
#define CPLD_SYS_STATUS_LED_RED      (0x01)
#define CPLD_SYS_STATUS_LED_OFF      (0x0) 
#define FPGA_RST_DEBUG                0x18
    volatile unsigned int debug /*0x18*/;
#define FPGA_DEV_RST_CTRL             0x1C
    volatile unsigned int dev_rst_ctrl /*0x1C*/;
#define FPGA_LED                      0x20
    volatile unsigned int flash_led /*0x20*/;
#define FPGA_IRQ0_STS                 0x24
    volatile unsigned int intr_irq0_sts /*0x24*/;
#define FPGA_IRQ0_MSK                 0x28
    volatile unsigned int intr_irq0_msk /*0x28*/;
#define FPGA_IRQ5_STS                 0x2C
    volatile unsigned int intr_irq5_sts /*0x2c*/;
#define FPGA_IRQ5_MSK                 0x30
    volatile unsigned int intr_irq5_msk /*0x30*/;
    volatile unsigned char pd1[0x10]; /*0x44-0x30-4*/
#define FPGA_MGT_CTRL                 0x44
    volatile unsigned int mgt_ctrl /*0x44*/;
#define FPGA_ALM                      0x48
    volatile unsigned int alarm /*0x48*/;
    volatile unsigned char pd2[0x08]; /*0x54-(0x48+4)*/
#define FPGA_TST                      0x54
    volatile unsigned int tst /*0x54*/;
#define FPGA_CTRL0                    0x58
    volatile unsigned int ctrl0 /*0x58*/;
#define FPGA_CTRL1                    0x5C
    volatile unsigned int ctrl1 /*0x5c*/;
#define FPGA_PWR                      0x60
    volatile unsigned int pwr /*0x60*/;
    volatile unsigned char pd3[0x1c];  /*0x80-0x60-0x4 = 0x1C */
#define FPGA_BD_TYPE                  0x80
    volatile unsigned int brd /*0x80*/;
#define FPGA_VERTYPE                  0x84
    volatile unsigned int ver /*0x84*/;
    volatile unsigned int main_fpga_ver /*0x88*/;
    volatile unsigned char pd4[0x24];
    volatile unsigned int config_hdr_ctrl /* 0xB0 */;
    volatile unsigned int config_hdr_debug /* 0xB4 */;
    volatile unsigned int config_hdr_ptr /* 0xB8 */;

} rst_cpld_t;

// SmartFan Defines
#define FAN_SMARTFAN_CTRL_START                   0x1
#define FAN_SMARTFAN_CTRL_FAN_OFFSET              4
#define FAN_SMARTFAN_CTRL_FAN_MASK                0x3
#define FAN_SMARTFAN_STAT_BUSY                    0x1
#define FAN_SMARTFAN_STAT_TACH_LOW_MIN            0x4
#define FAN_SMARTFAN_STAT_TACH_LOW_MAX            0x8
#define FAN_SMARTFAN_STAT_BIT_MIN                 0x10
#define FAN_SMARTFAN_STAT_BIT_TYP                 0x20
#define FAN_SMARTFAN_STAT_BIT_MAX                 0x40
#define FAN_SMARTFAN_STAT_FIFO_FULL               0x100
#define FAN_SMARTFAN_STAT_FIFO_EMPTY              0x200
#define FAN_SMARTFAN_STAT_COUNT_OFFSET            16
#define FAN_SMARTFAN_STAT_COUNT_MASK              0x3FF   // After offset

// Macros

#define MACRO_FPGA_SMARTFAN_BUSY (smartfan_is_busy())
#define MACRO_FPGA_SMARTFAN_FIFO_EMPTY (smartfan_fifo_empty())

//------------------------------------------------------------------------------
typedef struct fan_envmnt_reg {
//------------------------------------------------------------------------------
  volatile unsigned int env_fan_status1;          // 0x03_2200
  volatile unsigned int env_fan_contro1;          // 0x03_2204
  volatile unsigned int env_fan_pwm_slope;        // 0x03_2208
  volatile unsigned int env_fan_pad1;             // 0x03_220c
  volatile unsigned int env_fan_tach1;            // 0x03_2210
  volatile unsigned int env_fan_tach2;            // 0x03_2214
  volatile unsigned int env_fan_tach3;            // 0x03_2218
  volatile unsigned int env_fan_tach4;            // 0x03_221C
  volatile unsigned int env_fan_speed1;           // 0x03_2220
  volatile unsigned int env_fan_speed2;           // 0x03_2224
  volatile unsigned int env_fan_speed3;           // 0x03_2228
  volatile unsigned int env_fan_speed4;           // 0x03_222C
  volatile unsigned int pad1[20];                 // 0x03_2230 - 0x03_227C
  volatile unsigned int env_fan_smartfan_control; // 0x03_2280
  volatile unsigned int env_fan_smartfan_status;  // 0x03_2284
  volatile unsigned int pad2[2];                  // 0x03_2288 - 0x03_228C
  volatile unsigned int env_fan_smartfan_fifo;    // 0x03_2290
  volatile unsigned int env_fan_smartfan_pwm;     // 0x03_2294
  volatile unsigned int env_fan_smartfan_debug;   // 0x03_229C
} fan_envmnt_reg_t ;

/* For Graffham */
#define NGVM_SYNC_OUT_SYNC_TRIG_OUT    0x30
#define NGVM_SYNC_OUT1_CTRL            0x34
#define SYNC_TRIG_IN_SYNC_IN_DBG       0xc0
#define NGDC_I2C_ADDR_IO_PORT          0x1F  /* 3E >> 1 */
#define NGDC_I2C_ADDR_IO_PORT1         0x1E  /* 3C >> 1 */

/*
 * Externs
 */
extern unsigned long dash_fpga;
extern int fpga_reset_api(uint, uint, uint, uint);
extern int fpga_read_reg(uint, uint *);
extern int fpga_write_reg(uint, uint);
extern int fpga_reg_rd_util(int);
extern int fpga_reg_wr_util(int);
extern int tabei_fpga_utils(int );
extern int fpga_vol_margin(uint8_t);
extern int fpga_poe_detect(void);
extern int fpga_poe_detect_util(void);
extern int uart_lpbk_txrx(int, char*, int, char*, int *, int, int);
extern int set_nios_mode(int);
extern int is_promethium(void);
extern int is_promethium_l(void);
extern int is_tabeil(void);
extern int is_fortnite(void);
extern int is_not_promethium(void);
extern int is_not_tabeil(void);
extern int is_not_fortnite(void);
extern int has_4core(void);
extern int has_nim(void);
extern int has_pim(void);
extern int has_dimm_slot(void);
extern int has_emmc(void);
extern int has_hdd(void);
extern int has_tpm(void);
extern int has_phy1514(void);
extern int has_phy1543(void);
extern int has_esw6390(void);
extern int has_i350(void);
extern int has_m2sata(void);
extern int has_m2pcie(void);
extern int has_bios_eeprom(void);
extern int has_barometer(void);
extern boolean smartfan_is_busy(void);
extern boolean smartfan_fifo_empty(void);
extern uchar smartfan_fifo_rd(void);
extern void smartfan_start(uchar);
extern int fpga_register_operation(uint, uint, uint); 
extern int diag_fpga_i2c_read_sfp_util(void);
extern int diag_fpga_i2c_write_sfp_util(void);
extern unsigned long get_platform_net_clk_ptp_conf_base(void);
extern int diag_fpga_i2c_dump_sfp_util(void);
extern int diag_fpga_i2c_read_sfp_vendor_name(int , char *);


#endif   /* __PLATFORM_FPGA_H__ */

/*-------------------------------------------------
 * $Log: diag_fpga.h,v $
 * Revision 1.6  2020/10/07 08:20:48  kehuang2
 * CSCvv99413: Collapse Promethium-L into main trunk
 *
 * Revision 1.5  2020/08/06 07:54:55  kehuang2
 * Collapse Promethium into main trunk
 *
 * Revision 1.4  2019/12/30 05:59:18  kehuang2
 * CSCvs55860: Support Gaffham
 *
 * Revision 1.3  2019/11/25 08:55:51  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.40  2019/09/27 07:57:23  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.39  2019/09/05 08:50:36  olin2
 * Support FPGA serial IRQ interrupt util
 *
 * Revision 1.1.2.38  2019/09/02 08:38:02  olin2
 * support display smart fan info util
 *
 * Revision 1.1.2.37  2019/08/26 07:55:00  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.36  2019/08/20 10:30:12  kehuang2
 * Support POE detect Utility
 *
 * Revision 1.1.2.35  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.2.34  2019/07/15 09:40:02  kehuang2
 * Update Voltage Margin utility
 *
 * Revision 1.1.2.33  2019/07/11 03:38:39  kehuang2
 * Support M.2 detect mechanism on fortnite
 *
 * Revision 1.1.2.32  2019/07/08 01:49:20  kehuang2
 * Rename variable to avoid redefined with PIM module
 *
 * Revision 1.1.2.31  2019/07/04 03:23:35  kehuang2
 * Combine Tabei-L sereies image together(Fortnite, Tabei-L, Promethium)
 *
 * Revision 1.1.2.30  2019/06/20 06:21:13  kehuang2
 *
 * 1. Support linux_block_test function
 * 2. Update Diag menu item base on currently project information
 *
 * Revision 1.1.2.29  2019/06/11 08:14:14  kehuang2
 * Update LED utility
 *
 * Revision 1.1.2.28  2019/05/29 06:59:22  olin2
 * Add cpld register structure
 *
 * Revision 1.1.2.27  2019/05/29 03:16:17  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.2.26  2019/05/24 09:56:11  kehuang2
 *
 * 1.Update Temp Interrupt test
 * 2.Clean up code
 *
 * Revision 1.1.2.25  2019/05/21 03:18:00  kehuang2
 *
 * 1.SFP EN LED Support base on PreP2B respin
 * 2.Support SFP Mux access utility
 *
 * Revision 1.1.2.24  2019/05/07 06:08:33  olin2
 * Check M.2 device present through FPGA
 *
 * Revision 1.1.2.23  2019/04/25 01:57:51  olin2
 * Support bootflash 1 test
 *
 * Revision 1.1.2.22  2019/03/26 09:58:45  kehuang2
 * Support LED Test
 *
 * Revision 1.1.2.21  2019/03/19 09:26:26  kehuang2
 * Merge Sku1 and Sku2 into same image
 *
 * Revision 1.1.2.20  2019/01/21 10:42:09  harrchan
 * Update for sku1 future use
 *
 * Revision 1.1.2.19  2019/01/03 03:16:48  harrchan
 * Add distinguish sku function
 *
 * Revision 1.1.2.18  2018/12/26 03:48:33  harrchan
 * LED Test
 *
 * Revision 1.1.2.17  2018/12/25 12:00:27  harrchan
 * Update FPGA address
 *
 * Revision 1.1.2.16  2018/12/25 07:24:38  olin2
 * Clean up code
 *
 * Revision 1.1.2.15  2018/12/25 06:41:49  olin2
 * Update GE PHY init sequence
 *
 * Revision 1.1.2.14  2018/12/05 06:41:06  olin2
 * Update Fan control for NIOS
 *
 * Revision 1.1.2.13  2018/11/28 07:37:27  olin2
 * Update fan util
 *
 * Revision 1.1.2.12  2018/11/16 05:42:09  olin2
 * Clean up code
 *
 * Revision 1.1.2.11  2018/11/15 06:56:06  olin2
 * initial commit for Fan utils
 *
 * Revision 1.1.2.10  2018/11/06 07:17:31  kodko
 * Fix FPGA register test.
 *
 * Revision 1.1.2.9  2018/10/29 01:29:55  olin2
 * Remove unused function
 *
 * Revision 1.1.2.8  2018/10/26 08:40:50  kodko
 * Add support for PIM LTE and test card modules.
 *
 * Revision 1.1.2.7  2018/10/18 11:06:52  olin2
 * Support NIM testcard UART test
 *
 * Revision 1.1.2.6  2018/10/17 06:14:27  olin2
 * Support FPGA I2C scan
 *
 * Revision 1.1.2.5  2018/10/16 11:33:14  olin2
 * Update NIM test
 *
 * Revision 1.1.2.4  2018/10/16 02:26:02  kodko
 * Support Tabei-L ACT2 & Cookie programming.
 *
 * Revision 1.1.2.3  2018/10/15 11:48:28  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.2.2  2018/10/09 09:22:04  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.2.1  2018/10/02 01:49:58  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
