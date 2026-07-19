/* $Id: fpga_szalinski.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/fpga_szalinski.h,v $
 *------------------------------------------------------------------
 * Filename: fpga_szalinski.h
 *
 * Description: Header file of Skye FPGA(Szalinski) info.
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FPGA_SZALINSKI_H__
#define __FPGA_SZALINSKI_H__

/* Common definition */
#define SKYE_CPU0          0
#define SKYE_CPU1          1

#define UNDER_RESET_MODE        0
#define UNDER_NORMAL_MODE       1

/* Based on ShrinkRay HFS(EDCS-1249942), 6 seconds duration
 * is recommend to complete the programming on the required
 * frequency registers.
 */
#define SRAY_FREQ_SETUP_TIME    0x3C

#define TEN_B                  10
#define ONE_BYTE                1
#define TWO_BYTES               2

/*
 * Definition of Register Offsets
 */
/* FPGA General Registers */
#define FPGA_REV_REG_OFF        0x00
#define BOARD_ID_REG_OFF        0x05
#define CPU0_DEV_RST_REG_OFF    0x06
#define CPU1_DEV_RST_REG_OFF    0x07
#define CPU_RST_CTRL_REG_OFF    0x08
#define LED_CTRL_REG_OFF        0x09
#define CPU0_WDT_REG_OFF        0x0A
#define CPU1_WDT_REG_OFF        0x0B
#define GPIO_EXP_L8_REG_OFF     0x0C
#define GPIO_EXP_H8_REG_OFF     0x0D
#define CLK_MARGIN_REG_OFF      0x0E
#define CPU0_SPI_MUX_REG_OFF    0x0F
#define SYS_PWR_SAV_REG_OFF     0x10
#define PRBS_10GKR_REG_OFF      0x11
#define USB_MOD_CTRL_REG_OFF    0x12
#define CPU0_ROM_WP_REG_OFF     0x13
#define CPU1_ROM_WP_REG_OFF     0x14
#define EUSB_CTRL_REG_OFF       0x15
#define RST_LOG_REG_OFF         0x16
#define PCIE_MOD_CTRL_REG_OFF   0x17
#define INTERRUPT_REG_OFF       0x18
#define INTERRUPT_EN_REG_OFF    0x19
#define INTR_ACC_SEL_REG_OFF    0x1E

/* FPGA Mulitboot Registers */
#define MBOOT_CTRL_REG_OFF      0x20   /* MBOOT: multi-boot */
#define MBOOT_STAT_REG_OFF      0x24
#define MBOOT_HID_REG_OFF       0x28   /* HID: Header ID */
#define MBOOT_HDATA_REG_OFF     0x2C   /* HDATA: Header Data */
#define MBOOT_HFLAG_REG_OFF     0x30   /* HFLAG: Header Flag */
#define MBOOT_MAGNUM_REG_OFF    0x34   /* MAGNUM: Magic Number */
#define MBOOT_HISTORY_REG_OFF   0x38

/* FPGA SPI Master Controller Registers */
#define FPGA_SPI_CTRL_REG_OFF   0x40
#define FPGA_SPI_STAT_REG_OFF   0x44
#define FPGA_SPI_RSZ_REG_OFF    0x48   /* RSZ: Read Size */
#define FPGA_SPI_DATA_REG_OFF   0x4C
#define FPGA_SPI_ADDR_REG_OFF   0x50

/* Secure Boot Registers */
#define SB_CORE_STAT_G_REG_OFF  0x60   /* SB: secure boot, G: Golden */
#define SB_CHK_STAT_G_REG_OFF   0x64   /* CHK: check */
#define SB_SIG_ADDR_G_REG_OFF   0x68   /* SIG: signature */
#define SB_SIG_SZ_G_REG_OFF     0x6C   /* SZ: size */
#define SB_CORE_STAT_U_REG_OFF  0x70   /* U: Upgrade */
#define SB_CHK_STAT_U_REG_OFF   0x74
#define SB_SIG_ADDR_U_REG_OFF   0x78
#define SB_SIG_SZ_U_REG_OFF     0x7C


/*
 * Bit definition of FPGA registers
 */
/* FPGA General registers */
/* Definition of FPGA revision register(0x00) */
#define FPGA_CREATE_YEAR_MSK      0xFF000000
#define FPGA_CREATE_YEAR_OFF      24
#define FPGA_CREATE_MON_MSK       0x00FF0000
#define FPGA_CREATE_MON_OFF       16
#define FPGA_CREATE_DAY_MSK       0x0000FF00
#define FPGA_CREATE_DAY_OFF       8
#define FPGA_CREATE_HOUR_MSK      0x000000FF

/* Definition of Host Type register(0x04) */
#define HOST_TYPE                 0x10
#define HOST_TYPE_OFF             4

/* Reg.0x04, Bit[4] - Host Type */
#define HOST_IS_G2                1
#define HOST_IS_NGIO              0

/* Definition of Board ID register(0x05) */
#define BOARD_REV                 0x0F

/* Definition of CPU0 Attched Individual Device Reset register(0x06) */
#define CPU0_I2C_MUX_RST          0x20
#define CPU0_I2C_MUX_RST_OFF      5
#define EXT_10GKR_RST             0x10
#define EXT_10GKR_RST_OFF         4
#define EXT_CPU0_EUSB_RST         0x08
#define EXT_CPU0_EUSB_RST_OFF     3
#define EXT_CPU0_USB1_RST         0x04
#define EXT_CPU0_USB1_RST_OFF     2
#define EXT_CPU0_USB0_RST         0x02
#define EXT_CPU0_USB0_RST_OFF     1
#define EXT_GE_PHY0_RST           0x01

/* Definition of CPU1 Attched Individual Device Reset register(0x07) */
#define CPU1_I2C_MUX_RST          0x04
#define CPU1_I2C_MUX_RST_OFF      2
#define EXT_CPU1_USB0_RST         0x02
#define EXT_CPU1_USB0_RST_OFF     1
#define EXT_GE_PHY1_RST           0x01

/* Definition of CPU Reset register(0x08) */
#define CPU1_RST                  0x02
#define CPU1_RST_OFF              1
#define CPU0_RST                  0x01

/* Definition of LED Control register(0x09) */
#define SYS_LED_BLINK_MSK         0x04
#define SYS_LED_BLINK_OFF         2
#define SYS_LED_COLOR             0x03

/* Reg.0x09, Bit[2] - System LED Control */
#define SYS_LED_BLINKING          1
#define SYS_LED_SOLID             0

/* Reg.0x09, Bit[1:0] - System LED Color */
#define SYS_LED_OFF               0
#define SYS_LED_GREEN             1
#define SYS_LED_YELLOW            2
#define SYS_LED_RESERVED          3

/* Definition of CPU0 WDT register(0x0A) */
#define CPU0_WDT_TIMER_MSK        0xFF

/* Definition of CPU1 WDT register(0x0B) */
#define CPU1_WDT_TIMER_MSK        0xFF

/* Definition of GPIO Expander Low 8-bit register(0x0C) */
#define GPIO_EXP_RST_CONF         0x20   /* EXP: expander */
#define UART_MUX_SEL              0x10
#define UART_MUX_SEL_OFF          4
#define PRI_INTF_READY            0x08   /* PRI: primary, INTF: interface */
#define ALIEN_MOD_RST             0x04
#define BOOT_SEL                  0x02
#define BOOT_SEL_OFF              1
#define DB_PRESENT                0x01

/* Reg.0x0C, Bit[4] - UART Mux Select */
#define PRI_UART_TO_HOST          0
#define SEC_UART_TO_HOST          1

/* Reg.0x0C, Bit[1] - Boot Select */
#define BOOT_GOLDEN_IMG           0
#define BOOT_UPGRADE_IMG          1

/* Definition of GPIO Expander High 8-bit register(0x0D) */
#define HOST_E1_SUPPORT_10G       0x02
#define HOST_E0_SUPPORT_10G       0x01

/* Definition of Clock Margin setting register(0x0E) */
#define CLK_MARGIN_STAT_MSK       0xC0
#define CLK_MARGIN_STAT_OFF       6
#define CLK_MARGIN_TIMER_EN_MSK   0x3F

/* Reg.0x0E, Bit[7:6] - Clock Margin Status */
#define CLK_SLOW_MODE             0x0
#define CLK_NORMAL_MODE           0x1
#define CLK_FAST_MODE             0x3

/* Definition of CPU0 Mux Control register(0x0F) */
#define SPI_MUX_TRANS_DONE        0x02
#define CPU0_SPI_MUX_SEL          0x01

/* Reg.0x0F, Bit[0] - CPU0 SPI ROM Mux Selection */
#define TO_BS_CFG_ROM             1   /* BS: bit-stream */
#define TO_CPU0_SPI_FLASH         0

/* Definition of System Power Saving register(0x10) */
#define PSE2_BS_CFG_DONE          0x20
#define PSE2_SUSPEND              0x10
#define PWR_MODE_10GKR_B          0x02   /* B: channel B */
#define PWR_MODE_10GKR_B_OFF      1
#define PWR_MODE_10GKR_A          0x01   /* A: channel A */

/* Reg.0x10, Bit[1] & Bit[0] - 10GKR power mode */
#define IN_10GKR_NORMAL_MODE      1
#define IN_10GKR_PWR_DOWN_MODE    0

/* Definition of 10GKR PRBS Control/Status register(0x11) */
#define TLK_LS_OK_OUT_A           0x80   /* LS: Lane aligenment Status */
#define TLK_LOS_A                 0x40   /* LOS: Lose Of Signal */
#define TLK_LS_OK_OUT_B           0x20
#define TLK_LOS_B                 0x10
#define PRBS_10GKR_PASS           0x08
#define PRBS_10GKR_EN             0x04
#define PRBS_10GKR_EN_OFF         2

/* Definition of USB Mode Control register(0x12) */
#define USB0_PWR_STAT             0x02
#define USB0_PWR_STAT_OFF         1
#define CPU0_USB0_MOD_CTRL        0x01

/* Reg.0x12, Bit[1] - USB0 Power Status(USB Fault) */
#define USB0_IS_OK                1
#define USB0_IS_FAULT             0

/* Reg.0x12, Bit[0] - USB0 Mode Control */
#define USB0_HOST_MOD             1
#define USB0_DEV_MOD              0

/* Definition of CPU0 BIB and eUSB WP Control register(0x13) */
#define CPU0_EUSB_WP              0x02   /* WP: Write Protection */
#define CPU0_BIB_WP               0x01

/* Definition of CPU1 BIB WP Control register(0x14) */
#define CPU1_BIB_WP               0x01

/* Definition of eUSB Control register(0x15) */
#define EUSB_LED_TEST_EN          0x04
#define EUSB_LED_TEST             0x02
#define EUSB_LED_TEST_OFF         0x02
#define EUSB_PWR_EN               0x01

/* Reg.0x15, Bit[1] - eUSB LED (ON/OFF) */
#define EUSB_LED_OFF              1
#define EUSB_LED_ON               0

/* Definition of Reset Event Log register(0x16) */
#define RESET_LOG_MSK             0x0F

/* Reg. 0x16, Bit[3:0] - Reset Events */
#define SECURE_BOOT_RST           0x5
#define CLK_MARGIN_RST            0x4
#define CPU1_WDT_RST              0x3
#define CPU0_WDT_RST              0x2
#define ALIEN_SUB_RST             0x1
#define MOD_PWR_UP_RST            0x0

/* Definition of PCIe Mode Control register(0x17) */
#define PCIE_MODE_CTRL            0x01

/* Reg.0x17, Bit[0] - PCIe Mode Control */
#define CPU0_EP_CPU1_RC_MODE      1     /* EP: end-point, RC: root complex. */
#define CPU1_EP_CPU0_RC_MODE      0

/* Definition of Interrupt register(0x18) */
#define SW_INTRUPT_SIM            0x80
#define CPU1_PWR_EN_MSK           0x60
#define CPU1_PWR_EN_OFF           5
#define CPU0_AMB_OVERTEMP         0x10   /* AMB: Ambient */
#define CPU1_PCB_OVERTEMP         0x08
#define CPU1_TJ_OVERTEMP          0x04
#define CPU0_TJ_OVERTEMP          0x02
#define CPU1_WDT_TIMEOUT          0x01

/* Reg. 0x18, Bit[6:5] - CPU1 power ON/OFF state */
#define CPU1_PWR_ON               0x3
#define CPU1_GO_PWR_ON            0x2
#define CPU1_GO_PWR_OFF           0x1
#define CPU1_PWR_OFF              0x0

/* Definition of Interrupt Enable register(0x19) */
#define INTRUPT_TEST_EN           0x80
#define CPU1_PWR_EN               0x20
#define CPU0_PCB_TS_EN            0x10   /* TS: Thermal sensor */
#define CPU1_AMB_TS_EN            0x08
#define CPU1_TJ_TS_EN             0x04
#define CPU0_TJ_TS_EN             0x02
#define CPU1_WDT_EN               0x01

/* FPGA Multiboot registers */
/* Definition of Multiboot Control register(0x20) */
#define FPGA_FALLBACK_OCCUR       0x00000010
#define FPGA_IMG_TYPE             0x0000000C
#define FPGA_IMG_TYPE_OFF         2
#define RESET_FSM                 0x00000002
#define READ_UPGRADE_HEADER       0x00000001

/* Reg.0x20, Bit[3:2] - Image Type */
#define FPGA_GOLDEN_IMG           0
#define FPGA_UPGRADE_IMG          1

/* Definition of Multiboot Status register(0x24) */
#define MBOOT_RECONF_ATTEMPT      0x00000001

/* Definition of Multiboot Header ID register(0x28) */
#define MBOOT_HEADER_ID           0xFFFFFFFF

/* Definition of Multiboot Header DATA register(0x2C) */
#define FPGA_REV_DATA             0xFFFFFFFF

/* Definition of Multiboot Header Flag register(0x30) */
#define FPGA_PAD_UP_TYPE          0x000000FC   /* UP: update */
#define FPGA_PAD_UP_TYPE_OFF      2
#define FPGA_UP_TYPE              0x00000003

/* Reg.0x30, Bit[7:2] - Update Type (Pad) */
#define FPGA_PAD_MATCH            0x28   /* 101000b - Pad matches */

/* Reg.0x30, Bit[1:0] - Update Type */
#define FPGA_ALWAYS_UP            0x0
#define FPGA_UP_NEW_REV           0x1   /* Upgrade if NEW Rev ID > OLD Rev ID */
#define FPGA_UP_REV_NOT_SAME      0x2   /* Upgrade if NEW Rev ID != OLD Rev ID */
#define FPGA_NOT_UP               0x3   /* Do not upgrade FPGA */

/* Definition of Multiboot Magic Number register(0x34) */
#define MBOOT_MAGIC_NUM           0xFFFFFFFF

#define FPGA_UPGRADE_IMG_MGANUM   0x7E4F5D06

/* Definition of Multiboot State History register(0x38) */
#define MBOOT_MB_STAT_HIS         0xFFFFFFFF

/* Reg.0x38, Bit[31:0] - State Types */
#define RESET_STAT                0x00000000
#define SET_IMG_TYPE_STAT         0x00000001
#define READ_IMG_TYPE_STAT        0x00000002
#define READ_PROM_STAT            0x00000003
#define READ_PROM_HEADER_STAT     0x00000004
#define WRITE_HEADER_STAT         0x00000005
#define READ_MAG_NUM_STAT         0x00000006
#define DET_IMG_TYPE_STAT         0x00000007   /* Dertermine Image type and Fallback */
#define RECONF_STAT               0x00000008
#define STOP_STAT                 0x00000009

/* FPGA SPI Master Controller registers */
/* Definition of FPGA SPI Control register(0x40) */
#define SPI_DONE_INTRUPT_EN       0x00008000
#define SPI_BAUD_DIV_MSK          0x000000F0   /* Baud Rate Divisor */
#define SPI_BAUD_DIV_OFF          4
#define SPI_DUMMY_BYTE            0x00000004
#define SPI_DATA_DIR              0x00000002
#define SPI_DATA_DIR_OFF          1
#define SPI_ADDR_FIELD            0x00000001

/* Reg.0x40, Bit[1] - Data Direction */
#define SPI_WRITE_DIR             1   /* Host -> SPI device (write) */
#define SPI_READ_DIR              0   /* SPI device -> Host (read) */

/* Definition of FPGA SPI Status register(0x44) */
#define SPI_OP_DONE               0x00008000   /* OP: operation */
#define SPI_WR_FIFO_FULL          0x00000010   /* WR: write */
#define SPI_WR_FIFO_EMPTY         0x00000008
#define SPI_RD_FIFO_FULL          0x00000004   /* RD: read */
#define SPI_RD_FIFO_EMPTY         0x00000002
#define SPI_WR_FIFO_OR            0x00000001   /* OR: overrun */

/* Reg.0x44, Bit[15] - SPI Operation Done */
#define CLEAR_SPI_OP_DONE         0x00008000

/* Definition of FPGA SPI Read size register(0x48) */
#define SPI_RD_SIZE               0x000000FF

/* Definition of FPGA SPI Data register(0x4C) */
#define SPI_DATA                  0x000000FF

/* Definition of FPGA SPI Address & Opcode register(0x50) */
#define SPI_OPCODE                0xFF000000
#define SPI_OPCODE_OFF            24
#define SPI_ADDR                  0x00FFFFFF

/* Secure Boot registers */
/* Definition of Secure Boot Core Status Golden register(0x60) */
/* Definition of Secure Boot Core Status Upgrade register(0x70) */
#define SB_SYS_INVALID            0x02000000
#define SB_DONE                   0x01000000
#define SB_READY                  0x00800000
#define SB_VERSION                0x007C0000
#define SB_VERSION_OFF            18
#define SB_PRE_STAGE              0x00030000
#define SB_PRE_STAGE_OFF          16
#define SB_CURR_STAGE             0x0000C000
#define SB_CURR_STAGE_OFF         14
#define SB_CTRL_STAT              0x00003000
#define SB_CTRL_STAT_OFF          12
#define SB_BC_COUNT               0x00000F00   /* BC: Boot check */
#define SB_BC_COUNT_OFF           8
#define SB_DS_TICK_COUNT          0x000000FF

/* Reg.0x60, Bit[22:18] - Secure Boot version */
#define SB_KEY_VER                0x10
#define SB_KEY_VER_OFF            4
#define SB_DEV_VER                0   /* DEV_VER: development version */
#define SB_REL_VER                1   /* REL_VER: release resion */
#define SB_CORE_VER               0x0F

/* Reg.0x60, Bit[15:14] - Stage of core operation */
#define SB_INIT                   0
#define SB_VERIFY_FPGA_BS         1   /* Verify FPGA Bitstream */
#define SB_VERIFY_BOOT_SEQ        2
#define SB_FINISH                 3

/* Reg.0x60, Bit[13:12] - Control statue */
#define SB_START                  0
#define SB_RUN                    1
#define SB_WAIT                   2
#define SB_STOP                   3

/* Definition of Secure Boot Check Status Golden register(0x64) */
/* Definition of Secure Boot Check Status Upgrade register(0x74) */
#define SB_RCF_TIMEOUT            0x40000000
#define SB_BOOT_TIMEOUT           0x10000000
#define SB_CS_TYPE                0x0C000000   /* CS: Code Signing */
#define SB_CS_TYPE_OFF            26
#define SB_CS_INDEX               0x03C00000
#define SB_CS_INDEX_OFF           22
#define SB_CS_RET_CODE            0x003F0000
#define SB_CS_RET_CODE_OFF        16
#define SB_CS_GOOD                0x00008000
#define SB_CS_DONE                0x00004000
#define SB_CHK_COUNT_ERR          0x00002000
#define SB_RUNTIME_ERR            0x00001000
#define SB_WR_ADDR_ERR            0x00000800
#define SB_RST_VECTOR_ERR         0x00000400
#define SB_CONF_ERR               0x00000200
#define SB_RST_ASSERT_ERR         0x00000100
#define SB_RT_HASH_VALID          0x00000080
#define SB_RT_HASH_VERIFIED       0x00000040
#define SB_BOOT_VER               0x00000020
#define SB_BOOT_VER_ERR           0x00000010
#define SB_FPGA_TIMEOUT           0x00000008
#define SB_FPGA_VALID             0x00000004
#define SB_FPGA_VERIFIED          0x00000002
#define SB_FPGA_VER               0x00000001

/* Reg.0x64, Bit[27:26] - Key Type for Bootloader Code Signing */
#define SB_CS_REL                 1
#define SB_CS_DEV                 2
#define SB_CS_ROLLOVER            3

/* Definition of Secure Boot Signature Address Golden register(0x68) */
/* Definition of Secure Boot Signature Address Upgrade register(0x78) */
#define SB_SIGN_ADDR              0xFFFFFFFF

/* Definition of Secure Boot Signature Size Golden register(0x6C) */
/* Definition of Secure Boot Signature Size Upgrade register(0x7C) */
#define SB_SIGN_SIZE              0x0000FFFF


/* Definition of szalinski Upgrade start address */
#define SZALINSKI_GOLDEN_IMAGE_BASE      0x000000
#define SZALINSKI_UPGRADE_IMAGE_BASE     0x200000
#define SZALINSKI_FLASH_SECTOR_SIZE      0x10000
#define SZALINSKI_FLASH_MAX_LEN          0x1000

/* Skye SPI flash definitions */
#define SKYE_CPU_SROM    0x00
#define SKYE_FPGA_SROM   0x03

/* FPGA SPI flash Lock/Unlock definitions */
#define SROM_STATUS_BP_MSK     0x1C

#define UNLOCK_FPGA_SROM_GLD   0x00
#define LOCK_FPGA_SROM_GLD     0x18

#endif /* __FPGA_SZALINSKI_H__ */

/*
$Log: fpga_szalinski.h,v $
Revision 1.2  2015/05/25 03:59:10  steja
Add Support Skye SM

Revision 1.1.4.4  2015/05/23 19:06:21  palin2
Add utilites to lock/unlock Skye FPGA SPI flash.

Revision 1.1.4.3  2015/05/11 13:45:41  steja
Code clean up <CSCuu14285>

Revision 1.1.4.2  2015/04/29 11:36:26  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.2  2014/08/27 11:20:34  palin2
Update szalinski utilities for Skye.

Revision 1.1.2.1  2014/07/21 01:56:37  palin2
Initial check-in Skye module side Diag code.

$Endlog$
*/

