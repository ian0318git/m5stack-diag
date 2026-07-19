/* $Id: diag_peci_lib.h,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_peci_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_peci_lib.h - Header file for PECI library
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_PECI_LIB__
#define __DIAG_PECI_LIB__

#include "types.h"

#define PECI_DRV_NAME                           "/dev/pecidrv"

#define MEANINGLESS                            (0)

/* PACKAGE READ INFORMATION */

#define PACKAGE_ID_RD                          (0x00)
#define CPUID_INFO                             (0x0000)
#define PLATFORM_ID                            (0x0001)
#define UNCORE_DEV_ID                          (0x0002)
#define MAX_THREAD_ID                          (0x0003)
#define CPU_MICROCODE_REV                      (0x0004)
#define MCA_STATUS                             (0x0005)

#define MAX_TX_BUF_SIZE                        (0x14)
#define MAX_RX_BUF_SIZE                        (0x14)

typedef struct pecitransstruct {
    uint8_t TxCount;
    uint8_t RxCount;
    uint8_t ResultCode;
    uint8_t tx[MAX_TX_BUF_SIZE];
    uint8_t rx[MAX_RX_BUF_SIZE];
} PeciTransStruct;

typedef enum {
    GET_DIB = 0xF7,
    GET_TEMP = 0x01,
    RD_PKG_CFG = 0xA1,
    WR_PKG_CFG = 0xA5,
    RD_IA_MSR = 0xB1,
    RD_PCI_CFG = 0x61,
    RD_PCI_CFG_LOCAL = 0xE1,
    WR_PCI_CFG_LOCAL = 0xE5,
} peci_cmd_t;

#define DEFAULT_HOST_ID         (0x00)
#define HOST_ID_AND_RETRY(x)    ((DEFAULT_HOST_ID<<1) | (x & 0x01))

#define PECI_TRANSACTION_PENDING               (0x99)
#define PECI_ERROR_BAD_FCS1                    (0x61)
#define PECI_ERROR_BAD_FCS2                    (0x62)
#define PECI_ERROR_HW_NOT_ACTIVE               (0x63)
#define PECI_SUCCESS                           (0x00)

extern int diag_peci_get_family_id(uint8_t, uint8_t *);
extern int diag_peci_get_cpu_model(uint8_t, uint8_t *);
extern int diag_peci_get_cpu_id(uint8_t, uint32_t *);

#endif /* __DIAG_PECI_LIB__ */

/*---------------------------------------------------------------
$Log: diag_peci_lib.h,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/

