/* $Id: diag_fpga_lib.h,v 1.4 2015/02/14 12:48:41 kodko Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_fpga_lib.h,v $ 
 *------------------------------------------------------------------
 * diag_fpga_lib.h
 * 
 * February 2012, Leslie Chen
 * Copyright (c) 2014 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_LIB_H__
#define __DIAG_FPGA_LIB_H__

#define I2C_BURST_SIZE                      (8)

/* === Register Definition === */

/* Address Offset */
#define FPGA_NGIO_GPIO_REG                  (0x00)
#define FPGA_CPU_RST_CTRL_REG               (0x02)
#define FPGA_PWR_STS_CTRL_REG               (0x03)
#define FPGA_PWR_ON_RTRY_CTR_REG            (0x04)
#define FPGA_SM_RST_DEV_EN_REG              (0x05)
#define FPGA_DEV_SETTING_REG                (0x07)
#define FPGA_DEV_STATUS_SIG_REG             (0x08)
#define FPGA_CLK_MUX_STATUS_REG             (0x09)
#define FPGA_TRIG_MUX_CTRL_REG              (0x1D)
#define FPGA_JITT_CLEANER_STS_REG           (0x0A)
#define FPGA_SFP_TX_FAULT_REG               (0x0B)
#define FPGA_SFP_PRESENT_STS_REG            (0x0C)
#define FPGA_SFP_RX_LOSS_CTRL_STS_REG       (0x0D)
#define FPGA_SFP3_RATE_SEL_REG              (0x0E)
#define FPGA_SFP4_DIS_CTRL_STS_REG          (0x0F)
#define FPGA_INT_STS_REG                    (0x10)
#define FPGA_INT_MASK_REG                   (0x11)
#define FPGA_RST_SIG_REG                    (0x16)
#define FPGA_SCRATCHPAG_REG                 (0x4F)
#define FPGA_REMOTE_UPD_CFG_REG             (0x70)
#define FPGA_REMOTE_UPD_CTRL_REG            (0x71)
#define FPGA_REMOTE_UPD_STS_REG             (0x72)
#define FPGA_REMOTE_UPD_DATA1_IN            (0x73)
#define FPGA_REMOTE_UPD_DATA2_IN            (0x74)
#define FPGA_REMOTE_UPD_DATA3_IN            (0x75)
#define FPGA_REMOTE_UPD_DATA1_OUT           (0x76)
#define FPGA_REMOTE_UPD_DATA2_OUT           (0x77)
#define FPGA_REMOTE_UPD_DATA3_OUT           (0x78)
#define FPGA_REMOTE_UPD_DATA4_OUT           (0x79)
#define FPGA_REMOTE_UPD_SPI_ADDR1           (0x7A)
#define FPGA_REMOTE_UPD_SPI_ADDR2           (0x7B)
#define FPGA_REMOTE_UPD_SPI_ADDR3           (0x7C)
#define FPGA_REMOTE_UPD_DATA_IN             (0x7D)
#define FPGA_REMOTE_UPD_DATA_OUT             (0x7E)
#define FPGA_SCRATCHPAD_REG                 (0x4F)
#define FPGA_LOW_VER_REG                    (0xFE)
#define FPGA_HIGH_VER_REG                   (0xFF)

/* FPGA Local Bus Chip Select One Start Addr */
#define FPGA_LOCAL_BUS_START_ADDR    (0x1D060000)
#define FPGA_LOCAL_BUS_LENGTH    (0xFFFF)

/* Reset Signal */
#define FPGA_GE_PHY_RST_L                   (0x01)
#define FPGA_TLK10232_RST_L                 (0x02)
#define FPGA_88X2222_RST_L                  (0x04)
#define FPGA_CPU_PCIE_RST_L                 (0x08)
#define FPGA_ICS8413S_RST_L                 (0x10)
#define FPGA_ACT_RST_L                      (0x20)
#define FPGA_GE_PHY_3P3_RST_L               (0x40)

/* SFP Status Register */
#define FPGA_GEP0_G0_SFP_STS                (0x36)
#define FPGA_GEP0_G1_SFP_STS                (0x37)
#define FPGA_GEP0_G2_SFP_STS                (0x38)
#define FPGA_GEP0_G3_SFP_STS                (0x39)
#define FPGA_GEP1_G0_SFP_STS                (0x3a)
#define FPGA_GEP1_G1_SFP_STS                (0x3b)

/* SFP Control Register */
#define FPGA_GEP0_G0_SFP_CTL                (0x30)
#define FPGA_GEP0_G1_SFP_CTL                (0x31)
#define FPGA_GEP0_G2_SFP_CTL                (0x32)
#define FPGA_GEP0_G3_SFP_CTL                (0x33)
#define FPGA_GEP1_G0_SFP_CTL                (0x34)
#define FPGA_GEP1_G1_SFP_CTL                (0x35)

/* PHY Status LED Register */
#define PHY_STS_LED_REG_0                   (0x21)
#define PHY_STS_LED_REG_1                   (0x22)
#define PHY_STS_LED_REG_2                   (0x23)
#define PHY_STS_LED_REG_3                   (0x24)

/* SFP Control Register bit */
#define GEP_SFP_CTL_TX_FAULT_INT_EN         (0x80)
#define GEP_SFP_CTL_RX_LOSS_INT_EN          (0x40)
#define GEP_SFP_CTL_SFP_PRESENT_INT_EN      (0x20)
#define GEP_SFP_CTL_TX_FAULT_OVER           (0x10)
#define GEP_SFP_CTL_RX_LOSS_OVER            (0x08)
#define GEP_SFP_CTL_SFP_PRESENT_OVER        (0x04)
#define GEP_SFP_CTL_TX_DISABLE              (0x02)
#define GEP_SFP_CTL_SFP_RATE_SEL            (0x01)

/* SFP Status Register bit */
#define GEP_SFP_STS_TX_FAULT                (0x20)
#define GEP_SFP_STS_RX_LOSS                 (0x10)
#define GEP_SFP_STS_PRESENT                 (0x08)
#define GEP_SFP_STS_INT_TX_FAULT            (0x04)
#define GEP_SFP_STS_INT_RX_LOSS             (0x02)
#define GEP_SFP_STS_INT_PRESENT             (0x01)

/* FPGA Clock Mux Control Signal Register bit*/
#define FPGA_CLK_MUX_X2222P                 (0x0)
#define FPGA_CLK_MUX_CPU                    (0x1)
#define FPGA_CLK_MUX_GE0                    (0x2)
#define FPGA_CLK_MUX_GE1                    (0x3)

/* FPGA Trigger Mux Control Signal Register bit*/
#define FPGA_TRIG_GE0                       (0x0)
#define FPGA_TRIG_GE1                       (0x1)
#define FPGA_TRIG_CPU                       (0x2)
#define FPGA_TRIG_X2222P                    (0x3)

#define FPGA_TRIG_OR                        (0x0)
#define FPGA_TRIG_AND                       (0x4)
#define FPGA_TRIG_MUX                       (0x8)

#define FPGA_TRIG_MUX_GE0 (FPGA_TRIG_MUX | FPGA_TRIG_GE0) << 4
#define FPGA_TRIG_MUX_GE1 (FPGA_TRIG_MUX | FPGA_TRIG_GE1) << 4
#define FPGA_TRIG_MUX_CPU (FPGA_TRIG_MUX | FPGA_TRIG_CPU) << 4
#define FPGA_TRIG_MUX_X2222P (FPGA_TRIG_MUX | FPGA_TRIG_X2222P) << 4

#define SECTOR_ERASE_TIMEOUT                (2000) /* 2 seconds */
/* Remote Upgrade Flash Sector Offset */
#define REMOTE_UPDATE_GOLDEN_SECT_0         (0x00)
#define REMOTE_UPDATE_GOLDEN_SECT_1         (0x01)
#define REMOTE_UPDATE_NORMAL_SECT_0         (0x04)
#define REMOTE_UPDATE_NORMAL_SECT_1         (0x05)
#define REMOTE_UPDATE_NORMAL_SECT_2         (0x06)
#define REMOTE_UPDATE_NORMAL_SECT_3         (0x07)
#define REMOTE_UPDATE_START_PAGE               (0x00)
#define REMOTE_UPDATE_START_BYTE_ADDR     (0x00)

/* NGIO GPIO Expander Register, offset 0x0 */

/* Remote Update Configuration Register, offset 0x70 */
#define REMOTE_UPDATE_FLASH_UPDATE_EN       (0x20)

/* Remote Update Control Register, offset 0x71 */
#define REMOTE_UPDATE_FLASH_SECTOR_ERASE    (0X04)
#define REMOTE_UPDATE_ALTRU_RELOAD          (0x01)

/* Remote Update Status Register, offset 0x72 */
#define REMOTE_UPDATE_STS_BUSY              (0x01)
#define REMOTE_UPDATE_STS_POF_ERROR         (0x02)
#define REMOTE_UPDATE_STS_FLASH_BUSY        (0x04)

/* Device Setting Register (reg0x07) bit definition */
#define FLASH_CFG_P0_HIGH                   (0x1)

#define FPGA_3120_RESET                     (0x2)

/* Irq_L will become low when Test_Reg = 0x55 */
#define IRQL_LOW                            (0x55)
#define CLEAR_IRQL_LOW                      (0x0)

/* Write value 0xff to register 0x10 to clear previous intr status */
#define CLEAR_INTR_STATUS (0xff)

/* PTP FPGA Registe - IDT 813N252 */
#define IDT_CTRL_REG (0xb)
#define DEV_STATUS_REG (0xe)
#define SYNC_OUT_CLK_VALID (0x1)
#define SYNC_OUT_CLK_25M (0x0)
#define CLK_CTRL_25M (0x5)
#define CLK_CTRL_8K (0x1)
#define SYNC_OUT_CLK_MASK (0x6)
#define IDT_READY_MASK (0x1)
#define IDT_CLK0_25M (0x4)
#define IDT_CLK0_8K (0x0)
#define IDT_PHY_READY (0x0)
#define FPGA_SYNC_OUT_SHIFT (0x7)
#define FPGA_SYNC_OUT_VALID_SHIFT (0x6)
#define FPGA_SYNC_OUT_VALID_MASK (0x7f)
#define FPGA_RESET_SIGNAL_REG    (0x16)
#define GE_PHY_READY_MASK (0x1)
#define GE_PHY_READY (0x1)

/* PTP FPGA Trigger In Registe, BP -> FPGA */
#define FPGA_PTP_TRIG_OUT_SEL_REG (0xA1)
#define FPGA_SYCN_TRIG_OUT (0x4)

/* PTP FPGA Trigger In Registe, FPGA -> BP */
#define FPGA_PTP_TRIG_IN_SEL_REG (0xA2)
#define FPGA_PTP_TRIG_FRM_GE0 (0x1)
#define FPGA_PTP_TRIG_FRM_GE1 (0x2)
#define FPGA_PTP_TRIG_FRM_CPU (0x4)
#define FPGA_PTP_TRIG_FRM_X222P (0x8)
#define FPGA_SYCN_TRIG_IN (0x10)

/* SKU ID */
#define FPGA_ID_SKU1                            (0x1)
#define FPGA_ID_SKU2                            (0x0)
#define WOODLAWN_4GE_1XAUI                      (0x10)
#define WOODLAWN_6GE                            (0x11)
#define WOODLAWN_6GE_1XAUI                      (0x15) /* Not official SKU */
#define FPGA_ID_MASK                            (0xffcf)

/* SFP LED definition */
#define FPGA_SFP_LED_EN                         (0x1)
#define FPGA_SFP_LED_SPEED                      (0x2)

/* Platform GE0 is 10GKR capable */
#define FPGA_10GKR_CAPABLE (0x3)
#define GE0_10GKR_CAPABLE  (0x1)

/* === Register Definition === */

typedef uint8_t fpga_p;                     /* FPGA One Byte Size Register */
#define sfp_mask (0x80)

#define FPGA_WAIT_PHY_READY                     (100)
#define FPGA_TRIG_VERIFY_TIME                   (10000)
#define FPGA_TRIG_VERIFY_NUM                    (3)

extern int fpga_reg_read(int, char *);
extern int fpga_reg_write(int, char);
extern int fpga_reg_or(int, char);
extern int fpga_reg_nand(int, char);
extern int fpga_upgrade_sector_erase(int);
extern int is_sfp_present(int, int);
extern int get_sku_id(void);
extern uchar* fpga_get_local_bus_addr(void);
extern int enable_sfp_tx_transmit(int, int);
extern int fpga_toggle_sfp_led(int, int, int);
extern int config_fpga_clk_mux_sel(uint8_t);
extern int config_fpga_trig_mux_sel(uint8_t);
extern int verify_fpga_sync_clk_out(void);
extern int verify_fpga_sync_trig_out(void);

#endif
/*-------------------------------------------------
 * $Log: diag_fpga_lib.h,v $
 * Revision 1.4  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.3  2014/11/12 06:32:59  leschen
 * Support Greyhound switch a
 *
 * Revision 1.2.8.2  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.2.8.1  2014/03/11 02:29:30  leschen
 * Add macros to support 1588 Cclk/trig verificatoin.
 *
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.2  2013/05/09 05:42:02  leschen
 * Update fpga reset phy register mapping
 *
 * Revision 1.1.2.1  2013/04/24 10:37:15  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.5  2013/03/27 07:58:53  leslie
 * Add FPGA local bus macros
 *
 * Revision 1.4  2013/03/20 06:33:53  kuangik
 * Add I2C_BURST_SIZE
 *
 * Revision 1.1  2013/03/13 06:42:50  kuangik
 * Add for the first time
 *
 * Revision 1.18  2013/03/06 11:18:38  kuangik
 * Fix for 1112 internal and ge backplane loopback test
 *
 * Revision 1.16  2013/01/15 23:34:30  leslie
 * Define SKU id.
 *
 * Revision 1.15  2013/01/13 23:56:56  leslie
 * Add fpga id macros.
 *
 * Revision 1.14  2012/11/20 01:26:50  leslie
 * Add extern declaration of detect sfp present function.
 *
 * Revision 1.13  2012/11/19 02:22:17  leslie
 * Update for detect sfp module present function.
 *
 * Revision 1.12  2012/10/04 03:13:47  leslie
 * Update for FPGA program utility.
 *
 * Revision 1.11  2012/09/21 11:42:55  kody
 * Add FPGA reset definition.
 *
 * Revision 1.10  2012/08/28 08:33:05  leslie
 * Update for fpga test item.
 *
 * Revision 1.9  2012/08/03 10:16:55  leslie
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.6  2012/07/05 02:07:27  kody
 * Add Phy 3120 FW download utility.
 *
 * Revision 1.5  2012/05/30 01:33:47  leslie
 * Add typedef uint8_t fpga_p
 *
 * Revision 1.4  2012/04/16 12:33:39  kuangik
 * Add FPGA Firmware upgrade function
 *
 * Revision 1.3  2012/04/06 06:05:59  kuangik
 * Update for FPGA Test Item
 *
 * Revision 1.2  2012/02/10 06:46:16  leslie
 * Add Woodlawn fpga lib header file
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
