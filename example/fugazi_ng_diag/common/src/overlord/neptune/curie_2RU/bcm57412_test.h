/* $Id: bcm57412_test.h,v 1.1 2020/01/09 01:01:55 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/bcm57412_test.h,v $
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

#define BCM57412_PORT1_INT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh 4 offline"
#define BCM57412_PORT1_EXT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh 4 online"
#define BCM57412_PORT2_INT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh 5 offline"
#define BCM57412_PORT2_EXT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh 5 online"
#define BCM57412_PORT1_1G_SPEED "ethtool -s eth4 speed 1000"
#define BCM57412_PORT2_1G_SPEED "ethtool -s eth5 speed 1000"
#define BCM57412_PORT1_10G_SPEED "ethtool -s eth4 speed 10000"
#define BCM57412_PORT2_10G_SPEED "ethtool -s eth5 speed 10000"

#define BCM57412_PORT1_SPEED_GET "ethtool eth4 | grep Speed"
#define BCM57412_PORT2_SPEED_GET "ethtool eth5 | grep Speed"

#define BCM57412_PORT1_I2C "./load.sh -dev 1 -eval \"cdbg omi2c 0xa0 0 0 2\" | grep 00000000 | sed s/[[:space:]]//g | cut -c10-13 > /tmp/bcm57412_i2c_test.txt"
#define BCM57412_PORT2_I2C "./load.sh -dev 2 -eval \"cdbg omi2c 0xa0 0 0 2\" | grep 00000000 | sed s/[[:space:]]//g | cut -c10-13 > /tmp/bcm57412_i2c_test.txt"
#define BCM57412_I2C_RESULT "/tmp/bcm57412_i2c_test.txt"
#define SFP_PLUS_FIXED_ID "0304"

#define BCM57412_SFP_PLUS_EEPROM_DUMP "/diag_utils/curie/scripts/bcm57412_show_sfp_plus_eeprom.sh"
#define BCM57412_LPBK_RESULT "/tmp/bcm57412_lpbk.txt"
#define BCM57412_FAIL "FAIL"
#define BCM57412_PORT1 1
#define SUPPRESS_MESG "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG "echo 4 > /proc/sys/kernel/printk"
#define DISPLAY_PORT1_CAP "ethtool eth4"
#define DISPLAY_PORT2_CAP "ethtool eth5"
#define REMOVE_BCM57412_DRIVER "rmmod bnxt_en"
#define MODPROBE_BCM57412_DRIVER "modprobe bnxt_en"
#define ENTER_BCM57412_SCRIPT_DIR "/curie-1RU-diag"
#define ENTER_2RU_BCM57412_SCRIPT_DIR "/curie-2RU-diag"

#define BCM57412_2_PORT1_INT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh 6 offline"
#define BCM57412_2_PORT2_INT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh 7 offline"
#define BCM57412_2_PORT1_1G_SPEED "ethtool -s eth6 speed 1000"
#define BCM57412_2_PORT2_1G_SPEED "ethtool -s eth7 speed 1000"
#define BCM57412_2_PORT1_10G_SPEED "ethtool -s eth6 speed 10000"
#define BCM57412_2_PORT2_10G_SPEED "ethtool -s eth7 speed 10000"
#define BCM57412_2_PORT1_SPEED_GET "ethtool eth6 | grep Speed"
#define BCM57412_2_PORT2_SPEED_GET "ethtool eth7 | grep Speed"
#define BCM57412_2_DISPLAY_PORT1_CAP "ethtool eth6"
#define BCM57412_2_DISPLAY_PORT2_CAP "ethtool eth7"

#define BUFFER_ARRAY_SIZE 0x80
#define MB_57412_PORT1 4
#define MB_57412_PORT2 5
extern int ten_g_bcm57412_test (int);
extern int ten_g_bcm57412_2_test (int);

/*-----------------------------------------------------------------------------
$Log: bcm57412_test.h,v $
Revision 1.1  2020/01/09 01:01:55  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
