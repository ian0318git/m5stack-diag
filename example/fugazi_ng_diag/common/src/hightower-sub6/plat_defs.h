/* $Id: plat_defs.h,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - platform defines.
 *
 * June, 2019, Shuyuan Yu
 *
 * Copyright (c) 2019-2020  by Cisco Systems, Inc.
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

/*********************************************************************
 * $Log: plat_defs.h,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/11/25 02:38:06  alpeng
 * update cvs id field
 *
 *
 *********************************************************************
 */

