 /* $Id: diag_emmc_test.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_emmc_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_emmc_test.h
 * Description: Header file of diag eMMC test.
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_EMMC_TEST_H__
#define __DIAG_EMMC_TEST_H__

#define MAX_COMMAND_LENGTH      2048
#define EMMC_TEST_BUFFER_SIZE	512
#define EMMC_TEST_PATTERN_SIZE	128
#define OPEN_DEVFD_RETRY        10
#define EMMC_UDA_BLK	"/dev/mmcblk0"
#define EMMC_GPP_BLK	"/dev/mmcblk0gp0"
#define EMMC_UDA_AREA	0
#define EMMC_GPP_AREA	1
#define EMMC_CHECK_TEST_FINISH_DELAY    1    //one second

#endif   /* __DIAG_EMMC_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_emmc_test.h,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/06/29 08:25:45  lucywang
Modified displayed messages of full eMMC test

Revision 1.1.2.3  2018/06/06 11:38:06  lucywang
Modified the process to enable pSLC and create 8MB GPP on eMMC based on Cisco SW requirement

Revision 1.1.2.2  2018/03/27 03:01:40  lucywang
Added eMMC utilities for full test and pSLC mode

Revision 1.1.2.1  2018/02/27 08:06:34  harrchan
Initial viper application code base


$Endlog$
*/

