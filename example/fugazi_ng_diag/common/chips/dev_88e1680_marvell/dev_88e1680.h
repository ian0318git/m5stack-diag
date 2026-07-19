/* $Id: dev_88e1680.h,v 1.2 2019/12/11 10:10:22 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e1680_marvell/dev_88e1680.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : dev_88e1680.h
 * Description: Marvell 88e1680 PHY Device Driver
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_88E1680_H__
#define __DEV_88E1680_H__


#include "dev_object.h"
#include "common_utils.h"

#include "madApi.h"
#include "madHwCntl.h"

#define PHY_ADDR_BASE_88E1680    0

#define PHY_RESET    1
#define PHY_UNRESET    0

#define MRV88E1680_ERR_BUF_SIZE    (80)


#define GE0_INTER_LPBK  0
#define GE1_INTER_LPBK  1
#define GE0_EXTER_LPBK  2

#define PHY_CONTROL_REG                  0
#define PHY_SPECIFIC_CONTROL1_REG        16
#define PHY_SPECIFIC_STATUS1_REG         17
#define PHY_SPECIFIC_CONTROL2_REG        20
#define PHY_GLOBAL_INTR_STATUS_REG       23
#define PHY_SPECIFIC_CONTROL3_REG        26

#define PHY_QSGMII_INTR_ENA_REG          18
#define PHY_QSGMII_INTR_STATUS_REG       19

#define MRV88E1680_REG_PAGE_3                   3
#define MRV88E1680_TMR_CONTROL_REG              18

/* Page 3 Register 18 LED Timer Control Register */
#define PHY_TIMER_CNTRL_FORCE_INT       (0x8000)
#define PHY_TIMER_CNTRL_INTR_EN         (0x0080)

/* Copper Control Register (Page 0, Reg 0) */
#define PHY_COOPER_RST                   0x8000
#define PHY_LPBK_ENA                     0x4000
#define PHY_SPD_SEL_MASK                 0x2040
#define PHY_SPD_SEL_1000M                0x0040
#define PHY_SPD_SEL_100M                 0x2000
#define PHY_SPD_SEL_10M                  0x0000
#define PHY_AUTO_NEO_ENA                 0x1000
#define PHY_PWR_DOWN                     0x0800
#define PHY_RST_AUTO_NEO                 0x0200
#define PHY_COPPER_FULL_DUPLEX           0x0100

#define PHY_SPEED_MASK                   0x0007
#define PHY_SGMII_SPD_1000               0x0006
#define PHY_SGMII_SPD_100                0x0005
#define PHY_SGMII_SPD_10                 0x0004

/* Copper Auto-Nego Advertisement Register (Page 0, Reg 4) */
#define PHY_10BT_ADV                     0x60    
#define PHY_100BT_ADV                    0x180

/* 1000BASE-T control register (page 0, reg 9) */
#define PHY_1000BT_ADV                   0x300

/* Copper specific status register 1 (page 0, reg 17) */
#define PHY_COPPER_LINK                  0x0400

/* Cooper Specific Control Register 3 (page 0, Reg 26) */
#define PHY_P0_R26_DTE_DETECT            0x0100
#define PHY_P0_R26_DTE_STATUS_DROP_5S    0x0010
#define PHY_P0_R26_DTE_STATUS_DROP_MSK   0x00F0
#define PHY_P0_R26_CLASS_A               0x9000
#define PHY_P0_R17_DTE_NEED_POWER        0x0004

/* MAC specific control register 2 (page 2, reg 21) */
#define PHY_MAC_SPD_MASK                 0x7
#define PHY_MAC_SPD_1000M                0x6
#define PHY_MAC_SPD_100M                 0x5
#define PHY_MAC_SPD_10M                  0x4

/* QSGMII specific status register (page 4, reg 17) */
#define PHY_SYNC                         0x0020
#define PHY_LINK_SPEED_1000              0x8000
#define PHY_LINK_SPEED_100              0x4000
#define PHY_LINK_SPEED_10              0x0000
#define PHY_LINK_SPEED_MASK              0xc000
#define PHY_FULL_DUPLEX                  0x2000
 
/* QSGMII Interrupt Enable/Status Register (page 4, reg 18/19) */
#define PHY_QSGMII_SPEED_CHANGED         0x4000
#define PHY_QSGMII_FORCE_PIN             0x8000
#define PHY_QSGMII_LINK_STATUS_CHANGED   0x400

#define MRVL_PHONE_DETECT_TIME           5

/* check control (page 6, reg 18) */
#define PHY_ENA_STUB_TEST                0x8

#define PHY_INTR_DELAY                   500


/* */
#define E1680_REG_PAGE0    0
#define E1680_REG_PAGE1    1
#define E1680_REG_PAGE2    2
#define E1680_REG_PAGE3    3


/* ESW LED Control(page 3, reg 16) register */
#define E1680_LED_CONTR_REG     16
#define E1680_LCR_LED1_F_ON     (0x9 << 4)
#define E1680_LCR_LED0_F_ON     0x9
#define E1680_LCR_LED1_F_OFF    (0x8 << 4)
#define E1680_LCR_LED0_F_OFF    0x8
#define E1680_LCR_LED1_DEFAULT    (0x1 << 4)
#define E1680_LCR_LED0_DEFAULT    0x1

/* Led Func Ctrl (page 3, reg 16) */
#define PHY88E1680_LED_FUNC_CTRL_REG    16

typedef struct dev_88e1680_callin_fvt_t_ {

    int (*phy_start_mad_driver)(dev_object_t *, MAD_DEV *, int);   /* Load MAD dev driver. */
    int (*phy_config)(dev_object_t *, MAD_DEV *, int);   /* phy config. */
    int (*start_mac_lpbk)(dev_object_t *, MAD_DEV *, int, MAD_SPEED_MODE);   /* Start mac loopback */
    int (*start_ext_lpbk)(dev_object_t *, MAD_DEV *, int, MAD_SPEED_MODE);   /* Start ext loopback */
    int (*phy_force_speed)(dev_object_t *, MAD_DEV *, int, MAD_SPEED_MODE);   /* PHY config speed */
    int (*phy_register_tests)(dev_object_t *, MAD_DEV *, MAD_LPORT, MAD_U16, const reg_info_t *);   /* phy register test */
    int (*phy_reg_test_single)(dev_object_t *, MAD_DEV *, MAD_LPORT);   /* phy register test single */
    int (*phy_reg_test)(dev_object_t *, MAD_DEV *, uint);   /* phy register test */
    int (*phy_detect_phone)(dev_object_t *, MAD_DEV *, int);   /* phy detect phone */
    int (*print_phy_counter)(dev_object_t *, MAD_DEV *, uint);   /* print phy counter */
    int (*clear_phy_counter)(dev_object_t *, MAD_DEV *, uint);   /* clear phy counter */
    int (*dump_phy_reg)(dev_object_t *, MAD_DEV *, int);   /* dump phy register */
    int (*reset_phy)(dev_object_t *,MAD_DEV *);   /* reset phy */
    //int (*read_phy_reg_util)(dev_object_t *,MAD_DEV *, int);   /* read phy reg utility */
    int (*read_phy_reg_util)(dev_object_t *, MAD_DEV *, uint, uint, uint, uint *);
    int (*write_phy_reg_util)(dev_object_t *, MAD_DEV *, uint, uint, uint, uint);   /* write phy reg utility */
    int (*phy_intr_test)(dev_object_t *,MAD_DEV *, int);   /* phy interrupt test */
    int (*led_on)(dev_object_t *,MAD_DEV *, uint);   /* phy led on */
    int (*led_off)(dev_object_t *,MAD_DEV *, uint);   /* phy led off */
    int (*led_default)(dev_object_t *,MAD_DEV *, uint);   /* phy led default */
    int (*gen_int)(dev_object_t *,MAD_DEV *, uint);   /* generate interrupt */
    int (*clear_int)(dev_object_t *,MAD_DEV *, uint);   /* clear interrupt */
    int (*set_test_mode)(dev_object_t *, MAD_DEV *, uint, uint);   /* set test mode */
    
    	
} dev_88e1680_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                 platform
 */
typedef struct dev_88e1680_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    int (*rd)(int, ushort *);
    int (*wr)(int, ushort);
    int (*sgmii_lpbk_test)(void);
    int (*chk_intr_assert) (void);
    int (*chk_intr_deassert) (void);
    int (*cpss_driver_init) (void);
    int (*cpss_device_init) (void);
    int (*reg_pci_rd)(uint32_t, uint32_t, uint32_t *);
    int (*reg_pci_wr)(uint32_t, uint32_t, uint32_t);
    int (*led_init) (void);
    int (*smi_phy_init) (void);
    int(*phy_read_reg)(MAD_DEV *, MAD_LPORT, MAD_U16, MAD_U16, MAD_U32 *);
    int(*phy_write_reg)(MAD_DEV *, MAD_LPORT, MAD_U16, MAD_U16, MAD_U16);
    int(*phy_mad_load_driver)(MAD_DEV *, int);
    int(*phy_mad_unload_driver)(MAD_DEV *);
    int(*phy_mad_disable_int)(MAD_DEV *);
    int(*phy_mad_display_reg)(MAD_DEV *, MAD_U8, MAD_U16);
    int(*phy_mad_soft_reset)(MAD_DEV *, MAD_LPORT);
    int(*phy_mad_set_test_mode)(MAD_DEV *, uint, uint);
    int(*phy_mad_set_phy_enable)(MAD_DEV *, MAD_LPORT, int);
    int(*phy_mad_hw_page_reset)(MAD_DEV *, MAD_U8, MAD_U16);
    int(*phy_reset_api)(int);

} dev_88e1680_callout_fvt_t;

typedef enum {
    DEV_88E1680_DEV_STATE = 0,
    DEV_88E1680_ATTACH,
    DEV_88E1680_DETACH,
    DEV_88E1680_INIT,
    DEV_88E1680_SHOW,
    DEV_88E1680_DESTROY,
    DEV_88E1680_ALTER,
    DEV_88E1680_ALERT,
    DEV_88E1680_DISPLAY,
} dev_88e1680_report_code_t;

typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_88e1680_phy_setup_t;

/*
 * Define the 88E1680 device object structure
 */
typedef struct dev_88e1680_object_t {
    dev_object_t        base;
    dev_88e1680_callin_fvt_t        *callin_fvt;
    dev_88e1680_callout_fvt_t       *callout_fvt;
} dev_88e1680_object_t;


enum {
    DEV_88E1680_DISABLE,
    DEV_88E1680_ENABLE
};

extern void mrv88e1680_dev_create(dev_object_t *, dev_error_report_t);

#endif   /* __DEV_88E1680_H__ */


/******** History ******** 
 *
 *$Endlog$
*/
