/* $Id: nim_test_defs.h,v 1.2 2016/04/20 08:53:59 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/nim_test_defs.h,v $
 *------------------------------------------------------------------
 *
 * nim_test_defs.h - intel nim test defines.
 *
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __NIM_TEST_DEFS_H__
#define __NIM_TEST_DEFS_H__

#include "types.h"
#include "common.h"
#include "menu.h" 


enum {
    POE_PSU_ONE = 1,     /* PoE PSU 1 */
    POE_PSU_TWO,         /* PoE PSU 2 */
}; 

typedef struct diag_menu_def {
    char *name; 
    submenu_xtable_t *menu;
} diag_menu_def_t; 
    
typedef struct diag_test_list {
    char *name; 
    type_t (*xfunc)(); 
} diag_test_list_t;

#define BIT(x)                 (1 << x)

#define QUICK_MODE 1

#define TACHI_PCIE_SW_VID      (0x12D8)
#define TACHI_PCIE_SW_DID      (0x2304)
#define TESTCARD_PCIE_SW_VID   (0x10b5)
#define TESTCARD_PCIE_SW_DID   (0x8617)

#define NIM_ES2_8P             (0x0C6D) /* Dreamliner NIM: NIM-8-POE */
#define NIM_ES2_8              (0x0C6F) /* Dreamliner NIM: NIM-8     */
#define NIM_ES2_4              (0x0C72) /* Dreamliner NIM: NIM-4     */

#define TACHI_NIM_PCIE_BUS_NUM (0xb)

#endif  /* __NIM_TEST_DEFS_H__ */

/******** History ******** 
$Log: nim_test_defs.h,v $
Revision 1.2  2016/04/20 08:53:59  benchen2
add tachi fru portion

Revision 1.1.2.5  2015/10/05 10:21:39  alpeng
support single test, update loopback test

Revision 1.1.2.4  2015/09/30 06:02:19  alpeng
update dreamliner util, test and menu

Revision 1.1.2.3  2015/09/14 08:02:29  alpeng
update to support dreamliner

Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code

Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

$Endlog$
*/
