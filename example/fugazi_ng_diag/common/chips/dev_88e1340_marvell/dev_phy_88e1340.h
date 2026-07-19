/* $Id: dev_phy_88e1340.h,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e1340_marvell/dev_phy_88e1340.h,v $
 * -----------------------------------------------------------------------------
 * File Name: dev_phy_88e1340.h
 *
 * Description: Contains common definitions for the Marvell 88E1340 Quad-PHY.
 *      
 * James Lin - January 2010.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DEV_PHY_88E1340_H__
#define __DEV_PHY_88E1340_H__

#include "dev_object.h"

/************************* 88E1340 ************************/
#define MRV88E1340_REG_PAGE_0                   0
#define MRV88E1340_REG_PAGE_1                   1
#define MRV88E1340_REG_PAGE_2                   2
#define MRV88E1340_REG_PAGE_3                   3
#define MRV88E1340_REG_PAGE_4                   4
#define MRV88E1340_REG_PAGE_5                   5
#define MRV88E1340_REG_PAGE_6                   6
#define MRV88E1340_REG_PAGE_7                   7
#define MRV88E1340_REG_PAGE_8                   8
#define MRV88E1340_REG_PAGE_9                   9
#define MRV88E1340_REG_PAGE_12                  12
#define MRV88E1340_REG_PAGE_14                  14
#define MRV88E1340_REG_PAGE_29                  29
#define MRV88E1340_REG_PAGE_254                 254
#define MRV88E1340_REG_PAGE_MAX                 256

/* Page 0 Register Offsets - Copper */
#define MRV88E1340_CONTROL_REG                  0
#define MRV88E1340_STATUS_REG                   1
#define MRV88E1340_PHY_ID1                      2
#define MRV88E1340_PHY_ID2                      3
#define MRV88E1340_AUTONEG_ADVR_REG             4
#define MRV88E1340_LINK_PART_AV_REG             5
#define MRV88E1340_AUTONEG_EXPANSION_REG        6
#define MRV88E1340_NEXT_PAGE_REG                7
#define MRV88E1340_LP_NEXT_PAGE_REG             8
#define MRV88E1340_1000B_CNTL_REG               9                              
#define MRV88E1340_1000B_STATUS_REG             10
#define MRV88E1340_EXTENDED_STATUS_REG          15
#define MRV88E1340_SPECIFIC_CONTROL1_REG        16
#define MRV88E1340_SPECIFIC_STATUS1_REG         17
#define MRV88E1340_INT_ENABLE_REG               18
#define MRV88E1340_SPECIFIC_STATUS2_REG         19
#define MRV88E1340_SPECIFIC_CONTROL2_REG        20
#define MRV88E1340_REC_ERROR_COUNTER_REG        21
#define MRV88E1340_PAGE_ADDRESS_REG             22
#define MRV88E1340_GLOBAL_INTERRUPT_REG         23
#define MRV88E1340_SPECIFIC_CONTROL3_REG        26

/* Copper Control Register (Page 0, Reg 0) */
#define MRV88E1340_COOPER_RST                   0x8000
#define MRV88E1340_LPBK_ENA                     0x4000
#define MRV88E1340_SPD_SEL_MASK                 0x2040
#define MRV88E1340_SPD_SEL_1000M                0x0040
#define MRV88E1340_SPD_SEL_100M                 0x2000
#define MRV88E1340_SPD_SEL_10M                  0x0000
#define MRV88E1340_AUTO_NEO_ENA                 0x1000
#define MRV88E1340_PWR_DOWN                     0x0800
#define MRV88E1340_RST_AUTO_NEO                 0x0200
#define MRV88E1340_FULL_DUPLEX                  0x0100


#define MRV88E1340_SPEED_MASK                   0x0007
#define MRV88E1340_SGMII_SPD_1000               0x0006
#define MRV88E1340_SGMII_SPD_100                0x0005
#define MRV88E1340_SGMII_SPD_10                 0x0004

#define MRV88E1340_LPBK_ENA                     0x4000
#define MRV88E1340_AUTO_NEG                     0x1000
#define MRV88E1340_PWR_DOWN                     0x0800


#define MRV88E1340_PHY_OUI_HI                   0x0141  
#define MRV88E1340_PHY_OUI_LO                   0x03
#define MRV88E1340_PHY_OUI_LO_MASK              0xFC00

/* Copper Auto-Nego Advertisement Register (Page 0, Reg 4) */
#define MRV88E1340_10BT_ADV                     0x60    
#define MRV88E1340_100BT_ADV                    0x180
#define MRV88E1340_1000BT_ADV                   0x300

/* 1000BASE-T Control Register (offset 9) Bits defines */
#define PHY_GT_CTL_TEST_MASK    0xE000  /* Test Modes */
#define PHY_GT_CTL_NORMAL       0x0000  /* Normal mode */
#define PHY_GT_CTL_WV_TEST      0x2000  /* Test mode 1 - Waveform test */
#define PHY_GT_CTL_JT_MS        0x4000  /* Test mode 2 - Jitter (Master) */
#define PHY_GT_CTL_JT_SL        0x6000  /* Test mode 3 - Jitter (Slave) */
#define PHY_GT_CTL_DS_TEST      0x8000  /* Test mode 4 - Distortion test */
#define PHY_GT_CTL_TEST_MAX     PHY_GT_CTL_DS_TEST
#define PHY_GT_CTL_TEST_SHIFT   13      /* Test mode bits shift counts */

/* Copper Specific Control Register (Page 0, Reg 16) */
#define MRV88E1340_CNTL_FORCE_LINK              0x0400
#define MRV88E1340_P0_R16_PWR_DOWN              0x0004

/* Copper Specific Control Register 1 (Page 0, Reg 17) */
#define MRV88E1340_P0_R17_DTE_NEED_POWER        0x0004
#define MRV88E1340_LINK_SPEED_MASK              0xC000
#define MRV88E1340_LINK_SPEED_1000              0x8000
#define MRV88E1340_LINK_SPEED_100               0x4000
#define MRV88E1340_LINK_SPEED_10                0x0000
#define MRV88E1340_LINK_UP                      0x0400
#define MRV88E1340_SYNC                         0x0020

/* Cooper Specific Control Register 3 (page 0, Reg 26) */
#define MRV88E1340_P0_R26_DTE_DETECT            0x0100
#define MRV88E1340_P0_R26_DTE_STATUS_DROP_5S    0x0010
#define MRV88E1340_P0_R26_DTE_STATUS_DROP_MSK   0x00F0
#define MRV88E1340_P0_R26_CLASS_A               0x9000

/* Page 1 Register Offsets - Fiber */
#define MRV88E1340_P1_CONTROL_REG               0

/* Fober Control Register (Page 1, Reg 0) */
#define MRV88E1340_P1_R0_PWR_DOWN               0x0800
#define MRV88E1340_P1_R17_SYNC                  0x0020

/* Page 2 Register Offsets - MAC */
#define MRV88E1340_MAC_CNTL_REG2                21
#define MRV88E1340_MAC_SPD_MASK                 0x7
#define MRV88E1340_MAC_SPD_1000M                0x6
#define MRV88E1340_MAC_SPD_100M                 0x5
#define MRV88E1340_MAC_SPD_10M                  0x4

/* Page 6 Register Offsets */
#define MRV88E1340_P6_CHECKER_CTRL              18
#define MRV88E1340_P6_MISC_TEST                 26

/* Checker Control Register (Page 6, Reg 18) */
#define MRV88E1340_P6_R18_ENA_STUB_TEST         0x0008

/* MISC Test Register (Page 6, Reg 26) */
#define MRV88E1340_P6_R26_TEMP_THRESHOLD_MASK   0x1F00
#define MRV88E1340_P6_R26_ENA_TEMP_SENSOR_INTR  0x0080

#define MRV88E1340_P6_R26_TEMP_NEG_25C          0x0000
#define MRV88E1340_P6_R26_TEMP_0C               0x0500
#define MRV88E1340_P6_R26_TEMP_100C             0x1900

/* Page 29 Register Offsets */
#define MRV88E1340_REG_30                       30

/* Page 254 Register Offsets */
#define MRV88E1340_REG_24                       24
#define MRV88E1340_REG_26                       26


/*
 * General Defines
 */

#define MRV88E1340_PORTS                        4
#define PHY_LOG_BUF_SIZE                        384
#define MRVL_PHONE_DETECT_TIME                  5
#define MRV88E1340_ERR_MSG_LEN                  500
#define MRV88E1340_PHY_ADDR_DECR                0  /* decremental phy addr */
#define MRV88E1340_PHY_ADDR_INCR                1  /* incremental phy addr */


#define SMIREAD(d, x, y, z, u)           ((dev_88e1340_object_t *)(d))->  \
                                         callout_fvt->smi_read            \
                                         ((uint)(x), (uint)(y), (uint)(z),\
                                          (uint *)(u))
#define SMIWRITE(d, x, y, z, u)          ((dev_88e1340_object_t *)(d))->  \
                                         callout_fvt->smi_write           \
                                         ((uint)(x), (uint)(y), (uint)(z),\
                                          (uint)(u))

#define MVL_88E1340_REG_TEST(d)          ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->register_test        \
                                         ((dev_object_t *)(d))

#define MVL_88E1340_REG_TEST_SINGLE(d,e) ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->register_test_single \
                                         ((dev_object_t *)(d), (uint)e)

#define MVL_88E1340_SET_LPBK(d, x, y, z) ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->set_loopback         \
                                         ((dev_object_t *)(d), (int)(x),  \
                                          (int)(y), (int)(z))

#define MVL_88E1340_LPBK_MODE(d, x, y)   ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->lpbk_mode            \
                                         ((dev_object_t *)(d), (int)(x),  \
                                          (int)(y))

#define MVL_88E1340_CLR_LPBK(d, x)       ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->cleanup_loopback     \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1340_PWR_UP(d, x, y)      ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->power_up             \
                                         ((dev_object_t *)(d), (uint)(x), \
                                          (uint)(y))

#define MVL_88E1340_INTR_GEN(d, x)       ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->intr_gen             \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1340_INTR_CLR(d, x)       ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->intr_clr             \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1340_LOCKUP_FIX(d, x)     ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->lockup_fix           \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1340_SHOW_REG(d, x, y)    ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->show_reg             \
                                         ((dev_object_t *)(d),            \
                                          (print_fn_t)(x), (uint)(y))

#define MVL_88E1340_ALTER_REG(d)         ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->alter_reg            \
                                         ((dev_object_t *)(d))

#define MVL_88E1340_PHONE_DETECT(d, x)   ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->phone_detect         \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1340_SET_TEST_MODE(d, x)  ((dev_88e1340_object_t *)(d))->  \
                                         callin_fvt->set_test_mode        \
                                         ((dev_object_t *)(d), (uint)(x))

enum
{
    ETH_MODE_GE,
    ETH_MODE_FE100,
    ETH_MODE_FE10,
};

/* define loopback mode */
enum
{
    SGMII_LPBK_NONE,           /* no loopback */
    SGMII_LPBK_MAC,            /* internal loopback at ppc etsec */
    SGMII_LPBK_PCS,            /* internal loopback at cavium pcs */
    SGMII_LPBK_QLM,            /* line loopback at cavium QLM */
    SGMII_SW_LPBK_INTERNAL,    /* internal loopback at marvell GE switch */
    SGMII_PHY_LPBK_INTERNAL,   /* internal loopback at marvell GE PHY */
};

typedef struct mrvl_88e1340_phy_regs_t_
{
    const char *pagename;
    uint32_t    pagenum;
    const reg_info_t *pageregs;
} mrvl_88e1340_phy_regs_t;


/*
 * dev_error_report message codes
 */
typedef enum {
    MRVL_88E1340_DEV_STATE = 0,
    MRVL_88E1340_ATTACH,
    MRVL_88E1340_DETACH,
    MRVL_88E1340_INIT,
    MRVL_88E1340_SHOW,
    MRVL_88E1340_DESTROY,
    MRVL_88E1340_REG_TEST,
    MRVL_88E1340_ALTER_REG,
    MRVL_88E1340_SET_LPBK,
    MRVL_88E1340_CLN_LPBK,
    MRVL_88E1340_POWER_UP,
    MRVL_88E1340_INTR_GEN,   
    MRVL_88E1340_INTR_CLR,
    MRVL_88E1340_LOCKUP_FIX,
    MRVL_88E1340_PHONE_DETECT,   
    MRVL_88E1340_SET_TEST_MODE,
}mrvl_88e1340_report_code_t;
 

/*
 * device callin function - service provided and defined by the device
 */
typedef struct dev_88e1340_callin_fvt_ {
    int  (*register_test)(dev_object_t *);
    int  (*register_test_single)(dev_object_t *, uint);
    int  (*show_reg)(dev_object_t *, print_fn_t, uint);
    int  (*alter_reg)(dev_object_t *);
    int  (*set_loopback)(dev_object_t *, int, int, int);
    int  (*lpbk_mode)(dev_object_t *, int, int);
    int  (*cleanup_loopback)(dev_object_t *, uint);
    int  (*power_up)(dev_object_t *, uint, uint);
    int  (*intr_gen)(dev_object_t *, uint);
    int  (*intr_clr)(dev_object_t *, uint);
    int  (*lockup_fix)(dev_object_t *, uint);
    int  (*phone_detect)(dev_object_t *, uint);
    int  (*set_test_mode)(dev_object_t *, uint);
} dev_88e1340_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct dev_88e1340_callout_fvt_ {
    int (*smi_read)(uint, uint, uint, uint *);
    int (*smi_write)(uint, uint, uint, uint);
} dev_88e1340_callout_fvt_t;

/*
 * Define the Marvell GE device object structure.
 */
typedef struct dev_88e1340_object_t_ {
    dev_object_t              base;
    dev_88e1340_callout_fvt_t *callout_fvt;
    dev_88e1340_callin_fvt_t  *callin_fvt;
    uint32_t                  addr_seq;
    int                       base_phyaddr;
} dev_88e1340_object_t;

/* Prototypes */
int dev_88e1340_create (dev_object_t *, dev_error_report_t);

#endif /* __DEV_PHY_88E1340_H__ */

/******** History ******** 
$Log: dev_phy_88e1340.h,v $
Revision 1.2  2013/10/08 08:48:25  tirawan
Woodlawn collapsed to main trunk

Revision 1.1.4.2  2013/08/20 10:58:48  tirawan
Branch into woodlawn-branch2 and port woodlawn code

Revision 1.1.2.1  2013/04/24 10:58:02  tirawan
First Woodlawn linux integration

Revision 1.2  2013/03/27 04:49:44  kuangik
Code cleanup after -Wall

Revision 1.1  2013/03/13 06:42:08  kuangik
Add for the first time

Revision 1.8  2013/03/08 09:35:30  kuangik
Add PHY base address member

Revision 1.7  2012/10/24 10:48:57  leslie
Fix and clean up code.

Revision 1.6  2012/10/08 10:01:15  leslie
Add mapping table.

Revision 1.5  2012/08/28 08:22:46  leslie
*** empty log message ***

Revision 1.4  2012/08/03 10:16:50  leslie
Mapping to latest O2 source code on 20120726

Revision 1.2  2012/04/06 06:15:21  kuangik
Add Single Register Test

Revision 1.1.1.1  2012/02/10 05:59:50  kody
Initial imports Woodlawn project code base.

Revision 1.1  2010/05/26 07:58:56  jamlin
Initial check-in for Firebee

$Endlog$
*/
