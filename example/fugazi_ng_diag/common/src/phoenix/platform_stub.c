/* $Id: platform_stub.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_stub.c,v $
 *------------------------------------------------------------------
 *
 * platform_stub.c - Create the dummy functions for compiler issue.
 *
 * Feb. 2016, Sofian Teja
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
    printf("%s:TabeiL not support GESW \n", __FUNCTION__);
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
    //    printf("%s:Tabei-L not support GESW \n", __FUNCTION__); 
    return 0;
}

int get_gesw_line_loopback(int port_num)
{   
    //    printf("%s:Tabei-L not support GESW \n", __FUNCTION__); 
    return 0;
}
int ovld_bcm_check_port_init (void)
{   
    //    printf("%s:Tabei-L not support GESW \n", __FUNCTION__); 
    return 0;
}

void port_tx_util(void)
{   
    //    printf("%s:Tabei-L not support GESW \n", __FUNCTION__); 
    return;
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
 * Function : is_sword
 * Description: Return TRUE if platform is sword
 *              This function returns FALSE by default. If platform
 *              is Sword, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_sword (void)
    __attribute__((weak, alias("__is_sword")));
int __is_sword (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_dagger
 * Description: Return TRUE if platform is dagger
 *              This function returns FALSE by default. If platform
 *              is dagger, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_dagger (void)
    __attribute__((weak, alias("__is_dagger")));
int __is_dagger (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_vg450
 * Description: Return TRUE if platform is VG450
 *              This function returns FALSE by default. If platform
 *              is VG450, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_vg450 (void)
    __attribute__((weak, alias("__is_vg450")));
int __is_vg450 (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_juno_plx
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_juno_plx (void)
    __attribute__((weak, alias("__is_juno_plx")));
int __is_juno_plx (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_juno
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_juno (void)
    __attribute__((weak, alias("__is_juno")));
int __is_juno (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_neptune
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_neptune (void)
    __attribute__((weak, alias("__is_neptune")));
int __is_neptune (void)
{   
    return (FALSE);
}
/*-------------------------------------------------------------------
 *
 * Function : is_utah_plx
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_utah_plx (void)
    __attribute__((weak, alias("__is_utah_plx")));
int __is_utah_plx (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_ntpn_machines
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_ntpn_machines (void)
    __attribute__((weak, alias("__is_ntpn_machines")));
int __is_ntpn_machines (void)
{   
    return (FALSE);
}
/*-------------------------------------------------------------------
 *
 * Function : is_curie_1ru
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_curie_1ru (void)
    __attribute__((weak, alias("__is_curie_1ru")));
int __is_curie_1ru (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_curie_2ru
 * Description: Dummy function
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_curie_2ru (void)
    __attribute__((weak, alias("__is_curie_2ru")));
int __is_curie_2ru (void)
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

/*-------------------------------------------------------------------
 *
 * Function : display_env
 * Description: Dummy function for NIM Reva
 *              
 * INPUT:  None
 * OUTPUT: None
 * -------------------------------------------------------------------
 */

void display_env (void)
{
    /* Dummy function for NIM Reva */
}

/*-------------------------------------------------------------------
 *
 * Function : fortnite_is_not_support
 * Description: Fortnite is not support
 *              
 * INPUT:  None
 * OUTPUT: TRUE
 * -------------------------------------------------------------------
 */
int fortnite_is_not_support (void)
{
    printf("Fortnite is not support.\n");
    return (TRUE);
}

