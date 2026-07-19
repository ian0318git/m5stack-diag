 /* $Id: diag_emmc_test.h,v 1.2 2019/10/17 02:16:20 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_emmc_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_emmc_test.h
 * Description: Header file of diag eMMC test.
 * 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_EMMC_TEST_H__
#define __DIAG_EMMC_TEST_H__

#define MAX_COMMAND_LENGTH      2048
#define EMMC_TEST_BUFFER_SIZE	512
#define EMMC_TEST_PATTERN_SIZE	128
#define EMMC_TEST_LEN          	0x200
#define OPEN_DEVFD_RETRY        10
#define EMMC_UDA_BLK	"/dev/mmcblk0"
#define EMMC_GPP_BLK	"/dev/mmcblk0gp0"
#define EMMC_UDA_AREA	0
#define EMMC_GPP_AREA	1
#define EMMC_CHECK_TEST_FINISH_DELAY    1    //one second

#endif   /* __DIAG_EMMC_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_emmc_test.h,v $
Revision 1.2  2019/10/17 02:16:20  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.2  2019/06/20 06:21:13  kehuang2

1. Support linux_block_test function
2. Update Diag menu item base on currently project information

Revision 1.1.2.1  2018/10/25 02:37:58  harrchan
eMMC Test

$Endlog$
*/

