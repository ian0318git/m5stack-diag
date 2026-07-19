/* $Id: platform_stub.c,v 1.4 2019/07/11 12:31:32 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/platform_stub.c,v $
 *------------------------------------------------------------------
 *
 * platform_stub.c - Create the dummy functions for compiler issue.
 *
 * Feb. 2016, Sofian Teja
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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

int i2c_quack_reset(void)
{
    return (PASSED);
}

int quack_version(sc_context * con)
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

int
smart_cookie_read_write_eeprom(sc_context * con, cli_cookie_cmd * cli_cmd)
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

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 *
 **********************************************************************
 */

float get_mem_overhead_factor(void) {
   /* Overhead is needed, or oom-killer will delete pid.*/ 
   return 0.1;
}


/*-------------------------------------------------
$Log: platform_stub.c,v $
Revision 1.4  2019/07/11 12:31:32  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
