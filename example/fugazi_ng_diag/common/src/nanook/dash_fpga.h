 /* $Id: dash_fpga.h,v 1.3 2020/01/09 01:03:00 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/dash_fpga.h,v $
 *------------------------------------------------------------------
 * Filename:    dash_fpga.h
 *
 *
 * Copyright (c) 2012-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DASH_FPGA__
#define __DASH_FPGA__


#include "types.h"

/* !!!!!!! CPLD reset 0 TPM space */
#define DASH_VENDOR_ID            0x1172
#define DASH_DEVICE_ID            0xE001

#define USD_FPGA_VENDOR_ID            0x1137
#define USD_FPGA_DEVICE_ID            0x0130
/*Revision ID:  0x0001 ; Class Code:  0xFF0000
Subsystem Vendor ID:  0x1137; Subsystem ID:  0x0130
*/


#define DASH_SIZE                 0x50000
#define CPLD_SIZE                 0x1000
#define TOTAL_MSG_SIZE            0x15000

#define MSG_MCU_INDEX             0
#define MSG_SIZE                  0x400
#define MCU_BUF_INIT              1
#define MSG_DATA_SIZE             1024
#define ENV_MCU_TX_DONE           0x1
#define ENV_MCU_RX_DATA           0x2

#define CP   0
#define FP   1
#define NIOS 2

#define MAX_SFP  2
#define MAX_I2C  8

#define FPGA_DB_PRESENT_REG           0xC4
#define FPGA_DB_PRESENT_BIT           1


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

extern void show_cpu_temperature(void);
typedef struct nios_mbox_mem_t_ {
    volatile unsigned int cpu_tmp;       /* 0x0 */
} nios_mbox_mem_t ;



#define DASH_PAD(from_adr, to_adr, name) \
unsigned int name[(to_adr - from_adr)/sizeof(unsigned int)]


/* Externs */
extern int get_platform_selected_spi_prom();
extern int is_platform_cpu_hot();
extern int is_platform_thermal_trip();
extern int is_platform_catastrophic_err();
extern void set_platform_irq0_sts_msk(int);
extern void remove_platform_irq0_sts_msk(int);
extern void platform_irq0_test();
extern int sel_platform_ctrl0_reg(int);
extern int unsel_platform_ctrl0_reg(int);
extern unsigned int get_platform_brd_type(unsigned int);
extern unsigned int get_platform_brd_subtype(unsigned int);
extern unsigned int get_platform_brd_type(unsigned int);
extern int get_platform_ver(unsigned int, unsigned int *,
                            unsigned int *, unsigned int *,
                            unsigned int *);
extern void sata_cfg(boolean mode);
extern boolean is_sata_present(int);
extern int get_platform_boot(unsigned int, unsigned int *, unsigned int *);
extern int dash_rd_wr_test(int);
extern int get_platform_plane(void);
extern int get_platform_top_intr(void);
extern int set_nios_mode(int);


/* for reg 0x1c bit def */
#define FPGA_RST_USB_CONS                  0x200000
#define FPGA_RST_GE                        0x400
#define FPGA_RST_PCIE                      0x40
#define FPGA_RST_USB1_DIS                  0x20
#define FPGA_RST_USB0_DIS                  0x10
#define FPGA_RST                           0x8
#define FPGA_RST_ACT2                      0x4
#define FPGA_RST_FLASH                     0x2

#define FPGA_HOT_INTR_MSK                 0x2000 
#define FPGA_TRIP_INTR_MSK                0x800 
#define FPGA_CATAS_INTR_MSK               0x200

#define FPGA_IRQ0_INTR_TEST               0x40000 

#define FPGA_RESET_MSK                     1
#define FPGA_SPI_DBG_SEL                   0x10000000 /*RO */
#define FPGA_STORED_SPI_SEL                0x20000000 /*RO */
#define FPGA_BOOT_SPI_SEL                  0x40000000
#define FPGA_BOOT_SPI_SEL_OVRIDE           0x80000000 

typedef struct rst_cpld_t_ {
    volatile unsigned int rsv  /*0x00*/;
#define FPGA_RST_REASON   0x4
    volatile unsigned int rst_reason /*0x04*/;
    
#define FPGA_MAGIC_COOKIE 0x8
    volatile unsigned int magic_cookie  /*0x08*/;

#define FPGA_STS          0xC
#define FPGA_BOOT_DEV1_MASK             0x20
#define FPGA_SEL_DEV1_MASK              0x10
    volatile unsigned int sts /*0x0c*/;
    
#define FPGA_RST_CTRL     0x10
    volatile unsigned int rst_ctrl /*0x10*/;
#define FPGA_RST_LEDL     0x14
    volatile unsigned int led /*0x14*/;
#define FPGA_RST_DEBUG    0x18
    volatile unsigned int debug /*0x18*/;
#define FPGA_DEV_RST_CTRL 0x1C
    volatile unsigned int dev_rst_ctrl /*0x1C*/;
#define FPGA_LED          0x20
    volatile unsigned int flash_led /*0x20*/;
#define FPGA_IRQ0_STS     0x24
    volatile unsigned int intr_irq0_sts /*0x24*/;
#define FPGA_IRQ0_MSK     0x28
    volatile unsigned int intr_irq0_msk /*0x28*/;
#define FPGA_IRQ5_STS     0x2C
    volatile unsigned int intr_irq5_sts /*0x2c*/;
#define FPGA_IRQ5_MSK     0x30
    volatile unsigned int intr_irq5_msk /*0x30*/;
    volatile unsigned char pd1[0x10];     /*0x44-0x30-4*/
#define FPGA_MGT_CTRL     0x44
    volatile unsigned int mgt_ctrl /*0x44*/;
#define FPGA_ALM          0x48
    volatile unsigned int alarm /*0x48*/;
    volatile unsigned char pd2[0x08];  /*0x54-(0x48+4)*/
#define FPGA_TST          0x54
    volatile unsigned int tst /*0x54*/;
#define FPGA_CTRL0        0x58
    volatile unsigned int ctrl0 /*0x58*/;
#define FPGA_CTRL1        0x5C
    volatile unsigned int ctrl1 /*0x5c*/;
#define FPGA_PWR          0x60
    volatile unsigned int pwr /*0x60*/;
    volatile unsigned char pd3[0x1c];  /*0x80-0x60-0x4 = 0x1C */
#define FPGA_BD_TYPE      0x80
    volatile unsigned int brd /*0x80*/;
#define FPGA_VERTYPE      0x84
    volatile unsigned int ver /*0x84*/;
    volatile unsigned int main_fpga_ver /*0x88*/;
    volatile unsigned char pd4[0x24];
    volatile unsigned int config_hdr_ctrl /* 0xB0 */;
    volatile unsigned int config_hdr_debug /* 0xB4 */;
    volatile unsigned int config_hdr_ptr /* 0xB8 */;

} rst_cpld_t;

/* __________________________________________________________________
 !!!!!!! system level status reg offset 0 */
extern void reset_platform_ext_dev(int);
extern void unreset_plat_dev(unsigned int mask);
extern void reset_plat_dev(unsigned int mask);
extern void unreset_platform_ext_dev(int);
extern void reset_platform_in_dev(int, int);
extern int reset_nios (int bit, int print);
extern void unreset_platform_in_dev(int);
extern int dash_reset_ext(int val);
extern int dash_reset_int(int val);
extern void set_platform_ext_pin_ctr(int);
extern int get_platform_hw_brd_rev();
extern void enable_platform_ext_pin_ctrl(int);
extern void disable_platform_ext_pin_ctrl(int);

/* External Device Reset Register (+0x04) */
#define FPGA_SATA_SER_MUL_SEL               2
#define FPGA_CK_MOD_OUT_EN                  1
#define FPGA_EXT_DEVICE_RST_REG        0x4
#define FPGA_EXT_10GE_DUAL_RST         0x800
#define FPGA_EXT_PSU_I2C_MUX_RST       0x400 
#define FPGA_EXT_FP_PCIE_RST           0x200 
#define FPGA_EXT_FP_RST                0x100 
#define FPGA_EXT_POE_RST               0x80 
#define FPGA_EXT_BAR_RST               0x40 
#define FPGA_EXT_CLK_RST               0x20 
#define FPGA_EXT_I2C_MUX_RST           0x10 
#define FPGA_EXT_PCIE_SWITCH_HLT       0x8 
#define FPGA_EXT_PCIE_SWITCH_RST       0x4 
#define FPGA_EXT_GE_RST                0x2
#define FPGA_EXT_GE_QUAD_RST           0x1
#define FPGA_DEV_RST_88E1543_RST       (1 << 14)

/* FPGA and board revision
 */
#define FPGA_BD_HW_REV_MSK                  0x07000000
#define FPGA_MAJOR_REV_MSK                  0x007F0000
#define FPGA_MINOR_REV_MSK                  0x0000FF00
#define FPGA_BD_HW_REV_SHFT                 24
#define FPGA_MAJOR_REV_SHFT                 16
#define FPGA_MINOR_REV_SHFT                 8
#define OVLD_PILOT_REV                      4 /* Value 4 is for rev 5 */

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

/* Master FPGA Borad Type Register (+0x80) */
/* Note: refer to the PCIe register address in the spec.
   Note LPC address */
#define FPGA_BD_TYPE_MSK               0x000000F0
#define FPGA_BD_SUBTYPE_HI_MSK         0xFF000000
#define FPGA_BD_SUBTYPE_LO_MSK         0x0000000F
#define FPGA_BD_TYPE_SHFT              4
#define FPGA_BD_TYPE_ROUTE_PROC        0x00000001

#define FPGA_BD_SUBTYPE_HI_SHFT        20 /* shift to bit[11:4] */
#define FPGA_BD_SUBTYPE_MSK            0x00000FFF
#define FPGA_BD_SUBTYPE_OVLD           0x00000008
#define FPGA_BD_SUBTYPE_JUNO           0x00000015
#define FPGA_BD_SUBTYPE_UTAH           0x00000016
#define FPGA_BD_SUBTYPE_SWORD          0x00000017
#define FPGA_BD_SUBTYPE_DAGGER         0x00000018
#define FPGA_BD_SUBTYPE_GOLDBEACH      0x00000019
#define FPGA_BD_SUBTYPE_NEPTUNE        0x00000031
#define FPGA_BD_SUBTYPE_TRITON         0x00000032
#define FPGA_BD_SUBTYPE_NESO           0x00000033
#define FPGA_BD_SUBTYPE_PROTEUS        0x00000034
#define FPGA_BD_SUBTYPE_VG450          0x00000043
#define FPGA_BD_SUBTYPE_NEPTUNIUM      0x0000004C
#define FPGA_BD_SUBTYPE_URANIUM        0x0000004D
#define FPGA_BD_SUBTYPE_THORIUM        0x0000004E
#define FPGA_BD_SUBTYPE_RADIUM         0x0000004F
#define FPGA_BD_SUBTYPE_POLONIUM       0x00000050
#define FPGA_BD_SUBTYPE_THALLIUM       0x00000051
/* Neptune temporary values used before SW finalized the above values */
#define FPGA_BD_SUBTYPE_NEPTUNE_TMP        0x000000f0
#define FPGA_BD_SUBTYPE_NEPTUNE_TMP_1      0x00000030
#define FPGA_BD_SUBTYPE_NANOOK         0x00000056
#define FPGA_BD_SUBTYPE_NANOOK_PLUS_4G 0x00000057
#define FPGA_BD_SUBTYPE_NANOOK_PLUS_8G 0x00000058


/* Master FPGA Revision Register (+0x84) */
/* Note: refer to the PCIe register address in the spec.
   Note LPC address */
#define FPGA_REV_TYPE                 0x80000000
#define DASH_FPGA_HW_BRD_REV          0x07000000
#define DASH_FPGA_HW_BRD_OFF          24
#define DASH_FPGA_REV_MSK             0x007FFFFF
#define DASH_FPGA_REV_MAJOR           0x007F0000
#define DASH_FPGA_REV_MAJOR_OFF       16
#define DASH_FPGA_REV_MINOR           0x0000FF00
#define DASH_FPGA_REV_MINOR_OFF       8
#define DASH_FPGA_REV_DEBUG           0x000000FF

/* PSU status Register (+0x10) */
#define PSU1_PRESENT_MSK              0x1
#define PSU1_AC_IN_OK                 0x2
#define PSU1_12V_OUT_OK               0x4
#define PSU1_INTERRUPT_STAT           0x8
#define PSU2_PRESENT_MSK              0x10
#define PSU2_AC_IN_OK                 0x20
#define PSU2_12V_OUT_OK               0x40
#define PSU2_INTERRUPT_STAT           0x80

/* PoE PSU status Register (+0x20) */
#define POE_PSU1_PRESENT_MSK          0x01
#define POE_PSU1_OUT_OK_MSK           0x04
#define POE_PSU2_PRESENT_MSK          0x10
#define POE_PSU2_OUT_OK_MSK           0x40

#define DASH_FPGA_MISCELLANEOUS_INT_STS_REG    0x350
#define MISC_INT_AC3_INT_STS    (1 << 16)
#define MISC_INT_CROCUS16_INT_STS    (1 << 15)
#define MISC_INT_CROCUS32_INT_STS    (1 << 14)
#define MISC_INT_88E1543_INT_STS    (1 << 13)
#define MISC_INT_88E1680_PHY2_INT_STS    (1 << 12)
#define MISC_INT_88E1680_PHY1_INT_STS    (1 << 11)
#define MISC_INT_88E1680_PHY0_INT_STS    (1 << 10)

#define MISC_INT_AC3_INT_BIT    (16)
#define MISC_INT_88E1543_INT_BIT    (13)
#define MISC_INT_88E1680_PHY2_INT_BIT    (12)
#define MISC_INT_88E1680_PHY1_INT_BIT    (11)
#define MISC_INT_88E1680_PHY0_INT_BIT    (10)

#define MISC_INT_AC3_INT_PENDING    (0)
#define MISC_INT_88E1543_INT_PENDING    (0)
#define MISC_INT_88E1680_INT_PENDING    (0)

#define FPGA_EXTER_DEV_RST_REG       0x00004
#define FPGA_DEV_RST_AC3_RST    (1 << 18)
#define FPGA_DEV_RST_88E1680_2_RST    (1 << 17)
#define FPGA_DEV_RST_88E1680_1_RST    (1 << 16)
#define FPGA_DEV_RST_88E1680_0_RST    (1 << 15)
#define FPGA_DEV_RST_88E1543_RST    (1 << 14)

/* LPC Register 0xFED4000*/

/* LPC Version Register */
#define FPGA_LPC_VERSION_REG    0x84

/* LPC Status LED Control Register */
#define FPGA_LPC_LED_CTRL_REG    0x14
#define SYS_LED_OFF    0x0
#define SYS_LED_AMBER_BLINK    0x1
#define SYS_LED_AMBER    0x2
#define SYS_LED_GREEN    0x3


typedef struct sys_lvl_t_ {
    volatile unsigned int pad1;
    volatile unsigned int ext_rst  /*0x04*/;                   
    volatile unsigned int in_rst /*0x08*/;
    volatile unsigned int pad2; /*0xC; */
#define EXT_PIN_CTRL_MAGIC_VAL  (0xCA << 8)
#define EXT_PIN_CTRL_NEXT_SEL  (1 << 4)
    volatile unsigned int ext_pin_ctrl  /*0x10*/;
    volatile unsigned char pad3[0x6C];  /* 0x80 - 0x10 - 4 */
    volatile unsigned int brd /*0x80*/;
    volatile unsigned int ver /*0x84*/;
    volatile unsigned int slave_ver /*0x88*/;
    volatile unsigned int rev /*0x8C*/;
    volatile unsigned int nios_ver /*0x90*/;
    volatile unsigned char pad4[0x14] /*0xA8 - 0x90 - 4*/;
    volatile unsigned int jtg_sts /*0xA8*/;
    volatile unsigned char pad5[0x14] /*0xC0 - 0xA8 - 4*/;
    volatile unsigned int vtg_mrg_ctrl /*0xC0*/;
} sys_lvl_t;

#define FPGA_VTG_MRG_CTRL          0xC0
#define FPGA_VTG_MRG_MAGIC         0xC5C00000

#define SECURE_JTAG_MASK           0x000000FF
#define SECURE_JTAG_WORK           0xC4
#define LPC_POWER_CYCLE_KEY1       0x4FED0000
#define LPC_POWER_CYCLE_KEY2       0xA2BA0000
#define HEADLESS_MODE              0x40000

/* ________________________________________________________*/
/* !!!!!!! interupt status and control register offset 0x100
   FPGA_INTR_CTRL_REG_OFFSET                      0x100
*/
extern int get_platform_fpga_rev();
extern void get_platform_bd_rev(unsigned int*);
extern int get_platform_intr_sts(int);
extern void enable_platform_fp_intr(int);
extern void disable_platform_fp_intr(int);
extern void enable_platform_nios_intr(int);
extern void disable_platform_nios_intr(int);

extern int get_platform_sfp_intr_sts();
extern void enable_platform_sfp_intr(int);
extern void disable_platform_sfp_intr(int);
extern void sfp_intr_hndlr(int, void*);
extern void enable_platform_sfp_override_intr(int);
extern void clean_platform_sfp_override_intr(int);
extern int check_sfp_int_sts(int);

extern int get_platform_i2c_sts();
extern int get_platform_uart_sts(int);
extern void enable_platform_c2w_intr(int);
extern void enable_platform_c2w_override_intr(int);
extern void clear_platform_c2w_intr(int);
extern void disable_platform_c2w_intr(int);

extern void enable_platform_uart_intr(int);
extern void enable_platform_uart_override_intr(int);
extern void disable_platform_uart_intr(int);
extern void clear_platform_uart_intr(int);
extern void disable_platform_uart_intr(int);
extern void disable_platform_uart_ovr_intr(int);
extern void display_uart_regs(int mode);

extern void enable_platform_mcu_intr(int);
extern void disable_platform_mcu_intr(int);
extern void clear_platform_mcu_intr(int);
extern void enable_platform_mcu_override_intr(int);

extern void enable_platform_vm_mcu_intr(int dev);
extern void disable_platform_vm_mcu_intr(int dev);
extern void clear_platform_vm_mcu_intr(int dev);
extern void enable_platform_vm_mcu_override_intr(int dev);

extern void enable_platform_sm_oir_intr(int dev);
extern void enable_platform_sm_oir_override_intr(int dev);
extern void disable_platform_sm_oir_intr(int dev);
extern void enable_platform_wic_oir_intr(int dev);
extern void enable_platform_wic_oir_override_intr(int dev);
extern void disable_platform_wic_oir_intr(int dev);
extern void enable_platform_pim_oir_intr(int);
extern void disable_platform_pim_oir_intr(int);
extern void enable_platform_pim_oir_override_intr(int dev);
extern void enable_platform_sata_oir_intr(int);
extern void disable_platform_sata_oir_intr(int);
extern void enable_platform_sata_oir_override_intr(int dev);

extern void enable_platform_uart_console_intr(int dev);
extern void disable_platform_uart_console_intr(int dev);
extern void clear_platform_uart_console_intr(int dev);
extern void enable_platform_uart_console_override_intr(int dev);
extern int get_poe_psu_intr();
extern void enable_poe_psu_intr(int);
extern void disable_poe_psu_intr(int);
extern void poe_dc_intr_hndlr(int, void*);
extern void poe2_output_intr_hndlr(int, void*);
extern void poe2_present_intr_hndlr(int, void*);
extern void poe1_output_intr_hndlr(int, void*);
extern void poe1_present_intr_hndlr(int, void*);

extern void enable_platform_env_intr();
extern void disable_platform_env_intr();

extern int get_platform_poe_psu_intr_stat(void);
extern void enable_platform_poe_psu_intr(int);
extern void disable_platform_poe_psu_intr(int);

extern int get_platform_env_intr_stat();
extern int get_secure_jtag_status();
/* UART */
#define AUX_UART_UIO_BASE 0x2f8
/* DLAB=0 */
#define UART_TX     0 /* out: transmit buffer */
#define UART_RX     0 /* in: receive buffer */
#define UART_IER    1 /* out: interrupt enable register */
#define UART_IIR    2 /* in: interrupt id register */
#define UART_FCR    2 /* out: fifo control register */
#define UART_LCR    3 /* out: line control register */
#define UART_MCR    4 /* out: modem control register */
#define UART_LSR    5 /* out: line status register */
/* DLAB=1 */
#define UART_DLL    0 /* out: divisor latch low */
#define UART_DLM    1 /* out: divisor latch high */

#define FPGA_CP_INTR_CTRL_REG_OFFSET                      0x100
#define FPGA_FP_INTR_CTRL_REG_OFFSET                      0x200
#define FPGA_NIOS_INTR_CTRL_REG_OFFSET                    0x300
#define FPGA_SYNC_ETH_PLL                              0x2000
#define FPGA_GE_SYNC_INTR                              0x1000
#define FPGA_CPU_CP_FP                                 0x800
#define FPGA_CPU_FP_CP                                 0x400
#define FPGA_CPU_CP_NIOS                               0x200
#define FPGA_CPU_NIOS_CP                               0x100
#define FPGA_PWR_SUPPLY_INTR                           0x40
#define FPGA_ENV_INTR                                  0x20
#define FPGA_MISC_INTR                                 0x10
#define FPGA_OIR_INTR                                  0x8
#define FPGA_UART_INTR                                 0x4
#define FPGA_I2C_INTR                                  0x2
#define FPGA_SFP_INTR                                  0x1

#define FPGA_MISC_NIOS_SPI                              0x20
#define FPGA_MISC_FPGA_SPI                              0x10
#define FPGA_MISC_QUAD_PHY                              0x8
#define FPGA_MISC_UART_CONSOLE                          0x4
#define FPGA_MISC_VM_MCU                                0x2
#define FPGA_MISC_ENV_MCU                               0x1
typedef struct fpga_intr_t_ {
    volatile unsigned int top_sts;                      /* 0x0 */
    volatile unsigned int top_en;                       /* 0x4 */
    volatile unsigned int pad1;                     /* 0x8 */
    volatile unsigned int pad2;                     /* 0xC */
    volatile unsigned int sfp_sts;                  /* 0x10 */
    volatile unsigned int sfp_en;                   /* 0x14 */
    volatile unsigned int pad3[2];                  /* 0x18, 0x1C*/
    volatile unsigned int c2w_sts;                  /* 0x20 */
    volatile unsigned int c2w_en;                   /* 0x24 */
    volatile unsigned int c2w_ovr;             /* 0x28 */
    volatile unsigned int pad3a;                    /* 0x2C */
    volatile unsigned int uart_sts;                 /* 0x30 */
    volatile unsigned int uart_en;                  /* 0x34 */
    volatile unsigned int uart_ovr;                 /* 0x38 */
    volatile unsigned int pad3b;                    /* 0x3C */

#define FPGA_OIR_PIM       0x10000
#define FPGA_OIR_SATA      0x1000
#define FPGA_OIR_NGWIC3    0x40
#define FPGA_OIR_NGWIC2    0x20
#define FPGA_OIR_NGWIC1    0x10
#define FPGA_OIR_NGSM4     8
#define FPGA_OIR_NGSM3     4
#define FPGA_OIR_NGSM2     2
#define FPGA_OIR_NGSM1     1
    volatile unsigned int oir_sts;                  /* 0x40 */
    volatile unsigned int oir_en;                   /* 0x44 */
    volatile unsigned int oir_ovr;                  /* 0x48 */
    volatile unsigned int pad4;                     /* 0x4C */

#define FPGA_MISC_SPI                                0x10
    //predefined #define FPGA_MISC_UART                               0x04
    //predefined #define FPGA_MISC_VM_MCU                             0x02
    //predefined #define FPGA_MISC_ENV_MCU                            0x01
    volatile unsigned int misc_sts;                 /* 0x50 */
    volatile unsigned int misc_intr;                /* 0x54 */
    volatile unsigned int misc_ovr;                 /* 0x58 */
} fpga_intr_t;

/* ___________________________________________________________*/

typedef struct sfp_t_ {
    volatile unsigned int intr_sts0;
    volatile unsigned int conf0;
    volatile unsigned int intr_sts1;
    volatile unsigned int conf1;
    volatile unsigned int intr_sts2;
    volatile unsigned int conf2;
    volatile unsigned int intr_sts3;
    volatile unsigned int conf3;
} sfp_t;

#define SYS_LOW_LEVEL_OFFSET 0x0
extern unsigned long get_sys_low_level_base();

/* LED control register */
#define LED_CONTROL_OFFSET         0x400
extern unsigned long get_platform_led_ctrl_base();
#define LED_CTRL_MISC                 0x0
#define LED_CTRL_CF                   0x1
#define LED_CTRL_PWR                  0x2
#define LED_CTRL_POE_PWR              0x3
#define LED_CTRL_POE_DAUGH            0x4
#define LED_CTRL_HD_DRIVER            0x5
#define LED_CTRL_BLINK_DURA           0x6
#define LED_CTRL_RJ45_BLINK_EN        0x7
#define LED_CTRL_ETH_BLINK_EN         0x8
#define LED_CTRL_RJ45_ONOFF           0x9
#define LED_CTRL_SFP_ONOFF            0xA
#define LED_CTRL_MGMT_ONOFF           0xB
#define LED_CTRL_DEBUG                0xC
#define LED_CTRL_ENV                  0xD
typedef struct led_t_ {
    volatile unsigned int misc;
    volatile unsigned int cf;
    volatile unsigned int pwr;
    volatile unsigned int poe_pwr;
    volatile unsigned int poe_daugh;
    volatile unsigned int hd_driver;
    volatile unsigned int env;
    volatile unsigned int pad1;        /* 0x1C */
    volatile unsigned int blink_duration;
    volatile unsigned int rj45_blink_en;
    volatile unsigned int eth_blink_en;
    volatile unsigned int rj45_onoff;
    volatile unsigned int sfp_onoff;
    volatile unsigned int mgmt_onoff;
    volatile unsigned int debug;
} led_t;

#define PLUG_FPGA_SYS_SCR_REG           (0x00)
#define PLUG_FPGA_SYS_BTYPE_REG         (0x80)
#define PLUG_FPGA_SYS_REV_REG           (0x84)
#define PLUG_FPGA_SYS_SEC_REV_REG       (0x8C)

/* PLUG Interrupt Rewg Offset */
#define PLUG_INTR_REG_OFFSET            (FPGA_CP_INTR_CTRL_REG_OFFSET)
#define PLUG_INTR_STAT_REG              (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x00)
#define PLUG_INTR_ENA_REG               (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x04)
#define PLUG_I2C_INTR_STAT_REG          (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x20)
#define PLUG_I2C_INTR_ENA_REG           (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x24)
#define PLUG_I2C_INTR_OVRI_REG          (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x28)
#define PLUG_UART_INTR_STAT_REG         (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x30)
#define PLUG_UART_INTR_ENA_REG          (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x34)
#define PLUG_UART_INTR_OVRI_REG         (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x38)
#define PLUG_OIR_INTR_STAT_REG          (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x40)
#define PLUG_OIR_INTR_ENA_REG           (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x44)
#define PLUG_OIR_INTR_OVRI_REG          (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x48)
#define PLUG_MISC_INTR_STAT_REG         (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x50)
#define PLUG_MISC_INTR_ENA_REG          (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x54)
#define PLUG_MISC_INTR_OVRI_REG         (FPGA_CP_INTR_CTRL_REG_OFFSET + 0x58)

#define PLUG_INTR_STAT_I2C_BIT          (2)
#define PLUG_INTR_ENA_I2C_BIT           (2)
#define PLUG_I2C0_INTR_STAT_BIT         (1)
#define PLUG_I2C0_INTR_ENA_BIT          (1)
#define PLUG_I2C0_INTR_OVRI_BIT         (1)
#define PLUG_I2C_INTR_OVRI_CLEAN        (0)

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

/* PLUG Module Register */
#define PLUG_MODULE_OFFSET              (0x32800)
#define FPGA_MODULE_DEB_CTL_REG         (PLUG_MODULE_OFFSET + 0x00)
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
 * FPGA Multiboot Register
 */
#define FPGA_MULTIBOOT_OFFSET           (0x22000)
#define FPGA_RECONF_CTRL_REG            (0x00)
#define FPGA_RECONF_STS_REG             (0x04)
#define FPGA_CACHED_REVID_REG           (0x08)
#define FPGA_CACHED_REVDATE_REG         (0x0C)
#define FPGA_CACHED_FLAGS_REG           (0x10)
#define FPGA_CACHED_MAGIC_REG           (0x14)
#define FPGA_MULTIBOOT_STATE_REG        (0x18)
#define FPGA_MULTIBOOT_SEC_RSLT_REG     (0x1C)

/* FPGA/NIOS SPI PROM Programming */
#define PLUG_FPGA_SPI_PROM_PROG_OFFSET       (0x31800)
#define PLUG_FPGA_SPI_PROM_CTRL_REG          (0x00)
#define PLUG_FPGA_SPI_PROM_STS_REG           (0x04)
#define PLUG_FPGA_SPI_PROM_RD_SIZE_REG       (0x08)
#define PLUG_FPGA_SPI_PROM_RW_DATA_REG       (0x0C)
#define PLUG_FPGA_SPI_PROM_OP_ADDR_REG       (0x10)

/* FPGA_M2_CTL register */
#define FPGA_M2_CTLSTS_REG              (0x320A0)
#define FPGA_M2_INT_EN_REG              (0x320A4)
#define FPGA_M2_DEB_REG                 (0x320A8)
#define FPGA_M2_MODULE_PRESENT          (0x10)
#define FPGA_M2_REMOVAL_INTR            (0x100)
#define FPGA_M2_INSTERT_INTR            (0x200)
#define FPGA_M2_DEVICE_PRESENT_MASK     (0x30000)
#define FPGA_M2_SATA_PRESENT            (0x00000)
#define FPGA_M2_USB_PRESENT             (0x20000)
#define FGPA_M2_NO_DEVICE               (0xA5A5)  /* Magic number for no M.2 device */

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

#include "uart_fpga.h"

/*--------------------------------------------------------*/

#define UART_MUX_CONTROL_OFFSET           0x900
#define MUX_REG_USB_CONSOLE_GPIO_VAL      0x2000
#define MUX_REG_USB_CONSOLE_CABLE_DET     0x0200
#define MUX_REG_USB_MANUAL_MUX_SEL        0x0004
#define MUX_REG_USB_MUX_SEL               0x0002
#define USB_CONSOLE_SRC              1
#define RJ45_CONSOLE_SRC             0
typedef struct console_t_ {
    volatile unsigned int multiplex;  /* 0 */
    volatile unsigned int intr;       /* 4 */
#define FPGA_USB_CONSOLE_CABLE_INTR_EN  0x200 
#define FPGA_USB_CONSOLE_INTR_EN        0x2000 
    volatile unsigned int intr_en;    /* 8 */
    volatile unsigned int pad;        /* 0xC */
#define MUX_SEL_AUX2FPGA   1       
#define MUX_SEL_AUX2CC     0           /* aux to cave creek */
    volatile unsigned int aux;        /* 0x10 */
} console_t;

/*
typedef struct ng_module_t_ {
    volatile unsigned int debounce;
    volatile unsigned int oir;
    volatile unsigned int sm1_ctrl;
    volatile unsigned int sm1_intr_en;
    volatile unsigned int sm2_ctrl;
    volatile unsigned int sm2_intr_en;
    volatile unsigned int ngwic1_ctrl;
    volatile unsigned int ngwic1_intr_en;
    volatile unsigned int ngwic2_ctrl;
    volatile unsigned int ngwic2_intr_en;
    volatile unsigned int ngwic3_ctrl;
    volatile unsigned int ngwic3_intr_en;
    volatile unsigned int ngvm_ctrl;
} ng_module_t;
*/
typedef struct ge_t_ {
    volatile unsigned int sts;
    volatile unsigned int intr_en;
    volatile unsigned int ngsm1_ctrl;
    volatile unsigned int ngsm2_ctrl;
    volatile unsigned int ngwic1_ctrl;
    volatile unsigned int ngwic2_ctrl;
    volatile unsigned int ngwic3_ctrl;
    volatile unsigned int phy_ctrl;
    volatile unsigned int ge_ctrl;
    volatile unsigned int cavium_ctrl;
    volatile unsigned int ngvm_ctrl;
} ge_t;

#define MBX_SIZE            0x400
#define MBX_OFFSET          0x14000
typedef struct mailbox_t_ {
    volatile unsigned int h2n_ctrl;
    volatile unsigned int h2n_addr;
    volatile unsigned int n2h_ctrl;
    volatile unsigned int n2h_addr;
    volatile unsigned int host_intr;
    volatile unsigned int nios_intr;
} mailbox_t;


#define SFP_STATUS_CONTROL_OFFSET   0x10000
#define SFP0_CONF_OFFSET            0x10004
#define SFP1_CONF_OFFSET            0x1000C
/*for interrupt reg related */
#define SFP_TX_FAULT_INTR           0x4
#define SFP_LOSS_SIG_INTR           0x2
#define SFP_PRESENT_INTR            0x1
/*for configuration reg related */
#define SFP_TX_FAULT                0x40000  /* bit 18 */
#define SFP_LOSS_SIG                0x20000  /* bit 17 */
#define SFP_PRESENT                 0x10000  /* bit 16 */
#define SFP_TX_FAULT_BIT            (1 << 18)
#define SFP_LOSS_SIG_BIT            (1 << 17)
#define SFP_PRESENT_BIT             (1 << 16)
#define SFP_TX_DISABLE              0x100
#define SFP_TX_FAULT_INTR_OVERRIDE  0x20
#define SFP_LOSS_SIG_INTR_OVERRIDE  0x10
#define SFP_PRESENT_INTR_OVERRIDE   0x8
#define SFP_TX_FAULT_INTR_EN        0x4
#define SFP_LOSS_SIG_INTR_EN        0x2
#define SFP_PRESENT_INTR_EN         0x1
typedef struct sfp_stat_ctrl_t_ {
    volatile unsigned int sfp0_intr;  /* 0x0 */
    volatile unsigned int sfp0_conf;  /* 0x4 */
    volatile unsigned int sfp1_intr;  /* 0x8 */
    volatile unsigned int sfp1_conf;  /* 0xc */
    volatile unsigned int sfp2_intr;  /* 0x10 */
    volatile unsigned int sfp2_conf;  /* 0x14 */
    volatile unsigned int sfp3_intr;  /* 0x18 */
    volatile unsigned int sfp3_conf;  /* 0x1c */
    volatile unsigned int debnce;     /* 0x20: SFP Debounce Reg */
    volatile unsigned int pad0;       /* 0x24 */
    volatile unsigned int pad1;       /* 0x28 */
    volatile unsigned int pad2;       /* 0x2c */
    volatile unsigned int pad3[4];    /* 0x30, 0x34, 0x38, 0x3c */
    volatile unsigned int sfp_p0_intr;  /* 0x40 */
    volatile unsigned int sfp_p0_conf;  /* 0x44 */
    volatile unsigned int sfp_p1_intr;  /* 0x48 */
    volatile unsigned int sfp_p1_conf;  /* 0x4c */
} sfp_stat_ctrl_t;

#define FPGA_HEADER_OFFSET    0x22000
typedef struct hdr_t_ {
    volatile unsigned int reconf_ctrl;  /* 0x0 */
    volatile unsigned int reconf_sts;  /* 0x4 */
    volatile unsigned int upgrade_rev;  /* 0x8 */
    volatile unsigned int upgrade_data;  /* 0xc */
    volatile unsigned int upgrade_flag;  /* 0x10 */
    volatile unsigned int upgrade_magic;  /* 0x14 */
    volatile unsigned int state_hist;  /* 0x18 */
    volatile unsigned int result_hist;  /* 0x1c */
    volatile unsigned int code_sign_boot_sts;     /* 0x20:  */
    volatile unsigned int secure_boot_sts;     /* 0x24: */
    volatile unsigned int secure_boot_sys;     /* 0x28: */
    volatile unsigned int secure_boot_core;     /* 0x2C: */
    volatile unsigned int secure_boot_sig;     /* 0x30: */
    volatile unsigned int secure_boot_sig_sz;     /* 0x34: */
} hdr_t;
extern unsigned long get_platform_multiboot_base(void);
extern int display_multiboot(int);


#define FPGA_PS_ENV_OFFSET     0x32100
extern unsigned long get_platform_ps_env_base();
#define EXT_ENV_INTR_EN     0x1
/* PSU and Environmental Registers */
typedef struct psu_t_ {
    volatile uint32_t debounce;          /* 0x00: PSU and POE PSU Debounce Reg */
    volatile uint32_t reserved1;         /* 0x04 */
    volatile uint32_t env_int_stat;      /* 0x08: Ext. ENV Interrupt Reg */
    volatile uint32_t env_int_en;        /* 0x0C: Ext. ENV Interrupt Enable Reg */
    volatile uint32_t psu_stat;          /* 0x10: PSU Status Reg */
    volatile uint32_t psu_int_stat;      /* 0x14: PSU Interrupt Reg*/
    volatile uint32_t psu_int_en;        /* 0x18: PSU Interrupt Enable Reg */
    volatile uint32_t reserved2;         /* 0x1C */
    volatile uint32_t poe_psu_stat;      /* 0x20: POE PSU Status Reg */
#define POE_DC              0x200
#define POE_PSU2_OUTPUT_OK  0x40
#define POE_PSU2_PRESENT    0x10
#define POE_PSU1_OUTPUT_OK  0x4
#define POE_PSU1_PRESENT    0x1

    volatile uint32_t poe_psu_int_stat;  /* 0x24: POE PSU Interrupt Reg */
    volatile uint32_t poe_psu_int_en;    /* 0x28: POE PSU Interrupt Enable Reg */
} psu_t;


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
/* Define for GOLDBEACH */
#define GB_FAN4_OPTION     0x800
#define GB_FAN3_OPTION     0x400
#define GB_FAN2_OPTION     0x200
#define GB_FAN1_OPTION     0x100
/* Define for O2/USD */
#define FAN4_OPTION     0x80
#define FAN3_OPTION     0x40
#define FAN2_OPTION     0x20
#define FAN1_OPTION     0x10
/* Define for Curie/Neptune */
#define FAN4_ENABLE     0x800
#define FAN3_ENABLE     0x400
#define FAN2_ENABLE     0x200
#define FAN1_ENABLE     0x100

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

/* NIOS Mailbox Fan Register */
#define NIOS_MAILBOX_FAN_1_OFFSET    0x34260
#define NIOS_MAILBOX_FAN_2_OFFSET    0x34262
#define NIOS_MAILBOX_FAN_3_OFFSET    0x34264

/* NIOS Mailbox Fan Limit */
#define NIOS_MAILBOX_FAN_MIN        0x2B2F
#define NIOS_MAILBOX_FAN_MAX        0x6B6C

#define FPGA_ENV_MCU_OFFSET     0x33000
extern unsigned long get_platform_env_mcu_base(int);
typedef struct env_dwnld_t_ {
    volatile unsigned int ctrl;
    volatile unsigned int sts;
    volatile unsigned int intr_en;
    volatile unsigned int data;
} env_dwnld_t ;

#define FPGA_VOL_MON_OFFSET     0x33100
extern unsigned long get_platform_vm_base(int);
typedef struct vol_dwnld_t_ {
    volatile unsigned int ctrl;
    volatile unsigned int sts;
    volatile unsigned int intr_en;
    volatile unsigned int data;
} vol_dwnld_t ;

unsigned char cpu_mblx[0x400];

//#include "platform_prom.h"

#define FPGA_CP_SPI_PROM_OFFSET   0x31800
#define FPGA_NIOS_SPI_PROM_OFFSET   0x31900
#define FPGA_AIKIDO_SPI_MASTER_OFFSET   0x31A00
/* aikido using the same structure as platform_prom.h */

typedef struct pci_intr_t_ {


} pci_intr_t;

typedef struct watchdog_t_ {


} watchdog_t;


#define FPGA_BASE_OFFSET                   0x0
#define FPGA_I2C_BASE                      0x30000
#define FPGA_I2C_OFFSET                    0x100
typedef struct ovld_i2c_ctrl_t_ {
    volatile unsigned int ctrl;
    volatile unsigned int pad0;
    volatile unsigned int stat;
    volatile unsigned int stat_mask;
    volatile unsigned int sla_addr;
    volatile unsigned int sla_sub_addr;
    volatile unsigned int bit_bang;      /* 0x18 */
    volatile unsigned int byte_count;
    volatile unsigned int pad1[8];       /* skipped 0x20 to 0x3C */
    volatile unsigned int data_fifo;     /* 0x40 */
    volatile unsigned int data_fifo_rw_ptr;  /* 0x44 */
} ovld_i2c_ctrl_t ;
extern int clean_env_alert(int);

/* mailobox__________________________________________________________*/

#define FPGA_MLBX_CP_FP_OFFSET                 0x11000
#define FPGA_MLBX_CP_FP_MSG_OFFSET             0x14000
#define MCU_MSG_OFFSET_TBD                     0x100 /*TBD */

extern void platform_fixup();
extern unsigned char *get_platform_fpga_fw();
extern unsigned int get_platform_fpga_size();
extern unsigned int get_platform_fpga_date();
extern int get_platform_booted_spi_prom();
extern int get_platform_selected_spi_prom();
extern unsigned long get_platform_aikido_addr();
extern void change_platform_prom_wr_protect(int);
extern void aux_multiplex(int);
extern unsigned long get_platform_prom_addr();
extern unsigned long get_platform_nios_prom_addr();
extern unsigned long get_platform_mbx_addr();
extern unsigned long get_platform_uart_addr(int);
extern unsigned long get_platform_intr_ctrl_addr();
extern unsigned long get_platform_i2c_addr(int);
extern unsigned long get_platform_uart_mux_addr();
extern unsigned long get_platform_sfp_stat_ctrl_addr();
extern int read_eeprom_block(unsigned int, unsigned int size, unsigned char *);
//extern int write_eeprom_block(unsigned int offset, unsigned int,
//                              unsigned char *buf);
//extern int read_eeprom_block(unsigned int, unsigned int, char *);
extern int write_eeprom_block(unsigned int, char *);
extern int fpga_i2c_scan_test(int option);
extern void *get_n2g_i2c_if(uint8_t i2c, uint8_t, uint8_t);
extern int get_i2c_fd(int cpu);
extern int get_fpga_fd();
extern int write_i2c_reg(int);
extern int read_i2c_reg(int);
extern int override_intr_test(int);
extern int platform_intr_test();
extern void set_led_off(int dev, int bit);
extern void set_led_on(int dev, int bit);
extern void uart_lpbk(int lpbk);
extern unsigned long get_fpga_addr();
extern unsigned long get_cpld_addr();
extern void flush_uart_fifo (int);
extern unsigned int get_platform_uart_mux_ctrl_reg();
extern int switch_console_usb(int);
extern void cpld_reset(void);

typedef struct fpga_msg_t_ {
    unsigned int cmd;
    unsigned int resp;
    unsigned int len; /* len of data stored in data */
    unsigned char data[MSG_DATA_SIZE];
} fpga_msg;

extern unsigned long get_platform_net_clk_ptp_conf_base(void);
typedef struct ntclk_ {
    volatile uint32_t status;               /* 0x00*/
    volatile uint32_t pll_intr;             /* 0x04 */
    volatile uint32_t pll_ref;              /* 0x08 */
    volatile uint32_t mult5;                /* 0x0C */

#define SYNC_TRIG_OUTPUT_ENABLE             0x1000000
#define ENABLE_PRE_SCALER_DIV_5             0x400 
#define ENABLE_PRE_SCALER_DIV_3125          0x200 
#define SYNC_OUT_ENABLE                     0x100 
    
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


/* Sync Registers */
#define NET_CLK_PTP_CONF_REG_OFF   0x10100
#define SYNC_DEBUG_REG_OFF         0x101C0

/* Definition of SYNC_OUT/SYNC_TRIG_OUT register */
#define NGVM_SYNC_OUT_SYNC_TRIG_OUT    0x30
#define NGVM_SYNC_OUT1_CTRL            0x34
#define SYNC_TRIG_IN_SYNC_IN_DBG       0xc0


/* PLL */
#define EXT_DEV_RESET_REG_OFF      0x00004
#define EXT_PLL_REF_SEL_REG_OFF    0x10108
#define EXT_SYNC_PLL_CONF_REG_OFF  0x10200
#define EXT_SYNC_PLL_STAT_REG_OFF  0x10204

/* Definition of Ext. Sync Clock PLL Status Register (0x10204) */
#define LOSS_OF_SIG2_STAT          0x00000400
#define LOSS_OF_SIG1_STAT          0x00000200
#define LOSS_OF_LOCK_STAT          0x00000100
#define LOSS_OF_SIG2_INT           0x00000004
#define LOSS_OF_SIG1_INT           0x00000002
#define LOSS_OF_LOCK_INT           0x00000001


/* Externs */
//#ifndef LINUX_KLM
extern unsigned long dash_cpld; /* TPM space */
extern unsigned long dash_fpga;
//#endif

/* platform board type/check related */
extern void set_board_type(void);
extern int mb_board_type(void);
extern void prepare_pcie_sw_info(unsigned int *);
extern boolean is_nanook(void);
extern boolean is_nanook_plus(void);
extern boolean is_overlord(void);
extern boolean is_juno(void);
extern boolean is_juno_plx(void);
extern boolean is_plx(void);
extern int is_utah(void);
extern int is_utah_plx(void);
extern boolean is_not_plx(void);
extern boolean is_plx_wrapper(void);
extern int is_sword(void);
extern int is_dagger(void);
extern int is_goldbeach(void);
extern int is_dg_machines(void);
extern int is_usd_machines(void);
extern int is_us_machines(void);
extern int is_utah_false(void);
extern int is_neptune(void);
extern int is_vg450(void);
extern int is_triton(void);
extern int is_proteus(void);
extern int is_neso(void);
extern int is_ntpn_machines(void);
extern int is_curie_1ru(void);
extern int is_curie_1ru_4ge_port(void);
extern int is_curie_1ru_p1c_and_later(void);
extern int is_radium(void);
extern int is_polonium(void);
extern int is_thallium(void);
extern int exist_sm_slot1(void);
extern int get_plat_sku_fpga(void);
extern int get_plat_sku(void);
extern int chk_plat_sku(int*);
extern int is_uranium(void);
extern int is_thorium(void);
extern int is_curie_2ru(void);

extern int  get_led_status(int);
extern void set_led_reg(int, int);

extern void set_fpga_upgrade_header_read(void);
extern void set_fpga_reconf_fsm(void);
extern void reset_fpga_upgrade_header_read(void);
extern void reset_fpga_reconf_fsm(void);

extern unsigned char *dash_fpga_fw_array;
extern unsigned int dash_fpga_fw_size;
extern int uart_lpbk_txrx(int, char*, int, char*, int *, int, int);
extern int dash_uart_tx (int port, int test_speed, char* tx_str, int test_sz, int);
extern int dash_uart_rx(int port, int *, char* rx_str);
extern void dash_uart_reset(int port);
extern int fpga_vol_margin (int);
extern int dash_fpga_reg_write(uint, uint);
extern int dash_fpga_reg_read(uint, uint *);

#endif  /* #if __DASH_FPGA */
/*------------------------------------------------------------------
 * $Log: dash_fpga.h,v $
 * Revision 1.3  2020/01/09 01:03:00  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.2  2019/12/11 10:10:27  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 */
