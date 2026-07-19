/* $Id: diag_tpm_test.h,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_tpm_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_tpm_test.h - TPM test header file.
 *
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define TPM_LOG_CREATE_TIME    3000
#define TPM_BUF_LOG_LEN        2048
#define TPM_TEST_LOG           "tpm_test.log"
#define TPM_PASS_STR           "Pass"
#define TPM_CHK_TOOL           "TPM2_CHK_x86.run -l tpm_test.log"
#define FOPEN_RONLY            "r"

extern int diag_tpm_test(int);

/*-------------------------------------------------
 * $Log: diag_tpm_test.h,v $
 * Revision 1.2  2019/10/17 02:16:23  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.2  2019/07/29 06:13:52  kodko
 * Clean up code based on off-line code review
 *
 * Revision 1.1.2.1  2018/12/25 02:06:29  kodko
 * Add TPM chip test that is verified by vendor provided tool.
 *
 * $Endlog$
 *-------------------------------------------------
 */
