/* $Id: plug_host_fpga_lib.h,v 1.2 2018/11/23 08:49:52 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_host_fpga_lib.h,v $
 *------------------------------------------------------------------
 * Filename: plug_host_fpga_lib.h
 * Description: Header file for plug host functions for various fpga components
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_HOST_FPGA_LIB__
#define __PLUG_HOST_FPGA_LIB__


typedef enum
{
    FPGA_BIT_OPS_ON,
    FPGA_BIT_OPS_OFF
} fpga_bit_ops;

#define BIT(x)                          (1 << (x))
#define BIT_VAL(a,b)                    ((a & BIT(b)) >> b)

#define PLUG_FPGA_OFFSET                (0x10000)

#define PLUG_FPGA_SYS_SCR_REG           (PLUG_FPGA_OFFSET + 0x00)
#define PLUG_FPGA_SYS_BTYPE_REG         (PLUG_FPGA_OFFSET + 0x80)
#define PLUG_FPGA_SYS_REV_REG           (PLUG_FPGA_OFFSET + 0x84)
#define PLUG_FPGA_SYS_SEC_REV_REG       (PLUG_FPGA_OFFSET + 0x8C)
#define PLUG_FPGA_DBG_LED_ADDR_REG      (PLUG_FPGA_OFFSET + 0xF0)

#define PLUG_FPGA_SCRATCHPAD_OFFSET     (PLUG_FPGA_OFFSET)


/* PLUG Interrupt Rewg Offset */
#define PLUG_INTR_REG_OFFSET            (PLUG_FPGA_OFFSET + 0x0100)

#define PLUG_INTR_STAT_REG              (PLUG_INTR_REG_OFFSET + 0x00)
#define PLUG_INTR_ENA_REG               (PLUG_INTR_REG_OFFSET + 0x04)
#define PLUG_I2C_INTR_STAT_REG          (PLUG_INTR_REG_OFFSET + 0x20)
#define PLUG_I2C_INTR_ENA_REG           (PLUG_INTR_REG_OFFSET + 0x24)
#define PLUG_I2C_INTR_OVRI_REG          (PLUG_INTR_REG_OFFSET + 0x28)
#define PLUG_UART_INTR_STAT_REG         (PLUG_INTR_REG_OFFSET + 0x30)
#define PLUG_UART_INTR_ENA_REG          (PLUG_INTR_REG_OFFSET + 0x34)
#define PLUG_UART_INTR_OVRI_REG         (PLUG_INTR_REG_OFFSET + 0x38)
#define PLUG_OIR_INTR_STAT_REG          (PLUG_INTR_REG_OFFSET + 0x40)
#define PLUG_OIR_INTR_ENA_REG           (PLUG_INTR_REG_OFFSET + 0x44)
#define PLUG_OIR_INTR_OVRI_REG          (PLUG_INTR_REG_OFFSET + 0x48)
#define PLUG_MISC_INTR_STAT_REG         (PLUG_INTR_REG_OFFSET + 0x50)
#define PLUG_MISC_INTR_ENA_REG          (PLUG_INTR_REG_OFFSET + 0x54)
#define PLUG_MISC_INTR_OVRI_REG         (PLUG_INTR_REG_OFFSET + 0x58)


#define PLUG_INTR_STAT_I2C_BIT          (2)
#define PLUG_INTR_ENA_I2C_BIT           (2)
#define PLUG_I2C0_INTR_STAT_BIT         (1)
#define PLUG_I2C0_INTR_ENA_BIT          (1)
#define PLUG_I2C0_INTR_OVRI_BIT         (1)
#define PLUG_I2C_INTR_OVRI_CLEAN        (0)



/* PLUG I2C Controller Offset */
#define PLUG_I2C_CTRL_OFFSET            (PLUG_FPGA_OFFSET + 0x2000)
#define PLUG_FPGA_I2C_OFFSET            (0x100)


/* PLUG Module Register */
#define PLUG_MODULE_OFFSET              (PLUG_FPGA_OFFSET + 0x3000)
#define FPGA_MODULE_DEB_CTL_REG         (PLUG_MODULE_OFFSET + 0x00)
#define FPGA_PLUG1_STSCTL_REG           (PLUG_MODULE_OFFSET + 0x50)
#define FPGA_PLUG1_INTEN_REG            (PLUG_MODULE_OFFSET + 0x54)
#define FPGA_PLUG1_DEB_REG              (PLUG_MODULE_OFFSET + 0x58)
#define FPGA_PLUG2_STSCTL_REG           (PLUG_MODULE_OFFSET + 0x60)
#define FPGA_PLUG2_INTEN_REG            (PLUG_MODULE_OFFSET + 0x64)
#define FPGA_PLUG2_DEB_REG              (PLUG_MODULE_OFFSET + 0x68)

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

/* UART Controller Register */
#define PLUG_UART_CONTROLLER_OFFSET     (PLUG_FPGA_OFFSET + 0x1000)
#define PLUG_UART_CONTROLLER_1_OFFSET   (PLUG_FPGA_OFFSET + 0x1000 + 0x100)
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

/*
 * FPGA Multiboot Register
 */
#define FPGA_MULTIBOOT_OFFSET           (PLUG_FPGA_OFFSET + 0x4000)
#define FPGA_RECONF_CTRL_REG            (FPGA_MULTIBOOT_OFFSET + 0x00)
#define FPGA_RECONF_STS_REG             (FPGA_MULTIBOOT_OFFSET + 0x04)
#define FPGA_CACHED_REVID_REG           (FPGA_MULTIBOOT_OFFSET + 0x08)
#define FPGA_CACHED_REVDATE_REG         (FPGA_MULTIBOOT_OFFSET + 0x0C)
#define FPGA_CACHED_FLAGS_REG           (FPGA_MULTIBOOT_OFFSET + 0x10)
#define FPGA_CACHED_MAGIC_REG           (FPGA_MULTIBOOT_OFFSET + 0x14)
#define FPGA_MULTIBOOT_STATE_REG        (FPGA_MULTIBOOT_OFFSET + 0x18)
#define FPGA_MULTIBOOT_SEC_RSLT_REG     (FPGA_MULTIBOOT_OFFSET + 0x1C)

/* FPGA/NIOS SPI PROM Programming */
#define PLUG_FPGA_SPI_PROM_PROG_OFFSET       (PLUG_FPGA_OFFSET + 0x5000)
#define PLUG_FPGA_SPI_PROM_CTRL_REG          (PLUG_FPGA_SPI_PROM_PROG_OFFSET + 0x00)
#define PLUG_FPGA_SPI_PROM_STS_REG           (PLUG_FPGA_SPI_PROM_PROG_OFFSET + 0x04)
#define PLUG_FPGA_SPI_PROM_RD_SIZE_REG       (PLUG_FPGA_SPI_PROM_PROG_OFFSET + 0x08)
#define PLUG_FPGA_SPI_PROM_RW_DATA_REG       (PLUG_FPGA_SPI_PROM_PROG_OFFSET + 0x0C)
#define PLUG_FPGA_SPI_PROM_OP_ADDR_REG       (PLUG_FPGA_SPI_PROM_PROG_OFFSET + 0x10)

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

#define ACT2_RESET_UNRESET_DELAY    (500)
#define QUACK_RETRY                 8
#define ACT2_UNRESET_DELAY          (5000)

/*
 * Debug LED register bit
 */
#define PLUG_LTE_GPS_SYNC_STATUS_0        8
#define PLUG_LTE_GPS_SYNC_STATUS_1        9


#define FPGA_PLUG_OFFSET_BY_SLOT(offset, slot)  (offset + ((slot - 1) * 0x10))
#define PLUG_UART_CONTROL_OFFSET_BY_SLOT(offset, slot) (offset + ((slot - 1) * 0x100))

#define BDTYPE_STAR_C1101                (1)
#define BDTYPE_STAR_C1109_4P             (2) /*Need be change later*/


#define PLUG_FPGA_I2C_OP_DELAY                      (3)
#define PLUG_FPGA_I2C_OP_TOUT                       (500)
#define PLUG_FPGA_I2C_IDLE_TIMEOUT                   30
#define PLUG_FPGA_REG_WRITE_DELAY                   1000

#define SCL_DRIVE_TIMES   100
#define FIRST_SLOT                      (1)
#define MAX_PLUG_SLOT_C1101             (1)
#define MAX_PLUG_SLOT_NUMBER            (2)
#define MAX_PLUG_USB_MASS_STORE_ON_SYS  (8)


#define FPGA_EXTER_DEV_RST_REG       0x1004

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

/* Star GPS define diff with Curie */
#define PLUG_TESTCARD_GPS_SLOT1              (0x100)
#define PLUG_TESTCARD_GPS_SLOT2              (0x200)

typedef enum {
    PLUG_SLOT_1 = 1,
    PLUG_SLOT_2
} plug_slot_no;

typedef enum {
    PLUG_SLOT_PWR_OFF,
    PLUG_SLOT_PWR_ON
} plug_power_ctrl;

/*******************************
 *           extern
 *******************************/

extern boolean star_plug_is_present(uint); 
extern int plug_pwr_on_util(void); 
extern int plug_pwr_off_util(void); 
extern int i2c_err_no(uint32_t *);
extern int plug_fpga_i2c_rd(int, uint8_t, uint32_t, int32_t, 
                            uint32_t, uint32_t, uchar *);
extern int plug_fpga_i2c_wr(int, uint8_t, uint32_t, int32_t, 
                            uint32_t, uint32_t, uchar *);
extern unsigned long get_plug_fpga_i2c_addr(int);
extern int show_plug_fpga_ver(int);
extern int plug_fpga_reg_read(uint, uint *);
extern int plug_fpga_reg_write(uint, uint);
extern int plug_fpga_reg_or(uint, uint);
extern int plug_fpga_reg_nand(uint, uint);
extern int diag_fpga_reg_bitops(uint, uint, uint);
extern int fpga_read_32_reg(uint, uint *);
extern int fpga_write_32_reg(uint, uint );

#endif
/*-------------------------------------------------
$Log: plug_host_fpga_lib.h,v $
Revision 1.2  2018/11/23 08:49:52  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.5  2018/10/22 09:38:07  hondwang
move plug_slot_module_info to common codeplug_common/plug_common_lib.c

Revision 1.1.2.4  2018/10/16 07:08:45  hondwang
plug_tc_host_sgmii_present should be platform code, modified

Revision 1.1.2.3  2018/10/15 09:50:52  hondwang
move GPS pin define back to platform code, because Star and Curie have diff FPGA configure

Revision 1.1.2.2  2018/10/15 07:43:09  shjung
Re-struct for pluggable-LTE common codes

Revision 1.1.2.1  2018/10/15 06:44:32  hondwang
pluggable common code re-instruct add and remove files


$Endlog$
*/

