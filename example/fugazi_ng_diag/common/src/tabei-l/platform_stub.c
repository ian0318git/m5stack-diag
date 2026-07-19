 /* $Id: platform_stub.c,v 1.4 2020/01/09 01:02:43 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_stub.c,v $
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
 * Function : is_overlord
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_overlord (void)
    __attribute__((weak, alias("__is_overlord")));
int __is_overlord (void)
{   
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_triton
 * Description: Dummy function
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_triton (void)
    __attribute__((weak, alias("__is_triton")));
int __is_triton (void)
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

/*-------------------------------------------------------------------
 *
 * Function: get_platform_bd_rev
 *
 * Dummy function for Graffham
 * based on get_platform_ver() to return [26:24]@84
 *
 * Input:  brev - pointer of board revision
 * Output: none
 *
 *-------------------------------------------------------------------
 */
void get_platform_bd_rev (unsigned int *brev)
{
    return;
}

/*-------------------------------------------------
 * $Log: platform_stub.c,v $
 * Revision 1.4  2020/01/09 01:02:43  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.3  2019/12/30 05:59:18  kehuang2
 * CSCvs55860: Support Gaffham
 *
 * Revision 1.2  2019/10/17 02:16:27  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.12  2019/09/24 09:43:18  kehuang2
 * Sync NIM with main trunk
 *
 * Revision 1.1.4.11  2019/07/30 06:56:29  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.10  2019/03/26 06:09:16  olin2
 * Support Dreamliner on Tabei-L
 *
 * Revision 1.1.4.9  2019/03/13 09:08:22  olin2
 * Remove dummy function
 *
 * Revision 1.1.4.8  2019/03/07 06:38:10  olin2
 * Support Arkentone on Tabei-L
 *
 * Revision 1.1.4.7  2019/02/22 06:43:03  olin2
 * Add Reva on Tabei-L
 *
 * Revision 1.1.4.6  2018/11/02 02:39:03  kodko
 * Support cookie read for NIM and PIM modules.
 *
 * Revision 1.1.4.5  2018/10/16 11:33:14  olin2
 * Update NIM test
 *
 * Revision 1.1.4.4  2018/10/15 11:48:29  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.4.3  2018/10/09 09:22:05  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.4.2  2018/10/02 01:50:03  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
