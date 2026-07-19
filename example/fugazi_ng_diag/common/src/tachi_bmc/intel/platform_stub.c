/* $Id: platform_stub.c,v 1.5 2020/01/09 01:02:40 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/platform_stub.c,v $
 *------------------------------------------------------------------
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include "types.h"
#include "menu.h"
#include "common.h"
#include "nim_test_defs.h"

/* used by menu.c, cannot be removed. */
static struct mitem utilmenuitems[] = {
    {"Dummy ", 0, 0,
     (type_t(*)())0, (type_t *)0, 0, (type_t(*)())0,   0},
};
static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems)/sizeof(struct mitem),
    utilmenuitems,
};
struct menuinfo *utilmenup = &utilmenu;
/* end of used by menu.c */

int sgmii_lpbk_util (int dummy, int dummy2)
{
    printf("%s To be developed...\n", __FUNCTION__);
    return 0 ; 
}

int get_ctrl_plane_sgmii_port (void)
{
    printf("%s To be developed...\n", __FUNCTION__);
    return(1); // port 1 
}
/***********************************************************************
 *
 * Function:    get_sgmii_port_num()
 *
 * Description: Legacy code port from ISR-G2. Used by NGIO cards
 *              which also support ISR-G2 systems.
 *              Return the sgmii port number of the device attached
 *              to the sgmii port.
 *
 * Input:       port - 0 based port number of the type
 *              type - TYPE_SWITCH
 *
 * Output:      -1   if the type is invalid or the port number is out of range
 *              else the sgmii port number for the device of type TYPE, and port
 *
 ************************************************************************
 */
int
get_sgmii_port_num (uint port, uint type)
{
    return(get_ctrl_plane_sgmii_port());
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

int is_curie_1ru (void)
    __attribute__((weak, alias("__is_curie_1ru")));
int __is_curie_1ru (void)
{
    return (FALSE);
}

int is_curie_2ru (void)
    __attribute__((weak, alias("__is_curie_2ru")));
int __is_curie_2ru (void)
{
    return (FALSE);
}
/******** History ********
$Log: platform_stub.c,v $
Revision 1.5  2020/01/09 01:02:40  jiajliu
Merge Curie 2RU to main trunk

Revision 1.4  2019/08/06 06:56:18  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3  2017/08/10 10:12:52  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.2  2016/04/20 08:54:00  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/12/29 12:31:06  alpeng
support get_mb_pid for check MB sku

Revision 1.1.2.3  2015/12/23 11:09:51  alpeng
update stub

Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code

Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

$Endlog$
*/
                
