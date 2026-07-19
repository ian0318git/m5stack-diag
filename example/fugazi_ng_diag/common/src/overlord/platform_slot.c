/* $Id: platform_slot.c,v 1.45 2021/04/12 13:36:45 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_slot.c,v $
 *------------------------------------------------------------------
 *
 * platform_slot.c - Platform specific slot support functions.
 *
 * Sept 2008, Shih-Nan Huang ported from Xformers, updated for O2.
 *
 * Copyright (c) 2012 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
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
#include "plat_defs.h"     

/* all slots start at FIRST_SLOT */

extern int patriot_sm_test(sm_iface_t *patriot_sm_iface);
extern int patriot_sm_iface_test(sm_iface_t *sm);
extern int canis_sm_test(sm_iface_t *);
extern int shedir_ngwic_test(void *wic);
extern int draco_ngsm_test(sm_iface_t *);
extern int oak_ngsm_test(sm_iface_t *);
extern int fortitude_test(void *wic);
extern int grimlock_test(void *wic);
extern int prince_test(void *wic);
extern int reva_test(void *wic);
extern int overdrive_test(void *);
extern int dreamliner_test (void *wic);
extern int aquila_ngsm_test(sm_iface_t *);
extern int thule_test(void *);
extern int graffham_vm_test(void *);
extern uint32_t ngsm_testcard(void *);
extern uint32_t ngwic_testcard(void *);
extern int lebowski_sm_test(sm_iface_t *);
extern int lebowski_sm_iface_test(void *sm);
extern int nightwatch_sm_test(sm_iface_t *);
extern int woodlawn_sm_test(void *);
extern int goldschlager_ngwic_test(void *wic);
extern int dynamo_test(void *wic);
extern int mb_board_type(void);
extern int f35_nim_test(void *);
extern int wallander_test(void *);
extern int timingcard_vm_test(void *);
extern int skye_sm_test(void *);
extern int arkenstone_nim_test(void *);
extern int kalamata_nim_test(void *);
extern int switzer_test(void *);
int get_canis_or_draco(void *);
int get_canis_or_aquila(void *);
int get_dynamo_or_oakenshield(void *);
int get_fortitude_or_grimlock(void *);
unsigned int ngio_testing_now = NOW_TESTING_NONE; 

#define MAX_MOD_IDS (sizeof(port_module_tbl) / sizeof(struct module_info))

static uint8_t sm_uart_ctrl[] = {0,  SM1_UART_CTRL, SM2_UART_CTRL, SM3_UART_CTRL, SM4_UART_CTRL};
static uint8_t wic_uart_ctrl[] = {0,  WIC1_UART_CTRL, WIC2_UART_CTRL, WIC3_UART_CTRL};

/* put your NGIOSM and NGIOWIC module here */
static struct module_info port_module_tbl[] = {
    /* example */
    //    {.name = "END", .id = SLOT_ILLCODE, .diag = NULL, .intf_diag= NULL, early_unreset, mod_info_flags},
    {.name = "SM-X-1T3/E3", 	        .id = 0x775,
     .diag = (PFT)patriot_sm_test,      .intf_diag = (PFT)patriot_sm_iface_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = BOARD_SHARE_ID,            .id = 0x0761,
     .diag = (PFT)get_canis_or_aquila,    .intf_diag = (PFT)get_canis_or_aquila,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "SM-UCS-E160D",        .id = 0x0B41,
     .diag = (PFT)canis_sm_test,    .intf_diag = (PFT)canis_sm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = BOARD_SHARE_ID,        .id = 0x0B3F,
     .diag = (PFT)get_canis_or_draco,    .intf_diag = (PFT)get_canis_or_draco,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "SM-UCS-E160S",        .id = 0x0789,
     .diag = (PFT)canis_sm_test,    .intf_diag = (PFT)canis_sm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Graffham VM",             .id = 0xB0B,
     .diag = (PFT)graffham_vm_test,     .intf_diag = (PFT)graffham_vm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Graffham TestCard",       .id = 0xBC8,
     .diag = (PFT)graffham_vm_test,     .intf_diag = (PFT)graffham_vm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Fortitude", .id = 0x0783,              
     .diag = (PFT)get_fortitude_or_grimlock,   .intf_diag = (PFT)get_fortitude_or_grimlock,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_DC_IS_VM},
    {.name = "Operation-Prince",    .id = NGWIC_PRINCE_1T,
     .diag = (PFT)prince_test,      .intf_diag = (PFT)prince_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Prince",    .id = NGWIC_PRINCE_2T,
     .diag = (PFT)prince_test,      .intf_diag = (PFT)prince_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Prince",    .id = NGWIC_PRINCE_4T,
     .diag = (PFT)prince_test,      .intf_diag = (PFT)prince_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Reva",      .id = NIM_REVA_16A,
     .diag = (PFT)reva_test,        .intf_diag = (PFT)reva_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Reva",      .id = NIM_REVA_24A,
     .diag = (PFT)reva_test,        .intf_diag = (PFT)reva_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Reva-SM",   .id = SM_REVA_64A,
     .diag = (PFT)reva_test,        .intf_diag = (PFT)reva_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "NGSM Thule",          .id = NGSM_THULE,
     .diag = (PFT)thule_test,       .intf_diag = (PFT)thule_test,
     .early_unreset = 0,            .mod_info_flags = (MOD_INFO_USE_PCIE | MOD_INFO_DC_IS_NIM)},
    {.name = "NGSM TestCard",           .id = NGSM_TESTCARD,
     .diag = (PFT)ngsm_testcard,        .intf_diag = (PFT)ngsm_testcard,
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NGWIC TestCard",          .id = NGWIC_TESTCARD,
     .diag = (PFT)ngwic_testcard,       .intf_diag = (PFT)ngwic_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM-10GKR TestCard",      .id = NIM_10GKR_TESTCARD,
     .diag = (PFT)ngwic_testcard,       .intf_diag = (PFT)ngwic_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "SM-10GKR TestCard",       .id = SM_10GKR_TESTCARD,
     .diag = (PFT)ngsm_testcard,       .intf_diag = (PFT)ngsm_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "SM-BCM57412 TestCard",    .id = SM_BCM57412_TESTCARD,
     .diag = (PFT)ngsm_testcard,       .intf_diag = (PFT)ngsm_testcard,
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "NGWIC Overdrive",     .id = 0xB9E,
     .diag = (PFT)overdrive_test,   .intf_diag = (PFT)overdrive_test,
     .early_unreset = 1,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "NGWIC Overdrive",     .id = 0xBB6,
     .diag = (PFT)overdrive_test,   .intf_diag = (PFT)overdrive_test,
     .early_unreset = 1,            .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "LEBOWSKI 16-port ILP",    .id = 0x0B49,
     .diag = (PFT)lebowski_sm_test,     .intf_diag = (PFT)lebowski_sm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "LEBOWSKI 24-port ILP",    .id = 0x0B4A,
     .diag = (PFT)lebowski_sm_test,     .intf_diag = (PFT)lebowski_sm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "LEBOWSKI 48-port ILP",    .id = 0x0B4B,
     .diag = (PFT)lebowski_sm_test,     .intf_diag = (PFT)lebowski_sm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIGHTWATCH 24-port",      .id = 0x108D,
     .diag = (PFT)nightwatch_sm_test,   .intf_diag = (PFT)nightwatch_sm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "NIGHTWATCH 48-port",      .id = 0x109a,
     .diag = (PFT)nightwatch_sm_test,   .intf_diag = (PFT)nightwatch_sm_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "SM-X-6X1G",           .id = NGSM_WOODLAWN_6G,
     .diag = (PFT)woodlawn_sm_test, .intf_diag = (PFT)woodlawn_sm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "SM-X-10G-4X1G",       .id = NGSM_WOODLAWN_10G4G,
     .diag = (PFT)woodlawn_sm_test, .intf_diag = (PFT)woodlawn_sm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = BOARD_SHARE_ID,        .id = DYNAMO_NIM1,
     .diag = (PFT)get_dynamo_or_oakenshield,  .intf_diag = (PFT)get_dynamo_or_oakenshield,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Operation-Dynamo, NIM2",  .id = DYNAMO_NIM2,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Kazirznga",           .id = NIM_KAZIRZNGA,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Goldschlager",        .id = 0x0C42,  
     .diag = (PFT)goldschlager_ngwic_test, .intf_diag = (PFT)goldschlager_ngwic_test, 
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Shedir",           .id = 0xCAD,  
     .diag = (PFT)shedir_ngwic_test, .intf_diag = (PFT)shedir_ngwic_test, 
     .early_unreset = 0              },
    {.name = "F-35 LTE",                .id = 0xC76,  
     .diag = (PFT)f35_nim_test,         .intf_diag = (PFT)f35_nim_test, 
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Wallander 1GE",           .id = NIM_WALLANDER_1GE,
        .diag = (PFT)wallander_test,       .intf_diag = (PFT)wallander_test,
        .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Wallander 2GE",           .id = NIM_WALLANDER_2GE,
        .diag = (PFT)wallander_test,       .intf_diag = (PFT)wallander_test,
        .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    /*** NOT a supporting module, has NOT gone thru complete testing, eg. EDVT ***/
#ifdef ENABLE_TIMINGCARD
    {.name = "VM TimingCard",        .id = TIMINGCARD_VM,
      .diag = (PFT)timingcard_vm_test,     .intf_diag = (PFT)timingcard_vm_test,
      .early_unreset = 0},
#endif
    {.name = "Operation-Dreamliner", .id = NIM_ES2_8P, 
     .diag = (PFT)dreamliner_test, .intf_diag = (PFT)dreamliner_test,
     .early_unreset = 1,             .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "Operation-Dreamliner", .id = NIM_ES2_8, 
     .diag = (PFT)dreamliner_test, .intf_diag = (PFT)dreamliner_test,
     .early_unreset = 1,             .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "Operation-Dreamliner", .id = NIM_ES2_4, 
     .diag = (PFT)dreamliner_test, .intf_diag = (PFT)dreamliner_test,
     .early_unreset = 1,             .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "Skye 1 CPU",                   .id = NGSM_SKYE_1CPU,
     .diag = (PFT)skye_sm_test,        .intf_diag = (PFT)skye_sm_test,
     .early_unreset = 0, MOD_INFO_NULL},
    {.name = "Skye 2 CPUs",                  .id = NGSM_SKYE_2CPU,
     .diag = (PFT)skye_sm_test,        .intf_diag = (PFT)skye_sm_test,
     .early_unreset = 0, MOD_INFO_NULL},
    {.name = "Arkenstone 4G-LTE-LA",	.id = NIM_4G_LTE_LA,
     .diag = (PFT)arkenstone_nim_test,  .intf_diag = (PFT)arkenstone_nim_test, 
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Arkenstone LTEA",         .id = NIM_LTEA,
     .diag = (PFT)arkenstone_nim_test,  .intf_diag = (PFT)arkenstone_nim_test, 
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "Kalamata GSHDSL",         .id = NIM_KALAMATA_GSHDSL,
     .diag = (PFT)kalamata_nim_test,  .intf_diag = (PFT)kalamata_nim_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "C-NIM-1X",                .id = C_NIM_1X,
     .diag = (PFT)switzer_test,         .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-SM-NIM-ADPT",           .id = C_SM_NIM_ADPT,
     .diag = (PFT)switzer_test,         .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE | MOD_INFO_DC_IS_NIM | MOD_INFO_SM_IS_CARRIER},
    {.name = "C-NIM-2M",                .id = C_NIM_2M,
     .diag = (PFT)switzer_test,         .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-4T",                .id = C_NIM_4T,
     .diag = (PFT)switzer_test,         .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-1M",                .id = C_NIM_1M,
     .diag = (PFT)switzer_test,         .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "C-NIM-2T",                .id = C_NIM_2T,
     .diag = (PFT)switzer_test,         .intf_diag = (PFT)switzer_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_USE_PCIE},
};


static struct module_info ngsm_canis_draco[] = {
    {.name = "SM_UCS-E140S",        .id = 0x0B3F,
     .diag = (PFT)canis_sm_test,    .intf_diag = (PFT)canis_sm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "SM UCS-E160S-M3/K9",  .id = 0x0B3F,  
     .diag = (PFT)draco_ngsm_test, .intf_diag = (PFT)draco_ngsm_test, 
     .early_unreset = 0,             .mod_info_flags = MOD_INFO_NULL},
};

static struct module_info ngsm_canis_aquila[] = {
    {.name = "SM_UCS-E140D",        .id = 0x0761,
     .diag = (PFT)canis_sm_test,    .intf_diag = (PFT)canis_sm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "SM AQUILA",           .id = 0x0761,  
     .diag = (PFT)aquila_ngsm_test, .intf_diag = (PFT)aquila_ngsm_test, 
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
};

static struct module_info ngsm_dynamo_oakenshield[] = {
    {.name = "Operation-Dynamo, NIM1",  .id = DYNAMO_NIM1,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "SM-Oakenshield",      .id = OAKENSHIELD_SM,
     .diag = (PFT)oak_ngsm_test,    .intf_diag = (PFT)oak_ngsm_test,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
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
 * Description:	This function return the number of the first SM Slot.
 * Inputs:	none.
 * Output:	SM_SLOT1 (1).
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
 * Function:	get_sm_real_slot
 * Description:	Determine the "real" slot number of the sm module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 * Inputs:	slot  - specifies slot number
 * Output:	real slot number.
 */
int 
get_sm_real_slot(int slot)
{
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
 * Inputs:	slot  - specifies slot number
 * Output:	real slot number.
 */
int 
get_vm_real_slot(int slot)
{
    int real_slot;
    real_slot = (slot <= MAX_VM ) ? slot : slot - MAX_VM;
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
        if (port_module_tbl[i].id == id) {
            *index = i;
            return (struct module_info *)(&port_module_tbl[i]);      
        }
    }
    return (struct module_info *)NULL;
}

/*
 * Function:	get_max_sm_slots
 * Description:	return max number of SM slots
 * Inputs:	NONE
 * Output:	max number of SM slots
 */
int
get_max_sm_slots (void)
{
    int board_type;
    
    board_type = mb_board_type();

    switch(board_type) {
    case BDTYPE_OVERLORD:
        return MAX_SM_O2;
    break;
    case BDTYPE_JUNO:
        return MAX_SM_JUNO;
    break;
    case BDTYPE_UTAH:
        return MAX_SM_UTAH;
    break;
    case BDTYPE_SWORD:
        return MAX_SM_SWORD;
    break;
    case BDTYPE_DAGGER:
        return MAX_SM_DAGGER;
    break;
    case BDTYPE_NEPTUNE:
        /* Neptune P1C need to support SM slot 4 */
        return MAX_SM_NEPTUNE;
    break;
    case BDTYPE_VG450:
        return MAX_SM_VG450;
    break;
    case BDTYPE_TRITON:
        return MAX_SM_TRITON; 
    break;
    case BDTYPE_PROTEUS:
        return MAX_SM_PROTEUS; 
    break;
    case BDTYPE_NESO:   
        return MAX_SM_NESO;    
    break;
    case BDTYPE_GOLDBEACH:
        return MAX_SM_GOLDBEACH;
    break;
    case BDTYPE_URANIUM:
    case BDTYPE_THORIUM:
        return MAX_SM_URANIUM;
    case BDTYPE_RADIUM:    
    case BDTYPE_THALLIUM:    
    case BDTYPE_POLONIUM:  
        return MAX_SM_RADIUM;    
    break;
    default:
        printf("Unknown board type %d, using MAX SM num %d\n", board_type, MAX_SM);
        return MAX_SM;
    break;
    }
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
    case BDTYPE_OVERLORD:
        return MAX_WIC;
    break;
    case BDTYPE_JUNO:
        return MAX_WIC_JUNO;
    break;
    case BDTYPE_UTAH:
        return MAX_WIC_UTAH;
    break;
    case BDTYPE_SWORD:
        return MAX_WIC_SWORD;
    break;
    case BDTYPE_DAGGER:
        return MAX_WIC_DAGGER;
    break;
    case BDTYPE_NEPTUNE:
        return MAX_WIC_NEPTUNE; 
    break;
    case BDTYPE_VG450:
        return MAX_WIC_VG450; 
    break;
    case BDTYPE_TRITON:
        return MAX_WIC_TRITON; 
    break;
    case BDTYPE_PROTEUS:
        return MAX_WIC_PROTEUS; 
    break;
    case BDTYPE_NESO:   
        return MAX_WIC_NESO;    
    break;
    case BDTYPE_GOLDBEACH:
        return MAX_WIC_GOLDBEACH;
    break;
    case BDTYPE_URANIUM:
    case BDTYPE_THORIUM:
        return MAX_WIC_URANIUM;
    break;
    case BDTYPE_RADIUM:    
    case BDTYPE_THALLIUM:    
        return MAX_WIC_RADIUM;    
    break;
    case BDTYPE_POLONIUM:  
        return MAX_WIC_POLONIUM; 
    break;
    default:
        printf("Unknown board type %d, using MAX wic num %d\n", board_type, MAX_WIC);
        return MAX_WIC;
    break;
    }

}

/*
 * Function:	get_max_vm_slots
 * Description:	return max number of vm slots
 * Inputs:	NONE
 * Output:	max number of vm slots
 */
int
get_max_vm_slots (void)
{
    int board_type; 
    unsigned int plat_bd_rev = 0;

    board_type = mb_board_type();

    switch(board_type) {
    case BDTYPE_OVERLORD:
    case BDTYPE_JUNO:
    case BDTYPE_UTAH:
    case BDTYPE_SWORD:
    case BDTYPE_DAGGER:
    case BDTYPE_TRITON:
    case BDTYPE_PROTEUS:
    case BDTYPE_NESO:
        return MAX_VM; 
    break;
    case BDTYPE_NEPTUNE:
	get_platform_bd_rev(&plat_bd_rev);
	if (plat_bd_rev == 2) {
	  /* Neptune P1C board rev is 2. It supports NGVM for
	   * Triton and VG450 design validation purpose.
	   */
	  return MAX_VM_NEPTUNE;
	}
        return MAX_VM_NEPTUNE;  /* 0 */
    case BDTYPE_VG450:
        return MAX_VM_NEPTUNE;
    break;
    case BDTYPE_GOLDBEACH:
        return MAX_VM_GOLDBEACH;
    break;
    case BDTYPE_URANIUM:
    case BDTYPE_THORIUM:
        return MAX_VM_URANIUM;
    break;
    case BDTYPE_RADIUM:
    case BDTYPE_THALLIUM:
    case BDTYPE_POLONIUM:
        return MAX_VM_RADIUM;
    break;
    default:
        printf("Unknown board type %d, using MAX vm num %d\n", board_type, MAX_VM);
        return MAX_VM;
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
    
#if 0
    ulong temp;

    assert(slot <= get_max_sm_slots());
    assert(dev_num <= MAX_NUM_DEVS_PER_SM);

    temp = slot_cntl[slot].reg_io_base_address;
    return((void *)(temp + (dev_num * PCI_DEV_IO_SEPARATION)));
#endif    
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
    switch (slot) {
    case 1:
        if (is_utah_plx() || is_juno_plx()) {
            return(0xc);
        } else if (is_sword()) {
            return(4);
        } else if (is_dagger()) {
            return(4);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(8);
        }
        break;
    case 2:
        if (is_utah_plx() || is_juno_plx()) {
            return(0xd);
        } else if (is_sword()) {
            return(0xc);
        } else if (is_dagger()) {
            return(5);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(0xa);
        }
        break;
    case 3:
        if (is_utah_plx() || is_juno_plx()) {
            return(0xe);
        } else if (is_sword()) {
            assert(!"incorrect wic slot. unable to set Gen speed.");
            return(0);
        } else if (is_dagger()) {
            assert(!"incorrect wic slot. unable to set Gen speed.");
            return(0);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(2);
        }
        break;
    default:
        assert(!"incorrect wic slot. unable to get wic serdes num.");
    }
    return(0);
}

int
get_wic_device_no (int slot)
{
    switch (slot) {
    case 1:
        if (is_utah_plx() || is_juno_plx()) {
            return(3);
        } else if (is_sword()) {
            return(2);
        } else if (is_dagger()) {
            return(1);
        } else if (is_ntpn_machines() || is_vg450()) {
            return(1);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(8);
        }
        break;
    case 2:
        if (is_utah_plx() || is_juno_plx()) {
            return(0xb);
        } else if (is_sword()) {
            return(3);
        } else if (is_dagger()) {
            return(5);
        } else if (is_ntpn_machines() || is_vg450()) {
            return(2);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(0xa);
        }
        break;
    case 3:
        if (is_utah_plx() || is_juno_plx()) {
            return(0xd);
        } else if (is_sword()) {
            assert(!"incorrect wic slot. unable to set Gen speed.");
            return(0);
        } else if (is_dagger()) {
            assert(!"incorrect wic slot. unable to set Gen speed.");
            return(0);
        } else if (is_ntpn_machines() || is_vg450()) {
            return(3);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(2);
        }
        break;
    default:
        assert(!"incorrect wic slot. unable to get wic pcie bus num.");
    }
    return(0);
}

int
get_sm_device_no (int slot)
{
    switch (slot) {
    case 1:
        if (is_utah_plx() || is_juno_plx() || is_sword()) {
            return(1);
        } else if (is_dagger()) {
            assert(!"incorrect sm slot. unable to set Gen speed.");
            return(0);
        } else if (is_ntpn_machines() || is_vg450()) {
            return(7);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(0xc);
        }
        break;
    case 2:
        if (is_utah_plx() || is_juno_plx()) {
            return(2);
        } else if (is_sword() || is_dagger()) {
            assert(!"incorrect sm slot. unable to set Gen speed.");
            return(0);
        } else if (is_ntpn_machines() || is_vg450()) {
            return(8);
        } else {  /* Utah IDT, O2/Juno IDT */
            return(0xe);
        }
        break;
    case 3: 
        if (is_ntpn_machines() || is_vg450()) {
            return(9);
        } else {
            assert(!"incorrect sm slot. unable to set Gen speed.");
            return(0);
        }
        break;
    default:
        assert(!"incorrect wic slot. unable to get sm pcie bus num.");
    }
    return(0);
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

int get_canis_or_aquila (void *ngsm)
{
    struct module_info *p;
    struct ngio_intf_t *ngsm_iface = (struct ngio_intf_t *)ngsm;
    char board_pid[20] = {0};
    int test_err;

    get_slot_bd_pid(ngsm_iface->cookie, board_pid);

    printf("PID: %s\n", board_pid);

    if ((strncmp(board_pid, AQUILA_12C_PID, AQUILA_PID_LEN_S) == 0) ||
        (strncmp(board_pid, AQUILA_8C_PID, AQUILA_PID_LEN_S) == 0)) {
        printf("Aquila\n");
        p = &ngsm_canis_aquila[1]; 

    } else {
        printf("Canis\n");
        p = &ngsm_canis_aquila[0];
    }

    ngsm_iface->diag = p->diag;

    sprintf((char *)ngsm_iface->name,(char *)p->name);

    test_err = ngsm_iface->diag((void *)ngsm_iface);
         
    return (test_err);

}


int get_canis_or_draco (void *ngsm)
{
    struct module_info *p;
    struct ngio_intf_t *ngsm_iface = (struct ngio_intf_t *)ngsm;
    char board_pid[20] = {0};
    int test_err;

    get_slot_bd_pid(ngsm_iface->cookie, board_pid);

    printf("PID: %s\n", board_pid);

    if (strncmp(board_pid, DRACO_PID, DRACO_PID_LEN_S) == 0) {

        p = &ngsm_canis_draco[1]; 

    } else {

        p = &ngsm_canis_draco[0];

    }

    ngsm_iface->diag = p->diag;

    sprintf((char *)ngsm_iface->name,(char *)p->name);

    test_err = ngsm_iface->diag((void *)ngsm_iface);
         
    return (test_err);

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

/*******************************************************************************
 *
 * Function   : get_sm_uart_ctrl (int slot)
 * Description: returns dash fpag uart num of sm
 *
 * Inputs     : slot number
 *
 * Outputs    : uart num
 *
 *******************************************************************************
 */
uint8_t
get_sm_uart_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_sm_uart_ctrl: wrong slot 0");
    }
    return (sm_uart_ctrl[slot]);
}

int get_dynamo_or_oakenshield (void *ngsm) 
{
    struct module_info *p;
    struct ngio_intf_t *ngsm_iface = (struct ngio_intf_t *)ngsm;
    char board_pid[20] = {0};
    int test_err;

    get_slot_bd_pid(ngsm_iface->cookie, board_pid);

    printf("PID: %s\n", board_pid);

    if (strncmp(board_pid, OAKENSHIELD_PID, OAKENSHIELD_PID_LEN_S) == 0) {
        p = &ngsm_dynamo_oakenshield[1];
    } else {
        p = &ngsm_dynamo_oakenshield[0];
    }
    ngsm_iface->diag = p->diag;

    sprintf((char *)ngsm_iface->name,(char *)p->name);

    test_err = ngsm_iface->diag((void *)ngsm_iface);

    return (test_err);
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

/*-------------------------------------------------------------------
 *
 * Function : set_ngio_now_testing 
 * Description: set flag ngio_testing_now to distinguish 
 *              which ngio/slot is testing
 *
 * INPUT:  ngio_ptr - pointer to ngio structure. 
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
void set_ngio_now_testing (struct ngio_intf_t* ngio_ptr) 
{
    if ((ngio_ptr->mod_type == WIC_MODULE)) {
        switch (ngio_ptr->slot) {
        case NGWIC1_SLOT: 
            ngio_testing_now = NOW_TESTING_NIM1; 
        break; 
        case NGWIC2_SLOT: 
            ngio_testing_now = NOW_TESTING_NIM2; 
        break; 
        case NGWIC3_SLOT: 
            ngio_testing_now = NOW_TESTING_NONE; 
        break; 
    
        default: 
            printf("%s: slot%d is not matched \n", __FUNCTION__, ngio_ptr->slot); 
        break; 
        }
    } else if ((ngio_ptr->mod_type == SM_MODULE)) {
        switch (ngio_ptr->slot) {
        /* for curie 1ru , it is an entry for return eth port number of ngio
         * on platform_eth_pkt_txrx.c */
        case NGSM1_SLOT: 
            ngio_testing_now = NOW_TESTING_SM1; 
            /* GE1 might not be download port, we don't assign ip here */
        break; 
        case NGSM2_SLOT: 
            ngio_testing_now = NOW_TESTING_SM2;
        break;
        case NGSM3_SLOT: 
            ngio_testing_now = NOW_TESTING_NONE; 
        break; 
        }
    } else if (ngio_ptr->mod_type == DAUGHTER_CARD) {
        if(ngio_ptr->pc->slot == NGSM1_SLOT) {
            switch (ngio_ptr->slot) {
                case NGSM_WIC1_SLOT:
                    ngio_testing_now = NOW_TESTING_SM1_NIM1;
                    break;
                case NGSM_WIC2_SLOT:
                    ngio_testing_now = NOW_TESTING_SM1_NIM2;
                    break;
                default:
                    printf("%s: slot%d is not matched \n", __FUNCTION__, ngio_ptr->slot);
                    break;
            }
        } else if (ngio_ptr->pc->slot == NGSM2_SLOT) {
            switch (ngio_ptr->slot) {
                case NGSM_WIC1_SLOT:
                    ngio_testing_now = NOW_TESTING_SM2_NIM1;
                    break;
                case NGSM_WIC2_SLOT:
                    ngio_testing_now = NOW_TESTING_SM2_NIM2;
                    break;
                default:
                    printf("%s: slot%d is not matched \n", __FUNCTION__, ngio_ptr->slot);
                    break;
            }
        } else {
            printf("%s: slot%d is not matched \n", __FUNCTION__, ngio_ptr->pc->slot);
        }
    } else { 
        /* 20190920 -  except NIM/SM and their DC, 
         * the other mod_type has no eth intf, 
         * we don't want to print extra message to confuse people.
         */

    }

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function : get_ngio_testing_now
 * Description: return the number for current testing. 
 *
 * INPUT:  none
 * OUTPUT: number of testing ngio.
 * -------------------------------------------------------------------
 */
int get_ngio_testing_now (void)
{
    return (ngio_testing_now); 
}

/*-------------------------------------------------------------------
 *
 * Function : skip_oakenshield_ge1_lpbk 
 * Description: Return TRUE if platform is Curie 1RU to skip
 *              Oakenshield GE1 lpbk test
 *
 * INPUT: NONE 
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int skip_oakenshield_ge1_lpbk (void)
    __attribute__((weak, alias("__skip_oakenshield_ge1_lpbk")));
int __skip_oakenshield_ge1_lpbk (void)
{
    if (is_curie_1ru() || is_curie_2ru()) {
        /* 
         * Curie skips Oakenshield GE1 lpbk test
         * based on Curie hardware and Oakenshield Uboot owner's opinions.
         * Oakenshield Uboot initialize GE0 only so Curie Intel internal
         * Ethernet port can't get link when doing Oakenshield GE1 lpbk.
         * 
         */
        printf("\nCurie platform skips Oakenshield GE1 lpbk test\n");
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*-------------------------------------------------------------------
 *
 * Function : resend_skye_kernel_boot_cmd
 * Description: Return TRUE if platform is Curie 1RU to resend 
 *              Skye kernel boot command 
 *
 * INPUT: NONE 
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int resend_skye_kernel_boot_cmd (void)
    __attribute__((weak, alias("__resend_skye_kernel_boot_cmd")));
int __resend_skye_kernel_boot_cmd (void)
{
    if (is_curie_1ru() || is_curie_2ru()) {
        /* 
         * There is a chance that Skye SM card can't boot up
         * it's kernel on Curie platform so we resend boot command
         * when Skye is trying to get Linux prompt 
         */
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*-------------------------------------------------------------------
 *
 * Function : is_curie_1ru
 * Description: Return TRUE if platform is Curie 1RU
 *              This function returns FALSE by default. If platform
 *              is curie_1ru, declare this function in platform code
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
 * Description: Return TRUE if platform is Curie 2RU
 *              This function returns FALSE by default. If platform
 *              is curie_2ru, declare this function in platform code
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
/*-------------------------------------------------------------------
 *
 * Function : switzer_test
 * Description: older platfroms are not support switzer 
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int switzer_test (void *ptr)
    __attribute__((weak, alias("__switzer_test")));
int __switzer_test (void *ptr)
{
    printf("%s : not support \n", __FUNCTION__); 
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_carrier_get_wic_i2c_ctrl
 * Description: older platfroms are not support switzer-carrier
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return 0
 * -------------------------------------------------------------------
 */
uint8_t switzer_carrier_get_wic_i2c_ctrl (int slot)
    __attribute__((weak, alias("__switzer_carrier_get_wic_i2c_ctrl")));
uint8_t __switzer_carrier_get_wic_i2c_ctrl(int slot)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_carrier_get_wic_i2c_base
 * Description: older platfroms are not support switzer-carrier
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return 0
 * -------------------------------------------------------------------
 */
unsigned long switzer_carrier_get_wic_i2c_base (int slot)
    __attribute__((weak, alias("__switzer_carrier_get_wic_i2c_base")));
unsigned long __switzer_carrier_get_wic_i2c_base(int slot)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_carrier_get_wic_ngio
 * Description: older platfroms are not support switzer-carrier
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return 0
 * -------------------------------------------------------------------
 */
unsigned long switzer_carrier_get_wic_ngio (int slot)
    __attribute__((weak, alias("__switzer_carrier_get_wic_ngio")));
unsigned long __switzer_carrier_get_wic_ngio(int slot)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_carrier_get_wic_i2c_quack
 * Description: older platfroms are not support switzer-carrier
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return NULL
 * -------------------------------------------------------------------
 */
void* switzer_carrier_get_wic_i2c_quack (int slot)
    __attribute__((weak, alias("__switzer_carrier_get_wic_i2c_quack")));
void* __switzer_carrier_get_wic_i2c_quack(int slot)
{
    return NULL;
}

/*-------------------------------------------------------------------
 *
 * Function : sm_slot
 * Description: return the current testing SM card slot.
 *
 * INPUT:  none
 * OUTPUT: number of testing SM slot.
 * -------------------------------------------------------------------
 */
static int sm_slot(void)
{
    int test_slot = get_ngio_testing_now();
    int sm_slot = 0;

    switch (test_slot) {
    case NOW_TESTING_SM1:
    case NOW_TESTING_SM2:
    case NOW_TESTING_SM3:
    case NOW_TESTING_SM4:
    case NOW_TESTING_SM5:
        sm_slot = test_slot + 1;
        break;
    case NOW_TESTING_SM1_NIM1:
    case NOW_TESTING_SM1_NIM2:
        sm_slot = 1;
        break;
    case NOW_TESTING_SM2_NIM1:
    case NOW_TESTING_SM2_NIM2:
        sm_slot = 2;
        break;
    default:
        cterr('f', 0, "SM slot info not correct");
        break;
    }
    return sm_slot;
}

/*-------------------------------------------------------------------
 *
 * Function : get_sm_dc_wic_i2c_ctrl
 * Description: get SM daughter card i2c ctrl no
 *
 * INPUT:  daughter card slot in SM card
 * OUTPUT: return sm dc i2c ctrl
 * -------------------------------------------------------------------
 */
uint8_t get_sm_dc_wic_i2c_ctrl(int slot)
{
    uint8_t i2c_ctrl = 0;
    struct ngio_intf_t *ngio;

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(sm_slot());

    switch (ngio->id) {
    case C_SM_NIM_ADPT:
        i2c_ctrl = switzer_carrier_get_wic_i2c_ctrl(slot);
        break;
    default:
        cterr('f', 0, "Card type %d not support", ngio->id);
        break;
    }
    return i2c_ctrl;
}

/*-------------------------------------------------------------------
 *
 * Function : get_sm_dc_wic_i2c_base
 * Description: get SM daughter card i2c base addr
 *
 * INPUT:  daughter card i2c ctrl no
 * OUTPUT: return sm dc i2c base addr
 * -------------------------------------------------------------------
 */
unsigned long get_sm_dc_wic_i2c_base(int i2c_ctrl)
{
    unsigned long i2c_base = 0;
    struct ngio_intf_t *ngio;
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(sm_slot());

    switch (ngio->id) {
    case C_SM_NIM_ADPT:
        i2c_base = switzer_carrier_get_wic_i2c_base(i2c_ctrl);
        break;
    default:
        cterr('f', 0, "Card type %d not support", ngio->id);
        break;
    }
    return i2c_base;
}

/*-------------------------------------------------------------------
 *
 * Function : get_sm_dc_wic_ngio
 * Description: get SM daughter card ngio pointer
 *
 * INPUT:  daughter card slot in SM card
 * OUTPUT: return sm dc ngio pointer
 * -------------------------------------------------------------------
 */
unsigned long get_sm_dc_wic_ngio(int slot)
{
    unsigned long wic_ngio = 0;
    struct ngio_intf_t *ngio;
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(sm_slot());

    switch (ngio->id) {
    case C_SM_NIM_ADPT:
        wic_ngio = switzer_carrier_get_wic_ngio(slot);
        break;
    default:
        cterr('f', 0, "Card type %d not support", ngio->id);
        break;
    }
    return wic_ngio;
}

/*-------------------------------------------------------------------
 *
 * Function : get_sm_dc_wic_i2c_quack
 * Description: get SM daughter card i2c act2 pointer
 *
 * INPUT:  daughter card slot in SM card
 * OUTPUT: return sm dc i2c act2 pointer
 * -------------------------------------------------------------------
 */
void* get_sm_dc_wic_i2c_quack(int slot)
{
    void *i2c_p = NULL;
    struct ngio_intf_t *ngio;
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(sm_slot());

    switch (ngio->id) {
    case C_SM_NIM_ADPT:
        i2c_p = switzer_carrier_get_wic_i2c_quack(slot);
        break;
    default:
        cterr('f', 0, "Card type %d not support", ngio->id);
        break;
    }
    return i2c_p;
}


/******** History ******** 
$Log: platform_slot.c,v $
Revision 1.45  2021/04/12 13:36:45  xiaolaya
*** empty log message ***

Revision 1.44  2021/02/24 03:46:27  xiaolaya
Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie

Revision 1.43  2021/01/12 06:47:18  xiaolaya
swizter-carrier daughter card eeprom access bug fix2

Revision 1.42  2021/01/12 04:04:58  xiaolaya
switzer-carrier daughter card eeprom access bug fix

Revision 1.41  2020/05/22 02:28:34  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.40  2020/03/31 12:39:24  jiajliu
CSCvt62482: Fix PVDM (DC) failure on Curie2RU NIM2 Fortitude and Grimlock

Revision 1.39  2020/03/13 12:13:28  letsai
Merge Grimlock NIM to maintrunk

Revision 1.38  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.37  2019/11/12 03:09:20  alpeng
per MFG request, remove extra message

Revision 1.36  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.35  2019/07/19 08:34:08  alpeng
support sm testcard w/ bcm57412

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
