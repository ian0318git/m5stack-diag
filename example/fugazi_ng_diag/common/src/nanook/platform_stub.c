 /* $Id: platform_stub.c,v 1.2 2019/12/11 10:10:35 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_stub.c,v $
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

int interface_up( char* ethname )
{
    printf("FIX THIS: %s %s", __FUNCTION__, __FILE__);
    return (PASSED);
}

int quack_version(sc_context * con)
{
    return (PASSED);
}


int
smart_cookie_read_write_eeprom(sc_context * con, cli_cookie_cmd * cli_cmd)
{
    return (PASSED);
}

type_t
smartchip_authenticate_retest(uchar type, uchar slot)
{
        return 0;
}


int
smartchip_authenticate(uchar type, uchar slot)
{
        return 0;
}

int set_gesw_line_loopback(int port_num, int onoff)
{
    printf("%s:Nanook not support GESW \n", __FUNCTION__);
    return 0;
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

int
ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port)
{
    return 0;
}

int get_gesw_line_loopback(int port_num)
{
    return 0;
}

int ovld_bcm_check_port_init (void)
{
    return 0;
}

void port_tx_util(void)
{
    return;
}

uint32_t check_poe_psu_present (uint32_t psu_no, uint32_t option)
{
    printf("%s:Curie not support POE PSU   \n", __FUNCTION__);
    return 0;
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

/*-------------------------------------------------------------------
 *
 * Function : is_goldbeach
 * Description: Return TRUE if platform is Goldbeach
 *              This function returns FALSE by default. If platform
 *              is Goldbeach, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_goldbeach (void)
    __attribute__((weak, alias("__is_goldbeach")));
int __is_goldbeach (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : utah_port_is_linkup
 * Description: Return TRUE if platform is USD/Goldbeach 
 *              This function returns FALSE by default. If platform
 *              is USD/Goldbeach, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int utah_port_is_linkup (void)
    __attribute__((weak, alias("__utah_port_is_linkup")));
int __utah_port_is_linkup (void)
{
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : netstat_main
 * Description: Return TRUE if platform is USD/Goldbeach
 *              This function returns FALSE by default. If platform
 *              is USD/Goldbeach, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int netstat_main (void)
    __attribute__((weak, alias("__netstat_main")));
int __netstat_main (void)
{
    return (FALSE);
}

/*-------------------------------------------------
 * $Log: platform_stub.c,v $
 * Revision 1.2  2019/12/11 10:10:35  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
