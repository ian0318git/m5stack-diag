/* $Id: diag_fpga_lib.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_fpga_lib.h,v $ 
 *------------------------------------------------------------------
 * diag_fpga_lib.h
 * 
 * Apr 2014, Xiaoying Zhang
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_LIB_H__
#define __DIAG_FPGA_LIB_H__

/* FPGA Local Bus Chip Select One Start Addr */
#define FPGA_LOCAL_BUS_START_ADDR               (0x1D060000)
#define FPGA_LOCAL_BUS_LENGTH                   (0xFFFF)

/* Address Offset */
#define FPGA_BOARD_ID                           (0x0004)
#define FPGA_CPU_DEV_RST                        (0x0005)
#define FPGA_SFP_LED_CTRL                       (0x0007)
#define FPGA_GPIO_EXP_LOW                       (0x0009)
#define FPGA_GPIO_EXP_HIGH                      (0x000A)
#define FPGA_PORT0_INT_CTRL                     (0x0013)
#define FPGA_PORT0_INT_STS                      (0x0014)
#define FPGA_PORT2_INT_CTRL                     (0x0015)
#define FPGA_PORT2_INT_STS                      (0x0016)
#define FPGA_PHY_ZL30254_CTRL_STAT              (0x001b)

#define FPGA_TS_CTRL                            (0x1000)
#define FPGA_TS_INT_CTRL                        (0x1001)
#define FPGA_TS_DATA_CNT                        (0x1002)
#define FPGA_TS_STAT                            (0x1003)

#define FPGA_TS_DATA_START                      (0x1100)
#define FPGA_TS_DATA_LENGTH                     (0x100)

#define FPGA_MB_REG_BASE                        (0x8100)
#define MB_CTRL_OFFSET                          (0x0)
#define MB_STAT_OFFSET                          (0x4)
#define MB_HDR_ID_OFFSET                        (0x8)
#define MB_HDR_DATE_OFFSET                      (0xc)
#define MB_HDR_FLAG_OFFSET                      (0x10)
#define MB_HDR_MAGIC_NUM_OFFSET                 (0x14)
#define MB_HISTORY_OFFSET                       (0x18)

#define FPGA_SPI_REG_BASE                       (0x9040)
#define FPGA_SPI_CTRL_OFFSET                    (0x0)
#define FPGA_SPI_STAT_OFFSET                    (0x4)
#define FPGA_SPI_RD_SIZE_OFFSET                 (0x8)
#define FPGA_SPI_DATA_OFFSET                    (0xc)
#define FPGA_SPI_ADDR_OFFSET                    (0x10)

/* Defines for CPU Attached Device reset */
#define FPGA_ZL_RST_L                           (0x01)
#define FPGA_PHY_RST_L                          (0x02)
#define FPGA_BOOTFLASH_RST_L                    (0x04)

/* Defines for GPIO Expander Registers */
#define FPGA_RST_CFG_L                          (0x20)
#define FPGA_UART_MUX_SEL                       (0x10)
#define FPGA_PRI_IF_RDY                         (0x08)
#define FPGA_ALIEN_SB_RST_L                     (0x04)
#define FPGA_BOOT_SEL                           (0x02)
#define FPGA_DB_PRESENT_L                       (0x01)

/* SFP Status Register */
#define FPGA_GEP0_G0_SFP_STS                    (0x14)
#define FPGA_GEP0_G2_SFP_STS                    (0x16)

/* SFP Control Register */
#define FPGA_GEP0_G0_SFP_CTL                    (0x30)
#define FPGA_GEP0_G1_SFP_CTL                    (0x31)
#define FPGA_GEP0_G2_SFP_CTL                    (0x32)
#define FPGA_GEP0_G3_SFP_CTL                    (0x33)
#define FPGA_GEP1_G0_SFP_CTL                    (0x34)
#define FPGA_GEP1_G1_SFP_CTL                    (0x35)

/* PHY Status LED Register */
#define PHY_STS_LED_REG_0                       (0x21)
#define PHY_STS_LED_REG_1                       (0x22)
#define PHY_STS_LED_REG_2                       (0x23)
#define PHY_STS_LED_REG_3                       (0x24)

/* SFP Control LED Register bit */
#define GEP_SFP_LED_Y                           (0x08)
#define GEP_SFP_LED_G                           (0x04)
#define GEP_SFP_LED_SPD                         (0x03)

/* SFP Control Register bit */
#define GEP_SFP_CTL_TX_FAULT_INT_EN             (0x80)
#define GEP_SFP_CTL_RX_LOSS_INT_EN              (0x40)
#define GEP_SFP_CTL_SFP_PRESENT_INT_EN          (0x20)
#define GEP_SFP_CTL_TX_FAULT_OVER               (0x10)
#define GEP_SFP_CTL_RX_LOSS_OVER                (0x08)
#define GEP_SFP_CTL_SFP_PRESENT_OVER            (0x04)
#define GEP_SFP_CTL_TX_DISABLE                  (0x02)
#define GEP_SFP_CTL_SFP_RATE_SEL                (0x01)

/* SFP Status Register bit */
#define GEP_SFP_STS_TX_FAULT                    (0x20)
#define GEP_SFP_STS_RX_LOSS                     (0x10)
#define GEP_SFP_STS_PRESENT                     (0x08)
#define GEP_SFP_STS_INT_TX_FAULT                (0x04)
#define GEP_SFP_STS_INT_RX_LOSS                 (0x02)
#define GEP_SFP_STS_INT_PRESENT                 (0x01)

/* Defines for PHY and ZL30245 Control and Status */
#define PHY_COMA_MODE_INPUT                     (0x40)
#define PHY_COMA_MODE_OUTPUT_EN                 (0x20)
#define PHY_COMA_MODE_OUTPUT                    (0x02)

/* Defines for PHY Timestamp Control and Status */
#define PHY_TS_ENABLE                           (0x04)
#define PHY_TS_INTR_ENABLE                      (0x02)
#define PHY_TS_INTR_OVERRIDE                    (0x01)

/* Defines for PHY Timestamp Interrupt Control */
#define PHY_TS_INT                              (0x04)

/* Defines for PHY Timestamp Status Control */
#define PHY_TS_START_TX                         (0x02)
#define PHY_TS_READY                            (0x01)

/* === Register Definition === */
typedef struct fpga_mb_reg_t_ {
    uint32_t mb_ctrl;           /* 0x8100, multiboot control register */
    uint32_t mb_stat;           /* 0x8104, multiboot status register */
    uint32_t mb_hdr_id;         /* 0x8108, multiboot header ID register */
    uint32_t mb_hdr_date;       /* 0x810c, multiboot header date register */
    uint32_t mb_hdr_flag;       /* 0x8110, multiboot header flag register */
    uint32_t mb_hdr_magic_num;  /* 0x8114, multiboot header magic num register */
    uint32_t mb_history;        /* 0x8118, multiboot state history register */
} fpga_mb_reg_t;


typedef struct fpga_spi_reg_t_ {
    uint8_t fpga_spi_ctrl[4];    /* 0x9040, FPGA spi control register */
    uint8_t fpga_spi_stat[4];    /* 0x9044, FPGA spi status register */
    uint8_t fpga_spi_rd_size[4]; /* 0x9048, FPGA spi read size register */
    uint8_t fpga_spi_data[4];    /* 0x904c, FPGA spi data register */
    uint8_t fpga_spi_addr[4];    /* 0x9050, FPGA spi address & opcode register */
} fpga_spi_reg_t;


/* define for fpga_spi_ctrl register */
#define FPGA_SPI_ADDR_EN                        0x1
#define FPGA_SPI_WRITE                          0x2
#define FPGA_SPI_DUMMY_BYTE_EN                  0x4

/* define for fpga_spi_stat register */
#define FPGA_SPI_WR_FIFO_ORUN                   0x1
#define FPGA_SPI_RD_FIFO_EMPTY                  0x2
#define FPGA_SPI_RD_FIFO_FULL                   0x4
#define FPGA_SPI_WR_FIFO_EMPTY                  0x8
#define FPGA_SPI_WR_FIFO_FULL                   0x10
#define FPGA_SPI_DONE                           0x80

#define FPGA_SPI_PAGE_SIZE                      256
#define FPGA_SECTOR_SIZE                        0x10000
#define SECONDARY_FPGA_IMAGE_START_ADDR         0x100000
#define GOLDEN_FPGA_IMAGE_START_ADDR            0x0

/* define for SPI flash opcode */
#define RD_IDENTIFICATION                       0x9f
#define RD_STATUS                               0x05
#define RD_DATA_BYTES                           0x03
#define RD_DATA_BYTES_HIGH_SPEED                0x0b
#define PAGE_PROGRAM                            0x02
#define WRITE_ENABLE                            0x06
#define SECTOR_ERASE                            0xd8
#define WR_STATUS                               0x01

/* Define for SPI flash status register */
#define RDSR_WIP                                0x01
#define RDSR_WEL                                0x02

#define SECTOR_ERASE_TIMEOUT                    (2000) /* 2 seconds */

/* Remote Upgrade Flash Sector Offset */
#define REMOTE_UPDATE_GOLDEN_SECT_0             (0x00)
#define REMOTE_UPDATE_GOLDEN_SECT_1             (0x01)
#define REMOTE_UPDATE_NORMAL_SECT_0             (0x04)
#define REMOTE_UPDATE_NORMAL_SECT_1             (0x05)
#define REMOTE_UPDATE_NORMAL_SECT_2             (0x06)
#define REMOTE_UPDATE_NORMAL_SECT_3             (0x07)
#define REMOTE_UPDATE_START_PAGE                (0x00)
#define REMOTE_UPDATE_START_BYTE_ADDR           (0x00)

// #define I2C_BURST_SIZE                       (8)
#define I2C_BURST_SIZE                          (4)

/* NGIO GPIO Expander Register, offset 0x0 */

/* Remote Update Configuration Register, offset 0x70 */
#define REMOTE_UPDATE_FLASH_UPDATE_EN           (0x20)

/* Remote Update Control Register, offset 0x71 */
#define REMOTE_UPDATE_FLASH_SECTOR_ERASE        (0X04)
#define REMOTE_UPDATE_ALTRU_RELOAD              (0x01)

/* Remote Update Status Register, offset 0x72 */
#define REMOTE_UPDATE_STS_BUSY                  (0x01)
#define REMOTE_UPDATE_STS_POF_ERROR             (0x02)
#define REMOTE_UPDATE_STS_FLASH_BUSY            (0x04)

/* Device Setting Register (reg0x07) bit definition */
#define FLASH_CFG_P0_HIGH                       (0x1)

#define FPGA_3120_RESET                         (0x2)

/* Irq_L will become low when Test_Reg = 0x55 */
#define IRQL_LOW                                (0x55)
#define CLEAR_IRQL_LOW                          (0x0)

/* Write value 0xff to register 0x10 to clear previous intr status */
#define CLEAR_INTR_STATUS (0xff)

/* SKU ID */
#define FPGA_ID_SKU1                            (0x1)
#define FPGA_ID_SKU2                            (0x0)
#define WALLANDER_1GE                           (0x0)
#define WALLANDER_2GE                           (0x1)
#define FPGA_ID_MASK                            (0x1)

/* SFP LED definition */
#define FPGA_SFP_LED_Y                          (0x1)
#define FPGA_SFP_LED_G                          (0x2)
#define FPGA_SFP_LED_SPD                        (0x3)

#define FPGA_SFP_LED_ON                         (0x1)
#define FPGA_SFP_LED_OFF                        (0x0)

#define FPGA_SFP_LED_SPD_0_BLINK                (0x0)
#define FPGA_SFP_LED_SPD_1_BLINK                (0x1)
#define FPGA_SFP_LED_SPD_2_BLINK                (0x2)
#define FPGA_SFP_LED_SPD_3_BLINK                (0x3)

typedef uint8_t fpga_p;                     /* FPGA One Byte Size Register */
#define sfp_mask (0x80)

extern int show_fpga_version();
extern int fpga_reg_rd_util();
extern int fpga_reg_wr_util();
extern int fpga_reg_dp_util();

extern int fpga_reg_read(int, char *);
extern int fpga_reg_write(int, char);
extern int fpga_reg_or(int, char);
extern int fpga_reg_nand(int, char);
extern int fpga_reg_read32(int, ulong *);
extern int fpga_reg_write32(int, ulong);
extern int fpga_upgrade_sector_erase(int);
extern int is_sfp_present(int, int);
extern int enable_sfp_tx(int);
extern int show_sfp_status(int, int);
extern int get_board_id(void);
extern uchar* fpga_get_local_bus_addr(void);
extern int enable_sfp_tx_transmit(int, int);
// extern int fpga_toggle_sfp_led(int, int, int);
extern int fpga_ctrl_sfp_led (int, int, int);

extern int fpga_set_ready_bit(void);
extern int fpga_reset_phy(void);
extern int fpga_enable_phy_coma_mode_output(void);
extern int fpga_turn_on_phy_coma_mode(void);
extern int fpga_turn_off_phy_coma_mode(void);

extern int fpga_enable_ts(void);
extern int fpga_enable_ts_intr(void);
extern int fpga_check_ts_intr(void);
extern int fpga_check_ts_ready(void);

extern int  spi_peek_reg(void);
extern int  spi_poke_reg(void);
extern int  peek_spi_flash(void);
extern int  fpga_upgrade_secondary(void);
extern int  fpga_upgrade_golden(void);
extern int  fpga_lock_golden(void);
extern int  fpga_unlock_golden(void);

#endif
/*-------------------------------------------------
 * $Log: diag_fpga_lib.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
