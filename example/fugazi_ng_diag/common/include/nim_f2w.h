/* $Id: nim_f2w.h,v 1.4 2018/05/09 03:53:36 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/nim_f2w.h,v $
 *------------------------------------------------------------------
 *
 * Filename:  nim_f2w.h
 *
 * Alan Peng - Apr. 2016.
 *
 * Copyright (c) 2016 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 */

#ifndef __NIM_F2W_H__
#define __NIM_F2W_H__

#define REG_BIT(x) (1 << (x))

#define RELAY_NORMAL      0
#define RELAY_BYPASS      1
#define RELAY_OPEN        2
#define RELAY32           2
#define RELAY10           1

#define F2W_ETH_MAX_PORT  4
#define F2W_ETH_INTF      "f2w_eth"

#define NIM_F2W_I2C_ADDR_MCU     (0x31)   /* MCU: 0x62 >> 1 */
#define NIM_F2W_I2C_ADDR_GPIO    (0x21)   /* GPIO PCA9555: 0x42 >> 1 */
#define NIM_F2W_I2C_ADDR_HS      (0x4B)   /* Hot swap, LTC4215: 0x96 >> 1 */
#define NIM_F2W_I2C_ADDR_ACT2    (0x74)   /* ACT2: 0xE8 >> 1 */
#define NIM_F2W_I2C_ADDR_I350    (0x49)   /* I350: 0x92 >> 1 */

#define NIM_F2W_GPIO_PORT0_REG    (0x4b)   /* P1 additional 0x4b, old F2W is 0xb */
#define NIM_F2W_GPIO_PORT1_REG    (0x14)   

/* minimal threshold is 1 sec, using 0.1 sec for make up
   penalty time on request i2c bus */
#define NIM_F2W_WATCHDOG_REQ_I2C   (100)  

#define F2W_LED_TEST        1
#define F2W_LED_ALL_OFF     2
#define F2W_LED_10M_ON 	    3
#define F2W_LED_100M_ON     4
#define F2W_LED_ALL_ON 	    5

#define F2W_I350_I2C_BUS_TYPE   8
#define F2W_I350_I2C_CTRL       12
#define F2W_I350_I2C_MUX        0
#define F2W_I350_I2C_OFFSET	    -1
#define F2W_I350_I2C_SIZE       2

#endif  /*  __NIM_F2W_H__ */

/* ------ History ------------ 
$Log: nim_f2w.h,v $
Revision 1.4  2018/05/09 03:53:36  hondwang
Fix F2W P2 board issue

Revision 1.3  2018/02/09 07:36:24  hondwang
Modify F2W MCU utility to support offset R/W

Revision 1.2  2016/10/19 02:52:28  hondwang
Add I350 I2C and LED test

Revision 1.1  2016/06/04 09:22:19  alpeng
initial check in for f2w



$Endlog$
*/
