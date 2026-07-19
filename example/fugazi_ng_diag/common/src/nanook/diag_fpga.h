 /* $Id: diag_fpga.h,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_fpga.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_fpga.h
 * Description: Header file of FPGA Diag.
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
#define WAITTIME_3500_MS        3500
#define WAITTIME_5000_MS        5000
#define DELAY_FOR_LED_TEST      1500 
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


#define MAX_UART                  8

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

#define FPGA_STAT_AND_CTRL_REG        0x90C




/****** TBD Should be removed ********/
typedef struct nanook_reg_bit {
    char           *name;
    unsigned int   offset;
} nanook_reg_bit_t;


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
#define FPGA_PWR_SUPPLY_LED          0x00408
#define FPGA_ENV_LED                 0x00418
#define FPGA_I350_RJ45_LED           0x0042C
#define FPGA_I350_SFP_LED            0x00430
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

#define FPGA_RECONFIG_CTRL_REG       0xC00


/* (0x001C) */
#define FPGA_ACT2_RST_L             (1 << 2)

/* FPGA External Device Reset Reg(0x804) */
#define I2C_CONTROLLER_RESET        (1 << 0)
#define I2C_CONTROLLER_OUT_RESET    (0 << 0)

/* FPGA Internal Device Reset Reg(0x808) */
#define INT_I2C_RESET               (1 << 0)

/* FPGA LED Reg */
#define LED_OFF                     (0 << 0)
#define ENV_LED_YELLOW               0x14000
#define ENV_LED_GREEN                0x28000
#define PWR_SUP_LED_YELLOW           0x5
#define PWR_SUP_LED_GREEN            0xA
#define RJ45_LED_YELLOW              0x44
#define RJ45_LED_GREEN               0x8888
#define SFP_LED_YELLOW               0x4444
#define SFP_LED_GREEN                0x8888

#define FPGA_BTYPE_CURIE             0x0
#define FPGA_BTYPE_SKU_BIT           0x10000000
#define FPGA_BTYPE_SKU_BIT_SHIFT     0x1C

#define FPGA_UPGRADE_HEADER_NOTE_READ 0x0
#define FPGA_UPGRADE_RECONF_FSM_RST 0x2


#define FPGA_IOS_WDT_COUNT_MASK 0xFFFFF
#define FPGA_IOS_WDT_ENABLE_KEY 0xAC000000
#define FPGA_IOS_WDT_DISABLE_KEY 0xCA000000

/* LPC Reset Button Reg(0x00C4) *///TBD
#define RST_BUTTON_MSK_OFFSET       1
#define RST_BUTTON_MSK              (1 << RST_BUTTON_MSK_OFFSET)
#define RST_BUTTON_IS_MASKED        (1 << RST_BUTTON_MSK_OFFSET)
#define RST_BUTTON_STAT             (1 << 0)
#define RST_BUTTON_PRESSED          0x1


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
#define FAN_SPD_0PER_PWM           0x0
#define FAN_SPD_35PER_PWM       0x2BC   /* 35% PWM duty cycle */
#define FAN_SPD_100PER_PWM       0x7D0  /* 100% PWM duty cycle */
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
#define FPGA_SATAHDD_CTLSTS_REG         (NGIO_MODULE_OFFSET + 0xA0)
#define FPGA_SATAHDD_INT_EN_REG         (NGIO_MODULE_OFFSET + 0xA4)
#define FPGA_SATAHDD_DEB_REG            (NGIO_MODULE_OFFSET + 0xA8)

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


#define FPGA_DEV_RST_AC3_RST    (1 << 18)
#define FPGA_DEV_RST_88E1680_2_RST    (1 << 17)
#define FPGA_DEV_RST_88E1680_1_RST    (1 << 16)
#define FPGA_DEV_RST_88E1680_0_RST    (1 << 15)
#define FPGA_DEV_RST_88E1543_RST    (1 << 14)

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
extern int nanook_fpga_utils(int );
extern int fpga_vol_margin(uint8_t);
extern int uart_lpbk_txrx(int, char*, int, char*, int *, int, int);
extern int set_nios_mode(int);
extern int this_is_curie(void);
extern int this_is_nanook(void);
extern int this_is_sku1(void);
extern int this_is_sku2(void);

#endif   /* __PLATFORM_FPGA_H__ */

/*-------------------------------------------------
 * $Log: diag_fpga.h,v $
 * Revision 1.2  2019/12/11 10:10:29  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
