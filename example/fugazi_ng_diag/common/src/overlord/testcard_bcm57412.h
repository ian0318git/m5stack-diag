/* $Id: testcard_bcm57412.h,v 1.4 2020/12/29 03:10:51 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_bcm57412.h,v $
 *-----------------------------------------------------------------------------
 * bcm57412_test.h - Header for SM BCM57412 10G NIC.
 *
 *
 * Jan 2019, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define BCM57412_PORT_LPBK "/diag_utils/bcm-tools/bcm57412_lpbk.sh "
#define CURIE2RU_BCM57412_PORT_LPBK "/diag_utils/curie/scripts/bcm57412_lpbk.sh "

#define BCM57412_SM_PORT1_I2C "ethtool -m eth10 offset 0 length 512 hex on | grep 0000 | sed s/[[:space:]]//g | cut -c8-11 > /diag_utils/bcm-tools/bcm57412_i2c_test.txt"
#define BCM57412_SM_PORT2_I2C "ethtool -m eth11 offset 0 length 512 hex on | grep 0000 | sed s/[[:space:]]//g | cut -c8-11 > /diag_utils/bcm-tools/bcm57412_i2c_test.txt"

#define SM_PORT1_EEPROM_DUMP_A0 "./load.sh -dev 3 -eval \"cdbg omi2c 0xa0 0 0 256\""
#define SM_PORT1_EEPROM_DUMP_A2 "./load.sh -dev 3 -eval \"cdbg omi2c 0xa2 0 0 256\""
#define SM_PORT2_EEPROM_DUMP_A0 "./load.sh -dev 3 -eval \"cdbg omi2c 0xa0 0 0 256\""
#define SM_PORT2_EEPROM_DUMP_A2 "./load.sh -dev 3 -eval \"cdbg omi2c 0xa2 0 0 256\""

#define BCM57412_I2C_RESULT "/diag_utils/bcm-tools/bcm57412_i2c_test.txt"
#define SFP_PLUS_FIXED_ID "0304"

#define CURIE2RU_BCM57412_LPBK_RESULT "/tmp/bcm57412_lpbk.txt"
#define BCM57412_LPBK_RESULT "/diag_utils/bcm-tools/bcm57412_lpbk.txt"
#define BCM57412_FAIL "FAIL"
#define BCM57412_PORT1 1
#define BCM57412_PORT2 2
#define SUPPRESS_MESG "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG "echo 4 > /proc/sys/kernel/printk"
#define REMOVE_BCM57412_DRIVER "rmmod bnxt_en"
#define MODPROBE_BCM57412_DRIVER "modprobe bnxt_en"
#define ENTER_BCM57412_SCRIPT_DIR "/diag_utils/bcm-tools/"
#define BUFFER_ARRAY_SIZE 0x80

#define CURIE_BCM57412_PORT1    10
#define CURIE_BCM57412_PORT2    11
#define NEPTUNE_BCM57412_PORT1  3
#define NEPTUNE_BCM57412_PORT2  4
#define CURIE2RU_BCM57412_PORT1 12
#define CURIE2RU_BCM57412_PORT2 13
#define UNKNOWN_BCM57412_PORT   0xFF

extern int sm_bcm57412_test (int);

/*-------------------------------------------------
$Log: testcard_bcm57412.h,v $
Revision 1.4  2020/12/29 03:10:51  leschen
Remove bnxt_en operations.

Revision 1.3  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.2  2019/07/22 00:52:19  alpeng
 support sm testcard w/ bcm57412


$Endlog $
*/
