/* $Id: platform_slot.c,v 1.2 2018/08/30 06:47:15 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/vg400/platform_slot.c,v $
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
#include "sm_slot.h"
#include "platform_cookie.h"
#include <assert.h>
#include "cookie_4.h"
#include "cross_platform.h"
#include "dash_fpga.h" 
#include "strings.h"
#include <string.h>
#include "platform_slot.h"

/* all slots start at FIRST_SLOT */

extern int vg400_vir_test(void *wic);
extern int mb_board_type(void);

#define MAX_MOD_IDS (sizeof(port_module_tbl) / sizeof(struct module_info))

static uint8_t sm_uart_ctrl[] = {0,  SM1_UART_CTRL, SM2_UART_CTRL, SM3_UART_CTRL, SM4_UART_CTRL};
static uint8_t wic_uart_ctrl[] = {0,  WIC1_UART_CTRL, WIC2_UART_CTRL, WIC3_UART_CTRL}; 

/* put your NGIOSM and NGIOWIC module here */
static struct module_info port_module_tbl[] = {
    /* example */
    //    {.name = "END", .id = SLOT_ILLCODE, .diag = NULL, .intf_diag= NULL, early_unreset, mod_info_flags},
    {.name = "Atreide Virtual NIM",     .id = ATREIDES_VIRTUAL_NIM,
     .diag = (PFT)vg400_vir_test,        .intf_diag = (PFT)vg400_vir_test, 
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
};


/*
 * Function:	slot_start_with
 * Description:	This function return the number of the first SM Slot.
 */

int slot_start_with (void)
{
    return (FIRST_SLOT);
}

/*
 * Function:	get_real_slot
 * Description:	Determine the "real" slot number of the module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 */
int get_wic_real_slot (int slot)
{
    int real_slot;
    real_slot = (slot <= MAX_WIC ) ? slot : slot - MAX_WIC;

    return (real_slot);
}

/*
 * Function:	get_sm_real_slot
 * Description:	Determine the "real" slot number of the sm module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 */
int get_sm_real_slot (int slot)
{
    /* Used by Virtual NIM */
    int real_slot; 
    real_slot = (slot <= MAX_SM ) ? slot : slot - MAX_SM;

    return(real_slot);
}

/*
 * Function:	get_vm_real_slot
 * Description:	Determine the "real" slot number of the vm module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 */
int get_vm_real_slot (int slot)
{
    /* Vg400 doesn't have VM slot */ 
    return (0);
}

/*
 * Function:	get_dc_real_slot
 * Description:	Determine the "real" slot number of the daughter card module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 */
int get_dc_real_slot (int slot)
{
    /* Vg400 doesn't have slot */ 
    return (0);
}

/*
 * Function:	get_platform_slot_table
 * Description:	serach port_module_tbl for the entry that contains
 *               a certain cookie id
 * 		a main menu entry.
 * Inputs:	id - controller type
 * Output:	index; index value into port_module_tbl
 */
struct module_info * get_platform_slot_table (int *index, unsigned short id)
{
    int ix;
    for (ix = 0; ix < MAX_MOD_IDS; ix++) {
        if (port_module_tbl[ix].id == id) {
            *index = ix;
            return (struct module_info *)(&port_module_tbl[ix]);      
        }
    }
    return (struct module_info *)NULL;
}

/*
 * Function:	get_max_sm_slots
 */
int get_max_sm_slots (void)
{
    return MAX_SM;
}

/*
 * Function:	get_max_wic_slots
 */
int get_max_wic_slots (void)
{
    /* vg400 doesn't have WIC slots */
    return (0);  
}

/*
 * Function:	get_max_vm_slots
 */
int get_max_vm_slots (void)
{
    /* Vg400 doesn't have VM slots */
    return (0);
}

/*
 * Function:    get_pci_dev_num
 *
 * Description: get dev number of SM.
 *
 */

int get_pci_dev_num (int slot, int dev)
{
    /* Vg400 doesn't have SM slots */
    return (0);
}

/*
 * Function: get_pci_device_base_offset
 * Descriptoin: dummy fucntion
 * Input: slot    -  not used
 *        dev_num - not used
 *
 * Output: NONE
 */
ulong get_pci_device_base_offset (uint slot, uint dev_num)
{
    return (PASSED);
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
void * get_pci_device_base (uint slot, uint dev_num)
{
    printf("\n NEED TO FIX %s\n", __FUNCTION__);
    return (PASSED);
    
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
int hwic_slot_start_with (void)
{
    return (FIRST_SLOT);      /* start with NGWIC slot 1 */
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
int get_max_hwic_slots (void)
{
    return (MAX_WIC);
}

int get_wic_serdes_no (int slot)
{
    /* Vg400 doesn't have WIC slot */
    return (0);
}

int get_wic_device_no (int slot)
{
    /* Vg400 doesn't have WIC slot */ 
    return (0);
}

int get_sm_device_no (int slot)
{
    /* Vg400 doesn't have SM slot */ 
    return (0);
}

/*-----------------------------------------------------------------------------
 *
 * Function slot_get_bd_pid
 *
 * This function will return the board PID
 *
 * Inputs : 
 *          eeprom_data - pointer to eeprom data.
 *          board_pid - borad Product ID
 *
 * Returns : serial number of board.
 */
int get_slot_bd_pid (uchar *eeprom_data, char *board_pid)
{
    uchar *data_ptr;
    uchar num_byte;

    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
        if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
            (eeprom_data, PRODUCT_ID, &num_byte, FALSE)) == NULL) {
            sprintf((char *)board_pid, (char *)"NO PID");
            return (FAILED);

        } else {
            memcpy(board_pid, data_ptr, num_byte);
        }
        return (PASSED);
    } else {
        sprintf((char *)board_pid, (char *)"NO PID");
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function   : get_wic_uart_ctrl (int slot)
 * Description: returns dash fpag uart num of wic 
 *
 *******************************************************************************
 */
uint8_t get_wic_uart_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_wic_uart_ctrl");
    }
    return (wic_uart_ctrl[slot]);
}

/*******************************************************************************
 *
 * Function   : get_sm_uart_ctrl (int slot)
 * Description: returns dash fpag uart num of sm
 *
 *
 *******************************************************************************
 */
uint8_t get_sm_uart_ctrl (int slot)
{
    /* Used by Virtual NIM */ 
    if (slot == 0) {
        assert(!"get_sm_uart_ctrl: wrong slot 0");
    }
    return (sm_uart_ctrl[slot]);
}


/******** History ******** 
$Log: platform_slot.c,v $
Revision 1.2  2018/08/30 06:47:15  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.34  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.33  2018/02/24 07:25:46  letsai
Collapse Kalamata-branch to Main Trunk.

Revision 1.32  2017/10/18 01:51:20  olin2
Improve common code for Poll slot (CSCvg14997)

Revision 1.31  2017/09/26 07:48:14  harrchan
CSCvg04479: Support oakenshield firmware tftp download

Revision 1.30  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.29  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.28  2017/03/21 08:41:57  olin2
Collapse Aquila-branch to Main Trunk.

Revision 1.27  2017/03/16 10:55:48  umlin
Reva-SM: Commit Reva-SM platform side code to main trunk. RevaSM controller type is 0x0D77.

Revision 1.26  2017/03/16 02:44:59  haohsu
Add PID for kaziranga

Revision 1.25.2.24  2018/05/17 10:50:23  alpeng
 sync with trunk <trunk-051618>

Revision 1.25.2.23  2018/04/20 08:45:40  alpeng
support kalamata on Neptune

Revision 1.25.2.22  2018/01/30 09:03:40  alpeng
check regsiter before compare with cookie; fixed diag entry for sm testcard

Revision 1.25.2.21  2018/01/29 23:15:09  ptong
Set SM 10GKR Testcard cookie controller ID to 0x1066

Revision 1.25.2.20  2018/01/16 06:46:30  alpeng
first check in for 10G-KR SM testcard; we need to apply correct id once hw ready for it

Revision 1.25.2.19  2017/11/28 06:11:33  leschen
Return correct MAX SM/WIC/VM slot for VG450.

Revision 1.25.2.18  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.25.2.17  2017/10/19 01:05:02  alpeng
dynamo need to use get_dynamo_or_oak()

Revision 1.25.2.16  2017/10/18 23:16:59  ptong
Neptune P1C supports NGVM. Fixed poll slot to detect it

Revision 1.25.2.15  2017/10/18 07:16:43  alpeng
apply code diff for oakenshield naming fix

Revision 1.25.2.14  2017/09/25 07:11:13  alpeng
fixed typo from original code

Revision 1.25.2.13  2017/09/19 10:18:51  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.25.2.12  2017/08/11 02:41:03  leschen
Support Neptune SM4 slot.

Revision 1.25.2.11  2017/08/07 08:38:11  alpeng
support ntpn pcie device no

Revision 1.25.2.10  2017/06/03 06:36:10  alpeng
add vm portion

Revision 1.25.2.9  2017/04/05 06:45:03  leschen
Sync with <ng_diag-tag-032917>

Revision 1.25.2.8  2017/01/23 10:36:52  alpeng
update ngio slot info for triton, proteus and neso

Revision 1.25.2.7  2017/01/19 08:23:27  meho
HW requests do not poll slot 4 on Neptune.

Revision 1.25.2.6  2017/01/10 05:54:25  alpeng
remove sm4

Revision 1.25.2.5  2017/01/06 07:26:00  alpeng
fix poll slot, neptune has no vm

Revision 1.25.2.4  2016/12/16 07:35:05  alpeng
add max wic num for nep, it is used for poll slot

Revision 1.25.2.3  2016/11/24 02:58:19  leschen
Add functoin to return SM4 slot test directly.

Revision 1.25.2.2  2016/11/04 05:13:03  alpeng
update uart info to return uart ctrl number on slot.c

Revision 1.25.2.1  2016/10/18 18:58:55  alpeng
support sm3 and sm4, update intr table

Revision 1.33  2018/02/24 07:25:46  letsai
Collapse Kalamata-branch to Main Trunk.

Revision 1.32  2017/10/18 01:51:20  olin2
Improve common code for Poll slot (CSCvg14997)

Revision 1.31  2017/09/26 07:48:14  harrchan
CSCvg04479: Support oakenshield firmware tftp download

Revision 1.30  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.29  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.28  2017/03/21 08:41:57  olin2
Collapse Aquila-branch to Main Trunk.

Revision 1.27  2017/03/16 10:55:48  umlin
Reva-SM: Commit Reva-SM platform side code to main trunk. RevaSM controller type is 0x0D77.

Revision 1.26  2017/03/16 02:44:59  haohsu
Add PID for kaziranga

Revision 1.25  2016/04/26 02:15:43  umlin
Initial check-in for Reva.
Merge Reva to maintrunk.

Revision 1.24  2016/04/15 10:19:25  xiaoyizh
Initial check-in for Arkenstone.

Revision 1.23  2016/01/21 01:50:04  olin2
Collapse Draco-branch to Main Trunk.

Revision 1.22  2015/06/05 06:13:12  alpeng
fix poll slot issue

Revision 1.21  2015/05/25 03:56:24  steja
Add support Skye SM

Revision 1.20  2015/05/14 03:32:32  hondwang
Merge shedir nim to Maintrunk

Revision 1.19  2015/02/27 10:02:25  iachang

Add support dreamliner NIM

Revision 1.18  2015/02/14 12:48:41  kodko
Collapse timing card branch code into main trunk.

Revision 1.17  2015/02/12 05:58:13  bowang3
Add support to NIM Wallander

Revision 1.16  2015/01/14 02:42:51  alpeng
using early unreset to resolve testcard link training error issue

Revision 1.15  2014/11/01 05:16:39  srane
Add F35 NIM

Revision 1.14  2014/09/18 12:48:55  danchung
Disable the pcie reference clock flag for old test card to fix the pcie
link training error message

Revision 1.13  2014/09/17 03:32:15  jamlin
Add support for Goldschlager NIM.

Revision 1.12  2014/09/03 08:47:14  alpeng
enable the MOD_INFO_USE_PCIE for PCIE related modules

Revision 1.11  2014/07/02 08:09:43  alpeng
add new testcard id for en/disable menu item and select smi addr

Revision 1.10  2014/07/01 09:07:29  bowang3
Add support to NGSM carrier card Thule

Revision 1.9  2014/06/03 06:03:09  alpeng
first check in for plx on testcard; update the code for tlk10232 on testcard

Revision 1.8  2014/05/15 07:46:18  alpeng
update pcie device num for ngio

Revision 1.7  2014/03/26 19:23:16  siyen
Added Dynamo supports at the platform (CSCun82755).

Revision 1.6  2014/03/25 01:07:00  ptong
Added mod_info_flags to struc module_info

Revision 1.5  2014/03/03 06:34:39  palin2
-Initial check-in ShrinkRay host side Diag.
-Add ShrinkRay related info(ID, entry function,...) to O2 port module table.

Revision 1.4.6.2  2014/04/25 06:56:34  kodko
Support ZL30361 reference 2 clock input test.

Revision 1.4.6.1  2014/02/24 09:02:43  kodko
Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.

Revision 1.4  2013/10/08 08:48:26  tirawan
Woodlawn collapsed to main trunk

Revision 1.3  2013/10/07 03:40:13  alpeng
introducing a flag early_unreset for ngio to put reset state on early stage

Revision 1.2  2013/09/12 19:14:00  mcharon
fix typo

Revision 1.1  2013/07/16 10:02:00  alpeng
put platform_slot.c for general using

Revision 1.29  2013/05/31 12:51:04  danchung
Add checking board type for Juno.

Revision 1.28  2013/04/23 17:18:16  mcharon
add get_wic_device_no to support overdrive

Revision 1.27  2013/04/15 02:52:46  iachang
Remove Un-used Cookie ID

Revision 1.26  2013/03/31 05:03:08  iachang
Support SM Lebowski on Overlord

Revision 1.25  2013/03/08 19:06:32  mcharon
add 0xBB6 id for overdriver

Revision 1.24  2013/03/05 02:30:17  liwwang
 correct syntax error of '}'

Revision 1.23  2013/03/05 02:11:26  liwwang
add ngwic prince support

Revision 1.22  2013/02/28 00:36:12  srane
Add support for NGVM testcard.

Revision 1.21  2013/01/15 20:34:57  huanngo
Change Patriot Product ID name to SM-X-1T3/E3

Revision 1.20  2012/11/07 10:58:15  alpeng
remove useless file and clean up code

Revision 1.19  2012/11/06 20:39:51  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.18  2012/10/17 10:39:34  alpeng
first check in to support overdrive

Revision 1.17  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.16  2012/10/01 23:20:23  ywen
Add Fortitude interface test.

Revision 1.15  2012/09/28 18:57:05  palin2
Add NGSM & NGWIC TestCard into IO interface test.

Revision 1.14  2012/09/26 03:23:07  alpeng
check sata available via PID

Revision 1.13  2012/09/24 09:15:30  alpeng
if sata one or two are present, return max_wic -1

Revision 1.12  2012/09/24 01:13:35  srane
add interface test for NGVM.

Revision 1.11  2012/09/19 22:11:21  shhuang
Changed Canis module names to SM-UCS-E1{4|6}0{S|D} and test entries.

Revision 1.10  2012/09/19 18:34:38  huanngo
Support new utility for secure boot and interface test

Revision 1.9  2012/09/18 23:16:28  mcharon
if sata exists, get_max_wic_slot should return 2, not 3

Revision 1.8  2012/09/18 19:19:56  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.7  2012/09/12 10:26:08  palin2
Add NGWIC TestCard support from Host side(Overlord) DiagMenu.

Revision 1.6  2012/08/20 13:22:58  palin2
Add NGSM TestCard support from Host side(Overlord) DiagMenu.

Revision 1.5  2012/05/16 07:29:24  srane
Daughter card support.

Revision 1.4  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.3  2012/04/20 01:00:10  shhuang
Added entry to support Canis 6C DW (CID: 0x0B41).
Added entry to support Canis 4C SW (CID: 0x0BBF).

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
