/* $Id: platform_stub.c,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_stub.c,v $
 *------------------------------------------------------------------
 *
 * platform_stub.c - Create the dummy functions for compiler issue.
 *
 * Feb. 2016, Sofian Teja
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "types.h"
#include "common.h"
#include "error.h"
#include "cli_cmd.h"
#include "platform_stub.h"


/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Global Variable
 ************************************************************************/


int sgmii_lpbk_util(int port, int pkt_cnt)
{
    printf("FIX THIS: %s %s", __FUNCTION__, __FILE__);
    return (PASSED);
}

int interface_up( char* ethname )
{
    printf("FIX THIS: %s %s", __FUNCTION__, __FILE__);
    return (PASSED);
}

int ngiosm_i2c_reset(void *p)
{
    return (PASSED);
}

int ngiosm_i2c_unreset(void *p)
{
    return (PASSED);
}

int ngiovm_i2c_reset(void *p)
{
    return (OK);
}

int ngiovm_i2c_unreset(void *p)
{
    return (OK);
}

int ngiowic_i2c_reset(void *p)
{
    return (OK);
}

int ngiowic_i2c_unreset(void *p)
{
    return (PASSED);
}

struct ngio_intf_t *slot_get_ngiosm(int slot)
{
    return (PASSED);
}

struct ngio_intf_t *slot_get_ngiovm(int slot)
{
    return (PASSED);
}

struct ngio_intf_t *slot_get_ngiowic(int slot)
{
    return (PASSED);
}

int slot_start_with(void)
{
    return (PASSED);
}

int get_max_sm_slots(void)
{
    return (PASSED);
}

int ilp_poe_reset(void)
{
    printf("FIX THIS: %s %s", __FUNCTION__, __FILE__);
    return (PASSED);
}

int read_eeprom_block (unsigned int offset,
                       unsigned int size, unsigned char *buf)
{
    return (PASSED);
}
        
float get_mem_overhead_factor(void) {
   /* Overhead is needed, or oom-killer will delete pid.*/ 
   return 0.006;
}


/*-------------------------------------------------
 * $Log: platform_stub.c,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/08/04 08:37:07  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.3  2020/06/18 01:43:37  iachang
 * Sync with Curie-1RU test 99.4% of free memory (origianl test 90% of free memory)
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
