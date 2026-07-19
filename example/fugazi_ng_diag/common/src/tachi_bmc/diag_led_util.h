/* $Id: diag_led_util.h,v 1.3 2018/05/15 01:28:17 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_led_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.h - Header file for LED Utility
 *
 * January 2016, IYC
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef DIAG_LED_UTIL_H_
#define DIAG_LED_UTIL_H_

#define MB_LED_CTRL_REG             (0x400)
#define MB_LED_EMMC_REG             (MB_LED_CTRL_REG + 0x4)
#define MB_LED_PWR_SUP_REG          (MB_LED_CTRL_REG + 0x8)
#define MB_LED_POE_PWR_SUP_REG      (MB_LED_CTRL_REG + 0xC)
#define MB_LED_POE_DAU_CARD_REG     (MB_LED_CTRL_REG + 0x10)
#define MB_LED_ENV_REG              (MB_LED_CTRL_REG + 0x18)
#define MB_LED_BLINK_DUR_REG        (MB_LED_CTRL_REG + 0x20)
#define MB_LED_RJ45_BLINK_EN_REG    (MB_LED_CTRL_REG + 0x24)
#define MB_LED_RJ45_ETH_REG         (MB_LED_CTRL_REG + 0x2C)
#define MB_LED_SFP_REG              (MB_LED_CTRL_REG + 0x30)
#define MB_LED_DEB_REG              (MB_LED_CTRL_REG + 0x38)
#define MB_LED_M2_REG               (MB_LED_CTRL_REG + 0x40)
#define MB_LED_DEFAULT              (0)
#define MB_LED_GREEN                (0xA0)
#define MB_LED_AMBER                (0xB0)
#define MB_LED_CYCLE                (0xC0)
#define LED_OFF                     (0x0)
#define SFP0_SPEED_GREEN            (0x1)
#define SFP0_LINK_GREEN             (0x8)
#define SFP0_LINK_AMBER             (0x4)
#define SFP0_LINK_AMBER             (0x4)
#define SFP1_SPEED_GREEN            (0x10)
#define SFP1_LINK_GREEN             (0x80)
#define SFP1_LINK_AMBER             (0x40)
#define RJ450_SPEED_GREEN           (0x1)
#define RJ450_LINK_GREEN            (0x8)
#define RJ451_SPEED_GREEN           (0x10)
#define RJ451_LINK_GREEN            (0x80)
#define RJ450_BLK_ONE               (0x1)
#define RJ450_BLK_TWO               (0x2)
#define RJ450_BLK_THREE             (0x3)
#define RJ451_BLK_ONE               (0x4)
#define RJ451_BLK_TWO               (0x8)
#define RJ451_BLK_THREE             (0xC)
#define SFP0_BLK_ONE                (0x100)
#define SFP0_BLK_TWO                (0x200)
#define SFP0_BLK_THREE              (0x300)
#define SFP1_BLK_ONE                (0x400)
#define SFP1_BLK_TWO                (0x800)
#define SFP1_BLK_THREE              (0xC00)
#define PSU1_GREEN                  (0x2)
#define PSU1_AMBER                  (0x1)
#define PSU2_GREEN                  (0x8)
#define PSU2_AMBER                  (0x4)
#define POE_GREEN                   (0x2)
#define POE_AMBER                   (0x1)
#define POE_DC_GREEN                (0x2)
#define POE_DC_AMBER                (0x1)
#define TEM_LED_GREEN               (0x20000)
#define TEM_LED_AMBER               (0x10000)
#define FAN_LED_GREEN               (0x8000)
#define FAN_LED_AMBER               (0x4000)
#define FAN_LED_AMBER_BLK           (0x800)
#define EMMC_GREEN                  (0x20000)
#define EMMC_GREEN_BLK              (0x10000)
#define EMMC_AMBER                  (0x4000)
#define M2_GREEN                    (0x20000)
#define M2_GREEN_BLK                (0x10000)
#define PHY1512_SPEED_LED_ON           (0x1009)
#define PHY1512_SPEED_LED_OFF          (0x1008)
#define PHY1512_LINK_LED_ON            (0x1900)
#define PHY1512_LINK_LED_OFF           (0x1800)
#define SFP_SPEED_GREEN             0
#define SFP_LINK_GREEN              1
#define SFP_LINK_AMBER              2
#define SFP_LED_OFF                 3
#define SFP_MENU_EXIT               4
#define RJ45_SPEED_GREEN            0
#define RJ45_LINK_GREEN             1
#define RJ45_LED_OFF                2 
#define RJ45_MENU_EXIT              3
#define RJ45_BLK_ONE                0
#define RJ45_BLK_TWO                1 
#define RJ45_BLK_THREE              2
#define RJ45_BLK_OFF                3
#define RJ45_BLK_MENU_EXIT          4
#define SFP_BLK_ONE                 0
#define SFP_BLK_TWO                 1
#define SFP_BLK_THREE               2
#define SFP_BLK_OFF                 3
#define SFP_BLK_MENU_EXIT           4
#define PSU_LED_GREEN               0
#define PSU_LED_AMBER               1
#define PSU_LED_OFF                 2 
#define PSU_LED_MENU_EXIT           3
#define POE_LED_GREEN               0
#define POE_LED_AMBER               1
#define POE_LED_OFF                 2
#define POE_LED_MENU_EXIT           3 
#define POE_DC_LED_GREEN            0
#define POE_DC_LED_AMBER            1
#define POE_DC_LED_OFF              2
#define POE_DC_LED_MENU_EXIT        3
#define TEMP_LED_GREEN              0
#define TEMP_LED_AMBER              1
#define FANS_LED_GREEN              2
#define FANS_LED_AMBER              3
#define FAN_LED_BLK                 4
#define ENV_LED_OFF                 5
#define ENV_MENU_EXIT               6
#define EMMC_LED_GREEN              0
#define EMMC_LED_BLK                1
#define EMMC_LED_AMBER              2     
#define EMMC_LED_OFF                3
#define EMMC_MENU_EXIT              4
#define M2_LED_GREEN                0 
#define M2_LED_BLK                  1
#define M2_LED_AMBER                2
#define M2_MENU_EXIT                3
#define E1512_SPEED_LED_GREEN       0
#define E1512_LINK_LED_GREEN        1
#define E1512_LED_OFF               2
#define E1512_MENU_EXIT             3
extern int diag_led_util(void);

#endif /* DIAG_LED_UTIL_H_ */
/*---------------------------------------------------------------
$Log: diag_led_util.h,v $
Revision 1.3  2018/05/15 01:28:17  haohsu
Added individual LED test for Tachi

Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/01/08 12:51:54  benchen2
cleanup code

Revision 1.1.2.2  2016/01/08 12:49:10  benchen2
add led log section

$Endlog$
*/

