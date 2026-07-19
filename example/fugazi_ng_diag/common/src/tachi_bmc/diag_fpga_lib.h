/* $Id: diag_fpga_lib.h,v 1.7 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_lib.h - Header file for FPGA functions
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_LIB__
#define __DIAG_FPGA_LIB__

typedef enum
{
    FPGA_BIT_OPS_ON,
    FPGA_BIT_OPS_OFF
} fpga_bit_ops;

/* System Low Level */
#define LOW_LEVEL_OFFSET                (0x00)
#define FPGA_SCRATCHPAD_REG             (LOW_LEVEL_OFFSET + 0X00)
#define FPGA_EXT_RESET_REG              (LOW_LEVEL_OFFSET + 0x04)
#define FPGA_INT_RESET_REG              (LOW_LEVEL_OFFSET + 0x08)
#define FPGA_EXT_PIN_CTRL_REG           (LOW_LEVEL_OFFSET + 0x10)
#define FPGA_PLAT_RESET_REASON_REG      (LOW_LEVEL_OFFSET + 0x14)
#define FPGA_BOARD_TYPE_REG0            (LOW_LEVEL_OFFSET + 0x60)
#define FPGA_BOOT_TIMER_REG             (LOW_LEVEL_OFFSET + 0x64)
#define FPGA_BMC_RESET_REG              (LOW_LEVEL_OFFSET + 0x68)
#define FPGA_HW_TYPE_REV_REG            (LOW_LEVEL_OFFSET + 0x84)
#define FPGA_VER_REG                    (LOW_LEVEL_OFFSET + 0x8C)
#define FPGA_NIOS_VER_REG               (LOW_LEVEL_OFFSET + 0x90)
#define FPGA_HW_TYPE_REV_REG2           (LOW_LEVEL_OFFSET + 0x90)
#define FPGA_CONF_HDR_CTRSTS_REG        (LOW_LEVEL_OFFSET + 0xB0)
#define FPGA_CONF_HDR_DBG_REG           (LOW_LEVEL_OFFSET + 0xB4)
#define FPGA_CONF_HDR_PTR_REG           (LOW_LEVEL_OFFSET + 0xB8)
#define FPGA_VOLT_MARG_REG              (LOW_LEVEL_OFFSET + 0xC0)
#define FPGA_DEBUG_REG                  (LOW_LEVEL_OFFSET + 0xF0)

/* External Device Rest */
#define FPGA_I350_RESET                 (0x00000001)
#define FPGA_CETUS_RESET                (0x00000002)
#define FPGA_PCIE_SW_RESET              (0x00000004)
#define FPGA_SFP_I2C_MUX_RESET          (0x00000010)
#define FPGA_BARO_RESET                 (0x00000040)
#define FPGA_POE_RESET                  (0x00000080)
#define FPGA_PSU_I2C_MUX_RESET          (0x00000400)
#define FPGA_VNIC_RESET                 (0x00000800)
#define FPGA_EMMC_RESET                 (0x00020000)
#define FPGA_ACT2_RESET                 (0x00040000)
#define FPGA_USB0_RESET                 (0x00100000)
#define FPGA_BT_RESET                   (0x02000000)
#define FPGA_MGMT_PORT_RESET            (0x04000000)

/* System Low Level Register */
#define FPGA_MAS_REV_REG                (0x84)

/* Hardware Type/Revision Register */
#define HW_BOARD_TYPE_MASK              (0x30000000)

/* BMC/NIOS Interrupt Status and Control */
#define BMC_INT_STS_CTL_OFFSET          (0x100)
#define NIOS_INT_STS_CTL_OFFSET         (0x300)
#define FPGA_INT_STS_REG                (0x00)
#define FPGA_INT_EN_REG                 (0x04)
#define FPGA_SFP_INT_STS_REG            (0x10)
#define FPGA_SFP_INT_EN_REG             (0x14)
#define FPGA_I2C_INT_STS_REG            (0x20)
#define FPGA_I2C_INT_EN_REG             (0x24)
#define FPGA_I2C_INT_TEST_REG           (0x28)
#define FPGA_UART_INT_STS_REG           (0x30)
#define FPGA_UART_INT_EN_REG            (0x34)
#define FPGA_UART_INT_TST_REG           (0x38)
#define FPGA_MODULE_OIR_INT_STS_REG     (0x40)
#define FPGA_MODULE_OIR_INT_EN_REG      (0x44)
#define FPGA_MODULE_OIR_INT_TEST_REG    (0x48)
#define FPGA_MISC_INT_STS_REG           (0x50)
#define FPGA_MISC_INT_EN_REG            (0x54)
#define FPGA_MISC_INT_TEST_REG          (0x58)

/* LED Control */
#define LED_CTRL_OFFSET                 (0x400)
#define FPGA_LED_MISC_ONOFF_REG         (LED_CTRL_OFFSET + 0x00)
#define FPGA_LED_EMMC_ONOFF_REG         (LED_CTRL_OFFSET + 0x04)
#define FPGA_LED_PWRSUP_ONOFF_REG       (LED_CTRL_OFFSET + 0x08)
#define FPGA_LED_POE_PWRSUP_ONOFF_REG   (LED_CTRL_OFFSET + 0x0C)
#define FPGA_LED_POE_DC_ONOFF_REG       (LED_CTRL_OFFSET + 0x10)
#define FPGA_LED_ENV_REG                (LED_CTRL_OFFSET + 0x18)
#define FPGA_LED_BLINK_DUR_REG          (LED_CTRL_OFFSET + 0x20)
#define FPGA_LED_RJ45SFP_BLINK_EN_REG   (LED_CTRL_OFFSET + 0x24)
#define FPGA_LED_RJ45_ONOFF_REG         (LED_CTRL_OFFSET + 0x2C)
#define FPGA_LED_SFP_ONOFF_REG          (LED_CTRL_OFFSET + 0x30)
#define FPGA_LED_DEBUG_REG              (LED_CTRL_OFFSET + 0x38)
#define FPGA_LED_M2_ONOFF_REG           (LED_CTRL_OFFSET + 0x40)
#define FPGA_HDD1_STATUS_LED_REG        (LED_CTRL_OFFSET + 0x80)
#define FPGA_HDD2_STATUS_LED_REG        (LED_CTRL_OFFSET + 0x88)
enum {
    SM1_UART_CTRL = 0,
    SM2_UART_CTRL,
    WIC1_UART_CTRL,
    WIC2_UART_CTRL,
    WIC3_UART_CTRL,
    VM1_UART_CTRL,
    CAV0_UART_CTRL,
    CAV1_UART_CTRL,
    UART_UART_CTRL
};

/* UART Mux */
#define UART_MUX_OFFSET                 (0x900)
#define FPGA_UART_DBG_MUX_REG           (UART_MUX_OFFSET + 0x00)
#define FPGA_UART_MOD_MUX_REG           (UART_MUX_OFFSET + 0x04)
#define FPGA_MUX_SEL_CTRL_REG1          (UART_MUX_OFFSET + 0x10)
#define FPGA_MUX_SEL_CTRL_REG2          (UART_MUX_OFFSET + 0x14)

/* UART Mux Selection Bits (Foxconn FPGA) */
#define FPGA_UART_MUX_SEL_FP            (0x00)
#define FPGA_UART_MUX_SEL_AUX           (0x01)
#define FPGA_UART_MUX_SEL_BT            (0x02)
#define FPGA_UART_MUX_SEL_INTEL         (0x03)
#define FPGA_UART_MUX_SEL_BMC_UART0     (0x04)
#define FPGA_UART_MUX_SEL_BMC_UART1     (0x05)
#define FPGA_UART_MUX_SEL_BMC_UART2     (0x06)
#define FPGA_UART_MUX_SEL_NIOS          (0x07)
#define FPGA_UART_MUX_SEL_NIM1          (0x08)
#define FPGA_UART_MUX_SEL_ISC           (0x0B)
#define FPGA_UART_MUX_SEL_CETUS         (0x0C)
#define FPGA_UART_MUX_SEL_NULL          (0x0F)

/* BMC UART1 Selection (Cisco FPGA) */
#define FPGA_BMC_UART1_RAID             (0x00)
#define FPGA_BMC_UART1_NIM1             (0x02)
#define FPGA_BMC_UART1_NIM2             (0x03)
#define FPGA_BMC_UART1_NIM3             (0x04)
#define FPGA_BMC_UART1_ISP_CARD        (0x05)
#define FPGA_BMC_UART1_BT               (0x09)
#define FPGA_BMC_UART1_CETUS            (0x0A)
#define FPGA_BMC_UART1_SEL              (0x00)

/* UART Mux Selection Bit Position */
#define FPGA_UART_NIOS_SEL              (28)
#define FPGA_UART_BMC_UART2_SEL         (24)
#define FPGA_UART_BMC_UART1_SEL         (20)
#define FPGA_UART_BMC_UART0_SEL         (16)
#define FPGA_UART_INTEL_SEL             (12)
#define FPGA_UART_BT_SEL                (8)
#define FPGA_UART_AUX_SEL               (4)
#define FPGA_UART_CNS_SEL               (0)

#define FPGA_UART_CETUS_SEL             (16)
#define FPGA_UART_ISC_SEL               (12)
#define FPGA_UART_NIM1_SEL              (0)

/* SFP Status and Control */
#define SFP_STS_CTRL_OFFSET             (0x10000)
#define FPGA_SFP0_INT_STS_REG           (SFP_STS_CTRL_OFFSET + 0x00)
#define FPGA_SFP0_CONF_REG              (SFP_STS_CTRL_OFFSET + 0x04)
#define FPGA_SFP1_INT_STS_REG           (SFP_STS_CTRL_OFFSET + 0x08)
#define FPGA_SFP1_CONF_REG              (SFP_STS_CTRL_OFFSET + 0x0C)

#define SFP_PRSNT_OUTPUT_EN             (0x200)

/* Sync Registers */
#define NET_CLK_PTP_CONF_REG_OFF        (0x10100)
#define FPGA_NGSM1_STNC_TRIG_CTRL       (NET_CLK_PTP_CONF_REG_OFF + 0x10)
#define FPGA_NGSM2_STNC_TRIG_CTRL       (NET_CLK_PTP_CONF_REG_OFF + 0x14)
#define FPGA_NIM1_STNC_TRIG_CTRL        (NET_CLK_PTP_CONF_REG_OFF + 0x20)
#define FPGA_NIM2_STNC_TRIG_CTRL        (NET_CLK_PTP_CONF_REG_OFF + 0x24)
#define FPGA_NIM3_STNC_TRIG_CTRL        (NET_CLK_PTP_CONF_REG_OFF + 0x28)
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


/* UART */
#define UART0_OFFSET                    (0x20000)
#define UART1_OFFSET                    (0x20100)
#define UART2_OFFSET                    (0x20200)
#define UART3_OFFSET                    (0x20300)
#define UART4_OFFSET                    (0x20400)
#define UART5_OFFSET                    (0x20500)
#define UART6_OFFSET                    (0x20600)
#define UART7_OFFSET                    (0x20700)
#define UART8_OFFSET                    (0x20800)
#define UART9_OFFSET                    (0x20900)
#define FPGA_UART_RBRTHRDLL_REG         (0x00)
#define FPGA_UART_IER_DLM_REG           (0x04)
#define FPGA_UART_IIR_FCR_REG           (0x08)
#define FPGA_UART_LCR_REG               (0x0C)
#define FPGA_UART_MCR_REG               (0x10)
#define FPGA_UART_LSR_REG               (0x14)
#define FPGA_UART_MSR_REG               (0x18)
#define FPGA_UART_SCR_REG               (0x1C)

/* FPGA Multiboot */
#define FPGA_MULTIBOOT_OFFSET           (0x22000)
#define FPGA_RECONF_CTRL_REG            (FPGA_MULTIBOOT_OFFSET + 0x00)
#define FPGA_RECONF_STS_REG             (FPGA_MULTIBOOT_OFFSET + 0x04)
#define FPGA_CACHED_REVID_REG           (FPGA_MULTIBOOT_OFFSET + 0x08)
#define FPGA_CACHED_REVDATE_REG         (FPGA_MULTIBOOT_OFFSET + 0x0C)
#define FPGA_CACHED_FLAGS_REG           (FPGA_MULTIBOOT_OFFSET + 0x10)
#define FPGA_CACHED_MAGIC_REG           (FPGA_MULTIBOOT_OFFSET + 0x14)
#define FPGA_MULTIBOOT_STATE_REG        (FPGA_MULTIBOOT_OFFSET + 0x18)
#define FPGA_MULTIBOOT_SEC_RSLT_REG     (FPGA_MULTIBOOT_OFFSET + 0x1C)
#define FPGA_MULTIBOOT_DBG_REG          (FPGA_MULTIBOOT_OFFSET + 0x20)
#define FPGA_SECBOOT_STS_REG            (FPGA_MULTIBOOT_OFFSET + 0x24)
#define FPGA_SECBOOT_SYS_STS_REG        (FPGA_MULTIBOOT_OFFSET + 0x28)
#define FPGA_SECBOOT_CORE_STS_REG       (FPGA_MULTIBOOT_OFFSET + 0x2C)
#define FPGA_SECBOOT_SIGN_REG           (FPGA_MULTIBOOT_OFFSET + 0x30)
#define FPGA_SECBOOT_SIGN_SIZE_REG      (FPGA_MULTIBOOT_OFFSET + 0x34)

/* I2C Controller */
#define I2C0_CTRL_OFFSET                (0x30000)
#define I2C2_CTRL_OFFSET                (I2C0_CTRL_OFFSET + 0x200)
#define I2C4_CTRL_OFFSET                (I2C0_CTRL_OFFSET + 0x400)
#define I2C7_CTRL_OFFSET                (I2C0_CTRL_OFFSET + 0x700)
#define I2C8_CTRL_OFFSET                (I2C0_CTRL_OFFSET + 0x800)
#define I2C10_CTRL_OFFSET               (I2C0_CTRL_OFFSET + 0xA00)
#define I2C11_CTRL_OFFSET               (I2C0_CTRL_OFFSET + 0xB00)
#define I2C12_CTRL_OFFSET               (I2C0_CTRL_OFFSET + 0xC00)
#define I2C13_CTRL_OFFSET               (I2C0_CTRL_OFFSET + 0xD00)
#define FPGA_I2C_MASTER_CTRL_REG        (0x00)
#define FPGA_I2C_MASTER_STS_REG         (0x08)
#define FPGA_I2C_MASTER_MASK_REG        (0x0C)
#define FPGA_I2C_SLAVE_ADDR_REG         (0x10)
#define FPGA_I2C_SLAVE_SUBADDR_REG      (0x14)
#define FPGA_I2C_BITBANG_REG            (0x18)
#define FPGA_I2C_BYTECNT_REG            (0x1C)
#define FPGA_I2C_DATA_FIFO_REG          (0x40)
#define FPGA_I2C_DATA_FIFO_PTR_REG      (0x44)

/* ACT2 I2C Controller */
#define FPGA_I2C_ACT2_CTRL              (I2C0_CTRL_OFFSET)

/* Temperature Sensor, Barometer, MCU */
#define FPGA_I2C_TEMP_BARO_MCU_CTRL     (I2C2_CTRL_OFFSET)

/* NIM 1 */
#define FPGA_I2C_NIM1_CTRL              (I2C10_CTRL_OFFSET)

/* NIOS UART */
#define NIOS_UART_OFFSET                (0x31500)
#define FPGA_NIOS_UART_RCV_DATA_REG     (NIOS_UART_OFFSET + 0x00)
#define FPGA_NIOS_UART_TX_DATA_REG      (NIOS_UART_OFFSET + 0x04)
#define FPGA_NIOS_UART_STS_REG          (NIOS_UART_OFFSET + 0x08)
#define FPGA_NIOS_UART_CTRL_REG         (NIOS_UART_OFFSET + 0x0C)
#define FPGA_NIOS_UART_DIV_REG          (NIOS_UART_OFFSET + 0x10)

/* FPGA/NIOS SPI PROM Programming */
#define FPGA_SPI_PROM_PROG_OFFSET       (0x31800)
#define FPGA_NIOS_SPI_PROG_OFFSET       (0x31900)
#define FPGA_SPI_PROM_CTRL_REG          (FPGA_SPI_PROM_PROG_OFFSET + 0x00)
#define FPGA_SPI_PROM_STS_REG           (FPGA_SPI_PROM_PROG_OFFSET + 0x04)
#define FPGA_SPI_PROM_RD_SIZE_REG       (FPGA_SPI_PROM_PROG_OFFSET + 0x08)
#define FPGA_SPI_PROM_RW_DATA_REG       (FPGA_SPI_PROM_PROG_OFFSET + 0x0C)
#define FPGA_SPI_PROM_OP_ADDR_REG       (FPGA_SPI_PROM_PROG_OFFSET + 0x10)

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

/* NGIO Status / Control Register */ 
#define NGIO_HDLS_MODE                  (0x40000)
#define NGIO_DW_MODE                    (0x20000)
#define NGIO_PWR_OK                     (0x10000)
#define NGIO_FLT_INTR                   (0x400)
#define NGIO_INS_INTR                   (0x200)
#define NGIO_RMV_INTR                   (0x100)
#define NGIO_PRSNT                      (0x80)
#define NGIO_I2C_OK                     (0x40)
#define NGIO_UART_TX                    (0x20)
#define NGIO_PWR_EN                     (0x10)
#define NGIO_SRC_SEL                    (8)
#define NGIO_PCI_RDY                    (4)
#define NGIO_RESET                      (2)
#define NGIO_I2C_RESET                  (1)

/* Power Supply and PSE */
#define PSU_PSE_OFFSET                  (0x32100)
#define FPGA_PSU_PSE_DEB_REG            (PSU_PSE_OFFSET + 0x00)
#define FPGA_PSU_STS_REG                (PSU_PSE_OFFSET + 0x10)
#define FPGA_PSU_INT_REG                (PSU_PSE_OFFSET + 0x14)
#define FPGA_PSU_INT_EN_REG             (PSU_PSE_OFFSET + 0x18)
#define FPGA_POE_STS_REG                (PSU_PSE_OFFSET + 0x20)
#define FPGA_POE_INT_REG                (PSU_PSE_OFFSET + 0x24)
#define FPGA_POE_INT_EN_REG             (PSU_PSE_OFFSET + 0x28)

/* PSU status Register (0x32100+0x10) */
#define PSU1_PRESENT_MSK              0x1
#define PSU1_AC_IN_OK                 0x2
#define PSU1_12V_OUT_OK               0x4
#define PSU1_INTERRUPT_STAT           0x8
#define PSU2_PRESENT_MSK              0x10
#define PSU2_AC_IN_OK                 0x20
#define PSU2_12V_OUT_OK               0x40
#define PSU2_INTERRUPT_STAT           0x80

/* Environmental Fan Control */
#define ENV_FAN_CTRL_OFFSET             (0x32200)
#define FPGA_ENV_FAN_STS_REG            (ENV_FAN_CTRL_OFFSET + 0x00)
#define FPGA_ENV_FAN_CTRL1_REG          (ENV_FAN_CTRL_OFFSET + 0x04)
#define FPGA_ENV_FAN_PWM_SLOPE_REG      (ENV_FAN_CTRL_OFFSET + 0x08)
#define FPGA_ENV_FAN1_TACH_RPM_REG      (ENV_FAN_CTRL_OFFSET + 0x10)
#define FPGA_ENV_FAN2_TACH_RPM_REG      (ENV_FAN_CTRL_OFFSET + 0x14)
#define FPGA_ENV_FAN3_TACH_RPM_REG      (ENV_FAN_CTRL_OFFSET + 0x18)
#define FPGA_ENV_FAN4_TACH_RPM_REG      (ENV_FAN_CTRL_OFFSET + 0x1C)
#define FPGA_ENV_FAN1_TACH_SPEED_REG    (ENV_FAN_CTRL_OFFSET + 0x20)
#define FPGA_ENV_FAN2_TACH_SPEED_REG    (ENV_FAN_CTRL_OFFSET + 0x24)
#define FPGA_ENV_FAN3_TACH_SPEED_REG    (ENV_FAN_CTRL_OFFSET + 0x28)
#define FPGA_ENV_FAN4_TACH_SPEED_REG    (ENV_FAN_CTRL_OFFSET + 0x2C)

/* BMC to NIOS CPU Mailbox */
#define BMC_NIOS_MBOX_OFFSET            (0x32400)
#define FPGA_BMC_NIOS_CTRL_REG          (BMC_NIOS_MBOX_OFFSET + 0x00)
#define FPGA_BMC_NIOS_ADDR_REG          (BMC_NIOS_MBOX_OFFSET + 0x04)
#define FPGA_NIOS_BMC_CTRL_REG          (BMC_NIOS_MBOX_OFFSET + 0x08)
#define FPGA_NIOS_BMC_ADDR_REG          (BMC_NIOS_MBOX_OFFSET + 0x0C)
#define FPGA_BMC_INTR_REG               (BMC_NIOS_MBOX_OFFSET + 0x10)
#define FPGA_NIOS_CPU_INTR_REG          (BMC_NIOS_MBOX_OFFSET + 0x14)
#define FPGA_NIOS_SPECIAL_CTRL_REG      (BMC_NIOS_MBOX_OFFSET + 0x20)
#define FPGA_NIOS_SPECIAL_STAT_REG      (BMC_NIOS_MBOX_OFFSET + 0x24)

#define BMC_ENV_MCU_DNLD_REG            (0x33000)
#define BMC_VOLT_MON_DNLD_REG           (0x33100)
#define BMC_MCU_DNLD_CTRL_REG           (0x00)
#define BMC_MCU_DNLD_STS_REG            (0x04)
#define BMC_MCU_INT_EN_REG              (0x08)
#define BMC_MCU_DATA_REG                (0x0C)

/* BMC SGPIO */
#define BMC_SGPIO_OFFSET                (0x34000)
#define FPGA_BMC_SGPI_REG1              (BMC_SGPIO_OFFSET + 0x00)
#define FPGA_BMC_SGPI_REG2              (BMC_SGPIO_OFFSET + 0x04)
#define FPGA_BMC_SGPO_REG1              (BMC_SGPIO_OFFSET + 0x10)
#define FPGA_BMC_SGPO_REG2              (BMC_SGPIO_OFFSET + 0x14)

/* Port Status and Control */
#define PORT_STS_CTL_OFFSET             (0x34100)

/* SPI Shift */
#define SPI_SHIFT_OFFSET                (0x34200)
#define FPGA_X86_CS0_SHIFT_REG          (SPI_SHIFT_OFFSET + 0x10)
#define FPGA_X86_CS1_SHIFT_REG          (SPI_SHIFT_OFFSET + 0x14)
#define FPGA_X86_MON_SHIFT_IN_REG       (SPI_SHIFT_OFFSET + 0x18)
#define FPGA_BMC_CS0_SHIFT_REG          (SPI_SHIFT_OFFSET + 0x20)
#define FPGA_BMC_CS1_SHIFT_REG          (SPI_SHIFT_OFFSET + 0x24)
#define FPGA_BMC_MON_SHIFT_IN_REG       (SPI_SHIFT_OFFSET + 0x28)

/* Remote Update Purpose */
#define REMOTE_UPDATE_OFFSET            (0x34300)
#define FPGA_REMOTE_UPDATE_CONF_REG     (REMOTE_UPDATE_OFFSET + 0x20)
#define FPGA_REMOTE_UPDATE_CONT_REG     (REMOTE_UPDATE_OFFSET + 0x21)
#define FPGA_REMOTE_UPDATE_STS_REG      (REMOTE_UPDATE_OFFSET + 0x22)
#define FPGA_REMOTE_UPDATE_DATA_IN_REG  (REMOTE_UPDATE_OFFSET + 0x23)
#define FPGA_REMOTE_UPDATE_DATA_OUT_REG (REMOTE_UPDATE_OFFSET + 0x24)
#define FPGA_REMUP_SPI_ADDR_REG         (REMOTE_UPDATE_OFFSET + 0x25)
#define FPGA_REMUP_SPI_DATAIN_REG       (REMOTE_UPDATE_OFFSET + 0x26)
#define FPGA_REMUP_SPI_DATAOUT_REG      (REMOTE_UPDATE_OFFSET + 0x27)

/* NIOS Watchdog Timer */
#define NIOS_WDOG_TIMER_OFFSET          (0x50800)
#define FPGA_NIOS_WDOG_TIMER_STS_REG    (NIOS_WDOG_TIMER_OFFSET + 0x00)
#define FPGA_NIOS_WDOG_TIMER_CTRL_REG   (NIOS_WDOG_TIMER_OFFSET + 0x04)
#define FPGA_NIOS_WDOG_TIMER_TOUT_REG   (NIOS_WDOG_TIMER_OFFSET + 0x08)
#define FPGA_NIOS_WDOG_TIMER_CNTR_REG   (NIOS_WDOG_TIMER_OFFSET + 0x10)

/* NIOS Timer */
#define NIOS_TIMER_OFFSET               (0x51000)
#define FPGA_NIOS_TIMER_STS_REG         (NIOS_TIMER_OFFSET + 0x00)
#define FPGA_NIOS_TIMER_CTRL_REG        (NIOS_TIMER_OFFSET + 0x04)
#define FPGA_NIOS_TIMER_TOUT_L_REG      (NIOS_TIMER_OFFSET + 0x08)
#define FPGA_NIOS_TIMER_TOUT_H_REG      (NIOS_TIMER_OFFSET + 0x0C)
#define FPGA_NIOS_TIMER_CNTR_L_REG      (NIOS_TIMER_OFFSET + 0x10)
#define FPGA_NIOS_TIMER_CNTR_H_REG      (NIOS_TIMER_OFFSET + 0x14)

#define FPGA_SPI_CHAR_DEV               "/dev/fpgaspi"

#define NIOS_MODE_REG                   (0x34010)
#define NIOS_STATUS_REG                 (0x34000)
#define NIOS_VERSION_REG                (0x34002)
#define NIOS_NORMAL_MODE                (0x0)
#define NIOS_DISABLE_MODE               (0x1)
#define NIOS_DIAG_MODE                  (0x3)
/* check value for the normal mode */
#define NIOS_NORMAL_CHECK               (0x4E49)


#define NIOS_CHECK_RETRY                (10)

/* Voltage Margin Control Register */
#define VOLT_3P3_SHIFT                  (0)
#define VOLT_1P5_SHIFT                  (2)
#define VOLT_MARG_NORM                  (0x0)
#define VOLT_MARG_LOW                   (0x1)
#define VOLT_MARG_HIGH                  (0x2)
#define VOLT_MARG_MASK                  (VOLT_MARG_LOW | VOLT_MARG_HIGH)
/* for fpga 1.1.0, need to add C5C0 on bit 31:16 */
#define VOLT_MARG_TUNE                  (0xC5C00000)

/* DaughterCard PRSNT */
#define DC_ACT2_RESET                   (0x80)
#define DC_ACT2_UNREST                  (0x83)


/* sgpio check*/
#define SGPIO_BIT_8_9                   (0x200)

#define ENABLE_PRE_SCALER_DIV_3125       0x200 
#define SYNC_OUT_ENABLE                  0x100 

/* Dynamo NIM impletment on Tachi */
#define DYNAMO_TX_RX_RESET               0xC6
#define DYNAMO_SETUP_RATE                0x83
#define DYNAMO_ENB_FIFO                  0x1
#define DYNAMO_LPK_ENB_DTR_RTS           0x13
#define DYNAMO_LPK_ON                    0x10
#define DYNAMO_ENB_DTR_RTS               0x3



typedef enum {
    SPI_IOCTL_RD_CMD = 0,
    SPI_IOCTL_WR_CMD,
    SPI_IOCTL_START_INTR_TEST=100,
    SPI_IOCTL_GET_INTR_RESULT,
    SPI_IOCTL_STOP_INTR_TEST,
} fpga_ioctl_req_t;

typedef struct {
    int offset;
    int data;
} fpga_req;

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

extern int diag_fpga_reg_bitops(int, int, int);
extern int diag_fpga_reg_read(int, int *);
extern int diag_fpga_reg_write(int, int);
extern int diag_fpga_reg_or(int, int);
extern int diag_fpga_reg_nand(int, int);
extern int diag_margin_fpga_reg_nand(int, int);
extern int diag_fpga_ext_reset(int);
extern int diag_fpga_ext_unreset(int);
extern int mb_board_type(void);
extern int set_nios_mode(int);
extern void diag_fpga_start_int_test(void);
extern void diag_fpga_stop_int_test(void);
extern void diag_fpga_get_intr_test_result(int *);
extern int dash_uart_tx (int port, int nest_speed, char* tx_str, int test_sz, int);
extern int dash_uart_rx(int port, int *, char* rx_str);
extern void dash_uart_reset(int port);

#endif /* __DIAG_FPGA_LIB__ */

/*---------------------------------------------------------------
$Log: diag_fpga_lib.h,v $
Revision 1.7  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.6.10.2  2017/02/21 03:51:29  haohsu
Add NIM Dynamo to TACHI

Revision 1.6  2016/08/09 07:44:47  hondwang
Add RAID SGPIO testing

Revision 1.5  2016/05/05 01:01:33  benchen2
fix margin issue

Revision 1.4  2016/05/04 09:15:51  benchen2
add C5C0 to all margin

Revision 1.3  2016/05/04 02:47:43  benchen2
tachi_l:for fpga 1.1.0, need to add C5C0 on bit 31:16

Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.21  2016/03/08 03:07:07  jimmyya
Add ISP testcard uart test

Revision 1.1.2.20  2016/03/07 07:10:06  benchen2
sgpio test

Revision 1.1.2.19  2016/02/26 02:04:21  benchen2
add sfp present output enable

Revision 1.1.2.18  2016/01/26 06:27:55  benchen2
add daughter card ACT2 programming

Revision 1.1.2.17  2015/12/23 11:16:13  alpeng
support PEM(PSU) utility and its fan utils

Revision 1.1.2.16  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.15  2015/11/13 07:57:29  tirawan
Add Voltage and Frequency Margin

Revision 1.1.2.14  2015/11/13 00:50:32  tirawan
Remove FPGA SGPIO and add FPGA Interrupt

Revision 1.1.2.13  2015/10/26 12:41:51  tirawan
Correct set NIOS mode

Revision 1.1.2.12  2015/10/15 06:23:22  benchen2
add set_nios_mode

Revision 1.1.2.11  2015/09/23 09:06:17  alpeng
update console switch util to support cetus and nim; intel not yet

Revision 1.1.2.10  2015/09/18 02:40:41  tirawan
No support on MCU firmware upgrade for now

Revision 1.1.2.9  2015/09/04 01:45:44  alpeng
update console swtich to use ttyS2(BMC UART1)

Revision 1.1.2.8  2015/08/01 01:37:40  tirawan
Update FPGA SPI Read/Write function

Revision 1.1.2.7  2015/07/31 10:39:59  alpeng
first check in for testcard

Revision 1.1.2.6  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.5  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.4  2015/07/24 06:59:58  alpeng
Add ngio.c to support NIM test

Revision 1.1.2.3  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.2  2015/07/12 06:52:45  tirawan
Add Console Switch Utility, SPI driver and FPGA programming

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/
