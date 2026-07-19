/* $Id: platform_stub.c,v 1.3 2018/02/09 09:56:55 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_stub.c,v $
 *------------------------------------------------------------------
 *
 * platform_stub.c - Create the dummy functions for compiler issue.
 *
 * Feb. 2016, Sofian Teja
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
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

int i2c_quack_read_bytes(void)
{
    return (PASSED);
}

int i2c_quack_write_bytes(void)
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


/******** History ********
$Log: platform_stub.c,v $
Revision 1.3  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 07:21:47  hondwang
Fix some merge branch issue

Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3.2.1  2017/07/08 07:27:27  steja
Code Clean up

Revision 1.1.4.3  2016/06/30 08:31:53  steja
Fix compiler issue

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.7  2016/05/20 03:37:38  leschen
Support wifi

Revision 1.1.2.6  2016/04/22 11:34:00  steja
check-in for first release

Revision 1.1.2.5  2016/03/24 10:35:04  steja
Add Cookie and Act2 programming

Revision 1.1.2.4  2016/03/21 02:56:06  steja
Add debug card test items

Revision 1.1.2.3  2016/03/16 08:57:54  steja
add usb test

Revision 1.1.2.2  2016/03/14 14:32:03  steja
Add memory test

Revision 1.1.2.1  2016/03/08 09:55:11  steja
Initial Check-in

$Endlog $
*/
