/* $Id:
 * $Source:
 *------------------------------------------------------------------
 *
 * plat_defs.h - platform defines.
 *
 * June, 2019, Shuyuan Yu
 *
 * Copyright (c) 2019 ~  by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_


/*
 * Common
 */
#define LINUX_KER_V4_4_52_STRING "4.4.52"
#define LINUX_KER_V4_4_8_STRING  "4.4.8"
#define LINUX_KERNEL_V4_4_52     4452
#define LINUX_KERNEL_V4_4_8      448

/* storage common */
#define EMMC_TEST_BUFFER_SIZE	512
#define EMMC_TEST_PATTERN_SIZE	128
#define EMMC_BLK	"/dev/mmcblk0"
#define BOOTFLASH_BLK "/dev/mtdblock2"
#define NAND_FLASH_MTD_DEV "/dev/mtd3"
#define NAND_FLASH_MTD_BLK "/dev/mtdblock3"


/* ETH common */


/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)


extern int quiet_launch;



#endif                          /* _PLAT_DEFS_H_ */
