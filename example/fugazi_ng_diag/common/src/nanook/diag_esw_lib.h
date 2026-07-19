/* $Id: diag_esw_lib.h,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_esw_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_esw_lib.h
 * Description: Header file of Ethernet Switch Library
 * 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __DIAG_ESW_LIB_H__
#define __DIAG_ESW_LIB_H__


#include "dev_98dxc323.h"
#include "dev_88e1680.h"

#define pow pow
#define sin  sin
#define log10  log10
#define sqrt  sqrt

#define NANOOK_XCAT3_MAX_LED_INTERFACE    2

#define NANOOK_1680_GROUP_NUM    3
#define NANOOK_1680_GROUP_0_START_ADDR    0
#define NANOOK_1680_GROUP_1_START_ADDR    8
#define NANOOK_1680_GROUP_2_START_ADDR    16
#define NANOOK_1680_NUM_PHY_IN_ONE_GRUOP    8

#define NANOOK_ESW_PORT_NUM    24
#define NANOOK_ESW_SMI_PHY_ADDR    0
#define NANOOK_ESW_SMI0_PORT_START_NUM    0
#define NANOOK_ESW_SMI1_PORT_START_NUM    16
#define NANOOK_AC3_CPSS_DEV    0
#define NANOOK_88E1680_PHY_ADDR_BASE    0
#define NANOOK_XCAT_PCIE_BUS    0x03
#define NANOOK_XCAT3_USED_PORT    24

#define ETH_RM_AC3_NIM_DM_MODULE           "rmmod nim_dm &> /dev/null"
#define ETH_INSMOD_AC3_NIM_DM_MODULE   "insmod /lib/modules/4.14.3/nim_dm_nanook_AC3_20190507.ko &> /dev/null"



#define GE0_XCAT3_PORT        24
#define GE1_XCAT3_PORT        25

#define GE_XCAT3_PORT        24

#define XCAT3_PORT_00        0
#define XCAT3_PORT_01        1
#define XCAT3_PORT_02        2
#define XCAT3_PORT_03        3
#define XCAT3_PORT_04        4
#define XCAT3_PORT_05        5
#define XCAT3_PORT_06        6
#define XCAT3_PORT_07        7
#define XCAT3_PORT_08        8
#define XCAT3_PORT_09        9
#define XCAT3_PORT_10        10
#define XCAT3_PORT_11        11
#define XCAT3_PORT_12        12
#define XCAT3_PORT_13        13
#define XCAT3_PORT_14        14
#define XCAT3_PORT_15        15
#define XCAT3_PORT_16        16
#define XCAT3_PORT_17        17
#define XCAT3_PORT_18        18
#define XCAT3_PORT_19        19
#define XCAT3_PORT_20        20
#define XCAT3_PORT_21        21
#define XCAT3_PORT_22        22
#define XCAT3_PORT_23        23

#define ESW_PORT_00        0
#define ESW_PORT_01        1
#define ESW_PORT_02        2
#define ESW_PORT_03        3
#define ESW_PORT_04        4
#define ESW_PORT_05        5
#define ESW_PORT_06        6
#define ESW_PORT_07        7
#define ESW_PORT_08        8
#define ESW_PORT_09        9
#define ESW_PORT_10        10
#define ESW_PORT_11        11
#define ESW_PORT_12        12
#define ESW_PORT_13        13
#define ESW_PORT_14        14
#define ESW_PORT_15        15
#define ESW_PORT_16        16
#define ESW_PORT_17        17
#define ESW_PORT_18        18
#define ESW_PORT_19        19
#define ESW_PORT_20        20
#define ESW_PORT_21        21
#define ESW_PORT_22        22
#define ESW_PORT_23        23

#define XCAT3_PORT_NEIBORHOOD        1
     

#define GPIO_LED_CTRL                    32
#define GPIO_HIGH_DATA_OUT_ENA_REG       0x00010144
#define GPIO_HIGH_DATA_OUT_REG           0x00010140

/* define for SMI configuration */
#define PHY_ADDR_REG0_OFF                0x04004030
#define PHY_AUTO_NEG_REG0_OFF            0x04004034
#define SMI0_MANAGE_REG_OFF              0x04004054
#define LMS0_MISC_CONFIG_REG_OFF         0x07004200

#define PHY_ADDR_REG1_OFF                0x04804030
#define PHY_AUTO_NEG_REG1_OFF            0x04804034

#define PHY_ADDR_REG2_OFF                0x05004030
#define PHY_AUTO_NEG_REG2_OFF            0x05004034
#define SMI1_MANAGE_REG_OFF              0x05004054
#define LMS1_MISC_CONFIG_REG             0x05004200

#define PHY_ADDR_REG3_OFF                0x05804030
#define PHY_AUTO_NEG_REG3_OFF            0x05804034


#define LINKDOWN 0
#define LINKUP 1

#define VLAN_1                1
#define VLAN_2                2  
#define VLAN_3                3
#define VLAN_4                4
#define VLAN_5                5

/* Page 0, Reg 17 BIT10 and BIT3*/
#define COOPER_AND_GLOBAL_LINK_UP  0x408

/* Page 0, Reg 17 BIT15 and BIT14*/
#define SPEED_MASK  0xC000
#define SPEED_SHIFT_BIT  14

#define SPD_10MBPS          10
#define SPD_100MBPS         100
#define SPD_1000MBPS        1000

#define ESW_WAIT_10000MS   10000
#define ESW_WAIT_5000MS   5000
#define ESW_WAIT_2000MS   2000
#define ESW_WAIT_1000MS   1000
#define ESW_WAIT_500MS   500
#define ESW_WAIT_200MS   200
#define ESW_WAIT_100MS   100

#define ESW_PORT_PWR_UP   1
#define ESW_PORT_PWR_DOWN   0

typedef enum {
    DEV_ESW_SPD_10 = 0,
    DEV_ESW_SPD_100,
    DEV_ESW_SPD_1000
} dev_esw_speed_t;


 enum {
    XCAT3_10GKR_TEST_MODE_DISABLE =0,
    XCAT3_10GKR_TEST_MODE_PRBS09,
    XCAT3_10GKR_TEST_MODE_PRBS15,
    XCAT3_10GKR_TEST_MODE_PRBS31,
    XCAT3_10GKR_TEST_MODE_8180,
    XCAT3_10GKR_TEST_MODE_CUSTOMIZE
};   

 #define AC3_LANE_6    6

extern int diag_esw_init(void);
extern int diag_esw_exit(void);
extern int diag_reset_esw_to_default(int);
extern int diag_esw_98dxc323_dev_create(dev_98dxc323_object_t *);
extern int diag_phy_88e1680_dev_create (dev_88e1680_object_t *);
extern int diag_esw_ext_lpbk_test (void); 
extern int diag_config_port_speed (uint, uint, uint);
extern int diag_port_power_control(uint, uint, uint);
extern int diag_esw_all_phy_led_on (void);
extern int diag_esw_all_phy_led_off (void);


#endif  /* __DIAG_ESW_LIB_H__*/
  
  
/*-------------------------------------------------
 * $Log: diag_esw_lib.h,v $
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
