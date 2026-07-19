/* $Id: diag_peci_lib.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_peci_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_peci_lib.c - PECI Library
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "diag_peci_lib.h"

int diag_peci_get_family_id(uint8_t, uint8_t *);
int diag_peci_get_cpu_model(uint8_t, uint8_t *);
int diag_peci_get_cpu_id(uint8_t, uint32_t *);

static int diag_peci_rd_pkg_config(uint8_t, uint8_t, uint16_t, uint32_t *);
static int diag_execute_peci_cmd(PeciTransStruct *);
static void diag_peci_print_pecitransstruct(FILE *, PeciTransStruct *);
static int diag_peci_print_trans_error(FILE *, int);
static uint32_t diag_peci_make_dword_from_bytearray(uint8_t *);

static int verbose = 0;

int diag_peci_get_family_id (uint8_t dev_addr, uint8_t *family_id)
{
    uint32_t data;
	int rc = 0;
	
	rc = diag_peci_get_cpu_id(dev_addr, &data);
	*family_id = (uint8_t)((data>>8) & 0xf);

	return (rc);
}

int diag_peci_get_cpu_model (uint8_t dev_addr, uint8_t *family_id)
{
    uint32_t data;
	int rc = 0;
	
	rc = diag_peci_get_cpu_id(dev_addr, &data);
	*family_id = (uint8_t)((data>>4) & 0xf);

	return (rc);
}

int diag_peci_get_cpu_id (uint8_t dev_addr, uint32_t *cpu_id)
{
    return (diag_peci_rd_pkg_config(dev_addr, PACKAGE_ID_RD, CPUID_INFO, 
                                    cpu_id));
} 

static int diag_peci_rd_pkg_config (uint8_t DevAddr, uint8_t Index, 
                                    uint16_t Param, uint32_t *Data) 
{
    PeciTransStruct pt;
    uint8_t CCode;

    pt.tx[0] = DevAddr; /* client/cpu address */
    pt.tx[1] = 5;       /* write length */
    pt.tx[2] = 5;       /* read length */
    pt.tx[3] = RD_PKG_CFG; /* cmd */
    pt.tx[4] = HOST_ID_AND_RETRY(0); /* no retry */
    pt.tx[5] = Index;
    pt.tx[6] = Param & 0xff;
    pt.tx[7] = (Param >> 8) & 0xff;
    pt.TxCount = 8;
    pt.RxCount = 5;

    diag_execute_peci_cmd(&pt);
    if( pt.ResultCode != PECI_SUCCESS ) {
        return -1;
    }

    CCode = pt.rx[0];
    if( CCode != 0x40 ) {
        diag_peci_print_trans_error(stdout, CCode);
        return -1;
    }

    *Data = diag_peci_make_dword_from_bytearray(pt.rx + 1); 
    return 0;                                    

}


static int diag_execute_peci_cmd (PeciTransStruct *p)
{
    int fd, retry;
    int rc = -1;
    int sleep_us;
    
    fd = open(PECI_DRV_NAME, O_RDONLY, 0);
    
    if (fd == -1) {
        printf("Error in opening %s\n", PECI_DRV_NAME);
        return (-1);
    }
    
    sleep_us = (1 * 1000 * 1000) / 8; /* 1/8th second between retries */
    retry    = 5 * 8;    /* retry for 5 seconds */
    do {
        rc = flock(fd, LOCK_EX);
        if (rc < 0) {
            usleep(sleep_us);
        }
    } while (rc < 0 && (--retry > 0));
    
    rc = ioctl(fd, MEANINGLESS, p);
    close(fd);
    
    if (verbose) {
        printf("%s,%d: rc=%d ResultCode=0x%02x\n",
          __func__, __LINE__, rc, p->ResultCode);
        if (rc != 0 || verbose > 1) {
            printf("---AFTER\n");
            diag_peci_print_pecitransstruct(stdout, p);
        }
    }   
    
    return (rc);
}


static void diag_peci_print_pecitransstruct (FILE *out, PeciTransStruct *p)
{
    int ix;

    fprintf(out, "TxCount: %d, RxCount: %d, ResultCode: 0x%02x\n",
            p->TxCount, p->RxCount, p->ResultCode);
    if (p->ResultCode != PECI_SUCCESS) {
        fprintf(out, "\t");
        diag_peci_print_trans_error(out, p->ResultCode);
    }
    fprintf(out, "\ttx:");
    for (ix = 0; ix < sizeof (p->tx); ++ix) {
        if (ix == p->TxCount) {
            fprintf(out, " #");
        }
        fprintf(out, " %02x", p->tx[ix]);
    }
    fprintf(out, "\n\trx:");
    for (ix = 0; ix < sizeof (p->rx); ++ix) {
        if (ix == p->RxCount) {
            fprintf(out, " #");
        }
        fprintf(out, " %02x", p->rx[ix]);
    }
    fprintf(out, "\n");
}

static int diag_peci_print_trans_error (FILE *out, int cc)
{
    char *msg;

    if (!cc & 0x80) {
        /* Not an error message */
        return (0);
    }
    switch (cc) {
    case 0x80:
    case 0x81:
        msg = "Reponse Timeout";
        break;
    case 0x82:
        msg = "Low Power State";
        break;
    case 0x90:
        msg = "Unknown/Invalid/Illegal Request";
        break;
    case 0x91:
        msg = "PECI Logial Error";
        break;
    case PECI_TRANSACTION_PENDING:
        msg = "peci_driver_transaction_pending";
        break;
    case PECI_ERROR_BAD_FCS1:
        msg = "peci_driver_bad_fcs1";
        break;
    case PECI_ERROR_BAD_FCS2:
        msg = "peci_driver_bad_fcs2";
        break;
    case PECI_ERROR_HW_NOT_ACTIVE:
        msg = "peci_driver_hw_not_active";
        break;
    default:
        msg = "Unknown error code";
        break;
    }
    /* return number of characters printed */
    return (fprintf(out, "Failed, error: %s. cc=0x%02x\n", msg, cc));
}

static uint32_t diag_peci_make_dword_from_bytearray (uint8_t *Buf)
{
    return ((Buf[3] << 24) | (Buf[2] << 16) | (Buf[1] << 8) | Buf[0]);
}


/*---------------------------------------------------------------
$Log: diag_peci_lib.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
