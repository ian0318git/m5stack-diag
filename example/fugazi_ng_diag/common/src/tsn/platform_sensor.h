/* $Id: platform_sensor.h,v 1.3 2018/11/23 08:49:52 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_sensor.h,v $
 *------------------------------------------------------------------
 * Filename:    platform_sensor.h
 *
 * Description: TSN Diode Sensor. This file is based on Max31730 datasheet.
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SENSOR_H__
#define __PLATFORM_SENSOR_H__

typedef uint8_t sn_d;		/* Max31730 data */

/* Max31730 is Command driven. Commands defines - */
#define MAX31730_CMD_RLTS_MSB	0x00	/* Read local temperature MSB*/
#define MAX31730_CMD_RLTS_LSB	0x01	/* Read local temperature LSB*/

#define MAX31730_CMD_RRTE_MSB_1	0x02	/* Read remote 1 temperature MSB*/
#define MAX31730_CMD_RRTE_LSB_1	0x03	/* Read remote 1 temperature LSB*/

#define MAX31730_CMD_RRTE_MSB_2	0x04	/* Read remote 2 temperature MSB*/
#define MAX31730_CMD_RRTE_LSB_2	0x05	/* Read remote 2 temperature LSB*/

#define MAX31730_CMD_RRTE_MSB_3	0x06	/* Read remote 3 temperature MSB*/
#define MAX31730_CMD_RRTE_LSB_3	0x07	/* Read remote 3 temperature LSB*/

#define MAX31730_CMD_RCL	0x13	/* Read/Write configuration byte : standby, POR, timeout, extended range, comparator interrupt mdoe, one-shot, filter*/

#define MAX31730_CMD_RLHI_MSB	0x20	/* Read/Write local THIGH limit MSB*/
#define MAX31730_CMD_RLHI_LSB	0x21	/* Read/Write local THIGH limit LSB*/

#define MAX31730_CMD_RRHI_MSB_1	0x22	/* Read/Write remote 1 THIGH limit MSB*/
#define MAX31730_CMD_RRHI_LSB_1	0x23	/* Read/Write remote 1 THIGH limit LSB*/

#define MAX31730_CMD_RRHI_MSB_2	0x24	/* Read/Write remote 2 THIGH limit MSB*/
#define MAX31730_CMD_RRHI_LSB_2	0x25	/* Read/Write remote 2 THIGH limit LSB*/

#define MAX31730_CMD_RRHI_MSB_3	0x26	/* Read/Write remote 3 THIGH limit MSB*/
#define MAX31730_CMD_RRHI_LSB_3	0x27	/* Read/Write remote 3 THIGH limit LSB*/

#define MAX31730_CMD_RALI_MSB	0x30	/* Read/Write all channels TLOW limit MSB*/ 
#define MAX31730_CMD_RALI_LSB	0x31	/* Read/Write all channels TLOW limit LSB*/ 

#define MAX31730_CMD_MFGID	0x50	/* Read manufacturer ID code */
#define MAX31730_CMD_REVID	0x51	/* Read Device ID code */


/* Configuration-Byte */
#define MAX31730_RCL_STOP	0x80	/* Standby-Mode Control Bit : 1 = ADC disabled and reduce supply current to 2.5uA*/
#define MAX31730_RCL_TIMEOUT	0x20	/* SMBus timeout */
#define MAX31730_RCL_INTERRUPT	0x10	/* Interrupt/Comparator */
#define MAX31730_RCL_EXT	0x02	/* Extended-Range Enable Bit */


/* Manufacturing and Device IDs */
#define MAX31730_MFG_ID		0x4D	/* Maxim 31730 */
#define MAX31730_REV_ID		0x01

/* Global variables */
#define TEST_M   0
#define ALTER_M  1
#define COUNT200000  200000
#define DEG10        10
#define DEFAULT127   127
#define DEFAULT0xC9  0xC9
#define DEFAULTMIN55  (-55)

/* Function prototypes */
int show_snsr_reg(void);
int alter_snsr_reg(void);
int set_threshold(int);
int mb_int_test(void);
int wifi_int_test(void);

/* Extern */
extern int max31730_register_test(void);
extern int tsn_display_temp_errormsg(void);
extern int do_all_menu_items(struct menuinfo *);
extern int build_snsr_menu(boolean);
extern int build_wifi_snsr_menu(boolean);
#endif                          /* __PLATFORM_SENSOR_H__ */

/*------------------------------------------------------------------
$Log: platform_sensor.h,v $
Revision 1.3  2018/11/23 08:49:52  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.80.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2017/08/02 14:21:49  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2.2.4  2017/07/20 11:29:02  steja
Code cleanup

Revision 1.1.4.2.2.3  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.2.2.2  2017/07/17 14:41:00  steja
code cleanup

Revision 1.1.4.2.2.1  2017/07/08 07:27:26  steja
Code Clean up

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility


$Endlog$
*/
