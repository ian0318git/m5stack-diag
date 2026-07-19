/* $Id: platform_slot.c,v 1.6 2021/04/12 13:38:05 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_slot.c,v $
 *------------------------------------------------------------------
 *
 * platform_slot.c - Platform specific slot support functions.
 *
 * Sept 2008, Shih-Nan Huang ported from Xformers, updated for O2.
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
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
#include "platform_slot.h"
#include <string.h>

/* all slots start at FIRST_SLOT */
extern uint32_t ngwic_testcard(void *);
extern int f35_nim_test (void *);
extern uint32_t tachi_dl_test(void *);
uint32_t goldschlager_ngwic_test(void *);
extern int arkenstone_nim_test(void *);
extern int dynamo_test(void *);
extern uint32_t fortitude_test(void *);
extern int grimlock_test(void *wic);
extern int wallander_test(void *);
extern int reva_test(void *); 
extern int dreamliner_test (void *wic);
extern int prince_test(void *wic);
extern int switzer_test(void *);
extern int kalamata_nim_test(void *);
extern int graffham_vm_test(void *);
static uint8_t wic_uart_ctrl[] = {0,  WIC1_UART_CTRL, WIC2_UART_CTRL, WIC3_UART_CTRL};
int get_fortitude_or_grimlock(void *);

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
    {.name = "Operation-Dreamliner", .id = NIM_ES2_8P,
     .diag = (PFT)dreamliner_test, .intf_diag = (PFT)dreamliner_test,
     .early_unreset = 1,             .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "Operation-Dreamliner", .id = NIM_ES2_8,
     .diag = (PFT)dreamliner_test, .intf_diag = (PFT)dreamliner_test,
     .early_unreset = 1,             .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "Operation-Dreamliner", .id = NIM_ES2_4,
     .diag = (PFT)dreamliner_test, .intf_diag = (PFT)dreamliner_test,
     .early_unreset = 1,             .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "NIM Goldschlager",        .id = 0x0C42,
     .diag = (PFT)goldschlager_ngwic_test, .intf_diag = (PFT)goldschlager_ngwic_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Arkenstone 4G-LTE-LA",    .id = NIM_4G_LTE_LA,
     .diag = (PFT)arkenstone_nim_test,  .intf_diag = (PFT)arkenstone_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Arkenstone LTEA",         .id = NIM_LTEA,
     .diag = (PFT)arkenstone_nim_test,  .intf_diag = (PFT)arkenstone_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Dynamo",              .id = DYNAMO_NIM1,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Dynamo",              .id = DYNAMO_NIM2,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Fortitude",     .id = 0x0783,              
     .diag = (PFT)get_fortitude_or_grimlock,   .intf_diag = (PFT)get_fortitude_or_grimlock,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_DC_IS_VM},
    {.name = "Operation-Reva",      .id = NIM_REVA_16A,
     .diag = (PFT)reva_test,        .intf_diag = (PFT)reva_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Reva",      .id = NIM_REVA_24A,
     .diag = (PFT)reva_test,        .intf_diag = (PFT)reva_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Prince",    .id = NGWIC_PRINCE_1T,
     .diag = (PFT)prince_test,      .intf_diag = (PFT)prince_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Prince",    .id = NGWIC_PRINCE_2T,
     .diag = (PFT)prince_test,      .intf_diag = (PFT)prince_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Prince",    .id = NGWIC_PRINCE_4T,
     .diag = (PFT)prince_test,      .intf_diag = (PFT)prince_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "C-NIM-1X",            .id = C_NIM_1X,
     .diag = (PFT)switzer_test,     .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-2M",            .id = C_NIM_2M,
     .diag = (PFT)switzer_test,     .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-4T",            .id = C_NIM_4T,
     .diag = (PFT)switzer_test,     .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-1M",            .id = C_NIM_1M,
     .diag = (PFT)switzer_test,     .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-2T",            .id = C_NIM_2T,
     .diag = (PFT)switzer_test,     .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "Kalamata GSHDSL",     .id = NIM_KALAMATA_GSHDSL,
     .diag = (PFT)kalamata_nim_test,  .intf_diag = (PFT)kalamata_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Wallander",           .id = NIM_WALLANDER_1GE,
     .diag = (PFT)wallander_test, .intf_diag = (PFT)wallander_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Wallander",           .id = NIM_WALLANDER_2GE,
     .diag = (PFT)wallander_test, .intf_diag = (PFT)wallander_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Graffham VM",             .id = 0xB0B,
     .diag = (PFT)graffham_vm_test,     .intf_diag = (PFT)graffham_vm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},

};

static struct module_info ngsm_fortitude_grimlock[] = {
    {.name           = "FORTITUDE",        
     .id             = 0x0783,
     .diag           = (PFT)fortitude_test,    
     .intf_diag      = (PFT)fortitude_test,
     .early_unreset  = 0,            
     .mod_info_flags = MOD_INFO_NULL},
    {.name           = "GRIMLOCK",           
     .id             = 0x0783,  
     .diag           = (PFT)grimlock_test, 
     .intf_diag      = (PFT)grimlock_test, 
     .early_unreset  = 0,            
     .mod_info_flags = MOD_INFO_NULL},
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

int get_fortitude_or_grimlock (void *ngsm)
{
    struct module_info *p;
    struct ngio_intf_t *ngsm_iface = (struct ngio_intf_t *)ngsm;
    char board_pid[20] = {0};
    int test_err;

    memset(board_pid, 0, 20);
    get_slot_bd_pid(ngsm_iface->cookie, board_pid);

    printf("PID: %s\n", board_pid);
    p = &ngsm_fortitude_grimlock[0];

    /* Identify the PID is GRIMLOCK or FORTITUDE */
    if ((strncmp(board_pid, "NIM-PVDM-32", 11) == 0) ||
        (strncmp(board_pid, "NIM-PVDM-64", 11) == 0) ||
        (strncmp(board_pid, "NIM-PVDM-128", 12) == 0) ||
        (strncmp(board_pid, "NIM-PVDM-256", 12) == 0)) {
        printf("GRIMLOCK\n");
        p = &ngsm_fortitude_grimlock[1]; 

    } else {
        printf("FORTITUDE\n");
        p = &ngsm_fortitude_grimlock[0];
    }

    ngsm_iface->diag = p->diag;
    sprintf((char *)ngsm_iface->name,(char *)p->name);
    test_err = ngsm_iface->diag((void *)ngsm_iface);
         
    return (test_err);
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
    
    return 1;

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
    return (0);
}

int
get_wic_device_no (int slot)
{
    return (0);
}

/*******************************************************************************
 *
 * Function   : get_wic_uart_ctrl (int slot)
 * Description: returns dash fpag uart num of wic 
 *
 * Inputs     : slot number
 *
 * Outputs    : uart num
 *
 *******************************************************************************
 */
uint8_t
get_wic_uart_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_wic_uart_ctrl");
    }
    return (wic_uart_ctrl[slot]);

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
int
get_slot_bd_pid (uchar *eeprom_data, char *board_pid)
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

uint8_t get_sm_uart_ctrl (int slot)
{   
    return 0;
}

int get_sm_real_slot(int slot)
{   
    return 0;
}

int get_vm_real_slot(int slot)
{
    return 0;
}

int get_max_vm_slots (void)
{
    return 0;
}

int get_sm_device_no (int slot)
{   
    return 0;
}


/******** History ******** 
$Log: platform_slot.c,v $
Revision 1.6  2021/04/12 13:38:05  xiaolaya
*** empty log message ***

Revision 1.5  2020/03/13 12:19:46  letsai
Support Grimlock NIM

Revision 1.4  2019/12/30 05:59:18  kehuang2
CSCvs55860: Support Gaffham

Revision 1.3  2019/11/25 08:55:52  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2019/10/17 02:16:26  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.11  2019/07/30 07:31:18  olin2
Support Dynamo on Tabei-L

Revision 1.1.2.10  2019/07/02 01:25:31  olin2
Support Switzer on Tabei-L

Revision 1.1.2.9  2019/05/22 07:52:43  olin2
Support Prince on Tabei-L

Revision 1.1.2.8  2019/03/26 06:09:16  olin2
Support Dreamliner on Tabei-L

Revision 1.1.2.7  2019/03/07 06:38:10  olin2
Support Arkentone on Tabei-L

Revision 1.1.2.6  2019/02/22 06:43:03  olin2
Add Reva on Tabei-L

Revision 1.1.2.5  2019/02/15 06:14:28  olin2
Add Goldschlager on Tabei-L

Revision 1.1.2.4  2018/10/18 08:32:33  olin2
Support NIM testcard FPGA reg test

Revision 1.1.2.3  2018/10/18 06:02:43  olin2
Clean up unused function

Revision 1.1.2.2  2018/10/15 11:48:29  olin2
Update for using common slot.c

Revision 1.1.2.1  2018/10/09 09:22:05  olin2
Initial commit for NIM test



$Endlog$
*/
