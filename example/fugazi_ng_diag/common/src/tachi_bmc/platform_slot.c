/* $Id: platform_slot.c,v 1.12 2018/12/18 13:29:11 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_slot.c,v $
 *------------------------------------------------------------------
 *
 * platform_slot.c - Platform specific slot support functions.
 *
 * Sept 2008, Shih-Nan Huang ported from Xformers, updated for O2.
 *
 * Copyright (c) 2012 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "common.h"
#include "types.h"
#include "slot.h"
#include "dev_print.h"
#include "proto.h"
#include "error.h"
#include "ngio.h"
#include "menu.h"
#include "platform_cookie.h"
#include <assert.h>
#include "cookie_4.h"
#include "cross_platform.h"
#include "dash_fpga.h" 
#include "diag_fpga_lib.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "pca.h"
#include "platform_slot_test.h"
#include <string.h>

/* all slots start at FIRST_SLOT */
extern uint32_t ngwic_testcard(void *);
extern int f35_nim_test (void *);
extern int f2w_nim_test (void *);
extern uint32_t tachi_dl_test(void *);
uint32_t goldschlager_ngwic_test(void *);
extern int arkenstone_nim_test(void *);
extern int dynamo_test(void *);
extern uint32_t fortitude_test(void *);
extern int wallander_test(void *);
extern int reva_test(void *); 

#define MAX_MOD_IDS (sizeof(port_module_tbl) / sizeof(struct module_info))

/* put your NIM module here */
static struct module_info port_module_tbl[] = {
    /* example */
    //    {.name = "END", .id = SLOT_ILLCODE, .diag = NULL, .intf_diag= NULL, early_unreset, mod_info_flags},
    {.name = "NGWIC TestCard",          .id = NGWIC_TESTCARD,
     .diag = (PFT)ngwic_testcard,       .intf_diag = (PFT)ngwic_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM-10GKR TestCard",      .id = NIM_10GKR_TESTCARD,
     .diag = (PFT)ngwic_testcard,       .intf_diag = (PFT)ngwic_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "F-35 LTE",                .id = 0xC76,
     .diag = (PFT)f35_nim_test,         .intf_diag = (PFT)f35_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
 /* f2w need to be powered up later */
    {.name = "F2W    ",                 .id = NIM_F2W_ID,
     .diag = (PFT)f2w_nim_test,         .intf_diag = (PFT)f2w_nim_test,
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_NULL},
 /*    {.name = "Operation-Dreamliner",    .id = NIM_ES2_8P,
  *     .diag = (PFT)tachi_dl_test,        .intf_diag = (PFT)tachi_dl_test,
  *     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
  *    {.name = "Operation-Dreamliner",    .id = NIM_ES2_8,
  *     .diag = (PFT)tachi_dl_test,        .intf_diag = (PFT)tachi_dl_test,
  *     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
  *    {.name = "Operation-Dreamliner",    .id = NIM_ES2_4,
  *     .diag = (PFT)tachi_dl_test,        .intf_diag = (PFT)tachi_dl_test,
  *     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
  */   
    {.name = "NIM Goldschlager",        .id = 0x0C42,
     .diag = (PFT)goldschlager_ngwic_test, .intf_diag = (PFT)goldschlager_ngwic_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},     
    {.name = "Arkenstone 4G-LTE-LA",    .id = NIM_4G_LTE_LA,
     .diag = (PFT)arkenstone_nim_test,  .intf_diag = (PFT)arkenstone_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Arkenstone LTEA",         .id = NIM_LTEA,
     .diag = (PFT)arkenstone_nim_test,  .intf_diag = (PFT)arkenstone_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
 /*    {.name = "NIM Dynamo",              .id = DYNAMO_NIM1,
  *     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
  *     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
  *    {.name = "NIM Dynamo",              .id = DYNAMO_NIM2,
  *     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
  *     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
  */     
    {.name = "NGWIC Fortitude",           .id = NGWIC_FORTITUDE,
     .diag = (PFT)fortitude_test, .intf_diag = (PFT)fortitude_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Wallander",           .id = NIM_WALLANDER_1GE,
     .diag = (PFT)wallander_test, .intf_diag = (PFT)wallander_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Wallander",           .id = NIM_WALLANDER_2GE,
     .diag = (PFT)wallander_test, .intf_diag = (PFT)wallander_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Reva",             .id = NIM_REVA_24A,
     .diag = (PFT)reva_test,               .intf_diag = (PFT)reva_test,
     .early_unreset = 0,                   .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Reva",             .id = NIM_REVA_16A,
     .diag = (PFT)reva_test,               .intf_diag = (PFT)reva_test,
     .early_unreset = 0,                   .mod_info_flags = MOD_INFO_NULL},
};

/*
 * Function:	slot_start_with
 * Description:	This function return the number of the first Slot.
 * Inputs:	none.
 * Output:	NIM_SLOT1 (1).
 */

int 
slot_start_with (void)
{
    return(FIRST_SLOT);
}
/*
 * Function:	get_real_slot
 * Description:	Determine the "real" slot number of the module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 * Inputs:	slot  - specifies slot number
 * Output:	real slot number.
 */
int 
get_wic_real_slot (int slot)
{
    int real_slot;
    real_slot = (slot <= MAX_WIC ) ? slot : slot - MAX_WIC;

    return(real_slot);
}

/*
 * Function:	get_dc_real_slot
 * Description:	Determine the "real" slot number of the daughter card module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 * Inputs:	slot  - specifies slot number
 * Output:	real slot number.
 */
int 
get_dc_real_slot(int slot)
{
    int real_slot;
    real_slot = (slot <= MAX_DC ) ? slot : slot - MAX_DC;

    return(real_slot);
}

/*
 * Function:	get_platform_slot_table
 * Description:	serach port_module_tbl for the entry that contains
 *               a certain cookie id
 * 		a main menu entry.
 * Inputs:	id - controller type
 * Output:	index; index value into port_module_tbl
 */
struct module_info *
get_platform_slot_table (int *index, unsigned short id)
{
    int i;
    for (i = 0; i < MAX_MOD_IDS; i++) {
        if ((unsigned short)port_module_tbl[i].id == id) {
            *index = i;
            return (struct module_info *)(&port_module_tbl[i]);      
        }
    }
    return (struct module_info *)NULL;
}

/*
 * Function:	get_max_wic_slots
 * Description:	return max number of wic slots
 * Inputs:	NONE
 * Output:	max number of wic slots
 */
int
get_max_wic_slots (void)
{
    int board_type;
    
    board_type = mb_board_type();

    switch(board_type) {
    case BDTYPE_TACHI_ENTRY:
        return MAX_WIC_TACHI_L;
    break;
    case BDTYPE_TACHI_HIGH:
        return MAX_WIC_TACHI_H;
    break;
    default:
        printf("Unknown board type %d, using MAX wic num %d\n", board_type, MAX_WIC);
        return MAX_WIC;
    break;
    }

}

/*
 * Function:    get_pci_dev_num
 *
 * Description: get dev number of SM.
 *
 * Inputs:      slot, dev
 *
 * Output:      SM slot dev no
 */

int
get_pci_dev_num (int slot, int dev)
{
    return(dev);
}

/*
 * Function: get_pci_device_base_offset
 * Descriptoin: dummy fucntion
 * Input: slot    -  not used
 *        dev_num - not used
 *
 * Output: NONE
 */
ulong 
get_pci_device_base_offset (uint slot, uint dev_num)
{
    return PASSED;
}

/*
 * Function: get_pci_device_base
 *
 * Given slot number and device number, this function will return the
 * PCI device base address.
 *
 * Input: slot    -  slot number
 *        dev_num -  PCI device number (IDSEL value).
 *
 * Output: pointer to IO base address for the device
 */
void *
get_pci_device_base (uint slot, uint dev_num)
{
    printf("\n NEED TO FIX %s\n", __FUNCTION__);
    return PASSED;
    
}

/*
 * Function: hwic_slot_start_with
 *
 * This function will return the start number of NGWIC slot.
 *
 * Input: none.
 *
 * Output: 0.
 */
int
hwic_slot_start_with (void)
{
    return(FIRST_SLOT);      /* start with NGWIC slot 1 */
}
/*
 * Function: get_max_hwic_slots
 *
 * This function will return the maximum number of HWIC slots.
 *
 * Input: none.
 *
 * Output: number of hwic slots
 */
int
get_max_hwic_slots (void)
{
    return(MAX_WIC);
}

int
get_wic_serdes_no (int slot)
{
    printf("FIX ME %s \n", __FUNCTION__);
    return (FAILED);
}

int
get_wic_device_no (int slot)
{
    printf("FIX ME %s \n", __FUNCTION__);
    return (FAILED);
}


/******** History ******** 
$Log: platform_slot.c,v $
Revision 1.12  2018/12/18 13:29:11  hondwang
Fix CDETs CSCvn58971 with Goldschlager and Wallander on Tachi-L

Revision 1.11  2018/11/23 01:08:11  haohsu
Support NIM Goldschlager on Tachi-L

Revision 1.10  2018/06/12 01:41:49  haohsu
Add REVA NIM for TACHI platform

Revision 1.9  2018/02/08 02:28:02  hondwang
F2W P1 additional HW change support

Revision 1.8  2017/06/06 00:48:19  haohsu
Modify for phase 2 NIM support

Revision 1.7  2017/05/24 06:34:10  haohsu
Modify NIM support for Tachi

Revision 1.6  2017/05/24 06:00:55  haohsu
Modify NIM support for Tachi

Revision 1.5  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.4.10.5  2017/02/24 06:25:43  hondwang
Fix Sam replace files miss wallander and fortitude

Revision 1.4.10.4  2017/02/20 08:29:48  haohsu
Add dynamo to Tachi-l

Revision 1.4.10.3  2017/01/09 12:17:29  hondwang
Add Wallander support

Revision 1.4.10.2  2016/12/27 10:28:36  hondwang
Add Fortitude support

Revision 1.4.10.1  2016/12/22 01:07:08  haohsu
Add NIM to Tachi-l

Revision 1.4  2016/07/12 01:53:19  hondwang
Fix F2W bug and add PCAMAP ID

Revision 1.3  2016/06/04 09:22:20  alpeng
initial check in for f2w

Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.6  2016/04/18 06:56:19  alpeng
remove menu.h on smart_cookie.h

Revision 1.1.2.5  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.4  2015/09/26 05:22:35  alpeng
update nim test entry

Revision 1.1.2.3  2015/09/14 09:23:32  alpeng
build goldschlager and dreamliner entry; update testcard util to support ge test manually

Revision 1.1.2.2  2015/08/11 07:44:28  meho
Added f35 nim tests.

Revision 1.1.2.1  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c


$Endlog$
*/
