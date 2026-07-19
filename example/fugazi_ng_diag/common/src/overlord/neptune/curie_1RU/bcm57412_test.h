/* $Id: bcm57412_test.h,v 1.4 2021/10/18 06:28:59 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm57412_test.h,v $
 *-----------------------------------------------------------------------------
 * bcm57412_test.h - Header for BCM57412 10G NIC.
 *
 *
 * July 2018, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define BCM57412_PORT1_INT_LPBK "/diag_utils/bcm-tools/bcm57412_lpbk.sh 4 offline"
#define BCM57412_PORT1_EXT_LPBK "/diag_utils/bcm-tools/bcm57412_lpbk.sh 4 online"
#define BCM57412_PORT2_INT_LPBK "/diag_utils/bcm-tools/bcm57412_lpbk.sh 5 offline"
#define BCM57412_PORT2_EXT_LPBK "/diag_utils/bcm-tools/bcm57412_lpbk.sh 5 online"
#define BCM57412_PORT1_1G_SPEED "ethtool -s eth4 speed 1000"
#define BCM57412_PORT2_1G_SPEED "ethtool -s eth5 speed 1000"
#define BCM57412_PORT1_10G_SPEED "ethtool -s eth4 speed 10000"
#define BCM57412_PORT2_10G_SPEED "ethtool -s eth5 speed 10000"

#define BCM57412_PORT1_SPEED_GET "ethtool eth4 | grep Speed"
#define BCM57412_PORT2_SPEED_GET "ethtool eth5 | grep Speed"

#define BCM57412_PORT1_I2C "ethtool -m eth4 offset 0 length 512 hex on | grep 0000 | sed s/[[:space:]]//g | cut -c8-11 > /diag_utils/bcm-tools/bcm57412_i2c_test.txt"
#define BCM57412_PORT2_I2C "ethtool -m eth5 offset 0 length 512 hex on | grep 0000 | sed s/[[:space:]]//g | cut -c8-11 > /diag_utils/bcm-tools/bcm57412_i2c_test.txt"
#define BCM57412_I2C_RESULT "/diag_utils/bcm-tools/bcm57412_i2c_test.txt"
#define SFP_PLUS_FIXED_ID "0304"

#define BCM57412_PORT1_SFP_PLUS_EEPROM_DUMP "ethtool -m eth4 offset 0 length 512 hex on"
#define BCM57412_PORT2_SFP_PLUS_EEPROM_DUMP "ethtool -m eth5 offset 0 length 512 hex on"
#define BCM57412_LPBK_RESULT "/diag_utils/bcm-tools/bcm57412_lpbk.txt"
#define GLC_TE_TEST_RESULT "/diag_utils/bcm-tools/glc_te_test.txt"
#define GLC_TE_FAIL "FAIL"
#define GLC_TE_TEST_1 "/diag_utils/bcm-tools/glc_te_test.sh 4 5"
#define GLC_TE_TEST_2 "/diag_utils/bcm-tools/glc_te_test.sh 5 4"
#define BCM57412_FAIL "FAIL"
#define BCM57412_PORT1 1
#define SUPPRESS_MESG "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG "echo 4 > /proc/sys/kernel/printk"
#define DISPLAY_PORT1_CAP "ethtool eth4"
#define DISPLAY_PORT2_CAP "ethtool eth5"
#define REMOVE_BCM57412_DRIVER "rmmod bnxt_en"
#define MODPROBE_BCM57412_DRIVER "modprobe bnxt_en"
#define ENTER_BCM57412_SCRIPT_DIR "/curie-1RU-diag"
#define BUFFER_ARRAY_SIZE 0x80
#define MB_57412_PORT1 4
#define MB_57412_PORT2 5
extern int ten_g_bcm57412_test (int);

/*-------------------------------------------------
$Log: bcm57412_test.h,v $
Revision 1.4  2021/10/18 06:28:59  leschen
Support SFP GLC-TE

Revision 1.3  2020/12/29 03:09:03  leschen
Remove bnxt_en operations.

Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.6  2019/07/16 07:50:47  alpeng
move bcm57412 driver from curie dir to diag_utils/bcm-tools

Revision 1.1.2.5  2019/01/19 06:51:14  leschen
Add 1G speed setting and checking mechanism.

Revision 1.1.2.4  2018/12/27 09:45:56  leschen
Based on PRRQ CSCvn30794-2 comments to modify the codes.

Revision 1.1.2.3  2018/09/28 07:57:37  leschen
Add i2c test to verify the path between bcm57412 and sfp+

Revision 1.1.2.2  2018/08/13 20:23:38  leschen
Completing BCM57412 tests and utility

Revision 1.1.2.1  2018/08/02 08:44:11  leschen
Initial check in bcm57412 files


$Endlog $
*/
