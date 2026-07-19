/* $Id: diag_bcm57412_test.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm57412_test.h,v $
 *------------------------------------------------------------------
 * diag_bcm57412_test.h - Header for BCM57412 10G NIC.
 *
 * Jan 2019, Ian Chang
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __FUGAZI_BCM57412_TEST_H__
#define __FUGAZI_BCM57412_TEST_H__

#define BCM57412_PORT_LPBK              "/diag_utils/fugazi/scripts/bcm57412_lpbk.sh"
#define BCM57412_PORT_INT_LPBK_TAIL     "offline"
#define BCM57412_PORT_SPEED_GET         "ethtool eth"
#define BCM57412_PORT_SPEED_GET_TAIL    "| grep Speed"
#define BCM57412_PORT_SPEED             "ethtool -s eth"
#define BCM57412_PORT1_1G_SPEED_TAIL    "speed 1000"
#define BCM57412_PORT1_10G_SPEED_TAIL   "speed 10000"
#define DISPLAY_PORT_CAP                "ethtool eth"
#define BCM57412_I2C                    "./load.sh -dev"
#define BCM57412_I2C_TAIL               "-eval \"cdbg omi2c 0xa0 0 0 2\" | grep 00000000 | sed s/[[:space:]]//g | cut -c10-13 > /fugazi-diag/bcm57412_i2c_test.txt"
#define BCM57412_I2C_RESULT             "/fugazi-diag/bcm57412_i2c_test.txt"
#define REMOVE_BCM57412_I2C_RESULT      "rm -rf /fugazi-diag/bcm57412_i2c_test.txt"
#define BCM57412_REG                    "./load.sh -dev"
#define BCM57412_REG_TAIL               "-eval \"nictest -none -T A1\" | grep \"Block BDERBD Register Test\"  > /fugazi-diag/bcm57412_reg_test.txt"
#define BCM57412_REG_RESULT             "/fugazi-diag/bcm57412_reg_test.txt"
#define REMOVE_BCM57412_REG_RESULT      "rm -rf /fugazi-diag/bcm57412_reg_test.txt"
#define FIXED_ID_LEN                    2
#define SFP_PLUS_FIXED_ID               0x0304
#define BCM57412_REG_PASS               "passed"
#define BCM57412_REG_FAIL               "failed"
#define BCM57412_REG_NULL               "NULL"

#define BCM57412_LPBK_RESULT            "/fugazi-diag/bcm57412_lpbk.txt"
#define REMOVE_BCM57412_LPBK_RESULT     "rm -rf /fugazi-diag/bcm57412_lpbk.txt"
#define BCM57412_FAIL                   "FAIL"
#define BCM57412_PORT                   12
#define BCM57412_SFP_PORT               12
#define BCM57412_PORT_STAR              0
#define BCM57412_PORT1                  1
#define BCM57412_1G                     0
#define BCM57412_10G                    1
#define CDIAG_RETRY                     3

#define SUPPRESS_MESG                   "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG                       "echo 4 > /proc/sys/kernel/printk"
#define REMOVE_BCM57412_DRIVER          "rmmod bnxt_en"
#define MODPROBE_BCM57412_DRIVER        "modprobe bnxt_en"
#define ENTER_BCM57412_SCRIPT_DIR       "/fugazi-diag"
#define BCM57412_SFP_PLUS_EEPROM_DUMP   "/diag_utils/fugazi/scripts/bcm57412_show_sfp_plus_eeprom.sh"
#define FUGAZI_UPDATE_BCM57412_FW       "source /diag_utils/fugazi/scripts/function.sh; fugazi_upgrade_bcm57412 all"
#define FUGAZI_UPDATE_BCM57412_CFG      "source /diag_utils/fugazi/scripts/function.sh; fugazi_upgrade_cfg_bcm57412 all"
#define FUGAZI_UPDATE_BCM57412_NO_LASI_CFG     "source /diag_utils/fugazi/scripts/function.sh; fugazi_upgrade_cfg_bcm57412 no_lasi"
#define FUGAZI_UPDATE_BCM57412_PHYADD_CFG      "source /diag_utils/fugazi/scripts/function.sh; fugazi_bcm57412_cfg_phyadd"
#define BCM57412_LCDIAG                 "./load.sh"

extern int ten_g_bcm57412_test(int);
#endif /* __FUGAZI_BCM57412_TEST_H__ */
/*-------------------------------------------------
 * $Log: diag_bcm57412_test.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2020/11/06 03:27:01  iachang
 * CSCvo59196-21:Support BCM57412 LASI/No-LASI config program.
 *
 * Revision 1.1.8.3  2020/09/04 08:19:51  iachang
 * BCM57412 Config Program add phy_address cfg update.
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.13  2020/08/05 09:02:41  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.12  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.11  2020/08/04 07:03:39  iachang
 * Code clean up.
 *
 * Revision 1.1.6.10  2020/03/06 05:53:15  iachang
 * Implement BCM57412 sideband tx_dis setup function.
 *
 * Revision 1.1.6.9  2019/09/27 08:03:58  iachang
 * Changed "BCM57412 SFP i2c Test" from Broadcom CDiag to Netlink.
 *
 * Revision 1.1.6.8  2019/05/28 06:04:44  iachang
 * Separated BCM57412 FW/Cfg program at two items.
 *
 * Revision 1.1.6.7  2019/05/07 08:01:52  iachang
 * Fixed BCM57412 register test intermittent issue.
 *
 * Revision 1.1.6.6  2019/04/25 03:11:41  iachang
 * Add Broadcom lcdiag tool in utility
 *
 * Revision 1.1.6.5  2019/04/11 19:31:42  iachang
 * Support BCM57412 register test
 *
 * Revision 1.1.6.4  2019/03/29 18:43:26  iachang
 * Support BCM57412 firmware upgrade utility.
 *
 * Revision 1.1.6.3  2019/03/14 18:42:04  iachang
 * Bring up BCM57412 on Fugazi
 *
 * Revision 1.1.6.2  2019/03/14 03:48:24  letsai
 * Initial check in.
 *
 * Revision 1.1.2.3  2019/01/14 09:30:56  iachang
 * Add BCM57412 SFP I2C interface test
 *
 * Revision 1.1.2.2  2019/01/10 09:07:22  iachang
 * Modify BCM57412 test for all ports
 *
 * $Endlog$
 * */

