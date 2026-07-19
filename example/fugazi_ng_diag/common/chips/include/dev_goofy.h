/* $Id: dev_goofy.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/dev_goofy.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: 
 *
 * June 2006 - Bao Buu
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _GOOFY_DEV_H_
#define _GOOFY_DEV_H_

#include "dev_object.h"
#include "goofy_reset.h"
#include "goofy_hsib.h"
#include "goofy_gpio.h"
#include "goofy_global.h"
#include "goofy_dbgbus.h"
#include "goofy_intr.h"
#include "goofy_i2c.h"

#define GFY_BSWAP32(x)  ((((x)&0xff)<<24) + (((x)&0xff00)<<8) + (((x)&0xff0000)>>8) + (((x)&0xff000000)>>24))

/*
 * ASIC revision ID and revision defines
 */
#define KOMYUTA_MAIN_FPGA_ID   0x4b4d  /* ASCII "KM" */
#define KOMYUTA_WAN_FPGA_ID    0x4b57  /* ASCII "KW" */
#define GOOFY_1_DEVICE_VER     0x1     /* Device version for Goofy 1 */
#define GOOFY_FPGA_ID     0x4746  /* ASCII "GF" */
#define GOOFY_ASIC_ID     0x4741  /* ASCII "GA" */
#define SCROOGE_FPGA_ID   0x4446  /* ASCII "DF" */
#define SCROOGE_ASIC_ID   0x4441  /* ASCII "DA" */
#define PLUTO_FPGA_ID     0x5046  /* ASCII "PF" */
#define DAFFY_FPGA_ID     0x4446  /* ASCII "DF" */
#define DEVICE_VER_MASK      0xf0000000
#define ASIC_ID_MASK         0xffff0000
#define ASIC_MAJOR_REV_MSK   0x0000ff00
#define ASIC_MINOR_REV_MSK   0x000000ff
#define DEVICE_VER_SHFT      28
#define ASIC_ID_SHFT         16
#define ASIC_MAJOR_REV_SHFT  8
#define ASIC_MINOR_REV_SHFT  0

/*
 * These major number are according to what is hardcoded in
 * the revision_id register in global register
 */
#define KOMYUTA_MAIN_MAJOR_REV_1    0x01
#define KOMYUTA_MAIN_MAJOR_REV_2    0x02
#define KOMYUTA_MAIN_MAJOR_REV_3    0x03
#define KOMYUTA_MAIN_MAJOR_REV_F3   0xf3
#define GOOFY_MAJOR_REV_1           0x01

/*
 * Enum for ASIC major and minor revision
 */
enum {
  ASIC_MAJOR_REV_0 = 0,
  ASIC_MAJOR_REV_1,
  ASIC_MAJOR_REV_2,
  ASIC_MAJOR_REV_MAX,
};

enum {
  ASIC_MINOR_REV_0 = 0,
  ASIC_MINOR_REV_1,
  ASIC_MINOR_REV_2,
  ASIC_MINOR_REV_MAX,
};

enum {
  ASIC_JTAG_DEV_VER_1 = 1,
  ASIC_JTAG_DEV_VER_2,
};

/*
 * ASIC supported features
 */
#define GOOFY_US_LINK_WIDTH   4
#define GOOFY_DS_PORT_MAX     4
#define GOOFY_HWIC_PORT_MAX   4
#define GOOFY_TDM_PORT_MAX    128
#define GOOFY_HDLC_CORE_MAX   8
#define GOOFY_SCC_CORE_MAX    8
#define GOOFY_PVDM_PORT_MAX   4
#define GOOFY_GPIO_BIT_MAX    48
#define GOOFY_SGPIO_BIT_MAX   64
#define GOOFY_I2C_PORT_MAX    5

#define SCROOGE_US_LINK_WIDTH   2
#define SCROOGE_DS_PORT_MAX     2
#define SCROOGE_HWIC_PORT_MAX   2
#define SCROOGE_TDM_PORT_MAX    32
#define SCROOGE_HDLC_CORE_MAX   4
#define SCROOGE_SCC_CORE_MAX    4
#define SCROOGE_PVDM_PORT_MAX   0
#define SCROOGE_GPIO_BIT_MAX    20
#define SCROOGE_SGPIO_BIT_MAX   64
#define SCROOGE_I2C_PORT_MAX    3

/*
 * Features same for goofy and scrooge
 */
#define GOOFY_WDOG_MAX    2
#define GOOFY_REG_SZ      32
#define GOOFY_PCIE_CORE_ID      0x001e1137
#define SCROOGE_PCIE_CORE_ID    0x001f1137

/****************************************************/
/* Goofy ASIC device node and device address map    */
/****************************************************/
/*
 * device offset for QHWIC devices
 * Goofy EAS defined the 4 hwic remote region occupies 8MB of
 * continuous memory space with 2MB each.
 */
#define GOOFY_HWIC_REMOTE_BLK_SZ   0x200000
#define GOOFY_HWIC0_REMOTE_OFFSET  0
#define GOOFY_HWIC1_REMOTE_OFFSET  (GOOFY_HWIC0_REMOTE_OFFSET + GOOFY_HWIC_REMOTE_BLK_SZ)
#define GOOFY_HWIC2_REMOTE_OFFSET  (GOOFY_HWIC1_REMOTE_OFFSET + GOOFY_HWIC_REMOTE_BLK_SZ)
#define GOOFY_HWIC3_REMOTE_OFFSET  (GOOFY_HWIC2_REMOTE_OFFSET + GOOFY_HWIC_REMOTE_BLK_SZ)
#define GOOFY_QHWIC_BLK_SZ         (GOOFY_HWIC_PORT_MAX * GOOFY_HWIC_REMOTE_BLK_SZ)

/*
 * device offset for WAN devices (HDLC, PKT PUMP, SCC, TDM)
 * Goofy EAS defined the following memory space allocation of the WAN
 * devices in the device gasket section.
 */
#define GOOFY_HDLC_OFFSET      0
#define GOOFY_HDLC_MAX_NUM     GOOFY_HDLC_CORE_MAX
#define GOOFY_HDLC_BLK_SZ      0x10000
#define GOOFY_PKT_PUMP_OFFSET  (GOOFY_HDLC_OFFSET + (GOOFY_HDLC_MAX_NUM * GOOFY_HDLC_BLK_SZ))
#define GOOFY_PKT_PUMP_BLK_SZ  0x10000
#define GOOFY_SCC_OFFSET       (GOOFY_PKT_PUMP_OFFSET + GOOFY_PKT_PUMP_BLK_SZ)
#define GOOFY_SCC_BLK_SZ       0x10000
#define GOOFY_TDM_OFFSET       (GOOFY_SCC_OFFSET + GOOFY_SCC_BLK_SZ)
#define GOOFY_TDM_BLK_SZ       0x20000
#define GOOFY_WAN_BLK_SZ       0x00100000 /* 1MB */

/*
 * device offset for Global Reg block (Interrupt, GPIO, Global Reg, ...)
 * Goofy EAS defined the following memory space allocation of the
 * Global Register block
 */
#define GOOFY_INTERRUPT_OFFSET     0
#define GOOFY_INTERRUPT_BLK_SZ     0x200
#define GOOFY_GPIO_OFFSET          (GOOFY_INTERRUPT_OFFSET + GOOFY_INTERRUPT_BLK_SZ)
#define GOOFY_GPIO_BLK_SZ          0x200
#define GOOFY_GLOBAL_REG_OFFSET    (GOOFY_GPIO_OFFSET + GOOFY_GPIO_BLK_SZ)
#define GOOFY_GLOBAL_REG_BLK_SZ    0x80
#define GOOFY_RESET_CNTRL_OFFSET   (GOOFY_GLOBAL_REG_OFFSET + GOOFY_GLOBAL_REG_BLK_SZ)
#define GOOFY_RESET_CNTRL_BLK_SZ   0x80
#define GOOFY_RESERVED_OFFSET      (GOOFY_RESET_CNTRL_OFFSET + GOOFY_RESET_CNTRL_BLK_SZ)
#define GOOFY_RESERVED_BLK_SZ      0xFB00
#define GOOFY_I2C_CNTRL_OFFSET     (GOOFY_RESERVED_OFFSET + GOOFY_RESERVED_BLK_SZ)
#define GOOFY_I2C_CNTRL_BLK_SZ     0x10000
#define GOOFY_I2C0_OFFSET          GOOFY_I2C_CNTRL_OFFSET
#define GOOFY_I2C1_OFFSET          (GOOFY_I2C0_OFFSET + GOOFY_I2C_CNTRL_BLK_SZ)
#define GOOFY_I2C2_OFFSET          (GOOFY_I2C1_OFFSET + GOOFY_I2C_CNTRL_BLK_SZ)
#define GOOFY_I2C3_OFFSET          (GOOFY_I2C2_OFFSET + GOOFY_I2C_CNTRL_BLK_SZ)
#define GOOFY_I2C4_OFFSET          (GOOFY_I2C3_OFFSET + GOOFY_I2C_CNTRL_BLK_SZ)
#define GOOFY_GLB_REG_BLK_SZ       (GOOFY_I2C4_OFFSET + GOOFY_I2C_CNTRL_BLK_SZ) /* 384K */

/* Expanded LUT2 to accomodate SPI PROM for Pluto FPGA */
#define PLUTO_GLB_REG_BLK_SZ       (GOOFY_GLB_REG_BLK_SZ + GOOFY_I2C_CNTRL_BLK_SZ) 

/*
 * device offset for QHWIC local register
 * Goofy EAS defined the 4 hwic local host region occupies 64KB of
 * continuous memory space with 16KB each.
 */
#define GOOFY_HWIC_LOCAL_BLK_SZ    0x4000           /* 16K each */
#define GOOFY_HWIC0_LOCAL_OFFSET   0
#define GOOFY_HWIC1_LOCAL_OFFSET   (GOOFY_HWIC0_LOCAL_OFFSET + GOOFY_HWIC_LOCAL_BLK_SZ)
#define GOOFY_HWIC2_LOCAL_OFFSET   (GOOFY_HWIC1_LOCAL_OFFSET + GOOFY_HWIC_LOCAL_BLK_SZ)
#define GOOFY_HWIC3_LOCAL_OFFSET   (GOOFY_HWIC2_LOCAL_OFFSET + GOOFY_HWIC_LOCAL_BLK_SZ)
#define GOOFY_QHWIC_LOCAL_BLK_SZ   (GOOFY_HWIC_PORT_MAX *  GOOFY_HWIC_LOCAL_BLK_SZ)

/****************************************************/
/* Scrooge ASIC device node and device address map   */
/****************************************************/
/*
 * device offset for QHWIC devices
 * Goofy EAS defined the 2 hwic remote region occupies 4MB of
 * continuous memory space with 2MB each.
 */
#define SCROOGE_HWIC_REMOTE_BLK_SZ   0x200000
#define SCROOGE_HWIC0_REMOTE_OFFSET  0
#define SCROOGE_HWIC1_REMOTE_OFFSET  (SCROOGE_HWIC0_REMOTE_OFFSET + SCROOGE_HWIC_REMOTE_BLK_SZ)
#define SCROOGE_QHWIC_BLK_SZ         (SCROOGE_HWIC_PORT_MAX * SCROOGE_HWIC_REMOTE_BLK_SZ)

/*
 * device offset for WAN devices (HDLC, PKT PUMP, SCC, TDM)
 * Goofy EAS defined the following memory space allocation of the WAN
 * devices in the device gasket section. Scrooge and Goofy share the
 * same device address map inside the WAN device node. Scrooge does
 * not have packet pump, and has less number of HDLC and SCC devices.
 */
#define SCROOGE_HDLC_OFFSET           0
#define SCROOGE_HDLC_MAX_NUM          SCROOGE_HDLC_CORE_MAX
#define SCROOGE_HDLC_BLK_SZ           GOOFY_HDLC_BLK_SZ
#define SCROOGE_SCC_OFFSET            GOOFY_SCC_OFFSET
#define SCROOGE_SCC_BLK_SZ            GOOFY_SCC_BLK_SZ
#define SCROOGE_TDM_OFFSET            GOOFY_TDM_OFFSET
#define SCROOGE_TDM_BLK_SZ            GOOFY_TDM_BLK_SZ
#define SCROOGE_WAN_BLK_SZ            GOOFY_WAN_BLK_SZ

/*
 * device offset for Global Reg block (Interrupt, GPIO, Global Reg, ...)
 * Goofy EAS defined the following memory space allocation of the
 * Global Register block. Scrooge and Goofy share the same address map
 * of the Global device block. Scrooge has less I2C masters.
 */
#define SCROOGE_INTERRUPT_OFFSET     0
#define SCROOGE_INTERRUPT_BLK_SZ     GOOFY_INTERRUPT_BLK_SZ
#define SCROOGE_GPIO_OFFSET          GOOFY_GPIO_OFFSET
#define SCROOGE_GPIO_BLK_SZ          GOOFY_GPIO_BLK_SZ
#define SCROOGE_GLOBAL_REG_OFFSET    GOOFY_GLOBAL_REG_OFFSET
#define SCROOGE_GLOBAL_REG_BLK_SZ    GOOFY_GLOBAL_REG_BLK_SZ
#define SCROOGE_RESET_CNTRL_OFFSET   GOOFY_RESET_CNTRL_OFFSET
#define SCROOGE_RESET_CNTRL_BLK_SZ   GOOFY_RESET_CNTRL_BLK_SZ
#define SCROOGE_RESERVED_OFFSET      GOOFY_RESERVED_OFFSET
#define SCROOGE_RESERVED_BLK_SZ      GOOFY_RESERVED_BLK_SZ
#define SCROOGE_I2C_CNTRL_OFFSET     GOOFY_I2C_CNTRL_OFFSET
#define SCROOGE_I2C_CNTRL_BLK_SZ     GOOFY_I2C_CNTRL_BLK_SZ
#define SCROOGE_I2C0_OFFSET          GOOFY_I2C0_OFFSET
#define SCROOGE_I2C1_OFFSET          GOOFY_I2C1_OFFSET
#define SCROOGE_I2C2_OFFSET          GOOFY_I2C2_OFFSET
#define SCROOGE_GLB_REG_BLK_SZ       GOOFY_GLB_REG_BLK_SZ

/* Expanded LUT2 to accomodate SPI PROM for Daffy FPGA */
#define DAFFY_GLB_REG_BLK_SZ       PLUTO_GLB_REG_BLK_SZ

/*
 * device offset for QHWIC local register
 * Goofy EAS defined the 3 hwic local host region occupies 48KB of
 * continuous memory space with 16KB each.
 */
#define SCROOGE_HWIC_LOCAL_BLK_SZ    0x4000           /* 16K each */
#define SCROOGE_HWIC0_LOCAL_OFFSET   0
#define SCROOGE_HWIC1_LOCAL_OFFSET   (SCROOGE_HWIC0_LOCAL_OFFSET + SCROOGE_HWIC_LOCAL_BLK_SZ)
#define SCROOGE_HWIC2_LOCAL_OFFSET   (SCROOGE_HWIC1_LOCAL_OFFSET + SCROOGE_HWIC_LOCAL_BLK_SZ)
#define SCROOGE_QHWIC_LOCAL_BLK_SZ   (SCROOGE_HWIC_PORT_MAX *  SCROOGE_HWIC_LOCAL_BLK_SZ)

				
/*
 * Goofy internal core frequency
 */
#define GOOFY_CORE_CLK_50                50   /* 50 MHz. Koymuta fpga */
#define GOOFY_CORE_CLK_250               250  /* 250 MHz Goofy ASIC */

/*
 * Goofy device nodes. The order is fixed by the HW.
 */
typedef enum {
    DN_0_PCI_US = 0,
    DN_1_PCI_DS0,
    DN_2_PCI_DS1,
    DN_3_PCI_DS2,
    DN_4_PCI_DS3,
    DN_5_GLB_REG,
    DN_6_QHWIC,
    DN_7_WAN,
    GOOFY_MAX_NUM_DN,
} goofy_dn_t;

/*
 * Goofy functional block that occupies system memory space
 */
typedef enum {
    GOOFY_DEV_QHWIC = 0,
    GOOFY_DEV_QHWIC_LOCAL,
    GOOFY_DEV_HDLC,
    GOOFY_DEV_PKTPUMP,
    GOOFY_DEV_SCC,
    GOOFY_DEV_TDM,
    GOOFY_DEV_INTERRUPT,
    GOOFY_DEV_GPIO,
    GOOFY_DEV_GLOBAL_REG,
    GOOFY_DEV_RESET,
    GOOFY_DEV_I2C_CNTRL,
    GOOFY_DEV_HSIB,
    GOOFY_DEV_PCIE_US0,
    GOOFY_DEV_PCIE_DS0,
    GOOFY_DEV_PCIE_DS1,
    GOOFY_DEV_PCIE_DS2,
    GOOFY_DEV_PCIE_DS3,
    GOOFY_MAX_NUM_DEV,
} goofy_dev_t;

typedef struct dnode_addr_t_ {
    ulong addr_ofst; /* offset from the beginning of the HSIB menory space */
    boolean valid;
} dnode_addr_t;

/*
 * Data structure which contains the ASIC information.
 * Goofy has a simpler version called Scrooge, which has
 * less on chip hardware resource.
 */
typedef struct asic_info_t_ {
    /*
     * Individual asic information
     */
    uint32_t id;    /* ASIC ID number assigned by the platform */
    char * name;    /* ASIC name assigned by the platform */
    uint32_t jtag_id;   /* JTAG ID in global register */
    uint32_t rev_id;   /* ASIC revision ID in global register */
    uint32_t ustream_link_width;
    uint32_t dstream_port_max;
    uint32_t hwic_port_max;
    uint32_t tdm_port_max;
    uint32_t hdlc_core_max;
    uint32_t scc_core_max;
    uint32_t pvdm_port_max;
    uint32_t gpio_bit_max;
    uint32_t i2c_port_max;
    uint32_t core_clk_rate;

    /* Cavium 5xxx cpu can only support 2 interrupt priority levels. The 
     * management interrupt level is not used with this cpu 
     */
    boolean use_2_level_intr;

    /* 
     * Little endian CPU (e.g. Intel) needs software byte swap to
     * access HSIB config space
     */
    boolean hsib_config_access_byte_swap_flag;

    /*
     * Goofy internal device node address map
     */
    ulong asic_addr; /* asic's system address */
    uint32_t bar0; /* value programmed in the upstream bar0 reg */
    uint32_t hsib_conf_ofst; /* hsib config space offset from bar0 */
    uint32_t qhwic_local_base_ofst; /* quad hwic local register base offset */
    dnode_addr_t dnode_addr[GOOFY_MAX_NUM_DN];
} asic_info_t;

/*
 * Macro for asic information display control
 */
#define FLAG_DISP_ASIC_ID    0x00000001
#define FLAG_DISP_ASIC_ADDR  0x00000002
#define FLAG_DISP_ASIC_LUT   0x00000004

/*
 * device callin function - service provided and defined by the device
 */
typedef struct dev_goofy_callin_fvt_ {
    /*  
     * General purpose driver functions
     */ 
    uint32_t (*get_asic_id)(dev_object_t *dev);
    uint16_t (*get_goofy_id)(dev_object_t *dev);
    uint8_t (*get_device_ver)(dev_object_t *dev);
    uint8_t (*get_major_rev)(dev_object_t *dev);
    uint8_t (*get_minor_rev)(dev_object_t *dev);
    char * (*get_asic_name)(dev_object_t *dev);
    ulong (*get_dn_addr)(dev_object_t *, goofy_dn_t);
    char * (*get_dn_name)(goofy_dn_t);
    ulong  (*get_dev_base_addr)(dev_object_t *, goofy_dev_t, int);
    char * (*get_dev_name) (goofy_dev_t);
    ulong (*get_reg_addr)(dev_object_t *, goofy_dev_t, int, uint);
    void (*wr_reg) (dev_object_t *dev, goofy_dev_t goofy_dev,
		    uint32_t module_num, uint32_t reg_ofst, uint32_t val);
    uint32_t (*rd_reg) (dev_object_t *dev, goofy_dev_t goofy_dev,
			uint32_t module_num, uint32_t reg_ofst);
    void   (*register_display)(dev_object_t *, goofy_dev_t, uint32_t);

    /*
     * Global register driver functions
     */
    int  (*global_reg_test)(dev_object_t *);
    void (*global_reg_dev_init)(dev_object_t *);
    void (*en_glb_reg_sti_err_intr)(dev_object_t *);

    /*
     * HSIB driver functions
     */
    ulong (*get_hsib_config_base_addr) (dev_object_t *dev);
    ulong (*get_dn_hsib_config_addr) (dev_object_t *dev, goofy_dn_t dn);
    ulong (*get_hsib_config_reg_addr) (dev_object_t *dev, goofy_dn_t dn, uint32 reg_ofst);
    void (*set_lut_entry)(dev_object_t *, goofy_dn_t, uint32_t, uint32_t, uint32_t);
    void (*clr_asic_lut)(dev_object_t *dev);
    void (*show_asic_lut)(dev_object_t *dev);
    void (*hsib_display_err_stat)(dev_object_t *, int, boolean);
    void (*init_hsib) (dev_object_t *dev);
    uint32_t (*hsib_cfg_rd)(dev_object_t *dev, volatile uint32_t *reg_ptr);
    void (*hsib_cfg_wr)(dev_object_t *dev, volatile uint32_t *reg_ptr, uint32_t data);
    void (*en_all_hsib_err_intr)(dev_object_t *dev);

    /*
     * GPIO driver functions
     */
    int    (*gpio_reg_test)(dev_object_t *);
    void (*write_gpio_reg) (dev_object_t *dev, gpio_reg_group_t reg_group,
			   uint32_t pin_num, uint32_t val);
    uint32_t (*read_gpio_reg) (dev_object_t *dev, gpio_reg_group_t reg_group,
			  uint32_t pin_num);
    void (*config_gpio_pin) (dev_object_t *dev, uint32_t pin_num,
			     goofy_gpio_inout_type_enum inout,
			     goofy_gpio_polar_enum polar);
    void (*set_gpio_pin) (dev_object_t *dev, uint32_t pin_num, uint32_t val);
    uint8_t (*get_gpio_pin) (dev_object_t *dev, uint32_t pin_num);
    void (*set_gpio_intr) (dev_object_t *dev, uint32_t pin_num,
			   goofy_gpio_intr_type_enum gpio_intr_type,
			   boolean onoff);
    void (*config_gpio_intr_pin) (dev_object_t *dev, uint32_t pin_num,
				  goofy_gpio_intr_type_enum gpio_intr_type,
				  goofy_gpio_latched_intr_enum latch,
				  goofy_gpio_polar_enum polar,
				  boolean onoff);
    void (*config_sgpio_pin) (dev_object_t *dev, uint32_t pin_num,
			     goofy_gpio_inout_type_enum inout,
			     goofy_gpio_polar_enum polar);
    void (*set_sgpio_pin) (dev_object_t *dev, uint32_t pin_num, uint32_t val);
    uint8_t (*get_sgpio_pin) (dev_object_t *dev, uint32_t pin_num,
			      goofy_gpio_inout_type_enum inout);
    void (*set_sgpio_intr) (dev_object_t *dev, uint32_t pin_num,
			    goofy_gpio_intr_type_enum gpio_intr_type,
			    boolean onoff);
    void (*config_sgpio_intr_pin) (dev_object_t *dev, uint32_t pin_num,
				   goofy_gpio_intr_type_enum gpio_intr_type,
				   goofy_gpio_polar_enum polar,
				   boolean onoff);
    void (*set_sgpio_led_conf) (dev_object_t *dev, uint32_t reg_field_ofst,
				uint32_t time_value);
    void (*set_sgpio_led_blink) (dev_object_t *dev, uint32_t led_num,
				 uint32_t blink_count);

    /*
     * I2C driver functions
     */
    int (*i2c_master_reg_test)(dev_object_t *dev, uint8_t i2c_num);
    void (*init_dbgbus)(dev_object_t *dev);
    void (*init_i2c_masters)(dev_object_t *dev);
    int (*i2c_reset)(goofy_i2c_t *i2c);
    int (*i2c_init)(goofy_i2c_t *i2c, uint32_t op_spd);
    int (*i2c_rd)(goofy_i2c_t *i2c, uint32_t slv_addr, uint32_t sub_addr_sz,
		  uint32_t reg_addr, uint32_t data_len, uchar *data_buf);
    int (*i2c_wr)(goofy_i2c_t *i2c, uint32_t slv_addr, uint32_t sub_addr_sz,
		  uint32_t reg_addr, uint32_t data_len, uchar *data_buf);
    int (*i2c_dma_wr)(goofy_i2c_t *i2c, uint32_t slv_addr, 
		      uint32_t sub_addr_sz, uint32_t reg_addr,
		      uint32_t data_len, uchar *data_buf);
    int (*i2c_bus_recovery)(goofy_i2c_t *i2c);

    /* old i2c code, only used by shinkansen */
    int    (*i2c_op)(dev_object_t *, goofy_i2c_info_t *);

    /*
     * Interrupt driver functions
     */
    int    (*intr_reg_test)(dev_object_t *);
    void   (*init_interrupt)(dev_object_t *dev, uint32_t cpu,
			     goofy_intr_type_enum type, 
			     msi_multi_msg_enum multi_msg, boolean msi_en,
			     uint32_t msg_addr, uint32_t msg_data,
			     uint32_t intr_mask);
    void   (*intr_handler)(dev_object_t *dev, uint32_t cpu, uint32_t type,
			   uint32_t intr_data);
    PFI    (*install_isr_vect)(dev_object_t *dev, goofy_intr_class_t class,
			       uint32_t intr_num, uint32_t type, PFI vect);
    int    (*set_interrupt)(dev_object_t *dev, goofy_intr_class_t class,
		   uint32_t intr_num, uint32_t type, uint32_t cpu,
		   boolean enable);
    int (*set_hsib_err_intr) (dev_object_t *dev, uint32_t hsib_err_intr_num,
		       uint32_t ini_tgt_error, boolean onoff);

    /*
     * Reset controller driver functions
     */
    int    (*reset_reg_test)(dev_object_t *);
    int    (*reset_goofy)(dev_object_t *);
    void   (*init_reset)(dev_object_t *);
    void   (*set_cpu_host_reset)(dev_object_t *, goofy_reset_class_t);
    void   (*set_func_blk_reset)(dev_object_t *, goofy_dev_t, int, boolean);
    void   (*set_dn_reset)(dev_object_t *, goofy_dn_t, boolean, boolean);
    void   (*set_wd_enable)(dev_object_t *, int, boolean);
    void   (*set_wd_limit)(dev_object_t *, int, uint32_t);
    void   (*service_wd)(dev_object_t *, int);  

    /*
     * HWIC driver functions
     */
    int    (*hwic_reg_test)(dev_object_t *);
    int    (*hwic_slot_reg_test)(dev_object_t *, int );
} dev_goofy_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct dev_goofy_callout_fvt_ {
    boolean (*is_ehwic_host_valid)(dev_object_t *, int);
    void * (*get_internal_dev_inst)(dev_object_t *dev, goofy_dev_t d_type,
				    int num);
    void * (*get_plat_gfydev_inst)(goofy_dev_t d_type, uint32_t num);
    void * (*get_plat_goofy_inst)(uint32_t mod_type, uint32_t mod_num);
    int (*wr_dbgbus)(uint32_t asic_id, uint32_t reg_addr, 
			  uint32_t data);
    int (*rd_dbgbus)(uint32_t asic_id, uint32_t reg_addr,
			  uint32_t *data_buff, uchar reg_count);
} dev_goofy_callout_fvt_t;

/*
 * Define the Goofy ASIC device object structure.
 */
typedef struct dev_goofy_object_t_ {
    dev_object_t            base;
    dev_goofy_callout_fvt_t *callout_fvt;
    dev_goofy_callin_fvt_t  *callin_fvt;
    asic_info_t             asic_info;
    intr_data_t             goofy_top_intr_data_tbl[MAX_INTR_TYPE][GOOFY_REG_SZ];
    intr_data_t             goofy_gpio_intr_data_tbl[GOOFY_GPIO_BIT_MAX];
    intr_data_t             goofy_sgpio_intr_data_tbl[GOOFY_SGPIO_BIT_MAX];
    intr_data_t             goofy_i2c_intr_data_tbl[GOOFY_I2C_PORT_MAX];
} dev_goofy_object_t;

/*
 * Function Prototype
 */
extern void dev_goofy_create (dev_object_t *dev,
			      dev_error_report_t error_report_fn);
extern void goofy_set_gpio_intr (dev_object_t *, int, int, int);
extern void goofy_set_sgpio_intr (dev_object_t *, int, int, int);

#endif /* _GOOFY_DEV_H_  */

/******** History ******** 
$Log: dev_goofy.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
