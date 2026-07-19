/* $Id: dev_88e6390.h,v 1.2 2019/01/10 06:19:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e6390_marvell/dev_88e6390.h,v $
 *------------------------------------------------------------------
 *
 * dev_88e6390.h
 *
 * Description:	Marvell 88E6390 Device Driver
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_88E6390_H__
#define __DEV_88E6390_H__

#include "dev_object.h"

/* Common */
#define REG_PAGE(x)  (x)
#define REG_ADDR(x)  (x)
#define MRV88E6390_ERR_BUF_SIZE    (80)
#define SEC_TO_MICROSEC            1000000
#define ESW_MAX_POLLINGTIME_USEC   5000000   /* 5sec */
#define ESW_ACCESS_WAITTIME        20
#define ESW_PHY_RST_TIMEOUT        1000
#define MAX_POLLINGTIME_USEC       10000000   /* 10sec */
#define SMI_BUS_PREPARE            5 /* 5ms */
#define MAX_POLLING_ROUND          2000
#define INTR_POLLING_PERIOD        50 /* 50ms */
#define INTR_POLLING_ROUND         1000

#define COMPARE_AND     0 /* read data & pattern == 0       */
#define COMPARE_EQL     1 /* read data           == pattern */
#define COMPARE_AND_EQL 2 /* read data & pattern == pattern */

#define SPD_10MBPS     10
#define SPD_100MBPS    100
#define SPD_1000MBPS   1000

#define ESW_PORT0       0
#define ESW_PORT1       1
#define ESW_PORT2       2
#define ESW_PORT3       3
#define ESW_PORT4       4
#define ESW_PORT5       5
#define ESW_PORT6       6
#define ESW_PORT7       7
#define ESW_PORT8       8
#define ESW_PORT9       9
#define ESW_PORT10      10

/* SERDES Port, Device and Register*/
#define ESW_SERDES_PORT9   9

#define ESW_VLAN1       1
#define ESW_VLAN2       2
#define ESW_VLAN3       3
#define ESW_VLAN4       4

#define ESW_TESTMODE_NORMAL   0
#define ESW_TESTMODE1         1
#define ESW_TESTMODE2         2
#define ESW_TESTMODE3         3
#define ESW_TESTMODE4         4
#define ESW_TESTMODE1_REG_VAL 0x3F00
#define ESW_TESTMODE2_REG_VAL 0x5F00
#define ESW_TESTMODE3_REG_VAL 0x7700
#define ESW_TESTMODE4_REG_VAL 0x9F00


#define MRVL88E6390_MCA_SMI_CMD_REG    0x0
#define MRVL88E6390_MCA_SMI_DATA_REG   0x1

#define ESW_SMI_CMD_REG    MRVL88E6390_MCA_SMI_CMD_REG
#define ESW_SMI_DATA_REG   MRVL88E6390_MCA_SMI_DATA_REG


#define ALL_ESW_LEDS         0xF
#define ESW_LED_F_ON         1
#define ESW_LED_F_OFF        0 

#define ESW_RESET_ONE_SEC    1000   /* 1sec = 1000ms */
#define SMIOP_TIMEOUT_CTR    200
#define SMI_WAIT_CUNTR       200

/* Multi Chip Addressing mode */
/* SMI command register(0x0) */
#define SMI_CMD_SMIBUSY       (1 << 15)
#define SMI_CMD_SMIMODE_C22   (1 << 12)
#define SMI_CMD_SMIMODE_C45   (0 << 12)
#define SMI_CMD_SMIOP_RD      (1 << 11)   /* 11:10 0x2 Read Data Reg. */
#define SMI_CMD_SMIOP_WR      (1 << 10)   /* 11:10 0x1 Write Data Reg. */
#define SMIOP_C45_WR_ADDR     (0 << 10)   /* [11:10] C45 0x0 Write Addr. Reg. */
#define SMIOP_C45_WR_DATA     (1 << 10)   /* [11:10] C45 0x1 Write Data Reg. */
#define SMIOP_C45_RD_DATA_PI  (2 << 10)   /* [11:10] C45 0x2 Read Data Reg. with post increament on Addr. Reg. */
#define SMIOP_C45_RD_DATA     (3 << 10)   /* [11:10] C45 0x3 Read Data Reg. */

/* ESW SMI Device Register MAP */
/* Device Addr. */
#define ESW_SMIDEV_GLOB1      0x1B
#define ESW_SMIDEV_GLOB2      0x1C

#define ESW_PHYCTR_REG        0x1
#define ESW_PORTCTR_REG       0x4
#define ESW_PORT_VLAN_REG     0x6
#define ESW_PORTVLAN_ID_REG   0x7
#define ESW_PORTCTR2_REG      0x8

#define ESW_PCR_PSTAT_MSK     0x3
#define ESW_PCR_DISABLE       0x0
#define ESW_PCR_BLOCKING      0x1
#define ESW_PCR_LEARNING      0x2
#define ESW_PCR_FORWARD       0x3

/* ESW Port Status Reg.(0x0) */
#define ESW_PSR_ADDR             0x0
#define ESW_PSR_LINKUP           (1 << 11)
#define ESW_PSR_LINK             (1 << 11)
#define ESW_PSR_FULLDPX          (1 << 10)
#define ESW_PSR_DPX              (1 << 10)
#define ESW_PSR_SPD_MSK          0x0300
#define ESW_PSR_10MBPS           0x0000
#define ESW_PSR_100MBPS          0x0100
#define ESW_PSR_1000MBPS         0x0200
#define ESW_PSR_10GBPS           0x0300

/* ESW Physical Control Reg.(0x1) */
#define ESW_PCR_ADDR             0x1
#define ESW_PCR_RGMII_RX_DELAY   (1 << 15)
#define ESW_PCR_RGMII_TX_DELAY   (1 << 14)
#define ESW_PCR_FORCE_SPEED      (1 << 13)
#define ESW_PCR_MII_MAC_MODE     (0 << 11)
#define ESW_PCR_MII_PHY_MODE     (1 << 11)
#define ESW_PCR_F_LINKUP         (1 << 5)
#define ESW_PCR_FORCE_LINK       (1 << 4)
#define ESW_PCR_F_FULLDPX        (1 << 3)
#define ESW_PCR_FORCE_DPX        (1 << 2)
#define ESW_PCR_10MBPS           0x0
#define ESW_PCR_100MBPS          0x1
#define ESW_PCR_1000MBPS         0x2
#define ESW_PCR_10GBPS           0x3

/* ESW Port Control Reg.(0x4) */
#define ESW_PORT_CTL_REG_OFFSET  0x4
#define ESW_PCR_PS_MSK           0x3
#define ESW_PCR_PORT_DIS         0x0
#define ESW_PCR_PORT_FORWARD     0x3

/* ESW Port Based VLAN Map Reg.(0x6) */
#define ESW_PBVM_VLAN_TBL_MSK    0x7F
#define ESW_PBVM_VLAN_TBL(x)     (1 << x)

/* ESW Default Port VLAN ID & Priority */
#define ESW_PVID_FORCE_DVID      (1 << 12)
#define ESW_PVID_DVID_MSK        0xFFF

/* ESW Port Control 2 Reg.(0x08) */
#define ESW_PCR2_8021Q_MODE_MSK  (0x3 << 10)
#define ESW_PCR2_8021Q_SECURE    (0x3 << 10)

/* Global 2(0x1C) SMI register addr. */
#define ESW_GLOB2_PC          0x18   /* SMI PHY Command */
#define ESW_GLOB2_PD          0x19   /* SMI PHY Data */

/* ESW GE PHY Register Map */
#define ESW_GEPHY_PAGE_ADDR   22

/* ESW Copper Control register Map */
#define ESW_CCR_PWRDWN        (1 << 11)

/* ESW LED Control(0x16) register */
#define ESW_LED_CONTR_REG     0x16
#define ESW_LCR_UPDATE        (1 << 15)
#define ESW_LCR_LED1_F_ON     (0xF << 4)
#define ESW_LCR_LED0_F_ON     0xF
#define ESW_LCR_LED1_F_OFF    (0xE << 4)
#define ESW_LCR_LED0_F_OFF    0xE

/* ESW SGMII register Map */
#define ESW_SGMII_DEVNUM      4
#define ESW_SGMII_CONTR_REG   0x2000
#define ESW_SGMII_PWRDWN      (1 << 11)

/* ESW Global 1 Reg. Map */
#define ESW_G1_VTUFID_REG            0x2
#define ESW_G1_VTUOP_REG             0x5
#define ESW_G1_VTUVID_REG            0x6
#define ESW_G1_VTUDATA_0TO7_REG      0x7
#define ESW_G1_VTUDATA_8TO10_REG     0x8

#define ESW_G1_VID_ENTRY_VALID       (1 << 12)

#define ESW_G1_DATA_FRAME_UNTAGGED   0x1
#define ESW_G1_MEMBER_STATE_MSK      0x3

#define ESW_G1_OP_VTUBUSY            (1 << 15)
#define ESW_G1_OP_VTULOAD            (3 << 12)

/* ESW PHY */
#define ESW_ALL_PHY_PORTS            0xA

#define ESW_SET_PORT_10M             0x1
#define ESW_SET_PORT_100M            0x2
#define ESW_SET_PORT_1G              0x3

#define ESW_SET_PORT_HD              0x0
#define ESW_SET_PORT_FD              0x1
#define ESW_ENABLE_PORT              0x7F
#define ESW_DISABLE_PORT             0x7C

/* Marvell phy register number and bit mask
 */
#define PHY_REG(x) (x)
#define PHY_PAGE(x) (x)
#define PHY_REG_BIT(x) (1 << (x))

/* Copper Control Reg. (0_0) */
#define ESWPHY_CCR_ADDR              0x0
#define ESWPHY_CCR_COP_RST           (1 << 15)
#define ESWPHY_CCR_LPBK              (1 << 14)
#define ESWPHY_CCR_AN_EN             (1 << 12)
#define ESWPHY_CCR_DUPLEX_FULL       (1 << 8)

#define COP_SPD_MASK            0x2040
#define COP_SPD_10Mbps          0x0000
#define COP_SPD_100Mbps         0x2000
#define COP_SPD_1000Mbps        0x0040

/* Copper Auto-Negotitation Advertisement Reg. (4_0) */
#define ESWPHY_CANAR_ADDR            0x4
#define ESWPHY_COP_ANAR_100FD        (1 << 8)
#define ESWPHY_COP_ANAR_100HD        (1 << 7)
#define ESWPHY_COP_ANAR_10FD         (1 << 6)
#define ESWPHY_COP_ANAR_10HD         (1 << 5)

/* 1000BASE-T Control Reg. (9_0) */
#define ESWPHY_1000TCR_ADDR          0x9
#define ONEK_CNTL_TESTMODE_SHIFT     13
#define ONEK_CNTL_TESTMODE_MASK      (7 << 13)   /* 0xE000 */
#define ESWPHY_1G_CNTR_1000FD        (1 << 9)
#define ESWPHY_1G_CNTR_1000HD        (1 << 8)

/* Copper Specific Status Reg1 (17_0) */
#define ESWPHY_CSSR1_SPEED           (3 << 14)
#define ESWPHY_CSSR1_DUPLEX          (1 << 13)
#define ESWPHY_CSSR1_RESOLVED        (1 << 11)
#define ESWPHY_CSSR1_COP_LINK        (1 << 10)
#define ESWPHY_CSSR1_LINK_STAT       (1 << 3)

#define ESWPHY_CSSR1_SPD_1000MBPS    0x8000
#define ESWPHY_CSSR1_SPD_100MBPS     0x4000
#define ESWPHY_CSSR1_SPD_10MBPS      0x0000
#define ESWPHY_CSSR1_FULLDUP         0x2000
#define ESWPHY_CSSR1_RT_LINK_UP      0x0400
#define ESWPHY_CSSR1_COP_LINK_UP     0x0008

/* Copper Specific Interrupt Enable Register (18_0) */
#define ENABLE_ALL_INT_PHY_REG         0xFFFF
#define DISABLE_ALL_INT_PHY_REG        0x0
#define COP_SPEC_INT_EN_REG            18

/* Checker Control (18_6) */
#define CHECKER_CONTROL_OFFSET       18
#define ESW_ENABLE_PHY_STUB          0x8
#define ESW_DISABLE_PHY_STUB         0

/* MAC Specific Control Reg2 (21_2) */
#define ESWPHY_MSCR2_MACSPD_MSK        0x7   /* 21_2.2:0 */
#define ESWPHY_MSCR2_MACSPD_10MBPS     0x4   /* 10 MBPS  (21_2.2:0 = 100) */
#define ESWPHY_MSCR2_MACSPD_100MBPS    0x5   /* 100 MBPS (21_2.2:0 = 101) */
#define ESWPHY_MSCR2_MACSPD_1000MBPS   0x6   /* 1000 MBPS(21_2.2:0 = 110) */

/* Misc Test Reg.(26_6) */
#define MRV88E6390_PAGE4_REG27    27
#define GEPHY_MISC_TEST_REG    26
#define MTR_TX_TCLK_EN         (1 << 15)

/* Interrupt REG*/
#define ESW_GLOBAL1_REG                0x1B
#define ESW_GLOBAL2_REG                0x1C
#define ESW_GLOBAL_CONTROL_REG         0x04
#define ESW_INT_MASK_REG               0x01
#define ENABLE_ESW_GLOBAL_CONTROL_REG  0x41FF
#define ENABLE_ESW_INT_MASK_REG        0xF81F
#define IMP_COMM_DBG_REG               0x13 /* belong to Global 2(0x1c) */
#define EEPROM_CMD_REG                 0x14 /* belong to Global 2(0x1c) */
#define EEBUSY                         0x8000 /* EEPROM_CMD_REG bit[15] = 1*/


#define COP_AUTONEG_ADV_REG4  4
#define COP_STATUS_REG17      17

/* Copper specific status register(17) */
#define COP_P0R17_SPEED           (3 << 14)
#define COP_P0R17_SPEED_OFFSET    14
#define COP_P0R17_SPEED_1000      0x2
#define COP_P0R17_SPEED_100       0x1
#define COP_P0R17_SPEED_10        0x0
#define COP_P0R17_DUPLEX_FULL     (1 << 13)
#define COP_P0R17_COP_LINK_UP     (1 << 10)
#define COP_P0R17_GLOBAL_LINK_UP  (1 << 3)

typedef struct dev_88e6390_callin_fvt_t_ {
    int (*register_test)(dev_object_t *);                                 /* [Device Test Function] Register Test */
    int (*esw_phy_mac_lpbk_test)(dev_object_t *, int, int);               /* [Device Test Function] MAC Loopback Test */
    int (*ext_lpbk_test)(dev_object_t *, int, int);                       /* [Device Test Function] External Loopback Test */
    int (*intr_test)(dev_object_t *);                                     /* [Device Test Function] Interrupt Test */
    int (*esw_reg_read_util)(dev_object_t *);                             /* Ethernet Switch reg Read*/
    int (*esw_reg_write_util)(dev_object_t *);                            /* Ethernet Switch reg Write*/
    int (*esw_phy_reg_read_util)(dev_object_t *);                         /* Ethernet Switch phy reg Read*/
    int (*esw_phy_reg_write_util)(dev_object_t *);                        /* Ethernet Switch phy reg Write*/
    int (*esw_c45_phy_reg_read_util)(dev_object_t *);                     /* Ethernet Switch Clause 45 phy reg Read*/
    int (*esw_c45_phy_reg_write_util)(dev_object_t *);                    /* Ethernet Switch Clause 45 phy reg Write*/
    int (*esw_reg_read)(dev_object_t *, int, int, ushort *);              /* Ethernet Switch reg Read*/
    int (*esw_reg_write)(dev_object_t *, int, int, ushort);               /* Ethernet Switch reg Write*/
    int (*esw_phy_reg_read)(dev_object_t *, int, int, int, ushort *);     /* Ethernet Switch phy reg Read*/
    int (*esw_phy_reg_write)(dev_object_t *, int, int,  int, ushort);     /* Ethernet Switch phy reg Write*/
    int (*esw_c45_phy_reg_read)(dev_object_t *, int, int, int, ushort *); /* Ethernet Switch Clause 45 phy reg Read*/
    int (*esw_c45_phy_reg_write)(dev_object_t *, int, int,  int, ushort); /* Ethernet Switch Clause 45 phy reg Write*/
    int (*esw_pwr_up_ge_port)(dev_object_t *, int);                       /* Power up ge port */
    int (*esw_pwr_up_serdes_port)(dev_object_t *);                        /* Power up serdes port */
    int (*esw_set_port_forward)(dev_object_t *, int);                     /* Set port forward */
    int (*esw_enable_phy_port_interrupt)(dev_object_t *, int, boolean);   /* Enable phy port interrupt */
    int (*esw_enable_int_mask_and_reg)(dev_object_t *);                   /* Enable interrupt for interrupt mask*/
    int (*esw_config_pvlan)(dev_object_t *, int);                         /* Configures Port-based VLAN */
    int (*esw_set_led_on)(dev_object_t *, int);                           /* Set port led on */
    int (*esw_set_led_off)(dev_object_t *, int);                          /* Set port led off */
    int (*esw_set_testmode_util)(dev_object_t *);                         /* Set IEEE test mode */
    int (*esw_set_vod_util)(dev_object_t *);                              /* Set Output Voltage */
} dev_88e6390_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                 platform
 */
typedef struct dev_88e6390_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    int (*rd)(int, ushort *);
    int (*wr)(int, ushort);
    int (*esw_phy_tx_rx_test)(void);
    int (*reset) (void);
    int (*chk_intr_assert) (void);
    int (*chk_intr_deassert) (void);
} dev_88e6390_callout_fvt_t;

typedef enum {
    DEV_88E6390_DEV_STATE = 0,
    DEV_88E6390_ATTACH,
    DEV_88E6390_DETACH,
    DEV_88E6390_INIT,
    DEV_88E6390_SHOW,
    DEV_88E6390_DESTROY,
    DEV_88E6390_ALTER,
    DEV_88E6390_ALERT,
    DEV_88E6390_DISPLAY,
} dev_88e6390_report_code_t;

typedef enum {
    DEV_88E6390_VLAN_PROFILE_1,
    DEV_88E6390_VLAN_PROFILE_END
} dev_88e6390_vlan_profile_t;

typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_88e6390_phy_setup_t;

/*
 * Define the 88E6390 device object structure
 */
typedef struct dev_88e6390_object_t {
    dev_object_t        base;
    dev_88e6390_callin_fvt_t        *callin_fvt;
    dev_88e6390_callout_fvt_t       *callout_fvt;
} dev_88e6390_object_t;

typedef struct dev_88e6390_pvlan_profile_t_ {
    int port_vtable[ESW_PORT7];
} dev_88e6390_pvlan_profile_t;

extern void mrv88e6390_dev_create(dev_object_t *, dev_error_report_t);
#endif   /* __DEV_88E6390_H__ */

/*-------------------------------------------------
 * $Log: dev_88e6390.h,v $
 * Revision 1.2  2019/01/10 06:19:23  wilbhuan
 * The beginning of Marvell 88E6390 Ethernet Switch PHY device driver.
 *
 *-------------------------------------------------
 */
