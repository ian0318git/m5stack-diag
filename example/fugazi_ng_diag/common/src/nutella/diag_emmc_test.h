/* $Id: diag_emmc_test.h,v 1.4 2019/07/11 12:31:27 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_emmc_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_emmc_test.h
 * Description: Header file of diag eMMC test.
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
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

extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int show_emmc_info (void);
extern int emmc_pslc_fully_enable(int);
extern int emmc_full_test(int);

#endif   /* __DIAG_EMMC_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_emmc_test.h,v $
Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
