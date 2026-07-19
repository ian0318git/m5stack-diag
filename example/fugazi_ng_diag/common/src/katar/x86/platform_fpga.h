/* $Id: platform_fpga.h,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_fpga.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : katar_fpga.h
 * Description: Header file of Katar FPGA Diag.
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __KATAR_PLATFORM_FPGA_H__
#define __KATAR_PLATFORM_FPGA_H__
 
#include "defs.h"
#include "common_utils.h"
#include "types.h"

#define FPGA_SPEC_1_7_VER		(0x18112609)

#define FPGA_BASE_REG_OFFSET	(0x0000)
#define FPGA_PROM_REG_OFFSET	(0x0C00)
#define FPGA_NON_IO_REG_OFFSET  (0x4000)

#define FPGA_RONLY    (READ_ONLY | REG_ACCESS)
#define FPGA_RW       (READ_WRITE | REG_ACCESS)
#define MASK_32 	  0xFFFFFFFF

#define FPGA_IRQ_ALL	0xFF
#define FPGA_IRQ_NONE   0x00

/* Register Definition */
/* Sys Reg Offset */
#define FPGA_LPC_RESET_REASON_REG    0x004
#define FPGA_LPC_SCRATCHPAD_REG      0x008
#define FPGA_LPC_STATUS_REG          0x00C
#define FPGA_LPC_CPRSTCNTRL_REG      0x010
#define FPGA_LPC_STAT_LED_CTRL_REG   0x014
#define FPGA_LPC_DEBUG_CTRL_REG      0x018
#define FPGA_LPC_EXT_DEV_RST_REG     0x01C
#define FPGA_LPC_RP_LED_CTRL_REG     0x020
#define FPGA_LPC_IRQ09_STAT_REG      0x024
#define FPGA_LPC_IRQ09_MASK_REG      0x028
#define FPGA_LPC_IRQ10_STAT_REG      0x02C
#define FPGA_LPC_IRQ10_MASK_REG      0x030
#define FPGA_LPC_IRQ11_STAT_REG      0x034
#define FPGA_LPC_IRQ11_MASK_REG      0x038
#define FPGA_LPC_SCRATCHPAD1_REG     0x03C
#define FPGA_LED_CTRL_REG            0x048
#define FPGA_IRQ_TEST_REG            0x054
#define FPGA_LPC_SPI_CTRL_REG        0x058
#define FPGA_LPC_BOOT_TIMER_REG      0x064
#define FPGA_LPC_BOARDTYPE_REG       0x080
#define FPGA_LPC_VERSION_REG         0x084
#define FPGA_SEC_BOOT_STAT_REG       0x088
#define FPGA_SEC_BOOT_SIGN_REG       0x08C
#define FPGA_SEC_FP_STATUS_2_REG     0x090
#define FPGA_SEC_VERSION_DATE_REG    0x094
#define FPGA_SEC_VERSION_ID_REG      0x098
#define FPGA_CONF_HEADER_CTRL_REG	 0x0B0
#define FPGA_CONF_HEADER_DBG_REG     0x0B4
#define FPGA_LPC_SKU_FEATURE_REG	 0x0C0
#define FPGA_FAN_INTR_STAT_REG       0x100
#define FPGA_FAN_INTR_MASK_REG       0x104
#define FPAG_FAN_STAT_REG         	 0x108
#define FPGA_FAN_CTRL_REG            0x110
#define FPGA_IOS_WATCHDOG_REG		 0x124
#define FPGA_EXT_INTR_PEND_REG		 0x128
#define FPGA_EXT_INTR_MASK_REG       0x12C
#define FPGA_EXT_INTR_FORCE_REG      0x130
#define FPGA_USB_CONSOLE_REG         0x148
#define FPGA_INTR_STAT_REG           0x200
#define FPGA_INTR_MASK_REG           0x204
#define FPGA_INTR_FORCE_REG          0x214
#define FPGA_RST_BUTTON_REG          0x218
#define FPGA_RST_BUTTON_MASK_REG     0x21C
#define FPGA_SEC_CC_STATUS_REG       0x404
#define FPGA_SEC_FP_STATUS_REG       0x604

typedef struct katar_sys_lvl_t_ {
    volatile unsigned int pad1;  							/*0x000*/
    volatile unsigned int rst_reason;			/*0x004*/
    volatile unsigned int scratchpad; 			/*0x008*/
    volatile unsigned int status; 				/*0x00C*/
    volatile unsigned int cprstcntrl;  			/*0x010*/
    volatile unsigned int stat_led;  			/*0x014*/
    volatile unsigned int dbg_ctrl;				/*0x018*/
    volatile unsigned int dev_rst;				/*0x01C*/
    volatile unsigned int rp_led_ctrl;  		/*0x020*/
    volatile unsigned int irq09_stat;			/*0x024*/
    volatile unsigned int irq09_mask;			/*0x028*/
    volatile unsigned int irq10_stat;           /*0x02C*/
    volatile unsigned int irq10_mask;           /*0x030*/
	volatile unsigned int irq11_stat;           /*0x034*/
    volatile unsigned int irq11_mask;           /*0x038*/
    volatile unsigned int scratchpad1;          /*0x03C*/
    volatile unsigned char pad3[0x8];                       /*0x048 - 0x03C - 4 = 0x8*/
    volatile unsigned int led_ctrl;				/*0x048*/
    volatile unsigned char pad4[0x8];						/*0x054 - 0x048 - 4 = 0x8*/
    volatile unsigned int irq_test;				/*0x054*/
    volatile unsigned int spi_ctrl;				/*0x058*/
    volatile unsigned char pad5[0x8];                       /*0x064 - 0x058 - 4 = 0x8*/
    volatile unsigned int boot_timer;           /*0x064*/
    volatile unsigned char pad6[0x18];						/*0x080 - 0x064 - 4 = 0x18*/
    volatile unsigned int brd_type;				/*0x080*/
    volatile unsigned int ver;					/*0x084*/
    volatile unsigned int boot_stat;			/*0x088*/
    volatile unsigned int boot_sign;			/*0x08C*/
    volatile unsigned int fp_status_2;          /*0x090*/
    volatile unsigned int sec_ver_date;         /*0x094*/
    volatile unsigned int sec_ver_id;           /*0x098*/
    volatile unsigned char pad7[0x14];                      /*0x0B0 - 0x098 - 4 = 0x14*/
	volatile unsigned int conf_ctrl;            /*0x0B0*/
    volatile unsigned int conf_dbg;             /*0x0B4*/
    volatile unsigned char pad8[0x8];                       /*0x0C0 - 0x0B4 - 4 = 0x8*/
    volatile unsigned int sku;           		/*0x0C0*/
    volatile unsigned char pad9[0x3C];                      /*0x100 - 0x0C0 - 4 = 0x3C*/
    volatile unsigned int fan_intr_stat;        /*0x100*/
    volatile unsigned int fan_intr_mask;        /*0x104*/
    volatile unsigned int fan_stat;             /*0x108*/
    volatile unsigned char pada[0x4];                       /*0x110 - 0x108 - 4 = 0x4*/
    volatile unsigned int fan_ctrl;				/*0x110*/
    volatile unsigned char padb[0x10];						/*0x124 - 0x110 - 4 = 0x10*/
    volatile unsigned int ios_wd;               /*0x124*/
    volatile unsigned int ext_intr_pend;        /*0x128*/
    volatile unsigned int ext_intr_mask;        /*0x12C*/
    volatile unsigned int ext_intr_force;       /*0x130*/
    volatile unsigned char padc[0x14];                      /*0x148 - 0x130 - 4 = 0x14*/
    volatile unsigned int usb_com;				/*0x148*/
    volatile unsigned char padd[0xB4];						/*0x200 - 0x148 - 4 = 0xB4*/
    volatile unsigned int intr_stat;			/*0x200*/
    volatile unsigned int intr_mask;			/*0x204*/
    volatile unsigned char pade[0xC];						/*0x214 - 0x204 - 4 = 0xC*/
    volatile unsigned int intr_force;			/*0x214*/
    volatile unsigned int rst_button;           /*0x218*/
    volatile unsigned int rst_button_mask;      /*0x21C*/
    volatile unsigned char padf[0x1E4];                     /*0x404 - 0x21C - 4 = 0x1E4*/
    volatile unsigned int cc_status;            /*0x404*/
    volatile unsigned char padg[0x1FC];                     /*0x604 - 0x404 - 4 = 0x1FC*/
    volatile unsigned int fp_status;            /*0x604*/
} katar_sys_lvl_t;


/* SPI Reg Offset */
#define FPGA_SPI_PROM_CTRL_REG    	 		0x000
#define FPGA_SPI_PROM_STAT_REG       		0x004
#define FPGA_SPI_PROM_READ_SIZE_REG         0x008
#define FPGA_SPI_PROM_DATA_REG   			0x00C
#define FPGA_SPI_PROM_OP_CODE_REG      		0x010

typedef struct katar_spi_lvl_t_ {
    volatile unsigned int ctrl;  				/*0x000*/
    volatile unsigned int stat;					/*0x004*/
    volatile unsigned int read_size; 			/*0x008*/
    volatile unsigned int data; 				/*0x00C*/
    volatile unsigned int op_code;  			/*0x010*/
} katar_spi_lvl_t;


/* Non-IO Reg Offset */
#define FPGA_NIO_LED_CTRL_REG               0x000
#define FPGA_NIO_POE_POW_REG                0x004
#define FPGA_NIO_POE_RESET_REG              0x008
#define FPGA_NIO_POE_INTR_STAT_REG          0x00C
#define FPGA_NIO_POE_INTR_MASK_REG          0x010
#define FPGA_NIO_POE_INTR_FORCE_REG         0x014

typedef struct katar_nio_lvl_t_ {
    volatile unsigned int led_ctrl;             /*0x000*/
    volatile unsigned int poe_pow;              /*0x004*/
    volatile unsigned int poe_reset;            /*0x008*/
    volatile unsigned int poe_intr_stat;        /*0x00C*/
    volatile unsigned int poe_intr_mask;        /*0x010*/
    volatile unsigned int poe_intr_force;       /*0x014*/
} katar_nio_lvl_t;

/* IO Reg Offset */
#define FPGA_IO_RESET_CTRL_REG				0x000
#define FPGA_IO_SFP_STATUS_REG              0x004
#define FPGA_IO_SFP_INTR_STAT_REG           0x008
#define FPGA_IO_SFP_INTR_MASK_REG           0x00C
#define FPGA_IO_SFP_INTR_FORCE_REG          0x010

typedef struct katar_io_lvl_t_ {
    volatile unsigned int dev_rst;              /*0x000*/
    volatile unsigned int sfp_stat;             /*0x004*/
    volatile unsigned int sfp_intr_stat;        /*0x008*/
    volatile unsigned int sfp_intr_mask;        /*0x00C*/
    volatile unsigned int sfp_intr_force;       /*0x010*/
} katar_io_lvl_t;

/* Reg Content */
/* FPGA_LED_CTRL_REG(0x048) */
typedef enum {
    LED_SYS = 0,
    LED_HA,
    LED_ALARM,
    LED_MAX
} led_port_t;	

#define STAT_NO_BLINK                0x0
#define STAT_SLOW_BLINK              0x1
#define STAT_FAST_BLINK              0x2
#define STAT_LED_G                   0x1
#define STAT_LED_A                   0x2

#define OFFSET_LED_SYSTEM            0
#define OFFSET_BLINK_SYSTEM          2
#define OFFSET_LED_HA                4
#define OFFSET_BLINK_HA              6
#define OFFSET_LED_ALARM             8
#define OFFSET_BLINK_ALARM           10
#define MASK_LED_SYSTEM              0x00000003
#define MASK_BLINK_SYSTEM            0x0000000C
#define MASK_LED_HA                  0x00000030
#define MASK_BLINK_HA                0x000000C0
#define MASK_LED_ALARM               0x00000300
#define MASK_BLINK_ALARM             0x00000C00


/* FPGA_FAN_CTRL_REG(0x110) */
#define OFFSET_FAN_PWM_SET           0
#define OFFSET_FAN_PWM_CUR           8
#define OFFSET_FAN_RPM_TH            16
#define MASK_FAN_PWM_SET             0x000000FF
#define MASK_FAN_PWM_CUR             0x0000FF00
#define MASK_FAN_RPM_TH              0xFFFF0000


/* FPAG_FAN_STAT_REG(0x114) */
#define OFFSET_FAN_SPEED_0           0
#define OFFSET_FAN_SPEED_1           16


/* FPGA_USB_CONSOLE_REG(0x148) */
#define OFFSET_USB_COM_DETECT        0
#define OFFSET_USB_COM_RESET         2
#define OFFSET_USB_COM_SELECT        3
#define OFFSET_USB_COM_MANUAL		 4
#define MASK_USB_COM_DETECT          (1<<OFFSET_USB_COM_DETECT)
#define MASK_USB_COM_RESET           (1<<OFFSET_USB_COM_RESET)
#define MASK_USB_COM_SELECT          (1<<OFFSET_USB_COM_SELECT)
#define MASK_USB_COM_MANUAL          (1<<OFFSET_USB_COM_MANUAL)


typedef enum {
	RSTDEV_USB30 = 0,
	RSTDEV_USB_HUB,
	RSTDEV_GE_PHY_0,
	RSTDEV_GE_PHY_1,
	RSTDEV_EMMC,
	RSTDEV_POE,
	RSTDEV_25G_PHY,
	RSTDEV_SFP_MUX,
	RSTDEV_10G_PHY_B,
	RSTDEV_10G_PHY_A,
	RSTDEV_MAX
}rst_dev_type_t;

typedef enum {
	FPGA_RSTDEV_REG,
	NIO_RSTDEV_REG,
	IO_RSTDEV_REG
}rst_reg_type_t;

/* FPGA_LPC_EXT_DEV_RST_REG(0x01C) */
#define OFFSET_RSTDEV_USB30          5
#define OFFSET_RSTDEV_USB_HUB        6
#define OFFSET_RSTDEV_GE_PHY_0       21
#define OFFSET_RSTDEV_GE_PHY_1       22
#define OFFSET_RSTDEV_EMMC           23
#define MASK_RSTDEV_USB30            (1<<OFFSET_RSTDEV_USB30)
#define MASK_RSTDEV_USB_HUB          (1<<OFFSET_RSTDEV_USB_HUB)
#define MASK_RSTDEV_GE_PHY_0         (1<<OFFSET_RSTDEV_GE_PHY_0)
#define MASK_RSTDEV_GE_PHY_1         (1<<OFFSET_RSTDEV_GE_PHY_1)
#define MASK_RSTDEV_EMMC             (1<<OFFSET_RSTDEV_EMMC)
/* FPGA_NIO_POE_RESET_REG(0x008) */
#define OFFSET_RSTDEV_POE            0
#define MASK_RSTDEV_POE              (1<<OFFSET_RSTDEV_POE)
/* #define FPGA_IO_RESET_CTRL_REG (0x000) */
#define OFFSET_RSTDEV_25G_PHY        1
#define OFFSET_RSTDEV_SFP_MUX        4
#define OFFSET_RSTDEV_10G_PHY_B      24
#define OFFSET_RSTDEV_10G_PHY_A      25
#define MASK_RSTDEV_25G_PHY          (1<<OFFSET_RSTDEV_25G_PHY)
#define MASK_RSTDEV_SFP_MUX          (1<<OFFSET_RSTDEV_SFP_MUX)
#define MASK_RSTDEV_10G_PHY_B        (1<<OFFSET_RSTDEV_10G_PHY_B)
#define MASK_RSTDEV_10G_PHY_A        (1<<OFFSET_RSTDEV_10G_PHY_A)

/* FPGA_LPC_SPI_CTRL_REG(0x058) */
typedef enum {
	SPICTL_BOOT_GOLDEN = 0,
	SPICTL_BOOT_UPGRADE,
	SPICTL_BOOT_MAX
}boot_spi_type_t;

#define OFFSET_SPICTL_BOOT_OVRD		 31
#define OFFSET_SPICTL_BOOT_SEL		 30


/* SPI Reg Content */
/* FPGA_SPI_PROM_CTRL_REG(0x000) */
#define OFFSET_PROM_CTRL_USEADDR	0
#define OFFSET_PROM_CTRL_DIR        1
#define OFFSET_PROM_CTRL_USEDUMMY   2
#define OFFSET_PROM_CTRL_SWAPBYTE	3

/* FPGA_SPI_PROM_STAT_REG(0x004) */
#define OFFSET_PROM_STAT_DONE		15
#define OFFSET_PROM_STAT_W_ERR		0
#define OFFSET_PROM_STAT_R_EMPTY	1
#define OFFSET_PROM_STAT_R_FULL		2
#define OFFSET_PROM_STAT_W_EMPTY	3
#define OFFSET_PROM_STAT_W_FULL		4


/* FPGA_SPI_PROM_READ_SIZE_REG(0x008) */
#define MASK_PROM_READ_SIZE			0x000000FF


/* FPGA_SPI_PROM_DATA_REG(0x00C) */
#define MASK_PROM_RW_DATA			0x000000FF


/* FPGA_SPI_PROM_OP_CODE_REG(0x010) */
#define OFFSET_PROM_OP_ADDR			0
#define OFFSET_PROM_OP_CODE			24
#define MASK_PROM_OP_ADDR			0x00FFFFFF
#define MASK_PROM_OP_CODE			0xFF000000


/* FPGA_NIO_LED_CTRL_REG(0x000) */
typedef enum {
    LED_POE_P0 = 0,
    LED_POE_P1,
    LED_POE_SYS,
    LED_POE_MAX
} led_poe_port_t;

#define STAT_LED_OFF                 0x0
#define STAT_LED_ON                  0x1

#define OFFSET_LED_POE_P0            12
#define OFFSET_LED_POE_P1            13
#define OFFSET_LED_POE_SYS           14
#define MASK_LED_POE_P0              (1<<OFFSET_LED_POE_P0)
#define MASK_LED_POE_P1              (1<<OFFSET_LED_POE_P1)
#define MASK_LED_POE_SYS             (1<<OFFSET_LED_POE_SYS)


typedef enum {
	INTR_USB_COM,
	INTR_DIMM_OVERHEAT,
	INTR_RESET_BTN,
	INTR_FAN_TACH_LOW,
	INTR_POE,
	INTR_GE_SW,
	INTR_SFP_P1_PRESENT,
    INTR_SFP_P0_PRESENT,
    INTR_CCCP_READY,
    INTR_FPCP_READY,
    INTR_PKT_READY,
	INTR_ILL_ACC,
    INTR_SFP_P1_LOS,
    INTR_SFP_P0_LOS,
    INTR_SFP_P1_FAULT,
    INTR_SFP_P0_FAULT,
	INTR_MAX,
	INTR_ALL
}intr_type_t;

typedef enum {
	INTR_IRQ09_REG,		//0x24
	INTR_IRQ10_REG,		//0x2C
	INTR_IRQ11_REG,		//0x34
	INTR_MISC_REG,		//0x200
	INTR_RST_BTN_REG,	//0x218
	INTR_POE_REG,		//0x0C in non-io
	INTR_SFP_REG,		//0x08 in io
	INTR_ALL_REG
}intr_reg_type_t;

/* FPGA_INTR_STAT_REG(0x200) */
#define OFFSET_INTR_USB_COM			 2
#define MASK_INTR_USB_COM            (1<<OFFSET_INTR_USB_COM)
/* FPGA_RST_BUTTON_REG(0x218) */
#define OFFSET_INTR_RESET_BTN        0
#define MASK_INTR_RESET_BTN          (1<<OFFSET_INTR_RESET_BTN)
/* FPGA_LPC_IRQ09_STAT_REG(0x024) */
#define OFFSET_INTR_ILL_ACC			 2
#define MASK_INTR_ILL_ACC			 (1<<OFFSET_INTR_ILL_ACC)
#define MASK_INTR_IRQ09_ALL			 MASK_INTR_ILL_ACC
/* FPGA_LPC_IRQ10_STAT_REG(0x02C) */
#define OFFSET_INTR_DIMM             3
#define OFFSET_INTR_FAN_TACH         5
#define MASK_INTR_DIMM               (1<<OFFSET_INTR_DIMM)
#define MASK_INTR_FAN_TACH           (1<<OFFSET_INTR_FAN_TACH)
#define MASK_INTR_IRQ10_ALL			 (MASK_INTR_DIMM|MASK_INTR_FAN_TACH)
/* FPGA_LPC_IRQ11_STAT_REG(0x034) */
#define OFFSET_INTR_CCCP_READY		 1
#define OFFSET_INTR_FPCP_READY       1
#define OFFSET_INTR_PKT_READY        2
#define MASK_INTR_CCCP_READY         (1<<OFFSET_INTR_CCCP_READY)
#define MASK_INTR_FPCP_READY         (1<<OFFSET_INTR_FPCP_READY)
#define MASK_INTR_PKT_READY          (1<<OFFSET_INTR_PKT_READY)
#define MASK_INTR_IRQ11_ALL			 (MASK_INTR_CCCP_READY|MASK_INTR_PKT_READY)
/* FPGA_NIO_POE_INTR_STAT_REG(0x00C) */
#define OFFSET_INTR_POE		         0
#define MASK_INTR_POE                (1<<OFFSET_INTR_POE)
/* FPGA_IO_SFP_INTR_STAT_REG(0x008) */
#define OFFSET_INTR_GE_SW	         7
#define OFFSET_INTR_SFP1_PRE         8
#define OFFSET_INTR_SFP1_LOS         9
#define OFFSET_INTR_SFP1_FAU         10
#define OFFSET_INTR_SFP0_PRE         11
#define OFFSET_INTR_SFP0_LOS         12
#define OFFSET_INTR_SFP0_FAU         13
#define MASK_INTR_GE_SW           	 (1<<OFFSET_INTR_GE_SW)
#define MASK_INTR_SFP1_PRE           (1<<OFFSET_INTR_SFP1_PRE)
#define MASK_INTR_SFP1_LOS           (1<<OFFSET_INTR_SFP1_LOS)
#define MASK_INTR_SFP1_FAU         	 (1<<OFFSET_INTR_SFP1_FAU)
#define MASK_INTR_SFP0_PRE           (1<<OFFSET_INTR_SFP0_PRE)
#define MASK_INTR_SFP0_LOS           (1<<OFFSET_INTR_SFP0_LOS)
#define MASK_INTR_SFP0_FAU         	 (1<<OFFSET_INTR_SFP0_FAU)


/* FPGA_RST_BUTTON_REG(0x218) */
#define OFFSET_RST_BTN_STAT          0
#define OFFSET_RST_BTN_DUR           1
#define MASK_RST_BTN_STAT            (1<<OFFSET_RST_BTN_STAT)
#define MASK_RST_BTN_DUR             (1<<OFFSET_RST_BTN_DUR)


/* FPGA_IO_SFP_STATUS_REG(0x004) */
#define OFFSET_SFP_P0_TX_DIS		 0
#define OFFSET_SFP_P0_PRESENT        1
#define OFFSET_SFP_P0_RS			 2
#define OFFSET_SFP_P0_TS			 3
#define OFFSET_SFP_P1_TX_DIS		 4
#define OFFSET_SFP_P1_PRESENT        5
#define OFFSET_SFP_P1_RS			 6
#define OFFSET_SFP_P1_TS			 7
#define MASK_SFP_P0_TX_DIS			 (1<<OFFSET_SFP_P0_TX_DIS)
#define MASK_SFP_P0_PRESENT          (1<<OFFSET_SFP_P0_PRESENT)
#define MASK_SFP_P0_RS			 	 (1<<OFFSET_SFP_P0_RS)
#define MASK_SFP_P0_TS          	 (1<<OFFSET_SFP_P0_TS)
#define MASK_SFP_P1_TX_DIS			 (1<<OFFSET_SFP_P1_TX_DIS)
#define MASK_SFP_P1_PRESENT          (1<<OFFSET_SFP_P1_PRESENT)
#define MASK_SFP_P1_RS			 	 (1<<OFFSET_SFP_P1_RS)
#define MASK_SFP_P1_TS          	 (1<<OFFSET_SFP_P1_TS)


/* Externs */
#ifndef LINUX_KLM
extern unsigned long dash_cpld; /* TPM space */
extern unsigned long dash_fpga;
#endif

int read_fpga_reg (int verbose);
int write_fpga_reg (int verbose);
void katar_disable_boot_timer(void);
/* FPAG_FAN_STAT_REG(0x114) */
int katar_get_fan_speed (int fan_num);
/* FPGA_FAN_CTRL_REG(0x110) */
int katar_get_fan_pwm_setting (void);
int katar_get_fan_pwm_current (void);
int katar_get_fan_rpm_threshold (void);
void katar_set_fan_pwm (int pwm_setting);
void katar_set_fan_threshold (int pwm_th);
/* FPGA_LPC_STAT_LED_CTRL_REG(0x014) */
void katar_get_led_control(void);
/* FPGA_LED_CTRL_REG(0x048) */
void katar_set_led_ctrl (uint led_port, int blink, int color);
int katar_get_led_ctrl_reg (void);
int katar_get_led_color (uint led_port);
int katar_get_led_blink (uint led_port);
void katar_set_poe_led_color (uint led_port, int color);
int katar_get_poe_led_color (uint led_port);
/* FPGA_LPC_SCRATCHPAD_REG(0x008) */
unsigned long get_scratchpad_reg_addr (void);
int clear_fpga_status(void);
/* FPGA_INTR_STAT_REG(0x200) / FPGA_INTR_MASK_REG(0x204) / FPGA_INTR_FORCE_REG(0x214) */
int katar_check_interupt (uint intr_typ,int bClear);
int katar_clear_interupt (uint intr_typ);
int katar_check_interupt_mask (uint intr_typ);
void katar_interupt_mask_control (uint intr_typ ,int bSet);
int katar_force_interupt (uint intr_typ);
void katar_interupt_mask_clear_for_mb_test(void);
/* FPGA_USB_CONSOLE_REG(0x148) */
int katar_get_usb_com_stat (void);
int katar_get_usb_com_manual (void);
void katar_set_usb_com_manual (int bEnable);
void katar_set_usb_com_control (int bEnable);
/* FPGA_RESET_BUTTON_REG(0x10C) */
int katar_get_rst_btn_info (int *btn_stat,int *btn_dur,int bClear);
/* FPGA_LPC_EXT_DEV_RST_REG(0x01C) */
void katar_reset_device (uint dev_typ, uint bReset);
/* FPGA_LPC_BOARDTYPE_REG(0x080) */
int katar_get_usrlogic_ver (void);
int katar_get_platform_ver (unsigned int verbose, unsigned int *cpld_ver,
                      unsigned int *fpga_ver, unsigned int *cpld_brd,
                      unsigned int *fpga_brd);
/* FPGA_LPC_SPI_CTRL_REG(0x058) */
int katar_boot_spi_select_control(int bClear, unsigned int boot_typ);
/* FPGA_IO_SFP_STATUS_REG(0x004) */
int katar_is_sfp_present (int portnum);

/* SPI Reg Functions */
void katar_set_prom_opcode (int opcode,int address);
void katar_set_prom_read_length(int legth);
void katar_set_prom_write_data(uint8_t data);
uint8_t katar_get_prom_read_data(void);
int katar_check_prom_FIFO_Empty(boolean bCheckRead);
int katar_clear_prom_op_done(void);
int katar_clear_prom_FIFO_status(boolean bClrRead);
int katar_check_prom_op_done(boolean bClear);
void katar_set_prom_control(boolean bWrite, boolean bUseAddr, boolean bUseDummy, boolean bSwapByte);

int display_fpga_regs (int dummy);

#endif   /* __KATAR_PLATFORM_FPGA_H__ */

/*
 *------------------------------------------------------------------
 * $Log: platform_fpga.h,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.3  2019/03/05 07:29:37  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.2  2019/02/20 02:54:48  mikech2
 * Add SFP present test in SPF intr test
 *
 * Revision 1.1.2.1  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.4  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.3  2018/11/22 06:55:08  mikech2
 * Add security FPGA version info
 *
 * Revision 1.1.2.2  2018/11/01 08:55:02  mikech2
 * Disable boot timer when enter diag
 *
 * Revision 1.1.2.1  2018/10/22 08:02:25  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.11  2018/10/11 06:49:04  mikech2
 * Update FPGA intr test for SERIRQ
 *
 * Revision 1.1.2.10  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.9  2018/09/12 08:32:48  mikech2
 * Fix userlogic FPGA update & system info version issue
 *
 * Revision 1.1.2.8  2018/09/07 02:16:52  mikech2
 * Fix FPGA util issue
 *
 * Revision 1.1.2.7  2018/07/19 06:32:03  mikech2
 * modify logic FPGA upgrade flow
 *
 * Revision 1.1.2.6  2018/06/29 06:48:11  mikech2
 * Add spi boot control function
 *
 * Revision 1.1.2.5  2018/06/28 03:32:56  mikech2
 * Add interrupt mask control menu
 *
 * Revision 1.1.2.4  2018/06/27 01:26:29  mikech2
 * Add reset/unreset device menu
 *
 * Revision 1.1.2.3  2018/06/25 08:24:53  mikech2
 * Add interupt test menu
 *
 * Revision 1.1.2.2  2018/06/21 08:24:09  mikech2
 * remove unused menu, add scratchpad reg test
 *
 * Revision 1.1.2.1  2018/06/20 07:31:13  mikech2
 * Add fan/led/margin control menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
