/* $Id: platform_led.h,v 1.1 2020/01/09 01:02:02 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_led.h,v $
 *--------------------------------------------------------------------
 * Filename: platform_led.h
 *
 * Description: Platform specific code for controlling the system LED
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __PLATFORM_LED_H__
#define __PLATFORM_LED_H__


/*
 *  LED Miscellaneous On/Off Register (+0x00) 
 */
#define MSK_VOICE_MODULE_LED             0x0003
#define VOICE_MODULE_LED_OFF             0x0000
#define VOICE_MODULE_LED_GREEN           0x0002
#define VOICE_MODULE_LED_YELLOW          0x0001
 
/*
 *  LED Compact Flash On/Off Register (+0x04)
 */
#define MSK_CF_ONOFF_LED               0x3FFFF
#define CF_LED_GREEN                   0x30000 /* bit [17:16] */
#define OFFSET_CF_LED_REG              16
#define CF_LED_YELLOW                  0x04000 /* bit 14 */
#define CF_LED_POLARITY_INVER          0x02000 /* bit 13 */
#define CF_LED_SIG_STATUS              0x01000 /* bit 12 */
#define OFFSET_CF_SIG_STATUS_REG       11
#define CF_LED_GREEN_FUNC              0x00C00 /* bit [11:10] */
#define CF_LED_GREEN_OFFSET            10 
#define CF_LED_TIMER                   0x003FF /* bit [9:0] */

/*
 *  LED Power Supply On/Off Register (+0x08)   
 */
#define MSK_PWR_SUPPLY_UNIT1_LED             0x0003
#define PWR_SUPPLY_UNIT1_LED_OFF             0x0000
#define PWR_SUPPLY_UNIT1_LED_GREEN           0x0002
#define PWR_SUPPLY_UNIT1_LED_YELLOW          0x0001

#define MSK_PWR_SUPPLY_UNIT2_LED             0x000C
#define PWR_SUPPLY_UNIT2_LED_OFF             0x0000
#define PWR_SUPPLY_UNIT2_LED_GREEN           0x0008
#define PWR_SUPPLY_UNIT2_LED_YELLOW          0x0004

/*
 *  LED POE Power Supply On/Off Register (+0x0C) 
 */
#define MSK_POE_PWR_SUPPLY_UNIT1_LED             0x0003
#define POE_PWR_SUPPLY_UNIT1_LED_OFF             0x0000
#define POE_PWR_SUPPLY_UNIT1_LED_GREEN           0x0002
#define POE_PWR_SUPPLY_UNIT1_LED_YELLOW          0x0001

#define MSK_POE_PWR_SUPPLY_UNIT2_LED             0x000C
#define POE_PWR_SUPPLY_UNIT2_LED_OFF             0x0000
#define POE_PWR_SUPPLY_UNIT2_LED_GREEN           0x0008
#define POE_PWR_SUPPLY_UNIT2_LED_YELLOW          0x0004

#define MSK_POE_PWR_SUPPLY_BOOST_LED             0x0010
#define POE_PWR_SUPPLY_BOOST_LED_OFF             0x0000
#define POE_PWR_SUPPLY_BOOST_LED_GREEN           0x0010


/*
 *  LED POE Daughter Card On/Off Register (+0x10) 
 */
#define MSK_POE_DAG_CARD_LED             0x0003
#define POE_DAG_CARD_LED_OFF             0x0000
#define POE_DAG_CARD_LED_GREEN           0x0002
#define POE_DAG_CARD_LED_YELLOW          0x0001


/*
 * LED Hard Disk Drive Register (+0x14)
 */
#define HDD_STAT_LED_MSK          0x00003000
#define HDD_LED_OFF               0x00000000
#define HDD_LED_GREEN             0x00001000
#define HDD_LED_YELLOW            0x00002000
#define HDD_ACT_INVERSION         0x00008000
#define HDD_ACT_STAT              0x00004000
#define HDD_ACT_FUN_MSK           0x00000C00
#define HDD_ACT_BLINK             0x00000C00
#define HDD_ACT_BLINK_B12_ON      0x00000800
#define HDD_ACT_EXT_ONLY          0x00000400
#define HDD_ACT_DRV_FROM_B12      0x00000000
#define HDD_ACT_TIME_MSK          0x000003FF


/*
 * LED Environmental Register (+0x18)
 */
#define TEMP_GREEN_LED           0x00020000
#define TEMP_YELLOW_LED          0x00010000
#define FAN_GREEN_LED            0x00008000
#define FAN_YELLOW_LED           0x00004000
#define FAN_RED_LED              0x00002000
#define FAN_YELLOW_LED_BLINK     0x00000800
#define FAN_LED_BLINK_TIME_MSK   0x000003FF


/*
 *  LED Blink Duration Register (+0x20)
 */
#define MSK_BLINK_DURA_LED           0xFF0F
#define MSK_LED_ON_TIME_REG          0xF000
#define MSK_LED_OFF_TIME_REG         0x0F00 
#define MSK_LED_PAUSE_TIME_REG       0x000F
#define OFFSET_ETH_LED_CFG_ON        12 
#define OFFSET_ETH_LED_CFG_OFF       8
#define OFFSET_ETH_LED_CFG_PAUSE     0

/*
 *  LED RJ-45/SFP Blink Enable Register   0x24
 */
#define MSK_SFP_BLINK_LED              0xFF00
#define MSK_SFP_PORT3_BLINK_LED        0xC000
#define MSK_SFP_PORT2_BLINK_LED        0x3000
#define MSK_SFP_PORT1_BLINK_LED        0x0C00
#define MSK_SFP_PORT0_BLINK_LED        0x0300
#define SFP_PORT_BLINK_LED_OFF         0x0000

#define SFP_PORT3_BLINK_ONE            0x4000
#define SFP_PORT3_BLINK_TWO            0x8000
#define SFP_PORT3_BLINK_THREE          0xC000
#define SFP_PORT2_BLINK_ONE            0x1000
#define SFP_PORT2_BLINK_TWO            0x2000
#define SFP_PORT2_BLINK_THREE          0x3000
#define SFP_PORT1_BLINK_ONE            0x0400
#define SFP_PORT1_BLINK_TWO            0x0800
#define SFP_PORT1_BLINK_THREE          0x0C00
#define SFP_PORT0_BLINK_ONE            0x0100
#define SFP_PORT0_BLINK_TWO            0x0200
#define SFP_PORT0_BLINK_THREE          0x0300
#define OFFSET_SFP_BLINK_LED_REG       8

#define MSK_RJ45_ETH_BLINK_LED         0x00FF
#define MSK_RJ45_ETH_PORT3_BLINK_LED   0x00C0
#define MSK_RJ45_ETH_PORT2_BLINK_LED   0x0030
#define MSK_RJ45_ETH_PORT1_BLINK_LED   0x000C
#define MSK_RJ45_ETH_PORT0_BLINK_LED   0x0003

#define RJ45_ETH_PORT3_BLINK_ONE       0x0040
#define RJ45_ETH_PORT3_BLINK_TWO       0x0080
#define RJ45_ETH_PORT3_BLINK_THREE     0x00C0
#define RJ45_ETH_PORT2_BLINK_ONE       0x0010
#define RJ45_ETH_PORT2_BLINK_TWO       0x0020
#define RJ45_ETH_PORT2_BLINK_THREE     0x0030
#define RJ45_ETH_PORT1_BLINK_ONE       0x0004
#define RJ45_ETH_PORT1_BLINK_TWO       0x0008
#define RJ45_ETH_PORT1_BLINK_THREE     0x000C
#define RJ45_ETH_PORT0_BLINK_ONE       0x0001
#define RJ45_ETH_PORT0_BLINK_TWO       0x0002
#define RJ45_ETH_PORT0_BLINK_THREE     0x0003

/*
 *  LED Management Ethernet Port Blink Enable Register (+0x28) 
 */
#define MSK_MNG_ETH_BLINK_LED          0x0003
#define MNG_ETH_BLINK_LED_OFF          0x0000
#define MNG_ETH_BLINK_ONE              0x0001
#define MNG_ETH_BLINK_TWO              0x0002
#define MNG_ETH_BLINK_THREE            0x0003

/*
 *  LED RJ-45 Ethernet On/Off Register (+0x2C)  
 */
#define MSK_RJ45_ETH_PORT0_LED        0x000D
#define RJ45_ETH_PORT0_LED_OFF        0x0000
#define RJ45_ETH_PORT0_LED_GREEN      0x0008
#define RJ45_ETH_PORT0_LED_YELLOW     0x0004
#define RJ45_ETH_PORT0_LED_BLINK      0x0001

#define MSK_RJ45_ETH_PORT1_LED        0x00D0
#define RJ45_ETH_PORT1_LED_OFF        0x0000
#define RJ45_ETH_PORT1_LED_GREEN      0x0080
#define RJ45_ETH_PORT1_LED_YELLOW     0x0040
#define RJ45_ETH_PORT1_LED_BLINK      0x0010

#define MSK_RJ45_ETH_PORT2_LED        0x0900
#define RJ45_ETH_PORT2_LED_OFF        0x0000
#define RJ45_ETH_PORT2_LED_GREEN      0x0800
#define RJ45_ETH_PORT2_LED_BLINK      0x0100

#define MSK_RJ45_ETH_PORT3_LED        0x9000
#define RJ45_ETH_PORT3_LED_OFF        0x0000
#define RJ45_ETH_PORT3_LED_GREEN      0x8000
#define RJ45_ETH_PORT3_LED_BLINK      0x1000

/*
 *  LED SFP On/Off Register (+0x30)  
 */
#define SFP_PLUS_0_LED_GREEN    0x80000
#define SFP_PLUS_0_LED_YELLOW   0x40000
#define SFP_PLUS_0_LED_ACT_EN   0x20000
#define SFP_PLUS_0_LED_BLINK    0x10000

#define SFP_PLUS_1_LED_GREEN    0x800000
#define SFP_PLUS_1_LED_YELLOW   0x400000
#define SFP_PLUS_1_LED_ACT_EN   0x200000
#define SFP_PLUS_1_LED_BLINK    0x100000

#define MSK_SFP0_LED        0x000D
#define SFP0_LED_OFF        0x0000
#define SFP0_LED_GREEN      0x0008
#define SFP0_LED_YELLOW     0x0004
#define SFP0_LED_BLINK      0x0001

#define MSK_SFP1_LED        0x00D0
#define SFP1_LED_OFF        0x0000
#define SFP1_LED_GREEN      0x0080
#define SFP1_LED_YELLOW     0x0040
#define SFP1_LED_BLINK      0x0010

#define MSK_SFP2_LED        0x0D00
#define SFP2_LED_OFF        0x0000
#define SFP2_LED_GREEN      0x0800
#define SFP2_LED_YELLOW     0x0400
#define SFP2_LED_BLINK      0x0100

#define MSK_SFP3_LED        0xD000
#define SFP3_LED_OFF        0x0000
#define SFP3_LED_GREEN      0x8000
#define SFP3_LED_YELLOW     0x4000
#define SFP3_LED_BLINK      0x1000

/*
 *  LED Management Ethernet On/Off Register (+0x34)  
 */
#define MSK_MNG_ETH_LED_GREEN_LINK         0x000C
#define MNG_ETH_LED_GREEN_LINK_ON          0x0008
#define MNG_ETH_LED_GREEN_LINK_ASSERT      0x0004
#define MSK_MNG_ETH_LED_GREEN_SPD          0x0003
#define MNG_ETH_LED_GREEN_SPD_ASSERT       0x0002
#define MNG_ETH_LED_GREEN_SPD_ON           0x0001


extern struct menuinfo *led_menup;
#define FPGA_LED_CMD_MASK	0x00F0	/* Force All Leds */
#define FPGA_LED_CMD_GRN_ON	0x00A0	/* All Green Leds Enabled */
#define FPGA_LED_CMD_AMB_ON	0x00B0	/* All Ember Leds Enabled */
#define FPGA_LED_CMD_CYC	0x00C0	/* Cycle All Leds every 0.25 seconds */

#define FORCE_ALL_LED_GREEN	    FPGA_LED_CMD_GRN_ON
#define FORCE_ALL_LED_YELLOW	    FPGA_LED_CMD_AMB_ON
#define FORCE_ALL_LED_CYCLE	    FPGA_LED_CMD_CYC
#define FORCE_ALL_LED_DEFAULT	    FPGA_LED_SYS_OFF
#define MSK_FORCE_ALL_LED	    FPGA_LED_CMD_MASK

int platform_led_info_init(void);
#endif /* __PLATFORM_LED_H__ */

/* ------- End of file ------- */

/*
 *-----------------------------------------------------------------------------
$Log: platform_led.h,v $
Revision 1.1  2020/01/09 01:02:02  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
